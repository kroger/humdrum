//
// BaseTest.cpp
//
// $Id: //depot/XML/Main/public/samples/src/BaseTest.cpp#4 $
//
// Sample application for CenterPoint/XML.
//
// The contents of this file are subject to the 
// CenterPoint Public License Version 1.0 (the "License"); 
// you may not use this file except in compliance with the 
// License. You may obtain a copy of the License at 
// http://www.cpointc.com/CPL
//
// Software distributed under the License is distributed on 
// an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either
// express or implied. See the License for the specific 
// language governing rights and limitations under the License.
//
// The Original Code is CenterPoint/XML.
//
// The Initial Developer of the Original Code is CenterPoint - 
// Connective Software Engineering GmbH. Portions created by
// CenterPoint - Connective Software Engineering GmbH. are 
// Copyright (c) 1998-2002 CenterPoint - Connective Software
// Engineering GmbH. All Rights Reserved.
//
// Contributor(s):
//


#include "XMLParserBase.h"
#include "XMLUtils.h"
#include "XMLException.h"
#include <iostream>
#include <fstream>


using namespace CSL::XML;


//
// CXMLParser
//


class CXMLParser: public CXMLParserBase
{
public:
	CXMLParser(const XML_Char* encoding = NULL);
	virtual ~CXMLParser();

protected:
	virtual void HandleStartElement(const XML_Char* name, const XML_Char** atts);
	virtual void HandleEndElement(const XML_Char* name);
	virtual void HandleCharacterData(const XML_Char* s, int len);
	virtual void HandleProcessingInstruction(const XML_Char* target, const XML_Char* data);
	virtual void HandleDefault(const XML_Char* s, int len);
	virtual void HandleUnparsedEntityDecl(const XML_Char* entityName, const XML_Char* base,
		                                  const XML_Char* systemId, const XML_Char* publicId,
					        		      const XML_Char* notationName);
	virtual void HandleNotationDecl(const XML_Char* notationName, const XML_Char* base,
		                            const XML_Char* systemId, const XML_Char* publicId);
	virtual int HandleExternalEntityRef(const XML_Char* openEntityNames, const XML_Char* base,
									    const XML_Char* systemId, const XML_Char* publicId);
	virtual int HandleUnknownEncoding(const XML_Char* name, XML_Encoding* info);

	virtual void HandleComment(const XML_Char* data);
	virtual void HandleStartCDataSection();
	virtual void HandleEndCDataSection();
	virtual void HandleStartNamespace(const XML_Char* prefix, const XML_Char* uri);
	virtual void HandleEndNamespace(const XML_Char* prefix);

	virtual XMLByteInputStream* OpenExternalEntityStream(const XMLString& fileName);
	virtual void CloseExternalEntityStream(XMLByteInputStream* istr);
};


CXMLParser::CXMLParser(const XML_Char* encoding):
            CXMLParserBase(encoding)
{
}


CXMLParser::~CXMLParser()
{
}


void CXMLParser::HandleStartElement(const XML_Char* name, const XML_Char** atts)
{
	cout
		<< "StartElement" << endl
		<< "\tName: " << name << endl
		<< "\tAttributes: ";
	while (*atts)
	{
		cout << *(atts++) << "=\"";
		cout << *(atts++) << "\" ";
	}
	cout
		<< endl
		<< "\tSpecified Attributes: " << GetSpecifiedAttributeCount() << endl
		<< "\tLine: " << GetCurrentLineNumber() << endl
		<< "\tColumn: " << GetCurrentColumnNumber() << endl;
	
}


void CXMLParser::HandleEndElement(const XML_Char* name)
{
	cout
		<< "EndElement" << endl
		<< "\tName: " << name << endl;
}


void CXMLParser::HandleCharacterData(const XML_Char* s, int len)
{
	cout
		<< "CharacterData" << endl
		<< "\tLength: " << len << endl
		<< "\tData: " << XMLString(s, 0, len) << endl;
}


void CXMLParser::HandleProcessingInstruction(const XML_Char* target, const XML_Char* data)
{
	cout
		<< "ProcessingInstruction" << endl
		<< "\tTarget: " << target << endl
		<< "\tData: " << data << endl;
}


