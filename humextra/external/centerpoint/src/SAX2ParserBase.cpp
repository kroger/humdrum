//
// SAX2ParserBase.cpp
//
// $Id: //depot/XML/Main/src/SAX2ParserBase.cpp#9 $
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


#include "SAX2ParserBase.h"
#include "SAX/EntityResolver.h"
#include "SAX/EntityResolverImp.h"
#include "SAX/DTDHandler.h"
#include "SAX/ContentHandler.h"
#include "SAX/LexicalHandler.h"
#include "SAX/DeclHandler.h"
#include "SAX/NamespaceHandler.h"
#include "SAX/AnyHandler.h"
#include "SAX/AttributesImpl.h"
#include "SAX/InputSource.h"
#include "SAX/ErrorHandler.h"
#include "SAX/SAXParseException.h"
#include "XMLUtils.h"


XML_BEGIN


#ifdef XML_UNICODE_WCHAR_T
#define XML_NS_SEPARATOR L'\t'
#else
#define XML_NS_SEPARATOR '\t'
#endif


const XMLString CSAX2ParserBase::EMPTY_STRING;
const XMLString CSAX2ParserBase::TYPE_CDATA   = CXMLUtils::AsciiToXML("CDATA");


CSAX2ParserBase::CSAX2ParserBase(const XML_Char* encoding): CXMLParserBase(encoding)
{
	m_pEntityResolver       = NULL;
	m_pDTDHandler           = NULL;
	m_pContentHandler       = NULL;
	m_pLexicalHandler       = NULL;
	m_pDeclHandler          = NULL;
	m_pDefaultHandler       = NULL;
	m_pInputSource          = NULL;
	m_pErrorHandler         = NULL;
	m_includeExtGenEntities = true;
	m_includeExtParEntities = true;
	EnableNamespaceProcessing(false);
}


void CSAX2ParserBase::SetEntityResolver(CEntityResolver* pResolver)
{
	m_pEntityResolver = pResolver;
}


void CSAX2ParserBase::SetDTDHandler(CDTDHandler* pDTDHandler)
{
	m_pDTDHandler = pDTDHandler;
}


void CSAX2ParserBase::SetContentHandler(CContentHandler* pContentHandler)
{
	m_pContentHandler = pContentHandler;
}


void CSAX2ParserBase::SetErrorHandler(CErrorHandler* pErrorHandler)
{
	m_pErrorHandler = pErrorHandler;
}


void CSAX2ParserBase::SetInputSource(CInputSource* pInputSource)
{
	m_pInputSource = pInputSource;
}


void CSAX2ParserBase::SetLexicalHandler(CLexicalHandler* pLexicalHandler)
{
	m_pLexicalHandler = pLexicalHandler;
}


void CSAX2ParserBase::SetDeclHandler(CDeclHandler* pDeclHandler)
{
	m_pDeclHandler = pDeclHandler;
}


void CSAX2ParserBase::SetDefaultHandler(CAnyHandler* pDefaultHandler)
{
	m_pDefaultHandler = pDefaultHandler;
}


CEntityResolver* CSAX2ParserBase::GetEntityResolver() const
{
	return m_pEntityResolver;
}


CDTDHandler* CSAX2ParserBase::GetDTDHandler() const
{
	return m_pDTDHandler;
}


CContentHandler* CSAX2ParserBase::GetContentHandler() const
{
	return m_pContentHandler;
}


CErrorHandler* CSAX2ParserBase::GetErrorHandler() const
{
	return m_pErrorHandler;
}


CLexicalHandler* CSAX2ParserBase::GetLexicalHandler() const
{
	return m_pLexicalHandler;
}


CDeclHandler* CSAX2ParserBase::GetDeclHandler() const
{
	return m_pDeclHandler;
}


CAnyHandler* CSAX2ParserBase::GetDefaultHandler() const
{
	return m_pDefaultHandler;
}


void CSAX2ParserBase::IncludeExternalGeneralEntities(bool state)
{
	m_includeExtGenEntities = state;
}


void CSAX2ParserBase::IncludeExternalParameterEntities(bool state)
{
	m_includeExtParEntities = state;
	SetParamEntityParsing(m_includeExtParEntities ? XML_PARAM_ENTITY_PARSING_ALWAYS : XML_PARAM_ENTITY_PARSING_NEVER);
}


