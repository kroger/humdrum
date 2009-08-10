//
// XMLException.h
//
// $Id: //depot/XML/Main/include/XMLException.h#12 $
//
// Base class for all XML and SAX exceptions
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


#ifndef XMLException_H_INCLUDED
#define XMLException_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifdef CSL_XML_STANDALONE
#include <exception>
#else
#ifndef CSException_H_INCLUDED
#include "CSException.h"
#endif
#endif
#ifndef STD_STRING_INCLUDED
#include <string>
#define STD_STRING_INCLUDED
#endif


XML_BEGIN


#ifdef CSL_XML_STANDALONE
class XML_API CXMLException
#else
class XML_API CXMLException: public CSL::Base::CException
#endif
{
public:
	enum
	{
		EBase = 0x04400,
		EXMLException = EBase,
		EXMLNoMemory,                    // begin expat error codes (must be the first ones)
		EXMLSyntax,
		EXMLNoElements,
		EXMLInvalidToken,
		EXMLUnclosedToken,
		EXMLPartialChar,
		EXMLTagMismatch,
		EXMLDuplicateAttribute,
		EXMLJunkAfterDocElement,
		EXMLParamEntityRef,
		EXMLUndefinedEntity,
		EXMLRecursiveEntityRef,
		EXMLAsyncEntity,
		EXMLBadCharRef,
		EXMLBinaryEntityRef,
		EXMLAttributeExternalEntityRef,
		EXMLMisplacedPI,
		EXMLUnknownEncoding,
		EXMLIncorrectEncoding,
		EXMLUnclosedCDATASection,
		EXMLExternalEntityHandling,
		EXMLNotStandalone,               // end expat error codes

		ESAXException,                   // begin SAX exception codes
		ESAXNotSupportedException,
		ESAXNotRecognizedException,
		ESAXParseException,              // end SAX error codes

		EXMLBadSystemId,                 // begin XMLParser exception codes
		EXMLBadInputSource,              // end XMLParser exception codes

		EXMLNamePoolFull,

		EDOMException,                   // begin DOM exception codes
		EDOMIndexSizeErr,
		EDOMStringSizeErr,
		EDOMHierarchyRequestErr,
		EDOMWrongDocumentErr,
		EDOMInvalidCharacterErr,
		EDOMNoDataAllowedErr,
		EDOMNoModificationAllowedErr,
		EDOMNotFoundErr,
		EDOMNotSupportedErr,
		EDOMInUseAttributeErr,           // end DOM level 1 exception codes
		EDOMInvalidStateErr,             // begin DOM level 2 exception codes
		EDOMSyntaxErr,
		EDOMInvalidModificationErr,
		EDOMNamespaceErr,
		EDOMInvalidAccessErr,            // end DOM level 2 exception codes
		EDOMUnspecifiedEventTypeErr,     // begin/end DOM level 2 Event exception codes
		EXMLIOError                      // generic input/output error
	};

	CXMLException(long errorCode);
	CXMLException(long errorCode, const string& errMsg);
	CXMLException(const CXMLException& e);
	~CXMLException() throw();

#ifdef CSL_XML_STANDALONE
public:
	long GetResult() const;
	string GetMsg() const;
	void SetMsg(const string& msg);
	const char* what() const throw();

private:
	long   m_errorCode;
	string m_strMessage;
#else
protected:
	const char* GetStaticErrorText() const throw();
#endif
};


XML_END


#endif // XMLException_H_INCLUDED
