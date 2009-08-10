//
// XMLParserBase.h
//
// $Id: //depot/XML/Main/include/XMLParserBase.h#8 $
//
// Definition of CXMLParserBase.
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


#ifndef XMLParserBase_H_INCLUDED
#define XMLParserBase_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef XmlParse_INCLUDED
#include "xmlparse.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif
#ifndef XMLStream_H_INCLUDED
#include "XMLStream.h"
#endif


XML_BEGIN


class XML_API CXMLParserBase
	//: This class provides an object-oriented, stream-based, 
	//+ low-level interface to the XML Parser Toolkit (expat).
	//+ Requires expat version 1.02 or higher.
	//+
	//+ NOTE: It is strongly recommended, that you use
	//+ the SAX parser classes (which are based upon this
	//+ class) instead of this class, since they provide 
	//. a standardized, higher-level interface to the parser.
{
public:
	CXMLParserBase(const XML_Char* encoding = NULL);
		//; Encoding is passed on to Initialize.

	virtual ~CXMLParserBase();

	virtual void Parse(XMLByteInputStream* istr);
	virtual void ParseChar(XMLCharInputStream* istr);
		//; Parses an entity from the given stream.

	// The following methods are wrappers for the
	// corresponding expat functions.
	virtual void SetBase(const XMLString& base);
		//; Sets the base for the current entity.

	virtual XMLString GetBase() const;
		//; Returns the base for the current entity.
	
	virtual int GetCurrentLineNumber() const;
		//; Returns the current line number.

	virtual int GetCurrentColumnNumber() const;
		//; Returns the current column number.

	virtual int GetCurrentByteIndex() const;
		//; Returns the current byte index.

	virtual int GetSpecifiedAttributeCount() const;
		//: Returns the number of attributes, passed to the current
		//+ call of HandleStartElement, that were specified in
		//+ the start tag rather than defaulted.
		//. May only be called from within the HandleStartElement handler.

	// New for expat 1.1
	virtual void EnableNamespaceProcessing(bool flag = true);
		//; Enables or disables Namespace processing.

	virtual void SetNamespaceSeparator(XMLChar sep);
		//: Sets the namespace separator character.
		//. If '\0' is specified, no separator is used.

	virtual void SetEncoding(const XMLString& encoding);
		//; Sets the encoding used by expat.

	virtual void ExpandInternalEntities(bool flag = true);
		//: Enables/disables expansion of internal entities (enabled by
		//+ default). If entity expansion is disabled, internal entities 
		//. are reported via the default handler.

	virtual void SetParamEntityParsing(enum XML_ParamEntityParsing parsing);
		//: Controls parsing of parameter entities (including the external DTD
		//+ subset). If parsing of parameter entities is enabled, then references
		//+ to external parameter entities (including the external DTD subset)
		//+ will be passed to HandleExternalEntityRef().
		//+ The context passed will be 0.
		//+ Unlike external general entities, external parameter entities can only
		//+ be parsed synchronously.  If the external parameter entity is to be
		//+ parsed, it must be parsed during the call to the external entity ref
		//. handler. 

protected:
	virtual void Initialize();
		//; Initializes expat

	virtual bool IsAbsolutePath(const XMLString& fileName);
		//: Returns true if the given fileName contains an absolute path.
		//. Called by ResolveSystemId.

	virtual XMLString ConcatPath(const XMLString& path, const XMLString& fileName);
		//: Constructs and returns a path name by concatenating path and fileName.
		//. Called by ResolveSystemId.

	virtual XMLString ResolveSystemId(const XMLString& base, const XMLString& systemId);
		//: Constructs and returns a path from base and systemId.
		//+ If systemId contains an absolute path (see IsAbsolutePath()), 
		//+ systemId is returned. Otherwise, base and systemId are concatenated 
		//+ (see ConcatPath) and the result is returned.
		//+ Note that this only works with native filesystem paths.
		//+ If you're working with URL's etc., you should write your own implementation
		//+ of this method.

		//+ see IsAbsolutePath, ConcatPath

		//.

	virtual XMLByteInputStream* OpenExternalEntityStream(const XMLString& fileName);
		//: Creates and opens a stream for parsing an external entity.
		//+ The default implementation just returns NULL.
		//+ A simple implementation for this method would be:
		//. return new ifstream(CXMLUtils::XMLToAscii(fileName).c_str());

	virtual void CloseExternalEntityStream(XMLByteInputStream* istr);
		//: Closes (and possibly deletes) a stream created for parsing an external entity.
		//+ The default implementation does nothing.
		//+ If OpenExternalEntityStream() is implemented as above, a simple 
		//+ implementation for this method would be:
		//. delete istr;

	virtual void ParseExternal(XML_Parser extParser, XMLByteInputStream* istr);
	virtual void ParseCharExternal(XML_Parser extParser, XMLCharInputStream* istr);
		//; Parses an external entity from the given stream, with a separate parser.

	// expat handler methods
	virtual void HandleStartElement(const XML_Char* name, const XML_Char** atts);
		//: atts is array of name/value pairs, terminated by 0;
		//. names and values are 0 terminated.

	virtual void HandleEndElement(const XML_Char* name);
	
	virtual void HandleCharacterData(const XML_Char* s, int len);
		//; s is not 0 terminated.

	virtual void HandleProcessingInstruction(const XML_Char* target, const XML_Char* data);
		//; target and data are 0 terminated

	virtual void HandleDefault(const XML_Char* s, int len);
		//: This is called for any characters in the XML document for which there is no
		//+ appropriate handler. This includes both
		//+ characters that are part of markup which is of a kind that is
		//+ not reported (comments, markup declarations), or characters
		//+ that are part of a construct which could be reported but
		//+ for which no handler has been supplied. The characters are passed
		//+ exactly as they were in the XML document except that
		//. they will be encoded in UTF-8.

	virtual void HandleUnparsedEntityDecl(const XML_Char* entityName, const XML_Char* base,
		                                  const XML_Char* systemId, const XML_Char* publicId,
					        		      const XML_Char* notationName);
		//: This is called for a declaration of an unparsed (NDATA)
		//+ entity.  The base argument is whatever was set by XML_SetBase.
		//+ The entityName, systemId and notationName arguments will never be null.
		//. The other arguments may be.

	virtual void HandleNotationDecl(const XML_Char* notationName, const XML_Char* base,
		                            const XML_Char* systemId, const XML_Char* publicId);
		//: This is called for a declaration of notation.
		//+ The base argument is whatever was set by XML_SetBase.
		//. The notationName will never be null.  The other arguments can be.

	virtual int HandleExternalEntityRef(const XML_Char* openEntityNames, const XML_Char* base,
									    const XML_Char* systemId, const XML_Char* publicId);
		//: The default implementation tries to resolve the systemId (using ResolveSystemId())
		//+ and calls OpenExternalEntityStream() to open a stream for parsing the external entity.
		//+ The external entity is parsed (using ParseExternal()), and, upon completion, the
		//+ CloseExternalEntityHandler() is called to close the stream.
		//. Note that publicId may be NULL.

	virtual int HandleUnknownEncoding(const XML_Char* name, XML_Encoding* info);
		//: This is called for an encoding that is unknown to the parser.
		//+ The encodingHandlerData argument is that which was passed as the
		//+ second argument to XML_SetUnknownEncodingHandler.
		//+ The name argument gives the name of the encoding as specified in
		//+ the encoding declaration.
		//+ If the callback can provide information about the encoding,
		//+ it must fill in the XML_Encoding structure, and return 1.
		//+ Otherwise it must return 0.
		//+ If info does not describe a suitable encoding,
		//+ then the parser will return an XML_UNKNOWN_ENCODING error.
		//. Default implementation returns 0.

	// new 1.1
	virtual void HandleComment(const XML_Char* data);
		//; data is 0 terminated

	virtual void HandleStartCDataSection();
	virtual void HandleEndCDataSection();
	
	virtual void HandleStartNamespace(const XML_Char* prefix, const XML_Char* uri);
		//: When namespace processing is enabled, these are called once for
		//+ each namespace declaration. The call to the start and end element
		//+ handlers occur between the calls to the start and end namespace
		//+ declaration handlers. For an xmlns attribute, prefix will be null.
		//. For an xmlns="" attribute, uri will be null.

	virtual void HandleEndNamespace(const XML_Char* prefix);
	
	// new 1.2
	virtual void HandleStartDoctypeDecl(const XML_Char* doctypeName);
		//: This is called for the start of the DOCTYPE declaration when the
		//. name of the DOCTYPE is encountered.

	virtual void HandleEndDoctypeDecl();
		//: This is called for the start of the DOCTYPE declaration when the
		//. closing > is encountered, but after processing any external subset.

	virtual void HandleExternalParsedEntityDecl(const XML_Char* entityName, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId);
	virtual void HandleInternalParsedEntityDecl(const XML_Char* entityName, const XML_Char* replacementText, int replacementTextLength);

	// error handling
	virtual void HandleError(int errNo);

	XML_Parser GetExpatParser() const;
		//; Returns a reference to the underlying expat parser.

private:
	// expat handler procedures
	static void StartElementHandler(void* userData, const XML_Char* name, const XML_Char** atts);
	static void EndElementHandler(void* userData, const XML_Char* name);
	static void CharacterDataHandler(void* userData, const XML_Char* s, int len);
	static void ProcessingInstructionHandler(void* userData, const XML_Char* target, const XML_Char* data);
	static void DefaultHandler(void* userData, const XML_Char* s, int len);
	static void UnparsedEntityDeclHandler(void* userData, const XML_Char* entityName, const XML_Char* base,
		                                  const XML_Char* systemId, const XML_Char* publicId,
										  const XML_Char* notationName);
	static void NotationDeclHandler(void* userData, const XML_Char* notationName, const XML_Char* base,
		                            const XML_Char* systemId, const XML_Char* publicId);
	static int ExternalEntityRefHandler(XML_Parser parser,
		                                 const XML_Char* openEntityNames, const XML_Char* base,
										 const XML_Char* systemId, const XML_Char* publicId);
	static int UnknownEncodingHandler(void* encodingHandlerData, const XML_Char* name, XML_Encoding* info);
	
	// new 1.1 handlers
	static void CommentHandler(void* userData, const XML_Char* data);
	static void StartCdataSectionHandler(void* userData);
	static void EndCdataSectionHandler(void* userData);

	// 1.1 Namespaces support
	static void StartNamespaceDeclHandler(void* userData, const XML_Char* prefix, const XML_Char* uri);
	static void EndNamespaceDeclHandler(void* userData, const XML_Char* prefix);

	// 1.2 DTD support
	static void StartDoctypeDeclHandler(void* userData, const XML_Char* doctypeName);
	static void EndDoctypeDeclHandler(void* userData);
	static void ExternalParsedEntityDeclHandler(void* userData, const XML_Char* entityName, const XML_Char* base, 
	                                            const XML_Char* systemId, const XML_Char* publicId);
	static void InternalParsedEntityDeclHandler(void* userData, const XML_Char* entityName,
						                        const XML_Char* replacementText, int replacementTextLength);



	// data members
private:
	XML_Parser m_parser;        // the expat parser
	char*      m_pBuffer;       // parser input buffer
	bool       m_nullEncoding;  // a NULL encoding was specified
	XMLString  m_encoding;      // encoding used by expat (if not NULL)
	XMLChar    m_nsSeparator;   // namespace separator (if any)
	bool       m_processNS;     // enable namespace processing
	XMLString  m_base;
	bool       m_baseSpecified;
	bool       m_expandIntEnt;  // expand internal entities
	enum XML_ParamEntityParsing m_parsing;
};


XML_END


#endif // XMLParserBase_H_INCLUDED
