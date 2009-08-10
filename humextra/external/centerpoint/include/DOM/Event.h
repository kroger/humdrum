//
// Event.h
//
// $Id: //depot/XML/Main/include/DOM/Event.h#6 $
//
// Document Object Model (Events) Level 2
//
// Interface Event
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


#ifndef DOM_Event_H_INCLUDED
#define DOM_Event_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif
#ifndef DOM_DOMObject_H_INCLUDED
#include "DOM/DOMObject.h"
#endif
#ifndef DOM_DOMTimeStamp_H_INCLUDED
#include "DOM/DOMTimeStamp.h"
#endif


XML_BEGIN


class EventTarget;


class XML_API Event: public DOMObject
	//: The Event interface is used to provide contextual information about an 
	//+ event to the handler processing the event. An object which
	//+ implements the Event interface is generally passed as the first 
	//+ parameter to an event handler. More specific context information is passed to
	//+ event handlers by deriving additional interfaces from Event which contain 
	//+ information directly relating to the type of event they accompany.
	//+ These derived interfaces are also implemented by the object passed 
	//. to the event listener.
{
public:
	enum PhaseType
	{
		CAPTURING_PHASE = 1, //; The event is currently being evaluated at the target EventTarget. 
		AT_TARGET       = 2, //; The current event phase is the bubbling phase. 
		BUBBLING_PHASE  = 3  //; The current event phase is the capturing phase. 
	};

	const XMLString& getType() const;
		//; The name of the event (case-insensitive). The name must be an XML name.

	EventTarget* getTarget() const;
		//; Used to indicate the EventTarget to which the event was originally dispatched.

	EventTarget* getCurrentTarget() const;
		//: Used to indicate the EventTarget whose EventListeners are currently being 
		//. processed. This is particularly useful during capturing and bubbling.

	PhaseType getEventPhase() const;
		//; Used to indicate which phase of event flow is currently being evaluated.

	bool getBubbles() const;
		//: Used to indicate whether or not an event is a bubbling event. 
		//. If the event can bubble the value is true, else the value is false.

	bool getCancelable() const;
		//: Used to indicate whether or not an event can have its default action 
		//+ prevented. If the default action can be prevented the value is
		//. true, else the value is false.

	DOMTimeStamp getTimeStamp() const;
		//: Used to specify the time (in milliseconds relative to the epoch) at 
		//+ which the event was created. Due to the fact that some
		//+ systems may not provide this information the value of timeStamp may 
		//+ be not available for all events. When not available, a
		//+ value of 0 will be returned. Examples of epoch time are the time of the 
		//+ system start or 0:0:0 UTC 1st January 1970.
		//. This implementation always returns 0.

	void stopPropagation();
		//: The stopPropagation method is used prevent further propagation of an 
		//+ event during event flow. If this method is called by
		//+ any EventListener the event will cease propagating through the tree. 
		//+ The event will complete dispatch to all listeners on the
		//+ current EventTarget before event flow stops. This method may be used 
		//. during any stage of event flow. 

	void preventDefault();
		//: If an event is cancelable, the preventDefault method is used to signify 
		//+ that the event is to be canceled, meaning any default
		//+ action normally taken by the implementation as a result of 
		//+ the event will not occur. If, during any stage of event flow, the
		//+ preventDefault method is called the event is canceled. Any default 
		//+ action associated with the event will not occur. Calling
		//+ this method for a non-cancelable event has no effect. Once 
		//+ preventDefault has been called it will remain in effect throughout
		//+ the remainder of the event's propagation. This method may be 
		//. used during any stage of event flow. 

	void initEvent(const XMLString& eventType, bool canBubble, bool isCancelable);
		//: The initEvent method is used to initialize the value of an 
		//+ Event created through the DocumentEvent interface. This method
		//+ may only be called before the Event has been dispatched via the 
		//+ dispatchEvent method, though it may be called multiple
		//+ times during that phase if necessary. If called multiple 
		//+ times the final invocation takes precedence. If called from 
		//+ a subclass of Event interface only the values specified in the 
		//. initEvent method are modified, all other attributes are left unchanged. 

	// IMPLEMENTATION
	Event(const XMLString& type);
	Event(const XMLString& type, EventTarget* pTarget, bool canBubble, bool isCancelable);
	virtual ~Event();

	bool isCanceledNP() const;
		//; returns true if and only if the event has been cancelled.

	bool isStoppedNP() const;
		//; returns true if and only if propagation of the event has been stopped.

	void setTargetNP(EventTarget* pTarget);
		//; sets the target

	void setCurrentPhaseNP(PhaseType phase);
		//; sets the current phase

	void setCurrentTargetNP(EventTarget* pTarget);
		//; sets the current target

private:
	XMLString    m_type;
	EventTarget* m_pTarget;
	EventTarget* m_pCurrentTarget;
	PhaseType    m_currentPhase;
	bool         m_bubbles;
	bool         m_cancelable;
	bool         m_canceled;
	bool         m_stopPropagation;
	bool         m_preventDefault;
};


XML_END


#endif // DOM_Event_H_INCLUDED
