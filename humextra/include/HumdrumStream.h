//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Dec 11 16:03:43 PST 2012
// Last Modified: Tue Dec 11 16:03:46 PST 2012
// Filename:      ...sig/include/sigInfo/HumdrumStream.h
// Web Address:   http://sig.sapp.org/include/sigInfo/HumdrumStream.h
// Syntax:        C++ 
//
// Description:   Multi-movement manager for Humdrum files.  The class
//                will accept files, standard input, URLs, URIs which
//                have more than one data start/stop sequence.  This usually
//                indicates multiple movements if stored in one file, or
//                multiple works if coming in from standard input.
//

#ifndef _HUMDRUMSTREAM_H_INCLUDED
#define _HUMDRUMSTREAM_H_INCLUDED


#include "HumdrumStream.h"

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

class HumdrumStream : public HumdrumStreamBasic {
   public:
                      HumdrumStream      (void);

   int                is_open            (void);
   int                hasData            (void);
   
   int                getFile            (HumdrumFile& infile);

   protected:
      ifstream            instream;      // used to read from list of files.
      Array<char>         newfilebuffer; // used to keep track of !!!!newfile: records

      Array<Array<char> > filelist;      // used when not using cin
      int                 curfile;       // index into filelist

      Array<Array<char> > universals;    // storage for universal comments

   

};

#endif /* _HUMDRUMSTREAM_H_INCLUDED */



// md5sum: 8e155fdb7b2d0af7bbfa1d92cd7ccd85 HumdrumStream.h [20050403]
