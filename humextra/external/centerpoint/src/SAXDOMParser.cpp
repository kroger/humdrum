//
// SAXDOMParser.cpp
//
// $Id: //depot/XML/Main/src/SAXDOMParser.cpp#10 $
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


#include "SAXDOMParser.h"
#include "DOM/Document.h"
#include "DOM/DocumentFragment.h"
#include "DOM/Element.h"
#include "DOM/Attr.h"
#include "DOM/Text.h"
#include "DOM/CDATASection.h"
#include "DOM/Comment.h"
#include "DOM/ProcessingInstruction.h"
#include "DOM/Entity.h"
#include "DOM/Notation.h"
#include "DOM/NamedNodeMap.h"
#include "SAX/EntityResolver.h"
#include "SAX/DTDHandler.h"
#include "SAX/ContentHandler.h"
#include "SAX/LexicalHandler.h"
#include "SAX/AttributesImpl.h"
#include "SAX/InputSource.h"
#include "SAX/ErrorHandler.h"
#include "SAX/SAXException.h"
#include "SAX/SAXParseException.h"
#include "SAX/SAXNotSupportedException.h"
#include "SAX/SAXNotRecognizedException.h"
#include "XMLException.h"
#include "XMLUtils.h"


XML_BEGIN


const XMLString CSAXDOMParser::SAX_FEATURE_NAMESPACES         = CXMLUtils::AsciiToXML("http://xml.org/sax/features/namespaces");
const XMLString CSAXDOMParser::SAX_FEATURE_NAMESPACE_PREFIXES = CXMLUtils::AsciiToXML("http://xml.org/sax/features/namespace-prefixes");
const XMLString CSAXDOMParser::SAX_PROPERTY_DECL_HANDLER      = CXMLUtils::AsciiToXML("http://xml.org/sax/handlers/DeclHandler");
const XMLString CSAXDOMParser::SAX_PROPERTY_LEXICAL_HANDLER   = CXMLUtils::AsciiToXML("http://xml.org/sax/handlers/LexicalHandler");
const XMLString CSAXDOMParser::TYPE_CDATA                     = CXMLUtils::AsciiToXML("CDATA");


CSAXDOMParser::CSAXDOMParser()
{
	m_pEntityResolver = NULL;
	m_pDTDHandler     = NULL;
	m_pContentHandler = NULL;
	m_pErrorHandler   = NULL;
	m_pDeclHandler    = NULL;
	m_pLexicalHandler = NULL;
}


CSAXDOMParser::~CSAXDOMParser()
{
}


void CSAXDOMParser::SetEntityResolver(CEntityResolver* pResolver)
{
	m_pEntityResolver = pResolver;
}


void CSAXDOMParser::SetDTDHandler(CDTDHandler* pDTDHandler)
{
	m_pDTDHandler = pDTDHandler;
}


void CSAXDOMParser::SetContentHandler(CContentHandler* pContentHandler)
{
	m_pContentHandler = pContentHandler;
}


void CSAXDOMParser::SetErrorHandler(CErrorHandler* pErrorHandler)
{
	m_pErrorHandler = pErrorHandler;
}


CEntityResolver* CSAXDOMParser::GetEntityResolver() const
{
	return m_pEntityResolver;
}


CDTDHandler* CSAXDOMParser::GetDTDHandler() const
{
	return m_pDTDHandler;
}


CContentHandler* CSAXDOMParser::GetContentHandler() const
{
	return m_pContentHandler;
}


CErrorHandler* CSAXDOMParser::GetErrorHandler() const
{
	return m_pErrorHandler;
}


void CSAXDOMParser::SetFeature(const XMLString& featureId, bool state)
{
	if (featureId == SAX_FEATURE_NAMESPACES)
	{
		m_featureNamespaces = state;
	}
	else if (featureId == SAX_FEATURE_NAMESPACE_PREFIXES)
	{
		throw CSAXNotSupportedException(CXMLUtils::XMLToAscii(SAX_FEATURE_NAMESPACE_PREFIXES));
	}
	else
	{
		throw CSAXNotRecognizedException(CXMLUtils::XMLToAscii(featureId));
	}
}


bool CSAXDOMParser::GetFeature(const XMLString& featureId) const
{
	if (featureId == SAX_FEATURE_NAMESPACES)
	{
		return m_featureNamespaces;
	}
	else if (featureId == SAX_FEATURE_NAMESPACE_PREFIXES)
	{
		return !m_featureNamespaces;
	}
	else
	{
		throw CSAXNotRecognizedException(CXMLUtils::XMLToAscii(featureId));
	}
}


void CSAXDOMParser::SetProperty(const XMLString& propertyId, const XMLString& value)
{
	if (propertyId == SAX_PROPERTY_DECL_HANDLER    ||
	    propertyId == SAX_PROPERTY_LEXICAL_HANDLER)
	{
		throw CSAXNotSupportedException(string("property does not take a string value: ") + CXMLUtils::XMLToAscii(propertyId));
	}
	else
	{
		throw CSAXNotRecognizedException(CXMLUtils::XMLToAscii(propertyId));
	}
}


void CSAXDOMParser::SetProperty(const XMLString& propertyId, void* value)
{
	if (propertyId == SAX_PROPERTY_DECL_HANDLER)
	{
		m_pDeclHandler = reinterpret_cast<CDeclHandler*>(value);
	}
	else if (propertyId == SAX_PROPERTY_LEXICAL_HANDLER)
	{
		m_pLexicalHandler = reinterpret_cast<CLexicalHandler*>(value);
	}
	else
	{
		throw CSAXNotRecognizedException(CXMLUtils::XMLToAscii(propertyId));
	}
}


