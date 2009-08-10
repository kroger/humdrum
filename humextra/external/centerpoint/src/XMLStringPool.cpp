//
// XMLStringPool.cpp
//
// $Id: //depot/XML/Main/src/XMLStringPool.cpp#5 $
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


#include "XMLStringPool.h"
#ifndef CSL_XML_STANDALONE
#include "CSAutoLock.h"
#endif


XML_BEGIN


CXMLStringPool::CXMLStringPool(unsigned long size)
{
	csl_assert (size > 1);

	m_size  = size;
	m_pPool = new Item[size];
	for (unsigned long i = 0; i < size; ++i) m_pPool[i].m_used = false;
}


CXMLStringPool::~CXMLStringPool()
{
	delete [] m_pPool;
}


const XMLString& CXMLStringPool::Get(const XMLString& str)
{
#ifndef CSL_XML_STANDALONE
	CSL::Base::CAutoLock lock(m_mtx);
#endif

	register unsigned long i = 0;
	register unsigned long n = Hash(str) % m_size;

	while (m_pPool[n].m_used && m_pPool[n].m_data != str && i++ < m_size) n = (n + 1) % m_size;
	if (!m_pPool[n].m_used) 
	{
		m_pPool[n].m_data = str;
		m_pPool[n].m_used = true;
	}
	else if (i > m_size) return str;

	return m_pPool[n].m_data;
}


unsigned long CXMLStringPool::Hash(const XMLString& str)
{
	register unsigned long h = 0;
	register const XMLChar* s = str.c_str();

	while (*s) h = (h << 5) + h + (unsigned char) *s++;

	return h;
}


XML_END
