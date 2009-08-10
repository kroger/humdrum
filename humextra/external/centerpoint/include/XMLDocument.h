//
// XMLDocument.h
//
// $Id: //depot/XML/Main/include/XMLDocument.h#7 $
//
// The classes defined here provide an object-oriented,
// tree based interface to a XML document.
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


#ifndef XMLDocument_H_INCLUDED
#define XMLDocument_H_INCLUDED


#ifndef XMLStd_H_INCLUDED
#include "XMLStd.h"
#endif
#ifndef SAX_AttributeList_H_INCLUDED
#include "SAX/AttributeList.h"
#endif
#ifndef XMLString_H_INCLUDED
#include "XMLString.h"
#endif
#ifndef XMLStream_H_INCLUDED
#include "XMLStream.h"
#endif
#ifndef XMLStringPool_H_INCLUDED
#include "XMLStringPool.h"
#endif


XML_BEGIN


enum XMLObjectType
	//: These are the values returned by CXMLObject::GetType
	//+ and passed to CXMLFilteringIterator.
	//. The values must be powers of two.
{
	xmlObject                     = 1,
	xmlContainer                  = 2,
	xmlElement                    = 4,
	xmlCharacterData              = 8,
	xmlCDATASection               = 16,
	xmlComment                    = 32,
	xmlGenericData                = 64,
	xmlProcessingInstruction      = 128,
	xmlExternalEntity             = 256,
	xmlNotationDeclaration        = 512,
	xmlUnparsedEntityDeclaration  = 1024,
	xmlDocument                   = 2048,
	xmlAttribute                  = 4096
};


class CDocumentHandler;


class XML_API CXMLObject
	//: This is the base class for all objects
	//. in a document tree.
{
	friend class CXMLIterator;
	friend class CXMLContainer;

public:
	CXMLObject();
	virtual ~CXMLObject();

	virtual XMLString GetBase() const;
		//: Returns the object's document base URI.
		//. Used by the parser to resolve relative systemId's.

	virtual XMLObjectType GetType() const;
		//; Returns the object's type.

	CXMLObject* GetParent() const;
		//; Returns a pointer to the object's parent.

	virtual CXMLObject* Copy(CXMLObject* pCopy = NULL) const;
		//: Returns a shallow copy of the object.
		//+ For containers, no child objects are copied, therefor
		//+ a shallow copy of a container object doesn't have
		//+ any child objects.
		//. The attributes of an element are copied.

	virtual CXMLObject* DeepCopy(CXMLObject* pCopy = NULL) const;
		//: Returns a deep copy of the object. For a container
		//. object, all child objects are copied recursively.

	virtual void Dump(ostream* pStr) const;
		//: Dumps the contents of the object to
		//. the specified stream (for debugging purposes).

	virtual CXMLObject* Zoom() const;
		//; Returns a pointer to the first child.

	CXMLObject* GetNext() const;
		//; Returns a pointer to the next sibling.

	virtual void SetUserData(void* data);
		//; Sets the user-specified data item.

	virtual void* GetUserData() const;
		//; Returns the user-specified data item.

protected:
	virtual void DeleteAll();
	void SetNext(CXMLObject* pNext);

	static CXMLStringPool m_stringPool;

private:
	CXMLObject* m_pParent;
	CXMLObject* m_pNext;
	void*       m_userData;
};


class XML_API CXMLIterator
	//: This class is used for traversing the
	//. XML document tree.
{
public:
	CXMLIterator(CXMLObject* pObj);
	CXMLIterator(const CXMLIterator& iter);
	virtual ~CXMLIterator();

	virtual void Next();
		//; Moves the iterator to the next object.

	virtual void ZoomIn();
		//: If the current object has children, the iterator
		//+ is moved to the first child, otherwise the iterator
		//. is left unchanged.
	
	virtual void ZoomOut();
		//; Moves the iterator to the parent object.

	virtual CXMLObject* GetObj() const;
		//; Returns the current object.

private:
	CXMLObject* m_pCurrent;
	CXMLObject* m_pParent;
};


class XML_API CXMLFilteringIterator: public CXMLIterator
	//: A special kind of iterator that only
	//+ delivers certain object types.
	//+ Only objects with a type code & filterSpec != 0
	//. are delivered.
{
public:
	CXMLFilteringIterator(int filterSpec, CXMLObject* pObj);
	CXMLFilteringIterator(int filterSpec, const CXMLIterator& iter);
	CXMLFilteringIterator(const CXMLFilteringIterator& iter);

	virtual void Next();
	virtual void ZoomIn();
	virtual void ZoomOut();

private:
	int m_filterSpec;
};


