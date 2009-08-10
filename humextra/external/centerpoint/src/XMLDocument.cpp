//
// XMLDocument.cpp
//
// $Id: //depot/XML/Main/src/XMLDocument.cpp#8 $
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


#include "XMLDocument.h"
#include "XMLUtils.h"


XML_BEGIN


//
// CXMLObject
//


CXMLStringPool CXMLObject::m_stringPool(997);


CXMLObject::CXMLObject()
{
	m_pParent  = NULL;
	m_pNext    = NULL;
	m_userData = NULL;
}


CXMLObject::~CXMLObject()
{
}


XMLString CXMLObject::GetBase() const
{
	if (m_pParent)
		return m_pParent->GetBase();
	else
		return XMLString();
}


XMLObjectType CXMLObject::GetType() const
{
	return xmlObject;
}


CXMLObject* CXMLObject::GetParent() const
{
	return m_pParent;
}


void CXMLObject::Dump(ostream* pStr) const
{
	*pStr << "<Object/>" << endl;
}


CXMLObject* CXMLObject::Zoom() const
{
	return NULL;
}


CXMLObject* CXMLObject::GetNext() const
{
	return m_pNext;
}


void CXMLObject::SetNext(CXMLObject* pNext)
{
	m_pNext = pNext;
}


void CXMLObject::SetUserData(void* data)
{
	m_userData = data;
}


void* CXMLObject::GetUserData() const
{
	return m_userData;
}


void CXMLObject::DeleteAll()
{
	if (m_pNext) m_pNext->DeleteAll();
	delete this;
}


CXMLObject* CXMLObject::Copy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLObject();
	pCopy->SetUserData(m_userData);
	return pCopy;
}


CXMLObject* CXMLObject::DeepCopy(CXMLObject* pCopy) const
{
	return Copy(pCopy);
}


//
// CXMLIterator
//


CXMLIterator::CXMLIterator(CXMLObject* pObj)
{
	m_pCurrent = pObj;
	m_pParent  = pObj ? pObj->GetParent() : NULL;
}


CXMLIterator::CXMLIterator(const CXMLIterator& iter)
{
	m_pCurrent = iter.m_pCurrent;
	m_pParent  = iter.m_pParent;
}


CXMLIterator::~CXMLIterator()
{
}


void CXMLIterator::Next()
{
	if (m_pCurrent)
		m_pCurrent = m_pCurrent->GetNext();
}


void CXMLIterator::ZoomIn()
{
	if (m_pCurrent)
	{
		m_pParent  = m_pCurrent;
		m_pCurrent = m_pCurrent->Zoom();
	}
}


void CXMLIterator::ZoomOut()
{
	if (m_pCurrent)
		m_pCurrent = m_pCurrent->GetParent();
	else
		m_pCurrent = m_pParent;
	if (m_pCurrent) m_pParent = m_pCurrent->GetParent();
}


CXMLObject* CXMLIterator::GetObj() const
{
	return m_pCurrent;
}


//
// CXMLFilteringIterator
//


CXMLFilteringIterator::CXMLFilteringIterator(int filterSpec, CXMLObject* pObj):
                       CXMLIterator(pObj)
{
	m_filterSpec = filterSpec;
}


CXMLFilteringIterator::CXMLFilteringIterator(int filterSpec, const CXMLIterator& iter):
                       CXMLIterator(iter)
{
	m_filterSpec = filterSpec;
}


CXMLFilteringIterator::CXMLFilteringIterator(const CXMLFilteringIterator& iter):
                       CXMLIterator(iter)
{
	m_filterSpec = iter.m_filterSpec;
}


void CXMLFilteringIterator::Next()
{
	CXMLIterator::Next();
	while (GetObj() && (GetObj()->GetType() & m_filterSpec) == 0)
	{
		CXMLIterator::Next();
	}
}


void CXMLFilteringIterator::ZoomIn()
{
	CXMLIterator::ZoomIn();
	while (GetObj() && (GetObj()->GetType() & m_filterSpec) == 0)
	{
		CXMLIterator::Next();
	}
}


void CXMLFilteringIterator::ZoomOut()
{
	CXMLIterator::ZoomOut();
}


//
// CXMLContainer
//

CXMLContainer::CXMLContainer()
{
	m_pFirstChild = NULL;
}


void CXMLContainer::InsertChild(CXMLObject* pObj)
{
	// An object can only be connected to one parent.
	if (pObj->m_pParent) static_cast<CXMLContainer*>(pObj->m_pParent)->RemoveChild(pObj);

	if (m_pFirstChild)
	{
		CXMLObject* pCur = m_pFirstChild;
		while (pCur->m_pNext) pCur = pCur->m_pNext;
		pCur->m_pNext = pObj;
	}
	else
	{
		m_pFirstChild = pObj;
	}
	pObj->m_pNext = NULL;
	pObj->m_pParent = this;
}


