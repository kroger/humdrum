//
// AttributesImpl.h
//
// $Id: //depot/XML/Main/include/SAX/AttributesImpl.h#5 $
//
// Implementation of the SAX2 Attributes Interface.
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


#ifndef SAX_AttributesImpl_H_INCLUDED
#define SAX_AttributesImpl_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_Attributes_H_INCLUDED
#include "SAX/Attributes.h"
#endif
#ifndef STD_VECTOR_INCLUDED
#include <vector>
#define STD_VECTOR_INCLUDED
#endif


XML_BEGIN


class XML_API CAttributesImpl: public CAttributes
{
public:
	CAttributesImpl();
	CAttributesImpl(const CAttributes& list);

	virtual int GetIndex(const XMLString& name) const;
	virtual int GetIndex(const XMLString& uri, const XMLString& localPart) const;
	virtual int GetLength() const;
	virtual XMLString GetLocalName(int i) const;
	virtual XMLString GetQName(int i) const;
	virtual XMLString GetType(int i) const;
	virtual XMLString GetType(const XMLString& qname) const;
	virtual XMLString GetType(const XMLString& uri, const XMLString& localName) const;
	virtual XMLString GetValue(int i) const;
	virtual XMLString GetValue(const XMLString& qname) const;
	virtual XMLString GetValue(const XMLString& uri, const XMLString& localName) const;
	virtual XMLString GetURI(int i) const;

	bool GetSpecified(int i) const;
		//; Returns true if the attribute was specified (and not implied).

	bool GetSpecified(const XMLString& qname) const;
		//; Returns true if the attribute was specified (and not implied).

	bool GetSpecified(const XMLString& uri, const XMLString& localName) const;
		//; Returns true if the attribute was specified (and not implied).

	void SetValue(int i, const XMLString& value);
		//; Sets the value of an attribute.

	void SetValue(const XMLString& qname, const XMLString& value);
		//; Sets the value of an attribute.

	void SetValue(const XMLString& uri, const XMLString& localName, const XMLString& value);
		//; Sets the value of an attribute.

	void SetAttributes(const CAttributes& list);
		//; Copies the attributes from another attribute list.

	void SetAttribute(int i, const XMLString& uri, const XMLString& localName, const XMLString& qname, const XMLString& type, const XMLString& value);
		//; Sets an attribute.

	void AddAttribute(const XMLString& uri, const XMLString& localName, const XMLString& qname, const XMLString& type, const XMLString& value);
		//; Adds an attribute to the end of the list.

	void AddAttribute(const XMLString& uri, const XMLString& localName, const XMLString& qname, const XMLString& type, const XMLString& value, bool specified);
		//; Adds an attribute to the end of the list.

	void AddAttribute(const XMLChar* uri, const XMLChar* localName, const XMLChar* qname, const XMLChar* type, const XMLChar* value, bool specified);
		//; Adds an attribute to the end of the list.

	void RemoveAttribute(int i);
		//; Removes an attribute.

	void RemoveAttribute(const XMLString& qname);
		//; Removes an attribute.

	void RemoveAttribute(const XMLString& uri, const XMLString& localName);
		//; Removes an attribute.

	void Clear();
		//; Removes all attributes.

	void SetLocalName(int i, const XMLString& localName);
		//; Sets the local name of an attribute.

	void SetQName(int i, const XMLString& qname);
		//; Sets the qualified name of an attribute.

	void SetType(int i, const XMLString& type);
		//; Sets the type of an attribute.

	void SetURI(int i, const XMLString& uri);
		//; Sets the namespace URI of an attribute.

	virtual ~CAttributesImpl();

private:
	struct Attrib
	{
		Attrib() {}
		XMLString m_localName;
		XMLString m_uri;
		XMLString m_qname;
		XMLString m_value;
		XMLString m_type;
		bool      m_specified;
	};

	Attrib* Find(const XMLString& qname) const;
	Attrib* Find(const XMLString& uri, const XMLString& localName) const;

	vector<Attrib> m_attributes;
};


XML_END


#endif // SAX_AttributesImpl_H_INCLUDED

