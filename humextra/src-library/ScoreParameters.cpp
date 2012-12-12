//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb 10 19:42:45 PST 2002
// Last Modified: Tue Mar 24 18:40:22 PST 2009
// Last Modified: Fri Aug 24 08:33:10 PDT 2012 Renovation
// Filename:      ...sig/src/sigInfo/ScoreParameters.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/ScoreParameters.cpp
// Syntax:        C++ 
//
// Description:   Base parameter class for SCORE musical objects.  This
//                data structure consists of an array of fixed doubleing-point 
//                numbers which are the standard SCORE parameters for an object
//                as well as a list of "Extended" Score numberic parameters 
//                which are accessed as a key/value pair.
//

#include "ScoreParameters.h"

#include <string.h>
#include <math.h>

#ifndef OLDCPP
   using namespace std;
#endif


//////////////////////////////////////////////////////////////////////////
//
// ScoreNamedParameter fuctions
//

//////////////////////////////
//
// ScoreNamedParameter::ScoreNamedParameter -- constructor.
//

ScoreNamedParameter::ScoreNamedParameter(void) {
   name.setSize(0);
   value = 0.0;
}


ScoreNamedParameter::ScoreNamedParameter(ScoreNamedParameter& aKeyParam) {
   name = aKeyParam.name;
   value = aKeyParam.value;
}



//////////////////////////////
//
// ScoreNamedParameter::~ScoreNamedParameter -- deconstructor.
//

ScoreNamedParameter::~ScoreNamedParameter() {
   clear();
}



//////////////////////////////
//
// ScoreNamedParameter::clear -- Remove the name and set the value to 0.0.
//

void ScoreNamedParameter::clear(void) {
   name.setSize(0);
   value = 0.0;
}



//////////////////////////////
//
// ScoreNamedParameter::getName -- Get the key parameter's name, or "" if
//     no name.
//

const char* ScoreNamedParameter::getName(void) {
   if (name.getSize() > 0) {
      return name.getBase();
   } else {
      return "";
   }
}



//////////////////////////////
//
// ScoreNamedParameter::getValue -- Get the key parameter's value.
//

double ScoreNamedParameter::getValue(void) {
   return value;
}



//////////////////////////////
//
// ScoreNamedParameter::setNameAndValue -- Set the name and value of
//    the key parameter.
//

void ScoreNamedParameter::setNameAndValue(const char* string, double aValue) {
   int len = strlen(string);
   name.setSize(len+1);
   strcpy(name.getBase(), string);
   value = aValue;
}



//////////////////////////////
//
// ScoreNamedParameter::setName -- set the name of the key parameter.
//

void ScoreNamedParameter::setName(const char* aName) {
   int len = strlen(aName);
   name.setSize(len+1);
   strcpy(name.getBase(), aName);
}



//////////////////////////////
//
// ScoreNamedParameter::setValue -- Set the name of the key parameter.
//

void ScoreNamedParameter::setValue(double aValue) {
   value = aValue;
}



//////////////////////////////
//
// ScoreNamedParameter::isValid -- Returns true if the length of name is
//    not zero.
//

int ScoreNamedParameter::isValid(void) {
   return name.getSize() == 0 ? 0 : 1;
}


//////////////////////////////
//
// ScoreNamedParameter::isInvalid --
//

int ScoreNamedParameter::isInvalid(void) {
   return name.getSize() == 0 ? 1 : 0;
}



//////////////////////////////
//
// ScoreNamedParameter::isNamed --
//

