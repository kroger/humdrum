//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Feb 14 23:40:51 PST 2002
// Last Modified: Tue Mar 26 00:51:52 PST 2002 (item access by staff added)
// Filename:      ...sig/src/sigInfo/ScorePage.h
// Web Address:   http://sig.sapp.org/include/sigInfo/ScorePage.h
// Syntax:        C++ 
//
// Description:   A page of SCORE data with analysis functions.
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

      // system analysis functions 
      void           analyzeSystems       (void);
      int            getStaffCount        (void);
      int            getMaxStaff          (void);
      int            getSystemCount       (void);
      int            getMaxBarlineLength  (int staffno);
      int            getTrack             (int staffno);
      int            getSystem            (int staffno);
      ScoreRecord&   getStaff             (int staffno, int staffItem);
      ScoreRecord&   getStaffItem         (int staffno, int staffItem);
      int            getStaffSize         (int staffno);
      ScoreRecord&   getSystemItem        (int sysno, int index);
      int            getSystemSize        (int sysno);

      // pitch analysis functions 
      void           analyzePitch         (void);
      ScoreRecord&   getClefAtLocation    (int staffno, float position);
      ScoreRecord&   getKeysigAtLocation  (int staffno, float position);

      // duration analysis of each staff

   protected:
      // pitch analysis functions
      int            assignPitch          (int staff, int currentposition);
      void           resetAccidentals     (Array<int>& accidentals, int keysig);
      int            convertPitchToBase40 (int line, int acc, int clef, 
                                           int clefoffset, 
                                           Array<int>& accidentals);
 
      // system analysis functions
      void           buildSystemIndexDatabase(void);

};



#endif /* _SCOREPAGE_H_INCLUDED */


// md5sum: e86d0157bce66e5816d9a18a49d4d49a ScorePage.h [20050403]
