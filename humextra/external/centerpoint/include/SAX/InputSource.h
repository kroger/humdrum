//
// InputSource.h
//
// $Id: //depot/XML/Main/include/SAX/InputSource.h#5 $
//
// SAX InputSource - A single input source for an XML entity.
//
// NOTE: You must specify either a XMLCharInputStream or
//       a XMLByteInputStream. The parser will take whatever
//       is available.
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


#ifndef SAX_InputSource_H_INCLUDED
#define SAX_InputSource_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif
#ifndef XMLStream_H_INCLUDED
#include "XMLStream.h"
#endif


XML_BEGIN


class XML_API CInputSource
{
public:
	CInputSource();
	CInputSource(const XMLString& systemId);
	CInputSource(XMLByteInputStream* bistr);
	virtual ~CInputSource();

	virtual void SetPublicId(const XMLString& publicId);
	virtual void SetSystemId(const XMLString& systemId);
	virtual XMLString GetPublicId() const;
	virtual XMLString GetSystemId() const;
	virtual void SetByteStream(XMLByteInputStream* bistr);
	virtual XMLByteInputStream* GetByteStream() const;
	virtual void SetCharacterStream(XMLCharInputStream* cistr);
	virtual XMLCharInputStream* GetCharacterStream() const;
	virtual void SetEncoding(const XMLString& encoding);
	virtual XMLString GetEncoding() const;

private:
	XMLString m_publicId;
	XMLString m_systemId;
	XMLString m_encoding;
	XMLByteInputStream* m_bistr;
	XMLCharInputStream* m_cistr;
};


XML_END


#endif // SAX_InputSource_H_INCLUDED