int ScoreNamedParameter::isNamed(const char* aName) {
   if (strcmp(aName, getName()) == 0) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// ScoreNamedParameter::operator= --
//

ScoreNamedParameter& ScoreNamedParameter::operator=(ScoreNamedParameter& 
      aKeyParam) {
   if (&aKeyParam == this) {
      return *this;
   }
   name = aKeyParam.name;
   value = aKeyParam.value;
   return *this;
}



//////////////////////////////////////////////////////////////////////////


///////////////////////////////
//
// ScoreParameters::ScoreParameters -- constructor
//

ScoreParameters::ScoreParameters(void) { 
   fixedParameters.setSize(0);
   fixedParameters.setGrowth(32);
   keyParameters.setSize(0);
   keyParameters.setGrowth(32);
   text.setSize(0);
   textFont = 0;
}


ScoreParameters::ScoreParameters(ScoreParameters& item) { 
   fixedParameters = item.fixedParameters;
   keyParameters   = item.keyParameters;
   text            = item.text;
   textFont        = item.textFont;
}



///////////////////////////////
//
// ScoreParameters::~ScoreParameters -- destructor
//

ScoreParameters::~ScoreParameters() {
   clear();
}



///////////////////////////////
//
// ScoreParameters::clear --
//

void ScoreParameters::clear(void) {
   fixedParameters.setSize(0);
   text.setSize(0);
   clearKeyParameters();
}



///////////////////////////////
//
// ScoreParameters::getText -- Return the fixed text parameter value.
//    Only valid if P1=16; otherwise should return an empty string.
//

const char* ScoreParameters::getText(void) {
   if (text.getSize() > 0) {
      return text.getBase();
   } else {
      return "";
   }
}


const char* ScoreParameters::getTextData(void) {
   return getText();
}


///////////////////////////////
//
// ScoreParameters::setText -- Set the fixed text parameter value.
//    Only valid if P1=16; otherwise should be an empty string.
//

void ScoreParameters::setText(const char* aString) {
   int len = strlen(aString);
   text.setSize(len+1);
   strcpy(text.getBase(), aString);
}


void ScoreParameters::setTextData(const char* aString) {
   setText(aString);
}


///////////////////////////////
//
// ScoreParameters::getValue -- Get a fixed parameter value starting offset 
//       at 0 for parameter 1 (object type).  Use the P1, P2, P3, P4
//       enumerations as the index number.
//

double ScoreParameters::getValue(int index) { 
   double output = 0.0;
   if (index < fixedParameters.getSize()) {
      output = fixedParameters[index];
   }
   return output;
}


double ScoreParameters::getValue(const char* keyName) { 
   for (int i=0; i<keyParameters.getSize(); i++) {
      if (strcmp(keyName, keyParameters[i].getName()) == 0) {
         return keyParameters[i].getValue();
      }
   }
   // didn't find key parameter so return default value of 0.0:
   return 0.0;
}



//////////////////////////////
//
// ScoreParameters::getValue -- Return a specific digit of a parameter.
//    0 = 1's digit
//    1 = 10's digit
//    2 = 100's digit
//    3 = 1000's digit
//    -1 = 0.1's digit
//    -2 = 0.01's digit
//    -3 = 0.001's digit
//    You should not look at other digits as they are undefined in SCORE
//    (which uses doubles).
//

int ScoreParameters::getValue(int index, int digit) {
   double tempval = getValue(index);
   if (tempval < 0.0) {
      tempval = -tempval;
   }
   double dshift = pow(10.0, digit);
   return int(tempval/dshift) % 10;
}


int ScoreParameters::getPValue(int number, int digit) {
   return getValue(number-1, digit);
}



///////////////////////////////
//
// ScoreParameters::getPValue -- Get a fixed parameter value starting 
//      offset at 1 for parameter 1 (object type).
//

double ScoreParameters::getPValue(int number) { 
   return getValue(number-1);
}


double ScoreParameters::getPValue(const char* keyName) { 
   return getValue(keyName);
}



//////////////////////////////
//
// ScoreParameters::getKeyName -- returns the name of the nth nameded parameter.
//     It is invalid to get the name of a non-existing key parameter.
//

const char* ScoreParameters::getKeyName(int index) {
   return keyParameters[index].getName();
}



//////////////////////////////
//
// ScoreParameters::getKeyValue -- returns the value of the nth nameded parameter.
//     It is invalid to get the value of a non-existing key parameter.
//

double ScoreParameters::getKeyValue(int index) {
   return keyParameters[index].getValue();
}



///////////////////////////////
//
// ScoreParameters::setValue -- Set a value for a fixed/named parameter,
//     with fixed paramter indices starting offset at 0.
//

void ScoreParameters::setValue(int index, double value) { 
   if (index < fixedParameters.getSize()) {
      // seting a value for an existing fixed parameter.
      fixedParameters[index] = value;
   } else {
      // grow the fixed array size, and fill in 0.0 for unassigned slots.
      int oldsize = fixedParameters.getSize();
      fixedParameters.setSize(index+1);
      for (int i=oldsize; i<index; i++) {
         fixedParameters[i] = 0.0;
      }
      fixedParameters[index] = value;
   }
}


void ScoreParameters::setValue(const char* keyName, double keyValue) {
   int i;
   int invalid = -1;
   for (i=0; i<keyParameters.getSize(); i++) {
      if ((invalid < 0) && (keyParameters[i].isInvalid())) {
         invalid = i;
      }
      if (keyParameters[i].isNamed(keyName)) {
         keyParameters[i].setValue(keyValue);
         return;
      }
   }

   // Named parameter was not found; add it to the list.
   // First see if the was an invalid slot while searching the list:
   if (invalid >= 0) {
      keyParameters[invalid].setNameAndValue(keyName, keyValue);
      return;
   }

   // There was no invalid slot, so add a new one to the end of the list
   keyParameters.increase(1);
   keyParameters.last().setNameAndValue(keyName, keyValue);
}



///////////////////////////////
//
// ScoreParameters::setPValue -- Set a value for a fixed/named parameter,
//     with fixed paramter indices starting offset at 1.
//

void  ScoreParameters::setPValue(int number, double value) { 
   setValue(number-1, value);
}


void  ScoreParameters::setPValue(const char* keyName, double keyValue) { 
   setValue(keyName, keyValue);
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
   keyParameters   = anItem.keyParameters;
   textFont        = anItem.textFont;
   text            = anItem.text;

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

void ScoreParameters::setFixedSize(int aSize) {
   int oldsize = fixedParameters.getSize();
   fixedParameters.setSize(aSize);
   if (aSize > oldsize) {
      for (int i=oldsize; i<aSize; i++) {
         fixedParameters[i] = 0.0;
      }
   }
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
// ScoreParameters::print -- print both Fixed and Key parameter lists.
//

ostream& ScoreParameters::print(ostream& out) {
   printFixedParameters(out);
   out << endl;
   printKeyParameters(out);
   return out;
}



//////////////////////////////
//
// ScoreParameters::printFixedParameters -- print only Fixed parameter list.
//      Does not add a newline at the end of the list, you have to add
//      afterwards.
//

ostream& ScoreParameters::printFixedParameters(ostream& out) {
   for (int i=0; i<fixedParameters.getSize(); i++) {
      cout << fixedParameters[i] << " ";
   }
   return out;
}



//////////////////////////////
//
// ScoreParameters::printKeyParameters -- print only Key parameter list.
//

ostream& ScoreParameters::printKeyParameters(ostream& out) {
   for (int i=0; i<keyParameters.getSize(); i++) {
      out << "@" << keyParameters[i].getName() 
          << ":\t" << keyParameters[i].getValue() << endl;
   }
   return out;
}



//////////////////////////////
//
// ScoreParameters::setAllocSize -- set the allocation size of the 
//   fixed parameter list if the current allocation size is smaller.
//

void ScoreParameters::setAllocSize(int aSize) {
   if (aSize > fixedParameters.getAllocSize()) {
      int oldsize = fixedParameters.getSize();
      fixedParameters.setSize(aSize);
      fixedParameters.setSize(oldsize);
   }
}



//////////////////////////////
//
// ScoreParameters::clearKeyParameters --
//

void ScoreParameters::clearKeyParameters(void) {
   keyParameters.setSize(0);
}



//////////////////////////////
//
// ScoreParameters::hasKey -- Return true if there is a key parameter
//    by that name.  The return value is the index position of the key parameter
//    in the list, plus one.
//

int ScoreParameters::hasKey(const char* aName) {
   for (int i=0; i<keyParameters.getSize(); i++) {
      if (keyParameters[i].isNamed(aName)) {
         return i+1;
      }
   }
   return 0;
}


int ScoreParameters::hasValue(const char* aName) {
   return hasKey(aName);
}



//////////////////////////////
//
// ScoreParameters::shrink -- Reduce the size of fixedParameters so that
//    there are no 0.0 values at the end of the array.
//

void ScoreParameters::shrink(void) {
   int i;
   for (i=getFixedSize()-1; i>0; i--) {
      if (getValue(i) != 0.0) {
         break;
      } else {
         setFixedSize(i);
      }
   }
}



//////////////////////////////
//
// ScoreParameters::hasExtraParameters --  Returns true if there are any 
//     extra (analysis) parameters store in the keyParameters database.
//

int ScoreParameters::hasExtraParameters(void) {
   for (int i =0; i<keyParameters.getSize(); i++) {
      if (keyParameters[i].isValid()) {
         return 1;
      }
   }
   return 0;
}


int ScoreParameters::hasKeyParameters(void) {
   return hasExtraParameters();
}



//////////////////////////////////////////////////////////////////////////
//
// Static Functions
// 

//////////////////////////////
//
// ScoreParameters::roundFractionDigits -- Parameter data in SCORE binary 
//    files are stored as floats.  This floats are not reliable past the third
//    decimal place in the number, so typically this fuction will be called
//    after reading a float from a binary file, using digits=3.
//

double ScoreParameters::roundFractionDigits(double number, int digits) {
   double dshift = pow(10.0, digits);
   if (number < 0.0) {
      return (int(number * dshift - 0.5))/dshift;
   } else {
      return (int(number * dshift + 0.5))/dshift;
   }
}


// md5sum: 28ccec826d946c540a9937f0129ce915 ScoreParameters.cpp [20050403]
