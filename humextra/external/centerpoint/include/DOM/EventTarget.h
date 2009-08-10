//
// EventTarget.h
//
// $Id: //depot/XML/Main/include/DOM/EventTarget.h#4 $
//
// Document Object Model (Events) Level 2
//
// Interface EventTarget
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


#ifndef DOM_EventTarget_H_INCLUDED
#define DOM_EventTarget_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef DOM_DOMObject_H_INCLUDED
#include "DOM/DOMObject.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class EventListener;
class Event;


class XML_API EventTarget: public DOMObject
	//: The EventTarget interface is implemented by all Nodes in an implementation which supports the DOM Event Model.
	//+ Therefore, this interface can be obtained by using binding-specific casting methods on an instance of the Node interface.
	//+ The interface allows registration and removal of EventListeners on an EventTarget and dispatch of events to that
	//. EventTarget.
{
public:
	virtual void addEventListener(const XMLString& type, EventListener* listener, bool useCapture) = 0;
		//: This method allows the registration of event listeners on 
		//+ the event target. If an EventListener is added to an
		//+ EventTarget while it is processing an event, it will not 
		//+ be triggered by the current actions but may be triggered
		//+ during a later stage of event flow, such as the bubbling phase.
		//+ If multiple identical EventListeners are registered on the same 
		//+ EventTarget with the same parameters the duplicate instances are 
		//+ discarded. They do not cause the EventListener to be called twice and since they are
		//. discarded they do not need to be removed with the removeEventListener method. 	
	
	virtual void removeEventListener(const XMLString& type, EventListener* listener, bool useCapture) = 0;
		//: This method allows the removal of event listeners from the event 
		//+ target. If an EventListener is removed from an EventTarget while it is 
		//+ processing an event, it will not be triggered by the current actions. 
		//+ EventListeners can never be invoked after being removed.
		//+ Calling removeEventListener with arguments which do not identify 
		//. any currently registered EventListener on the EventTarget has no effect. 

	virtual bool dispatchEvent(Event* evt) = 0;
		//: This method allows the dispatch of events into the implementations 
		//+ event model. Events dispatched in this manner will have the same capturing and 
		//+ bubbling behavior as events dispatched directly by the
		//+ implementation. The target of the event is the EventTarget on 
		//. which dispatchEvent is called. 

	// IMPLEMENTATION
	virtual ~EventTarget();
};


XML_END


#endif // DOM_EventTarget_H_INCLUDED
