//
// DOMImpl.cpp
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


#include "DOM/DOMObject.h"
#include "DOM/DOMAutoReleasePool.h"
#include "DOM/DOMException.h"
#include "DOM/Document.h"
#include "DOM/NodeImp.h"
#include "DOM/NodeList.h"
#include "DOM/NamedNodeMap.h"
#include "DOM/CharacterData.h"
#include "DOM/Comment.h"
#include "DOM/CDATASection.h"
#include "DOM/DocumentType.h"
#include "DOM/DocumentFragment.h"
#include "DOM/DOMImplementation.h"
#include "DOM/Element.h"
#include "DOM/Attr.h"
#include "DOM/Entity.h"
#include "DOM/EntityReference.h"
#include "DOM/Notation.h"
#include "DOM/ProcessingInstruction.h"
#include "DOM/Text.h"
#include "DOM/Event.h"
#include "DOM/MutationEvent.h"
#include "DOM/EventListener.h"
#include "DOM/EventException.h"
#include "XMLDocument.h"
#include "XMLUtils.h"
#include "XMLNamePool.h"
#include <set>
#include <limits.h>


XML_BEGIN


//
// For performance reasons we define this as a macro.
// (An inline method in NodeImp.h would cause a circular reference
// between Document.h and NodeImp.h).
//


#define mustDoEvents() (m_pOwner ? m_pOwner->getEventSuspendLevel() == 0 : true)


//
// Implementation specific classes not defined
// in separate header files.
//


class XML_API ChildNodeList: public NodeList
	// Objects of this kind are returned by
	// Node::getChildNodes().
	// This object just maintains a reference
	// to the node containing the children.
{
public:
	virtual Node* item(unsigned long index) const;
	virtual unsigned long getLength() const;

	// IMPLEMENTATION
	ChildNodeList(const Node* pParent);
	virtual void autoRelease(DOMAutoReleasePool* pAutoReleasePool);

private:
	const Node* m_pParent;
};


class XML_API ElementByTagNameList: public NodeList
	// Objects of this kind are returned by
	// Document::getElementsByTagName() and
	// Element::getElementsByTagName().
	// This object just maintains a reference
	// to the node containing the children,
	// together with the desired name.
{
public:
	virtual Node* item(unsigned long index) const;
	virtual unsigned long getLength() const;

	// IMPLEMENTATION
	ElementByTagNameList(const Node* pParent, const XMLString& name);
	virtual void autoRelease(DOMAutoReleasePool* pAutoReleasePool);

private:
	virtual Node* find(const Node* pParent, unsigned long index) const;

	const Node*           m_pParent;
	XMLString             m_name;
	mutable unsigned long m_count;
};


class XML_API ElementByTagNameListNS: public NodeList
	// Objects of this kind are returned by
	// Document::getElementsByTagNameNS() and
	// Element::getElementsByTagNameNS().
	// This object just maintains a reference
	// to the node containing the children,
	// together with the desired name.
{
public:
	virtual Node* item(unsigned long index) const;
	virtual unsigned long getLength() const;

	// IMPLEMENTATION
	ElementByTagNameListNS(const Node* pParent, const XMLString& namespaceURI, const XMLString& localName);
	virtual void autoRelease(DOMAutoReleasePool* pAutoReleasePool);

private:
	virtual Node* find(const Node* pParent, unsigned long index) const;

	const Node*           m_pParent;
	XMLString             m_localName;
	XMLString             m_namespaceURI;
	mutable unsigned long m_count;
};


class XML_API AttrMap: public NamedNodeMap
	// Objects of this kind are returned by
	// Element::getAttributes()
{
public:
	virtual Node* getNamedItem(const XMLString& name) const;
	virtual Node* setNamedItem(Node* arg);
	virtual Node* removeNamedItem(const XMLString& name);
	virtual Node* item(unsigned long index) const;
	virtual unsigned long getLength() const;

	virtual Node* getNamedItemNS(const XMLString& namespaceURI, const XMLString& localName) const;
	virtual Node* setNamedItemNS(Node* arg);
	virtual Node* removeNamedItemNS(const XMLString& namespaceURI, const XMLString& localName);

	// IMPLEMENTATION
	AttrMap(Element* pElem);
	virtual void autoRelease(DOMAutoReleasePool* pAutoReleasePool);

private:
	Element* m_pElem;
};


class XML_API DeclMap: public NamedNodeMap
	// Objects of this kind are returned by
	// DocumentType::getEntities() and
	// DocumentType::getNotations().
{
public:
	virtual Node* getNamedItem(const XMLString& name) const;
	virtual Node* setNamedItem(Node* arg);
	virtual Node* removeNamedItem(const XMLString& name);
	virtual Node* item(unsigned long index) const;
	virtual unsigned long getLength() const;

	virtual Node* getNamedItemNS(const XMLString& namespaceURI, const XMLString& localName) const;
	virtual Node* setNamedItemNS(Node* arg);
	virtual Node* removeNamedItemNS(const XMLString& namespaceURI, const XMLString& localName);

	// IMPLEMENTATION
	DeclMap(Document* pDoc, unsigned short type);
	virtual void autoRelease(DOMAutoReleasePool* pAutoReleasePool);

private:
	Document*      m_pDoc;
	unsigned short m_type;
};


//
// DOMObject
//


DOMObject::DOMObject()
{
	m_refCount = 1;
}


DOMObject::~DOMObject()
{
}


void DOMObject::autoRelease(DOMAutoReleasePool* pAutoReleasePool)
{
	csl_assert (pAutoReleasePool);
	
	pAutoReleasePool->add(this);
}


//
// ChildNodeList
//


ChildNodeList::ChildNodeList(const Node* pParent)
{
	csl_assert (pParent != NULL);

	m_pParent = pParent;
}


Node* ChildNodeList::item(unsigned long index) const
{
	// Just walk through the node's children until
	// the index'th child is found.
	unsigned long n = 0;
	Node* pCur = m_pParent->getFirstChild();
	while (pCur && n < index)
	{
		n++;
		pCur = pCur->getNextSibling();
	}
	return pCur;
}


unsigned long ChildNodeList::getLength() const
{
	// Count the node's children.
	unsigned long n = 0;
	Node* pCur = m_pParent->getFirstChild();
	while (pCur)
	{
		n++;
		pCur = pCur->getNextSibling();
	}
	return n;
}


void ChildNodeList::autoRelease(DOMAutoReleasePool* pAutoReleasePool)
{
	if (pAutoReleasePool == NULL) pAutoReleasePool = m_pParent->getOwnerDocument()->getAutoReleasePool();
	NodeList::autoRelease(pAutoReleasePool);
}


//
// ElementByTagNameList
//


ElementByTagNameList::ElementByTagNameList(const Node* pParent, const XMLString& name)
{
	csl_assert (pParent != NULL);

	m_pParent = pParent;
	m_name    = name;
	m_count   = 0;
}


Node* ElementByTagNameList::item(unsigned long index) const
{
	m_count = 0;
	return find(m_pParent, index);
}


unsigned long ElementByTagNameList::getLength() const
{
	m_count = 0;
	find(m_pParent, ULONG_MAX);
	return m_count;
}


Node* ElementByTagNameList::find(const Node* pParent, unsigned long index) const
{
	static const XMLString asterisk = CXMLUtils::AsciiToXML("*");

	if (pParent == NULL) return NULL;

	// preorder search
	Node* pCur = pParent->getFirstChild();
	while (pCur)
	{
		if (pCur->getNodeType() == Node::ELEMENT_NODE && (m_name == asterisk || pCur->getNodeName() == m_name))
		{
			if (m_count == index) return pCur;
			m_count++;
		}
		Node* pNode = find(pCur, index);
		if (pNode) return pNode;
		pCur = pCur->getNextSibling();
	}
	return pCur;
}


void ElementByTagNameList::autoRelease(DOMAutoReleasePool* pAutoReleasePool)
{
	if (pAutoReleasePool == NULL) pAutoReleasePool = m_pParent->getOwnerDocument()->getAutoReleasePool();
	NodeList::autoRelease(pAutoReleasePool);
}


ElementByTagNameListNS::ElementByTagNameListNS(const Node* pParent, const XMLString& namespaceURI, const XMLString& localName)
{
	csl_assert (pParent != NULL);

	m_pParent      = pParent;
	m_localName    = localName;
	m_namespaceURI = namespaceURI;
	m_count        = 0;
}


