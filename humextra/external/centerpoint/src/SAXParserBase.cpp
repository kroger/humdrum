//
// SAXParserBase.cpp
//
// $Id: //depot/XML/Main/src/SAXParserBase.cpp#9 $
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


#include "SAXParserBase.h"
#include "SAX/EntityResolver.h"
#include "SAX/EntityResolverImp.h"
#include "SAX/DTDHandler.h"
#include "SAX/DocumentHandler.h"
#include "SAX/LexicalHandler.h"
#include "SAX/NamespaceHandler.h"
#include "SAX/AnyHandler.h"
#include "SAX/AttributeListImp.h"
#include "SAX/InputSource.h"
#include "SAX/ErrorHandler.h"
#include "SAX/SAXParseException.h"
#include "XMLUtils.h"


XML_BEGIN


#ifdef XML_UNICODE_WCHAR_T
static const XMLChar _cdata[] = {L'C', L'D', L'A', L'T', L'A'};
#else
static const XMLChar _cdata[] = {'C', 'D', 'A', 'T', 'A'};
#endif


CSAXParserBase::CSAXParserBase(const XML_Char* encoding):
                CXMLParserBase(encoding)
{
	m_pEntityResolver       = NULL;
	m_pDTDHandler           = NULL;
	m_pDocumentHandler      = NULL;
	m_pLexicalHandler       = NULL;
	m_pNamespaceHandler     = NULL;
	m_pDefaultHandler       = NULL;
	m_pInputSource          = NULL;
	m_pErrorHandler         = NULL;
	m_includeExtGenEntities = true;
}


void CSAXParserBase::SetEntityResolver(CEntityResolver* pResolver)
{
	m_pEntityResolver = pResolver;
}


void CSAXParserBase::SetDTDHandler(CDTDHandler* pDTDHandler)
{
	m_pDTDHandler = pDTDHandler;
}


void CSAXParserBase::SetDocumentHandler(CDocumentHandler* pDocumentHandler)
{
	m_pDocumentHandler = pDocumentHandler;
}


void CSAXParserBase::SetErrorHandler(CErrorHandler* pErrorHandler)
{
	m_pErrorHandler = pErrorHandler;
}


void CSAXParserBase::SetInputSource(CInputSource* pInputSource)
{
	m_pInputSource = pInputSource;
}


void CSAXParserBase::SetLexicalHandler(CLexicalHandler* pLexicalHandler)
{
	m_pLexicalHandler = pLexicalHandler;
}


void CSAXParserBase::SetNamespaceHandler(CNamespaceHandler* pNamespaceHandler)
{
	m_pNamespaceHandler = pNamespaceHandler;
}


void CSAXParserBase::SetDefaultHandler(CAnyHandler* pDefaultHandler)
{
	m_pDefaultHandler = pDefaultHandler;
}


void CSAXParserBase::IncludeExternalGeneralEntities(bool state)
{
	m_includeExtGenEntities = state;
}


void CSAXParserBase::Parse(XMLByteInputStream* istr)
{
	if (m_pDocumentHandler) 
	{
		m_pDocumentHandler->SetDocumentLocator(*this);
		m_pDocumentHandler->StartDocument();
	}
	CXMLParserBase::Parse(istr);
	if (m_pDocumentHandler) m_pDocumentHandler->EndDocument();
}


void CSAXParserBase::ParseChar(XMLCharInputStream* istr)
{
	if (m_pDocumentHandler) 
	{
		m_pDocumentHandler->SetDocumentLocator(*this);
		m_pDocumentHandler->StartDocument();
	}
	CXMLParserBase::ParseChar(istr);
	if (m_pDocumentHandler) m_pDocumentHandler->EndDocument();
}


void CSAXParserBase::HandleStartElement(const XML_Char* name, const XML_Char** atts)
{
	csl_assert (name != NULL && atts != NULL);

	CAttributeListImp attrList;

	const XMLChar* attName;
	const XMLChar* attValue;
	
	int i = 0;
	int nSpecified = GetSpecifiedAttributeCount() / 2;

	while (*atts)
	{
		attName  = *(atts++);
		attValue = *(atts++);
		attrList.AddAttribute(attName, _cdata, attValue, i < nSpecified);
		i++;
	}
	if (m_pDocumentHandler) m_pDocumentHandler->StartElement(name, attrList);
}


void CSAXParserBase::HandleEndElement(const XML_Char* name)
{
	csl_assert (name != NULL);

	if (m_pDocumentHandler) m_pDocumentHandler->EndElement(name);
}


