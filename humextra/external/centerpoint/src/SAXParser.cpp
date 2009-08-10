//
// SAXParser.cpp
//
// $Id: //depot/XML/Main/src/SAXParser.cpp#6 $
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


#include "SAXParser.h"
#include "SAX/EntityResolver.h"
#include "SAX/DTDHandler.h"
#include "SAX/DocumentHandler.h"
#include "SAX/AnyHandler.h"
#include "SAX/AttributeListImp.h"
#include "SAX/InputSource.h"
#include "SAX/ErrorHandler.h"
#include "XMLException.h"
#include "SAX/SAXException.h"
#include "SAX/SAXParseException.h"
#include "SAX/SAXNotSupportedException.h"
#include "SAX/SAXNotRecognizedException.h"
#include "XMLUtils.h"
#include <fstream>


//
// Core features recognized by the parser
//
#define FEATURE_VALIDATION         "http://xml.org/sax/features/validation"
#define FEATURE_NAMESPACES         "http://xml.org/sax/features/namespaces"
#define FEATURE_NORMALIZE          "http://xml.org/sax/features/normalize-text"
#define FEATURE_USE_LOCATOR        "http://xml.org/sax/features/use-locator"
#define FEATURE_EXT_GEN_ENTITIES   "http://xml.org/sax/features/external-general-entities"
//
// Core properties recognized by the parser
//
#define PROPERTY_NAMESPACE_SEP     "http://xml.org/sax/properties/namespace-sep"
#define PROPERTY_DECL_HANDLER      "http://xml.org/sax/handlers/DeclHandler"
#define PROPERTY_LEXICAL_HANDLER   "http://xml.org/sax/handlers/LexicalHandler"
#define PROPERTY_NAMESPACE_HANDLER "http://xml.org/sax/handlers/NamespaceHandler"
// non-standard extensions
#define PROPERTY_DOCUMENT_BASE     "http://cpointc.com/xml/parser/DocumentBase"
#define PROPERTY_DEFAULT_HANDLER   "http://cpointc.com/xml/parser/DefaultHandler"


XML_BEGIN


//
// CSAXParser
//

CSAXParser::CSAXParser()
{
	m_featureNamespaces     = false;
	m_featureUseLocator     = true;
	m_featureExtGenEntities = true;
	m_base.IncludeExternalGeneralEntities(m_featureExtGenEntities);
}


CSAXParser::CSAXParser(const XMLString& encoding): m_base(encoding.c_str())
{
	m_featureNamespaces     = false;
	m_featureUseLocator     = true;
	m_featureExtGenEntities = true;
	m_base.IncludeExternalGeneralEntities(m_featureExtGenEntities);
}


CSAXParser::~CSAXParser()
{
}


void CSAXParser::SetLocale(const XMLString& locale)
{
	// not supported
}


void CSAXParser::SetEntityResolver(CEntityResolver* pResolver)
{
	m_base.SetEntityResolver(pResolver);
}


void CSAXParser::SetDTDHandler(CDTDHandler* pDTDHandler)
{
	m_base.SetDTDHandler(pDTDHandler);
}


void CSAXParser::SetDocumentHandler(CDocumentHandler* pDocumentHandler)
{
	m_base.SetDocumentHandler(pDocumentHandler);
}


void CSAXParser::SetErrorHandler(CErrorHandler* pErrorHandler)
{
	m_base.SetErrorHandler(pErrorHandler);
}


void CSAXParser::Parse(CInputSource* pSource)
{
	m_base.SetInputSource(pSource);
	m_base.SetBase(pSource->GetSystemId());
	if (pSource->GetCharacterStream())
	{
		m_base.ParseChar(pSource->GetCharacterStream());
	}
	else if (pSource->GetByteStream())
	{
		m_base.Parse(pSource->GetByteStream());
	}
	else
	{
		throw CSAXParseException("no input stream", XMLString(), pSource->GetSystemId(), 0, 0, CXMLException(CXMLException::EXMLBadInputSource, ""));
	}
}


void CSAXParser::Parse(const XMLString& systemId)
{
	CInputSource source;
	// Since we do not know better,
	// we create a byte input stream.
	ifstream file;
	file.open(CXMLUtils::XMLToAscii(systemId).c_str(), ios::in);
	if (file.good())
	{
		source.SetByteStream(&file);
		source.SetSystemId(systemId);
		m_base.SetInputSource(&source);
		m_base.SetBase(systemId);
		m_base.Parse(&file);
		file.close();
	}
	else
	{
#ifdef CSL_XML_STANDALONE
		throw CSAXParseException("file cannot be opened", XMLString(), source.GetSystemId(), 0, 0, CXMLException(CXMLException::EXMLBadSystemId, ""));
#else
		throw CSL::Base::CException(CSL::Base::CException::EOpenFile, CXMLUtils::XMLToAscii(systemId));
#endif
	}
}


