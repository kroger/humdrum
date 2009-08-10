//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May 14 14:21:10 PDT 2001
// Last Modified: Mon May 14 14:21:08 PDT 2001
// Last Modified: Sun Mar 24 12:10:00 PST 2002 (small changes for visual c++)
// Filename:      ...sig/include/sigInfo/NoteList.cpp
// Web Address:   http://sig.sapp.org/include/sigInfo/NoteList.cpp
// Syntax:        C++ 
//
// Description:   Used in chord and key identification in HumdrumFile class.
//

#include "Convert.h"
#include "NoteList.h"

#ifndef OLDCPP
   using namespace std;
#endif



//////////////////////////////
//
// NoteList::NoteList --
//

NoteList::NoteList(void) { 
   // do nothing
}



//////////////////////////////
//
// NoteList::~NoteList --
//

NoteList::~NoteList() { 
   clear();
}



//////////////////////////////
//
// NoteList::clear --
//

void NoteList::clear(void) {
      line      = 0;	 // the line number in the original HumdrumFile
      track     = 0.0;   // the track number in the original HumdrumFile
      dur       = 0.0;   // the tied duration of the note
      level     = 0.0;   // the metric level of the note
      absbeat   = 0.0;   // the absolute beat position of the note
      pitch     = 0;     // the pitch of the note
      realpitch = 0;     // the base 40 value of the note
}



//////////////////////////////
//
// NoteList::getLine --
//
 
const int NoteList::getLine(void) { 
   return line;
}



//////////////////////////////
//
// NoteList::getSpine --
//
 
const int NoteList::getSpine(void) { 
   return (int)spine;
}



//////////////////////////////
//
// NoteList::getToken --
//
 
const int NoteList::getToken(void) { 
   return (int)token;
}



//////////////////////////////
//
// NoteList::getPitch --
//

const int NoteList::getPitch(void) { 
   return pitch;
}



//////////////////////////////
//
// NoteList::getLevel --
//

const double NoteList::getLevel(void) { 
   return level;
}



//////////////////////////////
//
// NoteList::getAbsBeat --
//

const double NoteList::getAbsBeat(void) { 
   return absbeat;
}



//////////////////////////////
//
// NoteList::getFirstDur --
//

const double NoteList::getFirstDur(void) { 
   return firstdur;
}



//////////////////////////////
//
// NoteList::getDur --
//

const double NoteList::getDur(void) { 
   return dur;
}



//////////////////////////////
//
// NoteList::getDuration --
//

const double NoteList::getDuration(void) {
   return dur;
}



//////////////////////////////
//
// NoteList::getTrack --
//

const double NoteList::getTrack(void) { 
   return track;
}



//////////////////////////////
//
// NoteList::setLine --
//
 
void NoteList::setLine(int aValue) { 
   line = aValue;
}



//////////////////////////////
//
// NoteList::setSpine --
//
 
void NoteList::setSpine(int aValue) { 
   spine = (unsigned short)aValue;
}



//////////////////////////////
//
// NoteList::setToken --
//
 
void NoteList::setToken(int aValue) { 
   token = (unsigned short)aValue;
}



//////////////////////////////
//
// NoteList::setPitch --
//

void NoteList::setPitch(int aValue) { 
   pitch = aValue;
   realpitch = aValue;
}



//////////////////////////////
//
// NoteList::setLevel --
//

void NoteList::setLevel(double aValue) { 
   level = (float)aValue;
}



//////////////////////////////
//
// NoteList::setAbsBeat --
//

void NoteList::setAbsBeat(double aValue) { 
   absbeat = (float)aValue;
}



//////////////////////////////
//
// NoteList::setFirstDur --
//

void NoteList::setFirstDur(double aValue) { 
   firstdur = (float)aValue;
}



//////////////////////////////
//
// NoteList::setDur --
//

void NoteList::setDur(double aValue) { 
   dur = (float)aValue;
}



//////////////////////////////
//
// NoteList::setDuration --
//

void NoteList::setDuration(double aValue) {
   dur = (float)aValue;
}



//////////////////////////////
//
// NoteList::setTrack --
//

void NoteList::setTrack(double aValue) { 
   track = (float)aValue;
}



//////////////////////////////
//
// NoteList::pitchBase12 --
//

void NoteList::pitchBase12(void) { 
   pitch = Convert::base40ToMidiNoteNumber(realpitch);
}



//////////////////////////////
//
// NoteList::pitchBase12Class --
//

void NoteList::pitchBase12Class(void) { 
   pitch = Convert::base40ToMidiNoteNumber(realpitch) % 12;
}



//////////////////////////////
//
// NoteList::pitchBase40 --
//

void NoteList::pitchBase40(void) { 
   pitch = realpitch;
}



//////////////////////////////
//
// NoteList::pitchBase40Class --
//

void NoteList::pitchBase40Class(void) { 
   pitch = realpitch % 40;
}


//////////////////////////////////////////////////////////////////////////
// 
// NoteListArray class definitions
//


//////////////////////////////
//
// NoteListArray::NoteListArray --
//

NoteListArray::NoteListArray(void) { 
   setSize(0);
   allowGrowth(1);
}



//////////////////////////////
//
// NoteListArray::~NoteListArray --
//

NoteListArray::~NoteListArray() {
   clear();
}



//////////////////////////////
//
// NoteListArray::clear --
//

void NoteListArray::clear(void) {
   int i, size;
   size = getSize();
   for (i=0; i<size; i++) {
      (*this)[i].clear();
   }
}



//////////////////////////////
//
// NoteListArray::pitchBase12 --
//

void NoteListArray::pitchBase12(void) {
   int i, size;
   size = getSize();
   for (i=0; i<size; i++) {
      (*this)[i].pitchBase12();
   }
}



//////////////////////////////
//
// NoteListArray::pitchBase12Class --
//

void NoteListArray::pitchBase12Class (void) {
   int i, size;
   size = getSize();
   for (i=0; i<size; i++) {
      (*this)[i].pitchBase12Class();
   }
}



//////////////////////////////
//
// NoteListArray::pitchBase40 --
//

void NoteListArray::pitchBase40(void) {
   int i, size;
   size = getSize();
   for (i=0; i<size; i++) {
      (*this)[i].pitchBase40();
   }
}



//////////////////////////////
//
// NoteListArray::pitchBase40Class --
//

void NoteListArray::pitchBase40Class(void) {
   int i, size;
   size = getSize();
   for (i=0; i<size; i++) {
      (*this)[i].pitchBase40Class();
   }
}



//////////////////////////////
//
// operator<< --
//

ostream& operator<<(ostream& out, NoteList& nl) {
   out << "(" << nl.getLine() << ", "
               << nl.getSpine() << ", "
               << nl.getToken() << ") p="
               << nl.getPitch() << ", d="
               << nl.getDur() << ", l="
               << nl.getLevel();
   return out;
}


// md5sum: d6f0e91d2e7799b3fefbf2bba7d21635 NoteList.cpp [20050403]
