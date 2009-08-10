//
// XMLWriter2.cpp
//
// $Id: //depot/XML/Main/src/XMLWriter2.cpp#19 $
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


#include "XMLWriter2.h"
#include "XMLUtils.h"
#include "XMLException.h"
#include "SAX/AttributesImpl.h"
#include <stdio.h>


XML_BEGIN


#ifdef XML_UNICODE_WCHAR_T
static const XMLChar amp  = L'&';
static const XMLChar lt   = L'<';
static const XMLChar gt   = L'>';
static const XMLChar quot = L'"';
static const XMLChar apos = L'\'';
static const XMLChar tab  = L'\t';
static const XMLChar lf   = L'\n';
static const XMLChar cr   = L'\r';
#else
static const XMLChar amp  = '&';
static const XMLChar lt   = '<';
static const XMLChar gt   = '>';
static const XMLChar quot = '"';
static const XMLChar apos = '\'';
static const XMLChar tab  = '\t';
static const XMLChar lf   = '\n';
static const XMLChar cr   = '\r';
#endif


const string CXMLWriter2::MARKUP_QUOTENC = "&quot;";
const string CXMLWriter2::MARKUP_APOSENC = "&apos;";
const string CXMLWriter2::MARKUP_AMPENC  = "&amp;";
const string CXMLWriter2::MARKUP_LTENC   = "&lt;";
const string CXMLWriter2::MARKUP_GTENC   = "&gt;";
const string CXMLWriter2::MARKUP_LT      = "<";
const string CXMLWriter2::MARKUP_COLON   = ":";
const string CXMLWriter2::MARKUP_SPACE   = " ";
const string CXMLWriter2::MARKUP_EQQUOT  = "=\"";
const string CXMLWriter2::MARKUP_QUOT    = "\"";
const string CXMLWriter2::MARKUP_GT      = ">";
const string CXMLWriter2::MARKUP_SLASHGT = "/>";
const string CXMLWriter2::MARKUP_GTSLASH = "</";
const string CXMLWriter2::MARKUP_TAB     = "\t";


#ifdef XML_UNICODE
#define UNSIGNED_XMLCHAR(c) (unsigned) (c)
#else
#define UNSIGNED_XMLCHAR(c) (unsigned) (unsigned char) (c)
#endif


//
// Transcoder
//


class NilTranscoder: public CXMLWriter2::Transcoder
{
public:
	void Transcode(unsigned chr, vector<unsigned char>& target)
	{
		target.push_back((unsigned char) chr);
	}
};


class UTF16toUTF16Transcoder: public CXMLWriter2::Transcoder
{
public:
	void Transcode(unsigned chr, vector<unsigned char>& target)
	{
		target.push_back((unsigned char) ((chr >> 8) & 0xFF));
		target.push_back((unsigned char) (chr & 0xFF));
	}
};


class UTF8toUTF16Transcoder: public CXMLWriter2::Transcoder
{
private:
	unsigned m_comp;
	unsigned m_complen;

public:
	UTF8toUTF16Transcoder()
	{
		m_comp    = 0;
		m_complen = 0;
	}

	void Transcode(unsigned chr, vector<unsigned char>& target)
	{
		if (chr <= 0x7F)
		{
			target.push_back(0);
			target.push_back((unsigned char) chr);
			m_comp    = 0;
			m_complen = 0;
		}
		else if ((chr & 0xFC) == 0xFC)
		{
			m_comp    = chr & 0x01;
			m_complen = 5;
		}
		else if ((chr & 0xF8) == 0xF8)
		{
			m_comp    = chr & 0x03;
			m_complen = 4;
		}
		else if ((chr & 0xF0) == 0xF0)
		{
			m_comp    = chr & 0x07;
			m_complen = 3;
		}
		else if ((chr & 0xE0) == 0xE0)
		{
			m_comp    = chr & 0x0F;
			m_complen = 2;
		}
		else if ((chr & 0xC0) == 0xC0)
		{
			m_comp    = chr & 0x1F;
			m_complen = 1;
		}
		else 
		{
			m_comp <<= 6;
			m_comp |= (chr & 0x3F);
			if (--m_complen == 0)
			{
				target.push_back((unsigned char) ((m_comp >> 8) & 0xFF));
				target.push_back((unsigned char) (m_comp & 0xFF));
			}
		}
	}
};


