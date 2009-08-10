//
// SAX2Parser.cpp
//
// $Id: //depot/XML/Main/src/SAX2Parser.cpp#11 $
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


#include "SAX2Parser.h"
#include "SAX2ParserBase.h"
#include "SAX2ParserNS.h"
#include "SAX2ParserNSP.h"
#include "SAX/EntityResolver.h"
#include "SAX/DTDHandler.h"
#include "SAX/ContentHandler.h"
#include "SAX/AnyHandler.h"
#include "SAX/AttributesImpl.h"
#include "SAX/InputSource.h"
#include "SAX/ErrorHandler.h"
#include "XMLException.h"
#include "SAX/SAXException.h"
#include "SAX/SAXParseException.h"
#include "SAX/SAXNotSupportedException.h"
#include "SAX/SAXNotRecognizedException.h"
#include "XMLUtils.h"
#include <fstream>


#ifdef CSL_XML_STANDALONE
#define PARSE_EXCEPTION CXMLException
#else
#define PARSE_EXCEPTION CSL::Base::CException
#endif


XML_BEGIN


//
// CISO88591Filter
//


class XML_API CISO88591Filter: public CContentHandler
{
private:
	CContentHandler* m_pContentHandler;

protected:
#ifndef XML_UNICODE
	void Transcode(const XMLChar ch[], int start, int length, XMLString& target)
	{
		unsigned comp = 0;
		int complen = 0;
		
		while (length > 0)
		{
			register unsigned chr = (unsigned char) ch[start];
			if (chr <= 0x7F)
			{
				target += (XMLChar) chr;
				comp    = 0;
				complen = 0;
			}
			else if ((chr & 0xFC) == 0xFC)
			{
				comp    = chr & 0x01;
				complen = 5;
			}
			else if ((chr & 0xF8) == 0xF8)
			{
				comp    = chr & 0x03;
				complen = 4;
			}
			else if ((chr & 0xF0) == 0xF0)
			{
				comp    = chr & 0x07;
				complen = 3;
			}
			else if ((chr & 0xE0) == 0xE0)
			{
				comp    = chr & 0x0F;
				complen = 2;
			}
			else if ((chr & 0xC0) == 0xC0)
			{
				comp    = chr & 0x1F;
				complen = 1;
			}
			else 
			{
				comp <<= 6;
				comp |= (chr & 0x3F);
				if (--complen == 0)
				{
					target += ((XMLChar) (comp <= 0xFF ? comp : 'X'));
				}
			}
			++start;
			--length;
		}
	}
#endif

public:
	CISO88591Filter()
	{
		m_pContentHandler = NULL;
	}
	
	void SetContentHandler(CContentHandler* pContentHandler)
	{
		csl_assert (pContentHandler);
		
		m_pContentHandler = pContentHandler;
	}
	
	void SetDocumentLocator(const CLocator& loc)
	{
		m_pContentHandler->SetDocumentLocator(loc);
	}
	
	void StartDocument()
	{
		m_pContentHandler->StartDocument();
	}
	
	void EndDocument()
	{
		m_pContentHandler->EndDocument();
	}
	
