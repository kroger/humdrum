//
// XMLReader.h
//
// $Id: //depot/XML/Main/include/SAX/XMLReader.h#3 $
//
// SAX2 XMLReader Interface.
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


#ifndef SAX_XMLReader_H_INCLUDED
#define SAX_XMLReader_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class CEntityResolver;
class CDTDHandler;
class CContentHandler;
class CErrorHandler;
class CInputSource;
class CLexicalHandler;
class CNamespaceHandler;


class XML_API CXMLReader
	//: Interface for reading an XML document using callbacks. 
	//+ Note: despite its name, this interface does not extend the 
	//+ standard Java Reader interface, because reading XML is a
	//+ fundamentally different activity than reading character data.
	//+ XMLReader is the interface that an XML parser's SAX2 driver must 
	//+ implement. This interface allows an application to set and
	//+ query features and properties in the parser, to register event handlers 
	//+ for document processing, and to initiate a document parse.
	//+ All SAX interfaces are assumed to be synchronous: the parse methods must not 
	//+ return until parsing is complete, and readers
	//. must wait for an event-handler callback to return before reporting the next event.
{
public:
	virtual void SetEntityResolver(CEntityResolver* pResolver) = 0;
		//; Allow an application to register an entity resolver.

	virtual void SetDTDHandler(CDTDHandler* pDTDHandler) = 0;
		//; Allow an application to register a DTD event handler.

	virtual void SetContentHandler(CContentHandler* pContentHandler) = 0;
		//; Allow an application to register a content event handler.

	virtual void SetErrorHandler(CErrorHandler* pErrorHandler) = 0;
		//; Allow an application to register an error event handler.

	virtual CEntityResolver* GetEntityResolver() const = 0;
		//; Return the current entity resolver.

	virtual CDTDHandler* GetDTDHandler() const = 0;
		//; Return the current DTD handler.

	virtual CContentHandler* GetContentHandler() const = 0;
		//; Return the current content handler.

	virtual CErrorHandler* GetErrorHandler() const = 0;
		//; Return the current error handler.
	
	virtual void SetFeature(const XMLString& featureId, bool state) = 0;
		//; Set the state of a feature.

	virtual bool GetFeature(const XMLString& featureId) const = 0;
		//; Look up the value of a feature.

	virtual void SetProperty(const XMLString& propertyId, const XMLString& value) = 0;
		//; Set the value of a property.

	virtual void SetProperty(const XMLString& propertyId, void* value) = 0;
		//; Set the value of a property.

	virtual void* GetProperty(const XMLString& propertyId) const = 0;
		//: Look up the value of a property.
		//. String values are returned as XMLChar*

	virtual void Parse(CInputSource* pSource) = 0;
		//; Parse an XML document.

	virtual void Parse(const XMLString& systemId) = 0;
		//; Parse an XML document from a system identifier.

	virtual ~CXMLReader();
};


XML_END


#endif // SAX_XMLReader_H_INCLUDED
