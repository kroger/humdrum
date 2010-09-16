//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 21:44:58 PDT 1998
// Last Modified: Tue Jun 30 21:45:02 PDT 1998
// Last Modified: Thu Jul  1 16:28:54 PDT 1999
// Last Modified: Fri Jun  4 18:01:18 PDT 2010 (changed recordString to Array)
// Last Modified: Thu Jun 10 00:11:08 PDT 2010 (added functions)
// Filename:      ...humdrum++/src/MuseRecordBasic.cpp
// Syntax:        C++
// $Smake:        cc -g -c -I../include %f && rm %b.o
//

#include "MuseRecordBasic.h"
#include "Enum_muserec.h"
#include <string.h>


//////////////////////////////
//
// MuseRecordBasic::MuseRecordBasic --
//

MuseRecordBasic::MuseRecordBasic(void) {
   recordString.setSize(81);
   recordString[0] = '\0';
   recordString.setGrowth(1024);
   recordString.setSize(0);
   setType(E_muserec_unknown);
   lineindex = -1;

   absbeat.setValue(0,1);
   lineduration.setValue(0,1);
   noteduration.setValue(0,1);

   b40pitch = -100;
   nexttiednote = -1;
   lasttiednote = -1;
}


// default value: index = -1;
MuseRecordBasic::MuseRecordBasic(const char* aLine, int index) {
   recordString.setSize(81);
   recordString.setGrowth(1024);
   recordString.setSize(0);
   setLine(aLine);
   setType(E_muserec_unknown);
   lineindex = index;

   absbeat.setValue(0,1);
   lineduration.setValue(0,1);
   noteduration.setValue(0,1);

   b40pitch = -100;
   nexttiednote = -1;
   lasttiednote = -1;
}


MuseRecordBasic::MuseRecordBasic(MuseRecordBasic& aRecord) {
   *this = aRecord;
}



//////////////////////////////
//
// MuseRecordBasic::~MuseRecordBasic --
//

MuseRecordBasic::~MuseRecordBasic() {
   recordString.setSize(0);

   absbeat.setValue(0,1);
   lineduration.setValue(0,1);
   noteduration.setValue(0,1);

   b40pitch = -100;
   nexttiednote = -1;
   lasttiednote = -1;
}



//////////////////////////////
//
// MuseRecordBasic::extract -- extracts the character columns from the
//	storage string.  Appends a null character to the end of the
//	copied string.
//

void MuseRecordBasic::extract(char* output, int start, int end) {
   output[0] = '\0';
   int i;
   for (i=0; i<=end-start; i++) {
      if (i+start <= getLength()) {
         output[i] = getColumn(i+start);
      } else {
         output[i] = ' ';
      }
   }
   output[i] = '\0';
}



//////////////////////////////
//
// MuseRecordBasic::getColumn -- same as operator[] but with an
//	offset of 1 rather than 0.
//

char& MuseRecordBasic::getColumn(int columnNumber) {
   int realindex = columnNumber - 1;
   int length = recordString.getSize();
   if (realindex < 0 || realindex >= 80) {
      cerr << "Error trying to access column: " << columnNumber  << endl;
      exit(1);
   } else if (realindex >= getLength()) {
      recordString.setSize(realindex);
      for (int i=length; i<=realindex; i++) {
         recordString[i] = ' ';
      }
      recordString.setSize(recordString.getSize()+1);
      recordString[recordString.getSize()-1] = '\0';
      recordString.setSize(recordString.getSize()-1);
   }

   return recordString[realindex];
}



//////////////////////////////
//
// MuseRecordBasic::getLength -- returns the size of the
//	character string being stored.  A number between
//	0 and 80.
//

int MuseRecordBasic::getLength(void) const {
   return recordString.getSize();
}



//////////////////////////////
//
// MuseRecordBasic::getLine -- returns a pointer to data record
//

const char* MuseRecordBasic::getLine(void) {
   // install a hidden null character after text, then return char* 
   recordString.setSize(recordString.getSize()+1);
   recordString[recordString.getSize()-1] = '\0';
   recordString.setSize(recordString.getSize()-1);
   return recordString.getBase();
}



