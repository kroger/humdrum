//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May  3 21:54:58 PDT 2010
// Last Modified: Thu May  6 23:16:19 PDT 2010
// Last Modified  Mon Feb  7 17:55:04 PST 2011 (fixed md5sum calculation)
// Filename:      ...sig/examples/all/make64.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/humpdf.cpp
// Syntax:        C++; museinfo
//
// Description:   Embed a Humdrum file into a PDF file as an attachment.
//
// Links: 
// PDF 1.7 reference (ISO 32000.1 2008):
//    http://www.adobe.com/devnet/acrobat/pdfs/PDF32000_2008.pdf
//    http://blogs.adobe.com/pdfdevjunkie/PDF_Inside_and_Out.pdf
// PDF 1.4 reference:
//    http://www.adobe.com/devnet/pdf/pdfs/PDFReference.pdf
//        section 3.10.3 Embedded File Streams, page 123
//

#include "humdrum.h"
#include "PerlRegularExpression.h"
#include "PDFFile.h"
#include "CheckSum.h"

#ifndef OLDCPP
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   #include <iostream>
   #include <fstream>
   #include <string>
   #include <cassert>
   #include <cstring>
   using namespace std;
#else
   #ifdef VISUAL
      #include <strstrea.h>     /* for windows 95 */
   #else
      #include <strstream.h>
   #endif
   #include <iostream.h>
   #include <fstream.h>
   #include <string.h>
   #include <cassert.h>
   #include <cstring.h>
   #define SSTREAM strstream
   #define CSTRING str()
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

   
void      checkOptions        (Options& opts, int argc, char* argv[]);
void      example             (void);
void      usage               (const char* command);
void      printMimeEncoding   (ostream& out, int count, char char1, char char2, 
                               char char3);
void      createStreamData    (ostream& out, SSTREAM& datatoencode,
                               const char* filename);
int       printStreamObject   (ostream& out, int objnum, SSTREAM& datatoencode, 
                               const char* filename, Array<int>& objectindex,
                               Array<int>& offsetindex, int initialoffset);
int       createFileEntry     (SSTREAM& out, HumdrumFile& infile, 
                               const char* filename, int nextobject,
                               Array<int>& objectindex, 
                               Array<int>& offsetindex, int initialoffset);
int       generateNewXref     (SSTREAM& out, Array<int>& objectindex, 
                               Array<int>& offsetindex, int filesize);
void      printPdfDate        (ostream& out, struct tm* date);
void      addTrailerPrev      (Array<char>& trailerstring, int newprevoffset);
int       linkToRootObject    (ostream& out, Array<int>& objectindex, 
                               Array<int>& offsetindex, int initialoffset, 
                               Array<char> trailerstring, int xrefoffset, 
                               istream& file, int nextobject,
                               PDFFile& pdffile);
void      getObject           (ostream& out, istream& file, int offset);
int       updateNamesObject   (ostream& out, Array<int>& objectindex, 
                               Array<int>& offsetindex, int initialoffset, 
                               ostream& file, int nextobject, int ndoffset);
int       updateRootObject    (ostream& out, int rootobjnum, int initialoffset, 
                               PDFFile& pdffile, Array<char>& rootstring, 
                               Array<int>& objectindex, Array<int>& offsetindex,
                               int embedcount, int nextobject);
void      addDictionaryEntry   (Array<char>& objectstring, Array<char>& entry);
int       getSequentialObjectCount(Array<int>& list, int starti);

