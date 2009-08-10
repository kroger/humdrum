//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jun  8 21:45:27 PDT 1998
// Last Modified: Tue Jun 23 14:06:21 PDT 1998
// Filename:      ...sig/src/sigInfo/Enumeration.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/Convert.cpp
// Syntax:        C++ 
//
// Description:   Messy yet functional way of handling enumerations
//                and their string equivalents.
//

#include "Enumeration.h"

#ifndef OLDCPP
   using namespace std;
#endif



//////////////////////////////
//
// Enumeration::Enumeration --
//

Enumeration::Enumeration(void) {
   associations.allowGrowth();
   associations.setSize(0);
   sortByValue.allowGrowth();
   sortByValue.setSize(0);
   sortByName.allowGrowth();
   sortByName.setSize(0);

   EnumerationData aDatum(E_unknown, "", ENUM_FIXED_ALLOC);
   associations.append(aDatum);
   EnumerationData* pointer = &associations[0];
   sortByValue.append(pointer);
   sortByName.append(pointer);

   sortQ = 1;
}


Enumeration::Enumeration(const Enumeration& aSet) {
   int size = aSet.associations.getSize();
   associations.allowGrowth();
   associations.setSize(size);
   sortByValue.allowGrowth();
   sortByValue.setSize(size);
   sortByName.allowGrowth();
   sortByName.setSize(size);
   sortQ = aSet.sortQ;

   for (int i=0; i<size; i++) {
      associations[i] = aSet.associations[i];
      sortByValue[i] = aSet.sortByValue[i];
      sortByName[i] = aSet.sortByName[i];
   }
}



//////////////////////////////
//
// Enumeration::~Enumeration --
//

Enumeration::~Enumeration() {
   // nothing
}



//////////////////////////////
//
// Enumeration::add --
//	default value: allocType = ENUM_TRANSIENT_ALLOC
//

void Enumeration::add(int aValue, const char* aName, int allocType) {
   EnumerationData aDatum(aValue, aName, allocType);
   add(aDatum);
}


void Enumeration::add(const EnumerationData& aDatum) {
   associations[associations.getSize()] = aDatum;
   sortByValue[sortByValue.getSize()] = NULL;
   sortByName[sortByName.getSize()] = NULL;
   sortQ = 0;
}



//////////////////////////////
//
// Enumeration::associate --
//	default value: allocType = ENUM_TRANSIENT_ALLOC
//

int Enumeration::associate(const char* aName, int allocType) {
   int associationValue = getFreeValue();
   add(associationValue, aName, allocType);
   return associationValue;
}



//////////////////////////////
//
// Enumeration::getAssociation --
//

const char* Enumeration::getAssociation(int aValue) {
   return getName(aValue);
}


int Enumeration::getAssociation(const char* aName) {
   return getValue(aName);
}



//////////////////////////////
//
// Enumeration::getName --
//

const char* Enumeration::getName(int aValue) {
   checksort();
   EnumerationData aDatum(aValue, "", ENUM_FIXED_ALLOC);
   EnumerationData* pointer = &aDatum;
   void* searchResult;
   searchResult = bsearch(&pointer, sortByValue.getBase(), 
         sortByValue.getSize(), sizeof(EnumerationData*), 
         data_compare_by_value_only);

   if (searchResult == NULL) {
      return "";
   } else {
      return (*(EnumerationData**)searchResult)->getName();
   }
}



//////////////////////////////
//
// Enumeration::getValue --
//

int Enumeration::getValue(const char* aName) {
   checksort();
   EnumerationData aDatum(E_unknown, aName, ENUM_FIXED_ALLOC);
   EnumerationData* pointer = &aDatum;
   void* searchResult;
   searchResult = bsearch(&pointer, sortByName.getBase(), sortByName.getSize(),
         sizeof(EnumerationData*), data_compare_by_name_only);

   if (searchResult == NULL) {
      return E_unknown;
   } else {
      return (*(EnumerationData**)searchResult)->getValue();
   }

   return E_unknown;         // for dumb compilers
}



//////////////////////////////
//
// Enumeration::memberQ --
//

int Enumeration::memberQ(int aValue) {
   checksort();
   EnumerationData aDatum(aValue, "", ENUM_FIXED_ALLOC);
   EnumerationData* pointer = &aDatum;
   void* searchResult;
   searchResult = bsearch(&pointer, sortByValue.getBase(), 
         sortByValue.getSize(), sizeof(EnumerationData*), 
         data_compare_by_value_only);

   if (searchResult == NULL) {
      return 0;
   } else {
      return 1;
   }
}


