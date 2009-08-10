//
// Notation.h
//
// $Id: //depot/XML/Main/include/DOM/Notation.h#7 $
//
// Document Object Model (Core) Level 1
//
// Interface Notation
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


#ifndef DOM_Notation_H_INCLUDED
#define DOM_Notation_H_INCLUDED


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


class XML_API Notation: public SimpleNode
	//; This interface represents a notation declared in the DTD.
{
public:
	virtual const XMLString& getPublicId() const;
		//; Returns the public identifier of this notation.

	virtual const XMLString& getSystemId() const;
		//; Returns the system identifier of this notation.

	// Node
	virtual const XMLString& getNodeName() const;
	virtual unsigned short getNodeType() const;

	// IMPLEMENTATION
	Notation(Document* pOwnerDocument, const XMLString& name, const XMLString& publicId, const XMLString& systemId);

protected:
	Notation(const Notation& notation);
	virtual ~Notation();

private:
	XMLString m_name;
	XMLString m_publicId;
	XMLString m_systemId;
};


XML_END


#endif // DOM_Notation_H_INCLUDED
