//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Nov  2 23:39:21 PST 1996
// Last Modified: Tue Jan 27 22:50:00 GMT-0800 1998
// Last Modified: Thu Apr 27 17:05:57 PDT 2000
// Filename:      ...sig/code/misc/sigControl.h
// Web Address:   http://sig.sapp.org/include/sig/sigControl.h
// Syntax:        C++ 
//
// Description:   Includes all of the header files for using the sigControl
//                subLibrary.
//

#ifndef _SIGCONTROL_H_INCLUDED
#define _SIGCONTROL_H_INCLUDED

// include MIDI definitions (channels, instruments, notenames):
#include "mididefines.h"


// include headers for control classes
#include "SigTimer.h"
#include "Idler.h"
#include "MidiMessage.h"
#include "MidiOutPort_visual.h"
#include "MidiOutPort_unsupported.h"
#include "MidiOutPort.h"
#include "MidiOutput.h"
#include "MidiInPort_visual.h"
#include "MidiInPort_unsupported.h"
#include "MidiInPort.h"
#include "MidiInput.h"
#include "MidiPort.h"
#include "MidiIO.h"
#include "RadioBaton.h"
#include "AdamsStick.h"
#include "Synthesizer.h"
#include "Voice.h"
#include "KeyboardInput.h"

#include "MidiPerform.h"

// Event classes
#include "EventBuffer.h"
#include "NoteEvent.h"
#include "FunctionEvent.h"


#endif  /* _SIGCONTROL_H_INCLUDED */



// md5sum: 75e7b04bf4dd5d727a8c9cefcb362b6a sigControl.h [20020518]
