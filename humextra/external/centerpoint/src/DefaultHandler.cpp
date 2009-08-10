//
// DefaultHandler.cpp
//
// $Id: //depot/XML/Main/src/DefaultHandler.cpp#2 $
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


#include "SAX/DefaultHandler.h"


XML_BEGIN


CInputSource* CDefaultHandler::ResolveEntity(const XMLString* publicId, const XMLString& systemId)
{
	return NULL;
}


void CDefaultHandler::DoneWithInputSource(CInputSource* pSource)
{
}


void CDefaultHandler::NotationDecl(const XMLString& name, const XMLString* publicd, const XMLString* systemId)
{
}


void CDefaultHandler::UnparsedEntityDecl(const XMLString& name, const XMLString* publicId, const XMLString& systemId, const XMLString& notationName)
{
}


void CDefaultHandler::SetDocumentLocator(const CLocator& loc)
{
}


void CDefaultHandler::StartDocument()
{
}


void CDefaultHandler::EndDocument()
{
}


void CDefaultHandler::StartElement(const XMLString& uri, const XMLString& localName, const XMLString& qname, const CAttributes& attrList)
{
}


void CDefaultHandler::EndElement(const XMLString& uri, const XMLString& localName, const XMLString& qname)
{
}


void CDefaultHandler::Characters(const XMLChar ch[], int start, int length)
{
}


void CDefaultHandler::IgnorableWhitespace(const XMLChar ch[], int start, int length)
{
}


void CDefaultHandler::ProcessingInstruction(const XMLString& target, const XMLString& data)
{
}


void CDefaultHandler::StartPrefixMapping(const XMLString& prefix, const XMLString& uri)
{
}


void CDefaultHandler::EndPrefixMapping(const XMLString& prefix)
{
}


void CDefaultHandler::SkippedEntity(const XMLString& prefix)
{
}


void CDefaultHandler::Warning(CSAXException& e)
{
}


void CDefaultHandler::Error(CSAXException& e)
{
}


void CDefaultHandler::FatalError(CSAXException& e)
{
}


CDefaultHandler::~CDefaultHandler()
{
}


XML_END