void CXMLParser::HandleDefault(const XML_Char* s, int len)
{
	cout
		<< "DefaultHandler" << endl
		<< "\tLength: " << len << endl
		<< "\tData: " << XMLString(s, 0, len) << endl;
}


void CXMLParser::HandleUnparsedEntityDecl(const XML_Char* entityName, const XML_Char* base,
		                                  const XML_Char* systemId, const XML_Char* publicId,
					        		      const XML_Char* notationName)
{
	cout
		<< "UnparsedEntityDecl" << endl
		<< "\tName: " << entityName << endl
		<< "\tBase: " << base << endl
		<< "\tSystemId: " << systemId << endl
		<< "\tPublicId: " << (publicId == NULL ? "(null)" : publicId) << endl
		<< "\tNotationName: " << notationName << endl;
}


void CXMLParser::HandleNotationDecl(const XML_Char* notationName, const XML_Char* base,
		                            const XML_Char* systemId, const XML_Char* publicId)
{
	cout
		<< "NotationDecl" << endl
		<< "\tNotationName: " << notationName << endl
		<< "\tBase: " << base << endl
		<< "\tSystemId: " << (systemId == NULL ? "(null)" : systemId) << endl
		<< "\tPublicId: " << (publicId == NULL ? "(null)" : publicId) << endl;
}


int CXMLParser::HandleExternalEntityRef(const XML_Char* openEntityNames, const XML_Char* base,
									    const XML_Char* systemId, const XML_Char* publicId)
{
	cout
		<< "ExternalEntityRef" << endl
		<< "\tOpenEntityNames: " << openEntityNames << endl
		<< "\tBase: " << (base == NULL ? "(null)" : base) << endl
		<< "\tSystemId: " << systemId << endl
		<< "\tPublicId: " << (publicId == NULL ? "(null)" : publicId) << endl;

	return CXMLParserBase::HandleExternalEntityRef(openEntityNames, base, systemId, publicId);
}


int CXMLParser::HandleUnknownEncoding(const XML_Char* name, XML_Encoding* info)
{
	cout
		<< "UnknownEncoding" << endl
		<< "\tName: " << name << endl;

	return CXMLParserBase::HandleUnknownEncoding(name, info);
}


void CXMLParser::HandleComment(const XML_Char* data)
{
	cout
		<< "Comment" << endl
		<< "\tData: " << data << endl;
}


void CXMLParser::HandleStartCDataSection()
{
	cout
		<< "Start CDATA" << endl;
}


void CXMLParser::HandleEndCDataSection()
{
	cout
		<< "End CDATA" << endl;
}


void CXMLParser::HandleStartNamespace(const XML_Char* prefix, const XML_Char* uri)
{
	cout
		<< "Start Namespace" << endl
		<< "\tPrefix: " << (prefix ? prefix : "NULL") << endl
		<< "\tURI: " << (uri ? uri : "NULL") << endl;
}


void CXMLParser::HandleEndNamespace(const XML_Char* prefix)
{
	cout
		<< "End Namespace" << endl
		<< "\tPrefix: " << (prefix ? prefix : "NULL") << endl;
}


XMLByteInputStream* CXMLParser::OpenExternalEntityStream(const XMLString& fileName)
{
	cout << "** opening external entity stream " << fileName << endl;

	return new ifstream(CXMLUtils::XMLToAscii(fileName).c_str());
}


void CXMLParser::CloseExternalEntityStream(XMLByteInputStream* istr)
{
	cout
		<< "** closing external entity stream" << endl;

	if (istr) delete istr;
}


//
// main
//


int main(int argc, char** argv)
{
	ifstream istr;
	CXMLParser parser;

	if (argc < 2)
	{
		cerr << "Error: no filename specified" << endl;
		exit(1);
	}

	parser.EnableNamespaceProcessing();

	istr.open(argv[1]);
	if (istr.good())
	{
		try
		{
			parser.Parse(&istr);
		}
		catch (CXMLException& e)
		{
			cerr << "Error: " << e.GetMsg() << " (line: " << parser.GetCurrentLineNumber() << ")" << endl;
			exit(1);
		}
		istr.close();
	}
	else
	{
		cerr << "Error: cannot open input file" << endl;
		exit(1);
	}
	return 0;
}

