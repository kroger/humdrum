//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May 18 13:52:59 PDT 1998
// Last Modified: Thu Jul  1 16:20:41 PDT 1999
// Last Modified: Thu Apr 13 18:34:28 PDT 2000 added generic interpretation
// Last Modified: Fri May  5 13:12:21 PDT 2000 added sub-spine access
// Last Modified: Fri Oct 13 12:45:45 PDT 2000 added spine tracing vars
// Last Modified: Wed May 16 18:20:55 PDT 2001 added changeToken
// Last Modified: Sun Mar 24 12:10:00 PST 2002 small changes for visual c++
// Last Modified: Wed Apr  3 08:13:29 PST 2002 allow DOS newline input
// Last Modified: Mon May 17 12:46:36 PDT 2004 added getSpinePrediction
// Last Modified: Wed Jun  2 15:50:13 PDT 2004 setSpineWidth adjusts interp
// Last Modified: Tue Apr 21 10:02:00 PDT 2009 fixed comment prob. in setLine
// Last Modified: Tue Apr 28 14:34:13 PDT 2009 added isTandem
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 renamed SigCollection class
// Last Modified: Mon Jun 15 15:55:22 PDT 2009 fixed bug in getPrimaryTrack
// Last Modified: Thu Jun 18 15:29:49 PDT 2009 modified getExInterp behavior
// Last Modified: Thu Jun 18 15:57:53 PDT 2009 added hasSpines()
// Last Modified: Mon Nov 23 14:30:35 PST 2009 fixed equalDataQ()
// Last Modified: Sat May 22 10:29:39 PDT 2010 added RationalNumber
// Last Modified: Sun Dec 26 12:18:34 PST 2010 added setToken
// Last Modified: Mon Jul 30 16:10:45 PDT 2012 added setSize and setAllFields
// Last Modified: Mon Dec 10 10:14:08 PST 2012 added Array<char> getToken
// Filename:      ...sig/src/sigInfo/HumdrumRecord.cpp
// Webpage:       http://sig.sapp.org/src/sigInfo/HumdrumRecord.cpp
// Syntax:        C++ 
//
// Description:   Stores the data for one line in a HumdrumFile and
//                segments data into spines.
//

#include "Convert.h"
#include "HumdrumRecord.h"
#include "PerlRegularExpression.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifndef OLDCPP
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #ifdef VISUAL
      #include <strstrea.h>   /* for Windows 95 */
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif
   


//////////////////////////////
//
// HumdrumRecord::HumdrumRecord --
//

HumdrumRecord::HumdrumRecord(void) {
   duration = 0.0;
   durationR.zero();
   meterloc = 0.0;
   meterlocR.zero();
   absloc   = 0.0;
   abslocR.zero();
   spinewidth = 0;

   type = E_unknown;
   recordString = new char[1];
   recordString[0] = '\0';
   modifiedQ = 0;
   lineno = -1;

   recordFields.allowGrowth(1);
   recordFields.setSize(32);   
   recordFields.setGrowth(132);   
   recordFields.setSize(0);   

   interpretation.allowGrowth(1);
   interpretation.setSize(32);
   interpretation.setGrowth(132);
   interpretation.setSize(0);

   spineids.allowGrowth(1);
   spineids.setSize(32);
   spineids.setGrowth(132);
   spineids.setSize(0);

   dotline.setSize(32);
   dotline.setGrowth(132);
   dotline.setSize(0);

   dotspine.setSize(32);
   dotspine.setGrowth(132);
   dotspine.setSize(0);
}


HumdrumRecord::HumdrumRecord(const char* aLine, int aLineNum) {
   duration = 0.0;
   durationR.zero();
   meterloc = 0.0;
   meterlocR.zero();
   absloc   = 0.0;
   abslocR.zero();
   spinewidth = 0;

   lineno = aLineNum;
   type = determineType(aLine);
   recordString = new char[1];
   recordString[0] = '\0';
   modifiedQ = 0;
   interpretation.allowGrowth(1);
   interpretation.setSize(32);
   interpretation.setGrowth(132);
   interpretation.setSize(0);
   recordFields.allowGrowth(1);
   recordFields.setSize(32);
   recordFields.setGrowth(132);
   recordFields.setSize(0);
   spineids.allowGrowth(1);
   spineids.setSize(32);
   spineids.setGrowth(132);
   spineids.setSize(0);
   dotline.allowGrowth(1);
   dotline.setSize(0);
   dotline.setSize(32);
   dotline.setGrowth(132);
   dotspine.allowGrowth(1);
   dotspine.setSize(32);
   dotspine.setGrowth(132);
   dotspine.setSize(0);
   setLine(aLine);
}


HumdrumRecord::HumdrumRecord(const HumdrumRecord& aRecord) {
   duration = aRecord.duration;
   durationR= aRecord.durationR;
   meterloc = aRecord.meterloc;
   meterlocR= aRecord.meterlocR;
   absloc   = aRecord.absloc;
   abslocR  = aRecord.abslocR;
   spinewidth = aRecord.spinewidth;
   type = aRecord.type;
   lineno = aRecord.lineno;
   recordString = new char[strlen(aRecord.recordString)+1];
   strcpy(recordString, aRecord.recordString);
   modifiedQ = aRecord.modifiedQ;
   interpretation.allowGrowth();
   interpretation.setSize(aRecord.interpretation.getSize());
   recordFields.allowGrowth();
   recordFields.setSize(aRecord.recordFields.getSize());
   spineids.setSize(aRecord.spineids.getSize());
   dotline.setSize(aRecord.dotline.getSize());
   dotspine.setSize(aRecord.dotspine.getSize());

   int i;
   for (i=0; i<aRecord.recordFields.getSize(); i++) {
      interpretation[i] = aRecord.interpretation[i];
      recordFields[i] = new char[strlen(aRecord.recordFields[i]) + 1];
      strcpy(recordFields[i], aRecord.recordFields[i]);
      spineids[i] = new char[strlen(aRecord.spineids[i]) + 1];
      strcpy(spineids[i], aRecord.spineids[i]);
      dotline[i] = aRecord.dotline[i];
      dotspine[i] = aRecord.dotspine[i];
   }
}



//////////////////////////////
//
// HumdrumRecord::~HumdrumRecord --
//

HumdrumRecord::~HumdrumRecord() {
   if (recordString != NULL) {
      delete [] recordString;
      recordString = NULL;
   }
   int i;
   for (i=0; i<recordFields.getSize(); i++) {
      if (recordFields[i] != NULL) {
         delete [] recordFields[i];
         recordFields[i] = NULL;
      }
   }
   recordFields.setSize(0);

   for (i=0; i<spineids.getSize(); i++) {
      if (spineids[i] != NULL) {
         delete [] spineids[i];
         spineids[i] = NULL;
      }
   }
   spineids.setSize(0);

   dotline.setSize(0);
   dotspine.setSize(0);

   lineno = -1;
   spinewidth = 0;
}



//////////////////////////////
//
// HumdrumRecord::appendField --
//     default value: anInterp = E_unknown
//	

void HumdrumRecord::appendField(const char* aField, int anInterp, 
      const char* spinetrace) {
   insertField(-1, aField, anInterp, spinetrace);
}



//////////////////////////////
//
// HumdrumRecord::appendFieldEI --
//     default value: anInterp = "**unknown"
//     default value: spinetrace = ""
//	

void HumdrumRecord::appendFieldEI(const char* aField, const char* anInterp, 
      const char* spinetrace) {
   insertField(-1, aField, anInterp, spinetrace);
}



//////////////////////////////
//
// HumdrumRecord::changeField -- changes the contents of a field
//

void HumdrumRecord::changeField(int aField, const char* aString) {
   // don't change self
   if (aString == recordFields[aField]) {
      return;
   }

   if (recordFields[aField] != NULL) {
      delete [] recordFields[aField];
      recordFields[aField] = NULL;
   }
   recordFields[aField] = new char[strlen(aString) + 1];
   strcpy(recordFields[aField], aString);
   
   modifiedQ = 1;
}



//////////////////////////////
//
// HumdrumRecord::copySpineInfo -- 
//     Default value: line = 0;
//

void HumdrumRecord::copySpineInfo(char** info, int size, int line) {
   if (size != getFieldCount()) {
      cout << "Error: new spine information is not the right size" << endl;
      int i;
      for (i=0; i<size; i++) {
         cout << info[i];
         if (i<size-1) {
            cout << '\t';
         }
      }
      cout << endl;
      cout << "Required size is: " << getFieldCount() << endl;
      cout << "New size would be: " << size << endl;
      cout << "Input line of error: " << line << endl;
      cout << "Line: " << (*this) << endl;
      exit(1);
   }

   // delete the old spine info
   int i;
   for (i=0; i<size; i++) {
      if (spineids[i] != NULL) {
         delete [] spineids[i];
         spineids[i] = new char[strlen(info[i]) + 1];
         strcpy(spineids[i], info[i]);
      }
   }
}


void HumdrumRecord::copySpineInfo(HumdrumRecord& aRecord, int line) {
   int size = aRecord.spineids.getSize();
   char** base = aRecord.spineids.getBase();
   copySpineInfo(base, size, line);
}

void HumdrumRecord::copySpineInfo(SigCollection<char*>& aCollection, int line) {
   int size = aCollection.getSize();
   char** base = aCollection.getBase();
   copySpineInfo(base, size, line);
}



//////////////////////////////
//
// HumdrumRecord::equalDataQ -- return true if all tokens on a line
//      match to aValue.
//

int HumdrumRecord::equalDataQ(const char* aValue) {
   int output = 1;
   for (int i=0; i<getFieldCount(); i++) {
      if (strcmp(recordFields[i], aValue) != 0) {
         output = 0;
         break;
      }
   }
 
   return output;
}
   
 


//////////////////////////////
//
// HumdrumRecord::equalFieldsQ -- returns true if all record fields
//	are identical
//

int HumdrumRecord::equalFieldsQ(void) {
   int output = 1;
   for (int i=1; i<getFieldCount(); i++) {
      if (strcmp(recordFields[i], recordFields[0]) != 0) {
         output = 0;
         break;
      }
   }
 
   return output;
}



//
// just check to see if all exclusive interpretation fields are the same
//

int HumdrumRecord::equalFieldsQ(int anInterp) {
   int output = 1;
   int index = 0;
   while (index < getFieldCount() && getExInterpNum(index) != anInterp) {
      index++;
   }
   for (int i=index+1; i<getFieldCount(); i++) {
      if (getExInterpNum(i) == anInterp) {
         if (strcmp(recordFields[index], recordFields[i]) != 0) {
            output = 0;
            break;
         }
      }
   }

   return output;
}


int HumdrumRecord::equalFieldsQ(const char* anInterp) {
   return equalFieldsQ(Convert::exint.getValue(anInterp));
}


int HumdrumRecord::equalFieldsQ(int anInterp, const char* compareString) {
   int output = 1;
   for (int i=0; i<getFieldCount(); i++) {
      if (getExInterpNum(i) == anInterp) {
         if (strcmp(recordFields[i], compareString) != 0) {
            output = 0;
            break;
         }
      }
   }
   return output;
}
 

int HumdrumRecord::equalFieldsQ(const char* anInterp, 
      const char* compareString) {
   return equalFieldsQ(Convert::exint.getValue(anInterp), compareString);
}



//////////////////////////////
//
// getBibKey -- return the key value of a bibliographic record
//    default value: maxsize = 0
//

char* HumdrumRecord::getBibKey(char* buffer, int maxsize) {
   if (!isBibliographic()) {
      buffer[0] = '\0';
      return buffer;
   }

   if (maxsize > 0) {
      strncpy(buffer, recordFields[0]+3, maxsize);
   } else {
      strcpy(buffer, recordFields[0]+3);
   }
   int i;
   int length = strlen(buffer);
   for (i=0; i<length; i++) {
      if (buffer[i] == ':') {
         buffer[i] = '\0';
         return buffer;
      }
   }

   buffer[0] = '\0';
   return buffer;
}



//////////////////////////////
//
// getBibValue -- return the key value of a bibliographic record
//    default value: maxsize = 0
//

char* HumdrumRecord::getBibValue(char* buffer, int maxsize) {

   if (!isBibliographic()) {
      buffer[0] = '\0';
      return buffer;
   }

   PerlRegularExpression pre;
   if (pre.search(recordString, "^!!![^:]+:\\s*(.*)\\s*$", "")) {
      strcpy(buffer, pre.getSubmatch(1));
      return buffer;
   }

/*
   int length = strlen(recordString);
   int i;
   int j;

   for (i=3; i<length; i++) {
      if (recordString[i] == ':') {
         i++;
         while ((i < length) && (isspace(recordString[i]))) {
            i++;
         }
         strncpy(buffer, recordString + i, maxsize);
         length = strlen(buffer);
         for (j=length-1; j>=0; j++) {
            if (isspace(buffer[j])) {
               buffer[j] = '\0';
            } else {
               break;
            }
         }
         return buffer;
      }

   }
*/
   buffer[0] = '\0';
   return buffer;
}



//////////////////////////////
//
// HumdrumRecord::getAbsBeat -- returns the absolute beat location
//    in the file.  This value is by default 0, but can be set
//    manally with the setAbsBeat() function, or by calling
//    HumdrumFile::analyzeRhythm().  abslocR() returns the rational
//    value of the absolute beat.
//

double HumdrumRecord::getAbsBeat(void) const { 
   return abslocR.getFloat();
}


RationalNumber HumdrumRecord::getAbsBeatR(void) const { 
   return abslocR;
}



//////////////////////////////
//
// HumdrumRecord::getDotLine -- return the line to which a "."
//   value is place holding.
//

int HumdrumRecord::getDotLine(int index) {  
   return dotline[index];
}



//////////////////////////////
//
// HumdrumRecord::getDotSpine -- return the spine to which a "."
//   value is place holding.
//

int HumdrumRecord::getDotSpine(int index) {  
   return dotspine[index];
}



//////////////////////////////
//
// HumdrumRecord::getBeat -- returns the metrical beat location
//    in the file.  This value is by default 0, but can be set
//    manally with the setBeat() function, or by calling
//    HumdrumFile::analyzeRhythm().
//

double HumdrumRecord::getBeat(void) const { 
   return meterlocR.getFloat();
}


RationalNumber HumdrumRecord::getBeatR(void) const { 
   return meterlocR;
}



//////////////////////////////
//
// HumdrumRecord::getDuration -- returns the duration of the current
//    HumdrumRecord line in a HumdrumFile.  This value is by default 0, 
//    but can be set manally with the setAbsBeat() function, or by 
//    calling HumdrumFile::analyzeRhythm().
//

double HumdrumRecord::getDuration(void) const { 
   return durationR.getFloat();
}

RationalNumber HumdrumRecord::getDurationR(void) const { 
   return durationR;
}



//////////////////////////////
//
// HumdrumRecord::getExInterpNum -- returns the exclusive interpretation
//	enumeration assigned to the given data field.
//

int HumdrumRecord::getExInterpNum(int index) const {
   if (index >= interpretation.getSize()) {
      cout << "Error: accessing too large a spine field number A: " << index 
           << endl;
      exit(1);
   }

   return interpretation[index];
}


//////////////////////////////
//
// HumdrumRecord::getExInterp -- returns the exclusive interpretation
//	name assigned to the given data field, such as "**kern".
//	If you ask for the exclusive interpretation of a non-spine
//	data line, an empty string will be returned.
//

const char* HumdrumRecord::getExInterp(int index) const {
   if (index >= interpretation.getSize()) {
      if(interpretation.getSize() == 0) {
         // return "" for lines such as Global Comments which are
         // not spine specific.
         return "";
      } else {
         cout << "Error: accessing too large a spine field number B: " << index
	      << endl;;
         cout << "Size of interpretation list: " << interpretation.getSize() 
	      << endl;
         cout << "But you tried to access index: " << index << endl;
         if (recordString != NULL) {
            cout << "Line is: " << recordString << endl;
         }
         exit(1);
      }
   }
   
   return Convert::exint.getName(interpretation[index]);
}



//////////////////////////////
//
// HumdrumRecord::getFieldCount -- returns the number of data
//	fields in the record.
//

int HumdrumRecord::getFieldCount(void) const {
   return recordFields.getSize();
}


int HumdrumRecord::getFieldCount(const char* exinterp) const {
   return getFieldCount(Convert::exint.getValue(exinterp));
}


int HumdrumRecord::getFieldCount(int exinterp) const {
   int i;
   int count = 0;
   for (i=0; i<recordFields.getSize(); i++) {
      if (getExInterpNum(i) == exinterp) {
         count++;
      }
   }

   return count;
}



//////////////////////////////
//
// HumdrumRecord::getFieldsByExInterp --  Returns the number of fields on the line which
//     belong to a particular exclusive interpretation.
//

int HumdrumRecord::getFieldsByExInterp(Array<int>& fields, const char* exinterp) {
   HumdrumRecord& aRecord = *this;
   fields.setSize(aRecord.getFieldCount());
   fields.setSize(0);

   int i;
   for (i=0; i<aRecord.getFieldCount(); i++) {
      if (aRecord.isExInterp(i, exinterp)) {
         fields.append(i);
      }
   }
   return fields.getSize();
}



//////////////////////////////
//
// HumdrumRecord::getTracksByExInterp --  Returns the number of fields on the line which
//     belong to a particular exclusive interpretation.
//

int HumdrumRecord::getTracksByExInterp(Array<int>& tracks, const char* exinterp) {
   HumdrumRecord& aRecord = *this;
   tracks.setSize(aRecord.getFieldCount());
   tracks.setSize(0);

   int i;
   int track;
   for (i=0; i<aRecord.getFieldCount(); i++) {
      if (aRecord.isExInterp(i, exinterp)) {
         track = aRecord.getPrimaryTrack(i);
         tracks.append(track);
      }
   }
   return tracks.getSize();
}



//////////////////////////////
//
// HumdrumRecord::getLine -- returns a pointer to data record
//

const char* HumdrumRecord::getLine(void) {
   if (modifiedQ == 0) {
      return recordString;
   } else {
      makeRecordString();
      return recordString;
   }
}



//////////////////////////////
//
// HumdrumRecord::getLineNum -- returns the assigned file line number
//     for this record.
//

int HumdrumRecord::getLineNum(void) const {
   return lineno;
}



//////////////////////////////
//
// HumdrumRecord::getSpinePrediction -- return the number of spines
//    the following line in a Humdrum file should have based on the
//    information found in this record.
//

int HumdrumRecord::getSpinePrediction(void) {
   if (getType() != E_humrec_interpretation) {
      return getFieldCount();
   }

   int vcount = 0;
   int mcount = 0;
   int scount = 0;

   int i;
   for (i=0; i<getFieldCount(); i++) {
      if (strcmp((*this)[i], "*v") == 0) {
         vcount++;
      } else if (strcmp((*this)[i], "*^") == 0) {
         scount++;
      } else if (strcmp((*this)[i], "*-") == 0) {
         mcount++;
      }
   }

   int output = getFieldCount();
   output = output - mcount + scount;
   if (vcount > 1) {
      output = output - vcount + 1;
   }
   return output;
}



//////////////////////////////
//
// HumdrumRecord::getTrack -- returns 0 if invalid,
//   otherwise starts indexing at 1.
//

double HumdrumRecord::getTrack(int spineNumber) {
   const char* info = getSpineInfo(spineNumber);
   int i = 0;
   while (info[i] != '\0' && !isdigit(info[i])) {
      i++;
   }
   double output;
   int inttrack;
   sscanf(&info[i], "%d", &inttrack);
   output = inttrack;
   i++;
   int subtrack = 0;
   if (info[i] != '\0') {
      i++;
      if (isalpha(info[i])) {
         subtrack = tolower(info[i]) - 'a';
         output += subtrack/1000.0;
      }
   }
  
   return output;
}



//////////////////////////////
//
// HumdrumRecord::getTrackColumn -- Search for the first column which contains
//     the matches the given track.  Return -1 if no matching track found.
//

int HumdrumRecord::getTrackColumn(int track) {
   int i;
   int primarytrack;
   for (i=0; i<getFieldCount(); i++) {
      primarytrack = getPrimaryTrack(i);
      if (primarytrack == track) {
         return i;
      }
   }
   return -1;
}



//////////////////////////////
//
// HumdrumRecord::getPrimaryTrack -- returns 0 if invalid,
//   otherwise starts indexing at 1.  Trying to get the primary
//   track number of a non-spine line type will return 0.  For example,
//   global comments do not belong to specific spines, and therefore
//   have a primary track of 0. 
//

int HumdrumRecord::getPrimaryTrack(int spineNumber) {
   if (spineNumber < 0 || spineNumber >= getFieldCount()) {
      return 0;
   }
   const char* info = getSpineInfo(spineNumber);
   int i = 0;
   while (info[i] != '\0' && !isdigit(info[i])) {
      i++;
   }
   int track = 0;
   if (!sscanf(&info[i], "%d", &track)) {
      track = 0;
   }
   return track;
}



//////////////////////////////
//
// HumdrumRecord::getSpineInfo --
//

const char* HumdrumRecord::getSpineInfo(int index) const {
   return spineids[index];
}



//////////////////////////////
//
// HumdrumRecord::getSpineWidth -- returns the number of spines
//	in the data at the current point in a Humdrum File.
//	Useful to know if trying to generate data during a
//	global comment or bibliographic record.
//

int HumdrumRecord::getSpineWidth(void) {
   return spinewidth;
}


//////////////////////////////
//
// changeToken --
//     default value: separator = ' '
//

void HumdrumRecord::changeToken(int spineIndex, int tokenIndex,  
      const char* newtoken, char separator) {
   HumdrumRecord& record = *this;
   char separatorstr[2] = {0};
   separatorstr[0] = separator;

   char *buff;
   buff = new char[strlen(record[spineIndex]) + strlen(newtoken) + 1];
   buff[0] = '\0';
   char* oldtoken = strtok((char*)record[spineIndex], separatorstr);
   int token = 0;
   while (oldtoken != NULL) {
      if (token == tokenIndex) {
         strcat(buff, newtoken);
      } else {
         strcat(buff, oldtoken);
      }

      token++;
      oldtoken = strtok(NULL, separatorstr);
      if (oldtoken != NULL) {
         strcat(buff, separatorstr);
      }
   }

   delete [] recordFields[spineIndex];
   recordFields[spineIndex] = buff;
   modifiedQ = 1;
}



//////////////////////////////
//
// HumdrumRecord::getTokenCount -- returns the number of 
//    tokens in the particular spine (1 + the number of spaces)
//    Default value: separator = ' '.
//

int HumdrumRecord::getTokenCount(int fieldIndex, char separator) {
   if (globalCommentQ() || nullQ()) {
      return 0;
   }
   if (localCommentQ() || interpretationQ()) {
      return 1;
   }

   int count = 1;
   const char* string = (*this)[fieldIndex];
   int index = 0;
   while (string[index] != '\0') {
      if (string[index] == separator) {
         count++;
      }
      index++;
   }
   return count;
}



//////////////////////////////
//
// HumdrumRecord::getToken -- returns the filled buffer.
//    Puts the specified subtoken into buffer, or null if
//    subtoken is null.  If buffersize is positive, then
//    do not add more than buffersize characters to buffer.
//    Default values: buffersize = -1, separator = ' '.
//

char* HumdrumRecord::getToken(char* buffer, int fieldIndex, int tokenIndex,
      int buffersize, char separator) {
   
   if (buffersize < 1) {
      buffersize = 0x7fffffff;
   }
  
   char sepstring[2];
   sepstring[0] = separator;
   sepstring[1] = '\0';

   int location = 0;
   const char* string = (*this)[fieldIndex];
   // char temp[strlen(string) + 1];  // can't do in MS Visual C++ 6.0
   static char temp[1024] = {0};      // doing this instead
   strcpy(temp, string);
   char *current = NULL;
   current = strtok(temp, sepstring);
   while (current != NULL && location < tokenIndex) {
      current = strtok(NULL, sepstring);
      location++;
   }

   if (current == NULL) {
      buffer[0] = '\0';
      return buffer;
   }

   int length = strlen(current);
   if (length > buffersize) {
      length = buffersize - 1;
   }

   int i;
   for (i=0; i<length; i++) {
      buffer[i] = current[i];
   }
   buffer[length] = '\0'; 
   return buffer;
}


