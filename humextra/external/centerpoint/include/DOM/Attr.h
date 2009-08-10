//
// Attr.h
//
// $Id: //depot/XML/Main/include/DOM/Attr.h#12 $
//
// Document Object Model (Core) Level 1
//
// Interface Attr
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


#ifndef DOM_Attr_H_INCLUDED
#define DOM_Attr_H_INCLUDED


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
struct XMLName;


class XML_API Attr: public SimpleNode
	//; The Attr interface represents an attribute in an Element object.
{
public:
	virtual const XMLString& getName() const;
		//; Returns the name of this attribute.

	virtual bool getSpecified() const;
		//: If this attribute was explicitly given a value in
		//+ the original document, this is true; otherwise,
		//. it is false.

	virtual const XMLString& getValue() const;
		//; Returns the value of the attribute as a string.

	virtual void setValue(const XMLString& value);
		//; Sets the value of the attribute as a string.

	// Node
	virtual Node* getParentNode() const;
	virtual const XMLString& getNodeName() const;
	virtual const XMLString& getNodeValue() const;
	virtual void setNodeValue(const XMLString& value);
	virtual unsigned short getNodeType() const;
	virtual Node* getPreviousSibling() const;
	virtual Node* cloneNode(bool deep) const;

	// DOM Level 2
	virtual const XMLString& getNamespaceURI() const;
	virtual const XMLString& getLocalName() const;
	virtual Element* getOwnerElement() const;

	// IMPLEMENTATION
	Attr(Document* pOwnerDocument, Element* pOwnerElement, const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname, const XMLString& value);

protected:
	Attr(const Attr& attr);
	virtual ~Attr();

	void changeOwnerDocument(Document* pOwnerDocument);

private:
	const XMLName* m_pName;
	XMLString      m_value;

	friend class Document;
	friend class Element;
};


class XML_API UnspecAttr: public Attr
{
public:
	virtual bool getSpecified() const;
		//; Always returns false.

	// Node
	virtual Node* cloneNode(bool deep) const;

	// IMPLEMENTATION
	UnspecAttr(Document* pOwnerDocument, Element* pOwnerElement, const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname, const XMLString& value);

protected:
	UnspecAttr(const UnspecAttr& attr);
};


XML_END


#endif // DOM_Attr_H_INCLUDED
