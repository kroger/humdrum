//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Sep  5 22:00:43 GMT-0800 1997
// Last Modified: Fri Jan 16 20:56:18 GMT-0800 1998
// Last Modified: Thu Nov  5 12:21:23 PST 1998
// Last Modified: Sun Jun 11 14:26:51 PDT 2000 (added floatValue() function)
// Filename:      ...sig/src/sigControl/Event/Event.cpp
// Web Address:   http://sig.sapp.org/src/sig/Event.cpp
// Syntax:        C++ 
//
// Description:   A base class for "events" for storage in the 
//                EventBuffer class.  This class stores performance
//                data for MIDI events to be performed in the future.
//

#include "Event.h"


//////////////////////////////
//
// Event::Event --
//

Event::Event(void) {
   // does nothing. Initial contents of an Event are undefined
}



//////////////////////////////
//
// Event::~Event --
//

Event::~Event() {
   // does nothing
}



//////////////////////////////
//
// Event::action -- must be defined in inherited class
//

void Event::action(EventBuffer& midiOutput) {
   switch (getType() & 0x7) {
      case EVENT_ONESTAGE:   
         ((OneStageEvent*)(this))->OneStageEvent::action(midiOutput);   
         break;
      case EVENT_TWOSTAGE:   
         ((TwoStageEvent*)(this))->TwoStageEvent::action(midiOutput);   
         break;
      case EVENT_MULTISTAGE:   
         ((MultiStageEvent*)(this))->MultiStageEvent::action(midiOutput);   
         break;
      default:  // don't know what it is, so kill the thing:
         setStatus(EVENT_STATUS_OFF);
   }
}


void Event::action(EventBuffer* midiOutput) {
   action(*midiOutput);
}


/////////////////////////////
//
// Event::activate -- set the status on alive, but not yet on.
//

void Event::activate(void) {
   setStatus(EVENT_STATUS_ACTIVE);
}


//////////////////////////////
//
// Event::getActionTime -- time at which to perform an 
//    operation with the event data.
//

int Event::getActionTime(void) const {
   switch (getType() & 0x07) {
      case EVENT_ONESTAGE:           
         // no break
      case EVENT_MULTISTAGE:
         return getTime1();
         break;
      case EVENT_TWOSTAGE:
         switch (getStatus()) {
            case EVENT_STATUS_ACTIVE:
               return getTime1();
               break;
            case EVENT_STATUS_ON:
               return getTime2() + getTime1();
               break;
         }
         break;
   }

   // some sort of error, make the action time very long
   return 0x7fffffff;
}



//////////////////////////////
//
// Event::getGroup  -- returns the group number assiciated with
//     the Event.
//

int Event::getGroup(void) const {
   return (int)*((ushort*)&data[2]);
}



//////////////////////////////
//
// getP0 -- return zeroth parameter
//

int Event::getP0(void) const {
   return (int)data[8];
}



//////////////////////////////
//
// getP1 -- return zeroth parameter
//

int Event::getP1(void) const {
   return (int)data[9];
}



//////////////////////////////
//
// getP2 -- return zeroth parameter
//

int Event::getP2(void) const {
   return (int)data[10];
}



//////////////////////////////
//
// getP3 -- return zeroth parameter
//

int Event::getP3(void) const {
   return (int)data[11];
}



//////////////////////////////
//
// Event::getStatus -- returns true if alive, false if dead
//

int Event::getStatus(void) const {
   return (int)(data[1] & 0x03);
}



//////////////////////////////
//
// Event::getTime1 -- returns first time variable
//

int Event::getTime1(void) const {
   return  (int)(*((long*)&data[4]));
}



//////////////////////////////
//
// Event::getTime2 -- returns first time variable
//

int Event::getTime2(void) const {
   return  (int)(*((long*)&data[12]));
}



//////////////////////////////
//
// Event::getType  -- returns the type of Event that the object is.
//    OneStageEvent: bit(0)  is turned on
//    OneStageEvent: bit(1)  is turned on
//    OneStageEvent: bit(2)  is turned on
//    bits 3-7 give the subtype in each of the above three categories.
//

int Event::getType(void) const {
   return (int)data[0];
}



//////////////////////////////
//
// Event::intValue -- returns storage space to an int in the 
//    Event extended storage.
//

int& Event::intValue(int index) {
   if (index < 0 || index > 12) {
      cout << "Error: to large an index for Event::intValue: " 
           << index << endl;
      exit(1);
   }

   return *(int*)(&data[16 + index]);
}

//////////////////////////////
//
// Event::floatValue -- returns storage space to an int in the 
//    Event extended storage.
//

float& Event::floatValue(int index) {
   if (index < 0 || index > 12) {
      cout << "Error: to large an index for Event::floatValue: " 
           << index << endl;
      exit(1);
   }

   return *(float*)(&data[16 + index]);
}



//////////////////////////////
//
// Event::shortValue -- returns storage space to a short in the 
//    Event extended storage.
//

short& Event::shortValue(int index) {
   if (index < 0 || index > 14) {
      cout << "Error: to large an index for Event::shortValue: " 
           << index << endl;
      exit(1);
   }

   return *(short*)(&data[16 + index]);
}



//////////////////////////////
//
// Event::charValue -- returns storage space to a char in the 
//    Event extended storage.
//

