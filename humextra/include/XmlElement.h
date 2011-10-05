//
// Copyright 2011 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jun  6 14:21:45 PDT 2011
// Last Modified: Fri Jun 24 15:10:39 PDT 2011
// Filename:      ...sig/include/SigInfo/XmlElement.h
// Web Address:   http://sig.sapp.org/include/sigInfo/XmlElement.h
// Syntax:        C++ 
//
// Description:   A class that stores a parsing of an XML item.
//                For use inside of the XmlFile class.
//

#ifndef _XMLENTRY_H_INCLUDED
#define _XMLENTRY_H_INCLUDED

#define XMLENTRY_UNKNOWN      0x0       /* unparsed entry type  */
#define XMLENTRY_TEXT         0x1       /* text outside of tags */
#define XMLENTRY_SPACE        0x2       /* whitespace before/after tag */
#define XMLENTRY_ELEMENTSTART 0x4       /* <...>      */
#define XMLENTRY_ELEMENTSOLO  0x8       /* <.../>     */
#define XMLENTRY_ELEMENTEND   0x16      /* </...>     */
#define XMLENTRY_COMMENT      0x32      /* <!--...--> */
#define XMLENTRY_TAGBANG      0x64      /* <!...>     */
#define XMLENTRY_TAGQUESTION  0x128     /* <?...>     */


#include "XmlItem.h"
#include "SigString.h"

class XmlElement {
   public:
                  XmlElement        (void);
                 ~XmlElement        ();

      void        setTypeByString   (XmlItem& anItem);
      int         getType           (void);
      int         getStartSerial    (void);
      void        setStartSerial    (int aserial);
      int         getEndSerial      (void);
      void        setEndSerial      (int aserial);
      void        clear             (void);
      void        setName           (const char* astring);
      void        setName           (Array<char>& astring);
      void        setName           (SigString& astring);
      const char* getName           (void);

      int         isRegularElement  (void);  /* <...></...> or <.../> */
      int         isComment         (void);  /* <!--...--> */
      int         isTagBang         (void);  /* <!...> excluding comment */
      int         isTagQuestion     (void);  /* <?...> */

      void        parseXmlAttributes(XmlItem& anItem);

      int         getAttributeSize  (void);

      int         getAttributeIndex (const char* name);
      const char* getAttributeValue (const char* name);
      int         hasAttribute      (const char* name);

      const SigString& getAttributeName  (int index);
      const SigString& getAttributeValue (int index);

   protected:
      int               entryType;   // what type of XML element
      int               startSerial; // used to acess raw string
      int               endSerial;   // used for Element end marker
      SigString         name;        // name of the element
      Array<SigString*> atkey;       // attribute keys
      Array<SigString*> atvalue;     // attribute values
      
};


#endif  /* _XMLELEMENT_H_INCLUDED */


// md5sum: 051e558c8e271327a795fcad656d760b XmlElement.h [20050403]
