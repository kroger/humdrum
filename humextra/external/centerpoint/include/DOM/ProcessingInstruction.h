//
// ProcessingInstruction.h
//
// $Id: //depot/XML/Main/include/DOM/ProcessingInstruction.h#8 $
//
// Document Object Model (Core) Level 1
//
// Interface ProcessingInstruction
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


#ifndef DOM_ProcessingInstruction_H_INCLUDED
#define DOM_ProcessingInstruction_H_INCLUDED


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


class XML_API ProcessingInstruction: public SimpleNode
	//: The ProcessingInstruction interface represents a
	//+ "processing instruction", used in XML as a way
	//+ to keep processor-specific information in the text
	//. of the document.
{
public:
	virtual const XMLString& getTarget() const;
		//: Returns the target of this processing instruction.
		//+ XML defines this as being the first token following
		//. the markup that begins the processing instruction.

	virtual const XMLString& getData() const;
		//: Returns the content of this processing instruction.
		//+ This is the first non white space character after
		//+ the target to the character immediately preceding
		//. the ?>.

	virtual void setData(const XMLString& data);
		//; Sets the content of this processing instruction.

	// Node
	virtual const XMLString& getNodeName() const;
	virtual const XMLString& getNodeValue() const;
	virtual void setNodeValue(const XMLString& data);
	virtual Node* cloneNode(bool deep) const;
	virtual unsigned short getNodeType() const;

	// IMPLEMENTATION
	ProcessingInstruction(Document* pOwnerDocument, const XMLString& target, const XMLString& data);

protected:
	ProcessingInstruction(const ProcessingInstruction& pi);
	virtual ~ProcessingInstruction();

private:
	XMLString m_target;
	XMLString m_data;
};


XML_END


#endif // DOM_ProcessingInstruction_H_INCLUDED
