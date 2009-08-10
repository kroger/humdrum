//
// DOMBuilder.h
//
// $Id: //depot/XML/Main/include/DOM/DOMBuilder.h#7 $
//
// Document Object Model (Core) Level 1
//
// Interface DOMBuilder
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


#ifndef DOM_DOMBuilder_H_INCLUDED
#define DOM_DOMBuilder_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_ContentHandler_H_INCLUDED
#include "SAX/ContentHandler.h"
#endif
#ifndef SAX_LexicalHandler_H_INCLUDED
#include "SAX/LexicalHandler.h"
#endif
#ifndef SAX_DTDHandler_H_INCLUDED
#include "SAX/DTDHandler.h"
#endif
#ifndef SAX2Parser_H_INCLUDED
#include "SAX2Parser.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class Document;
class CEntityResolver;
class CErrorHandler;
class SimpleNode;
class ContainerNode;
class XMLNamePool;


class XML_API DOMBuilder: public CDTDHandler, public CContentHandler, public CLexicalHandler
	//: This class builds a tree representation of a
	//+ XML document, according to the W3C DOM
	//. specification.
{
public:
	DOMBuilder(XMLNamePool* pNamePool = NULL);
	DOMBuilder(const XMLString& encoding, XMLNamePool* pNamePool = NULL);
		//; Sets the encoding for the underlying SAX parser.

	virtual ~DOMBuilder();

	virtual void setFeature(const XMLString& name, bool state);
		//: Set the state of a feature.
		//+ The feature name has the same form as a DOM hasFeature string.
		//. It is possible for a DOMBuilder to recognize a feature name but to be unable to set its value

	virtual bool supportsFeature(const XMLString& name) const;
		//: Query whether the DOMBuilder recognizes a feature name.
		//+ The feature name has the same form as a DOM hasFeature string.
		//+ It is possible for a DOMBuilder to recognize a feature name but to 
		//+ be unable to set its value. For example, a non-validating
		//+ parser would recognize the feature "validation", would report that its 
		//+ value was false, and would raise an exception if an
		//. attempt was made to enable validation by setting the feature to true.

	virtual bool getFeature(const XMLString& name) const;
		//: Look up the value of a feature.
		//. The feature name has the same form as a DOM hasFeature string

	virtual bool canSetFeature(const XMLString& name, bool state);
		//: Query whether setting a feature is supported.
		//+ The feature name has the same form as a DOM hasFeature string.
		//. It is possible for a DOMBuilder to recognize a feature name but to be unable to set its value. 

	virtual Document* parseURI(const XMLString& uri);
		//: Parse an XML document from a location identified by an URI.
		//. The URI must denote a local file.

	virtual Document* parseDOMInputSource(CInputSource* pInputSource);
		//: Parse an XML document from a location identified by an DOMInputSource.
		//. In this implementation, DOMInputSource is a CInputSource.

	virtual CEntityResolver* getEntityResolver() const;
		//; Returns the entity resolver.

	virtual void setEntityResolver(CEntityResolver* pEntityResolver);
		//; Sets the entity resolver.

	virtual CErrorHandler* getErrorHandler() const;
		//; Returns the error handler.

	virtual void setErrorHandler(CErrorHandler* pErrorHandler);
		//; Sets the error handler.

protected:
	// CDTDHandler
	virtual void NotationDecl(const XMLString& name, const XMLString* publicId, const XMLString* systemId);
	virtual void UnparsedEntityDecl(const XMLString& name, const XMLString* publicId, const XMLString& systemId, const XMLString& notationName);

	// CContentHandler
	virtual void SetDocumentLocator(const CLocator& loc);
	virtual void StartDocument();
	virtual void EndDocument();
	virtual void StartElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const CAttributes& attrList);
	virtual void EndElement(const XMLString& uri, const XMLString& localName, const XMLString& qname);
	virtual void Characters(const XMLChar ch[], int start, int length);
	virtual void IgnorableWhitespace(const XMLChar ch[], int start, int length);
	virtual void ProcessingInstruction(const XMLString& target, const XMLString& data);
	virtual void StartPrefixMapping(const XMLString& prefix, const XMLString& uri);
	virtual void EndPrefixMapping(const XMLString& prefix);
	virtual void SkippedEntity(const XMLString& prefix);

	// CLexicalHandler
	virtual void StartDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId);
	virtual void EndDTD();
	virtual void StartEntity(const XMLString& name);
	virtual void EndEntity(const XMLString& name);
	virtual void StartCDATA();
	virtual void EndCDATA();
	virtual void Comment(const XMLChar ch[], int start, int length); 

	bool IsWhiteSpace(const XMLString& str);

private:
	static const XMLString EMPTY_STRING;
	static const XMLString FEATURE_NAMESPACES;

	CSAX2Parser    m_parser;
	Document*      m_pDoc;
	ContainerNode* m_pParent;
	SimpleNode*    m_pPrev;
	bool           m_inCDATA;
	bool           m_namespaces;
	bool           m_whiteSpaceInContent;
	XMLNamePool*   m_pNamePool;
};


XML_END


#endif // DOM_DOMBuilder_H_INCLUDED
