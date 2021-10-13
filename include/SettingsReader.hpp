#ifndef CUNPACK_SETTINGS_READER_HEADER
#define CUNPACK_SETTINGS_READER_HEADER
#include <vector>
#include <TString.h>
#include <TXMLEngine.h>

namespace compass_unpack {

class SettingsReader {
public:
	SettingsReader(const TString& runDir);
	~SettingsReader();
	TString GetNodeContent(const TString& path);
	TString GetAcquisitionMode();
	TString GetFileFormat();
	std::vector<TString> GetBoardLabels();
	std::vector<TString> GetBoardDPPTypes();
	
private:
	XMLNodePointer_t GetAcquisitionMementoNode();
	std::vector<XMLNodePointer_t> GetBoardNodes();
	TString GetContentFromAcqMemento(const TString& nodename);
	std::vector<TString> GetContentFromBoard(const TString& nodename);
	
private:
	TXMLEngine fXML;
	XMLDocPointer_t fXMLDoc;
};

}


#endif