void CXMLContainer::InsertChildBefore(CXMLObject* pObj, CXMLObject* pSuccessor)
{
	if (pSuccessor == NULL)
	{
		InsertChild(pObj);
		return;
	}

	// An object can only be connected to one parent.
	if (pObj->m_pParent) static_cast<CXMLContainer*>(pObj->m_pParent)->RemoveChild(pObj);

	if (m_pFirstChild && pSuccessor != m_pFirstChild)
	{
		CXMLObject* pCur = m_pFirstChild;
		while (pCur->m_pNext && pCur->m_pNext != pSuccessor) pCur = pCur->m_pNext;
		pObj->m_pNext = pSuccessor;
		pCur->m_pNext = pObj;
	}
	else
	{
		m_pFirstChild = pObj;
		pObj->m_pNext = pSuccessor;
	}
	pObj->m_pParent = this;
}


void CXMLContainer::RemoveChild(CXMLObject* pObj)
{
	if (pObj == m_pFirstChild)
	{
		m_pFirstChild = m_pFirstChild->m_pNext;
		pObj->m_pNext = NULL;
		pObj->m_pParent = NULL;
	}
	else
	{
		CXMLObject* pCur = m_pFirstChild;
		while (pCur && pCur->m_pNext != pObj) pCur = pCur->m_pNext;
		if (pCur) 
		{
			pCur->m_pNext = pCur->m_pNext->m_pNext;
			pObj->m_pNext = NULL;
			pObj->m_pParent = NULL;
		}
	}
}


void CXMLContainer::Empty()
{
	if (m_pFirstChild) m_pFirstChild->DeleteAll();
}


XMLObjectType CXMLContainer::GetType() const
{
	return xmlContainer;
}


void CXMLContainer::Dump(ostream* pStr) const
{
	*pStr << "<Container>" << endl;
	CXMLObject* pObj = m_pFirstChild;
	while (pObj)
	{
		pObj->Dump(pStr);
		pObj = pObj->m_pNext;
	}
	*pStr << "</Container>" << endl;
}


CXMLObject* CXMLContainer::Zoom() const
{
	return m_pFirstChild;
}


void CXMLContainer::DeleteAll()
{
	if (m_pFirstChild) m_pFirstChild->DeleteAll();
	CXMLObject::DeleteAll();
}


CXMLObject* CXMLContainer::Copy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLContainer();
	return CXMLObject::Copy(pCopy);
}


CXMLObject* CXMLContainer::DeepCopy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLContainer();
	CXMLObject::DeepCopy(pCopy);
	CXMLObject* pChild = m_pFirstChild;
	while (pChild)
	{
		static_cast<CXMLContainer*>(pCopy)->InsertChild(pChild->DeepCopy());
		pChild = pChild->GetNext();
	}
	return pCopy;
}


//
// CXMLAttribute
//


CXMLAttribute::CXMLAttribute()
{
}


CXMLAttribute::CXMLAttribute(const XMLString& name, const XMLString& type, const XMLString& value, bool specified)
{
	m_name      = m_stringPool.Get(name);
	m_type      = m_stringPool.Get(type);
	m_value     = value;
	m_specified = specified;
}


CXMLAttribute::CXMLAttribute(const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname, const XMLString& type, const XMLString& value, bool specified)
{
	m_name      = m_stringPool.Get(qname);
	m_localName = m_stringPool.Get(localName);
	m_uri       = m_stringPool.Get(namespaceURI);
	m_type      = m_stringPool.Get(type);
	m_value     = value;
	m_specified = specified;
}


void CXMLAttribute::SetName(const XMLString& name)
{
	m_name = m_stringPool.Get(name);
}


void CXMLAttribute::SetLocalName(const XMLString& localName)
{
	m_localName = m_stringPool.Get(localName);
}


void CXMLAttribute::SetNamespaceURI(const XMLString& uri)
{
	m_uri = m_stringPool.Get(uri);
}


XMLString CXMLAttribute::GetName() const
{
	return m_name;
}


XMLString CXMLAttribute::GetLocalName() const
{
	return m_localName;
}


XMLString CXMLAttribute::GetNamespaceURI() const
{
	return m_uri;
}


void CXMLAttribute::SetAttrType(const XMLString& type)
{
	m_type = m_stringPool.Get(type);
}


XMLString CXMLAttribute::GetAttrType() const
{
	return m_type;
}


void CXMLAttribute::SetValue(const XMLString& value)
{
	m_value     = value;
	m_specified = true;
}


XMLString CXMLAttribute::GetValue() const
{
	return m_value;
}


void CXMLAttribute::SetSpecified(bool flag)
{
	m_specified = flag;
}


bool CXMLAttribute::GetSpecified() const
{
	return m_specified;
}


XMLObjectType CXMLAttribute::GetType() const
{
	return xmlAttribute;
}


