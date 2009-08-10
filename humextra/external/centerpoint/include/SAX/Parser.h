//
// Parser.h
//
// $Id: //depot/XML/Main/include/SAX/Parser.h#5 $
//
// SAX Parser Interface.
//
// DEPRECATED. Use XMLReader.h instead.
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


#ifndef SAX_Parser_H_INCLUDED
#define SAX_Parser_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class CEntityResolver;
class CDTDHandler;
class CDocumentHandler;
class CErrorHandler;
class CInputSource;
class CLexicalHandler;
class CNamespaceHandler;


class XML_API CParser
{
public:
	virtual void SetLocale(const XMLString& locale) = 0;
	virtual void SetEntityResolver(CEntityResolver* pResolver) = 0;
	virtual void SetDTDHandler(CDTDHandler* pDTDHandler) = 0;
	virtual void SetDocumentHandler(CDocumentHandler* pDocumentHandler) = 0;
	virtual void SetErrorHandler(CErrorHandler* pErrorHandler) = 0;
	virtual void Parse(CInputSource* pSource) = 0;
	virtual void Parse(const XMLString& systemId) = 0;

	virtual ~CParser();
};


XML_END


#endif // SAX_Parser_H_INCLUDED
