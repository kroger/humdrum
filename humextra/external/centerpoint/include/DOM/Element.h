//
// Element.h
//
// $Id: //depot/XML/Main/include/DOM/Element.h#13 $
//
// Document Object Model (Core) Level 1
//
// Interface Element
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


#ifndef DOM_Element_H_INCLUDED
#define DOM_Element_H_INCLUDED


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


class Attr;
class NodeList;
class Document;
struct XMLName;


class XML_API Element: public ContainerNode
	//; The Element represents an Element in a XML document.
{
public:
	virtual const XMLString& getTagName() const;
		//; Returns the name of the element.

	virtual const XMLString& getAttribute(const XMLString& name) const;
		//; Retrieves an attribute value by name.

	virtual void setAttribute(const XMLString& name, const XMLString& value);
		//: Adds a new attribute. If an attribute with that name
		//+ is already present in the element, its value is changed
		//. to be that of the value parameter.

	virtual void removeAttribute(const XMLString& name);
		//; Removes an attribute by name.

	virtual Attr* getAttributeNode(const XMLString& name) const;
		//; Retrieves an Attr node by name.

	virtual Attr* setAttributeNode(Attr* newAttr);
		//: Adds a new attribute. If an attribute with that name is already
		//. present in the element, it is replaced by the new one.

	virtual Attr* removeAttributeNode(Attr* oldAttr);
		//; Removes the specified attribute.

	virtual NodeList* getElementsByTagName(const XMLString& name) const;
		//: Returns a NodeList of all descendant elements with a given tag
		//+ name, in the order in which they would be encountered in a
		//+ preorder traversal of the Element tree.
		//. The special name "*" matches all tags.

	virtual void normalize();
		//: Puts all Text nodes in the full depth of the sub-tree underneath
		//+ this Element in to a "normal" form where only markup (e.g. tags,
		//+ comments, PI, CDATA, ...) separates Text nodes, i.e., there are
		//. no adjacent Text nodes.

	// Node
	virtual const XMLString& getNodeName() const;
	virtual NamedNodeMap* getAttributes() const;
	virtual Node* cloneNode(bool deep) const;
	virtual unsigned short getNodeType() const;

	// DOM Level 2
	const XMLString& getAttributeNS(const XMLString& namespaceURI, const XMLString& localName) const;
		//; Retrieves an attribute value by name.

	void setAttributeNS(const XMLString& namespaceURI, const XMLString& qualifiedName, const XMLString& value);
		//: Adds a new attribute. If an attribute with that name
		//+ is already present in the element, its value is changed
		//. to be that of the value parameter.

	void removeAttributeNS(const XMLString& namespaceURI, const XMLString& localName);
		//; Removes an attribute by name.

	Attr* getAttributeNodeNS(const XMLString& namespaceURI, const XMLString& localName) const;
		//; Retrieves an Attr node by name.

	Attr* setAttributeNodeNS(Attr* newAttr);
		//: Adds a new attribute. If an attribute with that name is already
		//. present in the element, it is replaced by the new one.

	bool hasAttribute(const XMLString& name) const;
		//; Returns true if and only if the element has the specified attribute.

	bool hasAttributeNS(const XMLString& namespaceURI, const XMLString& localName) const;
		//; Returns true if and only if the element has the specified attribute.

	NodeList* getElementsByTagNameNS(const XMLString& namespaceURI, const XMLString& localName) const;
		//: Returns a NodeList of all the descendant Elements with a given local name and namespace URI 
		//. in the order in which they are encountered in a preorder traversal of this Element tree.

	const XMLString& getNamespaceURI() const;
	const XMLString& getLocalName() const;
	bool hasAttributes() const;

	// IMPLEMENTATION
	Element(Document* pOwnerDocument, const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname);
	void dispatchNodeRemovedFromDocument();
	void dispatchNodeInsertedIntoDocument();

protected:
	Element(const Element& elem);
	virtual ~Element();

	void changeOwnerDocument(Document* pOwnerDocument);

private:
	const XMLName* m_pName;
	Attr*          m_pFirstAttr;

	friend class Attr;
	friend class Document;
	friend class AttrMap;
};


XML_END


#endif // DOM_Element_H_INCLUDED