void CXMLAttribute::Dump(ostream* pStr) const
{
	*pStr
		<< "<Attribute Name=\"" << CXMLUtils::XMLToAscii(m_name) << "\" "
		<< "Type=\"" << CXMLUtils::XMLToAscii(m_type) << "\" "
		<< "Value=\"" << CXMLUtils::XMLToAscii(m_value) << "\" "
		<< "Specified=\"" << (m_specified ? "TRUE" : "FALSE") << "\">" << endl;
}


CXMLObject* CXMLAttribute::Copy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLAttribute();
	static_cast<CXMLAttribute*>(pCopy)->SetName(m_name);
	static_cast<CXMLAttribute*>(pCopy)->SetLocalName(m_localName);
	static_cast<CXMLAttribute*>(pCopy)->SetNamespaceURI(m_uri);
	static_cast<CXMLAttribute*>(pCopy)->SetAttrType(m_type);
	static_cast<CXMLAttribute*>(pCopy)->SetValue(m_value);
	static_cast<CXMLAttribute*>(pCopy)->SetSpecified(m_specified);
	return CXMLObject::Copy(pCopy);
}


CXMLObject* CXMLAttribute::DeepCopy(CXMLObject* pCopy) const
{
	return Copy(pCopy);
}


//
// CXMLAttributeList
//


CXMLAttributeList::CXMLAttributeList()
{
	m_pFirstAttr = NULL;
}


CXMLAttributeList::~CXMLAttributeList()
{
	Clear();
}


int CXMLAttributeList::GetLength() const
{
	int count = 0;
	CXMLAttribute* pCur = m_pFirstAttr;
	while (pCur)
	{
		pCur = static_cast<CXMLAttribute*>(pCur->GetNext());
		count++;
	}
	return count;
}


XMLString CXMLAttributeList::GetName(int i) const
{
	CXMLAttribute* pAttr = GetAttribute(i);
	if (pAttr) 
		return pAttr->GetName();
	else
		return XMLString();
}


XMLString CXMLAttributeList::GetLocalName(int i) const
{
	CXMLAttribute* pAttr = GetAttribute(i);
	if (pAttr) 
		return pAttr->GetLocalName();
	else
		return XMLString();
}


XMLString CXMLAttributeList::GetNamespaceURI(int i) const
{
	CXMLAttribute* pAttr = GetAttribute(i);
	if (pAttr) 
		return pAttr->GetNamespaceURI();
	else
		return XMLString();
}


XMLString CXMLAttributeList::GetType(int i) const
{
	CXMLAttribute* pAttr = GetAttribute(i);
	if (pAttr) 
		return pAttr->GetAttrType();
	else
		return XMLString();
}


XMLString CXMLAttributeList::GetValue(int i) const
{
	CXMLAttribute* pAttr = GetAttribute(i);
	if (pAttr) 
		return pAttr->GetValue();
	else
		return XMLString();
}


bool CXMLAttributeList::GetSpecified(int i) const
{
	CXMLAttribute* pAttr = GetAttribute(i);
	if (pAttr) 
		return pAttr->GetSpecified();
	else
		return false;
}


XMLString CXMLAttributeList::GetType(const XMLString& name) const
{
	CXMLAttribute* pAttr = GetAttribute(name);
	if (pAttr) 
		return pAttr->GetAttrType();
	else
		return XMLString();
}


XMLString CXMLAttributeList::GetType(const XMLString& namespaceURI, const XMLString& localName) const
{
	CXMLAttribute* pAttr = GetAttribute(namespaceURI, localName);
	if (pAttr) 
		return pAttr->GetAttrType();
	else
		return XMLString();
}


XMLString CXMLAttributeList::GetValue(const XMLString& name) const
{
	CXMLAttribute* pAttr = GetAttribute(name);
	if (pAttr) 
		return pAttr->GetValue();
	else
		return XMLString();
}


XMLString CXMLAttributeList::GetValue(const XMLString& namespaceURI, const XMLString& localName) const
{
	CXMLAttribute* pAttr = GetAttribute(namespaceURI, localName);
	if (pAttr) 
		return pAttr->GetValue();
	else
		return XMLString();
}


bool CXMLAttributeList::GetSpecified(const XMLString& name) const
{
	CXMLAttribute* pAttr = GetAttribute(name);
	if (pAttr) 
		return pAttr->GetSpecified();
	else
		return false;
}


bool CXMLAttributeList::GetSpecified(const XMLString& namespaceURI, const XMLString& localName) const
{
	CXMLAttribute* pAttr = GetAttribute(namespaceURI, localName);
	if (pAttr) 
		return pAttr->GetSpecified();
	else
		return false;
}


CXMLAttribute* CXMLAttributeList::GetAttribute(const XMLString& name) const
{
	CXMLAttribute* pCur = m_pFirstAttr;
	while (pCur)
	{
		if (pCur->GetName() == name) return pCur;
		pCur = static_cast<CXMLAttribute*>(pCur->GetNext());
	}
	return NULL;
}


