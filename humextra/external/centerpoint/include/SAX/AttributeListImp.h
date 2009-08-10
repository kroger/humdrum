//
// AttributeListImp.h
//
// $Id: //depot/XML/Main/include/SAX/AttributeListImp.h#6 $
//
// A simple implementation of CAttributeList provided for your convenience.
//
// DEPRECATED. Use AttributesImpl.h instead.
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


#ifndef SAX_AttributeListImp_H_INCLUDED
#define SAX_AttributeListImp_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_AttributeList_H_INCLUDED
#include "SAX/AttributeList.h"
#endif
#ifndef STD_VECTOR_INCLUDED
#include <vector>
#define STD_VECTOR_INCLUDED
#endif


XML_BEGIN


class XML_API CAttributeListImp: public CAttributeList
{
public:
	CAttributeListImp();
	CAttributeListImp(const CAttributeList& list);
	virtual ~CAttributeListImp();

	int GetLength() const;
	XMLString GetName(int i) const;
	XMLString GetType(int i) const;
	XMLString GetValue(int i) const;
	XMLString GetType(const XMLString& name) const;
	XMLString GetValue(const XMLString& name) const;

	void SetValue(int i, const XMLString& value);
	void SetValue(const XMLString& name, const XMLString& value);

	bool GetSpecified(int i) const;
	bool GetSpecified(const XMLString& name) const;

	void SetAttributeList(const CAttributeList& list);
	void AddAttribute(const XMLString& name, const XMLString& type, const XMLString& value);
	void AddAttribute(const XMLString& name, const XMLString& type, const XMLString& value, bool specified);
	void AddAttribute(const XMLChar* name, const XMLChar* type, const XMLChar* value, bool specified);
	void RemoveAttribute(const XMLString& name);
	void Clear();

private:
	struct Attrib
	{
		Attrib() {}
		XMLString m_name;
		XMLString m_value;
		XMLString m_type;
		bool      m_specified;
	};

	Attrib* Find(const XMLString& name) const;

	vector<Attrib> m_attributes;
};


XML_END


#endif // SAX_AttributeListImp_H_INCLUDED

