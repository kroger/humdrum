//
// SAXTest.cpp
//
// $Id: //depot/XML/Main/public/samples/src/SAXTest.cpp#4 $
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


#include "SAXParser.h"
#include "SAX/HandlerBase.h"
#include "SAX/Configurable.h"
#include "SAX/EntityResolver.h"
#include "SAX/LexicalHandler.h"
#include "SAX/NamespaceHandler.h"
#include "SAX/AttributeList.h"
#include "SAX/AttributeListImp.h"
#include "SAX/SAXException.h"
#include "SAX/EntityResolverImp.h"
#include "SAX/Locator.h"
#include "XMLUtils.h"
#include "XMLException.h"
#include <iostream>
#include <fstream>


//
// Features recognized by the parser
//
#define FEATURE_VALIDATION "http://xml.org/sax/features/validation"
#define FEATURE_NAMESPACES "http://xml.org/sax/features/namespaces"
#define FEATURE_NORMALIZE  "http://xml.org/sax/features/normalize-text"
#define FEATURE_USE_LOCATOR        "http://xml.org/sax/features/use-locator"
#define FEATURE_EXT_GEN_ENTITIES   "http://xml.org/sax/features/external-general-entities"


//
// Properties recognized by the parser
//
#define PROPERTY_NAMESPACE_SEP     "http://xml.org/sax/properties/namespace-sep"
#define PROPERTY_DECL_HANDLER      "http://xml.org/sax/handlers/DeclHandler"
#define PROPERTY_LEXICAL_HANDLER   "http://xml.org/sax/handlers/LexicalHandler"
#define PROPERTY_NAMESPACE_HANDLER "http://xml.org/sax/handlers/NamespaceHandler"
#define PROPERTY_DOCUMENT_BASE     "http://cpointc.com/xml/parser/DocumentBase"
#define PROPERTY_DEFAULT_HANDLER   "http://cpointc.com/xml/parser/DefaultHandler"


using namespace CSL::XML;


//
// CXMLProcessor
//


class CProcessor: public CHandlerBase, public CLexicalHandler, public CNamespaceHandler
{
	// CHandlerBase
	void SetDocumentLocator(const CLocator& loc);
	void NotationDecl(const XMLString& name, const XMLString* publicd, const XMLString* systemId);
	void UnparsedEntityDecl(const XMLString& name, const XMLString* publicId, const XMLString& systemId, const XMLString& notationName);
	void StartDocument();
	void EndDocument();
	void StartElement(const XMLString& name, const CAttributeList& attrList);
	void EndElement(const XMLString& name);
	void Characters(const XMLChar ch[], int start, int length);
	void IgnorableWhitespace(const XMLChar ch[], int start, int length);
	void ProcessingInstruction(const XMLString& target, const XMLString& data);
	void Warning(CSAXException& e);
	void Error(CSAXException& e);
	void FatalError(CSAXException& e);

	// CLexicalHandler
	void StartDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId);
	void EndDTD();
	void StartEntity(const XMLString& name);
	void EndEntity(const XMLString& name);
	void StartCDATA();
	void EndCDATA();
	void Comment(const XMLChar ch[], int start, int length); 

	// CNamespaceHandler
	void StartNamespaceDeclScope(const XMLString& prefix, const XMLString& uri);
	void EndNamespaceDeclScope(const XMLString& prefix);

private:
	const CLocator* m_pLocator;
};


void CProcessor::SetDocumentLocator(const CLocator& loc)
{
	cout
		<< "SetDocumentLocator" << endl;

	m_pLocator = &loc;
	cout
		<< "\tlocation/publicId=" << m_pLocator->GetPublicId() << endl
		<< "\tlocation/systemId=" << m_pLocator->GetSystemId() << endl;
}


void CProcessor::NotationDecl(const XMLString& name, const XMLString* publicId, const XMLString* systemId)
{
	cout
		<< "NotationDecl" << endl
		<< "\tname=" << name << endl
		<< "\tpublicId=" << (publicId ? publicId->c_str() : "(null)") << endl
		<< "\tsystemId=" << (systemId ? systemId->c_str() : "(null)") << endl;
}


void CProcessor::UnparsedEntityDecl(const XMLString& name, const XMLString* publicId, const XMLString& systemId, const XMLString& notationName)
{
	cout
		<< "UnparsedEntityDecl" << endl
		<< "\tname=" << name << endl
		<< "\tpublicId=" << (publicId ? publicId->c_str() : "(null)") << endl
		<< "\tsystemId=" << systemId << endl
		<< "\tnotationName=" << notationName << endl;
}


