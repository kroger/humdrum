//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Feb 14 23:40:51 PST 2002
// Last Modified: Mon Feb 25 18:27:30 PST 2002
// Last Modified: Sun Mar 24 22:27:25 PST 2002 (changed reading of trailer)
// Last Modified: Tue Mar 26 00:52:54 PST 2002 (added staff access data)
// Last Modified: Tue Mar 31 16:14:05 PST 2009 (updates for winscore version)
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Filename:      ...sig/src/sigInfo/ScorePageBase.h
// Web Address:   http://sig.sapp.org/include/sigInfo/ScorePageBase.h
// Syntax:        C++ 
//
// Description:   Data structure for a page of SCORE data
//

#include "ScorePageBase.h"

#include <string.h>
#include <math.h>

#ifndef OLDCPP
   #include <iostream>
   #include <iomanip>
   #include <fstream>
   using namespace std;
#else
   #include <iostream.h>
   #include <iomanip.h>
   #include <fstream.h>
#endif

typedef long TEMP64BITFIX;

//////////////////////////////
//
// ScorePageBase::ScorePageBase --
//

ScorePageBase::ScorePageBase(void) { 
   sortQ = 0;

   data.setSize(10000);
   data.setGrowth(10000);
   data.setSize(0);

   trailer.setSize(100);
   trailer.setGrowth(100);
   trailer.setSize(0);
   initializeTrailer();

   // system analysis values
   staffsystem.setSize(100);
   staffsystem.setGrowth(100);
   staffsystem.setSize(0);
   systemind.setSize(0);
   systemSize.setSize(0);
   systemStart.setSize(0);
   track.setSize(100);
   track.setGrowth(100);
   track.setSize(0);
   voice.setSize(100);
   voice.setGrowth(100);
   voice.setSize(0);
   systemAnalysisQ = 0;
   systemCount     = 0;
   staffCount      = 0;
   maxStaffNumber  = 0;

   // maximum staff number in SCORE is 32.  Allow up to number 99 in this class:
   staffStart.setSize(100);
   staffStart.setAll(-1);
   staffSize.setSize(100);
   staffSize.setAll(0);

}



//////////////////////////////
//
// ScorePageBase::~ScorePageBase --
//

ScorePageBase::~ScorePageBase() { 
   // clear();
}



//////////////////////////////
//
// ScorePageBase::clear --
//

void ScorePageBase::clear(void) {
   data.setSize(0);
   initializeTrailer();

   // system analysis variables
   staffsystem.setSize(0);
   systemAnalysisQ = 0;
   systemCount     = 0;
   staffCount      = 0;
   maxStaffNumber  = 0;
   staffStart.setAll(-1);
   staffSize.setAll(0);
   systemind.setSize(0);
   systemSize.setSize(0);
   systemStart.setSize(0);
}



//////////////////////////////
//
// ScorePageBase::operator= --
//

ScorePageBase& ScorePageBase::operator=(ScorePageBase &aPage) {
   if (this == &aPage) {
      return *this;
   }
   data           = aPage.data;
   trailer        = aPage.trailer;
   sortQ          = aPage.sortQ;
   systemAnalysisQ= aPage.systemAnalysisQ;
   staffsystem    = aPage.staffsystem;
   track          = aPage.track;
   voice          = aPage.voice;
   systemCount    = aPage.systemCount;
   staffCount     = aPage.staffCount;
   maxStaffNumber = aPage.maxStaffNumber;
   staffStart     = aPage.staffStart;
   staffSize      = aPage.staffSize;
   systemind      = aPage.systemind;
   systemStart    = aPage.systemStart;
   systemSize     = aPage.systemSize;

   return *this;
}



//////////////////////////////
//
// ScorePageBase::findStaff -- find the index of the first occurance of a
//     score item on the given staff.  Returns -1 if there is no
//     staff or items on that staff.
//

