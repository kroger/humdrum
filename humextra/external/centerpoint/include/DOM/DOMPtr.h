//
// DOMPtr.h
//
// $Id: //depot/XML/Main/include/DOM/DOMPtr.h#3 $
//
// Smart Pointer for DOM objects.
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


#ifndef DOM_DOMPtr_H_INCLUDED
#define DOM_DOMPtr_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif


XML_BEGIN


template <class T> class DOMPtr
	//; This is a smart pointer class for DOM objects.
{
private:
	T* m_pObj;

public:
	DOMPtr()
	{
		m_pObj = NULL;
	}

	DOMPtr(T* pObj)
	{
		m_pObj = pObj;
		if (m_pObj) m_pObj->duplicate();
	}

	DOMPtr(const DOMPtr& ptr)
	{
		m_pObj = ptr.m_pObj;
		if (m_pObj) m_pObj->duplicate();
	}

	~DOMPtr()
	{
		if (m_pObj) m_pObj->release();
	}

	DOMPtr& operator=(const DOMPtr& ptr)
	{
		if (m_pObj) m_pObj->release();
		m_pObj = ptr.m_pObj;
		if (m_pObj) m_pObj->duplicate();
		return *this;
	}

	DOMPtr& operator=(T* pObj)
	{
		if (m_pObj) m_pObj->release();
		m_pObj = pObj;
		if (m_pObj) m_pObj->duplicate();
		return *this;
	}

	inline T* operator->()
	{
		return m_pObj;
	}

	inline const T* operator->() const
	{
		return m_pObj;
	}

	inline operator T*() const
	{
		return m_pObj;
	}
};


XML_END


#endif // DOM_DOMPtr_H_INCLUDED
