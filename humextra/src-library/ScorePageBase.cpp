//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Feb 14 23:40:51 PST 2002
// Last Modified: Sun Mar 24 22:27:25 PST 2002 Changed reading of trailer
// Last Modified: Tue Mar 26 00:52:54 PST 2002 Added staff access data
// Last Modified: Tue Mar 31 16:14:05 PST 2009 Updates for winscore version
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 Renamed SigCollection class
// Last Modified: Sun Aug 26 00:55:11 PDT 2012 Renovated
// Filename:      ...sig/src/sigInfo/ScorePageBase.h
// Web Address:   http://sig.sapp.org/include/sigInfo/ScorePageBase.h
// Syntax:        C++ 
//
// Description:   Base class for ScorePage.  This class handles reading/writing
//                of a ScorePage, and handles all of the data variables for
//                a ScorePage (the main storage array and lots of analysis
//                arrays).
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
// ScorePageBase::ScorePageBase -- Constructors.
//

ScorePageBase::ScorePageBase(void) { 
   // Initialize space for the main data array, set the allocation 
   // size to 10,000 and the regrowth size to +100,000 if it exceeds 
   // the initial allocation size.
   data.setSize(10000);
   data.setGrowth(100000);
   data.setSize(0);

   // clear all other variables
   clear();
}


ScorePageBase::ScorePageBase(ScorePageBase& aPage) { 
   data.setSize(10000);
   data.setGrowth(100000);
   data.setSize(0);

   ScorePageBase& thispage = *this;
   thispage = aPage;
}



//////////////////////////////
//
// ScorePageBase::~ScorePageBase -- Destructor.
//

ScorePageBase::~ScorePageBase() { 
   clear();
}



//////////////////////////////
//
// ScorePageBase::operator= -- Copying data from another ScorePageBase..
//

ScorePageBase& ScorePageBase::operator=(ScorePageBase &aPage) {
   if (this == &aPage) {
      return *this;
   }
   clear();

   data.setSize(aPage.data.getSize());
   int i;
   for (i=0; i<data.getSize(); i++) {
      data[i] = new ScoreRecord;
      *(data[i]) = *(aPage.data[i]);
   }

   trailer           = aPage.trailer;

   // Data sequences
   // Simple staff analysis sequences:
   pagePrintSequence = aPage.pagePrintSequence;
   lineStaffSequence.setSize(aPage.lineStaffSequence.getSize());
   for (i=0; i<lineStaffSequence.getSize(); i++) {
      lineStaffSequence[i] = aPage.lineStaffSequence[i];
   }
   pageStaffList     = aPage.pageStaffList;
   maxStaffNumber    = aPage.maxStaffNumber;

   // System analysis sequences:
   lineSystemSequence.setSize(aPage.lineSystemSequence.getSize());
   for (i=0; i<lineSystemSequence.getSize(); i++) {
      lineSystemSequence[i] = aPage.lineSystemSequence[i];
   }
   pageSystemSequence = aPage.pageSystemSequence;
   pageStaffSequence.setSize(aPage.pageStaffSequence.getSize());
   for (i=0; i<pageStaffSequence.getSize(); i++) {
      pageStaffSequence[i] = aPage.pageStaffSequence[i];
   }

   // Lookup tables
   objectSysStaffIndex   = aPage.objectSysStaffIndex;
   staffToSystemIdx      = aPage.staffToSystemIdx;
   staffToSystemStaffIdx = aPage.staffToSystemStaffIdx;

   // Analytic data
   systemRhythm.setSize(aPage.systemRhythm.getSize());
   for (i=0; i<systemRhythm.getSize(); i++) {
      systemRhythm[i] = aPage.systemRhythm[i];
   }
   systemDuration  = aPage.systemDuration;
   pageDuration    = aPage.pageDuration;

   // Analytic booleans
   sortAnalysisQ   = aPage.sortAnalysisQ;
   printAnalysisQ  = aPage.printAnalysisQ;
   systemAnalysisQ = aPage.systemAnalysisQ;
   pitchAnalysisQ  = aPage.pitchAnalysisQ;
   rhythmAnalysisQ = aPage.rhythmAnalysisQ;

   return *this;
}



