//
// Node.h
//
// $Id: //depot/XML/Main/include/DOM/Node.h#12 $
//
// Document Object Model (Core) Level 1
//
// Interface Node
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


#ifndef DOM_Node_H_INCLUDED
#define DOM_Node_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef DOM_EventTarget_H_INCLUDED
#include "DOM/EventTarget.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class NamedNodeMap;
class Document;
class NodeList;


class XML_API Node: public EventTarget
	//: The Node interface is the primary datatype for the entire
	//+ Document Object Model. It represents a single node in the
	//+ document tree.
	//+ In this implementation, Node is an abstract superclass
	//. for ObjectNode and ContainerNode.
{
public:
	enum
	{
		ELEMENT_NODE = 1,             //; The node is an Element.
		ATTRIBUTE_NODE,               //; The node is an Attr.
		TEXT_NODE,                    //; The node is a Text node.
		CDATA_SECTION_NODE,           //; The node is a CDATASection.
		ENTITY_REFERENCE_NODE,        //; The node is an EntityReference.
		ENTITY_NODE,                  //; The node is an Entity.
		PROCESSING_INSTRUCTION_NODE,  //; The node is a ProcessingInstruction.
		COMMENT_NODE,                 //; The node is a Comment.
		DOCUMENT_NODE,                //; The node is a Document.
		DOCUMENT_TYPE_NODE,           //; The node is a DocumentType.
		DOCUMENT_FRAGMENT_NODE,       //; The node is a DocumentFragment.
		NOTATION_NODE                 //; The node is a Notation.
	};

	virtual const XMLString& getNodeName() const = 0;
		//; Returns the name of this node, depending on its type.

	virtual const XMLString& getNodeValue() const = 0;
		//; Returns the value of this node, depending on its type.

	virtual void setNodeValue(const XMLString& value) = 0;
		//: Sets the value of this node. Throws an exception
		//. if the node is read-only.

	virtual unsigned short getNodeType() const = 0;
		//: Returns a code representing the type of the underlying object
		//. (see above).

	virtual Node* getParentNode() const = 0;
		//: Returns the parent of this node. If the node has no parent,
		//. NULL is returned

	virtual NodeList* getChildNodes() const = 0;
		//: Returns a NodeList containing all children of this node.
		//+ When you are done with the NodeList, send it a release()
		//. message to get rid of it.

	virtual Node* getFirstChild() const = 0;
		//: Returns the first child of this node. If there is no such
		//. node, this returns NULL.

	virtual Node* getLastChild() const = 0;
		//: Returns the last child of this node. If there is no such
		//. node, this returns NULL.

	virtual Node* getPreviousSibling() const = 0;
		//: Returns the node immediately preceding this node. If there
		//. is no such node, this returns NULL.

	virtual Node* getNextSibling() const = 0;
		//: Returns the node immediately following this node. If there
		//. is no such node, this returns NULL.

	virtual NamedNodeMap* getAttributes() const = 0;
		//: Returns a NamedNodeMap containing the attributes of this
		//+ node (if it is an Element) or NULL otherwise.
		//+ When you are done with the NamedNodeMap, send it a
		//. release() message to get rid of it.

	virtual Document* getOwnerDocument() const = 0;
		//: Returns the Document object associated with this node.
		//. When this node is a Document, this is NULL.

	virtual Node* insertBefore(Node* newChild, Node* refChild) = 0;
		//: Inserts the node newChild before the existing child node refChild.
		//+ If refChild is NULL, insert newChild at the end of the list of children.
		//+ If newChild is a DocumentFragment object, all of its children are
		//+ inserted in the same order, before refChild. If the newChild is already
		//. in the tree, it is first removed.

	virtual Node* replaceChild(Node* newChild, Node* oldChild) = 0;
		//: Replaces the child node oldChild with newChild in the list of children
		//+ and returns the oldChild node. If newChild is already in the tree,
		//. it is first removed.

	virtual Node* removeChild(Node* oldChild) = 0;
		//: Removes the child node indicated by oldChild from the list of children
		//+ and returns it. To actually get rid of the node (thus, delete it), send
		//. it the Release() message.

	virtual Node* appendChild(Node* newChild) = 0;
		//: Appends the node newChild to the end of the list of children of this node.
		//. If newChild is already in the tree, it is first removed.

	virtual bool hasChildNodes() const = 0;
		//; Returns TRUE if the node has any children.

	virtual Node* cloneNode(bool deep) const = 0;
		//; Returns a duplicate of this node. The duplicate node has no parent.

	// DOM Level 2
	virtual void normalize() = 0;
		//: Puts all Text nodes in the full depth of the sub-tree underneath
		//+ this Element in to a "normal" form where only markup (e.g. tags,
		//+ comments, PI, CDATA, ...) separates Text nodes, i.e., there are
		//. no adjacent Text nodes.

	virtual bool isSupported(const XMLString& feature, const XMLString& version) const = 0;
		//: Tests whether the DOM implementation implements a specific 
		//. feature and that feature is supported by this node.

	virtual const XMLString& getNamespaceURI() const = 0;
		//; Returns the namespace URI of the node.

	virtual XMLString getPrefix() const = 0;
		//; Returns the namespace prefix from the qualified name of the node.

	virtual const XMLString& getLocalName() const = 0;
		//; Returns the local name of the node.

	virtual bool hasAttributes() const = 0;
		//; Returns true if and only if the node has attributes.
};


XML_END


#endif // DOM_Node_H_INCLUDED