class XML_API CXMLContainer: public CXMLObject
	//; A base class for all objects that can contain other objects.
{
public:
	CXMLContainer();

	virtual void InsertChild(CXMLObject* pObj);
		//; Inserts pObj behind the last child of the current node.

	virtual void InsertChildBefore(CXMLObject* pObj, CXMLObject* pSuccessor);
		//: Inserts an object before the specified child object.
		//+ The results are undefined if pSuccessor is not a child of the current node.
		//. If pSuccessor is NULL, the child is inserted behind the last child.

	virtual void RemoveChild(CXMLObject* pObj);
		//; Removes a child object.

	virtual void Empty();
		//; Removes all child objects.

	virtual CXMLObject* Zoom() const;
		//; Returns the first child object.

	virtual XMLObjectType GetType() const;

	virtual void Dump(ostream* pStr) const;

	virtual CXMLObject* Copy(CXMLObject* pCopy = NULL) const;
	virtual CXMLObject* DeepCopy(CXMLObject* pCopy = NULL) const;

protected:
	virtual void DeleteAll();

private:
	CXMLObject* m_pFirstChild;
};
	

class XML_API CXMLAttribute: public CXMLObject
	//; This class represents a XML element's attribute.
{
	friend class CXMLAttributeList;

public:
	CXMLAttribute();
	CXMLAttribute(const XMLString& qname, const XMLString& type, const XMLString& value, bool specified = true);
	CXMLAttribute(const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname, const XMLString& type, const XMLString& value, bool specified = true);

	virtual void SetName(const XMLString& name);
		//; Sets the attribute's qualified name.

	virtual void SetLocalName(const XMLString& localName);
		//; Sets the attribute's local name.

	virtual void SetNamespaceURI(const XMLString& uri);
		//; Sets the attribute's namespace URI.

	virtual XMLString GetName() const;
		//; Returns the qualified attribute name.

	virtual XMLString GetLocalName() const;
		//; Returns the attribute's local name.

	virtual XMLString GetNamespaceURI() const;
		//; Returns the attribute's namespace URI.

	virtual void SetAttrType(const XMLString& type);
		//: Sets the attribute type. A valid XML
		//+ attribute type name should be specified.
		//. The attribute type is currently not used.

	virtual XMLString GetAttrType() const;
		//; Returns the attribute type.

	virtual void SetValue(const XMLString& value);
		//: Sets the attribute value.
		//. Note: setting a value sets the specified flag to true.

	virtual XMLString GetValue() const;
		//; Returns the attribute value.

	virtual void SetSpecified(bool flag);
		//: Set whether this attribute has been specified in
		//+ the element's attribute list, or whether this
		//. attribute has a default value.

	virtual bool GetSpecified() const;
		//: Returns true if the attribute has been specified
		//+ in the element's attribute list; false if this
		//. attribute has a default value.

	virtual XMLObjectType GetType() const;

	virtual void Dump(ostream* pStr) const;

	virtual CXMLObject* Copy(CXMLObject* pCopy = NULL) const;
	virtual CXMLObject* DeepCopy(CXMLObject* pCopy = NULL) const;

private:
	XMLString m_name;
	XMLString m_localName;
	XMLString m_uri;
	XMLString m_type;
	XMLString m_value;
	bool      m_specified;
};