Node* ElementByTagNameListNS::item(unsigned long index) const
{
	m_count = 0;
	return find(m_pParent, index);
}


unsigned long ElementByTagNameListNS::getLength() const
{
	m_count = 0;
	find(m_pParent, ULONG_MAX);
	return m_count;
}


Node* ElementByTagNameListNS::find(const Node* pParent, unsigned long index) const
{
	static const XMLString asterisk = CXMLUtils::AsciiToXML("*");

	if (pParent == NULL) return NULL;

	// preorder search
	Node* pCur = pParent->getFirstChild();
	while (pCur)
	{
		if (pCur->getNodeType() == Node::ELEMENT_NODE && (m_localName == asterisk || pCur->getLocalName() == m_localName) && (m_namespaceURI == asterisk || pCur->getNamespaceURI() == m_namespaceURI))
		{
			if (m_count == index) return pCur;
			m_count++;
		}
		Node* pNode = find(pCur, index);
		if (pNode) return pNode;
		pCur = pCur->getNextSibling();
	}
	return pCur;
}


void ElementByTagNameListNS::autoRelease(DOMAutoReleasePool* pAutoReleasePool)
{
	if (pAutoReleasePool == NULL) pAutoReleasePool = m_pParent->getOwnerDocument()->getAutoReleasePool();
	NodeList::autoRelease(pAutoReleasePool);
}


//
// SimpleNode
//


const XMLString SimpleNode::NODE_NAME    = CXMLUtils::AsciiToXML("#node");
const XMLString SimpleNode::EMPTY_STRING;


SimpleNode::SimpleNode(Document* pOwner)
{
	m_pParent         = NULL;
	m_pNext           = NULL;
	m_pOwner          = pOwner;
	m_pEventListeners = NULL;
}


SimpleNode::SimpleNode(const SimpleNode& node)
{
	m_pParent         = NULL;
	m_pNext           = NULL;
	m_pOwner          = node.m_pOwner;
	m_pEventListeners = NULL;
}


SimpleNode::~SimpleNode()
{
	if (m_pEventListeners) delete m_pEventListeners;
	if (m_pNext) m_pNext->release();
}


void SimpleNode::autoRelease(DOMAutoReleasePool* pAutoReleasePool)
{
	if (pAutoReleasePool == NULL) pAutoReleasePool = m_pOwner->getAutoReleasePool();
	Node::autoRelease(pAutoReleasePool);
}


const XMLString& SimpleNode::getNodeName() const
{
	return NODE_NAME;
}


const XMLString& SimpleNode::getNodeValue() const
{
	return EMPTY_STRING;
}


void SimpleNode::setNodeValue(const XMLString& value)
{
	throw DOMException(DOMException::NO_DATA_ALLOWED_ERR);
}


Node* SimpleNode::getParentNode() const
{
	return m_pParent;
}


NodeList* SimpleNode::getChildNodes() const
{
	return new ChildNodeList(this);
}


Node* SimpleNode::getFirstChild() const
{
	return NULL;
}


Node* SimpleNode::getLastChild() const
{
	return NULL;
}


Node* SimpleNode::getPreviousSibling() const
{
	if (m_pParent)
	{
		register SimpleNode* pSibling = m_pParent->m_pFirstChild;
		while (pSibling)
		{
		    if (pSibling->m_pNext == this) return pSibling;
		    pSibling = pSibling->m_pNext;
		}
	}
	return NULL;
}


Node* SimpleNode::getNextSibling() const
{
	return m_pNext;
}


NamedNodeMap* SimpleNode::getAttributes() const
{
	return NULL;
}


Document* SimpleNode::getOwnerDocument() const
{
	return m_pOwner;
}


Node* SimpleNode::insertBefore(Node* newChild, Node* refChild)
{
	throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
}


Node* SimpleNode::replaceChild(Node* newChild, Node* oldChild)
{
	throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
}


Node* SimpleNode::removeChild(Node* oldChild)
{
	throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR);
}


Node* SimpleNode::appendChild(Node* newChild)
{
	throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
}


bool SimpleNode::hasChildNodes() const
{
	return false;
}


Node* SimpleNode::cloneNode(bool deep) const
{
	throw DOMException(DOMException::NOT_SUPPORTED_ERR);
}


void SimpleNode::normalize()
{
}


bool SimpleNode::isSupported(const XMLString& feature, const XMLString& version) const
{
	return DOMImplementation::getInstance()->hasFeature(feature, version);
}


const XMLString& SimpleNode::getNamespaceURI() const
{
	return EMPTY_STRING;
}


XMLString SimpleNode::getPrefix() const
{
	const XMLString& name = getNodeName();
	XMLString::size_type pos = name.find_first_of(':');
	if (pos != XMLString::npos)
		return name.substr(0, pos);
	else
		return EMPTY_STRING;
}


const XMLString& SimpleNode::getLocalName() const
{
	return EMPTY_STRING;
}


bool SimpleNode::hasAttributes() const
{
	return false;
}


void SimpleNode::addEventListener(const XMLString& type, EventListener* listener, bool useCapture)
{
	EventListenerEntry elEntry;
	elEntry.m_type       = type;
	elEntry.m_pListener  = listener;
	elEntry.m_useCapture = useCapture;
	if (!m_pEventListeners) m_pEventListeners = new EventListenerVec;
	m_pEventListeners->push_back(elEntry);
}


void SimpleNode::removeEventListener(const XMLString& type, EventListener* listener, bool useCapture)
{
	if (m_pEventListeners)
	{
		for (EventListenerVec::iterator it = m_pEventListeners->begin(); it != m_pEventListeners->end(); ++it)
		{
			if (it->m_type == type && it->m_pListener == listener && it->m_useCapture == useCapture)
			{
				m_pEventListeners->erase(it);
				break;
			}
		}
	}
}


bool SimpleNode::dispatchEvent(Event* evt)
{
	if (!mustDoEvents()) return true;

	if (evt->getType().empty()) throw EventException(EventException::UNSPECIFIED_EVENT_TYPE_ERR);

	evt->setTargetNP(this);
	evt->setCurrentPhaseNP(Event::CAPTURING_PHASE);

	if (m_pParent) m_pParent->captureEvent(evt);

	if (!evt->isStoppedNP() && m_pEventListeners)
	{
		evt->setCurrentPhaseNP(Event::AT_TARGET);
		evt->setCurrentTargetNP(this);
		EventListenerVec::size_type listenerCount = m_pEventListeners->size();
		set<EventListener*> listeners;
		for (EventListenerVec::iterator it = m_pEventListeners->begin(); it != m_pEventListeners->end();)
		{
			if (it->m_type == evt->getType())
			{
				try
				{
					if (listeners.find(it->m_pListener) == listeners.end())
					{
						it->m_pListener->handleEvent(evt);
						listeners.insert(it->m_pListener);
						if (m_pEventListeners->size() != listenerCount) // listeners could have been added/removed in handleEvent
						{
							it = m_pEventListeners->begin();
							listenerCount = m_pEventListeners->size();
							continue;
						}
					}
				}
				catch (...)
				{
				}
			}
			++it;
		}
	}
	if (!evt->isStoppedNP() && evt->getBubbles() && m_pParent)
	{
		evt->setCurrentPhaseNP(Event::BUBBLING_PHASE);
		m_pParent->bubbleEvent(evt);
	}

	return evt->isCanceledNP();
}


void SimpleNode::captureEvent(Event* evt)
{
	if (m_pParent) m_pParent->captureEvent(evt);
	
	if (!evt->isStoppedNP() && m_pEventListeners)
	{
		evt->setCurrentTargetNP(this);
		EventListenerVec::size_type listenerCount = m_pEventListeners->size();
		set<EventListener*> listeners;
		for (EventListenerVec::iterator it = m_pEventListeners->begin(); it != m_pEventListeners->end();)
		{
			if (it->m_type == evt->getType() && it->m_useCapture)
			{
				try
				{
					if (listeners.find(it->m_pListener) == listeners.end())
					{
						it->m_pListener->handleEvent(evt);
						listeners.insert(it->m_pListener);
						if (m_pEventListeners->size() != listenerCount) // listeners could have been added/removed in handleEvent
						{
							it = m_pEventListeners->begin();
							listenerCount = m_pEventListeners->size();
							continue;
						}
					}
				}
				catch (...)
				{
				}
			}
			++it;
		}
	}
}


