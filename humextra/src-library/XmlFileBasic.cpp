//
// Copyright 2011 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun  7 13:04:03 PDT 2011
// Last Modified: Tue Jun  7 13:04:06 PDT 2011
// Filename:      ...sig/src/sigInfo/XmlFileBasic.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/XmlFileBasic.cpp
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
   
#include "XmlFileBasic.h"
#include <string.h>


///////////////////////////////////////////////////////////////////////////
//
// XmlFileBasic class functions --
//


//////////////////////////////
//
// XmlFileBasic::XmlFileBasic --
//

XmlFileBasic::XmlFileBasic(void) { 
   itemList.setSize(1123123);
   itemList.setSize(0);
   itemList.setGrowth(10123123);
   serialgen = 0;   // the serial number of the next item
   serialMap.setSize(1123123);
   serialMap.setSize(0);
   serialMap.setGrowth(10123123);
}

XmlFileBasic::XmlFileBasic(int allocationSize) { 
   itemList.setSize(allocationSize);
   itemList.setSize(0);
   itemList.setGrowth(allocationSize);
   serialgen = 0;   // the serial number of the next item
}



//////////////////////////////
//
// XmlFileBasic::~XmlFileBasic --
//

XmlFileBasic::~XmlFileBasic() { 
   clear();
}



//////////////////////////////
//
// XmlFileBasic::clear -- remove all contents of the data.
//

void XmlFileBasic::clear(void) {
   int i;
   int asize = itemList.getSize();
   for (i=0; i<asize; i++) {
      delete itemList[i];
      itemList[i] = NULL;
   }
   itemList.setSize(0);
   serialgen = 0;     // reset the serial number generator
}



//////////////////////////////
//
// XmlFileBasic::getSize -- returns the number of stored items.
//

int XmlFileBasic::getSize(void) const {
   return itemList.getSize();
}



//////////////////////////////
//
// XmlFileBasic::getIndexBySerial -- returns the index number of stored item
//     according to its serial number.  Serial number will remain constant,
//     but the index number may change if items are inserted or deleted.
//     Non-mapped serial number returns an index of -1.
//

int XmlFileBasic::getIndexBySerial(int aserial) const {
   return serialMap[aserial];
}



//////////////////////////////
//
// XmlFileBasic::getSerial -- returns the serial number of stored item.
//

int XmlFileBasic::getSerial(int index) const {
   return itemList[index]->getSerial();
}



//////////////////////////////
//
// XmlFileBasic::operator[] --
//

XmlItem& XmlFileBasic::operator[](int index) {
   return *itemList[index];
}



//////////////////////////////
//
// XmlFileBasic::read --
//

void XmlFileBasic::read(const char* filename) { 
   parseXmlFile(filename);
}


void XmlFileBasic::read(istream& input) { 
   parseXmlStream(input);
}


//////////////////////////////
//
// XmlFileBasic::appendItem -- add an item to the end of the
//    data, and return the serial number attached to it.
//

int XmlFileBasic::appendItem(Array<char>& item) {
   XmlItem* ptr;
   int index = itemList.getSize();
   ptr = new XmlItem(assignSerialMapping(index), item);
   itemList.append(ptr);
   return ptr->getSerial();
}


//////////////////////////////
//
// XmlFileBasic::insertItem -- place an element at the given
//    index into the list of items.  0 = instart at start of list.
//    Use appendItem to place at end of list.  Return value is the
//    serial number of the item.
//

int XmlFileBasic::insertItem(Array<char>& item, int index) {
   XmlItem* ptr;
   ptr = new XmlItem(assignSerialMapping(index), item);
   itemList.setSize(itemList.getSize()+1);

   memmove(itemList.getBase()+index+1, itemList.getBase()+index, 
      sizeof(void*)*(itemList.getSize()-index-1));

   itemList[index] = ptr;
   return ptr->getSerial();
}



//////////////////////////////
//
// XmlFileBasic::deleteItem -- returns true if an item was deleted.
//

int XmlFileBasic::deleteItem(int index) {
   if (index < 0 || index >= itemList.getSize()) {
      return 0;
   }

   serialMap[itemList[index]->getSerial()] = -1;

   delete itemList[index];
   XmlItem** ptr  = &itemList[index];
   XmlItem** ptr2 = &itemList[index+1];
   int asize = itemList.getSize() - index - 1;
   memmove(ptr, ptr2, sizeof(void*)*asize);
   itemList.setSize(itemList.getSize()-1);

   return 1;
}



///////////////////////////////////////////////////////////////////////////
//
// private functions --
//


//////////////////////////////
//
// XmlFileBasic::parseXmlFile --
//

void XmlFileBasic::parseXmlFile(const char* filename) {
   fstream input;
   input.open(filename, ios::in);
   parseXmlStream(input);
}



//////////////////////////////
//
// parseXmlStream --
//

void XmlFileBasic::parseXmlStream(istream& input) {
   clear();
   itemList.setSize(1123123);
   itemList.setGrowth(1123123);
   itemList.setSize(0);

   char null = '\0';
   int ch;
   Array<char> space;
   Array<char> tag;
   Array<char> text;

   while (!input.eof()) {
      ch = input.peek();

      if (ch < 0) {
         // end of data stream
         break;
      }
      if (isspace((char)ch)) {
         extractWhiteSpace(space, input);
         space.append(null); 
         appendItem(space);
      } else if ((char)ch == '<') {
         extractTag(tag, input);
         tag.append(null); 
         appendItem(tag);
      } else {
         extractText(text, space, input);
         text.append(null); 
         appendItem(text);
         if (space.getSize() > 0) {
            space.append(null); 
            appendItem(space);
         }
      }
   }
}



