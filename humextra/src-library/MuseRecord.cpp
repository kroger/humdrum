//
// Copyright 1998,2010 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 22:41:24 PDT 1998
// Last Modified: Sat Dec 25 11:50:24 PST 2010 added more functions
// Last Modified: Wed Oct 12 23:06:08 PDT 2011 fixed prob in setNoteheadShape
// Filename:      ...sig/src/sigInfo/MuseRecord.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/MuseRecord.cpp
// Syntax:        C++ 
// 
// Description:   A class that stores one line of data for a Musedata file.
//
// To do: check on gracenotes/cuenotes with chord notes.
//

#include "Convert.h"
#include "MuseRecord.h"
#include "PerlRegularExpression.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef OLDCPP
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #ifdef VISUAL
      #include <strstrea.h>     /* for Windows 95 */
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif
   


//////////////////////////////
//
// MuseRecord::MuseRecord --
//

MuseRecord::MuseRecord(void) : MuseRecordBasic() { }
MuseRecord::MuseRecord(char* aLine) : MuseRecordBasic(aLine) { }
MuseRecord::MuseRecord(MuseRecord& aRecord) : MuseRecordBasic(aRecord) { }



//////////////////////////////
//
// MuseRecord::~MuseRecord --
//

MuseRecord::~MuseRecord() {
   // do nothing
}



//////////////////////////////////////////////////////////////////////////
//
// functions which work with note records
//


//////////////////////////////
//
// MuseRecord::getNoteField -- returns the string containing the pitch,
//	accidental and octave characters.
//

char* MuseRecord::getNoteField(char* output) {

   switch (getType()) {
      case E_muserec_note_regular:
         extract(output, 1, 4);
         break;
      case E_muserec_note_chord:
      case E_muserec_note_cue:
      case E_muserec_note_grace:
         extract(output, 2, 5);
         break;
      default:
         cerr << "Error: cannot use getNoteField function on line: " 
              << getLine() << endl;
         exit(1);
   }

   return output;
}




//////////////////////////////
//
// MuseRecord::getOctave -- returns the first numeric character
//	in the note field of a MuseData note record
//

int MuseRecord::getOctave(void) {

   char recordInfo[16];
   getNoteField(recordInfo);
   int index = 0;
   while (recordInfo[index] != '\0' && !isdigit(recordInfo[index])) {
      index++;
   }

   if (recordInfo[index] == '\0') {
      cerr << "Error: no octave specification in note field: " << recordInfo
           << endl;
      exit(1);
   } 

   return recordInfo[index] - '0';
}


char* MuseRecord::getOctave(char* output) {

   char recordInfo[16];
   getNoteField(recordInfo);
   int index = 0;
   while (recordInfo[index] != '\0' && !isdigit(recordInfo[index])) {
      index++;
   }

   if (recordInfo[index] == '\0') {
      cerr << "Error: no octave specification in note field: " << recordInfo
           << endl;
      exit(1);
   } 

   output[0] = recordInfo[index];
   output[1] = '\0';

   return output;
}
   


//////////////////////////////
//
// MuseRecord::getPitch -- int version returns the base40 representation
//

int MuseRecord::getPitch(void) {
   char recordInfo[16];
   getNoteField(recordInfo);
   return Convert::museToBase40(recordInfo);
}


char* MuseRecord::getPitch(char* output) {
   getNoteField(output);
   int length = strlen(output);
   int index = length-1;
   while (index >= 0 && output[index] == ' ') {
      output[index] = '\0';
      index--;
   }

   return output;
}
   


//////////////////////////////
//
// MuseRecord::getPitchClass -- returns the pitch without the octave information
//

int MuseRecord::getPitchClass(void) {
   return getPitch() % 40;
}


char* MuseRecord::getPitchClass(char* output) {
   getNoteField(output);
   int index = 0;
   while (output[index] != '\0' &&  !isdigit(output[index])) {
      index++;
   }
   output[index] = '\0';

   return output;
}
   


//////////////////////////////
//
// MuseRecord::getAccidental -- int version return -2 for double flat,
//	-1 for flat, 0 for natural, +1 for sharp, +2 for double sharp
//

int MuseRecord::getAccidental(void) {
   char recordInfo[16];
   getNoteField(recordInfo);
   
   int output = 0;
   int index = 0;
   while (recordInfo[index] != '\0' && index < 16) {
      if (recordInfo[index] == 'f') {
         output--;
      } else if (recordInfo[index] == '#') {
         output++;
      }
      index++;
   }
   return output;
}


char* MuseRecord::getAccidental(char* output) {
   int type = getAccidental();
   
   switch (type) {
      case -2: strcpy(output, "ff"); break;
      case -1: strcpy(output,  "f"); break; 
      case  0: strcpy(output,   ""); break;
      case  1: strcpy(output,  "#"); break;
      case  2: strcpy(output, "##"); break;
      default:
         getNoteField(output);
         cerr << "Error: unknown type of accidental: " << output << endl;
         exit(1);
   }

   return output;
}



//////////////////////////////
//
// MuseRecord::getBase40 -- return the base40 pitch value of the data
// line.  Middle C set to 40 * 4 + 2;  Returns -100 for non-pitched items.
// (might have to update for note_cur_chord and note_grace_chord which
// do not exist yet.
//

int MuseRecord::getBase40(void) {
   switch (getType()) {
      case E_muserec_note_regular:
      case E_muserec_note_chord:
      case E_muserec_note_cue:
      case E_muserec_note_grace:
         break;
      default:
         return -100;
   }
   return getPitch();
}



//////////////////////////////
//
// MuseRecord::setStemDown --
//

void MuseRecord::setStemDown(void) {
   getColumn(23) = 'd';
}



//////////////////////////////
//
// MuseRecord::setStemUp --
//

void MuseRecord::setStemUp(void) {
   getColumn(23) = 'u';
}



//////////////////////////////
//
// MuseRecord::setPitch -- input is a base40 value which gets converted
// to a diatonic pitch name.
//    Default value: chordnote = 0
//    Default value: gracenote = 0
//

void MuseRecord::setPitch(int base40, int chordnote, int gracenote) {
   char diatonic[2] = {0};
   switch (Convert::base40ToDiatonic(base40) % 7) {
      case 0:  diatonic[0] = 'C'; break;
      case 1:  diatonic[0] = 'D'; break;
      case 2:  diatonic[0] = 'E'; break;
      case 3:  diatonic[0] = 'F'; break;
      case 4:  diatonic[0] = 'G'; break;
      case 5:  diatonic[0] = 'A'; break;
      case 6:  diatonic[0] = 'B'; break;
      default: diatonic[0] = 'X'; 
   }

   char octave[2] = {0};
   octave[0]   = '0' + base40 / 40; 

   char accidental[4] = {0};
   int acc = Convert::base40ToAccidental(base40);
   switch (acc) {
      case -2:   strcpy(accidental, "ff"); break;
      case -1:   strcpy(accidental, "f");  break;
      case +1:   strcpy(accidental, "#");  break;
      case +2:   strcpy(accidental, "##"); break;
   }
   char pitchname[8] = {0};
   strcpy(pitchname, diatonic);
   strcat(pitchname, accidental);
   strcat(pitchname, octave);

   if (chordnote) {
      if (gracenote) {
         setGraceChordPitch(pitchname);
      } else {
         setChordPitch(pitchname);
      }
   } else {
      setPitch(pitchname);
   }
}


void MuseRecord::setChordPitch(const char* pitchname) {
   getColumn(1) = ' ';
   setPitchAtIndex(1, pitchname);
}

void MuseRecord::setGracePitch(const char* pitchname) {
   getColumn(1) = 'g';
   setPitchAtIndex(1, pitchname);
}

void MuseRecord::setGraceChordPitch(const char* pitchname) {
   getColumn(1) = 'g';
   getColumn(2) = ' ';
   setPitchAtIndex(2, pitchname);
}

void MuseRecord::setCuePitch(const char* pitchname) {
   getColumn(1) = 'c';
   setPitchAtIndex(1, pitchname);
}


void MuseRecord::setPitch(const char* pitchname) {
   int start = 0;
   // if the record is already set to a grace note or a cue note,
   // then place pitch information starting at column 2 (index 1).
   if ((getColumn(1) == 'g') || (getColumn(1) == 'c')) {
      start = 1;
   }
   setPitchAtIndex(start, pitchname);
}


void MuseRecord::setPitchAtIndex(int index, const char* pitchname) {
   int len = strlen(pitchname);
   if ((len > 4) && (strcmp(pitchname, "irest")!= 0)) {
      cerr << "Error in MuseRecord::setPitchAtIndex: " << pitchname << endl;
      exit(1);
   }
   insertString(index+1, pitchname);

   // clear any text fields not used by current pitch data
   int i;
   for (i=4-len-1; i>=0; i--) {
      (*this)[index + len + i] = ' ';
   }
}



//////////////////////////////
//
// MuseRecord::getTickDurationField -- returns the string containing the 
//      duration, and tie information.
//

char* MuseRecord::getTickDurationField(char* output) {
   switch (getType()) {
      case E_muserec_figured_harmony:
      case E_muserec_note_regular:
      case E_muserec_note_chord:
      case E_muserec_rest:
      case E_muserec_backward:
      case E_muserec_forward:
         extract(output, 6, 9);
         break;
      // these record types do not have duration, per se:
      case E_muserec_note_cue:
      case E_muserec_note_grace:
      default:
         output[0] = '\0';
         // cerr << "Error: cannot use getTickDurationField function on line: " 
         //      << getLine() << endl;
         // exit(1);
   }

   return output;
}



