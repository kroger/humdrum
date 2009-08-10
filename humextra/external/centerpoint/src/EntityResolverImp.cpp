//
// EntityResolverImp.cpp
//
// $Id: //depot/XML/Main/src/EntityResolverImp.cpp#3 $
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


#include "SAX/EntityResolverImp.h"
#include "SAX/InputSource.h"
#include "XMLUtils.h"
#include <fstream>


XML_BEGIN


CInputSource* CEntityResolverImp::ResolveEntity(const XMLString* publicId, const XMLString& systemId)
{
	CInputSource* pInputSource = new CInputSource();

	pInputSource->SetSystemId(systemId);
	if (publicId) pInputSource->SetPublicId(*publicId);

	// Since we do not know better, we create
	// a byte input stream.
	ifstream* pFile = new ifstream();

	pFile->open(CXMLUtils::XMLToAscii(systemId).c_str(), ios::in);
	if (pFile->good())
	{
		pInputSource->SetByteStream(pFile);
		return pInputSource;
	}
	else
	{
		delete pInputSource;
		delete pFile;
		return NULL;
	}
}

		
void CEntityResolverImp::DoneWithInputSource(CInputSource* pSource)
{
	delete pSource->GetByteStream();
	delete pSource;
}


XML_END
