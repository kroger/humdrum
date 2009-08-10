//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 16 13:39:10 PDT 2000
// Last Modified: Sun Jul 16 16:56:01 PDT 2000
// Filename:      ...sig/code/control/AdamsStick/AdamsStick.cpp
// Web Address:   http://sig.sapp.org/include/sig/AdamsStick.cpp
// Syntax:        C++
//
// Description:   A structure for handling the Interval stick variables
//                and the MIDI connections from the computer to 
//                the stick.  The MIDI protocol is described at the
//                bottom of this file.
//

#include "AdamsStick.h"


//////////////////////////////
//
// AdamsStick::AdamsStick --
//

AdamsStick::AdamsStick(void) { 
   setStateSize(STICK_DEFAULT_STATE_SIZE);
   currentMode = STICK_POLL_MODE;
   connectedQ = 0;
   versionInfo = -1;

   t1s = t2s = t3s = 0;
   s1p = s2p = s3p = 0;
   s1f = s2f = s3f = 0;
   loc1 = loc2 = loc3 = -1;
   loc1t = loc2t = loc3t = -1;

   trigger1level = trigger2level = trigger3level = 1500;
   trigger1delta = trigger2delta = trigger3delta = 200;
   state1 = state2 = state3 = 0;
   state1on = state2on = state3on = 1;
   state1off = state2off = state3off = 0;

   response    = AdamsStickEmptyBehavior;
   fsr1ontrig  = AdamsStickEmptyBehavior;
   fsr1offtrig = AdamsStickEmptyBehavior;
   fsr2ontrig  = AdamsStickEmptyBehavior;
   fsr2offtrig = AdamsStickEmptyBehavior;
   fsr3ontrig  = AdamsStickEmptyBehavior;
   fsr3offtrig = AdamsStickEmptyBehavior;

   pollTimer.setPeriod(STICK_DEFAULT_POLL_PERIOD);
   pollTimer.reset();
   pollTimer.update(-1);
}


AdamsStick::AdamsStick(int outputDevice, int inputDevice) :
      MidiIO(outputDevice, inputDevice) { 
   setStateSize(STICK_DEFAULT_STATE_SIZE);
   currentMode = STICK_POLL_MODE;
   connectedQ = 0;
   versionInfo = -1;

   t1s = t2s = t3s = 0;
   s1p = s2p = s3p = 0;
   s1f = s2f = s3f = 0;
   loc1 = loc2 = loc3 = -1;
   loc1t = loc2t = loc3t = -1;

   trigger1level = trigger2level = trigger3level = 1500;
   trigger1delta = trigger2delta = trigger3delta = 200;
   state1 = state2 = state3 = 0;
   state1on = state2on = state3on = 1;
   state1off = state2off = state3off = 0;

   response    = AdamsStickEmptyBehavior;
   fsr1ontrig  = AdamsStickEmptyBehavior;
   fsr1offtrig = AdamsStickEmptyBehavior;
   fsr2ontrig  = AdamsStickEmptyBehavior;
   fsr2offtrig = AdamsStickEmptyBehavior;
   fsr3ontrig  = AdamsStickEmptyBehavior;
   fsr3offtrig = AdamsStickEmptyBehavior;

   pollTimer.setPeriod(STICK_DEFAULT_POLL_PERIOD);
   pollTimer.reset();
   pollTimer.update(-1);
}



//////////////////////////////
//
// AdamsStick::~AdamsStick --
//

AdamsStick::~AdamsStick() { 
   // do nothing
}



//////////////////////////////
//
// AdamsStick::checkPoll -- compare the next poll time to 
//   the current time and poll the stick if it is time to
//   do so.
//