//////////////////////////////
//
// MuseRecord::getTickDuration -- returns the string containing the duration,
//

char* MuseRecord::getTickDuration(char* output) {
   getTickDurationField(output);
   int length = strlen(output);
   int i = length - 1;
   while (i>0 && (output[i] == '-' || output[i] == ' ')) {
      output[i] = '\0';
      i--;
      length--;
   }

   int start = 0;
   while (output[start] == ' ') {
      start++;
   }

   if (start != 0) {
      for (i=0; i<length-start; i++) {
         output[i] = output[start+i];
      }
   }
   output[length-start] = '\0';

   return output;
}



//////////////////////////////
//
// MuseRecord::getTickDurationField -- return the tick value found
//    in columns 6-8 in some data type, returning 0 if the record
//    type does not have a duration field.
//

int MuseRecord::getTickDurationField(void) {
   char recordInfo[16];
   getTickDuration(recordInfo);
   return atoi(recordInfo);
}



//////////////////////////////
//
// MuseRecord::getLineTickDuration -- returns the logical duration of the
//      data line.  Supresses the duration field of secondary chord notes.
//

int MuseRecord::getLineTickDuration(void) {
   if (getType() == E_muserec_note_chord) {
      return 0;
   }

   char recordInfo[16];
   getTickDuration(recordInfo);
   int value = atoi(recordInfo);
   if (getType() == E_muserec_backspace) {
      return -value;
   }
   return value;
}

int MuseRecord::getTicks(void) {
   return getLineTickDuration();
}


//////////////////////////////
//
// MuseRecord::getNoteTickDuration -- Similar to getLineTickDuration,
//    but do not suppress the duration of secondary chord-tones.
//

int MuseRecord::getNoteTickDuration(void) {
   char recordInfo[16];
   getTickDuration(recordInfo);
   int value = atoi(recordInfo);
   if (getType() == E_muserec_backspace) {
      return -value;
   }
   return value;
}



//////////////////////////////
//
// MuseRecord::setDots -- Only one or two dots allowed
//

void MuseRecord::setDots(int value) {
   switch (value) {
      case 0: getColumn(18) = ' ';   break;
      case 1: getColumn(18) = '.';   break;
      case 2: getColumn(18) = ':';   break;
      case 3: getColumn(18) = ';';   break;
      case 4: getColumn(18) = '!';   break;
      default: cerr << "Error in MuseRecord::setDots : " << value << endl;
               exit(1);
   }
}



//////////////////////////////
//
// MuseRecord::setNoteheadShape -- Duration with augmentation dot component
//      removed.  Duration of 1 is quarter note.
//

void MuseRecord::setNoteheadShape(RationalNumber& duration) {
   RationalNumber note8th(1,2);
   RationalNumber note16th(1,4);
   RationalNumber note32th(1,8);
   RationalNumber note64th(1,16);
   RationalNumber note128th(1,32);
   RationalNumber note256th(1,64);

   if (duration > 16) {                // maxima
      setNoteheadMaxima();
   } else if (duration > 8) {          // long
      setNoteheadLong();
   } else if (duration > 4) {          // breve
      if (roundBreve) {
         setNoteheadBreveRound();
      } else {
         setNoteheadBreve();
      }
   } else if (duration > 2) {           // whole note
      setNoteheadWhole();
   } else if (duration > 1) {           // half note
      setNoteheadHalf();
   } else if (duration > note8th) {     // quarter note
      setNoteheadQuarter();
   } else if (duration > note16th) {   // eighth note
      setNotehead8th();
   } else if (duration > note32th) {   // 16th note
      setNotehead16th();
   } else if (duration > note64th) {   // 32nd note
      setNotehead32nd();
   } else if (duration > note128th) {  // 64th note
      setNotehead64th();
   } else if (duration > note256th) {  // 128th note
      setNotehead128th();
   } else if (duration == note256th) {  // 256th note
      // not allowing tuplets on the 256th note level.
      setNotehead256th();
   } else {
      cerr << "Error in duration: " << duration << endl;
      exit(1);
   }
}



//////////////////////////////
//
// MuseRecord::setNoteheadShape -- Duration with augmentation dot component
//      removed.  Duration of 1 is quarter note.
//

void MuseRecord::setNoteheadShapeMensural(RationalNumber& duration) {
   RationalNumber note8th(1,2);
   RationalNumber note16th(1,4);
   RationalNumber note32th(1,8);
   RationalNumber note64th(1,16);
   RationalNumber note128th(1,32);
   RationalNumber note256th(1,64);

   if (duration > 16) {                 // maxima
      setNoteheadMaxima();
   } else if (duration > 8) {           // long
      setNoteheadLong();
   } else if (duration > 4) {           // breve
      setNoteheadBreve();
   } else if (duration > 2) {           // whole note
      setNoteheadWholeMensural();
   } else if (duration > 1) {           // half note
      setNoteheadHalfMensural();
   } else if (duration > note8th) {     // quarter note
      setNoteheadQuarterMensural();
   } else if (duration > note16th) {    // eighth note
      setNotehead8thMensural();
   } else if (duration > note32th) {    // 16th note
      setNotehead16thMensural();
   } else if (duration > note64th) {    // 32nd note
      setNotehead32ndMensural();
   } else if (duration > note128th) {   // 64th note
      setNotehead64thMensural();
   } else if (duration > note256th) {   // 128th note
      setNotehead128thMensural();
   } else if (duration >= note256th) {  // 256th note
      // don't allow tuplets on 256th note level.
      setNotehead256thMensural();
   } else {
      cerr << "Error in duration: " << duration << endl;
      exit(1);
   }
}

void MuseRecord::setNoteheadMaxima(void) {
   if ((*this)[0] == 'c' || ((*this)[0] == 'g')) {
      cerr << "Error: cue/grace notes cannot be maximas in setNoteheadLong" 
           << endl;
      exit(1);
   } else {
      getColumn(17) = 'M';
   }
}

void MuseRecord::setNoteheadLong(void) {
   if ((*this)[0] == 'c' || ((*this)[0] == 'g')) {
      cerr << "Error: cue/grace notes cannot be longs in setNoteheadLong" 
           << endl;
      exit(1);
   } else {
      getColumn(17) = 'L';
   }
}

void MuseRecord::setNoteheadBreve(void) {
   setNoteheadBreveSquare();
}

void MuseRecord::setNoteheadBreveSquare(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = 'A';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = 'A';
   } else {                        // normal note
      getColumn(17) = 'B';
   }
}

void MuseRecord::setNoteheadBreveRound(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = 'A';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = 'A';
   } else {                        // normal note
      getColumn(17) = 'b';
   }
}

void MuseRecord::setNoteheadBreveMensural(void) {
   setNoteheadBreveSquare();
}

void MuseRecord::setNoteheadWhole(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '9';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '9';
   } else {                        // normal note
      getColumn(17) = 'w';
   }
}

void MuseRecord::setNoteheadWholeMensural(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '9';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '9';
   } else {                        // normal note
      getColumn(17) = 'W';
   }
}

void MuseRecord::setNoteheadHalf(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '8';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '8';
   } else {                        // normal note
      getColumn(17) = 'h';
   }
}

void MuseRecord::setNoteheadHalfMensural(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '8';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '8';
   } else {                        // normal note
      getColumn(17) = 'H';
   }
}

void MuseRecord::setNoteheadQuarter(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '7';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '7';
   } else {                        // normal note
      getColumn(17) = 'q';
   }
}

void MuseRecord::setNoteheadQuarterMensural(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '7';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '7';
   } else {                        // normal note
      getColumn(17) = 'Q';
   }
}

void MuseRecord::setNotehead8th(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '6';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '6';
   } else {                        // normal note
      getColumn(17) = 'e';
   }
}

void MuseRecord::setNotehead8thMensural(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '6';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '6';
   } else {                        // normal note
      getColumn(17) = 'E';
   }
}

void MuseRecord::setNotehead16th(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '5';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '5';
   } else {                        // normal note
      getColumn(17) = 's';
   }
}

void MuseRecord::setNotehead16thMensural(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '5';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '5';
   } else {                        // normal note
      getColumn(17) = 'S';
   }
}

void MuseRecord::setNotehead32nd(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '4';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '4';
   } else {                        // normal note
      getColumn(17) = 't';
   }
}

void MuseRecord::setNotehead32ndMensural(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '4';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '4';
   } else {                        // normal note
      getColumn(17) = 'T';
   }
}

void MuseRecord::setNotehead64th(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '3';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '3';
   } else {                        // normal note
      getColumn(17) = 'x';
   }
}

void MuseRecord::setNotehead64thMensural(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '3';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '3';
   } else {                        // normal note
      getColumn(17) = 'X';
   }
}

void MuseRecord::setNotehead128th(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '2';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '2';
   } else {                        // normal note
      getColumn(17) = 'y';
   }
}

void MuseRecord::setNotehead128thMensural(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '2';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '2';
   } else {                        // normal note
      getColumn(17) = 'Y';
   }
}

void MuseRecord::setNotehead256th(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '1';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '1';
   } else {                        // normal note
      getColumn(17) = 'z';
   }
}

