//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Sep  5 22:00:43 GMT-0800 1997
// Last Modified: Sat Jan 17 11:19:25 GMT-0800 1998
// Last Modified: Tue Nov 10 16:31:54 PST 1998
// Filename:      .../control/Event/MultiStageEvent/FunctionEvent.cpp
// Web Address:   http://sig.sapp.org/src/sig/FunctionEvent.cpp
// Syntax:        C++ 
//
// Description    For storage in the EventBuffer class.  This class is
//                used to store a pointer to a function which will be
//                called at a specific time, usually to generate a 
//                MIDI note.
//

#include "FunctionEvent.h"


//////////////////////////////
//
// FunctionEvent::FunctionEvent --
//

FunctionEvent::FunctionEvent(void) {
   setType(EVENT_MULTISTAGE | EVENT_MULTISTAGE_FUNCTION);
   setFunction(NULL);
}



//////////////////////////////
//
// FunctionEvent::~FunctionEvent --
//

FunctionEvent::~FunctionEvent() {
   // does nothing
}



//////////////////////////////
//
// FunctionEvent::action --
//

void FunctionEvent::action(EventBuffer& midiOutput) {
   switch (getStatus()) {
      case EVENT_STATUS_ACTIVE:
      case EVENT_STATUS_ON:
         getFunction()(*this, midiOutput);
         break;
      default:
        break;
   }
}


void FunctionEvent::action(EventBuffer* midiOutput) {
   action(*midiOutput);
}



//////////////////////////////
//
// FunctionEvent::getChan -- returns the channel setting
//

int FunctionEvent::getChan(void) const {
   return getP0();
}



//////////////////////////////
//
// FunctionEvent::getChannel -- returns the channel setting
//

int FunctionEvent::getChannel(void) const {
   return getP0();
}



//////////////////////////////
//
// FunctionEvent::getFunction --
//

Algorithm FunctionEvent::getFunction(void) const {
   return *((Algorithm*)(&data[16]));
}



//////////////////////////////
//
// FunctionEvent::getKey --
//

int FunctionEvent::getKey(void) const {
   return getP1();
}



//////////////////////////////
//
// FunctionEvent::getKeyno --
//

int FunctionEvent::getKeyno(void) const {
   return getP1();
}



//////////////////////////////
//
// FunctionEvent::getOffVel --
//

int FunctionEvent::getOffVel(void) const {
   return getP3();
}



//////////////////////////////
//
// FunctionEvent::getOffVelocity --
//

int FunctionEvent::getOffVelocity(void) const {
   return getP3();
}



//////////////////////////////
//
// FunctionEvent::getVel --
//

int FunctionEvent::getVel(void) const {
   return getP2();
}



//////////////////////////////
//
// FunctionEvent::getVelocity --
//

int FunctionEvent::getVelocity(void) const {
   return getP2();
}



//////////////////////////////
//
// FunctionEvent::off  --
//

void FunctionEvent::off(EventBuffer& midiOutput) {
   switch (getStatus()) {
      case EVENT_STATUS_ACTIVE:
      case EVENT_STATUS_ON:
      case EVENT_STATUS_OFF:
      default:
         setStatus(EVENT_STATUS_OFF);
         break;
   }
}


void FunctionEvent::off(EventBuffer* midiOutput) {
   off(*midiOutput);
}



//////////////////////////////
//
// FunctionEvent::print --
//

void FunctionEvent::print(void) { }



//////////////////////////////
//
// FunctionEvent::setChan --
//     default value: aChannel = 0
//

void FunctionEvent::setChan(int aChannel) {
   setP0(aChannel);
}



//////////////////////////////
//
// FunctionEvent::setChannel --
//     default value: aChannel = 0
//

void FunctionEvent::setChannel(int aChannel) {
   setP0(aChannel);
}



//////////////////////////////
//
// FunctionEvent::setFunction --
//

void FunctionEvent::setFunction(Algorithm aFunction) {
   *((Algorithm*)&data[16]) = aFunction;
}



//////////////////////////////
//
// FunctionEvent::setKey --
//	default value: aKey = 0
//

void FunctionEvent::setKey(int aKey) {
   p1() = (uchar)aKey;
}



//////////////////////////////
//
// FunctionEvent::setKeyno --
//	default value: aKey = 0
//

void FunctionEvent::setKeyno(int aKey) {
   p1() = (uchar)aKey;
}



//////////////////////////////
//
// FunctionEvent::setVel
//	default value: aVelocity = 127
//

void FunctionEvent::setVel(int aVelocity) {
   p2() = (uchar)aVelocity;
}



//////////////////////////////
//
// FunctionEvent::setVelocity
//	default value: aVelocity = 127
//

void FunctionEvent::setVelocity(int aVelocity) {
   p2() = (uchar)aVelocity;
}



// md5sum: 686e77acb5e1b317f3e5336abb842ebc FunctionEvent.cpp [20020518]
