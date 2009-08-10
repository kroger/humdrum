//
// SAXDOMParser.h
//
// $Id: //depot/XML/Main/include/SAXDOMParser.h#8 $
//
// Definition of CSAXDOMParser.
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


#ifndef SAXDOMParser_H_INCLUDED
#define SAXDOMParser_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_XMLReader_H_INCLUDED
#include "SAX/XMLReader.h"
#endif


XML_BEGIN


class Node;
class CDeclHandler;
class CLexicalHandler;
class Element;
class Text;
class Comment;
class ProcessingInstruction;
class Entity;
class CDATASection;
class Notation;
class Document;
class DocumentFragment;


class XML_API CSAXDOMParser: public CXMLReader
	//: This class provides a SAX/SAX2 (Simple API for XML) interface for
	//. iterating over an XML DOM document.
{
public:
	CSAXDOMParser();
	virtual ~CSAXDOMParser();

	virtual void SetEntityResolver(CEntityResolver* pResolver);
	virtual void SetDTDHandler(CDTDHandler* pDTDHandler);
	virtual void SetContentHandler(CContentHandler* pContentHandler);
	virtual void SetErrorHandler(CErrorHandler* pErrorHandler);
	virtual CEntityResolver* GetEntityResolver() const;
	virtual CDTDHandler* GetDTDHandler() const;
	virtual CContentHandler* GetContentHandler() const;
	virtual CErrorHandler* GetErrorHandler() const;
	virtual void SetFeature(const XMLString& featureId, bool state);
	virtual bool GetFeature(const XMLString& featureId) const;
	virtual void SetProperty(const XMLString& propertyId, const XMLString& value);
	virtual void SetProperty(const XMLString& propertyId, void* value);
	virtual void* GetProperty(const XMLString& propertyId) const;

	void Parse(const Node* pNode);
		//; Parses an XML DOM document or document fragment.

protected:
	virtual void Parse(CInputSource* pSource);
	virtual void Parse(const XMLString& systemId);

	void Traverse(const Node* pNode) const;
	void Process(const Node* pNode) const;
	void ProcessElement(const Element* pElem) const;
	void ProcessCharacterData(const Text* pData) const;
	void ProcessComment(const Comment* pComment) const;
	void ProcessPI(const ProcessingInstruction* pPI) const;
	void ProcessExternalEntity(const Entity* pObj) const;
	void ProcessCDATASection(const CDATASection* pCDATA) const;
	void ProcessNotationDecl(const Notation* pDecl) const;
	void ProcessDocument(const Document* pObj) const;
	void ProcessFragment(const DocumentFragment* pObj) const;

private:
	static const XMLString SAX_FEATURE_NAMESPACES;
	static const XMLString SAX_FEATURE_NAMESPACE_PREFIXES;
	static const XMLString SAX_PROPERTY_DECL_HANDLER;
	static const XMLString SAX_PROPERTY_LEXICAL_HANDLER;
	static const XMLString TYPE_CDATA;

	bool             m_featureNamespaces;
	CEntityResolver* m_pEntityResolver;
	CDTDHandler*     m_pDTDHandler;
	CContentHandler* m_pContentHandler;
	CErrorHandler*   m_pErrorHandler;
	CDeclHandler*    m_pDeclHandler;
	CLexicalHandler* m_pLexicalHandler;
};


XML_END


#endif // SAXDOMParser_H_INCLUDED
