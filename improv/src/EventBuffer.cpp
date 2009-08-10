//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Feb 17 23:34:43 PST 1996
// Last Modified: Mon Feb 16 22:20:34 GMT-0800 1998
// Last Modified: Thu Nov  5 17:06:33 PST 1998
// Last Modified: Fri Apr 21 15:12:11 PDT 2000 (revisions finalized)
// Filename:      ...sig/src/control/EventBuffer/EventBuffer.cpp
// Web Address:   http://sig.sapp.org/src/sig/EventBuffer.cpp
// Syntax:        C++ 
//
// Description:   A storage and performance class that holds notes,
//                etc. until a certain time when they are performed.

#include "EventBuffer.h"

#include <string.h>


//////////////////////////////
//
// EventBuffer::EventBuffer --
//      default value: aSize = 1024;
//

EventBuffer::EventBuffer(int aSize) {
   if (aSize < 1) {
      cout << "Error: eventBuffer size cannot be less than 1" << endl;
      exit(1);
   }
   storageSize = aSize;
   eventStorage = new Event[storageSize];
   eventList = new _EBPrivate[storageSize + 1];
   freeSlots.setSize(storageSize);
   pollTimer.setPeriod(10);
   reset();
}



//////////////////////////////
//
// EventBuffer::~EventBuffer --
//

EventBuffer::~EventBuffer(void) {
   if (eventStorage != NULL) {
      delete [] eventStorage;
       eventStorage = NULL;
   } 
   if (eventList != NULL) {
      delete [] eventList;
      eventList = NULL;
   }

   storageSize = 0;
}
   


//////////////////////////////
//
// EventBuffer::aquire -- reserve a space in the buffer.  Returns
//     the index in the buffer that can be used.
//

int EventBuffer::aquire(void) {
   return freeSlots.extract();
}



//////////////////////////////
//
// EventBuffer::activate -- put an aquired element into the 
//    event buffer.
//

void EventBuffer::activate(int index) {
   if (index < 0 || index >= storageSize) {
      cerr << "Error: invalid index in EventBuffer::activate." << endl;
      exit(1);
   }

   if (eventList[index].next != storageSize ||
         eventList[index].last != storageSize    ) {
      cerr << "Error: trying to reactivate an element in EventBuffer." << endl;
      exit(1);
   }


   if (eventList[storageSize].last == storageSize) {
      eventList[storageSize].next = index;
      eventList[storageSize].last = index;
      eventList[index].next = storageSize;
      eventList[index].last = storageSize;
   } else {
      eventList[eventList[storageSize].last].next = index;
      eventList[index].last = eventList[storageSize].last;
      eventList[index].next = storageSize;
      eventList[storageSize].last = index;
   }
}



//////////////////////////////
//
// EventBuffer::xcheck -- look at each element in the 
// 	buffer to see if they need to be executed.
//

void EventBuffer::xcheck(void) {
   xcheck((long)timer.getTime());
}


void EventBuffer::xcheck(long currentTime) {
   int item = eventList[storageSize].next;
   int olditem;
   while (item < storageSize) {
      if (eventStorage[item].getActionTime() <= currentTime) {
         eventStorage[item].action(*this);
      }
      olditem = item;
      item = eventList[item].next;
      if (eventStorage[olditem].isdead()) {
         removeEvent(olditem);
      }
   }
}



//////////////////////////////
//
// EventBuffer::checkPoll -- will check the buffer if the poll time
//     has arrived.  Returns true if the buffer was checked
//