//////////////////////////////
//
// XmlFileBasic::extractText --  Extract a text string from an XML file stream
//    excluding any space characters at the end of the text string which
//    are stored separately.  Assumes first character is not '<' and 
//    the first character should also not be a space character, unless
//    you want a text item to start with space.
//          output = text (with no null termination so that UTF-16 can
//                   eventually be processed.
//          trailingspace = any white space found after text and before any
//                   subsequent tag.
//

void XmlFileBasic::extractText(Array<char>& output, Array<char>& trailingspace,
      istream& input) {

   output.setSize(0);
   Array<char>& whitespace = trailingspace;
   whitespace.setSize(0);
   
   int i;
   int ch;
   char cch;

   ch = input.get();
   while ((!input.eof()) && ((char)ch != '<')) {
      cch = (char)ch;
      if (isspace(cch)) {
         whitespace.append(cch);
      } else {
         for (i=0; i<whitespace.getSize(); i++) {
            output.append(*(whitespace.getBase()+i));
         }
         whitespace.setSize(0);
         output.append(cch);
      }
      ch = input.get();
   }
   if ((char)ch == '<') {
      input.putback((char)ch);
   }

}



//////////////////////////////
//
// extractTag -- assumes first character in stream at this point is "<";
//
// read through until a ">" is found.  Whenever an "=" is found, switch
// to an attribute-reading mode, looking for paired double or single quotes
//

void XmlFileBasic::extractTag(Array<char>& output, istream& input) {
   int attributeMode =  0;
   int parenState    = 'x';
   int finishedQ     =  0;
   int commentQ      =  0;
   int secondchar    = -1;
   int thirdchar     = -1;
   int fourthchar    = -1;
   int charcount     =  0;
   int lastchar      = -1;
   int lastlastchar  = -1;

   int ch = -1;
   char cch;
   lastlastchar = lastchar;
   lastchar = ch;
   ch = input.get();
   charcount++;
   output.setSize(0);
   while ((!input.eof()) && (!finishedQ)) {
      cch = (char)ch;
      if      (charcount == 2) { secondchar = ch; }
      else if (charcount == 3) { thirdchar = ch; }
      else if (charcount == 4) { fourthchar = ch; 
         if ((secondchar == '!') && (thirdchar == '-') && (fourthchar == '-')) {
            commentQ = 1;
         }
      }

      switch (attributeMode) {
         case 0:   // in tag but outside of an attribute value
            if (cch == '=') {
               if (!commentQ) {
                  // uncomment to parse attribute data:
                  // attributeMode = 1;
               } 
            } else if (cch == '>') {
               if (commentQ) {
                  if ((lastlastchar == '-') && (lastchar == '-')) {
                     finishedQ = 1;
                  }
               } else {
                  finishedQ = 1;
               }
            }
            output.append(cch);
            break;

         case 1:   // start of attribute value but not inside of parens
            if (isspace(cch)) {
               output.append(cch);
            } else if (cch == '\'') {
               parenState = '\'';
               attributeMode = 2;
            } else if (cch == '"') {
               parenState = '"';
               attributeMode = 2;
            } else {
               cerr << "ERROR READING ATTRIBUTE VALUE\n";
               exit(1);
            }
            if (commentQ) {
               attributeMode = 0;
               parenState = 'x';
            }

         case 2:   // in attribute value within parens.
            if (cch == parenState) {
              attributeMode = 0;
            }
            if (cch == '>') {
               cerr << "ERROR: '>' cannot occur within an attribute value\n";
               exit(1);
            } if (cch == '<') {
               cerr << "ERROR: '<' cannot occur within an attribute value\n";
               exit(1);
            }
            output.append(cch);
      }
      if (finishedQ) {
         break;
      }
      lastlastchar = lastchar;
      lastchar = ch;
      ch = input.get();
      charcount++;
   }

}



//////////////////////////////
//
// extractWhiteSpace --
//

void XmlFileBasic::extractWhiteSpace(Array<char>& output, istream& input) {
   int ch;
   char cch;
   output.setSize(0);
   ch = input.get();
   while (!input.eof() && isspace((char)ch)) {
      cch = (char)ch;
      output.append(cch);
      ch = input.get();
   }
   if (!input.eof()) {
      input.putback((char)ch);
   }
}


//////////////////////////////
//
// XmlFileBasic::assignSerialMapping --
//

int XmlFileBasic::assignSerialMapping(int index) {
   if (index < 0) {
      return -1;
   }
   if (serialgen == serialMap.getSize()) {
      serialMap.append(serialgen);
   } else if (serialgen < serialMap.getSize()) {
      // this should not happen...
      serialMap[serialgen] = index;
   } else {
      // this should also not happen...
      int oldsize = serialMap.getSize();
      serialMap.setSize(serialgen+1);
      int i;
      for (i=oldsize; i<serialMap.getSize()-1; i++) {
         serialMap[i] = -1;
      }
      serialMap[serialgen] = index;
   }

   serialgen++;
   return serialgen-1;
}



///////////////////////////////////////////////////////////////////////////
//
// friendly functions
//


//////////////////////////////
//
// operator<< --
//

ostream& operator<<(ostream& out, XmlFileBasic& xmlfile) {
   int i, j;
   int asize = xmlfile.getSize();
   int bsize;
   char* ptr;
   for (i=0; i<asize; i++) {
      bsize = xmlfile[i].getSize();
      ptr = xmlfile[i].getBase();
      for (j=0; j<bsize; j++) {
         out << ptr[j];
      }
   }
   return out;
}




// md5sum: 05e1e67705ebcb2bfa1afad9e504582c XmlFileBasic.cpp [20050403]
