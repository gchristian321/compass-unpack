#include <iostream>
#include <cassert>
#include <TFile.h>
#include <TTree.h>
#include <TNDArray.h>
#include <TClonesArray.h>
#include "Event.hpp"
#include "InputFileBin.hpp"
#include "InputFileRoot.hpp"
#include "EventHandlerRootSimple.hpp"

namespace cu = compass_unpack;
using namespace std;

cu::EventHandlerRootSimple::EventHandlerRootSimple
(const string& outputFileName, 
 const string& outputTreeName, 
 const string& outputTreeTitle):
	fFilename(outputFileName),
	fTreename(outputTreeName),
	fTreetitle(outputTreeTitle),
	fFile(nullptr),
	fTree(nullptr)
{ }

cu::EventHandlerRootSimple::~EventHandlerRootSimple()
{
	;
}

void cu::EventHandlerRootSimple::ReceiveInputFileInformation(
	const std::type_info& inputType, const std::string& inputFileDir)
{
	fInputFileType = inputType.hash_code();
	fInputFileDir = inputFileDir;
}

void cu::EventHandlerRootSimple::FigureOutBoardChannelCombos()
{
	if(fInputFileType == typeid(InputFileRoot).hash_code()) {
		//
		// ROOT input file
		std::unique_ptr<cu::InputFile> in(new InputFileRoot(fInputFileDir));
		throw (12345);
		// need to implement
	}
	else if(fInputFileType == typeid(InputFileBin).hash_code()) {
		//
		// Binary input file
		std::cout << "\nBinary file -- Found Board Channel Combos:\n";
		auto boardChannel = InputFileBin::GetBoardChannelCombos(fInputFileDir);
		for(size_t id=0; id< boardChannel.size(); ++id){
			fCombos.emplace(make_pair(boardChannel[id].first, boardChannel[id].second), id);
			cout << "--> " << boardChannel[id].first << ", " << boardChannel[id].second << "\n";
		}
		cout << "--------------------\n";
	}
	else {
		//
		// invalid directory - neither ROOT or binary files found
		std::cerr << "ERROR: invalid input directory!\n";
		exit(1);
	}
}

void cu::EventHandlerRootSimple::BeginningOfRun()
{
	fFile.reset(new TFile(fFilename.c_str(), "recreate"));
  assert(fFile.get() && !(fFile->IsZombie()));
  fTree = new TTree(fTreename.c_str(), fTreetitle.c_str());

	FigureOutBoardChannelCombos();
	vEnergy.resize(fCombos.size());
	vEnergyShort.resize(fCombos.size());
	vTimestamp.resize(fCombos.size());
	vFlags.resize(fCombos.size());
	vWaveforms.resize(fCombos.size());
	
	for(const auto& bc : fCombos){
		const size_t id = bc.second;
		const UShort_t board = bc.first.first;
		const UShort_t channel = bc.first.second;
		
		fTree->Branch(Form("E_%i_%i",board,channel),  &(vEnergy.at(id)), Form("E_%i_%i/s",board,channel));
		fTree->Branch(Form("ES_%i_%i",board,channel), &(vEnergyShort.at(id)), Form("ES_%i_%i/s",board,channel));
		fTree->Branch(Form("T_%i_%i",board,channel),  &(vTimestamp.at(id)), Form("T_%i_%i/s",board,channel));
		fTree->Branch(Form("Flg_%i_%i",board,channel),&(vFlags.at(id)), Form("Flg_%i_%i/s",board,channel));
		fTree->Branch(Form("W_%i_%i",board,channel),  &(vWaveforms.at(id)));
	}
}

void cu::EventHandlerRootSimple::BeginningOfEvent()
{
	for(const auto& bc : fCombos){
		const size_t id = bc.second;
		vEnergy.at(id) = 0;
		vEnergyShort.at(id) = 0;
		vTimestamp.at(id) = 0;
		vFlags.at(id) = 0;
		vWaveforms.at(id).clear();
	}
}

Long64_t cu::EventHandlerRootSimple::HandleEvent
(Long64_t eventNo, const vector<std::shared_ptr<Event> >& matches)
{
	Long64_t nEventsSaved = 0;
	for(auto& event : matches) {
		auto bc = fCombos.find(make_pair(event->GetBoard(), event->GetChannel()));
		if(bc == fCombos.end()){
			cerr << "WARNING: found unmatched board, channel combo (" <<
				event->GetBoard() << ", " << event->GetChannel() <<
				")... Skipping event...\n";
			continue;
		}
		const size_t id = bc->second;
		// Take first hit only
		if(vTimestamp.at(id) == 0 || event->GetTimestamp() < vTimestamp.at(id)) {
			vEnergy.at(id) = event->GetEnergy();
			vEnergyShort.at(id) = event->GetEnergyShort();
			vTimestamp.at(id) = event->GetTimestamp();
			vFlags.at(id) = event->GetFlags();
			vWaveforms.at(id).clear();
			if(event->GetWaveform().size()) {
				vWaveforms.at(id).reserve(event->GetWaveform().size());
				for(const auto& point : event->GetWaveform()){
					vWaveforms.at(id).push_back(point);
				}
			}
			++nEventsSaved;
		}
	}
	return nEventsSaved;
}

void cu::EventHandlerRootSimple::EndOfEvent()
{
  // Fill Tree
  fTree->Fill();
}

void cu::EventHandlerRootSimple::EndOfRun()
{
  fFile->Write();
	fFile.reset(nullptr);
}
