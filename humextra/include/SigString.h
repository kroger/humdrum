//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb 20 13:22:05 PST 2011
// Last Modified: Sun Feb 20 13:22:08 PST 2011
// Filename:      ...sig/maint/code/base/SigString/SigString.h
// Web Address:   http://sig.sapp.org/include/sigBase/SigString.h
// Syntax:        C++ 
//
// Description:   Character array with a hidden null character at the
//                end of the array, so that either binary or c string
//                characters can be processed.
//

#ifndef _SIGSTRING_H_INCLUDED
#define _SIGSTRING_H_INCLUDED

#include "Array.h"

#ifndef OLDCPP
   #include <ostream>
   using namespace std;
#else
   #include <ostream.h>
#endif

class SigString {
   public:
                 SigString         (void);
                 SigString         (int arraySize);
                 SigString         (SigString& aString);
                 SigString         (Array<char>& aString);
                 SigString         (int aSize, const char *cstring);
                 SigString         (const char *cstring);
                ~SigString         ();

      void       clear             (void);

      void       setSize           (int aSize);
      int        getSize           (void) const;
      int        strlen            (void) { return getSize(); }
      char*      getBase           (void) const { return data.getBase(); }

      char&      operator[]        (int index);

      int        operator==        (const SigString& aString);
      int        operator==        (const Array<char>& aString);
      int        operator==        (const char* aString);

      SigString& operator=         (const SigString& aString);
      SigString& operator=         (Array<char>& aString);
      SigString& operator=         (const char* cstring);

      // append string to end
      SigString& operator+=        (const SigString& aString);
      SigString& operator+=        (Array<char>& aString);
      SigString& operator+=        (const char* cstring);

      // prepend string to start
      SigString& operator-=        (const SigString& aString);
      SigString& operator-=        (Array<char>& aString);
      SigString& operator-=        (const char* cstring);

   protected:
      Array<char> data;
};


// special function for printing SigString values:
ostream& operator<<(ostream& out, SigString& astring);


#endif  /* _SIGSTRING_H_INCLUDED */


// md5sum: 09d1b1f8e70ecde53f484548e48f33c3 SigString.h [20030102]