class XML_API CXMLAttributeList: public CAttributeList
	//; This class implements a list of attributes.
{
public:
	CXMLAttributeList();
	~CXMLAttributeList();

	// CAttributeList
	virtual int GetLength() const;
		//; Returns the number of attributes in the list.

	virtual XMLString GetName(int i) const;
		//: Returns the i'th element's qualified name.
		//. An empty string is returned if i is not a valid index.

	virtual XMLString GetLocalName(int i) const;
		//: Returns the i'th element's local name.
		//+ An empty string is returned if i is not a valid index,
		//. or no namespaces are in use.

	virtual XMLString GetNamespaceURI(int i) const;
		//: Returns the i'th element's namespace URI.
		//+ An empty string is returned if i is not a valid index,
		//. or no namespaces are in use.
	
	virtual XMLString GetType(int i) const;
		//: Returns the i'th element's type.
		//. An empty string is returned if i is not a valid index.

	virtual XMLString GetValue(int i) const;
		//: Returns the i'th element's value.
		//. An empty string is returned if i is not a valid index.

	virtual bool GetSpecified(int i) const;
		//: Returns true if the i'th element has been specified
		//+ in the element's attribute list; false if the
		//. attribute has a default value.

	virtual XMLString GetType(const XMLString& qname) const;
		//: Returns the type of the attribute with the specified qualified name.
		//. An empty string is returned if the attribute does not exist.

	virtual XMLString GetType(const XMLString& namespaceURI, const XMLString& localName) const;
		//: Returns the type of the attribute with the specified local name and namspace URI.
		//. An empty string is returned if the attribute does not exist.

	virtual XMLString GetValue(const XMLString& qname) const;
		//: Returns the value of the attribute with the specified qualified name.
		//. An empty string is returned if the attribute does not exist.

	virtual XMLString GetValue(const XMLString& namespaceURI, const XMLString& localName) const;
		//: Returns the value of the attribute with the specified local name and namspace URI.
		//. An empty string is returned if the attribute does not exist.

	virtual bool GetSpecified(const XMLString& qname) const;
		//: Returns true if the specified attribute has been specified
		//+ in the element's attribute list; false if the
		//. attribute has a default value.

	virtual bool GetSpecified(const XMLString& namespaceURI, const XMLString& localName) const;
		//: Returns true if the specified attribute has been specified
		//+ in the element's attribute list; false if the
		//. attribute has a default value.

	virtual CXMLAttribute* GetAttribute(const XMLString& qname) const;
		//: Returns the attribute object with the specified qualified name.
		//. Returns NULL if the attribute does not exist.

	virtual CXMLAttribute* GetAttribute(const XMLString& namespaceURI, const XMLString& localName) const;
		//: Returns the attribute object with the specified qualified name.
		//. Returns NULL if the attribute does not exist.

	virtual CXMLAttribute* GetAttribute(int i) const;
		//: Returns the attribute object with the specified index.
		//. Returns NULL if the attribute does not exist.

	virtual void SetAttribute(CXMLAttribute* pAttr);
		//; Adds or changes the specified attribute to/in the list.

	virtual CXMLAttribute* RemoveAttribute(const XMLString& qname);
		//; Removes the specified attribute from the list.

	virtual CXMLAttribute* RemoveAttribute(const XMLString& namespaceURI, const XMLString& localName);
		//; Removes the specified attribute from the list.

	virtual CXMLAttribute* RemoveAttribute(CXMLAttribute* pAttr);
		//; Removes the specified attribute from the list.

	virtual void Clear();
		//; Removes all attributes from the list.

protected:
	virtual void AddAttribute(CXMLAttribute* pAttr);

private:
	CXMLAttribute* m_pFirstAttr;
};


class XML_API CXMLElement: public CXMLContainer
	//; This class represents a XML element.
{
public:
	CXMLElement();
	CXMLElement(const XMLString& name);
	CXMLElement(const XMLString& namespaceURI, const XMLString& localName, const XMLString& qname);
	virtual ~CXMLElement();

	virtual XMLString GetName() const;
		//; Returns the element's qualified name.

	virtual XMLString GetLocalName() const;
		//; Returns the element's local name.

	virtual XMLString GetNamespaceURI() const;
		//; Returns the element's namespace URI.

	virtual void SetName(const XMLString& name);
		//; Sets the element's qualified name.

	virtual void SetLocalName(const XMLString& localName);
		//; Sets the element's local name.

	virtual void SetNamespaceURI(const XMLString& uri);
		//; Sets the element's namespace URI.

	virtual CXMLAttributeList& GetAttributes();
		//; Returns a reference to the element's attribute list.

	virtual const CXMLAttributeList& GetAttributes() const;
		//; Returns a const reference to the element's attribute list.

	virtual XMLObjectType GetType() const;

	virtual void Dump(ostream* pStr) const;

	virtual CXMLObject* Copy(CXMLObject* pCopy = NULL) const;
	virtual CXMLObject* DeepCopy(CXMLObject* pCopy = NULL) const;

private:
	XMLString m_name;
	XMLString m_localName;
	XMLString m_uri;
	CXMLAttributeList m_attributes;
};


class XML_API CXMLCharacterData: public CXMLObject
	//; This class represents XML character data.
{
public:
	CXMLCharacterData();
	CXMLCharacterData(const XMLString& data);

	virtual XMLString GetData() const;
		//; Returns the character string.

	virtual void SetData(const XMLString& data);
		//; Sets the character string.

	virtual XMLObjectType GetType() const;

	virtual XMLString GetSubString(int offset, int count) const;
		//; Returns a substring of the character data.

	virtual void AppendData(const XMLString& data);
		//; Appends a string to the character data.

	virtual void InsertData(int offset, const XMLString& data);
		//; Inserts a string at the specified offset to the character data.

	virtual void DeleteData(int offset, int count);
		//; Deletes count characters, starting at offset, from the character data.

	virtual void ReplaceData(int offset, int count, const XMLString& data);
		//; Replaces count characters, starting at offset, with data.

	virtual int GetLength() const;
		//; Returns the length of the character data.

	virtual void Dump(ostream* pStr) const;

	virtual CXMLObject* Copy(CXMLObject* pCopy = NULL) const;
	virtual CXMLObject* DeepCopy(CXMLObject* pCopy = NULL) const;

private:
	XMLString m_data;
};


