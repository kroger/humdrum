//
// XMLParserBase.cpp
//
// $Id: //depot/XML/Main/src/XMLParserBase.cpp#7 $
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


#include "XMLParserBase.h"
#include "XMLStream.h"
#include "XMLUtils.h"
#include "XMLException.h"
#ifndef CSL_XML_STANDALONE
#include "CSFileSystem.h"
#endif


XML_BEGIN


#define PARSE_BUFFER_SIZE 4096


CXMLParserBase::CXMLParserBase(const XML_Char* encoding)
{
	m_parser        = NULL;
	m_nullEncoding  = encoding == NULL;
	m_encoding      = encoding == NULL ? XMLString() : XMLString(encoding);
	m_pBuffer       = new char[PARSE_BUFFER_SIZE];
	m_processNS     = false;
	m_nsSeparator   = '\0';
	m_baseSpecified = false;
	m_expandIntEnt  = true;
	m_parsing       = XML_PARAM_ENTITY_PARSING_NEVER;
}


CXMLParserBase::~CXMLParserBase()
{
	if (m_parser) XML_ParserFree(m_parser);
	if (m_pBuffer) delete [] m_pBuffer;
}


void CXMLParserBase::EnableNamespaceProcessing(bool flag)
{
	m_processNS = flag;
}


void CXMLParserBase::ExpandInternalEntities(bool flag)
{
	m_expandIntEnt = flag;
}


void CXMLParserBase::SetNamespaceSeparator(XMLChar sep)
{
	m_nsSeparator = sep;
}


void CXMLParserBase::SetEncoding(const XMLString& encoding)
{
	m_encoding = encoding;
}


void CXMLParserBase::HandleError(int errNo)
{
	throw CXMLException(CXMLException::EBase + errNo);
}	


void CXMLParserBase::Parse(XMLByteInputStream* istr)
{
	int n;
	int res;

	Initialize();

	istr->read(m_pBuffer, PARSE_BUFFER_SIZE);
	n = istr->gcount();
	while (n > 0)
	{
		res = XML_Parse(m_parser, m_pBuffer, n, 0);
		if (res == 0)
		{
			HandleError(XML_GetErrorCode(m_parser));
		}
		if (istr->good())
		{
			istr->read(m_pBuffer, PARSE_BUFFER_SIZE);
			n = istr->gcount();
		}
		else n = 0;
	}
	res = XML_Parse(m_parser, m_pBuffer, 0, 1);
	if (res == 0)
	{
		HandleError(XML_GetErrorCode(m_parser));
	}
}


void CXMLParserBase::ParseChar(XMLCharInputStream* istr)
{
	int n;
	int res;

	Initialize();

	istr->read(reinterpret_cast<XMLChar*>(m_pBuffer), PARSE_BUFFER_SIZE/sizeof(XMLChar));
	n = istr->gcount();
	while (n > 0)
	{
		res = XML_Parse(m_parser, m_pBuffer, n*sizeof(XMLChar), 0);
		if (res == 0)
		{
			HandleError(XML_GetErrorCode(m_parser));
		}
		if (istr->good())
		{
			istr->read(reinterpret_cast<XMLChar*>(m_pBuffer), PARSE_BUFFER_SIZE/sizeof(XMLChar));
			n = istr->gcount();
		}
		else n = 0;
	}
	res = XML_Parse(m_parser, m_pBuffer, 0, 1);
	if (res == 0)
	{
		HandleError(XML_GetErrorCode(m_parser));
	}
}


void CXMLParserBase::ParseExternal(XML_Parser extParser, XMLByteInputStream* istr)
{
	int n;
	int res;
	char *pBuffer = new char[PARSE_BUFFER_SIZE];

	try
	{
		istr->read(pBuffer, PARSE_BUFFER_SIZE);
		n = istr->gcount();
		while (n > 0)
		{
			res = XML_Parse(extParser, pBuffer, n, 0);
			if (res == 0)
			{
				HandleError(XML_GetErrorCode(extParser));
			}
			if (istr->good())
			{
				istr->read(pBuffer, PARSE_BUFFER_SIZE);
				n = istr->gcount();
			} 
			else n = 0;
		}
		res = XML_Parse(extParser, pBuffer, 0, 1);
		if (res == 0)
		{
			HandleError(XML_GetErrorCode(extParser));
		}
	}
	catch (...)
	{
		delete [] pBuffer;
		throw;
	}
	delete [] pBuffer;
}


