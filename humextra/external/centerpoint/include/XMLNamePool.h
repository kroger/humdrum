//
// XMLNamePool.h
//
// $Id: //depot/XML/Main/include/XMLNamePool.h#7 $
//
// A pool for XML names.
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


#ifndef XMLNamePool_H_INCLUDED
#define XMLNamePool_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


struct XMLName
	//: An XML element or attribute name, consisting of a
	//. qualified name, a namespace URI and a local name.
{
	XMLString m_qname;
	XMLString m_namespaceURI;
	XMLString m_localName;
};


class XML_API XMLNamePool
	//: A hashtable that stores XML names consisting of an URI, a
	//. local name and a qualified name.
{
public:
	XMLNamePool(unsigned long size = 251);
		//; Creates a CStringPool with room for size strings.

	virtual ~XMLNamePool();
	
	virtual const XMLName* Insert(const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname);	
		//: Returns a pointer to an XMLName for the given names.
		//+ Creates the XMLName if it does not already exist.
		//. Throws an exception (EXMLNamePoolFull) if the name pool is full.

	void Duplicate();
		//; Increments the reference count.

	void Release();
		//; Decrements the reference count and deletes the object if the reference count reaches zero.

protected:
	unsigned long Hash(const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname);

private:
	struct Item: public XMLName
	{
		bool m_used;
	};
	Item*         m_pPool;
	unsigned long m_size;
	int           m_refCount;
};


XML_END


#endif // XMLNamePool_H_INCLUDED
