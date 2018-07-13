#ifdef HAVE_NPLIB // only compile if we have nptool dependence
#include "Event.hpp"
#include "EventHandlerNptool.hpp"

namespace cu = compass_unpack;

cu::EventHandlerNptool::EventHandlerNptool
(const std::string& outputFileName, 
 const std::string& outputTreeName, 
 const std::string& outputTreeTitle)
{  }

cu::EventHandlerNptool::~EventHandlerNptool()
{  }

void cu::EventHandlerNptool::BeginningOfRun()
{  }

void cu::EventHandlerNptool::BeginningOfEvent()
{

}

Long64_t cu::EventHandlerNptool::HandleEvent
(Long64_t eventNo, const std::vector<std::shared_ptr<Event> >&)
{

}

void cu::EventHandlerNptool::EndOfEvent()
{

}

void cu::EventHandlerNptool::EndOfRun()
{  }


#endif
