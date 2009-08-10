//
// InputSource.cpp
//
// $Id: //depot/XML/Main/src/InputSource.cpp#3 $
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


#include "SAX/InputSource.h"


XML_BEGIN


CInputSource::CInputSource()
{
	m_bistr = NULL;
	m_cistr = NULL;
}


CInputSource::CInputSource(const XMLString& systemId)
{
	m_bistr = NULL;
	m_cistr = NULL;
	m_systemId = systemId;
}


CInputSource::CInputSource(XMLByteInputStream* bistr)
{
	m_bistr = bistr;
	m_cistr = NULL;
}


CInputSource::~CInputSource()
{
}


void CInputSource::SetPublicId(const XMLString& publicId)
{
	m_publicId = publicId;
}


void CInputSource::SetSystemId(const XMLString& systemId)
{
	m_systemId = systemId;
}


XMLString CInputSource::GetPublicId() const
{
	return m_publicId;
}


XMLString CInputSource::GetSystemId() const
{
	return m_systemId;
}


void CInputSource::SetByteStream(XMLByteInputStream* bistr)
{
	m_bistr = bistr;
}


XMLByteInputStream* CInputSource::GetByteStream() const
{
	return m_bistr;
}


void CInputSource::SetCharacterStream(XMLCharInputStream* cistr)
{
	m_cistr = cistr;
}


XMLCharInputStream* CInputSource::GetCharacterStream() const
{
	return m_cistr;
}


void CInputSource::SetEncoding(const XMLString& encoding)
{
	m_encoding = encoding;
}


XMLString CInputSource::GetEncoding() const
{
	return m_encoding;
}


XML_END

