//
// XMLDocumentBuilder.cpp
//
// $Id: //depot/XML/Main/src/XMLDocumentBuilder.cpp#5 $
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


#include "XMLDocumentBuilder.h"
#include "XMLDocument.h"
#include "SAX/InputSource.h"
#include "SAX/SAXParseException.h"
#include "SAX/AttributeListImp.h"
#include "XMLStream.h"
#include "XMLUtils.h"


XML_BEGIN


CXMLDocumentBuilder::CXMLDocumentBuilder()
{
	m_pDoc    = NULL;
	m_pFact   = NULL;
	m_pParent = NULL;
	m_pPrev   = NULL;
	m_inCDATA = false;

	m_parser.SetEntityResolver(this);
	m_parser.SetDTDHandler(this);
	m_parser.SetDocumentHandler(this);
	m_parser.SetErrorHandler(this);
	try
	{
		m_parser.SetProperty(CXMLUtils::AsciiToXML("http://xml.org/sax/handlers/LexicalHandler"), static_cast<CLexicalHandler*>(this));
	}
	catch (CSAXException&)
	{
		// ignored
	}
	IncludeExternalGeneralEntities(true);
}


CXMLDocumentBuilder::CXMLDocumentBuilder(const XMLString& encoding): m_parser(encoding)
{
	m_pDoc    = NULL;
	m_pFact   = NULL;
	m_pParent = NULL;
	m_pPrev   = NULL;
	m_inCDATA = false;

	m_parser.SetEntityResolver(this);
	m_parser.SetDTDHandler(this);
	m_parser.SetDocumentHandler(this);
	m_parser.SetErrorHandler(this);
	try
	{
		m_parser.SetProperty(CXMLUtils::AsciiToXML("http://xml.org/sax/handlers/LexicalHandler"), static_cast<CLexicalHandler*>(this));
	}
	catch (CSAXException&)
	{
		// ignored
	}
	IncludeExternalGeneralEntities(true);
}


CXMLDocumentBuilder::~CXMLDocumentBuilder()
{
}


void CXMLDocumentBuilder::EnableNamespaceProcessing(bool flag)
{
	m_parser.SetFeature(CXMLUtils::AsciiToXML("http://xml.org/sax/features/namespaces"), flag);
}


void CXMLDocumentBuilder::SetNamespaceSeparator(const XMLString& sep)
{
	m_parser.SetProperty(CXMLUtils::AsciiToXML("http://xml.org/sax/properties/namespace-sep"), sep);
}


void CXMLDocumentBuilder::SetBase(const XMLString& base)
{
	m_parser.SetProperty(CXMLUtils::AsciiToXML("http://cpointc.com/xml/parser/DocumentBase"), base);
}


void CXMLDocumentBuilder::IncludeExternalGeneralEntities(bool flag)
{
	m_includeExtGenEntities = flag;
	m_parser.SetFeature(CXMLUtils::AsciiToXML("http://xml.org/sax/features/external-general-entities"), flag);
}


void CXMLDocumentBuilder::BuildDocument(XMLByteInputStream* pIstr, CXMLDocument* pDoc, CXMLFactory* pFact)
{
	m_pDoc    = pDoc;
	m_pFact   = pFact;
	m_pParent = m_pDoc;
	m_pPrev   = NULL;

	CInputSource source(pIstr);

	m_parser.Parse(&source);
}


void CXMLDocumentBuilder::BuildDocument(CInputSource* pSource, CXMLDocument* pDoc, CXMLFactory* pFact)
{
	m_pDoc    = pDoc;
	m_pFact   = pFact;
	m_pParent = m_pDoc;
	m_pPrev   = NULL;

	m_parser.Parse(pSource);
}


void CXMLDocumentBuilder::BuildDocument(const XMLString& systemId, CXMLDocument* pDoc, CXMLFactory* pFact)
{
	m_pDoc    = pDoc;
	m_pFact   = pFact;
	m_pParent = m_pDoc;
	m_pPrev   = NULL;

	m_parser.Parse(systemId);
}


void CXMLDocumentBuilder::SetLocale(const XMLString& locale)
{
	m_parser.SetLocale(locale);
}


void CXMLDocumentBuilder::StartElement(const XMLString& name, const CAttributeList& attrList)
{
	CXMLElement* pElem = m_pFact->CreateElement(name);
	CAttributeListImp& attrListImp = (CAttributeListImp&) attrList; // dirty thing, but in this case we can trust that the parser delivers us a CAttributeListImp
	for (int i = 0; i < attrList.GetLength(); i++)
	{
		pElem->GetAttributes().SetAttribute(m_pFact->CreateAttribute(attrListImp.GetName(i), attrListImp.GetType(i), attrListImp.GetValue(i), attrListImp.GetSpecified(i)));
	}
	m_pParent->InsertChild(pElem);
	m_pParent = pElem;
	m_pPrev = pElem;
}


