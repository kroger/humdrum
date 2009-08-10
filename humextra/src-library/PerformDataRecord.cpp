//
// Copyright 1999 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul  2 23:05:34 PDT 1999
// Last Modified: Sun Jul  4 13:23:41 PDT 1999
// Filename:      ...sig/src/sigInfo/PerformDataRecord.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/PerformDataRecord.cpp
// Syntax:        C++
//
// Description:   Data for a somewhat defunct class, Perform.
//

#include "PerformDataRecord.h"

#include <stdio.h>
#include <string.h>

#ifndef VISUAL
   #include <sys/types.h>   /* needed by regex.h */
   #include <regex.h>
#endif

#ifndef OLDCPP
   #include <iomanip>
   using namespace std;
#else
   #include <iomanip.h>
#endif



//////////////////////////////
//
// PerformDataRecord::PerformDataRecord --
//

PerformDataRecord::PerformDataRecord(void) { 
   time = -1;
   type = PERFORM_TYPE_NULL;
   data.setSize(1);
   data[0] = '\0';
   data.allowGrowth(0);
}


PerformDataRecord::PerformDataRecord(PerformDataRecord& aRecord) { 
   time = aRecord.time;
   type = aRecord.type;
   data = aRecord.data;
}



//////////////////////////////
//
// PerformDataRecord::~PerformDataRecord --
//

PerformDataRecord::~PerformDataRecord() { 
   // do nothing
}



//////////////////////////////
//
// PerformDataRecord::barQ -- return true if the data is a measure
//    type, otherwise returns false.
//

int PerformDataRecord::barQ(void) {
   return measureQ();
}



//////////////////////////////
//
// PerformDataRecord::beginQ -- return true if the data is a beginning
//    of file character.
//

