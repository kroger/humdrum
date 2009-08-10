//
// EntityReference.h
//
// $Id: //depot/XML/Main/include/DOM/EntityReference.h#7 $
//
// Document Object Model (Core) Level 1
//
// Interface EntityReference
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


#ifndef DOM_EntityReference_H_INCLUDED
#define DOM_EntityReference_H_INCLUDED


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


class XML_API EntityReference: public SimpleNode
	//: EntityReference objects may be inserted into the
	//+ structure model when an entity reference is in the
	//+ source document, or when the user wishes to insert
	//+ an entity reference.
	//. EntityReference is currently unsupported in this implementation.
{
public:
	// Node
	const XMLString& getNodeName() const;
	unsigned short getNodeType() const;

	// IMPLEMENTATION
	EntityReference(Document* pOwnerDocument, const XMLString& name);

protected:
	EntityReference(const EntityReference& ref);
	virtual ~EntityReference();

private:
	XMLString m_name;
};


XML_END


#endif // DOM_EntityReference_H_INCLUDED