int ScorePageBase::findStaff(int staffno) {
   if (!sortQ) {
      sortByStaff();
   }

   ScoreRecord testrec;
   testrec.setValue(0, 0.0);     // 0 staff item
   testrec.setValue(1, (float)staffno);
   testrec.setValue(2, 0.0);     // horizontal pos
   testrec.setValue(3, 0.0);     // vertical pos

   void* searchresult;
   searchresult = bsearch(&testrec, data.getBase(), data.getSize(), 
         sizeof(ScoreRecord), staffsearch);
   if (searchresult == NULL) {
      return -1;
   } else {
      int start = ((TEMP64BITFIX)searchresult - 
                   (TEMP64BITFIX)data.getBase())/sizeof(ScoreRecord);
      int i;
      for (i=start; i>=0; i++) {
         if ((int)data[i].getPValue(2) != staffno) {
            return i+1;
         }
      }
      return 0;
   }
   return -1;
}



//////////////////////////////
//
// ScorePageBase::readFile -- check whether the file is a binary or
//    ascii file, and read accordingly.
//     default value: verboseQ = 0
//

void ScorePageBase::readFile(const char* filename, int verboseQ) {
   int binaryQ = 0;

   #ifndef OLDCPP
      #ifdef VISUAL
         ifstream testfile(filename, ios::binary);
      #else 
         ifstream testfile(filename);
      #endif
   #else 
      #ifdef VISUAL
         ifstream testfile(filename, ios::nocreate | ios::binary);
      #else 
         ifstream testfile(filename, ios::nocreate);
      #endif
   #endif

   if (!testfile.is_open()) {
      cerr << "Error: cannot read the file: " << filename << endl;
      exit(1);
   }

   testfile.seekg(-4, ios::end);
   unsigned char databytes[4] = {0xff};
   testfile.read((char*)databytes, 4); 
   if (databytes[0] == 0x00 && databytes[1] == 0x3c && databytes[2] == 0x1c && 
         databytes[3] == 0xc6) {
      binaryQ = 1;
   } else {
      binaryQ = 0;
   }

   if (binaryQ) {
      readBinary(filename, verboseQ);
   } else {
      readAscii(filename, verboseQ);
   }
}



//////////////////////////////
//
// ScorePageBase::readAscii -- Read data in PMX format from given input 
//     textfile.
//     default value: verboseQ = 0
//

void ScorePageBase::readAscii(const char* filename, int verboseQ) { 
   #ifndef OLDCPP
      fstream infile(filename, ios::in);
   #else
      fstream infile(filename, ios::in | ios::nocreate);
   #endif

   if (!infile.is_open()) {
      cerr << "Error: cannot read file: " << filename << endl;
      exit(1);
   }

   ScoreRecord record;
   int i = 0;
   data.allowGrowth(1);
   while (!infile.eof()) {
      readAsciiScoreLine(infile, data[i++]);
   }
   data.setSize(data.getSize()-1);
   data.allowGrowth(0);

   union { long i; float f; } u;
   u.i = 0x50504153;

   initializeTrailer();
   infile.close();
}



//////////////////////////////
//
// initializeTrailer --
//   devault value: serial = 0x50504153
//

void ScorePageBase::initializeTrailer(long serial) {
   union { long i; float f; } u;
   u.i = serial;

   trailer.setSize(4);
   trailer[0] = 0.0;  // units: 0.0 = inches, 1.0 = centimeters
   trailer[1] = 3.0;  // version of score being used
   trailer[2] = u.f;  // serial number of program 
   trailer[3] = 0.0;  // always zero?
   trailer.allowGrowth(0);
}



//////////////////////////////
//
// ScorePageBase::printAscii -- Print data in PMX format for readable text.
//     default value: roundQ   = 0
//     default value: verboseQ = 0
//

