//
// DOMImplementation.h
//
// $Id: //depot/XML/Main/include/DOM/DOMImplementation.h#7 $
//
// Document Object Model (Core) Level 1
//
// Interface DOMImplementation
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


#ifndef DOM_DOMImplementation_H_INCLUDED
#define DOM_DOMImplementation_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class DocumentType;
class Document;
class XMLNamePool;


class XML_API DOMImplementation
	//: The DOMImplementation interface provides a number of methods for
	//+ performing operations that are independent of any particular instance
	//+ of the document object model.
	//. In this implementation, DOMImplementation is implemented as a singleton.
{
public:
	virtual bool hasFeature(const XMLString& feature, const XMLString& version) const;
		//: Tests if the DOM implementation implements a specific feature.
		//+ Currently, the only supported features are "XML", version "1.0" and
		//. "Core", version "2.0".
	
	// DOM Level 2
	DocumentType* createDocumentType(const XMLString& qualifiedName, const XMLString& publicId, const XMLString& systemId) const;
		//: Creates an empty DocumentType node. Entity declarations and notations 
		//+ are not made available. Entity reference expansions and default attribute 
		//. additions do not occur.

	Document* createDocument(const XMLString& namespaceURI, const XMLString& qualifiedName, DocumentType* doctype) const;
		//; Creates an XML Document object of the specified type with its document element.

	Document* createDocumentNP(const XMLString& namespaceURI, const XMLString& qualifiedName, DocumentType* doctype, XMLNamePool* pNamePool) const;
		//; Creates an XML Document object of the specified type with its document element.

	// IMPLEMENTATION
	static DOMImplementation* getInstance();
	static XMLString localFromQualified(const XMLString& qname);

protected:
	virtual ~DOMImplementation();

private:
	static DOMImplementation* m_pInstance;
};


XML_END


#endif // DOM_DOMImplementation_H_INCLUDED
