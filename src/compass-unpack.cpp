#include <deque>
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>
#include <TROOT.h>
#include <TChain.h>
#include <TSystem.h>
#include "json/json.h"
#include "Event.hpp"
#include "LockGuard.hpp"
#include "StatusBar.hpp"
#include "TTreeMerger.hpp"
#include "InputFileBin.hpp"
#include "EventHandlerRoot.hpp"

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

}

#define LOCK_STD std::lock_guard<std::mutex> lock_(::gMutex)
#define LOCK_STD_PTR std::unique_ptr<std::lock_guard<std::mutex> > (new std::lock_guard<std::mutex>(::gMutex))

int main(int argc, char** argv)
{
	if(argc != 2) {
		std::cout << "usage: compass-unpack <config.json>\n";
		return 1;
	}

	Json::Value config;
  std::string configFileName = argv[1];
  std::ifstream configStream(configFileName.c_str());
  configStream >> config;
  configStream.close();

	std::string input_dir;
	int runnum,nthreads;
	Long64_t kMatchWindow = 20e6;
	Long64_t kMaxTime = 10e12;
	std::string outputFile("matched.root");
	std::string treeName("MatchedData");
	std::string treeTitle("Matched CoMPASS Data");
	
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
			kMatchWindow = it->asInt()*1e6;
		}
		else if(it.key().asString() == "maxBufferTime") {
			kMaxTime = it->asInt()*1e12;
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
	}

	std::string runDir =
		Form("%s/DAQ/run_%i", input_dir.c_str(), runnum);
	std::string inputFileDir =
		Form("%s/UNFILTERED", runDir.c_str());

	if(std::string(gSystem->DirName(outputFile.c_str())) == ".") {
		// no directory specified, put in run dir
		std::string of = outputFile;
		outputFile = runDir + "/" + of;
	}
	
	cu::InputFileBin in(inputFileDir);
	gStatusBar.Reset(in.GetTotalEvents());

	
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
	
	Long64_t numread(0), numwritten(0);
	auto readerLoop = [&](){
		std::vector<std::shared_ptr<Event> > vectorOfMatches;

		Event lastEvent;
		Long64_t nread = 0;
		do {
			auto evt = std::make_shared<cu::Event>();
			nread = in.ReadEvent(evt);

			if(nread == -1) { // we are done, flush buffer
				// take the lock and append to shared event queue
				LOCK_STD;
				evtQueue.emplace_back(vectorOfMatches);				
			} else { // we have an event!
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
					{	LOCK_STD;
						evtQueue.emplace_back(vectorOfMatches);
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
		
		std::unique_ptr<EventHandler> handler
		(new EventHandlerRoot
		 (fn.c_str(), treeName,	treeTitle));
		
		handler->BeginningOfRun();
		
		while(true) {
			{
				LOCK_STD;
				if(doneReading && evtQueue.empty()) {
					break;
				}
			}
			Long64_t thisEvent;
			std::vector<std::shared_ptr<Event> > matches;
			auto lock = LOCK_STD_PTR;
			if(!evtQueue.empty()) {
				matches = *evtQueue.begin();
				evtQueue.erase(evtQueue.begin());
				thisEvent = eventNo++;
				lock.reset(nullptr);

				//
				handler->BeginningOfEvent();
				handler->HandleEvent(thisEvent, matches);
				handler->EndOfEvent();

				//
				lock.reset(new std::lock_guard<std::mutex>(::gMutex));
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


	std::cout << "\n----------- Unpacking Events (" << nthreads << " threads) -----------";
	readerThread.join();
	for (auto& ht : handlerThread) { ht.join(); }
	
	std::cout << "\n\n----------- Summary -----------\n";
	std::cout << "Events read: " << numread << "\n" <<
		"Events written: " << numwritten << "\n" <<
		"Events in queue: " << evtQueue.size() << "\n";
}
