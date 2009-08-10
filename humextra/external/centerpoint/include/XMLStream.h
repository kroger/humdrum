//
// XMLStream.h
//
// $Id: //depot/XML/Main/include/XMLStream.h#4 $
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
// We use our own stream class for XML streams.
// Depending on various preprocessor symbols,
// this stream class is either implemented as
// C++ istream class, as C++ wistream class,
// or as basic_istream<unsigned short>.
// The following table describes the relationships.
//
// XML_UNICODE  XML_UNICODE_WCHAR_T  XMLString
//  #define'd        #define'd         impl.
// --------------------------------------------------------------
//     N                 N           istream
//     N                 Y           wistring
//     Y                 Y           wistring
//     Y                 N           basic_istream<unsigned short>
//
// Note that the preprocessor symbols and types we
// use are the same ones that expat uses.
//


#ifndef XMLStream_H_INCLUDED
#define XMLStream_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#if defined(__DECCXX_V5) || defined(__GNUG__) || defined(__RWSTD_V1)
#ifndef STD_IOSTREAM_INCLUDED
#include <iostream>
#define STD_IOSTREAM_INCLUDED
#endif
#else
#ifndef STD_ISTREAM_INCLUDED
#include <istream>
#define STD_ISTREAM_INCLUDED
#endif
#ifndef STD_OSTREAM_INCLUDED
#include <ostream>
#define STD_OSTREAM_INCLUDED
#endif
#endif


XML_BEGIN


// The byte input stream is always defined the same way.
typedef istream XMLByteInputStream;
typedef ostream XMLByteOutputStream;


#if defined(XML_UNICODE_WCHAR_T)
//
// We assume that the implementation of WCHAR_T uses UNICODE.
//

// XML_UNICODE_WCHAR_T implies XML_UNICODE
#ifndef XML_UNICODE
#define XML_UNICODE
#endif


typedef wistream XMLCharInputStream;
typedef wostream XMLCharOutputStream;


#elif defined(XML_UNICODE)
//
// A UNICODE character is represented as unsigned short.
//


typedef basic_istream<unsigned short> XMLCharInputStream;
typedef basic_ostream<unsigned short> XMLCharOutputStream;


#else
//
// No UNICODE support.
//


typedef istream XMLCharInputStream;
typedef ostream XMLCharOutputStream;



#endif // XML_UNICODE_WCHAR_T


XML_END


#endif // XMLStream_H_INCLUDED

