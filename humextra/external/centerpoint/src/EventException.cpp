//
// EventException.cpp
//
// $Id: //depot/XML/Main/src/EventException.cpp#2 $
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



#include "DOM/EventException.h"


XML_BEGIN


EventException::EventException(int code): CXMLException(code < EventException::_MAX_CODE_ERR ? code + CXMLException::EDOMUnspecifiedEventTypeErr : code)
{
}


unsigned short EventException::getCode() const
{
	return GetResult() - CXMLException::EDOMUnspecifiedEventTypeErr;
}


XML_END