CXMLAttribute* CXMLAttributeList::GetAttribute(const XMLString& namespaceURI, const XMLString& localName) const
{
	CXMLAttribute* pCur = m_pFirstAttr;
	while (pCur)
	{
		if (pCur->GetNamespaceURI() == namespaceURI && pCur->GetLocalName() == localName) return pCur;
		pCur = static_cast<CXMLAttribute*>(pCur->GetNext());
	}
	return NULL;
}


CXMLAttribute* CXMLAttributeList::GetAttribute(int i) const
{
	CXMLAttribute* pCur = m_pFirstAttr;
	int n = 0;
	while (pCur)
	{
		if (n == i) return pCur;
		pCur = static_cast<CXMLAttribute*>(pCur->GetNext());
		n++;
	}
	return NULL;
}


void CXMLAttributeList::SetAttribute(CXMLAttribute* pAttr)
{
	CXMLAttribute* pOld;
	if (pAttr->GetNamespaceURI().empty())
		pOld = GetAttribute(pAttr->GetName());
	else
		pOld = GetAttribute(pAttr->GetNamespaceURI(), pAttr->GetLocalName());
	if (pOld) RemoveAttribute(pOld);
	AddAttribute(pAttr);
	delete pOld;
}


CXMLAttribute* CXMLAttributeList::RemoveAttribute(const XMLString& name)
{
	CXMLAttribute* pAttr = GetAttribute(name);
	if (pAttr) RemoveAttribute(pAttr);
	return pAttr;
}


CXMLAttribute* CXMLAttributeList::RemoveAttribute(const XMLString& namespaceURI, const XMLString& localName)
{
	CXMLAttribute* pAttr = GetAttribute(namespaceURI, localName);
	if (pAttr) RemoveAttribute(pAttr);
	return pAttr;
}


CXMLAttribute* CXMLAttributeList::RemoveAttribute(CXMLAttribute* pAttr)
{
	CXMLAttribute* pCur = m_pFirstAttr;
	CXMLAttribute* pDel = NULL;
	if (pCur == pAttr)
	{
		pDel = pCur;
		m_pFirstAttr = static_cast<CXMLAttribute*>(pDel->GetNext());
	}
	else
	{
		while (pCur)
		{
			if (pCur->GetNext() == pAttr)
			{
				pDel = static_cast<CXMLAttribute*>(pCur->GetNext());
				pCur->SetNext(static_cast<CXMLAttribute*>(pDel->GetNext()));
				break;
			}
			pCur = static_cast<CXMLAttribute*>(pCur->GetNext());
		}
	}
	pDel->SetNext(NULL);
	return pDel;
}


void CXMLAttributeList::Clear()
{
	CXMLAttribute* pCur = m_pFirstAttr;
	CXMLAttribute* pDel = NULL;
	while (pCur)
	{
		pDel = pCur;
		pCur = static_cast<CXMLAttribute*>(pCur->GetNext());
		delete pDel;
	}
}


void CXMLAttributeList::AddAttribute(CXMLAttribute* pAttr)
{
	CXMLAttribute* pCur = m_pFirstAttr;
	if (pCur)
	{
		while (pCur->GetNext())
		{
			pCur = static_cast<CXMLAttribute*>(pCur->GetNext());
		}
		pCur->SetNext(pAttr);
	}
	else
	{
		m_pFirstAttr = pAttr;
	}
}


//
// CXMLElement
//


CXMLElement::CXMLElement()
{
}


CXMLElement::CXMLElement(const XMLString& name)
{
	m_name = m_stringPool.Get(name);
}


CXMLElement::CXMLElement(const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname)
{
	m_name      = m_stringPool.Get(qname);
	m_localName = m_stringPool.Get(localName);
	m_uri       = m_stringPool.Get(namespaceURI);
}


CXMLElement::~CXMLElement()
{
}


XMLString CXMLElement::GetName() const
{
	return m_name;
}


XMLString CXMLElement::GetLocalName() const
{
	return m_localName;
}


XMLString CXMLElement::GetNamespaceURI() const
{
	return m_uri;
}


void CXMLElement::SetName(const XMLString& name)
{
	m_name = m_stringPool.Get(name);
}


void CXMLElement::SetLocalName(const XMLString& localName)
{
	m_localName = m_stringPool.Get(localName);
}


void CXMLElement::SetNamespaceURI(const XMLString& uri)
{
	m_uri = m_stringPool.Get(uri);
}


CXMLAttributeList& CXMLElement::GetAttributes()
{
	return m_attributes;
}


const CXMLAttributeList& CXMLElement::GetAttributes() const
{
	return m_attributes;
}


XMLObjectType CXMLElement::GetType() const
{
	return xmlElement;
}


