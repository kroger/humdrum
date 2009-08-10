//
// SAX2Parser.h
//
// $Id: //depot/XML/Main/include/SAX2Parser.h#7 $
//
// Definition of CSAX2Parser.
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


#ifndef SAX2Parser_H_INCLUDED
#define SAX2Parser_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_Parser_H_INCLUDED
#include "SAX/XMLReader.h"
#endif
#ifndef SAX_Configurable_H_INCLUDED
#include "SAX/Configurable.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class CSAX2ParserBase;
class CEntityResolver;
class CDTDHandler;
class CContentHandler;
class CErrorHandler;
class CInputSource;
class CLexicalHandler;
class CDeclHandler;
class CNamespaceHandler;
class CAnyHandler;
class CISO88591Filter;


class XML_API CSAX2Parser: public CXMLReader
	//: This class provides a SAX2 (Simple API for XML) interface to expat, 
	//+ the XML parser toolkit.
	//+ The following SAX2 core features are supported:
	//+   - http://xml.org/sax/features/external-general-entities
	//+   - http://xml.org/sax/features/external-parameter-entities
	//+   - http://xml.org/sax/features/namespaces
	//+   - http://xml.org/sax/features/namespace-prefixes (READ ONLY!)
	//+ The following SAX2-ext properties are supported:
	//+   - http://xml.org/sax/handlers/LexicalHandler
	//+   - http://xml.org/sax/handlers/DeclHandler
	//+ In addition, the following proprietary properties are supported:
	//+   - http://cpointc.com/xml/parser/DocumentBase
	//+     Sets the document base URI. The parser uses it to resolve
	//+     relative systemId's.
	//+   - http://cpointc.com/xml/parser/DefaultHandler
	//+     Used for passing a DefaultHandler interface to the parser.
	//+ The parser also support the following proprietary feature:
	//+   - http://www.cpointc.com/xml/parser/use-iso-8859-1
	//.     Encodes all characters in ISO 8859-1 instead of UTF-8.
{
public:
	CSAX2Parser();
	CSAX2Parser(const XMLString& encoding);

	virtual ~CSAX2Parser();

	void SetEntityResolver(CEntityResolver* pResolver);
		//; Allows an application to register a custom entity resolver.

	void SetDTDHandler(CDTDHandler* pDTDHandler);
		//; Allow an application to register a DTD event handler.

	void SetContentHandler(CContentHandler* pContentHandler);
		//; Allow an application to register a content event handler.

	void SetErrorHandler(CErrorHandler* pErrorHandler);
		//; Allow an application to register an error event handler.

	CEntityResolver* GetEntityResolver() const;
		//; Return the current entity resolver.

	CDTDHandler* GetDTDHandler() const;
		//; Return the current DTD handler.

	CContentHandler* GetContentHandler() const;
		//; Return the current content handler.

	CErrorHandler* GetErrorHandler() const;
		//; Return the current error handler.

	void Parse(CInputSource* pSource);
		//; Parses an XML document.

	void Parse(const XMLString& systemId);
		//; Parses an XML document from a system identifier (URI).

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
	CSAX2ParserBase* MakeParser(const XMLString& encoding);

private:
	// Core features recognized by the parser
	static const XMLString FEATURE_VALIDATION;
	static const XMLString FEATURE_NAMESPACES;
	static const XMLString FEATURE_NAMESPACE_PREFIXES;
	static const XMLString FEATURE_EXT_GEN_ENTITIES;
	static const XMLString FEATURE_EXT_PAR_ENTITIES;
	static const XMLString FEATURE_STRING_INTERNING;
	// Core properties recognized by the parser
	static const XMLString PROPERTY_DECL_HANDLER;
	static const XMLString PROPERTY_LEXICAL_HANDLER;
	// Non-standard extensions
	static const XMLString FEATURE_USE_ISO_8859_1;
	static const XMLString PROPERTY_DOCUMENT_BASE;
	static const XMLString PROPERTY_DEFAULT_HANDLER;

	XMLString        m_encoding;
	bool             m_featureNamespaces;
	bool             m_featureNamespacePrefixes;
	bool             m_featureExtGenEntities;
	bool             m_featureExtParEntities;
	bool             m_featureUseISO88591;
	XMLString        m_documentBase;
	CEntityResolver* m_pEntityResolver;
	CDTDHandler*     m_pDTDHandler;
	CContentHandler* m_pContentHandler;
	CErrorHandler*   m_pErrorHandler;
	CLexicalHandler* m_pLexicalHandler;
	CDeclHandler*    m_pDeclHandler;
	CAnyHandler*     m_pDefaultHandler;
	CISO88591Filter* m_pContentFilter;
};


XML_END


#endif // SAX2ParserBase_H_INCLUDED
