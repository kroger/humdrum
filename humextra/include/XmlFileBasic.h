//
// Copyright 2011 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jun  6 14:21:45 PDT 2011
// Last Modified: Tue Jun  7 13:02:16 PDT 2011
// Filename:      ...sig/include/SigInfo/XmlFileBasic.h
// Web Address:   http://sig.sapp.org/include/sigInfo/XmlFileBasic.h
// Syntax:        C++ 
//
// Description:   A class that stores a list of XML text fragments.
//

#ifndef _XMLFILEBASIC_H_INCLUDED
#define _XMLFILEBASIC_H_INCLUDED

#include "Array.h"
#include "XmlItem.h"


class XmlFileBasic {
   public:
                 XmlFileBasic     (void);
                 XmlFileBasic     (int allocationSize);
                 XmlFileBasic     (XmlFileBasic& input);
                ~XmlFileBasic     ();

      int        insertItem       (Array<char>& item, int index);
      int        appendItem       (Array<char>& item);
      int        deleteItem       (int index);
      void       read             (const char* filename);
      void       read             (istream& input);
      void       clear            (void);
      int        getItemSize      (void) const;
      int        getElementSize   (void);
      int        getSerial        (int index) const;
      int        getSize          (void) const;
      int        getIndexBySerial (int index) const;
      XmlItem&   operator[]       (int index);

   private:

      Array<XmlItem*> itemList;
      Array<int>      serialMap;
      int             serialgen;

   protected:
      int        assignSerialMapping (int index);
      void       parseXmlFile        (const char* filename);
      void       parseXmlStream      (istream& input);

      void       extractText         (Array<char>& output, 
                                      Array<char>& trailingspace,
                                        istream& input);
      void       extractTag          (Array<char>& output, istream& input);
      void       extractWhiteSpace   (Array<char>& output, 
                                        istream& input);

};
   

ostream& operator<<(ostream& out, XmlFileBasic& xmlfile);



#endif  /* _XMLFILEBASIC_H_INCLUDED */


// md5sum: 051e558c8e271327a795fcad656d760b XmlFileBasic.h [20050403]