class UTF8toISO88591Transcoder: public CXMLWriter2::Transcoder
{
private:
	unsigned m_comp;
	unsigned m_complen;

public:
	UTF8toISO88591Transcoder()
	{
		m_comp    = 0;
		m_complen = 0;
	}

	void Transcode(unsigned chr, vector<unsigned char>& target)
	{
		if (chr <= 0x7F)
		{
			target.push_back((unsigned char) chr);
			m_comp    = 0;
			m_complen = 0;
		}
		else if ((chr & 0xFC) == 0xFC)
		{
			m_comp    = chr & 0x01;
			m_complen = 5;
		}
		else if ((chr & 0xF8) == 0xF8)
		{
			m_comp    = chr & 0x03;
			m_complen = 4;
		}
		else if ((chr & 0xF0) == 0xF0)
		{
			m_comp    = chr & 0x07;
			m_complen = 3;
		}
		else if ((chr & 0xE0) == 0xE0)
		{
			m_comp    = chr & 0x0F;
			m_complen = 2;
		}
		else if ((chr & 0xC0) == 0xC0)
		{
			m_comp    = chr & 0x1F;
			m_complen = 1;
		}
		else 
		{
			m_comp <<= 6;
			m_comp |= (chr & 0x3F);
			if (--m_complen == 0)
			{
				target.push_back((unsigned char) (m_comp <= 0xFF ? m_comp : '?'));
			}
		}
	}
};


class UTF16toUTF8Transcoder: public CXMLWriter2::Transcoder
{
public:
	void Transcode(unsigned chr, vector<unsigned char>& target)
	{
		if (chr <= 0x7F)
		{
			target.push_back((unsigned char) chr);
		}
		else if (chr <= 0x7FF)
		{
			target.push_back((unsigned char) ((chr >> 6) & 0x1F | 0xC0));
			target.push_back((unsigned char) ((chr & 0x3F) | 0x80));
		}
		else if (chr <= 0xFFFF)
		{
			target.push_back((unsigned char) ((chr >> 12) & 0x0F | 0xE0));
			target.push_back((unsigned char) ((chr >> 6) & 0x3F | 0x80));
			target.push_back((unsigned char) ((chr & 0x3F) | 0x80));
		}
		else if (chr <= 0x1FFFFF)
		{
			target.push_back((unsigned char) ((chr >> 18) & 0x07 | 0xF0));
			target.push_back((unsigned char) ((chr >> 12) & 0x3F | 0x80));
			target.push_back((unsigned char) ((chr >> 6) & 0x3F | 0x80));
			target.push_back((unsigned char) ((chr & 0x3F) | 0x80));
		}
		else if (chr <= 0x3FFFFFF)
		{
			target.push_back((unsigned char) ((chr >> 24) & 0x03 | 0xF8));
			target.push_back((unsigned char) ((chr >> 18) & 0x3F | 0x80));
			target.push_back((unsigned char) ((chr >> 12) & 0x3F | 0x80));
			target.push_back((unsigned char) ((chr >> 6) & 0x3F | 0x80));
			target.push_back((unsigned char) ((chr & 0x3F) | 0x80));
		}
		else if (chr <= 0x7FFFFFFF)
		{
			target.push_back((unsigned char) ((chr >> 30) & 0x01 | 0xFC));
			target.push_back((unsigned char) ((chr >> 24) & 0x3F | 0x80));
			target.push_back((unsigned char) ((chr >> 18) & 0x3F | 0x80));
			target.push_back((unsigned char) ((chr >> 12) & 0x3F | 0x80));
			target.push_back((unsigned char) ((chr >> 6) & 0x3F | 0x80));
			target.push_back((unsigned char) ((chr & 0x3F) | 0x80));
		}
	}
};


class UTF16toISO88591Transcoder: public CXMLWriter2::Transcoder
{
public:
	void Transcode(unsigned chr, vector<unsigned char>& target)
	{
		target.push_back((unsigned char) (chr <= 0xFF ? chr : '?'));
	}
};


