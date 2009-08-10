//
// DOMBuilder.cpp
//
// $Id: //depot/XML/Main/src/DOMBuilder.cpp#12 $
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


#include "DOM/DOMBuilder.h"
#include "DOM/Document.h"
#include "DOM/NamedNodeMap.h"
#include "DOM/CharacterData.h"
#include "DOM/Comment.h"
#include "DOM/CDATASection.h"
#include "DOM/Element.h"
#include "DOM/Attr.h"
#include "DOM/Entity.h"
#include "DOM/EntityReference.h"
#include "DOM/Notation.h"
#include "DOM/ProcessingInstruction.h"
#include "DOM/Text.h"
#include "DOM/DocumentType.h"
#include "DOM/DOMException.h"
#include "SAX2Parser.h"
#include "SAX/Attributes.h"
#include "SAX/AttributesImpl.h"
#include "XMLUtils.h"
#include <exception>
#include <limits.h>


XML_BEGIN


const XMLString DOMBuilder::EMPTY_STRING;
const XMLString DOMBuilder::FEATURE_NAMESPACES = CXMLUtils::AsciiToXML("namespaces");


#define AppendNode(node)				\
{										\
	if (m_pPrev && m_pPrev != m_pParent)\
	{									\
		m_pPrev->m_pNext = node;		\
		node->m_pParent = m_pParent;	\
		node->duplicate();				\
	}									\
	else								\
	{									\
		m_pParent->appendChild(node);	\
	}									\
}


DOMBuilder::DOMBuilder(XMLNamePool* pNamePool)
{
	m_pNamePool           = pNamePool;
	m_pDoc                = NULL;
	m_pParent             = NULL;
	m_pPrev               = NULL;
	m_inCDATA             = false;
	m_namespaces          = true;
	m_whiteSpaceInContent = true;
	m_parser.SetContentHandler(this);
	m_parser.SetDTDHandler(this);
	m_parser.SetProperty(CXMLUtils::AsciiToXML("http://xml.org/sax/handlers/LexicalHandler"), static_cast<CLexicalHandler*>(this));
	m_parser.SetFeature(CXMLUtils::AsciiToXML("http://xml.org/sax/features/namespaces"), true);
	m_parser.SetFeature(CXMLUtils::AsciiToXML("http://xml.org/sax/features/namespace-prefixes"), true);

	if (m_pNamePool) m_pNamePool->Duplicate();
}


DOMBuilder::DOMBuilder(const XMLString& encoding, XMLNamePool* pNamePool): m_parser(encoding)
{
	m_pNamePool           = pNamePool;
	m_pDoc                = NULL;
	m_pParent             = NULL;
	m_pPrev               = NULL;
	m_inCDATA             = false;
	m_namespaces          = true;
	m_whiteSpaceInContent = true;
	m_parser.SetContentHandler(this);
	m_parser.SetDTDHandler(this);
	m_parser.SetProperty(CXMLUtils::AsciiToXML("http://xml.org/sax/handlers/LexicalHandler"), static_cast<CLexicalHandler*>(this));
	m_parser.SetFeature(CXMLUtils::AsciiToXML("http://xml.org/sax/features/namespaces"), true);
	m_parser.SetFeature(CXMLUtils::AsciiToXML("http://xml.org/sax/features/namespace-prefixes"), true);

	if (m_pNamePool) m_pNamePool->Duplicate();
}


DOMBuilder::~DOMBuilder()
{
	if (m_pNamePool) m_pNamePool->Release();
}


void DOMBuilder::setFeature(const XMLString& name, bool state)
{
	if (name == FEATURE_NAMESPACES)
		m_parser.SetFeature(CXMLUtils::AsciiToXML("http://xml.org/sax/features/namespaces"), state);
	else if (name == CXMLUtils::AsciiToXML("external-general-entities"))
		m_parser.SetFeature(CXMLUtils::AsciiToXML("http://xml.org/sax/features/external-general-entities"), state);
	else if (name == CXMLUtils::AsciiToXML("external-parameter-entities"))
		m_parser.SetFeature(CXMLUtils::AsciiToXML("http://xml.org/sax/features/external-parameter-entities"), state);
	else if (name == CXMLUtils::AsciiToXML("white-space-in-element-content"))
		m_whiteSpaceInContent = state;
	else if (name == CXMLUtils::AsciiToXML("use-iso-8859-1"))
		m_parser.SetFeature(CXMLUtils::AsciiToXML("http://www.cpointc.com/xml/parser/use-iso-8859-1"), state);
	else if (name == CXMLUtils::AsciiToXML("validation") ||
	         name == CXMLUtils::AsciiToXML("namespace-declarations") ||
			 name == CXMLUtils::AsciiToXML("validate-if-cm") ||
			 name == CXMLUtils::AsciiToXML("create-entity-ref-nodes") ||
			 name == CXMLUtils::AsciiToXML("entity-nodes") ||
			 name == CXMLUtils::AsciiToXML("cdata-nodes") ||
			 name == CXMLUtils::AsciiToXML("comments") ||
			 name == CXMLUtils::AsciiToXML("charset-overrides-xml-encoding"))
		throw DOMException(DOMException::NOT_SUPPORTED_ERR);
	else
		throw DOMException(DOMException::NOT_FOUND_ERR);
}