void MuseRecord::setNotehead256thMensural(void) {
   if ((*this)[0] == 'g') {        // grace note
      getColumn(8) = '1';  
   } else if ((*this)[0] == 'c') { // cue-sized note (with duration)
      getColumn(17) = '1';
   } else {                        // normal note
      getColumn(17) = 'Z';
   }
}


/////////////////////////////
//
// MuseRecord::setBack --
//

void MuseRecord::setBack(int value) {
   insertString(1, "back");
   setTicks(value);
}



/////////////////////////////
//
// MuseRecord::setTicks -- return the numeric value in columns 6-9.
//

void MuseRecord::setTicks(int value) {
   if ((value < 0) || (value >= 1000)) {
      cerr << "@ Error: ticks out of range in MuseRecord::setTicks" << endl;
   }
   char buffer[8] = {0};
   sprintf(buffer, "%d", value);
   int len = strlen(buffer);
   insertString(5+3-len+1, buffer);
}



//////////////////////////////
//
// MuseRecord::getTie --
//

char* MuseRecord::getTie(char* output) {
   output[0] = getColumn(9);
   if (output[0] == ' ') {
      output[0] = '\0';
   } else {
      output[1] = '\0';
   }

   return output;
}


int MuseRecord::getTie(void) {
   return tieQ();
}


//////////////////////////////
//
// MuseRecord::getTie -- Set a tie marker in column 9.  Currently
// the function does not check the type of data, so will overr-write any
// data found in column 9 (such as if the record is not for a note).
//
// If the input parameter hidden is true, then the visual tie is not
// displayed, but the sounding tie is displayed.
//

int MuseRecord::setTie(int hidden) {
   getColumn(9) = '-';
   if (!hidden) {
      return addAdditionalNotation('-');
   } else {
      return -1;
   }
}



//////////////////////////////
//
// MuseRecord::addAdditionalNotation -- ties, slurs and tuplets.
//    Currently not handling editorial levels.
//

int MuseRecord::addAdditionalNotation(char symbol) {
   // search columns 32 to 43 for the specific symbol.
   // if it is found, then don't add.  If it is not found,
   // then do add.
   int i;
   int blank = -1;
   int nonempty = 0;  // true if a non-space character was found.

   for (i=43; i>=32; i--) {
      if (getColumn(i) == symbol) {
         return i;
      } else if (!nonempty && (getColumn(i) == ' ')) {
         blank = i;
      } else {
         nonempty = i;
      }
   }

   if (symbol == '-') {
     // give preferential treatment to placing only ties in
     // column 32
     if (getColumn(32) == ' ') {
        getColumn(32) = '-';
        return 32;
     }
   }

   if (blank < 0) {
      cerr << "Error in MuseRecord::addAdditionalNotation: "
           << "no empty space for notation" << endl;
      exit(1);
   }

   if ((blank <= 32) && (getColumn(33) == ' ')) {
      // avoid putting non-tie items in column 32.
      blank = 33;
   }

   getColumn(blank) = symbol;
   return blank;
}


// add a multi-character additional notation (such as a dynamic like mf):

int MuseRecord::addAdditionalNotation(const char* symbol) {
   int len = strlen(symbol);
   // search columns 32 to 43 for the specific symbol.
   // if it is found, then don't add.  If it is not found,
   // then do add.
   int i, j;
   int blank = -1;
   int found = 0;
   int nonempty = 0;  // true if a non-space character was found.

   for (i=43-len; i>=32; i--) {
      found = 1;
      for (j=0; j<len; j++) {
         if (getColumn(i+j) != symbol[j]) {
            found = 0;
            break;
         }
      }
      if (found) {
         return i;
      } else if (!nonempty && (getColumn(i) == ' ')) {
// cout << "@COLUMN " << i << " is blank: " << getColumn(i) << endl;
         blank = i;
         // should check that there are enough blank lines to the right
         // as well...
      } else if (getColumn(i) != ' ') {
         nonempty = i;
      }
   }

   if (blank < 0) {
      cerr << "Error in MuseRecord::addAdditionalNotation2: "
           << "no empty space for notation" << endl;
      exit(1);
   }

// cout << "@ GOT HERE symbol = " << symbol << " and blank = " << blank << endl;
   if ((blank <= 32) && (getColumn(33) == ' ')) {
      // avoid putting non-tie items in column 32.
      blank = 33;
      // not worrying about overwriting something to the right
      // of column 33 since the empty spot was checked starting
      // on the right and moving towards the left.
   }
// cout << "@COLUMN 33 = " << getColumn(33) << endl;
// cout << "@ GOT HERE symbol = " << symbol << " and blank = " << blank << endl;

   for (j=0; j<len; j++) {
      getColumn(blank+j) = symbol[j];
   }
   return blank;
}



//////////////////////////////
//
// MuseRecord::tieQ -- returns true if the current line contains
//   a tie to a note in the future.  Does not check if there is a tie
//   to a note in the past.
//

int MuseRecord::tieQ(void) {
   int output = 0;
   switch (getType()) {
      case E_muserec_note_regular:
      case E_muserec_note_chord:
      case E_muserec_note_cue:
      case E_muserec_note_grace:
         if (getColumn(9) == '-') {
            output = 1;
         } else if (getColumn(9) == ' ') {
            output = 0;
         } else {
            output = -1;
         }
         break;
      default:
         return 0;
   }

   return output;
}


//////////////////////////////////////////////////////////////////////////
//
// graphical and intrepretive information for notes
//

//////////////////////////////
//
// MuseRecord::getFootnoteFlagField -- returns column 13 value
//

char* MuseRecord::getFootnoteFlagField(char* output) {
   allowFigurationAndNotesOnly("getFootnoteField");
   extract(output, 13, 13);
   return output;
}
         

//////////////////////////////
//
// MuseRecord::getFootnoteFlag --
//

char* MuseRecord::getFootnoteFlag(char* output) {
   getFootnoteFlagField(output);
   if (output[0] == ' ') {
      output[0] = '\0';
   }

   return output;
}


int MuseRecord::getFootnoteFlag(void) {
   int output = 0;
   char recordInfo[16];
   getFootnoteFlag(recordInfo);
   if (recordInfo[0] == ' ') {
      output = -1;
   } else {
      output = strtol(recordInfo, NULL, 36);
   }

   return output;
}
 


//////////////////////////////
//
// MuseRecord::footnoteFlagQ --
//

int MuseRecord::footnoteFlagQ(void) {
   int output = 0;
   char recordInfo[16];
   getFootnoteFlagField(recordInfo);
   if (recordInfo[0] == ' ') {
      output = 0;
   } else {
      output = 1;
   }

   return output;
}
      


//////////////////////////////
//
// MuseRecord::getLevelField -- return column 14
//

char* MuseRecord::getLevelField(char* output) {
   allowFigurationAndNotesOnly("getLevelField");
   extract(output, 14, 14);
   return output;
}



//////////////////////////////
//
// MuseRecord::getLevel --
//

char* MuseRecord::getLevel(char* output) {
   getLevelField(output);
   if (output[0] == ' ') {
      output[0] = '\0';
   }
   return output;
}   


int MuseRecord::getLevel(void) {
   int output = 1;
   char recordInfo[16];
   getLevelField(recordInfo);
   if (recordInfo[0] == ' ') {
      output = 1;
   } else {
      output = strtol(recordInfo, NULL, 36);
   }

   return output;
}
 


//////////////////////////////
//
// MuseRecord::levelQ --
//

int MuseRecord::levelQ(void) {
   int output = 0;
   char recordInfo[16];
   getLevelField(recordInfo);
   if (recordInfo[0] == ' ') {
      output = 0;
   } else {
      output = 1;
   }

   return output;
}
      


//////////////////////////////
//
// MuseRecord::getTrackField -- return column 15
//

char* MuseRecord::getTrackField(char* output) {
   allowNotesOnly("getTrackField");
   extract(output, 15, 15);
   return output;
}



//////////////////////////////
//
// MuseRecord::getTrack --
//

char* MuseRecord::getTrack(char* output) {
   getTrackField(output);
   if (output[0] == ' ') {
      output[0] = '\0';
   }
   return output;
}   


int MuseRecord::getTrack(void) {
   int output = 1;
   char recordInfo[16];
   getTrackField(recordInfo);
   if (recordInfo[0] == ' ') {
      output = 1;
   } else {
      output = strtol(recordInfo, NULL, 36);
   }

   return output;
}
 


//////////////////////////////
//
// MuseRecord::trackQ --
//

int MuseRecord::trackQ(void) {
   int output = 0;
   char recordInfo[16];
   getTrackField(recordInfo);
   if (recordInfo[0] == ' ') {
      output = 0;
   } else {
      output = 1;
   }

   return output;
}



//////////////////////////////
//
// MuseRecord::getGraphicNoteTypeField -- return column 17
//

