//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Sep  5 22:00:43 GMT-0800 1997
// Last Modified: Sat Dec  6 22:24:47 GMT-0800 1997
// Last Modified: Mon Nov  9 13:28:13 PST 1998
// Filename:      ...sig/maint/code/control/Event/OneStageEvent/MidiEvent.cpp
// Web Address:   http://sig.sapp.org/src/sig/MidiEvent.cpp
// Syntax:        C++ 
//
// Description:   For use in the EventBuffer class.  Stores the
//                performance data for a MIDI event.
//

#include "MidiEvent.h"


//////////////////////////////
//
// MidiEvent::MidiEvent
//

MidiEvent::MidiEvent(void) {
   // does nothing
}



//////////////////////////////
//
// MidiEvent::~MidiEvent
//

MidiEvent::~MidiEvent() {
   // does nothing
}



//////////////////////////////
//
// MidiEvent::action
//

void MidiEvent::action(EventBuffer& midiOutput) {
   midiOutput.send(p0(), p1(), p2());
   off();
}


void MidiEvent::action(EventBuffer* midiOutput) {
   action(*midiOutput);
}



//////////////////////////////
//
// MidiEvent::off -- turn off the event (i.e., don't play it)
//

void MidiEvent::off(void) {
   setStatus(0);
}



// md5sum: 3271929605d981e121ee3ef33722432b MidiEvent.cpp [20020518]