void SimpleNode::bubbleEvent(Event* evt)
{
	evt->setCurrentTargetNP(this);
	if (m_pEventListeners)
	{
		EventListenerVec::size_type listenerCount = m_pEventListeners->size();
		set<EventListener*> listeners;
		for (EventListenerVec::iterator it = m_pEventListeners->begin(); it != m_pEventListeners->end();)
		{
			if (it->m_type == evt->getType() && !it->m_useCapture)
			{
				try
				{
					if (listeners.find(it->m_pListener) == listeners.end())
					{
						it->m_pListener->handleEvent(evt);
						listeners.insert(it->m_pListener);
						if (m_pEventListeners->size() != listenerCount) // listeners could have been added/removed in handleEvent
						{
							it = m_pEventListeners->begin();
							listenerCount = m_pEventListeners->size();
							continue;
						}
					}
				}
				catch (...)
				{
				}
			}
			++it;
		}
	}
	if (m_pParent && !evt->isStoppedNP()) m_pParent->bubbleEvent(evt);
}


void SimpleNode::dispatchSubtreeModified()
{
	MutationEvent* pEvent = new MutationEvent(MutationEvent::DOMSubtreeModified, this, true, false, NULL);
	dispatchEvent(pEvent);
	pEvent->release();
}


void SimpleNode::dispatchNodeInserted()
{
	MutationEvent* pEvent = new MutationEvent(MutationEvent::DOMNodeInserted, this, true, false, getParentNode());
	dispatchEvent(pEvent);
	pEvent->release();
}


void SimpleNode::dispatchNodeRemoved()
{
	MutationEvent* pEvent = new MutationEvent(MutationEvent::DOMNodeRemoved, this, true, false, getParentNode());
	dispatchEvent(pEvent);
	pEvent->release();
}


void SimpleNode::dispatchNodeRemovedFromDocument()
{
	MutationEvent* pEvent = new MutationEvent(MutationEvent::DOMNodeRemovedFromDocument, this, false, false, NULL);
	dispatchEvent(pEvent);
	pEvent->release();
}


void SimpleNode::dispatchNodeInsertedIntoDocument()
{
	MutationEvent* pEvent = new MutationEvent(MutationEvent::DOMNodeInsertedIntoDocument, this, false, false, NULL);
	dispatchEvent(pEvent);
	pEvent->release();
}


void SimpleNode::dispatchAttrModified(Attr* pAttr, MutationEvent::AttrChangeType changeType, const XMLString& prevValue, const XMLString& newValue)
{
	MutationEvent* pEvent = new MutationEvent(MutationEvent::DOMAttrModified, this, true, false, pAttr, prevValue, newValue, pAttr->getName(), changeType);
	dispatchEvent(pEvent);
	pEvent->release();
}


void SimpleNode::dispatchCharacterDataModified(const XMLString& prevValue, const XMLString& newValue)
{
	MutationEvent* pEvent = new MutationEvent(MutationEvent::DOMCharacterDataModified, this, true, false, NULL, prevValue, newValue, EMPTY_STRING, MutationEvent::MODIFICATION);
	dispatchEvent(pEvent);
	pEvent->release();
}


void SimpleNode::changeOwnerDocument(Document* pOwnerDocument)
{
	m_pOwner = pOwnerDocument;
}


//
// ContainerNode
//


ContainerNode::ContainerNode(Document* pOwnerDocument): SimpleNode(pOwnerDocument)
{
	m_pFirstChild = NULL;
}


ContainerNode::ContainerNode(const ContainerNode& node): SimpleNode(node)
{
	m_pFirstChild = NULL;
}


ContainerNode::~ContainerNode()
{
	SimpleNode* pChild = static_cast<SimpleNode*>(m_pFirstChild);
	while (pChild)
	{
		SimpleNode* pDelNode = pChild;
		pChild = pChild->m_pNext;
		pDelNode->m_pNext   = NULL;
		pDelNode->m_pParent = NULL;
		pDelNode->release();
	}
}


Node* ContainerNode::getFirstChild() const
{
	return m_pFirstChild;
}


Node* ContainerNode::getLastChild() const
{
	register SimpleNode* pChild = m_pFirstChild;
	if (pChild)
	{
		while (pChild->m_pNext) pChild = pChild->m_pNext;
		return pChild;
	}
	return NULL;
}


Node* ContainerNode::insertBefore(Node* newChild, Node* refChild)
{
	csl_check_ptr (newChild);

	if (static_cast<SimpleNode*>(newChild)->m_pOwner != m_pOwner && static_cast<SimpleNode*>(newChild)->m_pOwner != this)
		throw DOMException(DOMException::WRONG_DOCUMENT_ERR);

	if (refChild && static_cast<SimpleNode*>(refChild)->m_pParent != this)
		throw DOMException(DOMException::NOT_FOUND_ERR);

	if (newChild == refChild)
		return newChild;

	if (this == newChild)
		throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);

	SimpleNode* pFirst = NULL;
	SimpleNode* pLast  = NULL;

	if (newChild->getNodeType() == Node::DOCUMENT_FRAGMENT_NODE)
	{
		ContainerNode* pFrag = static_cast<ContainerNode*>(newChild);
		pFirst = pFrag->m_pFirstChild;
		pLast  = pFirst;
		if (pFirst)
		{
			while (pLast->m_pNext)
			{
				pLast->m_pParent = this;
				pLast = pLast->m_pNext;
			}
			pLast->m_pParent = this;
		}
		pFrag->m_pFirstChild = NULL;
	}
	else
	{
		newChild->duplicate();
		ContainerNode* pParent = static_cast<SimpleNode*>(newChild)->m_pParent;
		if (pParent) pParent->removeChild(newChild);
		pFirst = static_cast<SimpleNode*>(newChild);
		pLast  = pFirst;
		pFirst->m_pParent = this;
	}
	if (m_pFirstChild && pFirst)
	{
		SimpleNode* pCur = m_pFirstChild;
		if (pCur == refChild)
		{
			pLast->m_pNext = m_pFirstChild;
			m_pFirstChild  = pFirst;
		}
		else
		{
			while (pCur && pCur->m_pNext != refChild) pCur = pCur->m_pNext;
			if (pCur)
			{
				pLast->m_pNext = pCur->m_pNext;
				pCur->m_pNext = pFirst;
			}
			else
			{
				throw DOMException(DOMException::NOT_FOUND_ERR);
			}
		}
	}
	else
	{
		m_pFirstChild = pFirst;
	}
	if (mustDoEvents())
	{
		while (pFirst && pFirst != pLast->m_pNext)
		{
			pFirst->dispatchNodeInserted();
			pFirst->dispatchNodeInsertedIntoDocument();
			pFirst = pFirst->m_pNext;
		}
		dispatchSubtreeModified();
	}
	return newChild;
}


