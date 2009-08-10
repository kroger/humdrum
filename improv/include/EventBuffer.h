// class EventBuffer header file
//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Feb 17 23:19:30 PST 1996
// Last Modified: Mon Feb 16 22:17:30 GMT-0800 1998
// Last Modified: Wed Sep 30 13:48:15 PDT 1998
// Last Modified: Sat Jun 13 21:16:29 PDT 2009 (check --> xcheck for OSX)
// Filename:      ...sig/src/control/EventBuffer/EventBuffer.h
// Web Address:   http://sig.sapp.org/include/sig/EventBuffer.h
// Syntax:        C++ 
//
// Description:   A storage and performance class that holds notes,
//                etc. until a certain time when they are performed.
//

#ifndef _EVENTBUFFER_H_INCLUDED
#define _EVENTBUFFER_H_INCLUDED


#include "Event.h"
#include "CircularBuffer.h"
#include "MidiOutput.h"
#include "SigTimer.h"


class _EBPrivate {
   public:
      int last;
      int next;
};


class EventBuffer : public MidiOutput {
   public:
                EventBuffer        (int bufferSize = 1024);
               ~EventBuffer        (void);

      int       aquire             (void);
      void      activate           (int);
      void      xcheck             (void);
      void      xcheck             (long currentTime);
      int       checkPoll          (void);
      int       countEvents        (void) const;
      int       getBufferSize      (void) const;
      int       getFreeCount       (void) const;
      int       getPollPeriod      (void) const; 
      int       insert             (const Event* anEvent);
      int       insert             (const Event& anEvent);
      void      off                (void);
      Event&    operator[]         (int anIndex);
      void      print              (void) const;
      void      reset              (void);
      void      setBufferSize      (int);
      void      setPollPeriod      (double aPeriod);


   protected:
      Event*              eventStorage;     // ptr to Event storage location
      int                 storageSize;      // max num of elements in storage
      _EBPrivate*         eventList;        // list of events to play
      CircularBuffer<int> freeSlots;        // free event spaces in storage
      SigTimer            pollTimer;        // for period checking of poll
      SigTimer            timer;            // for getting current time


   // private functions:
      void      removeEvent      (int index);

};


#endif  /* _EVENTBUFFER_H_INCLUDED */



// md5sum: 8126d3542649d363e1dbb8e1e2000ee3 EventBuffer.h [20020518]
