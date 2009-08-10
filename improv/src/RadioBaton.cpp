//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 21 December 1997
// Last Modified: Wed Feb 11 22:43:15 GMT-0800 1998
// Last Modified: Wed Sep 30 13:22:51 PDT 1998
// Last Modified: Mon Mar 15 04:08:39 PST 1999
// Last Modified: Mon Nov 29 13:44:52 PST 1999 (name RadioDrum->RadioBaton)
// Last Modified: Thu Apr 27 17:59:04 PDT 2000 (readded scale and change fns)
// Last Modified: Sun Oct  1 15:19:13 PDT 2000 (revised for firmware "AE")
// Filename:      ...sig/code/control/RadioBaton/RadioBaton.cpp
// Web Address:   http://sig.sapp.org/include/sig/RadioBaton.cpp
// Syntax:        C++
//
// Description:   A structure for handling the Radio Baton variables
//                and the MIDI connections from the computer to 
//                the Radio Batons.
//

#include "RadioBaton.h"

#include <ctype.h>

#ifndef OLDCPP
   #include <fstream>
   #include <iomanip>
   using namespace std;
#else
   #include <fstream.h>
   #include <iomanip.h>
#endif

#ifndef VISUAL
   #include <unistd.h>
#endif


//////////////////////////////
// 
// RadioBaton::RadioBaton --
//

RadioBaton::RadioBaton(void) : MidiIO() {
   setStateSize(DEFAULT_STATE_SIZE);
   for (int i=0; i<16; i++) {
      completeBufQ[i] = 0;
   }
   reportingQ = 0;
   recordStateQ = 0;
   storedReportStatus = 0;
   errorQ = 0;
   
   stick1trig = RadioBatonEmptyBehavior;
   stick2trig = RadioBatonEmptyBehavior;
   stick1position = RadioBatonEmptyBehavior;
   stick2position = RadioBatonEmptyBehavior;

   dial1position = RadioBatonEmptyBehavior;
   dial2position = RadioBatonEmptyBehavior;
   dial3position = RadioBatonEmptyBehavior;
   dial4position = RadioBatonEmptyBehavior;

   b14plustrig = RadioBatonEmptyBehavior;
   b15plustrig = RadioBatonEmptyBehavior;
   b14minusuptrig = RadioBatonEmptyBehavior;
   b14minusdowntrig = RadioBatonEmptyBehavior;
   b15minusuptrig = RadioBatonEmptyBehavior;
   b15minusdowntrig = RadioBatonEmptyBehavior;

   xDirection = 1;
   yDirection = 1;
   zDirection = 1;
}


RadioBaton::RadioBaton(int outputDevice, int inputDevice) :
      MidiIO(outputDevice, inputDevice) {
   setStateSize(DEFAULT_STATE_SIZE);
   for (int i=0; i< 16; i++) {
      completeBufQ[i] = 0;
   }
   reportingQ = 0;
   recordStateQ = 0;
   storedReportStatus = 0;
   errorQ = 0;
   
   stick1trig = RadioBatonEmptyBehavior;
   stick2trig = RadioBatonEmptyBehavior;
   stick1position = RadioBatonEmptyBehavior;
   stick2position = RadioBatonEmptyBehavior;

   dial1position = RadioBatonEmptyBehavior;
   dial2position = RadioBatonEmptyBehavior;
   dial3position = RadioBatonEmptyBehavior;
   dial4position = RadioBatonEmptyBehavior;

   b14plustrig = RadioBatonEmptyBehavior;
   b15plustrig = RadioBatonEmptyBehavior;
   b14minusuptrig = RadioBatonEmptyBehavior;
   b14minusdowntrig = RadioBatonEmptyBehavior;
   b15minusuptrig = RadioBatonEmptyBehavior;
   b15minusdowntrig = RadioBatonEmptyBehavior;

   xDirection = 1;
   yDirection = 1;
   zDirection = 1;
}



//////////////////////////////
//
// RadioBaton::~RadioBaton --
//

RadioBaton::~RadioBaton() {
   if (recordStateQ == 1) {
      recordStateStop();
      recordStateQ = 0;
   }
}



//////////////////////////////
//
// RadioBaton::getBuf -- returns the currently
//    stored value for the given buffer.  Returns
//    -1 if the radio baton has not completed sending the 
//    value data, which is a 12-bit unsigned value
//    sent in 4 nibbles (most significant nibble is always 0).
//    Will also return -1 if an invalid array location is
//    accessed.
//

int RadioBaton::getBuf(int index) {
   if (index < 0 || index > 15) {
      cerr << "Invalid array index for buf: " << index << endl;
      return -1;
   }

   if (completeBufQ[index] == 0) {
      return -1;
   } else {
      return buf[index];
   }
}



//////////////////////////////
//
// RadioBaton::getError -- return the state of the 
//    MIDI input error printing value
//