char* HumdrumRecord::getToken(Array<char>& buffer, int fieldIndex, 
      int tokenIndex, int buffersize, char separator) {
   HumdrumRecord& arecord = *this;
   int len = strlen(arecord[fieldIndex]) + 1 + 4;
   buffer.setSize(len);
   arecord.getToken(buffer.getBase(), fieldIndex, tokenIndex, 
      len, separator);
   len = strlen(buffer.getBase()) + 1;
   buffer.setSize(len);
   return buffer.getBase();
}



//////////////////////////////
//
// HumdrumRecord::getType -- returns the type of the record.
//

int HumdrumRecord::getType(void) const {
   return type;
}



//////////////////////////////
//
// HumdrumRecord::is* --
//

int HumdrumRecord::isEmpty(void) const { 
   if (getType() == E_humrec_empty) {
      return 1;
   } else {
      return 0;
   }
}
int HumdrumRecord::isData(void) const { return dataQ(); }
int HumdrumRecord::isMeasure(void) const { return measureQ(); }
int HumdrumRecord::isLocalComment(void) const { return localCommentQ(); }
int HumdrumRecord::isGlobalComment(void) const { return globalCommentQ(); }
int HumdrumRecord::isInterpretation(void) const { return interpretationQ(); }
int HumdrumRecord::isBibliographic(void) const { return bibRecordQ(); }

int HumdrumRecord::hasSpines(void) const { 
   return isData() || isMeasure() || isLocalComment() || isInterpretation();
}

int HumdrumRecord::isComment(void) const { 
   return isBibliographic() | isGlobalComment() | isLocalComment();
}

int HumdrumRecord::isTandem(void) const {
   if (!isInterpretation()) {
      // can't be a tandem interpretation if it is not an interpretation...
      return 0;
   }
	 
   int i;
   for (i=0; i<getFieldCount(); i++) {
      if (strcmp(recordFields[i], "*^") == 0) {
         return 0; 
      }
      if (strcmp(recordFields[i], "*v") == 0) {
         return 0;
      }
      if (strcmp(recordFields[i], "*x") == 0) {
         return 0;
      }
      if (strncmp(recordFields[i], "**", 2) == 0) {
         return 0;
      }
      if (strcmp(recordFields[i], "*-") == 0) {
         // spine terminator operator
         return 0;
      }
      if (strcmp(recordFields[i], "*+") == 0) {
         // spine addition operator
         return 0;
      }
 
   }
 
   return 1;  // no spine-manipulators or exclusive interpretations found
}


int HumdrumRecord::isSpineManipulator(void) {
   return isInterpretation() && !isTandem();
}


int HumdrumRecord::isSpineManipulator(int index) {
   HumdrumRecord& aRecord = *this;
   if (strcmp(aRecord[index], "*x") == 0) {
      return 1;   // spine exchanger
   } else if (strcmp(aRecord[index], "*^") == 0) {
      return 1;   // spine splitter
   } else if (strcmp(aRecord[index], "*v") == 0) {
      return 1;   // spine joiner
   }

   // ignoring *+, *-, and ** for now
   return 0;
}


int HumdrumRecord::isExInterp(int index, const char* string) {
   HumdrumRecord& aRecord = *this;
   return !strcmp(aRecord.getExInterp(index), string);
}



//////////////////////////////
//
// HumdrumRecord::isOriginalClef -- returns true if a clef, but prefixed
//     with "o" to indicate the clef in the original source.
//

#define REGEX_ISORIGINALCLEF "^\\*oclef[CFG]v?\\d+" 