// global variables:
Options     options;
const char* pdffilename = "";  // used with -p option
int         footerQ  = 0;      // used with -A option
int         keepdirQ = 0;      // used with -D option
int         hiddenQ  = 0;      // used with --hidden option (not active)
int         debugQ   = 0;      // used with --debug option
int         prefixQ  = 0;      // used with -P option
const char* prefix   = "";     // used with -P option


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);

   istream *file;
   ifstream filestream;
   if (strcmp(pdffilename, "") == 0) {
      // read standard input if no -p option given
      file = &cin;
   } else {
      filestream.open(pdffilename, ios::in | ios::binary);
      file = &filestream;
      if (!filestream.is_open()) {
         cerr << "ERROR: cannot open file: " << pdffilename << "\n";
         exit(1);
      }
   }

   int i;
   int initialoffset = 0;
   HumdrumFile infile;
   PDFFile     pdffile;

   pdffile.process(*file); // reads structural information from PDF file.
                          // A pointer to ifstream file is stored in
			  // pdffile, so don't close file while still
			  // extracting data using pdffile.

   int filesize   = pdffile.getFileSize();
   int xrefoffset = pdffile.getXrefOffset(0);
   int nextobject = pdffile.getObjectCount();

   if (xrefoffset <= 5) {
      cerr << "ERROR: no xref offset found in file " << pdffilename << endl;
      exit(1);
   }

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   Array<SSTREAM*> filesegments;  // temporary storage for embedded files
   if (numinputs <= 0) {
      filesegments.setSize(1);
   } else {
      filesegments.setSize(numinputs);
   }
   filesegments.allowGrowth(0);
   for (i=0; i<filesegments.getSize(); i++) {
      filesegments[i] = new SSTREAM;
   }

   Array<int> objectindex(1000);
   Array<int> offsetindex(1000);
   objectindex.setGrowth(10000);
   offsetindex.setGrowth(10000);
   objectindex.setSize(0);
   offsetindex.setSize(0);

   const char* filename = "";
   int fcounter = 0;
   for (i=0; i<numinputs; i++) {
      infile.clear();
      filename = options.getArg(i+1);
      infile.read(filename);
      nextobject = createFileEntry(*filesegments[fcounter], infile, 
            filename, nextobject, objectindex, offsetindex, initialoffset);
      initialoffset += filesegments[fcounter]->str().length();
      fcounter++;
   }

   if (!footerQ) {
      // print initial contents of input PDF
      pdffile.print(cout);
      cout << flush;
   }

   for (i=0; i<filesegments.getSize(); i++) {
      cout << filesegments[i]->str() << flush;
   }

   Array<char> trailerstring;
   pdffile.getTrailerString(0, trailerstring);

   if (!hiddenQ) {
      // If "hidden", the file will disappear if Save As... is used to save,
      // because the document root does not know about it.
      SSTREAM rootlink;
      nextobject = linkToRootObject(rootlink, objectindex, offsetindex, 
		         initialoffset, trailerstring, xrefoffset, *file, 
			 nextobject, pdffile);
      cout << rootlink.str() << flush;
      initialoffset += rootlink.str().length();
   }

   // update the object count (/Size) in the trailer:
   char replacement[128] = {0};
   sprintf(replacement, "/Size %d", nextobject);
   PerlRegularExpression pre;
   pre.sar(trailerstring, "\\/Size\\s+\\d+", replacement);

   // remove any /Prev entry in trailer, and replace with 
   // new one
   addTrailerPrev(trailerstring, xrefoffset);

   SSTREAM xrefstream;

   int newxrefoffset = generateNewXref(xrefstream, objectindex, 
		   offsetindex, filesize);
   newxrefoffset += initialoffset;
   cout << xrefstream.str();
   cout << trailerstring.getBase() << endl;
   cout << "startxref\n";
   // byte location of new xref goes here:
   cout << newxrefoffset + filesize << endl;
   cout << "%%EOF" << endl;

   for (i=0; i<filesegments.getSize(); i++) {
      delete filesegments[i];
      filesegments[i] = NULL;
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// linkToRootObject -- 
//
//  Link embedded files to to root entry of PDF.  If the embedded 
//  file is not linked from the Root entry, then it looks like it 
//  is removed automatically when "Save As..." is used to save the file.
//  
//  Basic algorithm:
//
//  (1) Locate the Root object (found from the Trailer /Root dictionary entry).
//  (2) Identify any previous /Names entry in Root's dictionary
//      such as:  
//         /Names 261 0 R
//  (4) If no /Names entry, then add one to the Root object, and create entry.
//  (3) If Root has a /Names entry, then update the indirect object 
//       (and probably don't need to update Root entry).
//  (5) To create a /Names indirect object entry:
//  (6) In the /Names indirect object listed in the Root, add a line such as:
//             /EmbeddedFiles 400 0 R       
//         [Note: have to figure out how to deal with multiple embedded files.]
//         where 400 0 R is an indirect object for for an object which 
//	 contains a /Names entry (name of object internally):
//	 <<
//	    /Names [(_^@U^@n^@t^@i^@t^@l^@e^@d^@ ^@O^@b^@j^@e^@c^@t) 398 0 R]
//         >>
//	 The value for /Names entry is an array of two items:
//	    (A) the name of the object in UTF-16, usually called
//	        "_Untitled Object"
//            (B) An indirect object reference point to an object which
//                contains the /Type /FileSpec in the dictionary
//                (which contains the true filesystem's filename, and
//                 an indirect object reference to a stream object which
//                 contains the actual contents of a file.
//
// General Map of attaching the embedded file to the Root object:
//   Root object --> Names Dictionary --> Embedded List --> Embedded File
//        Specification --> Embedded File Stream
//
// * Root object has a /Names entry in its dictionary which points to 
//     an indirect object that gives the Names dictionary of the Root object.
//     (create a /Names entry if it does not already exist and update
//      the Root object in the PDF file; otherwise, leave the original
//      Root entry unchanged, and go to the Name Dictionary object for
//      further processing. Example Root object:
//      1 0 obj<</Type/Catalog/Pages 3 0 R/Metadata 11 0 R/Names 14 0 R>> endobj
//      or without a /Names entry in the dictionary:
//      1 0 obj<</Type/Catalog/Pages 3 0 R/Metadata 11 0 R>> endobj
// * Names Dictionary should have a dictionary entry called /EmbeddedFiles
//     which points to an indirect object which will list the embedded files.
//     Example Names Dictionary:
//        14 0 obj << /EmbeddedFiles 15 0 R >>
//     This entry says that the list of embedded files is found in indirect
//     object 15.  If the Names Dictionary does not contain an /EmbeddedFiles
//     entry, then update the object to add one.
// * Embedded List: A dictionary with a /Names entry which contains an
//     array of pairs of entries which list the embedded files.  The first
//     of the pair is a Unicode-16 string giving the embedded name of the
//     data (not the filename, and not really used for anything that I can
//     figure out.  The typical name is "_Untitled Object".  The second
//     value of the pair is a reference to an indirect object.  In the example
//     below, the indirect object is #13.  This is a link to the /FileSpec
//     entry for the embedded file.
//      15 0 obj
//      << /Names [(_^@U^@n^@t^@i^@t^@l^@e^@d^@ ^@O^@b^@j^@e^@c^@t) 13 0 R] >>
//      endobj
// * Embedded File Specification:
//         13 0 obj
//         << /Type /Filespec
//            /F    (file.krn)
//            /EF   << /F 12 0 R >>
//            /Desc (Short Description of File)
//         >>
//         endobj
//      The Embedded file specification lists the name of the file in the /F
//      entry, the embedded content stream is listed as an indirect object
//      in the /F entry in the dictionary of the /EF entry (in this case
//      object #12).
// * Embedded File Stream:
//      Contains the actual contents of the embedded file plus some
//      file content information:
//      
//      12 0 obj
//      <<
//         /Type    /EmbeddedFile
//         /SubType /application#2fx-humdrum
//         /Length  34
//         /Params
//         <<
//            /CreationDate (D:20100510042439-08'00')
//            /ModDate      (D:20100510042439-08'00')
//            /EmbedDate    (D:19991214040506-08'00')
//            /Size         34
//            /CheckSum     <e3740828edc5d78e85765c9daf1a0>
//         >>
//      >>
//      stream
//      **kern
//      *M4/4
//      *k[]
//      *c:
//      =-
//      1c
//      ==
//      *-
//      endstream
//      endobj
//
//     The /Length field is required and gives the number of bytes
//     between the string "stream\n" and endstream.  "stream" should
//     have the newline 0x0a or "0x0d 0x0a" after it (but not 0x0d alone).  
//     An optional newline before "endstream" is allowed, and will not be 
//     considered part of the data inside of the stream.
// 
//

int linkToRootObject(ostream& out, Array<int>& objectindex, 
      Array<int>& offsetindex, int initialoffset, Array<char> trailerstring, 
      int xrefoffset, istream& file, int nextobject, PDFFile& pdffile) { 

   // when this function is called, only embedded files have been
   // added to the PDF.  There are two indirect objects for each
   // embedded file (the /FileSpec entry and the actual contents,
   // So embedcount is the number of files which have been included:
   int embedcount = objectindex.getSize() / 2;

   int rootindex  = pdffile.getRootIndex();
   int rootoffset = pdffile.getObjectOffset(rootindex);

   //// Now go to root object and check to see if there is a /Names entry

   int i;
   if (debugQ) {
      cerr << ">>> Indirect object byte offset table:" << endl;
      for (i=0; i<pdffile.getObjectCount(); i++) {
         cerr << ">>> " << i << ":\t" << pdffile.getObjectOffset(i) << endl;
      }
   }

   SSTREAM rootstream;

   getObject(rootstream, file, rootoffset);

   Array<char> rootstring;
   rootstring.setSize(rootstream.str().length()+1);
   for (i=0; i<(int)rootstream.str().length(); i++) {
      rootstring[i] = rootstream.str()[i];
   }
   rootstring[rootstring.getSize()-1] = '\0';

   //// if there is a /Names entry in dictionary, then don't bother updating
   //// the Root entry and instead go directly to the /Names object and modify.
   //// If there is not a /Names entry, then add one as indirect object 
   //// and also insert a revised Root object.

   PerlRegularExpression pre;
   if (pre.search(rootstring.getBase(), "/Names\\s+(\\d+)\\s+(\\d)\\s+R", "")) {
      // int namesobj = atol(pre.getSubmatch(1));
      // int namesver = atol(pre.getSubmatch(2));
      // ggg
      // nextobject = updateNamesObject(out, objectindex, offsetindex, 
      //      initialoffset, file, nextobject, objectoffsets[namesobj]);
   } else {
      // update Root dictionary to add /Names entry, create Names dictionary
      // and create the list of Embedded files in another object.
      SSTREAM newroot;
      nextobject = updateRootObject(newroot, rootindex, initialoffset, pdffile,
         rootstring, objectindex, offsetindex, embedcount, nextobject);
      initialoffset += newroot.str().length();
      out << newroot.str() << flush;
   }

   return nextobject;
}



//////////////////////////////
//
// updateRootObject --
//   newroot        == Output data stream for revised root object
//   rootobjnum     == Root indirect object number (second value presumed 0)
//   initialoffset  == Byte offset from the start of the file to start
//                     of newroot stream.
//   pdffile        == Storage for byte offset data.
//   rootstring     == The original Root object entry which needs to have
//                     an added name dictionary, etc.
//   objectindex    == List of indirect objects created by this program
//                     (index in order of creation).
//   offsetindex    == List of byte offsets for objects created by this
//                     program.  (index in order of creation).
//   embedcount     == Number of embedded files added by program.
//

int updateRootObject(ostream& out, int rootobjnum, int initialoffset, 
      PDFFile& pdffile, Array<char>& rootstring, Array<int>& objectindex, 
      Array<int>& offsetindex, int embedcount, int nextobject) {

   SSTREAM newroot;
   newroot << "\n";
   // objectoffsets[rootobjnum] = initialoffset + newroot.str().length();
   objectindex.append(rootobjnum);
   int tval = newroot.str().length() + initialoffset;
   offsetindex.append(tval);

   int ndobjectnumber = pdffile.getObjectCount() + embedcount*2;
   int ndversion = 0;
   nextobject++;
 
   Array<char> entry;
   entry.setSize(1024);
   sprintf(entry.getBase(), " /Names %d %d R ", ndobjectnumber, ndversion);
   entry.setSize(strlen(entry.getBase())+1);
   addDictionaryEntry(rootstring, entry);

   int i;
   for (i=0; i<rootstring.getSize()-2; i++) {
      newroot << rootstring[i];
   }
   newroot << "\n";
   initialoffset += newroot.str().length();
   out << newroot.str() << flush;

   // create a name dictionary for the root object:

   SSTREAM namedict;
   namedict << "\n";
   objectindex.append(ndobjectnumber);
   int tempoffset = initialoffset + namedict.str().length();
   offsetindex.append(tempoffset);
   namedict << ndobjectnumber << " " << ndversion << " obj\n";
   namedict << "<<\n";

   int embedlistobjnum = pdffile.getObjectCount()+1 + embedcount*2;

   namedict << "/EmbeddedFiles " << embedlistobjnum << " 0 R\n";
   namedict << ">>\n";
   namedict << "endobj\n";
   initialoffset += namedict.str().length();
   nextobject++;

   out << namedict.str() << flush;

   // add the embedded file listing:

   objectindex.append(embedlistobjnum);
   SSTREAM embedlist;
   embedlist << "\n";
   tempoffset = initialoffset + embedlist.str().length();
   offsetindex.append(tempoffset);
   
   embedlist << embedlistobjnum << " 0 obj\n";
   embedlist << "<<\n"; 
   embedlist << "   /Names [\n";
   int tempobjnum;
   char nullchar = (char)0;
   for (i=0; i<embedcount; i++) {
      // point to the second object of each pair in the
      // objectindex array (up to the count of file added):
      tempobjnum = objectindex[i*2+1];
      embedlist << "           ";
      embedlist << "(";
      embedlist << (char)0xfe << (char)0xff;
      embedlist << nullchar << "U";
      embedlist << nullchar << "n";
      embedlist << nullchar << "t";
      embedlist << nullchar << "i";
      embedlist << nullchar << "t";
      embedlist << nullchar << "l";
      embedlist << nullchar << "e";
      embedlist << nullchar << "d";
      embedlist << nullchar << " ";
      embedlist << nullchar << "O";
      embedlist << nullchar << "b";
      embedlist << nullchar << "j";
      embedlist << nullchar << "e";
      embedlist << nullchar << "c";
      embedlist << nullchar << "t) ";
      embedlist << tempobjnum << " 0 R\n";
   }
   embedlist << "          ]\n";
   embedlist << ">>\n";
   embedlist << "endobj\n";

   initialoffset += embedlist.str().length();
   out << embedlist.str() << flush;

   return nextobject;
}



//////////////////////////////
//
// updateNamesObject -- Adds an EmbeddedFiles entry into the Root's
//     Name Dictionary, or creates an EmbeddedFiles entry if none
//     exists.
// 261 0 obj
// <<
//   /Dests 254 0 R
//   /EmbeddedFiles 400 0 R       % Added Embedded Files list
// >>
// endobj
//

int updateNamesObject(ostream& out, Array<int>& objectindex, 
      Array<int>& offsetindex, int initialoffset, ifstream& file, 
      int nextobject, int ndoffset) {

   SSTREAM ndstream;
   getObject(ndstream, file, ndoffset);
   Array<char> ndstring;
   ndstring.setSize(ndstream.str().length()+1);
   int i;
   for (i=0; i<(int)ndstream.str().length(); i++) {
      ndstring[i] = ndstream.str()[i];
   }
   ndstring[ndstring.getSize()-1] = '\0';

   PerlRegularExpression pre;

   // char buffer[128] = {0};
   Array<char> entry;
   entry.setSize(1000);

   if (pre.search(ndstring.getBase(), 
         "/EmbeddedFiles\\s+(\\d+)\\s+(\\d+)\\s+R", "")) {
      // nothing to change in Name Dictionary, just
      // go to the list of embedded files...
      // int iobject = atol(pre.getSubmatch(1));
      // nextobject = updateEmbeddedFileList(out, objectindex, offsetindex, 
      //         initialoffset, file, nextobject, offsetindex[iobject]) 
      // ggg
   } else {
      // Add an /EmbeddedFiles entry to the Name Dictionary
      int assignednum = nextobject++;
      int version = 0;
      sprintf(entry.getBase(), " /EmbeddedFiles %d %d R ", assignednum, 
         version);
      entry.setSize(strlen(entry.getBase())+1);
      addDictionaryEntry(ndstring, entry);
      // print the new Root's name dictionay object

      // and create a list of embedded files.
  
      // ggg

   }

   return nextobject;
}



//////////////////////////////
//
// addDictionaryEntry -- add an entry to a dictionary.  You should
//    do a check before calling this function to make sure that
//    the name key is not already in the dictionary.
//

void addDictionaryEntry(Array<char>& objectstring, Array<char>& entry) {
   int level = 0;
   Array<char> newobject;
   newobject.setSize(objectstring.getSize()+entry.getSize()+100);
   newobject.setGrowth(1000);
   newobject.setSize(0);
   int inserted = 0;
   char ch;
   int i, j;
   // char buffer[128] = {0};
   // int plen;

   for (i=0; i<objectstring.getSize(); i++) {
      newobject.append(objectstring[i]);
      if (objectstring[i] == '<') {
         level++;
      }
      if (objectstring[i] == '>') {
         level--;
      }
      if (inserted || (level != 2)) {
         continue;
      }

      inserted = 1;
      for (j=0; j<entry.getSize()-1; j++) {
         newobject.append(entry[j]);
      }

      continue;
   }

   ch = '\0'; newobject.append(ch);
   objectstring.setSize(newobject.getSize());
   strncpy(objectstring.getBase(), newobject.getBase(), newobject.getSize());
}



//////////////////////////////
//
// getObject -- get an object's contents (but not intended for stream objects,
//    although it will usually work for them as well).
//

void getObject(ostream& out, istream& file, int offset) {
   int level = 0;
   int endindex = 0;
   char endstate[128] = {0};
   strcpy(endstate, "endobj");
   int endtarget = strlen(endstate);

   char ch;
   file.seekg(offset, ios::beg);
   file.get(ch);

   while (!file.eof()) {
      if (ch == '<') {
         level++;
      } else if (ch == '>') {
         level--;
      }
      if (level <= 0) {
         if (endstate[endindex] == ch) {
            endindex++;
         } else {
            endindex = 0;
         }
         if (endindex == endtarget) {
            out << ch;
            return;
         }
      }
      out << ch;
      file.get(ch);
   }

   cerr << "ERROR: end of Object had strange error" << endl;
   exit(1);

   return;
}



//////////////////////////////
//
// generateNewXref --
//    filesize == size in bytes of original file.
//

int generateNewXref(SSTREAM& finalout, Array<int>& objectindex, 
      Array<int>& offsetindex, int filesize) {

   SSTREAM out;

   // temporary code for testing:
   // filesize = 0;
  
   int output = 0;

   out << "\n";

   output = out.str().length();

   out << "xref\n";
   // don't need the null object:
   // out << "0 1\n";
   // out << "0000000000 65535 f" << (char)0x0d << (char)0x0a;

   // output the starting object number in a sequence
   // and then how many follow, then the offset and version numbers
   // followed by " n" 0x0d 0x0a.

   if (objectindex.getSize() == 0) {
      return output;
   }

   int i;
   int currenti = 0;
   int currentlen = getSequentialObjectCount(objectindex, currenti);
   int value;

   while (currenti < objectindex.getSize()) {
      out << objectindex[currenti] << " " << currentlen << "\n";

      for (i=currenti; i<currenti+currentlen; i++) {
         value = offsetindex[i] + filesize;
         if (value < 0) {
            cerr << "ERROR: offset is negative which is impossible." << endl;
	    cerr << "Value is " << value << endl;
	    cerr << "original filesize is " << filesize << endl;
	    cerr << "offset value is " << offsetindex[i] << endl;
            exit(1);
         }
         if (value < 10)         { out << '0'; }  // 0-
         if (value < 100)        { out << '0'; }  // 00-
         if (value < 1000)       { out << '0'; }  // 000-
         if (value < 10000)      { out << '0'; }  // 0000-
         if (value < 100000)     { out << '0'; }  // 00000-
         if (value < 1000000)    { out << '0'; }  // 000000-
         if (value < 10000000)   { out << '0'; }  // 0000000-
         if (value < 100000000)  { out << '0'; }  // 00000000-
         if (value < 1000000000) { out << '0'; }  // 000000000-
         out << value;
         out << " ";
         out << "00000";
         out << " ";
         out << "n";
         out << (char)0x0d << (char)0x0a;
      }
      currenti  += currentlen;
      currentlen = getSequentialObjectCount(objectindex, currenti);
   }

   finalout << out.str();
   return output;
}



//////////////////////////////
//
// getSequentialObjectCount --
//

int getSequentialObjectCount(Array<int>& list, int starti) {
   int i;
   int output = 1;
   for (i=starti+1; i<list.getSize(); i++) {
      if (list[i] - list[i-1] != 1) {
         break;
      } else {
         output++;
      }
   }

   return output;
}



//////////////////////////////
//
// addTrailerPrev -- Add a /Prev entry to a trailer, or change one
//     if it already exists in the trailer.
//

void addTrailerPrev(Array<char>& trailerstring, int newprevoffset) {
   int level = 0;
   Array<char> newtrailer;
   newtrailer.setSize(trailerstring.getSize()+1000);
   newtrailer.setGrowth(1000);
   newtrailer.setSize(0);
   PerlRegularExpression pre;
   int prevprinted = 0;
   char ch;
   int i, j;
   char buffer[128] = {0};
   int plen;
   for (i=0; i<trailerstring.getSize(); i++) {
      if (trailerstring[i] == '<') {
         level++;
      }
      if (trailerstring[i] == '>') {
         level--;
      }
      if (level != 2) {
         newtrailer.append(trailerstring[i]);
         continue;
      }
      if (trailerstring[i] != '/') {
         newtrailer.append(trailerstring[i]);
         continue;
      }
      if (pre.search(trailerstring.getBase()+i, "^(/Prev\\s+\\d+)", "")) {
         const char* ptr = pre.getSubmatch(1);
         plen = strlen(ptr);
         i+= plen-1;
         sprintf(buffer, "/Prev %d", newprevoffset);
         j = 0;
         while (buffer[j] != '\0') {
            newtrailer.append(buffer[j++]);
         }
         prevprinted = 1;
         continue;
      }
      newtrailer.append(trailerstring[i]);
      continue;
   }
   ch = '\0'; newtrailer.append(ch);

   if (!prevprinted) {
      // need to insert a /Prev entry (at end of dictionary)
      level = 0;
      for (i=newtrailer.getSize()-1; i>=0; i--) {
         if (newtrailer[i] == '>') { 
            level++;
         }
         if (newtrailer[i] == '<') { 
            level--;
         }
         if (level != 2) {
            continue;
         }
         sprintf(buffer, " /Prev %d\n", newprevoffset);
         plen = strlen(buffer);
         int oldlen = newtrailer.getSize();
         newtrailer.setSize(oldlen+plen);
         int newlen = newtrailer.getSize();
         for (j=0; j<oldlen-i+1; j++) {
            newtrailer[newlen-j-1] = newtrailer[newlen-j-plen-1];
         }
         for (j=0; j<plen; j++) {
            newtrailer[i+j] = buffer[j];
         }
	 break;
      }
   }

   trailerstring.setSize(newtrailer.getSize());
   strncpy(trailerstring.getBase(), newtrailer.getBase(), newtrailer.getSize());
}



//////////////////////////////
//
// createFileEntry --
//
//

int createFileEntry(SSTREAM& out, HumdrumFile& infile, const char* filename,
       int nextobject, Array<int>& objectindex, Array<int>& offsetindex,
       int initialoffset) {
   SSTREAM datatoencode;
   infile.write(datatoencode);
   nextobject = printStreamObject(out, nextobject, datatoencode, filename,
                      objectindex, offsetindex, initialoffset);
   return nextobject;
}



/////////////////////////////
//
// printStreamObject --
//

int printStreamObject(ostream& finalout, int objnum, SSTREAM& datatoencode, 
      const char* filename, Array<int>& objectindex, Array<int>& offsetindex,
      int initialoffset) {
   SSTREAM streamcontents;
   SSTREAM out;
   createStreamData(streamcontents, datatoencode, filename);
   int contentsize = streamcontents.str().length();
   char newline = 0x0a;
   int version = 0;

   int initiallen = out.str().length();

   // print the embedded file content stream object ///////////////////

   out << "\n";

   objectindex.append(objnum);
   int offset = out.str().length() - initiallen;
   offset += initialoffset;
   offsetindex.append(offset);

   out << objnum++ << " " << version << " obj\n";
   out << "<<\n";
   out << "   /Type    /EmbeddedFile\n";
   out << "   /SubType /application#2fx-humdrum\n";
   out << "   /Length  " << contentsize << "\n";

   struct stat attrib;
   stat(filename, &attrib);
   struct tm* moddate;
   moddate = gmtime(&(attrib.st_mtime));
   out << "   /Params\n";
   out << "   <<\n";
   out << "      /CreationDate (";
   printPdfDate(out, moddate); // example: D:20050727132644-04'00'
   out << ")\n";
   out << "      /ModDate      (";
   printPdfDate(out, moddate); // example D:20050727143111-04'00'
   out << ")\n";
   out << "      /Size         " <<  datatoencode.str().length() << "\n";
   out << "      /CheckSum     <";
   CheckSum::getMD5Sum(out, datatoencode); 
   // such as 5C94A7BE7C695C70271E29A26B5705C1
   out << ">\n";
   out << "   >>\n";

   out << ">>\n";
   out << "stream" << newline;
   out << streamcontents.str();
   int len1 = streamcontents.str().length();
   if ((streamcontents.str()[len1-1] != 0x0a) && 
       (streamcontents.str()[len1-1] != 0x0d)) {
      out << "\n";
   }
   out << "endstream\n";
   out << "endobj\n";

   // print the file spec object: /////////////////////////////////////
  
   Array<char> outfilename;
   int len = strlen(filename);
   outfilename.setSize(1000 + len);
   outfilename.setGrowth(1000);
   strcpy(outfilename.getBase(), filename);
   outfilename.setSize(len+1);
   
   PerlRegularExpression pre;

   if (!keepdirQ) {
      pre.sar(outfilename, ".*/", "", "g");
   }

   out << "\n";

   objectindex.append(objnum);
   offset = out.str().length() - initiallen;
   offset += initialoffset;
   offsetindex.append(offset);

   out << objnum << " " << version << " obj\n";
   objnum++;
   out << "<<\n";
   out << "   /Type /Filespec\n";
   out << "   /F    (";
   if (prefixQ) {
      out << prefix;
   }
   out << outfilename.getBase();
   out << ")\n";
   out << "   /EF   << /F " << objnum-2 << " 0 R >>\n"; // object with contents
   out << "   /Desc (Embedded Humdrum File)\n";         // descripion of file
   out << ">>\n";
   out << "endobj\n";

   finalout << out.str();

   return objnum;
}



//////////////////////////////
//
// printPdfDate -- time is printed in UTC  plus deviation from UTC for 
//       localtime.
// example D:20050727143111-04'00'
//         D:yyyymmddhhmmss-HH'MM'
//

void printPdfDate(ostream& out, struct tm* date) {
   char buffer[128] = {0};
   strftime(buffer, 128, "D:%Y%m%d%H%M%S", date);
   out << buffer;

   // print time zone information (need to check on daylight savings)
// Remove timezone info for now since Apple OS X is having difficulties.
int value  = 0; 
//   int value = timezone;
   char sign = '-';
//   if (timezone < 0) {
//      value = -timezone;
//   }

   int hour = value / 3600;
   int min  = value - hour * 3600;    

   if (min < 0) { min = 0; }
   out << sign;
   if (hour < 10) { out << "0"; }
   out << hour << "'";
   if (min < 10) { out << "0"; }
   out << min << "'";

}



//////////////////////////////
//
// createStreamData --
//

void createStreamData(ostream& out, SSTREAM& datatoencode, 
      const char* filename) {
   out << datatoencode.str();
}


void createStreamDataOld(ostream& out, SSTREAM& datatoencode, 
      const char* filename) {
   datatoencode << ends;

   out << "<humdrum encoding='base64' source='";
   out << filename;
   out << "'>\n";

   string sss = datatoencode.str();
   // out << "Length of string " << sss.length() << endl;

   int count = sss.length() - 1;
   int packets = count / 3;
   // deal with leftovers later...
   int i;
   for (i=0; i<packets; i++) {
      printMimeEncoding(out, 3, sss[i*3], sss[i*3+1], sss[i*3+2]);
      if ((i+1) % 19 == 0) {
         out << endl;
      }
   }
   int leftovers = count % 3;
   switch (leftovers) {
      case 1:
         printMimeEncoding(out, 1, sss[count-1], 0, 0);
         break;
      case 2:
         printMimeEncoding(out, 2, sss[count-2], sss[count-1], 0);
         break;
   }
   out << "\n</humdrum>\n";
}



//////////////////////////////
//
// printMimeEncoding -- Not used any longer...
//

void printMimeEncoding(ostream& out, int count, char char1, char char2, 
      char char3) {
   static char table[64] = {
         'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
         'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
         'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
         'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
         '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

   out << table[char1 >> 2];
   out << table[((char1 & 0x03) << 4) | (char2 >> 4)];
   if (count == 1) {
      out << "==";
      return;
   }
   out << table[((char2 & 0x0f) << 2) | (char3 >> 6)];
   if (count == 2) {
      out << "=";
      return;
   }
   out << table[char3 & 0x3f];

}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("p|pdf=s:", "PDF file on which file(s) will be attached");
   opts.define("A|append-only=b", "output only data to append to PDF");
   opts.define("debug=b", "print debugging statements to standard error");
   opts.define("D|keep-directory=b", "keep directory in filename");
   opts.define("d|directory=b",      "append directory path to filename");
   opts.define("P|prefix=s:",        "prepend path to written filename");

   opts.define("author=b",  "author of program");
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");
   opts.define("help=b",    "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, May 2010" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 5 May 2010" << endl;
      cout << "compiled: " << __DATE__ << endl;
      cout << MUSEINFO_VERSION << endl;
      exit(0);
   } else if (opts.getBoolean("help")) {
      usage(opts.getCommand());
      exit(0);
   } else if (opts.getBoolean("example")) {
      example();
      exit(0);
   }

   if (opts.getBoolean("pdf")) {
      pdffilename = opts.getString("pdf");
   } else {
      //// No -p option is now allowed.  It means that the PDF will be
      //// coming into the program from standard input.
      // cerr << "Error: -p file.pdf option is required." << endl;
      // exit(1);
   }

   footerQ  = opts.getBoolean("append-only");
   debugQ   = opts.getBoolean("debug");
   keepdirQ = opts.getBoolean("keep-directory");
   prefixQ  = opts.getBoolean("prefix");
   if (prefixQ) {
      prefix = opts.getString("prefix");
      keepdirQ = 0;
   }
}



//////////////////////////////
//
// example -- example usage of the quality program
//

void example(void) {
   cout <<
   "                                                                         \n"
   << endl;
}



//////////////////////////////
//
// usage -- gives the usage statement for the meter program
//

void usage(const char* command) {
   cout <<
   "                                                                         \n"
   << endl;
}




// md5sum: 9202c66af90809a144c86dd132461f0b humpdf.cpp [20111105]