	void StartElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const CAttributes& attrList)
	{
#ifdef XML_UNICODE
		m_pContentHandler->StartElement(uri, localName, qname, attrList);
#else
		CAttributesImpl transAttrList;
		int attrCount = attrList.GetLength();
		for (int i = 0; i < attrCount; ++i)
		{
			XMLString transUri;
			XMLString uri = attrList.GetURI(i);
			Transcode(uri.c_str(), 0, uri.length(), transUri);
			XMLString transLocalName;
			XMLString localName = attrList.GetLocalName(i);
			Transcode(localName.c_str(), 0, localName.length(), transLocalName);
			XMLString transQname;
			XMLString qname = attrList.GetQName(i);
			Transcode(qname.c_str(), 0, qname.length(), transQname);
			XMLString transValue;
			XMLString value = attrList.GetValue(i);
			Transcode(value.c_str(), 0, value.length(), transValue);
			transAttrList.AddAttribute(transUri, transLocalName, transQname, attrList.GetType(i), transValue);
		}
		XMLString transUri;
		XMLString transLocalName;
		XMLString transQname;
		Transcode(uri.c_str(), 0, uri.length(), transUri);
		Transcode(localName.c_str(), 0, localName.length(), transLocalName);
		Transcode(qname.c_str(), 0, qname.length(), transQname);
		m_pContentHandler->StartElement(transUri, transLocalName, transQname, transAttrList);
#endif
	}
	
	void EndElement(const XMLString& uri, const XMLString& localName, const XMLString& qname)
	{
#ifdef XML_UNICODE
		m_pContentHandler->EndElement(uri, localName, qname);
#else
		XMLString transUri;
		XMLString transLocalName;
		XMLString transQname;
		Transcode(uri.c_str(), 0, uri.length(), transUri);
		Transcode(localName.c_str(), 0, localName.length(), transLocalName);
		Transcode(qname.c_str(), 0, qname.length(), transQname);
		m_pContentHandler->EndElement(transUri, transLocalName, transQname);
#endif
	}
	
	void Characters(const XMLChar ch[], int start, int length)
	{
#ifdef XML_UNICODE
		m_pContentHandler->Characters(ch, start,length);
#else
		XMLString transCh;
		Transcode(ch, start, length, transCh);
		m_pContentHandler->Characters(transCh.c_str(), 0, transCh.length());
#endif
	}
	
	void IgnorableWhitespace(const XMLChar ch[], int start, int length)
	{
		m_pContentHandler->IgnorableWhitespace(ch, start, length);
	}
	
	void ProcessingInstruction(const XMLString& target, const XMLString& data)
	{
#ifdef XML_UNICODE
		m_pContentHandler->ProcessingInstruction(target,data);
#else
		XMLString transTarget;
		XMLString transData;
		Transcode(target.c_str(), 0, target.length(), transTarget);
		Transcode(data.c_str(), 0, data.length(), transData);
		m_pContentHandler->ProcessingInstruction(transTarget, transData);
#endif
	}
	
	void StartPrefixMapping(const XMLString& prefix, const XMLString& uri)
	{
#ifdef XML_UNICODE
		m_pContentHandler->StartPrefixMapping(prefix, uri);
#else
		XMLString transPrefix;
		XMLString transUri;
		Transcode(prefix.c_str(), 0, prefix.length(), transPrefix);
		Transcode(uri.c_str(), 0, uri.length(), transUri);
		m_pContentHandler->StartPrefixMapping(transPrefix, transUri);
#endif
	}
	
	void EndPrefixMapping(const XMLString& prefix)
	{
#ifdef XML_UNICODE
		m_pContentHandler->EndPrefixMapping(prefix);
#else
		XMLString transPrefix;
		Transcode(prefix.c_str(), 0, prefix.length(), transPrefix);
		m_pContentHandler->EndPrefixMapping(transPrefix);
#endif
	}
	
	void SkippedEntity(const XMLString& name)
	{
#ifdef XML_UNICODE
		m_pContentHandler->SkippedEntity(name);
#else
		XMLString transName;
		Transcode(name.c_str(), 0, name.length(), transName);
		m_pContentHandler->SkippedEntity(transName);
#endif
	}
};


