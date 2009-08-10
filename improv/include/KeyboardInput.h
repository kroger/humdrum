//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jan 12 10:05:56 PST 1999
// Last Modified: Tue Jan 12 10:06:01 PST 1999
// Filename:      ...sig/maint/code/control/KeyboardInput/KeyboardInput.h
// Web Address:   http://sig.sapp.org/include/sig/KeyboardInput.h
// Syntax:        C++ 
//
// Description:   Controls the behaviour of the computer keyboard so
//                that individual keys from the keyboard can be read
//                immediately after they are pressed rather than when
//                the enter key is pressed.  
//

#ifndef _KEYBOARDINPUT_H_INCLUDED
#define _KEYBOARDINPUT_H_INCLUDED

#ifdef VISUAL
   #define KEYBOARDINPUT  KeyboardInput_visual
   #include "KeyboardInput_visual.h"
#elif defined(LINUX)
   #define KEYBOARDINPUT  KeyboardInput_unix
   #include "KeyboardInput_unix.h"
#elif defined(OSXPC) || defined(OSXOLD)
   #define KEYBOARDINPUT  KeyboardInput_osx
   #include "KeyboardInput_osx.h"
#else
   #define KEYBOARDINPUT  KeyboardInput_unix
   #include "KeyboardInput_unix.h"
#endif


class KeyboardInput : protected KEYBOARDINPUT {
   public:
      KeyboardInput(void) : KEYBOARDINPUT() { };
     ~KeyboardInput() { KEYBOARDINPUT::deinitialize(); }

      int    hit(void)    { return KEYBOARDINPUT::hit(); }
      int    getch(void)  { return KEYBOARDINPUT::getch(); }
      void   newset(void) { KEYBOARDINPUT::newset(); }
      void   oldset(void) { KEYBOARDINPUT::oldset(); }
      void   reset(void)  { KEYBOARDINPUT::reset(); }

      void   deinitialize(void)  { KEYBOARDINPUT::deinitialize(); }
};


#endif  /* _KEYBOARDINPUT_H_INCLUDED */



// md5sum: be4da17ef48f52cae2da0d9e5285892b KeyboardInput.h [20050403]
