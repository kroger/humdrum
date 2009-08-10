//
// CDATASection.h
//
// $Id: //depot/XML/Main/include/DOM/CDATASection.h#7 $
//
// Document Object Model (Core) Level 1
//
// Interface CDATASection
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


#ifndef DOM_CDATASection_H_INCLUDED
#define DOM_CDATASection_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef DOM_Text_H_INCLUDED
#include "DOM/Text.h"
#endif


XML_BEGIN


class Document;


class XML_API CDATASection: public Text
	//: CDATA sections are used to escape blocks of text
	//+ containing characters that would otherwise be regarded
	//+ as markup.
	//+ The CDATASection interface inherits the CharacterData interface
	//. through the Text interface.
{
public:
	// Text
	Text* splitText(unsigned long offset);

	// Node
	virtual const XMLString& getNodeName() const;
	virtual Node* cloneNode(bool deep) const;
	virtual unsigned short getNodeType() const;

	// IMPLEMENTATION
	CDATASection(Document* pOwnerDocument, const XMLString& data);

protected:
	CDATASection(const CDATASection& sec);
	virtual ~CDATASection();

private:
	static const XMLString NODE_NAME;
};


XML_END


#endif // DOM_CDATASection_H_INCLUDED
