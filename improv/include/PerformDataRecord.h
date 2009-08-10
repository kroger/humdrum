//
// Copyright 1999 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul  2 23:05:34 PDT 1999
// Last Modified: Mon Jul  5 10:54:00 PDT 1999
// Filename:      .../sig/include/sigInfo/PerformDataRecord.h
// Web Address:   http://sig.sapp.org/src/sigInfo/PerformDataRecord.h
// Syntax:        C++
//
// Description:   Data for a somewhat defunct class, Perform.
//

#ifndef _PERFORMDATARECORD_H_INCLUDED
#define _PERFORMDATARECORD_H_INCLUDED

#include "Array.h"

#define PERFORM_TYPE_NULL    (0)
#define PERFORM_TYPE_TEXT    (1)
#define PERFORM_TYPE_MIDI    (2)
#define PERFORM_TYPE_MEASURE (3)
#define PERFORM_TYPE_TEMPO   (4)
#define PERFORM_TYPE_CLEAR   (5)
#define PERFORM_TYPE_IGNORED (6)
#define PERFORM_TYPE_BEGIN   (7)
#define PERFORM_TYPE_END     (8)


class PerformDataRecord {
   public:
                   PerformDataRecord      (void);
                   PerformDataRecord      (PerformDataRecord& aRecord);
                  ~PerformDataRecord      ();

      char*        getData                (void);
      int          getMeasureNumber       (void);
      int          getLength              (void);
      double       getTempoNumber         (void);
      int          getTime                (void);
      int          getType                (void);
      int          match                  (const char* matchString);
      PerformDataRecord& operator=        (PerformDataRecord& aRecord);
      ostream&     print                  (ostream& out = cout);
      void         setBar                 (int aTime, const char* measureData,
                                             int length = -1);
      void         setBar                 (int aTime, int aMeasure);
      void         setClear               (int aTime);
      void         setMeasure             (int aTime, const char* measureData,
                                             int length = -1);
      void         setMeasure             (int aTime, int aMeasure);
      void         setMidi                (int aTime, const char* someData, 
                                             int length);
      void         setTempo               (int aTime, const char* tempoData,
                                             int length = -1);
      void         setTempo               (int aTime, int aTempo);
      void         setText                (int aTime, const char* someText,
                                             int length = -1);
      void         setTime                (int aTime);
      void         setType                (int aType);

      // convenience functions for determining type of data:
      int          barQ                   (void);
      int          beginQ                 (void);
      int          endQ                   (void);
      int          measureQ               (void);
      int          midiQ                  (void);
      int          tempoQ                 (void);
      int          textQ                  (void);

   protected:
      int          time;                  // time to perform data
      int          type;                  // type of data
      Array<char>  data;                  // data of record
};


#endif  /* _PERFORMDATARECORD_H_INCLUDED */



// md5sum: 3dc5143fb717febc5c18c6ff84427c6b PerformDataRecord.h [20020518]
