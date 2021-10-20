#include <deque>
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>
#include <TROOT.h>
#include <TChain.h>
#include <TSystem.h>
#include <TString.h>
#include "json/json.h"
#include "Event.hpp"
#include "LockGuard.hpp"
#include "StatusBar.hpp"
#include "TTreeMerger.hpp"
#include "InputFileBin.hpp"
#include "InputFileRoot.hpp"
#include "EventHandlerRoot.hpp"
#include "EventHandlerRootSimple.hpp"
#include "EventHandlerNptool.hpp"
#include "EventHandlerPlugin.hpp"
#include "SettingsReader.hpp"

namespace cu = compass_unpack;
using namespace compass_unpack;
using namespace std;



#include <set>
#include <TRandom3.h>

typedef std::deque<std::vector<std::shared_ptr<Event> > > Queue_t;

namespace {
struct CompareEventPointer {
	bool operator()(const std::shared_ptr<Event>& l, const std::shared_ptr<Event>& r)
		{ return *l < *r; }
};

template<class T> Long64_t LengthInTime(const T& t)
{
	return t.size() < 2 ? 0 : t.back()->GetTimestamp() - t.front()->GetTimestamp();
}

template<class T> Long64_t TimeDiff(const T& t1, const T& t2)
{
	return abs(t2->GetTimestamp() - t1->GetTimestamp());
}

std::mutex gMutex;

typedef std::unique_ptr<std::lock_guard<std::mutex> > LockGuard;
inline LockGuard MakeLockGuard() { LockGuard l(new std::lock_guard<std::mutex>(::gMutex)); return std::move(l); }
inline void ReleaseLockGuard(LockGuard& l) { l.reset(nullptr); }
inline void ResetLockGuard(LockGuard& l) { l.reset(new std::lock_guard<std::mutex>(::gMutex)); }
}

