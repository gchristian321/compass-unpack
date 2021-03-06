#include <limits>
#include <string>
#include <iostream>
#include <TFile.h>
#include <TChain.h>
#include <TSystem.h>
#include <TError.h>
#include <TObjString.h>
#include "Event.hpp"
#include "InputFileRoot.hpp"

namespace cu = compass_unpack;

namespace {

class ErrorIgnore{
	Int_t level_;
public:
	ErrorIgnore(Int_t level) : level_(gErrorIgnoreLevel) { gErrorIgnoreLevel = level; }
	~ErrorIgnore() { gErrorIgnoreLevel = level_; }
};

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
		ErrorIgnore ei(kError+1);
		files = gSystem->GetFromPipe( (std::string("ls ") + dir + "/compass_run*.root 2>/dev/null").c_str() );
	} else {
		ErrorIgnore ei(kError+1);
		files = gSystem->GetFromPipe( (std::string("ls ") + "compass_run*.root 2>/dev/null").c_str() );
	}
	if(files.EqualTo("")) {
		return std::vector<std::string>();
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
  fChain(new TChain("Data")),fBoard(0),fChannel(0),fEnergy(0),fEnergyShort(0),fFlags(0)
{
  std::vector<std::string> runs = GetRunsInDir(run_directory);
	if(runs.empty()) {
		fChain.reset();
	} else {
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
		fReturnedEvents = 0;
	}
}

cu::InputFileRoot::~InputFileRoot()
{
	;
}

Long64_t cu::InputFileRoot::GetTotalEvents() const
{
  return fChain->GetEntries();
}


Long64_t cu::InputFileRoot::ReadEvent(std::shared_ptr<compass_unpack::Event>& event)
{
	bool readEvent = false;
  if(fEventNo < fChain->GetEntries()) {
    // Read entry from tree and increment event number
		readEvent = true;
    fChain->GetEntry(fEventNo++);
    if(fTimestampUnsigned > std::numeric_limits<Long64_t>::max()) {
      std::cerr << "ERROR:: timestamp value overflows 64-bit signed max! "
								<< "(Entry: " << fEventNo-1 << "\n";
      exit(1);
    }

    // Copy read parameters to local event class
		std::shared_ptr<Event> nextEvent(new Event());
    nextEvent->fBoard       = fBoard;
    nextEvent->fChannel     = fChannel;
    nextEvent->fEnergy      = fEnergy;
    nextEvent->fEnergyShort = fEnergyShort;
    nextEvent->fTimestamp   = fTimestampUnsigned;
    nextEvent->fFlags       = fFlags;

		// push into local buffer
		fLocalBuffer.insert(nextEvent);
	}
	
	auto Pop = [&]() {
		event = *fLocalBuffer.begin();
		fLocalBuffer.erase(fLocalBuffer.begin());
		return ++fReturnedEvents;
	};
	
	if(!readEvent) {
		return fLocalBuffer.empty() ? -1 : Pop();
	}

	// pop earliest event if we have enough
	if((*fLocalBuffer.rbegin())->fTimestamp - (*fLocalBuffer.begin())->fTimestamp > 10e12) {
		return Pop();
	}

	return 0;
}