class XML_API CXMLCDATASection: public CXMLCharacterData
	//; This class represents a XML CDATA section.
{
public:
	CXMLCDATASection();
	CXMLCDATASection(const XMLString& data);

	virtual XMLObjectType GetType() const;

	virtual void Dump(ostream* pStr) const;

	virtual CXMLObject* Copy(CXMLObject* pCopy = NULL) const;
	virtual CXMLObject* DeepCopy(CXMLObject* pCopy = NULL) const;
};


class XML_API CXMLComment: public CXMLCharacterData
	//; This class represents an XML comment.
{
public:
	CXMLComment();
	CXMLComment(const XMLString& data);

	virtual XMLObjectType GetType() const;

	virtual void Dump(ostream* pStr) const;

	virtual CXMLObject* Copy(CXMLObject* pCopy = NULL) const;
	virtual CXMLObject* DeepCopy(CXMLObject* pCopy = NULL) const;
};


class XML_API CXMLGenericData: public CXMLCharacterData
	//; This class represents XML generic data (e.g., ignorable whitespace)
{
public:
	CXMLGenericData();
	CXMLGenericData(const XMLString& data);

	virtual XMLObjectType GetType() const;

	virtual void Dump(ostream* pStr) const;

	virtual CXMLObject* Copy(CXMLObject* pCopy = NULL) const;
	virtual CXMLObject* DeepCopy(CXMLObject* pCopy = NULL) const;
};


class XML_API CXMLProcessingInstruction: public CXMLObject
	//; This class represents a XML processing instruction.
{
public:
	CXMLProcessingInstruction();
	CXMLProcessingInstruction(const XMLString& target, const XMLString& data);

	virtual XMLString GetTarget() const;
		//; Returns the target of the processing instruction.

	virtual XMLString GetData() const;
		//; Returns the data of the processing instruction.

	virtual void SetTarget(const XMLString& target);
		//; Sets the target of the processing instruction.

	virtual void SetData(const XMLString& data);
		//; Sets the data of the processing instruction.

	virtual XMLObjectType GetType() const;

	virtual void Dump(ostream* pStr) const;

	virtual CXMLObject* Copy(CXMLObject* pCopy = NULL) const;
	virtual CXMLObject* DeepCopy(CXMLObject* pCopy = NULL) const;

private:
	XMLString m_target;
	XMLString m_data;
};


class XML_API CXMLExternalEntity: public CXMLContainer
	//: This class represents a XML (external general) entity.
	//+ Note that external entity nodes are only included
	//+ in the document tree when external generyl entities
	//. are not automatically included by the parser.
{
public:
	CXMLExternalEntity();
	CXMLExternalEntity(const XMLString& systemId, const XMLString& publicId);

	virtual XMLString GetSystemId() const;
		//; Returns the system identifier (URI) of the external entity.

	virtual XMLString GetPublicId() const;
		//; Returns the public identifier of the external entity.
		 
	virtual void SetSystemId(const XMLString& systemId);
		//; Sets the system identifier (URI) of the external entity.

	virtual void SetPublicId(const XMLString& publicId);
		//; Sets the public identifier of the external entity.

	virtual XMLObjectType GetType() const;

	virtual void Dump(ostream* pStr) const;

	virtual CXMLObject* Copy(CXMLObject* pCopy = NULL) const;
	virtual CXMLObject* DeepCopy(CXMLObject* pCopy = NULL) const;

private:
	XMLString m_systemId;
	XMLString m_publicId;
};


class XML_API CXMLNotationDeclaration: public CXMLObject
	//; This class represents a XML notation declaration.
{
public:
	CXMLNotationDeclaration();
	CXMLNotationDeclaration(const XMLString& name, const XMLString& systemId, const XMLString& publicId);

	virtual XMLString GetName() const;
		//; Returns the name of the notation.

	virtual XMLString GetSystemId() const;
		//; Returns the system identifier (URI) of the notation declaration.

	virtual XMLString GetPublicId() const;
		//; Returns the public identifier of the notation declaration.

	virtual void SetName(const XMLString& name);
		//; Sets the name of the notation.

	virtual void SetSystemId(const XMLString& systemId);
		//; Sets the system identifier (URI) of the notation.

	virtual void SetPublicId(const XMLString& publicId);
		//; Sets the public identifier of the notation.

	virtual XMLObjectType GetType() const;

	virtual void Dump(ostream* pStr) const;

	virtual CXMLObject* Copy(CXMLObject* pCopy = NULL) const;
	virtual CXMLObject* DeepCopy(CXMLObject* pCopy = NULL) const;

private:
	XMLString m_name;
	XMLString m_systemId;
	XMLString m_publicId;
};


