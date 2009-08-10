//
// DocumentEvent.h
//
// $Id: //depot/XML/Main/include/DOM/DocumentEvent.h#4 $
//
// Document Object Model (Events) Level 2
//
// Interface DocumentEvent
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


#ifndef DOM_DocumentEvent_H_INCLUDED
#define DOM_DocumentEvent_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class Event;


class XML_API DocumentEvent
	//: The DocumentEvent interface provides a mechanism by which the user can 
	//+ create an Event of a type supported by the implementation. It is
	//+ expected that the DocumentEvent interface will be implemented on the same 
	//+ object which implements the Document interface in an
	//. implementation which supports the Event model.
{
public:
	virtual Event* createEvent(const XMLString& eventType) const = 0;
		//; Creates an event of the specified type.

	// IMPLEMENTATION
	virtual ~DocumentEvent();
};


XML_END


#endif // DOM_DocumentEvent_H_INCLUDED
