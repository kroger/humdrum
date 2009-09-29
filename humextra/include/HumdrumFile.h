//
// Copyright 1998-2001 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May 18 13:43:47 PDT 1998
// Last Modified: Mon Oct 16 18:04:47 PDT 2000 (revised for spines and rhythm)
// Last Modified: Tue Nov 28 11:35:23 PST 2000 (rebuilt deleted include)
// Last Modified: Sat Dec  2 11:53:10 PST 2000 (added basic assemble command)
// Last Modified: Wed Dec  6 13:22:08 PST 2000 (added analyzeMetricLevel())
// Last Modified: Wed Dec 13 13:52:07 PST 2000 (added analyzeTempoMarkings())
// Last Modified: Sat Dec 16 13:37:19 PST 2000 (added analyzeDataIndex())
// Last Modified: Sat Dec 16 14:41:14 PST 2000 (added analyzeCliche())
// Last Modified: Mon May 14 18:21:37 PDT 2001 (moved chord functions)
// Last Modified: Mon Nov  5 17:55:54 PST 2001 (added getNextDatum/getLastDatum)
// Last Modified: Wed Jan  1 16:23:10 PST 2003 (removed Maxwell functions)
// Last Modified: Thu Aug 21 20:13:10 PDT 2003 (add *free, *strict markers)
// Last Modified: Wed Jun 16 21:15:06 PDT 2004 (indep. tracks in analyzeKeyKS)
// Last Modified: Mon Jun  5 06:59:27 PDT 2006 (add fixIrritatingPickupProblem)
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Last Modified: Sat Sep  5 22:03:28 PDT 2009 (ArrayInt to Array<int>)
// Filename:      ...sig/include/sigInfo/HumdrumFile.h
// Web Address:   http://sig.sapp.org/include/sigInfo/HumdrumFile.h
// Syntax:        C++ 
//
// Description:   Higher-level functions for processing Humdrum files.
//                Inherits HumdrumFileBasic and adds rhythmic and other
//                types of analyses to the HumdrumFile class.
//

#ifndef _HUMDRUMFILE_H_INCLUDED
#define _HUMDRUMFILE_H_INCLUDED


#include "HumdrumFileBasic.h"
#include "NoteList.h"
#include "ChordQuality.h"

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif


//////////////////////////////
//
// HumdrumFile::getNoteList defines
//

#define  NL_NORESTS  (0<<0)  /* don't include rests in list */
#define  NL_RESTS    (1<<0)  /* include rests */
#define  NL_NOFILL   (0<<1)  /* ignore null tokens */
#define  NL_FILL     (1<<1)  /* expand null token meanings */
#define  NL_NOMIDI   (0<<2)  /* pitch list output in Base-40 note numbers */
#define  NL_MIDI     (1<<2)  /* pitch list output in MIDI note numbers */
#define  NL_NOPC     (0<<3)  /* output pitches with octave information */
#define  NL_PC       (1<<3)  /* output pitch class values */
#define  NL_NOSORT   (0<<4)  /* don't sort pitches from lowest to highest */
#define  NL_SORT     (1<<4)  /* sort pitches from lowest to highest */
#define  NL_NOUNIQ   (0<<5)  /* don't remove redundant pitches */
#define  NL_UNIQ     (1<<5)  /* remove redundant pitches */
#define  NL_TIED     (0<<6)  /* don't remove tied notes */
#define  NL_NOTIED   (1<<6)  /* remove tied notes */

//
// HumdrumFile analysis* flag settings
//

#define DEBUG_BIT            0
#define COMPOUND_METER_BIT   1
#define PITCH_BASE_BIT       2

#define AFLAG_NODEBUG          (0<<DEBUG_BIT)
#define AFLAG_DEBUG            (1<<DEBUG_BIT)

#define AFLAG_NOCOMPOUND_METER (0<<COMPOUND_METER_BIT)
#define AFLAG_COMPOUND_METER   (1<<COMPOUND_METER_BIT)

#define AFLAG_BASE40_PITCH     (0<<PITCH_BASE_BIT)
#define AFLAG_BASE12_PITCH     (1<<PITCH_BASE_BIT)
#define AFLAG_MIDI_PITCH       (1<<PITCH_BASE_BIT)

///////////////////////////////////////////////////////////////////////////

class HumdrumFile : public HumdrumFileBasic {
   public:
                             HumdrumFile      (void);
                             HumdrumFile      (const HumdrumFile& aHumdrumFile);
                             HumdrumFile      (const HumdrumFileBasic& 
                                                 aHumdrumFile);
                             HumdrumFile      (const char* filename);
                            ~HumdrumFile      ();

