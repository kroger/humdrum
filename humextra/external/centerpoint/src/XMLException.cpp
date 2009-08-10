//
// XMLException.cpp
//
// $Id: //depot/XML/Main/src/XMLException.cpp#8 $
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


#include "XMLException.h"


XML_BEGIN


static const char* g_messages[] =
{
	"XML exception",
	"No memory",
	"XML not well-formed",
	"No element found",
	"Invalid XML token",
	"Unclosed XML token",
	"Partial XML character",
	"Mismatched XML tag",
	"Duplicate XML attribute",
	"Junk after XML document element",
	"Illegal XML parameter entity reference",
	"Undefined XML entity",
	"Recursive XML entity reference",
	"Asynchronous XML entity",
	"Reference to invalid XML character number",
	"Reference to XML binary entity",
	"Reference to external XML entity in attribute",
	"XML processing instruction not at start of external entity",
	"Unknown XML encoding",
	"Encoding specified in XML declaration is incorrect",
	"Unclosed XML CDATA section",
	"Error in processing external entity reference",
	"XML document is not standalone",

	"SAX exception",
	"SAX Option is not supported",
	"SAX Option is not recognized",
	"XML Parse exception",

	"Bad XML system ID",
	"Bad XML input source",

	"Name pool is full",

	"DOM exception",
	"DOM index out of range",
	"DOM string too large",
	"DOM node does not belong here",
	"DOM node used in wrong document",
	"Invalid character",
	"DOM node does not support data",
	"No modification to DOM allowed",
	"DOM Node not found",
	"DOM Object type not supported",
	"DOM Attribute is already in use",

	"DOM Invalid state",
	"DOM Syntax error",
	"DOM Invalid modification",
	"DOM Namespace error",
	"DOM Invalid access",
	"DOM Unspecified event type",

	"XML input/output error",

	"XML error"
};


#ifndef CSL_XML_STANDALONE


using CSL::Base::CException;


CXMLException::CXMLException(long errorCode): CException(errorCode)
{
}


CXMLException::CXMLException(long errorCode, const string& msg): CException(errorCode, msg)
{
}


CXMLException::CXMLException(const CXMLException& e): CException(e)
{
}


const char* CXMLException::GetStaticErrorText() const throw()
{
	if ((GetResult() & EMaskClass) == CXMLException::EBase)
	{
		int nMsgs = sizeof (g_messages) / sizeof (char*);
		int msgNo = GetResult() & EMaskMessage;
		return g_messages[csl_min(msgNo, nMsgs - 1)];
	}
	else
	{
		return CException::GetStaticErrorText();
	}
}


#else


CXMLException::CXMLException(long errCode)
{
	m_errorCode  = errCode;
}


CXMLException::CXMLException(long errCode, const string& errMsg)
{
	m_errorCode  = errCode;
	m_strMessage = errMsg;
}


CXMLException::CXMLException(const CXMLException& e)
{
	m_errorCode  = e.m_errorCode;
	m_strMessage = e.m_strMessage;
}


long CXMLException::GetResult() const
{
	return m_errorCode;
}


string CXMLException::GetMsg() const
{
	string msg = what();
	if (!m_strMessage.empty())
	{
		msg += ": ";
		msg += m_strMessage;
	}
	return msg;
}


void CXMLException::SetMsg(const string& msg)
{
	m_strMessage = msg;
}


const char* CXMLException::what() const throw()
{
	int msgNo = m_errorCode & 0xFF;
	if (msgNo < sizeof (g_messages) / sizeof (char*))
		return g_messages[msgNo];
	else
		return g_messages[0];
}


#endif


CXMLException::~CXMLException() throw()
{
}


XML_END
