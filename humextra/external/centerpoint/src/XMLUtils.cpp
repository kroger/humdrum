//
// XMLUtils.cpp
//
// $Id: //depot/XML/Main/src/XMLUtils.cpp#3 $
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


#include "XMLUtils.h"


#if defined(WIN32) && defined(XML_UNICODE) && !defined(XML_UNICODE_WCHAR_T)
// The WIN32 API provides conversion routines from
// ASCII to Unicode and vice versa.
#include <windows.h>
#endif


#if defined(XML_UNICODE_WCHAR_T)
// #include wide char conversion routines
// from C library
#include <stdlib.h>
#endif


XML_BEGIN


#if defined(XML_UNICODE_WCHAR_T)
//
// wchar_t implementation
//


string CXMLUtils::XMLToAscii(const XMLString& str)
{
	int n = str.length()*2 + 1;
	char* asciiStr = new char[n];

	wcstombs(asciiStr, str.c_str(), n);

	string res = asciiStr;
	delete [] asciiStr;
	return res;
}


XMLString CXMLUtils::AsciiToXML(const string& str)
{
	int n = str.length() + 1;
	XMLChar* xmlStr = new XMLChar[n];

	mbstowcs(xmlStr, str.c_str(), n);

	XMLString res = xmlStr;
	delete [] xmlStr;
	return res;
}


#elif defined(XML_UNICODE)
//
// Unicode implementation
//


#if defined(WIN32)
//
// WIN32 Unicode implementation
//


string CXMLUtils::XMLToAscii(const XMLString& str)
{
	int n = (str.length() + 1)*2;
	char* asciiStr = new char[n];

	WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, asciiStr, n, NULL, NULL);

	string res = asciiStr;
	delete [] asciiStr;
	return res;
}


XMLString CXMLUtils::AsciiToXML(const string& str)
{
	int n = str.length() + 1;
	XMLChar* xmlStr = new XMLChar[n];

	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, xmlStr, n);

	XMLString res = xmlStr;
	delete [] xmlStr;
	return res;
}


#else
//
// Default Unicode implementation
//


string CXMLUtils::XMLToAscii(const XMLString& str)
{
	string asciiStr;
	register int n = str.length();
	for (register int i = 0; i < n; i++)
	{
		register XMLChar c = str[i];
		asciiStr += (c <= 127) ? c : '_';
	}
	return asciiStr;
}


XMLString CXMLUtils::AsciiToXML(const string& str)
{
	XMLString xmlStr;
	register int n = str.length();
	for (register int i = 0; i < n; i++)
	{
		register char c = str[i];
		xmlStr += (c <= 127) ? c : '_';
	}
	return xmlStr;
}


#endif // WIN32


#else
//
// ASCII implementation
//


string CXMLUtils::XMLToAscii(const XMLString& str)
{
	return str;
}


XMLString CXMLUtils::AsciiToXML(const string& str)
{
	return str;
}


#endif // XML_UNICODE_WCHAR_T


XML_END
