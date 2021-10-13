#ifndef CUNPACK_INPUT_FILE_BIN_HEADER
#define CUNPACK_INPUT_FILE_BIN_HEADER
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include "Event.hpp"
#include "ComparePointer.hpp"
#include "InputFile.hpp"


namespace compass_unpack {

class Event;
class SettingsReader;

class InputFileBin : public InputFile {
public:
	InputFileBin(const std::string& run_directory, SettingsReader& settings);
	virtual ~InputFileBin();

	virtual Long64_t ReadEvent(std::shared_ptr<Event>& event);
	virtual Long64_t GetTotalEvents() const { return fSize;   }
	virtual Long64_t GetEventNumber() const { return fEventNo;}
	virtual bool Good() const { return !(fStreams.empty()); }
			
private:
	typedef std::unique_ptr<std::ifstream> Strm_t;
	std::pair<Long64_t, Long64_t> GetFileSizeAndReopen(size_t i_stream, const char* filename);
	Long64_t ReadEventFromStream(size_t i_strm, Event& event);
	void InsertLocalBuffer(size_t i_stream);
	bool CheckPSD(Strm_t&, const std::string& filename);

public:
	static std::vector<std::string> GetFilesInDirectory(const std::string& directory);
	static std::vector<std::pair<const int, const int> > GetBoardChannelCombos(const std::string& directory);
		
private:
	bool      fWaves;
	std::vector<TString> fBoardLabels;
	std::vector<TString> fBoardDPPTypes;
	Long64_t  fEventNo;
	Long64_t  fSize;
	std::vector<Long64_t>  fAvgEventSize;
	std::vector<Strm_t>  fStreams;
	std::vector<bool> fPSD;
	UInt_t fDoneStreams;
	// <Event, <stream indx, number of bytes read> >
	std::map<std::shared_ptr<Event>,
					 std::pair<size_t, Long64_t>,
					 CompareSharedPointer<compass_unpack::Event> >
	fLocalBuffer;
};

}


#endif
