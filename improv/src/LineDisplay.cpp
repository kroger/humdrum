//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Feb 27 02:44:02 PST 1999
// Last Modified: Sun Feb 28 14:31:01 PST 1999
// Last Modified: Sat Mar 27 18:26:07 PST 1999
// Last Modified: Mon Mar 12 11:52:54 PST 2001 (fixing bugs)
// Filename:      ...sig/maint/code/control/LineDisplay/LineDisplay.cpp
// Web Address:   http://sig.sapp.org/src/sig/ineDisplay.cpp
// Syntax:        C++
// Note:          all functions are not completely tested yet
//
// Description:   Simple Line display in the terminal where the
//                current line of text output can be modified.
//
//

#include "LineDisplay.h"

#include <stdlib.h>
#include <string.h>


//////////////////////////////
//
// LineDisplay::LineDisplay --
//

LineDisplay::LineDisplay(void) { 
   outstream = &cout;        // output stream location
   line = new char[70 + 2];      
   line[81] = '\0';
   linelength = 70;      // max length of a display line
   column = 1;           // current column of display (offset 1)
   displayColumn = 1;    // max display column used (offset 1)
   tabsize = 8;          // default size of tabs
   tabchar = ' ';        // character to fill tabs with
   mode = DISPLAY_TRIM_MODE; // for line wrap or not
}


LineDisplay::LineDisplay(int len, ostream& out) { 
   outstream = &out;         // output stream location
   if (len < 1) {
      cout << "Error: cannot have a non-positive line length" << endl;
      exit(1);
   }
   line = new char[len + 2];      
   line[len + 1] = '\0';
   linelength = len;     // max length of a display line
   column = 1;           // current column of display (offset 1)
   displayColumn = 1;    // max display column used (offset 1)
   tabsize = 8;          // default size of tabs
   tabchar = ' ';        // character to fill tabs with
   mode = DISPLAY_TRIM_MODE; // for line wrap or not
}


//////////////////////////////
//
// LineDisplay::~LineDisplay --
//

LineDisplay::~LineDisplay() { 
   newline();
   if (line != NULL) {
      delete [] line;
   }
   line = NULL;
};



//////////////////////////////
//
// LineDisplay::centerline -- display the given line in the center
//    of the current line, erase everything on the current line.
//    default value centerColumn = -1;
//

void LineDisplay::centerline(const char* aLine, int centerColumn) { 
   int length = strlen(aLine);
   if (centerColumn < 0) {
      centerColumn = linelength / 2;
   }
   int stringcenter = length / 2;

   if (stringcenter >= centerColumn) {
      eraseline();
      display(aLine);
      return;
   }

   eraseline();
   int blanks = centerColumn - stringcenter;
   for (int j=0; j<blanks; j++) {
      display(' ');
   }
   display(aLine);
}



//////////////////////////////
//
// LineDisplay::display -- display the character(s) starting at
//    the current position.  If the line length is exceded, then
//    displayed character(s) are determined by the mode (wrap or trim).
//    default value: flushQ = 1.
//

void LineDisplay::display(const char* aString, int flushQ) { 
   int length = strlen(aString);
   int i, j;
   for (i=0; i<length; i++) {
      switch (aString[i]) {
         case '\t': 
            for (j=0; j<tabsize; j++) {
               display(tabchar, 0);
            }
            break;
         default:
            display(aString[i], 0);
      }
   }
   if (flushQ) {
      outstream->flush();
   }
}


void LineDisplay::display(char aChar, int flushQ) { 
   if (getColumn() <= linelength) {
      *outstream << aChar;
      if (aChar == '\n') {
         for (int j=0; j<column; j++) {
            line[j] = '\0';
         }
         column = 1;
      } else {
         if (getColumn() == 0) {
            line[getColumn()] = aChar;
            line[getColumn()+1] = '\0';
         } else {
            line[getColumn()-1] = aChar;
            line[getColumn()] = '\0';
         }
         increaseColumn();
      }
      if (flushQ) {
         outstream->flush();
      }
   } else {
      switch (mode) {
         case DISPLAY_TRIM_MODE:
            backspace();
            display(aChar, flushQ);
            break;
   
         case DISPLAY_WRAP_MODE:
            newline();
            display(aChar, flushQ);
            break;
      }
   }
}



//////////////////////////////
//
// LineDisplay::eraseline -- make the current line blank
//

void LineDisplay::eraseline(void) { 
   backspace(getColumn()-1);
}



//////////////////////////////
//
// LineDisplay::backspace -- go back one column and blank out the
//     current position.  Cannot go back to a previous line.
//     default value: number = 1, flushQ = 1;
//

