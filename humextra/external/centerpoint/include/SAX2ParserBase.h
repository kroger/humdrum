//
// SAX2ParserBase.h
//
// $Id: //depot/XML/Main/include/SAX2ParserBase.h#4 $
//
// Definition of CSAX2ParserBase.
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


#ifndef SAX2ParserBase_H_INCLUDED
#define SAX2ParserBase_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLParserBase_H_INCLUDED
#include "XMLParserBase.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif
#ifndef XMLStream_H_INCLUDED
#include "XMLStream.h"
#endif
#ifndef SAX_Locator_H_INCLUDED
#include "SAX/Locator.h"
#endif


XML_BEGIN


class CEntityResolver;
class CDTDHandler;
class CContentHandler;
class CErrorHandler;
class CInputSource;
class CLexicalHandler;
class CDeclHandler;
class CNamespaceHandler;
class CAnyHandler;


class XML_API CSAX2ParserBase: public CXMLParserBase, public CLocator
	//; This class is a wrapper for CXMLParserBase to be used by CSAX2Parser.
{
public:
	CSAX2ParserBase(const XML_Char* encoding = NULL);

	void SetEntityResolver(CEntityResolver* pResolver);
		//; Allows an application to register a custom entity resolver.

	void SetDTDHandler(CDTDHandler* pDTDHandler);
		//; Allow an application to register a DTD event handler.

	void SetContentHandler(CContentHandler* pContentHandler);
		//; Allow an application to register a content event handler.

	void SetErrorHandler(CErrorHandler* pErrorHandler);
		//; Allow an application to register an error event handler.

	void SetLexicalHandler(CLexicalHandler* pLexicalHandler);
		//; Allow an application to register a lexical handler.

	void SetDeclHandler(CDeclHandler* pDeclHandler);
		//; Allow an application to register a declaration handler.

	void SetDefaultHandler(CAnyHandler* pAnyHandler);
		//: Allow an application to register a default handler.
		//+ The default handler is used for all items, for which
		//. no specific handler is registered.

	CEntityResolver* GetEntityResolver() const;
		//; Return the current entity resolver.

	CDTDHandler* GetDTDHandler() const;
		//; Return the current DTD handler.

	CContentHandler* GetContentHandler() const;
		//; Return the current content handler.

	CErrorHandler* GetErrorHandler() const;
		//; Return the current error handler.

	CAnyHandler* GetDefaultHandler() const;
		//; Return the default handler.

	CLexicalHandler* GetLexicalHandler() const;
		//; Return the lexical handler.

	CDeclHandler* GetDeclHandler() const;
		//; Return the declaration handler.

	void SetInputSource(CInputSource* pInputSource);
		//; Sets the input source for parsing.

	void IncludeExternalGeneralEntities(bool state = true);
		//; Enables or disables automatic inclusion of external general entities.

	void IncludeExternalParameterEntities(bool state = true);
		//; Enables or disables automatic inclusion of external parameter entities.

	void Parse(XMLByteInputStream* bistr);
		//; Parses an XML document from a byte input stream.

	void ParseChar(XMLCharInputStream* cistr);
		//; Parses an XML document from a character input stream.

	// Locator methods
	XMLString GetPublicId() const;
		//; Returns the public identifier taken from the input source.

	XMLString GetSystemId() const;
		//; Returns the system identifier taken from the input source.

	int GetLineNumber() const;
		//; Returns the current line number within the XML document.

	int GetColumnNumber() const;
		//; Returns the current column number within the XML document.

protected:
	// expat handler methods
	void HandleStartElement(const XML_Char* name, const XML_Char** atts);
	void HandleEndElement(const XML_Char* name);
	void HandleCharacterData(const XML_Char* s, int len);
	void HandleProcessingInstruction(const XML_Char* target, const XML_Char* data);
	void HandleDefault(const XML_Char* s, int len);
	void HandleUnparsedEntityDecl(const XML_Char* entityName, const XML_Char* base,
		                          const XML_Char* systemId, const XML_Char* publicId,
			        		      const XML_Char* notationName);
	void HandleNotationDecl(const XML_Char* notationName, const XML_Char* base,
		                    const XML_Char* systemId, const XML_Char* publicId);
	int HandleExternalEntityRef(const XML_Char* openEntityNames, const XML_Char* base,
							    const XML_Char* systemId, const XML_Char* publicId);

	// expat 1.1 / SAX2 extensions
	virtual void HandleComment(const XML_Char* data);
	virtual void HandleStartCDataSection();
	virtual void HandleEndCDataSection();
	virtual void HandleStartDoctypeDecl(const XML_Char* doctypeName);
	virtual void HandleEndDoctypeDecl();
	virtual void HandleExternalParsedEntityDecl(const XML_Char* entityName, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId);
	virtual void HandleInternalParsedEntityDecl(const XML_Char* entityName, const XML_Char* replacementText, int replacementTextLength);

	void HandleError(int errNo);

	static const XMLString EMPTY_STRING;
	static const XMLString TYPE_CDATA;

	CEntityResolver* m_pEntityResolver;
	CDTDHandler*     m_pDTDHandler;
	CContentHandler* m_pContentHandler;
	CErrorHandler*   m_pErrorHandler;
	CLexicalHandler* m_pLexicalHandler;
	CDeclHandler*    m_pDeclHandler;
	CAnyHandler*     m_pDefaultHandler;
	CInputSource*    m_pInputSource;

private:
	bool m_includeExtGenEntities;
	bool m_includeExtParEntities;
};


XML_END


#endif // SAX2ParserBase_H_INCLUDED
