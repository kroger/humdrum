//
// SAXDocumentParser.cpp
//
// $Id: //depot/XML/Main/src/SAXDocumentParser.cpp#5 $
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


#include "SAXDocumentParser.h"
#include "SAX/DTDHandler.h"
#include "SAX/DocumentHandler.h"
#include "SAX/DefaultHandler.h"
#include "SAX/AttributeListImp.h"
#include "SAX/InputSource.h"
#include "SAX/LexicalHandler.h"
#include "SAX/ErrorHandler.h"
#include "XMLException.h"
#include "SAX/SAXException.h"
#include "SAX/SAXParseException.h"
#include "SAX/SAXNotSupportedException.h"
#include "SAX/SAXNotRecognizedException.h"
#include "XMLDocument.h"
#include "XMLUtils.h"


//
// Core properties recognized by the parser
//
#define PROPERTY_LEXICAL_HANDLER   "http://xml.org/sax/handlers/LexicalHandler"


XML_BEGIN


//
// CSAXDocumentParser
//

CSAXDocumentParser::CSAXDocumentParser()
{
}


CSAXDocumentParser::~CSAXDocumentParser()
{
}


void CSAXDocumentParser::SetLocale(const XMLString& locale)
{
	// not supported
}


void CSAXDocumentParser::SetEntityResolver(CEntityResolver* pResolver)
{
}


void CSAXDocumentParser::SetDTDHandler(CDTDHandler* pDTDHandler)
{
	m_pDTDHandler = pDTDHandler;
}


void CSAXDocumentParser::SetDocumentHandler(CDocumentHandler* pDocumentHandler)
{
	m_pDocumentHandler = pDocumentHandler;
}


void CSAXDocumentParser::SetErrorHandler(CErrorHandler* pErrorHandler)
{
	// ignored
}


void CSAXDocumentParser::Parse(const CXMLObject* pNode)
{
	Traverse(pNode);
}


void CSAXDocumentParser::Parse(CInputSource* pSource)
{
}


void CSAXDocumentParser::Parse(const XMLString& systemId)
{
}


void CSAXDocumentParser::SetFeature(const XMLString& featureId, bool state)
{
	throw CSAXNotRecognizedException(CXMLUtils::XMLToAscii(featureId));
}


bool CSAXDocumentParser::GetFeature(const XMLString& featureId) const
{
	throw CSAXNotRecognizedException(CXMLUtils::XMLToAscii(featureId));
}


void CSAXDocumentParser::SetProperty(const XMLString& propertyId, const XMLString& value)
{
	string aId = CXMLUtils::XMLToAscii(propertyId);
	if (aId == PROPERTY_LEXICAL_HANDLER)
	{
		throw CSAXNotSupportedException(string("property does not take a string value: ") + aId);
	}
	else
	{
		throw CSAXNotRecognizedException(aId);
	}
}


void CSAXDocumentParser::SetProperty(const XMLString& propertyId, void* value)
{
	string aId = CXMLUtils::XMLToAscii(propertyId);
	if (aId == PROPERTY_LEXICAL_HANDLER)
	{
		m_pLexicalHandler = reinterpret_cast<CLexicalHandler*>(value);
	}
	else
	{
		throw CSAXNotRecognizedException(aId);
	}
}


void* CSAXDocumentParser::GetProperty(const XMLString& propertyId) const
{
	string aId = CXMLUtils::XMLToAscii(propertyId);
	if (aId == PROPERTY_LEXICAL_HANDLER)
	{
		return m_pLexicalHandler;
	}
	else
	{
		throw CSAXNotSupportedException(aId);
	}
}


void CSAXDocumentParser::Traverse(const CXMLObject* pObject) const
{
	while (pObject)
	{
		Process(pObject);
		pObject = pObject->GetNext();
	}
}


