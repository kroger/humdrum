//
// EventListener.h
//
// $Id: //depot/XML/Main/include/DOM/EventListener.h#3 $
//
// Document Object Model (Events) Level 2
//
// Interface EventListener
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


#ifndef DOM_EventListener_H_INCLUDED
#define DOM_EventListener_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class Event;


class XML_API EventListener
	//: The EventListener interface is the primary method for handling events. Users 
	//+ implement the EventListener interface and register their
	//+ listener on an EventTarget using the AddEventListener method. 
	//+ The users should also remove their EventListener from its
	//+ EventTarget after they have completed using the listener.
	//+ When a Node is copied using the cloneNode method the EventListeners 
	//+ attached to the source Node are not attached to the copied
	//+ Node. If the user wishes the same EventListeners to be added to the 
	//. newly created copy the user must add them manually.
{
public:
	virtual void handleEvent(Event* evt) = 0;
		//: This method is called whenever an event occurs of the 
		//. type for which the EventListener interface was registered.

	// IMPLEMENTATION
	virtual ~EventListener();
};


XML_END


#endif // DOM_EventListener_H_INCLUDED
