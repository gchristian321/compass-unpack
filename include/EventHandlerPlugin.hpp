#ifndef CU_EVENT_HANDLER_PLUGIN_HPP
#define CU_EVENT_HANDLER_PLUGIN_HPP
#include <map>
#include <string>
#include <TFile.h>
#include <TTree.h>
#include "EventHandlerRootSimple.hpp"


class TClonesArray;

namespace compass_unpack {

class EventHandlerPlugin: public EventHandlerRootSimple {
public:
	EventHandlerPlugin(
		const std::string& pluginFileName,
		const std::string& outputFileName,
		const std::string& outputTreeName,
		const std::string& outputTreeTitle);
	virtual ~EventHandlerPlugin();
	virtual void BeginningOfRun();
	virtual void BeginningOfEvent();
	virtual Long64_t HandleEvent
	(Long64_t eventNo,
	 const std::vector<std::shared_ptr<Event> >& matches);
	virtual void EndOfEvent();
	virtual void EndOfRun();

private:
	EventHandlerPlugin(const EventHandlerPlugin& eh):
		EventHandlerRootSimple(eh) {}
	const EventHandlerPlugin& operator=(const EventHandlerPlugin&)
		{ return *this; }
	
private:
	void(*fRunBegin)();
	void(*fEventBegin)();
	void(*fEventEnd)();
	void(*fRunEnd)();
	Long64_t(*fHandleEvent)(std::map<std::string, double>&);
};
  
}


#endif