class ISO88591toUTF8Transcoder: public CXMLWriter2::Transcoder
{
public:
	void Transcode(unsigned chr, vector<unsigned char>& target)
	{
		if (chr <= 0x7F)
		{
			target.push_back((unsigned char) chr);
		}
		else
		{
			target.push_back((unsigned char) ((chr >> 6) & 0x1F | 0xC0));
			target.push_back((unsigned char) ((chr & 0x3F) | 0x80));
		}
	}
};


class ISO88591toUTF16Transcoder: public CXMLWriter2::Transcoder
{
public:
	void Transcode(unsigned chr, vector<unsigned char>& target)
	{
		target.push_back((unsigned char) ((chr >> 8) & 0xFF));
		target.push_back((unsigned char) (chr & 0xFF));
	}
};


CXMLWriter2::PendingNS::PendingNS()
{
}


CXMLWriter2::PendingNS::PendingNS(const XMLString& prefix, const XMLString& uri)
{
	m_prefix = prefix;
	m_uri    = uri;
}


void CXMLWriter2::Transcoder::Transcode(const XMLString& str, vector<unsigned char>& target)
{
	target.reserve(target.size() + str.size()*2);
	XMLString::const_iterator it  = str.begin();
	XMLString::const_iterator end = str.end();
	while (it != end)
	{
		Transcode(UNSIGNED_XMLCHAR(*it++), target);
	}
}


//
// XMLName
//


CXMLWriter2::XMLName::XMLName()
{
}


CXMLWriter2::XMLName::XMLName(const XMLString& uri, const XMLString& localName, const XMLString& qname)
{
	m_uri       = uri;
	m_localName = localName;
	m_qname     = qname;
}


//
// CXMLWriter2
//


CXMLWriter2::CXMLWriter2(XMLByteOutputStream& str, bool writeXMLDeclaration):
             m_str(str),
			 m_writeXMLDecl(writeXMLDeclaration)
{
	m_level        = -1;
	m_nElems       = 0;
	m_inCDATA      = false;
	m_dataWritten  = false;
	m_prettyPrint  = false;
	m_writeBOM     = false;
#ifdef XML_UNICODE
	m_pTranscoder = new UTF16toUTF8Transcoder;
#else
	m_pTranscoder = new NilTranscoder;
#endif

#ifdef _WIN32
	SetLineEnding(LE_DOS);
#else
	SetLineEnding(LE_UNIX);
#endif
	m_needFinalizeStartElement = false;
}


CXMLWriter2::CXMLWriter2(XMLByteOutputStream& str, const XMLString& encoding, bool writeXMLDeclaration, bool writeBOM, bool assumeISO88591):
             m_str(str),
			 m_encoding(encoding),
			 m_writeXMLDecl(writeXMLDeclaration),
			 m_writeBOM(writeBOM)
{
	m_level       = -1;
	m_nElems      = 0;
	m_inCDATA     = false;
	m_dataWritten = false;
	m_prettyPrint = false;
#ifdef XML_UNICODE
	if (m_encoding == CXMLUtils::AsciiToXML("UTF-8") || encoding.empty())
		m_pTranscoder = new UTF16toUTF8Transcoder;
	else if (m_encoding == CXMLUtils::AsciiToXML("ISO-8859-1"))
		m_pTranscoder = new UTF16toISO88591Transcoder;
	else
		m_pTranscoder = new UTF16toUTF16Transcoder;
#else
	if (m_encoding == CXMLUtils::AsciiToXML("UTF-16") && !assumeISO88591)
		m_pTranscoder = new UTF8toUTF16Transcoder;
	else if (m_encoding == CXMLUtils::AsciiToXML("UTF-8") && !assumeISO88591)
		m_pTranscoder = new NilTranscoder;
	else if (m_encoding == CXMLUtils::AsciiToXML("ISO-8859-1") && !assumeISO88591)
		m_pTranscoder = new UTF8toISO88591Transcoder;
	else if (m_encoding == CXMLUtils::AsciiToXML("UTF-16"))
		m_pTranscoder = new ISO88591toUTF16Transcoder;
	else if (m_encoding == CXMLUtils::AsciiToXML("UTF-8"))
		m_pTranscoder = new ISO88591toUTF8Transcoder;
	else
		m_pTranscoder = new NilTranscoder;
#endif

#ifdef _WIN32
	SetLineEnding(LE_DOS);
#else
	SetLineEnding(LE_UNIX);
#endif
	m_needFinalizeStartElement = false;
}