const XMLString CSAX2Parser::FEATURE_VALIDATION         = CXMLUtils::AsciiToXML("http://xml.org/sax/features/validation");
const XMLString CSAX2Parser::FEATURE_NAMESPACES         = CXMLUtils::AsciiToXML("http://xml.org/sax/features/namespaces");
const XMLString CSAX2Parser::FEATURE_NAMESPACE_PREFIXES = CXMLUtils::AsciiToXML("http://xml.org/sax/features/namespace-prefixes");
const XMLString CSAX2Parser::FEATURE_EXT_GEN_ENTITIES   = CXMLUtils::AsciiToXML("http://xml.org/sax/features/external-general-entities");
const XMLString CSAX2Parser::FEATURE_EXT_PAR_ENTITIES   = CXMLUtils::AsciiToXML("http://xml.org/sax/features/external-parameter-entities");
const XMLString CSAX2Parser::FEATURE_STRING_INTERNING   = CXMLUtils::AsciiToXML("http://xml.org/sax/features/string-interning");
const XMLString CSAX2Parser::FEATURE_USE_ISO_8859_1     = CXMLUtils::AsciiToXML("http://www.cpointc.com/xml/parser/use-iso-8859-1");
const XMLString CSAX2Parser::PROPERTY_DECL_HANDLER      = CXMLUtils::AsciiToXML("http://xml.org/sax/handlers/DeclHandler");
const XMLString CSAX2Parser::PROPERTY_LEXICAL_HANDLER   = CXMLUtils::AsciiToXML("http://xml.org/sax/handlers/LexicalHandler");
const XMLString CSAX2Parser::PROPERTY_DOCUMENT_BASE     = CXMLUtils::AsciiToXML("http://cpointc.com/xml/parser/DocumentBase");
const XMLString CSAX2Parser::PROPERTY_DEFAULT_HANDLER   = CXMLUtils::AsciiToXML("http://cpointc.com/xml/parser/DefaultHandler");


CSAX2Parser::CSAX2Parser()
{
	m_featureNamespaces        = true;
	m_featureNamespacePrefixes = false;
	m_featureExtGenEntities    = true;
	m_featureExtParEntities    = true;
	m_featureUseISO88591       = false;
	m_pEntityResolver          = NULL;
	m_pDTDHandler              = NULL;
	m_pContentHandler          = NULL;
	m_pErrorHandler            = NULL;
	m_pLexicalHandler          = NULL;
	m_pDeclHandler             = NULL;
	m_pDefaultHandler          = NULL;
	m_pContentFilter           = NULL;
}


CSAX2Parser::CSAX2Parser(const XMLString& encoding)
{
	m_encoding                 = encoding;
	m_featureNamespaces        = true;
	m_featureNamespacePrefixes = false;
	m_featureExtGenEntities    = true;
	m_featureExtParEntities    = true;
	m_featureUseISO88591       = false;
	m_pEntityResolver          = NULL;
	m_pDTDHandler              = NULL;
	m_pContentHandler          = NULL;
	m_pErrorHandler            = NULL;
	m_pLexicalHandler          = NULL;
	m_pDeclHandler             = NULL;
	m_pDefaultHandler          = NULL;
	m_pContentFilter           = NULL;
}


CSAX2Parser::~CSAX2Parser()
{
	if (m_pContentFilter) delete m_pContentFilter;
}


void CSAX2Parser::SetEntityResolver(CEntityResolver* pResolver)
{
	m_pEntityResolver = pResolver;
}


void CSAX2Parser::SetDTDHandler(CDTDHandler* pDTDHandler)
{
	m_pDTDHandler = pDTDHandler;
}


void CSAX2Parser::SetContentHandler(CContentHandler* pContentHandler)
{
	m_pContentHandler = pContentHandler;
}


void CSAX2Parser::SetErrorHandler(CErrorHandler* pErrorHandler)
{
	m_pErrorHandler = pErrorHandler;
}


CEntityResolver* CSAX2Parser::GetEntityResolver() const
{
	return m_pEntityResolver;
}


CDTDHandler* CSAX2Parser::GetDTDHandler() const
{
	return m_pDTDHandler;
}


CContentHandler* CSAX2Parser::GetContentHandler() const
{
	return m_pContentHandler;
}


CErrorHandler* CSAX2Parser::GetErrorHandler() const
{
	return m_pErrorHandler;
}


