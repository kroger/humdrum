//
// AttributeListImp.cpp
//
// $Id: //depot/XML/Main/src/AttributeListImp.cpp#4 $
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


#include "SAX/AttributeListImp.h"


XML_BEGIN


CAttributeListImp::CAttributeListImp()
{
}


CAttributeListImp::CAttributeListImp(const CAttributeList& list)
{
	SetAttributeList(list);
}


CAttributeListImp::~CAttributeListImp()
{
}


int CAttributeListImp::GetLength() const
{
	return m_attributes.size();
}


XMLString CAttributeListImp::GetName(int i) const
{
	return m_attributes[i].m_name;
}


XMLString CAttributeListImp::GetType(int i) const
{
	return m_attributes[i].m_type;
}


XMLString CAttributeListImp::GetValue(int i) const
{
	return m_attributes[i].m_value;
}


bool CAttributeListImp::GetSpecified(int i) const
{
	return m_attributes[i].m_specified;
}


XMLString CAttributeListImp::GetType(const XMLString& name) const
{
	Attrib* pAttr = Find(name);
	if (pAttr)
		return pAttr->m_type;
	else
		return XMLString();
}

	
XMLString CAttributeListImp::GetValue(const XMLString& name) const
{
	Attrib* pAttr = Find(name);
	if (pAttr)
		return pAttr->m_value;
	else
		return XMLString();
}


bool CAttributeListImp::GetSpecified(const XMLString& name) const
{
	Attrib* pAttr = Find(name);
	if (pAttr)
		return pAttr->m_specified;
	else
		return false;
}


void CAttributeListImp::SetValue(int i, const XMLString& value)
{
	m_attributes[i].m_value = value;
}


void CAttributeListImp::SetValue(const XMLString& name, const XMLString& value)
{
	Attrib* pAttr = Find(name);

	if (pAttr) pAttr->m_value = value;
}


void CAttributeListImp::SetAttributeList(const CAttributeList& list)
{
	int count = list.GetLength();
	m_attributes.clear();
	for (int i = 0; i < count; i++)
	{
		AddAttribute(list.GetName(i), list.GetType(i), list.GetValue(i));
	}
}


void CAttributeListImp::AddAttribute(const XMLString& name, const XMLString& type, const XMLString& value)
{
	vector<Attrib>::iterator it = m_attributes.insert(m_attributes.end(), Attrib());
	it->m_name      = name;
	it->m_value     = value;
	it->m_type      = type;
	it->m_specified = true;
}


void CAttributeListImp::AddAttribute(const XMLString& name, const XMLString& type, const XMLString& value, bool specified)
{
	vector<Attrib>::iterator it = m_attributes.insert(m_attributes.end(), Attrib());
	it->m_name      = name;
	it->m_value     = value;
	it->m_type      = type;
	it->m_specified = specified;
}


void CAttributeListImp::AddAttribute(const XMLChar* name, const XMLChar* type, const XMLChar* value, bool specified)
{
	vector<Attrib>::iterator it = m_attributes.insert(m_attributes.end(), Attrib());
	it->m_name      = name;
	it->m_value     = value;
	it->m_type      = type;
	it->m_specified = specified;
}


void CAttributeListImp::RemoveAttribute(const XMLString& name)
{
	vector<Attrib>::iterator it = m_attributes.begin();

	while (it != m_attributes.end())
	{
		if (it->m_name == name)
		{
			m_attributes.erase(it);
			break;
		}
		++it;
	}
}


void CAttributeListImp::Clear()
{
	m_attributes.clear();
}


CAttributeListImp::Attrib* CAttributeListImp::Find(const XMLString& name) const
{
	vector<Attrib>::const_iterator it;
	for (it = m_attributes.begin(); it != m_attributes.end(); ++it)
	{
		if (it->m_name == name) return const_cast<Attrib*>(&(*it));
	}
	return NULL;
}


XML_END
