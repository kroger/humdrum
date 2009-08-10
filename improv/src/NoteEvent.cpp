//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Sep  5 22:00:43 GMT-0800 1997
// Last Modified: Sat Jan 17 11:19:25 GMT-0800 1998
// Last Modified: Tue Nov 10 15:08:01 PST 1998
// Filename:      .../control/Event/TwoStageEvent/NoteEvent/NoteEvent.cpp
// Web Address:   http://sig.sapp.org/src/sig/NoteEvent.cpp
// Syntax:        C++ 
//
// Description:   Used in the storage of the EventBuffer class.
//

#include "NoteEvent.h"


//////////////////////////////
//
// NoteEvent::NoteEvent --
//

NoteEvent::NoteEvent(void) {
   setType(EVENT_TWOSTAGE | EVENT_TWOSTAGE_NOTE);
   setChannel(0);
   setKeyno(0);
   setVelocity(127);
   setOffVelocity(127);
}



//////////////////////////////
//
// NoteEvent::~NoteEvent --
//

NoteEvent::~NoteEvent() {
   // does nothing
}



//////////////////////////////
//
// NoteEvent::action --
//

void NoteEvent::action(EventBuffer& midiOutput) {
   switch (getStatus()) {
      case EVENT_STATUS_ACTIVE:
         midiOutput.play(getChannel(), getKeyno(), getVelocity());
         setStatus(EVENT_STATUS_ON);
         break;
      case EVENT_STATUS_ON:
         midiOutput.MidiOutput::off(getChannel(), getKeyno(), getOffVelocity());
         setStatus(EVENT_STATUS_OFF);
         break;
      default:
        break;
   }
}


void NoteEvent::action(EventBuffer* midiOutput) {
   action(*midiOutput);
}



//////////////////////////////
//
// NoteEvent::getChannel --
//

int NoteEvent::getChannel(void) const {
   return getP0();
}



//////////////////////////////
//
// NoteEvent::getChan --
//

int NoteEvent::getChan(void) const {
   return getP0();
}



//////////////////////////////
//
// NoteEvent::getKey --
//

int NoteEvent::getKey(void) const {
   return getP1();
}



//////////////////////////////
//
// NoteEvent::getKeyno --
//

int NoteEvent::getKeyno(void) const {
   return getP1();
}



//////////////////////////////
//
// NoteEvent::getOffVel --
//

int NoteEvent::getOffVel(void) const {
   return getP3();
}



//////////////////////////////
//
// NoteEvent::getOffVelocity --
//

int NoteEvent::getOffVelocity(void) const {
   return getP3();
}



//////////////////////////////
//
// NoteEvent::getVel --
//

int NoteEvent::getVel(void) const {
   return getP2();
}



//////////////////////////////
//
// NoteEvent::getVelocity --
//

int NoteEvent::getVelocity(void) const {
   return getP2();
}



//////////////////////////////
//
// NoteEvent::off --
//

void NoteEvent::off(EventBuffer& midiOutput) {
   switch (getStatus()) {
      case EVENT_STATUS_ACTIVE:
         setStatus(EVENT_STATUS_OFF);
         break;
      case EVENT_STATUS_ON:
         midiOutput.MidiOutput::off(getChannel(), getKeyno(), getOffVelocity());
         setStatus(EVENT_STATUS_OFF);
         break;
      case EVENT_STATUS_OFF:
         break;
      default:
         setStatus(EVENT_STATUS_OFF);
         break;
   }
}


void NoteEvent::off(EventBuffer* midiOutput) {
   off(*midiOutput);
}



//////////////////////////////
//
// NoteEvent::print --
//

void NoteEvent::print(void) { }



//////////////////////////////
//
// NoteEvent::setChan --
//	default value: aChannel = 0
//

void NoteEvent::setChan(int aChannel) {
   p0() = (uchar)aChannel;
}



//////////////////////////////
//
// NoteEvent::setChannel --
//	default value: aChannel = 0
//

void NoteEvent::setChannel(int aChannel) {
   p0() = (uchar)aChannel;
}



//////////////////////////////
//
// NoteEvent::setKey --
//	default value: aKey = 0
//

void NoteEvent::setKey(int aKey) {
   p1() = (uchar)aKey;
}



//////////////////////////////
//
// NoteEvent::setKeyno --
//	default value: aKey = 0
//

void NoteEvent::setKeyno(int aKey) {
   p1() = (uchar)aKey;
}



//////////////////////////////
//
// NoteEvent::setVel --
//	default value: aVelocity = 127
//

void NoteEvent::setVel(int aVelocity) {
   p2() = (uchar)aVelocity;
}



//////////////////////////////
//
// NoteEvent::setVelocity --
//	default value: aVelocity = 127
//

void NoteEvent::setVelocity(int aVelocity) {
   p2() = (uchar)aVelocity;
}



//////////////////////////////
//
// NoteEvent::setOffVel --
//	default value: aVelocity = 127
//

void NoteEvent::setOffVel(int aVelocity) {
   p3() = (uchar)aVelocity;
}



//////////////////////////////
//
// NoteEvent::setOffVelocity --
//	default value: aVelocity = 127
//

void NoteEvent::setOffVelocity(int aVelocity) {
   p3() = (uchar)aVelocity;
}



// md5sum: d912583d72b80c3c49533145f7890aa2 NoteEvent.cpp [20020518]
