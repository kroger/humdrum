//
// SAX2ParserNSP.h
//
// $Id: //depot/XML/Main/include/SAX2ParserNSP.h#3 $
//
// Definition of CSAX2ParserNSP.
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


#ifndef SAX2ParserNSP_H_INCLUDED
#define SAX2ParserNSP_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX2ParserNS_H_INCLUDED
#include "SAX2ParserNS.h"
#endif
#ifndef SAX_NamespaceSupport_H_INCLUDED
#include "SAX/NamespaceSupport.h"
#endif


XML_BEGIN


class XML_API CSAX2ParserNSP: public CSAX2ParserNS
	//; This class adds namespaces and namespace prefixes support to the basic SAX2 parser.
{
public:
	CSAX2ParserNSP(const XML_Char* encoding = NULL);

protected:
	void HandleStartElement(const XML_Char* name, const XML_Char** atts);
	void HandleEndElement(const XML_Char* name);
	void HandleStartNamespace(const XML_Char* prefix, const XML_Char* uri);
	void HandleEndNamespace(const XML_Char* prefix);

private:
	CNamespaceSupport m_namespaceSupport;

	static const XMLString COLON;
};


XML_END


#endif // SAX2ParserNSP_H_INCLUDED
