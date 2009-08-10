//
// DOMException.h
//
// $Id: //depot/XML/Main/include/DOM/DOMException.h#7 $
//
// Document Object Model (Core) Level 1
//
// Exception DOMException
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


#ifndef DOM_DOMException_H_INCLUDED
#define DOM_DOMException_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLException_H_INCLUDED
#include "XMLException.h"
#endif
#ifndef STD_STRING_INCLUDED
#include <string>
#define STD_STRING_INCLUDED
#endif


XML_BEGIN


class XML_API DOMException: public CXMLException
{
public:
	enum
	{
		INDEX_SIZE_ERR = 1,          //; index or size is negative or greater than allowed value
		DOMSTRING_SIZE_ERR,          //; the specified range of text does not fit into a DOMString (not used)
		HIERARCHY_REQUEST_ERR,       //; a node is inserted somewhere it doesn't belong
		WRONG_DOCUMENT_ERR,          //; a node is used in a different document thatn the one that created it
		INVALID_CHARACTER_ERR,       //; an invalid character is specified (not used)
		NO_DATA_ALLOWED_ERR,         //; data is specified for a node which does not support data
		NO_MODIFICATION_ALLOWED_ERR, //; an attempt is made to modify an object where modifications are not allowed
		NOT_FOUND_ERR,               //; an attempt was made to reference a node in a context where it does not exist
		NOT_SUPPORTED_ERR,           //; the implementation does not support the type of object requested
		INUSE_ATTRIBUTE_ERR,         //; an attempt is made to add an attribute that is already in use elsewhere
		INVALID_STATE_ERR,           //; a parameter or an operation is not supported by the underlying object
		SYNTAX_ERR,                  //; an invalid or illegal string is specified
		INVALID_MODIFICATION_ERR,    //; an attempt is made to modify the type of the underlying object
		NAMESPACE_ERR,               //; an attempt is made to create or change an object in a way which is incorrect with regard to namespaces
		INVALID_ACCESS_ERR,          //; an attempt is made to use an object that is not, or is no longer, usable
		_MAX_CODE_ERR
	};

	DOMException(int code);
	DOMException(const DOMException& exc);
	~DOMException() throw();

	unsigned short getCode() const;
		//: Returns the DOM exception code, starting at 1
		//. (as opposed to the exception code returned by GetResult).
};


XML_END


#endif // DOM_DOMException_H_INCLUDED
