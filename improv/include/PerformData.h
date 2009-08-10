//
// Copyright 1999 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul  2 23:05:34 PDT 1999
// Last Modified: Tue Jul  6 00:10:53 PDT 1999
// Filename:      .../sig/include/sigInfo/PerformData.h
// Web Address:   http://sig.sapp.org/include/sigInfo/PerformData.h
// Syntax:        C++
//
// Description:   Data for a somewhat defunct class, Perform.
//

#ifndef _PERFORMDATA_H_INCLUDED
#define _PERFORMDATA_H_INCLUDED

#include "PerformDataRecord.h"

#define PERFORM_TIME_UNKNOWN -1
#define PERFORM_TIME_ABSOLUTE 1
#define PERFORM_TIME_RELATIVE 2


class PerformData {
   public:
                            PerformData          (void);
                           ~PerformData          ();

      int                   add                  (PerformDataRecord& record);
      void                  back                 (void);
      int                   bof                  (void);
      void                  clear                (void);
      int                   determineTimeType    (void);
      int                   eof                  (void);
      void                  inputAsciiMidiFile   (const char* filename);
      void                  inputHumdrumMidiFile (const char* filename);
      void                  inputMidiFile        (const char* filename);
      int                   getBar               (void);
      char*                 getData              (void);
      int                   getIndex             (void);
      int                   getLength            (void);
      int                   getMeasure           (void);
      int                   getSize              (void);
      double                getTempo             (void);
      int                   getTime              (void);
      int                   getTimeType          (void);
      int                   getType              (void);
      void                  markAsAbsoluteTime   (void);
      void                  markAsRelativeTime   (void);
      int                   match                (const char* matchString);
      void                  next                 (void);
      PerformDataRecord&    operator[]           (int index);
      ostream&              print                (ostream& out = cout);
      int                   ready                (int aTime);
      void                  setIndex             (int index);
      void                  setTime              (int aTime);
      void                  setTimeType          (int aTimeType);
      void                  setType              (int aType);
      void                  swap                 (int index1, int index2,
                                                    int timeHeld = 0);
      void                  sort                 (void);

   protected:
      int                   currentIndex;   // current performance index
      Array<PerformDataRecord*>  records;   // data
      PerformDataRecord     begin;          // first record in list
      PerformDataRecord     end;            // last record in list
      int                   timeFormat;     // times are delta or absolute

      static int  performRecordCompare      (const void* a, const void* b);

   private:  // helping functions for inputHumdrumMidiFile function
      double        last_tempo;                // for backward tempo tracking
      int           getLineType                (const char* line);
      int           getMeasureValue            (const char* string);
      int           getNextLine                (char* buffer, istream& input);
      int           getSpineCount              (const char* aString);
      void          processHumdrumData         (const char* currentLine, 
                                                  Array<int>& spine_list,
                                                  Array<int>& chan_list);
      void          processInterpLine          (const char* line,
                                                  Array<int>& spine_list,
                                                  Array<int>& chan_list,
                                                  Array<int>& path_list);
      void          processExclusiveInterpLine (const char* line,
                                                  Array<int>& chan_list);
      Array<char*>* segment                    (const char* line);  


};


#endif  /* _PERFORMDATA_H_INCLUDED */



// md5sum: 2a9603b95c5f959a0a307e4515ffd63d PerformData.h [20020518]