CXMLWriter2::~CXMLWriter2()
{
	delete m_pTranscoder;
}


void CXMLWriter2::SetDocumentLocator(const CLocator& loc)
{
}


void CXMLWriter2::SetDocumentType(const XMLString& publicId, const XMLString& systemId)
{
	m_publicId = publicId;
	m_systemId = systemId;
}


void CXMLWriter2::StartDocument()
{
	if (m_level != -1)
	{
		throw CXMLException(CXMLException::EXMLSyntax, "StartDocument() called for the second time");
	}
	m_level  = 0;
	m_nElems = 0;

	if (m_writeBOM) WriteBOM();
	if (m_writeXMLDecl)
	{
		// write the XML header
		WriteMarkup("<?xml version=\"1.0\"");
		if (m_encoding.length() > 0)
		{
			WriteMarkup(" encoding=\"");
			WriteXML(m_encoding);
			WriteMarkup("\"");
		}
		WriteMarkup("?>");
		WriteNL();
		if (!m_publicId.empty())
		{
			WriteMarkup("<!DOCTYPE ");
			WriteXML(m_publicId);
			WriteMarkup(" SYSTEM \"");
			WriteXML(m_systemId);
			WriteMarkup("\">");
			WriteNL();
		}
	}
	CheckStream();
	m_dataWritten = true; // avoids an additional newline when pretty printing is active
	m_pendingNS.clear();
}


void CXMLWriter2::EndDocument()
{
	if (m_level > 0)
	{
		throw CXMLException(CXMLException::EXMLSyntax, "at least one StartElement() has no matching EndElement()");
	}
	m_nElems = 0;
	m_level  = -1;
	WriteNL();
}


void CXMLWriter2::StartFragment()
{
	if (m_level != -1)
	{
		throw CXMLException(CXMLException::EXMLSyntax, "StartFragment() called for the second time");
	}
	m_level  = 1;
	m_nElems = 0;
	m_pendingNS.clear();
	if (m_writeBOM) WriteBOM();
}


void CXMLWriter2::EndFragment()
{
	if (m_level > 1)
	{
		throw CXMLException(CXMLException::EXMLSyntax, "at least one StartElement() has no matching EndElement()");
	}
	m_nElems = 0;
	m_level  = -1;
	WriteNL();
}


void CXMLWriter2::StartElement(const XMLString& uri, const XMLString& localName, const XMLString& qname)
{
	CAttributesImpl attrs;
	StartElement(uri, localName, qname, attrs);
}


void CXMLWriter2::StartElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const CAttributes& attrList)
{
	if (m_level == 0 && m_nElems > 1)
	{
		throw CXMLException(CXMLException::EXMLSyntax, "More than one root element");
	}
	if (m_needFinalizeStartElement) FinalizeStartElement();
	PrettyPrint();
	m_level++;
	m_nElems++;
	WriteStartElement(uri, localName, qname, attrList, false);
	CheckStream();
	m_elemStack.push_back(XMLName(uri, localName, qname));
	m_dataWritten = false;
}


void CXMLWriter2::EndElement(const XMLString& uri, const XMLString& localName, const XMLString& qname)
{
	if (m_level == 0)
	{
		throw CXMLException(CXMLException::EXMLSyntax, "EndElement() has no matching StartElement()");
	}
	if (!(qname == m_elemStack.back().m_qname || uri == m_elemStack.back().m_uri && localName == m_elemStack.back().m_localName))
	{
		throw CXMLException(CXMLException::EXMLTagMismatch);
	}
	m_elemStack.pop_back();
	m_level--;
	if (!m_needFinalizeStartElement) PrettyPrint();
	WriteEndElement(uri, localName, qname);
	if (m_level == 0)
	{
		WriteNL();
	}
	CheckStream();
	m_dataWritten = false;
}


void CXMLWriter2::EmptyElement(const XMLString& uri, const XMLString& localName, const XMLString& qname)
{
	CAttributesImpl attr;
	EmptyElement(uri, localName, qname, attr);
}


