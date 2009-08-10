//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jan 12 09:03:35 PST 1999
// Last Modified: Tue Jan 12 09:03:35 PST 1999
// Filename:      KeyboardInput_visual.h
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/src/KeyboardInput_visual.cpp
// Syntax:        C++ 
//
// Description:   Controls the behaviour of the computer keyboard so
//                that individual keys from the keyboard can be read
//                immediately after they are pressed rather than when
//                the enter key is pressed.  Specific to DOS computers.
//
//

#ifdef VISUAL

#include "KeyboardInput_visual.h"
#include <conio.h>  /* for kbhit() function */

// declarations of static variables:
int  KeyboardInput_visual::classCount = 0;


//////////////////////////////
//
// KeyboardInput_visual::KeyboardInput_visual --
//

KeyboardInput_visual::KeyboardInput_visual(void) {
   if (classCount <= 0) {
      initialize();
   }
   classCount++;
}



//////////////////////////////
//
// KeyboardInput_visual::~KeyboardInput_visual --
//

KeyboardInput_visual::~KeyboardInput_visual() {
   classCount--;
   if (classCount <= 0) {
      deinitialize();
   }
}



//////////////////////////////
//
// KeyboardInput_visual::hit -- returns true if a keyboard character
//   has been hit.  Polls stdin to see if there is any input
//   available.
//

int KeyboardInput_visual::hit(void) {
   return ::kbhit();
}



//////////////////////////////
//
// KeyboardInput_visual::getch -- returns the next character typed
//   one the keyboard, will be -1 if there is not keyboard char to get.
//   (not sure what returns if there is not keyboard char to get.)
//

int KeyboardInput_visual::getch(void) {
   return ::getch();
}



//////////////////////////////
//
// KeyboardInput_visual::newset -- 
//

void KeyboardInput_visual::newset(void) {
   // do nothing
}



//////////////////////////////
//
// KeyboardInput_visual::oldset -- 
//

void KeyboardInput_visual::oldset(void) {
   // do nothing
}



//////////////////////////////
//
// KeyboardInput_visual::reset -- 
//

void KeyboardInput_visual::reset(void) {
   // do nothing
}



///////////////////////////////////////////////////////////////////////////
//
// private functions
//


//////////////////////////////
//
// KeyboardInput_visual::initialize -- 
//

void KeyboardInput_visual::initialize(void) {
   // do nothing
}



//////////////////////////////
//
// KeyboardInput_visual::deinitialize -- 
//

void KeyboardInput_visual::deinitialize(void) {
   // do nothing
}



#endif  // VISUAL
// md5sum: 618df8c02f340914f3a15d7c1f844326 KeyboardInput_visual.cpp [20050403]
