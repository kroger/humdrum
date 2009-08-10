//
// XMLString.h
//
// $Id: //depot/XML/Main/include/XMLString.h#3 $
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
//
// We use our own string class for XML strings.
// Depending on various preprocessor symbols,
// this string class is either implemented as
// C++ string class, as C++ wstring class,
// or as basic_string<unsigned short>.
// The following table describes the relationships.
//
// XML_UNICODE  XML_UNICODE_WCHAR_T  XMLString
//  #define'd        #define'd         impl.
// --------------------------------------------------------------
//     N                 N           string
//     N                 Y           wstring
//     Y                 Y           wstring
//     Y                 N           basic_string<unsigned short>
//
// Note that the preprocessor symbols and types we
// use are the same ones that expat uses.
//


#ifndef XMLString_H_INCLUDED
#define XMLString_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef STD_STRING_INCLUDED
#include <string>
#define STD_STRING_INCLUDED
#endif


XML_BEGIN


#if defined(XML_UNICODE_WCHAR_T)
//
// We assume that the implementation of WCHAR_T uses UNICODE.
//

// XML_UNICODE_WCHAR_T implies XML_UNICODE
#ifndef XML_UNICODE
#define XML_UNICODE
#endif


typedef wchar_t XMLChar;
typedef wstring XMLString;


#elif defined(XML_UNICODE)
//
// A UNICODE character is represented as unsigned short.
//


typedef unsigned short XMLChar;
typedef basic_string<XMLChar> XMLString;


#else
//
// No UNICODE support.
//


typedef char XMLChar;
typedef string XMLString;



#endif // XML_UNICODE_WCHAR_T


XML_END


#endif // XMLString_H_INCLUDED