int PerformDataRecord::beginQ(void) {
   if (getType() == PERFORM_TYPE_BEGIN) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// PerformDataRecord::endQ -- return true if the data is an ending
//    of file type.
//

int PerformDataRecord::endQ(void) {
   if (getType() == PERFORM_TYPE_END) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// PerformDataRecord::getData -- returns a pointer to the data
//     stored in this record.
//

char* PerformDataRecord::getData(void) { 
   return data.getBase();
}



//////////////////////////////
//
// PerformDataRecord::getLength -- returns the size of the data
//    stored in this record. The last element is a zero value
//    which is part of the PerformDataRecord and not part of the
//    data.  Text fields will return one more element in size
//    then strlen() will return, because of the string's terminating
//    zero character being included in the count.
//

int PerformDataRecord::getLength(void) { 
   return data.getSize() - 1;
}



//////////////////////////////
//
// PerformDataRecord::getMeasureNumber -- return the measure number
//    of the data record if it is measure data
//

int PerformDataRecord::getMeasureNumber(void) {
   int measure;
   if (measureQ()) {
      sscanf("BAR%4d", getData(), &measure);      
      return measure;
   } 

   cout << "Error: this data is not a measure marker" << endl;
   exit(1);

   return 0; // for compilers with buggy warning statements
}



//////////////////////////////
//
// PerformDataRecord::getTempoNumber -- return the tempo number
//    of the data record if it is tempo data
//

double PerformDataRecord::getTempoNumber(void) {
   int measure;
   if (measureQ()) {
      sscanf("TEMPO%4d", getData(), &measure);      
      return measure;
   }
   cout << "Error: this data is not a measure marker" << endl;
   exit(1);

   return 0; // for compilers with buggy warning statements
}



//////////////////////////////
//
// PerformDataRecord::getTime -- returns the time that this
//     data is supposed to be performed, if absolute time;
//     or it could be a delta time.
//

int PerformDataRecord::getTime(void) { 
   return time;
}



//////////////////////////////
//
// PerformDataRecord::getType -- returns the type of the data
//     stored in this record.
//

int PerformDataRecord::getType(void) { 
   return type;
}



//////////////////////////////
//
// PerformDataRecord::match -- return true if the match string
//    was found in the data.
//

int PerformDataRecord::match(const char* matchString) {
   #ifdef VISUAL
      int i=0;
      int length = strlen(matchString);
      char* stringToSearch = getData();
      int found = 0;
      while (stringToSearch[i] != '\0' && stringToSearch[i] != matchString[0]) {
         if (strncmp(matchString, &stringToSearch[i], length) == 0) {
            found = 1;
            break;
         } 
         i++;
      }
      return found;  
   #else
      char* stringToSearch = getData();
      regex_t    preg;     // compiled regular expression string structure
      regmatch_t pmatch;   // sub expression matching structure
    
      // first, compile the regular expression to search with
      int status = regcomp(&preg, matchString, REG_EXTENDED | REG_NOSUB);
    
      // Whenever you get a non-zero return code from regcomp()
      // or regexec(), the regerror() function can provide a detailed
      // message explaining what went wrong.
      if (status != 0) {
         cout << "Error: could not compile search string" << endl;
         exit(1);
      }

      status = regexec(&preg, stringToSearch, 1, &pmatch, 0);
    
      regfree(&preg);

      if (status == 0) {
         return 1;          // string was matched
      } else {
         return 0;          // string was not matched
      }
     
   #endif
}



//////////////////////////////
//
// PerformDataRecord::measureQ -- return true if the data is a measure
//    type, otherwise returns false.
//

int PerformDataRecord::measureQ(void) {
   if (getType() == PERFORM_TYPE_MEASURE) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// PerformDataRecord::midiQ -- return true if the data is a midi
//    type, otherwise returns false.
//

int PerformDataRecord::midiQ(void) {
   if (getType() == PERFORM_TYPE_MIDI) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// PerformDataRecord::operator= -- defines how PerformDataRecords are
//   copied.
//

PerformDataRecord& PerformDataRecord::operator=(PerformDataRecord& aRecord) {
   if (&aRecord == this) {
      return *this;
   }
   time = aRecord.time;
   type = aRecord.type;
   data = aRecord.data;

   return *this;
}



//////////////////////////////
//
// PerformDataRecord::print -- print the record information
//     default value: out = cout.
//

ostream& PerformDataRecord::print(ostream& out) {
   out << "time=" << getTime() << "  \ttype=";

   switch (getType()) {
      case PERFORM_TYPE_NULL:
         out << "null";
         break;
      case PERFORM_TYPE_TEXT:
         out << "text: ";
         out << getData();
         break;
      case PERFORM_TYPE_MIDI:
         {
         out << "midi: ";
         int i = 0;
         char* datax = getData();
         int size = data.getSize() - 1;
         while (i < size) {
            if (datax[i] >= 0) {
               out << (unsigned int)((unsigned char)datax[i]);
               out << " ";
            } else {
               out << "0x";
               out << hex << (unsigned int)((unsigned char)datax[i]) << dec;
               out << " ";
            }
            i++;
         }
         }
         break;
      case PERFORM_TYPE_MEASURE:
         out << "bar:  ";
         out << getData();
         break;
      case PERFORM_TYPE_TEMPO:
         out << "tempo ";
         out << getData();
         break;
      case PERFORM_TYPE_CLEAR:
         out << "clear ";
         out << getData();
         break;
      case PERFORM_TYPE_IGNORED:
         out << "ignored ";
         break;
      case PERFORM_TYPE_BEGIN:
         out << "begin ";
         break;
      case PERFORM_TYPE_END:
         out << "end ";
         break;
      default:
         out << "UNKNOWN";
   }

   return out;
}



//////////////////////////////
//
// PerformDataRecord::setBar -- set the data in this record
//   to be of the measure marker type.
//      default value: length = -1.
//

void PerformDataRecord::setBar(int aTime, const char* measureData, 
      int length) { 
   setMeasure(aTime, measureData, length);
}


void PerformDataRecord::setBar(int aTime, int aMeasure) {
   setMeasure(aTime, aMeasure);
}



//////////////////////////////
//
// PerformDataRecord::setClear -- set the data in this record
//   to be of the clear screen marker type.
//

void PerformDataRecord::setClear(int aTime) {
   data.setSize(5 + 2);
   data.setSize(5 + 1);
   strcpy(data.getBase(), "CLEAR");
   data.allowGrowth(1);
   char temp = '\0';
   data.append(temp);
   data.allowGrowth(0);

   setType(PERFORM_TYPE_CLEAR);
   setTime(aTime);
}



//////////////////////////////
//
// PerformDataRecord::setMeasure -- set the data in this record
//   to be of the measure marker type.
//      default value: length = -1.
//

void PerformDataRecord::setMeasure(int aTime, const char* measureData, 
      int length) { 
   if (length == -1) {
      length = strlen(measureData);
   }

   data.setSize(length + 2);
   data.setSize(length + 1);
   strcpy(data.getBase(), measureData);
   data.allowGrowth(1);
   char temp = '\0';
   data.append(temp);
   data.allowGrowth(0);

   setType(PERFORM_TYPE_MEASURE);
   setTime(aTime);
}


void PerformDataRecord::setMeasure(int aTime, int aMeasure) {
   char measureString[128] = {0};
   sprintf(measureString, "BAR%3d", aMeasure);
   setMeasure(aTime, measureString);
}



//////////////////////////////
//
// PerformDataRecord::setTempo -- set the data in this record
//   to be of the tempo marker type.
//      default value: length = -1.
//

void PerformDataRecord::setTempo(int aTime, const char* tempoData, 
      int length) { 
   if (length == -1) {
      length = strlen(tempoData);
   }

   data.setSize(length + 2);
   data.setSize(length + 1);
   strcpy(data.getBase(), tempoData);
   data.allowGrowth(1);
   char temp = '\0';
   data.append(temp);
   data.allowGrowth(0);

   setType(PERFORM_TYPE_TEMPO);
   setTime(aTime);
}


void PerformDataRecord::setTempo(int aTime, int aTempo) {
   char tempoString[128] = {0};
   sprintf(tempoString, "TEMPO%3d", aTempo);
   setTempo(aTime, tempoString);
}



//////////////////////////////
//
// PerformDataRecord::setMidi -- set the data in this record
//    to be MIDI data which will be sent out at a specific time.
//

void PerformDataRecord::setMidi(int aTime, const char* someData, 
      int length) { 
   data.setSize(length + 2);             // extra space for safty
   data.setSize(length);
   strncpy(data.getBase(), someData, length);
   data.allowGrowth(1);
   char element = '\0';
   data.append(element);                  // for string searching
   data.allowGrowth(0);

   setType(PERFORM_TYPE_MIDI);
   setTime(aTime);
}



//////////////////////////////
//
// PerformDataRecord::setText -- set the data in this record
//    to be text data to display at a specific time.
//     default value: length = -1
//

void PerformDataRecord::setText(int aTime, const char* someText,
       int length) { 
   if (length == -1) {
      length = strlen(someText);
   }

   data.setSize(length + 2);
   data.setSize(length + 1);
   strcpy(data.getBase(), someText);
   data.allowGrowth(1);
   char temp = '\0';
   data.append(temp);
   data.allowGrowth(0);

   setType(PERFORM_TYPE_TEXT);
   setTime(aTime);
}



//////////////////////////////
//
// PerformDataRecord::setTime -- set the performance time of the
//   data.
//

void PerformDataRecord::setTime(int aTime) { 
   time = aTime;
}



//////////////////////////////
//
// PerformDataRecord::setType -- sets the type of the performance data.
//

void PerformDataRecord::setType(int aType) { 
   type = aType;
}



//////////////////////////////
//
// PerformDataRecord::tempoQ -- return true if the data is a tempo
//    type, otherwise returns false.
//

int PerformDataRecord::tempoQ(void) {
   if (getType() == PERFORM_TYPE_TEMPO) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// PerformDataRecord::textQ -- return true if the data is a text
//    type, otherwise returns false.
//

int PerformDataRecord::textQ(void) {
   if (getType() == PERFORM_TYPE_TEXT) {
      return 1;
   } else {
      return 0;
   }
}



// md5sum: 88ed433f0e1993e60cf68a9c39474172 PerformDataRecord.cpp [20050403]