int RadioBaton::getError(void) const {
   return errorQ;
}



//////////////////////////////
//
// RadioBaton::getPositionReporting --
//

int RadioBaton::getPositionReporting(void) const {
   return reportingQ;
}



//////////////////////////////
//
// RadioBaton::getReportStatus --
//

int RadioBaton::getReportStatus(void) const {
   return reportingQ;
}



//////////////////////////////
//
// RadioBaton::getWhack1Time -- returns the 
//    most recent trigger time of stick 1.
//

long RadioBaton::getWhack1Time(void) const {
   return t1t;
}



//////////////////////////////
//
// RadioBaton::getWhack2Time -- returns the more recent trigger 
//     time of stick 2
//

long RadioBaton::getWhack2Time(void) const {
   return t2t;
}



//////////////////////////////
//
// RadioBaton::getXaxisDirection -- returns the axis direction
//     of the x-axis (1=default, -1=reverse of default)
//

int RadioBaton::getXaxisDirection(void) const {
   return xDirection;
}



//////////////////////////////
//
// RadioBaton::getYaxisDirection -- returns the axis direction
//     of the y-axis (1=default, -1=reverse of default)
//

int RadioBaton::getYaxisDirection(void) const {
   return yDirection;
}



//////////////////////////////
//
// RadioBaton::getZaxisDirection -- returns the axis direction
//     of the z-axis (1=default, -1=reverse of default)
//

int RadioBaton::getZaxisDirection(void) const {
   return zDirection;
}



//////////////////////////////
//
// RadioBaton::positionReportingOff -- returns the previously
//     set state of position reporting.
//

int RadioBaton::positionReportingOff(void) {
   int oldState = reportingQ;
   stoprunningstatus();
   rawsend(BAT_MIDI_COMMAND, REPORT_POS, POS_REPORT_OFF);
   reportingQ = 0;
   return oldState;
}



//////////////////////////////
// 
// RadioBaton::positionReportingOn -- returns the previously
//     set state of position reporting.
//

int RadioBaton::positionReportingOn(void) { 
   int oldState = reportingQ;
   stoprunningstatus();
   rawsend(BAT_MIDI_COMMAND, REPORT_POS, POS_REPORT_ON);
   reportingQ = 1;
   return oldState;
}



//////////////////////////////
//
// RadioBaton::processIncomingMessages --
// 

void RadioBaton::processIncomingMessages(void) {
   while (MidiInput::getCount() > 0) {
      interpretCommand(MidiInput::extract());
   }
}



//////////////////////////////
//
// RadioBaton::recordingQ -- returns true if currently recording,
//	false if not recording.
//

int RadioBaton::recordingQ(void) const {
   return recordStateQ;
}



//////////////////////////////
//
// RadioBaton::recordStateStart -- 
//

void RadioBaton::recordStateStart(const char* aFileName) {
   if (recordStateQ == 1) {
      recordStateStop();
   }

   recordOutput.open(aFileName, ios::out);
   if (!recordOutput) {   // open file failed
      cerr << "Error: cannot open file " << aFileName << endl;
      recordStateQ = 0;
   } else {
      recordStateQ = 1;
      timer.reset();
      recordTimeOffset = -1;
   }
}



//////////////////////////////
//
// RadioBaton::recordStateStop --
//

void RadioBaton::recordStateStop(void) {
   if (recordStateQ == 0)  return;
   if (recordOutput.is_open()) {
      recordOutput.close();
      recordTimeOffset = -1;
      recordStateQ = 0;
   }
}



//////////////////////////////
//
// RadioBaton::sendMessage -- sends a message to the radio drum.
//    Messages are those related to the BAT_SEND set
//    found in baton.h .
//

void RadioBaton::sendMessage(int aMessage) {
   stoprunningstatus();
   rawsend(BAT_MIDI_COMMAND, BAT_SEND, aMessage);
}



//////////////////////////////
//
// RadioBaton::setError -- if true, then will print out
//    unknown MIDI commands which seem to be bad.
//    In other words, if an 0xa0 command byte was 
//    received, and it doesn't match any baton messages,
//    then an error message will be print to cerr.
//    If false, then don't print any message for bad
//    commands.
//

void RadioBaton::setError(int errorStatus) {
   if (errorStatus) {
      errorQ = 1;
   } else {
      errorQ = 0;
   }
}



//////////////////////////////
//
// RadioBaton::setReportStatus --
//

void RadioBaton::setReportStatus(int aStatus) {
   if (aStatus == 0) {
      reportingQ = 0;
      positionReportingOff();
   } else {
      reportingQ = 1;
      positionReportingOn();
   }
}


//////////////////////////////
//
// RadioBaton::setStateSize -- set the length of the
//    histories of each state variable in the object.
//

