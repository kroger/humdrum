//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb 10 19:42:45 PST 2002
// Last Modified: Tue Mar 24 18:40:22 PST 2009
// Filename:      ...sig/src/sigInfo/ScoreParameters.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/ScoreParameters.cpp
// Syntax:        C++ 
//
// Description:   Base class for SCORE musical objects.
//

#include "ScoreParameters.h"

#include <string.h>

#ifndef OLDCPP
   using namespace std;
#endif



///////////////////////////////
//
// ScoreParameters::ScoreParameters --
//

ScoreParameters::ScoreParameters(void) { 
   fixedParameters.setSize(0);
   keyParameters.setSize(0);
}


ScoreParameters::ScoreParameters(ScoreParameters& item) { 
   fixedParameters = item.fixedParameters;
   clearKeyParams();
   keyParameters.setSize(item.keyParameters.getSize());
   keyParameters.setSize(0);
   int i;
   for (i=0; i<item.keyParameters.getSize(); i++) {
      appendKeyParameter(item.keyParameters[i].getBase());
   }
}



///////////////////////////////
//
// ScoreParameters::clear --
//

void ScoreParameters::clear(void) {
   fixedParameters.setSize(0);
   clearKeyParams();
}



///////////////////////////////
//
// ScoreParameters::getValue -- starting offset at 1.
//

float ScoreParameters::getValue(int index) { 
   float output = 0.0;
   if (index < fixedParameters.getSize()) {
      output = fixedParameters[index];
   }
   return output;
}



///////////////////////////////
//
// ScoreParameters::getPValue -- starting offset at 1.
//

float ScoreParameters::getPValue(int index) { 
   float output = 0.0;
   if (index-1 < fixedParameters.getSize()) {
      output = fixedParameters[index-1];
   }
   return output;
}



///////////////////////////////
//
// ScoreParameters::setValue -- starting offset at 0.
//

void  ScoreParameters::setValue(int index, float value) { 
   if (index < fixedParameters.getSize()) {
      fixedParameters[index] = value;
   } else {
      int oldsize = fixedParameters.getSize();
      fixedParameters.setSize(index+1);
      for (int i=oldsize; i<index+1; i++) {
         fixedParameters[i] = 0.0;
      }
      fixedParameters[index] = value;
   }
}



///////////////////////////////
//
// ScoreParameters::setPValue -- starting offset at 1.
//

void  ScoreParameters::setPValue(int index, float value) { 
   if (index-1 < fixedParameters.getSize()) {
      fixedParameters[index-1] = value;
   } else {
      int oldsize = fixedParameters.getSize();
      fixedParameters.setSize(index);
      for (int i=oldsize; i<index; i++) {
         fixedParameters[i] = 0.0;
      }
      fixedParameters[index-1] = value;
   }
}



///////////////////////////////
//
// ScoreParameters::operator= --
//

ScoreParameters& ScoreParameters::operator=(ScoreParameters& anItem) {
   if (&anItem == this) {
      return *this;
   }
   fixedParameters = anItem.fixedParameters;
   keyParameters = anItem.keyParameters;

   return *this;
}



//////////////////////////////
//
// ScoreParameters::getFixedSize --
//

int ScoreParameters::getFixedSize(void) {
   return fixedParameters.getSize();
}



//////////////////////////////
//
// ScoreParameters::setFixedSize --
//

void ScoreParameters::setFixedSize(int value) {
   fixedParameters.setSize(value);
}



//////////////////////////////
//
// ScoreParameters::getKeySize --
//

int ScoreParameters::getKeySize(void) {
   return keyParameters.getSize();
}



//////////////////////////////
//
// ScoreParameters::print --
//

void ScoreParameters::print(void) {
   int i;
   for (i=0; i<fixedParameters.getSize(); i++) {
      cout << fixedParameters[i] << " ";
   }
   cout << endl;
   for (i=0; i<keyParameters.getSize(); i++) {
      cout << keyParameters[i].getBase() << endl;
   }
}



//////////////////////////////
//
// ScoreParameters::setAllocSize -- set the allocation size of the 
//   class if the current allocation size is smaller.
//

void ScoreParameters::setAllocSize(int aSize) {
   if (aSize > fixedParameters.getAllocSize()) {
      int oldsize = fixedParameters.getSize();
      fixedParameters.setSize(aSize);
      fixedParameters.setSize(oldsize);
   }
}



//////////////////////////////////////////////////////////////////////////
//
// Private Functions
// 


//////////////////////////////
//
// ScoreParameters::clearKeyParams --
//

void ScoreParameters::clearKeyParams(void) {
   keyParameters.setSize(0);
}


////////////////////////////////
//
// ScoreParameters::appendKeyParameter --
//

void ScoreParameters::appendKeyParameter(const char* string) {
    int length = strlen(string);
    int index = keyParameters.getSize();
    keyParameters.setSize(keyParameters.getSize()+1);
    keyParameters[index].setSize(length+1);
    strcpy(keyParameters[index].getBase(), string);
}




// md5sum: 28ccec826d946c540a9937f0129ce915 ScoreParameters.cpp [20050403]
