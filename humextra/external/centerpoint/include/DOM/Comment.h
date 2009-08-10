//
// Comment.h
//
// $Id: //depot/XML/Main/include/DOM/Comment.h#7 $
//
// Document Object Model (Core) Level 1
//
// Interface Comment
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


#ifndef DOM_Comment_H_INCLUDED
#define DOM_Comment_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef DOM_CharacterData_H_INCLUDED
#include "DOM/CharacterData.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class Document;


class XML_API Comment: public CharacterData
	//: This represents the content of a comment, i.e. all the
	//. characters between the starting '<!--' and ending '-->'.
{
public:
	// Node
	virtual const XMLString& getNodeName() const;
	virtual Node* cloneNode(bool deep) const;
	virtual unsigned short getNodeType() const;

	// IMPLEMENTATION
	Comment(Document* pOwnerDocument, const XMLString& data);

protected:
	Comment(const Comment& comment);
	virtual ~Comment();

private:
	static const XMLString NODE_NAME;
};


XML_END


#endif // DOM_Comment_H_INCLUDED