char& Event::charValue(int index) {
   if (index < 0 || index > 15) {
      cout << "Error: to large an index for Event::charValue: " 
           << index << endl;
      exit(1);
   }

   return *(char*)(&data[16 + index]);
}



//////////////////////////////
//
// Event::isdead -- returns true if event is inactive.
//

int Event::isdead(void) const {
   return !getStatus();
}



//////////////////////////////
//
// Event::kill -- how to stop an event.
//

// will kill the Event if aGroup matches the Event's group ID:

void Event::kill(int aGroup, EventBuffer& midiOutput) {
   if (aGroup == getGroup()) {
      off(midiOutput);
   }
}


void Event::kill(int aGroup, EventBuffer* midiOutput) {
   kill(aGroup, *midiOutput);
}



//////////////////////////////
//
// Event::off -- turn off the Event
//

void Event::off(EventBuffer& midiOutput) {
   switch (getType() & 0x7) {
      case EVENT_ONESTAGE:   
         ((OneStageEvent*)(this))->off(midiOutput);
         break;
      case EVENT_TWOSTAGE:   
         ((TwoStageEvent*)(this))->off(midiOutput);
         break;
      case EVENT_MULTISTAGE:   
         ((MultiStageEvent*)(this))->off(midiOutput);
         break;
      default:  // don't know what it is, so just turn if off
         setStatus(EVENT_STATUS_OFF);
   }
}


void Event::off(EventBuffer* midiOutput) {
   off(*midiOutput);
}



//////////////////////////////
//
// Event::operator=
//

Event& Event::operator=(const Event& anEvent) {
   // don't copy onto self:
   if (&anEvent == this) {
      return *this;
   }

   for (long i=0; i<3; i++) {
     ((ulong*)(&data))[i] = ((ulong*)&anEvent.data)[i];
   }

   return *this;
}



//////////////////////////////
//
// p0 -- return zeroth parameter
//

uchar& Event::p0(void) {
   return data[8];
}



//////////////////////////////
//
// p1 -- return zeroth parameter
//

uchar& Event::p1(void) {
   return data[9];
}



//////////////////////////////
//
// p2 -- return zeroth parameter
//

uchar& Event::p2(void) {
   return data[10];
}



//////////////////////////////
//
// p3 -- return zeroth parameter
//

uchar& Event::p3(void) {
   return data[11];
}



//////////////////////////////
//
// Event::print -- print the data elements (never used)
//

void Event::print(void) {
   cout << "Event Type    = " << getType() << '\n';
   cout << "Status Byte 1 = "; printBits(data[4]); cout << '\n';
   cout << "Event Group   = " << getGroup() << '\n';
   cout << "Time 1        = " << *((double*)(&data[8])) << '\n';
   cout << "Time 2        = " << *((double*)(&data[16])) << '\n';
   cout << "byte[16]      = " << (int) data[16] << '\n';
   cout << "byte[17]      = " << (int) data[17] << '\n';
   cout << "byte[18]      = " << (int) data[18] << '\n';
   cout << "byte[19]      = " << (int) data[19] << '\n';
   cout << "byte[20]      = " << (int) data[20] << '\n';
   cout << "byte[21]      = " << (int) data[21] << '\n';
   cout << "byte[22]      = " << (int) data[22] << '\n';
   cout << "byte[23]      = " << (int) data[23] << endl;
}



//////////////////////////////
//
// Event::setGroup 
//

void Event::setGroup(int aGroup) {
   *((short*)(&data[2])) = (short)aGroup;
}



//////////////////////////////
//
// setP0 -- sets zeroth parameter
//

void Event::setP0(int aValue) {
   data[8] = (uchar)aValue;
}



//////////////////////////////
//
// setP1 -- sets zeroth parameter
//

void Event::setP1(int aValue) {
   data[9] = (uchar)aValue;
}



//////////////////////////////
//
// setP2 -- sets zeroth parameter
//

void Event::setP2(int aValue) {
   data[10] = (uchar)aValue;
}



//////////////////////////////
//
// setP3 -- sets zeroth parameter
//

void Event::setP3(int aValue) {
   data[11] = (uchar)aValue;
}



//////////////////////////////
//
// Event::setTime1 -- sets the first time variable
//

void Event::setTime1(int aTime) {
   *(long*)&data[4] = (long)aTime;
}



//////////////////////////////
//
// Event::setTime2 -- sets the second time variable
//

void Event::setTime2(int aTime) {
   *(long*)&data[12] = (long)aTime;
}



//////////////////////////////
//
// Event::setType 
//
 
void Event::setType(int aType) {
   data[0] = (uchar)aType;
}





///////////////////////////////////////////////////////////////////////////
//
// Private Functions
//


//////////////////////////////
//
// Event::printBits == prints a byte in binary form
//

void Event::printBits(uchar aByte, ostream& output) const {
   for (int i=7; i<=0; i++) {
      if (aByte & (1 << i)) {
         output << '1';
      } else {
         output << '0';
      }
   }
}



//////////////////////////////
//
// Event::setStatus == prints a byte in binary form
//

void Event::setStatus(int aStatus) {
   data[1] = (uchar)aStatus;
}   



// md5sum: f6a317fbb83bc90efcf10cb690fc5b07 Event.cpp [20020518]
