#include <map>
#include <limits>
#include <string>
#include <iostream>
#include <TFile.h>
#include <TChain.h>
#include <TSystem.h>
#include "Event.hpp"
#include "InputFileBin.hpp"


namespace cu = compass_unpack;

namespace {
int extract_channel(const std::string& name) {
	size_t last_slash = name.rfind("/");
	size_t last_at = name.rfind("@");
	
	std::string number =
		last_slash < name.size() ?
								 name.substr(1+last_slash,last_at-last_slash-1) :
		name.substr(0,last_at);
			
	return atoi(number.c_str());
}

int extract_board(const std::string& name) {
	size_t last_slash = name.rfind("/");
	std::string name1 =
		last_slash < name.size() ? name.substr(1+last_slash) : name;

	size_t first_pound = name1.find("#");
	size_t first_dash = name1.find("-");
	std::string board_str = name1.substr(first_pound+1,first_dash-first_pound-1);

	return atoi(board_str.c_str());
}

struct ifs_reopen {
	ifs_reopen(const std::string& file, std::unique_ptr<std::ifstream>& ifs):
		fFile(file), fIfs(ifs) {
		fIfs->close();
		fIfs->open(fFile.c_str(), std::ios::binary);
	}
	~ifs_reopen() { fIfs->close(); fIfs->open(fFile.c_str(), std::ios::binary); }
private:
	std::string fFile;
	std::unique_ptr<std::ifstream>& fIfs;
};

}

std::vector<std::string> cu::InputFileBin::GetFilesInDirectory(const std::string& dir){
	TString files;
	if(dir != "") {
		files = gSystem->GetFromPipe( (std::string("ls ") + dir + "/*.bin").c_str() );
	} else {
		files = gSystem->GetFromPipe( (std::string("ls ") + "*.bin").c_str() );
	}
	std::unique_ptr<TObjArray> filesTokenized(files.Tokenize("\n"));
	std::vector<std::string> filesOut(filesTokenized->GetEntries());
	for(int i=0; i< filesTokenized->GetEntries(); ++i){
		filesOut[i] = static_cast<TObjString*>(filesTokenized->At(i))->GetString().Data();
	}
	
	std::sort (filesOut.begin(), filesOut.end(),
						 [&](const std::string& lhs, const std::string& rhs){
							 int boardL = extract_board(lhs), boardR = extract_board(rhs);
							 int channelL = extract_channel(lhs), channelR = extract_channel(rhs);
							 return boardL != boardR ?
								 boardL < boardR : channelL < channelR;
						 });
	return filesOut;
}

std::pair<Long64_t, Long64_t> // total size, avg. event size
cu::InputFileBin::GetFileSizeAndReopen(size_t i_strm, const char* filename) 
{
  auto& ifs = fStreams.at(i_strm);
	ifs_reopen ifsr(filename, fStreams.at(i_strm)); // ensure file goes back to beginning
	
  // Check total file size
  std::streampos fsize = ifs->tellg();
  ifs->seekg(0,std::ios::end);
  Long64_t file_size = Long64_t(ifs->tellg() - fsize);
  ifs->close();

  // Read average size of first 10 events
  ifs->open(filename, std::ios::binary);
	int nevts = 0;
	Long64_t total_bytes = 0;
	while(nevts < 10){
		Event dummy;
		Long64_t nbytes = ReadEventFromStream(i_strm, dummy);
		if(nbytes == -1) { break; }
		total_bytes += nbytes;
		++nevts;
	}
	Long64_t avg_size = nevts == 0 ? 0 : total_bytes / nevts;    
  return std::make_pair(file_size, avg_size);
}

bool cu::InputFileBin::CheckPSD(Strm_t& fstrm, const std::string& filename)
{	
	auto check_board_channel =
		[&fstrm, &filename](bool& success, bool psd, int board, int channel) {

		success = false;
		ifs_reopen c(filename, fstrm);
		
		size_t evtsize = psd ?
		5*sizeof(UShort_t) + sizeof(Long64_t) + sizeof(UInt_t) :
		4*sizeof(UShort_t) + sizeof(Long64_t) + sizeof(UInt_t);
		std::vector<char> buf(evtsize);
		fstrm->read(&buf[0], evtsize);
		UShort_t board1 = *((UShort_t*)&buf[0]);
		UShort_t channel1 = *((UShort_t*)&buf[sizeof(UShort_t)]);
		
		UShort_t nwaves = *((UShort_t*)&buf[evtsize-sizeof(UInt_t)]);
		std::vector<char> buf2(nwaves*sizeof(UShort_t));
		fstrm->read(&buf2[0], buf2.size());

		fstrm->read(&buf[0], evtsize);
		UShort_t board2 = *((UShort_t*)&buf[0]);
		UShort_t channel2 = *((UShort_t*)&buf[sizeof(UShort_t)]);

		if(board1 == board2 && board == board1){
			success = true;
		}
	};

 
	bool success = false;
	int board = extract_board(filename);
	int channel = extract_channel(filename);
		
	// first try w/o PSD
	check_board_channel(success,false,board,channel);
	if(success) {
		return false; // no psd
	}
	// then try w/o PSD
	check_board_channel(success,true,board,channel);
	if(success) {
		return true; // yes psd
	}
	// shouldn't get here
	std::cerr << "ERROR: Couldn't determine PSD type for file: \"" <<
		filename << "\"\n";
	exit(1);
}