Node* ContainerNode::replaceChild(Node* newChild, Node* oldChild)
{
	csl_check_ptr (newChild);
	csl_check_ptr (oldChild);

	if (static_cast<SimpleNode*>(newChild)->m_pOwner != m_pOwner && static_cast<SimpleNode*>(newChild)->m_pOwner != this)
		throw DOMException(DOMException::WRONG_DOCUMENT_ERR);

	if (static_cast<SimpleNode*>(oldChild)->m_pParent != this)
		throw DOMException(DOMException::NOT_FOUND_ERR);

	if (newChild == oldChild)
		return newChild;

	if (this == newChild)
		throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);

	bool doEvents = mustDoEvents();

	if (newChild->getNodeType() == Node::DOCUMENT_FRAGMENT_NODE)
	{
		insertBefore(newChild, oldChild);
		removeChild(oldChild);
	}
	else
	{
		ContainerNode* pParent = static_cast<SimpleNode*>(newChild)->m_pParent;
		if (pParent) pParent->removeChild(newChild);

		if (oldChild == m_pFirstChild)
		{
			if (doEvents)
			{
				m_pFirstChild->dispatchNodeRemoved();
				m_pFirstChild->dispatchNodeRemovedFromDocument();
			}
			static_cast<SimpleNode*>(newChild)->m_pNext   = static_cast<SimpleNode*>(oldChild)->m_pNext;
			static_cast<SimpleNode*>(newChild)->m_pParent = this;
			m_pFirstChild->m_pNext   = NULL;
			m_pFirstChild->m_pParent = NULL;
			m_pFirstChild = static_cast<SimpleNode*>(newChild);
			if (doEvents)
			{
				static_cast<SimpleNode*>(newChild)->dispatchNodeInserted();
				static_cast<SimpleNode*>(newChild)->dispatchNodeInsertedIntoDocument();
			}
		}
		else
		{
			SimpleNode* pCur = m_pFirstChild;
			while (pCur && pCur->m_pNext != oldChild) pCur = pCur->m_pNext;
			if (pCur)
			{	
				csl_assert (pCur->m_pNext == oldChild);

				if (doEvents)
				{
					static_cast<SimpleNode*>(oldChild)->dispatchNodeRemoved();
					static_cast<SimpleNode*>(oldChild)->dispatchNodeRemovedFromDocument();
				}
				static_cast<SimpleNode*>(newChild)->m_pNext   = static_cast<SimpleNode*>(oldChild)->m_pNext;
				static_cast<SimpleNode*>(newChild)->m_pParent = this;
				static_cast<SimpleNode*>(oldChild)->m_pNext   = NULL;
				static_cast<SimpleNode*>(oldChild)->m_pParent = NULL;
				pCur->m_pNext = static_cast<SimpleNode*>(newChild);
				if (doEvents)
				{
					static_cast<SimpleNode*>(newChild)->dispatchNodeInserted();
					static_cast<SimpleNode*>(newChild)->dispatchNodeInsertedIntoDocument();
				}
			}
			else
			{
				throw DOMException(DOMException::NOT_FOUND_ERR);
			}
		}
		newChild->duplicate();
		oldChild->autoRelease();
	}
	if (doEvents) dispatchSubtreeModified();
	return oldChild;
}


Node* ContainerNode::removeChild(Node* oldChild)
{
	csl_check_ptr (oldChild);

	bool doEvents = mustDoEvents();

	if (oldChild == m_pFirstChild)
	{
		if (doEvents)
		{
			static_cast<SimpleNode*>(oldChild)->dispatchNodeRemoved();
			static_cast<SimpleNode*>(oldChild)->dispatchNodeRemovedFromDocument();
		}
		m_pFirstChild = m_pFirstChild->m_pNext;
		static_cast<SimpleNode*>(oldChild)->m_pNext   = NULL;
		static_cast<SimpleNode*>(oldChild)->m_pParent = NULL;
	}
	else
	{
		SimpleNode* pCur = m_pFirstChild;
		while (pCur && pCur->m_pNext != oldChild) pCur = pCur->m_pNext;
		if (pCur)
		{
			if (doEvents)
			{
				static_cast<SimpleNode*>(oldChild)->dispatchNodeRemoved();
				static_cast<SimpleNode*>(oldChild)->dispatchNodeRemovedFromDocument();
			}
			pCur->m_pNext = pCur->m_pNext->m_pNext;
			static_cast<SimpleNode*>(oldChild)->m_pNext   = NULL;
			static_cast<SimpleNode*>(oldChild)->m_pParent = NULL;
		}
		else
		{
			throw DOMException(DOMException::NOT_FOUND_ERR);
		}
	}
	oldChild->autoRelease();
	if (doEvents) dispatchSubtreeModified();
	return oldChild;
}


Node* ContainerNode::appendChild(Node* newChild)
{
	return insertBefore(newChild, NULL);
};


void ContainerNode::dispatchNodeRemovedFromDocument()
{
	SimpleNode::dispatchNodeRemovedFromDocument();
	Node* pChild = getFirstChild();
	while (pChild)
	{
		static_cast<SimpleNode*>(pChild)->dispatchNodeRemovedFromDocument();
		pChild = pChild->getNextSibling();
	}
}


void ContainerNode::dispatchNodeInsertedIntoDocument()
{
	SimpleNode::dispatchNodeInsertedIntoDocument();
	Node* pChild = getFirstChild();
	while (pChild)
	{
		static_cast<SimpleNode*>(pChild)->dispatchNodeInsertedIntoDocument();
		pChild = pChild->getNextSibling();
	}
}



bool ContainerNode::hasChildNodes() const
{
	return m_pFirstChild != NULL;
}


bool ContainerNode::hasAttributes() const
{
	return false;
}


void ContainerNode::changeOwnerDocument(Document* pOwnerDocument)
{
	SimpleNode::changeOwnerDocument(pOwnerDocument);
	SimpleNode* pChild = m_pFirstChild;
	while (pChild)
	{
		pChild->changeOwnerDocument(pOwnerDocument);
		pChild = pChild->m_pNext;
	}
}


//
// DocumentFragment
//


const XMLString DocumentFragment::NODE_NAME = CXMLUtils::AsciiToXML("#document-fragment");


DocumentFragment::DocumentFragment(Document* pOwner): ContainerNode(pOwner)
{
}


DocumentFragment::DocumentFragment(const DocumentFragment& frag): ContainerNode(frag)
{
}


DocumentFragment::~DocumentFragment()
{
}


const XMLString& DocumentFragment::getNodeName() const
{
	return NODE_NAME;
}


Node* DocumentFragment::cloneNode(bool deep) const
{
	DocumentFragment* pClone = new DocumentFragment(*this);
	if (deep)
	{
		Node* pCur = getFirstChild();
		while (pCur)
		{
			pClone->appendChild(pCur->cloneNode(deep))->release();
			pCur = pCur->getNextSibling();
		}
	}
	return pClone;
}


unsigned short DocumentFragment::getNodeType() const
{
	return Node::DOCUMENT_FRAGMENT_NODE;
}


//
// CharacterData
//


CharacterData::CharacterData(Document* pOwner, const XMLString& data): SimpleNode(pOwner)
{
	m_data = data;
}


CharacterData::CharacterData(const CharacterData& data): SimpleNode(data)
{
	m_data = data.m_data;
}


CharacterData::~CharacterData()
{
}


const XMLString& CharacterData::getData() const
{
	return m_data;
}


void CharacterData::setData(const XMLString& data)
{
	XMLString oldData = getData();
	m_data = data;
	if (mustDoEvents()) dispatchCharacterDataModified(oldData, m_data);
}


unsigned long CharacterData::getLength() const
{
	return m_data.length();
}


XMLString CharacterData::substringData(unsigned long offset, unsigned long count) const
{
	if (offset < 0 || offset >= m_data.length() || count < 0)
	{
		throw DOMException(DOMException::INDEX_SIZE_ERR);
	}
	return m_data.substr(offset, count);
}


void CharacterData::appendData(const XMLString& arg)
{
	XMLString oldData = m_data;
	m_data.append(arg);
	if (mustDoEvents()) dispatchCharacterDataModified(oldData, m_data);
}


void CharacterData::insertData(unsigned long offset, const XMLString& arg)
{
	if (offset < 0 || offset >= m_data.length())
	{
		throw DOMException(DOMException::INDEX_SIZE_ERR);
	}
	XMLString oldData = m_data;
	m_data.insert(offset, arg);
	if (mustDoEvents()) dispatchCharacterDataModified(oldData, m_data);
}


void CharacterData::deleteData(unsigned long offset, unsigned long count)
{
	if (offset < 0 || offset >= m_data.length() || count < 0)
	{
		throw DOMException(DOMException::INDEX_SIZE_ERR);
	}
	XMLString oldData = m_data;
	m_data.replace(offset, count, EMPTY_STRING);
	if (mustDoEvents()) dispatchCharacterDataModified(oldData, m_data);
}


void CharacterData::replaceData(unsigned long offset, unsigned long count, const XMLString& arg)
{
	if (offset < 0 || offset >= m_data.length() || count < 0)
	{
		throw DOMException(DOMException::INDEX_SIZE_ERR);
	}
	XMLString oldData = m_data;
	m_data.replace(offset, count, arg);
	if (mustDoEvents()) dispatchCharacterDataModified(oldData, m_data);
}


const XMLString& CharacterData::getNodeValue() const
{
	return m_data;
}


