#ifdef HAVE_NPLIB // only compile if we have nptool dependence
#include <iostream>
#include <TString.h>
#include <TObjString.h>
#include <TObjArray.h>
#include <TTree.h>
#include <TFile.h>
#include "Event.hpp"
#include "EventHandlerNptool.hpp"
using namespace std;


namespace cu = compass_unpack;

cu::EventHandlerNptool::EventHandlerNptool
(const std::string& outputFileName, 
 const std::string& outputTreeName, 
 const std::string& outputTreeTitle,
 const std::string& detectorConfigFile) :
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
(Long64_t eventNo, const std::vector<std::shared_ptr<Event> >& matches)
{
	fEventNo = eventNo;
	for(auto& event : matches) {
		auto match = fFpdMap.find(std::make_pair(event->fBoard, event->fChannel));
		if(match != fFpdMap.end()) {
			auto& SetEnergy = match->second.first;
			auto& SetTime = match->second.second;
			SetEnergy(event->fEnergy);
			SetTime(event->fTimestamp);
		} else {
			std::cerr << "WARNING: channel map not found for (board, channel): (" << event->fBoard << ", " << event->fChannel << ")\n";
		}
	}
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
    cout << "**** Reading configuration file : " << fDetectorConfig << " ****" << endl;  
  }
  else{
    cout << "**** ERROR: Configuration file : " <<  fDetectorConfig << " not found ****"<< endl;
    exit(1);
  }
	
  string key,buffer, detector,token;
  int value,channel,module;
  // Reading setting line
  infile >> key; 
  while(key!="MAP"){
    if(key == "ADC"){
      infile >> token >> value;
    } 
    else if(key == "TDC"){
      infile >> token >> value;
    }
		else if(key.compare(0,1,"%")==0){
		 infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n')  ;
	 }
	 else{
		 cout << "**** INFO : Token unknown : " << token << " ****" << endl;
	 }
    infile >> key ;
  }

	auto Tokenize = [](const std::string& token, UShort_t& row, UShort_t& col) {
		std::unique_ptr<TObjArray> tok(TString(token.c_str()).Tokenize("_"));
		if(tok->GetEntries() == 4) {
			row = atoi(string(((TObjString*)tok->At(1))->String()).substr(1).c_str());
			col = atoi(string(((TObjString*)tok->At(2))->String()).substr(1).c_str());
			return true;
		}
		return false;
	};
	
  while(infile >> key >> module >> buffer >> channel >> detector >> token){
    if(key.compare(0,1,"%")!=0){
      if(key == "ADC") {
				if(detector == "FpdTAMU") {
					UShort_t row, col;
					if(Tokenize(token, row, col)) {
						std::string component = token.substr(0, token.find("_"));
						//
						if(component == "AWIRE") {
							fFpdMap.emplace
								(std::make_pair(module, channel),
								 std::make_pair(std::bind(&TFPDTamuData::Set_AWire_E, fFpd, row, col, std::placeholders::_1),
																std::bind(&TFPDTamuData::Set_AWire_T, fFpd, row, col, std::placeholders::_1)
									 ) );
						}
						else if(component.substr(0, component.size()-1) == "MICRO") {
							UShort_t det = atoi(component.substr(5).c_str());
							fFpdMap.emplace
								(std::make_pair(module, channel),
								 std::make_pair(std::bind(&TFPDTamuData::Set_Micro_E, fFpd, det, row, col, std::placeholders::_1),
																std::bind(&TFPDTamuData::Set_Micro_T, fFpd, det, row, col, std::placeholders::_1)
									 ) );
						}
						else if(component == "DELTA") {
							fFpdMap.emplace
								(std::make_pair(module, channel),
								 std::make_pair(std::bind(&TFPDTamuData::Set_Delta_E, fFpd, row, std::placeholders::_1),
																std::bind(&TFPDTamuData::Set_Delta_T, fFpd, row, std::placeholders::_1)
									 ) );
						}																				
						else if(component == "PLAST") {
							fFpdMap.emplace
								(std::make_pair(module, channel),
								 std::make_pair(std::bind(&TFPDTamuData::Set_Plast_E, fFpd, col, std::placeholders::_1),
																std::bind(&TFPDTamuData::Set_Plast_T, fFpd, col, std::placeholders::_1)
									 ) );
						}																				
						else {
							std::cerr << "WARNING: unrecognized FPDTAMU component: \"" << component << "\", skipping...\n";
						}
					}
					else {
						std::cerr << "WARNING: unecognized detector: \"" << detector << "\", skipping...\n";
					}
				}
			}
		}
	}
}


#endif
