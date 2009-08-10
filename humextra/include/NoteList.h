//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May 14 14:21:10 PDT 2001
// Last Modified: Mon May 14 14:21:08 PDT 2001
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Filename:      ...sig/include/sigInfo/NoteList.h
// Web Address:   http://sig.sapp.org/include/sigInfo/NoteList.h
// Syntax:        C++ 
//
// Description:   Used in chord and key identification in HumdrumFile class.
//


#ifndef _NOTELIST_H_INCLUDED
#define _NOTELIST_H_INCLUDED

#include "SigCollection.h"

#ifdef OLDCPP
   #include <ostream.h>
#else
   #include <ostream>
#endif

class NoteList {
   public: 
                   NoteList         (void);
                  ~NoteList         ();

      void         clear            (void);
  
      const int    getLine          (void);
      const int    getSpine         (void);
      const int    getToken         (void);
      const int    getPitch         (void);
      const double getLevel         (void);
      const double getAbsBeat       (void);
      const double getFirstDur      (void);
      const double getDur           (void);
      const double getDuration      (void);
      const double getTrack         (void);
  
      void         setLine          (int aValue);
      void         setSpine         (int aValue);
      void         setToken         (int aValue);
      void         setPitch         (int aValue);
      void         setLevel         (double aValue);
      void         setAbsBeat       (double aValue);
      void         setFirstDur      (double aValue);
      void         setDur           (double aValue);
      void         setDuration      (double aValue);
      void         setTrack         (double aValue);

      void         pitchBase12      (void);
      void         pitchBase12Class (void);
      void         pitchBase40      (void);
      void         pitchBase40Class (void);

   protected:
      int   line;	    // line number in the original HumdrumFile
      unsigned short spine; // spine number of the note
      unsigned short token; // token number of the note (chord note)
      float track;          // track number in the original HumdrumFile
      float dur;            // tied duration of the note
      float firstdur;       // duration of first note in tie
      float level;          // metric level of the note
      float absbeat;        // absolute beat position of the note
      int   pitch;          // pitch of the note
      int   realpitch;      // base 40 value of the note

};

class NoteListArray : public SigCollection<NoteList> {
   public:
                   NoteListArray          (void);
                  ~NoteListArray          ();

      void         clear            (void);
      void         pitchBase12      (void);
      void         pitchBase12Class (void);
      void         pitchBase40      (void);
      void         pitchBase40Class (void);
};

ostream& operator<<(ostream& out, NoteList& nl);

#endif /* _NOTELIST_H_INCLUDED */



// md5sum: fe36dca7b7c9d91dc60282b40c0fee63 NoteList.h [20050403]
