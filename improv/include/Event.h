//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Sep  5 21:34:57 GMT-0800 1997
// Last Modified: Fri Sep  5 21:34:58 GMT-0800 1997
// Last Modified: Sun Jun 11 14:26:51 PDT 2000 (added floatValue() function)
// Filename:      ...sig/src/control/Event/Event.h
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/include/Event.h
// Syntax:        C++ 
//
// Description:   A base class for "events" for storage in the 
//                EventBuffer class.  This class stores performance
//                data for MIDI events to be performed in the future.
//

#ifndef _EVENT_H_INCLUDED
#define _EVENT_H_INCLUDED

#define UNKNOWN -1

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

class EventBuffer;
class OneStageEvent;
class TwoStageEvent;
class MultiStageEvent;

typedef unsigned char uchar;

#define EVENT_ONESTAGE       (1)
#define EVENT_TWOSTAGE       (2)
#define EVENT_MULTISTAGE     (4)
#define EVENT_UNKNOWN        (7)

#define EVENT_STATUS_ACTIVE  (2)
#define EVENT_STATUS_ON      (1)
#define EVENT_STATUS_OFF     (0)


class Event {
   public:
                       Event          (void);
                       Event          (Event& anEvent);
        virtual       ~Event          ();

        virtual void   action         (EventBuffer* midiOutput);
        virtual void   action         (EventBuffer& midiOutput);
        void           activate       (void);
        int            getGroup       (void) const;
        int            getStatus      (void) const;
        int            getP0          (void) const;
        int            getP1          (void) const;
        int            getP2          (void) const;
        int            getP3          (void) const;
        int            getTime1       (void) const;
        int            getTime2       (void) const;
        int            getType        (void) const;
        int            getActionTime  (void) const;
        int            isdead         (void) const;
        virtual void   kill           (int aGroup, EventBuffer* midiOutput);
        virtual void   kill           (int aGroup, EventBuffer& midiOutput);
        virtual void   off            (EventBuffer* midiOutput);
        virtual void   off            (EventBuffer& midiOutput);
        Event&         operator=      (const Event& anEvent);
        uchar&         p0             (void);
        uchar&         p1             (void);
        uchar&         p2             (void);
        uchar&         p3             (void);
        void           print          (void);
        void           setGroup       (int aGroup = 0);
        void           setP0          (int aValue);
        void           setP1          (int aValue);
        void           setP2          (int aValue);
        void           setP3          (int aValue);
        void           setTime1       (int aTime);
        void           setTime2       (int aTime);
        int&           intValue       (int index);
        float&         floatValue     (int index);
        short&         shortValue     (int index);
        char&          charValue      (int index);


      // functions you should not use unless you know what you're doing
      uchar          getByte        (int index);
      void           setType        (int aType);
      void           setStatus      (int aStatus);

   protected:

      // Note that classes derived from the Event class CANNOT
      // contain any data fields since Event class is used
      // in array storage in the EventBuffer class.

      uchar         data[32];
                    // data[0] is the event type
                    // data[1] is the status byte
                    // data[2] through data[3]   is the event group number
                    // data[4] through data[7]   is the action time 
                    // data[8] is the first parameter byte
                    // data[9] is the first parameter byte
                    // data[10] is the first parameter byte
                    // data[11] is the first parameter byte
                    // data[12] through data[15] reserved for 2nd time value
                    // data[15] through data[31] free for derived class use


      void          printBits      (uchar aByte, ostream& output = cout) const;
};
    

#include "OneStageEvent.h"
#include "TwoStageEvent.h"
#include "MultiStageEvent.h"
#include "EventBuffer.h"


#endif  /* _EVENT_H_INCLUDED */



// md5sum: 0d089d8ea9667d7541b7e2a79a64051f Event.h [20050403]
