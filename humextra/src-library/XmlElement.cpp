//
// Copyright 2011 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jun  6 14:21:45 PDT 2011
// Last Modified: Tue Jun  7 13:02:16 PDT 2011
// Filename:      ...sig/include/SigInfo/XmlElement.h
// Web Address:   http://sig.sapp.org/include/sigInfo/XmlElement.h
// Syntax:        C++ 
//
// Description:   A class that stores a parsing of an XML item.
//                For use inside of the XmlFile class.
//

#include "XmlElement.h"
#include "PerlRegularExpression.h"

#include <string.h>


//////////////////////////////
//
// XmlElement::XmlElement --
//

XmlElement::XmlElement(void) { 
   entryType = XMLENTRY_UNKNOWN;
   startSerial    = -1;
   endSerial = -1;
   atkey.setSize(0);
   atvalue.setSize(0);
}



//////////////////////////////
//
// XmlElement::~XmlElement --
//

XmlElement::~XmlElement() { 
   clear();
}



//////////////////////////////
//
// XmlElement::clear -- erase the contents of the Entry
//

void XmlElement::clear(void) {
   int i;
   int asize = atkey.getSize();
   for (i=0; i<asize; i++) {
      delete atkey[i];
      atkey[i] = NULL;
      delete atvalue[i];
      atvalue[i] = NULL;
   }
   atkey.setSize(0);
   atvalue.setSize(0);
   entryType = XMLENTRY_UNKNOWN;
   startSerial    = -1;
   endSerial = -1;
}

//////////////////////////////
//
// XmlElement::setName --
//

void XmlElement::setName(const char* astring) {
  name = astring; 
}

void XmlElement::setName(Array<char>& astring) {
  name = astring; 
}

void XmlElement::setName(SigString& astring) {
  name = astring; 
}



//////////////////////////////
//
// XmlElement::getName --
//

const char* XmlElement::getName(void) {
   return name.getBase();
}



//////////////////////////////
//
// XmlElement::getType --
//

int XmlElement::getType(void) { 
   return entryType;
}



//////////////////////////////
//
// XmlElement::getSerial --
//

int XmlElement::getStartSerial(void) { 
   return startSerial;
}



//////////////////////////////
//
// XmlElement::setStartSerial --
//

void XmlElement::setStartSerial(int aserial) {
   startSerial = aserial;
}



//////////////////////////////
//
// XmlElement::getEndSerial --
//

int XmlElement::getEndSerial(void) { 
   return endSerial;
}



//////////////////////////////
//
// XmlElement::setEndSerial --
//

void XmlElement::setEndSerial(int aserial) { 
   endSerial = aserial;
}



//////////////////////////////
//
// XmlElement::isRegularElement --
//

int XmlElement::isRegularElement(void) { 
   return entryType & (XMLENTRY_ELEMENTSTART | XMLENTRY_ELEMENTSOLO);
}



//////////////////////////////
//
// XmlElement::isComment -- Returns true if the element (tag) is a comment
//     which starts with the string "<!--" and ends with the string "-->".
//     A comment may contain things which look like tags (angle brackets),
//     but are not supposed to be parsed.
//

int XmlElement::isComment(void) {
   return entryType & XMLENTRY_COMMENT;
}



//////////////////////////////
//
// XmlElement::isTagBang -- Returns true if the tag starts with "<!" but
//     not "<!--" (which is a comment).
//

int XmlElement::isTagBang(void) { 
   return entryType & XMLENTRY_TAGBANG;
}



//////////////////////////////
//
// XmlElement::isTagQuestion -- Returns true if the tag starts "<?".
//

int XmlElement::isTagQuestion(void) { 
   return entryType & XMLENTRY_TAGQUESTION;
}



//////////////////////////////
//
// XmlElement::parseXmlAttributes -- read an XML Elements attribute list.
//

void XmlElement::parseXmlAttributes(XmlItem& anItem) { 
//   setTypeByString(anItem);
//   startSerial = anItem.getSerial();
//   
// ggg
//


}



//////////////////////////////
//
// XmlElement::setTypeByString --
//

void setTypeByString(XmlItem& anItem) {
 // ggg
}


//////////////////////////////
//
// XmlElement::getAttributeSize --
//

int XmlElement::getAttributeSize(void) { 
  return atkey.getSize();
}



//////////////////////////////
//
// XmlElement::getAttributeIndex --
//

int XmlElement::getAttributeIndex(const char* name) { 
   int i;
   for (i=0; i<getAttributeSize(); i++) {
      if (strcmp(name, getAttributeName(i).getBase()) == 0) {
         return i;
      }
   }
   return -1;
}



//////////////////////////////
//
// XmlElement::getAttributeValue --
//

const char* XmlElement::getAttributeValue(const char* name) { 
   int index = getAttributeIndex(name);
   if (index < 0) {
      return "";
   }
 
   return getAttributeValue(index).getBase();
}



//////////////////////////////
//
// XmlElement::hasAttribute -- returns one plus the attribute name's
//     index if it is present, otherwise returns 0.
//

int XmlElement::hasAttribute(const char* name) { 
   return getAttributeIndex(name) + 1;
}



//////////////////////////////
//
// XmlElement::getAttributeName --
//

const SigString& XmlElement::getAttributeName(int index) { 
   static SigString empty;
   if (index < 0 || index >= getAttributeSize()) {
      return empty;
   }
   return *atkey[index];
}



//////////////////////////////
//
// XmlElement::getAttributeValue --
//

const SigString& XmlElement::getAttributeValue(int index) { 
   static SigString empty("");
   if (index < 0 || index >= getAttributeSize()) {
      return empty;
   }
   return *atvalue[index];
}




// md5sum: 051e558c8e271327a795fcad656d760b XmlElement.cpp [20050403]