void CSAX2ParserBase::Parse(XMLByteInputStream* istr)
{
	if (m_pContentHandler) 
	{
		m_pContentHandler->SetDocumentLocator(*this);
		m_pContentHandler->StartDocument();
	}
	CXMLParserBase::Parse(istr);
	if (m_pContentHandler) m_pContentHandler->EndDocument();
}


void CSAX2ParserBase::ParseChar(XMLCharInputStream* istr)
{
	if (m_pContentHandler) 
	{
		m_pContentHandler->SetDocumentLocator(*this);
		m_pContentHandler->StartDocument();
	}
	CXMLParserBase::ParseChar(istr);
	if (m_pContentHandler) m_pContentHandler->EndDocument();
}


void CSAX2ParserBase::HandleStartElement(const XML_Char* name, const XML_Char** atts)
{
	csl_assert (name != NULL && atts != NULL);

	if (m_pContentHandler)
	{
		CAttributesImpl attrList;

		const XMLChar* attName;
		const XMLChar* attValue;
		
		int i = 0;
		int nSpecified = GetSpecifiedAttributeCount() / 2;

		while (*atts)
		{
			attName  = *(atts++);
			attValue = *(atts++);
			attrList.AddAttribute(EMPTY_STRING, EMPTY_STRING, attName, TYPE_CDATA, attValue, i < nSpecified);
			i++;
		}
		m_pContentHandler->StartElement(EMPTY_STRING, EMPTY_STRING, name, attrList);
	}
}


void CSAX2ParserBase::HandleEndElement(const XML_Char* name)
{
	csl_assert (name != NULL);

	if (m_pContentHandler)
	{
		m_pContentHandler->EndElement(EMPTY_STRING, EMPTY_STRING, name);
	}
}


void CSAX2ParserBase::HandleCharacterData(const XML_Char* s, int len)
{
	csl_assert (s != NULL);

	if (m_pContentHandler) m_pContentHandler->Characters(s, 0, len);
}


void CSAX2ParserBase::HandleProcessingInstruction(const XML_Char* target, const XML_Char* data)
{
	csl_assert (target != NULL && data != NULL);
	if (m_pContentHandler) m_pContentHandler->ProcessingInstruction(target, data);
}


void CSAX2ParserBase::HandleDefault(const XML_Char* s, int len)
{
	csl_assert (s != NULL);

	if (m_pDefaultHandler) m_pDefaultHandler->Any(s, len, 0);
}


void CSAX2ParserBase::HandleUnparsedEntityDecl(const XML_Char* entityName, const XML_Char* base,
		                                       const XML_Char* systemId, const XML_Char* publicId,
					        		           const XML_Char* notationName)
{
	csl_assert (entityName != NULL && systemId != NULL && notationName != NULL);

	XMLString pId;
	if (publicId) pId = XMLString(publicId);
	if (m_pDTDHandler) m_pDTDHandler->UnparsedEntityDecl(entityName, publicId ? &pId : NULL, systemId, notationName);
}


void CSAX2ParserBase::HandleNotationDecl(const XML_Char* notationName, const XML_Char* base,
		                                 const XML_Char* systemId, const XML_Char* publicId)
{
	csl_assert (notationName != NULL);

	XMLString pId;
	if (publicId) pId = XMLString(publicId);
	XMLString sId;
	if  (systemId) sId = XMLString(systemId);
	if (m_pDTDHandler) m_pDTDHandler->NotationDecl(notationName, publicId ? &pId : NULL, systemId ? &sId : NULL);
}


