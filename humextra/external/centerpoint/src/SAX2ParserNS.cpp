//
// SAX2ParserNS.cpp
//
// $Id: //depot/XML/Main/src/SAX2ParserNS.cpp#3 $
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


#include "SAX2ParserNS.h"
#include "SAX/ContentHandler.h"
#include "SAX/AttributesImpl.h"


#ifdef XML_UNICODE_WCHAR_T
#define XML_NS_SEPARATOR L'\t'
#else
#define XML_NS_SEPARATOR '\t'
#endif


XML_BEGIN


CSAX2ParserNS::CSAX2ParserNS(const XML_Char* encoding): CSAX2ParserBase(encoding)
{
	EnableNamespaceProcessing(true);
	SetNamespaceSeparator(XML_NS_SEPARATOR);
}


void CSAX2ParserNS::HandleStartElement(const XML_Char* name, const XML_Char** atts)
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
			attrList.AddAttribute(attURI, attLocal, EMPTY_STRING, TYPE_CDATA, attValue, i < nSpecified);
			i++;
		}
		XMLString uri;
		XMLString local;
		SplitName(name, uri, local);
		m_pContentHandler->StartElement(uri, local, EMPTY_STRING, attrList);
	}
}


void CSAX2ParserNS::HandleEndElement(const XML_Char* name)
{
	csl_assert (name != NULL);

	if (m_pContentHandler)
	{
		XMLString uri;
		XMLString local;
		SplitName(name, uri, local);
		m_pContentHandler->EndElement(uri, local, EMPTY_STRING);
	}
}


void CSAX2ParserNS::HandleStartNamespace(const XML_Char* prefix, const XML_Char* uri)
{
	if (m_pContentHandler) m_pContentHandler->StartPrefixMapping((prefix ? XMLString(prefix) : EMPTY_STRING), (uri ? XMLString(uri) : EMPTY_STRING));
}


void CSAX2ParserNS::HandleEndNamespace(const XML_Char* prefix)
{
	if (m_pContentHandler) m_pContentHandler->EndPrefixMapping((prefix ? XMLString(prefix) : EMPTY_STRING));
}


void CSAX2ParserNS::SplitName(const XMLChar* qname, XMLString& uri, XMLString& localName)
{
	register const XMLChar* p = qname;
	while (*p)
	{
		if (*p == XML_NS_SEPARATOR)
		{
			uri = XMLString(qname, p - qname);
			localName = XMLString(p + 1);
			return;
		}
		++p;
	}
	localName = qname;
}


XML_END