class XML_API CXMLUnparsedEntityDeclaration: public CXMLObject
	//; This class represents a XML unparsed entity declaration.
{
public:
	CXMLUnparsedEntityDeclaration();
	CXMLUnparsedEntityDeclaration(const XMLString& entityName, const XMLString& systemId, 
		                          const XMLString& publicId, const XMLString& notationName);

	virtual XMLString GetName() const;
		//; Returns the name of the unparsed entity.

	virtual XMLString GetSystemId() const;
		//; Returns the system identifier (URI) of the unparsed entity.

	virtual XMLString GetPublicId() const;
		//; Returns the public identifier of the unparsed entity.

	virtual XMLString GetNotationName() const;
		//; Returns the notation name of the unparsed entity.

	virtual void SetName(const XMLString& name);
		//; Sets the name of the unparsed entity.

	virtual void SetSystemId(const XMLString& systemId);
		//; Sets the system identifier (URI) of the unparsed entity.

	virtual void SetPublicId(const XMLString& publicId);
		//; Sets the public identifier of the unparsed entity.

	virtual void SetNotationName(const XMLString& notationName);
		//; Sets the notation name of the unparsed entity.

	virtual XMLObjectType GetType() const;

	virtual void Dump(ostream* pStr) const;

	virtual CXMLObject* Copy(CXMLObject* pCopy = NULL) const;
	virtual CXMLObject* DeepCopy(CXMLObject* pCopy = NULL) const;

private:
	XMLString m_entityName;
	XMLString m_systemId;
	XMLString m_publicId;
	XMLString m_notationName;
};


class XML_API CXMLDocument: public CXMLContainer
	//: Represents a XML document.
	//+ Note that in order to be well formed, a 
	//+ document must not contain more than one element
	//. at root level.
{
public:
	CXMLDocument();
	CXMLDocument(const XMLString& name);
	virtual ~CXMLDocument();

	virtual XMLString GetName() const;
		//; Returns the document's name.

	virtual void SetName(const XMLString& name);
		//; Sets the document's name.

	virtual XMLString GetBase() const;
		//: Returns the document's base URI. The base URI
		//. is used to resolve external entity system identifiers.

	virtual void SetBase(const XMLString& base);
		//; Sets the document's base URI.

	virtual XMLObjectType GetType() const;

	virtual void Dump(ostream* pStr) const;

	virtual CXMLObject* Copy(CXMLObject* pCopy = NULL) const;
	virtual CXMLObject* DeepCopy(CXMLObject* pCopy = NULL) const;

private:
	XMLString m_name;
	XMLString m_base;
};


class XML_API CXMLFactory
	//; An abstract factory for XML document objects.
{
public:
	virtual CXMLAttribute* CreateAttribute(const XMLString& name, const XMLString& type, const XMLString& value, bool specified = true);
	virtual CXMLElement* CreateElement(const XMLString& name) const;
	virtual CXMLCharacterData* CreateCharacterData(const XMLString& data) const;
	virtual CXMLGenericData* CreateGenericData(const XMLString& data) const;
	virtual CXMLProcessingInstruction* CreateProcessingInstruction(const XMLString& target, 
		                                                           const XMLString& data) const;
	virtual CXMLExternalEntity* CreateExternalEntity(const XMLString& systemId, 
		                                             const XMLString& publicId) const;
	virtual CXMLNotationDeclaration* CreateNotationDeclaration(const XMLString& name, 
		                                                       const XMLString& systemId, 
															   const XMLString& publicId) const;
	virtual CXMLUnparsedEntityDeclaration* CreateUnparsedEntityDeclaration(const XMLString& entityName, 
		                                                                   const XMLString& systemId, 
																		   const XMLString& publicId, 
																		   const XMLString& notationName) const;
	virtual CXMLCDATASection* CreateCDATASection(const XMLString& data) const;
	virtual CXMLComment* CreateComment(const XMLString& data) const;
	virtual CXMLDocument* CreateDocument(const XMLString& name) const;
};


XML_END


#endif // XMLDocument_H_INCLUDED