void ScorePageBase::printAscii(ostream& out, int roundQ, int verboseQ) { 
   int i, j;
   if (verboseQ) {
      cout << "; OBJECTS TO WRITE: " << data.getSize() << endl;
   }
   Array<Array<char> > buffers;
   buffers.setSize(data.getSize() * 2);
   buffers.setSize(0);

   char buffer1[10000] = {0};
   char buffer2[10000] = {0};
   int index;
   Array<char> spaces;
   spaces.setSize(10000);
   spaces.setAll(1);
   int length;

   for (i=0; i<data.getSize(); i++) {
      // data[i].printAscii(out, verboseQ);
      data[i].printAscii(buffer1, buffer2, 10000, roundQ, verboseQ);
      index = buffers.getSize();
      buffers.setSize(buffers.getSize()+1);
      length = strlen(buffer1);
      buffers[index].setSize(length+1);
      strcpy(buffers[index].getBase(), buffer1);
      for (j=0; j<length; j++) {
         if (buffer1[j] != ' ') {
            spaces[j] = 0;
         }
      }
      
      length = strlen(buffer2);
      if (length > 0) {
         index = buffers.getSize();
         buffers.setSize(buffers.getSize()+1);
         buffers[index].setSize(length+1);
         strcpy(buffers[index].getBase(), buffer2);
      }
      
      // out << '\n';
   }

   for (i=0; i<buffers.getSize(); i++) {
      if ((i>0) && (strchr(buffers[i-1].getBase(), 't') != NULL)) {
         out << buffers[i].getBase();
      } else {
         for (j=0; j<buffers[i].getSize()-1; j++) {
            if (spaces[j] == 0) {
               out << buffers[i][j];
            } else if ((spaces[j] == 1) && (spaces[j+1] != 1)) {
               out << buffers[i][j];
            }
         }
	 // don't print the null character:
         // out << buffers[i][buffers[i].getSize()-1];
      }
      out << "\n";
   }

   out << flush;
}



//////////////////////////////
//
// setVersion -- 
//

void ScorePageBase::setVersion(float value) {
   trailer[1] = value;
}

void ScorePageBase::setVersionWinScore(void) {
   trailer[1] = 6.0;
}



//////////////////////////////
//
// getVersion --
//

float ScorePageBase::getVersion(void) {
   return trailer[1];
}



//////////////////////////////
//
// setSerial -- 
//

void ScorePageBase::setSerial(long value) {
   union { long i; float f; } u;
   u.i = value;
   trailer[2] = u.f;
}



//////////////////////////////
//
// getSerial -- 
//

long ScorePageBase::getSerial(void) {
   union { long i; float f; } u;
   u.f = trailer[2];
   return u.i;
}




//////////////////////////////
//
// ScorePageBase::writeBinary --
//

void ScorePageBase::writeBinary(const char* filename) {

   float version = getVersion();

   #ifndef OLDCPP
      #ifdef VISUAL
         fstream outfile(filename, ios::out | ios::binary);
      #else
         fstream outfile(filename, ios::out);
      #endif
   #else
      #ifdef VISUAL
         fstream outfile(filename, ios::out | ios::noreplace | ios::binary);
      #else
         fstream outfile(filename, ios::out | ios::noreplace);
      #endif
   #endif

   if (!outfile.is_open()) {
      cerr << "Error: cannot write file: " << filename << endl;
      exit(1);
   }

   // first write the number of numbers in the data file.
   // use a dummy value of 0 for now
   char dummy[4] = {0};
   if (version < 6) {
      outfile.write(dummy, 2);
   } else {
      outfile.write(dummy, 4);
   }
   int writecount = 0;   // number of numbers which have been read

   int i;
   for (i=0; i<data.getSize(); i++) {
      writecount += data[i].writeBinary(outfile);
   }

   // write the trailer
   for (i=trailer.getSize()-1; i>=0; i--) {
      writeLittleFloat(outfile, trailer[i]);
      writecount++;
   }
   // write the size of the trailer (plus one more for this field)
   writeLittleFloat(outfile, trailer.getSize() + 1);
   writecount++;

   // write the end of file marker
   writeLittleFloat(outfile, -9999.0);
   writecount++;

   if ((version < 6) && (writecount > 0x7fff)) {
      cerr << "Warning: data count (" << writecount << ") exceeds safe level"
           << endl;
   }

   // go back to the start of the file and fill in the number of numbers
   outfile.seekp(0);
   if (version < 6) {
      // write two byte counter
      unsigned char blo = writecount & 0xff;
      unsigned char bhi = (writecount >> 8) & 0xff;
      outfile.write((char*)&blo, 1);
      outfile.write((char*)&bhi, 1);
   } else {
      // write four-byte counter 
      unsigned char bloest = writecount & 0xff;
      unsigned char bloer  = (writecount >> 8)  & 0xff;
      unsigned char bhier  = (writecount >> 16) & 0xff;
      unsigned char bhiest = (writecount >> 24) & 0xff;
      outfile.write((char*)&bloest, 1);
      outfile.write((char*)&bloer,  1);
      outfile.write((char*)&bhier,  1);
      outfile.write((char*)&bhiest, 1);
   }
   outfile.close();
}