bool DOMBuilder::supportsFeature(const XMLString& name) const
{
	return name == FEATURE_NAMESPACES ||
	       name == CXMLUtils::AsciiToXML("external-general-entities") ||
		   name == CXMLUtils::AsciiToXML("external-parameter-entities") ||
		   name == CXMLUtils::AsciiToXML("white-space-in-element-content") ||
		   name == CXMLUtils::AsciiToXML("use-iso-8859-1");
}


bool DOMBuilder::getFeature(const XMLString& name) const
{
	if (name == FEATURE_NAMESPACES)
		return m_parser.GetFeature(CXMLUtils::AsciiToXML("http://xml.org/sax/features/namespaces"));
	else if (name == CXMLUtils::AsciiToXML("external-general-entities"))
		return m_parser.GetFeature(CXMLUtils::AsciiToXML("http://xml.org/sax/features/external-general-entities"));
	else if (name == CXMLUtils::AsciiToXML("external-parameter-entities"))
		return m_parser.GetFeature(CXMLUtils::AsciiToXML("http://xml.org/sax/features/external-parameter-entities"));
	else if (name == CXMLUtils::AsciiToXML("white-space-in-element-content"))
		return m_whiteSpaceInContent;
	else if (name == CXMLUtils::AsciiToXML("use-iso-8859-1"))
		return m_parser.GetFeature(CXMLUtils::AsciiToXML("http://www.cpointc.com/xml/parser/use-iso-8859-1"));
	else if (name == CXMLUtils::AsciiToXML("validation") ||
	         name == CXMLUtils::AsciiToXML("namespace-declarations") ||
			 name == CXMLUtils::AsciiToXML("validate-if-cm") ||
			 name == CXMLUtils::AsciiToXML("create-entity-ref-nodes") ||
			 name == CXMLUtils::AsciiToXML("entity-nodes") ||
			 name == CXMLUtils::AsciiToXML("white-space-in-element-content") ||
			 name == CXMLUtils::AsciiToXML("cdata-nodes") ||
			 name == CXMLUtils::AsciiToXML("comments") ||
			 name == CXMLUtils::AsciiToXML("charset-overrides-xml-encoding"))
		throw DOMException(DOMException::NOT_SUPPORTED_ERR);
	else
		throw DOMException(DOMException::NOT_FOUND_ERR);
}


bool DOMBuilder::canSetFeature(const XMLString& name, bool state)
{
	return name == FEATURE_NAMESPACES ||
	       name == CXMLUtils::AsciiToXML("external-general-entities") ||
		   name == CXMLUtils::AsciiToXML("external-parameter-entities") ||
		   name == CXMLUtils::AsciiToXML("white-space-in-element-content") ||
		   name == CXMLUtils::AsciiToXML("use-iso-8859-1");
}


Document* DOMBuilder::parseURI(const XMLString& uri)
{
	m_pDoc       = new Document(new DocumentType(NULL, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING), m_pNamePool);
	m_pParent    = m_pDoc;
	m_pPrev      = NULL;
	m_inCDATA    = false;
	m_namespaces = getFeature(FEATURE_NAMESPACES);
	m_pDoc->suspendEvents();
	m_parser.Parse(uri);
	m_pDoc->resumeEvents();
	m_pDoc->collectGarbage();
	return m_pDoc;
}


Document* DOMBuilder::parseDOMInputSource(CInputSource* pInputSource)
{
	m_pDoc       = new Document(new DocumentType(NULL, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING), m_pNamePool);
	m_pParent    = m_pDoc;
	m_pPrev      = NULL;
	m_inCDATA    = false;
	m_namespaces = getFeature(FEATURE_NAMESPACES);
	m_parser.Parse(pInputSource);
	m_pDoc->suspendEvents();
	m_pDoc->collectGarbage();
	m_pDoc->resumeEvents();
	return m_pDoc;
}


CEntityResolver* DOMBuilder::getEntityResolver() const
{
	return m_parser.GetEntityResolver();
}


void DOMBuilder::setEntityResolver(CEntityResolver* pEntityResolver)
{
	m_parser.SetEntityResolver(pEntityResolver);
}


CErrorHandler* DOMBuilder::getErrorHandler() const
{
	return m_parser.GetErrorHandler();
}


void DOMBuilder::setErrorHandler(CErrorHandler* pErrorHandler)
{
	m_parser.SetErrorHandler(pErrorHandler);
}


void DOMBuilder::NotationDecl(const XMLString& name, const XMLString* publicId, const XMLString* systemId)
{
	Notation* pObj = new Notation(m_pDoc, name, (publicId ? *publicId : EMPTY_STRING), (systemId ? *systemId : EMPTY_STRING));
	AppendNode(pObj);
	m_pPrev = pObj;
	pObj->release();
}


