//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Sep  5 21:34:57 GMT-0800 1997
// Last Modified: Fri Jan 16 20:49:38 GMT-0800 1998
// Filename:      .../sig/src/Event/TwoStageEvent/FunctionEvent.h
// Web Address:   http://sig.sapp.org/include/sig/FunctionEvent.h
// Syntax:        C++ 
//
// Description    For storage in the EventBuffer class.  This class is
//                used to store a pointer to a function which will be
//                called at a specific time, usually to generate a 
//                MIDI note.
//

#ifndef _FUNCTIONEVENT_H_INCLUDED
#define _FUNCTIONEVENT_H_INCLUDED

#include "TwoStageEvent.h"

typedef void (*Algorithm)(FunctionEvent& p, EventBuffer& midiOutput);

class FunctionEvent : public MultiStageEvent {
   public:
                     FunctionEvent    (void);
                    ~FunctionEvent    ();

      void           action           (EventBuffer& midiOutput);
      void           action           (EventBuffer* midiOutput);
      int            getChan          (void) const;
      int            getChannel       (void) const;
      Algorithm      getFunction      (void) const;
      int            getKey           (void) const;
      int            getKeyno         (void) const;
      int            getOffVel        (void) const;
      int            getOffVelocity   (void) const;
      int            getVel           (void) const;
      int            getVelocity      (void) const;
      void           off              (EventBuffer& midiOutput);
      void           off              (EventBuffer* midiOutput);
      void           print            (void);
      void           setFunction      (Algorithm aFunction);
      void           setChan          (int aChannel = 0);
      void           setChannel       (int aChannel = 0);
      void           setKey           (int aKey = 0);
      void           setKeyno         (int aKey = 0);
      void           setVel           (int aVelocity = 127);
      void           setVelocity      (int aVelocity = 127);


   protected:
      // no data variables allowed

};
    

#endif  /* _FUNCTIONEVENT_H_INCLUDED */



// md5sum: d8fe47d71aedd15fc65bb5b2b4286728 FunctionEvent.h [20020518]
