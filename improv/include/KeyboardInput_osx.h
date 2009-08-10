//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jun 12 12:59:26 PDT 2009
// Last Modified: Fri Jun 12 12:59:29 PDT 2009
// Filename:      ...improv/include/KeyboardInput_osx.h
// Web Address:   http://improv.sapp.org/include/KeyboardInput_osx.h
// Syntax:        C++ 
//
// Description:   Controls the behaviour of the computer keyboard so
//                that individual keys from the keyboard can be read
//                immediately after they are pressed rather than when
//                the enter key is pressed.  Specific to OSX computers.
//

#ifndef _KEYBOARDINPUT_OSX_H_INCLUDED
#define _KEYBOARDINPUT_OSX_H_INCLUDED

#if !defined(VISUAL) && !defined(LINUX)

#include <termios.h>

class KeyboardInput_osx {
   public:
                KeyboardInput_osx      (void);
               ~KeyboardInput_osx      ();

      int       hit                     (void);
      int       getch                   (void);
      void      newset                  (void);
      void      oldset                  (void);
      void      reset                   (void);

      void      initialize               (void);
      void      deinitialize             (void);

   protected:
      int                    termioset;
      static int             classCount;
      static struct termios  current_io;
      static struct termios  original_io;

};

#endif  /* VISUAL / LINUX */

#endif  /* _KEYBOARDINPUT_OSX_H_INCLUDED */



// md5sum: 823cae0ceecc1389483ed31d2eced3a5 KeyboardInput_osx.h [20050403]