      void                   appendLine       (const char* aLine);
      void                   appendLine       (HumdrumRecord& aRecord);
      void                   appendLine       (HumdrumRecord* aRecord);
      static int             assemble         (HumdrumFile& output, int count, 
                                                 HumdrumFile** pieces);
      static int             assemble         (HumdrumFile& output, int count, 
                                                 HumdrumFile* pieces);
      static int             combine          (HumdrumFile& output, 
                                                 HumdrumFile& A,   
                                                 HumdrumFile& B, int debug=0);
      void                   clear            (void);
      HumdrumFile            extract          (int aField);
      double                 getPickupDuration(void);
      double                 getPickupDur     (void);
      int                    getStartIndex    (double startbeat);
      int                    getStopIndex     (double stopbeat);
      double                 getAbsBeat       (int index);
      double                 getBeat          (int index);
      double                 getDuration      (int index);
      const char*            getLastDatum     (int index, int spine, 
                                               int options = 0);
      const char*            getNextDatum     (int index, int spine, 
                                               int options = 0);
      int                    getLastDatumLine (int& nspine, int index, int spine, 
                                               int options = 0);
      int                    getNextDatumLine (int& nspine, int index, int spine, 
                                               int options = 0);
      int                    getNoteList      (Array<int>& notes, int line, 
                                                   int flag);
      void                   getNoteArray     (Array<double>& absbeat, 
                                               Array<int>& pitches, 
                                               Array<double>& durations, 
                                               Array<double>& levels,
                                               int startLine = 0, 
                                               int endLine = 0, 
                                               int tracknum = -1);
      void                   getNoteArray2    (Array<double>& absbeat, 
                                               Array<int>& pitches, 
                                               Array<double>& durations, 
                                               Array<double>& levels,
                                               Array<Array<int> >& lastpitches,
                                               Array<Array<int> >& nextpitches,
                                               int startLine = 0, 
                                               int endLine = 0);
      double                 getTiedDuration  (int linenum, int field, 
                                                 int token = 0);
      double                 getTiedStartBeat (int linenum, int field,
                                                 int token = 0);
      double                 getTotalDuration (void);
      HumdrumFile&           operator=        (const HumdrumFile& aFile);
      void                   read             (const char* filename);
      void                   read             (istream& inStream);

      // analyses that generate internal data
      void                   analyzeRhythm    (const char* base = "", 
                                                 int debug = 0);
      int                    getMinTimeBase   (void);
      int                    rhythmQ          (void);

      //
      // analyses that generate external data
      //

      void analyzeDataIndex(Array<int>& indices, int segment = -1);

      // form analyses
      int  analyzeCliche(Array<int>& cliche, double duration, 
            int minimumcount, double start = -1.0, double stop = -1.0);

      // metrical analyses 
      void analyzeTempoMarkings(Array<double>& tempo, double tdefault = 60.0);
      void analyzeMeter(Array<double>& top, Array<double>& bottom, 
         int flag = AFLAG_NOCOMPOUND_METER);
      void analyzeBeatDuration(Array<double>& beatdur, 
         int flag = AFLAG_COMPOUND_METER);
      void analyzeAttackAccentuation(Array<int>& atakcent);
      void analyzeMetricLevel(Array<int>& metlev);

      // sonority harmonic analyses
      void analyzeSonorityQuality(Array<ChordQuality>& cq);
      void analyzeSonorityRoot(Array<int>& roots, 
            int flag = AFLAG_BASE40_PITCH);

      // Krumhansl-Schmuckler key-finding algorithms
      int analyzeKeyKS(Array<double>& scores, int startindex, int
            stopindex, int rhythmQ = 1, int binaryQ = 0, int tracknum = -1);
      int analyzeKeyKS2(Array<double>& scores, int startindex, int
            stopindex, double* majorprofile, double* minorprofile, 
            int rhythmQ = 1, int binaryQ = 0, int tracknum = -1);

   protected:
      int rhythmcheck;         // 1 = rhythm analysis has been done
      int minrhythm;           // the least common multiple of all rhythms
      double pickupdur;        // duration of a pickup measure

   private:
      static int intcompare(const void* a, const void* b);
      void   convertKernStringToArray(Array<int>& array, const char* string);

      // spine analysis functions:
      void       privateSpineAnalysis(void);
      int        predictNewSpineCount(HumdrumRecord& aRecord);
      void       makeNewSpineInfo(SigCollection<char*>&spineinfo, 
                        HumdrumRecord& aRecord, int newsize, int& spineid,
                        SigCollection<int>& ex);
      void       simplifySpineInfo(SigCollection<char*>& info, int index);


