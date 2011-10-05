//
// Copyright 2011 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun  7 13:04:03 PDT 2011
// Last Modified: Fri Jun 24 14:54:42 PDT 2011
// Filename:      ...sig/src/sigInfo/XmlFile.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/XmlFile.cpp
// Syntax:        C++ 
// 
// Description:   A class that stores a list of XML text fragments and
//                parses the element structure.
//

#ifndef OLDCPP
   #include <sstream>
   #include <fstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #ifdef VISUAL
      #include <strstrea.h>     /* for Windows 95 */
   #else
      #include <strstream.h>
   #endif
   #include <fstream.h>
   #define SSTREAM strstream
   #define CSTRING str()
#endif
   
#include "PerlRegularExpression.h"
#include "XmlFile.h"
#include <string.h>
#include <iomanip>


//////////////////////////////
//
// XmlFile::XmlFile --
//

XmlFile::XmlFile(void) { 
   elements.setSize(0);
   parsedQ = 0;
}


XmlFile::XmlFile(int allocationSize) { 
   elements.setSize(0);
   parsedQ = 0;
}



//////////////////////////////
//
// XmlFile::~XmlFile --
//

XmlFile::~XmlFile() { 
   clear();
}



//////////////////////////////
//
// XmlFile::clearElements -- remove all contents of the data and the 
//       parsed element data.
//

void XmlFile::clearElements(void) {
   int i;
   for (i=0; i<elements.getSize(); i++) {
      if (elements[i] != NULL) {
         delete elements[i];
      }
      elements[i] = NULL;
   }
   elements.setSize(0);
   parsedQ = 0;
}



//////////////////////////////
//
// XmlFile::clear -- remove all contents of the data and the parsed element 
//       data.
//

void XmlFile::clear(void) {
   clearElements();
   xmlitems.clear();
}



//////////////////////////////
//
// XmlFile::getItemSize -- returns the number of stored items in the
//      xmlitems variable.  Items are tags, text, and whitespace separating
//      tags and text.
//

int XmlFile::getItemSize(void) const {
   return xmlitems.getSize();
}



//////////////////////////////
//
// XmlFile::getElementSize -- returns the number of stored elements.
//    The fuction will parse the raw XML data for elements if it
//    has not yet been done.
//

int XmlFile::getElementSize(void) {
   if (!isParsed()) {
      parseElements();
   }
   return elements.getSize();
}



//////////////////////////////
//
// XmlFile::getItemIndexBySerial -- returns the index number of stored item
//     according to its serial number.  Serial number will remain constant,
//     but the index number may change if items are inserted or deleted.
//     Non-mapped serial number returns an index of -1.
//

int XmlFile::getItemIndexBySerial(int aserial) const {
   return xmlitems.getIndexBySerial(aserial);
}



//////////////////////////////
//
// XmlFile::getItemSerial -- returns the serial number of stored item.
//

int XmlFile::getItemSerial(int index) const {
   return xmlitems.getSerial(index);
}



//////////////////////////////
//
// XmlFile::operator[] --
//

XmlElement& XmlFile::operator[](int index) {
   return *(elements[index]);
}



//////////////////////////////
//
// XmlFile::getItem --
//

XmlItem& XmlFile::getItem(int index) {
   return xmlitems[index];
}



//////////////////////////////
//
// XmlFile::read --
//

void XmlFile::read(const char* filename) { 
   clear();
   xmlitems.read(filename);
   parsedQ = 0;
   parseElements();
}


void XmlFile::read(istream& input) { 
   clear();
   xmlitems.read(input);
   parsedQ = 0;
   parseElements();
}



//////////////////////////////
//
// XmlFile::appendItem -- add an item to the end of the
//    data, and return the serial number attached to it.
//

int XmlFile::appendItem(Array<char>& item) {
   parsedQ = 0;
   return xmlitems.appendItem(item);
}



//////////////////////////////
//
// XmlFile::insertItem -- place an element at the given
//    index into the list of items.  0 = instart at start of list.
//    Use appendItem to place at end of list.  Return value is the
//    serial number of the inserted item.
//

int XmlFile::insertItem(Array<char>& item, int index) {
   parsedQ = 0;
   return xmlitems.insertItem(item, index);
}



//////////////////////////////
//
// XmlFile::deleteItem -- returns true if an item was deleted.
//

int XmlFile::deleteItem(int index) {
   parsedQ = 0;
   return xmlitems.deleteItem(index);
}