void CSAX2Parser::Parse(CInputSource* pSource)
{
	csl_check_ptr (pSource);

	CSAX2ParserBase* pParser = MakeParser(pSource->GetEncoding());
	pParser->SetBase(pSource->GetSystemId());
	try
	{
		if (pSource->GetCharacterStream())
		{
			pParser->ParseChar(pSource->GetCharacterStream());
		}
		else if (pSource->GetByteStream())
		{
			pParser->Parse(pSource->GetByteStream());
		}
		else
		{
			throw CSAXParseException("no input stream", XMLString(), pSource->GetSystemId(), 0, 0, CXMLException(CXMLException::EXMLBadInputSource, ""));
		}
	}
	catch (PARSE_EXCEPTION&)
	{
		delete pParser;
		throw;
	}
	delete pParser;
}


void CSAX2Parser::Parse(const XMLString& systemId)
{
	// Since we do not know better,
	// we create a byte input stream.
	ifstream file;
	file.open(CXMLUtils::XMLToAscii(systemId).c_str(), ios::in);
	if (file.good())
	{
		CInputSource source;
		source.SetByteStream(&file);
		source.SetSystemId(systemId);
		source.SetEncoding(m_encoding);
		CSAX2ParserBase* pParser = MakeParser(m_encoding);
		pParser->SetInputSource(&source);
		pParser->SetBase(systemId);
		try
		{
			pParser->Parse(&file);
		}
		catch (PARSE_EXCEPTION&)
		{
			delete pParser;
			file.close();
			throw;
		}
		delete pParser;
		file.close();
	}
	else
	{
#ifdef CSL_XML_STANDALONE
		throw CSAXParseException("file cannot be opened", XMLString(), systemId, 0, 0, CXMLException(CXMLException::EXMLBadSystemId, string()));
#else
		throw CSL::Base::CException(CSL::Base::CException::EOpenFile, CXMLUtils::XMLToAscii(systemId));
#endif
	}
}


void CSAX2Parser::SetFeature(const XMLString& featureId, bool state)
{
	if (featureId == FEATURE_VALIDATION || featureId == FEATURE_STRING_INTERNING)
	{
		throw CSAXNotSupportedException(CXMLUtils::XMLToAscii(FEATURE_VALIDATION));
	}
	else if (featureId == FEATURE_EXT_GEN_ENTITIES)
	{
		m_featureExtGenEntities = state;
	}
	else if (featureId == FEATURE_EXT_PAR_ENTITIES)
	{
		m_featureExtParEntities = state;
	}
	else if (featureId == FEATURE_NAMESPACES)
	{
		m_featureNamespaces = state;
	}
	else if (featureId == FEATURE_NAMESPACE_PREFIXES)
	{
		m_featureNamespacePrefixes = state;
	}
	else if (featureId == FEATURE_USE_ISO_8859_1)
	{
		m_featureUseISO88591 = state;
	}
	else
	{
		throw CSAXNotRecognizedException(CXMLUtils::XMLToAscii(featureId));
	}
}


bool CSAX2Parser::GetFeature(const XMLString& featureId) const
{
	if (featureId == FEATURE_VALIDATION || featureId == FEATURE_STRING_INTERNING)
	{
		throw CSAXNotSupportedException(CXMLUtils::XMLToAscii(FEATURE_VALIDATION));
	}
	else if (featureId == FEATURE_EXT_GEN_ENTITIES)
	{
		return m_featureExtGenEntities;
	}
	else if (featureId == FEATURE_EXT_PAR_ENTITIES)
	{
		return m_featureExtParEntities;
	}
	else if (featureId == FEATURE_NAMESPACES)
	{
		return m_featureNamespaces;
	}
	else if (featureId == FEATURE_NAMESPACE_PREFIXES)
	{
		return m_featureNamespacePrefixes;
	}
	else if (featureId == FEATURE_USE_ISO_8859_1)
	{
		return m_featureUseISO88591;
	}
	else
	{
		throw CSAXNotRecognizedException(CXMLUtils::XMLToAscii(featureId));
	}
}