void CSAXParser::SetFeature(const XMLString& featureId, bool state)
{
	string aId = CXMLUtils::XMLToAscii(featureId);
	if (aId == FEATURE_VALIDATION)
	{
		throw CSAXNotSupportedException(FEATURE_VALIDATION);
	}
	else if (aId == FEATURE_NORMALIZE)
	{
		throw CSAXNotSupportedException(FEATURE_NORMALIZE);
	}
	else if (aId == FEATURE_EXT_GEN_ENTITIES)
	{
		m_featureExtGenEntities = state;
		m_base.IncludeExternalGeneralEntities(state);
	}
	else if (aId == FEATURE_NAMESPACES)
	{
		m_featureNamespaces = state;
		m_base.EnableNamespaceProcessing(state);
	}
	else if (aId == FEATURE_USE_LOCATOR)
	{
		m_featureUseLocator = state;
	}
	else
	{
		throw CSAXNotRecognizedException(aId);
	}
}


bool CSAXParser::GetFeature(const XMLString& featureId) const
{
	string aId = CXMLUtils::XMLToAscii(featureId);
	if (aId == FEATURE_VALIDATION)
	{
		throw CSAXNotSupportedException(FEATURE_VALIDATION);
	}
	else if (aId == FEATURE_NORMALIZE)
	{
		throw CSAXNotSupportedException(FEATURE_NORMALIZE);
	}
	else if (aId == FEATURE_EXT_GEN_ENTITIES)
	{
		return m_featureExtGenEntities;
	}
	else if (aId == FEATURE_NAMESPACES)
	{
		return m_featureNamespaces;
	}
	else if (aId == FEATURE_USE_LOCATOR)
	{
		return m_featureUseLocator;
	}
	else
	{
		throw CSAXNotRecognizedException(aId);
	}
}


void CSAXParser::SetProperty(const XMLString& propertyId, const XMLString& value)
{
	string aId = CXMLUtils::XMLToAscii(propertyId);
	if (aId == PROPERTY_NAMESPACE_SEP)
	{
		m_propertyNamespaceSep = value;
		if (value.empty())
			m_base.SetNamespaceSeparator((XML_Char) 0);
		else
			m_base.SetNamespaceSeparator(value[0]);
	}
	else if (aId == PROPERTY_DOCUMENT_BASE)
	{
		m_base.SetBase(value);
	}
	else if (aId == PROPERTY_DECL_HANDLER     ||
	         aId == PROPERTY_LEXICAL_HANDLER  ||
			 aId == PROPERTY_NAMESPACE_HANDLER)
	{
		throw CSAXNotSupportedException(string("property does not take a string value: ") + aId);
	}
	else
	{
		throw CSAXNotRecognizedException(aId);
	}
}


void CSAXParser::SetProperty(const XMLString& propertyId, void* value)
{
	string aId = CXMLUtils::XMLToAscii(propertyId);
	if (aId == PROPERTY_NAMESPACE_SEP || aId == PROPERTY_DOCUMENT_BASE)
	{
		throw CSAXNotSupportedException(string("property does not take a pointer value: ") + aId);
	}
	else if (aId == PROPERTY_DECL_HANDLER)
	{
		throw CSAXNotSupportedException(aId);
	}
	else if (aId == PROPERTY_LEXICAL_HANDLER)
	{
		m_base.SetLexicalHandler(reinterpret_cast<CLexicalHandler*>(value));
	}
	else if (aId == PROPERTY_NAMESPACE_HANDLER)
	{
		m_base.SetNamespaceHandler(reinterpret_cast<CNamespaceHandler*>(value));
	}
	else if (aId == PROPERTY_DEFAULT_HANDLER)
	{
		m_base.SetDefaultHandler(reinterpret_cast<CAnyHandler*>(value));
	}
	else
	{
		throw CSAXNotRecognizedException(aId);
	}
}


void* CSAXParser::GetProperty(const XMLString& propertyId) const
{
	string aId = CXMLUtils::XMLToAscii(propertyId);
	if (aId == PROPERTY_NAMESPACE_SEP)
	{
		return const_cast<XMLChar*>(m_propertyNamespaceSep.c_str());
	}
	else if (aId == PROPERTY_DOCUMENT_BASE)
	{
		return const_cast<XMLChar*>(m_base.GetBase().c_str());
	}
	else
	{
		throw CSAXNotSupportedException(aId);
	}
}


XML_END