//////////////////////////////
//
// ScorePageBase::clear --  Clear the contents of the page so that
//     data can be added to the file.
//

void ScorePageBase::clear(void) {
   for (int i=0; i<data.getSize(); i++) {
      delete data[i];
      data[i] = NULL;
   }
   data.setSize(0);

   // create a default trailer
   initializeTrailer();

   // clear analysis variables
   invalidateAnalyses();
}



//////////////////////////////
//
// ScorePageBase::invalidateAnalyses -- Mark the analyses as invalid and
//    clear the analysis arrays.
//

void ScorePageBase::invalidateAnalyses(void) {
   sortAnalysisQ   = 0;
   printAnalysisQ  = 0;
   systemAnalysisQ = 0;
   pitchAnalysisQ  = 0;
   rhythmAnalysisQ = 0;

   pagePrintSequence.setSize(0);

   // staff/system analysis variables:
   lineStaffSequence.setSize(0);
   maxStaffNumber = -1;
   pageStaffList.setSize(0);
   lineSystemSequence.setSize(0);
   pageSystemSequence.setSize(0);
   pageStaffSequence.setSize(0);
   objectSysStaffIndex.setSize(0);
   staffToSystemIdx.setSize(0);

   // rhythm analysis variables:
   systemRhythm.setSize(0);
   systemDuration.setSize(0);
   pageDuration = 0;
   
   // not deleting the pitch analyses which are stored as a key parameter
   // inside of each note object.
}



//////////////////////////////
//
// initializeTrailer --  Place dummy information in the trailer. 
//     This data may be overwritten when reading a binary SCORE file.
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


///////////////////////////////////////////////////////////////////////////
//
// Reading and Writing functions.
//

//////////////////////////////
//
// ScorePageBase::readFile -- check whether the file is a binary or
//    ascii file, and read accordingly.
//     default value: verboseQ = 0
//

