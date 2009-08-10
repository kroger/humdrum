//
// Copyright 1999-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul  2 23:05:34 PDT 1999
// Last Modified: Thu Jul  8 11:24:15 PDT 1999
// Last Modified: Sun Jun 11 12:44:02 PDT 2000
// Filename:      ...sig/src/sigInfo/PerformData.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/PerformData.cpp
// Syntax:        C++
//
// Description:   Data for a somewhat defunct class, Perform.
//

#include "PerformData.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifndef OLDCPP
   #include <iostream>
   #include <fstream>
   using namespace std;
#else
   #include <iostream.h>
   #include <fstream.h>
#endif



//////////////////////////////
//
// PerformData::PerformData --
//

PerformData::PerformData(void) {
   currentIndex = 0;
  
   records.setSize(10000);
   records.setSize(0);
   records.setGrowth(10000);
   records.allowGrowth(0);

   begin.setType(PERFORM_TYPE_BEGIN);
   begin.setTime(0);
   end.setType(PERFORM_TYPE_END);
   end.setTime(0x7fffffff);
 
   records.setSize(100000);
   records.setSize(0);
   records.allowGrowth(0);
   records.setGrowth(100000);

   timeFormat = PERFORM_TIME_UNKNOWN;  
}



//////////////////////////////
//
// PerformData::~PerformData --
//

PerformData::~PerformData() {
   clear();
}



//////////////////////////////
//
// PerformData::add -- add a record to the data list.
//

int PerformData::add(PerformDataRecord& record) { 
   PerformDataRecord* item = new PerformDataRecord(record);
   records.allowGrowth(1);
   records.append(item);
   records.allowGrowth(0);
   return records.getSize();
}



//////////////////////////////
//
// PerformData::back -- decrement to the previous element in the data.
//

void PerformData::back(void) {
   setIndex(getIndex() - 1);
}



//////////////////////////////
//
// PerformData::bof -- returns true if pointing to the start of the
//    data
//