void* CSAXDOMParser::GetProperty(const XMLString& propertyId) const
{
	if (propertyId == SAX_PROPERTY_DECL_HANDLER)
	{
		return m_pDeclHandler;
	}
	else if (propertyId == SAX_PROPERTY_LEXICAL_HANDLER)
	{
		return m_pLexicalHandler;
	}
	else
	{
		throw CSAXNotSupportedException(CXMLUtils::XMLToAscii(propertyId));
	}
}


void CSAXDOMParser::Parse(const Node* pNode)
{
	Process(pNode);
}


void CSAXDOMParser::Parse(CInputSource* pSource)
{
}


void CSAXDOMParser::Parse(const XMLString& systemId)
{
}


void CSAXDOMParser::Traverse(const Node* pNode) const
{
	while (pNode)
	{
		Process(pNode);
		pNode = pNode->getNextSibling();
	}
}


void CSAXDOMParser::Process(const Node* pNode) const
{
	switch (pNode->getNodeType())
	{
	case Node::ELEMENT_NODE:
		ProcessElement(static_cast<const Element*>(pNode)); 
		break;
	case Node::TEXT_NODE:
		ProcessCharacterData(static_cast<const Text*>(pNode)); 
		break;
	case Node::CDATA_SECTION_NODE:
		ProcessCDATASection(static_cast<const CDATASection*>(pNode)); 
		break;
	case Node::COMMENT_NODE:
		ProcessComment(static_cast<const Comment*>(pNode)); 
		break;
	case Node::PROCESSING_INSTRUCTION_NODE:
		ProcessPI(static_cast<const ProcessingInstruction*>(pNode)); 
		break;
	case Node::ENTITY_NODE:
		ProcessExternalEntity(static_cast<const Entity*>(pNode)); 
		break;
	case Node::NOTATION_NODE:
		ProcessNotationDecl(static_cast<const Notation*>(pNode)); 
		break;
	case Node::DOCUMENT_NODE:
		ProcessDocument(static_cast<const Document*>(pNode)); 
		break;
	case Node::DOCUMENT_FRAGMENT_NODE:
		ProcessFragment(static_cast<const DocumentFragment*>(pNode));
		break;
	}
}


void CSAXDOMParser::ProcessElement(const Element* pElem) const
{
	if (m_pContentHandler) 
	{
		NamedNodeMap* pAttrs = pElem->getAttributes();
		CAttributesImpl saxAttrs;
		for (unsigned i = 0; i < pAttrs->getLength(); ++i)
		{
			Attr* pAttr = dynamic_cast<Attr*>(pAttrs->item(i));
			saxAttrs.AddAttribute(pAttr->getNamespaceURI(), pAttr->getLocalName(), pAttr->getNodeName(), TYPE_CDATA, pAttr->getNodeValue(), true);
		}
		pAttrs->release();
		m_pContentHandler->StartElement(pElem->getNamespaceURI(), pElem->getLocalName(), pElem->getNodeName(), saxAttrs);
	}
	Traverse(pElem->getFirstChild());
	if (m_pContentHandler)
	{
		m_pContentHandler->EndElement(pElem->getNamespaceURI(), pElem->getLocalName(), pElem->getNodeName());
	}
}


void CSAXDOMParser::ProcessCharacterData(const Text* pData) const
{
	if (m_pContentHandler)
	{
		const XMLString& data = pData->getNodeValue();
		m_pContentHandler->Characters(data.c_str(), 0, data.length());
	}
}


void CSAXDOMParser::ProcessComment(const Comment* pComment) const
{
	if (m_pLexicalHandler)
	{
		const XMLString& data = pComment->getNodeValue();
		m_pLexicalHandler->Comment(data.c_str(), 0, data.length());
	}
}


void CSAXDOMParser::ProcessPI(const ProcessingInstruction* pPI) const
{
	if (m_pContentHandler)
	{
		m_pContentHandler->ProcessingInstruction(pPI->getNodeName(), pPI->getNodeValue());
	}
}


void CSAXDOMParser::ProcessExternalEntity(const Entity* pObj) const
{
	// currently ignored
}


void CSAXDOMParser::ProcessCDATASection(const CDATASection* pCDATA) const
{
	if (m_pLexicalHandler) 
	{
		m_pLexicalHandler->StartCDATA();
	}
	ProcessCharacterData(pCDATA);
	if (m_pLexicalHandler) 
	{
		m_pLexicalHandler->EndCDATA();
	}
}


void CSAXDOMParser::ProcessNotationDecl(const Notation* pDecl) const
{
	if (m_pDTDHandler) 
	{
		const XMLString& publicId = pDecl->getPublicId();
		const XMLString& systemId = pDecl->getSystemId();
		m_pDTDHandler->NotationDecl(pDecl->getNodeName(), &publicId, &systemId);
	}
}


void CSAXDOMParser::ProcessDocument(const Document* pObj) const
{
	if (m_pContentHandler)
	{
		m_pContentHandler->StartDocument();
	}
	Traverse(pObj->getFirstChild());
	if (m_pContentHandler)
	{
		m_pContentHandler->EndDocument();
	}
};


void CSAXDOMParser::ProcessFragment(const DocumentFragment* pObj) const
{
	Traverse(pObj->getFirstChild());
};


XML_END