void ScorePageBase::writeBinary2Byte(const char* filename) {
   setVersion(3.0);
   writeBinary(filename);
}


void ScorePageBase::writeBinary4Byte(const char* filename) {
   setVersion(6.0);
   writeBinary(filename);
}


//////////////////////////////
//
// ScorePageBase::readBinary --
//     default value: verboseQ = 0;
//

void ScorePageBase::readBinary(const char* filename, int verboseQ) {
   clear();

   #ifndef OLDCPP
      #ifdef VISUAL
         fstream infile(filename, ios::in | ios::binary);
      #else
         fstream infile(filename, ios::in);
      #endif
   #else
      #ifdef VISUAL
         fstream infile(filename, ios::in | ios::nocreate | ios::binary);
      #else
         fstream infile(filename, ios::in | ios::nocreate);
      #endif
   #endif

   if (!infile.is_open()) {
      cerr << "Error: cannot open file: " << filename << endl;
      exit(1);
   }

   // first read the number of numbers in the data file.
   int numbercount = readLittleShort(infile);
   int readcount = 0;   // number of numbers which have been read
   if (verboseQ) {
      cout << "; NumberCount = " << numbercount << endl;
   }

   // now read the number of numbers in the trailer
   infile.seekg(-8, ios::end);
   int trailerSize = (int)readLittleFloat(infile);

   // go back to the start of the file, after the first number
   infile.seekg(2, ios::beg);

   float number = 0;
   data.allowGrowth(1);
   // now read each data number and store
   while (!infile.eof()) {
      if (numbercount - readcount - trailerSize - 1 == 0) {
         break;
      } else if (numbercount - readcount - trailerSize - 1< 0) {
         cout << "Error reading file: data mixes with trailer: " 
              << numbercount - readcount - trailerSize - 1
              << endl;
         exit(1);
      } else {
         number = readLittleFloat(infile);
         readcount++;
         if (verboseQ) {
            if (number - (int)number > 0.0) {
               cout << "; Error in number parameter count: " << number << endl;
            }
         }

         readcount += (int)number;
         data[data.getSize()].readBinary(infile, (int)number);
      }
   }


   if (verboseQ) { 
      cout << "Elements: " << data.getSize() << endl;
      cout << "READING Trailer: " << endl;
   }



   while (number != -9999.0 && !infile.eof()) {
      number = readLittleFloat(infile);
      trailer.append(number);
      if (verboseQ) {
         cout << "TRAILER NUMBER: " << number << endl;
      }
      readcount++;
   }

   if (readcount != numbercount) {
      cerr << "Warning: expecting " << numbercount << " number in file "
           << " but read " << readcount << endl;
   }

}



//////////////////////////////
//
// ScorePageBase::getSize --
//

int ScorePageBase::getSize(void) {
   return data.getSize();
}



//////////////////////////////
//
// ScorePageBase::operator[] --
//

ScoreRecord& ScorePageBase::operator[](int index) { 
   return data[index];
}



//////////////////////////////
//
// ScorePageBase::appendItem --
//

void ScorePageBase::appendItem(ScoreRecord& aRecord) { 
   data.append(aRecord);
}

