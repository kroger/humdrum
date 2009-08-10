//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Sep  5 22:00:43 GMT-0800 1997
// Last Modified: Fri Jan 16 20:39:34 GMT-0800 1998
// Last Modified: Mon Nov  9 13:42:51 PST 1998
// Filename:      ...sig/maint/code/control/Event/OneStageEvent/MidiEvent.h
// Web Address:   http://sig.sapp.org/include/sig/MidiEvent.h
// Syntax:        C++ 
//
// Description:   For use in the EventBuffer class.  Stores the
//                performance data for a MIDI event.
//

#ifndef _MIDIEVENT_H_INCLUDED
#define _MIDIEVENT_H_INCLUDED

#include "OneStageEvent.h"


class MidiEvent : public OneStageEvent {
   public:
                  MidiEvent         (void);
                 ~MidiEvent         ();

      void        action            (EventBuffer& midiOutput);
      void        action            (EventBuffer* midiOutput);
      void        off               (void);

   protected:
      // no data members allowed


};


#endif  /* _MIDIEVENT_H_INCLUDED */



// md5sum: 424db21b19312de3749011d6990c7d3d MidiEvent.h [20020518]