      // rhythm analysis functions:
      void       privateRhythmAnalysis(const char* base = "", int debug = 0);
      double     determineDuration(HumdrumRecord& aRecord,
                         int& init, SigCollection<double>& lastdurations, 
                         SigCollection<double>& runningstatus,
                         Array<int>& rhythms, Array<int>& ignore);
      void       adjustForSpinePaths(HumdrumRecord& aRecord, 
                         SigCollection<double>& lastdurations, 
                         SigCollection<double>& runningstatus, int& init,
                         int& datastart, Array<int>& ignore);
      void       adjustForRhythmMarker(HumdrumRecord& aRecord,
                         int state, int spine, 
                         SigCollection<double>& lastdurations, 
                         SigCollection<double>& runningstatus, int& init, 
                         int& datastart, Array<int>& ignore);
      void       fixIncompleteBarMeter(SigCollection<double>& meterbeats, 
                         SigCollection<double>& timebase);
      void       fixIrritatingPickupProblem(void);
      void       spaceEmptyLines(void);
      void       initializeTracers(SigCollection<double>& lastduration,
                         SigCollection<double>& runningstatus, 
                         HumdrumRecord& currRecord);
      int        GCD      (int a, int b);
      int        findlcm  (Array<int>& rhythms);

      // determining the meaning of dots (null records)
      void       privateDotAnalysis(void);
      void       readjustDotArrays(Array<int>& lastline, Array<int>& lastspine, 
                       HumdrumRecord& record, int newsize);

      // for use with assemble()
      static int processLinesForCombine(HumdrumFile& output, HumdrumFile& A, 
                       HumdrumFile& B, int debug = 0);
                          
      // private function for analyzeCliche:
      int attemptMatch(Array<Array<int> >& allnotes, Array<int>& di, int
         starti, int i, int j, double duration);


   ///////////////////////////////////////////////////////////////////////
   // 
   // functions defined in HumdrumFile-chord.cpp
   //

   public:
      int measureChordRoot            (Array<double>& scores,
                                       Array<double>& parameters, 
                                       double startbeat, double stopbeat, 
                                       int algorithmno = 0, int debug = 0);
      int measureChordRoot            (Array<double>& scores,
                                       Array<double>& parameters, 
                                       int startindex, int stopindex, 
                                       int algorithmno = 0, int debug = 0);
      int  measureChordRoot0          (Array<double>& scores, 
                                       Array<double>& parameters, 
                                       NoteListArray& notelist);
      int  measureChordRoot1          (Array<double>& scores, 
                                       Array<double>& parameters, 
                                       NoteListArray& notelist);
      int  measureChordRoot2          (Array<double>& scores, 
                                       Array<double>& parameters, 
                                       NoteListArray& notelist);
      int  measureChordRoot3          (Array<double>& scores, 
                                       Array<double>& parameters, 
                                       NoteListArray& notelist);
      int  measureChordRoot4          (Array<double>& scores, 
                                       Array<double>& parameters, 
                                       NoteListArray& notelist);
      int  measureChordRoot5          (Array<double>& scores, 
                                       Array<double>& parameters, 
                                       NoteListArray& notelist);
      int  measureChordRoot6          (Array<double>& scores, 
                                       Array<double>& parameters, 
                                       NoteListArray& notelist);
      int  measureChordRoot7          (Array<double>& scores, 
                                       Array<double>& parameters, 
                                       NoteListArray& notelist);
      int  measureChordRoot8          (Array<double>& scores, 
                                       Array<double>& parameters, 
                                       NoteListArray& notelist);
      int  measureChordRoot9          (Array<double>& scores, 
                                       Array<double>& parameters, 
                                       NoteListArray& notelist);
      void generateNoteList           (NoteListArray& notelist, 
                                       int startLinst, int endLine);


   // old functions which should not be used:
  
   int analyzeChordProbabilityDurNorm(Array<double>& coef, 
      double startbeat, double stopbeat, Array<int>& scorelevels, 
      double empirical1, double empirical2, double sx, double sy);

   int analyzeChordProbabilityDur(Array<double>& coef, 
      double startbeat, double stopbeat, Array<int>& scorelevels, 
      double empirical1, double empirical2, double sx, double sy);

   int analyzeChordProbability(Array<double>& coef, 
      int start, int stop, Array<int>& scorelevels, double empirical1,
      double empirical2, double sx, double sy);

};

#endif /* _HUMDRUMFILE_H_INCLUDED */



// md5sum: 8e155fdb7b2d0af7bbfa1d92cd7ccd85 HumdrumFile.h [20050403]
