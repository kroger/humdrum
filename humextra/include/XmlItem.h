//
// Copyright 2011 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jun  6 14:21:45 PDT 2011
// Last Modified: Mon Jul 11 15:32:40 PDT 2011
// Filename:      ...sig/include/SigInfo/XmlItem.h
// Web Address:   http://sig.sapp.org/include/sigInfo/XmlItem.h
// Syntax:        C++ 
//
// Description:   A class that stores a list of XML text fragments.
//

#ifndef _XMLITEM_H_INCLUDED
#define _XMLITEM_H_INCLUDED

#include "Array.h"
#include "SigString.h"

// Type bits
// bit 0:  nontag
// bit 1:  tag
// bit 2:  element
// bit 3:  reserved
// --------------------
// bit 4:  whitespace
// bit 5:  text
// bit 6:  reserved
// bit 7:  reserved
// --------------------
// bit 8:  element solo
// bit 9:  element start
// bit 10: element end
// bit 11: reserved
// --------------------
// bit 12: comment
// bit 13: bang
// bit 14: question
// bit 15: not used (to avoid signed short int confusions)

#define XMLITEM_TYPE_UNKNOWN       0
#define XMLITEM_TYPE_ELEMENTSOLO   0x0106
#define XMLITEM_TYPE_ELEMENTSTART  0x0206
#define XMLITEM_TYPE_ELEMENTEND    0x0402
#define XMLITEM_TYPE_WHITESPACE    0x0011
#define XMLITEM_TYPE_TEXT          0x0021
#define XMLITEM_TYPE_COMMENT       0x1002
#define XMLITEM_TYPE_BANG          0x2002
#define XMLITEM_TYPE_QUESTION      0x4002

class XmlItem {
   public:

                 XmlItem     (int serialnum, Array<char>& item);
                 XmlItem     (void);

                ~XmlItem     (void)  { clear(); }


       void      clear       (void)  { serial = -1; string.setSize(0); }

       int       getSerial   (void) const { return serial; }
       int       getSize     (void)  { return string.getSize(); }
       char*     getBase     (void)  { return string.getBase(); }
       char*     cstr        (void)  { return string.getBase(); }
       char*     c_str       (void)  { return string.getBase(); }
       char*     cstring     (void)  { return string.getBase(); }

       int       isElement   (void) const;
       int       isText      (void) const;
       int       isSpace     (void) const;
       int       getType     (void) const;

   protected:
      SigString string;
      int       serial;
      int       itemtype;

   private:
      int       setType      (void);
      void      setSerial    (int aserial);
      int       setText      (Array<char>& item);
      int       setText      (const char* item);

};


ostream& operator<<(ostream& out, XmlItem& item);


#endif  /* _XMLITEM_H_INCLUDED */


// md5sum: 051e558c8e271327a795fcad656d760b XmlItem.h [20050403]