void CXMLParserBase::ParseCharExternal(XML_Parser extParser, XMLCharInputStream* istr)
{
	int n;
	int res;
	XMLChar *pBuffer = new XMLChar[PARSE_BUFFER_SIZE/sizeof(XMLChar)];

	try
	{
		istr->read(pBuffer, PARSE_BUFFER_SIZE/sizeof(XMLChar));
		n = istr->gcount();
		while (n > 0)
		{
			res = XML_Parse(extParser, reinterpret_cast<char*>(pBuffer), n*sizeof(XMLChar), 0);
			if (res == 0)
			{
				HandleError(XML_GetErrorCode(extParser));
			}
			if (istr->good())
			{
				istr->read(pBuffer, PARSE_BUFFER_SIZE/sizeof(XMLChar));
				n = istr->gcount();
			} 
			else n = 0;
		}
		res = XML_Parse(extParser, reinterpret_cast<char*>(pBuffer), 0, 1);
		if (res == 0)
		{
			HandleError(XML_GetErrorCode(extParser));
		}
	}
	catch (...)
	{
		delete [] pBuffer;
		throw;
	}
	delete [] pBuffer;
}


bool CXMLParserBase::IsAbsolutePath(const XMLString& fileName)
{
#ifdef CSL_XML_STANDALONE
#ifdef _WIN32
	return fileName[0] == '\\';
#else
	return fileName[0] == '/';
#endif
#else
	CSL::Base::CPath path(CXMLUtils::XMLToAscii(fileName));
	return path.IsAbsolute();
#endif
}


XMLString CXMLParserBase::ConcatPath(const XMLString& path, const XMLString& fileName)
{
#ifdef CSL_XML_STANDALONE
	XMLString cp = path;
#ifndef _WIN32
	cp += '\\';
#else
	cp += '/';
#endif
	cp += fileName;
	return cp;
#else
	CSL::Base::CPath p1(CXMLUtils::XMLToAscii(path));
	CSL::Base::CPath p2(CXMLUtils::XMLToAscii(fileName));
	p1.AppendPath(p2);
	return CXMLUtils::AsciiToXML(p1);
#endif
}


XMLString CXMLParserBase::ResolveSystemId(const XMLString& base, const XMLString& systemId)
{
	if (IsAbsolutePath(systemId))
	{
		return systemId;
	}
	else
	{
		return ConcatPath(base, systemId);
	}
}


XMLByteInputStream* CXMLParserBase::OpenExternalEntityStream(const XMLString& fileName)
{
	return NULL;
}


void CXMLParserBase::CloseExternalEntityStream(XMLByteInputStream* istr)
{
}


void CXMLParserBase::SetBase(const XMLString& base)
{
	m_baseSpecified = true;
	m_base          = base;
	if (m_parser) XML_SetBase(m_parser, base.c_str());
}


XMLString CXMLParserBase::GetBase() const
{
	return m_base;
}


int CXMLParserBase::GetCurrentLineNumber() const
{
	return XML_GetCurrentLineNumber(m_parser);
}


int CXMLParserBase::GetCurrentColumnNumber() const
{
	return XML_GetCurrentColumnNumber(m_parser);
}


int CXMLParserBase::GetCurrentByteIndex() const
{
	return XML_GetCurrentByteIndex(m_parser);
}


int CXMLParserBase::GetSpecifiedAttributeCount() const
{
	return XML_GetSpecifiedAttributeCount(m_parser);
}