void CharacterData::setNodeValue(const XMLString& value)
{
	setData(value);
}


//
// Attr
//


Attr::Attr(Document* pOwnerDocument, Element* pOwnerElement, const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname, const XMLString& value): SimpleNode(pOwnerDocument)
{
	m_pParent = pOwnerElement;
	m_pName   = getOwnerDocument()->getNamePool()->Insert(namespaceURI, localName, qname);
	m_value   = value;
}


Attr::Attr(const Attr& attr): SimpleNode(attr)
{
	m_pParent = NULL;
	m_pName   = attr.m_pName;
	m_value   = attr.m_value;
}


Attr::~Attr()
{
}


const XMLString& Attr::getName() const
{
	return m_pName->m_qname;
}


bool Attr::getSpecified() const
{
	return true;
}


const XMLString& Attr::getValue() const
{
	return m_value;
}


Node* Attr::getParentNode() const
{
	return NULL;
}


Node* Attr::getPreviousSibling() const
{
	if (m_pParent)
	{
		register Attr* pSibling = static_cast<Element*>(m_pParent)->m_pFirstAttr;
		while (pSibling)
		{
		    if (pSibling->m_pNext == const_cast<Attr*>(this)) return pSibling;
		    pSibling = static_cast<Attr*>(pSibling->m_pNext);
		}
		return pSibling;
	}
	return NULL;
}


void Attr::setValue(const XMLString& value)
{
	XMLString oldData = m_value;
	m_value     = value;
	if (m_pParent && mustDoEvents()) m_pParent->dispatchAttrModified(this, MutationEvent::MODIFICATION, oldData, value);
}


const XMLString& Attr::getNodeName() const
{
	return getName();
}


const XMLString& Attr::getNodeValue() const
{
	return getValue();
}


void Attr::setNodeValue(const XMLString& value)
{
	setValue(value);
}


unsigned short Attr::getNodeType() const
{
	return ATTRIBUTE_NODE;
}


Node* Attr::cloneNode(bool deep) const
{
	return new Attr(*this);
}


const XMLString& Attr::getNamespaceURI() const
{
	return m_pName->m_namespaceURI;
}


const XMLString& Attr::getLocalName() const
{
	return m_pName->m_localName;
}


Element* Attr::getOwnerElement() const
{
	return static_cast<Element*>(m_pParent);
}


void Attr::changeOwnerDocument(Document* pOwnerDocument)
{
	SimpleNode::changeOwnerDocument(pOwnerDocument);
	m_pName = pOwnerDocument->getNamePool()->Insert(m_pName->m_namespaceURI, m_pName->m_localName, m_pName->m_qname);	
}


//
// UnspecAttr
//


UnspecAttr::UnspecAttr(Document* pOwnerDocument, Element* pOwnerElement, const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname, const XMLString& value): Attr(pOwnerDocument, pOwnerElement, namespaceURI, localName, qname, value)
{
}


UnspecAttr::UnspecAttr(const UnspecAttr& attr): Attr(attr)
{
}


bool UnspecAttr::getSpecified() const
{
	return false;
}


Node* UnspecAttr::cloneNode(bool deep) const
{
	return new UnspecAttr(*this);
}


//
// AttrMap
//


AttrMap::AttrMap(Element* pElem)
{
	csl_assert (pElem != NULL);

	m_pElem = pElem;
}


Node* AttrMap::getNamedItem(const XMLString& name) const
{
	return m_pElem->getAttributeNode(name);
}


Node* AttrMap::setNamedItem(Node* arg)
{
	csl_check_ptr (arg);

	return m_pElem->setAttributeNode(static_cast<Attr*>(arg));
}


Node* AttrMap::removeNamedItem(const XMLString& name)
{
	Attr* pAttr = m_pElem->getAttributeNode(name);
	if (pAttr)
		return m_pElem->removeAttributeNode(pAttr);
	else
		return NULL;
}


Node* AttrMap::item(unsigned long index) const
{
	SimpleNode* pAttr = m_pElem->m_pFirstAttr;
	while (index-- > 0 && pAttr) pAttr = static_cast<SimpleNode*>(pAttr->getNextSibling());
	return pAttr;
}


unsigned long AttrMap::getLength() const
{
	unsigned long len = 0;
	SimpleNode* pAttr = m_pElem->m_pFirstAttr;
	while (pAttr) 
	{
		pAttr = static_cast<SimpleNode*>(pAttr->getNextSibling());
		++len;
	}
	return len;
}


Node* AttrMap::getNamedItemNS(const XMLString& namespaceURI, const XMLString& localName) const
{
	return m_pElem->getAttributeNodeNS(namespaceURI, localName);
}


Node* AttrMap::setNamedItemNS(Node* arg)
{
	csl_check_ptr (arg);

	return m_pElem->setAttributeNodeNS(static_cast<Attr*>(arg));
}


Node* AttrMap::removeNamedItemNS(const XMLString& namespaceURI, const XMLString& localName)
{
	Attr* pAttr = m_pElem->getAttributeNodeNS(namespaceURI, localName);
	if (pAttr)
		return m_pElem->removeAttributeNode(pAttr);
	else
		return NULL;
}


void AttrMap::autoRelease(DOMAutoReleasePool* pAutoReleasePool)
{
	if (pAutoReleasePool == NULL) pAutoReleasePool = m_pElem->getOwnerDocument()->getAutoReleasePool();
	NamedNodeMap::autoRelease(pAutoReleasePool);
}


//
// Element
//


Element::Element(const Element& elem): ContainerNode(elem)
{
	m_pName      = elem.m_pName;
	m_pFirstAttr = NULL;

	Attr* pAttr = elem.m_pFirstAttr;
	while (pAttr)
	{
		Attr* pClonedAttr = static_cast<Attr*>(pAttr->cloneNode(false));
		setAttributeNode(pClonedAttr);
		pClonedAttr->release();
		pAttr = static_cast<Attr*>(pAttr->m_pNext);
	}
}


Element::Element(Document* pOwnerDocument, const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname):
         ContainerNode(pOwnerDocument)
{
	m_pName      = getOwnerDocument()->getNamePool()->Insert(namespaceURI, localName, qname);	
	m_pFirstAttr = NULL;
}


Element::~Element()
{
	if (m_pFirstAttr) m_pFirstAttr->release();
}


const XMLString& Element::getTagName() const
{
	return m_pName->m_qname;
}


const XMLString& Element::getAttribute(const XMLString& name) const
{
	Attr* pAttr = getAttributeNode(name);
	if (pAttr)
		return pAttr->getValue();
	else
		return EMPTY_STRING;
}


void Element::setAttribute(const XMLString& name, const XMLString& value)
{
	Attr* pNewAttr = getOwnerDocument()->createAttribute(name);
	pNewAttr->setValue(value);
	setAttributeNode(pNewAttr);
	pNewAttr->release();
}


void Element::removeAttribute(const XMLString& name)
{
	Attr* pAttr = getAttributeNode(name);
	if (pAttr) removeAttributeNode(pAttr);
}


Attr* Element::getAttributeNode(const XMLString& name) const
{
	Attr* pAttr = m_pFirstAttr;
	while (pAttr && pAttr->m_pName->m_qname != name) pAttr = static_cast<Attr*>(pAttr->m_pNext);
	return pAttr;
}


Attr* Element::setAttributeNode(Attr* newAttr)
{
	csl_check_ptr (newAttr);

	if (newAttr->getOwnerDocument() != getOwnerDocument())
		throw DOMException(DOMException::WRONG_DOCUMENT_ERR);

	if (newAttr->getOwnerElement() != NULL)
		throw DOMException(DOMException::INUSE_ATTRIBUTE_ERR);

	Attr* oldAttr = getAttributeNode(newAttr->getName());
	if (oldAttr) removeAttributeNode(oldAttr);

	Attr* pCur = m_pFirstAttr;
	if (pCur)
	{
		while (pCur->m_pNext) pCur = static_cast<Attr*>(pCur->m_pNext);
		pCur->m_pNext = newAttr;
	}
	else
	{
		m_pFirstAttr = newAttr;
	}
	newAttr->duplicate();
	newAttr->m_pParent = this;
	if (mustDoEvents()) dispatchAttrModified(newAttr, MutationEvent::ADDITION, EMPTY_STRING, newAttr->getValue());
	return oldAttr;
}


