//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue May 11 21:54:37 PDT 2010
// Last Modified: Tue May 11 21:54:41 PDT 2010
// Filename:      ...sig/include/sigInfo/PDFFile.h
// Web Address:   http://sig.sapp.org/include/sigInfo/PDFFile.h
// Syntax:        C++ 
//
// Description:   
//

#ifndef _PDFFILE_H_INCLUDED
#define _PDFFILE_H_INCLUDED

#include "Array.h"

#ifndef OLDCPP
   #include <iostream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #include <iostream.h>
   #ifdef VISUAL
      #include <strstrea.h>
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif


///////////////////////////////////////////////////////////////////////////

class PDFFile {
   public:
                 PDFFile                 (void);
                 PDFFile                 (ifstream& file);
                 PDFFile                 (istream& file);
                ~PDFFile                 ();

      void       clear                   (void);
      int        getObjectCount          (void);
      int        getObjectOffset         (int index);
      int        getFileSize             (void);
      int        getRootIndex            (void);
      int        getXrefCount            (void);
      int        getXrefOffset           (int index);
      void       process                 (ifstream& file);
      void       process                 (istream& file);
      void       print                   (ostream& out);
      int        getEmbeddedFileCount    (void);
      int        getEmbeddedFileObject   (int index);
      void       getTrailerString        (int index, Array<char>& tstring);
      int        getObjectDictionary     (int objectindex, ostream& out);
      int        getObjectDictionary     (int objectindex, Array<char>& out);
      void       getEmbeddedFileContents (int embedindex, ostream& out);
      void       getEmbeddedFileContents (int embedindex, Array<char>& out);
      void       getStreamData           (int streamobject, ostream& out);
      int        isEmbeddedHumdrumFile   (int index);

   private:
      istream*           pdfdatastream;
      Array<Array<int> > indirectobjectoffset;
      Array<Array<int> > indirectobjectversions;
      int                pdfdatasize;     // number of bytes in the file
      int                rootobject;      // indirect object number of root
      Array<int>         xref;            // list of xref table offsets
                                          // from most recent to oldest
      Array<int>         embedFile;       // list of embedded files

   protected:
      void       initializeXrefTable     (void);
      static int getXrefOffset           (istream& file);
      void       buildIndirectObjectIndex(istream& file, int xrefoffset);
      static int getPreviousOffset       (istream& file);
      void       processXrefList         (istream& file, char firstdigit);
      void       getOriginalTrailer      (SSTREAM& trailer, int filesize, 
                                          int xref, istream& file);
      void       expandObjectListing     (int newsize);
      void       makeEmbeddedFileList    (void);

};

#endif /* _PDFFILE_H_INCLUDED */



// md5sum: 8e155fdb7b2d0af7bbfa1d92cd7ccd85 PDFFile.h [20050403]
