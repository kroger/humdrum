//
// Document.h
//
// $Id: //depot/XML/Main/include/DOM/Document.h#15 $
//
// Document Object Model (Core) Level 1
//
// Interface Document
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


#ifndef DOM_Document_H_INCLUDED
#define DOM_Document_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef DOM_NodeImp_H_INCLUDED
#include "DOM/NodeImp.h"
#endif
#ifndef DOM_DocumentEvent_H_INCLUDED
#include "DOM/DocumentEvent.h"
#endif
#ifndef DOM_DOMAutoReleasePool_H_INCLUDED
#include "DOM/DOMAutoReleasePool.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif
#ifndef XMLNamePool_H_INCLUDED
#include "XMLNamePool.h"
#endif


XML_BEGIN


class DocumentType;
class DOMImplementation;
class Element;
class DocumentFragment;
class Text;
class Comment;
class CDATASection;
class ProcessingInstruction;
class Attr;
class EntityReference;
class NodeList;


class XML_API Document: public ContainerNode, public DocumentEvent
	//: The Document interface represents the entire XML document.
	//+ Conceptually, it is the root of the document tree, and provides
	//+ the primary access to the document's data.
	//+ The Document interface also contains the factory methods to create
	//. elements, text nodes, comments and the like.
{
public:
	virtual DocumentType* getDoctype() const;
		//; Returns the Document Type Declaration associated with this document.

	virtual DOMImplementation* getImplementation() const;
		//; Returns the DOMImplementation object that handles this document.

	virtual Element* getDocumentElement() const;
		//; Returns the root element of the document.

	virtual Element* createElement(const XMLString& tagName) const;
		//; Creates an element of the type specified.

	virtual DocumentFragment* createDocumentFragment() const;
		//; Creates an empty DocumentFragment object.

	virtual Text* createTextNode(const XMLString& data) const;
		//; Creates a text node given the specified string.

	virtual Comment* createComment(const XMLString& data) const;
		//; Creates a comment node given the specified string.

	virtual CDATASection* createCDATASection(const XMLString& data) const;
		//; Creates a CDATASection node whose value is the specified string.

	virtual ProcessingInstruction* createProcessingInstruction(const XMLString& target, const XMLString& data) const;
		//; Creates a ProcessingInstruction node given the specified target and data strings.

	virtual Attr* createAttribute(const XMLString& name) const;	
		//: Creates an Attr of the given name. Note that the Attr instance can then be set
		//. on an Element using the setAttribute method.

	virtual EntityReference* createEntityReference(const XMLString& name) const;
		//; Creates an EntityReference object.

	virtual NodeList* getElementsByTagName(const XMLString& name) const;
		//: Returns a NodeList of all Elements with a given tag name in the order
		//+ in which they would be encountered in a preorder traversal of the
		//+ document tree.
		//+ When you are done with the NodeList, send it the release() message
		//. to get rid of it.

	// DOM Level 2
	virtual Node* importNode(Node* importedNode, bool deep);
		//: Imports a node from another document to this document. The 
		//+ returned node has no parent; (parentNode is null). The source
		//+ node is not altered or removed from the original document; 
		//+ this method creates a new copy of the source node.
		//+ For all nodes, importing a node creates a node object owned by 
		//+ the importing document, with attribute values identical to the
		//+ source node's nodeName and nodeType, plus the attributes related 
		//+ to namespaces (prefix, localName, and namespaceURI). As in the 
		//. cloneNode operation on a Node, the source node is not altered.

	virtual Element* createElementNS(const XMLString& namespaceURI, const XMLString& qualifiedName) const;
		//; Creates an element of the given qualified name and namespace URI.

	virtual Attr* createAttributeNS(const XMLString& namespaceURI, const XMLString& qualifiedName) const;
		//; Creates an attribute of the given qualified name and namespace URI.

	virtual NodeList* getElementsByTagNameNS(const XMLString& namespaceURI, const XMLString& localName) const;
		//: Returns a NodeList of all the Elements with a given local name and 
		//+ namespace URI in the order in which they are encountered in a 
		//. preorder traversal of the Document tree.

	virtual Element* getElementById(const XMLString& elementId) const;
		//: Returns the Element whose ID is given by elementId. If no such 
		//+ element exists, returns null. Behavior is not defined if more
		//+ than one element has this ID. 
		//+
		//+ Note: The DOM implementation must have information that says 
		//+ which attributes are of type ID. Attributes with the name "ID"
		//+ are not of type ID unless so defined. Implementations that do 
		//+ not know whether attributes are of type ID or not are expected to
		//. return NULL. This implementation therefor returns NULL.

	// DocumentEvent
	Event* createEvent(const XMLString& eventType) const;

	// Node
	virtual const XMLString& getNodeName() const;
	virtual unsigned short getNodeType() const;

	// EventTarget
	bool dispatchEvent(Event* evt);

	// IMPLEMENTATION
	Document(DocumentType* pDocType = NULL, XMLNamePool* pNamePool = NULL);
		//: Creates a new document. If pNamePool == NULL, the document
		//+ creates its own name pool, otherwise it uses the given name pool.
		//+ Sharing a name pool makes sense for documents containing instances
		//. of the same schema, thus reducing memory usage.

	XMLNamePool* getNamePool();
		//; Returns a pointer to the documents Name Pool.

	DOMAutoReleasePool* getAutoReleasePool();
		//; Returns a pointer to the documents Auto Release Pool.

	void collectGarbage();
		//; Releases all objects in the Auto Release Pool.

	void suspendEvents();
		//; Suspends all events until resumeEvents() is called.

	void resumeEvents();
		//; Resumes all events suspended with suspendEvent();

	inline int getEventSuspendLevel() const;
		//: Returns the event suspend level. If the level is > 0,
		//. events are suspended.

protected:
	virtual ~Document();

private:
	static const XMLString NODE_NAME;

	DocumentType*      m_pDocType;
	XMLNamePool*       m_pNamePool;
	DOMAutoReleasePool m_autoReleasePool;
	int                m_eventSuspendLevel;
};


//
// inline's
//


#if defined(_MSC_VER)
#pragma warning(disable: 4273)
#endif


int Document::getEventSuspendLevel() const
{
	return m_eventSuspendLevel;
}


#if defined(_MSC_VER)
#pragma warning(default: 4273)
#endif


XML_END


#endif // DOM_Document_H_INCLUDED
