//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Sep  5 21:34:57 GMT-0800 1997
// Last Modified: Fri Sep  5 21:34:58 GMT-0800 1997
// Last Modified: Tue Nov 10 14:25:34 PST 1998
// Filename:      ...sig/maint/code/control/Event/TwoStageEvent/TwoStageEvent.h
// Web Address:   http://sig.sapp.org/include/sig/TwoStageEvent.h
// Syntax:        C++ 
//
// Description:   For storage in the EventBuffer class.
//

#ifndef _TWOSTAGEEVENT_H_INCLUDED
#define _TWOSTAGEEVENT_H_INCLUDED

#include "Event.h"

class NoteEvent;

#define EVENT_TWOSTAGE_NOTE (8)


class TwoStageEvent : public Event {
   public:
                     TwoStageEvent     (void);
                    ~TwoStageEvent     ();

      void           action           (EventBuffer& midiOutput);
      void           action           (EventBuffer* midiOutput);
      int            getDur           (void) const;
      int            getDuration      (void) const;
      int            getOffTime       (void) const;
      int            getOnTime        (void) const;
      void           off              (EventBuffer& midiOutput);
      void           off              (EventBuffer* midiOutput);
      void           print            (void) const;
      void           setDur           (int aDuration);             
      void           setDuration      (int Duration);             
      void           setOffDur        (int aTime, int aDuration);
      void           setOnDur         (int aTime, int aDuration);
      void           setOff           (int aTime);
      void           setOffTime       (int aTime);
      void           setOnTime        (int aTime);

   protected:
      // no variables allowed
};

#include "NoteEvent.h"
    

#endif  /* _TWOSTAGEEVENT_H_INCLUDED */



// md5sum: ad095d2167fdfd22a7b4d71337d66977 TwoStageEvent.h [20020518]
