#ifndef HAVE_STATUS_BAR_CU
#define HAVE_STATUS_BAR_CU

namespace compass_unpack {

class StatusBar {
public:
	StatusBar(long long MaxEntries = 0);  
	void Reset(long long MaxEntries);
	void operator() (long long n = 1);
	void Incr(long long n) { (*this)(n); }    
private:
	long long fImax;
	long long fI;
	int fNlast;
	int fPrcntLast;
	bool fFirstTime;
};

extern StatusBar gStatusBar;

}

#endif