Attr* Element::removeAttributeNode(Attr* oldAttr)
{
	csl_check_ptr (oldAttr);

	if (mustDoEvents()) dispatchAttrModified(oldAttr, MutationEvent::REMOVAL, oldAttr->getValue(), EMPTY_STRING);

	if (oldAttr == m_pFirstAttr)
	{
		m_pFirstAttr = static_cast<Attr*>(m_pFirstAttr->m_pNext);
	}
	else
	{
		Attr* pCur = m_pFirstAttr;
		while (pCur->m_pNext != oldAttr) pCur = static_cast<Attr*>(pCur->m_pNext);
		if (pCur)
		{
			pCur->m_pNext = static_cast<Attr*>(pCur->m_pNext->m_pNext);
		}
		else
		{
			throw DOMException(DOMException::NOT_FOUND_ERR);
		}
	}
	oldAttr->m_pNext   = NULL;
	oldAttr->m_pParent = NULL;
	oldAttr->autoRelease();

	return oldAttr;
}


NodeList* Element::getElementsByTagName(const XMLString& name) const
{
	return new ElementByTagNameList(this, name);
}


NodeList* Element::getElementsByTagNameNS(const XMLString& namespaceURI, const XMLString& localName) const
{
	return new ElementByTagNameListNS(this, namespaceURI, localName);
}


void Element::normalize()
{
	Node* pCur = getFirstChild();
	while (pCur)
	{
		if (pCur->getNodeType() == Node::ELEMENT_NODE)
		{
			pCur->normalize();
		}
		else if (pCur->getNodeType() == Node::TEXT_NODE)
		{
			Node* pNext = pCur->getNextSibling();
			while (pNext && pNext->getNodeType() == Node::TEXT_NODE)
			{
				static_cast<Text*>(pCur)->appendData(pNext->getNodeValue());
				removeChild(pNext)->release();
				pNext = pCur->getNextSibling();
			}
		}
		pCur = pCur->getNextSibling();
	}
}


const XMLString& Element::getNodeName() const
{
	return getTagName();
}


NamedNodeMap* Element::getAttributes() const
{
	return new AttrMap(const_cast<Element*>(this));
}


Node* Element::cloneNode(bool deep) const
{
	Element* pClone = new Element(*this);
	if (deep)
	{
		Node* pNode = getFirstChild();
		while (pNode)
		{
			pClone->appendChild(pNode->cloneNode(true))->release();
			pNode = pNode->getNextSibling();
		}
	}
	return pClone;
}


unsigned short Element::getNodeType() const
{
	return Node::ELEMENT_NODE;
}


const XMLString& Element::getAttributeNS(const XMLString& namespaceURI, const XMLString& localName) const
{
	Attr* pAttr = getAttributeNodeNS(namespaceURI, localName);
	if (pAttr)
		return pAttr->getValue();
	else
		return EMPTY_STRING;
}


void Element::setAttributeNS(const XMLString& namespaceURI, const XMLString& qualifiedName, const XMLString& value)
{
	Attr* pNewAttr = m_pOwner->createAttributeNS(namespaceURI, qualifiedName);
	pNewAttr->setValue(value);
	setAttributeNodeNS(pNewAttr);
	pNewAttr->release();
}


void Element::removeAttributeNS(const XMLString& namespaceURI, const XMLString& localName)
{
	Attr* pAttr = getAttributeNodeNS(namespaceURI, localName);
	if (pAttr) removeAttributeNode(pAttr)->release();
}


Attr* Element::getAttributeNodeNS(const XMLString& namespaceURI, const XMLString& localName) const
{
	Attr* pAttr = m_pFirstAttr;
	while (pAttr && (pAttr->m_pName->m_namespaceURI != namespaceURI || pAttr->m_pName->m_localName != localName)) pAttr = static_cast<Attr*>(pAttr->m_pNext);
	return pAttr;
}


Attr* Element::setAttributeNodeNS(Attr* newAttr)
{
	csl_check_ptr (newAttr);

	if (newAttr->getOwnerDocument() != getOwnerDocument())
		throw DOMException(DOMException::WRONG_DOCUMENT_ERR);

	if (newAttr->getOwnerElement() != NULL)
		throw DOMException(DOMException::INUSE_ATTRIBUTE_ERR);

	Attr* oldAttr = getAttributeNodeNS(newAttr->getNamespaceURI(), newAttr->getLocalName());
	if (oldAttr) removeAttributeNode(oldAttr);

	Attr* pCur = m_pFirstAttr;
	if (pCur)
	{
		while (pCur->m_pNext) pCur = static_cast<Attr*>(pCur->m_pNext);
		pCur->m_pNext = newAttr;
	}
	else
	{
		m_pFirstAttr = newAttr;
	}
	newAttr->m_pParent = this;
	newAttr->duplicate();
	if (mustDoEvents()) dispatchAttrModified(newAttr, MutationEvent::ADDITION, EMPTY_STRING, newAttr->getValue());
	return oldAttr;
}


bool Element::hasAttribute(const XMLString& name) const
{
	return getAttributeNode(name) != NULL;
}


bool Element::hasAttributeNS(const XMLString& namespaceURI, const XMLString& localName) const
{
	return getAttributeNodeNS(namespaceURI, localName) != NULL;
}


const XMLString& Element::getNamespaceURI() const
{
	return m_pName->m_namespaceURI;
}


const XMLString& Element::getLocalName() const
{
	return m_pName->m_localName;
}


bool Element::hasAttributes() const
{
	return m_pFirstAttr != NULL;
}


void Element::dispatchNodeRemovedFromDocument()
{
	ContainerNode::dispatchNodeRemovedFromDocument();
	Attr* pAttr = m_pFirstAttr;
	while (pAttr)
	{
		pAttr->dispatchNodeRemovedFromDocument();
		pAttr = static_cast<Attr*>(pAttr->m_pNext);
	}
}


void Element::dispatchNodeInsertedIntoDocument()
{
	ContainerNode::dispatchNodeInsertedIntoDocument();
	Attr* pAttr = m_pFirstAttr;
	while (pAttr)
	{
		pAttr->dispatchNodeInsertedIntoDocument();
		pAttr = static_cast<Attr*>(pAttr->m_pNext);
	}
}


void Element::changeOwnerDocument(Document* pOwnerDocument)
{
	ContainerNode::changeOwnerDocument(pOwnerDocument);
	SimpleNode* pAttr = m_pFirstAttr;
	while (pAttr)
	{
		pAttr->changeOwnerDocument(pOwnerDocument);
		pAttr = pAttr->m_pNext;
	}
	m_pName = pOwnerDocument->getNamePool()->Insert(m_pName->m_namespaceURI, m_pName->m_localName, m_pName->m_qname);	
}


//
// Text
//


const XMLString Text::NODE_NAME = CXMLUtils::AsciiToXML("#text");


Text::Text(Document* pOwnerDocument, const XMLString& data): CharacterData(pOwnerDocument, data)
{
}


Text::Text(const Text& text): CharacterData(text)
{
}


Text::~Text()
{
}


Text* Text::splitText(unsigned long offset)
{
	Node* pParent = getParentNode();
	int count = getLength() - offset;
	Text* pNew = getOwnerDocument()->createTextNode(substringData(offset, count));
	deleteData(offset, count);
	if (pParent)
	{
		pParent->insertBefore(pNew, getNextSibling());
	}
	return pNew;
}


const XMLString& Text::getNodeName() const
{
	return NODE_NAME;
}


Node* Text::cloneNode(bool deep) const
{
	return new Text(*this);
}


unsigned short Text::getNodeType() const
{
	return Node::TEXT_NODE;
}


//
// Comment
//


const XMLString Comment::NODE_NAME = CXMLUtils::AsciiToXML("#comment");


Comment::Comment(Document* pOwner, const XMLString& data): CharacterData(pOwner, data)
{
}


Comment::Comment(const Comment& comment): CharacterData(comment)
{
}


Comment::~Comment()
{
}


const XMLString& Comment::getNodeName() const
{
	return NODE_NAME;
}


Node* Comment::cloneNode(bool deep) const
{
	return new Comment(*this);
}


unsigned short Comment::getNodeType() const
{
	return Node::COMMENT_NODE;
}


