//
// XMLWriter.cpp
//
// $Id: //depot/XML/Main/src/XMLWriter.cpp#8 $
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


#include "XMLWriter.h"
#include "XMLUtils.h"
#include "XMLException.h"
#include "XMLWriter2.h"
#include "SAX/AttributeList.h"
#include "SAX/AttributesImpl.h"
#include <stdio.h>


XML_BEGIN


CXMLWriter::CXMLWriter(XMLByteOutputStream& str)
{
	m_pWriter = new CXMLWriter2(str, XMLString(), true, false, false);
	m_pWriter->SetLineEnding(CXMLWriter2::LE_UNIX);
}


CXMLWriter::CXMLWriter(XMLByteOutputStream& str, const XMLString& encoding)
{
	m_pWriter = new CXMLWriter2(str, encoding, true, false, false);
	m_pWriter->SetLineEnding(CXMLWriter2::LE_UNIX);
}


CXMLWriter::CXMLWriter(XMLByteOutputStream& str, const XMLString& encoding, bool writeXMLDeclaration, bool writeBOM, bool assumeISO88591)
{
	m_pWriter = new CXMLWriter2(str, encoding, writeXMLDeclaration, writeBOM, assumeISO88591);
	m_pWriter->SetLineEnding(CXMLWriter2::LE_UNIX);
}


CXMLWriter::~CXMLWriter()
{
	delete m_pWriter;
}


void CXMLWriter::SetDocumentLocator(const CLocator& loc)
{
}


void CXMLWriter::SetDocumentType(const XMLString& publicId, const XMLString& systemId)
{
	m_pWriter->SetDocumentType(publicId, systemId);
}


void CXMLWriter::StartDocument()
{
	m_pWriter->StartDocument();
}


void CXMLWriter::EndDocument()
{
	m_pWriter->EndDocument();
}


void CXMLWriter::StartFragment()
{
	m_pWriter->StartFragment();
}


void CXMLWriter::EndFragment()
{
	m_pWriter->EndFragment();
}


void CXMLWriter::StartElement(const XMLString& name)
{
	m_pWriter->StartElement(XMLString(), XMLString(), name);
}


void CXMLWriter::StartElement(const XMLString& name, const CAttributeList& attrList)
{
	CAttributesImpl attrs;
	for (int i = 0; i < attrList.GetLength(); i++)
	{
		attrs.AddAttribute(XMLString(), XMLString(), attrList.GetName(i), attrList.GetType(i), attrList.GetValue(i));
	}
	m_pWriter->StartElement(XMLString(), XMLString(), name, attrs);
}


void CXMLWriter::EndElement(const XMLString& name)
{
	m_pWriter->EndElement(XMLString(), XMLString(), name);
}


void CXMLWriter::EmptyElement(const XMLString& name)
{
	m_pWriter->EmptyElement(XMLString(), XMLString(), name);
}


void CXMLWriter::EmptyElement(const XMLString& name, const CAttributeList& attrList)
{
	CAttributesImpl attrs;
	for (int i = 0; i < attrList.GetLength(); i++)
	{
		attrs.AddAttribute(XMLString(), XMLString(), attrList.GetName(i), attrList.GetType(i), attrList.GetValue(i));
	}
	m_pWriter->EmptyElement(XMLString(), XMLString(), name, attrs);
}


void CXMLWriter::Characters(const XMLChar ch[], int start, int length)
{
	m_pWriter->Characters(ch, start, length);
}


void CXMLWriter::Characters(const XMLString& str)
{
	m_pWriter->Characters(str);
}


void CXMLWriter::IgnorableWhitespace(const XMLChar ch[], int start, int length)
{
	m_pWriter->IgnorableWhitespace(ch, start, length);
}


void CXMLWriter::ProcessingInstruction(const XMLString& target, const XMLString& data)
{
	m_pWriter->ProcessingInstruction(target, data);
}


void CXMLWriter::DataElement(const XMLString& name, const XMLString& data, 
	                         const XMLString& att1, const XMLString& val1,
							 const XMLString& att2, const XMLString& val2,
							 const XMLString& att3, const XMLString& val3)
{
	m_pWriter->DataElement(XMLString(), XMLString(), name, data, att1, val1, att2, val2, att3, val3);
}


void CXMLWriter::StartCDATA()
{
	m_pWriter->StartCDATA();
}


void CXMLWriter::EndCDATA()
{
	m_pWriter->EndCDATA();
}


void CXMLWriter::Comment(const XMLChar ch[], int start, int length)
{
	m_pWriter->Comment(ch, start, length);
}


void CXMLWriter::StartDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId)
{
	m_pWriter->StartDTD(name, publicId, systemId);
}


void CXMLWriter::EndDTD()
{
	m_pWriter->EndDTD();
}


void CXMLWriter::StartEntity(const XMLString& name)
{
	m_pWriter->StartEntity(name);
}


void CXMLWriter::EndEntity(const XMLString& name)
{
	m_pWriter->EndEntity(name);
}


void CXMLWriter::SetPrettyPrint(bool enable)
{
	m_pWriter->SetPrettyPrint(enable);
}


XML_END
