#ifdef HAVE_NPLIB // only compile if we have nptool dependence
#include <iostream>
#include <sstream>
#include <TString.h>
#include <TObjString.h>
#include <TObjArray.h>
#include <TTree.h>
#include <TFile.h>
#include "Event.hpp"
#include "EventHandlerNptool.hpp"
using namespace std;


namespace cu = compass_unpack;

namespace {
class DummySetter {
public:
	void Set_E(int,double){}
	void Set_T(int,double){}
};
DummySetter gDummySetter;
}

cu::EventHandlerNptool::EventHandlerNptool
(const string& outputFileName, 
 const string& outputTreeName, 
 const string& outputTreeTitle,
 const string& detectorConfigFile) :
	fFilename(outputFileName),
	fTreename(outputTreeName),
	fTreetitle(outputTreeTitle),
	fDetectorConfig(detectorConfigFile),
	fFile(nullptr),
	fTree(nullptr),
	fFpd(new TFPDTamuData()),
	fEventNo(0)
{  }

cu::EventHandlerNptool::~EventHandlerNptool()
{
	if(fFpd) { delete fFpd; fFpd = nullptr; }
}

void cu::EventHandlerNptool::BeginningOfRun()
{
	ParseConfig();

	fFile.reset(new TFile(fFilename.c_str(), "recreate"));
  assert(fFile.get() && !(fFile->IsZombie()));
  fTree = new TTree(fTreename.c_str(), fTreetitle.c_str());

	fTree->Branch("FpdTAMU","TFPDTamuData",&fFpd);
	fTree->Branch("EventNo",  &fEventNo,  "EventNo/L");
}

void cu::EventHandlerNptool::BeginningOfEvent()
{
	fEventNo = -1;
	fFpd->Clear();
}

Long64_t cu::EventHandlerNptool::HandleEvent
(Long64_t eventNo, const vector<shared_ptr<Event> >& matches)
{
	fEventNo = eventNo;
	for(auto& event : matches) {
		auto match = fFpdMap.find(make_pair(event->fBoard, event->fChannel));
		if(match != fFpdMap.end()) {
			auto& SetEnergy = match->second.first;
			auto& SetTime = match->second.second;
			SetEnergy(event->fEnergy);
			SetTime(event->fTimestamp);
		} else {
			cerr << "WARNING: channel map not found for (board, channel): (" << event->fBoard << ", " << event->fChannel << ")\n";
		}
	}
	return matches.size();
}

void cu::EventHandlerNptool::EndOfEvent()
{
  // Fill Tree
  fTree->Fill();
}

void cu::EventHandlerNptool::EndOfRun()
{
  fFile->Write();
	fFile.reset(nullptr);
	delete fFpd; fFpd = nullptr;
}

void cu::EventHandlerNptool::ParseConfig()
{
	ifstream infile(fDetectorConfig.c_str());
  if(infile.is_open()){
    cout << "\n**** Reading configuration file : " << fDetectorConfig << " ****" << endl;  
  }
  else{
    cout << "\n**** ERROR: Configuration file : " <<  fDetectorConfig << " not found ****"<< endl;
    exit(1);
  }
	
  string key,buffer,detector,token;
  int value,channel,module;
  // Ignore everything until "MAP"
  do {
		infile >> key; 
		infile.ignore(numeric_limits<streamsize>::max(), '\n');
  } while(key!="MAP");
	
	auto Tokenize = [](const string& token, string& component, UShort_t& row, UShort_t& col) {
		unique_ptr<TObjArray> tok(TString(token.c_str()).Tokenize("_"));
		if(tok->GetEntries() == 4) {
			component = ((TObjString*)tok->At(0))->String();
			row = atoi(string(((TObjString*)tok->At(1))->String()).substr(1).c_str());
			col = atoi(string(((TObjString*)tok->At(2))->String()).substr(1).c_str());
			return true;
		}
		return false;
	};

	UShort_t row, col;
	string line, component;
  while(std::getline(infile, line)) {
		// Ignore comments and parse into tokens
		istringstream iss(line.substr(0, line.find("%")));
		iss >> key >> module >> buffer >> channel >> detector >> token;

		// Process ADC keys to map
		if(key == "ADC") {
			if(detector == "FpdTAMU") {
				if(Tokenize(token, component, row, col)) {
					//
					if(component == "AWIRE") {
						fFpdMap.emplace
							(make_pair(module, channel),
							 make_pair(bind(&TFPDTamuData::Set_AWire_E, fFpd, row, col, placeholders::_1),
												 bind(&TFPDTamuData::Set_AWire_T, fFpd, row, col, placeholders::_1)
								 ) );
					}
					else if(component.substr(0, component.size()-1) == "MICRO") {
						UShort_t det = atoi(component.substr(5).c_str());
						fFpdMap.emplace
							(make_pair(module, channel),
							 make_pair(bind(&TFPDTamuData::Set_Micro_E, fFpd, det, row, col, placeholders::_1),
												 bind(&TFPDTamuData::Set_Micro_T, fFpd, det, row, col, placeholders::_1)
								 ) );
					}
					else if(component == "DELTA") {
						fFpdMap.emplace
							(make_pair(module, channel),
							 make_pair(bind(&TFPDTamuData::Set_Delta_E, fFpd, row, placeholders::_1),
												 bind(&TFPDTamuData::Set_Delta_T, fFpd, row, placeholders::_1)
								 ) );
					}																				
					else if(component == "PLAST") {
						fFpdMap.emplace
							(make_pair(module, channel),
							 make_pair(bind(&TFPDTamuData::Set_Plast_E, fFpd, col, placeholders::_1),
												 bind(&TFPDTamuData::Set_Plast_T, fFpd, col, placeholders::_1)
								 ) );
					}
					else if(component == "EMPTY") {
						fFpdMap.emplace
							(make_pair(module, channel),
							 make_pair(bind(&DummySetter::Set_E, gDummySetter, col, placeholders::_1),
												 bind(&DummySetter::Set_T, gDummySetter, col, placeholders::_1)
								 ) );
					}
					else {
						cerr << "WARNING: unrecognized FPDTAMU component: \"" << component << "\", skipping...\n";
					}
				}
				else {
					cerr << "WARNING: unecognized detector: \"" << detector << "\", skipping...\n";
				}
			}
		}
	}
}


#endif