int HumdrumRecord::isOriginalClef(int index) {
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;
   if (pre.search(aRecord[index], "^\\*oclef[CFG]v?\\d+")) {
      return 1;
   } 
   if (strcmp("*oclefX", aRecord[index]) == 0) {
      //percussion clef
      return 1;
   }
   if (strcmp("*oclef-", aRecord[index]) == 0) {
      //no clef (explicit)
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllOriginalClef(void) {
   return HumdrumRecord::isParticularType(REGEX_ISORIGINALCLEF, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isClef -- returns true if the given cell is a
//    clef tandem interpretation record.
//

#define REGEX_ISCLEF "^\\*clef[CFG]v?\\d+" 

int HumdrumRecord::isClef(int index) {
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;
   if (pre.search(aRecord[index], REGEX_ISCLEF)) {
      return 1;
   } 
   if (strcmp("*clefX", aRecord[index]) == 0) {
      //percussion clef
      return 1;
   }
   if (strcmp("*clef-", aRecord[index]) == 0) {
      //no clef (explicit)
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllClef(void) {
   return HumdrumRecord::isParticularType(REGEX_ISCLEF, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isParticularType --  Only checks one type of 
//    exclusive interpretation (presumably **kern data).
//

int HumdrumRecord::isParticularType(const char* regexp, const char* exinterp) {
   int output = 1;
   int j;
   int count = 0;
   PerlRegularExpression pre;
   HumdrumRecord& aRecord = *this;
   for (j=0; j<aRecord.getFieldCount(); j++) {
      if (!aRecord.isExInterp(j, "**kern")) {
         continue;
      }
      if ((strcmp(aRecord[j], "!") == 0)|| 
          (strcmp(aRecord[j], "*") == 0) ) {
         continue;
      }
      count++;
      if (!pre.search(aRecord[j], regexp)) {
         output = 0;
         break;
      }
   }

   if (output && (count == 0)) {
      output = 0;
   }

   return output;
}



//////////////////////////////
//
// HumdrumRecord::isKey --
//

#define REGEX_ISKEY "^\\*[A-Ga-g][-#n]?:"

int HumdrumRecord::isKey(int index) { 
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISKEY)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllKey(void) {
   return HumdrumRecord::isParticularType(REGEX_ISKEY, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isKeySig --
//

#define REGEX_ISKEYSIG "^\\*[kK]\\[[A-Ga-g#-]*\\]"

int HumdrumRecord::isKeySig(int index) { 
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISKEYSIG)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllKeySig(void) {
   return HumdrumRecord::isParticularType(REGEX_ISKEYSIG, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isTempo --
//

#define REGEX_ISTEMPO "^\\*MM\\d+\\.?\\d*"

int HumdrumRecord::isTempo(int index) { 
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISTEMPO)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllTempo(void) {
   return HumdrumRecord::isParticularType(REGEX_ISTEMPO, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isTimeSig --
//

#define REGEX_ISTIMESIG "^\\*M\\d+/\\d+"

int HumdrumRecord::isTimeSig(int index) {
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISTIMESIG)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllTimeSig(void) {
   return HumdrumRecord::isParticularType(REGEX_ISTIMESIG, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isMetSig --
//

#define REGEX_ISMETSIG "^\\*met\\([^)]*\\)"

int HumdrumRecord::isMetSig(int index) {
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISMETSIG)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllMetSig(void) {
   return HumdrumRecord::isParticularType(REGEX_ISMETSIG, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isTranspose --  Work on splitting up this into two cases.
//

#define REGEX_ISTRANSPOSE "^\\*ITr"

int HumdrumRecord::isTranspose(int index) { 
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISTRANSPOSE)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllTranspose(void) {
   return HumdrumRecord::isParticularType(REGEX_ISTRANSPOSE, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isInstrumentType --
//

#define REGEX_ISINSTRUMENTTYPE "^\\*I[a-z]{2,5}"

int HumdrumRecord::isInstrumentType(int index) { 
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], "^\\*I[a-z]{2,5}")) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllInstrumentType(void) {
   return HumdrumRecord::isParticularType(REGEX_ISINSTRUMENTTYPE, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isInstrumentClass --
//

#define REGEX_ISINSTRUMENTCLASS "^\\*IC[a-z]+"

int HumdrumRecord::isInstrumentClass(int index) {
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISINSTRUMENTCLASS)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllInstrumentClass(void) {
   return HumdrumRecord::isParticularType(REGEX_ISINSTRUMENTCLASS, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isInstrumentName --
//

#define REGEX_ISINSTRUMENTNAME "^\\*I\""

int HumdrumRecord::isInstrumentName(int index) {
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISINSTRUMENTNAME)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllInstrumentName(void) {
   return HumdrumRecord::isParticularType(REGEX_ISINSTRUMENTNAME, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isInstrumentAbbr --
//

#define REGEX_ISINSTRUMENTABBR "^\\*I\'"

int HumdrumRecord::isInstrumentAbbr(int index) {
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISINSTRUMENTNAME)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllInstrumentAbbr(void) {
   return HumdrumRecord::isParticularType(REGEX_ISINSTRUMENTABBR, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isInstrumentNum --
//

#define REGEX_ISINSTRUMENTNUM "^\\*I#"

int HumdrumRecord::isInstrumentNum(int index) { 
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISINSTRUMENTNUM)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllInstrumentNum(void) {
   return HumdrumRecord::isParticularType(REGEX_ISINSTRUMENTNUM, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isLabelExpansion --
//

#define REGEX_ISLABELEXPANSION "^\\*>\\[[^]]*\\]$"

int HumdrumRecord::isLabelExpansion(int index) {
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISLABELEXPANSION)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllLabelExpansion(void) {
   return HumdrumRecord::isParticularType(REGEX_ISLABELEXPANSION, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isLabelVariant --
//

#define REGEX_ISLABELVARIANT "^\\*>[^[]+\\[[^]]*\\]$"

int HumdrumRecord::isLabelVariant(int index) {
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISLABELVARIANT)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllLabelVariant(void) {
   return HumdrumRecord::isParticularType(REGEX_ISLABELVARIANT, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isLabelMarker --
//

#define REGEX_ISLABELMARKER "^\\*>[^[]+$"

int HumdrumRecord::isLabelMarker(int index) { 
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISLABELMARKER)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllLabelMarker(void) {
   return HumdrumRecord::isParticularType(REGEX_ISLABELMARKER, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isStaffNumber --
//

#define REGEX_ISSTAFFNUMBER "^\\*staff\\d"

int HumdrumRecord::isStaffNumber(int index) { 
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISSTAFFNUMBER)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllStaffNumber(void) {
   return HumdrumRecord::isParticularType(REGEX_ISSTAFFNUMBER, "**kern");
}


//////////////////////////////
//
// HumdrumRecord::isSysStaffNumber -- Andreas's variant on staff numbering
//     which is local to a particular system on a specific page.
//

#define REGEX_ISSYSSTAFFNUMBER "^\\*staff:\\d"

int HumdrumRecord::isSysStaffNumber(int index) { 
   HumdrumRecord& aRecord = *this;
   PerlRegularExpression pre;   
   if (pre.search(aRecord[index], REGEX_ISSYSSTAFFNUMBER)) {
      return 1;
   }
   return 0;
}

int HumdrumRecord::isAllSysStaffNumber(void) {
   return HumdrumRecord::isParticularType(REGEX_ISSYSSTAFFNUMBER, "**kern");
}



//////////////////////////////
//
// HumdrumRecord::isNull -- true if all fields are ".", "!", or "*".
//

int HumdrumRecord::isNull(void) {
   const char* target = "";
   HumdrumRecord& aRecord = *this;
   if (aRecord.isData()) {
      target = ".";
   } else if (aRecord.isLocalComment()) {
      target = "!";
   } else if (aRecord.isInterpretation()) {
      target = "*";
   }

   if (strlen(target) == 0) {
      // Global comments like "!!" with nothing after them
      // might be good to call a NULL, but not for now.
      return 0;
   }
  
   int j;
   for (j=0; j<aRecord.getFieldCount(); j++) {
      if (strcmp(aRecord[j], target) != 0) {
         return 0;
      }
   }

   return 1;
}



//////////////////////////////
//
// HumdrumRecord::dataQ -- returns true if data
//

int HumdrumRecord::dataQ(void) const {
   if (getType() == E_humrec_data) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// HumdrumRecord::measureQ -- 
//

int HumdrumRecord::measureQ(void) const {
   if (getType() == E_humrec_data_kern_measure) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// HumdrumRecord::localCommentQ -- 
//

int HumdrumRecord::localCommentQ(void) const {
   if (getType() == E_humrec_data_comment) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// HumdrumRecord::globalComment -- 
//

int HumdrumRecord::globalCommentQ(void) const {
   if (getType() == E_humrec_global_comment) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// HumdrumRecord::bibRecordQ -- 
//

int HumdrumRecord::bibRecordQ(void) const {
   if (getType() == E_humrec_bibliography) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// HumdrumRecord::interpretationQ --
//

int HumdrumRecord::interpretationQ(void) const {
   if (getType() == E_humrec_data_interpretation) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// HumdrumRecord::nullQ --
//

int HumdrumRecord::nullQ(void) const {
   if (getType() == E_humrec_empty) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// HumdrumRecord::hasExclusiveQ --
//

int HumdrumRecord::hasExclusiveQ(void) const {
   if (getType() != E_humrec_data_interpretation) {
      return 0;
   } 

   int output = 0;
   int i;
   for (i=0; i<getFieldCount(); i++) {
      if (strncmp("**", recordFields[i], 2) == 0) {
         output = 1;
         break;
      }
   }

   return output;
}



//////////////////////////////
//
// HumdrumRecord::hasPathQ --
//

int HumdrumRecord::hasPathQ(void) const {
   if (getType() != E_humrec_data_interpretation) {
      return 0;
   } 

   int output = 0;
   int i;
   for (i=0; i<getFieldCount(); i++) {
      if (strcmp("*-", recordFields[i]) == 0) {
         output = 1;
         break;
      } else if (strcmp("*+", recordFields[i]) == 0) {
         output = 1;
         break;
      } else if (strcmp("*x", recordFields[i]) == 0) {
         output = 1;
         break;
      } else if (strcmp("*v", recordFields[i]) == 0) {
         output = 1;
         break;
      } else if (strcmp("*^", recordFields[i]) == 0) {
         output = 1;
         break;
      }
   }

   return output;
}



//////////////////////////////
//
// HumdrumRecord::insertField --
//     default values: anInterp = E_unknown, spinetrace = ""
//	

void HumdrumRecord::insertField(int index, const char* aField, int anInterp,
      const char* spinetrace) {
   if (index < 0) {
      index = getFieldCount();
   }      
   recordFields[recordFields.getSize()] = NULL;
   spineids[spineids.getSize()] = NULL;

   interpretation.setSize(interpretation.getSize()+1);
   interpretation[interpretation.getSize()-1] = E_unknown;

   for (int i=getFieldCount()-2; i>=0; i--) {
      if (i >= index) {
         recordFields[i+1]   = recordFields[i]; 
         spineids[i+1]       = spineids[i]; 
         interpretation[i+1] = interpretation[i]; 
      } else {
         break;
      }
   }

   // add the field
   interpretation[index] = anInterp;
   recordFields[index]   = new char[strlen(aField)+1];
   spineids[index]       = new char[strlen(spinetrace)+1];
   strcpy(recordFields[index], aField);
   strcpy(spineids[index], spinetrace);

   int dummy = -1;
   dotline.append(dummy);
   dotspine.append(dummy);

   modifiedQ = 1;
}


void HumdrumRecord::insertField(int index, const char* aField, 
      const char* anInterp, const char* spinetrace) {

   int interptype = Convert::exint.getValue(anInterp);
   if (interptype == E_unknown || interptype == E_UNKNOWN_EXINT) {
      Convert::exint.add(anInterp);
   }

   interpretation[index] = Convert::exint.getValue(anInterp);

   insertField(index, aField, interptype, spinetrace);
}



//////////////////////////////
//
// HumdrumRecord::operator= --
//

HumdrumRecord& HumdrumRecord::operator=(const HumdrumRecord& aRecord) {
   // don't copy onto self
   if (&aRecord == this) {
      return *this;
   }   

   duration = aRecord.duration;
   durationR= aRecord.durationR;
   meterloc = aRecord.meterloc;
   meterlocR= aRecord.meterlocR;
   absloc   = aRecord.absloc;
   abslocR  = aRecord.abslocR;

   int i;
   type = aRecord.type;
   if (recordString != NULL) {
      delete [] recordString;
      recordString = NULL;
   }
   if (aRecord.recordString != NULL) {
      recordString = new char[strlen(aRecord.recordString)+1];
      strcpy(recordString, aRecord.recordString);
   }
   modifiedQ = aRecord.modifiedQ;
   interpretation.setSize(aRecord.interpretation.getSize());

   for (i=0; i<recordFields.getSize(); i++) {
      if (recordFields[i] != NULL) {
         delete [] recordFields[i];
         recordFields[i] = NULL;
      }
   }

   for (i=0; i<spineids.getSize(); i++) {
      if (spineids[i] != NULL) {
         delete [] spineids[i];
         spineids[i] = NULL;
      }
   }

   recordFields.setSize(aRecord.recordFields.getSize());
   spineids.setSize(aRecord.spineids.getSize());

   int allocSize = 0;
   for (i=0; i<aRecord.getFieldCount(); i++) {
      if (interpretation.getSize() > 0) {
          interpretation[i] = aRecord.interpretation[i];
      }
      allocSize = strlen(aRecord.recordFields[i]) + 1;

      recordFields[i] = new char[allocSize];
      strcpy(recordFields[i], aRecord.recordFields[i]);

      spineids[i] = new char[strlen(aRecord.spineids[i]) + 1];
      strcpy(spineids[i], aRecord.spineids[i]);
   }
 
   return *this;
}


HumdrumRecord& HumdrumRecord::operator=(const HumdrumRecord* aRecord) {
   *this = *aRecord;
   return *this;
}


HumdrumRecord& HumdrumRecord::operator=(const char* aLine) {
   duration = 0.0;
   durationR.zero();
   meterloc = 0.0;
   meterlocR.zero();
   absloc   = 0.0;
   abslocR.zero();
   setLine(aLine);   
   return *this;
}



//////////////////////////////
//
// HumdrumRecord::operator[] --
//

const char* HumdrumRecord::operator[](int index) const {
   if (index > getFieldCount()) {
      cout << "Error trying to access invalid spine field: " << index << endl;
      exit(1);
   }

   return recordFields[index];
}



//////////////////////////////
//
// HumdrumRecord::setAbsBeat -- sets the absolute beat location
//    in the file.  This value is by default 0, but can be set
//    manally with the setAbsBeat() function, or by calling
//    HumdrumFile::analyzeRhythm().
//

void HumdrumRecord::setAbsBeat(double aValue) { 
   cerr << "ERROR not allowed to use HumdrumRecord::setAbsBeat at the moment" 
        << endl;
   absloc = (float)aValue;
   // this function cannot co-exists with the following one
   // and will have to be removed.
}

void HumdrumRecord::setAbsBeat(const RationalNumber& aValue) { 
   abslocR = aValue;
   absloc = abslocR.getFloat();
}

void HumdrumRecord::setAbsBeatR(const RationalNumber& aValue) { 
   abslocR = aValue;
   absloc = abslocR.getFloat();
}

void HumdrumRecord::setAbsBeatR(int top, int bottom) {
   abslocR.setValue(top, bottom);
}

void HumdrumRecord::setAbsBeat(int top, int bottom) {
   abslocR.setValue(top, bottom);
}



//////////////////////////////
//
// HumdrumRecord::setBeat -- sets the metrical beat location
//    in the file.  This value is by default 0, but can be set
//    manally with the setBeat() function, or by calling
//    HumdrumFile::analyzeRhythm().
//

void HumdrumRecord::setBeat(double aValue) { 
   cerr << "ERROR not allowed to use HumdrumRecord::setBeat at the moment" 
        << endl;
   meterloc = (float)aValue;
}

void HumdrumRecord::setBeat(int top, int bottom) { 
   meterlocR.setValue(top, bottom);
}

void HumdrumRecord::setBeatR(int top, int bottom) { 
   meterlocR.setValue(top, bottom);
}

void HumdrumRecord::setBeat(const RationalNumber& aValue) { 
   meterlocR = aValue;
}

void HumdrumRecord::setBeatR(const RationalNumber& aValue) { 
   meterlocR = aValue;
}


//////////////////////////////
//
// HumdrumRecord::setDotLine -- set place holding value line location.
//

void HumdrumRecord::setDotLine(int index, int value) {  
   dotline[index] = value;
}



//////////////////////////////
//
// HumdrumRecord::setDotSpine -- set place holding value spine location.
//

void HumdrumRecord::setDotSpine(int index, int value) {  
   dotspine[index] = value;
}


//////////////////////////////
//
// HumdrumRecord::setDuration -- sets the duration of the current
//    HumdrumRecord line in a HumdrumFile.  This value is by default 0, 
//    but can be set manally with the setAbsBeat() function, or by 
//    calling HumdrumFile::analyzeRhythm().
//

void HumdrumRecord::setDuration(double aValue) { 
   cerr << "Error: not allowed to use HumdrumRecord::setDuration at the moment"
        << endl;
   duration = (float)aValue;
}

void HumdrumRecord::setDuration(int top, int bottom) {
   durationR.setValue(top, bottom);
}

void HumdrumRecord::setDuration(RationalNumber aValue) { 
   durationR = aValue;
}

void HumdrumRecord::setDurationR(int top, int bottom) {
   durationR.setValue(top, bottom);
}

void HumdrumRecord::setDurationR(RationalNumber aValue) { 
   durationR = aValue;
}



//////////////////////////////
//
// HumdrumRecord::setExInterp -- assigns the exclusive interpretation
//	to the specified data field.
//

void HumdrumRecord::setExInterp(int index, int anInterpretation) {
   if (index >= interpretation.getSize()) {
      cout << "Error: accessing too large a field number A: " << index 
           << " in line: " << getLine() << endl;
      cout << "Size of record: " << getFieldCount() << endl;
      cout << "Size of interpretation array: " 
           << interpretation.getSize() << endl;
      exit(1);
   }
   
   interpretation[index] = anInterpretation;
}


void HumdrumRecord::setExInterp(int index, const char* interpString) {
   if (index >= interpretation.getSize()) {
      cout << "Error: accessing too large a field number B: " << index 
           << " in line: " << getLine() << endl;
      cout << "Size of interpretation list: " << interpretation.getSize() 
	   << endl;
      cout << "But you tried to access index: " << index << endl;
      exit(1);
   }


   int interptype = Convert::exint.getValue(interpString);
   if (interptype == E_unknown || interptype == E_UNKNOWN_EXINT) {
      Convert::exint.add(interpString);
   }

   interpretation[index] = Convert::exint.getValue(interpString);
}



//////////////////////////////
//
// HumdrumRecord::setToken --
//

void HumdrumRecord::setToken(int index, const char* aString) {
   delete [] recordFields[index];
   int len = strlen(aString);
   recordFields[index] = new char[len+1];
   strcpy(recordFields[index], aString);
   modifiedQ = 1;
}
   


//////////////////////////////
//
// HumdrumRecord::setLine -- sets the record to a (new) string
//

void HumdrumRecord::setLine(const char* aLine) {
   if (recordString != NULL) {
      delete [] recordString;
      recordString = NULL;
   }
   int length = strlen(aLine);
   recordString = new char[length+1];
   strcpy(recordString, aLine);
   if (recordString[length-1] == 0x0d || recordString[length-1] == 0x0a) {
      recordString[length-1] = '\0';
   }
   modifiedQ = 0;
   int i;

   if (recordFields.getSize() != 0) {
      for (i=0; i<recordFields.getSize(); i++) {
         if (recordFields[i] != NULL) {
            delete recordFields[i];
            recordFields[i] = NULL;
         }
      }
      recordFields.setSize(0);
   }

   if (spineids.getSize() != 0) {
      for (i=0; i<spineids.getSize(); i++) {
         if (spineids[i] != NULL) {
            delete spineids[i];
            spineids[i] = NULL;
         }
      }
      spineids.setSize(0);
   }

   type = determineType(recordString);
   int fieldCount = determineFieldCount(recordString);

   // allow for null records
   if (fieldCount == 0) {
      fieldCount = 1;
   }

   interpretation.setSize(fieldCount);
   for (i=0; i<fieldCount; i++) {
      interpretation[i] = E_unknown;
   }

   // chop up the input line into data fields to store in array.
   // but don't chop up if the fieldCount is just supposed to be 1...
   int start = 0;
   int stop = 0;
   i = 0;
   char* temp;
   int index;
   if (fieldCount == 1) {
      temp = new char[strlen(recordString)+1];
      strcpy(temp, recordString);
      index = recordFields.getSize(); 
      recordFields[index] = temp;
      spineids[index] = new char[1];
      spineids[index][0] = '\0';
   } else {
      while (recordString[i] != '\0') {
         if (recordString[i] != '\t') {
            stop++;
         } else {
            temp = new char[stop-start+1];
            strncpy(temp, &recordString[start], stop-start);
            temp[stop-start] = '\0';
            index = recordFields.getSize(); 
            recordFields[index] = temp;
            spineids[index] = new char[1];
            spineids[index][0] = '\0';
            start = stop = i+1;
         }
         i++;
      }
      temp = new char[stop-start+1];
      strncpy(temp, &recordString[start], stop-start);
      temp[stop-start] = '\0';
      index = recordFields.getSize();
      recordFields[index] = temp;
      spineids[index] = new char[1];
      spineids[index][0] = '\0';
   }

   dotline.setSize(fieldCount);
   dotspine.setSize(fieldCount);
   for (i=0; i<fieldCount; i++) {
      setDotLine(i, -1);
      setDotSpine(i, -1);
   }
}



//////////////////////////////
//
// HumdrumRecord::setLineNum -- 
//

void HumdrumRecord::setLineNum(int aNumber) {
   lineno = aNumber;
}



//////////////////////////////
//
// HumdrumRecord::setSpineID -- 
//

void HumdrumRecord::setSpineID(int index, const char* anID) {
   if (index < spineids.getSize() && spineids[index] != NULL) {
      delete [] spineids[index];
   }
   spineids[index] = new char[strlen(anID) + 1];
   strcpy(spineids[index], anID);
}



//////////////////////////////
//
// HumdrumRecord::setSpineWidth -- sets the number of spines
//	in the data at the current point in a Humdrum File.
//

void HumdrumRecord::setSpineWidth(int aSize) {
   if (aSize >= 0) {
      int oldsize = spinewidth;
      spinewidth = aSize;
      interpretation.setSize(spinewidth);
      int i;
      if (oldsize > spinewidth) {
         for (i=oldsize; i<spinewidth; i++) {
            interpretation[i] = E_UNKNOWN_EXINT;
         }
      } 
   }
}



//////////////////////////////
//
// HumdrumRecord::getBibliographicMeaning -- given a Humdrum Reference
//      Record code, return a brief summary of it.
//

const char* HumdrumRecord::getBibliographicMeaning(Array<char>& output, 
      const char* code) {
   int atcount = 0;
   char langbuf[128] = {0};
   char keybuf[128] = {0};

   // check for @ signs which indicate language of bibliographic value.
   int len = strlen(code);
   if (len > 100) {
      return "ERROR: Bibliographic key is too long.";
   }

   PerlRegularExpression pre;
   pre.search(code, "([a-z]+)(\\d*)(@*)([a-z]*)", "i");

   strcpy(langbuf, pre.getSubmatch(4));
   strcpy(keybuf, pre.getSubmatch(1));

   atcount = strlen(pre.getSubmatch(3));

   int numberfound = 0;
   float number = 2357911.131719;
   if (strlen(pre.getSubmatch(2)) > 0) {
      numberfound = 1;
      number = strtol(pre.getSubmatch(2), NULL, 10);
  
   }

   SSTREAM description;

   if (strcmp(keybuf, "ACO") == 0) {
      description << "Analytic collection";
   } else if (strcmp(keybuf, "AFR") == 0) {
      description << "Form designation";
   } else if (strcmp(keybuf, "AGN") == 0) {
      description << "Genre designation";
   } else if (strcmp(keybuf, "AIN") == 0) {
      description << "Instrumentation";
   } else if (strcmp(keybuf, "AMD") == 0) {
      description << "Mode designation";
   } else if (strcmp(keybuf, "AMT") == 0) {
      description << "Metric classification";
   } else if (strcmp(keybuf, "ARE") == 0) {
      description << "Geographical region";
   } else if (strcmp(keybuf, "ARL") == 0) {
      description << "Geographical location of origin";
   } else if (strcmp(keybuf, "AST") == 0) {
      description << "Style, period, or type of work designation";
   } else if (strcmp(keybuf, "ASW") == 0) {
      description << "Associated work";

   // C = Composer related codes
   } else if (strcmp(keybuf, "CDT") == 0) {
      description << "Composer's dates";
   } else if (strcmp(keybuf, "CNT") == 0) {
      description << "Composer's nationality";
   } else if (strcmp(keybuf, "COA") == 0) {
      description << "Attributed composer";
   } else if (strcmp(keybuf, "COC") == 0) {
      description << "Composer's corporate name";
   } else if (strcmp(keybuf, "COL") == 0) {
      description << "Stage name, alias";
   } else if (strcmp(keybuf, "COM") == 0) {
      description << "Composer's name";
   } else if (strcmp(keybuf, "COS") == 0) {
      description << "Suspected composer";

   // E = Edition related codes
   } else if (strcmp(keybuf, "EED") == 0) {
      description << "Electronic editor";
   } else if (strcmp(keybuf, "EEV") == 0) {
      description << "Electronic edition version";
   } else if (strcmp(keybuf, "EFL") == 0) {
      description << "File number";
   } else if (strcmp(keybuf, "EMD") == 0) {
      description << "Document modification description";
   } else if (strcmp(keybuf, "ENC") == 0) {
      description << "Music encoder";
   } else if (strcmp(keybuf, "END") == 0) {
      description << "Initial enc. date";
   } else if (strcmp(keybuf, "EST") == 0) {
      description << "Encoding status";

   // G = Group related codes
   } else if (strcmp(keybuf, "GCO") == 0) {
      description << "Collection designation";
   } else if (strcmp(keybuf, "GNM") == 0) {
      description << "Group number";
   } else if (strcmp(keybuf, "GTL") == 0) {
      description << "Group title";
   } else if (strcmp(keybuf, "GAW") == 0) {
      description << "Associated work";
   } else if (strcmp(keybuf, "HAO") == 0) {
      description << "Aural history";
   } else if (strcmp(keybuf, "HTX") == 0) {
      description << "Translation of vocal text";

   // L = Lyrics related codes
   } else if (strcmp(keybuf, "LAR") == 0) {
      description << "Arranger";
   } else if (strcmp(keybuf, "LDT") == 0) {
      description << "Lyric Date";
   } else if (strcmp(keybuf, "LIB") == 0) {
      description << "Librettist";
   } else if (strcmp(keybuf, "LOR") == 0) {
      description << "Orchestrator";
   } else if (strcmp(keybuf, "LYR") == 0) {
      description << "Lyricist";

   // M = Performance related codes
   } else if (strcmp(keybuf, "MCN") == 0) {
      description << "Conductor's name";
   } else if (strcmp(keybuf, "MLC") == 0) {
      description << "Performance location";
   } else if (strcmp(keybuf, "MPD") == 0) {
      description << "Date of first performance";
   } else if (strcmp(keybuf, "MPN") == 0) {
      description << "Performer's name";
   } else if (strcmp(keybuf, "MPS") == 0) {
      description << "Suspected performer";
   } else if (strcmp(keybuf, "MRD") == 0) {
      description << "Date of performance";

   // O = Work (opus) related codes
   } else if (strcmp(keybuf, "OAC") == 0) {
      description << "Act number";
   } else if (strcmp(keybuf, "OCL") == 0) {
      description << "Collector's name";
   } else if (strcmp(keybuf, "OCO") == 0) {
      description << "Commission";
   } else if (strcmp(keybuf, "OCY") == 0) {
      description << "Country of composition";
   } else if (strcmp(keybuf, "ODE") == 0) {
      description << "Dedication";
   } else if (strcmp(keybuf, "ODT") == 0) {
      description << "Date of composition";
   } else if (strcmp(keybuf, "OKY") == 0) {
      description << "Key";
   } else if (strcmp(keybuf, "OMD") == 0) {
      description << "Movement designation";
   } else if (strcmp(keybuf, "OMV") == 0) {
      description << "Movement number";
   } else if (strcmp(keybuf, "ONB") == 0) {
      description << "Free format note";
   } else if (strcmp(keybuf, "ONM") == 0) {
      description << "Number";
   } else if (strcmp(keybuf, "OPC") == 0) {
      description << "City of composition";
   } else if (strcmp(keybuf, "OPR") == 0) {
      description << "Title of parent work";
   } else if (strcmp(keybuf, "OPS") == 0) {
      description << "Opus number";
   } else if (strcmp(keybuf, "OPT") == 0) {
      description << "Parent work";
   } else if (strcmp(keybuf, "OSC") == 0) {
      description << "Scene number";
   } else if (strcmp(keybuf, "OTA") == 0) {
      description << "Alternative title";
   } else if (strcmp(keybuf, "OTL") == 0) {
      description << "Title";
   } else if (strcmp(keybuf, "OTP") == 0) {
      description << "Popular title";
   } else if (strcmp(keybuf, "OVM") == 0) {
      description << "Volume";

   // P = Publisher related codes
   } else if (strcmp(keybuf, "PC#") == 0) {
      description << "Publisher's cat. num.";
   } else if (strcmp(keybuf, "PDT") == 0) {
      description << "Date first published";
   } else if (strcmp(keybuf, "PPG") == 0) {
      description << "Page";
   } else if (strcmp(keybuf, "PPG") == 0) {
      description << "Publication page";
   } else if (strcmp(keybuf, "PPP") == 0) {
      description << "Place first published";
   } else if (strcmp(keybuf, "PPR") == 0) {
      description << "First publisher";
   } else if (strcmp(keybuf, "PUB") == 0) {
      description << "Publication status";

   // R = User related codes
   } else if (strcmp(keybuf, "RC#") == 0) {
      description << "Recording company's catalog number";
   } else if (strcmp(keybuf, "RDF") == 0) {
      description << "User-defined signifiers";
   } else if (strcmp(keybuf, "RDT") == 0) {
      description << "Date of recording";
   } else if (strcmp(keybuf, "RLC") == 0) {
      description << "Recording location";
   } else if (strcmp(keybuf, "RLN") == 0) {
      description << "ASCII language setting";
   } else if (strcmp(keybuf, "RMM") == 0) {
      description << "Manufacturer or sponsoring company";
   } else if (strcmp(keybuf, "RNB") == 0) {
      description << "Encoding note";
   } else if (strcmp(keybuf, "RNP") == 0) {
      description << "Name of the producer";
   } else if (strcmp(keybuf, "RRD") == 0) {
      description << "Date of release";
   } else if (strcmp(keybuf, "RTL") == 0) {
      description << "Title of album";
   } else if (strcmp(keybuf, "RT#") == 0) {
      description << "Track number";
   } else if (strcmp(keybuf, "RWG") == 0) {
      description << "Representation warning";

   // S = Scholarly/source related codes
   } else if (strcmp(keybuf, "SCA") == 0) {
      description << "Full cat. name";
   } else if (strcmp(keybuf, "SCT") == 0) {
      description << "Scholarly cat. num.";
   } else if (strcmp(keybuf, "SMA") == 0) {
      description << "Acknowledgment of manuscript access";
   } else if (strcmp(keybuf, "SML") == 0) {
      description << "Manuscript location";
   } else if (strcmp(keybuf, "SMS") == 0) {
      description << "Manuscript source name";

   } else if (strcmp(keybuf, "TRN") == 0) {
      description << "Translator of text";
   } else if (strcmp(keybuf, "TXL") == 0) {
      description << "Language of the encoded text";
   } else if (strcmp(keybuf, "TXO") == 0) {
      description << "Orignal language of text";
   } else if (strcmp(keybuf, "VTS") == 0) {
      description << "Checksum validation number";

   // X = translation related codes
   } else if (strcmp(keybuf, "XDE") == 0) {
      description << "Translated title (in German)";
   } else if (strcmp(keybuf, "XEN") == 0) {
      description << "Translated title (in English)";
   } else if (strcmp(keybuf, "XFR") == 0) {
      description << "Translated title (in French)";
   } else if (strcmp(keybuf, "XNI") == 0) {
      description << "Translated title (in Japanese)";
   } else if (strcmp(keybuf, "XAL") == 0) {
      description << "translated title in Albanian";
   } else if (strcmp(keybuf, "XAB") == 0) {
      description << "translated title in Arabic";
   } else if (strcmp(keybuf, "XAM") == 0) {
      description << "translated title in Armenian";
   } else if (strcmp(keybuf, "XAZ") == 0) {
      description << "translated title in Azeri";
   } else if (strcmp(keybuf, "XBE") == 0) {
      description << "translated title in Bengali";
   } else if (strcmp(keybuf, "XBU") == 0) {
      description << "translated title in Bulgarian";
   } else if (strcmp(keybuf, "XCB") == 0) {
      description << "translated title in Cambodian";
   } else if (strcmp(keybuf, "XCA") == 0) {
      description << "translated title in Cantonese";
   } else if (strcmp(keybuf, "XHR") == 0) {
      description << "translated title in Croatian";
   } else if (strcmp(keybuf, "XCE") == 0) {
      description << "translated title in Czech";
   } else if (strcmp(keybuf, "XDA") == 0) {
      description << "translated title in Danish";
   } else if (strcmp(keybuf, "XNE") == 0) {
      description << "translated title in Dutch";
   } else if (strcmp(keybuf, "XET") == 0) {
      description << "translated title in Estonian";
   } else if (strcmp(keybuf, "XSU") == 0) {
      description << "translated title in Finnish";
   } else if (strcmp(keybuf, "XFL") == 0) {
      description << "translated title in Flemish";
   } else if (strcmp(keybuf, "XGA") == 0) {
      description << "translated title in Gaelic";
   } else if (strcmp(keybuf, "XGR") == 0) {
      description << "translated title in Greek";
   } else if (strcmp(keybuf, "XHB") == 0) {
      description << "translated title in Hebrew";
   } else if (strcmp(keybuf, "XHI") == 0) {
      description << "translated title in Hindi";
   } else if (strcmp(keybuf, "XHU") == 0) {
      description << "translated title in Hungarian";
   } else if (strcmp(keybuf, "XIC") == 0) {
      description << "translated title in Icelandic";
   } else if (strcmp(keybuf, "XIT") == 0) {
      description << "translated title in Italian";
   } else if (strcmp(keybuf, "XJV") == 0) {
      description << "translated title in Javanese";
   } else if (strcmp(keybuf, "XKO") == 0) {
      description << "translated title in Korean";
   } else if (strcmp(keybuf, "XLI") == 0) {
      description << "translated title in Lithuanian";
   } else if (strcmp(keybuf, "XLA") == 0) {
      description << "translated title in Latin";
   } else if (strcmp(keybuf, "XLV") == 0) {
      description << "translated title in Latvian";
   } else if (strcmp(keybuf, "XMG") == 0) {
      description << "translated title in Malayalam";
   } else if (strcmp(keybuf, "XMA") == 0) {
      description << "translated title in Mandarin";
   } else if (strcmp(keybuf, "XMO") == 0) {
      description << "translated title in Mongolian";
   } else if (strcmp(keybuf, "XNO") == 0) {
      description << "translated title in Norwegian";
   } else if (strcmp(keybuf, "XPL") == 0) {
      description << "translated title in Polish";
   } else if (strcmp(keybuf, "XPR") == 0) {
      description << "translated title in Portuguese";
   } else if (strcmp(keybuf, "XRU") == 0) {
      description << "translated title in Russian";
   } else if (strcmp(keybuf, "XSR") == 0) {
      description << "translated title in Serbian";
   } else if (strcmp(keybuf, "XSK") == 0) {
      description << "translated title in Slovak";
   } else if (strcmp(keybuf, "XSN") == 0) {
      description << "translated title in Slovenian";
   } else if (strcmp(keybuf, "XES") == 0) {
      description << "translated title in Spanish";
   } else if (strcmp(keybuf, "XSW") == 0) {
      description << "translated title in Swahili";
   } else if (strcmp(keybuf, "XSV") == 0) {
      description << "translated title in Swedish";
   } else if (strcmp(keybuf, "XTA") == 0) {
      description << "translated title in Tamil";
   } else if (strcmp(keybuf, "XTH") == 0) {
      description << "translated title in Thai";
   } else if (strcmp(keybuf, "XTU") == 0) {
      description << "translated title in Turkish";
   } else if (strcmp(keybuf, "XUK") == 0) {
      description << "translated title in Ukranian";
   } else if (strcmp(keybuf, "XUR") == 0) {
      description << "translated title in Urdu";
   } else if (strcmp(keybuf, "XVN") == 0) {
      description << "translated title in Vietnamese";
   } else if (strcmp(keybuf, "XWE") == 0) {
      description << "translated title in Welsh";
   } else if (strcmp(keybuf, "XHO") == 0) {
      description << "translated title in Xhosa";
   } else if (strcmp(keybuf, "XZU") == 0) {
      description << "translated title in Zulu";

   // Y = copyright related codes
   } else if (strcmp(keybuf, "YEC") == 0) {
      description << "Copyright notice";
   } else if (strcmp(keybuf, "YED") == 0) {
      description << "Last edited";
   } else if (strcmp(keybuf, "YEM") == 0) {
      description << "Copyright message";
   } else if (strcmp(keybuf, "YEN") == 0) {
      description << "Country of copyright";
   } else if (strcmp(keybuf, "YEP") == 0) {
      description << "Publisher of electronic edition";
   } else if (strcmp(keybuf, "YER") == 0) {
      description << "Date electronic edition released";
   } else if (strcmp(keybuf, "YOE") == 0) {
      description << "Editor of printed source";
   } else if (strcmp(keybuf, "YOO") == 0) {
      description << "Original document owner";
   } else if (strcmp(keybuf, "YOR") == 0) {
      description << "Original document";
   } else if (strcmp(keybuf, "YOY") == 0) {
      description << "Original copyright year";

   // non-standard bibliographic codes
   } else if (strcmp(keybuf, "IMGURL") == 0) {
      description << "Image URL";
   } else if (strcmp(keybuf, "URL") == 0) {
      description << "URL";
   } else if (strcmp(keybuf, "id") == 0) {
      description << "Database catalog number";
   } else if (strcmp(keybuf, "isbn") == 0) {
      description << "ISBN";
   } else if (strcmp(keybuf, "title") == 0) {
      description << "Title expansion directive";
   } else if (strcmp(keybuf, "hum2abc") == 0) {
      description << "Humdrum to ABC+ options";
   } else if (strcmp(keybuf, "catalog") == 0) {
      description << "Catalog number";
   } else if (strcmp(keybuf, "tune") == 0) {
      description << "Tuning";
   } else if (strcmp(keybuf, "rism") == 0) {
      description << "RISM number";
   } else if (strcmp(keybuf, "meter") == 0) {
      description << "Meter(s)";
   } else if (strcmp(keybuf, "ref") == 0) {
      description << "Reference note";
   } else if (strcmp(keybuf, "minrhy") == 0) {
      description << "Smallest rhythmic unit";
   } else if (strcmp(keybuf, "bem") == 0) {
      description << "EsAC Remark";
   } else if (strcmp(keybuf, "cut[^a-z]?") == 0) {
      description << "EsAC Incipit";
   } else if (strcmp(keybuf, "fkt") == 0) {
      description << "EsAC Function/Genre";
   } else if (strcmp(keybuf, "fnt") == 0) {
      description << "EsAC Display font";
   } else if (strcmp(keybuf, "key") == 0) {
      description << "EsAC KEY[] field";
   } else if (strcmp(keybuf, "kkd") == 0) {
      description << "EsAC Source concordance";
   } else if (strcmp(keybuf, "trd") == 0) {
      description << "EsAC Source";
   } 


   if (atcount == 1) {
      description << "; translation language: ";
      description << HumdrumRecord::getLanguageName(langbuf);
   } else if (atcount == 2) {
      description << "; original language: ";
      description << HumdrumRecord::getLanguageName(langbuf);
   }

   if (numberfound) {
      description << " [" << number << "]";
   }

   description << ends;

   len = strlen(description.CSTRING);
   output.setSize(len+1);
   strcpy(output.getBase(), description.CSTRING);

   return output.getBase();
}



//////////////////////////////
//
// HumdrumRecord::getLanguageName -- given an ISO 639-2 or 639-1
//      code, return the name of the language that it represents.
//

const char* HumdrumRecord::getLanguageName(const char* code) {
   char string[8] = {0};
   strncpy(string, code, 3);
   int len = strlen(string);
   int i;
   for (i=0; i<len; i++) {
      string[i] = tolower(string[i]);
   }
	  
   if (strcmp(string, "aar") == 0) { return "Afar"; }
   if (strcmp(string, "aa" ) == 0) { return "Afar"; }
   if (strcmp(string, "abk") == 0) { return "Abkhazian"; }
   if (strcmp(string, "ab" ) == 0) { return "Abkhazian"; }
   if (strcmp(string, "ace") == 0) { return "Achinese"; }
   if (strcmp(string, "ach") == 0) { return "Acoli"; }
   if (strcmp(string, "ada") == 0) { return "Adangme"; }
   if (strcmp(string, "ady") == 0) { return "Adyghe; Adygei"; }
   if (strcmp(string, "afa") == 0) { return "Afro-Asiatic languages"; }
   if (strcmp(string, "afh") == 0) { return "Afrihili"; }
   if (strcmp(string, "afr") == 0) { return "Afrikaans"; }
   if (strcmp(string, "af" ) == 0) { return "Afrikaans"; }
   if (strcmp(string, "ain") == 0) { return "Ainu"; }
   if (strcmp(string, "aka") == 0) { return "Akan"; }
   if (strcmp(string, "ak" ) == 0) { return "Akan"; }
   if (strcmp(string, "akk") == 0) { return "Akkadian"; }
   if (strcmp(string, "alb") == 0) { return "Albanian"; }
   if (strcmp(string, "sqi") == 0) { return "Albanian"; }
   if (strcmp(string, "sq" ) == 0) { return "Albanian"; }
   if (strcmp(string, "ale") == 0) { return "Aleut"; }
   if (strcmp(string, "alg") == 0) { return "Algonquian languages"; }
   if (strcmp(string, "alt") == 0) { return "Southern Altai"; }
   if (strcmp(string, "amh") == 0) { return "Amharic"; }
   if (strcmp(string, "am" ) == 0) { return "Amharic"; }
   if (strcmp(string, "ang") == 0) { return "Old English"; }
   if (strcmp(string, "anp") == 0) { return "Angika"; }
   if (strcmp(string, "apa") == 0) { return "Apache languages"; }
   if (strcmp(string, "ara") == 0) { return "Arabic"; }
   if (strcmp(string, "ar" ) == 0) { return "Arabic"; }
   if (strcmp(string, "arc") == 0) { return "Official Aramaic; Imperial Aramaic"; }
   if (strcmp(string, "arg") == 0) { return "Aragonese"; }
   if (strcmp(string, "an" ) == 0) { return "Aragonese"; }
   if (strcmp(string, "arm") == 0) { return "Armenian"; }
   if (strcmp(string, "hye") == 0) { return "Armenian"; }
   if (strcmp(string, "hy" ) == 0) { return "Armenian"; }
   if (strcmp(string, "arn") == 0) { return "Mapudungun; Mapuche"; }
   if (strcmp(string, "arp") == 0) { return "Arapaho"; }
   if (strcmp(string, "art") == 0) { return "Artificial languages"; }
   if (strcmp(string, "arw") == 0) { return "Arawak"; }
   if (strcmp(string, "asm") == 0) { return "Assamese"; }
   if (strcmp(string, "as" ) == 0) { return "Assamese"; }
   if (strcmp(string, "ast") == 0) { return "Asturian; Bable; Leonese; Asturleonese"; }
   if (strcmp(string, "ath") == 0) { return "Athapascan languages"; }
   if (strcmp(string, "aus") == 0) { return "Australian languages"; }
   if (strcmp(string, "ava") == 0) { return "Avaric"; }
   if (strcmp(string, "av" ) == 0) { return "Avaric"; }
   if (strcmp(string, "ave") == 0) { return "Avestan"; }
   if (strcmp(string, "ae" ) == 0) { return "Avestan"; }
   if (strcmp(string, "awa") == 0) { return "Awadhi"; }
   if (strcmp(string, "aym") == 0) { return "Aymara"; }
   if (strcmp(string, "ay" ) == 0) { return "Aymara"; }
   if (strcmp(string, "aze") == 0) { return "Azerbaijani"; }
   if (strcmp(string, "az" ) == 0) { return "Azerbaijani"; }
   if (strcmp(string, "bad") == 0) { return "Banda languages"; }
   if (strcmp(string, "bai") == 0) { return "Bamileke languages"; }
   if (strcmp(string, "bak") == 0) { return "Bashkir"; }
   if (strcmp(string, "ba" ) == 0) { return "Bashkir"; }
   if (strcmp(string, "bal") == 0) { return "Baluchi"; }
   if (strcmp(string, "bam") == 0) { return "Bambara"; }
   if (strcmp(string, "bm" ) == 0) { return "Bambara"; }
   if (strcmp(string, "ban") == 0) { return "Balinese"; }
   if (strcmp(string, "baq") == 0) { return "Basque"; }
   if (strcmp(string, "eus") == 0) { return "Basque"; }
   if (strcmp(string, "eu" ) == 0) { return "Basque"; }
   if (strcmp(string, "bas") == 0) { return "Basa"; }
   if (strcmp(string, "bat") == 0) { return "Baltic languages"; }
   if (strcmp(string, "bej") == 0) { return "Beja; Bedawiyet"; }
   if (strcmp(string, "bel") == 0) { return "Belarusian"; }
   if (strcmp(string, "be" ) == 0) { return "Belarusian"; }
   if (strcmp(string, "bem") == 0) { return "Bemba"; }
   if (strcmp(string, "ben") == 0) { return "Bengali"; }
   if (strcmp(string, "bn" ) == 0) { return "Bengali"; }
   if (strcmp(string, "ber") == 0) { return "Berber languages)"; }
   if (strcmp(string, "bho") == 0) { return "Bhojpuri"; }
   if (strcmp(string, "bih") == 0) { return "Bihari"; }
   if (strcmp(string, "bh" ) == 0) { return "Bihari"; }
   if (strcmp(string, "bik") == 0) { return "Bikol"; }
   if (strcmp(string, "bin") == 0) { return "Bini; Edo"; }
   if (strcmp(string, "bis") == 0) { return "Bislama"; }
   if (strcmp(string, "bi" ) == 0) { return "Bislama"; }
   if (strcmp(string, "bla") == 0) { return "Siksika"; }
   if (strcmp(string, "bnt") == 0) { return "Bantu languages"; }
   if (strcmp(string, "bos") == 0) { return "Bosnian"; }
   if (strcmp(string, "bs" ) == 0) { return "Bosnian"; }
   if (strcmp(string, "bra") == 0) { return "Braj"; }
   if (strcmp(string, "bre") == 0) { return "Breton"; }
   if (strcmp(string, "br" ) == 0) { return "Breton"; }
   if (strcmp(string, "btk") == 0) { return "Batak languages"; }
   if (strcmp(string, "bua") == 0) { return "Buriat"; }
   if (strcmp(string, "bug") == 0) { return "Buginese"; }
   if (strcmp(string, "bul") == 0) { return "Bulgarian"; }
   if (strcmp(string, "bg" ) == 0) { return "Bulgarian"; }
   if (strcmp(string, "bur") == 0) { return "Burmese"; }
   if (strcmp(string, "mya") == 0) { return "Burmese"; }
   if (strcmp(string, "my" ) == 0) { return "Burmese"; }
   if (strcmp(string, "byn") == 0) { return "Blin; Bilin"; }
   if (strcmp(string, "cad") == 0) { return "Caddo"; }
   if (strcmp(string, "cai") == 0) { return "Central American Indian languages"; }
   if (strcmp(string, "car") == 0) { return "Galibi Carib"; }
   if (strcmp(string, "cat") == 0) { return "Catalan; Valencian"; }
   if (strcmp(string, "ca" ) == 0) { return "Catalan; Valencian"; }
   if (strcmp(string, "cau") == 0) { return "Caucasian languages"; }
   if (strcmp(string, "ceb") == 0) { return "Cebuano"; }
   if (strcmp(string, "cel") == 0) { return "Celtic languages"; }
   if (strcmp(string, "cha") == 0) { return "Chamorro"; }
   if (strcmp(string, "ch" ) == 0) { return "Chamorro"; }
   if (strcmp(string, "chb") == 0) { return "Chibcha"; }
   if (strcmp(string, "che") == 0) { return "Chechen"; }
   if (strcmp(string, "ce" ) == 0) { return "Chechen"; }
   if (strcmp(string, "chg") == 0) { return "Chagatai"; }
   if (strcmp(string, "chi") == 0) { return "Chinese"; }
   if (strcmp(string, "zho") == 0) { return "Chinese"; }
   if (strcmp(string, "zh" ) == 0) { return "Chinese"; }
   if (strcmp(string, "chk") == 0) { return "Chuukese"; }
   if (strcmp(string, "chm") == 0) { return "Mari"; }
   if (strcmp(string, "chn") == 0) { return "Chinook jargon"; }
   if (strcmp(string, "cho") == 0) { return "Choctaw"; }
   if (strcmp(string, "chp") == 0) { return "Chipewyan; Dene Suline"; }
   if (strcmp(string, "chr") == 0) { return "Cherokee"; }
   if (strcmp(string, "chu") == 0) { return "Church Slavic; Old Slavonic; Church Slavonic; Old Bulgarian; Old Church Slavonic"; }
   if (strcmp(string, "cu" ) == 0) { return "Church Slavic; Old Slavonic; Church Slavonic; Old Bulgarian; Old Church Slavonic"; }
   if (strcmp(string, "chv") == 0) { return "Chuvash"; }
   if (strcmp(string, "cv" ) == 0) { return "Chuvash"; }
   if (strcmp(string, "chy") == 0) { return "Cheyenne"; }
   if (strcmp(string, "cmc") == 0) { return "Chamic languages"; }
   if (strcmp(string, "cop") == 0) { return "Coptic"; }
   if (strcmp(string, "cor") == 0) { return "Cornish"; }
   if (strcmp(string, "kw" ) == 0) { return "Cornish"; }
   if (strcmp(string, "cos") == 0) { return "Corsican"; }
   if (strcmp(string, "co" ) == 0) { return "Corsican"; }
   if (strcmp(string, "cpe") == 0) { return "English-based Creoles and pidgins"; }
   if (strcmp(string, "cpf") == 0) { return "French-based Creoles and pidgins"; }
   if (strcmp(string, "cpp") == 0) { return "Portuguese-based Creoles and pidgins"; }
   if (strcmp(string, "cre") == 0) { return "Cree"; }
   if (strcmp(string, "cr" ) == 0) { return "Cree"; }
   if (strcmp(string, "crh") == 0) { return "Crimean Tatar; Crimean Turkish"; }
   if (strcmp(string, "crp") == 0) { return "Creoles and pidgins"; }
   if (strcmp(string, "csb") == 0) { return "Kashubian"; }
   if (strcmp(string, "cus") == 0) { return "Cushitic languages"; }
   if (strcmp(string, "cze") == 0) { return "Czech"; }
   if (strcmp(string, "ces") == 0) { return "Czech"; }
   if (strcmp(string, "cs" ) == 0) { return "Czech"; }
   if (strcmp(string, "dak") == 0) { return "Dakota"; }
   if (strcmp(string, "dan") == 0) { return "Danish"; }
   if (strcmp(string, "da" ) == 0) { return "Danish"; }
   if (strcmp(string, "dar") == 0) { return "Dargwa"; }
   if (strcmp(string, "day") == 0) { return "Land Dayak languages"; }
   if (strcmp(string, "del") == 0) { return "Delaware"; }
   if (strcmp(string, "den") == 0) { return "Slave (Athapascan)"; }
   if (strcmp(string, "dgr") == 0) { return "Dogrib"; }
   if (strcmp(string, "din") == 0) { return "Dinka"; }
   if (strcmp(string, "div") == 0) { return "Divehi; Dhivehi; Maldivian"; }
   if (strcmp(string, "dv" ) == 0) { return "Divehi; Dhivehi; Maldivian"; }
   if (strcmp(string, "doi") == 0) { return "Dogri"; }
   if (strcmp(string, "dra") == 0) { return "Dravidian languages"; }
   if (strcmp(string, "dsb") == 0) { return "Lower Sorbian"; }
   if (strcmp(string, "dua") == 0) { return "Duala"; }
   if (strcmp(string, "dum") == 0) { return "Middle Dutch"; }
   if (strcmp(string, "dut") == 0) { return "Dutch; Flemish"; }
   if (strcmp(string, "nld") == 0) { return "Dutch; Flemish"; }
   if (strcmp(string, "nl" ) == 0) { return "Dutch; Flemish"; }
   if (strcmp(string, "dyu") == 0) { return "Dyula"; }
   if (strcmp(string, "dzo") == 0) { return "Dzongkha"; }
   if (strcmp(string, "dz" ) == 0) { return "Dzongkha"; }
   if (strcmp(string, "efi") == 0) { return "Efik"; }
   if (strcmp(string, "egy") == 0) { return "Egyptian (Ancient)"; }
   if (strcmp(string, "eka") == 0) { return "Ekajuk"; }
   if (strcmp(string, "elx") == 0) { return "Elamite"; }
   if (strcmp(string, "eng") == 0) { return "English"; }
   if (strcmp(string, "en" ) == 0) { return "English"; }
   if (strcmp(string, "enm") == 0) { return "Middle English"; }
   if (strcmp(string, "epo") == 0) { return "Esperanto"; }
   if (strcmp(string, "eo" ) == 0) { return "Esperanto"; }
   if (strcmp(string, "est") == 0) { return "Estonian"; }
   if (strcmp(string, "et" ) == 0) { return "Estonian"; }
   if (strcmp(string, "ewe") == 0) { return "Ewe"; }
   if (strcmp(string, "ee" ) == 0) { return "Ewe"; }
   if (strcmp(string, "ewo") == 0) { return "Ewondo"; }
   if (strcmp(string, "fan") == 0) { return "Fang"; }
   if (strcmp(string, "fao") == 0) { return "Faroese"; }
   if (strcmp(string, "fo" ) == 0) { return "Faroese"; }
   if (strcmp(string, "fat") == 0) { return "Fanti"; }
   if (strcmp(string, "fij") == 0) { return "Fijian"; }
   if (strcmp(string, "fj" ) == 0) { return "Fijian"; }
   if (strcmp(string, "fil") == 0) { return "Filipino; Pilipino"; }
   if (strcmp(string, "fin") == 0) { return "Finnish"; }
   if (strcmp(string, "fi" ) == 0) { return "Finnish"; }
   if (strcmp(string, "fiu") == 0) { return "Finno-Ugrian languages"; }
   if (strcmp(string, "fon") == 0) { return "Fon"; }
   if (strcmp(string, "fre") == 0) { return "French"; }
   if (strcmp(string, "fra") == 0) { return "French"; }
   if (strcmp(string, "fr" ) == 0) { return "French"; }
   if (strcmp(string, "frm") == 0) { return "Middle French"; }
   if (strcmp(string, "fro") == 0) { return "Old French"; }
   if (strcmp(string, "frr") == 0) { return "Northern Frisian"; }
   if (strcmp(string, "frs") == 0) { return "Eastern Frisian"; }
   if (strcmp(string, "fry") == 0) { return "Western Frisian"; }
   if (strcmp(string, "fy" ) == 0) { return "Western Frisian"; }
   if (strcmp(string, "ful") == 0) { return "Fulah"; }
   if (strcmp(string, "ff" ) == 0) { return "Fulah"; }
   if (strcmp(string, "fur") == 0) { return "Friulian"; }
   if (strcmp(string, "gaa") == 0) { return "Ga"; }
   if (strcmp(string, "gay") == 0) { return "Gayo"; }
   if (strcmp(string, "gba") == 0) { return "Gbaya"; }
   if (strcmp(string, "gem") == 0) { return "Germanic languages"; }
   if (strcmp(string, "geo") == 0) { return "Georgian"; }
   if (strcmp(string, "kat") == 0) { return "Georgian"; }
   if (strcmp(string, "ka" ) == 0) { return "Georgian"; }
   if (strcmp(string, "ger") == 0) { return "German"; }
   if (strcmp(string, "deu") == 0) { return "German"; }
   if (strcmp(string, "de" ) == 0) { return "German"; }
   if (strcmp(string, "gez") == 0) { return "Geez"; }
   if (strcmp(string, "gil") == 0) { return "Gilbertese"; }
   if (strcmp(string, "gla") == 0) { return "Gaelic; Scottish Gaelic"; }
   if (strcmp(string, "gd" ) == 0) { return "Gaelic; Scottish Gaelic"; }
   if (strcmp(string, "gle") == 0) { return "Irish"; }
   if (strcmp(string, "ga" ) == 0) { return "Irish"; }
   if (strcmp(string, "glg") == 0) { return "Galician"; }
   if (strcmp(string, "gl" ) == 0) { return "Galician"; }
   if (strcmp(string, "glv") == 0) { return "Manx"; }
   if (strcmp(string, "gv" ) == 0) { return "Manx"; }
   if (strcmp(string, "gmh") == 0) { return "Middle High German"; }
   if (strcmp(string, "goh") == 0) { return "Old High German"; }
   if (strcmp(string, "gon") == 0) { return "Gondi"; }
   if (strcmp(string, "gor") == 0) { return "Gorontalo"; }
   if (strcmp(string, "got") == 0) { return "Gothic"; }
   if (strcmp(string, "grb") == 0) { return "Grebo"; }
   if (strcmp(string, "grc") == 0) { return "Ancient Greek"; }
   if (strcmp(string, "gre") == 0) { return "Modern Greek"; }
   if (strcmp(string, "ell") == 0) { return "Modern Greek"; }
   if (strcmp(string, "el" ) == 0) { return "Modern Greek"; }
   if (strcmp(string, "grn") == 0) { return "Guarani"; }
   if (strcmp(string, "gn" ) == 0) { return "Guarani"; }
   if (strcmp(string, "gsw") == 0) { return "Swiss German; Alemannic; Alsatian"; }
   if (strcmp(string, "guj") == 0) { return "Gujarati"; }
   if (strcmp(string, "gu" ) == 0) { return "Gujarati"; }
   if (strcmp(string, "gwi") == 0) { return "Gwich'in"; }
   if (strcmp(string, "hai") == 0) { return "Haida"; }
   if (strcmp(string, "hat") == 0) { return "Haitian; Haitian Creole"; }
   if (strcmp(string, "ht" ) == 0) { return "Haitian; Haitian Creole"; }
   if (strcmp(string, "hau") == 0) { return "Hausa"; }
   if (strcmp(string, "ha" ) == 0) { return "Hausa"; }
   if (strcmp(string, "haw") == 0) { return "Hawaiian"; }
   if (strcmp(string, "heb") == 0) { return "Hebrew"; }
   if (strcmp(string, "he" ) == 0) { return "Hebrew"; }
   if (strcmp(string, "her") == 0) { return "Herero"; }
   if (strcmp(string, "hz" ) == 0) { return "Herero"; }
   if (strcmp(string, "hil") == 0) { return "Hiligaynon"; }
   if (strcmp(string, "him") == 0) { return "Himachali"; }
   if (strcmp(string, "hin") == 0) { return "Hindi"; }
   if (strcmp(string, "hi" ) == 0) { return "Hindi"; }
   if (strcmp(string, "hit") == 0) { return "Hittite"; }
   if (strcmp(string, "hmn") == 0) { return "Hmong"; }
   if (strcmp(string, "hmo") == 0) { return "Hiri Motu"; }
   if (strcmp(string, "ho" ) == 0) { return "Hiri Motu"; }
   if (strcmp(string, "hrv") == 0) { return "Croatian"; }
   if (strcmp(string, "hr" ) == 0) { return "Croatian"; }
   if (strcmp(string, "hsb") == 0) { return "Upper Sorbian"; }
   if (strcmp(string, "hun") == 0) { return "Hungarian"; }
   if (strcmp(string, "hu" ) == 0) { return "Hungarian"; }
   if (strcmp(string, "hup") == 0) { return "Hupa"; }
   if (strcmp(string, "iba") == 0) { return "Iban"; }
   if (strcmp(string, "ibo") == 0) { return "Igbo"; }
   if (strcmp(string, "ig" ) == 0) { return "Igbo"; }
   if (strcmp(string, "ice") == 0) { return "Icelandic"; }
   if (strcmp(string, "isl") == 0) { return "Icelandic"; }
   if (strcmp(string, "is" ) == 0) { return "Icelandic"; }
   if (strcmp(string, "ido") == 0) { return "Ido"; }
   if (strcmp(string, "io" ) == 0) { return "Ido"; }
   if (strcmp(string, "iii") == 0) { return "Sichuan Yi; Nuosu"; }
   if (strcmp(string, "ii" ) == 0) { return "Sichuan Yi; Nuosu"; }
   if (strcmp(string, "ijo") == 0) { return "Ijo languages"; }
   if (strcmp(string, "iku") == 0) { return "Inuktitut"; }
   if (strcmp(string, "iu" ) == 0) { return "Inuktitut"; }
   if (strcmp(string, "ile") == 0) { return "Interlingue; Occidental"; }
   if (strcmp(string, "ie" ) == 0) { return "Interlingue; Occidental"; }
   if (strcmp(string, "ilo") == 0) { return "Iloko"; }
   if (strcmp(string, "ina") == 0) { return "Interlingua"; }
   if (strcmp(string, "ia" ) == 0) { return "Interlingua"; }
   if (strcmp(string, "inc") == 0) { return "Indic languages"; }
   if (strcmp(string, "ind") == 0) { return "Indonesian"; }
   if (strcmp(string, "id" ) == 0) { return "Indonesian"; }
   if (strcmp(string, "ine") == 0) { return "Indo-European languages"; }
   if (strcmp(string, "inh") == 0) { return "Ingush"; }
   if (strcmp(string, "ipk") == 0) { return "Inupiaq"; }
   if (strcmp(string, "ik" ) == 0) { return "Inupiaq"; }
   if (strcmp(string, "ira") == 0) { return "Iranian languages"; }
   if (strcmp(string, "iro") == 0) { return "Iroquoian languages"; }
   if (strcmp(string, "ita") == 0) { return "Italian"; }
   if (strcmp(string, "it" ) == 0) { return "Italian"; }
   if (strcmp(string, "jav") == 0) { return "Javanese"; }
   if (strcmp(string, "jv" ) == 0) { return "Javanese"; }
   if (strcmp(string, "jbo") == 0) { return "Lojban"; }
   if (strcmp(string, "jpn") == 0) { return "Japanese"; }
   if (strcmp(string, "ja" ) == 0) { return "Japanese"; }
   if (strcmp(string, "jpr") == 0) { return "Judeo-Persian"; }
   if (strcmp(string, "jrb") == 0) { return "Judeo-Arabic"; }
   if (strcmp(string, "kaa") == 0) { return "Kara-Kalpak"; }
   if (strcmp(string, "kab") == 0) { return "Kabyle"; }
   if (strcmp(string, "kac") == 0) { return "Kachin; Jingpho"; }
   if (strcmp(string, "kal") == 0) { return "Kalaallisut; Greenlandic"; }
   if (strcmp(string, "kl" ) == 0) { return "Kalaallisut; Greenlandic"; }
   if (strcmp(string, "kam") == 0) { return "Kamba"; }
   if (strcmp(string, "kan") == 0) { return "Kannada"; }
   if (strcmp(string, "kn" ) == 0) { return "Kannada"; }
   if (strcmp(string, "kar") == 0) { return "Karen languages"; }
   if (strcmp(string, "kas") == 0) { return "Kashmiri"; }
   if (strcmp(string, "ks" ) == 0) { return "Kashmiri"; }
   if (strcmp(string, "kau") == 0) { return "Kanuri"; }
   if (strcmp(string, "kr" ) == 0) { return "Kanuri"; }
   if (strcmp(string, "kaw") == 0) { return "Kawi"; }
   if (strcmp(string, "kaz") == 0) { return "Kazakh"; }
   if (strcmp(string, "kk" ) == 0) { return "Kazakh"; }
   if (strcmp(string, "kbd") == 0) { return "Kabardian"; }
   if (strcmp(string, "kha") == 0) { return "Khasi"; }
   if (strcmp(string, "khi") == 0) { return "Khoisan languages"; }
   if (strcmp(string, "khm") == 0) { return "Central Khmer"; }
   if (strcmp(string, "km" ) == 0) { return "Central Khmer"; }
   if (strcmp(string, "kho") == 0) { return "Khotanese; Sakan"; }
   if (strcmp(string, "kik") == 0) { return "Kikuyu; Gikuyu"; }
   if (strcmp(string, "ki" ) == 0) { return "Kikuyu; Gikuyu"; }
   if (strcmp(string, "kin") == 0) { return "Kinyarwanda"; }
   if (strcmp(string, "rw" ) == 0) { return "Kinyarwanda"; }
   if (strcmp(string, "kir") == 0) { return "Kirghiz; Kyrgyz"; }
   if (strcmp(string, "ky" ) == 0) { return "Kirghiz; Kyrgyz"; }
   if (strcmp(string, "kmb") == 0) { return "Kimbundu"; }
   if (strcmp(string, "kok") == 0) { return "Konkani"; }
   if (strcmp(string, "kom") == 0) { return "Komi"; }
   if (strcmp(string, "kv" ) == 0) { return "Komi"; }
   if (strcmp(string, "kon") == 0) { return "Kongo"; }
   if (strcmp(string, "kg" ) == 0) { return "Kongo"; }
   if (strcmp(string, "kor") == 0) { return "Korean"; }
   if (strcmp(string, "ko" ) == 0) { return "Korean"; }
   if (strcmp(string, "kos") == 0) { return "Kosraean"; }
   if (strcmp(string, "kpe") == 0) { return "Kpelle"; }
   if (strcmp(string, "krc") == 0) { return "Karachay-Balkar"; }
   if (strcmp(string, "krl") == 0) { return "Karelian"; }
   if (strcmp(string, "kro") == 0) { return "Kru languages"; }
   if (strcmp(string, "kru") == 0) { return "Kurukh"; }
   if (strcmp(string, "kua") == 0) { return "Kuanyama; Kwanyama"; }
   if (strcmp(string, "kj" ) == 0) { return "Kuanyama; Kwanyama"; }
   if (strcmp(string, "kum") == 0) { return "Kumyk"; }
   if (strcmp(string, "kur") == 0) { return "Kurdish"; }
   if (strcmp(string, "ku" ) == 0) { return "Kurdish"; }
   if (strcmp(string, "kut") == 0) { return "Kutenai"; }
   if (strcmp(string, "lad") == 0) { return "Ladino"; }
   if (strcmp(string, "lah") == 0) { return "Lahnda"; }
   if (strcmp(string, "lam") == 0) { return "Lamba"; }
   if (strcmp(string, "lao") == 0) { return "Lao"; }
   if (strcmp(string, "lo" ) == 0) { return "Lao"; }
   if (strcmp(string, "lat") == 0) { return "Latin"; }
   if (strcmp(string, "la" ) == 0) { return "Latin"; }
   if (strcmp(string, "lav") == 0) { return "Latvian"; }
   if (strcmp(string, "lv" ) == 0) { return "Latvian"; }
   if (strcmp(string, "lez") == 0) { return "Lezghian"; }
   if (strcmp(string, "lim") == 0) { return "Limburgan; Limburger; Limburgish"; }
   if (strcmp(string, "li" ) == 0) { return "Limburgan; Limburger; Limburgish"; }
   if (strcmp(string, "lin") == 0) { return "Lingala"; }
   if (strcmp(string, "ln" ) == 0) { return "Lingala"; }
   if (strcmp(string, "lit") == 0) { return "Lithuanian"; }
   if (strcmp(string, "lt" ) == 0) { return "Lithuanian"; }
   if (strcmp(string, "lol") == 0) { return "Mongo"; }
   if (strcmp(string, "loz") == 0) { return "Lozi"; }
   if (strcmp(string, "ltz") == 0) { return "Luxembourgish; Letzeburgesch"; }
   if (strcmp(string, "lb" ) == 0) { return "Luxembourgish; Letzeburgesch"; }
   if (strcmp(string, "lua") == 0) { return "Luba-Lulua"; }
   if (strcmp(string, "lub") == 0) { return "Luba-Katanga"; }
   if (strcmp(string, "lu" ) == 0) { return "Luba-Katanga"; }
   if (strcmp(string, "lug") == 0) { return "Ganda"; }
   if (strcmp(string, "lg" ) == 0) { return "Ganda"; }
   if (strcmp(string, "lui") == 0) { return "Luiseno"; }
   if (strcmp(string, "lun") == 0) { return "Lunda"; }
   if (strcmp(string, "luo") == 0) { return "Luo (Kenya and Tanzania)"; }
   if (strcmp(string, "lus") == 0) { return "Lushai"; }
   if (strcmp(string, "mac") == 0) { return "Macedonian"; }
   if (strcmp(string, "mkd") == 0) { return "Macedonian"; }
   if (strcmp(string, "mk" ) == 0) { return "Macedonian"; }
   if (strcmp(string, "mad") == 0) { return "Madurese"; }
   if (strcmp(string, "mag") == 0) { return "Magahi"; }
   if (strcmp(string, "mah") == 0) { return "Marshallese"; }
   if (strcmp(string, "mh" ) == 0) { return "Marshallese"; }
   if (strcmp(string, "mai") == 0) { return "Maithili"; }
   if (strcmp(string, "mak") == 0) { return "Makasar"; }
   if (strcmp(string, "mal") == 0) { return "Malayalam"; }
   if (strcmp(string, "ml" ) == 0) { return "Malayalam"; }
   if (strcmp(string, "man") == 0) { return "Mandingo"; }
   if (strcmp(string, "mao") == 0) { return "Maori"; }
   if (strcmp(string, "mri") == 0) { return "Maori"; }
   if (strcmp(string, "mi" ) == 0) { return "Maori"; }
   if (strcmp(string, "map") == 0) { return "Austronesian languages"; }
   if (strcmp(string, "mar") == 0) { return "Marathi"; }
   if (strcmp(string, "mr" ) == 0) { return "Marathi"; }
   if (strcmp(string, "mas") == 0) { return "Masai"; }
   if (strcmp(string, "may") == 0) { return "Malay"; }
   if (strcmp(string, "msa") == 0) { return "Malay"; }
   if (strcmp(string, "ms" ) == 0) { return "Malay"; }
   if (strcmp(string, "mdf") == 0) { return "Moksha"; }
   if (strcmp(string, "mdr") == 0) { return "Mandar"; }
   if (strcmp(string, "men") == 0) { return "Mende"; }
   if (strcmp(string, "mga") == 0) { return "Middle Irish"; }
   if (strcmp(string, "mic") == 0) { return "Mi'kmaq; Micmac"; }
   if (strcmp(string, "min") == 0) { return "Minangkabau"; }
   if (strcmp(string, "mis") == 0) { return "Uncoded languages"; }
   if (strcmp(string, "mkh") == 0) { return "Mon-Khmer languages"; }
   if (strcmp(string, "mlg") == 0) { return "Malagasy"; }
   if (strcmp(string, "mg" ) == 0) { return "Malagasy"; }
   if (strcmp(string, "mlt") == 0) { return "Maltese"; }
   if (strcmp(string, "mt" ) == 0) { return "Maltese"; }
   if (strcmp(string, "mnc") == 0) { return "Manchu"; }
   if (strcmp(string, "mni") == 0) { return "Manipuri"; }
   if (strcmp(string, "mno") == 0) { return "Manobo languages"; }
   if (strcmp(string, "moh") == 0) { return "Mohawk"; }
   if (strcmp(string, "mon") == 0) { return "Mongolian"; }
   if (strcmp(string, "mn" ) == 0) { return "Mongolian"; }
   if (strcmp(string, "mos") == 0) { return "Mossi"; }
   if (strcmp(string, "mul") == 0) { return "Multiple languages"; }
   if (strcmp(string, "mun") == 0) { return "Munda languages"; }
   if (strcmp(string, "mus") == 0) { return "Creek"; }
   if (strcmp(string, "mwl") == 0) { return "Mirandese"; }
   if (strcmp(string, "mwr") == 0) { return "Marwari"; }
   if (strcmp(string, "myn") == 0) { return "Mayan languages"; }
   if (strcmp(string, "myv") == 0) { return "Erzya"; }
   if (strcmp(string, "nah") == 0) { return "Nahuatl languages"; }
   if (strcmp(string, "nai") == 0) { return "North American Indian languages"; }
   if (strcmp(string, "nap") == 0) { return "Neapolitan"; }
   if (strcmp(string, "nau") == 0) { return "Nauru"; }
   if (strcmp(string, "na" ) == 0) { return "Nauru"; }
   if (strcmp(string, "nav") == 0) { return "Navajo; Navaho"; }
   if (strcmp(string, "nv" ) == 0) { return "Navajo; Navaho"; }
   if (strcmp(string, "nbl") == 0) { return "South Ndebele"; }
   if (strcmp(string, "nr" ) == 0) { return "South Ndebele"; }
   if (strcmp(string, "nde") == 0) { return "North Ndebele"; }
   if (strcmp(string, "nd" ) == 0) { return "North Ndebele"; }
   if (strcmp(string, "ndo") == 0) { return "Ndonga"; }
   if (strcmp(string, "ng" ) == 0) { return "Ndonga"; }
   if (strcmp(string, "nds") == 0) { return "Low German; Low Saxon;"; }
   if (strcmp(string, "nep") == 0) { return "Nepali"; }
   if (strcmp(string, "ne" ) == 0) { return "Nepali"; }
   if (strcmp(string, "new") == 0) { return "Nepal Bhasa; Newari"; }
   if (strcmp(string, "nia") == 0) { return "Nias"; }
   if (strcmp(string, "nic") == 0) { return "Niger-Kordofanian languages"; }
   if (strcmp(string, "niu") == 0) { return "Niuean"; }
   if (strcmp(string, "nno") == 0) { return "Norwegian Nynorsk"; }
   if (strcmp(string, "nn" ) == 0) { return "Norwegian Nynorsk"; }
   if (strcmp(string, "nob") == 0) { return "Norwegian Bokml"; }
   if (strcmp(string, "nb" ) == 0) { return "Norwegian Bokml"; }
   if (strcmp(string, "nog") == 0) { return "Nogai"; }
   if (strcmp(string, "non") == 0) { return "Old Norse"; }
   if (strcmp(string, "nor") == 0) { return "Norwegian"; }
   if (strcmp(string, "no" ) == 0) { return "Norwegian"; }
   if (strcmp(string, "nqo") == 0) { return "N'Ko"; }
   if (strcmp(string, "nso") == 0) { return "Pedi; Sepedi; Northern Sotho"; }
   if (strcmp(string, "nub") == 0) { return "Nubian languages"; }
   if (strcmp(string, "nwc") == 0) { return "Classical Newari; Old Newari; Classical Nepal Bhasa"; }
   if (strcmp(string, "nya") == 0) { return "Chichewa; Chewa; Nyanja"; }
   if (strcmp(string, "ny" ) == 0) { return "Chichewa; Chewa; Nyanja"; }
   if (strcmp(string, "nym") == 0) { return "Nyamwezi"; }
   if (strcmp(string, "nyn") == 0) { return "Nyankole"; }
   if (strcmp(string, "nyo") == 0) { return "Nyoro"; }
   if (strcmp(string, "nzi") == 0) { return "Nzima"; }
   if (strcmp(string, "oci") == 0) { return "Occitan"; }
   if (strcmp(string, "oc" ) == 0) { return "Occitan"; }
   if (strcmp(string, "oji") == 0) { return "Ojibwa"; }
   if (strcmp(string, "oj" ) == 0) { return "Ojibwa"; }
   if (strcmp(string, "ori") == 0) { return "Oriya"; }
   if (strcmp(string, "or" ) == 0) { return "Oriya"; }
   if (strcmp(string, "orm") == 0) { return "Oromo"; }
   if (strcmp(string, "om" ) == 0) { return "Oromo"; }
   if (strcmp(string, "osa") == 0) { return "Osage"; }
   if (strcmp(string, "oss") == 0) { return "Ossetian; Ossetic"; }
   if (strcmp(string, "os" ) == 0) { return "Ossetian; Ossetic"; }
   if (strcmp(string, "ota") == 0) { return "Ottoman Turkish"; }
   if (strcmp(string, "oto") == 0) { return "Otomian languages"; }
   if (strcmp(string, "paa") == 0) { return "Papuan languages"; }
   if (strcmp(string, "pag") == 0) { return "Pangasinan"; }
   if (strcmp(string, "pal") == 0) { return "Pahlavi"; }
   if (strcmp(string, "pam") == 0) { return "Pampanga; Kapampangan"; }
   if (strcmp(string, "pan") == 0) { return "Panjabi; Punjabi"; }
   if (strcmp(string, "pa" ) == 0) { return "Panjabi; Punjabi"; }
   if (strcmp(string, "pap") == 0) { return "Papiamento"; }
   if (strcmp(string, "pau") == 0) { return "Palauan"; }
   if (strcmp(string, "peo") == 0) { return "Old Persian"; }
   if (strcmp(string, "per") == 0) { return "Persian"; }
   if (strcmp(string, "fas") == 0) { return "Persian"; }
   if (strcmp(string, "fa" ) == 0) { return "Persian"; }
   if (strcmp(string, "phi") == 0) { return "Philippine languages)"; }
   if (strcmp(string, "phn") == 0) { return "Phoenician"; }
   if (strcmp(string, "pli") == 0) { return "Pali"; }
   if (strcmp(string, "pi" ) == 0) { return "Pali"; }
   if (strcmp(string, "pol") == 0) { return "Polish"; }
   if (strcmp(string, "pl" ) == 0) { return "Polish"; }
   if (strcmp(string, "pon") == 0) { return "Pohnpeian"; }
   if (strcmp(string, "por") == 0) { return "Portuguese"; }
   if (strcmp(string, "pt" ) == 0) { return "Portuguese"; }
   if (strcmp(string, "pra") == 0) { return "Prakrit languages"; }
   if (strcmp(string, "pro") == 0) { return "Old Provenal; Old Occitan"; }
   if (strcmp(string, "pus") == 0) { return "Pushto; Pashto"; }
   if (strcmp(string, "ps" ) == 0) { return "Pushto; Pashto"; }
   if (strcmp(string, "qaa-qtz") == 0) { return "Reserved for local use"; }
   if (strcmp(string, "que") == 0) { return "Quechua"; }
   if (strcmp(string, "qu" ) == 0) { return "Quechua"; }
   if (strcmp(string, "raj") == 0) { return "Rajasthani"; }
   if (strcmp(string, "rap") == 0) { return "Rapanui"; }
   if (strcmp(string, "rar") == 0) { return "Rarotongan; Cook Islands Maori"; }
   if (strcmp(string, "roa") == 0) { return "Romance languages"; }
   if (strcmp(string, "roh") == 0) { return "Romansh"; }
   if (strcmp(string, "rm" ) == 0) { return "Romansh"; }
   if (strcmp(string, "rom") == 0) { return "Romany"; }
   if (strcmp(string, "rum") == 0) { return "Romanian; Moldavian; Moldovan"; }
   if (strcmp(string, "ron") == 0) { return "Romanian; Moldavian; Moldovan"; }
   if (strcmp(string, "ro" ) == 0) { return "Romanian; Moldavian; Moldovan"; }
   if (strcmp(string, "run") == 0) { return "Rundi"; }
   if (strcmp(string, "rn" ) == 0) { return "Rundi"; }
   if (strcmp(string, "rup") == 0) { return "Aromanian; Arumanian; Macedo-Romanian"; }
   if (strcmp(string, "rus") == 0) { return "Russian"; }
   if (strcmp(string, "ru" ) == 0) { return "Russian"; }
   if (strcmp(string, "sad") == 0) { return "Sandawe"; }
   if (strcmp(string, "sag") == 0) { return "Sango"; }
   if (strcmp(string, "sg" ) == 0) { return "Sango"; }
   if (strcmp(string, "sah") == 0) { return "Yakut"; }
   if (strcmp(string, "sai") == 0) { return "South American Indian languages"; }
   if (strcmp(string, "sal") == 0) { return "Salishan languages"; }
   if (strcmp(string, "sam") == 0) { return "Samaritan Aramaic"; }
   if (strcmp(string, "san") == 0) { return "Sanskrit"; }
   if (strcmp(string, "sa" ) == 0) { return "Sanskrit"; }
   if (strcmp(string, "sas") == 0) { return "Sasak"; }
   if (strcmp(string, "sat") == 0) { return "Santali"; }
   if (strcmp(string, "scn") == 0) { return "Sicilian"; }
   if (strcmp(string, "sco") == 0) { return "Scots"; }
   if (strcmp(string, "sel") == 0) { return "Selkup"; }
   if (strcmp(string, "sem") == 0) { return "Semitic languages"; }
   if (strcmp(string, "sga") == 0) { return "Old Irish"; }
   if (strcmp(string, "sgn") == 0) { return "Sign Languages"; }
   if (strcmp(string, "shn") == 0) { return "Shan"; }
   if (strcmp(string, "sid") == 0) { return "Sidamo"; }
   if (strcmp(string, "sin") == 0) { return "Sinhala; Sinhalese"; }
   if (strcmp(string, "si" ) == 0) { return "Sinhala; Sinhalese"; }
   if (strcmp(string, "sio") == 0) { return "Siouan languages"; }
   if (strcmp(string, "sit") == 0) { return "Sino-Tibetan languages"; }
   if (strcmp(string, "sla") == 0) { return "Slavic languages"; }
   if (strcmp(string, "slo") == 0) { return "Slovak"; }
   if (strcmp(string, "slk") == 0) { return "Slovak"; }
   if (strcmp(string, "sk" ) == 0) { return "Slovak"; }
   if (strcmp(string, "slv") == 0) { return "Slovenian"; }
   if (strcmp(string, "sl" ) == 0) { return "Slovenian"; }
   if (strcmp(string, "sma") == 0) { return "Southern Sami"; }
   if (strcmp(string, "sme") == 0) { return "Northern Sami"; }
   if (strcmp(string, "se" ) == 0) { return "Northern Sami"; }
   if (strcmp(string, "smi") == 0) { return "Sami languages"; }
   if (strcmp(string, "smj") == 0) { return "Lule Sami"; }
   if (strcmp(string, "smn") == 0) { return "Inari Sami"; }
   if (strcmp(string, "smo") == 0) { return "Samoan"; }
   if (strcmp(string, "sm" ) == 0) { return "Samoan"; }
   if (strcmp(string, "sms") == 0) { return "Skolt Sami"; }
   if (strcmp(string, "sna") == 0) { return "Shona"; }
   if (strcmp(string, "sn" ) == 0) { return "Shona"; }
   if (strcmp(string, "snd") == 0) { return "Sindhi"; }
   if (strcmp(string, "sd" ) == 0) { return "Sindhi"; }
   if (strcmp(string, "snk") == 0) { return "Soninke"; }
   if (strcmp(string, "sog") == 0) { return "Sogdian"; }
   if (strcmp(string, "som") == 0) { return "Somali"; }
   if (strcmp(string, "so" ) == 0) { return "Somali"; }
   if (strcmp(string, "son") == 0) { return "Songhai languages"; }
   if (strcmp(string, "sot") == 0) { return "Southern Sotho"; }
   if (strcmp(string, "st" ) == 0) { return "Southern Sotho"; }
   if (strcmp(string, "spa") == 0) { return "Spanish; Castilian"; }
   if (strcmp(string, "es" ) == 0) { return "Spanish; Castilian"; }
   if (strcmp(string, "srd") == 0) { return "Sardinian"; }
   if (strcmp(string, "sc" ) == 0) { return "Sardinian"; }
   if (strcmp(string, "srn") == 0) { return "Sranan Tongo"; }
   if (strcmp(string, "srp") == 0) { return "Serbian"; }
   if (strcmp(string, "sr" ) == 0) { return "Serbian"; }
   if (strcmp(string, "srr") == 0) { return "Serer"; }
   if (strcmp(string, "ssa") == 0) { return "Nilo-Saharan languages"; }
   if (strcmp(string, "ssw") == 0) { return "Swati"; }
   if (strcmp(string, "ss" ) == 0) { return "Swati"; }
   if (strcmp(string, "suk") == 0) { return "Sukuma"; }
   if (strcmp(string, "sun") == 0) { return "Sundanese"; }
   if (strcmp(string, "su" ) == 0) { return "Sundanese"; }
   if (strcmp(string, "sus") == 0) { return "Susu"; }
   if (strcmp(string, "sux") == 0) { return "Sumerian"; }
   if (strcmp(string, "swa") == 0) { return "Swahili"; }
   if (strcmp(string, "sw" ) == 0) { return "Swahili"; }
   if (strcmp(string, "swe") == 0) { return "Swedish"; }
   if (strcmp(string, "sv" ) == 0) { return "Swedish"; }
   if (strcmp(string, "syc") == 0) { return "Classical Syriac"; }
   if (strcmp(string, "syr") == 0) { return "Syriac"; }
   if (strcmp(string, "tah") == 0) { return "Tahitian"; }
   if (strcmp(string, "ty" ) == 0) { return "Tahitian"; }
   if (strcmp(string, "tai") == 0) { return "Tai languages"; }
   if (strcmp(string, "tam") == 0) { return "Tamil"; }
   if (strcmp(string, "ta" ) == 0) { return "Tamil"; }
   if (strcmp(string, "tat") == 0) { return "Tatar"; }
   if (strcmp(string, "tt" ) == 0) { return "Tatar"; }
   if (strcmp(string, "tel") == 0) { return "Telugu"; }
   if (strcmp(string, "te" ) == 0) { return "Telugu"; }
   if (strcmp(string, "tem") == 0) { return "Timne"; }
   if (strcmp(string, "ter") == 0) { return "Tereno"; }
   if (strcmp(string, "tet") == 0) { return "Tetum"; }
   if (strcmp(string, "tgk") == 0) { return "Tajik"; }
   if (strcmp(string, "tg" ) == 0) { return "Tajik"; }
   if (strcmp(string, "tgl") == 0) { return "Tagalog"; }
   if (strcmp(string, "tl" ) == 0) { return "Tagalog"; }
   if (strcmp(string, "tha") == 0) { return "Thai"; }
   if (strcmp(string, "th" ) == 0) { return "Thai"; }
   if (strcmp(string, "tib") == 0) { return "Tibetan"; }
   if (strcmp(string, "bod") == 0) { return "Tibetan"; }
   if (strcmp(string, "bo" ) == 0) { return "Tibetan"; }
   if (strcmp(string, "tig") == 0) { return "Tigre"; }
   if (strcmp(string, "tir") == 0) { return "Tigrinya"; }
   if (strcmp(string, "ti" ) == 0) { return "Tigrinya"; }
   if (strcmp(string, "tiv") == 0) { return "Tiv"; }
   if (strcmp(string, "tkl") == 0) { return "Tokelau"; }
   if (strcmp(string, "tlh") == 0) { return "Klingon; tlhIngan-Hol"; }
   if (strcmp(string, "tli") == 0) { return "Tlingit"; }
   if (strcmp(string, "tmh") == 0) { return "Tamashek"; }
   if (strcmp(string, "tog") == 0) { return "Tonga (Nyasa)"; }
   if (strcmp(string, "ton") == 0) { return "Tonga"; }
   if (strcmp(string, "to" ) == 0) { return "Tonga"; }
   if (strcmp(string, "tpi") == 0) { return "Tok Pisin"; }
   if (strcmp(string, "tsi") == 0) { return "Tsimshian"; }
   if (strcmp(string, "tsn") == 0) { return "Tswana"; }
   if (strcmp(string, "tn" ) == 0) { return "Tswana"; }
   if (strcmp(string, "tso") == 0) { return "Tsonga"; }
   if (strcmp(string, "ts" ) == 0) { return "Tsonga"; }
   if (strcmp(string, "tuk") == 0) { return "Turkmen"; }
   if (strcmp(string, "tk" ) == 0) { return "Turkmen"; }
   if (strcmp(string, "tum") == 0) { return "Tumbuka"; }
   if (strcmp(string, "tup") == 0) { return "Tupi languages"; }
   if (strcmp(string, "tur") == 0) { return "Turkish"; }
   if (strcmp(string, "tr" ) == 0) { return "Turkish"; }
   if (strcmp(string, "tut") == 0) { return "Altaic languages"; }
   if (strcmp(string, "tvl") == 0) { return "Tuvalu"; }
   if (strcmp(string, "twi") == 0) { return "Twi"; }
   if (strcmp(string, "tw" ) == 0) { return "Twi"; }
   if (strcmp(string, "tyv") == 0) { return "Tuvinian"; }
   if (strcmp(string, "udm") == 0) { return "Udmurt"; }
   if (strcmp(string, "uga") == 0) { return "Ugaritic"; }
   if (strcmp(string, "uig") == 0) { return "Uighur; Uyghur"; }
   if (strcmp(string, "ug" ) == 0) { return "Uighur; Uyghur"; }
   if (strcmp(string, "ukr") == 0) { return "Ukrainian"; }
   if (strcmp(string, "uk" ) == 0) { return "Ukrainian"; }
   if (strcmp(string, "umb") == 0) { return "Umbundu"; }
   if (strcmp(string, "und") == 0) { return "Undetermined"; }
   if (strcmp(string, "urd") == 0) { return "Urdu"; }
   if (strcmp(string, "ur" ) == 0) { return "Urdu"; }
   if (strcmp(string, "uzb") == 0) { return "Uzbek"; }
   if (strcmp(string, "uz" ) == 0) { return "Uzbek"; }
   if (strcmp(string, "vai") == 0) { return "Vai"; }
   if (strcmp(string, "ven") == 0) { return "Venda"; }
   if (strcmp(string, "ve" ) == 0) { return "Venda"; }
   if (strcmp(string, "vie") == 0) { return "Vietnamese"; }
   if (strcmp(string, "vi" ) == 0) { return "Vietnamese"; }
   if (strcmp(string, "vol") == 0) { return "Volapk"; }
   if (strcmp(string, "vo" ) == 0) { return "Volapk"; }
   if (strcmp(string, "vot") == 0) { return "Votic"; }
   if (strcmp(string, "wak") == 0) { return "Wakashan languages"; }
   if (strcmp(string, "wal") == 0) { return "Wolaitta; Wolaytta"; }
   if (strcmp(string, "war") == 0) { return "Waray"; }
   if (strcmp(string, "was") == 0) { return "Washo"; }
   if (strcmp(string, "wel") == 0) { return "Welsh"; }
   if (strcmp(string, "cym") == 0) { return "Welsh"; }
   if (strcmp(string, "cy" ) == 0) { return "Welsh"; }
   if (strcmp(string, "wen") == 0) { return "Sorbian languages"; }
   if (strcmp(string, "wln") == 0) { return "Walloon"; }
   if (strcmp(string, "wa" ) == 0) { return "Walloon"; }
   if (strcmp(string, "wol") == 0) { return "Wolof"; }
   if (strcmp(string, "wo" ) == 0) { return "Wolof"; }
   if (strcmp(string, "xal") == 0) { return "Kalmyk; Oirat"; }
   if (strcmp(string, "xho") == 0) { return "Xhosa"; }
   if (strcmp(string, "xh" ) == 0) { return "Xhosa"; }
   if (strcmp(string, "yao") == 0) { return "Yao"; }
   if (strcmp(string, "yap") == 0) { return "Yapese"; }
   if (strcmp(string, "yid") == 0) { return "Yiddish"; }
   if (strcmp(string, "yi" ) == 0) { return "Yiddish"; }
   if (strcmp(string, "yor") == 0) { return "Yoruba"; }
   if (strcmp(string, "yo" ) == 0) { return "Yoruba"; }
   if (strcmp(string, "ypk") == 0) { return "Yupik languages"; }
   if (strcmp(string, "zap") == 0) { return "Zapotec"; }
   if (strcmp(string, "zbl") == 0) { return "Blissymbols; Blissymbolics; Bliss"; }
   if (strcmp(string, "zen") == 0) { return "Zenaga"; }
   if (strcmp(string, "zha") == 0) { return "Zhuang; Chuang"; }
   if (strcmp(string, "za" ) == 0) { return "Zhuang; Chuang"; }
   if (strcmp(string, "znd") == 0) { return "Zande languages"; }
   if (strcmp(string, "zul") == 0) { return "Zulu"; }
   if (strcmp(string, "zu" ) == 0) { return "Zulu"; }
   if (strcmp(string, "zun") == 0) { return "Zuni"; }
   if (strcmp(string, "zxx") == 0) { return "No linguistic content; Not applicable"; }
   if (strcmp(string, "zza") == 0) { return "Zaza; Dimili; Dimli; Kirdki; Kirmanjki; Zazaki"; }

   return "";
}



//////////////////////////////
//
// HumdrumRecord::getBibLangIso639_2 --  Returns the ISO 639.2 three-letter
//    language code for the given record if it is a bibliographic 
//    record.  Returns and empty string if there is no language
//    specification.
//
//    http://www.loc.gov/standards/iso639-2/ISO-639-2_8859-1.txt
//

const char* HumdrumRecord::getBibLangIso639_2(const char* string) {
   char buffer[1024] = {0};
   char* ptr = NULL;

   if (string != NULL) {
      // use like a static function later?
      return "";
   } else {
      getBibKey(buffer, 1000);
      ptr = strrchr(buffer, '@');
   }

   if (ptr == NULL) {
      return "";
   } else {
      ptr++;
   }

   int len = strlen(ptr);
   if ((len > 3) || (len < 2)) {
      // don't know what to do with a long code, since 2 or three letter
      // code is expected
      return "";
   }

   int i;
   for (i=0; i<len; i++) {
      ptr[i] = tolower(ptr[i]);
   }
   
   if (strcmp(ptr, "aar") == 0) { return "aar"; }  // Afar
   if (strcmp(ptr, "aa" ) == 0) { return "aar"; }  // Afar
   if (strcmp(ptr, "abk") == 0) { return "abk"; }  // Abkhazian
   if (strcmp(ptr, "ab" ) == 0) { return "abk"; }  // Abkhazian
   if (strcmp(ptr, "ace") == 0) { return "ace"; }  // Achinese
   if (strcmp(ptr, "ach") == 0) { return "ach"; }  // Acoli
   if (strcmp(ptr, "ada") == 0) { return "ada"; }  // Adangme
   if (strcmp(ptr, "ady") == 0) { return "ady"; }  // Adyghe; Adygei
   if (strcmp(ptr, "afa") == 0) { return "afa"; }  // Afro-Asiatic languages
   if (strcmp(ptr, "afh") == 0) { return "afh"; }  // Afrihili
   if (strcmp(ptr, "afr") == 0) { return "afr"; }  // Afrikaans
   if (strcmp(ptr, "af" ) == 0) { return "afr"; }  // Afrikaans
   if (strcmp(ptr, "ain") == 0) { return "ain"; }  // Ainu
   if (strcmp(ptr, "aka") == 0) { return "aka"; }  // Akan
   if (strcmp(ptr, "ak" ) == 0) { return "aka"; }  // Akan
   if (strcmp(ptr, "akk") == 0) { return "akk"; }  // Akkadian
   if (strcmp(ptr, "alb") == 0) { return "alb"; }  // Albanian
   if (strcmp(ptr, "sqi") == 0) { return "alb"; }  // Albanian
   if (strcmp(ptr, "sq" ) == 0) { return "alb"; }  // Albanian
   if (strcmp(ptr, "ale") == 0) { return "ale"; }  // Aleut
   if (strcmp(ptr, "alg") == 0) { return "alg"; }  // Algonquian languages
   if (strcmp(ptr, "alt") == 0) { return "alt"; }  // Southern Altai
   if (strcmp(ptr, "amh") == 0) { return "amh"; }  // Amharic
   if (strcmp(ptr, "am" ) == 0) { return "amh"; }  // Amharic
   if (strcmp(ptr, "ang") == 0) { return "ang"; }  // English, Old (ca.450-1100)
   if (strcmp(ptr, "anp") == 0) { return "anp"; }  // Angika
   if (strcmp(ptr, "apa") == 0) { return "apa"; }  // Apache languages
   if (strcmp(ptr, "ara") == 0) { return "ara"; }  // Arabic
   if (strcmp(ptr, "ar" ) == 0) { return "ara"; }  // Arabic
   if (strcmp(ptr, "arc") == 0) { return "arc"; }  // Official Aramaic (700-300 BCE); Imperial Aramaic (700-300 BCE)
   if (strcmp(ptr, "arg") == 0) { return "arg"; }  // Aragonese
   if (strcmp(ptr, "an" ) == 0) { return "arg"; }  // Aragonese
   if (strcmp(ptr, "arm") == 0) { return "arm"; }  // Armenian
   if (strcmp(ptr, "hye") == 0) { return "arm"; }  // Armenian
   if (strcmp(ptr, "hy" ) == 0) { return "arm"; }  // Armenian
   if (strcmp(ptr, "arn") == 0) { return "arn"; }  // Mapudungun; Mapuche
   if (strcmp(ptr, "arp") == 0) { return "arp"; }  // Arapaho
   if (strcmp(ptr, "art") == 0) { return "art"; }  // Artificial languages
   if (strcmp(ptr, "arw") == 0) { return "arw"; }  // Arawak
   if (strcmp(ptr, "asm") == 0) { return "asm"; }  // Assamese
   if (strcmp(ptr, "as" ) == 0) { return "asm"; }  // Assamese
   if (strcmp(ptr, "ast") == 0) { return "ast"; }  // Asturian; Bable; Leonese; Asturleonese
   if (strcmp(ptr, "ath") == 0) { return "ath"; }  // Athapascan languages
   if (strcmp(ptr, "aus") == 0) { return "aus"; }  // Australian languages
   if (strcmp(ptr, "ava") == 0) { return "ava"; }  // Avaric
   if (strcmp(ptr, "av" ) == 0) { return "ava"; }  // Avaric
   if (strcmp(ptr, "ave") == 0) { return "ave"; }  // Avestan
   if (strcmp(ptr, "ae" ) == 0) { return "ave"; }  // Avestan
   if (strcmp(ptr, "awa") == 0) { return "awa"; }  // Awadhi
   if (strcmp(ptr, "aym") == 0) { return "aym"; }  // Aymara
   if (strcmp(ptr, "ay" ) == 0) { return "aym"; }  // Aymara
   if (strcmp(ptr, "aze") == 0) { return "aze"; }  // Azerbaijani
   if (strcmp(ptr, "az" ) == 0) { return "aze"; }  // Azerbaijani
   if (strcmp(ptr, "bad") == 0) { return "bad"; }  // Banda languages
   if (strcmp(ptr, "bai") == 0) { return "bai"; }  // Bamileke languages
   if (strcmp(ptr, "bak") == 0) { return "bak"; }  // Bashkir
   if (strcmp(ptr, "ba" ) == 0) { return "bak"; }  // Bashkir
   if (strcmp(ptr, "bal") == 0) { return "bal"; }  // Baluchi
   if (strcmp(ptr, "bam") == 0) { return "bam"; }  // Bambara
   if (strcmp(ptr, "bm" ) == 0) { return "bam"; }  // Bambara
   if (strcmp(ptr, "ban") == 0) { return "ban"; }  // Balinese
   if (strcmp(ptr, "baq") == 0) { return "baq"; }  // Basque
   if (strcmp(ptr, "eus") == 0) { return "baq"; }  // Basque
   if (strcmp(ptr, "eu" ) == 0) { return "baq"; }  // Basque
   if (strcmp(ptr, "bas") == 0) { return "bas"; }  // Basa
   if (strcmp(ptr, "bat") == 0) { return "bat"; }  // Baltic languages
   if (strcmp(ptr, "bej") == 0) { return "bej"; }  // Beja; Bedawiyet
   if (strcmp(ptr, "bel") == 0) { return "bel"; }  // Belarusian
   if (strcmp(ptr, "be" ) == 0) { return "bel"; }  // Belarusian
   if (strcmp(ptr, "bem") == 0) { return "bem"; }  // Bemba
   if (strcmp(ptr, "ben") == 0) { return "ben"; }  // Bengali
   if (strcmp(ptr, "bn" ) == 0) { return "ben"; }  // Bengali
   if (strcmp(ptr, "ber") == 0) { return "ber"; }  // Berber languages)
   if (strcmp(ptr, "bho") == 0) { return "bho"; }  // Bhojpuri
   if (strcmp(ptr, "bih") == 0) { return "bih"; }  // Bihari
   if (strcmp(ptr, "bh" ) == 0) { return "bih"; }  // Bihari
   if (strcmp(ptr, "bik") == 0) { return "bik"; }  // Bikol
   if (strcmp(ptr, "bin") == 0) { return "bin"; }  // Bini; Edo
   if (strcmp(ptr, "bis") == 0) { return "bis"; }  // Bislama
   if (strcmp(ptr, "bi" ) == 0) { return "bis"; }  // Bislama
   if (strcmp(ptr, "bla") == 0) { return "bla"; }  // Siksika
   if (strcmp(ptr, "bnt") == 0) { return "bnt"; }  // Bantu languages
   if (strcmp(ptr, "bos") == 0) { return "bos"; }  // Bosnian
   if (strcmp(ptr, "bs" ) == 0) { return "bos"; }  // Bosnian
   if (strcmp(ptr, "bra") == 0) { return "bra"; }  // Braj
   if (strcmp(ptr, "bre") == 0) { return "bre"; }  // Breton
   if (strcmp(ptr, "br" ) == 0) { return "bre"; }  // Breton
   if (strcmp(ptr, "btk") == 0) { return "btk"; }  // Batak languages
   if (strcmp(ptr, "bua") == 0) { return "bua"; }  // Buriat
   if (strcmp(ptr, "bug") == 0) { return "bug"; }  // Buginese
   if (strcmp(ptr, "bul") == 0) { return "bul"; }  // Bulgarian
   if (strcmp(ptr, "bg" ) == 0) { return "bul"; }  // Bulgarian
   if (strcmp(ptr, "bur") == 0) { return "bur"; }  // Burmese
   if (strcmp(ptr, "mya") == 0) { return "bur"; }  // Burmese
   if (strcmp(ptr, "my" ) == 0) { return "bur"; }  // Burmese
   if (strcmp(ptr, "byn") == 0) { return "byn"; }  // Blin; Bilin
   if (strcmp(ptr, "cad") == 0) { return "cad"; }  // Caddo
   if (strcmp(ptr, "cai") == 0) { return "cai"; }  // Central American Indian languages
   if (strcmp(ptr, "car") == 0) { return "car"; }  // Galibi Carib
   if (strcmp(ptr, "cat") == 0) { return "cat"; }  // Catalan; Valencian
   if (strcmp(ptr, "ca" ) == 0) { return "cat"; }  // Catalan; Valencian
   if (strcmp(ptr, "cau") == 0) { return "cau"; }  // Caucasian languages
   if (strcmp(ptr, "ceb") == 0) { return "ceb"; }  // Cebuano
   if (strcmp(ptr, "cel") == 0) { return "cel"; }  // Celtic languages
   if (strcmp(ptr, "cha") == 0) { return "cha"; }  // Chamorro
   if (strcmp(ptr, "ch" ) == 0) { return "cha"; }  // Chamorro
   if (strcmp(ptr, "chb") == 0) { return "chb"; }  // Chibcha
   if (strcmp(ptr, "che") == 0) { return "che"; }  // Chechen
   if (strcmp(ptr, "ce" ) == 0) { return "che"; }  // Chechen
   if (strcmp(ptr, "chg") == 0) { return "chg"; }  // Chagatai
   if (strcmp(ptr, "chi") == 0) { return "chi"; }  // Chinese
   if (strcmp(ptr, "zho") == 0) { return "chi"; }  // Chinese
   if (strcmp(ptr, "zh" ) == 0) { return "chi"; }  // Chinese
   if (strcmp(ptr, "chk") == 0) { return "chk"; }  // Chuukese
   if (strcmp(ptr, "chm") == 0) { return "chm"; }  // Mari
   if (strcmp(ptr, "chn") == 0) { return "chn"; }  // Chinook jargon
   if (strcmp(ptr, "cho") == 0) { return "cho"; }  // Choctaw
   if (strcmp(ptr, "chp") == 0) { return "chp"; }  // Chipewyan; Dene Suline
   if (strcmp(ptr, "chr") == 0) { return "chr"; }  // Cherokee
   if (strcmp(ptr, "chu") == 0) { return "chu"; }  // Church Slavic; Old Slavonic; Church Slavonic; Old Bulgarian; Old Church Slavonic
   if (strcmp(ptr, "cu" ) == 0) { return "chu"; }  // Church Slavic; Old Slavonic; Church Slavonic; Old Bulgarian; Old Church Slavonic
   if (strcmp(ptr, "chv") == 0) { return "chv"; }  // Chuvash
   if (strcmp(ptr, "cv" ) == 0) { return "chv"; }  // Chuvash
   if (strcmp(ptr, "chy") == 0) { return "chy"; }  // Cheyenne
   if (strcmp(ptr, "cmc") == 0) { return "cmc"; }  // Chamic languages
   if (strcmp(ptr, "cop") == 0) { return "cop"; }  // Coptic
   if (strcmp(ptr, "cor") == 0) { return "cor"; }  // Cornish
   if (strcmp(ptr, "kw" ) == 0) { return "cor"; }  // Cornish
   if (strcmp(ptr, "cos") == 0) { return "cos"; }  // Corsican
   if (strcmp(ptr, "co" ) == 0) { return "cos"; }  // Corsican
   if (strcmp(ptr, "cpe") == 0) { return "cpe"; }  // Creoles and pidgins, English based
   if (strcmp(ptr, "cpf") == 0) { return "cpf"; }  // Creoles and pidgins, French-based
   if (strcmp(ptr, "cpp") == 0) { return "cpp"; }  // Creoles and pidgins, Portuguese-based
   if (strcmp(ptr, "cre") == 0) { return "cre"; }  // Cree
   if (strcmp(ptr, "cr" ) == 0) { return "cre"; }  // Cree
   if (strcmp(ptr, "crh") == 0) { return "crh"; }  // Crimean Tatar; Crimean Turkish
   if (strcmp(ptr, "crp") == 0) { return "crp"; }  // Creoles and pidgins
   if (strcmp(ptr, "csb") == 0) { return "csb"; }  // Kashubian
   if (strcmp(ptr, "cus") == 0) { return "cus"; }  // Cushitic languages
   if (strcmp(ptr, "cze") == 0) { return "cze"; }  // Czech
   if (strcmp(ptr, "ces") == 0) { return "cze"; }  // Czech
   if (strcmp(ptr, "cs" ) == 0) { return "cze"; }  // Czech
   if (strcmp(ptr, "dak") == 0) { return "dak"; }  // Dakota
   if (strcmp(ptr, "dan") == 0) { return "dan"; }  // Danish
   if (strcmp(ptr, "da" ) == 0) { return "dan"; }  // Danish
   if (strcmp(ptr, "dar") == 0) { return "dar"; }  // Dargwa
   if (strcmp(ptr, "day") == 0) { return "day"; }  // Land Dayak languages
   if (strcmp(ptr, "del") == 0) { return "del"; }  // Delaware
   if (strcmp(ptr, "den") == 0) { return "den"; }  // Slave (Athapascan)
   if (strcmp(ptr, "dgr") == 0) { return "dgr"; }  // Dogrib
   if (strcmp(ptr, "din") == 0) { return "din"; }  // Dinka
   if (strcmp(ptr, "div") == 0) { return "div"; }  // Divehi; Dhivehi; Maldivian
   if (strcmp(ptr, "dv" ) == 0) { return "div"; }  // Divehi; Dhivehi; Maldivian
   if (strcmp(ptr, "doi") == 0) { return "doi"; }  // Dogri
   if (strcmp(ptr, "dra") == 0) { return "dra"; }  // Dravidian languages
   if (strcmp(ptr, "dsb") == 0) { return "dsb"; }  // Lower Sorbian
   if (strcmp(ptr, "dua") == 0) { return "dua"; }  // Duala
   if (strcmp(ptr, "dum") == 0) { return "dum"; }  // Dutch, Middle (ca.1050-1350)
   if (strcmp(ptr, "dut") == 0) { return "dut"; }  // Dutch; Flemish
   if (strcmp(ptr, "nld") == 0) { return "dut"; }  // Dutch; Flemish
   if (strcmp(ptr, "nl" ) == 0) { return "dut"; }  // Dutch; Flemish
   if (strcmp(ptr, "dyu") == 0) { return "dyu"; }  // Dyula
   if (strcmp(ptr, "dzo") == 0) { return "dzo"; }  // Dzongkha
   if (strcmp(ptr, "dz" ) == 0) { return "dzo"; }  // Dzongkha
   if (strcmp(ptr, "efi") == 0) { return "efi"; }  // Efik
   if (strcmp(ptr, "egy") == 0) { return "egy"; }  // Egyptian (Ancient)
   if (strcmp(ptr, "eka") == 0) { return "eka"; }  // Ekajuk
   if (strcmp(ptr, "elx") == 0) { return "elx"; }  // Elamite
   if (strcmp(ptr, "eng") == 0) { return "eng"; }  // English
   if (strcmp(ptr, "en" ) == 0) { return "eng"; }  // English
   if (strcmp(ptr, "enm") == 0) { return "enm"; }  // English, Middle (1100-1500)
   if (strcmp(ptr, "epo") == 0) { return "epo"; }  // Esperanto
   if (strcmp(ptr, "eo" ) == 0) { return "epo"; }  // Esperanto
   if (strcmp(ptr, "est") == 0) { return "est"; }  // Estonian
   if (strcmp(ptr, "et" ) == 0) { return "est"; }  // Estonian
   if (strcmp(ptr, "ewe") == 0) { return "ewe"; }  // Ewe
   if (strcmp(ptr, "ee" ) == 0) { return "ewe"; }  // Ewe
   if (strcmp(ptr, "ewo") == 0) { return "ewo"; }  // Ewondo
   if (strcmp(ptr, "fan") == 0) { return "fan"; }  // Fang
   if (strcmp(ptr, "fao") == 0) { return "fao"; }  // Faroese
   if (strcmp(ptr, "fo" ) == 0) { return "fao"; }  // Faroese
   if (strcmp(ptr, "fat") == 0) { return "fat"; }  // Fanti
   if (strcmp(ptr, "fij") == 0) { return "fij"; }  // Fijian
   if (strcmp(ptr, "fj" ) == 0) { return "fij"; }  // Fijian
   if (strcmp(ptr, "fil") == 0) { return "fil"; }  // Filipino; Pilipino
   if (strcmp(ptr, "fin") == 0) { return "fin"; }  // Finnish
   if (strcmp(ptr, "fi" ) == 0) { return "fin"; }  // Finnish
   if (strcmp(ptr, "fiu") == 0) { return "fiu"; }  // Finno-Ugrian languages)
   if (strcmp(ptr, "fon") == 0) { return "fon"; }  // Fon
   if (strcmp(ptr, "fre") == 0) { return "fre"; }  // French
   if (strcmp(ptr, "fra") == 0) { return "fre"; }  // French
   if (strcmp(ptr, "fr" ) == 0) { return "fre"; }  // French
   if (strcmp(ptr, "frm") == 0) { return "frm"; }  // French, Middle (ca.1400-1600)
   if (strcmp(ptr, "fro") == 0) { return "fro"; }  // French, Old (842-ca.1400)
   if (strcmp(ptr, "frr") == 0) { return "frr"; }  // Northern Frisian
   if (strcmp(ptr, "frs") == 0) { return "frs"; }  // Eastern Frisian
   if (strcmp(ptr, "fry") == 0) { return "fry"; }  // Western Frisian
   if (strcmp(ptr, "fy" ) == 0) { return "fry"; }  // Western Frisian
   if (strcmp(ptr, "ful") == 0) { return "ful"; }  // Fulah
   if (strcmp(ptr, "ff" ) == 0) { return "ful"; }  // Fulah
   if (strcmp(ptr, "fur") == 0) { return "fur"; }  // Friulian
   if (strcmp(ptr, "gaa") == 0) { return "gaa"; }  // Ga
   if (strcmp(ptr, "gay") == 0) { return "gay"; }  // Gayo
   if (strcmp(ptr, "gba") == 0) { return "gba"; }  // Gbaya
   if (strcmp(ptr, "gem") == 0) { return "gem"; }  // Germanic languages
   if (strcmp(ptr, "geo") == 0) { return "geo"; }  // Georgian
   if (strcmp(ptr, "kat") == 0) { return "geo"; }  // Georgian
   if (strcmp(ptr, "ka" ) == 0) { return "geo"; }  // Georgian
   if (strcmp(ptr, "ger") == 0) { return "ger"; }  // German
   if (strcmp(ptr, "deu") == 0) { return "ger"; }  // German
   if (strcmp(ptr, "de" ) == 0) { return "ger"; }  // German
   if (strcmp(ptr, "gez") == 0) { return "gez"; }  // Geez
   if (strcmp(ptr, "gil") == 0) { return "gil"; }  // Gilbertese
   if (strcmp(ptr, "gla") == 0) { return "gla"; }  // Gaelic; Scottish Gaelic
   if (strcmp(ptr, "gd" ) == 0) { return "gla"; }  // Gaelic; Scottish Gaelic
   if (strcmp(ptr, "gle") == 0) { return "gle"; }  // Irish
   if (strcmp(ptr, "ga" ) == 0) { return "gle"; }  // Irish
   if (strcmp(ptr, "glg") == 0) { return "glg"; }  // Galician
   if (strcmp(ptr, "gl" ) == 0) { return "glg"; }  // Galician
   if (strcmp(ptr, "glv") == 0) { return "glv"; }  // Manx
   if (strcmp(ptr, "gv" ) == 0) { return "glv"; }  // Manx
   if (strcmp(ptr, "gmh") == 0) { return "gmh"; }  // German, Middle High (ca.1050-1500)
   if (strcmp(ptr, "goh") == 0) { return "goh"; }  // German, Old High (ca.750-1050)
   if (strcmp(ptr, "gon") == 0) { return "gon"; }  // Gondi
   if (strcmp(ptr, "gor") == 0) { return "gor"; }  // Gorontalo
   if (strcmp(ptr, "got") == 0) { return "got"; }  // Gothic
   if (strcmp(ptr, "grb") == 0) { return "grb"; }  // Grebo
   if (strcmp(ptr, "grc") == 0) { return "grc"; }  // Greek, Ancient (to 1453)
   if (strcmp(ptr, "gre") == 0) { return "gre"; }  // Greek, Modern (1453-)
   if (strcmp(ptr, "ell") == 0) { return "gre"; }  // Greek, Modern (1453-)
   if (strcmp(ptr, "el" ) == 0) { return "gre"; }  // Greek, Modern (1453-)
   if (strcmp(ptr, "grn") == 0) { return "grn"; }  // Guarani
   if (strcmp(ptr, "gn" ) == 0) { return "grn"; }  // Guarani
   if (strcmp(ptr, "gsw") == 0) { return "gsw"; }  // Swiss German; Alemannic; Alsatian
   if (strcmp(ptr, "guj") == 0) { return "guj"; }  // Gujarati
   if (strcmp(ptr, "gu" ) == 0) { return "guj"; }  // Gujarati
   if (strcmp(ptr, "gwi") == 0) { return "gwi"; }  // Gwich'in
   if (strcmp(ptr, "hai") == 0) { return "hai"; }  // Haida
   if (strcmp(ptr, "hat") == 0) { return "hat"; }  // Haitian; Haitian Creole
   if (strcmp(ptr, "ht" ) == 0) { return "hat"; }  // Haitian; Haitian Creole
   if (strcmp(ptr, "hau") == 0) { return "hau"; }  // Hausa
   if (strcmp(ptr, "ha" ) == 0) { return "hau"; }  // Hausa
   if (strcmp(ptr, "haw") == 0) { return "haw"; }  // Hawaiian
   if (strcmp(ptr, "heb") == 0) { return "heb"; }  // Hebrew
   if (strcmp(ptr, "he" ) == 0) { return "heb"; }  // Hebrew
   if (strcmp(ptr, "her") == 0) { return "her"; }  // Herero
   if (strcmp(ptr, "hz" ) == 0) { return "her"; }  // Herero
   if (strcmp(ptr, "hil") == 0) { return "hil"; }  // Hiligaynon
   if (strcmp(ptr, "him") == 0) { return "him"; }  // Himachali
   if (strcmp(ptr, "hin") == 0) { return "hin"; }  // Hindi
   if (strcmp(ptr, "hi" ) == 0) { return "hin"; }  // Hindi
   if (strcmp(ptr, "hit") == 0) { return "hit"; }  // Hittite
   if (strcmp(ptr, "hmn") == 0) { return "hmn"; }  // Hmong
   if (strcmp(ptr, "hmo") == 0) { return "hmo"; }  // Hiri Motu
   if (strcmp(ptr, "ho" ) == 0) { return "hmo"; }  // Hiri Motu
   if (strcmp(ptr, "hrv") == 0) { return "hrv"; }  // Croatian
   if (strcmp(ptr, "hr" ) == 0) { return "hrv"; }  // Croatian
   if (strcmp(ptr, "hsb") == 0) { return "hsb"; }  // Upper Sorbian
   if (strcmp(ptr, "hun") == 0) { return "hun"; }  // Hungarian
   if (strcmp(ptr, "hu" ) == 0) { return "hun"; }  // Hungarian
   if (strcmp(ptr, "hup") == 0) { return "hup"; }  // Hupa
   if (strcmp(ptr, "iba") == 0) { return "iba"; }  // Iban
   if (strcmp(ptr, "ibo") == 0) { return "ibo"; }  // Igbo
   if (strcmp(ptr, "ig" ) == 0) { return "ibo"; }  // Igbo
   if (strcmp(ptr, "ice") == 0) { return "ice"; }  // Icelandic
   if (strcmp(ptr, "isl") == 0) { return "ice"; }  // Icelandic
   if (strcmp(ptr, "is" ) == 0) { return "ice"; }  // Icelandic
   if (strcmp(ptr, "ido") == 0) { return "ido"; }  // Ido
   if (strcmp(ptr, "io" ) == 0) { return "ido"; }  // Ido
   if (strcmp(ptr, "iii") == 0) { return "iii"; }  // Sichuan Yi; Nuosu
   if (strcmp(ptr, "ii" ) == 0) { return "iii"; }  // Sichuan Yi; Nuosu
   if (strcmp(ptr, "ijo") == 0) { return "ijo"; }  // Ijo languages
   if (strcmp(ptr, "iku") == 0) { return "iku"; }  // Inuktitut
   if (strcmp(ptr, "iu" ) == 0) { return "iku"; }  // Inuktitut
   if (strcmp(ptr, "ile") == 0) { return "ile"; }  // Interlingue; Occidental
   if (strcmp(ptr, "ie" ) == 0) { return "ile"; }  // Interlingue; Occidental
   if (strcmp(ptr, "ilo") == 0) { return "ilo"; }  // Iloko
   if (strcmp(ptr, "ina") == 0) { return "ina"; }  // Interlingua (International Auxiliary Language Association)
   if (strcmp(ptr, "ia" ) == 0) { return "ina"; }  // Interlingua (International Auxiliary Language Association)
   if (strcmp(ptr, "inc") == 0) { return "inc"; }  // Indic languages
   if (strcmp(ptr, "ind") == 0) { return "ind"; }  // Indonesian
   if (strcmp(ptr, "id" ) == 0) { return "ind"; }  // Indonesian
   if (strcmp(ptr, "ine") == 0) { return "ine"; }  // Indo-European languages
   if (strcmp(ptr, "inh") == 0) { return "inh"; }  // Ingush
   if (strcmp(ptr, "ipk") == 0) { return "ipk"; }  // Inupiaq
   if (strcmp(ptr, "ik" ) == 0) { return "ipk"; }  // Inupiaq
   if (strcmp(ptr, "ira") == 0) { return "ira"; }  // Iranian languages
   if (strcmp(ptr, "iro") == 0) { return "iro"; }  // Iroquoian languages
   if (strcmp(ptr, "ita") == 0) { return "ita"; }  // Italian
   if (strcmp(ptr, "it" ) == 0) { return "ita"; }  // Italian
   if (strcmp(ptr, "jav") == 0) { return "jav"; }  // Javanese
   if (strcmp(ptr, "jv" ) == 0) { return "jav"; }  // Javanese
   if (strcmp(ptr, "jbo") == 0) { return "jbo"; }  // Lojban
   if (strcmp(ptr, "jpn") == 0) { return "jpn"; }  // Japanese
   if (strcmp(ptr, "ja" ) == 0) { return "jpn"; }  // Japanese
   if (strcmp(ptr, "jpr") == 0) { return "jpr"; }  // Judeo-Persian
   if (strcmp(ptr, "jrb") == 0) { return "jrb"; }  // Judeo-Arabic
   if (strcmp(ptr, "kaa") == 0) { return "kaa"; }  // Kara-Kalpak
   if (strcmp(ptr, "kab") == 0) { return "kab"; }  // Kabyle
   if (strcmp(ptr, "kac") == 0) { return "kac"; }  // Kachin; Jingpho
   if (strcmp(ptr, "kal") == 0) { return "kal"; }  // Kalaallisut; Greenlandic
   if (strcmp(ptr, "kl" ) == 0) { return "kal"; }  // Kalaallisut; Greenlandic
   if (strcmp(ptr, "kam") == 0) { return "kam"; }  // Kamba
   if (strcmp(ptr, "kan") == 0) { return "kan"; }  // Kannada
   if (strcmp(ptr, "kn" ) == 0) { return "kan"; }  // Kannada
   if (strcmp(ptr, "kar") == 0) { return "kar"; }  // Karen languages
   if (strcmp(ptr, "kas") == 0) { return "kas"; }  // Kashmiri
   if (strcmp(ptr, "ks" ) == 0) { return "kas"; }  // Kashmiri
   if (strcmp(ptr, "kau") == 0) { return "kau"; }  // Kanuri
   if (strcmp(ptr, "kr" ) == 0) { return "kau"; }  // Kanuri
   if (strcmp(ptr, "kaw") == 0) { return "kaw"; }  // Kawi
   if (strcmp(ptr, "kaz") == 0) { return "kaz"; }  // Kazakh
   if (strcmp(ptr, "kk" ) == 0) { return "kaz"; }  // Kazakh
   if (strcmp(ptr, "kbd") == 0) { return "kbd"; }  // Kabardian
   if (strcmp(ptr, "kha") == 0) { return "kha"; }  // Khasi
   if (strcmp(ptr, "khi") == 0) { return "khi"; }  // Khoisan languages
   if (strcmp(ptr, "khm") == 0) { return "khm"; }  // Central Khmer
   if (strcmp(ptr, "km" ) == 0) { return "khm"; }  // Central Khmer
   if (strcmp(ptr, "kho") == 0) { return "kho"; }  // Khotanese; Sakan
   if (strcmp(ptr, "kik") == 0) { return "kik"; }  // Kikuyu; Gikuyu
   if (strcmp(ptr, "ki" ) == 0) { return "kik"; }  // Kikuyu; Gikuyu
   if (strcmp(ptr, "kin") == 0) { return "kin"; }  // Kinyarwanda
   if (strcmp(ptr, "rw" ) == 0) { return "kin"; }  // Kinyarwanda
   if (strcmp(ptr, "kir") == 0) { return "kir"; }  // Kirghiz; Kyrgyz
   if (strcmp(ptr, "ky" ) == 0) { return "kir"; }  // Kirghiz; Kyrgyz
   if (strcmp(ptr, "kmb") == 0) { return "kmb"; }  // Kimbundu
   if (strcmp(ptr, "kok") == 0) { return "kok"; }  // Konkani
   if (strcmp(ptr, "kom") == 0) { return "kom"; }  // Komi
   if (strcmp(ptr, "kv" ) == 0) { return "kom"; }  // Komi
   if (strcmp(ptr, "kon") == 0) { return "kon"; }  // Kongo
   if (strcmp(ptr, "kg" ) == 0) { return "kon"; }  // Kongo
   if (strcmp(ptr, "kor") == 0) { return "kor"; }  // Korean
   if (strcmp(ptr, "ko" ) == 0) { return "kor"; }  // Korean
   if (strcmp(ptr, "kos") == 0) { return "kos"; }  // Kosraean
   if (strcmp(ptr, "kpe") == 0) { return "kpe"; }  // Kpelle
   if (strcmp(ptr, "krc") == 0) { return "krc"; }  // Karachay-Balkar
   if (strcmp(ptr, "krl") == 0) { return "krl"; }  // Karelian
   if (strcmp(ptr, "kro") == 0) { return "kro"; }  // Kru languages
   if (strcmp(ptr, "kru") == 0) { return "kru"; }  // Kurukh
   if (strcmp(ptr, "kua") == 0) { return "kua"; }  // Kuanyama; Kwanyama
   if (strcmp(ptr, "kj" ) == 0) { return "kua"; }  // Kuanyama; Kwanyama
   if (strcmp(ptr, "kum") == 0) { return "kum"; }  // Kumyk
   if (strcmp(ptr, "kur") == 0) { return "kur"; }  // Kurdish
   if (strcmp(ptr, "ku" ) == 0) { return "kur"; }  // Kurdish
   if (strcmp(ptr, "kut") == 0) { return "kut"; }  // Kutenai
   if (strcmp(ptr, "lad") == 0) { return "lad"; }  // Ladino
   if (strcmp(ptr, "lah") == 0) { return "lah"; }  // Lahnda
   if (strcmp(ptr, "lam") == 0) { return "lam"; }  // Lamba
   if (strcmp(ptr, "lao") == 0) { return "lao"; }  // Lao
   if (strcmp(ptr, "lo" ) == 0) { return "lao"; }  // Lao
   if (strcmp(ptr, "lat") == 0) { return "lat"; }  // Latin
   if (strcmp(ptr, "la" ) == 0) { return "lat"; }  // Latin
   if (strcmp(ptr, "lav") == 0) { return "lav"; }  // Latvian
   if (strcmp(ptr, "lv" ) == 0) { return "lav"; }  // Latvian
   if (strcmp(ptr, "lez") == 0) { return "lez"; }  // Lezghian
   if (strcmp(ptr, "lim") == 0) { return "lim"; }  // Limburgan; Limburger; Limburgish
   if (strcmp(ptr, "li" ) == 0) { return "lim"; }  // Limburgan; Limburger; Limburgish
   if (strcmp(ptr, "lin") == 0) { return "lin"; }  // Lingala
   if (strcmp(ptr, "ln" ) == 0) { return "lin"; }  // Lingala
   if (strcmp(ptr, "lit") == 0) { return "lit"; }  // Lithuanian
   if (strcmp(ptr, "lt" ) == 0) { return "lit"; }  // Lithuanian
   if (strcmp(ptr, "lol") == 0) { return "lol"; }  // Mongo
   if (strcmp(ptr, "loz") == 0) { return "loz"; }  // Lozi
   if (strcmp(ptr, "ltz") == 0) { return "ltz"; }  // Luxembourgish; Letzeburgesch
   if (strcmp(ptr, "lb" ) == 0) { return "ltz"; }  // Luxembourgish; Letzeburgesch
   if (strcmp(ptr, "lua") == 0) { return "lua"; }  // Luba-Lulua
   if (strcmp(ptr, "lub") == 0) { return "lub"; }  // Luba-Katanga
   if (strcmp(ptr, "lu" ) == 0) { return "lub"; }  // Luba-Katanga
   if (strcmp(ptr, "lug") == 0) { return "lug"; }  // Ganda
   if (strcmp(ptr, "lg" ) == 0) { return "lug"; }  // Ganda
   if (strcmp(ptr, "lui") == 0) { return "lui"; }  // Luiseno
   if (strcmp(ptr, "lun") == 0) { return "lun"; }  // Lunda
   if (strcmp(ptr, "luo") == 0) { return "luo"; }  // Luo (Kenya and Tanzania)
   if (strcmp(ptr, "lus") == 0) { return "lus"; }  // Lushai
   if (strcmp(ptr, "mac") == 0) { return "mac"; }  // Macedonian
   if (strcmp(ptr, "mkd") == 0) { return "mac"; }  // Macedonian
   if (strcmp(ptr, "mk" ) == 0) { return "mac"; }  // Macedonian
   if (strcmp(ptr, "mad") == 0) { return "mad"; }  // Madurese
   if (strcmp(ptr, "mag") == 0) { return "mag"; }  // Magahi
   if (strcmp(ptr, "mah") == 0) { return "mah"; }  // Marshallese
   if (strcmp(ptr, "mh" ) == 0) { return "mah"; }  // Marshallese
   if (strcmp(ptr, "mai") == 0) { return "mai"; }  // Maithili
   if (strcmp(ptr, "mak") == 0) { return "mak"; }  // Makasar
   if (strcmp(ptr, "mal") == 0) { return "mal"; }  // Malayalam
   if (strcmp(ptr, "ml" ) == 0) { return "mal"; }  // Malayalam
   if (strcmp(ptr, "man") == 0) { return "man"; }  // Mandingo
   if (strcmp(ptr, "mao") == 0) { return "mao"; }  // Maori
   if (strcmp(ptr, "mri") == 0) { return "mao"; }  // Maori
   if (strcmp(ptr, "mi" ) == 0) { return "mao"; }  // Maori
   if (strcmp(ptr, "map") == 0) { return "map"; }  // Austronesian languages
   if (strcmp(ptr, "mar") == 0) { return "mar"; }  // Marathi
   if (strcmp(ptr, "mr" ) == 0) { return "mar"; }  // Marathi
   if (strcmp(ptr, "mas") == 0) { return "mas"; }  // Masai
   if (strcmp(ptr, "may") == 0) { return "may"; }  // Malay
   if (strcmp(ptr, "msa") == 0) { return "may"; }  // Malay
   if (strcmp(ptr, "ms" ) == 0) { return "may"; }  // Malay
   if (strcmp(ptr, "mdf") == 0) { return "mdf"; }  // Moksha
   if (strcmp(ptr, "mdr") == 0) { return "mdr"; }  // Mandar
   if (strcmp(ptr, "men") == 0) { return "men"; }  // Mende
   if (strcmp(ptr, "mga") == 0) { return "mga"; }  // Irish, Middle (900-1200)
   if (strcmp(ptr, "mic") == 0) { return "mic"; }  // Mi'kmaq; Micmac
   if (strcmp(ptr, "min") == 0) { return "min"; }  // Minangkabau
   if (strcmp(ptr, "mis") == 0) { return "mis"; }  // Uncoded languages
   if (strcmp(ptr, "mkh") == 0) { return "mkh"; }  // Mon-Khmer languages
   if (strcmp(ptr, "mlg") == 0) { return "mlg"; }  // Malagasy
   if (strcmp(ptr, "mg" ) == 0) { return "mlg"; }  // Malagasy
   if (strcmp(ptr, "mlt") == 0) { return "mlt"; }  // Maltese
   if (strcmp(ptr, "mt" ) == 0) { return "mlt"; }  // Maltese
   if (strcmp(ptr, "mnc") == 0) { return "mnc"; }  // Manchu
   if (strcmp(ptr, "mni") == 0) { return "mni"; }  // Manipuri
   if (strcmp(ptr, "mno") == 0) { return "mno"; }  // Manobo languages
   if (strcmp(ptr, "moh") == 0) { return "moh"; }  // Mohawk
   if (strcmp(ptr, "mon") == 0) { return "mon"; }  // Mongolian
   if (strcmp(ptr, "mn" ) == 0) { return "mon"; }  // Mongolian
   if (strcmp(ptr, "mos") == 0) { return "mos"; }  // Mossi
   if (strcmp(ptr, "mul") == 0) { return "mul"; }  // Multiple languages
   if (strcmp(ptr, "mun") == 0) { return "mun"; }  // Munda languages
   if (strcmp(ptr, "mus") == 0) { return "mus"; }  // Creek
   if (strcmp(ptr, "mwl") == 0) { return "mwl"; }  // Mirandese
   if (strcmp(ptr, "mwr") == 0) { return "mwr"; }  // Marwari
   if (strcmp(ptr, "myn") == 0) { return "myn"; }  // Mayan languages
   if (strcmp(ptr, "myv") == 0) { return "myv"; }  // Erzya
   if (strcmp(ptr, "nah") == 0) { return "nah"; }  // Nahuatl languages
   if (strcmp(ptr, "nai") == 0) { return "nai"; }  // North American Indian languages
   if (strcmp(ptr, "nap") == 0) { return "nap"; }  // Neapolitan
   if (strcmp(ptr, "nau") == 0) { return "nau"; }  // Nauru
   if (strcmp(ptr, "na" ) == 0) { return "nau"; }  // Nauru
   if (strcmp(ptr, "nav") == 0) { return "nav"; }  // Navajo; Navaho
   if (strcmp(ptr, "nv" ) == 0) { return "nav"; }  // Navajo; Navaho
   if (strcmp(ptr, "nbl") == 0) { return "nbl"; }  // Ndebele, South; South Ndebele
   if (strcmp(ptr, "nr" ) == 0) { return "nbl"; }  // Ndebele, South; South Ndebele
   if (strcmp(ptr, "nde") == 0) { return "nde"; }  // Ndebele, North; North Ndebele
   if (strcmp(ptr, "nd" ) == 0) { return "nde"; }  // Ndebele, North; North Ndebele
   if (strcmp(ptr, "ndo") == 0) { return "ndo"; }  // Ndonga
   if (strcmp(ptr, "ng" ) == 0) { return "ndo"; }  // Ndonga
   if (strcmp(ptr, "nds") == 0) { return "nds"; }  // Low German; Low Saxon; German, Low; Saxon, Low
   if (strcmp(ptr, "nep") == 0) { return "nep"; }  // Nepali
   if (strcmp(ptr, "ne" ) == 0) { return "nep"; }  // Nepali
   if (strcmp(ptr, "new") == 0) { return "new"; }  // Nepal Bhasa; Newari
   if (strcmp(ptr, "nia") == 0) { return "nia"; }  // Nias
   if (strcmp(ptr, "nic") == 0) { return "nic"; }  // Niger-Kordofanian languages
   if (strcmp(ptr, "niu") == 0) { return "niu"; }  // Niuean
   if (strcmp(ptr, "nno") == 0) { return "nno"; }  // Norwegian Nynorsk; Nynorsk, Norwegian
   if (strcmp(ptr, "nn" ) == 0) { return "nno"; }  // Norwegian Nynorsk; Nynorsk, Norwegian
   if (strcmp(ptr, "nob") == 0) { return "nob"; }  // Bokml, Norwegian; Norwegian Bokml
   if (strcmp(ptr, "nb" ) == 0) { return "nob"; }  // Bokml, Norwegian; Norwegian Bokml
   if (strcmp(ptr, "nog") == 0) { return "nog"; }  // Nogai
   if (strcmp(ptr, "non") == 0) { return "non"; }  // Norse, Old
   if (strcmp(ptr, "nor") == 0) { return "nor"; }  // Norwegian
   if (strcmp(ptr, "no" ) == 0) { return "nor"; }  // Norwegian
   if (strcmp(ptr, "nqo") == 0) { return "nqo"; }  // N'Ko
   if (strcmp(ptr, "nso") == 0) { return "nso"; }  // Pedi; Sepedi; Northern Sotho
   if (strcmp(ptr, "nub") == 0) { return "nub"; }  // Nubian languages
   if (strcmp(ptr, "nwc") == 0) { return "nwc"; }  // Classical Newari; Old Newari; Classical Nepal Bhasa
   if (strcmp(ptr, "nya") == 0) { return "nya"; }  // Chichewa; Chewa; Nyanja
   if (strcmp(ptr, "ny" ) == 0) { return "nya"; }  // Chichewa; Chewa; Nyanja
   if (strcmp(ptr, "nym") == 0) { return "nym"; }  // Nyamwezi
   if (strcmp(ptr, "nyn") == 0) { return "nyn"; }  // Nyankole
   if (strcmp(ptr, "nyo") == 0) { return "nyo"; }  // Nyoro
   if (strcmp(ptr, "nzi") == 0) { return "nzi"; }  // Nzima
   if (strcmp(ptr, "oci") == 0) { return "oci"; }  // Occitan (post 1500)
   if (strcmp(ptr, "oc" ) == 0) { return "oci"; }  // Occitan (post 1500)
   if (strcmp(ptr, "oji") == 0) { return "oji"; }  // Ojibwa
   if (strcmp(ptr, "oj" ) == 0) { return "oji"; }  // Ojibwa
   if (strcmp(ptr, "ori") == 0) { return "ori"; }  // Oriya
   if (strcmp(ptr, "or" ) == 0) { return "ori"; }  // Oriya
   if (strcmp(ptr, "orm") == 0) { return "orm"; }  // Oromo
   if (strcmp(ptr, "om" ) == 0) { return "orm"; }  // Oromo
   if (strcmp(ptr, "osa") == 0) { return "osa"; }  // Osage
   if (strcmp(ptr, "oss") == 0) { return "oss"; }  // Ossetian; Ossetic
   if (strcmp(ptr, "os" ) == 0) { return "oss"; }  // Ossetian; Ossetic
   if (strcmp(ptr, "ota") == 0) { return "ota"; }  // Turkish, Ottoman (1500-1928)
   if (strcmp(ptr, "oto") == 0) { return "oto"; }  // Otomian languages
   if (strcmp(ptr, "paa") == 0) { return "paa"; }  // Papuan languages
   if (strcmp(ptr, "pag") == 0) { return "pag"; }  // Pangasinan
   if (strcmp(ptr, "pal") == 0) { return "pal"; }  // Pahlavi
   if (strcmp(ptr, "pam") == 0) { return "pam"; }  // Pampanga; Kapampangan
   if (strcmp(ptr, "pan") == 0) { return "pan"; }  // Panjabi; Punjabi
   if (strcmp(ptr, "pa" ) == 0) { return "pan"; }  // Panjabi; Punjabi
   if (strcmp(ptr, "pap") == 0) { return "pap"; }  // Papiamento
   if (strcmp(ptr, "pau") == 0) { return "pau"; }  // Palauan
   if (strcmp(ptr, "peo") == 0) { return "peo"; }  // Persian, Old (ca.600-400 B.C.)
   if (strcmp(ptr, "per") == 0) { return "per"; }  // Persian
   if (strcmp(ptr, "fas") == 0) { return "per"; }  // Persian
   if (strcmp(ptr, "fa" ) == 0) { return "per"; }  // Persian
   if (strcmp(ptr, "phi") == 0) { return "phi"; }  // Philippine languages)
   if (strcmp(ptr, "phn") == 0) { return "phn"; }  // Phoenician
   if (strcmp(ptr, "pli") == 0) { return "pli"; }  // Pali
   if (strcmp(ptr, "pi" ) == 0) { return "pli"; }  // Pali
   if (strcmp(ptr, "pol") == 0) { return "pol"; }  // Polish
   if (strcmp(ptr, "pl" ) == 0) { return "pol"; }  // Polish
   if (strcmp(ptr, "pon") == 0) { return "pon"; }  // Pohnpeian
   if (strcmp(ptr, "por") == 0) { return "por"; }  // Portuguese
   if (strcmp(ptr, "pt" ) == 0) { return "por"; }  // Portuguese
   if (strcmp(ptr, "pra") == 0) { return "pra"; }  // Prakrit languages
   if (strcmp(ptr, "pro") == 0) { return "pro"; }  // Provenal, Old (to 1500);Occitan, Old (to 1500)
   if (strcmp(ptr, "pus") == 0) { return "pus"; }  // Pushto; Pashto
   if (strcmp(ptr, "ps" ) == 0) { return "pus"; }  // Pushto; Pashto
   if (strcmp(ptr, "qaa-qtz") == 0) { return "qaa-qtz"; }  // Reserved for local use
   if (strcmp(ptr, "que") == 0) { return "que"; }  // Quechua
   if (strcmp(ptr, "qu" ) == 0) { return "que"; }  // Quechua
   if (strcmp(ptr, "raj") == 0) { return "raj"; }  // Rajasthani
   if (strcmp(ptr, "rap") == 0) { return "rap"; }  // Rapanui
   if (strcmp(ptr, "rar") == 0) { return "rar"; }  // Rarotongan; Cook Islands Maori
   if (strcmp(ptr, "roa") == 0) { return "roa"; }  // Romance languages
   if (strcmp(ptr, "roh") == 0) { return "roh"; }  // Romansh
   if (strcmp(ptr, "rm" ) == 0) { return "roh"; }  // Romansh
   if (strcmp(ptr, "rom") == 0) { return "rom"; }  // Romany
   if (strcmp(ptr, "rum") == 0) { return "rum"; }  // Romanian; Moldavian; Moldovan
   if (strcmp(ptr, "ron") == 0) { return "rum"; }  // Romanian; Moldavian; Moldovan
   if (strcmp(ptr, "ro" ) == 0) { return "rum"; }  // Romanian; Moldavian; Moldovan
   if (strcmp(ptr, "run") == 0) { return "run"; }  // Rundi
   if (strcmp(ptr, "rn" ) == 0) { return "run"; }  // Rundi
   if (strcmp(ptr, "rup") == 0) { return "rup"; }  // Aromanian; Arumanian; Macedo-Romanian
   if (strcmp(ptr, "rus") == 0) { return "rus"; }  // Russian
   if (strcmp(ptr, "ru" ) == 0) { return "rus"; }  // Russian
   if (strcmp(ptr, "sad") == 0) { return "sad"; }  // Sandawe
   if (strcmp(ptr, "sag") == 0) { return "sag"; }  // Sango
   if (strcmp(ptr, "sg" ) == 0) { return "sag"; }  // Sango
   if (strcmp(ptr, "sah") == 0) { return "sah"; }  // Yakut
   if (strcmp(ptr, "sai") == 0) { return "sai"; }  // South American Indian languages
   if (strcmp(ptr, "sal") == 0) { return "sal"; }  // Salishan languages
   if (strcmp(ptr, "sam") == 0) { return "sam"; }  // Samaritan Aramaic
   if (strcmp(ptr, "san") == 0) { return "san"; }  // Sanskrit
   if (strcmp(ptr, "sa" ) == 0) { return "san"; }  // Sanskrit
   if (strcmp(ptr, "sas") == 0) { return "sas"; }  // Sasak
   if (strcmp(ptr, "sat") == 0) { return "sat"; }  // Santali
   if (strcmp(ptr, "scn") == 0) { return "scn"; }  // Sicilian
   if (strcmp(ptr, "sco") == 0) { return "sco"; }  // Scots
   if (strcmp(ptr, "sel") == 0) { return "sel"; }  // Selkup
   if (strcmp(ptr, "sem") == 0) { return "sem"; }  // Semitic languages
   if (strcmp(ptr, "sga") == 0) { return "sga"; }  // Irish, Old (to 900)
   if (strcmp(ptr, "sgn") == 0) { return "sgn"; }  // Sign Languages
   if (strcmp(ptr, "shn") == 0) { return "shn"; }  // Shan
   if (strcmp(ptr, "sid") == 0) { return "sid"; }  // Sidamo
   if (strcmp(ptr, "sin") == 0) { return "sin"; }  // Sinhala; Sinhalese
   if (strcmp(ptr, "si" ) == 0) { return "sin"; }  // Sinhala; Sinhalese
   if (strcmp(ptr, "sio") == 0) { return "sio"; }  // Siouan languages
   if (strcmp(ptr, "sit") == 0) { return "sit"; }  // Sino-Tibetan languages
   if (strcmp(ptr, "sla") == 0) { return "sla"; }  // Slavic languages
   if (strcmp(ptr, "slo") == 0) { return "slo"; }  // Slovak
   if (strcmp(ptr, "slk") == 0) { return "slo"; }  // Slovak
   if (strcmp(ptr, "sk" ) == 0) { return "slo"; }  // Slovak
   if (strcmp(ptr, "slv") == 0) { return "slv"; }  // Slovenian
   if (strcmp(ptr, "sl" ) == 0) { return "slv"; }  // Slovenian
   if (strcmp(ptr, "sma") == 0) { return "sma"; }  // Southern Sami
   if (strcmp(ptr, "sme") == 0) { return "sme"; }  // Northern Sami
   if (strcmp(ptr, "se" ) == 0) { return "sme"; }  // Northern Sami
   if (strcmp(ptr, "smi") == 0) { return "smi"; }  // Sami languages
   if (strcmp(ptr, "smj") == 0) { return "smj"; }  // Lule Sami
   if (strcmp(ptr, "smn") == 0) { return "smn"; }  // Inari Sami
   if (strcmp(ptr, "smo") == 0) { return "smo"; }  // Samoan
   if (strcmp(ptr, "sm" ) == 0) { return "smo"; }  // Samoan
   if (strcmp(ptr, "sms") == 0) { return "sms"; }  // Skolt Sami
   if (strcmp(ptr, "sna") == 0) { return "sna"; }  // Shona
   if (strcmp(ptr, "sn" ) == 0) { return "sna"; }  // Shona
   if (strcmp(ptr, "snd") == 0) { return "snd"; }  // Sindhi
   if (strcmp(ptr, "sd" ) == 0) { return "snd"; }  // Sindhi
   if (strcmp(ptr, "snk") == 0) { return "snk"; }  // Soninke
   if (strcmp(ptr, "sog") == 0) { return "sog"; }  // Sogdian
   if (strcmp(ptr, "som") == 0) { return "som"; }  // Somali
   if (strcmp(ptr, "so" ) == 0) { return "som"; }  // Somali
   if (strcmp(ptr, "son") == 0) { return "son"; }  // Songhai languages
   if (strcmp(ptr, "sot") == 0) { return "sot"; }  // Sotho, Southern
   if (strcmp(ptr, "st" ) == 0) { return "sot"; }  // Sotho, Southern
   if (strcmp(ptr, "spa") == 0) { return "spa"; }  // Spanish; Castilian
   if (strcmp(ptr, "es" ) == 0) { return "spa"; }  // Spanish; Castilian
   if (strcmp(ptr, "srd") == 0) { return "srd"; }  // Sardinian
   if (strcmp(ptr, "sc" ) == 0) { return "srd"; }  // Sardinian
   if (strcmp(ptr, "srn") == 0) { return "srn"; }  // Sranan Tongo
   if (strcmp(ptr, "srp") == 0) { return "srp"; }  // Serbian
   if (strcmp(ptr, "sr" ) == 0) { return "srp"; }  // Serbian
   if (strcmp(ptr, "srr") == 0) { return "srr"; }  // Serer
   if (strcmp(ptr, "ssa") == 0) { return "ssa"; }  // Nilo-Saharan languages
   if (strcmp(ptr, "ssw") == 0) { return "ssw"; }  // Swati
   if (strcmp(ptr, "ss" ) == 0) { return "ssw"; }  // Swati
   if (strcmp(ptr, "suk") == 0) { return "suk"; }  // Sukuma
   if (strcmp(ptr, "sun") == 0) { return "sun"; }  // Sundanese
   if (strcmp(ptr, "su" ) == 0) { return "sun"; }  // Sundanese
   if (strcmp(ptr, "sus") == 0) { return "sus"; }  // Susu
   if (strcmp(ptr, "sux") == 0) { return "sux"; }  // Sumerian
   if (strcmp(ptr, "swa") == 0) { return "swa"; }  // Swahili
   if (strcmp(ptr, "sw" ) == 0) { return "swa"; }  // Swahili
   if (strcmp(ptr, "swe") == 0) { return "swe"; }  // Swedish
   if (strcmp(ptr, "sv" ) == 0) { return "swe"; }  // Swedish
   if (strcmp(ptr, "syc") == 0) { return "syc"; }  // Classical Syriac
   if (strcmp(ptr, "syr") == 0) { return "syr"; }  // Syriac
   if (strcmp(ptr, "tah") == 0) { return "tah"; }  // Tahitian
   if (strcmp(ptr, "ty" ) == 0) { return "tah"; }  // Tahitian
   if (strcmp(ptr, "tai") == 0) { return "tai"; }  // Tai languages
   if (strcmp(ptr, "tam") == 0) { return "tam"; }  // Tamil
   if (strcmp(ptr, "ta" ) == 0) { return "tam"; }  // Tamil
   if (strcmp(ptr, "tat") == 0) { return "tat"; }  // Tatar
   if (strcmp(ptr, "tt" ) == 0) { return "tat"; }  // Tatar
   if (strcmp(ptr, "tel") == 0) { return "tel"; }  // Telugu
   if (strcmp(ptr, "te" ) == 0) { return "tel"; }  // Telugu
   if (strcmp(ptr, "tem") == 0) { return "tem"; }  // Timne
   if (strcmp(ptr, "ter") == 0) { return "ter"; }  // Tereno
   if (strcmp(ptr, "tet") == 0) { return "tet"; }  // Tetum
   if (strcmp(ptr, "tgk") == 0) { return "tgk"; }  // Tajik
   if (strcmp(ptr, "tg" ) == 0) { return "tgk"; }  // Tajik
   if (strcmp(ptr, "tgl") == 0) { return "tgl"; }  // Tagalog
   if (strcmp(ptr, "tl" ) == 0) { return "tgl"; }  // Tagalog
   if (strcmp(ptr, "tha") == 0) { return "tha"; }  // Thai
   if (strcmp(ptr, "th" ) == 0) { return "tha"; }  // Thai
   if (strcmp(ptr, "tib") == 0) { return "tib"; }  // Tibetan
   if (strcmp(ptr, "bod") == 0) { return "tib"; }  // Tibetan
   if (strcmp(ptr, "bo" ) == 0) { return "tib"; }  // Tibetan
   if (strcmp(ptr, "tig") == 0) { return "tig"; }  // Tigre
   if (strcmp(ptr, "tir") == 0) { return "tir"; }  // Tigrinya
   if (strcmp(ptr, "ti" ) == 0) { return "tir"; }  // Tigrinya
   if (strcmp(ptr, "tiv") == 0) { return "tiv"; }  // Tiv
   if (strcmp(ptr, "tkl") == 0) { return "tkl"; }  // Tokelau
   if (strcmp(ptr, "tlh") == 0) { return "tlh"; }  // Klingon; tlhIngan-Hol
   if (strcmp(ptr, "tli") == 0) { return "tli"; }  // Tlingit
   if (strcmp(ptr, "tmh") == 0) { return "tmh"; }  // Tamashek
   if (strcmp(ptr, "tog") == 0) { return "tog"; }  // Tonga (Nyasa)
   if (strcmp(ptr, "ton") == 0) { return "ton"; }  // Tonga (Tonga Islands)
   if (strcmp(ptr, "to" ) == 0) { return "ton"; }  // Tonga (Tonga Islands)
   if (strcmp(ptr, "tpi") == 0) { return "tpi"; }  // Tok Pisin
   if (strcmp(ptr, "tsi") == 0) { return "tsi"; }  // Tsimshian
   if (strcmp(ptr, "tsn") == 0) { return "tsn"; }  // Tswana
   if (strcmp(ptr, "tn" ) == 0) { return "tsn"; }  // Tswana
   if (strcmp(ptr, "tso") == 0) { return "tso"; }  // Tsonga
   if (strcmp(ptr, "ts" ) == 0) { return "tso"; }  // Tsonga
   if (strcmp(ptr, "tuk") == 0) { return "tuk"; }  // Turkmen
   if (strcmp(ptr, "tk" ) == 0) { return "tuk"; }  // Turkmen
   if (strcmp(ptr, "tum") == 0) { return "tum"; }  // Tumbuka
   if (strcmp(ptr, "tup") == 0) { return "tup"; }  // Tupi languages
   if (strcmp(ptr, "tur") == 0) { return "tur"; }  // Turkish
   if (strcmp(ptr, "tr" ) == 0) { return "tur"; }  // Turkish
   if (strcmp(ptr, "tut") == 0) { return "tut"; }  // Altaic languages
   if (strcmp(ptr, "tvl") == 0) { return "tvl"; }  // Tuvalu
   if (strcmp(ptr, "twi") == 0) { return "twi"; }  // Twi
   if (strcmp(ptr, "tw" ) == 0) { return "twi"; }  // Twi
   if (strcmp(ptr, "tyv") == 0) { return "tyv"; }  // Tuvinian
   if (strcmp(ptr, "udm") == 0) { return "udm"; }  // Udmurt
   if (strcmp(ptr, "uga") == 0) { return "uga"; }  // Ugaritic
   if (strcmp(ptr, "uig") == 0) { return "uig"; }  // Uighur; Uyghur
   if (strcmp(ptr, "ug" ) == 0) { return "uig"; }  // Uighur; Uyghur
   if (strcmp(ptr, "ukr") == 0) { return "ukr"; }  // Ukrainian
   if (strcmp(ptr, "uk" ) == 0) { return "ukr"; }  // Ukrainian
   if (strcmp(ptr, "umb") == 0) { return "umb"; }  // Umbundu
   if (strcmp(ptr, "und") == 0) { return "und"; }  // Undetermined
   if (strcmp(ptr, "urd") == 0) { return "urd"; }  // Urdu
   if (strcmp(ptr, "ur" ) == 0) { return "urd"; }  // Urdu
   if (strcmp(ptr, "uzb") == 0) { return "uzb"; }  // Uzbek
   if (strcmp(ptr, "uz" ) == 0) { return "uzb"; }  // Uzbek
   if (strcmp(ptr, "vai") == 0) { return "vai"; }  // Vai
   if (strcmp(ptr, "ven") == 0) { return "ven"; }  // Venda
   if (strcmp(ptr, "ve" ) == 0) { return "ven"; }  // Venda
   if (strcmp(ptr, "vie") == 0) { return "vie"; }  // Vietnamese
   if (strcmp(ptr, "vi" ) == 0) { return "vie"; }  // Vietnamese
   if (strcmp(ptr, "vol") == 0) { return "vol"; }  // Volapk
   if (strcmp(ptr, "vo" ) == 0) { return "vol"; }  // Volapk
   if (strcmp(ptr, "vot") == 0) { return "vot"; }  // Votic
   if (strcmp(ptr, "wak") == 0) { return "wak"; }  // Wakashan languages
   if (strcmp(ptr, "wal") == 0) { return "wal"; }  // Wolaitta; Wolaytta
   if (strcmp(ptr, "war") == 0) { return "war"; }  // Waray
   if (strcmp(ptr, "was") == 0) { return "was"; }  // Washo
   if (strcmp(ptr, "wel") == 0) { return "wel"; }  // Welsh
   if (strcmp(ptr, "cym") == 0) { return "wel"; }  // Welsh
   if (strcmp(ptr, "cy" ) == 0) { return "wel"; }  // Welsh
   if (strcmp(ptr, "wen") == 0) { return "wen"; }  // Sorbian languages
   if (strcmp(ptr, "wln") == 0) { return "wln"; }  // Walloon
   if (strcmp(ptr, "wa" ) == 0) { return "wln"; }  // Walloon
   if (strcmp(ptr, "wol") == 0) { return "wol"; }  // Wolof
   if (strcmp(ptr, "wo" ) == 0) { return "wol"; }  // Wolof
   if (strcmp(ptr, "xal") == 0) { return "xal"; }  // Kalmyk; Oirat
   if (strcmp(ptr, "xho") == 0) { return "xho"; }  // Xhosa
   if (strcmp(ptr, "xh" ) == 0) { return "xho"; }  // Xhosa
   if (strcmp(ptr, "yao") == 0) { return "yao"; }  // Yao
   if (strcmp(ptr, "yap") == 0) { return "yap"; }  // Yapese
   if (strcmp(ptr, "yid") == 0) { return "yid"; }  // Yiddish
   if (strcmp(ptr, "yi" ) == 0) { return "yid"; }  // Yiddish
   if (strcmp(ptr, "yor") == 0) { return "yor"; }  // Yoruba
   if (strcmp(ptr, "yo" ) == 0) { return "yor"; }  // Yoruba
   if (strcmp(ptr, "ypk") == 0) { return "ypk"; }  // Yupik languages
   if (strcmp(ptr, "zap") == 0) { return "zap"; }  // Zapotec
   if (strcmp(ptr, "zbl") == 0) { return "zbl"; }  // Blissymbols; Blissymbolics; Bliss
   if (strcmp(ptr, "zen") == 0) { return "zen"; }  // Zenaga
   if (strcmp(ptr, "zha") == 0) { return "zha"; }  // Zhuang; Chuang
   if (strcmp(ptr, "za" ) == 0) { return "zha"; }  // Zhuang; Chuang
   if (strcmp(ptr, "znd") == 0) { return "znd"; }  // Zande languages
   if (strcmp(ptr, "zul") == 0) { return "zul"; }  // Zulu
   if (strcmp(ptr, "zu" ) == 0) { return "zul"; }  // Zulu
   if (strcmp(ptr, "zun") == 0) { return "zun"; }  // Zuni
   if (strcmp(ptr, "zxx") == 0) { return "zxx"; }  // No linguistic content; Not applicable
   if (strcmp(ptr, "zza") == 0) { return "zza"; }  // Zaza; Dimili; Dimli; Kirdki; Kirmanjki; Zazaki
   
   // don know what the language code means, so return empty string:
   return "";

}


///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
// private functions
//


//////////////////////////////
//
// HumdrumRecord::makeRecordString --
//

void HumdrumRecord::makeRecordString(void) {
   SSTREAM temp;
   char tab = '\t';
   for (int i=0; i<recordFields.getSize()-1; i++) {
      temp << recordFields[i] << tab;
   }
   temp << recordFields[recordFields.getSize()-1] << ends;

   if (recordString != NULL) {
      delete [] recordString;
      recordString = NULL;
   }
   recordString = new char[strlen(temp.CSTRING) + 1];
   strcpy(recordString, temp.CSTRING);
   modifiedQ = 0;
}



///////////////////////////////////////////////////////////////////////////
//
// generic functions
//

/////////////////////////////
//
// HumdrumRecord::determineType --
//

int HumdrumRecord::determineType(const char* aRecord) const {
   int index = 0;
   
   if (aRecord[index] == '\0') {
      return E_humrec_empty;
   }

   // see if a data interpretation or tandem interpretation
   if (aRecord[index] == '*') {
         return E_humrec_data_interpretation;
   }

   // check if a kern measure
   if (aRecord[0] == '=') {
      return E_humrec_data_kern_measure;
   }

   // if first character is not a '!', then must be data
   if (aRecord[index] != '!') {
      return E_humrec_data;
   } 

   // if only one '!', then this is a local comment in the data
   if (aRecord[index+1] != '!') {
      return E_humrec_data_comment;
   }
      
   // if two '!', then a global comment
   if (aRecord[index+2] != '!') {
      return E_humrec_global_comment;
   } 

   // if three or more '!', then is a bibliographic record
   return E_humrec_bibliography;
}



//////////////////////////////
//
// determinefieldCount --
//

int HumdrumRecord::determineFieldCount(const char* aLine) const {
   int count = 1;
   if (aLine[0] == '\0') {
      return 0;
   }
   if ((aLine[0] == '!') && (aLine[1] == '!')) {
      return 1;
   }

   int i=0;
   while (aLine[i] != '\0') {
      if (aLine[i] == '\t') {
         count++;
         if (aLine[i+1] == '\0') {
            cout << "Error: trailing tab character in line: " << aLine << endl;
            exit(1);
         }
      }
      i++;
   }

   return count;
}



//////////////////////////////
//
// HumdrumRecord::setSize -- change the number of fields in the record.
//    Currently this function is destructive of any previous data, but
//    will probably be changed to preserve any old data.
//

void HumdrumRecord::setSize(int asize) {

   recordFields.allowGrowth(1);
   recordFields.setSize(asize*4);   
   recordFields.setGrowth(132);   
   recordFields.setSize(asize);   

   spineids.allowGrowth(1);
   spineids.setSize(asize*4);
   spineids.setGrowth(132);
   spineids.setSize(asize);

   interpretation.allowGrowth(1);
   interpretation.setSize(asize*4);
   interpretation.setGrowth(132);
   interpretation.setSize(asize);

   dotline.setSize(asize*4);
   dotline.setGrowth(132);
   dotline.setSize(asize);

   dotspine.setSize(asize*4);
   dotspine.setGrowth(132);
   dotspine.setSize(asize);


   char buffer[32] = {0};
   int i;
   int len;
   for (i=0; i<recordFields.getSize(); i++) {
      recordFields[i] = new char[2];
      strcpy(recordFields[i], ".");
      sprintf(buffer, "%d", i+1);
      len = strlen(buffer);
      spineids[i] = new char[len+1];
      strcpy(spineids[i], buffer);
   }
  
   dotline.setAll(-1);
   dotspine.setAll(-1);
   interpretation.setAll(-1); // or 0?
}



//////////////////////////////
//
// HumdrumRecord::setAllFields -- Set all fields to the same string content.
//

void HumdrumRecord::setAllFields(const char* astring) {
   int i;
   for (i=0; i<getFieldCount(); i++) {
      changeField(i, astring);
   }
}



//////////////////////////////
//
// operator<< --
//

ostream& operator<<(ostream& out, HumdrumRecord& aRecord) {
   out << aRecord.getLine();
   return out;
}



// md5sum: b0731dc4d30ae0a5ea0fa793dc9a87da HumdrumRecord.cpp [20050403]