void CXMLWriter2::EmptyElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const CAttributes& attrList)
{
	if (m_level == 0 && m_nElems > 1)
	{
		throw CXMLException(CXMLException::EXMLSyntax, "More than one root element");
	}
	if (m_needFinalizeStartElement) FinalizeStartElement();
	PrettyPrint();
	m_nElems++;
	WriteStartElement(uri, localName, qname, attrList, true);
	CheckStream();
	m_dataWritten = false;
}


void CXMLWriter2::Characters(const XMLChar ch[], int start, int length)
{
	if (m_needFinalizeStartElement) FinalizeStartElement();
	if (m_inCDATA)
	{
		for (register int i = 0; i < length; i++, start++)
		{
			WriteXML(ch[start]);
		}
	}
	else
	{
		for (register int i = 0; i < length; i++, start++)
		{
			register unsigned char c = (unsigned char) ch[start];
			if (c == quot)
				WriteMarkup(MARKUP_QUOTENC);
			else if (c == apos)
				WriteMarkup(MARKUP_APOSENC);
			else if (c == amp)
				WriteMarkup(MARKUP_AMPENC);
			else if (c == lt)
				WriteMarkup(MARKUP_LTENC);
			else if (c == gt)
				WriteMarkup(MARKUP_GTENC);
			else if (c == tab || c == cr || c == lf || c >= 0x20)
				WriteXML(c);
			else
				throw CXMLException(CXMLException::EXMLInvalidToken);
		}
	}
	CheckStream();
	m_dataWritten = true;
}


void CXMLWriter2::Characters(const XMLString& str)
{
	Characters(str.c_str(), 0, str.length());
}


void CXMLWriter2::IgnorableWhitespace(const XMLChar ch[], int start, int length)
{
	Characters(ch, start, length);
}


void CXMLWriter2::ProcessingInstruction(const XMLString& target, const XMLString& data)
{
	if (m_needFinalizeStartElement) FinalizeStartElement();
	PrettyPrint();
	WriteMarkup("<?");
	WriteXML(target);
	if (data.length() > 0)
	{
		WriteMarkup(MARKUP_SPACE);
		WriteXML(data);
	}
	WriteMarkup("?>");
	if (m_level == 0) WriteNL();
	CheckStream();
}


void CXMLWriter2::DataElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, 
                             const XMLString& data, 
	                         const XMLString& att1, const XMLString& val1,
							 const XMLString& att2, const XMLString& val2,
							 const XMLString& att3, const XMLString& val3)
{
	CAttributesImpl attr;
	if (!att1.empty()) attr.AddAttribute(XMLString(), XMLString(), att1, CXMLUtils::AsciiToXML("CDATA"), val1);
	if (!att2.empty()) attr.AddAttribute(XMLString(), XMLString(), att2, CXMLUtils::AsciiToXML("CDATA"), val2);
	if (!att3.empty()) attr.AddAttribute(XMLString(), XMLString(), att3, CXMLUtils::AsciiToXML("CDATA"), val3);
	if (data.empty())
	{
		EmptyElement(uri, localName, qname, attr);
	}
	else
	{
		StartElement(uri, localName, qname, attr);
		Characters(data);
		EndElement(uri, localName, qname);
	}
}



void CXMLWriter2::StartPrefixMapping(const XMLString& prefix, const XMLString& uri)
{
	m_nsSupport.PushContext();
	m_nsSupport.DeclarePrefix(prefix, uri);
	m_pendingNS.push_back(PendingNS(prefix, uri));
}


void CXMLWriter2::EndPrefixMapping(const XMLString& prefix)
{
	m_nsSupport.PopContext();
}


void CXMLWriter2::SkippedEntity(const XMLString& name)
{
}


void CXMLWriter2::StartCDATA()
{
	if (m_needFinalizeStartElement) FinalizeStartElement();
	m_inCDATA = true;
	WriteMarkup("<![CDATA[");
	CheckStream();
}


void CXMLWriter2::EndCDATA()
{
	m_inCDATA = false;
	WriteMarkup("]]>");
	CheckStream();
}


