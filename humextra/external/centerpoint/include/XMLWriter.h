//
// XMLWriter.h
//
// $Id: //depot/XML/Main/include/XMLWriter.h#6 $
//
// Definition of CXMLWriter.
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


#ifndef XMLWriter_H_INCLUDED
#define XMLWriter_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_DocumentHandler_H_INCLUDED
#include "SAX/DocumentHandler.h"
#endif
#ifndef SAX_LexicalHandler_H_INCLUDED
#include "SAX/LexicalHandler.h"
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
class CXMLWriter2;


class XML_API CXMLWriter: public CDocumentHandler, public CLexicalHandler
	//: This class serializes SAX DocumentHandler/LexicalHandler 
	//+ events back into XML streams.
	//+ A few limited consistency checks are performed
	//+ (i.e. there can't be more than one root element
	//+ and every StartElement() must have a matching
	//+ EndElement()).
	//+ Basic pretty-printing support is available, too.
	//. This class is now just a compatibility wrapper for CXMLWriter2.
{
public:
	CXMLWriter(XMLByteOutputStream& str);
	CXMLWriter(XMLByteOutputStream& str, const XMLString& encoding);
	CXMLWriter(XMLByteOutputStream& str, const XMLString& encoding, bool writeXMLDeclaration, bool writeBOM = false, bool assumeISO88591 = false);
		//; The encoding is reflected in the XML declaration.

	virtual ~CXMLWriter();

	// CDocumentHandler
	virtual void SetDocumentLocator(const CLocator& loc);
		//; Currently unused.

	void SetDocumentType(const XMLString& publicId, const XMLString& systemId);
		//: Sets the public and system ID of the document's DTD.
		//. Causes a DOCTYPE definition to be written.

	void SetPrettyPrint(bool enable = true);
		//; Turns pretty printing on or off

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

	virtual void StartElement(const XMLString& name);
	virtual void StartElement(const XMLString& name, const CAttributeList& attrList);
		//; Writes an XML start element tag

	virtual void EndElement(const XMLString& name);
		//: Writes an XML end element tag.
		//+ Throws an exception if the name of doesn't match the
		//. last opened element.

	virtual void EmptyElement(const XMLString& name);
	virtual void EmptyElement(const XMLString& name, const CAttributeList& attrList);
		//; Writes an empty XML element tag (<elem/>)

	virtual void Characters(const XMLChar ch[], int start, int length);
		//: Writes XML character data. quotes, ampersand's, less-than and
		//. greater-than signs are escaped.

	virtual void Characters(const XMLString& str);
		//: Writes XML character data (convenience method, not part of the 
		//. DocumentHandler interface!)

	virtual void IgnorableWhitespace(const XMLChar ch[], int start, int length);
		//; Writes whitespace (invokes Characters())

	virtual void ProcessingInstruction(const XMLString& target, const XMLString& data);
		//; Writes a processing instruction

	virtual void DataElement(const XMLString& name, const XMLString& data, 
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

private:
	CXMLWriter2* m_pWriter;
};


XML_END


#endif // XMLWriter_H_INCLUDED
