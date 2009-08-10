//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Feb 27 02:44:02 PST 1999
// Last Modified: Sat Feb 27 02:44:07 PST 1999
// Last Modified: Sat Mar 27 18:26:16 PST 1999
// Filename:      ...sig/maint/code/control/LineDisplay/LineDisplay.h
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/include/LineDisplay.h
// Syntax:        C++
// 
// Description:   Simple Line display in the terminal where the
//                current line of text output can be modified.
//

#ifndef _LINEDISPLAY_INCLUDED_H
#define _LINEDISPLAY_INCLUDED_H

#define DISPLAY_TRIM_MODE 0
#define DISPLAY_WRAP_MODE 1

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif


class LineDisplay {
   public:

                  LineDisplay      (void);
                  LineDisplay      (int len, ostream& out = cout);
                 ~LineDisplay      ();

      void        centerline       (const char* aLine, int centerColumn = -1);
      void        center           (const char* aLine, int centerColumn = -1);
      void        display          (const char* aString, int flushQ = 1);
      void        display          (char aChar, int flushQ = 1);
      void        eraseline        (void);
      void        flush            (void);
      void        backspace        (int number = 1, int flushQ = 1);
      void        back             (int flushQ = 1);
      void        newline          (int count = 1, int flushQ = 1);
      const char* getLine          (void) const;
      int         getNumColumns    (void) const;
      int         getColumn        (void) const;
      int         getMode          (void) const;
      void        leftjustify      (const char* aLine, int flushQ = 1);
      void        moveto           (int col);
      int         setColumn        (int col, char fillCharacter = ' ');
      void        setLine          (const char* aString);
      void        setModeTrim      (void);
      void        setModeWrap      (void);
      void        setTabSize       (int size);

   protected:
      int tabsize;              // default size of tabs
      char tabchar;             // character to fill tabs with
      int mode;                 // trim = 0; wrap = 1; 
      int column;               // current column of display (offset 1)
      int linelength;           // max length of a display line
      int displayColumn;        // max length of a display line
      char* line;               // copy of current line
      ostream* outstream;       // output stream location

   private:
      void     increaseColumn   (void);

};


#endif  /* _LINEDISPLAY_INCLUDED_H */



// md5sum: 422941ba1c0583e6ee35bbcbd024dd28 LineDisplay.h [20050403]
