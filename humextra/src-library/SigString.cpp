//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb 20 13:22:05 PST 2011
// Last Modified: Sun Feb 20 13:22:08 PST 2011
// Filename:      ...sig/maint/code/base/SigString/SigString.cpp
// Web Address:   http://sig.sapp.org/include/sigBase/SigString.cpp
// Syntax:        C++ 
//
// Description:   Character array with a hidden null character at the
//                end of the array, so that either binary or c string
//                characters can be processed.
//

#include "SigString.h"
#include "Array.h"
#include <string.h>
#include "PerlRegularExpression.h"

#ifndef OLDCPP
   #include <ostream>
   using namespace std;
#else
   #include <ostream.h>
#endif

//////////////////////////////
//
// SigString::SigString -- constructors
//

SigString::SigString(void) { 
   clear();
}


SigString::SigString(int arraySize) { 
   setSize(arraySize);
}


SigString::SigString(SigString& aString) { 
   int aSize = aString.getSize();
   data.setSize(aSize);
   memcpy(data.getBase(), aString.getBase(), aSize);
}


SigString::SigString(Array<char>& aString) { 
   int aSize = aString.getSize();
   if ((aSize > 0) && (aString.last() == '\0')) {
      aSize--;
   }
   data.setSize(aSize);
   memcpy(data.getBase(), aString.getBase(), aSize);
}


SigString::SigString(int aSize, const char *cstring) { 
   data.setSize(aSize);
   memcpy(data.getBase(), cstring, aSize);
}


SigString::SigString(const char *cstring) { 
   int aSize = ::strlen(cstring);
   data.setSize(aSize);
   memcpy(data.getBase(), cstring, aSize);
}



//////////////////////////////
//
// SigString::~SigString -- desconstructor
//

SigString::~SigString() { 
   clear();
}



//////////////////////////////
//
// SigString::clear --
//

void SigString::clear(void) { 
   setSize(0);
}



//////////////////////////////
//
// SigString::setSize -- set the array size of the string, but add
//    an invisible null character at the end of the array.
//

void SigString::setSize(int aSize) { 
   data.setSize(aSize+1);
   data.last() = '\0';
   data.setSize(aSize);
}



//////////////////////////////
//
// SigString::getSize -- same a C language strlen() but does not include
//      the terminal null character.
//

int SigString::getSize(void) const { 
   return data.getSize();
}



//////////////////////////////
//
// SigString::operator[] -- access to individual characters.
//

char& SigString::operator[](int index) {
   return data[index];
}



//////////////////////////////
//
// SigString::operator== -- return true if equal, false otherwise.
//

int SigString::operator==(const SigString& aString) { 
   int aSize = aString.getSize();
   if (getSize() != aSize) {
      return 0;
   }
   int i;
   const char* ptr1 = getBase();
   const char* ptr2 = aString.getBase();
   unsigned long* lptr1 = (unsigned long*)ptr1;
   unsigned long* lptr2 = (unsigned long*)ptr2;
   int zsize = aSize / sizeof(unsigned long);
   int xsize = aSize % sizeof(unsigned long);
   for (i=0; i<zsize; i++) {
      if (*lptr1++ != *lptr2++) {
         return 0;
      }
   }
   zsize *= sizeof(unsigned long);  // change zsize to bytes;
   ptr1 += zsize;
   ptr2 += zsize;
   for (i=0; i<xsize; i++) {
      if (*ptr1++ != *ptr2++) {
         return 0;
      }
   }
   return 1;
}


int SigString::operator==(const Array<char>& aString) { 
   if (strcmp(getBase(), aString.getBase()) == 0) {
      return 1;
   } else {
      return 0;
   }
}


int SigString::operator==(const char* cstring) { 
   if (strcmp(getBase(), cstring) == 0) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// SigString::operator= --
//

SigString& SigString::operator=(const SigString& aString) { 
   int aSize = aString.getSize();
   setSize(aSize);
   memcpy(data.getBase(), aString.getBase(), aSize);
   return (*this);
}


SigString& SigString::operator=(Array<char>& aString) { 
   int aSize = aString.getSize();
   if ((aSize > 0) && (aString.last() == '\0')) {
      aSize--;
   }
   setSize(aSize);
   memcpy(data.getBase(), aString.getBase(), aSize);
   return (*this);
}


SigString& SigString::operator=(const char* cstring) {
   int aSize = ::strlen(cstring);
   setSize(aSize);
   memcpy(data.getBase(), cstring, aSize);
   return (*this);
}



//////////////////////////////
//
// SigString::operator+= -- append characters to end of string.
//

SigString& SigString::operator+=(const SigString& aString) { 
   int oldsize = getSize();
   setSize(oldsize + aString.getSize());
   memcpy(data.getBase()+oldsize, aString.getBase(), aString.getSize());
   return (*this);
}


SigString& SigString::operator+=(Array<char>& aString) { 
   int oldsize = getSize();
   int aSize = aString.getSize();
   if ((aSize > 0) && (aString.last() == '\0')) {
      aSize--;
   }
   setSize(oldsize + aSize);
   memcpy(data.getBase()+oldsize, aString.getBase(), aSize);
   return (*this);
}


SigString& SigString::operator+=(const char* cstring) { 
   int oldsize = getSize();
   int aSize = ::strlen(cstring);
   setSize(oldsize + aSize);
   memcpy(data.getBase()+oldsize, cstring, aSize);
   return (*this);
}



//////////////////////////////
//
// SigString::prepend --
//

SigString& SigString::operator-=(const SigString& aString) { 
   int oldsize = getSize();
   int aSize = aString.getSize();
   setSize(oldsize + aSize);
   memmove(getBase() + aSize, getBase(), aSize);
   memcpy(getBase(), aString.getBase(), aSize);
   return (*this);
}

SigString& SigString::operator-=(Array<char>& aString) { 
   int oldsize = getSize();
   int aSize = aString.getSize();
   if ((aSize > 0) && (aString.last() == '\0')) {
      aSize--;
   }
   setSize(oldsize + aSize);
   memmove(getBase() + aSize, getBase(), aSize);
   memcpy(getBase(), aString.getBase(), aSize);
   return (*this);
}


SigString& SigString::operator-=(const char* cstring) { 
   int oldsize = getSize();
   int aSize = ::strlen(cstring);
   setSize(oldsize + aSize);
   memmove(getBase() + aSize, getBase(), aSize);
   memcpy(getBase(), cstring, aSize);
   return (*this);
}



// special function for printing SigString values:
ostream& operator<<(ostream& out, SigString& aString) { 
   int i;
   int aSize = aString.getSize();
   const char* ptr = aString.getBase();
   for (i=0; i<aSize; i++) {
      out << *(ptr++);
   }
   return out;
}



// md5sum: 09d1b1f8e70ecde53f484548e48f33c3 SigString.cpp [20030102]

