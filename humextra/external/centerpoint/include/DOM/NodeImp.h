//
// NodeImp.h
//
// $Id: //depot/XML/Main/include/DOM/NodeImp.h#13 $
//
// Document Object Model (Core) Level 1
//
// IMPLEMENTATION
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


#ifndef DOM_NodeImp_H_INCLUDED
#define DOM_NodeImp_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef DOM_Node_H_INCLUDED
#include "DOM/Node.h"
#endif
#ifndef DOM_MutationEvent_H_INCLUDED
#include "DOM/MutationEvent.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif
#ifndef STD_VECTOR_INCLUDED
#include <vector>
#define STD_VECTOR_INCLUDED
#endif


XML_BEGIN


class ContainerNode;
class Attr;


class XML_API SimpleNode: public Node
	//: SimpleNode is an implementation of Node
	//+ for leaf elements (i.e. elements that
	//. do not contain other elements.
{
public:
	virtual const XMLString& getNodeName() const;
	virtual const XMLString& getNodeValue() const;
	virtual void setNodeValue(const XMLString& value);
	virtual Node* getParentNode() const;
	virtual NodeList* getChildNodes() const;
	virtual Node* getFirstChild() const;
	virtual Node* getLastChild() const;
	virtual Node* getPreviousSibling() const;
	virtual Node* getNextSibling() const;
	virtual NamedNodeMap* getAttributes() const;
	virtual Document* getOwnerDocument() const;
	virtual Node* insertBefore(Node* newChild, Node* refChild);
	virtual Node* replaceChild(Node* newChild, Node* oldChild);
	virtual Node* removeChild(Node* oldChild);
	virtual Node* appendChild(Node* newChild);
	virtual bool hasChildNodes() const;
	virtual Node* cloneNode(bool deep) const;
	virtual void normalize();
	virtual bool isSupported(const XMLString& feature, const XMLString& version) const;
	virtual const XMLString& getNamespaceURI() const;
	virtual XMLString getPrefix() const;
	virtual const XMLString& getLocalName() const;
	virtual bool hasAttributes() const;

	// EventTarget
	virtual void addEventListener(const XMLString& type, EventListener* listener, bool useCapture);
	virtual void removeEventListener(const XMLString& type, EventListener* listener, bool useCapture);
	virtual bool dispatchEvent(Event* evt);

	// IMPLEMENTATION
	SimpleNode(Document* pOwner);
	virtual void autoRelease(DOMAutoReleasePool* pAutoReleasePool = NULL);

protected:
	SimpleNode(const SimpleNode& node);
	virtual ~SimpleNode();

	void captureEvent(Event* evt);
	void bubbleEvent(Event* evt);
	void dispatchSubtreeModified();
	void dispatchNodeInserted();
	void dispatchNodeRemoved();
	void dispatchNodeRemovedFromDocument();
	void dispatchNodeInsertedIntoDocument();
	void dispatchAttrModified(Attr* pAttr, MutationEvent::AttrChangeType changeType, const XMLString& prevValue, const XMLString& newValue);
	void dispatchCharacterDataModified(const XMLString& prevValue, const XMLString& newValue);
	virtual void changeOwnerDocument(Document* pOwnerDocument);

	static const XMLString EMPTY_STRING;

private:
	static const XMLString NODE_NAME;

	struct EventListenerEntry
	{
		XMLString      m_type;
		EventListener* m_pListener;
		bool           m_useCapture;
	};

	typedef vector<EventListenerEntry> EventListenerVec;

	ContainerNode*    m_pParent;
	SimpleNode*       m_pNext;
	Document*         m_pOwner;
	EventListenerVec* m_pEventListeners;

	friend class ContainerNode;
	friend class Document;
	friend class Element;
	friend class Attr;
	friend class CharacterData;
	friend class DOMBuilder;
};


class XML_API ContainerNode: public SimpleNode
	//: Container is an implementation of Node
	//. that stores references to child elements.
{
public:
	virtual Node* getFirstChild() const;
	virtual Node* getLastChild() const;
	virtual Node* insertBefore(Node* newChild, Node* refChild);
	virtual Node* replaceChild(Node* newChild, Node* oldChild);
	virtual Node* removeChild(Node* oldChild);
	virtual Node* appendChild(Node* newChild);
	virtual bool hasChildNodes() const;
	virtual bool hasAttributes() const;

	// IMPLEMENTATION
	ContainerNode(Document* pOwnerDocument);

protected:
	ContainerNode(const ContainerNode& node);
	virtual ~ContainerNode();

	void dispatchNodeRemovedFromDocument();
	void dispatchNodeInsertedIntoDocument();
	void changeOwnerDocument(Document* pOwnerDocument);

private:
	SimpleNode* m_pFirstChild;

	friend class SimpleNode;
	friend class DOMBuilder;
};


XML_END


#endif // DOM_NodeImp_H_INCLUDED