//////////////////////////////
//
// XmlFile::printElementList -- used for debugging the parsing of elements.
//

ostream& XmlFile::printElementList(ostream& out) {
   int i;
   XmlFile& xmlfile = *this;
   for (i=0; i<getElementSize(); i++) {
      out << xmlfile[i].getName() << "\n";
   }
   return out;
}



///////////////////////////////////////////////////////////////////////////
//
// private functions --
//


//////////////////////////////
//
// XmlFile::parseElements -- parse element structure from xmlitems list.
//

void XmlFile::parseElements(void) {
   if (parsedQ) {
      return;
   }

   clearElements();
   elements.setSize(xmlitems.getSize());
   elements.setSize(0);

   int i;
   for (i=0; i<xmlitems.getSize(); i++) {
      if (xmlitems[i].isElement()) {
         i = processElement(i);
      } else {
      }
   }

   parseAttributes();

   parsedQ = 1;
}



//////////////////////////////
//
// XmlFile::parseAttributes -- Extract textual list of attributes 
//    and store them in key/value array for later access.
//

void XmlFile::parseAttributes(void) {
//  ggg do some work here

}



//////////////////////////////
//
// XmlFile::processElement -- Identify the start/end of an element.
//

int XmlFile::processElement(int index) {
   PerlRegularExpression pre;

   int startindex = index;
   int endindex   = index;

   if (pre.search(xmlitems[index].cstr(), "^\\s*<\\s*([^\\s>]+).*/\\s*>")) {
      // element starts/ends at the same index point.
      // No need to do any further processing so just store and continue.
      XmlElement *ptr = new XmlElement;
      ptr->setName(pre.getSubmatch(1));
      ptr->setStartSerial(xmlitems.getSerial(startindex));
      ptr->setEndSerial(xmlitems.getSerial(startindex));
      elements.append(ptr);
      return endindex;  // index will be incremented in calling function
   }

   // element-start tag is the input, so search for its ending
   // in the data.  Apply a recursive processing of children
   // elements at the same time.


   if (!pre.search(xmlitems[index].cstr(), "^\\s*<\\s*([^\\s>]+)")) {
      // malformed (empty) element tag, skip it.
      return index;
   }

   pre.getSubmatch(1); // fill in the internal match with element name

   // search for the ending tag like "</element>"
   SigString endname("<\\s*/\\s*");
   endname += pre.getSubmatch();
   endname += "\\s*>";

   // start name is the regular expression for a starting element name
   SigString startname("<\\s*");
   startname += pre.getSubmatch();
   startname += "[\\s>";

   // create and store the current element (so that intervening elements
   // can be place in the list in the correct order.
   XmlElement *ptr = new XmlElement;
   ptr->setName(pre.getSubmatch());
   ptr->setStartSerial(xmlitems.getSerial(startindex));
   elements.append(ptr);
   // set the end index later with ptr->setEndSerial();

   int i = index+1;
   PerlRegularExpression pre2;

   while (i<xmlitems.getSize()) {
      if (!xmlitems[i].isElement()) {
         // not an xml element marker, so continue to next item.
         i++;
         continue;
      }

      // check for single-tag element:
      if (pre2.search(xmlitems[i].cstr(), "^\\s*<([^\\s>]+).*/\\s*>")) {
         // found a single-item element process it and continue;
         i = processElement(i) + 1;
         continue;
      }

      // check for an ending tag which matches current element
      if (pre2.search(xmlitems[i].cstr(), endname.getBase())) {
         ptr->setEndSerial(xmlitems.getSerial(i));
         i++;
         continue;
      }

      // check for element starting, and process if so:
      if (pre2.search(xmlitems[i].cstr(), "^\\s*<([^\\s>]+).*\\s*>")) {
         i = processElement(i) + 1;
         continue;
      }

      i++;
   }

   return i;
}



//////////////////////////////
//
// XmlFile::isParsed --
//

int XmlFile::isParsed(void) {
   return parsedQ;
}



///////////////////////////////////////////////////////////////////////////
//
// friendly functions
//


//////////////////////////////
//
// operator<< --
//

ostream& operator<<(ostream& out, XmlFile& xmlfile) {
   int i;
   int asize = xmlfile.getItemSize();
   for (i=0; i<asize; i++) {
      out << xmlfile.getItem(i).getBase();
   }
   return out;
}




// md5sum: 05e1e67705ebcb2bfa1afad9e504582c XmlFile.cpp [20050403]
