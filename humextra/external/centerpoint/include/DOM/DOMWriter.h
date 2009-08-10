//
// DOMWriter.h
//
// $Id: //depot/XML/Main/include/DOM/DOMWriter.h#8 $
//
// Definition of class DOMWriter.
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


#ifndef DOM_DOMWriter_H_INCLUDED
#define DOM_DOMWriter_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif
#ifndef XMLStream_H_INCLUDED
#include "XMLStream.h"
#endif


XML_BEGIN


class Node;
class Document;


class XML_API DOMWriter
	//: This class serializes a DOM tree back into
	//+ an XML stream. It basically privides a convenient
	//+ encapsulation for a CSAXDOMParser and a CXMLWriter.
	//+ The interface is based on the DOM Level 3 Object Model 
	//. Load and Save draft specification from 19-Apr-2001.
{
public:
	enum Format
	{
		AS_IS       = 1,
		CANONICAL   = 2,
		REFORMATTED = 4
	};

	DOMWriter();
	~DOMWriter();

	void setEncoding(const XMLString& encoding);
		//; Sets the encoding, which will be reflected in the written XML declaration.

	void setEncoding(const XMLString& encoding, bool assumeISO88591);
		//: Sets the encoding, which will be reflected in the written XML declaration.
		//+ If assumeISO88591 is true, the writer assumes that the DOM document
		//. uses ISO 8859-1 encoding.

	const XMLString& getEncoding() const;
		//; Returns the encoding set with setEncoding.

	const XMLString& getLastEncoding() const;
		//; Returns the encoding that was last used.

	void setFormat(unsigned short format);
		//: Sets formatting options for writing XML.
		//+ Possible values are:
		//+
		//+ AS_IS: no reformatting whatever is applied.
		//+
		//+ CANONICAL: no XML declaration is written.
		//+ (DOM Level 3 CMLS specifies other things as well,
		//+ but these are currently unimplemented).
		//+
		//+ REFORMATTED: enable XML pretty printing.
		//+
		//+ CANONICAL and REFORMATTED may be combined to create
		//. pretty-printed XML without an XML declaration.

	unsigned short getFormat() const;
		//; Returns the formatting options set with setFormat().

	void setNewLine(const XMLString& newLine);
		//: Sets the newline character(s) used for terminating lines.
		//+ Possible values are:
		//+
		//+ "" (empty string): use default (CRLF on windows, LF everywhere else)
		//+
		//+ "CR":    (Macintosh style)
		//+
		//+ "CR-LF": (DOS style)
		//+
		//. "LF":    (UNIX style)

	const XMLString& getNewLine() const;
		//; Returns the newline character(s) set with setNewLine().

	void writeNode(XMLByteOutputStream* pOstr, const Node* pNode);
		//; Writes the node to the specified stream.

	void writeNode(XMLByteOutputStream* pOstr, const Document* pDocument);
		//; Writes the document to the specified stream.

	void writeNode(const XMLString& systemId, const Node* pNode);
		//; Writes the node to the file specified in systemId.

	// DEPRECATED methods
	void setPrettyPrint(bool enable = true);
		//: DEPRECATED. Included for backward compatibility.
		//+ Use setFormat(REFORMATTED) or setFormat(AS_IS) instead.
		//. Enables (or disables) pretty printing.

private:
	static const XMLString NEWLINE_CR;
	static const XMLString NEWLINE_CRLF;
	static const XMLString NEWLINE_LF;
	static const XMLString ENC_UTF16;
	static const XMLString SAX_LEXICALHANDLER;

	XMLString      m_encoding;
	XMLString      m_lastEncoding;
	unsigned short m_format;
	XMLString      m_newLine;
	bool           m_assumeISO88591;
};


XML_END


#endif // DOM_DOMWriter_H_INCLUDED