int Enumeration::memberQ(const char* aName) {
   checksort();
   EnumerationData aDatum(E_unknown, aName, ENUM_FIXED_ALLOC);
   EnumerationData* pointer = &aDatum;
   void* searchResult;
     searchResult = bsearch(&pointer, sortByName.getBase(), 
           sortByName.getSize(), sizeof(EnumerationData*), 
           data_compare_by_name_only);

   if (searchResult == NULL) {
      return 0;
   } else {
      return 1;
   }
}


int Enumeration::memberQ(const EnumerationData& aDatum) {
   checksort();
   void* searchResult;
   const EnumerationData* pointer = &aDatum;
   searchResult = bsearch(&pointer, sortByValue.getBase(), 
         sortByValue.getSize(), sizeof(EnumerationData*), 
         data_compare);

   if (searchResult == NULL) {
      return 0;
   } else {
      return 1;
   }
}
   


//////////////////////////////
//
// Enumeration::getFreeValue --
//

int Enumeration::getFreeValue(void) {
   checksort();
   return sortByValue[sortByValue.getSize()-1]->getValue() + 1;
}



//////////////////////////////
//
// Enumeration::print --
//
   
void Enumeration::print(ostream& out) {
   // don't print the null enumeration
   for (int i=1; i<associations.getSize(); i++) {
      out << associations[i] << '\n';
   }
}



//////////////////////////////
//
// Enumeration::printByName --
//
   
void Enumeration::printByName(ostream& out) {
   checksort();
   // don't print the null enumeration
   for (int i=0; i<sortByName.getSize(); i++) {
      if (sortByName[i]->getValue() != E_unknown) {
         out << sortByName[i] << '\n';
      }
   }
}



//////////////////////////////
//
// Enumeration::printByValue --
//
   
void Enumeration::printByValue(ostream& out) {
   checksort();
   // don't print the null enumeration
   for (int i=0; i<sortByValue.getSize(); i++) {
      if (sortByValue[i]->getValue() != E_unknown) {
         out << sortByValue[i] << '\n';
      }
   }
}



//////////////////////////////
//
// Enumeration::setNullName --
//	default value: allocType = ENUM_TRANSIENT_ALLOC
//
   
void Enumeration::setNullName(const char* aName, int allocType) {
   associations[0].setName(aName, allocType);
}



////////////////////////////////////////////////////////////////////////////
//
// protected functions
//


//////////////////////////////
//
// Enumeration::checksort --
//

void Enumeration::checksort(void) {
   if (!sortQ) {
      sort();
   }
}



//////////////////////////////
//
// Enumeration::sort --
//

void Enumeration::sort(void) {
   EnumerationData* pointer;
   for (int i=0; i<associations.getSize(); i++) {
      pointer = &associations[i];
      sortByValue[i] = pointer;
      sortByName[i] = pointer;
   }

   qsort(sortByValue.getBase(), sortByValue.getSize(), sizeof(EnumerationData*),
         data_compare_by_value);

   qsort(sortByName.getBase(), sortByName.getSize(), sizeof(EnumerationData*),
         data_compare_by_name);

   sortQ = 1;
}



////////////////////////////////////////////////////////////////////////////
//
// helping functions
//


//////////////////////////////
//
// Enumeration::data_compare --
//

int Enumeration::data_compare(const void* a, const void* b) {
   EnumerationData& valuea = *((EnumerationData*)a);
   EnumerationData& valueb = *((EnumerationData*)b);
   return valuea.compare(valueb);
}


//////////////////////////////
//
// Enumeration::data_compare_by_value --
//

int Enumeration::data_compare_by_value(const void* a, 
      const void* b) {
   EnumerationData& valuea = **((EnumerationData**)a);
   EnumerationData& valueb = **((EnumerationData**)b);
   return valuea.compareByValue(valueb);
}



//////////////////////////////
//
// Enumeration::data_compare_by_name --
//

int Enumeration::data_compare_by_name(const void* a, 
      const void* b) {
   EnumerationData& valuea = **((EnumerationData**)a);
   EnumerationData& valueb = **((EnumerationData**)b);
   return valuea.compareByName(valueb);
}



//////////////////////////////
//
// Enumeration::data_compare_by_value_only --
//

int Enumeration::data_compare_by_value_only(const void* a, 
      const void* b){
   EnumerationData& valuea = **((EnumerationData**)a);
   EnumerationData& valueb = **((EnumerationData**)b);
   return valuea.compare(valueb.getValue());
}



//////////////////////////////
//
// Enumeration::data_compare_by_name_only --
//

int Enumeration::data_compare_by_name_only(const void* a, 
      const void* b) {
   EnumerationData& valuea = **((EnumerationData**)a);
   EnumerationData& valueb = **((EnumerationData**)b);
   return valuea.compare(valueb.getName());
}



// md5sum: b7e92325a08cdbc20c58400a7bbba28c Enumeration.cpp [20050403]
