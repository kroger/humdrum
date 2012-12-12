//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Feb 14 23:40:51 PST 2002
// Last Modified: Tue Mar 26 00:51:52 PST 2002 item access by staff added
// Last Modified: Fri Jul 27 14:03:57 PDT 2012 added some system functionality
// Last Modified: Sat Sep  1 15:34:49 PDT 2012 Renovated.
// Filename:      ...sig/src/sigInfo/ScorePage.h
// Web Address:   http://sig.sapp.org/include/sigInfo/ScorePage.h
// Syntax:        C++ 
//
// Description:   A page of SCORE data with analysis functions which identify
//                non-explicit structural organization of the data.
//

#ifndef _SCOREPAGE_H_INCLUDED
#define _SCOREPAGE_H_INCLUDED

#include "ScorePageBase.h"

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif

class ScorePage : public ScorePageBase {
   public:
                     ScorePage            (void);
                    ~ScorePage            ();


      // Page staff related functions
      int            getMaxStaff          (void);
      int            getMaxStaffNumber    (void) { return getMaxStaff(); }
      int            getMaxPageStaff      (void) { return getMaxStaff(); }
      int            getMaxPageStaffNumber(void) { return getMaxStaff(); }

      int            getStaffCount        (void);
      int            getPageStaffCount    (void) { return getStaffCount(); }
      int            getP2ToConsecutiveStaff(int p2number);
      int            getConsecutiveStaffToP2(int consecutiveIndex);


      // System line related functions
      int            getStaffCount        (int sysidx);
      int            getLineSystemStaffCount(int sysidx) 
                                          { return getStaffCount(sysidx); }
      int            getSystemCount       (void);
      int            getLineSystemCount   (void) { return getSystemCount(); }


