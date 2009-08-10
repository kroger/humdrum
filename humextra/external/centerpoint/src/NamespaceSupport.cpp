//
// NamespaceSupport.cpp
//
// $Id$
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


#include "SAX/NamespaceSupport.h"
#include "XMLUtils.h"


XML_BEGIN


const XMLString CNamespaceSupport::EMPTY_STRING;
const XMLString CNamespaceSupport::XMLNS        = CXMLUtils::AsciiToXML("http://www.w3.org/XML/1998/namespace");
const XMLString CNamespaceSupport::XMLNS_PREFIX = CXMLUtils::AsciiToXML("xml");


bool CNamespaceSupport::DeclarePrefix(const XMLString& prefix, const XMLString& uri)
{
	csl_assert (m_contexts.size() > 0);

	m_contexts.back().insert(Context::value_type(prefix, uri));
	return true;
}


vector<XMLString> CNamespaceSupport::GetDeclaredPrefixes() const
{
	vector<XMLString> result;
	for (vector<Context>::const_reverse_iterator vit = m_contexts.rbegin(); vit != m_contexts.rend(); ++vit)
	{
		for (Context::const_iterator cit = vit->begin(); cit != vit->end(); ++cit)
		{
			result.push_back(cit->first);
		}
	}
	return result;
}


const XMLString& CNamespaceSupport::GetPrefix(const XMLString& uri) const
{
	for (vector<Context>::const_reverse_iterator vit = m_contexts.rbegin(); vit != m_contexts.rend(); ++vit)
	{
		for (Context::const_iterator cit = vit->begin(); cit != vit->end(); ++cit)
		{
			if (cit->second == uri) return cit->first;
		}
	}
	return EMPTY_STRING;
}


vector<XMLString> CNamespaceSupport::GetPrefixes() const
{
	vector<XMLString> result;
	for (vector<Context>::const_reverse_iterator vit = m_contexts.rbegin(); vit != m_contexts.rend(); ++vit)
	{
		for (Context::const_iterator cit = vit->begin(); cit != vit->end(); ++cit)
		{
			if (!cit->first.empty()) result.push_back(cit->first);
		}
	}
	return result;
}


vector<XMLString> CNamespaceSupport::GetPrefixes(const XMLString& uri) const
{
	vector<XMLString> result;
	for (vector<Context>::const_reverse_iterator vit = m_contexts.rbegin(); vit != m_contexts.rend(); ++vit)
	{
		for (Context::const_iterator cit = vit->begin(); cit != vit->end(); ++cit)
		{
			if (cit->second == uri && !cit->first.empty()) result.push_back(cit->first);
		}
	}
	return result;
}


const XMLString& CNamespaceSupport::GetURI(const XMLString& prefix) const
{
	for (vector<Context>::const_reverse_iterator vit = m_contexts.rbegin(); vit != m_contexts.rend(); ++vit)
	{
		Context::const_iterator cit = vit->find(prefix);
		if (cit != vit->end()) return cit->second;
	}
	return EMPTY_STRING;
}


void CNamespaceSupport::PushContext()
{
	m_contexts.push_back(Context());
}


void CNamespaceSupport::PopContext()
{
	m_contexts.pop_back();
}


bool CNamespaceSupport::ProcessName(const XMLString& qname, XMLString& uri, XMLString& localName, bool isAttribute) const
{
	XMLString prefix;

	XMLString::size_type pos = qname.find_first_of(':');
	if (pos != XMLString::npos) 
	{
		prefix.assign(qname, 0, pos);
		localName.assign(qname, pos + 1, qname.size() - pos - 1);
		uri = GetURI(prefix);
		return !uri.empty();
	}
	else
	{
		localName = qname;
		if (!isAttribute) 
		{
			uri = GetURI(XMLString());
			return !uri.empty();
		}
	}
	return true;
}


void CNamespaceSupport::Reset()
{
	m_contexts.clear();
	PushContext();
	DeclarePrefix(XMLNS_PREFIX, XMLNS);
}


CNamespaceSupport::CNamespaceSupport()
{
	Reset();
}


CNamespaceSupport::~CNamespaceSupport()
{
}


XML_END