//////////////////////////////
//
// MuseRecordBasic::getType -- returns the type of the record.
//

int MuseRecordBasic::getType(void) const {
   return type;
}



//////////////////////////////
//
// MuseRecordBasic::operator=
//

MuseRecordBasic& MuseRecordBasic::operator=(MuseRecordBasic& aRecord) {
   // don't copy onto self
   if (&aRecord == this) {
      return *this;
   }

   setLine(aRecord.getLine());
   setType(aRecord.getType());
   lineindex = aRecord.lineindex;

   absbeat = aRecord.absbeat;
   lineduration = aRecord.lineduration;
   noteduration = aRecord.noteduration;

   b40pitch     = aRecord.b40pitch;
   nexttiednote = aRecord.nexttiednote;
   lasttiednote = aRecord.lasttiednote;

   return *this;
}


MuseRecordBasic& MuseRecordBasic::operator=(MuseRecordBasic* aRecord) {
   *this = *aRecord;
   return *this;
}


MuseRecordBasic& MuseRecordBasic::operator=(char* aLine) {
   setLine(aLine);
   setType(aLine[0]);
   lineindex = -1;

   absbeat.setValue(0,1);
   lineduration.setValue(0,1);
   noteduration.setValue(0,1);

   b40pitch     = -100;
   nexttiednote = -1;
   lasttiednote = -1;

   return *this;
}



//////////////////////////////
//
// MuseRecordBasic::operator[] -- character array offset from 0.
//

char& MuseRecordBasic::operator[](int index) {
   return getColumn(index+1);
}



//////////////////////////////
//
// MuseRecordBasic::setLine -- sets the record to a (new) string
//

void MuseRecordBasic::setLine(const char* aLine) {
   int len = strlen(aLine);
   recordString.setSize(len+1);
   strcpy(recordString.getBase(), aLine);
   // hide the null character:
   recordString.setSize(recordString.getSize()-1);

   // string lengths should not exceed 80 characters according
   // to MuseData standard, so maybe have a warning or error if exceeded.
}



//////////////////////////////
//
// MuseRecordBasic::setType -- sets the type of the record
//

void MuseRecordBasic::setType(int aType) {
   type = aType;
}



//////////////////////////////
//
// MuseRecordBasic::shrink -- removes trailing spaces in a MuseData record
//

void MuseRecordBasic::shrink(void) {
   int i = recordString.getSize() - 1;
   while (i >= 0 && recordString[i] == ' ') {
      recordString[i] = '\0';
      recordString.setSize(recordString.getSize()-1);
      i--;
   }
}



//////////////////////////////
//
// MuseRecordBasic::insertString --
//

void MuseRecordBasic::insertString(int index, const char* string) {
   int len = strlen(string);
   // make sure that record has text data up to the end of sring in
   // final location:
   (*this)[index+len] = ' ';
   int i;
   for (i=0; i<len; i++) {
      (*this)[i+index] = string[i];
   }
}


//////////////////////////////
//
// MuseRecordBasic::setString --
//

void MuseRecordBasic::setString(Array<char>& astring) {
   recordString = astring;
   char nullchar = '\0';
   int size = recordString.getSize();
   if ((size > 0) && (recordString[size-1] == nullchar)) {
      // chomp any null character at end of string
      recordString.setSize(size-1);
   }
   recordString.append(nullchar);
   recordString.setSize(recordString.getSize()-1);
}


//////////////////////////////
//
// MuseRecordBasic::setAbsBeat --
//


void MuseRecordBasic::setAbsBeat(RationalNumber value) {
   absbeat = value;
}


// default value botval = 1
void MuseRecordBasic::setAbsBeat(int topval, int botval) {
   absbeat.setValue(topval, botval);
}



//////////////////////////////
//
// MuseRecordBasic::getAbsBeat --
//

double MuseRecordBasic::getAbsBeat(void) {
   return absbeat.getFloat();
}


RationalNumber MuseRecordBasic::getAbsBeatR(void) {
   return absbeat;
}