int CSAX2ParserBase::HandleExternalEntityRef(const XML_Char* context, const XML_Char* base,
									         const XML_Char* systemId, const XML_Char* publicId)
{
	csl_assert (systemId != NULL);

	if (context == NULL && !m_includeExtParEntities) return 0;
	if (context != NULL && !m_includeExtGenEntities) return 0;

	CInputSource* pInputSource = NULL;
	XMLString resolvedSystemId;
	if (base)
		resolvedSystemId = ResolveSystemId(base, systemId);
	else
		resolvedSystemId = systemId;

	CEntityResolver* pEntityResolver = NULL;
	CEntityResolverImp defaultResolver;

	XMLString pId;
	if (publicId) pId = XMLString(publicId);

	if (m_pEntityResolver)
	{
		pInputSource = m_pEntityResolver->ResolveEntity(publicId ? &pId : NULL, resolvedSystemId);
		pEntityResolver = m_pEntityResolver;
	}
	if (!pInputSource && m_includeExtGenEntities)
	{
		pInputSource = defaultResolver.ResolveEntity(publicId ? &pId : NULL, resolvedSystemId);
		pEntityResolver = &defaultResolver;
	}

	if (pInputSource)
	{
		XML_Parser extParser = XML_ExternalEntityParserCreate(GetExpatParser(), context, 0);

		XML_SetBase(extParser, resolvedSystemId.c_str());

		try
		{
			if (pInputSource->GetCharacterStream())
			{
				ParseCharExternal(extParser, pInputSource->GetCharacterStream());
			}
			else if (pInputSource->GetByteStream())
			{
				ParseExternal(extParser, pInputSource->GetByteStream());
			}
			else
			{
				throw CSAXParseException("no input stream", EMPTY_STRING, pInputSource->GetSystemId(), 0, 0, CXMLException(CXMLException::EXMLBadInputSource, string()));
			}
		}
		catch (CXMLException& e)
		{
			// cleanup before we propagate the exception
			pEntityResolver->DoneWithInputSource(pInputSource);
			XML_ParserFree(extParser);
			throw e;
		}
		pEntityResolver->DoneWithInputSource(pInputSource);
		XML_ParserFree(extParser);
		return 1;
	}
	else
	{
		return 0;
	}
}


void CSAX2ParserBase::HandleComment(const XML_Char* data)
{
	csl_assert (data != NULL);

	if (m_pLexicalHandler) 
	{
		int len = 0; 
		while (data[len]) len++;
		m_pLexicalHandler->Comment(data, 0, len);
	}
}


void CSAX2ParserBase::HandleStartCDataSection()
{
	if (m_pLexicalHandler) m_pLexicalHandler->StartCDATA();
}


void CSAX2ParserBase::HandleEndCDataSection()
{
	if (m_pLexicalHandler) m_pLexicalHandler->EndCDATA();
}


void CSAX2ParserBase::HandleStartDoctypeDecl(const XML_Char* doctypeName)
{
	if (m_pLexicalHandler) m_pLexicalHandler->StartDTD(doctypeName, NULL, NULL);
}


void CSAX2ParserBase::HandleEndDoctypeDecl()
{
	if (m_pLexicalHandler) m_pLexicalHandler->EndDTD();
}


void CSAX2ParserBase::HandleExternalParsedEntityDecl(const XML_Char* entityName, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId)
{
	XMLString strPublicId = publicId ? XMLString(publicId) : EMPTY_STRING;
	if (m_pDeclHandler) m_pDeclHandler->ExternalEntityDecl(entityName, publicId ? &strPublicId : NULL, systemId);
}


void CSAX2ParserBase::HandleInternalParsedEntityDecl(const XML_Char* entityName, const XML_Char* replacementText, int replacementTextLength)
{
	if (m_pDeclHandler) m_pDeclHandler->InternalEntityDecl(entityName, XMLString(replacementText, replacementTextLength));
}


void CSAX2ParserBase::HandleError(int errNo)
{
	CXMLException xmlExc(CXMLException::EBase + errNo, string());

	XMLString systemId;
	XMLString publicId;

	if (m_pInputSource)
	{
		systemId = m_pInputSource->GetSystemId();
		publicId = m_pInputSource->GetPublicId();
	}

	CSAXParseException saxExc("", publicId, systemId, GetCurrentLineNumber(), GetCurrentColumnNumber(), xmlExc);

	if (m_pErrorHandler) 
		m_pErrorHandler->Error(saxExc);
	else
		throw saxExc;
}


XMLString CSAX2ParserBase::GetPublicId() const
{
	if (m_pInputSource)	
		return m_pInputSource->GetPublicId();
	else
		return EMPTY_STRING;
}


XMLString CSAX2ParserBase::GetSystemId() const
{
	if (m_pInputSource)
		return m_pInputSource->GetSystemId();
	else
		return EMPTY_STRING;
}


int CSAX2ParserBase::GetLineNumber() const
{
	return GetCurrentLineNumber();
}


int CSAX2ParserBase::GetColumnNumber() const
{
	return GetCurrentColumnNumber();
}


XML_END
