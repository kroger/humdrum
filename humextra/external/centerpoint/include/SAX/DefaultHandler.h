//
// DefaultHandler.h
//
// $Id: //depot/XML/Main/include/SAX/DefaultHandler.h#5 $
//
// SAX2 DefaultHandler Class.
//
// Note that the function of this class has changed since
// the old SAX release, where it was a proprietary extension
// to the SAX interface. SAX2 now defines its own DefaultHandler
// interface, so this class had to change. The old functionality
// of the CDefaultHandler is now available from the CAnyHandler
// class.
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


#ifndef SAX_DefaultHandler_H_INCLUDED
#define SAX_DefaultHandler_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_EntityResolver_H_INCLUDED
#include "SAX/EntityResolver.h"
#endif
#ifndef SAX_DTDHandler_H_INCLUDED
#include "SAX/DTDHandler.h"
#endif
#ifndef SAX_ContentHandler_H_INCLUDED
#include "SAX/ContentHandler.h"
#endif
#ifndef SAX_ErrorHandler_H_INCLUDED
#include "SAX/ErrorHandler.h"
#endif


XML_BEGIN


class XML_API CDefaultHandler: public CEntityResolver, public CDTDHandler, public CContentHandler, public CErrorHandler
	//: Default base class for SAX2 event handlers. 
	//+ This class is available as a convenience base class for SAX2 applications: 
	//+ it provides default implementations for all of the
	//+ callbacks in the four core SAX2 handler classes:
	//+      EntityResolver 
	//+      DTDHandler 
	//+      ContentHandler 
	//+      ErrorHandler 
	//+ Application writers can extend this class when they need to implement only 
	//+ part of an interface; parser writers can instantiate this
	//. class to provide default handlers when the application has not supplied its own.
{
public:
	CInputSource* ResolveEntity(const XMLString* publicId, const XMLString& systemId);
	void DoneWithInputSource(CInputSource* pSource);
	void NotationDecl(const XMLString& name, const XMLString* publicd, const XMLString* systemId);
	void UnparsedEntityDecl(const XMLString& name, const XMLString* publicId, const XMLString& systemId, const XMLString& notationName);
	void SetDocumentLocator(const CLocator& loc) ;
	void StartDocument();
	void EndDocument();
	void StartElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const CAttributes& attrList);
	void EndElement(const XMLString& uri, const XMLString& localName, const XMLString& qname);
	void Characters(const XMLChar ch[], int start, int length);
	void IgnorableWhitespace(const XMLChar ch[], int start, int length);
	void ProcessingInstruction(const XMLString& target, const XMLString& data);
	void StartPrefixMapping(const XMLString& prefix, const XMLString& uri);
	void EndPrefixMapping(const XMLString& prefix);
	void SkippedEntity(const XMLString& prefix);
	void Warning(CSAXException& e);
	void Error(CSAXException& e);
	void FatalError(CSAXException& e);

	virtual ~CDefaultHandler();
};


XML_END


#endif // SAX_DefaultHandler_H_INCLUDED
