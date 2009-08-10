//
// EntityResolverImp.h
//
// $Id: //depot/XML/Main/include/SAX/EntityResolverImp.h#5 $
//
// An implementation of EntityResolver provided for your convenience.
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


#ifndef SAX_EntityResolverImp_H_INCLUDED
#define SAX_EntityResolverImp_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_EntityResolver_H_INCLUDED
#include "SAX/EntityResolver.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif


XML_BEGIN


class XML_API CEntityResolverImp: public CEntityResolver
{
public:
	virtual CInputSource* ResolveEntity(const XMLString* publicId, const XMLString& systemId);
	virtual void DoneWithInputSource(CInputSource* pSource);
};


XML_END


#endif // SAX_EntityResolverImp_H_INCLUDED
