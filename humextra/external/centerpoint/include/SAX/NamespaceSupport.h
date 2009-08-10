//
// NamespaceSupport.h
//
// $Id$
//
// Namespace support for SAX2.
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


#ifndef SAX_NamespaceSupport_H_INCLUDED
#define SAX_NamespaceSupport_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif
#ifndef STD_VECTOR_INCLUDED
#include <vector>
#define STD_VECTOR_INCLUDED
#endif
#ifndef STD_MAP_INCLUDED
#include <map>
#define STD_MAP_INCLUDED
#endif


XML_BEGIN


class XML_API CNamespaceSupport
	//: Encapsulate Namespace logic for use by SAX drivers. 
	//+ This class encapsulates the logic of Namespace processing: 
	//+ it tracks the declarations currently in force for each context and
	//+ automatically processes qualified XML 1.0 names into their Namespace 
	//+ parts; it can also be used in reverse for generating
	//+ XML 1.0 from Namespaces.
	//+ Namespace support objects are reusable, but the reset method 
	//. must be invoked between each session.
{
public:
	bool DeclarePrefix(const XMLString& prefix, const XMLString& uri);
		//: Declare a namespace prefix. Returns true if the prefix
		//. is legal, false otherwise.

	vector<XMLString> GetDeclaredPrefixes() const;
		//: Returns a vector of all prefixes declared in the context.
		//. The empty (default) prefix will be included in this enumeration.

	const XMLString& GetPrefix(const XMLString& uri) const;
		//; Returns one of the prefixes mapped to uri.

	vector<XMLString> GetPrefixes() const;
		//: Return an enumeration of all prefixes currently declared. 
		//+ Note: If there is a default prefix, it will not be returned in 
		//+ this enumeration; check for the default prefix using the getURI
		//. with an argument of "".

	vector<XMLString> GetPrefixes(const XMLString& uri) const;
		//: Return an enumeration of all prefixes currently declared for a URI. 
		//+ This method returns prefixes mapped to a specific Namespace URI. 
		//+ The xml: prefix will be included. If you want only
		//+ one prefix that's mapped to the Namespace URI, and you don't care 
		//+ which one you get, use the getPrefix method
		//+ instead.
		//+
		//+ Note: the empty (default) prefix is never included in this enumeration; 
		//+ to check for the presence of a default Namespace,
		//. use the getURI method with an argument of "".

	const XMLString& GetURI(const XMLString& prefix) const;
		//: Look up a prefix and get the currently-mapped Namespace URI. 
		//+ Use the empty string ("") for the default Namespace.
		//. Returns an empty string if prefix is not declared.

	void PushContext();
		//; Start a new Namespace context.

	void PopContext();
		//; Revert to the previous Namespace context.

	bool ProcessName(const XMLString& qName, XMLString& uri, XMLString& localName, bool isAttribute) const;
		//: Process a raw XML 1.0 name. 
		//+ This method processes a raw XML 1.0 name in the current context 
		//+ by removing the prefix and looking it up among the
		//+ prefixes currently declared. The result will be returned in
		//+ uri, localName and qname.
		//+ If the raw name has a prefix that has not been declared, then the return
		//+ value will be false, otherwise true.
		//+
		//+ Note that attribute names are processed differently than element names: 
		//+ an unprefixed element name will received the
		//. default Namespace (if any), while an unprefixed element name will not.

	void Reset();
		//; Reset the object for reuse.

	CNamespaceSupport();
	virtual ~CNamespaceSupport();

private:
	typedef multimap<XMLString, XMLString> Context;
	vector<Context> m_contexts;

	static const XMLString EMPTY_STRING;
	static const XMLString XMLNS;
	static const XMLString XMLNS_PREFIX;
};


XML_END


#endif // SAX_NamespaceSupport_H_INCLUDED
