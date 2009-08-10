//
// MutationEvent.h
//
// $Id: //depot/XML/Main/include/DOM/MutationEvent.h#6 $
//
// Document Object Model (Events) Level 2
//
// Interface MutationEvent
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


#ifndef DOM_MutationEvent_H_INCLUDED
#define DOM_MutationEvent_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef DOM_Event_H_INCLUDED
#include "DOM/Event.h"
#endif


XML_BEGIN


class Node;


class XML_API MutationEvent: public Event
	//: The MutationEvent interface provides specific contextual 
	//. information associated with Mutation events.
{
public:
	enum AttrChangeType
	{
		MODIFICATION = 1, //; The Attr was modified in place.
		ADDITION     = 2, //; The Attr was just added. 
		REMOVAL      = 3  //; The Attr was just removed. 
	};

	Node* getRelatedNode() const;
		//: relatedNode is used to identify a secondary node related to a mutation 
		//+ event. For example, if a mutation event is dispatched
		//+ to a node indicating that its parent has changed, the relatedNode is the 
		//+ changed parent. If an event is instead dispatched to a
		//+ subtree indicating a node was changed within it, the relatedNode is 
		//+ the changed node. In the case of the DOMAttrModified
		//. event it indicates the Attr node which was modified, added, or removed.

	const XMLString& getPrevValue() const;
		//: prevValue indicates the previous value of the Attr node in DOMAttrModified 
		//. events, and of the CharacterData node in DOMCharDataModified events.

	const XMLString& getNewValue() const;
		//: newValue indicates the new value of the Attr node in DOMAttrModified 
		//. events, and of the CharacterData node in DOMCharDataModified events.

	const XMLString& getAttrName() const;
		//; attrName indicates the name of the changed Attr node in a DOMAttrModified event.

	AttrChangeType getAttrChange() const;
		//: attrChange indicates the type of change which triggered the 
		//+ DOMAttrModified event. The values can be MODIFICATION,
		//. ADDITION, or REMOVAL.

	void initMutationEvent(const XMLString& type, bool canBubble, bool cancelable, Node* relatedNode, 
	                       const XMLString& prevValue, const XMLString& newValue, const XMLString& attrName, AttrChangeType change);
		//: The initMutationEvent method is used to initialize the value of a 
		//+ MutationEvent created through the DocumentEvent
		//+ interface. This method may only be called before the MutationEvent 
		//+ has been dispatched via the dispatchEvent method,
		//+ though it may be called multiple times during that phase if 
		//+ necessary. If called multiple times, the final invocation takes
		//. precedence.

	// Event Types
	static const XMLString DOMSubtreeModified;
	static const XMLString DOMNodeInserted;
	static const XMLString DOMNodeRemoved;
	static const XMLString DOMNodeRemovedFromDocument;
	static const XMLString DOMNodeInsertedIntoDocument;
	static const XMLString DOMAttrModified;
	static const XMLString DOMCharacterDataModified;

	// IMPLEMENTATION
	MutationEvent(const XMLString& type);
	MutationEvent(const XMLString& type, EventTarget* pTarget, bool canBubble, bool cancelable, Node* relatedNode);
	MutationEvent(const XMLString& type, EventTarget* pTarget, bool canBubble, bool cancelable, Node* relatedNode, 
				  const XMLString& prevValue, const XMLString& newValue, const XMLString& attrName, AttrChangeType change);
	virtual ~MutationEvent();

private:
	XMLString      m_prevValue;
	XMLString      m_newValue;
	XMLString      m_attrName;
	AttrChangeType m_change;
	Node*          m_pRelatedNode;
};


XML_END


#endif // DOM_MutationEvent_H_INCLUDED
