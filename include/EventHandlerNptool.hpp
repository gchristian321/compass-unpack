#ifndef CU_EVENT_HANDLER_NPTOOL_HPP
#define CU_EVENT_HANDLER_NPTOOL_HPP

#ifdef HAVE_NPLIB // Define real class w/ nplib dependence
#include "EventHandler.hpp"

namespace compass_unpack {

class Event;
class WaveformProcessor;

class EventHandler {
public:
	EventHandler() {}
	virtual ~EventHandler() {}
	virtual void BeginningOfRun() = 0;
	virtual void BeginningOfEvent() = 0;
	virtual Long64_t HandleEvent
	(Long64_t eventNo, const std::vector<std::shared_ptr<Event> >&) = 0;
	virtual void EndOfEvent() = 0;
	virtual void EndOfRun() = 0;
};

}

#else // Define dummy class
#include "EventHandler.hpp"

namespace compass_unpack {

class EventHandlerNptool {
private: // make creation impossible
	EventHandlerNptool() {}
public:
	virtual ~EventHandlerNptool() {}
	virtual void BeginningOfRun() {}
	virtual void BeginningOfEvent() {}
	virtual Long64_t HandleEvent
	(Long64_t eventNo, const std::vector<std::shared_ptr<Event> >&)
		{ return 0; }
	virtual void EndOfEvent() {}
	virtual void EndOfRun() {}
};

}

#endif


#endif
