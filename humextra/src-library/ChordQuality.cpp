//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jun  7 16:24:06 PDT 1998
// Last Modified: Sun Jun 11 12:39:40 PDT 2000
// Last Modified: Thu May 14 21:53:43 PDT 2009 (modified output display)
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Filename:      ...sig/src/sig/ChordQuality.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/ChordQuality.cpp
// Syntax:        C++ 
//
// Description:   Data class for storing the description of a chord.
//                The ChordQuality class has three properties:
//                   1) Root (the root note of the chord)
//                   2) Inversion (the inversion number of the chord)
//                   3) The chord type (E.g., major, minor, etc.)
//

#include "Convert.h"
#include "ChordQuality.h"
#include "HumdrumEnumerations.h"

#include <string.h>

#ifndef OLDCPP
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #ifdef VISUAL
      #include <strstrea.h>
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif
   

char* ChordQuality::displayString = NULL;


//////////////////////////////
//
// ChordQuality::ChordQuality --
//

ChordQuality::ChordQuality(void) {
   chordType = E_unknown;
   chordInversion = E_unknown;
   chordRoot = E_unknown;
   if (getDisplay() == NULL) {
      setDisplay("t:i:r");
   }
   chordNotes.setSize(0);
}


ChordQuality::ChordQuality(const ChordQuality& aChordQuality) {
   chordType = aChordQuality.getType();;
   chordInversion = aChordQuality.getInversion();;
   chordRoot = aChordQuality.getRoot();
   chordNotes.setSize(0);
}


ChordQuality::ChordQuality(int aType, int anInversion, int aRoot) {
   chordType = aType;
   chordInversion = anInversion;
   chordRoot = aRoot;
   chordNotes.setSize(0);
}



//////////////////////////////
//
// ChordQuality::~ChordQuality --
//

ChordQuality::~ChordQuality() {
   chordType = E_unknown;
   chordInversion = E_unknown;
   chordRoot = E_unknown;
   chordNotes.setSize(0);
}



//////////////////////////////
//
// ChordQuality::getBassNote --
//

int ChordQuality::getBassNote(void) const {
   return Convert::chordQualityToBaseNote(*this);
}



//////////////////////////////
//
// ChordQuality::getDisplay --
//

const char* ChordQuality::getDisplay(void) {
   return displayString;
}



//////////////////////////////
//
// ChordQuality::getInversion --
//

int ChordQuality::getInversion(void) const {
   return chordInversion;
}



//////////////////////////////
//
// ChordQuality::getInversionName --
//

const char* ChordQuality::getInversionName(void) const {
   return Convert::chordInversion.getName(getInversion());
}



//////////////////////////////
//
// ChordQuality::getNotesInChord --
//

SigCollection<int> ChordQuality::getNotesInChord(void) const {
   SigCollection<int>* output;
   output = new SigCollection<int>;
   Convert::chordQualityToNoteSet(*output, *this);
   return *output;
}


void ChordQuality::getNotesInChord(SigCollection<int>& notes) const {
   notes.setSize(0);
   if ((chordType != E_chord_unknown) && (chordNotes.getSize() > 0)) {
      notes.setSize(chordNotes.getSize());
      for (int i=0; i<notes.getSize(); i++) {
         notes[i] = chordNotes[i];
      }
   } else {
      Convert::chordQualityToNoteSet(notes, *this);
   }
}



//////////////////////////////
//
// ChordQuality::getRoot --
//

int ChordQuality::getRoot(void) const {
   return chordRoot;
}



//////////////////////////////
//
// ChordQuality::getRootName --
//

const char* ChordQuality::getRootName(void) const {
   return Convert::kernPitchClass.getName(getRoot());
}



//////////////////////////////
//
// ChordQuality::getType --
//

int ChordQuality::getType(void) const {
   return chordType;
}



//////////////////////////////
//
// ChordQuality::getTypeName --
//

const char* ChordQuality::getTypeName(void) const {
   return Convert::chordType.getName(getType());
}



//////////////////////////////
//
// ChordQuality::makeString --
//     Default value: pcsQ = 0
//

void ChordQuality::makeString(char* space, int pcsQ) {
   SSTREAM temp;
   temp << *this;
   if (pcsQ) {
      printPitchClasses(temp);
   }
   temp << ends;
   strcpy(space, temp.CSTRING);
}



//////////////////////////////
//
// printPitchClasses --  Only works for unknown pitches at the moment.
//

