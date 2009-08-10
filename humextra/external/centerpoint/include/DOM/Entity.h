//
// Entity.h
//
// $Id: //depot/XML/Main/include/DOM/Entity.h#7 $
//
// Document Object Model (Core) Level 1
//
// Interface Entity
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


#ifndef DOM_Entity_H_INCLUDED
#define DOM_Entity_H_INCLUDED


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


class XML_API Entity: public ContainerNode
	//: This interface represents an entity, either parsed
	//+ or unparsed, in an XML document. Note that this models
	//. the entity itself, not the entity declaration.
{
public:
	virtual const XMLString& getPublicId() const;
		//: Returns the public identifier associated with
		//. the entity, if specified.

	virtual const XMLString& getSystemId() const;
		//: Returns the system identifier associated with
		//. the entity, if specified.

	virtual const XMLString& getNotationName() const;
		//: Returns, for unparsed entities, the name of the
		//. notation for the entity.

	// Node
	virtual const XMLString& getNodeName() const;
	virtual unsigned short getNodeType() const;

	// IMPLEMENTATION
	Entity(Document* pOwnerDocument, const XMLString& name, const XMLString& publicId, const XMLString& systemId);

protected:
	Entity(const Entity& entity);
	virtual ~Entity();

private:
	static const XMLString NODE_NAME;

	XMLString m_name;
	XMLString m_publicId;
	XMLString m_systemId;
};


XML_END


#endif // DOM_Entity_H_INCLUDED
