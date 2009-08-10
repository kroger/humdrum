//
// DOMObject.h
//
// $Id: //depot/XML/Main/include/DOM/DOMObject.h#11 $
//
// Document Object Model (Core) Level 1
//
// IMPLEMENTATION
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


#ifndef DOM_DOMObject_H_INCLUDED
#define DOM_DOMObject_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif


XML_BEGIN


class DOMAutoReleasePool;


class XML_API DOMObject
	//: A base class for all DOM classes.
	//+ Rules for memory management:
	//+ Every object created with new or via a factory
	//+ method (e.g. Document::create*) must be released
	//+ with a call to release() or autoRelease() when it
	//+ is no longer needed.
	//+
	//+ Every object created by cloning or importing another
	//+ object must be released.
	//+ Every object whose ownership has been taken with
	//+ a call to addRef() or duplicate() must be released.
	//+ An object obtained via any other way must not be
	//+ released, except ownership of it has been explicitely
	//+ taken with a call to addRef() or duplicate().
	//+ Violating these rules results in memory leaks or
	//. dangling pointers.
{
public:
	inline void addRef();
		//: Increases the object's reference count.
		//+ The initial reference count is 1.
		//. Same as duplicate().

	inline void duplicate();
		//: Increases the object's reference count.
		//+ The initial reference count is 1.
		//. Same as addRef().

	inline void release();
		//: Decreases the object's reference count.
		//+ If the reference count becoms equal to
		//. or less than zero, the object is deleted.

	inline int countReferences() const;
		//; Returns the number of references to this object.

	virtual void autoRelease(DOMAutoReleasePool* pAutoReleasePool = NULL);
		//: Adds the document to the given DOMAutoReleasePool
		//. which then takes care of eventually releasing the object.

	// IMPLEMENTATION
	DOMObject();
	virtual ~DOMObject();

private:
	int m_refCount;
};


#if defined(_MSC_VER)
#pragma warning(disable: 4273)
#endif


void DOMObject::addRef()
{
	m_refCount++;
}


void DOMObject::duplicate()
{
	m_refCount++;
}


void DOMObject::release()
{
	if (--m_refCount <= 0) delete this;
}


int DOMObject::countReferences() const
{
	return m_refCount;
}


#if defined(_MSC_VER)
#pragma warning(default:4273)
#endif


XML_END


#endif // DOM_DOMObject_H_INCLUDED
