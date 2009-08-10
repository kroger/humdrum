//
// XMLWriter2.h
//
// $Id: //depot/XML/Main/include/XMLWriter2.h#16 $
//
// Definition of CXMLWriter2.
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


#ifndef XMLWriter2_H_INCLUDED
#define XMLWriter2_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_ContentHandler_H_INCLUDED
#include "SAX/ContentHandler.h"
#endif
#ifndef SAX_LexicalHandler_H_INCLUDED
#include "SAX/LexicalHandler.h"
#endif
#ifndef SAX_NamespaceSupport_H_INCLUDED
#include "SAX/NamespaceSupport.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif
#ifndef XMLStream_H_INCLUDED
#include "XMLStream.h"
#endif
#ifndef STD_VECTOR_INCLUDED
#include <vector>
#define STD_VECTOR_INCLUDED
#endif


XML_BEGIN


class CLocator;


class XML_API CXMLWriter2: public CContentHandler, public CLexicalHandler
	//: This class serializes SAX2 ContentHandler/LexicalHandler 
	//+ events back into XML streams.
	//+ A few limited consistency checks are performed
	//+ (i.e. there can't be more than one root element
	//+ and every StartElement() must have a matching
	//+ EndElement()).
	//+ Basic pretty-printing support is available, too.
	//. Namespaces are supported.
{
public:
	enum LineEnding
	{
		LE_DOS,
		LE_UNIX,
		LE_MAC
	};

	CXMLWriter2(XMLByteOutputStream& str, bool writeXMLDeclaration = true);
	CXMLWriter2(XMLByteOutputStream& str, const XMLString& encoding, bool writeXMLDeclaration = true, bool writeBOM = false, bool assumeISO88591 = false);
		//; The encoding is reflected in the XML declaration.
		//+ The default encoding is UTF-8 (if XMLChar is 8 bit)
		//+ or UTF-16 (if XMLChar is 16 bit).
		//+ If writeBOM is true, a Byte Order Mark (Unicode U+FFFE) is written as first char.
		//. If assumeISO88591 is true, the writer assumes that the strings it gets are in ISO 8859-1 encoding.
	virtual ~CXMLWriter2();

	// CDocumentHandler
	virtual void SetDocumentLocator(const CLocator& loc);
		//; Currently unused.

	void SetDocumentType(const XMLString& publicId, const XMLString& systemId);
		//: Sets the public and system ID of the document's DTD.
		//. Causes a DOCTYPE definition to be written.

	void SetPrettyPrint(bool enable = true);
		//; Turns pretty printing on or off

	void SetLineEnding(LineEnding ending);
		//: Sets the line ending for the resulting XML file.
		//+ Possible values are:
		//+   LE_DOS  (CRLF),
		//+   LE_UNIX (LF),
		//.   LE_MAC  (CR)

	virtual void StartDocument();
		//: Writes a generic XML declaration to the stream.
		//+ If a document type has been set (see SetDocumentType),
		//. a DOCTYPE declaration is also written.

	virtual void EndDocument();
		//; Checks that all elements are closed and prints a final newline

	virtual void StartFragment();
		//: Use this instead of StartDocument() if you want to write
		//+ a fragment rather than a document (no XML declaration and
		//. more than one "root" element allowed)

	virtual void EndFragment();
		//; Checks that all elements are closed and prints a final newline

	virtual void StartElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const CAttributes& attrList);
	virtual void StartElement(const XMLString& uri, const XMLString& localName, const XMLString& qname);
		//: Writes an XML start element tag.
		//+ Namespace support is as follows:
		//+
		//+ 1) If a qname, but no uri and localName are given, the qname is taken as element name.
		//+
		//+ 2) If a uri and a localName, but no qname is given, and the given namespace uri has been
		//+ declared earlier, the namespace prefix for the given uri together with the localName
		//+ is taken as element name. If the namespace has not been declared, or the declared prefix
		//+ is empty (uri = default namespace), only the local name is taken as element name.
		//+
		//+ 3) If all three are given, and the namespace given in uri has not been declared, it is declared now.
		//. Otherwise, see 2.

	virtual void EndElement(const XMLString& uri, const XMLString& localName, const XMLString& qname);
		//: Writes an XML end element tag.
		//+ Throws an exception if the name of doesn't match the
		//. last opened element.

	virtual void EmptyElement(const XMLString& uri, const XMLString& localName, const XMLString& qname);
	virtual void EmptyElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const CAttributes& attrList);
		//; Writes an empty XML element tag (<elem/>)

	virtual void Characters(const XMLChar ch[], int start, int length);
		//: Writes XML character data. quotes, ampersand's, less-than and
		//+ greater-than signs are escaped.
		//+ The characters must be presented in the specified encoding.
		//+ I.e., if no encoding is specified, and thus the default UTF-8
		//. encoding is active, the characters must be presented in UTF-8.

	virtual void Characters(const XMLString& str);
		//: Writes XML character data (convenience method, not part of the 
		//. DocumentHandler interface!)

	virtual void IgnorableWhitespace(const XMLChar ch[], int start, int length);
		//; Writes whitespace (invokes Characters())

	virtual void ProcessingInstruction(const XMLString& target, const XMLString& data);
		//; Writes a processing instruction

	virtual void StartPrefixMapping(const XMLString& prefix, const XMLString& uri);
		//: Begin the scope of a prefix-URI Namespace mapping.
		//. A namespace declaration is written with the next element.

	virtual void EndPrefixMapping(const XMLString& prefix);
		//; End the scope of a prefix-URI mapping.

	virtual void SkippedEntity(const XMLString& name);

	virtual void DataElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const XMLString& data, 
	                         const XMLString& att1 = XMLString(), const XMLString& val1 = XMLString(),
							 const XMLString& att2 = XMLString(), const XMLString& val2 = XMLString(),
							 const XMLString& att3 = XMLString(), const XMLString& val3 = XMLString());
		//; Writes a data element <name att1="val1"...>data</name>

	// CLexicalHandler
	virtual void StartCDATA();
		//.:Writes the <![CDATA[ string that begins a CDATA section.
		//. Use Characters() to write the actual character data.

	virtual void EndCDATA();
		//; Writes the ]]> string that ends a CDATA section.

	virtual void Comment(const XMLChar ch[], int start, int length);
		//; Writes a comment.

	virtual void StartDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId);
		//; A no-op.

	virtual void EndDTD();
		//; A no-op.

	virtual void StartEntity(const XMLString& name);
		//; A no-op.

	virtual void EndEntity(const XMLString& name);
		//; A no-op.

	class Transcoder
	{
	public:
		void Transcode(const XMLString& str, vector<unsigned char>& target);
		virtual void Transcode(unsigned chr, vector<unsigned char>& target) = 0;
	};

