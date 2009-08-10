//
// DeclHandler.h
//
// $Id: //depot/XML/Main/include/SAX/DeclHandler.h#4 $
//
// SAX2-ext DeclHandler Interface.
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


#ifndef SAX_DeclHandler_H_INCLUDED
#define SAX_DeclHandler_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class XML_API CDeclHandler
	//: This is an optional extension handler for SAX2 to provide information 
	//+ about DTD declarations in an XML document. XML
	//+ readers are not required to support this handler, and this handler is 
	//+ not included in the core SAX2 distribution.
	//+
	//+ Note that data-related DTD declarations (unparsed entities and notations) 
	//+ are already reported through the DTDHandler interface.
	//+ If you are using the declaration handler together with a lexical handler, 
	//+ all of the events will occur between the startDTD and the endDTD events.
	//+ To set the DeclHandler for an XML reader, use the setProperty method 
	//+ with the propertyId "http://xml.org/sax/properties/declaration-handler". 
	//+ If the reader does not support declaration events, it will throw a
	//+ SAXNotRecognizedException or a SAXNotSupportedException when you attempt to 
	//. register the handler.
{
public:
	virtual void AttributeDecl(const XMLString& eName, const XMLString& aName, const XMLString* valueDefault, const XMLString* value) = 0;
		//; Report an attribute type declaration.

	virtual void ElementDecl(const XMLString& name, const XMLString& model) = 0;
		//; Report an element type declaration.

	virtual void ExternalEntityDecl(const XMLString& name, const XMLString* publicId, const XMLString& systemId) = 0;
		//; Report an external entity declaration.

	virtual void InternalEntityDecl(const XMLString& name, const XMLString& value) = 0;
		//; Report an internal entity declaration.

	virtual ~CDeclHandler();
};


XML_END


#endif // SAX_DeclHandler_H_INCLUDED