      // Analysis functions 
      void           analyzeContent       (void);
      void           analyzeSystems       (void);


////////////////////////////////////////////////////////////////////////
//
// Functions to renovate:
//
/*
      void           analyzePitch         (int sysidx = - 1);
      void           analyzeRhythm        (int sysidx = - 1);
      void           analyzeTies          (void);
      void           analyzeBarlines      (void);
      void           analyzeLyrics        (void);
      void           analyzeDirections    (void);

      int            getStaffCount        (int sysidx);


      double         getSystemDuration    (int sysIdx);
      void           getSystemObjectIndexes(int sysidx, Array<int>& list);
      void           getStaffObjectIndexes(int sysidx, int staffidx, 
                                           Array<int>& list);
      int            getSystemIndex       (int objidx);
      int            getSystemStaffIndex  (int objidx);
      int            getPageStaffNumber   (int objidx);
      int            getPageStaff         (int objidx);
      int            getSystemStaffItemIndex(int objidx);
      int            getSysStaffIndex     (int objidx) 
                                          {return getSystemStaffIndex(objidx);}
      int            getMaxBarlineLength  (int staffno);
      int            getMaxBarlineLength  (int sysidx, int staffidx);
      int            getTrack             (int staffno);
      int            getSystem            (int staffno);
      ScoreRecord&   getStaff             (int staffno, int staffItem);
      ScoreRecord&   getStaffItem         (int staffno, int staffItem);
      int            getStaffSize         (int staffno);
      ScoreRecord&   getSystemItem        (int sysno, int index);
      int            getSystemItemNumber  (int sysno, int index);
      ScoreRecord&   getStreamItem        (int objidx);
      int            getStreamItemCount   (void);
      int            getItemCount         (void);
      int            getItemCount         (int sysidx, int staffidx);
      ScoreRecord&   getItem              (int objidx);
      ScoreRecord&   getItem              (int sysidx, int staffidx,int objidx);
      int            getItemNumber        (int sysidx,int staffidx, int objidx);
      int            getItemSystemNumber  (int objidx);
      int            getObjectCount       (void);
      int            getSystemItemCount   (int sysno);
      int            getSystemSize        (int sysno);
      int            getSystemStaffCount  (int sysidx);
      int            getSystemStaffObjectCount(int sysidx, int staffidx);
      int            getSystemStaffItemCount  (int sysidx, int staffidx);
      int            getSysStaffItemCount (int sysidx, int staffidx) {
                        return getSystemStaffItemCount(sysidx, staffidx); }
      int            getSystemStaffNumber (int sysidx, int staffidx);
      int            getSystemStaffItemNumber (int sysidx, int staffidx, 
                                               int objidx);
      int            getSysStaffItemNumber (int sysidx, int staffidx, 
                                            int objidx) {
                        return getSystemStaffItemNumber(sysidx, staffidx, 
                                                        objidx); }
      ScoreRecord&   getSystemStaffItem   (int sysidx,int staffidx, int objidx);
      ScoreRecord&   getSysStaffItem      (int sysidx,int staffidx, int objidx){
                        return getSystemStaffItem(sysidx, staffidx, objidx); }
      int            getStaffOffset       (int objidx);
      void           packBase40Pitch      (void);
      double         getPageDuration      (void);
      int            hasDuration          (int objidx);
      int            hasDuration          (int sysidx, int staffidx, 
                                           int objidx);
      double         getDuration          (int objidx);
      double         getDuration          (int sysidx, int staffidx, 
                                           int objidx);
      double         getPitch             (int objidx);
      double         getPitch             (int sysidx, int staffidx, 
                                           int objidx);
      void           getSlurEndNotes      (int objidx, Array<int>& leftside, 
                                           Array<int>& rightside);
      void           getNotesAtHpos       (Array<int>& notes, int sysidx, 
                                           int staffidx, double hpos);
      int            getSystemStaffLyricCount(int staffidx);
      int            getPageStaffLyricCount(int staffidx);
      void           identifySystemBreakHyphens(int sys1idx, int sys2idx, 
                                           int staffidx);
      int            getLowestStaffOnSystem (int sysidx);
      double         getNextNotePositionOnStaff(int objidx);
      double         getPreviousNotePositionOnStaff(int objidx);

      // pitch analysis functions 
      void           analyzePitchX        (void);
      ScoreRecord&   getClefAtLocation    (int staffno, float position);
      ScoreRecord&   getKeysigAtLocation  (int staffno, float position);

      void           resetPitchSpellings  (int staffidx, int barheight, 
                                           Array<Array<int> >& pitchstate, 
                                           Array<Array<int> >& keysig);
      void           fillPitchStatesWithKeySig(
                                           Array<Array<int> >& pitchstate, 
                                           int staffidx, 
                                           Array<Array<int> >& keysig);
      // duration analysis of each staff

   protected:
      // pitch analysis functions
      int            assignPitchX         (int staff, int currentposition);
      void           resetAccidentals     (Array<int>& accidentals, int keysig);
      int            convertPitchToBase40 (int line, int acc, int clef, 
                                           int clefoffset, 
                                           Array<int>& accidentals);
      void           numberMeasures      (int sysidx, int objidx, double hpos,
                                          int barnum, double delta);

   private:
      void           identifyLyricsOnStaff(int sysidx, int staffidx);
      void           processVerse      (int verse, int vpos, int sysidx, 
                                        int staffidx, Array<int>& objlist, 
                                        Array<double>& vposes);
      void           storeNextEvent    (Array<double>& expectedStaffBeat, 
                                        double nextevent);
      double         getSmallestEvent  (Array<double>& expectedStaffBeat);
      void           removeEvent       (Array<double>& expectedStaffBeat, 
                                        double currentSysDur);
 
      // system analysis functions
      void           buildSystemIndexDatabase(void);

*/
};



#endif /* _SCOREPAGE_H_INCLUDED */


// md5sum: e86d0157bce66e5816d9a18a49d4d49a ScorePage.h [20050403]
