//
// XMLStringPool.h
//
// $Id: //depot/XML/Main/include/XMLStringPool.h#5 $
//
// A re-implementation of CStringPool for XMLString.
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


#ifndef XMLStringPool_H_INCLUDED
#define XMLStringPool_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif
#ifndef CSL_XML_STANDALONE
#ifndef CSMultiThreading_H_INCLUDED
#include "CSMultiThreading.h"
#endif
#endif


XML_BEGIN


class XML_API CXMLStringPool
	//: A CXMLStringPool is very useful if you have many strings with the
	//+ same content (such as element names in XML documents) and you want
	//+ to save some memory. The CXMLStringPool utilizes the fact that the
	//+ C++ string class uses reference counting to share the actual
	//+ string data between two or more strings, if you assign one
	//+ to another. The string pool is multi-threading safe, so you
	//. can access the same pool from different threads.
{
private:
	struct Item
	{
		bool      m_used;
		XMLString m_data;
	};
	Item*             m_pPool;
	unsigned long     m_size;
#ifndef CSL_XML_STANDALONE
	CSL::Base::CMutex m_mtx;
#endif

	static unsigned long Hash(const XMLString& str);
	
public:
	CXMLStringPool(unsigned long size = 71);
		//; Creates a CStringPool with room for size strings.

	~CXMLStringPool();
	
	const XMLString& Get(const XMLString& str);	
		//: If a string equal to str is already in the pool, a reference
		//+ to that string is returned. Otherwise, str is added to the
		//+ pool and a reference to it is returned.
		//. If the pool is full, a reference to str is returned.
};


XML_END


#endif // XMLStringPool_H_INCLUDED
