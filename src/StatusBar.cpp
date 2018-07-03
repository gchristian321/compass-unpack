#include <string>
#include <iostream>
#include <TVirtualRWMutex.h>
#include "StatusBar.hpp"

namespace compass_unpack {
StatusBar gStatusBar;
}

compass_unpack::StatusBar::StatusBar(long long imax)
{
  Reset(imax);
}

void compass_unpack::StatusBar::Reset(long long MaxEntries)
{
  fImax = MaxEntries;
  fI = 0;
  fNlast = 0;
  fPrcntLast = -1;
  fFirstTime = true;
}

namespace { struct LG {
	LG() { if(ROOT::gCoreMutex){ ROOT::gCoreMutex->Lock();   } }
	~LG(){ if(ROOT::gCoreMutex){ ROOT::gCoreMutex->UnLock(); } }
}; }

void compass_unpack::StatusBar::operator() (long long niter)
{
	LG lg; // lock guard for thread safety
	
  if(fFirstTime){
    std::cout << "\nProgress: ";
    std::flush(std::cout);
    fFirstTime = false;
  }

	fI += niter;
  int prcnt = (1000*fI) / fImax;
  if(prcnt > fPrcntLast) {
    fPrcntLast = prcnt;

    for(int i=0; i< fNlast; ++i){
      std::cout << "\b";
    }
    char buf[4096];
    sprintf(buf,"%5.1f%%  |  %lli / %lli events", prcnt/10., fI, fImax);
    fNlast = std::string(buf).size();
    std::cout << buf;
    std::flush(std::cout);
  }
  if(fI >= fImax) {
    std::cout << "\nDone!\n";
    return;
  }
}
