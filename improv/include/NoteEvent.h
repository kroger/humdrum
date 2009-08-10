//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Sep  5 21:34:57 GMT-0800 1997
// Last Modified: Fri Jan 16 20:49:38 GMT-0800 1998
// Filename:      .../sig/src/Event/TwoStepEvent/MidiNote.h
// Web Address:   http://sig.sapp.org/include/sig/NoteEvent.h
// Syntax:        C++ 
//
// Description:   Used in the storage of the EventBuffer class.
//

#ifndef _NOTEEVENT_H_INCLUDED
#define _NOTEEVENT_H_INCLUDED

#include "TwoStageEvent.h"


class NoteEvent : public TwoStageEvent {
   public:
                     NoteEvent        (void);
                    ~NoteEvent        ();

      void           action           (EventBuffer& midiOutput);
      void           action           (EventBuffer* midiOutput);
      int            getChan          (void) const;
      int            getChannel       (void) const;
      int            getKey           (void) const;
      int            getKeyno         (void) const;
      int            getOffVel        (void) const;
      int            getOffVelocity   (void) const;
      int            getVel           (void) const;
      int            getVelocity      (void) const;
      void           off              (EventBuffer& midiOutput);
      void           off              (EventBuffer* midiOutput);
      void           print            (void);
      void           setChan          (int aChannel = 0);
      void           setChannel       (int aChannel = 0);
      void           setKey           (int aKey = 0);
      void           setKeyno         (int aKey = 0);
      void           setVel           (int aVelocity = 127);
      void           setVelocity      (int aVelocity = 127);
      void           setOffVel        (int aVelocity = 127);
      void           setOffVelocity   (int aVelocity = 127);


   protected:
      // no data variables allowed

};
    

#endif  /* _NOTEEVENT_H_INCLUDED */



// md5sum: b11936c3e1d8e8c98d744faf5df6dd3e NoteEvent.h [20020518]