char* MuseRecord::getGraphicNoteTypeField(char* output) {
// allowNotesOnly("getGraphicNoteTypefield");
   if (getLength() < 17) {
      strcpy(output, " ");
   } else {
      extract(output, 17, 17);
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::getGraphicNoteType --
//

char* MuseRecord::getGraphicNoteType(char* output) {
   getGraphicNoteTypeField(output);
   if (output[0] == ' ') {
      output[0] = '\0';
   }
   return output;
}


int MuseRecord::getGraphicNoteType(void) {
   int output = 0;
   char recordInfo[16];
   getGraphicNoteTypeField(recordInfo);
   if (recordInfo[0] == ' ') {
      cerr << "Error: no graphic note type specified: " << getLine() << endl;
      exit(1);
   }

   switch (recordInfo[0]) {
      case 'M':                            // Maxima
         output = -2;           break;
      case 'L':   case 'B':                // Longa
         output = -1;           break;
      case 'b':   case 'A':                // Breve
         output = 0;            break;
      case 'w':   case '9':                // Whole
         output = 1;            break;
      case 'h':   case '8':                // Half
         output = 2;            break;
      case 'q':   case '7':                // Quarter
         output = 4;            break;
      case 'e':   case '6':                // Eighth
         output = 8;            break;
      case 's':   case '5':                // Sixteenth
         output = 16;           break;
      case 't':   case '4':                // 32nd note
         output = 32;           break;
      case 'x':   case '3':                // 64th note
         output = 64;           break;
      case 'y':   case '2':                // 128th note
         output = 128;          break;
      case 'z':   case '1':                // 256th note
         output = 256;          break;
      default:
         cerr << "Error: unknown graphical note type in column 17: "
              << getLine() << endl;
         exit(1);
   }

   return output;
}
     
      
//////////////////////////////
//
// MuseRecord::graphicNoteTypeQ --
//

int MuseRecord::graphicNoteTypeQ(void) {
   int output = 0;
   char recordInfo[16];
   getGraphicNoteTypeField(recordInfo);
   if (recordInfo[0] == ' ') {
      output = 0;
   } else {
      output = 1;
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::graphicNoteTypeSize -- return 0 if cue note size,
//	otherwise, it will return 1 if regular size
//

int MuseRecord::getGraphicNoteTypeSize(void) {
   int output = 1;
   char recordInfo[16];
   getGraphicNoteTypeField(recordInfo);
   if (recordInfo[0] == ' ') {
      cerr << "Error: not graphic note specified in column 17: "
           << getLine() << endl;
      exit(1);
   }
 
   switch (recordInfo[0]) {
      case 'L': case 'b': case 'w': case 'h': case 'q': case 'e':
      case 's': case 't': case 'x': case 'y': case 'z': 
         output = 1;
         break;
      case 'B': case 'A': case '9': case '8': case '7': case '6':
      case '5': case '4': case '3': case '2': case '1':
         output = 0;
         break;
      default:
         cerr << "Error: unknown graphical note type in column 17: "
              << getLine() << endl;
         exit(1);
   }

   return output;
}



//////////////////////////////
//
// MuseRecord::getProlongationField -- returns column 18
//

char* MuseRecord::getProlongationField(char* output) {
//   allowNotesOnly("getProlongationField");   ---> rests also
   if (getLength() < 18) {
      strcpy(output, " ");
   } else {
      extract(output, 18, 18);
   }
 
   return output;
}



//////////////////////////////
//
// MuseRecord::getProlongation --
//

char* MuseRecord::getProlongation(char* output) {
   getProlongationField(output);
   if (output[0] == ' ') {
      output[0] = '\0';
   }
   return output;
}
  

int MuseRecord::getProlongation(void) {
   int output = 0;
   char recordInfo[16];
   getProlongationField(recordInfo);
   switch (recordInfo[0]) {
      case ' ':   output = 0;   break;
      case '.':   output = 1;   break;
      case ':':   output = 2;   break;
      default:
         cerr << "Error: unknon prologation character (column 18): "
              << getLine() << endl;
         exit(1);
   }
   
   return output;
}
         


//////////////////////////////
//
// MuseRecord::getStringProlongation --
//

char* MuseRecord::getStringProlongation(char* doutput) {
   switch (getProlongation()) {
      case 0:   strcpy(doutput, "");   break;
      case 1:   strcpy(doutput, ".");   break;
      case 2:   strcpy(doutput, "..");   break;
      default: 
         cerr << "Error: unknown number of prolongation dots (column 18): "
              << getLine() << endl;
         exit(1);
   }

   return doutput;
}



//////////////////////////////
//
// MuseRecord::prolongationQ --
//

int MuseRecord::prolongationQ(void) {
   return getProlongation();
}
   

//////////////////////////////
//
// MuseRecord::getNotatedAccidentalField -- actual notated accidental is
//	stored in column 19
//

char* MuseRecord::getNotatedAccidentalField(char* output) {
   allowNotesOnly("getNotatedAccidentalField");
   if (getLength() < 19) {
      strcpy(output, " ");
   } else {
      output[0] = getColumn(19);
      output[1] = '\0';
   }

   return output;
}



//////////////////////////////
//
// MuseRecord::getNotatedAccidental --
//

char* MuseRecord::getNotatedAccidental(char* output) {
   getNotatedAccidentalField(output);
   if (output[0] == ' ') {
      output[0] = '\0';
   }

   return output;
}


int MuseRecord::getNotatedAccidental(void) {
   int output = 0;
   char recordInfo[16];
   getNotatedAccidentalField(recordInfo);
   switch (recordInfo[0]) {
      case ' ':   output = 0;    break;
      case '#':   output = 1;    break;
      case 'n':   output = 0;    break;
      case 'f':   output = -1;   break;
      case 'x':   output = 2;    break;
      case 'X':   output = 2;    break;
      case '&':   output = -2;   break;
      case 'S':   output = 1;    break;
      case 'F':   output = -1;   break;
      default:
         cerr << "Error: unknown accidental: " << recordInfo[0] << endl;
         exit(1);
   }

   return output;
}



//////////////////////////////
//
// MuseRecord::notatedAccidentalQ --
//

int MuseRecord::notatedAccidentalQ(void) {
   int output;
   char recordInfo[16];
   getNotatedAccidentalField(recordInfo);
   if (recordInfo[0] == ' ') {
      output = 0;
   } else {
      output = 1;
   }

   return output;
}
   


///////////////////////////////
//
// MuseRecord::getTimeModificationField -- return columns 20 -- 22.
//

char* MuseRecord::getTimeModificationField(char* output) {
//   allowNotesOnly("getTimeModificationField");   ---> rests also
   if (getLength() < 20) {
      strcpy(output, "   ");
   } else {
      extract(output, 20, 22);
   }

   return output;
}



//////////////////////////////
//
// MuseRecord::getTimeModification --
//

char* MuseRecord::getTimeModification(char* output) {
   getTimeModificationField(output);
   int index = 2;
   while (index >= 0 && output[index] == ' ') {
      output[index] = '\0';
      index--;
   }
   if (output[0] == ' ') {
      output[0] = output[1];
      output[1] = output[2];
   }
   if (output[0] == ' ') {
      output[0] = output[1];
   }
   if (output[0] == ' ') {
      cerr << "Error: funny error occured in time modification "
           << "(columns 20-22): " << getLine() << endl;
      exit(1);
   }

   return output;
}



//////////////////////////////
//
// MuseRecord::getTimeModificationLeftField -- return column 20
//

char* MuseRecord::getTimeModificationLeftField(char* output) {
   getTimeModificationField(output);
   output[1] = '\0';
   return output;
}



//////////////////////////////
//
// MuseRecord::getTimeModificationLeft --
//

char* MuseRecord::getTimeModificationLeft(char* output) {
   getTimeModificationField(output);
   if (output[0] == ' ') {
      output[0] = '\0';
   } else {
      output[1] = '\0';
   }
   return output;
}


int MuseRecord::getTimeModificationLeft(void) {
   int output = 0;
   char recordInfo[16];
   getTimeModificationLeft(recordInfo);
   if (recordInfo[0] == ' ') {
      output = 0;
   } else {
      output = strtol(recordInfo, NULL, 36);
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::getTimeModificationRightField -- return column 20
//

char* MuseRecord::getTimeModificationRightField(char* output) {
   getTimeModificationField(output);
   output[0] = output[2];
   output[1] = '\0';
   return output;
}



//////////////////////////////
//
// MuseRecord::getTimeModificationRight --
//

char* MuseRecord::getTimeModificationRight(char* output) {
   getTimeModificationField(output);
   if (output[2] == ' ') {
      output[0] = '\0';
   } else {
      output[0] = output[2];
      output[1] = '\0';
   }
   return output;
}


int MuseRecord::getTimeModificationRight(void) {
   int output = 0;
   char recordInfo[16];
   getTimeModificationRight(recordInfo);
   if (recordInfo[2] == ' ') {
      output = 0;
   } else {
      output = strtol(&recordInfo[2], NULL, 36);
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::timeModificationQ --
//

int MuseRecord::timeModificationQ(void) {
   int output = 0;
   char recordInfo[16];
   getTimeModificationField(recordInfo);
   if (recordInfo[0] != ' ' || recordInfo[1] != ' ' || recordInfo[2] != ' ') {
      output = 1;
   } else {
      output = 0;
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::timeModificationLeftQ --
//
   
int MuseRecord::timeModificationLeftQ(void) {
   int output = 0;
   char recordInfo[16];
   getTimeModificationField(recordInfo);
   if (recordInfo[0] == ' ') {
      output = 0;
   } else {
      output = 1;
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::timeModificationRightQ --
//
   
int MuseRecord::timeModificationRightQ(void) {
   int output = 0;
   char recordInfo[16];
   getTimeModificationField(recordInfo);
   if (recordInfo[2] == ' ') {
      output = 0;
   } else {
      output = 1;
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::getStemDirectionField --
//

char* MuseRecord::getStemDirectionField(char* output) {
   allowNotesOnly("getStemDirectionField");
   if (getLength() < 23) {
      strcpy(output, " ");
   } else {
      output[0] = getColumn(23);
      output[1] = '\0';
   }
   
   return output;
}


//////////////////////////////
//
// MuseRecord::getStemDirection --
//

char* MuseRecord::getStemDirection(char* output) {
   getStemDirectionField(output);
   if (output[0] == ' ') {
      output[0] = '\0';
   }

   return output;
}


int MuseRecord::getStemDirection(void) {
   int output = 0;
   char recordInfo[16];
   getStemDirectionField(recordInfo);
   switch (recordInfo[0]) {
      case 'u':   output = 1;   break;
      case 'd':   output = -1;  break;
      case ' ':   output = 0;   break;
      default:
         cerr << "Error: unknown stem direction: " << recordInfo[0] << endl;
         exit(1);
   }

   return output;
}



//////////////////////////////
//
// stemDirectionQ --
//

int MuseRecord::stemDirectionQ(void) {
   int output = 0;
   char recordInfo[16];
   getStemDirectionField(recordInfo);
   if (recordInfo[0] == ' ') {
      output = 0;
   } else {
      output = 1;
   }
   
   return output;
} 


//////////////////////////////
//
// MuseRecord::getStaffField  -- returns column 24.
//

char* MuseRecord::getStaffField(char* output) {
   allowNotesOnly("getStaffField");
   if (getLength() < 24) {
      strcpy(output, " ");
   } else {
      output[0] = getColumn(24);
      output[1] = '\0';
   }
   return output;
}


//////////////////////////////
//
// MuseRecord::getStaff --
//

char* MuseRecord::getStaff(char* output) {
   getStaffField(output);
   if (output[0] == ' ') {
      output[0] = '\0';
   }
   return output;
}


int MuseRecord::getStaff(void) {
   int output = 1;
   char recordInfo[16];
   getStaffField(recordInfo);
   if (recordInfo[0] == ' ') {
      output = 1;
   } else {
      output = strtol(recordInfo, NULL, 36);
   }
   
   return output;
}



//////////////////////////////
//
// MuseRecord::staffQ --
//

int MuseRecord::staffQ(void) {
   int output = 0;
   char recordInfo[16];
   getStaffField(recordInfo);
   if (recordInfo[0] == ' ') {
      output = 0;
   } else {
      output = 1;
   }

   return output;
} 



//////////////////////////////
//
// MuseRecord::getBeamField --
//

char* MuseRecord::getBeamField(char* output) {
   allowNotesOnly("getBeamField");
   if (getLength() < 26) {
      strcpy(output, "      ");
   } else {
      extract(output, 26, 31);
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::setBeamInfo --
//

void MuseRecord::setBeamInfo(Array<char>& strang) {
   setColumns(strang, 26, 31);
}



//////////////////////////////
//
// MuseRecord::beamQ --
//

int MuseRecord::beamQ(void) {
   int output = 0;
   allowNotesOnly("beamQ");
   if (getLength() < 26) {
      output = 0;
   } else {
      for (int i=26; i<=31; i++) {
         if (getColumn(i) != ' ') {
            output = 1;
            break;
         }
      }
   }
   return output;
}




//////////////////////////////
//
// MuseRecord::getBeam8 -- column 26
//

char MuseRecord::getBeam8(void) {
   allowNotesOnly("getBeam8");
   return getColumn(26);
}



//////////////////////////////
//
// MuseRecord::getBeam16 -- column 27
//

char MuseRecord::getBeam16(void) {
   allowNotesOnly("getBeam16");
   return getColumn(27);
}



//////////////////////////////
//
// MuseRecord::getBeam32 -- column 28
//

char MuseRecord::getBeam32(void) {
   allowNotesOnly("getBeam32");
   return getColumn(28);
}



//////////////////////////////
//
// MuseRecord::getBeam64 -- column 29
//

char MuseRecord::getBeam64(void) {
   allowNotesOnly("getBeam64");
   return getColumn(29);
}



//////////////////////////////
//
// MuseRecord::getBeam128 -- column 30
//

char MuseRecord::getBeam128(void) {
   allowNotesOnly("getBeam128");
   return getColumn(30);
}



//////////////////////////////
//
// MuseRecord::getBeam256 -- column 31
//

char MuseRecord::getBeam256(void) {
   allowNotesOnly("getBeam256");
   return getColumn(31);
}



//////////////////////////////
//
// MuseRecord::beam8Q --
//

int MuseRecord::beam8Q(void) {
   int output = 0;
   if (getBeam8() == ' ') {
      output = 0;
   } else {
      output = 1;
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::beam16Q --
//

int MuseRecord::beam16Q(void) {
   int output = 0;
   if (getBeam16() == ' ') {
      output = 0;
   } else {
      output = 1;
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::beam32Q --
//

int MuseRecord::beam32Q(void) {
   int output = 0;
   if (getBeam32() == ' ') {
      output = 0;
   } else {
      output = 1;
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::beam64Q --
//

int MuseRecord::beam64Q(void) {
   int output = 0;
   if (getBeam64() == ' ') {
      output = 0;
   } else {
      output = 1;
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::beam128Q --
//

int MuseRecord::beam128Q(void) {
   int output = 0;
   if (getBeam128() == ' ') {
      output = 0;
   } else {
      output = 1;
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::beam256Q --
//

int MuseRecord::beam256Q(void) {
   int output = 0;
   if (getBeam256() == ' ') {
      output = 0;
   } else {
      output = 1;
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::getKernBeamStyle --
//

char* MuseRecord::getKernBeamStyle(char* output) {
   char beams[16];
   output[0] = '\0';
   getBeamField(beams);   // 6 characters wide
   for (int i=0; i<6; i++) {
      switch (beams[i]) {
         case '[':                 // start beam
            strcat(output, "L");
            break;
         case '=':                 // continue beam
            // do nothing
            break;
         case ']':                 // end beam
            strcat(output, "J");
            break;
         case 'K':                 // forward hook 
            strcat(output, "K");
            break;
         case 'k':                 // backward hook
            strcat(output, "k");
            break;
         default:
            ;  // do nothing
      }
   }
   return output;
}
 


//////////////////////////////
//
// MuseRecord::getAdditionalNotationsField -- returns the contents
// 	of columns 32-43.
//

char* MuseRecord::getAdditionalNotationsField(char* output) {
   allowNotesOnly("getAdditionalNotationsField");
   extract(output, 32, 43);
   return output;
}



//////////////////////////////
//
// MuseRecord::additionalNotationsQ --
//

int MuseRecord::additionalNotationsQ(void) {
   int output = 0;
   if (getLength() < 32) {
      output = 0;
   } else {
      for (int i=32; i<=43; i++) {
         if (getColumn(i) != ' ') {
            output = 1;
            break;
         }
      }
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::getAddCount -- returns the number of items
//	in the additional notations field
//

int MuseRecord::getAddCount(void) {
   char addString[128] = {0};     // entire additional notation field
   char addElement[128] = {0};    // element from the notation field
   getAdditionalNotationsField(addString);

   int count = 0;
   int index = 0;
   while (getAddElementIndex(index, addElement, addString)) {
      count++;
   }

   return count;
}



//////////////////////////////
//
// MuseRecord::getAddItem -- returns the specified item
//	in the additional notations field
//

char* MuseRecord::getAddItem(int elementIndex, char* output) {
   int count = 0;
   int index = 0;
   char addString[128] = {0};     // entire additional notation field
   getAdditionalNotationsField(addString);

   while (count <= elementIndex) {
      getAddElementIndex(index, output, addString);
      count++;
   }

   return output;
}



//////////////////////////////
//
// MuseRecord::getAddItemLevel -- returns the specified item's
//	editorial level in the additional notations field
//

int MuseRecord::getAddItemLevel(int elementIndex) {
   int count = 0;
   int index = 0;
   char number[4] = {0};
   char addString[128] = {0};     // entire additional notation field
   char elementString[128] = {0}; // element field
   getAdditionalNotationsField(addString);

   while (count < elementIndex) {
      getAddElementIndex(index, elementString, addString);
      count++;
   }

   int output = -1;
repeating:
   while (addString[index] != '&' && index >= 0) {
      index--;
   }
   if (addString[index] == '&' && !isalnum(addString[index+1])) {
      index--;
      goto repeating;
   } else if (addString[index] == '&') {
      number[0] = addString[index+1];
      output = strtol(number, NULL, 36);
   }

   return output;
}



//////////////////////////////
//
// MuseRecord::getEditorialLevels -- returns a string containing the
//	edit levels given in the additional notation fields
//

char* MuseRecord::getEditorialLevels(char* output) {
   int outputIndex = 0;

   char addString[128] = {0};     // entire additional notation field
   getAdditionalNotationsField(addString);

   for (int index = 0; index < 12-1; index++) {
      if (addString[index] == '&' && isalnum(addString[index+1])) {
         output[outputIndex++] = addString[index+1];
      }
   }
   output[outputIndex] = '\0';

   return output;
}



//////////////////////////////
//
// MuseRecord::addEditorialLevelQ -- returns true if there are any editorial
//	levels present in the additional notations fields
//

int MuseRecord::addEditorialLevelQ(void) {
   char addString[128] = {0};     // entire additional notation field
   getAdditionalNotationsField(addString);

   int output = 0;
   for (int i=0; i<12-1; i++) {   // minus one for width 2 (&0)
      if (addString[i] == '&' && isalnum(addString[i+1])) {
         output = 1;
      }
   }

   return output;
}



//////////////////////////////
//
// MuseRecord::findField -- returns true when it finds the first 
//	instance of the key in the additional fields record.
//

int MuseRecord::findField(const char* key) {
   int length = strlen(key);
   char notations[128];
   int output = 0;
   getAdditionalNotationsField(notations);
   for (int i=0; i<12-length; i++) {
      if (notations[i] == key[0]) {
         output = 1;
         for (int j=0; j<length; j++) {
            if (notations[i] != key[j]) {
               output = 0;
               goto endofloop;
            }
         }
      }

      if (output == 1) {
         break;
      }
endofloop: 
   ;
   }
   
   return output;
}
            


//////////////////////////////
//
// MuseRecord::findField --
//

int MuseRecord::findField(char key, int mincol, int maxcol) {
   int start = mincol;
   int stop = getLength() - 1;

   if (start > stop) {
      return -1;
   }

   if (maxcol < stop) {
      stop = maxcol;
   }

   int i;
   for (i=start; i<=stop; i++) {
      if (recordString[i-1] == key) {
         return i;   // return the column which is offset from 1
      }
   }

   return -1;
}



//////////////////////////////
//
// MuseRecord::getSlurStartColumn -- search column 32 to 43 for a slur
//    marker.  Returns the first one found from left to right.
//    returns -1 if a slur character was not found.
//

int MuseRecord::getSlurStartColumn(void) {
   int start = 31;
   int stop = getLength() - 1;
   if (stop >= 43) {
      stop = 42;
   }
   int i;
   for (i=start; i<=stop; i++) {
      switch (recordString[i]) {
         case '(':   // slur level 1
         case '[':   // slur level 2
         case '{':   // slur level 3
         case 'z':   // slur level 4
            return i+1;  // column is offset from 1
      }
   }

   return -1;
}

   

//////////////////////////////
//
// MuseRecord::getTextUnderlayField -- returns the contents
// 	of columns 44-80.
//

char* MuseRecord::getTextUnderlayField(char* output) {
   allowNotesOnly("getTextUnderlayField");
   extract(output, 44, 80);
   return output;
}



//////////////////////////////
//
// MuseRecord::textUnderlayQ --
//

int MuseRecord::textUnderlayQ(void) {
   int output = 0;
   if (getLength() < 44) {
      output = 0;
   } else {
      for (int i=44; i<=80; i++) {
         if (getColumn(i) != ' ') {
            output = 1;
            break;
         }
      }
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::getVerseCount --
//

int MuseRecord::getVerseCount(void) {
   if (!textUnderlayQ()) {
      return 0;
   }

   int count = 1;
   for (int i=44; i<=getLength() && i <= 80; i++) {
      if (getColumn(i) == '|') {
         count++;
      }
   }

   return count;
}



//////////////////////////////
//
// MuseRecord::getVerse --
//

char* MuseRecord::getVerse(int index, char* output) {
   if (!textUnderlayQ()) {
      strcpy(output, "");
      return output;
   }
   int verseCount = getVerseCount();
   if (index >= verseCount) {
      strcpy(output, "");
      return output;
   }

   int tindex = 44;
   int c = 0;
   while (c < index && tindex < 80) {
      if (getColumn(tindex) == '|') {
         c++;
      } 
      tindex++;
   }

   int oindex = 0;
   while (tindex <= 80 && getColumn(tindex) != '|') {
      output[oindex++] = getColumn(tindex++);
   }

   // remove trailing spaces
   int zindex = oindex-1;
   while (output[zindex] == ' ') {
      zindex--;
   }
   zindex++;
   output[zindex] = '\0';

   // remove leading spaces
   int spacecount = 0;
   while (output[spacecount] == ' ') {
      spacecount++;
   }

   for (int rr = 0; rr <= zindex-spacecount; rr++) {
      output[rr] = output[rr+spacecount];
   }

   return output;
}
      

//////////////////////////////
//
// MuseRecord::getKernNoteStyle --
//	default values: beams = 0, stems = 0
//

char* MuseRecord::getKernNoteStyle(char* output, int beams, int stems) {
   output[0] ='\0';
   char temp[16];

   // place the rhythm
   SSTREAM tempdur;
   int notetype = getGraphicNoteType();
   if (timeModificationLeftQ()) {
      notetype = notetype / 4 * getTimeModificationLeft();
      if (timeModificationRightQ()) {
         notetype = notetype * getTimeModificationRight();
      } else {
         notetype = notetype * 2;
      }
   }
   tempdur << notetype << ends;
   strcat(output, tempdur.CSTRING);

   // add any dots of prolongation to the output string
   strcat(output, getStringProlongation(temp));

   // add the pitch to the output string
   char kernpitch[16];
   char musepitch[16];
   Convert::musePitchToKernPitch(kernpitch, getPitch(musepitch));
   strcat(output, kernpitch);

   // if there is a notated natural sign, then add it now:
   if (strcmp(getNotatedAccidentalField(temp), "n") == 0) {
      strcat(output, "n");
   }

   // check if a grace note
   if (getType() == 'g') {
      strcat(output, "Q");
   }

   // if stems is true, then show stem directions
   if (stems && stemDirectionQ()) {
      switch (getStemDirection()) {
         case 1:                         // 'u' = up
            strcat(output, "/");
            break;
         case -1:                        // 'd' = down
            strcat(output, "\\");
         default:
            ; // nothing                 // ' ' = no stem (if stage 2)
      }
   }
  
   // if beams is true, then show any beams
   if (beams && beamQ()) {
      getKernBeamStyle(temp);
      strcat(output, temp);
   }

   return output;
}



//////////////////////////////
//
// MuseRecord::getKernNoteAccents --
//

char* MuseRecord::getKernNoteAccents(char* output) {
   char tempnote[32];
   output[0] = '\0';
   int addnotecount = getAddCount();
   for (int i=0; i<addnotecount; i++) {
      getAddItem(i, tempnote);
      switch (tempnote[0]) {
         case 'v':   strcat(output, "v");   break; // up-bow 
         case 'n':   strcat(output, "u");   break; // down-bow 
         case 'o':   strcat(output, "j");   break; // harmonic
         case 'O':   strcat(output, "I");   break; // open string (to generic)
         case 'A':   strcat(output, "^");   break; // accent up
         case 'V':   strcat(output, "^");   break; // accent down
         case '>':   strcat(output, "^");   break; // horizontal accent
         case '.':   strcat(output, "'");   break; // staccato
         case '_':   strcat(output, "~");   break; // tenuto
         case '=':   strcat(output, "~'");  break; // detached legato
         case 'i':   strcat(output, "s");   break; // spiccato
         case '\'':  strcat(output, ",");   break; // breath mark
         case 'F':   strcat(output, ";");   break; // fermata up
         case 'E':   strcat(output, ";");   break; // fermata down
         case 'S':   strcat(output, ":");   break; // staccato
         case 't':   strcat(output, "O");   break; // trill (to generic)
         case 'r':   strcat(output, "S");   break; // turn
         case 'k':   strcat(output, "O");   break; // delayed turn (to generic)
         case 'w':   strcat(output, "O");   break; // shake (to generic)
         case 'M':   strcat(output, "O");   break; // mordent (to generic)
         case 'j':   strcat(output, "H");   break; // glissando (slide)
     }
   }
   
   return output;
}
   




//////////////////////////////
//
// MuseRecord::getKernRestStyle --
//

char* MuseRecord::getKernRestStyle(char* output, int quarter) {
   output[0] ='\0';

   char temp[16];
   char rhythmstring[16];

   // place the rhythm
   SSTREAM tempdur;
  
   int notetype;
   float dnotetype;
   if (graphicNoteTypeQ()) {
      notetype = getGraphicNoteType();
         
      if (timeModificationLeftQ()) {
         notetype = notetype / 4 * getTimeModificationLeft();
      }
      if (timeModificationRightQ()) {
         notetype = notetype * getTimeModificationRight() / 2;
      }
      tempdur << notetype << ends;
      strcat(output, tempdur.CSTRING);

      // add any dots of prolongation to the output string
      strcat(output, getStringProlongation(temp));
   } else {   // stage 1 data:
      dnotetype = (float)getTickDurationField() / quarter;
      Convert::durationToKernRhythm(rhythmstring, dnotetype);
      strcat(output, rhythmstring);
   }

   // add the pitch to the output string
   strcat(output, "r");

   return output;
}



//////////////////////////////////////////////////////////////////////////
//
// functions which work with measure records
//


//////////////////////////////
//
// MuseRecord::getMeasureNumberField -- columns 9-12
//

char*  MuseRecord::getMeasureNumberField(char* output) {
   allowMeasuresOnly("getMeasureNumberField");
   extract(output, 9, 12);
   return output;
}



//////////////////////////////
//
// MuseRecord::getMeasureTypeField -- columns 1 -- 7
//

char*  MuseRecord::getMeasureTypeField(char* output) {
   allowMeasuresOnly("getMeasureTypeField");
   extract(output, 1, 7);
   return output;
}



//////////////////////////////
//
// MuseRecord::getMeasureNumber --
//

char*  MuseRecord::getMeasureNumber(char* output) {
   getMeasureNumberField(output);
   for (int i=3; i>=0; i--) {
      if (output[i] == ' ') {
         output[i] = '\0';
      }
   }
   return output;
}


int MuseRecord::getMeasureNumber(void) {
   char measureInfo[16];
   getMeasureNumber(measureInfo);
   return atoi(measureInfo);
}
   


//////////////////////////////
//
// MuseRecord::measureNumberQ --
//

int MuseRecord::measureNumberQ(void) {
   char temp[16];
   getMeasureNumber(temp);
   int i = 0;
   int output = 0;
   while (temp[i] != '\0') {
      if (temp[i] != ' ') {
         output = 1;
         break;
      }
   }

   return output;
}



//////////////////////////////
//
// MuseRecord::measureFermataQ -- returns true if there is a
//	fermata above or below the measure
//

int MuseRecord::measureFermataQ(void) {
   int output = 0;
   for (int i=17; i<=80 && i<= getLength(); i++) {
      if (getColumn(i) == 'F' || getColumn(i) == 'E') {
         output = 1;
         break;
      }
   }
   
   return output;
}


//////////////////////////////
//
// MuseRecord::measureFlagQ -- Returns true if there are non-space
//     characters in columns 17 through 80.   A more smarter way of 
//     doing this is checking the allocated length of the record, and
//     do not search non-allocated columns for non-space characters...
//

int MuseRecord::measureFlagQ(const char* key) {
   int output = 0;
   int length = strlen(key);
   for (int i=17; i<=80-length && i<getLength(); i++) {
      if (getColumn(i) == key[0]) {
         output = 1;
         for (int j=0; j<length; j++) {
            if (getColumn(i+j) != key[j]) {
               output = 0;
               break;
            }
         }
         if (output == 1) {
            break;
         }
      }
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::addMeasureFlag -- add the following characters to the 
//    Flag region of the measure flag area (columns 17-80).  But only
//    add the flag if it is not already present in the region.  If it is
//    not present, then append it after the last non-space character
//    in that region.  A space will be added between the last item
//    and the newly added parameter.
//

void MuseRecord::addMeasureFlag(const char* strang) {
   Array<char> flags;
   getColumns(flags, 17, 80);
   Array<char> flag;
   flag.setSize(strlen(strang)+1);
   PerlRegularExpression pre;
   // pre.setBasicSyntax();
   strcpy(flag.getBase(), strang);
   pre.sar(flag, "\\*", "\\*", "g");
   pre.sar(flag, "\\|", "\\|", "g");
   if (pre.search(flags, flag.getBase(), "")) {
      // flag was already found in flags, so don't do anything
      return;
   }

   pre.sar(flags, "\\s+$", "", "");
   flags.setSize(flags.getSize() + strlen(strang) + 1);
   strcat(flags.getBase(), " ");
   strcat(flags.getBase(), strang);
   setColumns(flags, 17, 80);
}



//////////////////////////////
//
// MuseRecord::getKernMeasureStyle --
//

char* MuseRecord::getKernMeasureStyle(char* output) {
   allowMeasuresOnly("getKernMeasureStyle");
   char temp[128];
   char tempstyle[16];
   getMeasureTypeField(tempstyle);

   strcpy(output, "=");
   if (strcmp(tempstyle, "mheavy2") == 0) {
      strcat(output, "=");
   } else if (strcmp(tempstyle, "mheavy3") == 0) {
      strcat(output, "=");
   } else if (strcmp(tempstyle, "mheavy4") == 0) {
      strcat(output, "=");
   }

   if (measureNumberQ()) {
      getMeasureNumber(temp);
      strcat(output, temp);
   }
   for (int i=0; i<7; i++) {
      temp[i] = tolower(temp[i]);
   }

   if (strcmp(tempstyle, "mheavy1") == 0) {
      strcat(output, "!");
   } else if (strcmp(tempstyle, "mheavy2") == 0) {
        if (measureFlagQ(":||:")) {
           strcat(output, ":|!:");
           zerase(output, 1);             // make "==" become "="
        } else if (measureFlagQ(":|")) {
           strcat(output, ":|!");
           zerase(output, 1);             // make "==" become "="
        }
   } else if (strcmp(tempstyle, "mheavy3") == 0) {
      strcat(output, "!|");
   } else if (strcmp(tempstyle, "mheavy4") == 0) {
      if (measureFlagQ(":||:")) {
         strcat(output, ":!!:");
      } else {
         strcat(output, "!!");
      }
   }



   return output;
}


//////////////////////////////////////////////////////////////////////////
//
// functions which work with musical attributes records
//

//////////////////////////////
//
// MuseRecord::getAttributeList --
//

char* MuseRecord::getAttributeList(char* output) {
   switch (getType()) {
      case E_muserec_musical_attributes:
         break;
      default:
         cerr << "Error: cannot use getAttributeList function on line: " 
              << getLine() << endl;
         exit(1);
   }

   int ending = 0;
   int index = 0;
   int tempcol;
   for (int column=4; column <= getLength(); column++) {
      if (getColumn(column) == ':') {
         tempcol = column - 1;
         while (tempcol > 0 && getColumn(tempcol) != ' ') {
            tempcol--;
         } 
         tempcol++;
         while (tempcol <= column) {
            output[index] = getColumn(tempcol);
            if (output[index] == 'D') {
               ending = 1;
            }
            tempcol++;
            index++;
         }
      }
      if (ending) {
         break;
      }
   }
   output[index] = '\0';
    
   return output;
}



//////////////////////////////
//
// MuseRecord::attributeQ --
//

int MuseRecord::attributeQ(const char* attribute) {
   switch (getType()) {
      case E_muserec_musical_attributes:
         break;
      default:
         cerr << "Error: cannot use getAttributeList function on line: " 
              << getLine() << endl;
         exit(1);
   }


   char attributelist[64];
   getAttributeList(attributelist);

   int output = 0;
   int attstrlength = strlen(attributelist);
   int attlength = strlen(attribute);

   for (int i=0; i<attstrlength-attlength+1; i++) {
      if (attributelist[i] == attribute[0]) {
         output = 1;
         for (int j=0; j<attlength; j++) {
            if (attributelist[i] != attribute[j]) {
               output = 0;
               break;
            }
         }
         if (output == 1) {
            break;
         }
      }
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::getAttributeInt --
//

int MuseRecord::getAttributeInt(char attribute) {
   switch (getType()) {
      case E_muserec_musical_attributes:
         break;
      default:
         cerr << "Error: cannot use getAttributeInt function on line: " 
              << getLine() << endl;
         exit(1);
   }

   int output = E_unknown;
   int ending = 0;
   int index = 0;
   int tempcol;
   int column;
   for (column=4; column <= getLength(); column++) {
      if (getColumn(column) == ':') {
         tempcol = column - 1;
         while (tempcol > 0 && getColumn(tempcol) != ' ') {
            tempcol--;
         }
         tempcol++;
         while (tempcol <= column) {
            if (getColumn(tempcol) == attribute) {
               ending = 2;
            } else if (getColumn(tempcol) == 'D') {
               ending = 1;
            } 
            tempcol++;
            index++;
         }
      }
      if (ending) {
         break;
      }
   }

   if (ending == 0 || ending == 1) {
      return output;
   } else {
      output = atoi(&getColumn(column+1));
      return output;
   }
}



//////////////////////////////
//
// MuseRecord::getAttributeString -- returns true if found attribute
//

int MuseRecord::getAttributeString(char* output, const char* attribute) {
   switch (getType()) {
      case E_muserec_musical_attributes:
         break;
      default:
         cerr << "Error: cannot use getAttributeInt function on line: " 
              << getLine() << endl;
         exit(1);
   }

   int returnValue = 0;
   int ending = 0;
   int index = 0;
   int tempcol;
   int column;
   for (column=4; column <= getLength(); column++) {
      if (getColumn(column) == ':') {
         tempcol = column - 1;
         while (tempcol > 0 && getColumn(tempcol) != ' ') {
            tempcol--;
         }
         tempcol++;
         while (tempcol <= column) {
            if (getColumn(tempcol) == attribute[0]) {
               ending = 2;
            } else if (getColumn(tempcol) == 'D') {
               ending = 1;
            } 
            tempcol++;
            index++;
         }
      }
      if (ending) {
         break;
      }
   }

   int tempindex = 0;
   if (ending == 0 || ending == 1) {
      return returnValue;
   } else {
      returnValue = 1;
      column++;
      while (getColumn(column) != ' ') {
         output[tempindex++] = getColumn(column++);
      }
      output[tempindex] = '\0';
      return returnValue;
   }
}



///////////////////////////////////////////////////////////////////////////
//
// functions which work with basso continuo figuration records (f):
//


//////////////////////////////
//
// MuseRecord::getFigureCountField -- column 2.
//

char* MuseRecord::getFigureCountField(char* output) {
   allowFigurationOnly("getFigureCountField");
   extract(output, 2, 2);
   return output;
}
   

//////////////////////////////
//
// MuseRecord::getFigurationCount --
//

char* MuseRecord::getFigureCount(char* output) {
   allowFigurationOnly("getFigureCount");
   extract(output, 2, 2);
   if (output[0] == ' ') {
      output[0] = '\0';
   }
   return output;
}
 

int MuseRecord::getFigureCount(void) {
   allowFigurationOnly("getFigureCount");
   char temp[4];
   getFigureCount(temp);
   int output = strtol(temp, NULL, 36);
   return output;
}



//////////////////////////////
//
// getFigurePointerField -- columns 6 -- 8.
//

char* MuseRecord::getFigurePointerField(char* output) {
   allowFigurationOnly("getFigurePointerField");
   extract(output, 6, 8);
   return output;
}


//////////////////////////////
//
// figurePointerQ --
//
   
int MuseRecord::figurePointerQ(void) {
   allowFigurationOnly("figurePointerQ");
   int output = 0;
   for (int i=6; i<=8; i++) {
      if (getColumn(i) != ' ') {
         output = 1;
         break;
      }
   }
   return output;
}



//////////////////////////////
//
// MuseRecord::getFigureFields -- columns 17 -- 80
//

char* MuseRecord::getFigureFields(char* output) {
   allowFigurationOnly("getFigureFields");
   extract(output, 17, 80);
   return output;
}
   

//////////////////////////////
//
// MuseRecord::figureFieldsQ --
//

int MuseRecord::figureFieldsQ(void) {
   allowFigurationOnly("figureFieldsQ");
   int output = 0;
   if (getLength() < 17) {
      output = 0;
   } else {
      for (int i=17; i<=80; i++) {
         if (getColumn(i) != ' ') {
            output = 1;
            break;
         }
      }
   }
   return output;
}
      


//////////////////////////////
//
// getFigure --
//

char* MuseRecord::getFigure(int index, char* output) {
   allowFigurationOnly("getFigure");
   if (index < 0 || index >= getFigureCount()) {
      output[0] = '\0';
      return output;
   }

   char temp[128] = {0};
   char *tempp;
   getFigureFields(temp);
   tempp = strtok(temp, " "); 
   for (int i=1; i<= index; i++) {
      tempp = strtok(NULL, " ");
   }
   strcpy(output, tempp);
   return output;
}



///////////////////////////////////////////////////////////////////////////
//
// protected functions
//


//////////////////////////////
//
// MuseRecord::allowFigurationOnly --
//

void MuseRecord::allowFigurationOnly(const char* functionName) {
   switch (getType()) {
      case E_muserec_figured_harmony:
        break;
      default:
         cerr << "Error: can only access " << functionName 
              << " on a figuration record.  Line is: " << getLine() << endl;
         exit(1);
   }
}



//////////////////////////////
//
// MuseRecord::allowFigurationAndNotesOnly --
//

void MuseRecord::allowFigurationAndNotesOnly(const char* functionName) {
   switch (getType()) {
      case E_muserec_figured_harmony:
      case E_muserec_note_regular:
      case E_muserec_note_chord:
      case E_muserec_note_grace:
      case E_muserec_note_cue:
        break;
      default:
         cerr << "Error: can only access " << functionName 
              << " on a figuration record.  Line is: " << getLine() << endl;
         exit(1);
   }
}



//////////////////////////////
//
// MuseRecord::allowMeasuresOnly --
//

void MuseRecord::allowMeasuresOnly(const char* functionName) {
   switch (getType()) {
      case E_muserec_measure:
        break;
      default:
         cerr << "Error: can only access " << functionName 
              << " on a measure record.  Line is: " << getLine() << endl;
         exit(1);
   }
}



//////////////////////////////
//
// MuseRecord::allowNotesOnly --
//

void MuseRecord::allowNotesOnly(const char* functionName) {
   switch (getType()) {
      case E_muserec_note_regular:
      case E_muserec_note_chord:
      case E_muserec_note_grace:
      case E_muserec_note_cue:
        break;
      default:
         cerr << "Error: can only access " << functionName 
              << " on a note record.  Line is: " << getLine() << endl;
         exit(1);
   }
}



//////////////////////////////
//
// MuseRecord::getAddElementIndex -- get the first element pointed
//	to by the specified index in the additional notations field.
//	returns 0 if no element was found, or 1 if an element was found.
//

int MuseRecord::getAddElementIndex(int& index, char* output, char* input) {
   int finished = 0;
   int outputIndex = 0;
   int count = 0;

   while (!finished) {
      switch (input[index]) {
         case '&':                     // editorial level marker
            // there is exactly one character following an editorial
            // marker.  neither the '&' nor the following character
            // is counted if the following character is in the set
            // [0..9, A..Z, a..z]
            index++;
            if (isalnum(input[index])) {
               index++;
            } else {
               // count '&' as an element
               count++;
               output[outputIndex++] = '&';
            }
            break;

         case 'p': case 'f':          // piano and forte
            // any sequence of 'p' and 'f' is considered one element
            count++;
            output[outputIndex++] = input[index++];
            while (input[index] == 'p' || input[index] == 'f') {
               output[outputIndex++] = input[index++];
            }
            break;

         case 'Z':                    // sfz, or Zp = sfp
            // elements starting with 'Z':
            //    Z      = sfz 
            //    Zp     = sfp
            count++;
            output[outputIndex++] = input[index++];
            if (input[index] == 'p') {
               output[outputIndex++] = input[index++];
            } 
            break;

         case 'm':                      // mezzo
            // a mezzo marking MUST be followed by a 'p' or an 'f'.
            count++;
            output[outputIndex++] = input[index++];
            if (input[index] == 'p' || input[index] == 'f') {
               output[outputIndex++] = input[index++];
            } else {
              cout << "Error at \'m\' in notation field: " << input << endl;
              exit(1);
            }
            break;

         case 'S':                     // arpeggiation
            // elements starting with 'S':
            //   S     = arpeggiate (up)
            //   Sd    = arpeggiate down)
            count++;
            output[outputIndex++] = input[index++];
            if (input[index] == 'd') {
               output[outputIndex++] = input[index++];
            } 
            break;

         case '1':                     // fingering
         case '2':                     // fingering
         case '3':                     // fingering
         case '4':                     // fingering
         case '5':                     // fingering
         // case ':':                  // finger substitution
            // keep track of finger substitutions
            count++;
            output[outputIndex++] = input[index++];
            if (input[index] == ':') {
               output[outputIndex++] = input[index++];
               output[outputIndex++] = input[index++];
            }
            break;

         //////////////////////////////
         // Ornaments
         //
         case 't':                     // trill (tr.)  
         case 'r':                     // turn  
         case 'k':                     // delayed turn  
         case 'w':                     // shake  
         case '~':                     // trill wavy line extension  
         case 'c':                     // continued wavy line  
         case 'M':                     // mordent
         case 'j':                     // slide (Schleifer)  
           // ornaments can be modified by accidentals:
           //    s     = sharp
           //    ss    = double sharp
           //    f     = flat
           //    ff    = double flat
           //    h     = natural
           //    u     = next accidental is under the ornament
           // any combination of these characters following a 
           // ornament is considered one element.
           //
           count++;
           index++;
           while (input[index] == 's' || input[index] == 'f' ||
                  input[index] == 'h' || input[index] == 'u') {
              output[outputIndex++] = input[index++];
           }
           break;

         //////////////////////////////////////////////////////////////
         // The following chars are uniquely SINGLE letter items:    //
         //                                                          //
         //                                                          // 
         case '-':                     // tie                        //
         case '(':                     // open  slur #1              //
         case ')':                     // close slur #1              //
         case '[':                     // open  slur #2              //
         case ']':                     // close slur #2              //
         case '{':                     // open  slur #3              //
         case '}':                     // close slur #3              //
         case 'z':                     // open  slur #4              //
         case 'x':                     // close slur #4              //
         case '*':                     // start written tuplet       //
         case '!':                     // end written tuplet         //
         case 'v':                     // up bow                     //
         case 'n':                     // down bow                   //
         case 'o':                     // harmonic                   //
         case 'O':                     // open string                //
         case 'Q':                     // thumb position             //
         case 'A':                     // accent (^)                 //
         case 'V':                     // accent (v)                 //
         case '>':                     // accent (>)                 //
         case '.':                     // staccatto                  //
         case '_':                     // tenuto                     //
         case '=':                     // detached tenuto            //
         case 'i':                     // spiccato                   //
         case '\'':                    // breath mark                //
         case 'F':                     // upright fermata            //
         case 'E':                     // inverted fermata           //
         case 'R':                     // rfz                        //
         case '^':                     // editorial accidental       //
         case '+':                     // cautionary accidental      //
            count++;                                                 //
            output[outputIndex++] = input[index++];                  //
            break;                                                   //
         //                                                          //
         //                                                          //
         //////////////////////////////////////////////////////////////
         case ' ':
            // ignore blank spaces and continue counting elements
            index++;
            break;
         default:
            cout << "Error: unknown additional notation: " 
                 << input[index] << endl;
            exit(1);
      }
      if (count != 0 || index >= 12) {
         finished = 1;
      }
   } // end of while (!finished) loop

   output[outputIndex] = '\0';
   return count;
}



////////////////////
//
// MuseRecord::zerase -- removes specified number of characters from
// 	the beginning of the string.
//
 
char* MuseRecord::zerase(char* output, int num) {
   int length = strlen(output);
   if (num >= length) {
      output[0] = '\0';
   } else {
      for (int i=num; i<=length; i++) {
         output[i-num] = output[i];
      }
   }
   return output;
}
      


// md5sum: 05e1e67705ebcb2bfa1afad9e504582c MuseRecord.cpp [20050403]
