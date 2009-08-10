//
// DOMTest.cpp
//
// $Id: //depot/XML/Main/public/samples/src/DOMTest.cpp#4 $
//
// Sample application for CenterPoint/XML.
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


#include "DOM/DOMBuilder.h"
#include "DOM/Attr.h"
#include "DOM/Text.h"
#include "DOM/CDATASection.h"
#include "DOM/Element.h"
#include "DOM/Comment.h"
#include "DOM/ProcessingInstruction.h"
#include "DOM/Entity.h"
#include "DOM/Notation.h"
#include "DOM/Document.h"
#include "DOM/NamedNodeMap.h"
#include "XMLUtils.h"
#include "XMLException.h"
#include <iostream>
#include <fstream>


using CSL::XML::Node;


class DocumentTraverser
	// This class will traverse a DOM document.
{
public:
	DocumentTraverser()
	{
		m_level = 0;
	}

	void Traverse(Node* pNode)
	{
		while (pNode)
		{
			Process(pNode);
			pNode = pNode->getNextSibling();
		}
	}

protected:
	void Process(Node* pNode)
	{
		switch (pNode->getNodeType())
		{
		case Node::ELEMENT_NODE:
			ProcessElement(static_cast<CSL::XML::Element*>(pNode)); 
			break;
		case Node::TEXT_NODE:
			ProcessText(static_cast<CSL::XML::Text*>(pNode)); 
			break;
		case Node::CDATA_SECTION_NODE:
			ProcessCDATASection(static_cast<CSL::XML::CDATASection*>(pNode)); 
			break;
		case Node::COMMENT_NODE:
			ProcessComment(static_cast<CSL::XML::Comment*>(pNode)); 
			break;
		case Node::PROCESSING_INSTRUCTION_NODE:
			ProcessPI(static_cast<CSL::XML::ProcessingInstruction*>(pNode)); 
			break;
		case Node::ENTITY_NODE:
			ProcessExternalEntity(static_cast<CSL::XML::Entity*>(pNode)); 
			break;
		case Node::NOTATION_NODE:
			ProcessNotationDecl(static_cast<CSL::XML::Notation*>(pNode)); 
			break;
		case Node::DOCUMENT_NODE:
			ProcessDocument(static_cast<CSL::XML::Document*>(pNode)); 
			break;
		}
		m_level++;
		Traverse(pNode->getFirstChild());
		m_level--;
	}

	void ProcessElement(CSL::XML::Element* pElement)
	{
		DoNode(pElement);
		PrefixNode();
		cout << "uri=" << pElement->getNamespaceURI() << endl;
		PrefixNode();
		cout << "local=" << pElement->getLocalName() << endl;
		CSL::XML::NamedNodeMap* pAttrList = pElement->getAttributes();
		for (unsigned i = 0; i < pAttrList->getLength(); i++)
		{
			CSL::XML::Attr* pAttr = static_cast<CSL::XML::Attr*>(pAttrList->item(i));
			PrefixNode();
			cout << pAttr->getName() << "=\"" << pAttr->getNodeValue() << "\" " << (pAttr->getSpecified() ? "*" : "") << endl;
		}
		pAttrList->release();
	}

	void ProcessText(CSL::XML::Text* pText)
	{
		DoNode(pText);
		DoText(pText);
	}	

	void ProcessComment(CSL::XML::Comment* pComment)
	{
		DoNode(pComment);
		DoText(pComment);
	}

	void ProcessPI(CSL::XML::ProcessingInstruction* pPI)
	{
		DoNode(pPI);		
	}

	void ProcessExternalEntity(CSL::XML::Entity* pEntity)
	{
	}

	void ProcessCDATASection(CSL::XML::CDATASection* pCDATA)
	{
		DoNode(pCDATA);
		DoText(pCDATA);
	}

	void ProcessNotationDecl(CSL::XML::Notation* pNotation)
	{
	}

	void ProcessDocument(CSL::XML::Document* pDocument)
	{
		DoNode(pDocument);
	}

	void BeginNode()
	{
		Indent();
		cout << "|" << endl;
		Indent();
		cout << "+--[";
	}

	void PrefixNode()
	{
		Indent();
		cout << "|  [";
	}

	void Indent()
	{
		for (int i = 0; i < m_level; i++) cout << "|  ";
	}

	void DoNode(Node* pNode)
	{
		BeginNode();
		cout << "type=";
		switch (pNode->getNodeType())
		{
		case Node::ELEMENT_NODE:
			cout << "Element"; break;
		case Node::TEXT_NODE:
			cout << "Text"; break;
		case Node::CDATA_SECTION_NODE:
			cout << "CDATASection"; break;
		case Node::COMMENT_NODE:
			cout << "Comment"; break;
		case Node::PROCESSING_INSTRUCTION_NODE:
			cout << "ProcessingInstruction"; break;
		case Node::ENTITY_NODE:
			cout << "Entity"; break;
		case Node::NOTATION_NODE:
			cout << "Notation"; break;
		case Node::DOCUMENT_NODE:
			cout << "Document"; break;
		}
		cout << endl;
		PrefixNode();
		cout << "name=" << pNode->getNodeName() << endl;		
	}

	void DoText(CSL::XML::CharacterData* pChars)
	{
		PrefixNode();
		cout << "text=";
		CSL::XML::XMLString data;
		if (pChars->getLength() > 16)
		{
			data = pChars->substringData(0, 16); data += "...";
		}
		else
		{
			data = pChars->getData();
		}
		for (unsigned i = 0; i < data.length(); i++)
		{
			if (data[i] < ' ') data[i] = ' ';
		}
		cout << data << endl;
	}

private:
	int m_level;
};


//
// main
//


int main(int argc, char** argv)
{
	CSL::XML::Document* pDoc;
	CSL::XML::DOMBuilder domBuilder;

	// configure DOMBuilder
	domBuilder.setFeature("namespaces", false);                    // turn off namespace support
	domBuilder.setFeature("white-space-in-element-content", true); // do not remove ignorable whitespace

	if (argc < 2)
	{
		cerr << argv[0] << ": no filename specified" << endl;
		exit(1);
	}

	try
	{
		// build a DOM tree by parsing an XML file
		pDoc = domBuilder.parseURI(argv[1]);

		DocumentTraverser t;
		t.Traverse(pDoc);

		pDoc->release();
	}
	catch (CSL::XML::CXMLException& e)
	{
		cerr << argv[0] << ": error: " << e.GetMsg() << endl;
		exit(1);
	}
	return 0;
}
