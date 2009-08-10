//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Nov 10 15:24:14 PST 1998
// Last Modified: Tue Nov 10 15:24:21 PST 1998
// Filename:      ...sig/maint/code/control/Event/MultiStageEvent.h
// Web Address:   http://sig.sapp.org/include/sig/MultiStageEvent.h
// Syntax:        C++ 
//
// Description:   Used in the storage of the EventBuffer class.
//

#ifndef _MULTISTAGEEVENT_H_INCLUDED
#define _MULTISTAGEEVENT_H_INCLUDED

#define EVENT_MULTISTAGE_FUNCTION (8)

#include "Event.h"

class FunctionEvent;


class MultiStageEvent : public Event {
   public:
                     MultiStageEvent  (void);
                    ~MultiStageEvent  ();

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
      void           setOnDur         (int aTime, int aDuration);
      void           setOnTime        (int aTime);

   protected:
      // no variables allowed
};

#include "FunctionEvent.h"
    

#endif  /* _MULTISTAGEEVENT_H_INCLUDED */



// md5sum: ab48bc224bd435ea42f8bbef3b2a8126 MultiStageEvent.h [20020518]
