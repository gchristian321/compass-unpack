#ifndef CU_EVENT_HANDLER_ROOT_SIMPLE_HPP
#define CU_EVENT_HANDLER_ROOT_SIMPLE_HPP
#include <map>
#include <utility>
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

private:
	EventHandlerRootSimple(const EventHandlerRootSimple&) {}
	const EventHandlerRootSimple& operator=(const EventHandlerRootSimple&) { return *this; }
	void FigureOutBoardChannelCombos();
	
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
};
  
}


#endif