void CXMLWriter2::Comment(const XMLChar ch[], int start, int length)
{
	if (m_needFinalizeStartElement) FinalizeStartElement();
	PrettyPrint();
	WriteMarkup("<!--");
	for (int i = 0; i < length; i++, start++)
	{
		WriteXML(ch[start]);
	}
	WriteMarkup("-->");
	CheckStream();
	m_dataWritten = false;
}


void CXMLWriter2::StartDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId)
{
}


void CXMLWriter2::EndDTD()
{
}


void CXMLWriter2::StartEntity(const XMLString& name)
{
}


void CXMLWriter2::EndEntity(const XMLString& name)
{
}


void CXMLWriter2::PrettyPrint() const
{
	if (m_prettyPrint && !m_dataWritten)
	{
		WriteNL();
		for (int i = 0; i < m_level; ++i) WriteTab();
	}
	CheckStream();
}


void CXMLWriter2::SetPrettyPrint(bool enable)
{
	m_prettyPrint = enable;
}


void CXMLWriter2::SetLineEnding(CXMLWriter2::LineEnding ending)
{
	switch (ending)
	{
	case LE_DOS:
		m_lineEnding = "\r\n"; break;
	case LE_UNIX:
		m_lineEnding = "\n"; break;
	case LE_MAC:
		m_lineEnding = "\r"; break;
	}
}


void CXMLWriter2::CheckStream() const
{
	if (m_str.bad()) throw CXMLException(CXMLException::EXMLIOError);
}


void CXMLWriter2::WriteStartElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const CAttributes& attrList, bool empty)
{
	m_nsSupport.PushContext();

	WriteMarkup(MARKUP_LT);
	if (!localName.empty() && qname.empty())
	{
		XMLString prefix = m_nsSupport.GetPrefix(uri);
		if (prefix.empty())
		{
			WriteXML(localName);
		}	
		else
		{
			WriteXML(prefix); WriteMarkup(MARKUP_COLON); WriteXML(localName);
		}
	}
	else if (uri.empty() && localName.empty() && !qname.empty())
	{
		WriteXML(qname);
	}
	else if (!localName.empty() && !qname.empty())
	{
		XMLString sLocalName;
		XMLString prefix;
		SplitName(qname, prefix, sLocalName);
		if (prefix.empty()) prefix = m_nsSupport.GetPrefix(uri);
		const XMLString& sUri = m_nsSupport.GetURI(prefix);
		if ((sUri.empty() || sUri != uri) && !uri.empty())
		{
			m_nsSupport.DeclarePrefix(prefix, uri);
			m_pendingNS.push_back(PendingNS(prefix, uri));
		}
		if (prefix.empty())
		{
			WriteXML(localName);
		}
		else
		{
			WriteXML(prefix); WriteMarkup(MARKUP_COLON); WriteXML(localName);
		}
	}
	else
	{
		throw CXMLException(CXMLException::EXMLTagMismatch);
	}

	int i;
	for (i = 0; i < attrList.GetLength(); i++)
	{
		XMLString sLocalName;
		XMLString prefix;
		XMLString attrUri       = attrList.GetURI(i);
		XMLString attrLocalName = attrList.GetLocalName(i);
		XMLString attrQname     = attrList.GetQName(i);
		if (!attrLocalName.empty())
		{
			SplitName(attrQname, prefix, sLocalName);
			if (prefix.empty()) prefix = m_nsSupport.GetPrefix(attrUri);
			const XMLString& sUri = m_nsSupport.GetURI(prefix);
			if ((sUri.empty() || sUri != attrUri) && !attrUri.empty())
			{
				m_nsSupport.DeclarePrefix(prefix, attrUri);
				m_pendingNS.push_back(PendingNS(prefix, attrUri));
			}
		}
	}
	for (vector<PendingNS>::const_iterator it = m_pendingNS.begin(); it != m_pendingNS.end(); ++it)
	{
		WriteMarkup(MARKUP_SPACE);
		if (it->m_prefix.empty())
		{
			WriteMarkup("xmlns");
		}
		else
		{
			WriteMarkup("xmlns:"); WriteXML(it->m_prefix);
		}
		WriteMarkup("=\""); WriteXML(it->m_uri); WriteMarkup("\"");
	}
	m_pendingNS.clear();

	for (i = 0; i < attrList.GetLength(); i++)
	{
		WriteMarkup(MARKUP_SPACE);
		
		XMLString attrUri       = attrList.GetURI(i);
		XMLString attrLocalName = attrList.GetLocalName(i);
		XMLString attrQname     = attrList.GetQName(i);
		
		if (!attrUri.empty() && !attrLocalName.empty())
		{
			XMLString prefix;
			if (attrUri != uri) prefix = m_nsSupport.GetPrefix(attrUri);
			if (prefix.empty())
			{
				WriteXML(attrLocalName);
			}
			else
			{
				WriteXML(prefix); WriteMarkup(MARKUP_COLON); WriteXML(attrLocalName);
			}
		}
		else
		{
			WriteXML(attrQname);
		}
		WriteMarkup(MARKUP_EQQUOT);
		Characters(attrList.GetValue(i));
		WriteMarkup(MARKUP_QUOT);
	}

	m_needFinalizeStartElement = !empty;
	if (empty) 
	{
		WriteMarkup(MARKUP_SLASHGT);
		m_nsSupport.PopContext();
	}
}