void ScorePageBase::appendItem(ScorePageBase& aPage) {
   int i;
   for (i=0; i<aPage.getSize(); i++) {
      appendItem(aPage[i]);
   }
}

void ScorePageBase::appendItem(SigCollection<ScoreRecord>& recs) {
   int i;
   for (i=0; i<recs.getSize(); i++) {
      appendItem(recs[i]);
   }
}



//////////////////////////////
//
// ScorePageBase::getItemPosition --
//   default value: tolerance = 0.1
//

void ScorePageBase::getItemsPosition(Array<int>& indices, float position, 
      int staff, float tolerance) {
   indices.setSize(0);
   indices.allowGrowth(1);
   int i; 
   for (i=0; i<data.getSize(); i++) {
      if (fabs(position - data[i].getPValue(3)) <= tolerance) {
         if (staff == 99) {
            indices.append(i);
         } else if (staff == (int)data[i].getPValue(2)) {
            indices.append(i);
         }
      }
   }
   indices.allowGrowth(0);
}



//////////////////////////////////////////////////////////////////////////
//
// private Functions
//


//////////////////////////////
//
// ScorePageBase::readLittleShort -- read a short int in little endian form.
//     Number is read as an unsigned short int.
//

int ScorePageBase::readLittleShort(istream& input) {
   unsigned char byteinfo[2];
   input.read((char*)byteinfo, 2);
   int output = 0;
   output = byteinfo[1];
   output = (output << 8) | byteinfo[0];
   return output;
}




//////////////////////////////
//
// ScorePageBase::writeLittleFloat --
//

void ScorePageBase::writeLittleFloat(ostream& out, float number) { 
   union { float f; unsigned int i; } num;
   num.f = number;
   char byteinfo[4];
   byteinfo[0] = (char)( num.i        & 0xff);
   byteinfo[1] = (char)((num.i >> 8)  & 0xff);
   byteinfo[2] = (char)((num.i >> 16) & 0xff);
   byteinfo[3] = (char)((num.i >> 24) & 0xff);
   out.write(byteinfo, 4);
}



//////////////////////////////
//
// ScorePageBase::readLittleFloat --
//

float ScorePageBase::readLittleFloat(istream& instream) { 
   unsigned char byteinfo[4];
   instream.read((char*)byteinfo, 4);
   union { float f; unsigned int i; } num;
   num.i = 0;
   num.i = byteinfo[3];
   num.i = (num.i << 8) | byteinfo[2];
   num.i = (num.i << 8) | byteinfo[1];
   num.i = (num.i << 8) | byteinfo[0];
   return num.f;
}



//////////////////////////////
//
// ScorePageBase::shrinkParameters --
//

void ScorePageBase::shrinkParameters(void) {
   int i;
   for (i=0; i<data.getSize(); i++) {
      data[i].shrink();
   }
}



//////////////////////////////
//
// ScorePageBase::readAsciiScoreLine -- read a PMX file for score data.
//   ignore any lines of text which do not start with a number.
//

void ScorePageBase::readAsciiScoreLine(istream& infile, ScoreRecord& record,
      int verboseQ) {
   char buffer[1024] = {0};
   infile.getline(buffer, 1000, '\n');
   if (verboseQ) {
      cout << "Read line: " << buffer << endl;
   }

   record.clear();
   record.setAllocSize(100);
   char* ptr = strtok(buffer, "\n\t ");
   float number = 0.0;
   int counter = 0;
   if (ptr != NULL) {
      if (strcmp(ptr, "t") == 0) {
         // text parameter
         number = 16.0;
         record.setValue(counter++, number);
         ptr = strtok(NULL, "\n\t ");
         while (ptr != NULL) {
            number = (float)strtod(ptr, NULL);
            record.setValue(counter++, number);
            ptr = strtok(NULL, "\n\t ");
         }
         // now read the text line for a text item
         infile.getline(buffer, 1000, '\n');
         if (verboseQ) {
            cout << "Read text line: " << buffer << endl;
         }
         record.setTextData(buffer);
         int length = strlen(buffer);
         record.setPValue(12, (float)length);
      } else {
         // non-text parameter
         number = (float)strtod(ptr, NULL);
         if (number == 0.0) {
            return;
         } else {
            record.setValue(counter++, number);
            ptr = strtok(NULL, "\n\t ");
            while (ptr != NULL) {
               number = (float)strtod(ptr, NULL);
               record.setValue(counter++, number);
               ptr = strtok(NULL, "\n\t ");
            }
         }
      }
   } 
}



