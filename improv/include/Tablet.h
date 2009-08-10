//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  9 13:04:03 PDT 2003
// Last Modified: Sat Aug  9 13:04:06 PDT 2003
// Filename:      ...sig/code/control/Tablet/Tablet.h
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/include/Tablet.h
// Syntax:        C++ 
//
//

#ifndef _TABLET_H_INCLUDED
#define _TABLET_H_INCLUDED

#ifdef VISUAL

#include "Tablet_visual.h"


class Tablet : public Tablet_visual { 
   public:

      // radio baton like functions
      void pause(void) { }
      void unpause(void) { }
      int recordingQ(void) { }
      void recordStateStop(void) { }
      void recordStateStart(const char* filename) { }
};


  
#endif  /* VISUAL */

#endif  /* _TABLET_H_INCLUDED */


// md5sum: 2b583798f5db5c4ee377232f305b73bc Tablet.h [20050403]