void CXMLElement::Dump(ostream* pStr) const
{
	*pStr << "<Element Name=\"" << CXMLUtils::XMLToAscii(m_name) << "\">" << endl;
	
	int count = m_attributes.GetLength();
	for (int i = 0; i < count; i++)
	{
		m_attributes.GetAttribute(i)->Dump(pStr);
	}

	CXMLContainer::Dump(pStr);
	*pStr << "</Element>" << endl;
}


CXMLObject* CXMLElement::Copy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLElement();
	static_cast<CXMLElement*>(pCopy)->SetName(m_name);
	static_cast<CXMLElement*>(pCopy)->SetLocalName(m_localName);
	static_cast<CXMLElement*>(pCopy)->SetNamespaceURI(m_uri);
	for (int i = 0; i < m_attributes.GetLength(); i++)
	{
		static_cast<CXMLElement*>(pCopy)->GetAttributes().SetAttribute(new CXMLAttribute(m_attributes.GetNamespaceURI(i), m_attributes.GetLocalName(i), m_attributes.GetName(i), m_attributes.GetType(i), m_attributes.GetValue(i), m_attributes.GetSpecified(i)));
	}
	return CXMLContainer::Copy(pCopy);
}


CXMLObject* CXMLElement::DeepCopy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLElement();
	static_cast<CXMLElement*>(pCopy)->SetName(m_name);
	static_cast<CXMLElement*>(pCopy)->SetLocalName(m_localName);
	static_cast<CXMLElement*>(pCopy)->SetNamespaceURI(m_uri);
	for (int i = 0; i < m_attributes.GetLength(); i++)
	{
		static_cast<CXMLElement*>(pCopy)->GetAttributes().SetAttribute(new CXMLAttribute(m_attributes.GetNamespaceURI(i), m_attributes.GetLocalName(i), m_attributes.GetName(i), m_attributes.GetType(i), m_attributes.GetValue(i), m_attributes.GetSpecified(i)));
	}
	return CXMLContainer::DeepCopy(pCopy);
}


//
// CXMLCharacterData
//


CXMLCharacterData::CXMLCharacterData()
{
}


CXMLCharacterData::CXMLCharacterData(const XMLString& data)
{
	m_data = data;
}


XMLString CXMLCharacterData::GetData() const
{
	return m_data;
}


void CXMLCharacterData::SetData(const XMLString& data)
{
	m_data = data;
}


XMLObjectType CXMLCharacterData::GetType() const
{
	return xmlCharacterData;
}


void CXMLCharacterData::Dump(ostream* pStr) const
{
	*pStr << "<CharacterData>" << endl;
	*pStr << CXMLUtils::XMLToAscii(m_data) << endl;
	*pStr << "</CharacterData>" << endl;
}


XMLString CXMLCharacterData::GetSubString(int offset, int count) const
{
	return m_data.substr(offset, count);
}


void CXMLCharacterData::AppendData(const XMLString& data)
{
	m_data += data;
}


void CXMLCharacterData::InsertData(int offset, const XMLString& data)
{
	m_data.insert(offset, data);
}


void CXMLCharacterData::DeleteData(int offset, int count)
{
	m_data.replace(offset, count, XMLString());
}


void CXMLCharacterData::ReplaceData(int offset, int count, const XMLString& data)
{
	m_data.replace(offset, count, data);
}


int CXMLCharacterData::GetLength() const
{
	return m_data.length();
}


CXMLObject* CXMLCharacterData::Copy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLCharacterData();
	static_cast<CXMLCharacterData*>(pCopy)->SetData(m_data);
	return CXMLObject::Copy(pCopy);
}


CXMLObject* CXMLCharacterData::DeepCopy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLCharacterData();
	static_cast<CXMLCharacterData*>(pCopy)->SetData(m_data);
	return CXMLObject::DeepCopy(pCopy);
}


//
// CXMLCDATASection
//


CXMLCDATASection::CXMLCDATASection()
{
}


CXMLCDATASection::CXMLCDATASection(const XMLString& data): CXMLCharacterData(data)
{
}


XMLObjectType CXMLCDATASection::GetType() const
{
	return xmlCDATASection;
}


void CXMLCDATASection::Dump(ostream* pStr) const
{
	*pStr << "<CDATA>" << endl;
	*pStr << CXMLUtils::XMLToAscii(GetData()) << endl;
	*pStr << "</CDATA>" << endl;
}


CXMLObject* CXMLCDATASection::Copy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLCDATASection();
	return CXMLCharacterData::Copy(pCopy);
}


CXMLObject* CXMLCDATASection::DeepCopy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLCDATASection();
	return CXMLCharacterData::DeepCopy(pCopy);
}


//
// CXMLComment
//


CXMLComment::CXMLComment()
{
}


CXMLComment::CXMLComment(const XMLString& data): CXMLCharacterData(data)
{
}


XMLObjectType CXMLComment::GetType() const
{
	return xmlComment;
}


