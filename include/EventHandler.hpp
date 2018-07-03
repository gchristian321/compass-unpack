#ifndef CU_EVENT_HANDLER_HPP
#define CU_EVENT_HANDLER_HPP
#include <Rtypes.h>

namespace compass_unpack {

class Event;

class EventHandler {
public:
	EventHandler() {}
	virtual ~EventHandler() {}
	virtual void BeginningOfRun() = 0;
	virtual void BeginningOfEvent() = 0;
	virtual Long64_t HandleEvent(const std::vector<std::shared_ptr<Event> >&) = 0;
	virtual void EndOfEvent() = 0;
	virtual void EndOfRun() = 0;
};
  
}


#endif
