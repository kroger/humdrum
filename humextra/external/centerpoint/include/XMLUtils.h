//
// XMLUtils.h
//
// $Id: //depot/XML/Main/include/XMLUtils.h#5 $
//
// Various utility methods used by XML.
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


#ifndef XMLUtils_H_INCLUDED
#define XMLUtils_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class XML_API CXMLUtils
	//; This class hosts various utility methods.
{
public:
	static string XMLToAscii(const XMLString& str);
		//: Converts an XML string (ASCII char,
		//+ Unicode wchar_t, or Unicode unsigned short
		//+ encoded) to a C++ string.
		//+ For conversions between wchar_t and char,
		//+ C library functions are used.
		//+ For other conversions, either OS specific
		//+ functions or a default implementation is
		//+ used.
		//+
		//+ WARNING: The default conversion routine
		//+ from Unicode to ASCII replaces all
		//. ASCII chars > 127 with an underscore!

	static XMLString AsciiToXML(const string& str);
		//: Converts an ASCII encoded C++ string to
		//+ a XML string (ASCII char, Unicode wchar_t
		//+ or Unicode unsigned short encoded).
		//+ For conversions between char and wchar_t,
		//+ C library functions are used.
		//+ For other conversions, either OS specifi
		//+ functions or a default implementation is
		//+ used.
		//+
		//+ WARNING: The default conversion routine
		//+ from ASCII to Unicode replaces all
		//. Unicode chars > 127 with an underscore!
};


XML_END


#endif // XMLUtils_H_INCLUDED