void RadioBaton::setStateSize(int aSize) {
   if (aSize <= 0) {
      cerr << "Error: state size too small: " << aSize << endl;
      exit(1);
   }

   t1pb.setSize(aSize);    // stick1 position time
   x1pb.setSize(aSize);    // stick1 x-axis position
   y1pb.setSize(aSize);    // stick1 y-axis position
   z1pb.setSize(aSize);    // stick1 z-axis position

   t2pb.setSize(aSize);    // stick2 position time
   x2pb.setSize(aSize);    // stick2 x-axis position
   y2pb.setSize(aSize);    // stick2 y-axis position
   z2pb.setSize(aSize);    // stick2 z-axis position   

   d1pb.setSize(aSize);    // dial1 position
   d2pb.setSize(aSize);    // dial2 position
   d3pb.setSize(aSize);    // dial3 position
   d4pb.setSize(aSize);    // dial4 position

   t1tb.setSize(aSize);    // stick1 trigger time
   x1tb.setSize(aSize);    // stick1 x-axis trigger pos.
   y1tb.setSize(aSize);    // stick1 y-axis trigger pos.
   w1tb.setSize(aSize);    // stick1 wack at trigger time

   t2tb.setSize(aSize);    // stick2 trigger time
   x2tb.setSize(aSize);    // stick2 x-axis trigger pos.
   y2tb.setSize(aSize);    // stick2 y-axis trigger pos.
   w2tb.setSize(aSize);    // stick2 whach at trigger time

   b14ptb.setSize(aSize);  // b14+ button trigger time buffer
   b15ptb.setSize(aSize);  // b15+ button trigger time buffer
   b14mdtb.setSize(aSize); // b14- pedal down trigger time buffer
   b14mutb.setSize(aSize); // b14- pedal up trigger time buffer
   b15mdtb.setSize(aSize); // b15- pedal down trigger time buffer
   b15mutb.setSize(aSize); // b15- pedal up trigger time buffer
}



//////////////////////////////
//
// RadioBaton::setXaxisDirection -- sets the axis direction
//     of the x-axis (1=default, -1=reverse of default)
//

void RadioBaton::setXaxisDirection(int aDirection) {
   if (aDirection > 0) {
      xDirection = 1;
   } else {
      xDirection = -1;
   }
}



//////////////////////////////
//
// RadioBaton::setYaxisDirection -- sets the axis direction
//     of the y-axis (1=default, -1=reverse of default)
//

void RadioBaton::setYaxisDirection(int aDirection) {
   if (aDirection > 0) {
      yDirection = 1;
   } else {
      yDirection = -1;
   }
}



//////////////////////////////
//
// RadioBaton::setZaxisDirection -- sets the axis direction
//     of the z-axis (1=default, -1=reverse of default)
//

void RadioBaton::setZaxisDirection(int aDirection) {
   if (aDirection > 0) {
      zDirection = 1;
   } else {
      zDirection = -1;
   }
}



//////////////////////////////
//
// RadioBaton::toggleXAxisDirection -- switches the axis direction
//     of the x-axis (1=default, -1=reverse of default) and
//     then returns the current direction
//

int RadioBaton::toggleXAxisDirection(void) {
   xDirection = -xDirection;
   return xDirection;
}



//////////////////////////////
//
// RadioBaton::toggleYAxisDirection -- switches the axis direction
//     of the y-axis (1=default, -1=reverse of default) and
//     then returns the current direction
//

int RadioBaton::toggleYAxisDirection(void) {
   yDirection = -yDirection;
   return yDirection;
}



//////////////////////////////
//
// RadioBaton::toggleZAxisDirection -- switches the axis direction
//     of the z-axis (1=default, -1=reverse of default) and
//     then returns the current direction
//

int RadioBaton::toggleZAxisDirection(void) {
   zDirection = -zDirection;
   return zDirection;
}



//////////////////////////////
//
// RadioBaton::toggleError -- switches the printing error
//    status when the class received a MIDI messages it
//    doesn't understand (and probably shouldn't have
//    ignored.)
//

void RadioBaton::toggleError(void) {
   errorQ = !errorQ;
}



//////////////////////////////
//
// RadioBaton::togglePositionReporting --
//

void RadioBaton::togglePositionReporting(void) {
   if (reportingQ) {
      reportingQ = 0;
      positionReportingOff();
   } else {
      reportingQ = 1;
      positionReportingOn();
   }
}



//////////////////////////////
//
// scaling functions -- functions which scale the standard
//      range of 0 to 127 into another range.
//
 
