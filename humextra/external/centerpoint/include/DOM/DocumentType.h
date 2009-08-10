//
// DocumentType.h
//
// $Id: //depot/XML/Main/include/DOM/DocumentType.h#8 $
//
// Document Object Model (Core) Level 1
//
// Interface DocumentType
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


#ifndef DOM_DocumentType_H_INCLUDED
#define DOM_DocumentType_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef DOM_NodeImp_H_INCLUDED
#include "DOM/NodeImp.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class Document;
class NamedNodeMap;


class XML_API DocumentType: public SimpleNode
	//: Each Document has a doctype attribute whose value is
	//+ either NULL of a DocumentType object.
	//. The DOM Level 1 does not support editing DocumentType nodes.
{
public:
	virtual const XMLString& getName() const;
		//; The name of the DTD.

	virtual NamedNodeMap* getEntities() const;
		//: A NamedNodeMap containing the general entities,
		//+ both external and internal, declared in the DTD.
		//+ Duplicates are discarded.
		//+ WARNING: In this implementation, only the
		//+ external entities are reported.
		//+ Every node in this map also implements the
		//+ Entity interface.
		//+ When you're done with the NamedNodeMap, send it
		//. a release() message to get rid of it.

	virtual NamedNodeMap* getNotations() const;
		//: A NamedNodeMap containing the notations declared
		//+ in the DTD. Duplicates are discarded.
		//+ Every node in this map also implements the
		//+ Notation interface.
		//+ When you're done with the NamedNodeMap, send it
		//. a release() message to get rid of it.

	// Node
	virtual const XMLString& getNodeName() const;
	virtual unsigned short getNodeType() const;
	
	// DOM Level 2
	virtual const XMLString& getPublicId() const;
		//; Returns the public identifier of the external DTD subset.

	virtual const XMLString& getSystemId() const;
		//; Returns the system identifier of the external DTD subset.

	virtual const XMLString& getInternalSubset() const;
		//: Returns the internal DTD subset. This implementation
		//. returns an empty string.

	// IMPLEMENTATION
	DocumentType(Document* pOwner, const XMLString& name, const XMLString& publicId, const XMLString& systemId);

protected:
	DocumentType(const DocumentType& dt);
	virtual ~DocumentType();

private:
	XMLString m_name;
	XMLString m_publicId;
	XMLString m_systemId;
};


XML_END


#endif // DOM_DocumentType_H_INCLUDED