//
// CDATASection
//


const XMLString CDATASection::NODE_NAME = CXMLUtils::AsciiToXML("#cdata-section");


CDATASection::CDATASection(Document* pOwner, const XMLString& data): Text(pOwner, data)
{
}


CDATASection::CDATASection(const CDATASection& sec): Text(sec)
{
}


CDATASection::~CDATASection()
{
}


const XMLString& CDATASection::getNodeName() const
{
	return NODE_NAME;
}


Text* CDATASection::splitText(unsigned long offset)
{
	Node* pParent = getParentNode();
	int count = getLength() - offset;
	CDATASection* pNew = getOwnerDocument()->createCDATASection(substringData(offset, count));
	deleteData(offset, count);
	if (pParent)
	{
		pParent->insertBefore(pNew, getNextSibling());
	}
	return pNew;
}


Node* CDATASection::cloneNode(bool deep) const
{
	return new CDATASection(*this);
}


unsigned short CDATASection::getNodeType() const
{
	return Node::CDATA_SECTION_NODE;
}


//
// Notation
//


Notation::Notation(Document* pOwner, const XMLString& name, const XMLString& publicId, const XMLString& systemId): SimpleNode(pOwner)
{
	m_name     = name;
	m_publicId = publicId;
	m_systemId = systemId;
}


Notation::Notation(const Notation& aNotation): SimpleNode(aNotation)
{
	m_name     = aNotation.m_name;
	m_publicId = aNotation.m_publicId;
	m_systemId = aNotation.m_systemId;
}


Notation::~Notation()
{
}


const XMLString& Notation::getPublicId() const
{
	return m_publicId;
}


const XMLString& Notation::getSystemId() const
{
	return m_systemId;
}


const XMLString& Notation::getNodeName() const
{
	return m_name;
}


unsigned short Notation::getNodeType() const
{
	return Node::NOTATION_NODE;
}


//
// Entity
//


const XMLString Entity::NODE_NAME = CXMLUtils::AsciiToXML("#entity");


Entity::Entity(Document* pOwner, const XMLString& name, const XMLString& publicId, const XMLString& systemId): ContainerNode(pOwner)
{
	m_name     = name;
	m_publicId = publicId;
	m_systemId = systemId;
}


Entity::Entity(const Entity& anEntity): ContainerNode(anEntity)
{
	m_name     = anEntity.m_name;
	m_publicId = anEntity.m_publicId;
	m_systemId = anEntity.m_systemId;
}


Entity::~Entity()
{
}


const XMLString& Entity::getPublicId() const
{
	return m_publicId;
}


const XMLString& Entity::getSystemId() const
{
	return m_systemId;
}


const XMLString& Entity::getNodeName() const
{
	return NODE_NAME;
}


const XMLString& Entity::getNotationName() const
{
	return m_name;
}


unsigned short Entity::getNodeType() const
{
	return Node::ENTITY_NODE;
}


//
// EntityReference
//


EntityReference::EntityReference(Document* pOwner, const XMLString& name): SimpleNode(pOwner)
{
	m_name = name;
}


EntityReference::EntityReference(const EntityReference& ref): SimpleNode(ref)
{
	m_name = ref.m_name;
}


EntityReference::~EntityReference()
{
}


const XMLString& EntityReference::getNodeName() const
{
	return m_name;
}


unsigned short EntityReference::getNodeType() const
{
	return Node::ENTITY_REFERENCE_NODE;
}


//
// ProcessingInstruction
//


ProcessingInstruction::ProcessingInstruction(Document* pOwner, const XMLString& target, const XMLString& data): SimpleNode(pOwner)
{
	m_target = target;
	m_data   = data;
}


ProcessingInstruction::ProcessingInstruction(const ProcessingInstruction& pi): SimpleNode(pi)
{
	m_target = pi.m_target;
	m_data   = pi.m_data;
}


ProcessingInstruction::~ProcessingInstruction()
{
}


const XMLString& ProcessingInstruction::getTarget() const
{
	return m_target;
}


const XMLString& ProcessingInstruction::getData() const
{
	return m_data;
}


void ProcessingInstruction::setData(const XMLString& data)
{
	m_data = data;
}


const XMLString& ProcessingInstruction::getNodeName() const
{
	return m_target;
}


const XMLString& ProcessingInstruction::getNodeValue() const
{
	return m_data;
}


void ProcessingInstruction::setNodeValue(const XMLString& data)
{
	setData(data);
}


Node* ProcessingInstruction::cloneNode(bool deep) const
{
	return new ProcessingInstruction(*this);
}


unsigned short ProcessingInstruction::getNodeType() const
{
	return Node::PROCESSING_INSTRUCTION_NODE;
}


//
// DeclMap
//


DeclMap::DeclMap(Document* pDoc, unsigned short type)
{
	m_pDoc = pDoc;
	m_type = type;
}


Node* DeclMap::getNamedItem(const XMLString& name) const
{
	Node* pCur = m_pDoc->getFirstChild();
	while (pCur)
	{
		if (pCur->getNodeType() == m_type && pCur->getNodeName() == name) return pCur;
		pCur = pCur->getNextSibling();
	}
	return pCur;
}


Node* DeclMap::setNamedItem(Node* arg)
{
	throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR);
}


Node* DeclMap::removeNamedItem(const XMLString& name)
{
	throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR);
}


Node* DeclMap::item(unsigned long index) const
{
	unsigned long n = 0;
	Node* pCur = m_pDoc->getFirstChild();
	while (pCur)
	{
		if (pCur->getNodeType() == m_type)
		{
			if (n == index) return pCur;
			n++;
		}
		pCur = pCur->getNextSibling();
	}
	return pCur;
}


unsigned long DeclMap::getLength() const
{
	unsigned long n = 0;
	Node* pCur = m_pDoc->getFirstChild();
	while (pCur)
	{
		if (pCur->getNodeType() == m_type) n++;
		pCur = pCur->getNextSibling();
	}
	return n;
}


Node* DeclMap::getNamedItemNS(const XMLString& namespaceURI, const XMLString& localName) const
{
	Node* pCur = m_pDoc->getFirstChild();
	while (pCur)
	{
		if (pCur->getNodeType() == m_type && pCur->getNamespaceURI() == namespaceURI && pCur->getLocalName() == localName) return pCur;
		pCur = pCur->getNextSibling();
	}
	return pCur;
}


Node* DeclMap::setNamedItemNS(Node* arg)
{
	throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR);
}


Node* DeclMap::removeNamedItemNS(const XMLString& namespaceURI, const XMLString& localName)
{
	throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR);
}


void DeclMap::autoRelease(DOMAutoReleasePool* pAutoReleasePool)
{
	if (pAutoReleasePool == NULL) pAutoReleasePool = m_pDoc->getAutoReleasePool();
	NamedNodeMap::autoRelease(pAutoReleasePool);
}


//
// DocumentType
//


DocumentType::DocumentType(Document* pOwner, const XMLString& name, const XMLString& publicId, const XMLString& systemId): SimpleNode(pOwner)
{
	m_name     = name;
	m_publicId = publicId;
	m_systemId = systemId;
}


DocumentType::~DocumentType()
{
}


const XMLString& DocumentType::getName() const
{
	return m_name;
}


NamedNodeMap* DocumentType::getEntities() const
{
	return new DeclMap(getOwnerDocument(), Node::ENTITY_NODE);
}


NamedNodeMap* DocumentType::getNotations() const
{
	return new DeclMap(getOwnerDocument(), Node::NOTATION_NODE);
}


const XMLString& DocumentType::getNodeName() const
{
	return getName();
}


unsigned short DocumentType::getNodeType() const
{
	return Node::DOCUMENT_TYPE_NODE;
}


const XMLString& DocumentType::getPublicId() const
{
	return m_publicId;
}


const XMLString& DocumentType::getSystemId() const
{
	return m_systemId;
}


const XMLString& DocumentType::getInternalSubset() const
{
	static XMLString emptyString;
	return emptyString;
}


//
// Document
//


const XMLString Document::NODE_NAME = CXMLUtils::AsciiToXML("#document");


