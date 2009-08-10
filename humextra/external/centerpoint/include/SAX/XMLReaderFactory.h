//
// XMLReaderFactory.h
//
// $Id: //depot/XML/Main/include/SAX/XMLReaderFactory.h#3 $
//
// SAX2 reader factory class.
// This one just creates an instance of the XML CSAX2Parser class.
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


#ifndef SAX_XMLReaderFactory_H_INCLUDED
#define SAX_XMLReaderFactory_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class CXMLReader;


class XML_API CXMLReaderFactory
	//: This factory can be used to create instances
	//. of CSAX2Parser.
{
public:
	static CXMLReader* MakeXMLReader();
		//; Creates and returns a new instance of CSAX2Parser.
};


XML_END


#endif // SAX_XMLReaderFactory_H_INCLUDED

