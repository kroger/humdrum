//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue May 11 21:51:09 PDT 2010
// Last Modified: Tue May 11 21:51:24 PDT 2010
// Filename:      ...sig/src/sigInfo/PDFFile.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/PDFFile.cpp
// Syntax:        C++ 
//
// Description:   Higher-level functions for processing Humdrum files.
//                Inherits PDFFileBasic and adds rhythmic and other
//                types of analyses to the PDFFile class.
//

#include "PDFFile.h"
#include "PerlRegularExpression.h"
#include <string.h>

#ifndef OLDCPP
   #include <fstream>
   #include <iostream>
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #include <fstream.h>
   #include <iostream.h>
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
// PDFFile::PDFFile --
//

PDFFile::PDFFile(void) {
   pdfdatastream = NULL;
   pdfdatasize = 0;
   rootobject = -1;
   xref.setSize(0);
   embedFile.setSize(0);
   indirectobjectoffset.setSize(0);
   indirectobjectversions.setSize(0);
}

PDFFile::PDFFile(istream& file) {
   clear();
   process(file);
}

PDFFile::PDFFile(ifstream& file) {
   clear();
   process(file);
}



//////////////////////////////
//
// PDFFile::~PDFFile
//

PDFFile::~PDFFile() {
   clear();
}



//////////////////////////////
//
// PDFFile::clear -- get rid of the contents.
//

void PDFFile::clear(void) {
   pdfdatastream = NULL;
   pdfdatasize = 0;
   rootobject = -1;
   xref.setSize(0);
   embedFile.setSize(0);
   indirectobjectoffset.setSize(0);
   indirectobjectversions.setSize(0);
}



//////////////////////////////
//
// PDFFile::getObjectCount -- return the number of indirect objects
//    in the file (one greater than higher indirect object number).
//

int PDFFile::getObjectCount(void) {
   return indirectobjectoffset.getSize();
}



//////////////////////////////
//
// PDFFile::getObjectOffset -- return the byte offset from the start 
//    of the file to the start object number for the specified object number.
//    Byte offset start at 0 for first byte in file.  First object is the
//    null object which always has an offset of 0.  Return a value of
//    -1 if there is no object at that index.
//

int PDFFile::getObjectOffset(int index) {
   if (indirectobjectoffset[index].getSize() > 0) {
      return indirectobjectoffset[index][0];
   } else {
      return -1;
   }
}



//////////////////////////////
//
// PDFFile::getObjectDictionary -- get an object's dictionary contents.
//      Return value is the byte offset from the start of the data file
//      for the end of the object's dictionary.
//

int PDFFile::getObjectDictionary(int objectindex, Array<char>& out) {
   SSTREAM tempstream;
   int returnval = getObjectDictionary(objectindex, tempstream);
   out.setSize(tempstream.str().length() + 1);
   int i;
   for (i=0; i<out.getSize()-1; i++) {
      out.getBase()[i] = tempstream.str()[i];
   }
   out[out.getSize()-1] = '\0';
   return returnval;
}


int PDFFile::getObjectDictionary(int objectindex, ostream& out) {
   istream& file = *pdfdatastream;
   int offset = getObjectOffset(objectindex);
   int indictionary = 0;
   int level  = 0;
   int slevel = 0;
   char ch;
   file.seekg(offset, ios::beg);
   while (!file.eof()) {
      file.get(ch);
      if (indictionary) {
         out << ch;
      }
      if (!slevel) {
         if (ch == '<') {
            level++;
            indictionary++;
	    out << ch;
         } else if (ch == '>') {
            level--;
         }
      } 
      if (ch == '(') {
         slevel++;
      } else if (ch == ')') {
         slevel--;
      }
      if (indictionary && (level == 0)) {
         break;
      }
   }

   return file.tellg();
}



//////////////////////////////
//
// PDFFile::getEmbeddedFileCount --
//

int PDFFile::getEmbeddedFileCount(void) {
   return embedFile.getSize();
}



//////////////////////////////
//
// PDFFile::getEmbeddedFileObject --
//

int PDFFile::getEmbeddedFileObject(int index) {
   if ((index < 0) || (index >= embedFile.getSize())) {
      return -1;
   } 
   return embedFile[index];
}



//////////////////////////////
//
// PDFFile::isEmbeddedHumdrumFile -- Returns true if the /Desc
//    entry of the file header contains the string "Embedded Humdrum File".
//

