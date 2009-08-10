//
// SAX2ParserNSP.cpp
//
// $Id: //depot/XML/Main/src/SAX2ParserNSP.cpp#3 $
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


#include "SAX2ParserNSP.h"
#include "SAX/ContentHandler.h"
#include "SAX/AttributesImpl.h"
#include "XMLUtils.h"


XML_BEGIN


const XMLString CSAX2ParserNSP::COLON = CXMLUtils::AsciiToXML(":");


CSAX2ParserNSP::CSAX2ParserNSP(const XML_Char* encoding): CSAX2ParserNS(encoding)
{
}


void CSAX2ParserNSP::HandleStartElement(const XML_Char* name, const XML_Char** atts)
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
			XMLString attURI;
			XMLString attLocal;
			SplitName(attName, attURI, attLocal);
			const XMLString& prefix = m_namespaceSupport.GetPrefix(attURI);
			if (prefix.empty())
				attrList.AddAttribute(attURI, attLocal, attLocal, TYPE_CDATA, attValue, i < nSpecified);
			else
				attrList.AddAttribute(attURI, attLocal, prefix + COLON + attLocal, TYPE_CDATA, attValue, i < nSpecified);
			i++;
		}
		XMLString uri;
		XMLString local;
		SplitName(name, uri, local);
		const XMLString& prefix = m_namespaceSupport.GetPrefix(uri);
		if (prefix.empty())
			m_pContentHandler->StartElement(uri, local, local, attrList);
		else
			m_pContentHandler->StartElement(uri, local, prefix + COLON + local, attrList);
	}
}


void CSAX2ParserNSP::HandleEndElement(const XML_Char* name)
{
	csl_assert (name != NULL);

	if (m_pContentHandler)
	{
		XMLString uri;
		XMLString local;
		SplitName(name, uri, local);
		const XMLString& prefix = m_namespaceSupport.GetPrefix(uri);
		if (prefix.empty())
			m_pContentHandler->EndElement(uri, local, local);
		else
			m_pContentHandler->EndElement(uri, local, prefix + COLON + local);
	}
}


void CSAX2ParserNSP::HandleStartNamespace(const XML_Char* prefix, const XML_Char* uri)
{
	m_namespaceSupport.PushContext();
	m_namespaceSupport.DeclarePrefix((prefix ? XMLString(prefix) : EMPTY_STRING), (uri ? XMLString(uri) : EMPTY_STRING));
	if (m_pContentHandler) m_pContentHandler->StartPrefixMapping((prefix ? XMLString(prefix) : EMPTY_STRING), (uri ? XMLString(uri) : EMPTY_STRING));
}


void CSAX2ParserNSP::HandleEndNamespace(const XML_Char* prefix)
{
	m_namespaceSupport.PopContext();
	if (m_pContentHandler) m_pContentHandler->EndPrefixMapping((prefix ? XMLString(prefix) : EMPTY_STRING));
}


XML_END
