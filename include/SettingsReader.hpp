#ifndef CUNPACK_SETTINGS_READER_HEADER
#define CUNPACK_SETTINGS_READER_HEADER
#include <memory>
#include <vector>
#include <TString.h>
#include <TXMLEngine.h>

namespace compass_unpack {

class SettingsReader {
public:
	SettingsReader(const TString& runDir);
	~SettingsReader();
	TString GetAcquisitionMode();
	TString GetFileFormat();
	const std::vector<TString>& GetBoardLabels();
	const std::vector<TString>& GetBoardDPPTypes();
	
private:
	TString GetNodeContent(const TString& path);
	XMLNodePointer_t GetAcquisitionMementoNode();
	std::vector<XMLNodePointer_t> GetBoardNodes();
	TString GetContentFromAcqMemento(const TString& nodename);
	std::vector<TString> GetContentFromBoard(const TString& nodename);
	
private:
	std::unique_ptr<std::vector<TString> > fBoardLabels;
	std::unique_ptr<std::vector<TString> > fBoardDPPTypes;
	std::unique_ptr<TString> fAcqMode;
	std::unique_ptr<TString> fFileFormat;
	
	TXMLEngine fXML;
	XMLDocPointer_t fXMLDoc;
};

}


#endif
