//
// Attributes.h
//
// $Id: //depot/XML/Main/include/SAX/Attributes.h#4 $
//
// SAX2 Attributes Interface.
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


#ifndef SAX_Attributes_H_INCLUDED
#define SAX_Attributes_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class XML_API CAttributes
	//: Interface for a list of XML attributes.
	//+ This interface allows access to a list of attributes in three different ways:
	//+   1.by attribute index; 
	//+   2.by Namespace-qualified name; or 
	//+   3.by qualified (prefixed) name. 
	//+ 
	//+ The list will not contain attributes that were declared #IMPLIED but not 
	//+ specified in the start tag. It will also not contain
	//+ attributes used as Namespace declarations (xmlns*) unless the 
	//+ http://xml.org/sax/features/namespace-prefixes
	//+ feature is set to true (it is false by default).
	//+ 
	//+ If the namespace-prefixes feature (see above) is false, access by 
	//+ qualified name may not be available; if the
	//+ http://xml.org/sax/features/namespaces feature is false, access by 
	//+ Namespace-qualified names may not be available.
	//+ This interface replaces the now-deprecated SAX1 AttributeList interface, 
	//+ which does not contain Namespace support. In
	//+ addition to Namespace support, it adds the getIndex methods (below).
	//+ The order of attributes in the list is unspecified, and will vary from 
	//. implementation to implementation.
{
public:
	virtual int GetIndex(const XMLString& name) const = 0;
		//; Look up the index of an attribute by a qualified name.

	virtual int GetIndex(const XMLString& uri, const XMLString& localPart) const = 0;
		//; Look up the index of an attribute by a namspace name.

	virtual int GetLength() const = 0;
		//; Return the number of attributes in the list.
	
	virtual XMLString GetLocalName(int i) const = 0;
		//; Look up a local attribute name by index.

	virtual XMLString GetQName(int i) const = 0;
		//; Look up a qualified attribute name by index.

	virtual XMLString GetType(int i) const = 0;
		//; Look up an attribute type by index.

	virtual XMLString GetType(const XMLString& qname) const = 0;
		//; Look up an attribute type by a qualified name.

	virtual XMLString GetType(const XMLString& uri, const XMLString& localName) const = 0;
		//; Look up an attribute type by a namespace name.

	virtual XMLString GetValue(int i) const = 0;
		//; Look up an attribute value by index.

	virtual XMLString GetValue(const XMLString& qname) const = 0;
		//; Look up an attribute value by a qualified name.

	virtual XMLString GetValue(const XMLString& uri, const XMLString& localName) const = 0;
		//; Look up an attribute value by a namespace name.

	virtual XMLString GetURI(int i) const = 0;
		//; Look up a namespace URI by index.

	virtual ~CAttributes();
};


XML_END


#endif // SAX_Attributes_H_INCLUDED

