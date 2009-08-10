//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May 18 13:52:59 PDT 1998
// Last Modified: Thu Jul  1 16:20:41 PDT 1999
// Last Modified: Thu Apr 13 18:34:28 PDT 2000 (added generic interpretation)
// Last Modified: Fri May  5 13:12:21 PDT 2000 (added sub-spine access)
// Last Modified: Fri Oct 13 12:45:45 PDT 2000 (added spine tracing vars)
// Last Modified: Wed May 16 18:20:55 PDT 2001 (added changeToken)
// Last Modified: Sun Mar 24 12:10:00 PST 2002 (small changes for visual c++)
// Last Modified: Wed Apr  3 08:13:29 PST 2002 (allow DOS newline input)
// Last Modified: Mon May 17 12:46:36 PDT 2004 (added getSpinePrediction)
// Last Modified: Wed Jun  2 15:50:13 PDT 2004 (setSpineWidth adjusts interp)
// Last Modified: Tue Apr 21 10:02:00 PDT 2009 (fixed comment prob. in setLine)
// Last Modified: Tue Apr 28 14:34:13 PDT 2009 (added isTandem)
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Last Modified: Mon Jun 15 15:55:22 PDT 2009 (fixed bug in getPrimaryTrack)
// Last Modified: Thu Jun 18 15:29:49 PDT 2009 (modified getExInterp behavior)
// Last Modified: Thu Jun 18 15:57:53 PDT 2009 (added hasSpines())
// Filename:      ...sig/src/sigInfo/HumdrumRecord.cpp
// Webpage:       http://sig.sapp.org/src/sigInfo/HumdrumRecord.cpp
// Syntax:        C++ 
//
// Description:   Stores the data for one line in a HumdrumFile and
//                segments data into spines.
//

#include "Convert.h"
#include "HumdrumRecord.h"

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
   meterloc = 0.0;
   absloc   = 0.0;
   spinewidth = 0;

   type = E_unknown;
   recordString = new char[1];
   recordString[0] = '\0';
   modifiedQ = 0;
   lineno = -1;

   recordFields.allowGrowth();
   recordFields.setSize(32);   
   recordFields.setGrowth(32);   
   recordFields.setSize(0);   

   interpretation.allowGrowth();
   interpretation.setSize(32);
   interpretation.setGrowth(32);
   interpretation.setSize(0);

   spineids.allowGrowth();
   spineids.setSize(32);
   spineids.setGrowth(32);
   spineids.setSize(0);

   dotline.setSize(0);
   dotspine.setSize(32);
   dotspine.setGrowth(32);
   dotspine.setSize(0);
}


HumdrumRecord::HumdrumRecord(const char* aLine, int aLineNum) {
   duration = 0.0;
   meterloc = 0.0;
   absloc   = 0.0;
   spinewidth = 0;

   lineno = aLineNum;
   type = determineType(aLine);
   recordString = new char[1];
   recordString[0] = '\0';
   modifiedQ = 0;
   interpretation.allowGrowth();
   interpretation.setSize(0);
   recordFields.allowGrowth();
   recordFields.setSize(0);
   spineids.allowGrowth();
   spineids.setSize(0);
   dotline.setSize(0);
   dotspine.setSize(0);
   setLine(aLine);
}


HumdrumRecord::HumdrumRecord(const HumdrumRecord& aRecord) {
   duration = aRecord.duration;
   meterloc = aRecord.meterloc;
   absloc   = aRecord.absloc;
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
// HumdrumRecord::equalDataQ --
//

int HumdrumRecord::equalDataQ(const char* aValue) {
   int output = 1;
   for (int i=1; i<getFieldCount(); i++) {
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

   buffer[0] = '\0';
   return buffer;
}



//////////////////////////////
//
// HumdrumRecord::getAbsBeat -- returns the absolute beat location
//    in the file.  This value is by default 0, but can be set
//    manally with the setAbsBeat() function, or by calling
//    HumdrumFile::analyzeRhythm().
//

double HumdrumRecord::getAbsBeat(void) const { 
   return absloc;
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
   return meterloc;
}



//////////////////////////////
//
// HumdrumRecord::getDuration -- returns the duration of the current
//    HumdrumRecord line in a HumdrumFile.  This value is by default 0, 
//    but can be set manally with the setAbsBeat() function, or by 
//    calling HumdrumFile::analyzeRhythm().
//

double HumdrumRecord::getDuration(void) const { 
   return duration;
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
   meterloc = aRecord.meterloc;
   absloc   = aRecord.absloc;

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
   meterloc = 0.0;
   absloc   = 0.0;
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
   absloc = (float)aValue;
}



//////////////////////////////
//
// HumdrumRecord::setBeat -- sets the metrical beat location
//    in the file.  This value is by default 0, but can be set
//    manally with the setBeat() function, or by calling
//    HumdrumFile::analyzeRhythm().
//

void HumdrumRecord::setBeat(double aValue) { 
   meterloc = (float)aValue;
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
   duration = (float)aValue;
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


///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// operator<< --
//

ostream& operator<<(ostream& out, HumdrumRecord& aRecord) {
   out << aRecord.getLine();
   return out;
}



// md5sum: b0731dc4d30ae0a5ea0fa793dc9a87da HumdrumRecord.cpp [20050403]