void CSAX2Parser::SetProperty(const XMLString& propertyId, const XMLString& value)
{
	if (propertyId == PROPERTY_DOCUMENT_BASE)
	{
		m_documentBase = value;
	}
	else if (propertyId == PROPERTY_DECL_HANDLER || propertyId == PROPERTY_LEXICAL_HANDLER)
	{
		throw CSAXNotSupportedException(string("property does not take a string value: ") + CXMLUtils::XMLToAscii(propertyId));
	}
	else
	{
		throw CSAXNotRecognizedException(CXMLUtils::XMLToAscii(propertyId));
	}
}


void CSAX2Parser::SetProperty(const XMLString& propertyId, void* value)
{
	if (propertyId == PROPERTY_DOCUMENT_BASE)
	{
		throw CSAXNotSupportedException(string("property does not take a pointer value: ") + CXMLUtils::XMLToAscii(propertyId));
	}
	else if (propertyId == PROPERTY_DECL_HANDLER)
	{
		m_pDeclHandler = reinterpret_cast<CDeclHandler*>(value);
	}
	else if (propertyId == PROPERTY_LEXICAL_HANDLER)
	{
		m_pLexicalHandler = reinterpret_cast<CLexicalHandler*>(value);
	}
	else if (propertyId == PROPERTY_DEFAULT_HANDLER)
	{
		m_pDefaultHandler = reinterpret_cast<CAnyHandler*>(value);
	}
	else
	{
		throw CSAXNotRecognizedException(CXMLUtils::XMLToAscii(propertyId));
	}
}


void* CSAX2Parser::GetProperty(const XMLString& propertyId) const
{
	if (propertyId == PROPERTY_DOCUMENT_BASE)
	{
		return (void*) m_documentBase.c_str();
	}
	else if (propertyId == PROPERTY_DECL_HANDLER)
	{
		return m_pDeclHandler;
	}
	else if (propertyId == PROPERTY_LEXICAL_HANDLER)
	{
		return m_pLexicalHandler;
	}
	else if (propertyId == PROPERTY_DEFAULT_HANDLER)
	{
		return m_pDefaultHandler;
	}
	else
	{
		throw CSAXNotSupportedException(CXMLUtils::XMLToAscii(propertyId));
	}
}


CSAX2ParserBase* CSAX2Parser::MakeParser(const XMLString& encoding)
{
	CSAX2ParserBase* pParser = NULL;
	if (m_featureNamespaces && m_featureNamespacePrefixes)
	{
		pParser = new CSAX2ParserNSP(encoding.empty() ? NULL : encoding.c_str());
	}
	else if (m_featureNamespaces)
	{
		pParser = new CSAX2ParserNS(encoding.empty() ? NULL : encoding.c_str());
	}
	else
	{
		pParser = new CSAX2ParserBase(encoding.empty() ? NULL : encoding.c_str());
	}
	
	if (m_featureUseISO88591)
	{
		if (!m_pContentFilter) m_pContentFilter = new CISO88591Filter;
		m_pContentFilter->SetContentHandler(m_pContentHandler);
	}
	
	pParser->IncludeExternalGeneralEntities(m_featureExtGenEntities);
	pParser->IncludeExternalParameterEntities(m_featureExtParEntities);
	pParser->SetEntityResolver(m_pEntityResolver);
	pParser->SetDTDHandler(m_pDTDHandler);
	pParser->SetContentHandler(m_featureUseISO88591 ? m_pContentFilter : m_pContentHandler);
	pParser->SetErrorHandler(m_pErrorHandler);
	pParser->SetLexicalHandler(m_pLexicalHandler);
	pParser->SetDeclHandler(m_pDeclHandler);
	pParser->SetDefaultHandler(m_pDefaultHandler);

	return pParser;
}


XML_END
