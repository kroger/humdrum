//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 21:44:58 PDT 1998
// Last Modified: Tue Jun 30 21:45:02 PDT 1998
// Last Modified: Thu Jul  1 16:28:54 PDT 1999
// Last Modified: Fri Jun  4 18:01:18 PDT 2010 (changed recordString to Array)
// Last Modified: Thu Jun 10 00:11:08 PDT 2010 (added some functions)
// Last Modified: Fri Jan 21 06:00:27 PST 2011 (added get/setColumns)
// Filename:      ...humdrum++/src/MuseRecordBasic.cpp
// Syntax:        C++
// $Smake:        cc -g -c -I../include %f && rm %b.o
//

#include "MuseRecordBasic.h"
#include "Enum_muserec.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

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
   roundBreve = 0;
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
   roundBreve = 0;
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
   roundBreve = 0;
}



//////////////////////////////
//
// MuseRecordBasic::clear -- remove content of record.
//

void MuseRecordBasic::clear(void) {
   recordString.setSize(1);
   recordString[0] = '\0';
   recordString.setSize(0);
}



/////////////////////////////
//
// MuseRecordBasic::isEmpty -- returns true if only spaces on line
//

int MuseRecordBasic::isEmpty(void) {
   int i;
   for (i=0; i<recordString.getSize(); i++) {
      if (!isprint(recordString[i])) {
         continue;
      }
      if (!isspace(recordString[i])) {
         return 0;
      }
   }
   return 1;
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
   // originally the limiit for data columns was 80:
   //if (realindex < 0 || realindex >= 80) {
   //the new limit is somewhere above 900, but limit to 180
   if (realindex < 0 || realindex >= 180) {
      cerr << "Error trying to access column: " << columnNumber  << endl;
      cerr << "CURRENT DATA: ===============================" << endl;
      cerr << (*this);
      exit(1);
   } else if (realindex >= recordString.getSize()) {
      recordString.setSize(realindex+1);
      for (int i=length; i<=realindex; i++) {
         recordString[i] = ' ';
      }
      // add a hidden null character after the real characters.
      recordString.setSize(recordString.getSize()+1);
      recordString[recordString.getSize()-1] = '\0';
      recordString.setSize(recordString.getSize()-1);
      // can't automatically grow; otherwise, the null character
      // will disappear...
      recordString.setGrowth(0);
      if ((int)strlen(recordString.getBase()) != recordString.getSize()) {
         cerr << "Funny error in MuseRecordBasic::getColumn: null found" << endl;
      }
   }

   return recordString[realindex];
}



//////////////////////////////
//
// MuseRecordBasic::getColumns --
//

void MuseRecordBasic::getColumns(Array<char>& data, int startcol, int endcol) {
   int charcount = endcol - startcol + 1;
   if (charcount <= 0) {
      data.setSize(1);
      data[0] = '\0';
      return;
   }
   data.setSize(charcount + 1);
   data.allowGrowth(0);
   
   int i;
   int ii = 0;
   for (i=startcol; i<=endcol; i++) {
      data[ii++] = getColumn(i);
   }
   data[ii] = '\0';
}



//////////////////////////////
//
// MuseRecordBasic::setColumns --
//

