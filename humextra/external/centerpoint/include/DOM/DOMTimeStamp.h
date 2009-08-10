//
// DOMTimeStamp.h
//
// $Id$
//
// Document Object Model (Core) Level 2
//
// Type DOMTimeStamp
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


#ifndef DOM_DOMTimeStamp_H_INCLUDED
#define DOM_DOMTimeStamp_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif


XML_BEGIN


#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int64 DOMTimeStamp;
#else
typedef unsigned long long DOMTimeStamp;
#endif


XML_END


#endif // DOM_DOMTimeStamp_H_INCLUDED