Document::Document(DocumentType* pDocType, XMLNamePool* pNamePool): ContainerNode(NULL)
{
	if (pNamePool == NULL)
	{
		m_pNamePool = new XMLNamePool();
	}
	else
	{
		m_pNamePool = pNamePool;
		m_pNamePool->Duplicate();
	}
	m_eventSuspendLevel = 0;
	m_pDocType = pDocType;
	if (m_pDocType) m_pDocType->changeOwnerDocument(this);
}


Document::~Document()
{
	if (m_pDocType) m_pDocType->release();
	m_pNamePool->Release();
}


DocumentType* Document::getDoctype() const
{
	return m_pDocType;
}


DOMImplementation* Document::getImplementation() const
{
	return DOMImplementation::getInstance();
}


Element* Document::getDocumentElement() const
{
	// There may be DTD nodes before the document element
	Node* pCur = getFirstChild();
	while (pCur)
	{
		if (pCur->getNodeType() == Node::ELEMENT_NODE) return static_cast<Element*>(pCur);
		pCur = pCur->getNextSibling();
	}
	return NULL;
}


Element* Document::createElement(const XMLString& tagName) const
{
	return new Element(const_cast<Document*>(this), EMPTY_STRING, EMPTY_STRING, tagName); 
}


DocumentFragment* Document::createDocumentFragment() const
{
	return new DocumentFragment(const_cast<Document*>(this));
}


Text* Document::createTextNode(const XMLString& data) const
{
	return new Text(const_cast<Document*>(this), data);
}


Comment* Document::createComment(const XMLString& data) const
{
	return new Comment(const_cast<Document*>(this), data);
}


CDATASection* Document::createCDATASection(const XMLString& data) const
{
	return new CDATASection(const_cast<Document*>(this), data);
}


ProcessingInstruction* Document::createProcessingInstruction(const XMLString& target, const XMLString& data) const
{
	return new ProcessingInstruction(const_cast<Document*>(this), target, data);
}


Attr* Document::createAttribute(const XMLString& name) const
{
	return new Attr(const_cast<Document*>(this), NULL, EMPTY_STRING, EMPTY_STRING, name, EMPTY_STRING);
}


EntityReference* Document::createEntityReference(const XMLString& name) const
{
	return new EntityReference(const_cast<Document*>(this), name);
}


NodeList* Document::getElementsByTagName(const XMLString& name) const
{
	return new ElementByTagNameList(const_cast<Document*>(this), name);	
}


const XMLString& Document::getNodeName() const
{
	return NODE_NAME;
}


unsigned short Document::getNodeType() const
{
	return Node::DOCUMENT_NODE;
}


Node* Document::importNode(Node* importedNode, bool deep)
{
	Node* pClone = importedNode->cloneNode(deep);
	static_cast<SimpleNode*>(pClone)->changeOwnerDocument(this);
	return pClone;
}


Element* Document::createElementNS(const XMLString& namespaceURI, const XMLString& qualifiedName) const
{
	return new Element(const_cast<Document*>(this), namespaceURI, DOMImplementation::localFromQualified(qualifiedName), qualifiedName);
}


Attr* Document::createAttributeNS(const XMLString& namespaceURI, const XMLString& qualifiedName) const
{
	return new Attr(const_cast<Document*>(this), NULL, namespaceURI, DOMImplementation::localFromQualified(qualifiedName), qualifiedName, EMPTY_STRING);
}


NodeList* Document::getElementsByTagNameNS(const XMLString& namespaceURI, const XMLString& localName) const
{
	return new ElementByTagNameListNS(const_cast<Document*>(this), namespaceURI, localName);	
}


Element* Document::getElementById(const XMLString& elementId) const
{
	return NULL;
}


Event* Document::createEvent(const XMLString& eventType) const
{
	if (eventType == MutationEvent::DOMSubtreeModified          ||
	    eventType == MutationEvent::DOMNodeInserted             ||
		eventType == MutationEvent::DOMNodeRemoved              ||
		eventType == MutationEvent::DOMNodeRemovedFromDocument  ||
		eventType == MutationEvent::DOMNodeInsertedIntoDocument ||
		eventType == MutationEvent::DOMAttrModified             ||
		eventType == MutationEvent::DOMCharacterDataModified)
	{
		return new MutationEvent(eventType);
	}
	else
	{
		throw DOMException(DOMException::NOT_SUPPORTED_ERR);
	}
}


bool Document::dispatchEvent(Event* evt)
{
	if (getEventSuspendLevel())
		return true;
	else
		return ContainerNode::dispatchEvent(evt);
}


XMLNamePool* Document::getNamePool()
{
	return m_pNamePool;
}


DOMAutoReleasePool* Document::getAutoReleasePool()
{
	return &m_autoReleasePool;
}


void Document::collectGarbage()
{
	m_autoReleasePool.release();
}


void Document::suspendEvents()
{
	++m_eventSuspendLevel;
}


void Document::resumeEvents()
{
	csl_precondition (m_eventSuspendLevel > 0);

	--m_eventSuspendLevel;
}


//
// NamedNodeMap
//


NamedNodeMap::~NamedNodeMap()
{
}


//
// NodeList
//


NodeList::~NodeList()
{
}


//
// CDOMImplementation
//


bool DOMImplementation::hasFeature(const XMLString& feature, const XMLString& version) const
{
	return (feature == CXMLUtils::AsciiToXML("XML")  || 
	        feature == CXMLUtils::AsciiToXML("xml")  ||
		    feature == CXMLUtils::AsciiToXML("Xml")) && 
		    version == CXMLUtils::AsciiToXML("1.0")  ||
			feature == CXMLUtils::AsciiToXML("Core") &&
			version == CXMLUtils::AsciiToXML("2.0")  ||
			feature == CXMLUtils::AsciiToXML("Events") &&
			version == CXMLUtils::AsciiToXML("2.0")  ||
			feature == CXMLUtils::AsciiToXML("MutationEvents") &&
			version == CXMLUtils::AsciiToXML("2.0");
}

		
DOMImplementation* DOMImplementation::getInstance()
{
	if (!m_pInstance) m_pInstance = new DOMImplementation();
	return m_pInstance;
}


DocumentType* DOMImplementation::createDocumentType(const XMLString& qualifiedName, const XMLString& publicId, const XMLString& systemId) const
{
	return new DocumentType(NULL, qualifiedName, publicId, systemId);
}


Document* DOMImplementation::createDocument(const XMLString& namespaceURI, const XMLString& qualifiedName, DocumentType* doctype) const
{
	Document* pDoc = new Document(doctype);
	if (namespaceURI.empty())
		pDoc->appendChild(pDoc->createElement(qualifiedName))->release();
	else
		pDoc->appendChild(pDoc->createElementNS(namespaceURI, qualifiedName))->release();
	return pDoc;
}


Document* DOMImplementation::createDocumentNP(const XMLString& namespaceURI, const XMLString& qualifiedName, DocumentType* doctype, XMLNamePool* pNamePool) const
{
	Document* pDoc = new Document(doctype, pNamePool);
	if (namespaceURI.empty())
		pDoc->appendChild(pDoc->createElement(qualifiedName))->release();
	else
		pDoc->appendChild(pDoc->createElementNS(namespaceURI, qualifiedName))->release();
	return pDoc;
}


DOMImplementation::~DOMImplementation()
{
	m_pInstance = NULL;
}


XMLString DOMImplementation::localFromQualified(const XMLString& qname)
{
	XMLString::size_type pos = qname.find_first_of(':');
	if (pos != XMLString::npos) 
	{
		return XMLString(qname, pos + 1, qname.size() - pos - 1);
	}
	else
	{
		return qname;
	}
}


DOMImplementation* DOMImplementation::m_pInstance = NULL;


//
// DOMException
//


DOMException::DOMException(int code): CXMLException(code < DOMException::_MAX_CODE_ERR ? code + CXMLException::EDOMException : code)
{
}


DOMException::DOMException(const DOMException& exc): CXMLException(exc)
{
}


DOMException::~DOMException() throw()
{
}


unsigned short DOMException::getCode() const
{
	return GetResult() - CXMLException::EDOMException;
}


//
// EventListener
//


EventListener::~EventListener()
{
}


//
// EventTarget
//


EventTarget::~EventTarget()
{
}


//
// DocumentEvent
//


DocumentEvent::~DocumentEvent()
{
}


XML_END
