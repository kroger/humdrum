//
// WriterTest.cpp
//
// $Id: //depot/XML/Main/public/samples/src/WriterTest.cpp#1 $
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
#include "DOM/Element.h"
#include "DOM/NamedNodeMap.h"
#include "DOM/Document.h"
#include "DOM/Comment.h"
#include "DOM/DOMImplementation.h"
#include "DOMWriter.h"
#include "XMLUtils.h"
#include "XMLException.h"
#include <iostream>
#include <fstream>


int main(int argc, char** argv)
{
	try
	{
		string xhtmlns = "http://www.w3.org/1999/xhtml";
		CSL::XML::DOMImplementation* pDomImpl = CSL::XML::DOMImplementation::getInstance();

		CSL::XML::Document* pDoc = pDomImpl->createDocument(xhtmlns, "html", pDomImpl->createDocumentType("", "html", "xhtml-sample-1.dtd"));
		CSL::XML::Element* pDocElem = pDoc->getDocumentElement();
		CSL::XML::Comment* pCommentElem = pDoc->createComment("CenterPoint/XML Sample");
		pDoc->insertBefore(pCommentElem, pDocElem)->release();
		CSL::XML::Element* pHeadElem = pDoc->createElementNS(xhtmlns, "head");
		CSL::XML::Element* pTitleElem = pDoc->createElementNS(xhtmlns, "title");
		pTitleElem->appendChild(pDoc->createTextNode("XHTML Sample"))->release();
		pHeadElem->appendChild(pTitleElem)->release();
		pDocElem->appendChild(pHeadElem)->release();
		CSL::XML::Element* pBodyElem = pDoc->createElementNS(xhtmlns, "body");
		CSL::XML::Element* pParaElem = pDoc->createElementNS(xhtmlns, "p");
		pParaElem->setAttribute("align", "center");
		pParaElem->appendChild(pDoc->createTextNode("Created with CenterPoint/XML."))->release();
		pBodyElem->appendChild(pParaElem)->release();
		pDocElem->appendChild(pBodyElem)->release();

		CSL::XML::DOMWriter domWriter;
		domWriter.setFormat(CSL::XML::DOMWriter::REFORMATTED | CSL::XML::DOMWriter::CANONICAL); // enable pretty-printing, no XML declaration
		domWriter.writeNode("test.xhtml", pDoc);
		domWriter.writeNode(&cout, pDoc);

		pDoc->release();
	}
	catch (CSL::XML::CXMLException& e)
	{
		cerr << "Error: " << e.GetMsg() << endl;
		exit(1);
	}
	return 0;
}

