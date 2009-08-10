//
// SAXParseException.h
//
// $Id: //depot/XML/Main/include/SAX/SAXParseException.h#5 $
//
// Exception class for SAX parsing errors or warnings.
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


#ifndef SAX_SAXParseException_H_INCLUDED
#define SAX_SAXParseException_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_SAXException_H_INCLUDED
#include "SAX/SAXException.h"
#endif
#ifndef SAX_Locator_H_INCLUDED
#include "SAX/Locator.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class XML_API CSAXParseException: public CSAXException
{
public:
	CSAXParseException(const CSAXParseException& e);
	CSAXParseException(const string& msg, const CLocator& loc);
	CSAXParseException(const string& msg, const CLocator& loc, const CXMLException& e);
	CSAXParseException(const string& msg, const XMLString& publicId, const XMLString& systemId, int lineNumber, int columnNumber);
	CSAXParseException(const string& msg, const XMLString& publicId, const XMLString& systemId, int lineNumber, int columnNumber, const CXMLException& e);
	~CSAXParseException() throw();

	XMLString GetPublicId() const;
		//; Returns the public identifier of the entity which caused the exception.

	XMLString GetSystemId() const;
		//; Returns the system identifier of the entity which caused the exception.

	int GetLineNumber() const;
		//; Returns the line number within the entity.

	int GetColumnNumber() const;
		//; Returns the column number within the entity.

private:
	XMLString m_publicId;
	XMLString m_systemId;
	int m_lineNumber;
	int m_columnNumber;
};


XML_END


#endif // SAX_SAXParseException_H_INCLUDED