void ScorePageBase::readFile(const char* filename, int verboseQ) {
   int binaryQ = 0;  // to test if reading a binary or ASCII data file.

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
   testfile.seekg(0, ios::beg);

   if (binaryQ) {
      readBinary(testfile, verboseQ);
   } else {
      readAscii(testfile, verboseQ);
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
      ifstream infile(filename, ios::in);
   #else
      ifstream infile(filename, ios::in | ios::nocreate);
   #endif

   if (!infile.is_open()) {
      cerr << "Error: cannot read file: " << filename << endl;
      exit(1);
   }

   readAscii(infile, verboseQ);
   infile.close();
}



//////////////////////////////
//
// ScorePageBase::readASCII --  Read a single SCORE page in the ASCII
//      PMX data format from an input stream.
//

void ScorePageBase::readAscii(istream& infile, int verboseQ) { 
   clear();

   ScoreRecord record;
   while (!infile.eof()) {
      readAsciiScoreLine(infile, record);
      appendItem(record);
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
      cout << "#Read line: " << buffer << endl;
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
            cout << "#Read text line: " << buffer << endl;
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
// ScorePageBase::readBinary -- Read a single SCORE page in the binary
//     data format.
//     default value: verboseQ = 0;
//

void ScorePageBase::readBinary(const char* filename, int verboseQ) {
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

   readBinary(infile, verboseQ);
}



//////////////////////////////
//
// ScorePageBase::readBinary --  Read a single SCORE page in the binary
//      data format from an input stream.
//

void ScorePageBase::readBinary(istream& infile, int verboseQ) {
   clear();

   // first read the count of 4-byte numbers/text chunks in the data file.
   int numbercount = readLittleShort(infile);
   int readcount = 0;   // number of numbers which have been read
   if (verboseQ) {
      cout << "#NUMBER COUNT OF FILE IS " << numbercount << endl;
   }

   // now read the count of numbers in the trailer
   infile.seekg(-8, ios::end);
   int trailerSize = (int)readLittleFloat(infile);

   // go back to the start of the file, after the first number
   infile.seekg(2, ios::beg);

   double number = 0.0;
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
         number = ScoreParameters::roundFractionDigits(number, 3);
         readcount++;
         if (verboseQ) {
            if (number - (int)number > 0.0) {
               cout << "# Error in number parameter count: " << number << endl;
               exit(1);
            }
         }

         readcount += (int)number;
         data[data.getSize()]->readBinary(infile, (int)number);
      }
   }

   if (verboseQ) { 
      cout << "#Elements: " << data.getSize() << endl;
      cout << "#READING Trailer: " << endl;
   }

   while (number != -9999.0 && !infile.eof()) {
      number = readLittleFloat(infile);
      trailer.append(number);
      if (verboseQ) {
         cout << "#TRAILER NUMBER: " << number << endl;
      }
      readcount++;
   }

   if (readcount != numbercount) {
      cerr << "#Warning: expecting " << numbercount << " number in file "
           << " but read " << readcount << endl;
   }
}



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
// ScorePageBase::readLittleFloat -- Read a 4-byte float in little-endian
//      format.
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
// ScorePageBase::writeBinary --  Write SCORE data to a binary files.
//

void ScorePageBase::writeBinary(const char* filename) {

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

   writeBinary(outfile);
   outfile.close();
}



//////////////////////////////
//
// ScorePageBase::writeBinary --  Write SCORE data to a binary output stream.
//    This function should be changed to write out in the print order
//    rather than the data order.
//

ostream& ScorePageBase::writeBinary(ostream& outfile) {
   float version = getVersion();

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
      writecount += data[i]->writeBinary(outfile);
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

   return outfile;
}



//////////////////////////////
//
// ScorePageBase::writeBinary2Byte -- Write a binary file using a 2-byte
//      number counter at the start of the file (the default method).
//

void ScorePageBase::writeBinary2Byte(const char* filename) {
   setVersion(3.0);
   writeBinary(filename);
}



//////////////////////////////
//
// ScorePageBase::writeBinary4Byte -- Write a binary file using a 4-byte
//      number counter at the start of the file.  This can be read by
//      WinScore (later versions of SCORE).  It must be used if there are
//      more than 0xffff numbers (and 4-byte character chunks).
//

void ScorePageBase::writeBinary4Byte(const char* filename) {
   setVersion(6.0);
   writeBinary(filename);
}



//////////////////////////////
//
// ScorePageBase::writeLittleFloat -- Write a 4-byte float in little-endian
//      format.
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
// ScorePageBase::printAscii -- Print data in PMX format for readable text.
//     default value: roundQ   = 0
//     default value: verboseQ = 0
//

void ScorePageBase::printAscii(ostream& out, int roundQ, int verboseQ) { 
   int i, j;
   if (verboseQ) {
      cout << "# OBJECTS TO WRITE: " << data.getSize() << endl;
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
      // data[i]->printAscii(out, verboseQ);
      data[i]->printAscii(buffer1, buffer2, 10000, roundQ, verboseQ);
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
// ScorePageBase::printAsciiWithExtraParameters -- Print data in PMX format
//   for readable text.  Extra analysis parameters are also printed in this
//   function.
//
//     default value: roundQ   = 0
//     default value: verboseQ = 0
//

void ScorePageBase::printAsciiWithExtraParameters(ostream& out, int roundQ, 
      int verboseQ) {
   int i, j;
   if (verboseQ) {
      cout << "# OBJECTS TO WRITE IS " << data.getSize() << endl;
   }
   Array<Array<char> > buffers;
   buffers.setSize(data.getSize() * 2);
   buffers.setSize(0);

   Array<int> bufferindex(data.getSize() * 2);
   bufferindex.setSize(0);

   char buffer1[10000] = {0};
   char buffer2[10000] = {0};
   int index;
   Array<char> spaces;
   spaces.setSize(10000);
   spaces.setAll(1);
   int length;

   for (i=0; i<data.getSize(); i++) {
      // data[i].printAscii(out, verboseQ);
      data[i]->printAscii(buffer1, buffer2, 10000, roundQ, verboseQ);
      index = buffers.getSize();
      buffers.setSize(buffers.getSize()+1);
      length = strlen(buffer1);
      bufferindex.append(i);
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
         bufferindex.append(i);
         buffers.setSize(buffers.getSize()+1);
         buffers[index].setSize(length+1);
         strcpy(buffers[index].getBase(), buffer2);
      }
      
      // out << '\n';
   }

   for (i=0; i<buffers.getSize(); i++) {
      if ((i>0) && (strncmp(buffers[i-1].getBase(), "t", 1) == 0)) {
         out << buffers[i].getBase();
         out << "\n";
         data[bufferindex[i]]->printAsciiExtraParameters(out, roundQ, verboseQ);
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
         out << "\n";
         if (strncmp(buffers[i].getBase(), "t", 1) != 0) {
            data[bufferindex[i]]->printAsciiExtraParameters(out, roundQ, 
                  verboseQ);
         }
      }
   }

   out << flush;
}

//
// Reading and Writing functions.
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//
// data accessor functions.
//

//////////////////////////////
//
// ScorePageBase::getSize --  Returns the number of objects on the page.
//    This returns the size of the data array, or equivalently, the 
//    printOrder array.
//

int ScorePageBase::getSize(void) {
   return data.getSize();
}



//////////////////////////////
//
// ScorePageBase::operator[] -- Accesses an object on the page in the order
//    in which it was read from a file (usually considered the printing order
//    for SCORE version 3 and less).
//

ScoreRecord& ScorePageBase::operator[](int index) { 
   return *(data[index]);
}



//////////////////////////////
//
// ScorePageBase::appendItem --  Add an item to the end of the main data array.
//

void ScorePageBase::appendItem(ScoreRecord& aRecord) { 
   invalidateAnalyses();

   data.increase(1);
   data.last() = new ScoreRecord;
   *(data.last()) = aRecord;
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

void ScorePageBase::appendItem(SigCollection<ScoreRecord*>& recs) {
   int i;
   for (i=0; i<recs.getSize(); i++) {
      appendItem(*(recs[i]));
   }
}



//////////////////////////////
//
// ScorePageBase::shrinkParameters -- Compact all ScoreRecords in the 
//      data array.  So that there are no values at the end of the 
//      parameter list which are 0.0.  This is used to compress the data
//      before printing it.
//

void ScorePageBase::shrinkParameters(void) {
   int i;
   for (i=0; i<data.getSize(); i++) {
      data[i]->shrink();
   }
}

//
// data accessor functions.
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//
// Trailer accessor functions.
//

//////////////////////////////
//
// setVersion -- Set the SCORE version for writing in binary SCORE file.
//

void ScorePageBase::setVersion(float value) {
   trailer[trailer.getSize()-1-3] = value;
}



//////////////////////////////
//
// ScorePageBase::setVersionWinScore -- set version to WinScore.
//

void ScorePageBase::setVersionWinScore(void) {
   trailer[trailer.getSize()-1-3] = 6.0;
}



//////////////////////////////
//
// getVersion -- Get the SCORE version that was read from a binary SCORE file.
//

float ScorePageBase::getVersion(void) {
   return trailer[trailer.getSize()-1-3];
}



//////////////////////////////
//
// setSerial -- Set the SCORE serial number for writing in binary SCORE file.
//

void ScorePageBase::setSerial(long value) {
   union { long i; float f; } u;
   u.i = value;
   trailer[trailer.getSize()-1-4] = u.f;
}



//////////////////////////////
//
// getSerial -- Get the SCORE serial number that was red from a binary SCORE
//      file.  This value may be 0.0 if SCORE version 3 or less.
//

long ScorePageBase::getSerial(void) {
   union { long i; float f; } u;
   u.f = trailer[trailer.getSize()-1-4];
   return u.i;
}

//
// Trailer accessor functions.
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//
// Staff sorting-related functions.
//

//////////////////////////////
//
// ScorePageBase::analyzeSort -- Process "data" array and sort for use
//     in pagePrintSequence and 
//
//

void ScorePageBase::analyzeSort(void) {
   createDefaultPrintSortOrder();
   createLineStaffSequence();
}



//////////////////////////////
//
// ScorePageBase::createDefaultPrintSortOrder -- place data indices in
//     the order in which they will be printed.  The default print
//     sort order is the order in which the data is read from the input
//     file.
//

void ScorePageBase::createDefaultPrintSortOrder(void) {
   pagePrintSequence.setSize(getSize());
   pagePrintSequence.allowGrowth(0);
   pagePrintSequence.setAll(0, 1);
   printAnalysisQ = 1;
}



//////////////////////////////
//
// ScorePageBase::createLineStaffSequence -- split the page data into
//      lists of objects divided by staff ownership, and sort each list
//      by hpos (then by vpos, then by p1 value).
//

void ScorePageBase::createLineStaffSequence(void) {
   int i;

   ScorePageBase& thispage = *this;
  
   // first set the number of staves to 100.  The list will be indexed by
   // the P2 value of the object (staff 0 will not be used for regular staves).
   // Currently staff count on a page of 99 or less is allowed.
   lineStaffSequence.setSize(100);
   for (i=0; i<lineStaffSequence.getSize(); i++) {
      lineStaffSequence[i].setSize(0);
   }

   // count the number of objects on each staff (so that the space
   // can be preallocated).
   Array<int> objcount(100);
   objcount.setAll(0);
   int staffnum;
   for (i=0; i<thispage.getSize(); i++) {
      staffnum = thispage[i].getStaffNumber();
      if (staffnum < 0 || staffnum >= 100) {
         continue;
      }
      objcount[staffnum]++;
   }

   // pre-allocate space for each staff;
   for (i=0; i<lineStaffSequence.getSize(); i++) {
      lineStaffSequence.setSize(objcount[i]);
      lineStaffSequence.setGrowth(1000);
      lineStaffSequence.setSize(0);
   }

   // split out objects by staff number:
   maxStaffNumber = -1;
   for (i=0; i<thispage.getSize(); i++) {
      staffnum = thispage[i].getStaffNumber();
      if (staffnum < 0 || staffnum >= 100) {
         continue;
      }
      if (staffnum > maxStaffNumber) {
         maxStaffNumber = staffnum;
      }
      lineStaffSequence[staffnum].append(i);
   }

   // Identify which staves have any data on them and store
   // a list of these staves in pageStaffList.
   pageStaffList.setSize(100);
   pageStaffList.setSize(0);
   for (i=0; i<lineStaffSequence.getSize(); i++) {
      if (lineStaffSequence[i].getSize() > 0) {
         pageStaffList.append(i);
      }
   }
   // Create a reverse mapping of pageStaffList which can find the 
   // consecutive staff enumeration from the P2 value.  The P2 value
   // must be between 0 (but really 1 for real data) and 99.
   pageStaffListReverse.setSize(100);
   pageStaffListReverse.setAll(-1);
   for (i=0; i<pageStaffList.getSize(); i++) {
      pageStaffListReverse[pageStaffList[i]] = i;
   }

   // Sort each stave from left to right (hpos) and ties from low to high
   // (vpos), and ties by P1 (object type).
   for (i=0; i<lineStaffSequence.getSize(); i++) {
      if (lineStaffSequence[i].getSize() == 0) {
         continue;
      }
      sortByHpos(lineStaffSequence[i]);
   }

   sortAnalysisQ = 1;
}



//////////////////////////////
//
// ScorePageBase::sortByHpos -- Sort a list of data indices by their horizontal
//    positions on a line (not distinguishing between different lines).
//

void ScorePageBase::sortByHpos(Array<int>& objects) {
   ScorePageBase::quickSortByDataIndex(objects, 0, objects.getSize()-1);
}



//////////////////////////////
//
// ScorePageBase::quickSortByDataIndex -- Called by sortByHpos() and does all
//    of the actual sorting.  Uses the contents of the data array for 
//    comparison of indices.
//

void ScorePageBase::quickSortByDataIndex(Array<int>& indexes, 
      int starti, int endi) {
   if (starti < endi) {
      int pivotvalue = indexes[starti];
      int left = starti - 1;
      int right = endi + 1;
 
      for(;;) {
         while (isGreater(indexes[--right], pivotvalue));
         while (isLess(indexes[++left], pivotvalue));
 
         if (left >= right) break;
 
         int temp = indexes[right];
         indexes[right] = indexes[left];
         indexes[left] = temp;
      }

      int pivot = right;
      quickSortByDataIndex(indexes, starti, pivot);
      quickSortByDataIndex(indexes, pivot+1, endi);
   }
}



//////////////////////////////
//
// isGreater --  returns true if the hpos of first item is greater
//     than second item.
//

int ScorePageBase::isGreater(int a, int b) {
   // primary sort by hpos
   if (data[a]->getHpos() > data[b]->getHpos()) {
      return 1;
   } else if (data[a]->getHpos() < data[b]->getHpos()) {
      return 0;
   }

   // ties sorted by vpos
   if (data[a]->getVpos() > data[b]->getVpos()) {
      return 1;
   } else if (data[a]->getVpos() < data[b]->getVpos()) {
      return 0;
   }

   // ties sorted by (int)P1 value
   if ((int)data[a]->getValue(P1) > (int)data[b]->getValue(P1)) {
      return 1;
   } else if ((int)data[a]->getValue(P1) < (int)data[b]->getValue(P1)) {
      return 0;
   }

   // same hpos, same vpos, same P1, so determine that two objects are equal
   // (neither is greater than the other):
   return 0;
}



//////////////////////////////
//
// isLess --  returns true if the hpos of first item is less
//     than second item.
//

int ScorePageBase::isLess(int a, int b) {
   // primary sort by hpos
   if (data[a]->getHpos() < data[b]->getHpos()) {
      return 1;
   } else if (data[a]->getHpos() > data[b]->getHpos()) {
      return 0;
   }

   // ties sorted by vpos
   if (data[a]->getVpos() < data[b]->getVpos()) {
      return 1;
   } else if (data[a]->getVpos() > data[b]->getVpos()) {
      return 0;
   }

   // ties sorted by (int)P1 value
   if ((int)data[a]->getValue(P1) < (int)data[b]->getValue(P1)) {
      return 1;
   } else if ((int)data[a]->getValue(P1) > (int)data[b]->getValue(P1)) {
      return 0;
   }

   // same hpos, same vpos, same P1, so determine that two objects are equal
   // (neither is less than the other):
   return 0;
}


///////////////////////////////////////////////////////////////////////////



//////////////////////////////
//
// ScorePageBase::compareSystemIndexes -- sort staff number, then 
//     horizontal position, then item code, then vertical position.
//

int ScorePageBase::compareSystemIndexes(const void* A, const void* B) {
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


/*

//////////////////////////////
//
// ScorePageBase::compareStaff -- sort staff number, then horizontal position,
//     then item code, then vertical position
//

int ScorePageBase::compareStaff(const void* A, const void* B) {
   ScoreRecord& a = **((ScoreRecord**)A);
   ScoreRecord& b = **((ScoreRecord**)B);

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

*/

/*

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

*/


// md5sum: e1c6f792fbddebf040ffe0355fedb619 ScorePageBase.cpp [20050403]