int RadioBaton::x1ps(int min, int max) { return scale((int)x1p, min, max); }
int RadioBaton::y1ps(int min, int max) { return scale((int)y1p, min, max); }
int RadioBaton::z1ps(int min, int max) { return scale((int)z1p, min, max); }
int RadioBaton::x2ps(int min, int max) { return scale((int)x2p, min, max); }
int RadioBaton::y2ps(int min, int max) { return scale((int)y2p, min, max); }
int RadioBaton::z2ps(int min, int max) { return scale((int)z2p, min, max); }
int RadioBaton::x1ts(int min, int max) { return scale((int)x1t, min, max); }
int RadioBaton::y1ts(int min, int max) { return scale((int)y1t, min, max); }
int RadioBaton::x2ts(int min, int max) { return scale((int)x2t, min, max); }
int RadioBaton::y2ts(int min, int max) { return scale((int)y2t, min, max); }
int RadioBaton::d1ps(int min, int max) { return scale((int)d1p, min, max); }
int RadioBaton::d2ps(int min, int max) { return scale((int)d2p, min, max); }
int RadioBaton::d3ps(int min, int max) { return scale((int)d3p, min, max); }
int RadioBaton::d4ps(int min, int max) { return scale((int)d4p, min, max); }

// scale to real numbers
 
double RadioBaton::x1psf(double min, double max) { 
   return MidiInput::fscale((int)x1p, min, max); }
double RadioBaton::y1psf(double min, double max) { 
   return MidiInput::fscale((int)y1p, min, max); }
double RadioBaton::z1psf(double min, double max) { 
   return MidiInput::fscale((int)z1p, min, max); }
double RadioBaton::x2psf(double min, double max) { 
   return MidiInput::fscale((int)x2p, min, max); }
double RadioBaton::y2psf(double min, double max) { 
   return MidiInput::fscale((int)y2p, min, max); }
double RadioBaton::z2psf(double min, double max) { 
   return MidiInput::fscale((int)z2p, min, max); }
double RadioBaton::x1tsf(double min, double max) { 
   return MidiInput::fscale((int)x1t, min, max); }
double RadioBaton::y1tsf(double min, double max) { 
   return MidiInput::fscale((int)y1t, min, max); }
double RadioBaton::x2tsf(double min, double max) { 
   return MidiInput::fscale((int)x2t, min, max); }
double RadioBaton::y2tsf(double min, double max) { 
   return MidiInput::fscale((int)y2t, min, max); }
double RadioBaton::d1psf(double min, double max) { 
   return MidiInput::fscale((int)d1p, min, max); }
double RadioBaton::d2psf(double min, double max) { 
   return MidiInput::fscale((int)d1p, min, max); }
double RadioBaton::d3psf(double min, double max) { 
   return MidiInput::fscale((int)d1p, min, max); }
double RadioBaton::d4psf(double min, double max) { 
   return MidiInput::fscale((int)d1p, min, max); }


//////////////////////////////
//
// changing functions -- returns true if changing to a new
//    boxed area.
//

int RadioBaton::x1pc(int min, int max) {
   return x1ps(min, max) - scale(x1pb[1], min, max); }
int RadioBaton::y1pc(int min, int max) {
   return y1ps(min, max) - scale(y1pb[1], min, max); }
int RadioBaton::z1pc(int min, int max) {
   return z1ps(min, max) - scale(z1pb[1], min, max); }
int RadioBaton::x2pc(int min, int max) {
   return x2ps(min, max) - scale(x2pb[1], min, max); }
int RadioBaton::y2pc(int min, int max) {
   return y2ps(min, max) - scale(y2pb[1], min, max); }
int RadioBaton::z2pc(int min, int max) {
   return z2ps(min, max) - scale(z2pb[1], min, max); }
int RadioBaton::x1tc(int min, int max) {
   return x1ts(min, max) - scale(x1tb[1], min, max); }
int RadioBaton::y1tc(int min, int max) {
   return y1ts(min, max) - scale(y1tb[1], min, max); }
int RadioBaton::x2tc(int min, int max) {
   return x2ts(min, max) - scale(x2tb[1], min, max); }
int RadioBaton::y2tc(int min, int max) {
   return y2ts(min, max) - scale(y2tb[1], min, max); }
int RadioBaton::d1pc(int min, int max) {
   return d1ps(min, max) - scale(d1pb[1], min, max); }
int RadioBaton::d2pc(int min, int max) {
   return d2ps(min, max) - scale(d2pb[1], min, max); }
int RadioBaton::d3pc(int min, int max) {
   return d3ps(min, max) - scale(d3pb[1], min, max); }
int RadioBaton::d4pc(int min, int max) {
   return d4ps(min, max) - scale(d4pb[1], min, max); }


///////////////////////////////////////////////////////////////////////////
//
// private functions
//


//////////////////////////////
//
// RadioBaton::interpretCommand -- incoming MIDI messages stored in the
//     MidiInput incoming buffer will be processed one at a time,
//     checking for MIDI messages from a radio drum.  This function
//     will know what to do with that MIDI input message.
//

