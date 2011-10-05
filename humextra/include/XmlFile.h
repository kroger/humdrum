//
// Copyright 2011 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jun  6 14:21:45 PDT 2011
// Last Modified: Thu Jul  7 13:22:35 PDT 2011
// Filename:      ...sig/include/SigInfo/XmlFile.h
// Web Address:   http://sig.sapp.org/include/sigInfo/XmlFile.h
// Syntax:        C++ 
//
// Description:   A class that stores a list of XML text fragments
//                and then parses the element structure of the file.
//

#ifndef _XMLFILE_H_INCLUDED
#define _XMLFILE_H_INCLUDED

#include "XmlFileBasic.h"
#include "XmlElement.h"

#define XMLENTRY_UNKNOWN      0x0       /* unparsed entry type  */
#define XMLENTRY_TEXT         0x1       /* text outside of tags */
#define XMLENTRY_SPACE        0x2       /* whitespace before/after tag */
#define XMLENTRY_ELEMENTSTART 0x4       /* <...>      */
#define XMLENTRY_ELEMENTSOLO  0x8       /* <.../>     */
#define XMLENTRY_ELEMENTEND   0x16      /* </...>     */
#define XMLENTRY_COMMENT      0x32      /* <!--...--> */
#define XMLENTRY_TAGBANG      0x64      /* <!...>     */
#define XMLENTRY_TAGQUESTION  0x128     /* <?...>     */


class XmlFile {
   public:
                   XmlFile             (void);
                   XmlFile             (int allocationSize);
      //           XmlFile             (XmlFile& input);
                  ~XmlFile             ();

      int          getItemSize         (void) const;
      int          getElementSize      (void);
      int          getItemIndexBySerial(int aserial) const;
      int          getItemSerial       (int index) const;
      XmlElement&  operator[]          (int index);
      XmlItem&     getItem             (int index);
      void         clear               (void);
      void         read                (const char* filename);
      void         read                (istream& input);
      const XmlFileBasic& raw          (void);
      int          appendItem          (Array<char>& item);
      int          insertItem          (Array<char>& item, int index);
      int          deleteItem          (int index);

      ostream&     printElementList    (ostream& out);

   private:

      XmlFileBasic       xmlitems;
      Array<XmlElement*> elements;
      int                parsedQ;     // true if element structure created

   protected:

      void   parseElements           (void);
      void   parseAttributes         (void);
      int    processElement          (int index);
      int    isParsed                (void);
      void   clearElements           (void);

};
   

ostream& operator<<(ostream& out, XmlFile& xmlfile);



#endif  /* _XMLFILE_H_INCLUDED */


// md5sum: 051e558c8e271327a795fcad656d760b XmlFile.h [20050403]
