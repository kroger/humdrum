//
// CharacterData.h
//
// $Id: //depot/XML/Main/include/DOM/CharacterData.h#7 $
//
// Document Object Model (Core) Level 1
//
// Interface CharacterData
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


#ifndef DOM_CharacterData_H_INCLUDED
#define DOM_CharacterData_H_INCLUDED


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


class XML_API CharacterData: public SimpleNode
	//: The CharacterData interface extends Node with a set of
	//+ attributes and methods for accessing data in the DOM.
	//+ For clarity this set is defined here rather than on each
	//+ object that uses these attributes and methods. No DOM
	//+ objects correspond directly to CharacterData, though
	//+ Text and others do inherit the interface from it.
	//. All offsets in this interface start from 0.
{
public:
	virtual const XMLString& getData() const;
		//: Returns the character data of the node that 
		//. implements the interface.

	virtual void setData(const XMLString& data);
		//: Sets the character data of the node that
		//. implements the interface.

	virtual unsigned long getLength() const;
		//: Returns the number of characters that are available
		//+ through getData and substringData. This may have the
		//. value zero.

	virtual XMLString substringData(unsigned long offset, unsigned long count) const;
		//: Extracts a range of data from the node.
		//+ If offset and count exceeds the length, then all
		//. the characters to the end of the data are returned.

	virtual void appendData(const XMLString& arg);
		//: Append the string to the end of the character data
		//. of the node.

	virtual void insertData(unsigned long offset, const XMLString& arg);
		//; Insert a string at the specified character offset.

	virtual void deleteData(unsigned long offset, unsigned long count);
		//; Remove a range of characters from the node.

	virtual void replaceData(unsigned long offset, unsigned long count, const XMLString& arg);
		//: Replace the characters starting at the specified character
		//. offset with the specified string.

	// Node
	virtual const XMLString& getNodeValue() const;
	virtual void setNodeValue(const XMLString& value);

	// IMPLEMENTATION
	CharacterData(Document* pOwnerDocument, const XMLString& data);

protected:
	CharacterData(const CharacterData& data);
	virtual ~CharacterData();

private:
	XMLString m_data;
};


XML_END


#endif // DOM_CharacterData_H_INCLUDED
