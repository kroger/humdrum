//
// LocatorImp.h
//
// $Id: //depot/XML/Main/include/SAX/LocatorImp.h#5 $
//
// An implementation of the SAX Locator interface, provided for your convenience.
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


#ifndef SAX_LocatorImp_H_INCLUDED
#define SAX_LocatorImp_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_Locator_H_INCLUDED
#include "SAX/Locator.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class XML_API CLocatorImp: public CLocator
{
public:
	CLocatorImp();
	CLocatorImp(const CLocator& loc);
	virtual ~CLocatorImp();

	virtual XMLString GetPublicId() const;
	virtual XMLString GetSystemId() const;
	virtual int GetLineNumber() const;
	virtual int GetColumnNumber() const;

	virtual void SetPublicId(const XMLString& publicId);
	virtual void SetSystemId(const XMLString& systemId);
	virtual void SetLineNumber(int lineNumber);
	virtual void SetColumnNumber(int columnNumber);

private:
	XMLString m_publicId;
	XMLString m_systemId;
	int m_lineNumber;
	int m_columnNumber;
};


XML_END


#endif // SAX_LocatorImp_H_INCLUDED