void DOMBuilder::UnparsedEntityDecl(const XMLString& name, const XMLString* publicId, const XMLString& systemId, const XMLString& notationName)
{
}


void DOMBuilder::SetDocumentLocator(const CLocator& loc)
{
}


void DOMBuilder::StartDocument()
{
}


void DOMBuilder::EndDocument()
{
}


void DOMBuilder::StartElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const CAttributes& attrList)
{
	Element* pElem = m_namespaces ? m_pDoc->createElementNS(uri, qname) : m_pDoc->createElement(qname);

	const CAttributesImpl& attrs = dynamic_cast<const CAttributesImpl&>(attrList);

	for (int i = 0; i < attrs.GetLength(); i++)
	{
		Attr* pAttr = attrs.GetSpecified(i)
		            ? new Attr(m_pDoc, NULL, attrs.GetURI(i), attrs.GetLocalName(i), attrs.GetQName(i), attrs.GetValue(i))
		            : new UnspecAttr(m_pDoc, NULL, attrs.GetURI(i), attrs.GetLocalName(i), attrs.GetQName(i), attrs.GetValue(i));
		if (m_namespaces) 
			pElem->setAttributeNodeNS(pAttr);
		else
			pElem->setAttributeNode(pAttr);
		pAttr->release();
	}
	if (!m_whiteSpaceInContent && m_pPrev && m_pPrev->getNodeType() == Node::TEXT_NODE && IsWhiteSpace(m_pPrev->getNodeValue()))
	{
		m_pParent->replaceChild(pElem, m_pPrev);
	}
	else
	{
		AppendNode(pElem);
	}
	m_pParent = pElem;
	m_pPrev   = pElem;
	pElem->release();
}


void DOMBuilder::EndElement(const XMLString& uri, const XMLString& localName, const XMLString& qname)
{
	if (!m_whiteSpaceInContent && m_pPrev && m_pPrev->getNodeType() == Node::TEXT_NODE && IsWhiteSpace(m_pPrev->getNodeValue()))
	{
		m_pParent->removeChild(m_pPrev);
	}
	m_pPrev   = m_pParent;
	m_pParent = static_cast<ContainerNode*>(m_pParent->getParentNode());
}


void DOMBuilder::Characters(const XMLChar ch[], int start, int length)
{
	if (m_inCDATA)
	{
		if (m_pPrev && m_pPrev->getNodeType() == Node::CDATA_SECTION_NODE)
		{
			static_cast<CDATASection*>(m_pPrev)->appendData(XMLString(ch + start, length));
		}
		else
		{
			CDATASection* pObj = m_pDoc->createCDATASection(XMLString(ch + start, length));
			AppendNode(pObj);
			m_pPrev = pObj;
			pObj->release();
		}
	}
	else
	{
		if (m_pPrev && m_pPrev->getNodeType() == Node::TEXT_NODE)
		{
			static_cast<Text*>(m_pPrev)->appendData(XMLString(ch + start, length));
		}
		else
		{
			Text* pObj = m_pDoc->createTextNode(XMLString(ch + start, length));
			AppendNode(pObj);
			m_pPrev = pObj;
			pObj->release();
		}
	}
}


void DOMBuilder::IgnorableWhitespace(const XMLChar ch[], int start, int length)
{
	Characters(ch, start, length);
}


void DOMBuilder::ProcessingInstruction(const XMLString& target, const XMLString& data)
{
	CSL::XML::ProcessingInstruction* pObj = m_pDoc->createProcessingInstruction(target, data);
	if (pObj)
	{
		AppendNode(pObj);
		m_pPrev = pObj;
		pObj->release();
	}
}


void DOMBuilder::StartPrefixMapping(const XMLString& prefix, const XMLString& uri)
{
}


void DOMBuilder::EndPrefixMapping(const XMLString& prefix)
{
}


void DOMBuilder::SkippedEntity(const XMLString& prefix)
{
}


void DOMBuilder::StartDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId)
{
}


void DOMBuilder::EndDTD()
{
}


void DOMBuilder::StartEntity(const XMLString& name)
{
}


void DOMBuilder::EndEntity(const XMLString& name)
{
}


void DOMBuilder::StartCDATA()
{
	m_inCDATA = true;
}


void DOMBuilder::EndCDATA()
{
	m_inCDATA = false;
}


void DOMBuilder::Comment(const XMLChar ch[], int start, int length)
{
	CSL::XML::Comment* pObj = m_pDoc->createComment(XMLString(ch + start, length));
	AppendNode(pObj);
	m_pPrev = pObj;
	pObj->release();
}


bool DOMBuilder::IsWhiteSpace(const XMLString& str)
{
	XMLString::const_iterator it  = str.begin();
	XMLString::const_iterator end = str.end();
	for (; it != end; ++it)
	{
		if (*it != '\r' && *it != '\n' && *it != '\t' && *it != ' ') return false;
	}
	return true;
}


XML_END
