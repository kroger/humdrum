//
// Copyright 1997-1999 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Mar  3 22:23:34 GMT-0800 1997
// Last Modified: Mon Mar  3 22:23:34 GMT-0800 1997
// Last Modified: Mon Nov 29 10:28:23 PST 1999 (name change MidiFile->MidiFile2)
// Filename:      ...sig/include/sigInfo/MidiFile2.h
// Web Address:   http://sig.sapp.org/src/sigInfo/MidiFile2.h
// Syntax:        C++ 
//
// Description:   Older version of the MidiFile class.  Different
//                memory management handling.
//

#ifndef _MIDIfILE2_H_INCLUDED
#define _MIDIfILE2_H_INCLUDED

#include "MidiMessage.h"
#include "sigTypes.h"

typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned short ushort;


class MidiFile2 {
   public:
                     MidiFile2            (void);
                     MidiFile2            (const char* filename);
                    ~MidiFile2            ();

      void           backtrack           (int aTrack);
      int            eot                 (int aTrack);
      MidiMessage    getNextMessage      (int aTrack);
      int            getNumTracks        (void);
      int            getTicksPerQuarterNote(void);
      int            input               (const char* filename);
      uchar          extractByte         (int aTrack);
      ulong          extractVLVtime      (int aTrack);
      void           printnext           (int aTrack);
      void           reset               (int aTrack);
      void           resetAllTracks      (void);

   protected:
      uchar*         midiData;         // storage space for actual MidiFile2
      int            dataSize;         // number of bytes in midiData
   
      // variables derived from/used with midiData values
      int            numTracks;        // the number of tracks in MidiFile2
      int*           trackStart;       // starting point of track chunk in array
      int*           trackSize;        // number of bytes in track
      int*           currTrackIndex;   // current pointer to track data
      int*           currTrackCommand; // current command read from track 
      int*           trackLoopCount;   // current iteration in track

      // private functions
      ushort         flip2bytes          (ushort aNumber);
      ulong          flip4bytes          (ulong aNumber);
      void           setupTrackStart     (void);

};


#endif  /* _MIDIfILE2_H_INCLUDED */



// md5sum: 30022b1efee8aafc2f5dfe87747e90fc MidiFile2.h [20050403]