void CXMLParserBase::Initialize()
{
	if (m_parser) 
	{
		XML_ParserFree(m_parser);
	}

	if (m_processNS)
	{
		m_parser = XML_ParserCreateNS(m_nullEncoding ? NULL : m_encoding.c_str(), m_nsSeparator);
	}
	else
	{
		m_parser = XML_ParserCreate(m_nullEncoding ? NULL : m_encoding.c_str());
	}
	XML_SetElementHandler(m_parser, StartElementHandler, EndElementHandler);
	XML_SetCharacterDataHandler(m_parser, CharacterDataHandler);
	XML_SetProcessingInstructionHandler(m_parser, ProcessingInstructionHandler);
	if (m_expandIntEnt)
	{
		XML_SetDefaultHandlerExpand(m_parser, DefaultHandler);
	}
	else
	{
		XML_SetDefaultHandler(m_parser, DefaultHandler);
	}
	XML_SetUnparsedEntityDeclHandler(m_parser, UnparsedEntityDeclHandler);
	XML_SetNotationDeclHandler(m_parser, NotationDeclHandler);
	XML_SetExternalEntityRefHandler(m_parser, ExternalEntityRefHandler);
	XML_SetUnknownEncodingHandler(m_parser, UnknownEncodingHandler, this);
	XML_SetCommentHandler(m_parser, CommentHandler);
	XML_SetCdataSectionHandler(m_parser, StartCdataSectionHandler, EndCdataSectionHandler);
	if (m_processNS)
	{
		XML_SetNamespaceDeclHandler(m_parser, StartNamespaceDeclHandler, EndNamespaceDeclHandler);
	}
	XML_SetUserData(m_parser, this);
	XML_SetParamEntityParsing(m_parser, m_parsing);
	if (m_baseSpecified) 
	{
		XML_SetBase(m_parser, m_base.c_str());
	}
}


void CXMLParserBase::HandleStartElement(const XML_Char* name, const XML_Char** atts)
{
}


void CXMLParserBase::HandleEndElement(const XML_Char* name)
{
}


void CXMLParserBase::HandleCharacterData(const XML_Char* s, int len)
{
}


void CXMLParserBase::HandleProcessingInstruction(const XML_Char* target, const XML_Char* data)
{
}


void CXMLParserBase::HandleDefault(const XML_Char* s, int len)
{
}


void CXMLParserBase::HandleUnparsedEntityDecl(const XML_Char* entityName, const XML_Char* base,
		                                      const XML_Char* systemId, const XML_Char* publicId,
					        		          const XML_Char* notationName)
{
}


void CXMLParserBase::HandleNotationDecl(const XML_Char* notationName, const XML_Char* base,
		                                const XML_Char* systemId, const XML_Char* publicId)
{
}


int CXMLParserBase::HandleExternalEntityRef(const XML_Char* openEntityNames, const XML_Char* base,
										    const XML_Char* systemId, const XML_Char* publicId)
{
	XMLString fileName;
	XMLByteInputStream* istr;

	XML_Parser extParser = XML_ExternalEntityParserCreate(m_parser, openEntityNames, 0);
	fileName = ResolveSystemId((base == NULL ? XMLString() : XMLString(base)), systemId);
	XML_SetBase(extParser, base);
	istr = OpenExternalEntityStream(fileName);
	if (istr)
	{
		try
		{
			ParseExternal(extParser, istr);
		}
		catch (CXMLException& e)
		{
			// cleanup before we propagate the exception
			CloseExternalEntityStream(istr);
			XML_ParserFree(extParser);
			throw e;
		}
		CloseExternalEntityStream(istr);
	}
	XML_ParserFree(extParser);
	
	return 1; // parsing was successfull
}	


int CXMLParserBase::HandleUnknownEncoding(const XML_Char* name, XML_Encoding* info)
{
	return 0; // we cannot provide encoding information
}


void CXMLParserBase::HandleComment(const XML_Char* data)
{
}


void CXMLParserBase::HandleStartCDataSection()
{
}


void CXMLParserBase::HandleEndCDataSection()
{
}


void CXMLParserBase::HandleStartNamespace(const XML_Char* prefix, const XML_Char* uri)
{
}


void CXMLParserBase::HandleEndNamespace(const XML_Char* prefix)
{
}


void CXMLParserBase::HandleStartDoctypeDecl(const XML_Char* doctypeName)
{
}


void CXMLParserBase::HandleEndDoctypeDecl()
{
}


void CXMLParserBase::HandleExternalParsedEntityDecl(const XML_Char* entityName, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId)
{
}


void CXMLParserBase::HandleInternalParsedEntityDecl(const XML_Char* entityName, const XML_Char* replacementText, int replacementTextLength)
{
}


void CXMLParserBase::StartElementHandler(void* userData, const XML_Char* name, const XML_Char** atts)
{
	static_cast<CXMLParserBase*>(userData)->HandleStartElement(name, atts);
}