int EventBuffer::checkPoll(void) {
   if (pollTimer.expired()) {
      xcheck();
      pollTimer.reset();
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// EventBuffer::countEvents -- count the number of Notes which are not 
//      OFF. 
//

int EventBuffer::countEvents(void) const {
   int count = 0;
   for (int i = 0; i < getBufferSize(); i++) {
      if (!eventStorage[i].isdead()) {
         count++;
      }
   }
   return count;
}



//////////////////////////////
//
// EventBuffer::getBufferSize -- returns the maximum number of events
//     that can be stored in the event buffer.
//

int EventBuffer::getBufferSize(void) const {
   return storageSize;
}



//////////////////////////////
//
// EventBuffer::getFreeCount -- returns the number of free elements
//     in the event buffer.
//

int EventBuffer::getFreeCount(void) const {
   return freeSlots.getCount();
}



//////////////////////////////
//
// EventBuffer::getPollPeriod -- 
//

int EventBuffer::getPollPeriod(void) const {
   return (int)pollTimer.getPeriod();
}



//////////////////////////////
//
// EventBuffer::insert -- returns the location in the buffer
//    where item was stored.
//

int EventBuffer::insert(const Event* newEvent) {
   int freeSpot = freeSlots.extract();
   eventStorage[freeSpot] = *newEvent;
   activate(freeSpot);
   return freeSpot;
}


int EventBuffer::insert(const Event& newEvent) {
   int freeSpot = freeSlots.extract();
   memcpy(&eventStorage[freeSpot], &newEvent, sizeof(Event));
   activate(freeSpot);
   return freeSpot;
}



//////////////////////////////
//
// EventBuffer::off -- turn off all events in the buffer.
//

void EventBuffer::off(void) {
   int item = eventList[storageSize].next;
   int olditem;
   while (item < storageSize) {
      eventStorage[item].off(*this);
      olditem = item;
      item = eventList[item].next;
      removeEvent(olditem);
   }
}



//////////////////////////////
//
// EventBuffer::operator[] --
//

Event& EventBuffer::operator[](int index) {
   if (index < 0 || index >= storageSize) {
      cout << "Error: invalid index for accessing EventBuffer" << endl;
      exit(1);
   }
   return eventStorage[index];
}



//////////////////////////////
//
// EventBuffer::print -- print a list of the current items
//

void EventBuffer::print(void) const {
   int counter = 0;
   cout << "Active elements in event buffer: " << '\n';
   int item = storageSize;
   item = eventList[item].next;
   while (item < storageSize) {
      cout << item << ' ';
      counter++;
      if (counter % 20 == 0) {
         cout << '\n';
      }
      item = eventList[item].next;
   }
   cout << endl;
}



//////////////////////////////
//
// EventBuffer::reset --
//

void EventBuffer::reset(void) {
   freeSlots.reset();

   for (int i=0; i<storageSize; i++) {

      // erase all storage cells
      eventStorage[i].setType(0);
      eventStorage[i].setStatus(0);

      eventList[i].next = storageSize;
      eventList[i].last = storageSize;

      freeSlots.insert(i);
   } 
   eventList[storageSize].next = storageSize;   // top of list
   eventList[storageSize].last = storageSize;   // bottom of list

   pollTimer.reset();
}



//////////////////////////////
//
// EventBuffer::setBufferSize --
//

void EventBuffer::setBufferSize(int aSize) {
   if (aSize <= 0) {
      cout << "Invalid EventBuffer size: " << aSize << endl;
      exit(1);
   }

   storageSize = aSize;
   if (eventStorage != NULL) {
      delete [] eventStorage;
   }
   eventStorage = new Event[storageSize];
   if (eventList != NULL) {
      delete [] eventList;
   }

   eventList = new _EBPrivate[storageSize + 1];
   freeSlots.setSize(storageSize);
   reset();
}



//////////////////////////////
//
// EventBuffer::setPollPeriod --
//

void EventBuffer::setPollPeriod(double aPeriod) {
   pollTimer.setPeriod(aPeriod);
}



///////////////////////////////////////////////////////////////////////////
//
// protected functions
//

//////////////////////////////
//
// EventBuffer::removeEvent -- take an event out of the linked list.
//    and make its location reuseable.
//

void EventBuffer::removeEvent(int index) {
   if (index < 0 || index >= storageSize) {
      cout << "Error: cannot remove event " << index << endl;
      exit(1);
   }
 
   eventList[eventList[index].next].last = eventList[index].last;
   eventList[eventList[index].last].next = eventList[index].next;

   eventList[index].next = storageSize;
   eventList[index].last = storageSize;

   freeSlots.insert(index);
}



// md5sum: 3560058918ace3d8710541828823e2f9 EventBuffer.cpp [20050403]