int PerformData::bof(void) {
   if (getIndex() == 0) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// PerformData::clear -- erase all the data
//

void PerformData::clear(void) {
   for (int i = 0; i < records.getSize(); i++) {
      if (records[i] != NULL) {
         delete records[i];
         records[i] = NULL;
      }
   }
   records.setSize(0);
}



//////////////////////////////
//
// PerformData::determineTimeType -- guess whether the Perform
//     data is in absolute or relative time values, or if the
//     time type is undetermined
//

int PerformData::determineTimeType(void) {
   if (records.getSize() <= 1) {
      return PERFORM_TIME_UNKNOWN;
   }

   int result = PERFORM_TIME_ABSOLUTE;
   int currentTime = records[records.getSize()-1]->getTime();
   int lastTime;
   for (int i=records.getSize()-2; i>0; i--) {
      lastTime = currentTime;
      currentTime = records[i]->getTime();
      if (currentTime > lastTime) {
         result = PERFORM_TIME_RELATIVE;
         break;
      }
   }

   return result;
}



//////////////////////////////
//
// PerformData::eof -- returns true if pointing to the end of the
//    data
//

int PerformData::eof(void) {
   if (getIndex() == records.getSize() + 1) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// PerformData::inputAsciiMidiFile -- read an Ascii Midi file
//     into the data structure.
//

void PerformData::inputAsciiMidiFile(const char* filename) { 
/*
   // open the input file for reading 
   fstream input(filename, ios::in | ios::nocreate);
   if (!input.is_open()) {
      cout << "Error: cannot open file: " << filename << " for reading" << endl;
      exit(1);
   }

   int header = 1;
   char currentLine[8096] = {0};
   int linetype;

   // process the lines of the file one by one.
   while (getNextAsciiMidiLine(currentLine, input) != 0) {
      linetype = getAsciiMidiLineType(currentLine);
      switch (linetype) {
         case LINE_TYPE_BARLINE:
            aRecord.setMeasure(0, getMeasureValue(currentLine));
            add(aRecord);
            break;
         case LINE_TYPE_CLEAR:
            aRecord.setClear(0);
            add(aRecord);
            break;
         case LINE_TYPE_COMMENT:
            if (currentLine[0] == '!' && currentLine[1] == '!') {
               aRecord.setText(0, currentLine + 2);
            } else {
               aRecord.setText(0, currentLine);
            }
            add(aRecord);
            break;
         case LINE_TYPE_INTERPRETATION:
            processInterpLine(currentLine, spine_list, chan_list, path_list);
            break;
         case LINE_TYPE_EXCLUSIVE_INTERPRETATION:
            exclusiveCount++;
            if (exclusiveCount > 1) {
               cout << "Error: only one exclusive interpretation per file." 
                    << endl;
               exit(1);
            }
            processExclusiveInterpLine(currentLine, chan_list);
            break;
         case LINE_TYPE_NULL:
            // ignore blank lines and lines starting with white spaces.
            break;
         case LINE_TYPE_DATA:
            processHumdrumData(currentLine, spine_list, chan_list);
            break;
         default:
            cout << "Error: unknown type of data in humdrum file:\n"
                    << currentLine << endl;
            exit(1);
      }
   } // end of while getNextAsciiMidiLine()
*/

}



//////////////////////////////
//
// PerformData::inputHumdrumMidiFile -- read a Humdrum **MIDI file
//    into the data structure.
//

#define LINE_TYPE_UNKNOWN                 -1
#define LINE_TYPE_BARLINE                  1
#define LINE_TYPE_CLEAR                    2
#define LINE_TYPE_COMMENT                  3
#define LINE_TYPE_INTERPRETATION           4
#define LINE_TYPE_EXCLUSIVE_INTERPRETATION 5
#define LINE_TYPE_NULL                     6
#define LINE_TYPE_DATA                     7

#define HUMDRUM_DEFAULT_TEMPO             80

void PerformData::inputHumdrumMidiFile(const char* filename) { 
   PerformDataRecord aRecord;

   clear();                              // remove any old data
   last_tempo = HUMDRUM_DEFAULT_TEMPO;   // initialize tempo tracker
   PerformDataRecord temp;            
   temp.setTempo(0, (int)last_tempo);
   add(temp);                            // add default tempo to performance

   // open the input file for reading 

   #ifndef OLDCPP
      fstream input(filename, ios::in);
   #else
      fstream input(filename, ios::in | ios::nocreate);
   #endif

   if (!input.is_open()) {
      cout << "Error: cannot open file: " << filename << " for reading" << endl;
      exit(1);
   }

   Array<int> chan_list(1024);   // set to 1 if **MIDI exinterp spine
   Array<int> spine_list(1024);  // MIDI channel for spine[i], from 1 to 16.
   Array<int> path_list(1024);   // set whenever path indicators are encountered

   chan_list.setAll(0);          // no spines are **MIDI yet
   spine_list.setAll(1);         // default channel is 1
   path_list.setAll(0);          // no path indicators yet
   
   int exclusiveCount = 0;
   char currentLine[8096] = {0};
   int linetype;

   // process the lines of the file one by one.
   while (getNextLine(currentLine, input) != 0) {
      linetype = getLineType(currentLine);
      switch (linetype) {
         case LINE_TYPE_BARLINE:
            aRecord.setMeasure(0, getMeasureValue(currentLine));
            add(aRecord);
            break;
         case LINE_TYPE_CLEAR:
            aRecord.setClear(0);
            add(aRecord);
            break;
         case LINE_TYPE_COMMENT:
            if (currentLine[0] == '!' && currentLine[1] == '!') {
               aRecord.setText(0, currentLine + 2);
            } else {
               aRecord.setText(0, currentLine);
            }
            add(aRecord);
            break;
         case LINE_TYPE_INTERPRETATION:
            processInterpLine(currentLine, spine_list, chan_list, path_list);
            break;
         case LINE_TYPE_EXCLUSIVE_INTERPRETATION:
            exclusiveCount++;
            if (exclusiveCount > 1) {
               cout << "Error: only one exclusive interpretation per file." 
                    << endl;
               exit(1);
            }
            processExclusiveInterpLine(currentLine, chan_list);
            break;
         case LINE_TYPE_NULL:
            // ignore blank lines and lines starting with white spaces.
            break;
         case LINE_TYPE_DATA:
            processHumdrumData(currentLine, spine_list, chan_list);
            break;
         default:
            cout << "Error: unknown type of data in humdrum file:\n"
                    << currentLine << endl;
            exit(1);
      }
   } // end of while getNextLine()

}



//////////////////////////////
//
// PerformData::inputMidiFile -- read a standard MIDI file
//     into the data structure.
//

void PerformData::inputMidiFile(const char* filename) { 
   cout << "Not finished yet" << endl;
   exit(1);
}



//////////////////////////////
//
// PerformData::getBar -- return the bar number of the 
//   current record if it is a bar type record.
//

int PerformData::getBar(void) {
   return records[getIndex() - 1]->getMeasureNumber();
}



//////////////////////////////
//
// PerformData::getData -- return a pointer to the current data.
//

char* PerformData::getData(void) { 
   if (getIndex() >= records.getSize() + 1) {
      return end.getData();
   } else if (getIndex() <= 0) {
      return begin.getData();
   } else {
      return records[getIndex()-1]->getData();
   }
}



//////////////////////////////
//
// PerformData::getIndex -- return the index of the current item.
//

int PerformData::getIndex(void) { 
   return currentIndex;
}



//////////////////////////////
//
// PerformData::getLength -- return the size of the current data.
//

int PerformData::getLength(void) { 
   if (getIndex() >= records.getSize() + 1) {
      return end.getLength();
   } else if (getIndex() <= 0) {
      return end.getLength();
   } else {
      return records[getIndex()-1]->getLength();
   }
}



//////////////////////////////
//
// PerformData::getMeasure -- return the measure number of the 
//   current record if it is a bar type record.
//

int PerformData::getMeasure(void) {
   return records[getIndex() - 1]->getMeasureNumber();
}



//////////////////////////////
//
// PerformData::getSize -- returns the number of data elements in
//   the data set which include two extras: the beginning and ending
//   data indicators
//

int PerformData::getSize(void) {
   return records.getSize() + 2;
}



//////////////////////////////
//
// PerformData::getTempo -- return the tempo if item is a tempo
//     marker
//

double PerformData::getTempo(void) {
   return records[getIndex() - 1]->getTempoNumber();
}



//////////////////////////////
//
// PerformData::getTime -- return the time that the current 
//    record is supposed to be performed.
//

int PerformData::getTime(void) {
   if (getIndex() >= records.getSize() + 1) {
      return end.getTime();
   } else if (getIndex() <= 0) {
      return begin.getTime();
   } else {
      return records[getIndex() - 1]->getTime();
   }
}



//////////////////////////////
//
// PerformData::getType -- return the type of data that the current
//     record contains.
//

int PerformData::getType(void) { 
   if (getIndex() >= records.getSize() + 1) {
      return end.getType();
   } else if (getIndex() <= 0) {
      return begin.getType();
   } else {
      return records[getIndex() - 1]->getType();
   }
}



//////////////////////////////
//
// PerformData::markAsAbsoluteTime -- indicate that the data
//    is in absolute time notation.
//

void PerformData::markAsAbsoluteTime(void) { 
   timeFormat = PERFORM_TIME_ABSOLUTE;
}



//////////////////////////////
//
// PerformData::markAsRelativeTime -- indicate that the data
//     is in relative time notation.
//

void PerformData::markAsRelativeTime(void) {
   timeFormat = PERFORM_TIME_RELATIVE;
}



//////////////////////////////
//
// PerformData::match -- try to match the search string with the
//    current data record.
//

int PerformData::match(const char* matchString) {
   if (getType() == PERFORM_TYPE_BEGIN || getType() == PERFORM_TYPE_END) {
      return 1;
   } else {
      return records[getIndex() - 1]->match(matchString);
   }
}



//////////////////////////////
//
// PerformData::next -- increment to the next element in the data.
//

void PerformData::next(void) {
   setIndex(getIndex() + 1);
}



//////////////////////////////
//
// PerformData::operator[] -- return the specified record
//

PerformDataRecord& PerformData::operator[](int index) { 
   if (index >= getSize()-1) {
      return end;
   } else if (index <= 0) {
      return begin;
   } else {
      return *records[index];
   }
}



//////////////////////////////
//
// PerformData::print -- print all of the records
//     default value: out = cout.
//

ostream& PerformData::print(ostream& out) {
   begin.print(out);
   out << '\n';
   for (int i=0; i<records.getSize(); i++) {
      records[i]->print(out); 
      out << '\n';
   };
   end.print(out);
   out << '\n';

   return out;
}



//////////////////////////////
//
// PerformData::ready -- returns true if it is time to play the 
//   current record.
//

int PerformData::ready(int aTime) {
   if (getTime() <= aTime) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// PerformData::setIndex -- set the current index location.
//    if the index is negative, make it zero, if the index
//    is too large, then go to the last element (the end record).
//

void PerformData::setIndex(int index) { 
   if (index < 0) {
      currentIndex = 0;
   } else if (index > getSize()-1) {
      currentIndex = getSize() - 1;
   } else {
      currentIndex = index;
   }
}



//////////////////////////////
//
// PerformData::setTime -- set the time of the current item.
//
//

void PerformData::setTime(int aTime) { 
   records[getIndex()-1]->setTime(aTime);
}



//////////////////////////////
//
// PerformData::setTimeType -- set the time type to relative, absolute,
//   or unknow.
//

void PerformData::setTimeType(int aTimeType) {
   switch (aTimeType) {
      case PERFORM_TIME_ABSOLUTE:
         timeFormat = PERFORM_TIME_ABSOLUTE;
         break;
      case PERFORM_TIME_RELATIVE:
         timeFormat = PERFORM_TIME_RELATIVE;
         break;
      default:
         timeFormat = PERFORM_TIME_UNKNOWN;
   }
}



//////////////////////////////
//
// PerformData::setType --
//

void PerformData::setType(int aType) { 
   records[getIndex()-1]->setType(aType);
}



//////////////////////////////
//
// PerformData::sort -- sort the data by time.  You should only use this
//    option if the data is arranged by absolute time.
//

void PerformData::sort(void) {
   qsort(records.getBase(), records.getSize(), sizeof(PerformDataRecord*),
      performRecordCompare);
}



//////////////////////////////
//
// PerformData::swap -- switch the data records for the
//   given two indices. If timeHeld != 0, then the time 
//   entry of the two items will not be switched so that the
//   data records will therefore be swapping their time values.
//     default value: timeHeld = 0;
//

void PerformData::swap(int index1, int index2, int timeHeld) {
   if (index1 == index2) {
      return;
   }

   PerformDataRecord* temp = records[index2 - 1];
   records[index2 - 1] = records[index1 - 1];
   records[index1 - 1] = temp;

   if (timeHeld) {
      int tempTime = records[index1 - 1]->getTime();
      temp = records[index2 - 1];

      records[index2 - 1]->setTime(records[index1 - 1]->getTime());
      records[index1 - 1]->setTime(tempTime);
   }
}



///////////////////////////////////////////////////////////////////////////
//
// private functions
//


///////////////////////////////////////////////////////////////////////////
//                                                                       //
// private function for use with inputHumdrumMidiFile() only             //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// PerformData::getLineType -- return the format type of the humdrum
//      file line.
//
 
int PerformData::getLineType(const char* line) {
   if (line == NULL || line[0] == '\0'|| isspace(line[0])) {
      return LINE_TYPE_NULL;
   } else if (line[0] == '=') {
      return LINE_TYPE_BARLINE;
   } else if (line[0] == '*') {
      if (line[1] == '*') {
         return LINE_TYPE_EXCLUSIVE_INTERPRETATION;
      } else {
         return LINE_TYPE_INTERPRETATION;
      }
   } else if (line[0] == '!') {
      if (strncmp(line, "!!CLS", 5) == 0) {
         return LINE_TYPE_CLEAR;
      } else {
         return LINE_TYPE_COMMENT;
      }
   } else {
      return LINE_TYPE_DATA;
   }
}



//////////////////////////////
//
// PerformData::getMeasureValue -- returns the measure number
//   of the first Humdrum spine in the string
// 

int PerformData::getMeasureValue(const char* string) {
   int measurenumber = 0;
   sscanf(string, "=%d", &measurenumber);
   return measurenumber;
}



//////////////////////////////
//
// PerformData::getNextLine -- reads in next non-comment line
//     Used only with the function inputHumdrumMidiFile().
//

int PerformData::getNextLine(char* buffer, istream& input) {
   static int line_in_file = 0;
   static PerformDataRecord pdr;

   if (buffer == NULL) {
      return line_in_file;
   }

   if (input.eof()) {
      buffer[0] = '\0';
      return 0;
   }

   input.getline(buffer, 960, '\n');
   line_in_file++;
   return 1;
}



//////////////////////////////
//
// PerformData::getSpineCount -- counts the number of tab characters
//     in a line and return a value one greater.  Used only with the
//     function inputHumdrumMidiFile
//

int PerformData::getSpineCount(const char* aString) {
   int output = 1;
   for (int i=0; aString[i] != '\0' ; i++) {
      if (aString[i] == '\t') {
         output++;
      }
   }
   return output;
}



//////////////////////////////
//
// PerformData::performRecordCompare -- for the sort() command
//

int PerformData::performRecordCompare(const void* a, const void* b) {
   PerformDataRecord& A = **((PerformDataRecord**)a);
   PerformDataRecord& B = **((PerformDataRecord**)b);

   if (A.getTime() < B.getTime()) {
      return -1;
   } else if (A.getTime() > B.getTime()) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// PerformData::processInterpLine -- reads a Humdrum file
//   interpretation from the inputHumdrumMidiFile function().
//

#define SPINE_TERMINATE  -1
#define SPINE_NOTHING     0
#define SPINE_ADD         1
#define SPINE_EXCHANGE    2
#define SPINE_SPLIT       3
#define SPINE_JOIN        4

void PerformData::processInterpLine(const char* line, 
      Array<int>& spine_list,
      Array<int>& chan_list,
      Array<int>& path_list   ) {
   int increase = 0;
   int decrease = 0;
   int join = 0;
   
   int active_spines = 0;

   Array<char*>* spineLine = segment(line);
   Array<char*>& spine = *spineLine;

   int i, j, k;
   for (k=0; k<spine.getSize(); k++) {
      if (strcmp(spine[k], "*+") == 0) {
         increase = 1;
         path_list[k] = SPINE_ADD;
         join = 0;
      } else if (strcmp(spine[k], "*^") == 0) {
         increase = 1;
         path_list[k] = SPINE_SPLIT;
         join = 0;
      } else if (strncmp(spine[k], "*Ch", 3) == 0) {
         sscanf(spine[k], "*Ch%d", &(spine_list[k]));
         if (spine_list[k]  <= 0 || spine_list[k] > 16) {
            cout << "Error: bad MIDI channel number: " << spine_list[k] 
                 << endl;
            exit(1);
         }
      } else if (strncmp(spine[k], "*MM", 3) == 0) {
         int beats;
         int status = sscanf(spine[k], "*MM%d", &beats);
         if (status == 1) {
            PerformDataRecord temp;
            if (beats != last_tempo) {
               temp.setTempo(0, (int)last_tempo);
               add(temp);
            }
            temp.setTempo(0, beats);
            add(temp);
            last_tempo = beats;
         }
         // just need the first tempo indicator 
         break;
      } else if (strcmp(spine[k], "*-") == 0) {
         path_list[k] = SPINE_TERMINATE;
         decrease = 1;
         join = 0;
      } else if (strcmp(spine[k], "*v") == 0) {
         if (join) { 
            path_list[k] = SPINE_TERMINATE;
            decrease = 1;
         } else {
            join = 1;
         }
      } else if (strcmp(spine[k], "*x") == 0) {
         increase = 1;
         path_list[k] = SPINE_EXCHANGE;
         join = 0;
      } 

   } // end of for loop



   // process any changes in spine statae
   if (increase || decrease) {
      for (i=0; i<active_spines; i++) {
         switch (path_list[i]) {
            case SPINE_ADD:
               path_list[i] = SPINE_NOTHING;
               for (j=active_spines-1; j>=i; j--) {
                  chan_list[j+1] = chan_list[j];
                  spine_list[j+1] = spine_list[j];
                  path_list[j+1] = path_list[j];
               }
               active_spines++;
               i++;
               chan_list[i] = -1;
               spine_list[i] = -1;
               break;
            case SPINE_SPLIT:
               path_list[i] = SPINE_NOTHING;
               for (j=active_spines-1; j>=i; j--) {
                  chan_list[j+1] = chan_list[j];
                  spine_list[j+1] = spine_list[j];
                  path_list[j+1] = path_list[j];
               }
               active_spines++;
               i++;
               break;
            case SPINE_TERMINATE:
               for (j=active_spines-1; j>=i; j--) {
                  chan_list[j+1] = chan_list[j];
                  spine_list[j+1] = spine_list[j];
                  path_list[j+1] = path_list[j];
               } 
               path_list[active_spines-1] = 0;
               active_spines--;
               i--;
               break;
            case SPINE_EXCHANGE:
               path_list[i] = SPINE_NOTHING;
               for (j=i+1; j<active_spines && path_list[j] != 2; j++) {
                  // do nothing??
               }
               if (j < active_spines) {
                  path_list[j] = SPINE_NOTHING;
                  int exchange = spine_list[j];
                  spine_list[j] = spine_list[i];
                  spine_list[i] = exchange;
                  exchange = chan_list[j];
                  chan_list[j] = chan_list[i];
                  chan_list[i] = exchange;
               } 
               break;
         }
      }
   }
}



//////////////////////////////
//
// PerformData::processExclusiveInterpLine -- reads a Humdrum file
//   interpretation from the inputHumdrumMidiFile function().
//

void PerformData::processExclusiveInterpLine(const char* line,
      Array<int>& chan_list) {
   Array<char*>* spineLine = segment(line);
   Array<char*>& spine = *spineLine;

   for (int i=0; i<spine.getSize(); i++) {
      if (strcmp(spine[i], "**MIDI") == 0) {
         chan_list[i] = 1;
      }
   }
}



//////////////////////////////
//
// PerformData::processHumdrumData -- read a humdrum data entry
//   other than measure records and interpretation records.
//

void PerformData::processHumdrumData(const char* line, 
      Array<int>& spine_list, Array<int>& chan_list) {

   Array<char*>* spineLine = segment(line);
   Array<char*>& spine = *spineLine;

   char buffer[1024];
   int num_matched;
   int timing;               // for sscnaf of MIDI delta time
   int command = 0x90;       // MIDI command byte
   int note;                 // for sscanf of MIDI keynote
   int velocity;             // for sscanf of MIDI velocity
   int delta = -1;

   for (int i=0; i<spine.getSize(); i++) {
      if (chan_list[i] != 1) {
         continue;
      }
      
      // we are interpreting a **MIDI spine at this point

      strncpy(buffer, spine[i], 1023);
      char* item = strtok(buffer, " ");
      while (item != NULL) {
         num_matched = sscanf(item, "%d/%d/%d", &timing, &note, &velocity);

         if (num_matched == 2) {
            num_matched = 3;
            velocity = 64;
         } 

         if (num_matched == EOF || num_matched != 3) {
            break;
         }

         if (timing < 0) {
            timing = 0;
         }

         if (delta < 0) {
            delta = timing;
         } else {
            delta = 0;
         }
  
         // Error checking: checks if a spine has been headed up correctly
         if (spine_list[i] == 0 && chan_list[i] == 1) {
            cout << spine_list[i] << " " << chan_list[i] 
                 << " perform: no channel specified on spine " << i << endl;
            exit(1);
         }

         if (note > 0) {
            command = 0x90 | (spine_list[i] - 1);
         } else if (note > -128) {
            command = 0x80 | (spine_list[i] - 1);
            note = -note;
         }

         char mididata[3];
         mididata[0] = (unsigned char)command;
         mididata[1] = (unsigned char)note;
         mididata[2] = (unsigned char)velocity;
         PerformDataRecord midicommand;
         midicommand.setMidi(delta, mididata, 3);
         add(midicommand);

         item = strtok(NULL, " ");
      }

   }  // end of for each spine loop

}



//////////////////////////////
//
// PerformData::segment -- reads a Humdrum file
//   interpretation from the inputHumdrumMidiFile function().
//

Array<char*>* PerformData::segment(const char* line) {

   static char buffer[1024] = {0};
   static Array<char*> output(0);

   // delete old data if it exists
   int i;
   for (i=0; i<output.getSize(); i++) {
      if (output[i] != NULL) {
         delete [] output[i];
         output[i] = NULL;
      }
   }
   output.setSize(0);

   char* temp;
   int length;
   strncpy(buffer, line, 1023);
   char* field = strtok(buffer, "\t"); 
   while (field != NULL) {
      length = strlen(field);
      temp = new char[length + 1];
      strcpy(temp, field);
      output.append(temp);      
      field = strtok(NULL, "\t"); 
   }

   return &output;
}



// md5sum: 078eb9ca7e420684043e35b5038312ed PerformData.cpp [20050403]
