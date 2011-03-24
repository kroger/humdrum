//
// Copyright 2009 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun  3 00:00:21 PDT 2010
// Last Modified: Wed Jun  9 12:24:37 PDT 2010
// Filename:      ...sig/include/SigInfo/MuseData.h
// Web Address:   http://sig.sapp.org/include/sigInfo/MuseData.h
// Syntax:        C++ 
//
// Description:   A class that stores a list of MuseRecords.
//

#ifndef _MUSEDATA_H_INCLUDED
#define _MUSEDATA_H_INCLUDED

#include "MuseRecord.h"
#include "Array.h"


// A MuseEventSet is a timestamp and then a list of pointers to all
// lines in the original file which occur at that time.
// The MuseData class contains a variable called "sequence" which is
// a list of MuseEventSet object pointers which are sorted by time.

class MuseEventSet {
   public:
                         MuseEventSet       (void);
                         MuseEventSet       (RationalNumber atime);
                        ~MuseEventSet       ()     { clear(); }

      void               clear              (void);
      void               setTime            (RationalNumber abstime);
      RationalNumber     getTime            (void);
      void               appendRecord       (MuseRecord* arecord);
      MuseRecord&        operator[]         (int index);
      MuseEventSet       operator=          (MuseEventSet& anevent);
      int                getEventCount      (void);

   protected:
      RationalNumber     absbeat;           // starting time of events
      Array<MuseRecord*> events;            // list of events on absbeat
};



class MuseData {
   public:
                        MuseData            (void);
                        MuseData            (MuseData& input);
                       ~MuseData            ();

      void              setFilename         (const char* filename);
      const char*       getFilename         (void);
      const char*       getPartName         (char* buffer, int maxx=81);
      int               isMember            (const char* mstring);
      int               getMembershipPartNumber(const char* mstring);
      void              selectMembership    (const char* selectstring);
      MuseData&         operator=           (MuseData& input);
      int               getLineCount        (void);
      int               getNumLines         (void) { return getLineCount(); }
      MuseRecord&       last                (void);
      int               isEmpty             (void);
      int               append              (MuseRecord& arecord);
      int               append              (MuseData& musedata);
      int               append              (Array<char>& charstring);
      void              insert              (int index, MuseRecord& arecord);
      void              clear               (void);
      int               getInitialTPQ       (void);

      void              read                (istream& input);
      void              read                (const char* filename);

      // additional mark-up analysis functions for post-processing:
      void              doAnalyses          (void);
      void              analyzeType         (void);
      void              analyzeRhythm       (void);
      void              analyzeTies         (void);
      void              analyzePitch        (void);

      // line-based (file-order indexing) accessor functions:
      MuseRecord&       operator[]          (int lindex);
      MuseRecord&       getRecord           (int lindex);
      double            getTiedDuration     (int lindex);
      RationalNumber    getTiedDurationR    (int lindex);

      RationalNumber    getAbsBeatR         (int lindex);
      double            getAbsBeat          (int lindex);

      int               getLineTickDuration (int lindex);

      // event-based (time-order indexing) accessor functions:
      MuseEventSet&     getEvent            (int eindex);
      int               getEventCount       (void);
      RationalNumber    getEventTime        (int eindex);

      MuseRecord&       getRecord           (int eindex, int erecord);

      int               getLineIndex        (int eindex, int erecord);

      double            getLineDuration     (int eindex, int erecord);
      RationalNumber    getLineDurationR    (int eindex, int erecord);

      double            getNoteDuration     (int eindex, int erecord);
      RationalNumber    getNoteDurationR    (int eindex, int erecord);

      int               getLastTiedNoteLineIndex(int eindex, int erecord);
      int               getNextTiedNoteLineIndex(int eindex, int erecord);

      double            getTiedDuration     (int eindex, int erecord);
      RationalNumber    getTiedDurationR    (int eindex, int erecord);

      int               getType             (int eindex, int erecord);
      void              cleanLineEndings    (void);



   private:
      Array<MuseRecord*>     data;
      Array<MuseEventSet*>   sequence;
      Array<char>            name;

   protected:
      void              processTie          (int eventindex, int recordindex, 
                                             int lastindex);
      int               searchForPitch      (int eventindex, int b40, 
                                             int track);
      int               getNextEventIndex   (int startindex, 
		                             RationalNumber target);
      void              constructTimeSequence(void);
      void              insertEventBackwards (RationalNumber atime, 
		                              MuseRecord* arecord);
      int               getPartNameIndex    (void);
};
   

ostream& operator<<(ostream& out, MuseData& musedata);


#endif  /* _MUSEDATA_H_INCLUDED */


// md5sum: 051e558c8e271327a795fcad656d760b MuseData.h [20050403]