void CXMLWriter2::WriteEndElement(const XMLString& uri, const XMLString& localName, const XMLString& qname)
{
	if (m_needFinalizeStartElement)
	{
		WriteMarkup(MARKUP_SLASHGT);
		m_needFinalizeStartElement = false;
	}
	else
	{
		WriteMarkup(MARKUP_GTSLASH);
		if (!localName.empty())
		{
			XMLString prefix = m_nsSupport.GetPrefix(uri);
			if (prefix.empty())
			{
				WriteXML(localName);
			}
			else
			{
				WriteXML(prefix); WriteMarkup(MARKUP_COLON); WriteXML(localName);
			}
		}
		else
		{
			WriteXML(qname);
		}
		WriteMarkup(MARKUP_GT);
	}
	m_nsSupport.PopContext();
}


void CXMLWriter2::FinalizeStartElement()
{
	m_needFinalizeStartElement = false;
	WriteMarkup(MARKUP_GT);
}


void CXMLWriter2::SplitName(const XMLString& qname, XMLString& prefix, XMLString& localName)
{
	XMLString::size_type pos = qname.find_first_of(':');
	if (pos != XMLString::npos) 
	{
		prefix.assign(qname, 0, pos);
		localName.assign(qname, pos + 1, qname.size() - pos - 1);
	}
	else
	{
		prefix.erase();
		localName.assign(qname);
	}
}


void CXMLWriter2::WriteBytes(const vector<unsigned char>& bytes) const
{
	vector<unsigned char>::const_iterator it  = bytes.begin();
	vector<unsigned char>::const_iterator end = bytes.end();
	while (it != end) m_str.put(*it++);
}


void CXMLWriter2::WriteMarkup(const string& str) const
{
	vector<unsigned char> codedBytes;
	codedBytes.reserve(str.size() * 2);
	string::const_iterator it  = str.begin();
	string::const_iterator end = str.end();
	while (it != end) m_pTranscoder->Transcode(UNSIGNED_XMLCHAR(*it++), codedBytes);
	WriteBytes(codedBytes);
}


void CXMLWriter2::WriteXML(const XMLString& str) const
{
	vector<unsigned char> codedBytes;
	codedBytes.reserve(str.size() * 2);
	m_pTranscoder->Transcode(str, codedBytes);
	WriteBytes(codedBytes);
}


void CXMLWriter2::WriteXML(XMLChar ch) const
{
	vector<unsigned char> codedBytes;
	m_pTranscoder->Transcode(UNSIGNED_XMLCHAR(ch), codedBytes);
	WriteBytes(codedBytes);
}


void CXMLWriter2::WriteNL() const
{
	WriteMarkup(m_lineEnding);
}


void CXMLWriter2::WriteTab() const
{
	WriteMarkup(MARKUP_TAB);
}


void CXMLWriter2::WriteBOM() const
{
	vector<unsigned char> codedBytes;
	m_pTranscoder->Transcode(0xFFFE, codedBytes);
	WriteBytes(codedBytes);
}


XML_END
