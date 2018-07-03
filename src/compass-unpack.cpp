#include <deque>
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>
#include <TVirtualRWMutex.h>
#include "Event.hpp"
#include "StatusBar.hpp"
#include "InputFileBin.hpp"
#include "EventHandlerRoot.hpp"

namespace cu = compass_unpack;
using namespace compass_unpack;
using namespace std;



#include <set>
#include <TRandom3.h>

typedef std::deque<std::shared_ptr<Event> > Queue_t;

namespace {
struct CompareEventPointer {
	bool operator()(const std::shared_ptr<Event>& l, const std::shared_ptr<Event>& r)
		{ return *l < *r; }
};

struct LG {
	LG() { if(ROOT::gCoreMutex){ ROOT::gCoreMutex->Lock();   } }
	~LG(){ if(ROOT::gCoreMutex){ ROOT::gCoreMutex->UnLock(); } }
};

template<class T> Long64_t LengthInTime(const T& t)
{
	return t.size() < 2 ? 0 : t.back()->GetTimestamp() - t.front()->GetTimestamp();
}

template<class T> Long64_t TimeDiff(const T& t1, const T& t2)
{
	return abs(t2->GetTimestamp() - t1->GetTimestamp());
}

}


int main()
{
	cu::InputFileBin in("n14-test-june2018-DPP_PSD/DAQ/run_1234/UNFILTERED");
	gStatusBar.Reset(in.GetTotalEvents());

	Long64_t kMatchWindow = 20e6;
	Long64_t kMaxTime = 10e12;
	
	Queue_t evtQueue;

	std::atomic<bool> doneReading(false);

	Long64_t numread(0), numwritten(0);
	auto readerLoop = [&](){
		while(true) {
			if(LengthInTime(evtQueue) > 2*kMaxTime) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			auto evt = std::make_shared<cu::Event>();
			Long64_t nread = in.ReadEvent(evt);
			if(nread == -1) {
				break;
			} else {
				::LG lg;
				assert(evtQueue.empty() || *(evtQueue.back()) < *evt);
				evtQueue.push_back(evt);
				++numread;
			}
		}
		doneReading = true;
		{
			LG lg__;
		}
	};
	auto readerThread = std::thread(readerLoop);

	auto handlerLoop = [&](){
		std::unique_ptr<EventHandler> handler
		(new EventHandlerRoot
		 ("n14-test-june2018-DPP_PSD/DAQ/run_1234/test-out.root",
			"MatchedData",
			"test"));
		handler->BeginningOfRun();
		
		while(true) {
			if(doneReading) {
				LG lg;
				if(evtQueue.empty()) {
					break;
				}
			}
			std::vector<std::shared_ptr<Event> > matches;
			{
				LG lg;
				if(doneReading || LengthInTime(evtQueue) > kMaxTime) {
					Queue_t::iterator itBegin = evtQueue.begin();
					Queue_t::iterator itEnd = evtQueue.begin();
					while(itEnd != evtQueue.end() && TimeDiff(*itEnd, *itBegin) < kMatchWindow) {
						++itEnd;
					}
					matches.assign(itBegin, itEnd);
					evtQueue.erase(itBegin, itEnd);

					handler->BeginningOfEvent();
					handler->HandleEvent(matches);
					handler->EndOfEvent();					
					gStatusBar.Incr(itEnd-itBegin);
					numwritten += (itEnd-itBegin);
				}
			}
		}
		handler->EndOfRun();
	};
	auto handlerThread = std::thread(handlerLoop);

	std::cout << "\n----------- Unpacking Events -----------";
	readerThread.join();
	handlerThread.join();

	std::cout << "\n----------- Summary -----------\n";
	std::cout << "Events read: " << numread << "\n" <<
		"Events written: " << numwritten << "\n";
}
