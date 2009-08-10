//
// DOMAutoReleasePool.cpp
//
// $Id: //depot/XML/Main/src/DOMAutoReleasePool.cpp#3 $
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


#include "DOM/DOMAutoReleasePool.h"
#include "DOM/DOMObject.h"


XML_BEGIN


DOMAutoReleasePool::DOMAutoReleasePool()
{
}


DOMAutoReleasePool::~DOMAutoReleasePool()
{
	release();
}


void DOMAutoReleasePool::add(DOMObject* pObject)
{
	if (pObject) m_objectList.push_front(pObject);
}


void DOMAutoReleasePool::release()
{
	while (!m_objectList.empty())
	{
		m_objectList.front()->release();
		m_objectList.pop_front();
	}
}


XML_END
