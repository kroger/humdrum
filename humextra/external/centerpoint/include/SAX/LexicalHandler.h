//
// LexicalHandler.h
//
// $Id: //depot/XML/Main/include/SAX/LexicalHandler.h#5 $
//
// SAX2-ext LexicalHandler Interface.
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


#ifndef SAX_LexicalHandler_H_INCLUDED
#define SAX_LexicalHandler_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class XML_API CLexicalHandler
{
public:
	virtual void StartDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId) = 0;
	virtual void EndDTD() = 0;
	virtual void StartEntity(const XMLString& name) = 0;
	virtual void EndEntity(const XMLString& name) = 0;
	virtual void StartCDATA() = 0;
	virtual void EndCDATA() = 0;
	virtual void Comment(const XMLChar ch[], int start, int length) = 0; 

	virtual ~CLexicalHandler();
};


XML_END


#endif // SAX_LexicalHandler_H_INCLUDED
