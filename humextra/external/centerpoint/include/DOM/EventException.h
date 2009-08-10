//
// EventException.h
//
// $Id: //depot/XML/Main/include/DOM/EventException.h#3 $
//
// Document Object Model (Event) Level 2
//
// Exception EventException
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


#ifndef DOM_EventException_H_INCLUDED
#define DOM_EventException_H_INCLUDED


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


class XML_API EventException: public CXMLException
{
public:
	enum
	{
		UNSPECIFIED_EVENT_TYPE_ERR = 0, //: If the Event's type was not specified by initializing the 
		                                //+ event before the method was called. Specification of the Event's
                                        //. type as null or an empty string will also trigger this exception. 
		_MAX_CODE_ERR
	};

	EventException(int code);
	unsigned short getCode() const;
		//: Returns the Event exception code, starting at 0
		//. (as opposed to the exception code returned by GetResult).
};


XML_END


#endif // DOM_EventException_H_INCLUDED
