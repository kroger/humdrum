//
// HandlerBase.h
//
// $Id: //depot/XML/Main/include/SAX/HandlerBase.h#5 $
//
// SAX default base class for handlers.
//
// DEPRECATED. Use DefaultHandler.h instead.
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


#ifndef SAX_HandlerBase_H_INCLUDED
#define SAX_HandlerBase_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_EntityResolver_H_INCLUDED
#include "SAX/EntityResolver.h"
#endif
#ifndef SAX_DTDHandler_H_INCLUDED
#include "SAX/DTDHandler.h"
#endif
#ifndef SAX_DocumentHandler_H_INCLUDED
#include "SAX/DocumentHandler.h"
#endif
#ifndef SAX_ErrorHandler_H_INCLUDED
#include "SAX/ErrorHandler.h"
#endif


XML_BEGIN


class XML_API CHandlerBase: public CEntityResolver, public CDTDHandler, public CDocumentHandler, public CErrorHandler
{
public:
	CInputSource* ResolveEntity(const XMLString* publicId, const XMLString& systemId);
	void DoneWithInputSource(CInputSource* pSource);
	void NotationDecl(const XMLString& name, const XMLString* publicd, const XMLString* systemId);
	void UnparsedEntityDecl(const XMLString& name, const XMLString* publicId, const XMLString& systemId, const XMLString& notationName);
	void SetDocumentLocator(const CLocator& loc);
	void StartDocument();
	void EndDocument();
	void StartElement(const XMLString& name, const CAttributeList& attrList);
	void EndElement(const XMLString& name);
	void Characters(const XMLChar ch[], int start, int length);
	void IgnorableWhitespace(const XMLChar ch[], int start, int length);
	void ProcessingInstruction(const XMLString& target, const XMLString& data);
	void Warning(CSAXException& e);
	void Error(CSAXException& e);
	void FatalError(CSAXException& e);
};


XML_END


#endif // SAX_HandlerBase_H_INCLUDED