int main(int argc, char** argv)
{
	auto showHelp = [](int retcode){
		std::cout << "usage: compass-unpack <config.json>\n";
		return retcode;
	};
	if(argc != 2) {
		return showHelp(1);
	} else {
		for(int i=1; i< argc; ++i){
			if(std::string(argv[i]) == "-h" ||
				 std::string(argv[i]) == "--help" ||
				 std::string(argv[i]) == "-help"
				) {
				return showHelp(1);
			}
		}
	}

	Json::Value config;
  std::string configFileName = argv[1];
  std::ifstream configStream(configFileName.c_str());
  configStream >> config;
  configStream.close();

	std::string input_dir;
	int runnum, nthreads(1);
	Long64_t kMatchWindow = 20e6;
	std::string outputFile("matched.root");
	std::string treeName("MatchedData");
	std::string treeTitle("Matched CoMPASS Data");
	std::string outputType("ROOT");
	std::string pluginFile("");
	std::string channelMapFile("");
	std::string detConfig("");
	bool fixWaves = false;
	
	for(Json::Value::iterator it = config.begin();it!=config.end();it++) {
		if(false) {  }
		else if(it.key().asString() == "projectDirectory") {
			input_dir = it->asString();
		}
		else if(it.key().asString() == "runNumber") {
			runnum = it->asInt();
		}
		else if(it.key().asString() == "numThreads") {
			nthreads = it->asInt();
		}
		else if(it.key().asString() == "matchWindow") {
			kMatchWindow = it->asDouble()*1e6;
		}
		else if(it.key().asString() == "outputFile") {
			outputFile = it->asString();
		}
		else if(it.key().asString() == "outputTreeName") {
			treeName = it->asString();
		}
		else if(it.key().asString() == "outputTreeTitle") {
			treeTitle = it->asString();
		}
		else if(it.key().asString() == "outputType") {
			outputType = ToUpper(TString(it->asString().c_str())).Data();
		}
		else if(it.key().asString() == "pluginFile") {
			pluginFile = it->asString();
		}
		else if(it.key().asString() == "channelMapFile") {
			channelMapFile = it->asString();
		}
		else if(it.key().asString() == "fixWaves") {
			fixWaves = it->asBool();
		}
		else if(it.key().asString() == "detectorConfig") {
			detConfig = it->asString();
		}
	}

	cout << "Match Window: " << kMatchWindow << " us" << endl;
	
	if(nthreads != 1) {
		std::stringstream err;
		err << "ERROR: " << nthreads << " specified in config file, but"
				<< " multi-threding is currently NOT WORKING. Please set "
				<< "\"numThreads\" to 1 and try again.";
		throw std::invalid_argument(err.str().c_str());
	}
	
	std::string runDir =
		Form("%s/DAQ/run_%i", input_dir.c_str(), runnum);
	std::string inputFileDir =
		Form("%s/UNFILTERED", runDir.c_str());

	SettingsReader settings(runDir.c_str());

	if(std::string(gSystem->DirName(outputFile.c_str())) == ".") {
		// no directory specified, put in run dir
		std::string of = outputFile;
		outputFile = runDir + "/" + of;
	}
	
	std::unique_ptr<cu::InputFile> in(nullptr);
	if(true) {
		// first try a ROOT input
		in.reset(new InputFileRoot(inputFileDir));
		if(in->Good() && settings.GetFileFormat() != "ROOT") {
			throw runtime_error(
				string("Deduced ROOT file type but settings.xml says ") + settings.GetFileFormat().Data());
		}			
	}
	if(!in->Good()) {
		// then try binary input
		in.reset(new InputFileBin(inputFileDir, settings));
		if(in->Good() && settings.GetFileFormat() != "BIN") {
			throw runtime_error(
				string("Deduced BIN file type but settings.xml says ") + settings.GetFileFormat().Data());
		}			
	}
	if(!in->Good()) {
		// invalid directory - neither ROOT or binary files found
		std::cerr << "ERROR: invalid input directory!\n";
		exit(1);
	}
	gStatusBar.Reset(in->GetTotalEvents());

	
	Queue_t evtQueue;	
	std::atomic<bool> doneReading(false);

	auto verifyOrder = [](Event& lastEvent, const Event& thisEvent){
		if(lastEvent < thisEvent) { // okay!
			lastEvent.fBoard = thisEvent.fBoard;
			lastEvent.fChannel = thisEvent.fChannel;
			lastEvent.fTimestamp = thisEvent.fTimestamp;	
		} else {
			std::cerr << "ERROR: events received out of order!\n";
			std::cerr << "Last Event:\n"; lastEvent.Print();
			std::cerr << "This Event:\n"; thisEvent.Print();
			exit(1);
		}
	};
	
	Long64_t numread(0), numwritten(0), nummatched(0);
	auto readerLoop = [&](){
		std::vector<std::shared_ptr<Event> > vectorOfMatches;

		Event lastEvent;
		Long64_t nread = 0;
		do {
			auto evt = std::make_shared<cu::Event>();
			nread = in->ReadEvent(evt);

			if(nread == -1) { // we are done, flush buffer
				// take the lock and append to shared event queue
				auto lock_ = MakeLockGuard();
				evtQueue.emplace_back(vectorOfMatches);
				++nummatched;
			} else if(nread > 0) { // we have an event!
				//
				// Verify events always received in order
				if(numread++) { verifyOrder(lastEvent, *evt); }
				//
				// Insert into vector of matched events
				if(vectorOfMatches.empty() || TimeDiff(evt, vectorOfMatches.front()) < kMatchWindow) {
					// We are either starting fresh, or have a match
					vectorOfMatches.emplace_back(evt);
				} else {
					// Not a match, push the existing match vector and start fresh
					//
					// But first wait until shared queue is down to a reasonable size
					while(evtQueue.size() > 10000) {
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
					}
					// Then take the lock and append to shared event queue
					{	auto lock_ = MakeLockGuard();
						evtQueue.emplace_back(vectorOfMatches);
						++nummatched;
					}
					// Clear local vector of matches
					vectorOfMatches.clear();
					vectorOfMatches.emplace_back(evt);
				}
			}
		} while(nread != -1);
		
		doneReading = true;
	};

	Long64_t eventNo = 0;
	auto handlerLoop = [&](int n){
		std::string fn = n < 0 ? outputFile :
		TTreeMerger::GetThreadFile(n, outputFile);
		
		std::unique_ptr<EventHandler> handler(nullptr);

		if     (outputType == "ROOT") {
			handler.reset(new EventHandlerRoot(fn.c_str(), treeName,	treeTitle));
		}
		else if(outputType == "ROOT_SIMPLE") {
			handler.reset(new EventHandlerRootSimple(fn.c_str(), treeName,	treeTitle));
			static_cast<EventHandlerRootSimple*>(handler.get())-> ReceiveInputFileInformation(
				typeid(*(in.get())), inputFileDir
				);
			if(!channelMapFile.empty()) {
				static_cast<EventHandlerRootSimple*>(handler.get())->
												 ReadChannelMapFile(channelMapFile);
			}
		}
		else if(outputType == "NPTOOL") {
			throw invalid_argument(
				"NPTOOL outputType no longer supported!");
			handler.reset(new EventHandlerNptool(fn.c_str(), treeName, treeTitle, detConfig));
		}
		else if(outputType == "PLUGIN") {
			if(pluginFile.empty()) {
				throw std::invalid_argument(
					"ERROR in json file: PLUGIN output type chosen, but no "
					"\"pluginFile\" specified."
					);
			}
			handler.reset(new EventHandlerPlugin(pluginFile,fn.c_str(), treeName,	treeTitle));
			static_cast<EventHandlerRootSimple*>(handler.get())-> ReceiveInputFileInformation(
				typeid(*(in.get())), inputFileDir
				);
			if(!channelMapFile.empty()) {
				static_cast<EventHandlerRootSimple*>(handler.get())->
												 ReadChannelMapFile(channelMapFile);
			}
		}
		else {
			std::stringstream err;
			err << "ERROR: invalid output type: \"" << outputType << "\", exiting!";
			throw std::runtime_error(err.str().c_str());
		}
		if(fixWaves) { handler->SetFixWavesTrue(); }

		Long64_t thisEvent;
		std::vector<std::shared_ptr<Event> > matches;
		
		handler->BeginningOfRun();
		while(true) {
			auto lock_ = MakeLockGuard();
			if(doneReading && evtQueue.empty()) {
				break;
			}
			if(!evtQueue.empty()) {
				// copy to local buffer & release the lock
				matches = *evtQueue.begin();
				evtQueue.erase(evtQueue.begin());
				thisEvent = eventNo++;
				ReleaseLockGuard(lock_);
					
				// handle the event from local buffer
				handler->BeginningOfEvent();
				handler->HandleEvent(thisEvent, matches);
				handler->EndOfEvent();
					
				// retake the lock and update printer
				ResetLockGuard(lock_);
				gStatusBar.Incr(matches.size());
				numwritten += (matches.size());
			}
		}
		handler->EndOfRun();
	};


	std::thread readerThread (readerLoop);
	std::vector<std::thread> handlerThread;
	TTreeMerger treeMerger(nthreads,outputFile,treeName,true);

	if(nthreads <= 1) {
		handlerThread.push_back(std::thread(handlerLoop, -1));
	}
	else {
		ROOT::EnableThreadSafety();
		for(int i=0; i< nthreads; ++i){
			handlerThread.push_back(std::thread(handlerLoop, i));
		}
	}


	std::string threads_ = nthreads == 1 ? "thread" : "threads";
	std::cout << "\n----------- Unpacking Events (" << nthreads << " " << threads_ << ") -----------";
	readerThread.join();
	for (auto& ht : handlerThread) { ht.join(); }
	
	std::cout << "\n\n----------- Summary -----------\n";
	std::cout << "Events read: " << numread << " (unmatched), " << nummatched <<" (matched)\n" <<
		"Events written: " << numwritten << " (unmatched), " << eventNo <<" (matched)\n" <<
		"Events in queue: " << evtQueue.size() << "\n";
}
