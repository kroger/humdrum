//
// Event.cpp
//
// $Id: //depot/XML/Main/src/Event.cpp#4 $
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


#include "DOM/Event.h"


XML_BEGIN


Event::Event(const XMLString& type)
{
	m_type            = type;
	m_pTarget         = NULL;
	m_pCurrentTarget  = NULL;
	m_currentPhase    = CAPTURING_PHASE;
	m_bubbles         = true;
	m_cancelable      = true;
	m_canceled        = false;
	m_stopPropagation = false;
	m_preventDefault  = false;
}


Event::Event(const XMLString& type, EventTarget* pTarget, bool canBubble, bool isCancelable)
{
	m_type            = type;
	m_pTarget         = pTarget;
	m_pCurrentTarget  = NULL;
	m_currentPhase    = CAPTURING_PHASE;
	m_bubbles         = canBubble;
	m_cancelable      = isCancelable;
	m_canceled        = false;
	m_stopPropagation = false;
	m_preventDefault  = false;
}


Event::~Event()
{
}


const XMLString& Event::getType() const
{
	return m_type;
}


EventTarget* Event::getTarget() const
{
	return m_pTarget;
}


EventTarget* Event::getCurrentTarget() const
{
	return m_pCurrentTarget;
}


Event::PhaseType Event::getEventPhase() const
{
	return m_currentPhase;
}


bool Event::getBubbles() const
{
	return m_bubbles;
}


bool Event::getCancelable() const
{
	return m_cancelable;
}


DOMTimeStamp Event::getTimeStamp() const
{
	return 0;
}


void Event::stopPropagation()
{
	m_stopPropagation = true;
}


void Event::preventDefault()
{
	m_canceled = true;
}


void Event::initEvent(const XMLString& eventType, bool canBubble, bool isCancelable)
{
	m_type            = eventType;
	m_bubbles         = canBubble;
	m_cancelable      = isCancelable;
	m_canceled        = false;
	m_stopPropagation = false;
}


bool Event::isCanceledNP() const
{
	return m_canceled;
}


bool Event::isStoppedNP() const
{
	return m_stopPropagation;
}


void Event::setTargetNP(EventTarget* pTarget)
{
	m_pTarget = pTarget;
}


void Event::setCurrentPhaseNP(PhaseType phase)
{
	m_currentPhase = phase;
}


void Event::setCurrentTargetNP(EventTarget* pTarget)
{
	m_pCurrentTarget = pTarget;
}


XML_END
