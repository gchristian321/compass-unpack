#include <sstream>
#include <memory>
#include <TObjArray.h>
#include <TObjString.h>
#include "SettingsReader.hpp"
using namespace std;
namespace cu = compass_unpack;


cu::SettingsReader::SettingsReader(const TString& runDir):
	fXML()
{
	fXMLDoc = fXML.ParseFile(runDir + "/settings.xml");
	if(fXMLDoc == 0) {
		stringstream err;
		err << "Couldn't open XML settings file: " <<
			runDir + "/settings.xml";
		throw std::runtime_error(err.str().c_str());
	}
}

cu::SettingsReader::~SettingsReader()
{
	fXML.FreeDoc(fXMLDoc);
}

TString cu::SettingsReader::GetNodeContent
(const TString& path)
{
	TString result;
	stringstream err;
	XMLNodePointer_t mainnode = fXML.DocGetRootElement(fXMLDoc);

	unique_ptr<TObjArray> tok(path.Tokenize("/"));
	if(tok->GetEntries() == 0) {
		err << "ERROR: empty tokenized string\n";
		throw invalid_argument(err.str().c_str());
	}
	if(TString(fXML.GetNodeName(mainnode)) !=
		 static_cast<TObjString*>(tok->At(0))->GetString())
	{
		err << "ERROR: Node name (" << fXML.GetNodeName(mainnode) <<
			") does not match first in path (" <<
			static_cast<TObjString*>(tok->At(0))->GetString() << ")\n";
		throw invalid_argument(err.str().c_str());
	}

	XMLNodePointer_t node = mainnode;
	for(Int_t i=1; i< tok->GetEntries(); ++i) {
		TString nodename = static_cast<TObjString*>(tok->At(i))->GetString();
		XMLNodePointer_t child = fXML.GetChild(node);
		while(child != 0) {
			if(TString(fXML.GetNodeName(child)) == nodename) {
				node = child;
				break;
			}
			child = fXML.GetNext(child);
		}
		if(node != child) {
			err << "ERROR: failed to find child node: " << nodename << "\n";
			throw invalid_argument(err.str().c_str());
		}
	}
	const char* content = fXML.GetNodeContent(node);
	if (content != 0) {
		result = content;
	}
	else {
		err << "ERROR: couldn't find any content in final node: "
				<< fXML.GetNodeName(node) << "\n";
		throw(err.str().c_str());
	}
	return result;
}
