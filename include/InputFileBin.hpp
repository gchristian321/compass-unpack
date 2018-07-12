#ifndef CUNPACK_INPUT_FILE_BIN_HEADER
#define CUNPACK_INPUT_FILE_BIN_HEADER
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include "Event.hpp"
#include "InputFile.hpp"


namespace compass_unpack {

  class Event;
  
  class InputFileBin : public InputFile {
  public:
    InputFileBin(const std::string& run_directory);
    virtual ~InputFileBin();

    virtual Long64_t ReadEvent(std::shared_ptr<Event>& event);
    virtual Long64_t GetTotalEvents() const { return fSize;   }
    virtual Long64_t GetEventNumber() const { return fEventNo;}
		virtual bool Good() const { return !(fStreams.empty()); }

 private:
		typedef std::unique_ptr<std::ifstream> Strm_t;
		std::vector<std::string> GetFilesInDirectory(const std::string& directory);
    std::pair<Long64_t, Long64_t> GetFileSizeAndReopen(size_t i_stream, const char* filename);
    Long64_t ReadEventFromStream(size_t i_strm, Event& event);
    void InsertLocalBuffer(size_t i_stream);
    bool CheckPSD(Strm_t&, const std::string& filename);
		
  private:
    Long64_t  fEventNo;
    Long64_t  fSize;
    std::vector<Long64_t>  fAvgEventSize;
    std::vector<Strm_t>  fStreams;
		std::vector<bool> fPSD;
    UInt_t fDoneStreams;
		struct CompareEventPointer {
			bool operator()(const std::shared_ptr<Event>& l, const std::shared_ptr<Event>& r)
				{ return *l < *r; }
		};
    // <Event, <stream indx, number of bytes read> >
    std::map<std::shared_ptr<Event>,
						 std::pair<size_t, Long64_t>,
						 CompareEventPointer>
		fLocalBuffer;
  };

}


#endif