void CProcessor::StartDocument()
{
	cout << "StartDocument" << endl;
}


void CProcessor::EndDocument()
{
	cout << "EndDocument" << endl;
}


void CProcessor::StartElement(const XMLString& name, const CAttributeList& attrList)
{
	cout
		<< "StartElement" << endl
		<< "\tname=" << name << endl
		<< "\tattrList=(";
	for (int i = 0; i < attrList.GetLength(); i++)
	{
		cout
			<< "(name=" << attrList.GetName(i)
			<< ";type=" << attrList.GetType(i)
			<< ";value=\"" << attrList.GetValue(i) << "\")";
	}
	cout 
		<< ")" << endl
		<< "\tlocation=(" << m_pLocator->GetLineNumber() << ";" << m_pLocator->GetColumnNumber() << ")" << endl;
}


void CProcessor::EndElement(const XMLString& name)
{
	cout
		<< "EndElement" << endl
		<< "\tname=" << name << endl;
}


void CProcessor::Characters(const XMLChar ch[], int start, int length)
{
	cout
		<< "Characters" << endl
		<< "\tvalue=" << XMLString(ch, start, length) << endl;
}


void CProcessor::IgnorableWhitespace(const XMLChar ch[], int start, int length)
{
	cout
		<< "IgnorableWhitespace" << endl
		<< "\tvalue=" << XMLString(ch, start, length) << endl;
}


void CProcessor::ProcessingInstruction(const XMLString& target, const XMLString& data)
{
	cout
		<< "ProcessingInstruction" << endl
		<< "\ttarget=" << target << endl
		<< "\tdata=" << data << endl;
}


void CProcessor::Warning(CSAXException& e)
{
	throw e;
}


void CProcessor::Error(CSAXException& e)
{
	throw e;
}


void CProcessor::FatalError(CSAXException& e)
{
	throw e;
}


void CProcessor::StartDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId)
{
	cout
		<< "StartDTD" << endl
		<< "\tname=" << name << endl
		<< "\tpublicId=" << publicId << endl
		<< "\tsystemId=" << systemId << endl;
}


void CProcessor::EndDTD()
{
	cout
		<< "EndDTD" << endl;
}


void CProcessor::StartEntity(const XMLString& name)
{
	cout
		<< "StartEntity" << endl
		<< "\tname=" << name << endl;
}


void CProcessor::EndEntity(const XMLString& name)
{
	cout
		<< "EndtEntity" << endl
		<< "\tname=" << name << endl;
}


void CProcessor::StartCDATA()
{
	cout
		<< "StartCDATA" << endl;
}


void CProcessor::EndCDATA()
{
	cout
		<< "EndCDATA" << endl;
}


void CProcessor::Comment(const XMLChar ch[], int start, int length)
{
	cout
		<< "Comment" << endl
		<< "\tvalue=" << XMLString(ch, start, length) << endl;
}


void CProcessor::StartNamespaceDeclScope(const XMLString& prefix, const XMLString& uri)
{
	cout
		<< "StartNamespaceDeclScope" << endl
		<< "\tprefix=" << prefix << endl
		<< "\turi=" << uri << endl;
}


void CProcessor::EndNamespaceDeclScope(const XMLString& prefix)
{
	cout
		<< "EndNamespaceDeclScope" << endl
		<< "\tprefix=" << prefix << endl;
}



//
// main
//


int main(int argc, char** argv)
{
	CSAXParser parser;
	CProcessor processor;
	CEntityResolverImp resolver;

	parser.SetEntityResolver(&resolver);
	parser.SetDTDHandler(&processor);
	parser.SetDocumentHandler(&processor);
	parser.SetErrorHandler(&processor);
	parser.SetProperty(PROPERTY_LEXICAL_HANDLER, (CLexicalHandler*) &processor);
	parser.SetProperty(PROPERTY_NAMESPACE_HANDLER, (CNamespaceHandler*) &processor);

	parser.SetFeature(FEATURE_NAMESPACES, true);

	if (argc < 2)
	{
		cerr << "Error: no filename specified" << endl;
		exit(1);
	}

	try
	{
		parser.Parse(argv[1]);
	}
	catch (CXMLException& e)
	{
		cerr << "Error: " << e.GetMsg() << endl;
		exit(1);
	}

	return 0;
}

