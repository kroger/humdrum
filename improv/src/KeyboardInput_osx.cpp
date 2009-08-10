//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jun 12 13:02:32 PDT 2009
// Last Modified: Fri Jun 12 13:02:35 PDT 2009
// Filename:      KeyboardInput_osx.cpp
// Web Address:   http://improv.sapp.org/src/KeyboardInput_osx.cpp
// Syntax:        C++ 
// Description:   Controls the behaviour of the computer keyboard so
//                that individual keys from the keyboard can be read
//                immediately after they are pressed rather than when
//                the enter key is pressed.  Specific to OSX computers.
//
// References:    Technical Note TN2071:
//                Porting Command-Line Unix Tools to Mac OS X
//                http://developer.apple.com/technotes/tn2002/tn2071.html
//                
//                http://developer.apple.com/documentation/Cocoa/Conceptual/EventOverview/HandlingKeyEvents/HandlingKeyEvents.html
//                http://www.mactech.com/macintosh-c/chap02-1.html
//                http://developer.apple.com/documentation/Cocoa/Conceptual/EventOverview/EventOverview.pdf
//
// To Consider:	  http://linuxreviews.org/man/termcap
//                The termcap database is an obsolete facility for
//                describing the capabilities of character-cell terminals
//                and printers. It is retained only for capability with
//                old programs; new ones should use the terminfo(5)
//                database and associated libraries. (1996-10-21)
//

#if !defined(VISUAL) && !defined(LINUX)

#include "KeyboardInput_osx.h"
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

#define KEYBOARD_OLDSET 0
#define KEYBOARD_NEWSET 1


// declarations of static variables:
int             KeyboardInput_osx::classCount = 0;
struct termios  KeyboardInput_osx::current_io;
struct termios  KeyboardInput_osx::original_io;


//////////////////////////////
//
// KeyboardInput_osx::KeyboardInput_osx --
//

KeyboardInput_osx::KeyboardInput_osx(void) {
   if (classCount <= 0) {
      initialize();
   }
   classCount++;
}



//////////////////////////////
//
// KeyboardInput_osx::~KeyboardInput_osx --
//

KeyboardInput_osx::~KeyboardInput_osx() {
   classCount--;
   if (classCount <= 0) {
      deinitialize();
   }
}



//////////////////////////////
//
// KeyboardInput_osx::hit -- returns true if a keyboard character
//   has been hit.  Polls stdin to see if there is any input
//   available.
//

int KeyboardInput_osx::hit(void) {
   fd_set rfds;
   struct timeval tv;
   FD_ZERO(&rfds);
   FD_SET(0, &rfds);
   tv.tv_sec = 0;
   tv.tv_usec = 0;
   select(1, &rfds, NULL, NULL, &tv);

   if (FD_ISSET(0, &rfds)) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// KeyboardInput_osx::getch -- returns the next character typed
//   one the keyboard, will be -1 if there is not keyboard char to get.
//   (not sure what returns if there is not keyboard char to get.)
//

int KeyboardInput_osx::getch(void) {
   char c;
   cin.read(&c, 1);
   return (int)c;
}



//////////////////////////////
//
// KeyboardInput_osx::newset -- use the unbuffered keyboard input
//

void KeyboardInput_osx::newset(void) {
   // int status = ioctl(0, TCSETA, &current_io);
   int status = tcsetattr(0, TCSANOW, &current_io);
   if (status < 0) {
      cout << "Error: coult not set Terminal I/O values." << endl;
      exit(1);
   }

   termioset = KEYBOARD_NEWSET;
}



//////////////////////////////
//
// KeyboardInput_osx::oldset -- use the buffered keyboard input
//

void KeyboardInput_osx::oldset(void) {
   // int status = ioctl(0, TCSETA, &original_io);
   int status = tcsetattr(0, TCSANOW, &original_io);
   if (status < 0) {
      cout << "Error: coult not set Terminal I/O values." << endl;
      exit(1);
   }

   termioset = KEYBOARD_OLDSET;
}



//////////////////////////////
//
// KeyboardInput_osx::reset -- when putting a program into the background
//	with ^Z and then returning to the program, you have to
// 	sent the current_io structure again.
//

void KeyboardInput_osx::reset(void) {
   if (termioset == KEYBOARD_NEWSET) {
      newset();
   } else {
      oldset();
   }
}



///////////////////////////////////////////////////////////////////////////
//
// private function
//


//////////////////////////////
//
// KeyboardInput_osx::initialize -- set up the variables necessary.
//

void KeyboardInput_osx::initialize(void) {
   int status;

   // get the original Terminal setting to restore when finished:
   // status = ioctl(0, TCGETA, &original_io);
   status = tcgetattr(0, &original_io);
   if (status < 0) {
      cout << "Error: could not access terminal settings" << endl;
      exit(1);
   }

   // copy and modify the current Terminal I/O settings
   memcpy (&current_io, &original_io, sizeof(struct termios));
   current_io.c_cc[VMIN] = 1;              // wait for one character
   current_io.c_cc[VTIME] = 0;             // don't wait to return character
   current_io.c_lflag &= ~ICANON;          // unbuffered input
   current_io.c_lflag &= ~ECHO;            // turn off local display

   // now, set the new Terminal I/O settings
   // status = ioctl(0, TCSETA, &current_io);
   status = tcsetattr(0, TCSANOW, &current_io);
   if (status < 0) {
      cout << "Warning: could not set Terminal I/O values." << endl;
   }

   termioset = KEYBOARD_NEWSET;
}



//////////////////////////////
//
// KeyboardInput_osx::deinitialize -- recover the original terminal i/o
//     settings.
//

void KeyboardInput_osx::deinitialize(void) {
   int status;
 
   // status = ioctl(0, TCSETA, &original_io);
   status = tcsetattr(0, TCSANOW, &original_io);
   if (status < 0) {
      cout << "Error: could not restore original terminal I/O settings." 
           << endl;
      exit(1);
   }

   termioset = KEYBOARD_OLDSET;
}



#endif  /* VISUAL / LINUX */

// md5sum: dcdce9834efb7580f429729d469c1ed7 KeyboardInput_osx.cpp [20020518]
