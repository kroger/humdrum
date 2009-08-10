//
// DocTest.cpp
//
// $Id: //depot/XML/Main/public/samples/src/DocTest.cpp#4 $
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


#include "XMLDocument.h"
#include "XMLDocumentBuilder.h"
#include "XMLUtils.h"
#include "XMLException.h"
#include <iostream>
#include <fstream>


using namespace CSL::XML;


class DocumentTraverser
{
public:
	DocumentTraverser()
	{
		m_level = 0;
	};

	void Traverse(CXMLObject* pRoot)
	{
		while (pRoot)
		{
			Process(pRoot);
			pRoot = pRoot->GetNext();
		}
	};

protected:
	void Process(CXMLObject* pObj)
	{
		switch (pObj->GetType())
		{
		case xmlElement:
			ProcessElement(pObj); break;
		case xmlCharacterData:
			ProcessCharacterData(pObj); break;
		case xmlCDATASection:
			ProcessCDATASection(pObj); break;
		case xmlComment:
			ProcessComment(pObj); break;
		case xmlGenericData:
			ProcessGenericData(pObj); break;
		case xmlProcessingInstruction:
			ProcessPI(pObj); break;
		case xmlExternalEntity:
			ProcessExternalEntity(pObj); break;
		case xmlNotationDeclaration:
			ProcessNotationDecl(pObj); break;
		case xmlUnparsedEntityDeclaration:
			ProcessUnparsedEntityDecl(pObj); break;
		case xmlDocument:
			ProcessDocument(pObj); break;
		}
		m_level++;
		Traverse(pObj->Zoom());
		m_level--;
	};
	void ProcessElement(CXMLObject* pObj)
	{
		CXMLElement* pE = (CXMLElement*) pObj;
		BeginNode();
		cout << "Element:<" << pE->GetName() << ">" << endl;
		for (int i = 0; i < pE->GetAttributes().GetLength(); i++)
		{
			PrefixNode();
			cout << pE->GetAttributes().GetName(i) << "=\"" << pE->GetAttributes().GetValue(i) << "\"" << endl;
		}
	};
	void ProcessCharacterData(CXMLObject* pObj)
	{
		CXMLCharacterData* pD = (CXMLCharacterData*) pObj;
		BeginNode();
		cout << "CharacterData" << endl;
		PrefixNode();
		CharacterData(pD);
	};	
	void ProcessComment(CXMLObject* pObj)
	{
		CXMLComment* pD = (CXMLComment*) pObj;
		BeginNode();
		cout << "Comment" << endl;
		PrefixNode();
		CharacterData(pD);
	};
	void ProcessGenericData(CXMLObject* pObj)
	{
		CXMLGenericData* pD = (CXMLGenericData*) pObj;
		BeginNode();
		cout << "GenericData" << endl;
		PrefixNode();
		CharacterData(pD);
	};
	void ProcessPI(CXMLObject* pObj)
	{
		CXMLProcessingInstruction* pPI = (CXMLProcessingInstruction*) pObj;
		BeginNode();
		cout << "ProcessingInstruction" << endl;
		PrefixNode();
		cout << "target=" << pPI->GetTarget() << endl;
		PrefixNode();
		cout << "data=" << pPI->GetData() << endl;
	};
	void ProcessExternalEntity(CXMLObject* pObj)
	{
		CXMLExternalEntity* pEI = (CXMLExternalEntity*) pObj;
		BeginNode();
		cout << "ExternalEntity" << endl;
		PrefixNode();
		cout << "publicId=" << pEI->GetPublicId() << endl;
		PrefixNode();
		cout << "systemId=" << pEI->GetSystemId() << endl;
	};
	void ProcessCDATASection(CXMLObject* pObj)
	{
		CXMLCDATASection* pD = (CXMLCDATASection*) pObj;
		BeginNode();
		cout << "CDATA" << endl;
		PrefixNode();
		CharacterData(pD);
	};
	void ProcessNotationDecl(CXMLObject* pObj)
	{
		CXMLNotationDeclaration* pN = (CXMLNotationDeclaration*) pObj;
		BeginNode();
		cout << "NotationDeclaration" << endl;
		PrefixNode();
		cout << "name=" << pN->GetName() << endl;
		PrefixNode();
		cout << "publicId=" << pN->GetPublicId() << endl;
		PrefixNode();
		cout << "systemId=" << pN->GetSystemId() << endl;
	};
	void ProcessUnparsedEntityDecl(CXMLObject* pObj)
	{
		CXMLUnparsedEntityDeclaration* pU = (CXMLUnparsedEntityDeclaration*) pObj;
		BeginNode();
		cout << "UnparsedEntityDeclaration" << endl;
		PrefixNode();
		cout << "name=" << pU->GetName() << endl;
		PrefixNode();
		cout << "publicId=" << pU->GetPublicId() << endl;
		PrefixNode();
		cout << "systemId=" << pU->GetSystemId() << endl;
		PrefixNode();
		cout << "notationName=" << pU->GetNotationName() << endl;
	};
	void ProcessDocument(CXMLObject* pObj)
	{
		CXMLDocument* pD = (CXMLDocument*) pObj;
		BeginNode();
		cout << "Document" << endl;
		PrefixNode();
		cout << "name=" << pD->GetName() << endl;
		PrefixNode();
		cout << "base=" << pD->GetBase() << endl;
	};

	void BeginNode()
	{
		Indent();
		cout << "|" << endl;
		Indent();
		cout << "+--[";
	};
	void PrefixNode()
	{
		Indent();
		cout << "|  [";
	};
	void Indent()
	{
		for (int i = 0; i < m_level; i++) cout << "|  ";
	};
	void CharacterData(CXMLCharacterData* pD)
	{
		cout << "data=";
		XMLString data;
		if (pD->GetLength() > 16)
		{
			data = pD->GetSubString(0, 16); data += "...";
		}
		else
		{
			data = pD->GetData();
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
	CXMLFactory factory;
	CXMLDocumentBuilder builder;
	CXMLDocument* pDocument;

	if (argc < 2)
	{
		cerr << "Error: no filename specified" << endl;
		exit(1);
	}

	try
	{
		pDocument = factory.CreateDocument("test");

		builder.BuildDocument(argv[1], pDocument, &factory);

		DocumentTraverser t;
		t.Traverse(pDocument);
	}
	catch (CXMLException& e)
	{
		cerr << "Error: " << e.GetMsg() << endl;
		exit(1);
	}
	return 0;
}