ostream& ChordQuality::printPitchClasses(ostream& out) {
   char buffer[128] = {0};
   if (chordNotes.getSize() == 0) {
      return out;
   }
   out << '[';
   int i;
   for (i=0; i<chordNotes.getSize(); i++) {
      out << Convert::base40ToKern(buffer, chordNotes[i] + 40*3);
   }
   out << ']';
   return out;
}



//////////////////////////////
//
// ChordQuality::print --
//

void ChordQuality::print(const char* aDisplayString, ostream& out) const {
   int i = 0;
   while (aDisplayString[i] != '\0') {
      switch (aDisplayString[i]) {
         case 't':
            out << getTypeName();
	    if (strcmp(getTypeName(), "X") == 0) {
               return; 
	    }
            break;
         case 'i':
            if (getType() == E_chord_note) {
               ;  // do nothing
            } else {
               out << getInversionName();
               if (strcmp(getInversionName(), "X") == 0) {
                  return; 
	       }
            }
            break;
         case 'r':
            out << getRootName();
            if (strcmp(getRootName(), "X") == 0) {
               return; 
	    }
            break;
         default:
            out << aDisplayString[i];
      }
      i++;
   }
}


void ChordQuality::print(ostream& out) const {
   print(getDisplay(), out);
}



//////////////////////////////
//
// ChordQuality::setDisplay --
//

void ChordQuality::setDisplay(const char* aDisplayFormat) {

   if (displayString != NULL) {
      delete [] displayString;
   }
   displayString = new char[strlen(aDisplayFormat) + 1];
   strcpy(displayString, aDisplayFormat);
}



//////////////////////////////
//
// ChordQuality::setInversion --
//

void ChordQuality::setInversion(int anInversion) {
   chordInversion = anInversion;
}


void ChordQuality::setInversion(const char* anInversionName) {
   chordInversion = Convert::chordInversion.getValue(anInversionName);
}



//////////////////////////////
//
// ChordQuality::setRoot --
//

void ChordQuality::setRoot(int aRoot) {
   chordRoot = aRoot;
}


void ChordQuality::setRoot(const char* aRoot) {
   chordRoot = Convert::kernPitchClass.getValue(aRoot);
}



//////////////////////////////
//
// ChordQuality::setQuality --
//

void ChordQuality::setQuality(const char* aQuality) {
   chordType      = Convert::chordQualityToType(aQuality);
   chordInversion = Convert::chordQualityToInversion(aQuality);
   chordRoot      = Convert::chordQualityToRoot(aQuality);
}



//////////////////////////////
//
// ChordQuality::setType --
//

void ChordQuality::setType(int aType) {
   chordType = aType;
}


void ChordQuality::setType(const char* aTypeName) {
   chordType = Convert::chordType.getValue(aTypeName);
}



//////////////////////////////
//
// ChordQuality::setPitchClasses -- store the pitch classes found
//     in the sonority (particularly if the sonority is unknown).
//

void ChordQuality::setPitchClasses(Array<int>& notes) {
   int i;
   chordNotes.setSize(notes.getSize());
   for (i=0; i<chordNotes.getSize(); i++) {
      chordNotes[i] = notes[i];
   }
}

void ChordQuality::setPitchClasses(SigCollection<int>& notes) {
   int i;
   chordNotes.setSize(notes.getSize());
   for (i=0; i<chordNotes.getSize(); i++) {
      chordNotes[i] = notes[i];
   }
}



//////////////////////////////
//
// ChordQuality::operator= --
//

ChordQuality& ChordQuality::operator=(ChordQuality& aQuality) {
   if (&aQuality == this) {
      return *this;
   }
   int i;
   chordNotes.setSize(aQuality.chordNotes.getSize());
   for (i=0; i<chordNotes.getSize(); i++) {
      chordNotes[i] = aQuality.chordNotes[i];
   }
   chordType      = aQuality.chordType;
   chordInversion = aQuality.chordInversion;
   chordRoot      = aQuality.chordRoot;

   return (*this);
}


///////////////////////////////////////////////////////////////////////////
//
// external functions
//


//////////////////////////////
//
// operator<<  --
//

ostream& operator<<(ostream& out, const ChordQuality& aChordQuality) {
   aChordQuality.print(out);
   return out;
}



// md5sum: b6feb8f4423ff92d21261a1b286f0a1d ChordQuality.cpp [20001204]