void CXMLComment::Dump(ostream* pStr) const
{
	*pStr << "<Comment>" << endl;
	*pStr << CXMLUtils::XMLToAscii(GetData()) << endl;
	*pStr << "</Comment>" << endl;
}


CXMLObject* CXMLComment::Copy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLComment();
	return CXMLCharacterData::Copy(pCopy);
}


CXMLObject* CXMLComment::DeepCopy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLComment();
	return CXMLCharacterData::DeepCopy(pCopy);
}


//
// CXMLGenericData
//


CXMLGenericData::CXMLGenericData()
{
}


CXMLGenericData::CXMLGenericData(const XMLString& data):
                 CXMLCharacterData(data)
{
}


XMLObjectType CXMLGenericData::GetType() const
{
	return xmlGenericData;
}


void CXMLGenericData::Dump(ostream* pStr) const
{
	*pStr << "<GenericData>" << endl;
	*pStr << CXMLUtils::XMLToAscii(GetData()) << endl;
	*pStr << "</GenericData>" << endl;
}


CXMLObject* CXMLGenericData::Copy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLGenericData();
	return CXMLCharacterData::Copy(pCopy);
}


CXMLObject* CXMLGenericData::DeepCopy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLGenericData();
	return CXMLCharacterData::DeepCopy(pCopy);
}


//
// CXMLProcessingInstruction
//


CXMLProcessingInstruction::CXMLProcessingInstruction()
{
}


CXMLProcessingInstruction::CXMLProcessingInstruction(const XMLString& target, const XMLString& data)
{
	m_target = target;
	m_data = data;
}


XMLString CXMLProcessingInstruction::GetTarget() const
{
	return m_target;
}


XMLString CXMLProcessingInstruction::GetData() const
{
	return m_data;
}


void CXMLProcessingInstruction::SetTarget(const XMLString& target)
{
	m_target = target;
}


void CXMLProcessingInstruction::SetData(const XMLString& data)
{
	m_data = data;
}


XMLObjectType CXMLProcessingInstruction::GetType() const
{
	return xmlProcessingInstruction;
}


void CXMLProcessingInstruction::Dump(ostream* pStr) const
{
	*pStr << "<ProcessingInstruction Data=\"" << CXMLUtils::XMLToAscii(m_data) << "\" Target=\"" << CXMLUtils::XMLToAscii(m_target) << "\"/>" << endl;
}


CXMLObject* CXMLProcessingInstruction::Copy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLProcessingInstruction();
	static_cast<CXMLProcessingInstruction*>(pCopy)->SetTarget(m_target);
	static_cast<CXMLProcessingInstruction*>(pCopy)->SetData(m_data);
	return CXMLObject::Copy(pCopy);
}


CXMLObject* CXMLProcessingInstruction::DeepCopy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLProcessingInstruction();
	static_cast<CXMLProcessingInstruction*>(pCopy)->SetTarget(m_target);
	static_cast<CXMLProcessingInstruction*>(pCopy)->SetData(m_data);
	return CXMLObject::DeepCopy(pCopy);
}


//
// CXMLExternalEntity
//


CXMLExternalEntity::CXMLExternalEntity()
{
}


CXMLExternalEntity::CXMLExternalEntity(const XMLString& systemId, const XMLString& publicId)
{
	m_systemId = systemId;
	m_publicId = publicId;
}


XMLString CXMLExternalEntity::GetSystemId() const
{
	return m_systemId;
}


XMLString CXMLExternalEntity::GetPublicId() const
{
	return m_publicId;
}


void CXMLExternalEntity::SetSystemId(const XMLString& systemId)
{
	m_systemId = systemId;
}


void CXMLExternalEntity::SetPublicId(const XMLString& publicId)
{
	m_publicId = publicId;
}


XMLObjectType CXMLExternalEntity::GetType() const
{
	return xmlExternalEntity;
}


void CXMLExternalEntity::Dump(ostream* pStr) const
{
	*pStr << "<ExternalEntity SystemId=\"" << CXMLUtils::XMLToAscii(m_systemId) << "\" PublicId=\"" << CXMLUtils::XMLToAscii(m_publicId) << "\">" << endl;
	CXMLContainer::Dump(pStr);
	*pStr << "</ExternalEntity>" << endl;
}


CXMLObject* CXMLExternalEntity::Copy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLExternalEntity();
	static_cast<CXMLExternalEntity*>(pCopy)->SetSystemId(m_systemId);
	static_cast<CXMLExternalEntity*>(pCopy)->SetPublicId(m_publicId);
	return CXMLContainer::Copy(pCopy);
}


CXMLObject* CXMLExternalEntity::DeepCopy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLExternalEntity();
	static_cast<CXMLExternalEntity*>(pCopy)->SetSystemId(m_systemId);
	static_cast<CXMLExternalEntity*>(pCopy)->SetPublicId(m_publicId);
	return CXMLContainer::DeepCopy(pCopy);
}


