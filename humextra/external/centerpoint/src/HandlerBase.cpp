//
// HandlerBase.cpp
//
// $Id: //depot/XML/Main/src/HandlerBase.cpp#3 $
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


#include "SAX/HandlerBase.h"


XML_BEGIN


CInputSource* CHandlerBase::ResolveEntity(const XMLString* publicId, const XMLString& systemId)
{
	return NULL;
}


void CHandlerBase::DoneWithInputSource(CInputSource* pSource)
{
	// There's nothing to do since ResolveEntity always returns NULL
}


void CHandlerBase::NotationDecl(const XMLString& name, const XMLString* publicd, const XMLString* systemId)
{
}


void CHandlerBase::UnparsedEntityDecl(const XMLString& name, const XMLString* publicId, const XMLString& systemId, const XMLString& notationName)
{
}


void CHandlerBase::SetDocumentLocator(const CLocator& pLoc)
{
}


void CHandlerBase::StartDocument()
{
}


void CHandlerBase::EndDocument()
{
}


void CHandlerBase::StartElement(const XMLString& name, const CAttributeList& attrList)
{
}


void CHandlerBase::EndElement(const XMLString& name)
{
}


void CHandlerBase::Characters(const XMLChar ch[], int start, int length)
{
}


void CHandlerBase::IgnorableWhitespace(const XMLChar ch[], int start, int length)
{
}


void CHandlerBase::ProcessingInstruction(const XMLString& target, const XMLString& data)
{
}


void CHandlerBase::Warning(CSAXException& e)
{
}


void CHandlerBase::Error(CSAXException& e)
{
}


void CHandlerBase::FatalError(CSAXException& e)
{
}


XML_END