void CXMLParserBase::EndElementHandler(void* userData, const XML_Char* name)
{
	static_cast<CXMLParserBase*>(userData)->HandleEndElement(name);
}


void CXMLParserBase::CharacterDataHandler(void* userData, const XML_Char* s, int len)
{
	static_cast<CXMLParserBase*>(userData)->HandleCharacterData(s, len);
}


void CXMLParserBase::ProcessingInstructionHandler(void* userData, const XML_Char* target, const XML_Char* data)
{
	static_cast<CXMLParserBase*>(userData)->HandleProcessingInstruction(target, data);
}


void CXMLParserBase::DefaultHandler(void* userData, const XML_Char* s, int len)
{
	static_cast<CXMLParserBase*>(userData)->HandleDefault(s, len);
}


void CXMLParserBase::UnparsedEntityDeclHandler(void* userData, const XML_Char* entityName, const XML_Char* base,
	                                           const XML_Char* systemId, const XML_Char* publicId,
									           const XML_Char* notationName)
{
	static_cast<CXMLParserBase*>(userData)->HandleUnparsedEntityDecl(entityName, base, systemId, publicId, notationName);
}


void CXMLParserBase::NotationDeclHandler(void* userData, const XML_Char* notationName, const XML_Char* base,
	                                     const XML_Char* systemId, const XML_Char* publicId)
{
	static_cast<CXMLParserBase*>(userData)->HandleNotationDecl(notationName, base, systemId, publicId);
}


int CXMLParserBase::ExternalEntityRefHandler(XML_Parser parser,
	                                         const XML_Char* openEntityNames, const XML_Char* base,
									         const XML_Char* systemId, const XML_Char* publicId)
{
	return static_cast<CXMLParserBase*>(XML_GetUserData(parser))->HandleExternalEntityRef(openEntityNames, base, systemId, publicId);
}


int CXMLParserBase::UnknownEncodingHandler(void* encodingHandlerData, const XML_Char* name, XML_Encoding* info)
{
	return static_cast<CXMLParserBase*>(encodingHandlerData)->HandleUnknownEncoding(name, info);
}


void CXMLParserBase::CommentHandler(void* userData, const XML_Char* data)
{
	static_cast<CXMLParserBase*>(userData)->HandleComment(data);
}


void CXMLParserBase::StartCdataSectionHandler(void* userData)
{
	static_cast<CXMLParserBase*>(userData)->HandleStartCDataSection();
}


void CXMLParserBase::EndCdataSectionHandler(void* userData)
{
	static_cast<CXMLParserBase*>(userData)->HandleEndCDataSection();
}


void CXMLParserBase::StartNamespaceDeclHandler(void* userData, const XML_Char* prefix, const XML_Char* uri)
{
	static_cast<CXMLParserBase*>(userData)->HandleStartNamespace(prefix, uri);
}


void CXMLParserBase::EndNamespaceDeclHandler(void* userData, const XML_Char* prefix)
{
	static_cast<CXMLParserBase*>(userData)->HandleEndNamespace(prefix);
}


void CXMLParserBase::StartDoctypeDeclHandler(void* userData, const XML_Char* doctypeName)
{
	static_cast<CXMLParserBase*>(userData)->HandleStartDoctypeDecl(doctypeName);
}


void CXMLParserBase::EndDoctypeDeclHandler(void* userData)
{
	static_cast<CXMLParserBase*>(userData)->HandleEndDoctypeDecl();
}


void CXMLParserBase::ExternalParsedEntityDeclHandler(void* userData, const XML_Char* entityName, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId)
{
	static_cast<CXMLParserBase*>(userData)->HandleExternalParsedEntityDecl(entityName, base, systemId, publicId);
}


void CXMLParserBase::InternalParsedEntityDeclHandler(void* userData, const XML_Char* entityName, const XML_Char* replacementText, int replacementTextLength)
{
	static_cast<CXMLParserBase*>(userData)->HandleInternalParsedEntityDecl(entityName, replacementText, replacementTextLength);
}


XML_Parser CXMLParserBase::GetExpatParser() const
{
	return m_parser;
}


void CXMLParserBase::SetParamEntityParsing(enum XML_ParamEntityParsing parsing)
{
	m_parsing = parsing;
}



XML_END
