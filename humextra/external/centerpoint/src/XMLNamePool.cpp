//
// XMLNamePool.cpp
//
// $Id: //depot/XML/Main/src/XMLNamePool.cpp#7 $
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


#include "XMLNamePool.h"
#include "XMLException.h"


XML_BEGIN


XMLNamePool::XMLNamePool(unsigned long size)
{
	csl_assert (size > 1);

	m_refCount = 1;
	m_size  = size;
	m_pPool = new Item[size];
	for (unsigned long i = 0; i < size; ++i) m_pPool[i].m_used = false;
}


XMLNamePool::~XMLNamePool()
{
	delete [] m_pPool;
}



void XMLNamePool::Duplicate()
{
	++m_refCount;
}


void XMLNamePool::Release()
{
	if (--m_refCount <= 0) delete this;
}


const XMLName* XMLNamePool::Insert(const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname)
{
	register unsigned long i = 0;
	register unsigned long n = Hash(namespaceURI, localName, qname) % m_size;

	while (m_pPool[n].m_used && (m_pPool[n].m_namespaceURI != namespaceURI || m_pPool[n].m_localName != localName || m_pPool[n].m_qname != qname) && i++ < m_size) n = (n + 1) % m_size;
	if (!m_pPool[n].m_used) 
	{
		m_pPool[n].m_namespaceURI = namespaceURI;
		m_pPool[n].m_localName    = localName;
		m_pPool[n].m_qname        = qname;
		m_pPool[n].m_used         = true;
	}
	else if (i > m_size) throw CXMLException(CXMLException::EXMLNamePoolFull);

	return &m_pPool[n];
}


unsigned long XMLNamePool::Hash(const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname)
{
	register unsigned long h = 0;
	register const XMLChar* s = namespaceURI.c_str();
	while (*s) h = (h << 5) + h + (unsigned long) *s++;
	s = localName.c_str();
	while (*s) h = (h << 5) + h + (unsigned long) *s++;
	s = qname.c_str();
	while (*s) h = (h << 5) + h + (unsigned long) *s++;
	return h;
}


XML_END