void RadioBaton::interpretCommand(MidiMessage aMessage) {
   ushort value;  // for the buff value receive commands

   if (aMessage.command() == BAT_MIDI_COMMAND) {
      switch (aMessage.p1()) {
         case BAT_STICK1_RESPONSE_X:         // stick 1 responding to poll; x
            s1ps(aMessage.time);
            s1pd(DATA_X, aMessage.p2());
            break;
         case BAT_STICK1_RESPONSE_Y:         // stick 1 responding to poll; y
            s1pd(DATA_Y, aMessage.p2());
            break;
         case BAT_STICK1_RESPONSE_Z:         // stick 1 responding to poll; z
            s1pd(DATA_Z, aMessage.p2());
            break;
         case BAT_STICK2_RESPONSE_X:         // stick 2 responding to poll; x
            s2ps(aMessage.time);
            s2pd(DATA_X, aMessage.p2());
            break;
         case BAT_STICK2_RESPONSE_Y:         // stick 2 responding to poll; y
            s2pd(DATA_Y, aMessage.p2());
            break;
         case BAT_STICK2_RESPONSE_Z:         // stick 2 responding to poll; z
            s2pd(DATA_Z, aMessage.p2());
            break;
         case BAT_POT1_RESPONSE:             // pot 1 responding to poll
            d1p = aMessage.p2();             // update global state variable
            dial1position();
            recordState(aMessage.time, DIAL1RECORD, d1p);
            d1pb.insert(aMessage.p2());
            break;
         case BAT_POT2_RESPONSE:             // pot 2 responding to poll
            d2p = aMessage.p2();             // update global state variable
            dial2position();
            recordState(aMessage.time, DIAL2RECORD, d2p);
            d2pb.insert(aMessage.p2());
            break;
         case BAT_POT3_RESPONSE:             // pot 3 responding to poll
            d3p = aMessage.p2();             // update global state variable
            dial3position();
            recordState(aMessage.time, DIAL3RECORD, d3p);
            d3pb.insert(aMessage.p2());
            break;
         case BAT_POT4_RESPONSE:             // pot 4 responding to poll
            d4p = aMessage.p2();             // update global state variable
            dial4position();
            recordState(aMessage.time, DIAL4RECORD, d4p);
            d4pb.insert(aMessage.p2());
            break;
         case BAT_STICK1_TRIGGER:            // stick 1 got triggered
            s1ts(aMessage.time);
            s1td(DATA_W, aMessage.p2());
            break;
         case BAT_STICK1_TRIG_X:             // stick 1 got triggered
            s1td(DATA_X, aMessage.p2());
            break;
         case BAT_STICK1_TRIG_Y:             // stick 1 got triggered
            s1td(DATA_Y, aMessage.p2());
            break;
         case BAT_STICK2_TRIGGER:            // stick 2 got triggered
            s2ts(aMessage.time);
            s2td(DATA_W, aMessage.p2());
            break;
         case BAT_STICK2_TRIG_X:             // stick 2 got triggered
            s2td(DATA_X, aMessage.p2());
            break;
         case BAT_STICK2_TRIG_Y:             // stick 2 got triggered
            s2td(DATA_Y, aMessage.p2());
            break;

         case BAT_BUTTON_FOOT_TRIGGER:       // button or pedal was triggered
            switch (aMessage.p2()) {
               case BAT_B14p_TRIGGER:        // B14+ button pressed
                  b14pt = aMessage.time;
                  b14ptb.insert(b14pt);
                  recordState(aMessage.time, BUTTON1RECORD);
                  b14plustrig();
                  break;
               case BAT_B15p_TRIGGER:        // B15+ button pressed
                  b15pt = aMessage.time;
                  recordState(aMessage.time, BUTTON2RECORD);
                  b15plustrig();
                  b15ptb.insert(b15pt);
                  break;
               case BAT_B14m_DOWN_TRIGGER:   // B14- pedal was depressed
                  b14mdt = aMessage.time;
                  b14mdtb.insert(b14mdt);
                  recordState(b14mdt, FOOTPEDAL1RECORD, 1);
                  b14minusdowntrig();
                  break;
               case BAT_B14m_UP_TRIGGER:     // B14- pedal was released
                  b14mut = aMessage.time;
                  b14mutb.insert(b14mut);
                  recordState(b14mut, FOOTPEDAL1RECORD, 0);
                  b14minusuptrig();
                  break;
               case BAT_B15m_DOWN_TRIGGER:   // B15- pedal was depressed
                  b15mdt = aMessage.time;
                  b15mdtb.insert(b15mdt);
                  recordState(b15mut, FOOTPEDAL2RECORD, 1);
                  b15minusdowntrig();
                  break;
               case BAT_B15m_UP_TRIGGER:     // B15- pedal was released
                  b15mut = aMessage.time;
                  b15mutb.insert(b15mut);
                  recordState(b15mut, FOOTPEDAL2RECORD, 0);
                  b15minusuptrig();
                  break;
            }
            break;

         case BAT_SEND:           
            // ignore this command 
            break;   
         default:
            if (errorQ) {
               cerr << "Baton command not recognized: 0x" << hex
                    << (int)aMessage.command() << dec
                    << ", param1 = " << (int)aMessage.p1()
                    << ", param2 = " << (int)aMessage.p2()
                    << endl;
            }
      }  // end of switch (aMessage.command())

   }  // end of if a baton command (0xa0)

   else if (aMessage.command() == 0xa1) {
      // do nothing: old calibration message
   } else if (aMessage.command() == 0xa2) {
      // do nothing: old calibration message
   } else if (aMessage.command() == 0xa3) {
      // do nothing: old calibration message
   } else if (aMessage.command() == 0xa4) {
      // do nothing: old calibration message
   }

   // take care of buf data
   else if (aMessage.command() == 0xa5) {
      // if p1 is not in the range from 0 to 15 then ignore the error:
      if (aMessage.p1() > 15) return;
      completeBufQ[aMessage.p1()] = 0;
      value = (ushort)(aMessage.p2() & 0x000f);
      buf[aMessage.p1()] &= 0x0fff;
      buf[aMessage.p1()] |= (value << 12);
   } else if (aMessage.command() == 0xa6) {
      // if p1 is not in the range from 0 to 15 then ignore the error:
      if (aMessage.p1() > 15) return;
      value = (ushort)(aMessage.p2() & 0x000f);
      buf[aMessage.p1()] &= 0xf0ff;
      buf[aMessage.p1()] |= (value << 8);
   } else if (aMessage.command() == 0xa7) {
      // if p1 is not in the range from 0 to 15 then ignore the error:
      if (aMessage.p1() > 15) return;
      value = (ushort)(aMessage.p2() & 0x000f);
      buf[aMessage.p1()] &= 0xff0f;
      buf[aMessage.p1()] |= (value << 4);
   } else if (aMessage.command() == 0xa8) {
      // if p1 is not in the range from 0 to 15 then ignore the error:
      if (aMessage.p1() > 15) return;
      value = (ushort)(aMessage.p2() & 0x000f);
      buf[aMessage.p1()] &= 0xfff0;
      buf[aMessage.p1()] |= value;
      completeBufQ[aMessage.p1()] = 1;

      // record buffer element to file if recording
      // NOTE: have to figure out what to put in the time slots below; 0 for now.
      switch (aMessage.p1()) {
         case 0:   recordState(timer.getTime(), ANTENNA0RECORD, buf[0]);   break;
         case 1:   recordState(timer.getTime(), ANTENNA1RECORD, buf[1]);   break;
         case 2:   recordState(timer.getTime(), ANTENNA2RECORD, buf[2]);   break;
         case 3:   recordState(timer.getTime(), ANTENNA3RECORD, buf[3]);   break;
         case 4:   recordState(timer.getTime(), ANTENNA4RECORD, buf[4]);   break;
         case 5:   recordState(timer.getTime(), POT4RECORD,     buf[5]);   break;
         case 6:   recordState(timer.getTime(), ANTENNA5RECORD, buf[6]);   break;
         case 7:   recordState(timer.getTime(), ANTENNA6RECORD, buf[7]);   break;
         case 8:   recordState(timer.getTime(), ANTENNA7RECORD, buf[8]);   break;
         case 9:   recordState(timer.getTime(), ANTENNA8RECORD, buf[9]);   break;
         case 10:  recordState(timer.getTime(), ANTENNA9RECORD, buf[10]);  break;
         case 11:  recordState(timer.getTime(), POT1RECORD,     buf[11]);  break;
         case 12:  recordState(timer.getTime(), POT2RECORD,     buf[12]);  break;
         case 13:  recordState(timer.getTime(), POT3RECORD,     buf[13]);  break;
         case 14:  recordState(timer.getTime(), B14RECORD,      buf[14]);  break;
         case 15:  recordState(timer.getTime(), B15RECORD,      buf[15]);  break;
      }
   }

   else if (aMessage.command() == 0xaa) {              // miditest
      currentmidi = aMessage.p2();
      if (currentmidi != 0) {
         if (currentmidi != lastmidi + 1) {
            midierrors++;
         }
         lastmidi = currentmidi;
         miditests++;
      }
   } else if (aMessage.command() == 0xab) {            // miditest
      currentmidi = aMessage.p2();
      if (currentmidi != 0) {
         if (currentmidi != lastmidi+1) { 
            midierrors++;
         }
         lastmidi = currentmidi;
         miditests++;
      }
   }

   // All other MIDI commands are ignored.  Maybe they are
   // for/from a synthesizer or something like that.
}



