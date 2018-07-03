#ifndef CU_LOCK_GUARD_HPP
#define CU_LOCK_GUARD_HPP
#include <TVirtualRWMutex.h>

namespace compass_unpack {

class LockGuard {
public:
	LockGuard()
		{
			if(ROOT::gCoreMutex){
				ROOT::gCoreMutex->Lock();
			}
		}
	~LockGuard()
		{
			if(ROOT::gCoreMutex){
				ROOT::gCoreMutex->UnLock();
			}
		}
};

}

#endif