void CXMLDocumentBuilder::EndElement(const XMLString& name)
{
	m_pPrev = m_pParent;
	m_pParent = static_cast<CXMLContainer*>(m_pParent->GetParent());
}


void CXMLDocumentBuilder::Characters(const XMLChar ch[], int start, int length)
{
	if (m_inCDATA)
	{
		if (m_pPrev && m_pPrev->GetType() == xmlCDATASection)
		{
			XMLString data = static_cast<CXMLCDATASection*>(m_pPrev)->GetData();
			static_cast<CXMLCDATASection*>(m_pPrev)->SetData(data + XMLString(ch + start, length));
		}
		else
		{
			CXMLCDATASection* pObj = m_pFact->CreateCDATASection(XMLString(ch + start, length));
			m_pParent->InsertChild(pObj);
			m_pPrev = pObj;
		}
	}
	else
	{
		if (m_pPrev && m_pPrev->GetType() == xmlCharacterData)
		{
			XMLString data = static_cast<CXMLCharacterData*>(m_pPrev)->GetData();
			static_cast<CXMLCharacterData*>(m_pPrev)->SetData(data + XMLString(ch + start, length));
		}
		else
		{
			CXMLCharacterData* pObj = m_pFact->CreateCharacterData(XMLString(ch + start, length));
			m_pParent->InsertChild(pObj);
			m_pPrev = pObj;
		}
	}
}


void CXMLDocumentBuilder::ProcessingInstruction(const XMLString& target, const XMLString& data)
{
	CXMLProcessingInstruction* pObj = m_pFact->CreateProcessingInstruction(target, data);
	if (pObj)
	{
		m_pParent->InsertChild(pObj);
		m_pPrev = pObj;
	}
}


void CXMLDocumentBuilder::IgnorableWhitespace(const XMLChar ch[], int start, int length)
{
	if (m_pPrev && m_pPrev->GetType() == xmlGenericData)
	{
		XMLString data = static_cast<CXMLGenericData*>(m_pPrev)->GetData();
		static_cast<CXMLGenericData*>(m_pPrev)->SetData(data + XMLString(ch + start, length));
	}
	else
	{
		CXMLGenericData* pObj = m_pFact->CreateGenericData(XMLString(ch + start, length));
		m_pParent->InsertChild(pObj);
		m_pPrev = pObj;
	}
}


void CXMLDocumentBuilder::UnparsedEntityDecl(const XMLString& name, const XMLString* publicId, const XMLString& systemId, const XMLString& notationName)
{
	CXMLUnparsedEntityDeclaration* pObj = m_pFact->CreateUnparsedEntityDeclaration(name, systemId, (publicId ? *publicId : XMLString()), notationName);
	if (pObj)
	{
		m_pParent->InsertChild(pObj);
		m_pPrev = pObj;
	}
}


void CXMLDocumentBuilder::NotationDecl(const XMLString& name, const XMLString* publicId, const XMLString* systemId)
{
	CXMLNotationDeclaration* pObj = m_pFact->CreateNotationDeclaration(name, (systemId ? *systemId : XMLString()), (publicId ? *publicId : XMLString()));
	if (pObj)
	{
		m_pParent->InsertChild(pObj);
		m_pPrev = pObj;
	}
}


CInputSource* CXMLDocumentBuilder::ResolveEntity(const XMLString* publicId, const XMLString& systemId)
{
	if (!m_includeExtGenEntities)
	{
		CXMLExternalEntity* pObj = m_pFact->CreateExternalEntity(systemId, (publicId ? *publicId : XMLString()));
		if (pObj)
		{
			m_pParent->InsertChild(pObj);
			m_pPrev = pObj;
		}
	}
	return NULL; // tell the parser to use its default resolver
}


void CXMLDocumentBuilder::Warning(CSAXException& e)
{
	throw e;
}


void CXMLDocumentBuilder::Error(CSAXException& e)
{
	throw e;
}


void CXMLDocumentBuilder::FatalError(CSAXException& e)
{
	throw e;
}


void CXMLDocumentBuilder::StartDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId)
{
	// not supported
}


void CXMLDocumentBuilder::EndDTD()
{
	// not supported
}


void CXMLDocumentBuilder::StartEntity(const XMLString& name)
{
	// not supported
}


void CXMLDocumentBuilder::EndEntity(const XMLString& name)
{
	// not supported
}


void CXMLDocumentBuilder::StartCDATA()
{
	m_inCDATA = true;
}


void CXMLDocumentBuilder::EndCDATA()
{
	m_inCDATA = false;
}


void CXMLDocumentBuilder::Comment(const XMLChar ch[], int start, int length)
{
	CXMLComment* pObj = m_pFact->CreateComment(XMLString(ch + start, length));
	m_pParent->InsertChild(pObj);
	m_pPrev = pObj;
}


XML_END
