#ifndef CU_COMPARE_POINTER
#define CU_COMPARE_POINTER
#include <memory>

namespace compass_unpack {

template <class T> class ComparePointer {
public:
	bool operator()(const T* const l, const T* const r)
		{ return *l < *r; }
};

template <class T> class CompareUniquePointer {
public:
	bool operator()(const std::unique_ptr<T>& l, const std::unique_ptr<T>& r)
		{ return *l < *r; }
};

template <class T> class CompareSharedPointer {
public:
	bool operator()(const std::shared_ptr<T>& l, const std::shared_ptr<T>& r)
		{ return *l < *r; }
};

}

#endif