protected:
	void PrettyPrint() const;
	void CheckStream() const;
	void WriteStartElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const CAttributes& attrList, bool empty);
	void WriteEndElement(const XMLString& uri, const XMLString& localName, const XMLString& qname);
	void SplitName(const XMLString& qname, XMLString& prefix, XMLString& localName);
	void WriteBytes(const vector<unsigned char>& bytes) const;
	void WriteMarkup(const string& str) const;
	void WriteXML(const XMLString& str) const;
	void WriteXML(XMLChar ch) const;
	void WriteNL() const;
	void WriteTab() const;
	void WriteBOM() const;
	void FinalizeStartElement();

private:
	struct PendingNS
	{
		PendingNS();
		PendingNS(const XMLString& prefix, const XMLString& uri);

		XMLString m_prefix;
		XMLString m_uri;
	};
	struct XMLName
	{
		XMLName();
		XMLName(const XMLString& uri, const XMLString& localName, const XMLString& qname);

		XMLString m_uri;
		XMLString m_localName;
		XMLString m_qname;
	};
	XMLByteOutputStream& m_str;
	int                  m_level;
	int                  m_nElems;
	bool                 m_inCDATA;
	bool                 m_prettyPrint;
	bool                 m_dataWritten;
	bool                 m_writeBOM;
	bool                 m_writeXMLDecl;
	XMLString            m_encoding;
	XMLString            m_publicId;
	XMLString            m_systemId;
	vector<XMLName>      m_elemStack;
	CNamespaceSupport    m_nsSupport;
	vector<PendingNS>    m_pendingNS;
	Transcoder*          m_pTranscoder;
	string               m_lineEnding;
	bool                 m_needFinalizeStartElement;

	static const string MARKUP_QUOTENC;
	static const string MARKUP_APOSENC;
	static const string MARKUP_AMPENC;
	static const string MARKUP_LTENC;
	static const string MARKUP_GTENC;
	static const string MARKUP_LT;
	static const string MARKUP_COLON;
	static const string MARKUP_SPACE;
	static const string MARKUP_EQQUOT;
	static const string MARKUP_QUOT;
	static const string MARKUP_GT;
	static const string MARKUP_SLASHGT;
	static const string MARKUP_GTSLASH;
	static const string MARKUP_TAB;
};


XML_END


#endif // XMLWriter2_H_INCLUDED
