#include <iostream>
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


TString cu::SettingsReader::GetContentFromAcqMemento(const TString& nodename)
{
	XMLNodePointer_t acqNode = GetAcquisitionMementoNode();
	XMLNodePointer_t child = 0;
	if(acqNode) {
		child = fXML.GetChild(acqNode);
		while(child != 0) {
			if(TString(fXML.GetNodeName(child)) == nodename) {
				break;
			}
			child = fXML.GetNext(child);
		}
	}
	else {
		throw invalid_argument("Coundn't find \"acquisitionMemento\" node!");
	}

	if(!child) {
		stringstream err;
		err << "Couldn't find \"acquisitionMemento/" << nodename << "\" node!";
		throw invalid_argument(err.str().c_str());
	}
	const char* content = fXML.GetNodeContent(child);
	if (content == 0) {
		stringstream err;
		err << "No content for \"acquisitionMemento/" << nodename << "\" node!";
		throw invalid_argument(err.str().c_str());
	}
	return TString(content);
}


TString cu::SettingsReader::GetAcquisitionMode()
{
	return GetContentFromAcqMemento("acquisitionMode");
}

TString cu::SettingsReader::GetFileFormat()
{
	return GetContentFromAcqMemento("fileFormat");
}

vector<TString> cu::SettingsReader::GetContentFromBoard(const TString& nodename)
{
	vector<XMLNodePointer_t> boardNodes = GetBoardNodes();
	vector<XMLNodePointer_t> children;
	if(boardNodes.size()) {
		XMLNodePointer_t child = 0;
		for(auto bNode : boardNodes) {
			child = fXML.GetChild(bNode);
			while(child != 0) {
				if(TString(fXML.GetNodeName(child)) == nodename) {
					children.push_back(child);
					break;
				}
				child = fXML.GetNext(child);
			}
			if(!child) {
				stringstream err;
				err << "Couldn't find \"board/" << nodename << "\" nodes!";
				throw invalid_argument(err.str().c_str());
			}
		}
	}
	else {
		throw invalid_argument("Coundn't find any \"board\" nodes!");
	}

	vector<TString> retvals;
	for(auto child : children) {
		const char* content = fXML.GetNodeContent(child);
		if (content == 0) {
			stringstream err;
			err << "No content for \"board/" << nodename << "\" nodes!";
			throw invalid_argument(err.str().c_str());
		}
		retvals.push_back(content);
	}
	return retvals;
}

vector<TString> cu::SettingsReader::GetBoardLabels()
{
	return GetContentFromBoard("label");
}

vector<TString> cu::SettingsReader::GetBoardDPPTypes()
{
	return GetContentFromBoard("dppType");
}

XMLNodePointer_t
cu::SettingsReader::GetAcquisitionMementoNode()
{
	XMLNodePointer_t mainnode = fXML.DocGetRootElement(fXMLDoc);
	XMLNodePointer_t child = fXML.GetChild(mainnode);
	while(child != 0) {
		if(TString(fXML.GetNodeName(child)) == "acquisitionMemento") {
			return child;
		}
		child = fXML.GetNext(child);
	}
	return 0;
}

vector<XMLNodePointer_t>
cu::SettingsReader::GetBoardNodes()
{
	vector<XMLNodePointer_t> boardNodes;
	XMLNodePointer_t mainnode = fXML.DocGetRootElement(fXMLDoc);
	XMLNodePointer_t child = fXML.GetChild(mainnode);
	while(child != 0) {
		if(TString(fXML.GetNodeName(child)) == "board") {
			boardNodes.push_back(child);
		}
		child = fXML.GetNext(child);
	}
	return boardNodes;
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