cu::InputFileBin::InputFileBin(const std::string& run_directory):
	fEventNo(0), fSize(0), fAvgEventSize(0), fDoneStreams(0)
{
  std::vector<std::string> files = GetFilesInDirectory(run_directory);

  std::cout << "\n----------- Opening Files -----------\n";
  size_t ifile = 0;
  for(auto& f : files) {
    fStreams.emplace_back
			(std::unique_ptr<std::ifstream>
			 (new std::ifstream(f.c_str(), std::ios::binary)));
		fPSD.push_back(CheckPSD(fStreams.back(), f));
														
    std::pair<Long64_t,Long64_t> sizes =
      GetFileSizeAndReopen(ifile, f.c_str());
    fSize += (sizes.first / sizes.second); // total size / avg. event size
    fAvgEventSize.push_back(sizes.second); // avg size for this stream
    ++ifile;

		std::string psd = fPSD.back() ? "yes" : "no";
	 	std::cout << f << " (PSD: " << psd << ")\n";
	}
}


cu::InputFileBin::~InputFileBin()
{   }

namespace { template<class T> void set_val(T& t, char*& p){
	t = *reinterpret_cast<T*>(p);
	p += sizeof(T);
} }

Long64_t cu::InputFileBin::ReadEventFromStream(size_t i_strm, cu::Event& event)
{
	// Structure --
	/*
		Board (UShort_t)
		Channel (UShort_t)
		Timestamp (ULong64_t)
		Energy (UShort_t)
		Energy short gate (UShort_t) - only if PSD mode
		Flags (UShort_t)
		Number of Waveform points (UInt_t)
		-->> waveform points (UShort_t)
	*/
  auto& pfs = fStreams.at(i_strm);
  if(!pfs->good()) {
    return -1;
  }

  // Get initial position
  std::streampos initialPos = pfs->tellg();
	
	// Read everything up to the waveforms
	size_t evtsize = fPSD.at(i_strm) ?
		5*sizeof(UShort_t) + sizeof(Long64_t) + sizeof(UInt_t) :
		4*sizeof(UShort_t) + sizeof(Long64_t) + sizeof(UInt_t);
	std::vector<char> buf(evtsize);
	pfs->read(&buf[0], evtsize);

	char* pbuf = &buf[0];
	set_val(event.fBoard, pbuf);
	set_val(event.fChannel, pbuf);
	ULong64_t tsUnsigned;
	set_val(tsUnsigned, pbuf);
	event.fTimestamp = tsUnsigned;
	set_val(event.fEnergy, pbuf);
	if(fPSD.at(i_strm)){
		set_val(event.fEnergyShort, pbuf);
	}
	UShort_t flags16;
	set_val(flags16, pbuf);
	event.fFlags = flags16;
	UInt_t nwaves;
	set_val(nwaves, pbuf);
	
	// Then read the waveforms
	event.fWaveform.resize(nwaves);
	pfs->read(reinterpret_cast<char*>(&event.fWaveform[0]), nwaves*sizeof(UShort_t));
	
  // Get final position
  std::streampos finalPos = pfs->tellg();
  
  // Return # of bytes read
  return Long64_t(finalPos - initialPos);
}

void cu::InputFileBin::InsertLocalBuffer(size_t i_stream)
{
	auto event = std::make_shared<cu::Event>();
  Long64_t nbytes = ReadEventFromStream(i_stream, *event);
  if(nbytes != -1) 
	{
		fLocalBuffer.insert
			(std::make_pair(event, std::make_pair(i_stream, nbytes)));
	}
  else
	{
		++fDoneStreams;
	}
}

Long64_t cu::InputFileBin::ReadEvent(std::shared_ptr<cu::Event>& event)
{
  // Need to make sure events are processed in
  // the order of their timestamps, which may be
  // different from the order they appear in the
  // different files.

  // N.B. return -1 -->> we are all done with all files
  
  // If local buffer is empty, we are either just
  // starting, or have processed everything
  if(fLocalBuffer.empty()){
    // We are finished if all streams are done
    if(fDoneStreams == fStreams.size()) {
      return -1;
    }
    
    // Just starting, insert events from all streams
    for(size_t i = 0; i< fStreams.size(); ++i){
      InsertLocalBuffer(i);
    }
    // But maybe there were no good ones to begin with
    if(fDoneStreams == fStreams.size()) {
      return -1; 
    }
  }

  assert(fLocalBuffer.empty() == false);
  assert(fLocalBuffer.size() + fDoneStreams == fStreams.size());
	
  // We now have a local buffer w/ events from every stream.
  // Process the earliest event.

  // Set the output event to be the first one in the local
	event = fLocalBuffer.begin()->first;
  
  // Now erase that event from the local buffer
  const size_t i_stream = fLocalBuffer.begin()->second.first;
  const Long64_t nbytes = fLocalBuffer.begin()->second.second;
  fLocalBuffer.erase(fLocalBuffer.begin());
  
  // And insert a new one from the same stream
  InsertLocalBuffer(i_stream);

  // Increment event counter according to number of
  // bytes we processed and avg. event size in this stream
  fEventNo += (nbytes / fAvgEventSize.at(i_stream));
  return fEventNo;
}
