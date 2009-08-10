//
// Configurable.h
//
// $Id: //depot/XML/Main/include/SAX/Configurable.h#6 $
//
// SAX2-alpha Configurable Interface.
//
// DEPRECATED. No longer exists in the final SAX2 specification.
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


#ifndef SAX_Configurable_H_INCLUDED
#define SAX_Configurable_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class XML_API CConfigurable
{
public:
	virtual void SetFeature(const XMLString& featureId, bool state) = 0;
	virtual bool GetFeature(const XMLString& featureId) const = 0;
	virtual void SetProperty(const XMLString& propertyId, const XMLString& value) = 0;
	virtual void SetProperty(const XMLString& propertyId, void* value) = 0;
	virtual void* GetProperty(const XMLString& propertyId) const = 0;
		//; NOTE: string values are returned as XMLChar*

	virtual ~CConfigurable();
};


XML_END


#endif // SAX_Configurable_H_INCLUDED
