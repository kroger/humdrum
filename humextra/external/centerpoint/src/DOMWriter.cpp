//
// DOMWriter.cpp
//
// $Id: //depot/XML/Main/src/DOMWriter.cpp#12 $
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


#include "DOM/DOMWriter.h"
#include "XMLWriter2.h"
#include "DOM/Document.h"
#include "DOM/DocumentFragment.h"
#include "DOM/DocumentType.h"
#include "DOM/DOMException.h"
#include "SAX/LexicalHandler.h"
#include "SAXDOMParser.h"
#include "XMLUtils.h"
#include "XMLException.h"
#include <fstream>


XML_BEGIN


const XMLString DOMWriter::NEWLINE_CR         = CXMLUtils::AsciiToXML("CR");
const XMLString DOMWriter::NEWLINE_CRLF       = CXMLUtils::AsciiToXML("CR-LF");
const XMLString DOMWriter::NEWLINE_LF         = CXMLUtils::AsciiToXML("LF");
const XMLString DOMWriter::ENC_UTF16          = CXMLUtils::AsciiToXML("UTF-16");
const XMLString DOMWriter::SAX_LEXICALHANDLER = CXMLUtils::AsciiToXML("http://xml.org/sax/handlers/LexicalHandler");


DOMWriter::DOMWriter()
{
	m_format         = AS_IS;
	m_assumeISO88591 = false;
}


DOMWriter::~DOMWriter()
{
}


void DOMWriter::setEncoding(const XMLString& encoding)
{
	m_encoding = encoding;
}


void DOMWriter::setEncoding(const XMLString& encoding, bool assumeISO88591)
{
	m_encoding       = encoding;
	m_assumeISO88591 = assumeISO88591;
}


const XMLString& DOMWriter::getEncoding() const
{
	return m_encoding;
}


const XMLString& DOMWriter::getLastEncoding() const
{
	return m_lastEncoding;
}


void DOMWriter::setFormat(unsigned short format)
{
	m_format = format;
}


unsigned short DOMWriter::getFormat() const
{
	return m_format;
}


void DOMWriter::setNewLine(const XMLString& newLine)
{
	if (newLine.empty() 
	    || newLine == NEWLINE_CR
	    || newLine == NEWLINE_CRLF
	    || newLine == NEWLINE_LF)
		m_newLine = newLine;
	else
		throw DOMException(DOMException::SYNTAX_ERR);
}


const XMLString& DOMWriter::getNewLine() const
{
	return m_newLine;
}


void DOMWriter::setPrettyPrint(bool enable)
{
	m_format = enable ? REFORMATTED : AS_IS;
}


void DOMWriter::writeNode(XMLByteOutputStream* pOstr, const Node* pNode)
{
	if (pNode->getNodeType() == Node::DOCUMENT_NODE)
	{
		writeNode(pOstr, static_cast<const Document*>(pNode));
	}
	else
	{
		CSAXDOMParser parser;
		CXMLWriter2   writer(*pOstr, m_encoding, (m_format & CANONICAL) == 0, m_encoding == ENC_UTF16, m_assumeISO88591);
		writer.SetPrettyPrint((m_format & REFORMATTED) != 0);
		if (m_newLine == NEWLINE_CR)
			writer.SetLineEnding(CXMLWriter2::LE_MAC);
		else if (m_newLine == NEWLINE_LF)
			writer.SetLineEnding(CXMLWriter2::LE_UNIX);
		else if (m_newLine == NEWLINE_CRLF)
			writer.SetLineEnding(CXMLWriter2::LE_DOS);
		parser.SetContentHandler(&writer);
		parser.SetProperty(SAX_LEXICALHANDLER, static_cast<CLexicalHandler*>(&writer));
		writer.StartFragment();
		parser.Parse(pNode);
		writer.EndFragment();
	}
}


void DOMWriter::writeNode(XMLByteOutputStream* pOstr, const Document* pDocument)
{
	CSAXDOMParser parser;
	CXMLWriter2   writer(*pOstr, m_encoding, (m_format & CANONICAL) == 0, m_encoding == ENC_UTF16, m_assumeISO88591);
	writer.SetPrettyPrint((m_format & REFORMATTED) != 0);
	if (m_newLine == NEWLINE_CR)
		writer.SetLineEnding(CXMLWriter2::LE_MAC);
	else if (m_newLine == NEWLINE_LF)
		writer.SetLineEnding(CXMLWriter2::LE_UNIX);
	else if (m_newLine == NEWLINE_CRLF)
		writer.SetLineEnding(CXMLWriter2::LE_DOS);
	DocumentType* pDocType = pDocument->getDoctype();
	if (pDocType) writer.SetDocumentType(pDocType->getPublicId(), pDocType->getSystemId());
	parser.SetContentHandler(&writer);
	parser.SetProperty(SAX_LEXICALHANDLER, static_cast<CLexicalHandler*>(&writer));
	parser.Parse(pDocument);
}


void DOMWriter::writeNode(const XMLString& systemId, const Node* pNode)
{
	ofstream str;

	str.open(CXMLUtils::XMLToAscii(systemId).c_str());
	if (str.good())
	{
		writeNode(&str, pNode);
	}
	else
	{
#ifdef CSL_XML_STANDALONE
		throw CXMLException(CXMLException::EXMLBadSystemId, CXMLUtils::XMLToAscii(systemId));
#else
		using CSL::Base::CException;
		throw CException(CException::ECreateFile, CXMLUtils::XMLToAscii(systemId));
#endif
	}
	str.close();
}


XML_END

