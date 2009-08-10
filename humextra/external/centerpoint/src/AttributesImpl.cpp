//
// AttributesImpl.cpp
//
// $Id: //depot/XML/Main/src/AttributesImpl.cpp#3 $
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


#include "SAX/AttributesImpl.h"


XML_BEGIN


CAttributesImpl::CAttributesImpl()
{
}


CAttributesImpl::CAttributesImpl(const CAttributes& list)
{
	SetAttributes(list);
}


CAttributesImpl::~CAttributesImpl()
{
}


int CAttributesImpl::GetIndex(const XMLString& qname) const
{
	int i = 0;
	vector<Attrib>::const_iterator it;
	for (it = m_attributes.begin(); it != m_attributes.end(); ++it)
	{
		if (it->m_qname == qname) return i;
		++i;
	}
	return -1;
}


int CAttributesImpl::GetIndex(const XMLString& uri, const XMLString& localName) const
{
	int i = 0;
	vector<Attrib>::const_iterator it;
	for (it = m_attributes.begin(); it != m_attributes.end(); ++it)
	{
		if (it->m_uri == uri && it->m_localName == localName) return i;
		++i;
	}
	return -1;
}


int CAttributesImpl::GetLength() const
{
	return m_attributes.size();
}


XMLString CAttributesImpl::GetLocalName(int i) const
{
	return m_attributes[i].m_localName;
}


XMLString CAttributesImpl::GetQName(int i) const
{
	return m_attributes[i].m_qname;
}


XMLString CAttributesImpl::GetType(int i) const
{
	return m_attributes[i].m_type;
}


XMLString CAttributesImpl::GetType(const XMLString& qname) const
{
	Attrib* pAttr = Find(qname);
	if (pAttr)
		return pAttr->m_type;
	else
		return XMLString();
}


XMLString CAttributesImpl::GetType(const XMLString& uri, const XMLString& localName) const
{
	Attrib* pAttr = Find(uri, localName);
	if (pAttr)
		return pAttr->m_type;
	else
		return XMLString();
}


XMLString CAttributesImpl::GetValue(int i) const
{
	return m_attributes[i].m_value;
}


XMLString CAttributesImpl::GetValue(const XMLString& qname) const
{
	Attrib* pAttr = Find(qname);
	if (pAttr)
		return pAttr->m_value;
	else
		return XMLString();
}


XMLString CAttributesImpl::GetValue(const XMLString& uri, const XMLString& localName) const
{
	Attrib* pAttr = Find(uri, localName);
	if (pAttr)
		return pAttr->m_value;
	else
		return XMLString();
}


XMLString CAttributesImpl::GetURI(int i) const
{
	return m_attributes[i].m_uri;
}


bool CAttributesImpl::GetSpecified(int i) const
{
	return m_attributes[i].m_specified;
}


bool CAttributesImpl::GetSpecified(const XMLString& qname) const
{
	Attrib* pAttr = Find(qname);
	if (pAttr)
		return pAttr->m_specified;
	else
		return false;
}


bool CAttributesImpl::GetSpecified(const XMLString& uri, const XMLString& localName) const
{
	Attrib* pAttr = Find(uri, localName);
	if (pAttr)
		return pAttr->m_specified;
	else
		return false;
}


void CAttributesImpl::SetValue(int i, const XMLString& value)
{
	m_attributes[i].m_value = value;
}


void CAttributesImpl::SetValue(const XMLString& qname, const XMLString& value)
{
	Attrib* pAttr = Find(qname);

	if (pAttr) pAttr->m_value = value;
}


void CAttributesImpl::SetValue(const XMLString& uri, const XMLString& localName, const XMLString& value)
{
	Attrib* pAttr = Find(uri, localName);

	if (pAttr) pAttr->m_value = value;
}


void CAttributesImpl::SetAttributes(const CAttributes& list)
{
	int count = list.GetLength();
	m_attributes.clear();
	for (int i = 0; i < count; i++)
	{
		AddAttribute(list.GetURI(i), list.GetLocalName(i), list.GetQName(i), list.GetType(i), list.GetValue(i));
	}
}


