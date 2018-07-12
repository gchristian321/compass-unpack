#include <limits>
#include <string>
#include <iostream>
#include <TFile.h>
#include <TChain.h>
#include <TSystem.h>
#include "Event.hpp"
#include "InputFileRoot.hpp"

namespace cu = compass_unpack;

namespace {
  struct compare_filenames {
    std::string base_;
    compare_filenames(const std::string& b): base_(b) {}
    bool operator() (const std::string& lhs, const std::string& rhs) {
      if(lhs == base_) { return true; }
      size_t baseSize = base_.find(".root");
      size_t lSize = lhs.find(".root");
      size_t rSize = rhs.find(".root");
    
      std::string digitsL = lhs.substr(baseSize+1, lSize-(baseSize+1));
      std::string digitsR = rhs.substr(baseSize+1, rSize-(baseSize+1));
      return atol(digitsL.c_str()) < atol(digitsR.c_str());
    }
  };

  std::vector<std::string> GetRunsInDir(const std::string& dir){
    TString files;
    if(dir != "") {
      files = gSystem->GetFromPipe( (std::string("ls ") + dir + "/compass_run*.root").c_str() );
    } else {
      files = gSystem->GetFromPipe( (std::string("ls ") + "compass_run*.root").c_str() );
    }
    std::unique_ptr<TObjArray> filesTokenized(files.Tokenize("\n"));
    std::vector<std::string> filesOut(filesTokenized->GetEntries());
    for(int i=0; i< filesTokenized->GetEntries(); ++i){
      filesOut[i] = static_cast<TObjString*>(filesTokenized->At(i))->GetString().Data();
    }
    std::sort(filesOut.begin(), filesOut.end(), compare_filenames
	      (*std::min_element(filesOut.begin(), filesOut.end())) );
    return filesOut;
  }
}

cu::InputFileRoot::InputFileRoot(const std::string& run_directory):
  fBoard(0),fChannel(0),fEnergy(0),fEnergyShort(0),fFlags(0)
{
  std::vector<std::string> runs = GetRunsInDir(run_directory);
  fChain = new TChain("Data");
  std::cout << "\n----------- Chaining Runs -----------\n";
  for(const auto& r : runs) {
    fChain->AddFile(r.c_str());
    std::cout << r << "\n";
  }

  fChain->SetBranchAddress("Board",&fBoard);
  fChain->SetBranchAddress("Channel",&fChannel);
  fChain->SetBranchAddress("Energy",&fEnergy);
  if(fChain->GetBranch("EnergyShort")){
    fChain->SetBranchAddress("EnergyShort",&fEnergyShort);
  }
  fChain->SetBranchAddress("Timestamp",&fTimestampUnsigned);
  fChain->SetBranchAddress("Flags",&fFlags);

  fEventNo = 0;
}

cu::InputFileRoot::~InputFileRoot()
{
  fChain->Delete();
}

Long64_t cu::InputFileRoot::GetTotalEvents() const
{
  return fChain->GetEntries();
}


Long64_t cu::InputFileRoot::ReadEvent(cu::Event& event)
{
  if(fEventNo < fChain->GetEntries()) {
    // Read entry from tree and increment event number
    fChain->GetEntry(fEventNo++);
    if(fTimestampUnsigned > std::numeric_limits<Long64_t>::max()) {
      std::cerr << "ERROR:: timestamp value overflows 64-bit signed max! "
		<< "(Entry: " << fEventNo-1 << "\n";
      exit(1);
    }

    // Copy read parameters to event class
    event.fBoard       = fBoard;
    event.fChannel     = fChannel;
    event.fEnergy      = fEnergy;
    event.fEnergyShort = fEnergyShort;
    event.fTimestamp   = fTimestampUnsigned;
    event.fFlags       = fFlags;
    event.fWaveform.clear();

    // return new event number
    return fEventNo;
  } else {

    // We are done with the chain, return -1
    return -1;
  }
}
