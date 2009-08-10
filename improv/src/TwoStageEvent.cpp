//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Sep  5 22:00:43 GMT-0800 1997
// Last Modified: Fri Jan 16 21:08:04 GMT-0800 1998
// Last Modified: Tue Nov 10 14:29:59 PST 1998
// Filename:      .../sig/maint//code/control/Event/TwoStageEvent.cpp
// Web Address:   http://sig.sapp.org/src/sig/TwoStageEvent.cpp
// Syntax:        C++ 
//
// Description:   For storage in the EventBuffer class.
//

#include "TwoStageEvent.h"


//////////////////////////////
//
// TwoStageEvent::TwoStageEvent
//

TwoStageEvent::TwoStageEvent(void) {
   setType(EVENT_TWOSTAGE);
}



//////////////////////////////
//
// TwoStageEvent::~TwoStageEvent
//

TwoStageEvent::~TwoStageEvent() {
   // does nothing
}



//////////////////////////////
//
// TwoStageEvent::action
//

void TwoStageEvent::action(EventBuffer& midiOutput) {
   switch ((getType() >> 3) << 3) {
      case EVENT_TWOSTAGE_NOTE:
         ((NoteEvent*)(this))->NoteEvent::action(midiOutput);
         break;
      default:
         break;
   }
}


void TwoStageEvent::action(EventBuffer* midiOutput) {
   action(*midiOutput);
}



//////////////////////////////
//
// TwoStageEvent::getDur
//

int TwoStageEvent::getDur(void) const {
   return getTime2();
}



//////////////////////////////
//
// TwoStageEvent::getDuration --
//

int TwoStageEvent::getDuration(void) const {
   return getTime2();
}



//////////////////////////////
//
// TwoStageEvent::getOffTime --
//

int TwoStageEvent::getOffTime(void) const {
   return getTime1() + getTime2();
}



//////////////////////////////
//
// TwoStageEvent::getOnTime --
//

int TwoStageEvent::getOnTime(void) const {
   return getTime1();
}



//////////////////////////////
//
// TwoStageEvent::off --
//

void TwoStageEvent::off(EventBuffer& midiOutput) {
   switch (getType() >> 3) {
      case EVENT_TWOSTAGE_NOTE:
         ((NoteEvent*)(this))->off(midiOutput);
         break;
      default:
          setStatus(EVENT_STATUS_OFF);
   }
}


void TwoStageEvent::off(EventBuffer* midiOutput) {
   off(*midiOutput);
}



//////////////////////////////
//
// TwoStageEvent::print
//

void TwoStageEvent::print(void) const { }



//////////////////////////////
//
// TwoStageEvent::setDur -- set the duration of the event
//

void TwoStageEvent::setDur(int aDuration) {
   setTime2(aDuration);
}



//////////////////////////////
//
// TwoStageEvent::setDuration -- set the duration of the event
//

void TwoStageEvent::setDuration(int aDuration) {
   setTime2(aDuration);
}



//////////////////////////////
//
// TwoStageEvent::setOffDur -- set the off time and the duration
//

void TwoStageEvent::setOffDur(int aTime, int aDuration) {
   setDur(aDuration);
   setOnTime(aTime - aDuration);
}



//////////////////////////////
//
// TwoStageEvent::setOff -- sets off time of the event
//

void TwoStageEvent::setOff(int aTime) {
   setDur(aTime - getTime1());
}



//////////////////////////////
//
// TwoStageEvent::setOnDur -- set the on time and the duration
//

void TwoStageEvent::setOnDur(int aTime, int aDuration) {
   setOnTime(aTime);
   setDur(aDuration);
}


//////////////////////////////
//
// TwoStageEvent:setOnTime --
//

void TwoStageEvent::setOnTime(int aTime) {
   setTime1(aTime);
}

         

// md5sum: 9f8be7db7a4b6ece02f004bccb86b6a8 TwoStageEvent.cpp [20050403]