void CSAXDocumentParser::Process(const CXMLObject* pObj) const
{
	switch (pObj->GetType())
	{
	case xmlElement:
		ProcessElement((const CXMLElement*) pObj); 
		break;
	case xmlCharacterData:
		ProcessCharacterData((const CXMLCharacterData*) pObj); 
		break;
	case xmlCDATASection:
		ProcessCDATASection((const CXMLCDATASection*) pObj); 
		break;
	case xmlComment:
		ProcessComment((const CXMLComment*) pObj); 
		break;
	case xmlGenericData:
		ProcessGenericData((const CXMLGenericData*) pObj); 
		break;
	case xmlProcessingInstruction:
		ProcessPI((const CXMLProcessingInstruction*) pObj); 
		break;
	case xmlExternalEntity:
		ProcessExternalEntity((const CXMLExternalEntity*) pObj); 
		break;
	case xmlNotationDeclaration:
		ProcessNotationDecl((const CXMLNotationDeclaration*) pObj); 
		break;
	case xmlUnparsedEntityDeclaration:
		ProcessUnparsedEntityDecl((const CXMLUnparsedEntityDeclaration*) pObj); 
		break;
	case xmlDocument:
		ProcessDocument((const CXMLDocument*) pObj); 
		break;
	case xmlContainer:
		Traverse(pObj->Zoom());
		break;
	}
}


void CSAXDocumentParser::ProcessElement(const CXMLElement* pElem) const
{
	if (m_pDocumentHandler) 
	{
		const CXMLAttributeList& attrList = pElem->GetAttributes();
		CAttributeListImp saxAttrList;
		for (int i = 0; i < attrList.GetLength(); ++i)
		{
			saxAttrList.AddAttribute(attrList.GetName(i), attrList.GetType(i), attrList.GetValue(i), attrList.GetSpecified(i));
		}
		m_pDocumentHandler->StartElement(pElem->GetName(), saxAttrList);
	}
	Traverse(pElem->Zoom());
	if (m_pDocumentHandler)
	{
		m_pDocumentHandler->EndElement(pElem->GetName());
	}
}


void CSAXDocumentParser::ProcessCharacterData(const CXMLCharacterData* pData) const
{
	if (m_pDocumentHandler)
	{
		XMLString data = pData->GetData();
		m_pDocumentHandler->Characters(data.c_str(), 0, data.length());
	}
}


void CSAXDocumentParser::ProcessComment(const CXMLComment* pComment) const
{
	if (m_pLexicalHandler)
	{
		XMLString data = pComment->GetData();
		m_pLexicalHandler->Comment(data.c_str(), 0, data.length());
	}
}


void CSAXDocumentParser::ProcessGenericData(const CXMLGenericData* pData) const
{
	if (m_pDocumentHandler)
	{
		XMLString data = pData->GetData();
		m_pDocumentHandler->IgnorableWhitespace(data.c_str(), 0, data.length());
	}
}


void CSAXDocumentParser::ProcessPI(const CXMLProcessingInstruction* pPI) const
{
	if (m_pDocumentHandler)
	{
		m_pDocumentHandler->ProcessingInstruction(pPI->GetTarget(), pPI->GetData());
	}
}


void CSAXDocumentParser::ProcessExternalEntity(const CXMLExternalEntity* pObj) const
{
	// currently ignored
}


void CSAXDocumentParser::ProcessCDATASection(const CXMLCDATASection* pCDATA) const
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


void CSAXDocumentParser::ProcessNotationDecl(const CXMLNotationDeclaration* pDecl) const
{
	if (m_pDTDHandler) 
	{
		XMLString publicId = pDecl->GetPublicId();
		XMLString systemId = pDecl->GetSystemId();
		m_pDTDHandler->NotationDecl(pDecl->GetName(), &publicId, &systemId);
	}
}


void CSAXDocumentParser::ProcessUnparsedEntityDecl(const CXMLUnparsedEntityDeclaration* pDecl) const
{
	if (m_pDTDHandler) 
	{
		XMLString publicId = pDecl->GetPublicId();
		m_pDTDHandler->UnparsedEntityDecl(pDecl->GetName(), &publicId, pDecl->GetSystemId(), pDecl->GetNotationName());
	}
}


void CSAXDocumentParser::ProcessDocument(const CXMLObject* pObj) const
{
	if (m_pDocumentHandler)
	{
		m_pDocumentHandler->StartDocument();
	}
	Traverse(pObj->Zoom());
	if (m_pDocumentHandler)
	{
		m_pDocumentHandler->EndDocument();
	}
};


XML_END
