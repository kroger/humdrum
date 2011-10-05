//
// Copyright 2011 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun  7 13:04:03 PDT 2011
// Last Modified: Tue Jun  7 13:04:06 PDT 2011
// Filename:      ...sig/src/sigInfo/XmlItem.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/XmlItem.cpp
// Syntax:        C++ 
// 
// Description:   A class that stores a list of XML text fragments.
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
#include "XmlItem.h"
#include <string.h>


///////////////////////////////////////////////////////////////////////////
//
// XmlItem class functions --
//


//////////////////////////////
//
// XmlItem::XmlItem --
//

XmlItem::XmlItem(void) {
   setSerial(-1);
   setText("");
}

XmlItem::XmlItem(int serialnum, Array<char>& item) {
   setSerial(serialnum);
   setText(item);
}



//////////////////////////////
//
// XmlItem::getType -- return the raw type of item.
//

int XmlItem::getType(void) const {
   return itemtype;
}



//////////////////////////////
//
// XmlItem::isElement -- return true if an element start tag or
//     element solo tag (note: returns false if an element stop tag).
//

int XmlItem::isElement(void) const {
   return 0x0004 & itemtype;
}



//////////////////////////////
//
// XmlItem::isSpace -- return true if an item is null or contains
//     only space characters (spaces, tabs, newlines).
//

int XmlItem::isSpace(void) const {
   return 0x0010 & itemtype;
}



//////////////////////////////
//
// XmlItem::isText -- return true if an item is not a tag or a
//     whitespace item.
//

int XmlItem::isText(void) const {
   return 0x0020 & itemtype;
}



///////////////////////////////////////////////////////////////////////////
//
// private functions
//


////////////////////////////
//
// XmlItem::setSerial --
//

void XmlItem::setSerial(int aserial) {
   serial = aserial;
}



//////////////////////////////
//
// XmlItem::setText -- set the text string for the XML item.  The serial
//    must be set elsewhere (hence this function is private).
//

int XmlItem::setText(Array<char>& item) {
   string = item;
   return setType();
}

int XmlItem::setText(const char* item) {
   string = item;
   return setType();
}



//////////////////////////////
//
// XmlItem::setType -- either a whitespace, text, element (element start tag
//     element tag), comment "<!--.*-->", bang "<!", question "<?".
//     Read the start of the text and determine what form the item takes.
//

int XmlItem::setType(void) {
   PerlRegularExpression pre;

   if (pre.search(string.getBase(), "^\\s*$")) {
      itemtype = XMLITEM_TYPE_WHITESPACE;
      return itemtype;
   }

   if (!pre.search(string.getBase(), "^\\s*<")) {
      itemtype = XMLITEM_TYPE_TEXT;
      return itemtype;
   }

   if (pre.search(string.getBase(), "^\\s*<\\s*!\\s*-\\s*-\\s*")) {  
      itemtype = XMLITEM_TYPE_COMMENT;
      return itemtype;
   }
   
   if (pre.search(string.getBase(), "^\\s*<\\s*!")) {
      itemtype = XMLITEM_TYPE_BANG;
      return itemtype;
   }

   if (pre.search(string.getBase(), "^\\s*<\\s*\\?")) {
      itemtype = XMLITEM_TYPE_QUESTION;
      return itemtype;
   }

   if (pre.search(string.getBase(), "^\\s*<\\s*[A-Z\\d].*\\s*/\\s*>")) {
      itemtype = XMLITEM_TYPE_ELEMENTSOLO;
      return itemtype;
   }

   if (pre.search(string.getBase(), "^\\s*<\\s*/\\s*[A-Z\\d].*\\s*>")) {
      itemtype = XMLITEM_TYPE_ELEMENTEND;
      return itemtype;
   }

   if (pre.search(string.getBase(), "^\\s*<\\s*[A-Za-z\\d].*\\s*>")) {
      itemtype = XMLITEM_TYPE_ELEMENTSTART;
      return itemtype;
   }

   itemtype = XMLITEM_TYPE_UNKNOWN;
   return itemtype;
}
   


///////////////////////////////////////////////////////////////////////////
//
// Friendly functions
//

ostream& operator<<(ostream& out, XmlItem& item) {
   out << item.cstr();
   return out;
}




// md5sum: 05e1e67705ebcb2bfa1afad9e504582c XmlItem.cpp [20050403]
