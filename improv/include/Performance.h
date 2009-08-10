//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul  2 23:05:34 PDT 1999
// Last Modified: Thu Jul  8 15:11:51 PDT 1999
// Filename:      .../sig/include/sigControl/Performance.h
// Web Address:   http://sig.sapp.org/include/sigControl/Performance.h
// Syntax:        C++
//
// Description:
//

#ifndef _PERFORMANCE_H_INCLUDED
#define _PERFORMANCE_H_INCLUDED

#include "PerformData.h"
#include "MidiOutput.h"


class Performance : public PerformData, public MidiOutput {
   public:
                 Performance             (void);
                ~Performance             ();

      void       action                  (void);
      int        getBar                  (void);
      int        getMeasure              (void);
      double     getTempo                (void);
      double     getTempoMultiplier      (void);
      int        getTextEcho             (void);
      double     getTicksPerQuarterNote  (int ticks);
      void       gotoBar                 (int aBar);
      void       pause                   (void);
      void       perform                 (void);
      void       play                    (void);
      void       reset                   (void);
      void       search                  (const char* regexpression, int dir);
      void       setTempo                (double aTempo);
      void       setTempoMultiplier      (double aMultiplier);
      void       setTextEcho             (int aState);
      void       setTicksPerQuarterNote  (int ticks);
      void       start                   (void);
      void       stop                    (void);
      void       tacet                   (void);
      void       unpause                 (void);

   protected:
      int        ticksPerQuarter;        // ticks per quarter note
      SigTimer   timer;                  // for keeping track of time
      double     tempoMultiplier;        // for altering the default tempos
      char       noteState[16][128];     // current on/off state of notes
      int        playingQ;               // for keeping track of performance
      int        nextActionTime;         // for keeping track in performance
      int        current_measure;        // current measure of performance
      double     current_tempo;          // current tempo of performance
      double     default_tempo;          // tempo to use if no tempo marks
      int        echoTextQ;              // for performing text.

   private:
      void       zeroNoteStates          (void);
      void       markOff                 (int channel, int key);
      void       markOn                  (int channel, int key);
};



#endif  /* _PERFORMANCE_H_INCLUDED */



// md5sum: 24404e393643c89cb43fb400afa0c494 Performance.h [20020518]
