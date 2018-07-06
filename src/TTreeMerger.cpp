#include <iostream>
#include <TFile.h>
#include <TChain.h>
#include <TSystem.h>
#include "TTreeMerger.hpp"

namespace cu = compass_unpack;


cu::TTreeMerger::TTreeMerger
(int nthreads,
 const std::string& main_filename,
 const std::string& tree_name,
 bool verb):
	fClosed(false),
	fVerb(verb),
	fNthreads(nthreads),
	fFilename(main_filename),
	fTreename(tree_name)
{
	std::string threadDir = GetThreadDir(fFilename);
	if(fNthreads > 1) {
		gSystem->Exec(Form("rm -fr %s ; mkdir -p %s",
											 threadDir.c_str(), threadDir.c_str()));
	}
}

void cu::TTreeMerger::Print()
{
	if(fNthreads > 1) {
		std::cout << "\nWrote output to file: " << fFilename.c_str() << "\n" <<
			"Containing TChain of files:\n";
		for(int i=0; i< fNthreads; ++i){
			std::cout << "  " << GetThreadFile(i, fFilename) << "\n";
		}
	} else {
		std::cout << "\nWrote output to file: " << fFilename.c_str() << "\n";
	}
}

void cu::TTreeMerger::Close()
{
	if(fClosed) { return; }
	if(fVerb) { Print(); }
	if(fNthreads > 1){
		TFile f(fFilename.c_str(), "recreate");
		if(f.IsZombie()){
			std::cerr << "ERROR: Couldn't create merged file: \"" << fFilename << "\"!\n";
		}
		TChain * ch = new TChain(fTreename.c_str());
		for(int n=0; n< fNthreads; ++n){
			ch->AddFile(GetThreadFile(n, fFilename).c_str());
		}
		ch->Write();
	}
	fClosed = true;
}

std::string cu::TTreeMerger::GetThreadDir
(const std::string& main_filename)
{
	return main_filename + "_files";
}

std::string cu::TTreeMerger::GetThreadFile
(int threadNo, const std::string& main_filename)
{
	return GetThreadDir(main_filename) + Form("/thread_%i.root", threadNo);
}
