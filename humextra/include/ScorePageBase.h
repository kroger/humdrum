//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Feb 14 23:40:51 PST 2002
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 Renamed SigCollection class
// Last Modified: Sat Aug 25 18:20:06 PDT 2012 Renovated
// Filename:      ...sig/src/sigInfo/ScorePageBase.h
// Web Address:   http://sig.sapp.org/include/sigInfo/ScorePageBase.h
// Syntax:        C++ 
//
// Description:   Base class for ScorePage.  This class handles reading/writing
//                of a ScorePage, and handles all of the data variables for
//                a ScorePage.
//

#ifndef _SCOREPAGEBASE_H_INCLUDED
#define _SCOREPAGEBASE_H_INCLUDED

#include "ScoreRecord.h"

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif


class ScorePageBase {
   public:
                     ScorePageBase     (void);
                     ScorePageBase     (ScorePageBase& aPage);
                    ~ScorePageBase     ();

      ScorePageBase& operator=         (ScorePageBase& aPage);
      void           clear             (void);
      void           invalidateAnalyses(void);
   private:
      void           initializeTrailer (long serial = 0x50504153);

   public:
   // file reading and writing functions:
      void           readFile          (const char* filename, int verboseQ = 0);
      void           readFile          (istream& infile, int verboseQ = 0);
      void           readAscii         (const char* filename, int verboseQ = 0);
      void           readAscii         (istream& infile, int verboseQ = 0);
      void           readAsciiScoreLine(istream& infile, ScoreRecord& record,
                                          int verboseQ = 0);
      void           readBinary        (const char* filename, int verboseQ = 0);
      void           readBinary        (istream& infile, int verboseQ = 0);
      void           writeBinary       (const char* filename);
      ostream&       writeBinary       (ostream& outfile);
      void           writeBinary2Byte  (const char* filename);
      void           writeBinary4Byte  (const char* filename);
      void           printAscii        (ostream& out, int roundQ = 1, 
		                        int verboseQ = 0);
      void           printAsciiWithExtraParameters(ostream& out, int roundQ, 
                                        int verboseQ);
   private:
      float          readLittleFloat   (istream& instream);
      int            readLittleShort   (istream& input);
      void           writeLittleFloat  (ostream& out, float number);

   public:
   // data access/manipulation functions
      int            getSize            (void);
      ScoreRecord&   operator[]         (int index);
 
      void           appendItem         (ScoreRecord& aRecord);
      void           appendItem         (ScorePageBase& aPage);
      void           appendItem         (SigCollection<ScoreRecord>& recs);
      void           appendItem         (SigCollection<ScoreRecord*>& recs);

      void           addItem(ScoreRecord& aRecord) { appendItem(aRecord); };
      void           addItem(ScorePageBase& aPage) { appendItem(aPage); };
      void           addItem            (SigCollection<ScoreRecord>& recs) 
                                           { appendItem(recs); };
   private:
      void           shrinkParameters   (void);

   public:
   // trailer access functions:
      void           setVersion         (float value);
      void           setVersionWinScore (void);
      float          getVersion         (void);
      void           setSerial          (long value);
      long           getSerial          (void);

   // basic analysis function (sorting).
      void           analyzeSort                (void);
      void           createDefaultPrintSortOrder(void);
      void           createLineStaffSequence    (void);

      ScoreRecord&   getItemByPrintOrder        (int index);   
      int            getItemIndexByPrintOrder   (int index);

   private:
      void           sortByHpos           (Array<int>& objects);
      void           quickSortByDataIndex (Array<int>& indexes, int starti, 
                                           int endi);

      int            isGreater           (int a, int b);
      int            isLess              (int a, int b);

   protected:
      // The data array contains a list of all SCORE objects on the page
      // in the order in which they were read from the input file.
      SigCollection<ScoreRecord*> data;

      // trailer is for storing the trailer of a SCORE binary file.
      // The trailer consists of at least 5 floats.  The numbers in
      // reverse order are:
      // 0: The last number is -9999.0 to indicate the end of the trailer
      // 1: The second to last number is a count of the number of 4-byte
      //    numbers in the trailer.  Typically this is 5.0, but may be 
      //    larger in new versions of SCORE.
      // 2: The measurement unit code: 0.0 = inches, 1.0 = centimeters.
      // 3: The program version number which created the file.
      // 4: The program serial number (or 0.0 for version 3 or earlier)
      // 5: The last number in the trailer (i.e., the first number of the
      //    trailer in the file is 0.0.  Normally this is the position in 
      //    the file which the parameter count for an object is given.
      //    Objects cannot have zero parameters, so when 0.0 is found,
      //    that indicates the start of the trailer.
      // The trailer is stored in the order that it is found in the binary
      // file (or the trailer will be empty if the data was read from an
      // ASCII PMX file).  In other words, -9999.0 should be the last
      // number in the trailer array if the data was read from a binary
      // SCORE file.
      Array<double> trailer;

      //////////////////////////////
      //
      // Indexing of data in various configurations with scope of line, page
      // and pageset.  These will be filled by ScorePage::analayzeContent().
      //

      // pagePrintSequence is an array of object in the order in which they
      // are to be printed.  When reading a file, the print order will be
      // defined by the order in which the objects are read from the file.
      Array<int> pagePrintSequence;

