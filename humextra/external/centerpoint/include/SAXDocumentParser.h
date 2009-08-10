//
// SAXDocumentParser.h
//
// $Id: //depot/XML/Main/include/SAXDocumentParser.h#4 $
//
// Definition of CSAXDocumentParser.
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


#ifndef SAXDocumentParser_H_INCLUDED
#define SAXDocumentParser_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_Parser_H_INCLUDED
#include "SAX/Parser.h"
#endif
#ifndef SAX_Configurable_H_INCLUDED
#include "SAX/Configurable.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class CXMLObject;
class CXMLDocument;
class CXMLElement;
class CXMLCharacterData;
class CXMLComment;
class CXMLGenericData;
class CXMLProcessingInstruction;
class CXMLExternalEntity;
class CXMLCDATASection;
class CXMLNotationDeclaration;
class CXMLUnparsedEntityDeclaration;
class CLexicalHandler;


class XML_API CSAXDocumentParser: public CParser, public CConfigurable
	//: This class provides a SAX/SAX2 (Simple API for XML) interface for
	//+ iterating over an XML document (see CXMLDocument, XMLDocument.h).
	//+ The following SAX2 core properties are supported:
	//+   - http://xml.org/sax/handlers/LexicalHandler
{
public:
	CSAXDocumentParser();
		//: Note: This is the only way to pass an encoding information to
		//. the parser. The encoding information in the InputSource is ignored!

	virtual ~CSAXDocumentParser();

	void SetLocale(const XMLString& locale);
		//; Only provided for compatibility. Locale information is ignored.

	void SetEntityResolver(CEntityResolver* pResolver);
		//: Allows an application to register a custom entity resolver.
		//. Ignored by this parser.

	void SetDTDHandler(CDTDHandler* pDTDHandler);
		//; Allow an application to register a DTD event handler.

	void SetDocumentHandler(CDocumentHandler* pDocumentHandler);
		//; Allow an application to register a document event handler.

	void SetErrorHandler(CErrorHandler* pErrorHandler);
		//: Allow an application to register an error event handler.
		//. Currently ignored by this parser.

	void Parse(const CXMLObject* pNode);
		//; Parses an XML document.

	// Configurable interface
	virtual void SetFeature(const XMLString& featureId, bool state);
		//: Enables or disables the specified feature.
		//. Throws an exception if the feature is unknown.

	virtual bool GetFeature(const XMLString& featureId) const;
		//: Returns true if the specified feature is available.
		//. Throws an exception if the feature is unknown.

	virtual void SetProperty(const XMLString& propertyId, const XMLString& value);
		//: Sets the specified property to a string.
		//. Throws an exception if the property is unknown or not supported.

	virtual void SetProperty(const XMLString& propertyId, void* value);
		//: Sets the specified property to a pointer value.
		//. Throws an exception if the property is unknown or not supported.

	virtual void* GetProperty(const XMLString& propertyId) const;
		//: Returns the value of the specified property.
		//+ String values are returned as XMLChar*
		//. Throws an exception if the property is unknown or not supported.

protected:
	void Parse(CInputSource* pSource);
		//; Does nothing (inherited from CParser).

	void Parse(const XMLString& systemId);
		//; Does nothing (inherited from CParser).

	void Traverse(const CXMLObject* pObject) const;
	void Process(const CXMLObject* pObject) const;
	void ProcessElement(const CXMLElement* pElem) const;
	void ProcessCharacterData(const CXMLCharacterData* pData) const;
	void ProcessComment(const CXMLComment* pObj) const;
	void ProcessGenericData(const CXMLGenericData* pData) const;
	void ProcessPI(const CXMLProcessingInstruction* pPI) const;
	void ProcessExternalEntity(const CXMLExternalEntity* pObj) const;
	void ProcessCDATASection(const CXMLCDATASection* pCDATA) const;
	void ProcessNotationDecl(const CXMLNotationDeclaration* pDecl) const;
	void ProcessUnparsedEntityDecl(const CXMLUnparsedEntityDeclaration* pDecl) const;
	void ProcessDocument(const CXMLObject* pObj) const;

private:
	CDTDHandler*      m_pDTDHandler;
	CDocumentHandler* m_pDocumentHandler;
	CLexicalHandler*  m_pLexicalHandler;
};


XML_END


#endif // SAXDocumentParserBase_H_INCLUDED
