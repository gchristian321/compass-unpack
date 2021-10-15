#ifndef CU_EVENT_HANDLER_ROOT_SIMPLE_HPP
#define CU_EVENT_HANDLER_ROOT_SIMPLE_HPP
#include <map>
#include <array>
#include <utility>
#include <typeinfo>
#include <TFile.h>
#include <TTree.h>
#include "EventHandler.hpp"


namespace compass_unpack {

class EventHandlerRootSimple: public EventHandler {
public:
	EventHandlerRootSimple
	(const std::string& outputFileName, 
	 const std::string& outputTreeName, 
	 const std::string& outputTreeTitle);
	virtual ~EventHandlerRootSimple();
	virtual void BeginningOfRun();
	virtual void BeginningOfEvent();
	virtual Long64_t HandleEvent
	(Long64_t eventNo, const std::vector<std::shared_ptr<Event> >& matches);
	virtual void EndOfEvent();
	virtual void EndOfRun();
	void ReceiveInputFileInformation(const std::type_info& inputType, const std::string& inputFileDir);
	void ReadChannelMapFile(const std::string& filename);
	void SetSaveAsDouble(Bool_t save) { fSaveAsDouble = save; }
	
protected:
	EventHandlerRootSimple(const EventHandlerRootSimple&) {}
	const EventHandlerRootSimple& operator=(const EventHandlerRootSimple&) { return *this; }
	void FigureOutBoardChannelCombos();

protected:
	std::map<std::string, double> fEventData;
	
private:
	std::string fFilename, fTreename, fTreetitle;
	std::unique_ptr<TFile> fFile;
	TTree* fTree;

	std::vector<UShort_t> vEnergy;
	std::vector<UShort_t> vEnergyShort;
	std::vector<Long64_t> vTimestamp;
	std::vector<UInt_t>   vFlags;
	std::vector<std::vector<UShort_t> > vWaveforms;
	std::map<std::pair<UShort_t, UShort_t>, std::size_t> fCombos;
	size_t fInputFileType; //typeid hash code
	std::string fInputFileDir;
	std::map<std::pair<UShort_t, UShort_t>, std::string> fChannelMap;
	std::map<size_t, std::array<std::string,5> > fBranchNames;
	Bool_t fSaveAsDouble;
};
  
}


#endif