//////////////////////////////
//
// RadioBaton::pushReportingStatus --
//

void RadioBaton::pushReportingStatus(void) {
   storedReportStatus = reportingQ;
}



//////////////////////////////
//
// RadioBaton::popReportingStatus --
//
 
void RadioBaton::popReportingStatus(void) {
   if (storedReportStatus == reportingQ)  {
      return;
   } else { 
      togglePositionReporting();
   }
}



//////////////////////////////
//
// RadioBaton::recordState --
//

void RadioBaton::recordState(long aTime, const char* aState) {
   if (recordingQ()) {
      if (recordTimeOffset == -1) {
         recordTimeOffset = aTime;
      }
      recordOutput << aTime-recordTimeOffset 
                   << '\t' << aState << '\n';
   }
}


void RadioBaton::recordState(long aTime, const char* aState, int value) {
   if (recordingQ()) {
      if (recordTimeOffset == -1) {
         recordTimeOffset = aTime;
      }
      recordOutput << aTime - recordTimeOffset << '\t' 
                   << aState << '\t' << value << '\n';
   }
}


void RadioBaton::recordState(long aTime, const char* aState, 
      int value1, int value2, int value3) {
   if (recordingQ()) {
      if (recordTimeOffset == -1) {
         recordTimeOffset = aTime;
      }
      recordOutput << aTime - recordTimeOffset << '\t' << aState 
                   << '\t' << value1 << '\t' << value2 
                   << '\t' << value3 << '\n';
   }
}



