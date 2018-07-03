#ifndef HAVE_STATUS_BAR_CU
#define HAVE_STATUS_BAR_CU

namespace compass_unpack {

class StatusBar {
public:
	StatusBar(long long MaxEntries = 0);  
	void Reset(long long MaxEntries);
	void operator() ();
	void Incr() { (*this)(); }    
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