void CSAXParserBase::HandleCharacterData(const XML_Char* s, int len)
{
	csl_assert (s != NULL);

	if (m_pDocumentHandler) m_pDocumentHandler->Characters(s, 0, len);
}


void CSAXParserBase::HandleProcessingInstruction(const XML_Char* target, const XML_Char* data)
{
	csl_assert (target != NULL && data != NULL);
	if (m_pDocumentHandler) m_pDocumentHandler->ProcessingInstruction(target, data);
}


void CSAXParserBase::HandleDefault(const XML_Char* s, int len)
{
	csl_assert (s != NULL);

	if (m_pDefaultHandler) m_pDefaultHandler->Any(s, len, 0);
}


void CSAXParserBase::HandleUnparsedEntityDecl(const XML_Char* entityName, const XML_Char* base,
		                                      const XML_Char* systemId, const XML_Char* publicId,
					        		          const XML_Char* notationName)
{
	csl_assert (entityName != NULL && systemId != NULL && notationName != NULL);

	XMLString pId;
	if (publicId) pId = XMLString(publicId);
	if (m_pDTDHandler) m_pDTDHandler->UnparsedEntityDecl(entityName, publicId ? &pId : NULL, systemId, notationName);
}


void CSAXParserBase::HandleNotationDecl(const XML_Char* notationName, const XML_Char* base,
		                                const XML_Char* systemId, const XML_Char* publicId)
{
	csl_assert (notationName != NULL);

	XMLString pId;
	if (publicId) pId = XMLString(publicId);
	XMLString sId;
	if  (systemId) sId = XMLString(systemId);
	if (m_pDTDHandler) m_pDTDHandler->NotationDecl(notationName, publicId ? &pId : NULL, systemId ? &sId : NULL);
}


int CSAXParserBase::HandleExternalEntityRef(const XML_Char* openEntityNames, const XML_Char* base,
									        const XML_Char* systemId, const XML_Char* publicId)
{
	csl_assert (systemId != NULL);

	CInputSource* pInputSource = NULL;
	XMLString resolvedSystemId;
	if (base)
	{
		resolvedSystemId = ResolveSystemId(base, systemId);
	}
	else
	{
		resolvedSystemId = systemId;
	}
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
		XML_Parser extParser = XML_ExternalEntityParserCreate(GetExpatParser(), openEntityNames, 0);

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
				throw CSAXParseException("no input stream", XMLString(), pInputSource->GetSystemId(), 0, 0, CXMLException(CXMLException::EXMLBadInputSource, ""));
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


void CSAXParserBase::HandleComment(const XML_Char* data)
{
	csl_assert (data != NULL);

	if (m_pLexicalHandler) 
	{
		int len = 0; 
		while (data[len]) len++;
		m_pLexicalHandler->Comment(data, 0, len);
	}
}


void CSAXParserBase::HandleStartCDataSection()
{
	if (m_pLexicalHandler) m_pLexicalHandler->StartCDATA();
}


void CSAXParserBase::HandleEndCDataSection()
{
	if (m_pLexicalHandler) m_pLexicalHandler->EndCDATA();
}


void CSAXParserBase::HandleStartNamespace(const XML_Char* prefix, const XML_Char* uri)
{
	if (m_pNamespaceHandler) m_pNamespaceHandler->StartNamespaceDeclScope((prefix ? XMLString(prefix) : XMLString()), (uri ? XMLString(uri) : XMLString()));
}


void CSAXParserBase::HandleEndNamespace(const XML_Char* prefix)
{
	if (m_pNamespaceHandler) m_pNamespaceHandler->EndNamespaceDeclScope((prefix ? XMLString(prefix) : XMLString()));
}


void CSAXParserBase::HandleError(int errNo)
{
	CXMLException xmlExc(CXMLException::EBase + errNo, "");

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


XMLString CSAXParserBase::GetPublicId() const
{
	if (m_pInputSource)	
		return m_pInputSource->GetPublicId();
	else
		return XMLString();
}


XMLString CSAXParserBase::GetSystemId() const
{
	if (m_pInputSource)
		return m_pInputSource->GetSystemId();
	else
		return XMLString();
}


int CSAXParserBase::GetLineNumber() const
{
	return GetCurrentLineNumber();
}


int CSAXParserBase::GetColumnNumber() const
{
	return GetCurrentColumnNumber();
}


XML_END
