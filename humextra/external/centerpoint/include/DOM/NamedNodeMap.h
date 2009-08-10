//
// NamedNodeMap.h
//
// $Id: //depot/XML/Main/include/DOM/NamedNodeMap.h#7 $
//
// Document Object Model (Core) Level 1
//
// Interface NamedNodeMap
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


#ifndef DOM_NamedNodeMap_H_INCLUDED
#define DOM_NamedNodeMap_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef DOM_DOMObject_H_INCLUDED
#include "DOM/DOMObject.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class Node;


class XML_API NamedNodeMap: public DOMObject
	//: Objects implementing the NamedNodeMap interface are used to
	//+ represent collections of nodes that can be accessed by name.
	//+ Note that NamedNodeMap does not inherit from NodeList.
	//+ NamedNodeMaps are not maintained in any particular order.
	//+ Objects contained in an object implementing NamedNodeMap
	//+ may also be accessed by an ordinal index.
	//+ After you are done with a NamedNodeMap, send it the Release()
	//. message to get rid of it.
{
public:
	virtual Node* getNamedItem(const XMLString& name) const = 0;
		//; Retrieves a node specified by name.

	virtual Node* setNamedItem(Node* arg) = 0;
		//; Adds a node using its nodeName attribute.

	virtual Node* removeNamedItem(const XMLString& name) = 0;
		//; Removes a node specified by name.

	virtual Node* item(unsigned long index) const = 0;
		//: Returns the index'th item in the map. If index is greater
		//+ than or equal to the number of nodes in the map, this
		//. returns NULL.

	virtual unsigned long getLength() const = 0;
		//: Returns the number of nodes in the map. The range of valid
		//. child node indices is 0 to length - 1 inclusive.
	
	// DOM Level 2
	virtual Node* getNamedItemNS(const XMLString& namespaceURI, const XMLString& localName) const = 0;
		//; Retrieves a node specified by name.

	virtual Node* setNamedItemNS(Node* arg) = 0;
		//; Adds a node using its nodeName attribute.

	virtual Node* removeNamedItemNS(const XMLString& namespaceURI, const XMLString& localName) = 0;
		//; Removes a node specified by name.
	
	// IMPLEMENTATION
protected:
	virtual ~NamedNodeMap();
};


XML_END


#endif // DOM_NamedNodeMap_H_INCLUDED