//////////////////////////////
//
// MuseRecordBasic::setLineDuration -- set the duration of the line
//     in terms of quarter notes.
//

void MuseRecordBasic::setLineDuration(RationalNumber value) {
   lineduration = value;
}


// default value botval = 1
void MuseRecordBasic::setLineDuration(int topval, int botval) {
   lineduration.setValue(topval, botval);
}



//////////////////////////////
//
// MuseRecordBasic::getLineDuration -- set the duration of the line
//     in terms of quarter notes.
//

double MuseRecordBasic::getLineDuration(void) { 
   return lineduration.getFloat();
}


RationalNumber MuseRecordBasic::getLineDurationR(void) { 
   return lineduration;
}



//////////////////////////////
//
// MuseRecordBasic::setNoteDuration -- set the duration of the note
//     in terms of quarter notes.  If the line does not represent a note,
//     then the note duration should probably be 0...
//

void MuseRecordBasic::setNoteDuration(RationalNumber value) { 
   noteduration = value;
}


// default value botval = 1
void MuseRecordBasic::setNoteDuration(int topval, int botval) { 
   noteduration.setValue(topval, botval);
}



//////////////////////////////
//
// MuseRecordBasic::getNoteDuration -- get the duration of the note
//     in terms of quarter notes.  If the line does not represent a note,
//     then the note duration should probably be 0...
//

double MuseRecordBasic::getNoteDuration(void) { 
   return noteduration.getFloat();
}


RationalNumber MuseRecordBasic::getNoteDurationR(void) { 
   return noteduration;
}



//////////////////////////////
//
// MuseRecordBasic::setLineIndex --
//

void MuseRecordBasic::setLineIndex(int index) {
   lineindex = index;
}



//////////////////////////////
//
// MuseRecordBasic::isTied -- True if the note is tied to a note
//    before or after it. 
//  0 = no ties
//  1 = tied to previous note
//  2 = tied to future note
//  3 = tied to both previous and future note
//

int MuseRecordBasic::isTied(void) { 
   int output = 0;
   if (getLastTiedNoteLineIndex() >= 0) {
      output += 1;
   }

   if (getNextTiedNoteLineIndex() >= 0) {
      output += 2;
   }
	 
   return output;
}



//////////////////////////////
//
// MuseRecordBasic::getLastTiedNoteLineIndex --
//


int MuseRecordBasic::getLastTiedNoteLineIndex(void) { 
   return lasttiednote;
}



//////////////////////////////
//
// MuseRecordBasic::getNextTiedNoteLineIndex --
//


int MuseRecordBasic::getNextTiedNoteLineIndex(void) { 
   return nexttiednote;
}



//////////////////////////////
//
// MuseRecordBasic::setLastTiedNoteLineIndex --
//


void MuseRecordBasic::setLastTiedNoteLineIndex(int index) { 
   lasttiednote = index;
}



//////////////////////////////
//
// MuseRecordBasic::setNextTiedNoteLineIndex --
//


void MuseRecordBasic::setNextTiedNoteLineIndex(int index) { 
   nexttiednote = index;
}



//////////////////////////////
//
// MuseRecordBasic::setMarkupPitch -- set the base-40 pitch information
//   in the markup area.  Does not change the original pitch in
//   the text line of the data.
//

void MuseRecordBasic::setMarkupPitch(int aPitch) {
   b40pitch = aPitch;
}



//////////////////////////////
//
// MuseRecordBasic::getMarkupPitch -- get the base-40 pitch information
//   in the markup area.  Does not look at the original pitch in
//   the text line of the data. A negative value is a rest (or invalid).
//

int MuseRecordBasic::getMarkupPitch(void) {
   return b40pitch;
}




///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// operator<< 
//

ostream& operator<<(ostream& out, MuseRecordBasic& aRecord) {
   aRecord.shrink();  // have to shrink automatically because
                      // muse2ps program chokes on line 9 of header
		      // if it has more than one space on a blank line.
   out << aRecord.getLine();
   return out;
}





// md5sum: 17da8974becd492dfd9f275dfcc4004e MuseRecordBasic.cpp [20050403]
