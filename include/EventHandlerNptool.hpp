#ifndef CU_EVENT_HANDLER_NPTOOL_HPP
#define CU_EVENT_HANDLER_NPTOOL_HPP

#ifdef HAVE_NPLIB // Define real class w/ nplib dependence
#include "EventHandler.hpp"

namespace compass_unpack {

class Event;

class EventHandlerNptool : public EventHandler {
public:
	EventHandlerNptool
	(const std::string& outputFileName, 
	 const std::string& outputTreeName, 
	 const std::string& outputTreeTitle);
	virtual ~EventHandlerNptool();
	virtual void BeginningOfRun();
	virtual void BeginningOfEvent();
	virtual Long64_t HandleEvent
	(Long64_t eventNo, const std::vector<std::shared_ptr<Event> >& event);
	virtual void EndOfEvent();
	virtual void EndOfRun();
};

}

#else // Define dummy class
#include <iostream>
#include "Event.hpp"
#include "EventHandler.hpp"

namespace compass_unpack {

class EventHandlerNptool : public EventHandler {
public:
	EventHandlerNptool
	(const std::string&,
	 const std::string&,
	 const std::string&)
		{
			std::cerr << "ERROR: Tried to create EventHandlerNptool without Nptool libraries!\n";
			exit(1);
		}
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
