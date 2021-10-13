#ifndef CUNPACK_SETTINGS_READER_HEADER
#define CUNPACK_SETTINGS_READER_HEADER
#include <TString.h>
#include <TXMLEngine.h>

namespace compass_unpack {

class SettingsReader {
public:
	SettingsReader(const TString& runDir);
	~SettingsReader();
	TString GetNodeContent(const TString& path);
	
private:
	TXMLEngine fXML;
	XMLDocPointer_t fXMLDoc;
};

}


#endif
