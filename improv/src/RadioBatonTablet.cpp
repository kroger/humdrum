//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  9 13:08:12 PDT 2003
// Last Modified: Wed Aug 13 14:09:38 PDT 2003
// Filename:      ...sig/code/control/Tablet/RadioBatonTablet.cpp
// Web Address:   http://sig.sapp.org/include/sig/RadioBatonTablet.cpp
// Syntax:        C++
//
// Description:   A structure for handling the Radio Baton variables
//                and the MIDI connections from the computer to 
//                the Radio Batons simulated with a Wacom Tablet.
//


#ifdef VISUAL

#include "RadioBatonTablet.h"


//////////////////////////////
//
// RadioBatonTablet::RadioBatonTablet --
//

RadioBatonTablet::RadioBatonTablet(void) {
   stick1trig		= penEmptyBehavior;
   stick2trig		= penEmptyBehavior;
   stick1position	= penEmptyBehavior;
   stick2position	= penEmptyBehavior;
   b14plustrig		= penEmptyBehavior;
   b15plustrig		= penEmptyBehavior;
   b14minusuptrig	= penEmptyBehavior;
   b14minusdowntrig	= penEmptyBehavior;
   b15minusuptrig	= penEmptyBehavior;
   b15minusdowntrig	= penEmptyBehavior;
   dial1position	= penEmptyBehavior;
   dial2position	= penEmptyBehavior;
   dial3position	= penEmptyBehavior;
   dial4position	= penEmptyBehavior;

   Tablet::pen1trigon 	  = mypen1trigon;
   Tablet::pen1trigoff 	  = mypen1trigoff;
   Tablet::pen2trigon 	  = mypen2trigon;
   Tablet::pen2trigoff 	  = mypen2trigoff;
   Tablet::pen1button1on  = mypen1button1on;
   Tablet::pen1button1off = mypen1button1off;
   Tablet::pen1button2on  = mypen1button2on;
   Tablet::pen1button2off = mypen1button2off;
   Tablet::pen2button1on  = mypen2button1on;
   Tablet::pen2button1off = mypen2button1off;
   Tablet::pen2button2on  = mypen2button2on;
   Tablet::pen2button2off = mypen2button2off;

   xDirection = 1;
   yDirection = 1;
   zDirection = 1;

   x1t = 0;
   y1t = 0;
   w1t = 0;
   x2t = 0;
   y2t = 0;
   w2t = 0;
}



RadioBatonTablet::RadioBatonTablet(int outputDevice, int inputDevice) {
   stick1trig		= penEmptyBehavior;
   stick2trig		= penEmptyBehavior;
   stick1position	= penEmptyBehavior;
   stick2position	= penEmptyBehavior;
   b14plustrig		= penEmptyBehavior;
   b15plustrig		= penEmptyBehavior;
   b14minusuptrig	= penEmptyBehavior;
   b14minusdowntrig	= penEmptyBehavior;
   b15minusuptrig	= penEmptyBehavior;
   b15minusdowntrig	= penEmptyBehavior;
   dial1position	= penEmptyBehavior;
   dial2position	= penEmptyBehavior;
   dial3position	= penEmptyBehavior;
   dial4position	= penEmptyBehavior;

   Tablet::pen1trigon 	  = mypen1trigon;
   Tablet::pen1trigoff 	  = mypen1trigoff;
   Tablet::pen2trigon 	  = mypen2trigon;
   Tablet::pen2trigoff 	  = mypen2trigoff;
   Tablet::pen1button1on  = mypen1button1on;
   Tablet::pen1button1off = mypen1button1off;
   Tablet::pen1button2on  = mypen1button2on;
   Tablet::pen1button2off = mypen1button2off;
   Tablet::pen2button1on  = mypen2button1on;
   Tablet::pen2button1off = mypen2button1off;
   Tablet::pen2button2on  = mypen2button2on;
   Tablet::pen2button2off = mypen2button2off;

   xDirection = 1;
   yDirection = 1;
   zDirection = 1;
 
   x1t = 0;
   y1t = 0;
   w1t = 0;
   x2t = 0;
   y2t = 0;
   w2t = 0;
}



//////////////////////////////
//
// RadioBatonTablet::~RadioBatonTablet --
//

RadioBatonTablet::~RadioBatonTablet(void) {
   // do nothing
}



//////////////////////////////
//
// trigger functions --
//

void RadioBatonTablet::mypen1trigon(void) { 
   x1t = Tablet::pen1.x;
   y1t = Tablet::pen1.y;
   w1t = 64;
   stick1trig();
}


void RadioBatonTablet::mypen1trigoff(void) { 
}


void RadioBatonTablet::mypen2trigon(void) { 
   x2t = Tablet::pen2.x;
   y2t = Tablet::pen2.y;
   w2t = 64;
   stick2trig();
}


void RadioBatonTablet::mypen2trigoff(void) { 
}


void RadioBatonTablet::mypen1button1on(void) { 
}


void RadioBatonTablet::mypen1button1off(void) { 
}


void RadioBatonTablet::mypen1button2on(void) { 
}


void RadioBatonTablet::mypen1button2off(void) { 
}


void RadioBatonTablet::mypen2button1on(void) { 
}


void RadioBatonTablet::mypen2button1off(void) { 
}


void RadioBatonTablet::mypen2button2on(void) { 
}


void RadioBatonTablet::mypen2button2off(void) { 
}


/////////////////////////////////////



//////////////////////////////
//
// RadioBatonTablet::processIncomingMessages -- 
//

