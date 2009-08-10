//
// AnyHandler.h
//
// $Id: //depot/XML/Main/include/SAX/AnyHandler.h#3 $
//
// AnyHandler Interface.
// This is a proprietary extension to the SAX2 interfaces.
// The AnyHandler can be used to handle any events for which
// no SAX handler exists. This class replaces the old
// DefaultHandler interface, which is now part of the SAX2
// specification, although with another meaning.
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


#ifndef SAX_AnyHandler_H_INCLUDED
#define SAX_AnyHandler_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class XML_API CAnyHandler
	//: The AnyHandler can be used to handle any events for which
	//. no SAX handler exists.
{
public:
	virtual void Any(const XMLChar ch[], int start, int length) = 0;
		//; Handle any event.

	virtual ~CAnyHandler();
};


XML_END


#endif // SAX_AnyHandler_H_INCLUDED
