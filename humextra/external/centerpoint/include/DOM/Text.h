//
// Text.h
//
// $Id: //depot/XML/Main/include/DOM/Text.h#7 $
//
// Document Object Model (Core) Level 1
//
// Interface Text
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


#ifndef DOM_Text_H_INCLUDED
#define DOM_Text_H_INCLUDED


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


class XML_API Text: public CharacterData
	//: The Text interface represents the textual content
	//. (termed character data in XML) of an Element.
{
public:
	virtual Text* splitText(unsigned long offset);
		//: Breaks this Text node into two Text nodes at the
		//+ specified offset, keeping both in the tree as
		//. siblings.

	// Node
	virtual const XMLString& getNodeName() const;
	virtual Node* cloneNode(bool deep) const;
	virtual unsigned short getNodeType() const;

	// IMPLEMENTATION
	Text(Document* pOwnerDocument, const XMLString& data);

protected:
	Text(const Text& text);
	virtual ~Text();

private:
	static const XMLString NODE_NAME;
};


XML_END


#endif // DOM_Text_H_INCLUDED