int AdamsStick::checkPoll(void) { 
   if (pollTimer.expired()) {
      poll();
      pollTimer.reset();
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// AdamsStick::getLevel -- returns trigger level for the given
//   fsr.
//

int AdamsStick::getLevel(int fsrnumber) {
   switch (fsrnumber) {
      case 1:   return trigger1level;
      case 2:   return trigger2level;
      case 3:   return trigger3level;
   }
   return -1;
}



//////////////////////////////
//
// AdamsStick::getMode -- returns (STICK_STREAM_MODE  1) or
//     (STICK_POLL_MODE 0).

int AdamsStick::getMode(void) { 
   return currentMode;
}



//////////////////////////////
//
// AdamsStick::getPollPeriod -- returns the poll period used
//     with checkPoll()
//

double AdamsStick::getPollPeriod(void) { 
   return pollTimer.getPeriod();
}



//////////////////////////////
//
// AdamsStick::getState -- returns 0 is fsr is off, otherwise 1.
//

int AdamsStick::getState(int fsrnumber) {
   switch (fsrnumber) {
      case 1:   return state1;     break;
      case 2:   return state2;     break;
      case 3:   return state3;     break;
   }
   return 0;
}



//////////////////////////////
//
// AdamsStick::getStateSize -- returns the storage size of the 
//   circular buffers used to store old state variables.
//

int AdamsStick::getStateSize(void) { 
   return s1pb.getSize();
}



//////////////////////////////
//
// AdamsStick::getThreshold -- returns the histeresis threshold value
//     for each FSR.  returns -1 if an invalid fsr number.
//

int AdamsStick::getThreshold(int fsrnumber) {
   switch (fsrnumber) {
      case 1:   return trigger1delta;   break;
      case 2:   return trigger2delta;   break;
      case 3:   return trigger3delta;   break;
   }
   return -1;
}



//////////////////////////////
//
// AdamsStick::getVersion -- returns the version number of the
//     stick.  return -1 if stick is not connected.
//

int AdamsStick::getVersion(void) { 
   if (versionInfo == -1) {
      sendVersionMessage();
      millisleep(40);
      processIncomingMessages();
   }

   return versionInfo;
}



//////////////////////////////
//
// AdamsStick::is_connected -- determine if the stick is connected
//   to the computer (both input and output).
//

int AdamsStick::is_connected(void) { 
   connectedQ = 0;
   sendVersionMessage();
   millisleep(40);
   processIncomingMessages();
   return connectedQ;
}



//////////////////////////////
//
// AdamsStick::poll -- ask the stick for current state information.
//

void AdamsStick::poll(void) { 
   sendPollMessage();
}



//////////////////////////////
//
// AdamsStick::processIncomingMessages --
//

void AdamsStick::processIncomingMessages(void) { 
   while (MidiInput::getCount() > 0) {
      interpretCommand(MidiInput::extract());
   }
}



//////////////////////////////
//
// AdamsStick::setLevel -- 
//

void AdamsStick::setLevel(int fsrnumber, int aValue) {
   switch (fsrnumber) {
      case 1: 
         if (aValue < trigger1delta + 20) {
            aValue = trigger1delta + 20;
         }
         trigger1level = aValue; 
         break;
      case 2: 
         if (aValue < trigger2delta + 20) {
            aValue = trigger2delta + 20;
         }
         trigger2level = aValue; 
         break;
      case 3: 
         if (aValue < trigger3delta + 20) {
            aValue = trigger3delta + 20;
         }
         trigger3level = aValue; 
         break;
   }
}


void AdamsStick::setLevel(int aValue) {
   setLevel(1, aValue);
   setLevel(2, aValue);
   setLevel(3, aValue);
}



//////////////////////////////
//
// AdamsStick::setMode -- 0 = polling, 1 = streaming
//

void AdamsStick::setMode(int aMode) { 
   if (aMode) {
      currentMode = 1;
   } else {
      currentMode = 0;
   }
}



//////////////////////////////
//
// AdamsStick::setPollMode -- set the stick into Poll mode.
//

void AdamsStick::setPollMode(void) { 
   sendPollingMessage();
}



//////////////////////////////
//
// AdamsStick::setPollPeriod -- set the poll period used
//     with checkPoll().  Time is in milliseconds.
//

void AdamsStick::setPollPeriod(double aPeriod) { 
   pollTimer.setPeriod(aPeriod);
}



//////////////////////////////
//
// AdamsStick::setStateSize -- set the length of the circular buffers
//     that store old state information.
//

void AdamsStick::setStateSize(int aSize) { 
   t1sb.setSize(aSize);
   s1pb.setSize(aSize);
   s1fb.setSize(aSize);

   t2sb.setSize(aSize);
   s2pb.setSize(aSize);
   s2fb.setSize(aSize);

   t3sb.setSize(aSize);
   s3pb.setSize(aSize);
   s3fb.setSize(aSize);
}



//////////////////////////////
//
// AdamsStick::setStreamMode --
//

void AdamsStick::setStreamMode(void) { 
   sendStreamingMessage();
}



//////////////////////////////
//
// AdamsStick::setThreshold -- sets the histeresis threshold value
//     for each FSR. 
//

void AdamsStick::setThreshold(int fsrnumber, int aValue) {
   if (aValue < 40) {
      aValue = 40;
   }
   switch (fsrnumber) {
      case 1:   trigger1delta = aValue;   break;
      case 2:   trigger2delta = aValue;   break;
      case 3:   trigger3delta = aValue;   break;
   }
}


void AdamsStick::setThreshold(int aValue) {
   setThreshold(1, aValue);
   setThreshold(2, aValue);
   setThreshold(3, aValue);
}
   


//////////////////////////////
//
// AdamsStick::toggleMode --
//

int AdamsStick::toggleMode(void) { 
   currentMode = !currentMode;
   if (currentMode) {
      sendStreamingMessage();
   } else {
      sendPollingMessage();
   }
   return currentMode;
}



//////////////////////////////
//
// AdamsStick::s1ps -- scale the 14 bit number into the range
//     given in the parameters
//

int AdamsStick::s1ps(int min, int max) { return scale14x((int)s1p, min, max); }
int AdamsStick::s1fs(int min, int max) { return scale14x((int)s1f, min, max); }
int AdamsStick::s2ps(int min, int max) { return scale14x((int)s2p, min, max); }
int AdamsStick::s2fs(int min, int max) { return scale14x((int)s2f, min, max); }
int AdamsStick::s3ps(int min, int max) { return scale14x((int)s3p, min, max); }
int AdamsStick::s3fs(int min, int max) { return scale14x((int)s3f, min, max); }


//////////////////////////////
//
// AdamsStick::s1p7 -- scale values to 7 bit numbers.
//

int AdamsStick::s1p7(void) { return scale14x((int)s1p, 0, 127); }
int AdamsStick::s1f7(void) { return scale14x((int)s1f, 0, 127); }
int AdamsStick::s2p7(void) { return scale14x((int)s2p, 0, 127); }
int AdamsStick::s2f7(void) { return scale14x((int)s2f, 0, 127); }
int AdamsStick::s3p7(void) { return scale14x((int)s3p, 0, 127); }
int AdamsStick::s3f7(void) { return scale14x((int)s3f, 0, 127); }
int AdamsStick::loc1_7(void) { 
   return scale14x((int)loc1, 0, 127); 
}
int AdamsStick::loc2_7(void) { 
   return scale14x((int)loc2, 0, 127); 
}
int AdamsStick::loc3_7(void) { 
   return scale14x((int)loc3, 0, 127); 
}



//////////////////////////////
//
// AdamsStick::s1ps -- double scaling values
//

double AdamsStick::s1ps(double min, double max) { 
   return fscale14x(s1p, min, max); 
}
double AdamsStick::s1fs(double min, double max) { 
   return fscale14x(s1f, min, max); 
}
double AdamsStick::s2ps(double min, double max) { 
   return fscale14x(s2p, min, max); 
}
double AdamsStick::s2fs(double min, double max) { 
   return fscale14x(s2f, min, max); 
}
double AdamsStick::s3ps(double min, double max) { 
   return fscale14x(s2p, min, max); 
}
double AdamsStick::s3fs(double min, double max) { 
   return fscale14x(s3f, min, max); 
}



//////////////////////////////
//
// AdamsStick::s1pc -- changing functions: returns the amount of change 
// between successive values of the input data.
//

int AdamsStick::s1pc(int min, int max) { 
   return scale14x(s1pb[0], min, max) - scale(s1pb[1], min, max);
}
int AdamsStick::s1fc(int min, int max) {
   return scale14x(s1fb[0], min, max) - scale14x(s1fb[1], min, max);
}
int AdamsStick::s2pc(int min, int max) {
   return scale14x(s2pb[0], min, max) - scale14x(s2pb[1], min, max);
}
int AdamsStick::s2fc(int min, int max) {
   return scale14x(s2fb[0], min, max) - scale14x(s2fb[1], min, max);
}
int AdamsStick::s3pc(int min, int max) {
   return scale14x(s3pb[0], min, max) - scale14x(s3pb[1], min, max);
}
int AdamsStick::s3fc(int min, int max) {
   return scale14x(s3fb[0], min, max) - scale14x(s3fb[1], min, max);
}


///////////////////////////////////////////////////////////////////////////
//
// private functions:
// 

//////////////////////////////
//
// AdamsStick::interpretCommand --
//

#define POSITION_THRESHOLD 2000

void AdamsStick::interpretCommand(MidiMessage aMessage) { 
   switch (aMessage.command()) {
      case 0x90:
         t1s = aMessage.time;
         t1sb.insert(t1s);
         s1p = convertTo14bits(aMessage.p1(), aMessage.p2());
         s1pb.insert(s1p);
         break;
      case 0x91:
         s1f = convertTo14bits(aMessage.p1(), aMessage.p2());
         s1fb.insert(s1f);
         if (s1f > POSITION_THRESHOLD) {
            loc1 = s1p;
            loc1t = t1s;
         }
         break;
      case 0x92:
         t2s = aMessage.time;
         t2sb.insert(t2s);
         s2p = convertTo14bits(aMessage.p1(), aMessage.p2());
         s2pb.insert(s2p);
         break;
      case 0x93:
         s2f = convertTo14bits(aMessage.p1(), aMessage.p2());
         s2fb.insert(s2f);
         if (s2f > POSITION_THRESHOLD) {
            loc2 = s2p;
            loc2t = t2s;
         }
         break;
      case 0x94:
         t3s = aMessage.time;
         t3sb.insert(t3s);
         s3p = convertTo14bits(aMessage.p1(), aMessage.p2());
         s3pb.insert(s3p);
         break;
      case 0x95:
         s3f = convertTo14bits(aMessage.p1(), aMessage.p2());
         s3fb.insert(s3f);
         if (s3f > POSITION_THRESHOLD) {
            loc3 = s3p;
            loc3t = t3s;
         }
         determineTriggers();
         response();
         break;
      case 0xf0:
         {
         // sample sysex message for version number: "0xf0 16 0 103 0xf7"
         int bufferNumber = aMessage.p1();
         int size = getSysexSize(bufferNumber);
         unsigned char *data = getSysex(bufferNumber);
         if (size != 5) return;
         if (data[1] != 16) return;
         if (data[2] !=  0) return;
         versionInfo = data[3];
         connectedQ = 1;
         }
         break;
      default:
         cout << "Unknown stick command: " << aMessage << endl;
   }
}

//////////////////////////////
//
// determineTrigger --  Determine if an on/off trigger needs to
//     be generated on one of the FSRs:
//

void AdamsStick::determineTriggers(void) {

   // see if it is time to trigger on fsr1 ////////////////////////////////
   if (state1 == 1) {
      if (s1f > trigger1level + trigger1delta) {
         state1on = 1;
      }
      if (state1on == 1 && s1f < trigger1level) {
         fsr1offtrig();
         state1 = 0;
         state1off = 0;
      } else if (state1on == 0 && s1f < trigger1level - trigger1delta) {
         fsr1offtrig();
         state1 = 0;
         state1off = 1;
      }
   } else {
      if (s1f < trigger1level - trigger1delta) {
         state1off = 1;
      }
      if (state1off == 1 && s1f > trigger1level) {
         fsr1ontrig();
         state1 = 1;
         state1on = 0;
      } else if (state1off == 0 && s1f > trigger1level + trigger1delta) {
         fsr1ontrig();
         state1 = 1;
         state1on = 1;
      }
   }

   // see if it is time to trigger on fsr2 ////////////////////////////////
   if (state2 == 1) {
      if (s2f > trigger2level + trigger2delta) {
         state2on = 1;
      }
      if (state2on == 1 && s2f < trigger2level) {
         fsr2offtrig();
         state2 = 0;
         state2off = 0;
      } else if (state2on == 0 && s2f < trigger2level - trigger2delta) {
         fsr2offtrig();
         state2 = 0;
         state2off = 1;
      }
   } else {
      if (s2f < trigger2level - trigger2delta) {
         state2off = 1;
      }
      if (state2off == 1 && s2f > trigger2level) {
         fsr2ontrig();
         state2 = 1;
         state2on = 0;
      } else if (state2off == 0 && s2f > trigger2level + trigger2delta) {
         fsr2ontrig();
         state2 = 1;
         state2on = 1;
      }
   }

   // see if it is time to trigger on fsr3 ////////////////////////////////
   if (state3 == 1) {
      if (s3f > trigger3level + trigger3delta) {
         state3on = 1;
      }
      if (state3on == 1 && s3f < trigger3level) {
         fsr3offtrig();
         state3 = 0;
         state3off = 0;
      } else if (state3on == 0 && s3f < trigger3level - trigger3delta) {
         fsr3offtrig();
         state3 = 0;
         state3off = 1;
      }
   } else {
      if (s3f < trigger3level - trigger3delta) {
         state3off = 1;
      }
      if (state3off == 1 && s3f > trigger3level) {
         fsr3ontrig();
         state3 = 1;
         state3on = 0;
      } else if (state3off == 0 && s3f > trigger3level + trigger3delta) {
         fsr3ontrig();
         state3 = 1;
         state3on = 1;
      }
   }

}



//////////////////////////////
//
// AdamsStick::sendVersionMessage --  transmit a message to the 
//     stick indicating that you want to know the version number
//     of its firmware.
//

void AdamsStick::sendVersionMessage(void) {
   unsigned char versionMessage[4] = {0xf0,  16,  4, 0xf7};
   MidiOutput::rawsend(versionMessage, 4);
}



//////////////////////////////
//
// AdamsStick::setStreamingMessage -- transmit a message to the 
//     stick indicating that you want the state data to be
//     reported at a constant rate of 16 milliseconds.
//

void AdamsStick::sendStreamingMessage(void) { 
   unsigned char streamingMessage[5] = {0xf0,  16,  1,  0,  0xf7};
   MidiOutput::rawsend(streamingMessage, 5);
}



//////////////////////////////
//
// AdamsStick::setPollingMessage -- transmit a message to the stick
//     indicating that you want the state data to be reported only 
//     when a poll request message is sent to the stick.
//

void AdamsStick::sendPollingMessage(void) { 
   unsigned char pollingMessage[5] = {0xf0,  16,  1,  1,  0xf7};
   MidiOutput::rawsend(pollingMessage, 5);
}



//////////////////////////////
//
// AdamsStick::sendPollMessage -- transmit a message to the stick
//     indicating that you want it to report its current state variables.
//

void AdamsStick::sendPollMessage(void) { 
   unsigned char pollRequest[4] = {0xf0,  16,  6,  0xf7};
   MidiOutput::rawsend(pollRequest, 4);
}



//////////////////////////////
//
// AdamsStick::sendPollMessage -- transmit a message to the stick
//     indicating that you want it to report its current state variables.
//

int AdamsStick::convertTo14bits(unsigned char msb, unsigned char lsb) {
   int output = (msb & 31) << 7;
   output |= (lsb & 0x7f);
   output = output << 2;
   return output;
}



//////////////////////////////
//
// scale14x -- converts a value in the range from 0 to 2^15-1
//      (not quite). into a number in a new range.  
//

int AdamsStick::scale14x(int value, int min, int max) {
   return value >= 16000 ? max : (int)(value/16000.0*(max-min+1)+min);
}



//////////////////////////////
//
// fscale14x -- converts a value in the range from 0 to 2^15-1
//      (almost) into a number in a new range.
//

double AdamsStick::fscale14x(int value, double min, double max) {
   return value >= 16000 ? max : (value/16000.0*(max-min)+min);
}



//////////////////////////////
//
// AdamsStickEmptyBehavior -- the following function will be the default 
//     function for the radio baton behavior function pointers. 
//

void AdamsStickEmptyBehavior(void) { }



///////////////////////////////////////////////////////////////////////////
// 
// MIDI specifications and wiring for the Adams Stick by reverse engineering.
// Bill Verplank and Craig Sapp, 14 July 2000.
// ==================================================
// 
// Description: This is a description of the MIDI I/O controls for a
// predecessor of the Talking Stick made for Laurie Anderson's Theatre Piece
// "Moby Dick" by The Development team for the Talking Stick at Interval
// Research: Bob Adams, Jessee Dorogusker, Dominici Robson, Geoff Smith,
// John Eichenseer, and Lukas Girling.  The Talking Stick uses a single
// potentiometer slider.
// 
// The "Adams Stick" which is a prototype for the Talking Stick is
// described below.  It uses 3 Force-sensing resistors which detect 
// both force and postion.  The top two FSRs are "fretted" and the 
// bottom FSR pair is "unfretted".
// 
// MIDI protocol for computer communication with the Adams Stick
// is sent to the stick in the form of System Exclusives messages:
// 
// Stream Mode On: 0xf0 16  1  0 0xf7 
// Poll Mode On:   0xf0 16  1  1 0xf7
// Poll:           0xf0 16  6    0xf7 
// Version:        0xf0 16  4    0xf7
// 
// By default when the stick is turned on, the Pole Mode is active.
// With this mode, when you send a Pole message to the stick it will
// replay the the position/pressure values for each FSR pair, which is
// a total of 6 MIDI messages.
// 
// The version of the stick I am working with returned the version answer:
// "0xf0 16 0 103 0xf7"
// 
// Both Streaming and Pole Mode sends the position/pressure values in 
// this order:
// 
// MIDI Note On, channel 1: The position on FSR 1 (top FSR)
// MIDI Note On, channel 2: The pressure on FSR 1 (top FSR)
// MIDI Note On, channel 3: The position on FSR 2 (middle FSR)
// MIDI Note On, channel 4: The pressure on FSR 2 (middle FSR)
// MIDI Note On, channel 5: The position on FSR 3 (bottom FSR)
// MIDI Note On, channel 6: The pressure on FSR 3 (bottom FSR)
//   
// The 12-bit data for each message resides in the key/velocity 
// parameter bytes for the Note On message.  The key parameter is 
// the 5 most significant bits, and the velocity contains the 7 least
// significant bits.
//   
// When data is sent in streaming mode, the position/pressure values
// update every 16 milliseconds.  The messages are sent in order
// starting with MIDI channel 1.  The next 5 messages are sent
// at the maximum MIDI rate (about 1 message per millisecond).
// Then there is a pause of 10 milliseconds until the next set
// of position/pressures are reported.
//   
// Here is a sample of the stick MIDI output as I start to press FSR #1:
//   
// Delta   MIDI
// Time  Command MSB LSB         standard MIDI interpretation
// ===============================================================
// 10      0x90   0   3            ; NOTE chan:1 key:C-1 vel:3
// 1       0x91   0   3            ; NOTE chan:2 key:C-1 vel:3
// 2       0x92   0   2            ; NOTE chan:3 key:C-1 vel:2
// 1       0x93   0   4            ; NOTE chan:4 key:C-1 vel:4
// 1       0x94   0   2            ; NOTE chan:5 key:C-1 vel:2
// 1       0x95   0   3            ; NOTE chan:6 key:C-1 vel:3
// 10      0x90  14  22            ; NOTE chan:1 key:D0 vel:22
// 1       0x91  17 123            ; NOTE chan:2 key:F0 vel:123
// 1       0x92   0   5            ; NOTE chan:3 key:C-1 vel:5
// 1       0x93   0   4            ; NOTE chan:4 key:C-1 vel:4
// 1       0x94   0   2            ; NOTE chan:5 key:C-1 vel:2
// 1       0x95   0   3            ; NOTE chan:6 key:C-1 vel:3
// 11      0x90  14 116            ; NOTE chan:1 key:D0 vel:116
// 6       0x91  25  24            ; NOTE chan:2 key:C#1 vel:24
// 0       0x92   0   7            ; NOTE chan:3 key:C-1 vel:7
// 0       0x93   0   4            ; NOTE chan:4 key:C-1 vel:4
// 0       0x94   0   2            ; NOTE chan:5 key:C-1 vel:2
// 0       0x95   0   3            ; NOTE chan:6 key:C-1 vel:3
//   
//   
// As an example here is how to extract the position/pressure value
// from the MIDI note-on message.  Given the message:
//         0x91  25  24  
// which is the pressure information on FSR1, take the binary form
// of the bytes 25 and 24:
//               00011001 00011000
// Now chop off the highest bit from each number (really just need to
// do on the second number since the top bit is always 0):
//                0011001  0011000
// Now combine the two numbers to create a single number:
//                  110010011000
// And for convenience, here is the number displayed in decimal:
//                3224
// So this is the pressure value of FSR #1 at the particular instant
// in time that the MIDI message was send out through MIDI.
// 
// The MIDI values for position/pressure are in the range from zero to 2 to
// the 13th power minus 1, or 0 to 8191.  In the Max patch originally used
// to control the stick, these numbers were multiplied by 4 (converted to
// 14-bit numbers with the least two bits always equal to zero.
// 
// Note that the pressure on the FSR jumps rapidly from 0 when you touch it.
// the value 3224 occurs when pressing very lightly, but to double the
// value to 6448, you will have to press significantly harder.  The low
// range of the FSR reporting range is therefore non-linear.  But after
// light pressure the reported values are more linear when related to the
// amount of pressure applied.  Position measurements are very linear for
// all positions.
// 
// 
// Electrical Wiring for the Adams Stick
// ======================================
// 
// Midi Input/Output from the stick comes through a single MIDI cable.
// Therefore you have to extract the MIDI Input and Output into separate
// cables in order to use it with other MIDI devices.
// 
// For the cable coming out of the Adams Stick:
//    Wire 1 = connect to +5 volt power supply.
//    Wire 2 = connect to ground.
//    Wire 3 = connect to pin 6 of a 6N138 Optoisolator.
//    Wire 4 = connect to pin 4 of the Stick MIDI output.
//    Wire 5 = connect to pin 5 of the Stick MIDI output.
// 
// For the Stick MIDI output connector:
//    Wire 2 = connect to ground.
//    Wire 4 = connect to Stick wire 4.
//    Wire 5 = connect to Stick wire 5.
// 
// For the Stick MIDI input connector:
//    Wire 2 = connect to ground.
//    Wire 4 = connect to a 220 Ohm resistor which connects to pin 2 on 6N138.
//    Wire 5 = connect to pin 3 on 6N138.
// 
// Other connections:
//    Connect a diode going from pin 3 to pin 2 on the 6N138.
//    Connect a ~220 Ohm from pin 6 to pin 8 on the 6N138.
//    Connect pin 5 on the 6N138 to ground.
//    Connect pin 8 on the 6N138 to +5v power supply.
// 


// md5sum: e507b0f0a36e51a07a8bc381f3bc4f09 AdamsStick.cpp [20020518]
