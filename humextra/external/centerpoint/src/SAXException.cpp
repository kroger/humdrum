//
// SAXException.cpp
//
// $Id: //depot/XML/Main/src/SAXException.cpp#4 $
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


#include "SAX/SAXException.h"
#include "SAX/SAXNotRecognizedException.h"
#include "SAX/SAXNotSupportedException.h"
#include "SAX/SAXParseException.h"


XML_BEGIN


CSAXException::CSAXException(const CSAXException& e): CXMLException(e)
{
}


CSAXException::CSAXException(const CXMLException& e): CXMLException(e)
{
}


CSAXException::CSAXException(const string& msg): CXMLException(CXMLException::ESAXException, msg)
{
}


CSAXException::CSAXException(const string& msg, CXMLException& e): CXMLException(e)
{
	SetMsg(msg);
}


CSAXException::~CSAXException() throw()
{
}


CSAXNotRecognizedException::CSAXNotRecognizedException(const string& msg): CSAXException(CXMLException(CXMLException::ESAXNotRecognizedException, msg))
{
}


CSAXNotRecognizedException::CSAXNotRecognizedException(const CSAXNotRecognizedException& exc): CSAXException(exc)
{
}


CSAXNotRecognizedException::~CSAXNotRecognizedException() throw()
{
}


CSAXNotSupportedException::CSAXNotSupportedException(const string& msg): CSAXException(CXMLException(CXMLException::ESAXNotSupportedException, msg))
{
}


CSAXNotSupportedException::CSAXNotSupportedException(const CSAXNotSupportedException& exc): CSAXException(exc)
{
}


CSAXNotSupportedException::~CSAXNotSupportedException() throw()
{
}


CSAXParseException::CSAXParseException(const CSAXParseException& e): CSAXException(e)
{
	m_publicId     = e.m_publicId;
	m_systemId     = e.m_systemId;
	m_lineNumber   = e.m_lineNumber;
	m_columnNumber = e.m_columnNumber;
}


CSAXParseException::CSAXParseException(const string& msg, const CLocator& loc): CSAXException(msg)
{
	m_publicId     = loc.GetPublicId();
	m_systemId     = loc.GetSystemId();
	m_lineNumber   = loc.GetLineNumber();
	m_columnNumber = loc.GetColumnNumber();
}


CSAXParseException::CSAXParseException(const string& msg, const CLocator& loc, const CXMLException& e): CSAXException(e)
{
	SetMsg(msg);
	m_publicId     = loc.GetPublicId();
	m_systemId     = loc.GetSystemId();
	m_lineNumber   = loc.GetLineNumber();
	m_columnNumber = loc.GetColumnNumber();
}


CSAXParseException::CSAXParseException(const string& msg, const XMLString& publicId, const XMLString& systemId, int lineNumber, int columnNumber): CSAXException(msg)
{
	m_publicId     = publicId;
	m_systemId     = systemId;
	m_lineNumber   = lineNumber;
	m_columnNumber = columnNumber;
}


CSAXParseException::CSAXParseException(const string& msg, const XMLString& publicId, const XMLString& systemId, int lineNumber, int columnNumber, const CXMLException& e): CSAXException(e)
{
	SetMsg(msg);
	m_publicId     = publicId;
	m_systemId     = systemId;
	m_lineNumber   = lineNumber;
	m_columnNumber = columnNumber;
}


CSAXParseException::~CSAXParseException() throw()
{
}


XMLString CSAXParseException::GetPublicId() const
{
	return m_publicId;
}


XMLString CSAXParseException::GetSystemId() const
{
	return m_systemId;
}


int CSAXParseException::GetLineNumber() const
{
	return m_lineNumber;
}


int CSAXParseException::GetColumnNumber() const
{
	return m_columnNumber;
};


XML_END