int PDFFile::isEmbeddedHumdrumFile(int index) {
   int fileheaderindex = getEmbeddedFileObject(index);
   Array<char> filedict;
   getObjectDictionary(fileheaderindex, filedict);
   PerlRegularExpression pre;
   if (pre.search(filedict.getBase(), 
      "/Desc\\s*\\(.*[Ee]mbedded\\s+[Hh]umdrum\\s+[Ff]ile.*\\)", "m")) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// PDFFile::getEmbeddedFileContents --
//

void PDFFile::getEmbeddedFileContents(int embedindex, Array<char>& out) {
   SSTREAM tempstream;
   getEmbeddedFileContents(embedindex, tempstream);
   out.setSize(tempstream.str().length()+1);
   int i;
   for (i=0; i<out.getSize()-1; i++) {
      out.getBase()[i] = tempstream.str()[i];
   }
   out[out.getSize()-1] = '\0';
}


void PDFFile::getEmbeddedFileContents(int embedindex, ostream& out) {
   int fileheaderindex = getEmbeddedFileObject(embedindex);
   Array<char> fileheaderdict;
   getObjectDictionary(fileheaderindex, fileheaderdict);
   PerlRegularExpression pre;
   int streamobject;
   if (pre.search(fileheaderdict.getBase(),
         "/EF\\s*<<[^>]*/F\\s+(\\d+)\\s+(\\d+)\\s+R", "m")) {
      streamobject = atol(pre.getSubmatch(1));
      getStreamData(streamobject, out);
   }
}



//////////////////////////////
//
// PDFFile::getStreamData --
//

void PDFFile::getStreamData(int streamobject, ostream& out) {
   Array<char> dictionary;
   int dictend = getObjectDictionary(streamobject, dictionary);
   PerlRegularExpression pre;
   int ssize = -1;
   if (pre.search(dictionary.getBase(), "/Length\\s+(\\d+)", "m")) {
      ssize = atol(pre.getSubmatch(1));
   }

   if (ssize <= 0) {
      return;
   }

   istream& file = *pdfdatastream;
   char ch;
   file.seekg(dictend+1); 
   file.get(ch);
   while ((!file.eof()) && isspace(ch)) {
      file.get(ch);
   }
   if (ch != 's') {
      cerr << "ERROR searching for start of 'stream'" << endl;
      exit(1);
   }
   file.get(ch);
   if (ch != 't') {
      cerr << "ERROR searching for 't' of 'stream'" << endl;
      exit(1);
   }
   file.get(ch);
   if (ch != 'r') {
      cerr << "ERROR searching for 'r' of 'stream'" << endl;
      exit(1);
   }
   file.get(ch);
   if (ch != 'e') {
      cerr << "ERROR searching for 'e' of 'stream'" << endl;
      exit(1);
   }
   file.get(ch);
   if (ch != 'a') {
      cerr << "ERROR searching for 'a' of 'stream'" << endl;
      exit(1);
   }
   file.get(ch);
   if (ch != 'm') {
      cerr << "ERROR searching for 'm' of 'stream'" << endl;
      exit(1);
   }

   while ((!file.eof()) && (ch != 0x0a)) {
      file.get(ch);
   }

   int i;
   for (i=0; i<ssize; i++) {
      file.get(ch);
      if (file.eof()) {
         break;
      }
      out << ch;
   }
}



//////////////////////////////
//
// PDFFile::getRootIndex -- return the indirect object number of the 
//   Root.
//

int PDFFile::getRootIndex(void) {
   return rootobject;
}



//////////////////////////////
//
// PDFFile::getFileSize -- return the number of bytes in the PDF file.
//

int PDFFile::getFileSize(void) {
   return pdfdatasize;
}



//////////////////////////////
//
// PDFFile::getXrefCount --
//

int PDFFile::getXrefCount(void) {
   return xref.getSize();
}



//////////////////////////////
//
// PDFFile::getXrefOffset --
//

int PDFFile::getXrefOffset(int index) {
   if ((index < 0) || (index >= xref.getSize())) {
      return -1;
   }
   return xref[index];
}



//////////////////////////////
//
// PDFFile::process -- link to an ifstream and build indirect object
//     table.
//

void PDFFile::process(ifstream& file) {
   clear();
   pdfdatastream = &file;
   initializeXrefTable();
   makeEmbeddedFileList();
}

void PDFFile::process(istream& file) {
   clear();
   pdfdatastream = &file;
   initializeXrefTable();
   makeEmbeddedFileList();
}



//////////////////////////////
//
// PDFFile::print -- Print entire content of PFD file to data stream.
//

void PDFFile::print(ostream& out) {
   istream& file = *pdfdatastream;

   char chariot;
   if (file.eof()) {
      file.clear();
   }
   file.seekg(0, ios::beg);
   file.get(chariot);
   while (!file.eof()) {
      cout << chariot;
      file.get(chariot);
   }

   if (file.eof()) {
      file.clear();
   }
   file.seekg(0, ios::beg);
}



//////////////////////////////
//
// PDFFile::getTrailerString --
//

void PDFFile::getTrailerString(int index, Array<char>& tstring) {
   istream& file = *pdfdatastream;

   tstring.setSize(1);
   tstring[0] = '\0';

   int xrefoffset = getXrefOffset(index); 
   if (xrefoffset <= 0) {
      cerr << "ERROR in getTrailerString: xrefoffset " << xrefoffset
           << " is too small." << endl;
      exit(1);
   }

   SSTREAM trailer;
   int filesize = getFileSize();
   char ch;

   int startindex = -1;
   int i;
   for (i=xrefoffset; i<filesize; i++) {
      startindex = -1;
      file.seekg(i, ios::beg);
      file.get(ch);
      if (ch != 't') { continue; } else { startindex = i; }
      i++; file.get(ch); if (ch != 'r') { continue; }
      i++; file.get(ch); if (ch != 'a') { continue; }
      i++; file.get(ch); if (ch != 'i') { continue; }
      i++; file.get(ch); if (ch != 'l') { continue; }
      i++; file.get(ch); if (ch != 'e') { continue; }
      i++; file.get(ch); if (ch != 'r') { continue; }
      break;
   }

   if (startindex <= 5) {
      cerr << "ERROR: Could not find the start of the trailer" << endl;
      exit(1);
   }

   int level = 0;
   int indictionary = 0;
   int slevel = 0;

   file.seekg(startindex, ios::beg);
   while (!file.eof()) {
      file.get(ch);
      trailer << ch;
      if (!slevel) {
         if (ch == '<') {
            level++;
            indictionary++;
         } else if (ch == '>') {
            level--;
         }
      } 
      if (ch == '(') {
         slevel++;
      } else if (ch == ')') {
         slevel--;
      }
      if (indictionary && (level == 0)) {
         break;
      }
   }


   tstring.setSize(trailer.str().length()+1);
   for (i=0; i<tstring.getSize()-1; i++) {
      tstring.getBase()[i] = trailer.str()[i];
   }
   tstring[tstring.getSize()-1] = '\0';

}



///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// PDFFile::getXrefOffset -- return the offset of the xref referred 
//      to in the trailer.
//

int PDFFile::getXrefOffset(istream& file) {
   if (file.eof()) {
      file.clear();
   }

   int output = -1;
   file.seekg(0, ios::end);
   int filesize = (int)file.tellg();
   int i = filesize - 4;  // skip over some of %%EOF string
   char ch;
   file.seekg(i, ios::beg);
   file.get(ch);
   while ((i > 0) && (ch != '%')) {
      i--;
      file.seekg(i, ios::beg);
      file.get(ch);
   }
   if (i <= 0) {
      cerr << "ERROR extracting xref offset in file " << endl;
      exit(1);
   }
   file.seekg(i-1, ios::beg);
   file.get(ch);
   if (ch != '%') {
      cerr << "ERROR1 extracting xref offset in file " << endl;
      cerr << "Expected %, but found " << ch << endl;
      exit(1);
   }
   file.seekg(i+1, ios::beg);
   file.get(ch);
   if (ch != 'E') {
      cerr << "ERROR2 extracting xref offset in file " << endl;
      cerr << "Expected E, but found " << ch << endl;
      exit(1);
   }
   file.seekg(i+2, ios::beg);
   file.get(ch);
   if (ch != 'O') {
      cerr << "ERROR3 extracting xref offset in file " << endl;
      cerr << "Expected O, but found " << ch << endl;
      exit(1);
   }
   file.seekg(i+3, ios::beg);
   file.get(ch);
   if (ch != 'F') {
      cerr << "ERROR4 extracting xref offset in file " << endl;
      cerr << "Expected F, but found " << ch << endl;
      exit(1);
   }

   int bindex = 100;
   char buffer[128] = {0};

   i = i-2;
   file.seekg(i, ios::beg);
   file.get(ch);
   while ((i>0) && (!isdigit(ch))) {
      i--;
      file.seekg(i, ios::beg);
      file.get(ch);
   }
   if (!isdigit(ch)) {
      cerr << "ERROR5 extracting xref offset in file " << endl;
      cerr << "Expected digit, but found " << ch << endl;
      exit(1);
   }
   buffer[bindex--] = ch;
   i--;
   file.seekg(i, ios::beg);
   file.get(ch);
   while ((bindex > 0) && (i>0) && (isdigit(ch))) {
      buffer[bindex--] = ch;
      i--;
      file.seekg(i, ios::beg);
      file.get(ch);
   }
   int status = sscanf(&buffer[bindex+1], "%d", &output);
   if (status != 1) {
      cerr << "ERROR6 reading number from string" << endl;
      cerr << "string is " << &buffer[bindex+1] << endl;
      exit(1);
   }

   return output;
}



//////////////////////////////
//
// PDFFile::initializeXrefTable --
//

void PDFFile::initializeXrefTable(void) {
   if (pdfdatastream == NULL) {
      return;
   }
   istream& file = *pdfdatastream;
   int xrefoffset = getXrefOffset(*pdfdatastream);

   xref.setSize(100);
   xref.setGrowth(1000);
   xref.setSize(0);
   xref.append(xrefoffset);

   file.seekg(0, ios::end);
   pdfdatasize = file.tellg();

   int& filesize = pdfdatasize;

   SSTREAM originaltrailer;
   getOriginalTrailer(originaltrailer, filesize, xrefoffset, file);
   originaltrailer << ends;
   Array<char> trailerstring;
   trailerstring.setSize(originaltrailer.str().length()+1);
   strcpy(trailerstring.getBase(), originaltrailer.CSTRING);

   int nextobject = -1;
   PerlRegularExpression pre;
   pre.search(originaltrailer.CSTRING, "/Size\\s+(\\d+)", "");
   if (!sscanf(pre.getSubmatch(1), "%d", &nextobject)) {
      cerr << "ERROR: cannot find object count in trailer" << endl;
      exit(1);
   }

   pre.search(originaltrailer.CSTRING, "/Root\\s+(\\d+)", "");
   if (!sscanf(pre.getSubmatch(1), "%d", &rootobject)) {
      cerr << "ERROR: cannot find root object number in trailer" << endl;
      exit(1);
   }

   if (nextobject < 0) {
      return;
   }
   
   indirectobjectoffset.setSize(nextobject);
   indirectobjectversions.setSize(nextobject);
   
   indirectobjectoffset.allowGrowth(0);
   indirectobjectversions.allowGrowth(0);
   int i;
   for (i=0; i<indirectobjectoffset.getSize(); i++) {
      indirectobjectoffset[i].setSize(0);
      indirectobjectversions[i].setSize(0);
   }

   char ch;
   file.seekg(xrefoffset, ios::beg);
   file.get(ch);
   if (ch != 'x') {
      cerr << "ERROR: expected 'x' from xref, but found: " << ch << endl;
      cerr << "int value is: " << (int)ch << endl;
      cerr << "Position in file: " << file.tellg() << endl;
      cerr << "xref offset is: " << xrefoffset << endl;
      exit(1);
   }
   file.get(ch);
   if (ch != 'r') {
      cerr << "ERROR: expected 'r' from xref, but found: " << ch << endl;
      exit(1);
   }
   file.get(ch);
   if (ch != 'e') {
      cerr << "ERROR: expected 'e' from xref, but found: " << ch << endl;
      exit(1);
   }
   file.get(ch);
   if (ch != 'f') {
      cerr << "ERROR: expected 'f' from xref, but found: " << ch << endl;
      exit(1);
   }
   
   while (!file.eof()) {
      file.get(ch);
      while (!file.eof() && !isdigit(ch) && (ch != 't')) {
         file.get(ch);
      }
      if (file.eof() || (ch == 't')) {
         break;
      } else {
         processXrefList(file, ch);
      }
   }
   
   if (file.eof()) {
      file.clear();
      file.seekg(0, ios::beg);
      return;
   }

   if (ch != 't') {
      cerr << "ERROR: expecting trailer after xref, but find: " << ch << endl;
      exit(1);
   }

   // Now search the trailer for an entry of the form:
   //   /Prev 144425
   // This is a byte offset of the previous xref entry, so call this
   // function again with that offset value.  If no /Prev entry is
   // found, before "%%EOF" string is found, then return to calling function.

   int previous = getPreviousOffset(file);
   if (previous >= 0) {
      buildIndirectObjectIndex(file, previous);
   } else{
      return;
   }
}



//////////////////////////////
//
// PDFFile::processXrefList -- reads a byte offset listing which starts
//   with two numbers: the first object in the following list,
//   then the count of the objects listed.  Following these two numbers is
//   a list of offsets and versions (usually 0) for each indirect object
//   sequentially enumerated starting at the count value of the second number.
//
// Example:
//    xref
//    0 1
//    0000000000 65535 f
//    12 2
//    0000003054 00000 n
//    0000003206 00000 n
//    
// The entry "0 1" means that the first entry on the following line is
// for the indirect object "0", and there is one line.  The entry "12 2"
// means that the next line specifies the offset data for indirect object 12,
// and there are two lines total (the last one being for indirect object 13).
//
// Each line is exactly 20 bytes line, there are two newline characters
// at the end of each line, after the "f" or "n": "0x0d, 0x0a".  0x0d could
// be something else.  If so that character should be ignored. "f" means
// that the object is "free" (indirect object has been deleted from file), 
// "n" means that it is in use.
//

void PDFFile::processXrefList(istream& file, char firstdigit) {

   if (file.eof()) {
      file.clear();
      return;
   }

   char ch;
   int startindex = firstdigit - '0';
   file.get(ch);
   while (!file.eof() && isdigit(ch)) {
      startindex *= 10;
      startindex += ch - '0';
      file.get(ch);
   }

   // now read the count of objects;
   // previous character should be a space and is not a digit.
   int objectcount = 0;
   file.get(ch);
   while (!file.eof() && isdigit(ch)) {
      objectcount *= 10;
      objectcount += ch - '0';
      file.get(ch);
   }

   int currentobject = startindex;
   char buffer[128] = {0};
   int offset = 0;
   int tempval = 0;

   // now read the list of objects:
   int i;
   while (!file.eof() && (isspace(ch))) {
      file.get(ch);
   }
   if (ch != '0') {
      cerr << "ERROR: funny error reading xref entry: " << ch << endl;
      cerr << "hex value of character: 0x" << hex << (int)ch << dec << endl;
      exit(1);
   }
   tempval = file.tellg();
   int version;
   tempval--;
   file.seekg(tempval, ios::beg);
   for (i=0; i<objectcount; i++) {
      file.read(buffer, 20);
      buffer[20] = '\0';
      int j;
      for (j=0; j<9; j++) {
         if (buffer[j] != '0') {
            break;
         }
      }
      offset = 0;
      offset = atol(&buffer[j]);

      for (j=11; j<15; j++) {
         if (buffer[j] != '0') {
            break;
         }
      }
      version = 0;
      version = atol(&buffer[j]);
      
      if (currentobject >= indirectobjectoffset.getSize()) {
         cerr << "Error in object count in xref entry." << endl;         
	 cerr << "Index: " << currentobject << endl;
	 cerr << "Max Index " << indirectobjectoffset.getSize()-1 << endl;
	 exit(1);
         // This might be useful to do instead:
         // expandObjectListing(currentobject+1);
      }

      indirectobjectoffset[currentobject].append(offset);
      indirectobjectversions[currentobject].append(version);
      
      currentobject++;
   }
}



//////////////////////////////
//
// PDFFile::expandObjectListing --
//

void PDFFile::expandObjectListing(int newsize) {
   int oldsize = indirectobjectoffset.getSize();

   if (newsize <= oldsize) {
      return;
   }

   indirectobjectoffset.setSize(newsize);
   indirectobjectversions.setSize(newsize);
   int i;
   for (i=oldsize; i<newsize; i++) {
      indirectobjectoffset[i].setSize(0);
      indirectobjectversions[i].setSize(0);
   }
   
}



//////////////////////////////
//
// PDFFile::getPreviousOffset -- search through a trailer until "%%EOF" is
//   found, looking for a /Prev entry.  If there is a /Prev entry, then return
//   the number following it.  The file is set to the first 'r' in "trailer"
//   when this function is called.
//

int PDFFile::getPreviousOffset(istream& file) {

   char ch;
   if (file.eof()) {
      cerr << "ERROR: file EOF while starting /Prev search" << endl;
      exit(1);
   }
   file.get(ch);
   if (ch != 'r') {
      cerr << "ERROR: unexpected character reading trailer: " << ch << endl;
      exit(1);
   }

   int level = 0;
   int eofindex = 0;
   int previndex = 0;
   char eofstate[128] = {0};
   char prevstate[128] = {0};
   strcpy(eofstate, "%%EOF");
   strcpy(prevstate, "/Prev");
   int prevtarget = strlen(prevstate);
   int eoftarget = strlen(eofstate);

   file.get(ch);
   while (!file.eof()) {
      if (ch == '<') {
         level++;
      } else if (ch == '>') {
         level--;
      }
      if (level == 2) {
         if (prevstate[previndex] == ch) {
            previndex++;
         } else {
            previndex = 0;
         }
         if (previndex == prevtarget) {
            break;
         }
      } else if (level <= 0) {
         if (eofstate[eofindex] == ch) {
            eofindex++;
         } else {
            eofindex = 0;
         }
         if (eofindex == eoftarget) {
            // found %%EOF, but did not find /Prev entry
            return -1;
         }
      }
      file.get(ch);
   }

   if (previndex != prevtarget) {
      cerr << "ERROR: strange thing happened in getPreviousOffset()" << endl;
      exit(1);
   }

   // skip any non-digit characters:
   file.get(ch);
   while (!file.eof() && !isdigit(ch)) {
      if (!isspace(ch)) {
         cerr << "ERROR: found a strange character after /Prev in trailer: " 
              << ch << endl;
      }
      file.get(ch);
   }
   if (file.eof()) {
      return -1;
   }
   int value = ch - '0';
   file.get(ch);
   while (!file.eof() && isdigit(ch)) {
      value *= 10;
      value += ch - '0';
      file.get(ch);
   }

   return value;
}



//////////////////////////////
//
// PDFFile::buildIndirectObjectIndex -- make a list of all indirect objects
//   found in the file (listed in the xref section(s)).  
//

void PDFFile::buildIndirectObjectIndex(istream& file, int xrefoffset) {

   xref.append(xrefoffset);

   char ch;
   file.seekg(xrefoffset, ios::beg);
   file.get(ch);
   if (ch != 'x') {
      cerr << "ERROR: expected 'x' from xref, but found: " << ch << endl;
      cerr << "int value is: " << (int)ch << endl;
      cerr << "Position in file: " << file.tellg() << endl;
      cerr << "xref offset is: " << xrefoffset << endl;
      exit(1);
   }
   file.get(ch);
   if (ch != 'r') {
      cerr << "ERROR: expected 'r' from xref, but found: " << ch << endl;
      exit(1);
   }
   file.get(ch);
   if (ch != 'e') {
      cerr << "ERROR: expected 'e' from xref, but found: " << ch << endl;
      exit(1);
   }
   file.get(ch);
   if (ch != 'f') {
      cerr << "ERROR: expected 'f' from xref, but found: " << ch << endl;
      exit(1);
   }
   
   while (!file.eof()) {
      file.get(ch);
      while (!file.eof() && !isdigit(ch) && (ch != 't')) {
         file.get(ch);
      }
      if (file.eof() || (ch == 't')) {
         break;
      } else {
         processXrefList(file, ch);
      }
   }
   
   if (file.eof()) {
      return;
   }

   if (ch != 't') {
      cerr << "ERROR: expecting trailer after xref, but find: " << ch << endl;
      exit(1);
   }

   // Now search the trailer for an entry of the form:
   //   /Prev 144425
   // This is a byte offset of the previous xref entry, so call this
   // function again with that offset value.  If no /Prev entry is
   // found, before "%%EOF" string is found, then return to calling function.

   int previous = getPreviousOffset(file);
   if (previous >= 0) {
      buildIndirectObjectIndex(file, previous);
   } else{
      return;
   }
}



//////////////////////////////
//
// PDFFile::getOriginalTrailer --
//

void PDFFile::getOriginalTrailer(SSTREAM& trailer, int filesize, int xref, 
      istream& file) {
   int i = filesize - 5;
   int startindex = -1;
   char ch;

   for (i=xref; i<filesize; i++) {
      startindex = -1;
      file.seekg(i, ios::beg);
      file.get(ch);
      if (ch != 't') { continue; } else { startindex = i; }
      i++; file.get(ch); if (ch != 'r') { continue; }
      i++; file.get(ch); if (ch != 'a') { continue; }
      i++; file.get(ch); if (ch != 'i') { continue; }
      i++; file.get(ch); if (ch != 'l') { continue; }
      i++; file.get(ch); if (ch != 'e') { continue; }
      i++; file.get(ch); if (ch != 'r') { continue; }
      break;
   }

   if (startindex <= 5) {
      cerr << "ERROR: Could not find the start of the trailer" << endl;
      exit(1);
   }
	       
   int level = 0;
   int indictionary = 0;
   int slevel = 0;

   file.seekg(startindex, ios::beg);
   while (!file.eof()) {
      file.get(ch);
      trailer << ch;
      if (!slevel) {
         if (ch == '<') {
            level++;
            indictionary++;
         } else if (ch == '>') {
            level--;
         }
      } 
      if (ch == '(') {
         slevel++;
      } else if (ch == ')') {
         slevel--;
      }
      if (indictionary && (level == 0)) {
         break;
      }
   }
}



//////////////////////////////
//
// PDFFile::makeEmbeddedFileList --  Search the Root entry for a
//    /Names entry.  If it does not have one, then there are no
//    embedded files.  If it does have a /Names entry, follow to
//    the Names dictionary and search for an entry in the dictionary
//    called /EmbeddedFiles.  If there is none, then there are no
//    embedded files.  If there is, then follow the reference to
//    a list of object pointers to the embedded files which is a 
//    value in the /Names entry of the pointed object from the
//    Names dictionary.
//

void PDFFile::makeEmbeddedFileList(void) {
   embedFile.setSize(1000);
   embedFile.setSize(0);
   embedFile.setGrowth(1000);

   int rootindex = getRootIndex();
   Array<char> rootdictionary;
   getObjectDictionary(rootindex, rootdictionary);
   PerlRegularExpression pre;
   if (!pre.search(rootdictionary.getBase(), 
         "/Names\\s+(\\d+)\\s+(\\d+)\\s+R", "")) {
      return;
   }
   int ndindex = atol(pre.getSubmatch(1));
			    
   Array<char> namedict;
   getObjectDictionary(ndindex, namedict);
   if (!pre.search(namedict.getBase(), 
         "/EmbeddedFiles\\s+(\\d+)\\s+(\\d+)\\s+R", "i")) {
      // allowing case insensitive search to allow for hidden
      // embedded files, you are not allowed to mix hidden and
      // unhidden sections at the present...
      return;
   }

   int embedlistindex = atol(pre.getSubmatch(1));
   Array<char> embedlisting;
   getObjectDictionary(embedlistindex, embedlisting);
   // embedlisting contains null bytes, so problematic to use
   // with regular expressions...
   int i; 

   int slevel = 0;
   for (i=0; i<embedlisting.getSize(); i++) {
      if (embedlisting[i] == '(') {
         slevel++;
      } else if (embedlisting[i] == ')') {
         slevel--;
      }
      if (slevel) {
         continue;
      }
      if (embedlisting[i] == '/') {
         if (pre.search(&embedlisting[i], "^/Names", "")) {
            break;
         }
      }
   }
   if (embedlisting[i] != '/') {
      return;
   }

   int rlevel = 0;
   slevel = 0;
   int value;
   for (i=i+strlen("Names"); i<embedlisting.getSize(); i++) {
      if (embedlisting[i] == '(') {
         slevel++;
      } else if (embedlisting[i] == ')') {
         slevel--;
         if ((rlevel == 1) && (slevel == 0)) {
            if (pre.search(&embedlisting[i+1], 
                  "^\\s*(\\d+)\\s+(\\d+)\\s+R", "")) {
               value = atol(pre.getSubmatch(1));
               embedFile.append(value);
            }
         }
      }
      if (slevel) {
         continue;
      }
      if (embedlisting[i] == '[') {
         rlevel++;
      } else if (embedlisting[i] == ']') {
         rlevel--;
         if (rlevel == 0) {
            break;
         }
      }
   }
}



// md5sum: 3997e8805220a29acf9afd717bf8dd20 PDFFile.cpp [20001204]
