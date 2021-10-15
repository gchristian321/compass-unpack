#include <algorithm>
#include <TROOT.h>
#include "Event.hpp"
#include "EventHandlerPlugin.hpp"
using namespace std;
namespace cu = compass_unpack;

cu::EventHandlerPlugin::EventHandlerPlugin
(const std::string& pluginFileName,
 const std::string& outputFileName,
 const std::string& outputTreeName,
 const std::string& outputTreeTitle):
	EventHandlerRootSimple(
		outputFileName,outputTreeName,outputTreeTitle)
{
	gROOT->ProcessLine(
		(string(".L ") + pluginFileName + "+").c_str());

	fRunBegin = reinterpret_cast<void(*)()>(
		gROOT->ProcessLineFast("return plugin_run_begin;"));
	fEventBegin = reinterpret_cast<void(*)()>(
		gROOT->ProcessLineFast("return plugin_event_begin;"));
	fEventEnd = reinterpret_cast<void(*)()>(
		gROOT->ProcessLineFast("return plugin_event_end;"));
	fRunEnd = reinterpret_cast<void(*)()>(
		gROOT->ProcessLineFast("return plugin_run_end;"));
	fHandleEvent = reinterpret_cast<
		Long64_t(*)(map<string,double>&)> (
			gROOT->ProcessLineFast("return plugin_handle_event;"));

	SetSaveAsDouble(kTRUE);
}

cu::EventHandlerPlugin::~EventHandlerPlugin()
{ }

void cu::EventHandlerPlugin::BeginningOfRun()
{
	EventHandlerRootSimple::BeginningOfRun();
	fRunBegin();
}

void cu::EventHandlerPlugin::BeginningOfEvent()
{
	EventHandlerRootSimple::BeginningOfEvent();
	fEventBegin();
}

Long64_t cu::EventHandlerPlugin::HandleEvent
(Long64_t eventNo, const std::vector<std::shared_ptr<Event> >& matches)
{
	EventHandlerRootSimple::HandleEvent(eventNo, matches);
	return fHandleEvent(fEventData);
}

void cu::EventHandlerPlugin::EndOfEvent()
{
	fEventEnd();
	EventHandlerRootSimple::EndOfEvent();
}

void cu::EventHandlerPlugin::EndOfRun()
{
	fRunEnd();
	EventHandlerRootSimple::EndOfRun();
}