void LineDisplay::backspace(int number, int flushQ) { 
   if (number > 0 && getColumn() > 1) {
      *outstream << "\b \b";
      if (flushQ) {
         outstream->flush();
      }
      column--;
      if (line[getColumn()+1] == '\0') {
         line[getColumn()] = '\0';
      }
      number--;
      if (number > 0) {
         backspace(number, flushQ);
      }
   }
}



//////////////////////////////
//
// LineDisplay::back -- go to column 1 but do not erase anything.
//     default value: flushQ = 1
//

void LineDisplay::back(int flushQ) { 
   *outstream << '\r';
   if (flushQ) {
      outstream->flush();
   }
}



//////////////////////////////
//
// LineDisplay::newline -- send a newline character and start a new line.
//     default values: count = 1, flushQ = 1
//

void LineDisplay::newline(int count, int flushQ) { 
   for (int i=0; i<count; i++) {
      *outstream << '\n';
   }
   column = 1;
   displayColumn = 1;
   if (flushQ) {
      outstream->flush();
   }
   strcpy(line, "");
}



//////////////////////////////
//
// LineDisplay::getLine -- return the character string of the current line
//     on the display.
//

const char* LineDisplay::getLine(void) const { 
   line[displayColumn] = '\0';
   return line;
}



//////////////////////////////
//
// LineDisplay::getNumColumns -- return the number of columns the 
//    display is intended to have.
//

int LineDisplay::getNumColumns(void) const { 
   return linelength;
}



//////////////////////////////
//
// LineDisplay::getColumn -- return the column number that the display
//     is currently on.  Offset from 1.
//

int LineDisplay::getColumn(void) const { 
   return column;
}



//////////////////////////////
//
// LineDisplay::getMode -- returns the current line-wrap mode
//

int LineDisplay::getMode(void) const {
   return mode;
}



/////////////////////////////
//
// LineDisplay::linejustify -- 
//     default value: flushQ = 1
//

void LineDisplay::leftjustify(const char* aLine, int flushQ) {
   int length = strlen(aLine);
   int start = linelength - length + 1;
   if (start < 1) {
      start = 1;
   } else if (start > linelength) {
      start = linelength;
   }
   moveto(start);
   display(aLine); 
}



//////////////////////////////
//
// LineDisplay::moveto -- move to specified column without destroying
//     any of the current display characters.
//

void LineDisplay::moveto(int col) { 
   if (col < 1 || col > linelength) {
      cout << "Error: invalid column specification: " << col << endl;
   }
 
   if (col == column) {
      return;
   }

   int count;
   if (col < column) {
      count = column - col;
      for (int i=0; i<count; i++) {
         *outstream << '\b';
         column--;
      }
      return;
   }

   count = col - column;
   for (int j=0; j<count; j++) {
      if (column < displayColumn) {
         display(line[column]);
      } else {
         display(' ');
      }
   }
}



//////////////////////////////
//
// LineDisplay::setColumn -- similar to the moveto command but
//    will fill in the columns between current location and the new 
//    location with the specified character.
//
//    default value: fillCharacter = ' '
//

int LineDisplay::setColumn(int col, char fillCharacter) { 
   if (col < 1 || col > linelength) {
      cout << "Error: invalid index: " << col << endl;
      exit(1);
   }

   int count = col - column;
 
   if (count == 0) {
      return column;
   }

   if (count > 0) {
      for (int i=0; i<count; i++) {
         display(fillCharacter);
      }
   } else {
      for (int j=0; j<-count; j++) {
         *outstream << '\r';
         column--;
         display(fillCharacter);
         *outstream << '\r';
         column--;
      }
   }

    return column;
}



//////////////////////////////
//
// LineDisplay::setLine -- 
//

void LineDisplay::setLine(const char* aString) { 
   eraseline();
   strcpy(line, "");
   display(aString);
}



//////////////////////////////
//
// LineDisplay::setModeWrap --
//

void LineDisplay::setModeWrap(void) { 
   mode = DISPLAY_WRAP_MODE;
}



//////////////////////////////
//
// LineDisplay::setModeTrim --
//

void LineDisplay::setModeTrim(void) { 
   mode = DISPLAY_TRIM_MODE;
}



//////////////////////////////
//
// LineDisplay::setTabSize --
//

void LineDisplay::setTabSize(int size) { 
   tabsize = size;
}



///////////////////////////////////////////////////////////////////////////
//
// private fucntions
//

//////////////////////////////
//
// LineDisplay::increaseColumn --
//

void LineDisplay::increaseColumn() {
   column++;
   if (getColumn() > displayColumn) {
      displayColumn = getColumn();
   }
}

// md5sum: 80106474ab1cff5f0d1d6c121b659c1e LineDisplay.cpp [20050403]