void RadioBatonTablet::processIncomingMessages(const char* filename) {
   Tablet::processIncomingMessages();

   x1t = 0;
   y1t = 0;
   w1t = 0;
   x2t = 0;
   y2t = 0;
   w2t = 0;
}



//////////////////////////////
//
// RadioBatonTablet::toggleXAxisDirection -- switches the axis direction
//     of the x-axis (1=default, -1=reverse of default) and
//     then returns the current direction
//

int RadioBatonTablet::toggleXAxisDirection(void) {
   xDirection = -xDirection;
   return xDirection;
}



//////////////////////////////
//
// RadioBatonTablet::toggleYAxisDirection -- switches the axis direction
//     of the y-axis (1=default, -1=reverse of default) and
//     then returns the current direction
//

int RadioBatonTablet::toggleYAxisDirection(void) {
   yDirection = -yDirection;
   return yDirection;
}



//////////////////////////////
//
// RadioBatonTablet::toggleZAxisDirection -- switches the axis direction
//     of the z-axis (1=default, -1=reverse of default) and
//     then returns the current direction
//

int RadioBatonTablet::toggleZAxisDirection(void) {
   zDirection = -zDirection;
   return zDirection;
}



//////////////////////////////
//
// RadioBatonTablet::toggleError -- does nothing just for compatibility
//

int RadioBatonTablet::toggleError(void) {
   return 0;   // always off (no errros possible)
}



//////////////////////////////
//
// RadioBatonTablet::pause -- does nothing just for compatibility
//

int RadioBatonTablet::pause(void) {
   // do nothing
}



//////////////////////////////
//
// RadioBatonTablet::unpause -- does nothing just for compatibility
//

int RadioBatonTablet::unpause(void) {
   // do nothing
}



//////////////////////////////
//
// RadioBatonTablet::getInputPort -- does nothing just for compatibility
//

int RadioBatonTablet::getInputPort(void) {
   return 0;
}



//////////////////////////////
//
// RadioBatonTablet::recordingQ -- does nothing just for compatibility
//

int RadioBatonTablet::recordingQ(void) {
   return 0;
}



//////////////////////////////
//
// RadioBatonTablet::setInputPort -- does nothing just for compatibility
//

void RadioBatonTablet::setInputPort(int aPort) {
   return 0;
}



//////////////////////////////
//
// RadioBatonTablet::openInputPort -- does nothing just for compatibility
//

void RadioBatonTablet::openInput(void) {
   // do nothing
}



//////////////////////////////
//
// RadioBatonTablet::openOutput -- does nothing just for compatibility
//

void RadioBatonTablet::openOutput(void) {
   // do nothing
}



//////////////////////////////
//
// RadioBatonTablet::togglePositionReporting -- does nothing just for 
//     compatibility.
//

void RadioBatonTablet::togglePositionReporting(void) {
   // do nothing
}



//////////////////////////////
//
// RadioBatonTablet::positionReportingOff -- does nothing just for 
//     compatibility.
//

void RadioBatonTablet::positionReportingOff(void) {
   // do nothing
}



//////////////////////////////
//
// RadioBatonTablet::positionReportingOn -- does nothing just for 
//     compatibility.
//

void RadioBatonTablet::positionReportingOn(void) {
   // do nothing
}



//////////////////////////////
//
// RadioBatonTablet::getPositionReporting -- does nothing just for 
//     compatibility.
//

void RadioBatonTablet::getPositionReporting(void) {
   return 1;
}



//////////////////////////////
//
// RadioBatonTablet::toggleOutputTrace -- does nothing just for 
//     compatibility.
//

void RadioBatonTablet::toggleOutputTrace(void) {
   // do nothing
}



//////////////////////////////
//
// RadioBatonTablet::getOutputTrace -- does nothing just for 
//     compatibility.
//

int RadioBatonTablet::getOutputTrace(void) {
   return 0;
}




//////////////////////////////
//
// RadioBatonTablet::toggleInputTrace -- does nothing just for 
//     compatibility.
//

void RadioBatonTablet::toggleInputTrace(void) {
   // do nothing
}



//////////////////////////////
//
// RadioBatonTablet::getInputTrace -- does nothing just for 
//     compatibility.
//

int RadioBatonTablet::getInputTrace(void) {
   return 0;
}



//////////////////////////////
//
// RadioBatonTablet::getNumInputPorts -- does nothing just for 
//     compatibility.
//

int RadioBatonTablet::getNumInputPorts(void) {
   return 1;
}



//////////////////////////////
//
// RadioBatonTablet::getNumOutputPorts -- does nothing just for 
//     compatibility.
//

int RadioBatonTablet::getNumOutputPorts(void) {
   return 1;
}



//////////////////////////////
//
// RadioBatonTablet::setOutputPort -- does nothing just for compatibility
//

void RadioBatonTablet::setOutputPort(int aPort) {
   return 0;
}



//////////////////////////////
//
// RadioBatonTablet::openOutputPort -- does nothing just for compatibility
//

void RadioBatonTablet::openOutput(void) {
   // do nothing
}



//////////////////////////////
//
// RadioBatonTablet::recordStateStop -- does nothing just for compatibility
//

void RadioBatonTablet::recordStateStop(void) {
   // do nothing
}



//////////////////////////////
//
// RadioBatonTablet::recordStateStart -- does nothing just for compatibility
//

void RadioBatonTablet::recordStateStart(const char* filename) {
   // do nothing
}





#endif /* VISUAL */


// md5sum: 1eeee028e6e996871bf46f37f7ccaf9b RadioBatonTablet.cpp [20050403]
