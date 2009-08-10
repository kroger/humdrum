//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Sep  5 22:00:43 GMT-0800 1997
// Last Modified: Sat Dec  6 22:24:47 GMT-0800 1997
// Last Modified: Mon Nov  9 13:28:13 PST 1998
// Filename:      ...sig/src/control/Event/OneStageEvent/OneStageEvent.cpp
// Web Address:   http://sig.sapp.org/src/sig/OneStageEvent.cpp
// Syntax:        C++ 
//
// Description:   Used in the storage of the EventBuffer class.
//

#include "OneStageEvent.h"


//////////////////////////////
//
// OneStageEvent::OneStageEvent --
//

OneStageEvent::OneStageEvent(void) {
   // does nothing
}



//////////////////////////////
//
// OneStageEvent::~OneStageEvent --
//

OneStageEvent::~OneStageEvent() {
   // does nothing
}



//////////////////////////////
//
// OneStageEvent::action --
//

void OneStageEvent::action(EventBuffer& midiOutput) {
   switch (getType() >> 3) {
      case EVENT_ONESTAGE_MIDI:
         ((MidiEvent*)this)->action(midiOutput);
         break;
      default:
         break;
   }
}


void OneStageEvent::action(EventBuffer* midiOutput) {
   action(*midiOutput);
}



//////////////////////////////
//
// OneStageEvent::getActionTime -- returns the On/Destroy Time.
//

int OneStageEvent::getActionTime(void) const {
   return getTime1();
}



//////////////////////////////
//
// OneStageEvent::getTime -- returns the On/Destroy Time.
//

int OneStageEvent::getTime(void) const {
   return getTime1();
}



//////////////////////////////
//
// OneStageEvent::off
//

void OneStageEvent::off(EventBuffer& midiOutput) {
   setStatus(EVENT_STATUS_OFF);
}


void OneStageEvent::off(EventBuffer* midiOutput) {
   off(*midiOutput);
}




//////////////////////////////
//
// OneStageEvent::setTime -- sets the On/Destroy Time.
//

void OneStageEvent::setTime(int aTime) {
   setTime1(aTime);
}



// md5sum: cd768942c0263bd5047b3282fb335b74 OneStageEvent.cpp [20020518]