//
// CXMLNotationDeclaration
//


CXMLNotationDeclaration::CXMLNotationDeclaration()
{
}


CXMLNotationDeclaration::CXMLNotationDeclaration(const XMLString& name, const XMLString& systemId, const XMLString& publicId)
{
	m_name = m_stringPool.Get(name);
	m_systemId = systemId;
	m_publicId = publicId;
}


XMLString CXMLNotationDeclaration::GetName() const
{
	return m_name;
}


XMLString CXMLNotationDeclaration::GetSystemId() const
{
	return m_systemId;
}


XMLString CXMLNotationDeclaration::GetPublicId() const
{
	return m_publicId;
}


void CXMLNotationDeclaration::SetName(const XMLString& name)
{
	m_name = m_stringPool.Get(name);
}


void CXMLNotationDeclaration::SetSystemId(const XMLString& systemId)
{
	m_systemId = systemId;
}


void CXMLNotationDeclaration::SetPublicId(const XMLString& publicId)
{
	m_publicId = publicId;
}


XMLObjectType CXMLNotationDeclaration::GetType() const
{
	return xmlNotationDeclaration;
}


void CXMLNotationDeclaration::Dump(ostream* pStr) const
{
	*pStr 
		<< "<NotationDeclaration Name=\"" << CXMLUtils::XMLToAscii(m_name) 
		<< "\" SystemId=\"" << CXMLUtils::XMLToAscii(m_systemId) 
		<< "\" PublicId=\"" << CXMLUtils::XMLToAscii(m_publicId) <<  "\"/>" << endl;
}


CXMLObject* CXMLNotationDeclaration::Copy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLNotationDeclaration();
	static_cast<CXMLNotationDeclaration*>(pCopy)->SetName(m_name);
	static_cast<CXMLNotationDeclaration*>(pCopy)->SetSystemId(m_systemId);
	static_cast<CXMLNotationDeclaration*>(pCopy)->SetPublicId(m_publicId);
	return CXMLObject::Copy(pCopy);
}


CXMLObject* CXMLNotationDeclaration::DeepCopy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLNotationDeclaration();
	static_cast<CXMLNotationDeclaration*>(pCopy)->SetName(m_name);
	static_cast<CXMLNotationDeclaration*>(pCopy)->SetSystemId(m_systemId);
	static_cast<CXMLNotationDeclaration*>(pCopy)->SetPublicId(m_publicId);
	return CXMLObject::DeepCopy(pCopy);
}


//
// CXMLUnparsedEntityDeclaration
//


CXMLUnparsedEntityDeclaration::CXMLUnparsedEntityDeclaration()
{
}


CXMLUnparsedEntityDeclaration::CXMLUnparsedEntityDeclaration(const XMLString& entityName, const XMLString& systemId, const XMLString& publicId, const XMLString& notationName)
{
	m_entityName = m_stringPool.Get(entityName);
	m_systemId = systemId;
	m_publicId = publicId;
	m_notationName = m_stringPool.Get(notationName);
}


XMLString CXMLUnparsedEntityDeclaration::GetName() const
{
	return m_entityName;
}


XMLString CXMLUnparsedEntityDeclaration::GetSystemId() const
{
	return m_systemId;
}


XMLString CXMLUnparsedEntityDeclaration::GetPublicId() const
{
	return m_publicId;
}


XMLString CXMLUnparsedEntityDeclaration::GetNotationName() const
{
	return m_notationName;
}


void CXMLUnparsedEntityDeclaration::SetName(const XMLString& name)
{
	m_entityName = m_stringPool.Get(name);
}


void CXMLUnparsedEntityDeclaration::SetSystemId(const XMLString& systemId)
{
	m_systemId = systemId;
}


void CXMLUnparsedEntityDeclaration::SetPublicId(const XMLString& publicId)
{
	m_publicId = publicId;
}


void CXMLUnparsedEntityDeclaration::SetNotationName(const XMLString& notationName)
{
	m_notationName = m_stringPool.Get(notationName);
}


XMLObjectType CXMLUnparsedEntityDeclaration::GetType() const
{
	return xmlUnparsedEntityDeclaration;
}


void CXMLUnparsedEntityDeclaration::Dump(ostream* pStr) const
{
	*pStr 
		<< "<UnparsedEntityDeclaration Name=\"" << CXMLUtils::XMLToAscii(m_entityName) 
		<< "\" SystemId=\"" << CXMLUtils::XMLToAscii(m_systemId) 
		<< "\" PublicId=\"" << CXMLUtils::XMLToAscii(m_publicId) 
		<< "\" NotationName=\"" << CXMLUtils::XMLToAscii(m_notationName) <<  "\"/>" << endl;
}


