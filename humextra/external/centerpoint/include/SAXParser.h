//
// SAXParser.h
//
// $Id: //depot/XML/Main/include/SAXParser.h#6 $
//
// Definition of CSAXParser.
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


#ifndef SAXParser_H_INCLUDED
#define SAXParser_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_Parser_H_INCLUDED
#include "SAX/Parser.h"
#endif
#ifndef SAX_Configurable_H_INCLUDED
#include "SAX/Configurable.h"
#endif
#ifndef SAXParserBase_H_INCLUDED
#include "SAXParserBase.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class XML_API CSAXParser: public CParser, public CConfigurable
	//: This class provides a SAX/SAX2 (Simple API for XML) interface to expat, 
	//+ the XML parser toolkit.
	//+ The following SAX2 core features are supported:
	//+   - http://xml.org/sax/features/external-general-entities
	//+   - http://xml.org/sax/features/namespaces
	//+   - http://xml.org/sax/features/use-locator
	//+ The following SAX2 core properties are supported:
	//+   - http://xml.org/sax/properties/namespace-sep
	//+   - http://xml.org/sax/handlers/LexicalHandler
	//+   - http://xml.org/sax/handlers/NamespaceHandler
	//+ In addition, the following proprietary properties are supported:
	//+   - http://cpointc.com/xml/parser/DocumentBase
	//+     Sets the document base URI. The parser uses it to resolve
	//+     relative systemId's.
	//+   - http://cpointc.com/xml/parser/DefaultHandler
	//.     Used for passing a DefaultHandler interface to the parser.
{
public:
	CSAXParser();
	CSAXParser(const XMLString& encoding);
		//: Note: This is the only way to pass an encoding information to
		//. the parser. The encoding information in the InputSource is ignored!

	virtual ~CSAXParser();

	void SetLocale(const XMLString& locale);
		//; Only provided for compatibility. Locale information is ignored.

	void SetEntityResolver(CEntityResolver* pResolver);
		//; Allows an application to register a custom entity resolver.

	void SetDTDHandler(CDTDHandler* pDTDHandler);
		//; Allow an application to register a DTD event handler.

	void SetDocumentHandler(CDocumentHandler* pDocumentHandler);
		//; Allow an application to register a document event handler.

	void SetErrorHandler(CErrorHandler* pErrorHandler);
		//; Allow an application to register an error event handler.

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

private:
	CSAXParserBase m_base;
	bool           m_featureNamespaces;
	bool           m_featureUseLocator;
	bool           m_featureExtGenEntities;
	XMLString      m_propertyNamespaceSep;
};


XML_END


#endif // SAXParserBase_H_INCLUDED
