//
// DOMAutoReleasePool.h
//
// $Id: //depot/XML/Main/include/DOM/DOMAutoReleasePool.h#3 $
//
// Definition of class DOMAutoReleasePool.
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


#ifndef DOM_DOMAutoReleasePool_H_INCLUDED
#define DOM_DOMAutoReleasePool_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef STD_DEQUE_INCLUDED
#include <deque>
#define STD_DEQUE_INCLUDED
#endif


XML_BEGIN


class DOMObject;


class XML_API DOMAutoReleasePool
	//: This class provides some simple garbage collection for DOMObject's.
	//+ It temporarily takes ownwership of objects that nobody else wants.
	//+ Sending a release() message to the DOMAutoReleasePool releases
	//. all objects.
{
public:
	DOMAutoReleasePool();
	virtual ~DOMAutoReleasePool();

	void add(DOMObject* pObject);
		//; adds the given object to the auto release pool.

	void release();
		//; sends a release() message to all objects in the pool.

private:
	deque<DOMObject*> m_objectList;	
};


XML_END


#endif // DOM_DOMAutoReleasePool_H_INCLUDED