CXMLObject* CXMLUnparsedEntityDeclaration::Copy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLUnparsedEntityDeclaration();
	static_cast<CXMLUnparsedEntityDeclaration*>(pCopy)->SetName(m_entityName);
	static_cast<CXMLUnparsedEntityDeclaration*>(pCopy)->SetSystemId(m_systemId);
	static_cast<CXMLUnparsedEntityDeclaration*>(pCopy)->SetPublicId(m_publicId);
	static_cast<CXMLUnparsedEntityDeclaration*>(pCopy)->SetNotationName(m_notationName);
	return CXMLObject::Copy(pCopy);
}


CXMLObject* CXMLUnparsedEntityDeclaration::DeepCopy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLUnparsedEntityDeclaration();
	static_cast<CXMLUnparsedEntityDeclaration*>(pCopy)->SetName(m_entityName);
	static_cast<CXMLUnparsedEntityDeclaration*>(pCopy)->SetSystemId(m_systemId);
	static_cast<CXMLUnparsedEntityDeclaration*>(pCopy)->SetPublicId(m_publicId);
	static_cast<CXMLUnparsedEntityDeclaration*>(pCopy)->SetNotationName(m_notationName);
	return CXMLObject::DeepCopy(pCopy);
}


//
// CXMLDocument
//


CXMLDocument::CXMLDocument()
{
}


CXMLDocument::CXMLDocument(const XMLString& name)
{
	m_name = m_stringPool.Get(name);
}


CXMLDocument::~CXMLDocument()
{
	Empty();
}


XMLString CXMLDocument::GetName() const
{
	return m_name;
}


void CXMLDocument::SetName(const XMLString& name)
{
	m_name = m_stringPool.Get(name);
}


XMLString CXMLDocument::GetBase() const
{
	return m_base;
}


void CXMLDocument::SetBase(const XMLString& base)
{
	m_base = base;
}


XMLObjectType CXMLDocument::GetType() const
{
	return xmlDocument;
}


void CXMLDocument::Dump(ostream* pStr) const
{
	*pStr << "<Document Name=\"" << CXMLUtils::XMLToAscii(m_name) << "\" Base=\"" << CXMLUtils::XMLToAscii(m_base) << "\">" << endl;
	CXMLContainer::Dump(pStr);
	*pStr << "</Document>" << endl;
}


CXMLObject* CXMLDocument::Copy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLDocument();
	static_cast<CXMLDocument*>(pCopy)->SetName(m_name);
	static_cast<CXMLDocument*>(pCopy)->SetBase(m_base);
	return CXMLContainer::Copy(pCopy);
}


CXMLObject* CXMLDocument::DeepCopy(CXMLObject* pCopy) const
{
	if (!pCopy) pCopy = new CXMLDocument();
	static_cast<CXMLDocument*>(pCopy)->SetName(m_name);
	static_cast<CXMLDocument*>(pCopy)->SetBase(m_base);
	return CXMLContainer::DeepCopy(pCopy);
}


//
// CXMLFactory
//


CXMLAttribute* CXMLFactory::CreateAttribute(const XMLString& name, const XMLString& type, const XMLString& value, bool specified)
{
	return new CXMLAttribute(name, type, value, specified);
}


CXMLElement* CXMLFactory::CreateElement(const XMLString& name) const
{
	return new CXMLElement(name);
}


CXMLCharacterData* CXMLFactory::CreateCharacterData(const XMLString& data) const
{
	return new CXMLCharacterData(data);
}


CXMLGenericData* CXMLFactory::CreateGenericData(const XMLString& data) const
{
	return new CXMLGenericData(data);
}


CXMLProcessingInstruction* CXMLFactory::CreateProcessingInstruction(const XMLString& target, const XMLString& data) const
{
	return new CXMLProcessingInstruction(target, data);
}


CXMLExternalEntity* CXMLFactory::CreateExternalEntity(const XMLString& systemId, const XMLString& publicId) const
{
	return new CXMLExternalEntity(systemId, publicId);
}


CXMLNotationDeclaration* CXMLFactory::CreateNotationDeclaration(const XMLString& name, const XMLString& systemId, const XMLString& publicId) const
{
	return new CXMLNotationDeclaration(name, systemId, publicId);
}


CXMLUnparsedEntityDeclaration* CXMLFactory::CreateUnparsedEntityDeclaration(const XMLString& entityName, const XMLString& systemId, const XMLString& publicId, const XMLString& notationName) const
{
	return new CXMLUnparsedEntityDeclaration(entityName, systemId, publicId, notationName);
}


CXMLCDATASection* CXMLFactory::CreateCDATASection(const XMLString& data) const
{
	return new CXMLCDATASection(data);
}


CXMLComment* CXMLFactory::CreateComment(const XMLString& data) const
{
	return new CXMLComment(data);
}


CXMLDocument* CXMLFactory::CreateDocument(const XMLString& name) const
{
	return new CXMLDocument(name);
}


XML_END

