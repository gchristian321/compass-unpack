#include <set>
#include <iostream>
#include <cassert>
#include <memory>
#include <TFile.h>
#include <TTree.h>
#include <TNDArray.h>
#include <TObjArray.h>
#include <TObjString.h>
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
	fTree(nullptr),
	fSaveAsDouble(kFALSE)
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
		throw runtime_error("ROOT input not implemented for ROOT_SIMPLE output");
	}
	else if(fInputFileType == typeid(InputFileBin).hash_code()) {
		//
		// Binary input file
		std::set<Int_t> theBoards;
		std::cout << "\nBinary file -- Found Board Channel Combos:\n";
		auto boardChannel = InputFileBin::GetBoardChannelCombos(fInputFileDir);
		for(size_t id=0; id< boardChannel.size(); ++id){
			theBoards.insert(boardChannel[id].first);
//			fCombos.emplace(make_pair(boardChannel[id].first, boardChannel[id].second), id);
			fCombos.emplace(make_pair(0, boardChannel[id].second), id); // todo fix this! --> need to recognize board # consistently!
			cout << "--> " << boardChannel[id].first << ", " << boardChannel[id].second << "\n";
		}
		cout << "--------------------\n";
		if(theBoards.size() != 1) {
			throw std::invalid_argument(
				"ERROR: can only use ROOT_SIMPLE with a single board (need to fix this...)!");
		}
	}
	else {
		//
		// invalid directory - neither ROOT or binary files found
		std::cerr << "ERROR: invalid input directory!\n";
		exit(1);
	}
}

void cu::EventHandlerRootSimple::ReadChannelMapFile(const std::string& filename)
{
	cout << "\n\n---- CHANNEL MAPPINGS ----\n";
	string line;
	ifstream ifs(filename.c_str());
	while(getline(ifs, line)) {
		if(line[0] == '#') { continue; } // comment lines with leading #
		unique_ptr<TObjArray> tok (TString(line.c_str()).Tokenize(","));
		if(tok->GetEntries() != 3) {
			throw runtime_error(
				"Invalid channel map file: must have three entries <board,channel,parameter> "
				"per active line (comment lines with leading #)");
		}
		UShort_t board   = static_cast<TObjString*>(tok->At(0))->GetString().Atoi();
		UShort_t channel = static_cast<TObjString*>(tok->At(1))->GetString().Atoi();
		string parameter = static_cast<TObjString*>(tok->At(2))->GetString().Data();
		fChannelMap.emplace(make_pair(board,channel), parameter);
		cout << "<" << board << ", " << channel << ">  -->  \"" << parameter << "\"\n";
	}
	cout << "---------------------------\n";
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

		auto it = fChannelMap.empty() ?
			fChannelMap.end() : fChannelMap.find(make_pair(board,channel));
		
		if(it == fChannelMap.end()) { // no mapping -- use defaults
			fTree->Branch(Form("E_%i_%i",board,channel),  &(vEnergy.at(id)), Form("E_%i_%i/s",board,channel));
			fTree->Branch(Form("ES_%i_%i",board,channel), &(vEnergyShort.at(id)), Form("ES_%i_%i/s",board,channel));
			fTree->Branch(Form("T_%i_%i",board,channel),  &(vTimestamp.at(id)), Form("T_%i_%i/L",board,channel));
			fTree->Branch(Form("Flg_%i_%i",board,channel),&(vFlags.at(id)), Form("Flg_%i_%i/I",board,channel));
			fTree->Branch(Form("W_%i_%i",board,channel),  &(vWaveforms.at(id)));

			fBranchNames[id] = {
				Form("E_%i_%i",board,channel),
				Form("ES_%i_%i",board,channel),
				Form("T_%i_%i",board,channel),
				Form("Flg_%i_%i",board,channel),
				Form("W_%i_%i",board,channel)
			};
		} else { // mapping found -- use it
			const string& parname = it->second;
			fTree->Branch(Form("%s_E",parname.c_str()),  &(vEnergy.at(id)), Form("%s_E/s",parname.c_str()));
			fTree->Branch(Form("%s_ES",parname.c_str()), &(vEnergyShort.at(id)), Form("%s_ES/s",parname.c_str()));
			fTree->Branch(Form("%s_T",parname.c_str()),  &(vTimestamp.at(id)), Form("%s_T/L",parname.c_str()));
			fTree->Branch(Form("%s_Flg",parname.c_str()),&(vFlags.at(id)), Form("%s_Flg/I",parname.c_str()));
			fTree->Branch(Form("%s_W",parname.c_str()),  &(vWaveforms.at(id)));
			
			fBranchNames[id] = {
				Form("%s_E",parname.c_str()),
				Form("%s_ES",parname.c_str()),
				Form("%s_T",parname.c_str()),
				Form("%s_Flg",parname.c_str()),
				Form("%s_W",parname.c_str())
			};
		}
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
				size_t ipoint = 0;
				for(const auto& point : event->GetWaveform()){
					if(fFixWaves == false || ipoint%2 == 0) {
						vWaveforms.at(id).push_back(point);
					}
					++ipoint;
				}
			}
			++nEventsSaved;
		}
		if(fSaveAsDouble) {
			fEventData[fBranchNames[id][0]] = vEnergy.at(id);
			fEventData[fBranchNames[id][1]] = vEnergyShort.at(id);
			fEventData[fBranchNames[id][2]] = vTimestamp.at(id);
			fEventData[fBranchNames[id][3]] = vFlags.at(id);
//				fEventData[fBranchNames[id][4]] = vWaveform.at(id);
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
