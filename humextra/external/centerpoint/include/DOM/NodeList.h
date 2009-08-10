//
// NodeList.h
//
// $Id: //depot/XML/Main/include/DOM/NodeList.h#6 $
//
// Document Object Model (Core) Level 1
//
// Interface NodeList
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


#ifndef DOM_NodeList_H_INCLUDED
#define DOM_NodeList_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef DOM_DOMObject_H_INCLUDED
#include "DOM/DOMObject.h"
#endif


XML_BEGIN


class Node;


class XML_API NodeList: public DOMObject
	//: The NodeList interface provides the abstraction of an ordered
	//+ collection of nodes, without defining or constraining how this
	//+ collection is implemented.
	//+ After you are done with a NodeList, send it the Release()
	//. message to get rid of it.
{
public:
	virtual Node* item(unsigned long index) const = 0;
		//: Returns the index'th item in the collection. If index is
		//+ greater than or equal to the number of nodes in the list,
		//. this returns NULL.

	virtual unsigned long getLength() const = 0;
		//: Returns the number of nodes in the list. The range of valid
		//. node indices is 0 to length - 1 inclusive.
	
	// IMPLEMENTATION
protected:
	virtual ~NodeList();
};


XML_END


#endif // DOM_NodeList_H_INCLUDED
