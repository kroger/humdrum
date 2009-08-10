//
// DocumentHandler.h
//
// $Id: //depot/XML/Main/include/SAX/DocumentHandler.h#5 $
//
// SAX DocumentHandler Interface.
//
// DEPRECATED. Use ContentHandler.h instead.
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


#ifndef SAX_DocumentHandler_H_INCLUDED
#define SAX_DocumentHandler_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class CLocator;
class CAttributeList;


class XML_API CDocumentHandler
{
public:
	virtual void SetDocumentLocator(const CLocator& loc) = 0;
	virtual void StartDocument() = 0;
	virtual void EndDocument() = 0;
	virtual void StartElement(const XMLString& name, const CAttributeList& attrList) = 0;
	virtual void EndElement(const XMLString& name) = 0;
	virtual void Characters(const XMLChar ch[], int start, int length) = 0;
	virtual void IgnorableWhitespace(const XMLChar ch[], int start, int length) = 0;
	virtual void ProcessingInstruction(const XMLString& target, const XMLString& data) = 0;

	virtual ~CDocumentHandler();
};


XML_END


#endif // SAX_DocumentHandler_H_INCLUDED
