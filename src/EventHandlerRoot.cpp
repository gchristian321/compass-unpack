#include <iostream>
#include <cassert>
#include <TFile.h>
#include <TTree.h>
#include <TNDArray.h>
#include <TClonesArray.h>
#include "Event.hpp"
#include "EventHandlerRoot.hpp"

namespace cu = compass_unpack;

cu::EventHandlerRoot::EventHandlerRoot
(const std::string& outputFileName, 
 const std::string& outputTreeName, 
 const std::string& outputTreeTitle):
	fFilename(outputFileName),
	fTreename(outputTreeName),
	fTreetitle(outputTreeTitle),
	fFile(nullptr),
	fTree(nullptr),
	vWaveforms(nullptr)
{ }

cu::EventHandlerRoot::~EventHandlerRoot()
{
	if(vWaveforms != nullptr) { vWaveforms->Delete(); }
}

void cu::EventHandlerRoot::BeginningOfRun()
{
	fFile.reset(new TFile(fFilename.c_str(), "recreate"));
  assert(fFile.get() && !(fFile->IsZombie()));
  fTree = new TTree(fTreename.c_str(), fTreetitle.c_str());

	fTree->Branch("EventNo",  &fEventNo,  "EventNo/L");
  fTree->Branch("Board",    &vBoard);
  fTree->Branch("Channel",  &vChannel);
  fTree->Branch("Energy",   &vEnergy);
  fTree->Branch("Timestamp",&vTimestamp);
  fTree->Branch("Flags",    &vFlags);

	vWaveforms = new TClonesArray("TNDArrayT<UShort_t>");
  vWaveforms->BypassStreamer();
  fTree->Branch("Waveforms",&vWaveforms,256000,1);
}

void cu::EventHandlerRoot::BeginningOfEvent()
{
  // Clear output vectors
	fEventNo = -1;
  vBoard.clear();
  vChannel.clear();
  vEnergy.clear();
  vTimestamp.clear();
  vFlags.clear();
  vWaveforms->Clear();
}

Long64_t cu::EventHandlerRoot::HandleEvent
(Long64_t eventNo, const std::vector<std::shared_ptr<Event> >& matches)
{
  Int_t nWaves = 0; // number of waveforms graphs

	for(auto& event : matches) {		
		// Fill vectors
		vBoard.push_back(event->GetBoard());
		vChannel.push_back(event->GetChannel());
		vEnergy.push_back(event->GetEnergy());
		vTimestamp.push_back(event->GetTimestamp());
		vFlags.push_back(event->GetFlags());

		if(event->GetWaveform().size()) {
			// We have waveforms so save them as TClonesArray of TNDArrayT<UShort_t>
			//
			// Create new TNDArrayT to go in the TClonesArray
			TClonesArray& ar = *vWaveforms;
			Int_t npoints = event->GetWaveform().size();
			TNDArrayT<UShort_t>* array =
				new(ar[nWaves++]) TNDArrayT<UShort_t>(1, &npoints);
			
			size_t point = 0;
			for(size_t i=0; i< event->GetWaveform().size(); ++i){
				array->At(i) = event->GetWaveform()[i];				
			}
		}
	}
	fEventNo = eventNo;
  return vEnergy.size();
}

void cu::EventHandlerRoot::EndOfEvent()
{
  // Fill Tree
  fTree->Fill();
}

void cu::EventHandlerRoot::EndOfRun()
{
  fFile->Write();
	fFile.reset(nullptr);
	if(vWaveforms) { vWaveforms->Delete(); }
	vWaveforms = nullptr;
}
