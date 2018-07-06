#ifndef CU_TTREE_MERGER_HPP
#define CU_TTREE_MERGER_HPP
#include <string>

namespace compass_unpack {

class TTreeMerger {
public:
	TTreeMerger(int nthreads,
							const std::string& main_filename,
							const std::string& tree_name,
							bool verbose = false);
	~TTreeMerger() { Close(); }
	void Close();
	void Print();
	bool GetVerbose() const { return fVerb; }
	void SetVerbose(bool verb) { fVerb = verb; }
public:
	static std::string GetThreadDir
	(const std::string& main_filename);

	static std::string GetThreadFile
	(int threadNo, const std::string& main_filename);

private:
	bool fClosed;
	bool fVerb;
	int fNthreads;
	std::string fFilename;
	std::string fTreename;
	std::string fThreadDir;
};

};


#endif
