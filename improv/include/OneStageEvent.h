//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Sep  5 22:00:43 GMT-0800 1997
// Last Modified: Fri Jan 16 20:39:34 GMT-0800 1998
// Last Modified: Mon Nov  9 13:42:51 PST 1998
// Filename:      ...sig/maint/code/control/Event/OneStageEvent/OneStageEvent.h
// Web Address:   http://sig.sapp.org/include/sig/OneStageEvent.h
// Syntax:        C++ 
//
// Description:   Used in the storage of the EventBuffer class.
//


#ifndef _ONESTAGEEVENT_H_INCLUDED
#define _ONESTAGEEVENT_H_INCLUDED

#define EVENT_ONESTAGE_MIDI (1)

#include "Event.h"

class MidiEvent;

class OneStageEvent : public Event {
   public:
                  OneStageEvent     (void);
                 ~OneStageEvent     ();

      void        action            (EventBuffer& midiOutput);
      void        action            (EventBuffer* midiOutput);
      int         getActionTime     (void) const;
      int         getTime           (void) const;
      void        off               (EventBuffer& midiOutput);
      void        off               (EventBuffer* midiOutput);
      void        setTime           (int aTime);

   protected:
      // no data members allowed


};

#include "MidiEvent.h"

#endif  /* _ONESTAGEEVENT_H_INCLUDED */



// md5sum: 06b3be275c86f5e277e0e849b6d70528 OneStageEvent.h [20020518]
