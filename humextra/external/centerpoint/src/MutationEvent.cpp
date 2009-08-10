//
// MutationEvent.cpp
//
// $Id: //depot/XML/Main/src/MutationEvent.cpp#5 $
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


#include "DOM/MutationEvent.h"
#include "XMLUtils.h"


XML_BEGIN


const XMLString MutationEvent::DOMSubtreeModified          = CXMLUtils::AsciiToXML("DOMSubtreeModified");
const XMLString MutationEvent::DOMNodeInserted             = CXMLUtils::AsciiToXML("DOMNodeInserted");
const XMLString MutationEvent::DOMNodeRemoved              = CXMLUtils::AsciiToXML("DOMNodeRemoved");
const XMLString MutationEvent::DOMNodeRemovedFromDocument  = CXMLUtils::AsciiToXML("DOMNodeRemovedFromDocument");
const XMLString MutationEvent::DOMNodeInsertedIntoDocument = CXMLUtils::AsciiToXML("DOMNodeInsertedIntoDocument");
const XMLString MutationEvent::DOMAttrModified             = CXMLUtils::AsciiToXML("DOMAttrModified");
const XMLString MutationEvent::DOMCharacterDataModified    = CXMLUtils::AsciiToXML("DOMCharacterDataModified");


MutationEvent::MutationEvent(const XMLString& type): Event(type, NULL, true, false)
{
	m_pRelatedNode = NULL;
}


MutationEvent::MutationEvent(const XMLString& type, EventTarget* pTarget, bool canBubble, bool cancelable, Node* relatedNode):
               Event(type, pTarget, canBubble, cancelable)
{
	m_pRelatedNode = relatedNode;
	m_change       = MODIFICATION;
}


MutationEvent::MutationEvent(const XMLString& type, EventTarget* pTarget, bool canBubble, bool cancelable, Node* relatedNode, 
	                         const XMLString& prevValue, const XMLString& newValue, const XMLString& attrName, AttrChangeType change):
               Event(type, pTarget, canBubble, cancelable)
{
	m_pRelatedNode = relatedNode;
	m_prevValue    = prevValue;
	m_newValue     = newValue;
	m_attrName     = attrName;
	m_change       = change;
}


MutationEvent::~MutationEvent()
{
}


Node* MutationEvent::getRelatedNode() const
{
	return m_pRelatedNode;
}


const XMLString& MutationEvent::getPrevValue() const
{
	return m_prevValue;
}


const XMLString& MutationEvent::getNewValue() const
{
	return m_newValue;
}


const XMLString& MutationEvent::getAttrName() const
{
	return m_attrName;
}


MutationEvent::AttrChangeType MutationEvent::getAttrChange() const
{
	return m_change;
}


void MutationEvent::initMutationEvent(const XMLString& type, bool canBubble, bool cancelable, Node* relatedNode, 
	                                  const XMLString& prevValue, const XMLString& newValue, const XMLString& attrName, AttrChangeType change)
{
	initEvent(type, canBubble, cancelable);
	m_pRelatedNode = relatedNode;
	m_prevValue    = prevValue;
	m_newValue     = newValue;
	m_attrName     = attrName;
	m_change       = change;
}

	 
XML_END
