//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Feb 14 23:40:51 PST 2002
// Last Modified: Tue Mar 26 00:51:52 PST 2002 (item access by staff added)
// Last Modified: Fri Jul 27 14:03:57 PDT 2012 (added some system functionality)
// Filename:      ...sig/src/sigInfo/ScorePageSet.h
// Web Address:   http://sig.sapp.org/include/sigInfo/ScorePageSet.h
// Syntax:        C++ 
//
// Description:   A page of SCORE data with analysis functions.
//

#ifndef _SCOREPAGESET_H_INCLUDED
#define _SCOREPAGESET_H_INCLUDED

#include "ScorePage.h"

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif


class PageItem {
   public:
      PageItem(void) { };
     ~PageItem() { };
      int page;
      int obj;
};

class ScorePageSet {
   public:
                     ScorePageSet        (void);
                    ~ScorePageSet        ();
      ScorePage&     operator[]          (int pageidx);
      int            getPageCount        (void);
      ScorePage&     getPage             (int pageidx);
      int            appendRead          (const char* filename, int verboseQ);
      double         getDuration         (void);
      double         getAbsBeat          (int objidx);
      int            getSystemStaffIndex (int objidx);
      int            getSysStaffIndex    (int objidx) 
                                            { return getSystemStaffIndex(objidx); }
      int            getSystemStaffCount (void);
      int            getSysStaffCount    (void) 
                                            { return getSystemStaffCount(); }
      int            getPartCount        (void);
      ScoreRecord&   getItem             (int objidx);
      int            getItemCount        (void);
      int            getObjectCount      (void) { return getItemCount(); }

      int            isAtEnd             (int objidx);

      void           analyzeContent      (void);
      void           analyzePartNames    (void);

      int            getTitle            (Array<char>& title);
      int            getComposer         (Array<char>& composer);
      int            getInstrumentName   (int staffidxi, 
                                          Array<char>& name);
      int            getInitialStaffIndent(void);
      int            getVerseCount       (int staffidx);
      int            getPartVerseCount   (int staffidx);
      int            getItemPage         (int objidx);
      int            getItemPageIndex    (int objidx);
      
   private:
      Array<ScorePage*> pages;
      double            workDuration;
      Array<PageItem>   systemItems;     // list of items sorted by page,system,sysindex,hpos,vpos,P1

      Array<int>        verseCount;      // summarizes results of ScorePage::analyzeLyrics()

   protected:
      void              identifyPageBreakHyphens  (int page1, int sys1idx, int page2, int sys2idx, 
                                                   int staffidx);

};



#endif /* _SCOREPAGESET_H_INCLUDED */


// md5sum: e86d0157bce66e5816d9a18a49d4d49a ScorePageSet.h [20050403]
