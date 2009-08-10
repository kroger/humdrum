//
// XMLDocumentBuilder.h
//
// $Id: //depot/XML/Main/include/XMLDocumentBuilder.h#4 $
//
// Definition of CXMLDocumentBuilder.
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


#ifndef XMLDocumentBuilder_H_INCLUDED
#define XMLDocumentBuilder_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_HandlerBase_H_INCLUDED
#include "SAX/HandlerBase.h"
#endif
#ifndef SAX_LexicalHandler_H_INCLUDED
#include "SAX/LexicalHandler.h"
#endif
#ifndef SAXParser_H_INCLUDED
#include "SAXParser.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class CXMLDocument;
class CXMLFactory;
class CXMLContainer;
class CXMLObject;


class XML_API CXMLDocumentBuilder: public CHandlerBase, public CLexicalHandler
	//: This class builds a document tree from
	//+ an XML stream. It uses an underlying SAX
	//. parser for parsing the stream.
{
public:
	CXMLDocumentBuilder();
	CXMLDocumentBuilder(const XMLString& encoding);
		//; Sets the encoding for the underlying SAX parser.

	virtual ~CXMLDocumentBuilder();

	virtual void BuildDocument(XMLByteInputStream* pIstr, CXMLDocument* pDoc, CXMLFactory* pFact);
		//; Builds the document from the given input stream.

	virtual void BuildDocument(CInputSource* pSource, CXMLDocument* pDoc, CXMLFactory* pFact);
		//; Builds the document from the given input source.

	virtual void BuildDocument(const XMLString& systemId, CXMLDocument* pDoc, CXMLFactory* pFact);
		//; Builds the document from the local file named systemId.

	virtual void SetLocale(const XMLString& locale);
		//; Sets the locale for the underlying SAX parser.

	virtual void EnableNamespaceProcessing(bool flag = true);
		//; Enables namespace processing for the underlying SAX parser.

	virtual void SetNamespaceSeparator(const XMLString& sep);
		//; Sets the namespace separator if namespace processing is allowed.

	virtual void SetBase(const XMLString& base);
		//: Sets the base URI for the underlying parser. The base URI is used
		//. for resolving relative systemId's.

	virtual void IncludeExternalGeneralEntities(bool flag = true);
		//; Enable automatic inclusion of external general entities.

protected:
	// CHandlerBase methods
	CInputSource* ResolveEntity(const XMLString* publicId, const XMLString& systemId);
	void NotationDecl(const XMLString& name, const XMLString* publicd, const XMLString* systemId);
	void UnparsedEntityDecl(const XMLString& name, const XMLString* publicId, const XMLString& systemId, const XMLString& notationName);
	void StartElement(const XMLString& name, const CAttributeList& attrList);
	void EndElement(const XMLString& name);
	void Characters(const XMLChar ch[], int start, int length);
	void IgnorableWhitespace(const XMLChar ch[], int start, int length);
	void ProcessingInstruction(const XMLString& target, const XMLString& data);
	void Warning(CSAXException& e);
	void Error(CSAXException& e);
	void FatalError(CSAXException& e);

	// CLexicalHandler methods
	void StartDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId);
	void EndDTD();
	void StartEntity(const XMLString& name);
	void EndEntity(const XMLString& name);
	void StartCDATA();
	void EndCDATA();
	void Comment(const XMLChar ch[], int start, int length); 

private:
	// State information
	CSAXParser     m_parser;
	CXMLDocument*  m_pDoc;
	CXMLFactory*   m_pFact;
	CXMLContainer* m_pParent;
	CXMLObject*    m_pPrev;
	bool           m_inCDATA;
	bool           m_includeExtGenEntities;
};


XML_END


#endif // XMLDocumentBuilder_H_INCLUDED
