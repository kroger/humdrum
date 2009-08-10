//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jul  9 14:25:29 PDT 1998
// Last Modified: Thu Jul  9 14:25:29 PDT 1998
// Filename:      ...sig/include/sigInfo/Identify.h
// Web Address:   http://sig.sapp.org/src/sigInfo/Identify.h
// Syntax:        C++ 
//
// Description:   Helping class to identify various string formats.
//

#ifndef _IDENFITY_H_INCLUDED
#define _IDENFITY_H_INCLUDED

class Identify {
   public:

   // identifications dealing with **kern
      static int humdrumNullField(const char* aField);
      static int kernTimeSignature(const char* kernString);
};


#endif /* _IDENFITY_H_INCLUDED */



// md5sum: 4faaac071629dc545cc9b34205c4f704 Identify.h [20050403]
