//
// LocatorImp.cpp
//
// $Id: //depot/XML/Main/src/LocatorImp.cpp#3 $
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


#include "SAX/LocatorImp.h"


XML_BEGIN


CLocatorImp::CLocatorImp()
{
	m_lineNumber = 0;
	m_columnNumber = 0;
}


CLocatorImp::CLocatorImp(const CLocator& loc)
{
	m_publicId = loc.GetPublicId();
	m_systemId = loc.GetSystemId();
	m_lineNumber = loc.GetLineNumber();
	m_columnNumber = loc.GetColumnNumber();
}


CLocatorImp::~CLocatorImp()
{
}


XMLString CLocatorImp::GetPublicId() const
{
	return m_publicId;
}


XMLString CLocatorImp::GetSystemId() const
{
	return m_systemId;
}


int CLocatorImp::GetLineNumber() const
{
	return m_lineNumber;
}


int CLocatorImp::GetColumnNumber() const
{
	return m_columnNumber;
}


void CLocatorImp::SetPublicId(const XMLString& publicId)
{
	m_publicId = publicId;
}


void CLocatorImp::SetSystemId(const XMLString& systemId)
{
	m_systemId = systemId;
}


void CLocatorImp::SetLineNumber(int lineNumber)
{
	m_lineNumber = lineNumber;
}


void CLocatorImp::SetColumnNumber(int columnNumber)
{
	m_columnNumber = columnNumber;
}


XML_END
