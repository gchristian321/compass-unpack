#ifndef CU_EVENT_HANDLER_ROOT_HPP
#define CU_EVENT_HANDLER_ROOT_HPP
#include <memory>
#include <TFile.h>
#include <TTree.h>
#include "EventHandler.hpp"


class TClonesArray;

namespace compass_unpack {

class EventHandlerRoot: public EventHandler {
public:
	EventHandlerRoot
	(const std::string& outputFileName, 
	 const std::string& outputTreeName, 
	 const std::string& outputTreeTitle);
	virtual ~EventHandlerRoot();
	virtual void BeginningOfRun();
	virtual void BeginningOfEvent();
	virtual Long64_t HandleEvent(const std::vector<std::shared_ptr<Event> >& matches);
	virtual void EndOfEvent();
	virtual void EndOfRun();

private:
	EventHandlerRoot(const EventHandlerRoot&) {}
	const EventHandlerRoot& operator=(const EventHandlerRoot&) { return *this; }
	
private:
	std::string fFilename, fTreename, fTreetitle;
	std::unique_ptr<TFile> fFile;
	TTree* fTree;

	std::vector<UShort_t> vBoard;
	std::vector<UShort_t> vChannel;
	std::vector<UShort_t> vEnergy;
	std::vector<Long64_t> vTimestamp;
	std::vector<UInt_t>   vFlags;
	TClonesArray* vWaveforms;
};
  
}


#endif