void CAttributesImpl::SetAttribute(int i, const XMLString& uri, const XMLString& localName, const XMLString& qname, const XMLString& type, const XMLString& value)
{
	m_attributes[i].m_uri       = uri;
	m_attributes[i].m_localName = localName;
	m_attributes[i].m_qname     = qname;
	m_attributes[i].m_type      = type;
	m_attributes[i].m_value     = value;
	m_attributes[i].m_specified = true;
}


void CAttributesImpl::AddAttribute(const XMLString& uri, const XMLString& localName, const XMLString& qname, const XMLString& type, const XMLString& value)
{
	vector<Attrib>::iterator it = m_attributes.insert(m_attributes.end(), Attrib());
	it->m_uri       = uri;
	it->m_localName = localName;
	it->m_qname     = qname;
	it->m_value     = value;
	it->m_type      = type;
	it->m_specified = true;
}


void CAttributesImpl::AddAttribute(const XMLString& uri, const XMLString& localName, const XMLString& qname, const XMLString& type, const XMLString& value, bool specified)
{
	vector<Attrib>::iterator it = m_attributes.insert(m_attributes.end(), Attrib());
	it->m_uri       = uri;
	it->m_localName = localName;
	it->m_qname     = qname;
	it->m_value     = value;
	it->m_type      = type;
	it->m_specified = specified;
}


void CAttributesImpl::AddAttribute(const XMLChar* uri, const XMLChar* localName, const XMLChar* qname, const XMLChar* type, const XMLChar* value, bool specified)
{
	vector<Attrib>::iterator it = m_attributes.insert(m_attributes.end(), Attrib());
	it->m_uri       = uri;
	it->m_localName = localName;
	it->m_qname     = qname;
	it->m_value     = value;
	it->m_type      = type;
	it->m_specified = specified;
}


void CAttributesImpl::RemoveAttribute(int i)
{
	vector<Attrib>::iterator it = m_attributes.begin();

	int cur = 0;
	while (it != m_attributes.end())
	{
		if (cur == i)
		{
			m_attributes.erase(it);
			break;
		}
		++it;
		++cur;
	}
}


void CAttributesImpl::RemoveAttribute(const XMLString& qname)
{
	vector<Attrib>::iterator it = m_attributes.begin();

	while (it != m_attributes.end())
	{
		if (it->m_qname == qname)
		{
			m_attributes.erase(it);
			break;
		}
		++it;
	}
}


void CAttributesImpl::RemoveAttribute(const XMLString& uri, const XMLString& localName)
{
	vector<Attrib>::iterator it = m_attributes.begin();

	while (it != m_attributes.end())
	{
		if (it->m_uri == uri && it->m_localName == localName)
		{
			m_attributes.erase(it);
			break;
		}
		++it;
	}
}


void CAttributesImpl::Clear()
{
	m_attributes.clear();
}


void CAttributesImpl::SetLocalName(int i, const XMLString& localName)
{
	m_attributes[i].m_localName = localName;
}


void CAttributesImpl::SetQName(int i, const XMLString& qname)
{
	m_attributes[i].m_qname = qname;
}


void CAttributesImpl::SetType(int i, const XMLString& type)
{
	m_attributes[i].m_type = type;
}


void CAttributesImpl::SetURI(int i, const XMLString& uri)
{
	m_attributes[i].m_uri = uri;
}


CAttributesImpl::Attrib* CAttributesImpl::Find(const XMLString& qname) const
{
	vector<Attrib>::const_iterator it;
	for (it = m_attributes.begin(); it != m_attributes.end(); ++it)
	{
		if (it->m_qname == qname) return const_cast<Attrib*>(&(*it));
	}
	return NULL;
}


CAttributesImpl::Attrib* CAttributesImpl::Find(const XMLString& uri, const XMLString& localName) const
{
	vector<Attrib>::const_iterator it;
	for (it = m_attributes.begin(); it != m_attributes.end(); ++it)
	{
		if (it->m_uri == uri && it->m_localName == localName) return const_cast<Attrib*>(&(*it));
	}
	return NULL;
}


XML_END
