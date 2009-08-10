//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Nov 10 15:25:18 PST 1998
// Last Modified: Tue Nov 10 15:25:22 PST 1998
// Filename:      .../sig/maint/code/control/Event/MultiStageEvent.cpp
// Web Address:   http://sig.sapp.org/src/sig/MultiStageEvent.cpp
// Syntax:        C++ 
//
// Description:   Used in the storage of the EventBuffer class.
//

#include "MultiStageEvent.h"


//////////////////////////////
//
// MultiStageEvent::MultiStageEvent
//

MultiStageEvent::MultiStageEvent(void) {
   setType(EVENT_MULTISTAGE);
}



//////////////////////////////
//
// MultiStageEvent::~MultiStageEvent
//

MultiStageEvent::~MultiStageEvent() {
   // does nothing
}



//////////////////////////////
//
// MultiStageEvent::action
//

void MultiStageEvent::action(EventBuffer& midiOutput) {
     switch ((getType() >> 3) << 3) {
        case EVENT_MULTISTAGE_FUNCTION:
           ((FunctionEvent*)(this))->FunctionEvent::action(midiOutput);
           break;
      default:
         break;
   }
  
}


void MultiStageEvent::action(EventBuffer* midiOutput) {
   action(*midiOutput);
}



//////////////////////////////
//
// MultiStageEvent::getDur -- gets the duration of the event
//

int MultiStageEvent::getDur(void) const {
   return getTime2();
}



//////////////////////////////
//
// MultiStageEvent::getDuration -- gets the duration of the event
//

int MultiStageEvent::getDuration(void) const {
   return getTime2();
}



//////////////////////////////
//
// MultiStageEvent::getOffTime -- 
//

int MultiStageEvent::getOffTime(void) const {
   return getTime2();
}



//////////////////////////////
//
// MultiStageEvent::getOnTime -- 
//

int MultiStageEvent::getOnTime(void) const {
   return getTime1();
}



//////////////////////////////
//
// MultiStageEvent::off()
//

void MultiStageEvent::off(EventBuffer& midiOutput) {
//   switch (getType() >> 3) {
//      case EVENT_TWOSTAGE_NOTE:
//         ((NoteEvent*)(this))->off(EventBuffer& midiOutput
//         break;
//      default:
          setStatus(EVENT_STATUS_OFF);
//   }
}

void MultiStageEvent::off(EventBuffer* midiOutput) {
   off(*midiOutput);
}



//////////////////////////////
//
// MultiStageEvent::print
//

void MultiStageEvent::print(void) const { }



//////////////////////////////
//
// MultiStageEvent::setDur -- set the duration of the event
//

void MultiStageEvent::setDur(int aDuration) {
   setTime2(aDuration);
}



//////////////////////////////
//
// MultiStageEvent::setDuration -- set the duration of the event
//

void MultiStageEvent::setDuration(int aDuration) {
   setTime2(aDuration);
}



//////////////////////////////
//
// MultiStageEvent::setOnDur -- set the on time and the duration
//

void MultiStageEvent::setOnDur(int aTime, int aDuration) {
   setOnTime(aTime);
   setDur(aDuration);
}



//////////////////////////////
//
// MultiStageEvent:setOnTime --
//

void MultiStageEvent::setOnTime(int aTime) {
   setTime1(aTime);
}



// md5sum: 838978dbe40a876cf3c29fb857a93ce6 MultiStageEvent.cpp [20050403]