//////////////////////////////
//
// RadioBaton::s1td -- "Stick 1 Trigger Data"
//    Stores trigger position data in temporary storage
//    until a complete set of tigger data is received.
//

void RadioBaton::s1td(int flag, uchar aValue) {
   switch (flag) {
      case DATA_W:
         w1t = aValue;         // set the immediate state variable
                               // whack1, trigtime1 set in the function s1ts
         s1tf += 1;            // set flag bit 1 (or create an error)
         break;
      case DATA_X:
         x1t = aValue;         // set the immediate state variable
         if (getXaxisDirection() == -1) {
            x1t = 127-x1t;
         }
         whack1x = 1;          // set the global state variable
         s1tf += 4;            // set flag bit 3 (or create an error)
         break;
      case DATA_Y:
         y1t = aValue;         // set the immediate state variable
         if (getYaxisDirection() == -1) {
            y1t = 127-y1t;
         }
         whack1y = 1;          // set the global state variable
         s1tf += 16;           // set flag bit 5 (or create an error)
         if (s1tf == 21) {     // if complete data then store trigger set
            t1tb.insert((uchar)t1t);
            w1tb.insert((uchar)w1t);
            x1tb.insert((uchar)x1t);
            y1tb.insert((uchar)y1t);
            stick1trig();               // call user-defined behavior function
            recordState(t1t, TRIGGER1RECORD, x1t, y1t, w1t);
         } else {
//            cerr << "Error: incomplete stick 1 position information" 
//                 << ", flag value is: " << (int)s1tf << endl;
         }
         break;
   }
}



//////////////////////////////
//
// RadioBaton::s1ts -- "Stick 1 Trigger Status"
//    Keeps track of a compete set of stick 1 trigger positions.  
//    This function resets temporary values to a new set of 
//    stick 1 trigger values.   Stores the timestamp and initilaizes
//    the s1tf status flag.
//

void RadioBaton::s1ts(long aTime) {
   trigger1 = 1;               // set the global state variable
   t1t = aTime;                // set the internal temporary value
   s1tf = 0x00;                // initialize the flag to determine good set
}



//////////////////////////////
//
// RadioBaton::s2td -- "Stick 2 Trigger Data"
//    Stores trigger position data in temporary storage
//    until a complete set of tigger data is received.
//

void RadioBaton::s2td(int flag, uchar aValue) {
   switch (flag) {
      case DATA_W:
         w2t = aValue;          // set the global state variable
                                // whack2, trigtime2 set in s2ts function
         s2tf += 1;             // set flag bit 1 (or create an error)
         break;
      case DATA_X:
         x2t = aValue;          // set the global state variable
         if (getXaxisDirection() == -1) {
            x2t = 127-x2t;
         }
         whack2x = 1;           // set the global state variable
         s2tf += 4;             // set flag bit 3 (or create an error)
         break;
      case DATA_Y:
         y2t = aValue;          // set the global state variable
         if (getYaxisDirection() == -1) {
            y2t = 127-y2t;
         }
         whack2y = 1;           // set the global state variable
         s2tf += 16;            // set flag bit 5 (or create an error)
         if (s2tf == 21) {      // if complete trigger, then store
            t2tb.insert(t2t); 
            w2tb.insert((uchar)w2t);
            x2tb.insert((uchar)x2t);
            y2tb.insert((uchar)y2t);
            stick2trig();               // call the user state function
            recordState(t2t, TRIGGER2RECORD, x2t, y2t, w2t);
         } else {
//            cerr << "Error: incomplete stick 2 trigger information" 
//                 << ", flag value is: " << (int)s2tf << endl;
         }
         break;
   }
}