void MuseRecordBasic::setColumns(Array<char>& data, int startcol, int endcol) {
   if (startcol > endcol) {
      int temp = startcol;
      startcol = endcol;
      endcol = temp;
   }

   int dsize = data.getSize();
   if (data[dsize-1] == '\0') {
      // don't insert any null-termination of the string.
      dsize--;
   }

   getColumn(endcol) = ' '; // allocate space if not already done
   int i;
   int ii;
   for (i=startcol; i<=endcol; i++) {
      ii = i - startcol;
      if (ii < dsize) {
         getColumn(i) = data[ii];
      } else {
         break;
      }
   }

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
// MuseRecordBasic::setTypeGraceNote -- put a "g" in the first column.
//    shift pitch information over if it exists?  Maybe later.
//    Currently will destroy any pitch information.
//

void MuseRecordBasic::setTypeGraceNote(void) {
   setType(E_muserec_note_grace);
   (*this)[0] = 'g';
}



//////////////////////////////
//
// MuseRecordBasic::setTypeGraceChordNote -- put a "g" in the first column,
//    and a space in the second column.  Shift pitch information over if 
//    it exists?  Maybe later.  Currently will destroy any pitch information.
//

void MuseRecordBasic::setTypeGraceChordNote(void) {
   setType(E_muserec_note_grace_chord);
   (*this)[0] = 'g';
   (*this)[1] = ' ';
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

void MuseRecordBasic::insertString(int column, const char* string) {
   int len = strlen(string);
   int index = column - 1;
   // make sure that record has text data up to the end of sring in
   // final location by preallocating the end location of string:
   (*this)[index+len-1] = ' ';
   int i;
   for (i=0; i<len; i++) {
      (*this)[i+index] = string[i];
   }
}



//////////////////////////////
//
// MuseRecordBasic::insertStringRight -- Insert string right-justified
//    starting at given index.
//

void MuseRecordBasic::insertStringRight(int column, const char* string) {
   int len = strlen(string);
   int index = column - 1;
   // make sure that record has text data up to the end of sring in
   // final location by preallocating the end location of string:
   (*this)[index] = ' ';
   int i;
   int ii;
   for (i=0; i<len; i++) {
      ii = index - i;
      if (ii < 0) {
         break;
      }
      (*this)[ii] = string[len-i-1];
   }
}



//////////////////////////////
//
// MuseRecordBasic::appendString -- add a string to the end of the current
//     data in the record.
//

void MuseRecordBasic::appendString(const char* astring) {
   insertString(getLength()+1, astring);
}



//////////////////////////////
//
// MuseRecord::appendInteger -- Insert an integer after the last character 
//     in the current line.
//

void MuseRecordBasic::appendInteger(int value) {
   char buffer[32] = {0};
   sprintf(buffer, "%d", value);
   insertString(getLength()+1, buffer);
}



//////////////////////////////
//
// MuseRecord::appendRational -- Insert a rational after the last character 
//     in the current line.
//

void MuseRecordBasic::appendRational(RationalNumber& value) {
   SSTREAM tout;
   value.printTwoPart(tout);
   tout << ends;
   insertString(getLength()+1, tout.CSTRING);
}



//////////////////////////////
//
// MuseRecord::append -- append multiple objects in sequence
// from left to right onto the record.  The format contains
// characters with two possibilities at the moment:
//    "i": integer value
//    "s": string value
//

void MuseRecordBasic::append(const char* format, ...) {
   va_list valist;
   int     i;

   va_start(valist, format);

   union Format_t {
      int   i;
      char *s;
      int  *r;  // array of two integers for rational number
   } FormatData;

   RationalNumber rn;

   for (i=0; format[i] != '\0'; i++) {
      switch (format[i]) {   // Type to expect.
         case 'i':
            FormatData.i = va_arg(valist, int);
            appendInteger(FormatData.i);
            break;

         case 's':
            FormatData.s = va_arg(valist, char *);
            if (strlen(FormatData.s) > 0) {
               appendString(FormatData.s);
            }
            break;

         case 'r':
             FormatData.r = va_arg(valist, int *);
             rn.setValue(FormatData.r[0], FormatData.r[1]);
             appendRational(rn);
            break;

         default:
            // don't put any character other than "i", "r" or "s" 
            // in the format string
            break;
      }
   }

   va_end(valist);
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
// MuseRecordBasic::setRoundedBreve -- set double whole notes rounded flag.
//

void MuseRecordBasic::setRoundedBreve(void) {
   roundBreve = 1;
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




//////////////////////////////
//
// MuseRecordBasic::cleanLineEnding -- remove spaces at the end of the
//    line;
//

void MuseRecordBasic::cleanLineEnding(void) {
   int i = recordString.getSize()-1;
   // don't remove first space on line.
   while ((i > 0) && (recordString[i] == ' ')) {  
      recordString[i] = '\0';
      recordString.setSize(recordString.getSize() - 1);
      i = recordString.getSize() - 1;
   }

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