      // lineStaffSequence is a array of objects sorted left-to right for
      // each staff on the page.  The first index number matches the P4 staff
      // number of the SCORE object.  The lineStaffSequence[0] list is not
      // used (possibly might be mapped to hidden parts on the line).  The
      // size is lineStaffSequence is set to 100 staves.  If a staff does 
      //  not have any objects on it, it will have a size of 0;
      Array<Array<int> > lineStaffSequence;

      // lineSystemSequence is similar to lineStaffSequence, but sorts all 
      // staves in the given system line from left to right.  The first index
      // number matches to the system line on the page.  The first system line
      // is at the top of the page, and the last is at the bottom of the page.
      // Each system contains a grouping of staves which are analyzed based 
      // on barlines in the analyzeSystems() function.  
      Array<Array<int> > lineSystemSequence;

      // pageSystemSequence is an ordering of objects as a single stream of
      // individual system lines (lineSystemSequence).
      Array<int> pageSystemSequence;

      // pageStaffSequence is an ordering of object similar to 
      // pageSystemSequence, but separated out by invidual system staves.  
      // The first index value is the system staff index (not the P4 staff 
      // number on the page).
      Array<Array<int> > pageStaffSequence;


      //////////////////////////////
      //
      // Lookup tables
      //

      // maxStaffNumber contains the largest staff number found on the page.
      int maxStaffNumber;

      // pageStaffList indexes the number of staves on the page.  Its size
      // may be smaller than maxStaffNumber if there are non-consecutive 
      // staff numbers.  The staves are indexed from 0 starting at the bottom
      // of the page (pageStaffList[0] is the P2 value of the lowest staff
      // on the page) going upwards (pageStaffList.last() is the P2 value
      // of the highest staff on the page.
      Array<int> pageStaffList;

      // pageStaffListReverse is a reverse mapping of P2 value to consecutive
      // staff index.  The size of pageStaffListReverse is 100 elements, with
      // invalid/unused staves indicated by -1 reverse mapping.
      Array<int> pageStaffListReverse;


      // objectSysStaffIndex -- reverse lookup of lineStaffSequence
      Array<int> objectSysStaffIndex;  

      // staffToSystemIdx -- mapping from pageStaff to systemIndex
      // The size of this list is 100 elements.  Staves which are not
      // associated with a staff (such as invisible staves and staves
      // which do not exist in the data) are set to -1.  The system
      // index starts at 0 for the top system of the page and increases
      // downward.
      Array<int> staffToSystemIdx;  

      // staffToSystemStaffIdx -- mapping from pageStaff to systemStaff
      // This list stores the mapping of P2 staff number to system
      // staff index.  
      Array<int> staffToSystemStaffIdx; 


      /////////////////////////////
      //
      // Analytic data -- filled by ScorePage class.
      //

      // systemRhythm -- durnation from start of system to systemObjects item
      Array<Array<double> > systemRhythm;   

      // systemDuration -- durations of each system (P7 values of rests & notes)
      Array<double> systemDuration;

      // pageDuration -- sum of systemDurations
      double pageDuration;


      /////////////////////////////
      //
      // Analytic booleans
      //

      // sortAnalysisQ: used to check if basic sorting was done.  Affects
      // the variables pagePrintSequence and lineStaffSequence.
      int sortAnalysisQ;         

      // printAnalysisQ: used to create the printing order.  This is 
      // usually the order of the data in the input file.  However, the
      // print order can be modified by other member functions, or could
      // be refined to conform with the WinScore print layer parameter.
      int printAnalysisQ;

      int systemAnalysisQ;       // for checking if system analysis was done
      int pitchAnalysisQ;        // for checking if pitch  analysis was done
      int rhythmAnalysisQ;       // for checking if rhythm analysis was done




/*
      Array<int> staffsystem;     // system ownerships of staves on page
      Array<int> track;           // staff number on system
      Array<int> voice;           // voice number on system
      int systemCount;            // number of systems on the page
      int staffCount;             // number of staves on the page
      Array<int> staffStart;      // starting index of the staff item
      Array<int> staffSize;       // number of items on each staff
      Array<int> systemind;       // indices for accessing by system
      Array<int> systemSize;      // number of items in each system
      Array<int> systemStart;        // starting index of items in systemind
      Array<Array<int> > systemStaff;// systemStaff to pageStaff mapping
      Array<int> pageStaffLyrics;    // lyric verse count on each SCORE 
                                     //    page staff
      Array<int> sysStaffLyrics;     // count of lyric verses for each "part" 
                                     //    (system staff set)
*/

   public:
      static int     compareStaff        (const void* A, const void* B);
      static int     compareSystem       (const void* A, const void* B);
      static int     compareSystemIndexes(const void* A, const void* B);
};


class SystemRecord {
   public: 
      SystemRecord(void) { clear(); }
     ~SystemRecord() { clear(); }
      void clear(void) { system = 0; index = 0; ptr = NULL; }
      int system;
      int index;
      ScoreRecord* ptr;
};

#endif /* _SCOREPAGEBASE_H_INCLUDED */


// md5sum: c3202867c565c42152c072c1ae65ffbe ScorePageBase.h [20050403]
