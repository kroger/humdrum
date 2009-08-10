//
// Copyright 1998-1999 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jun  8 20:19:26 PDT 1998
// Last Modified: Mon Jun  8 21:11:00 PDT 1998
// Last Modified: Thu Jul  1 16:19:16 PDT 1999
// Filename:      ...sig/src/sigInfo/EnumerationData.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/EnumerationData.cpp
// Syntax:        C++ 
//
// Description:   Used to store enumerations and their string equivalents 
//                in the Enumeration class.
//

#include "EnumerationData.h"

#include <string.h>
#include <stdlib.h>

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif



//////////////////////////////
//
// EnumerationData::EnumerationData
//

EnumerationData::EnumerationData(void) {
   value = E_unknown;
   name = NULL;
   allocationType = ENUM_TRANSIENT_ALLOC;
}


EnumerationData::EnumerationData(int aValue, const char* aName, int allocType) {
   value = aValue;
   allocationType = allocType;
   if (allocatedQ()) {
      name = new char[strlen(aName)+1];
      strcpy(name, aName);
   } else {
      name = (char*)aName;
   }
}


EnumerationData::EnumerationData(const EnumerationData& aDatum) {
   value = aDatum.value;
   allocationType = aDatum.allocationType;
   if (allocatedQ()) {
      name = new char[strlen(aDatum.name)+1];
      strcpy(name, aDatum.name);
   } else {
      name = aDatum.name;
   }
}



//////////////////////////////
//
// EnumerationData::~EnumerationData
//

EnumerationData::~EnumerationData(void) {
   value = E_unknown;
   if (allocatedQ()) {
      delete [] name;
      name = NULL;
      allocationType = ENUM_TRANSIENT_ALLOC;
   } else {
      name = NULL;
      allocationType = ENUM_TRANSIENT_ALLOC;
   }
}
    


//////////////////////////////
//
// EnumerationData::allocatedQ -- return true if the internal
//    data is privately allocated, returns false if the data
//    is allocated externally.
//

int EnumerationData::allocatedQ(void) const {
   if (allocationType == ENUM_FIXED_ALLOC) {
      return 0;
   } else if (allocationType == ENUM_TRANSIENT_ALLOC) {
      return 1;
   } 

   cerr << "Error: unknown allocation type: " << allocationType << endl;
   exit(1);

   return 0;   // for stupid compilers with buggy warnings
}



//////////////////////////////
//
// EnumerationData::compare
//

int EnumerationData::compare(int aValue) const {
   if (getValue() < aValue) {
      return -1;
   } else if (getValue() > aValue) {
      return +1;
   } else {
      return 0;
   }
}


int EnumerationData::compare(const char* aName) const {
   if (aName == NULL) {
      cerr << "Error: there is no string in the enumaration datum" << endl;
      exit(1);
   }
   return strcmp(name, aName);
}


int EnumerationData::compare(const EnumerationData& aDatum) const {
   int comparison = compare(aDatum.getValue());
   if (comparison == 0) {
      // break tie with comparison of name
      return compare(aDatum.getName());
   } else {
      return comparison;
   }
}



//////////////////////////////
//
// EnumerationData::compareByName
//

int EnumerationData::compareByName(const EnumerationData& aDatum) const {
   return compare(aDatum.getName());
}



//////////////////////////////
//
// EnumerationData::compareByValue
//

int EnumerationData::compareByValue(const EnumerationData& aDatum) const {
   return compare(aDatum.getValue());
}



//////////////////////////////
//
// EnumerationData::getName
//

const char* EnumerationData::getName(void) const {
   return name;
}



//////////////////////////////
//
// EnumerationData::getValue
//

int EnumerationData::getValue(void) const {
   return value;
}



//////////////////////////////
//
// EnumerationData::operator= 
//

EnumerationData& EnumerationData::operator=(const EnumerationData& aDatum) {
   // don't copy onto self
   if (&aDatum == this) {
      return *this;
   }

   value = aDatum.value;
   if (allocatedQ() && name != NULL) {
      delete [] name;
      name = NULL;
   }

   allocationType = aDatum.allocationType;
   if (allocatedQ()) {
      name = new char[strlen(aDatum.name)+1];
      strcpy(name, aDatum.name);
   } else {
      name = aDatum.name;
   }

   return *this;
}



//////////////////////////////
//
// EnumerationData::operator==
//

int EnumerationData::operator==(const EnumerationData& aDatum) {
   if (compare(aDatum) == 0) {
      return 1;
   } else { 
      return 0;
   }
}



//////////////////////////////
//
// EnumerationData::operator<
//

int EnumerationData::operator<(const EnumerationData& aDatum) {
   if (compare(aDatum) == -1) {
      return 1;
   } else { 
      return 0;
   }
}



//////////////////////////////
//
// EnumerationData::operator>
//

int EnumerationData::operator>(const EnumerationData& aDatum) {
   if (compare(aDatum) == 1) {
      return 1;
   } else { 
      return 0;
   }
}



//////////////////////////////
//
// EnumerationData::setRelation
//	default value: allocType = ENUM_TRANSIENT_ALLOC
//

void EnumerationData::setRelation(int aValue, const char* aName, int allocType) {
   value = aValue;
   if (allocatedQ()) {
      delete [] name;
      name = NULL;
   }
   allocationType = allocType;
   if (allocatedQ()) {
      name = new char[strlen(aName)+1];
      strcpy(name, aName);
   } else {
      name = (char*)aName;
   }
}



//////////////////////////////
//
// EnumerationData::setName
//	default value: allocType = ENUM_TRANSIENT_DATA
//

void EnumerationData::setName(const char* aName, int allocType) {
   if (allocatedQ() && name != NULL) {
      delete [] name;
      name = NULL;
   }
   allocationType = allocType;
   if (allocatedQ()) {
      name = new char[strlen(aName)+1];
      strcpy(name, aName);
   } else {
      name = (char*)aName;
   }
}



//////////////////////////////
//
// EnumerationData::setValue
//

void EnumerationData::setValue(int aValue) {
   value = aValue;
}



///////////////////////////////////////////////////////////////////////////
//
// friendly functions
//



////////////////////
//
// operator<<
//

ostream& operator<<(ostream& out, const EnumerationData& aDatum) {
   // don't print the null enumeration
   if (aDatum.getValue() != E_unknown) {
      out << aDatum.getValue() << ":\t" << aDatum.getName();
   }
   return out;
}


ostream& operator<<(ostream& out, const EnumerationData* aDatum) {
   // don't print the null enumeration
   if (aDatum->getValue() != E_unknown) {
      out << aDatum->getValue() << ":\t" << aDatum->getName();
   }
   return out;
}


// md5sum: 51bd1a66561b64944c243f48a8710e53 EnumerationData.cpp [20050403]