//////////////////////////////
//
// RadioBaton::s2ts -- "Stick 2 Trigger Status"
//    Keeps track of a compete set of stick 2 trigger positions.  
//    This function reset temporary values to new set of 
//    stick 2 trigger values.   Stores the timestamp and initilaizes
//    the s1tf status flag.
//

void RadioBaton::s2ts(long aTime) {
   trigger2 = 1;               // set the global state variable
   t2t = aTime;           // store the internal temporary value
   s2tf = 0x00;                // initialize the flag to determine good set
}



//////////////////////////////
//
// RadioBaton::s1pd -- "Stick 1 Position Data"
//    Stores position data in temporary storage
//    until a complete set of position data is received.
//

void RadioBaton::s1pd(int flag, uchar aValue) {
   switch (flag) {
      case DATA_X:
         x1p = aValue;         // set the global state variable
         if (getXaxisDirection() == -1) {
            x1p = 127-x1p;
         }
         s1pf += 1;            // set bit 1 (or create an error)
         break;
      case DATA_Y:
         y1p = aValue;          // set the global state variable
         if (getYaxisDirection() == -1) {
            y1p = 127-y1p;
         }
         s1pf += 4;             // set bit 3 (or create an error)
         break;
      case DATA_Z:
         z1p = aValue;          // set the global state variable
         if (getZaxisDirection() == -1) {
            z1p = 127-z1p;
         }
         s1pf += 16;            // set bit 5 (or create an error)
         if (s1pf == 21) {      // if complete temp position set then store
            x1pb.insert((uchar)x1p);
            y1pb.insert((uchar)y1p);
            z1pb.insert((uchar)z1p);
            stick1position();
            recordState(t1p, POSITION1RECORD, x1p, y1p, z1p);
         } else {
//            cerr << "Error: incomplete stick 1 position information"  
//                 << ", flag value is: " << (int)s1pf << endl;
         }
         break;
   }
}



//////////////////////////////
//
// RadioBaton::s1ps -- "Stick 1 Position Status"
//    Keeps track of a compete set of stick 1 positions.  
//    This function reset temporary values to new set of 
//    stick 1 position values.   Stores the timestamp and initilaizes
//    the s1pf status flag.
//

void RadioBaton::s1ps(long aTime) {
   t1p = aTime;
   t1pb.insert(t1p); 
   s1pf = 0x00;
}




//////////////////////////////
//
// RadioBaton::s2pd -- "Stick 2 Position Data"
//    Stores position data in temporary storage
//    until a complete set of position data is received.
//

void RadioBaton::s2pd(int flag, uchar aValue) {
   switch (flag) {
      case DATA_X:
         x2p = aValue;          // set the global state variable
         if (getXaxisDirection() == -1) {
            x2p = 127-x2p;
         }
         s2pf += 1;             // set flag bit 1 (or create an error)
         break;
      case DATA_Y:
         y2p = aValue;          // set the global state variable
         if (getYaxisDirection() == -1) {
            y2p = 127-y2p;
         }
         s2pf += 4;             // set flag bit 3 (or create an error)
         break;
      case DATA_Z:
         z2p = aValue;          // set the global state variable
         if (getZaxisDirection() == -1) {
            z2p = 127-z2p;
         }
         s2pf += 16;            // set flag bit 5 (or create an error)
         if (s2pf == 21) {      // if complete temp position set then store
            x2pb.insert((uchar)x2p);
            y2pb.insert((uchar)y2p);
            z2pb.insert((uchar)z2p);
            stick2position();
            recordState(t2p, POSITION2RECORD, x2p, y2p, z2p);
         } else {
//            cerr << "Error: incomplete stick 2 position information" 
//                 << ", flag value is: " << (int)s2pf << endl;          
         }
         break;
   }
}



//////////////////////////////
//
// RadioBaton::s2ps -- "Stick 2 Position Status"
//    Keeps track of a compete set of stick 2 positions.  
//    This function reset temporary values to new set of 
//    stick 2 position values.   Stores the timestamp and initilaizes
//    the s1pf status flag.
//

void RadioBaton::s2ps(long aTime) {
   t2p = aTime;
   t2pb.insert(t2p);
   s2pf = 0x00;
}



//////////////////////////////
//
// RadioBaton::stoprunningstatus -- some smarty-pants MIDI drivers
//     will go into running status by themselves.  This is a no-no
//     for the radio baton (the physical device) since it does
//     not understand running status (where the MIDI command is
//     not specified if the same command came just before).
//

void RadioBaton::stoprunningstatus(void) {
   rawsend(0xaf, 0, 0);
}



//////////////////////////////
//
// RadioBatonEmptyBehavior -- this function is the default function
//     to which the radio baton class behavior functions point to.
// 

void RadioBatonEmptyBehavior(void) {
   // nothing to do since it is empty
}



// md5sum: 2a51c93cbf466a672bc3ac05e4d7440e RadioBaton.cpp [20050403]