//////////////////////////////
//
// ScorePageBase::sortByStaff -- sort from lowest to highest staff,
//     and by horizontal position and code item number.
//

int ScorePageBase::sortByStaff(void) {
   sortQ = 1;
   qsort(data.getBase(), data.getSize(), sizeof(ScoreRecord), compareStaff);
   return sortQ;
}



//////////////////////////////
//
// ScorePageBase::compareStaff -- sort staff number, then horizontal position,
//     then item code, then vertical position
//

int ScorePageBase::compareStaff(const void* A, const void* B) {
   ScoreRecord& a = *((ScoreRecord*)A);
   ScoreRecord& b = *((ScoreRecord*)B);

   if (a.getPValue(2) < b.getPValue(2)) {
      return -1;
   } else if (a.getPValue(2) > b.getPValue(2)) {
      return 1;
   }

   // staff line tie: resolve by horizontal postion
   if (a.getPValue(3) < b.getPValue(3)) {
      return -1;
   } else if (a.getPValue(3) > b.getPValue(3)) {
      return 1;
   }

   // horizontal postion tie: resolve by code item
   if (a.getPValue(1) < b.getPValue(1)) {
      return -1;
   } else if (a.getPValue(1) > b.getPValue(1)) {
      return 1;
   }

   // still a tie: sort from lowest vertical postion to highest
   // vertical position
   if (a.getPValue(4) < b.getPValue(4)) {
      return -1;
   } else if (a.getPValue(4) > b.getPValue(4)) {
      return 1;
   }

   // still a tie: give up and say they are equal
   return 0;

}



//////////////////////////////
//
// ScorePageBase::compareSystem -- sort by system, then horizontal position,
//     then staff number, then item code, then vertical position
//

int ScorePageBase::compareSystem(const void* A, const void* B) {
   ScoreRecord& a = *(((SystemRecord*)A)->ptr);
   ScoreRecord& b = *(((SystemRecord*)B)->ptr);

   // Sort by horizontal postion
   if (a.getPValue(3) < b.getPValue(3)) {
      return -1;
   } else if (a.getPValue(3) > b.getPValue(3)) {
      return 1;
   }

   // then by staff line
   if (a.getPValue(2) < b.getPValue(2)) {
      return -1;
   } else if (a.getPValue(2) > b.getPValue(2)) {
      return 1;
   }

   // horizontal postion tie: resolve by code item
   if (a.getPValue(1) < b.getPValue(1)) {
      return -1;
   } else if (a.getPValue(1) > b.getPValue(1)) {
      return 1;
   }

   // still a tie: sort from lowest vertical postion to highest
   // vertical position
   if (a.getPValue(4) < b.getPValue(4)) {
      return -1;
   } else if (a.getPValue(4) > b.getPValue(4)) {
      return 1;
   }

   // still a tie: give up and say they are equal
   return 0;

}



//////////////////////////////
//
// ScorePageBase::staffsearch -- for finding staves in a list.
//

int ScorePageBase::staffsearch(const void* A, const void* B) {
   ScoreRecord& a = *((ScoreRecord*)A);
   ScoreRecord& b = *((ScoreRecord*)B);

   if (a.getPValue(2) < b.getPValue(2)) {
      return -1;
   } else if (a.getPValue(2) > b.getPValue(2)) {
      return 1;
   }
   return 0;
}




// md5sum: e1c6f792fbddebf040ffe0355fedb619 ScorePageBase.cpp [20050403]
