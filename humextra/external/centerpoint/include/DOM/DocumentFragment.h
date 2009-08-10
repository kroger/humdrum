//
// DocumentFragment.h
//
// $Id: //depot/XML/Main/include/DOM/DocumentFragment.h#7 $
//
// Document Object Model (Core) Level 1
//
// Interface DocumentFragment
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


#ifndef DOM_DocumentFragment_H_INCLUDED
#define DOM_DocumentFragment_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef DOM_NodeImp_H_INCLUDED
#include "DOM/NodeImp.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class Document;


class XML_API DocumentFragment: public ContainerNode
	//: DocumentFragment is a "lightweight" or "minimal" Document object.
	//+ It is useful for extracting a portion of a document tree or to
	//+ create a new fragment of a document.
	//+ When a DocumentFragment is inserted into a Document (or indeed
	//+ any other Node that may take children) the children of the
	//+ DocumentFragment and not the DocumentFragment itself are inserted
	//+ into the Node.
	//+ When you are done with a DocumentFragment object, send it the
	//. release() message to get rid of it.
{
public:
	// Node
	const XMLString& getNodeName() const;
	Node* cloneNode(bool deep) const;
	unsigned short getNodeType() const;

	// IMPLEMENTATION
	DocumentFragment(Document* pOwnerDocument);

protected:
	DocumentFragment(const DocumentFragment& frag);
	virtual ~DocumentFragment();

private:
	static const XMLString NODE_NAME;
};


XML_END


#endif // DOM_DocumentFragment_H_INCLUDED
