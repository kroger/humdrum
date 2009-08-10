//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun 25 21:54:28 PDT 2009
// Last Modified: Thu Jun 25 21:54:31 PDT 2009
// Filename:      ...sig/src/sig/PosixRegularExpression.cpp
// Web Address:   http://sig.sapp.org/src/sig/PosixRegularExpression.cpp
// Syntax:        C++; GNU regular expressions in C
//


#include <string.h>
#include "PosixRegularExpression.h"
#include "Array.h"

#ifndef OLDCPP
   #include <iostream>
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #include <iostream.h>
   #ifdef VISUAL
      #include <strstrea.h>
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif


//////////////////////////////
//
// PosixRegularExpression::PosixRegularExpression --
//

PosixRegularExpression::PosixRegularExpression(void) {
   valid       = 0;
   globalQ     = 0;
   extendedQ   = 1;
   ignorecaseQ = 0;

   search_string.setSize(1);
   search_string[0]  = '\0';

   replace_string.setSize(1);
   replace_string[0] = '\0';
}



//////////////////////////////
//
// PosixRegularExpression::~PosixRegularExpression --
//

PosixRegularExpression::~PosixRegularExpression() {
   // do nothing
}



//////////////////////////////
//
// PosixRegularExpression::setReplaceString -- store the search string
//

void PosixRegularExpression::setReplaceString(const char* replacestring) {
   int len = strlen(replacestring);
   replace_string.setSize(len+1);
   strcpy(replace_string.getBase(), replacestring);
}



//////////////////////////////
//
// PosixRegularExpression::setSearchString -- store the search string
//

void PosixRegularExpression::setSearchString(const char* searchstring) {
   if (strcmp(searchstring, search_string.getBase()) != 0) {
      valid = 0;
      int len = strlen(searchstring);
      search_string.setSize(len+1);
      strcpy(search_string.getBase(), searchstring);
   }
}



///////////////////////////////
//
// PosixRegularExpression::setGlobal -- do a global replace.
//

void PosixRegularExpression::setGlobal(void) {
   globalQ = 1;
}



///////////////////////////////
//
// PosixRegularExpression::setSingle -- do a single replace (not global)
//

void PosixRegularExpression::setSingle(void) {
   globalQ = 0;
}



///////////////////////////////
//
// PosixRegularExpression::setExtendedSyntax --
//

void PosixRegularExpression::setExtendedSyntax(void) {
   if (extendedQ != 1) {
      extendedQ = 1;
      valid = 0;
   }
}



///////////////////////////////
//
// PosixRegularExpression::setBasicSyntax --
//

void PosixRegularExpression::setBasicSyntax(void) {
   if (extendedQ != 0) {
      extendedQ = 0;
      valid = 0;
   }
}



///////////////////////////////
//
// PosixRegularExpression::setIgnoreCase --
//

void PosixRegularExpression::setIgnoreCase(void) {
   if (ignorecaseQ != 1) {
      ignorecaseQ = 1;
      valid = 0;
   }
}



///////////////////////////////
//
// PosixRegularExpression::setNoIgnoreCase --
//

void PosixRegularExpression::setNoIgnoreCase(void) {
   if (ignorecaseQ != 0) {
      ignorecaseQ = 0;
      valid = 0;
   }
}



///////////////////////////////
//
// initializeSearch -- set up a search with the current settings.
//

void PosixRegularExpression::initializeSearch(void) {

   int compflags = 0;
   if (extendedQ) {
      compflags |= REG_EXTENDED;
   } 
   if (ignorecaseQ) {
      compflags |= REG_ICASE;
   }

   int status = regcomp(&re, search_string.getBase(), compflags);
   if (status !=0) {
      Array<char> error_buffer(1024);
      regerror(status, &re, error_buffer.getBase(), error_buffer.getSize());
      cerr << error_buffer.getBase() << endl;
      exit(1);
   }

   valid = 1;
}



///////////////////////////////
//
// PosixRegularExpression::initializeSearch -- store the search string,
//    and complie the regular expression.
//

void PosixRegularExpression::initializeSearch(const char* searchstring) {

   setSearchString(searchstring);
   if (valid) {
      return;
   } else {
      initializeSearch();
   }

}
   

//////////////////////////////
//
// PosixRegularExpression::sar -- search and replace, destructively.
//   returns the number of replacements made in the search string.
//   default value: optionstring = NULL
//

int PosixRegularExpression::sar(Array<char>& inout, const char* searchstring,
      const char* replacestring, const char* optionstring)  {

   int i;
   if (optionstring != NULL) {
      i = 0; 
      int g_found = 0;
      int i_found = 0;
      while (optionstring[i] != '\0')  {
         switch (optionstring[i]) {
            case 'g': setGlobal(); g_found = 1; break;
            case 'i': setIgnoreCase(); i_found = 1; break;
         }
	 i++;
      }
      if (g_found == 0) {
         setSingle();
      }
      if (i_found == 0) {
         setNoIgnoreCase();
      }
   }

   if (search(inout.getBase(), searchstring) == 0) {
      return 0;
   }


   Array<char> temp_buffer;
   temp_buffer.setSize(inout.getSize());
   strcpy(temp_buffer.getBase(), inout.getBase());
   return searchAndReplace(inout, temp_buffer.getBase(), searchstring, 
         replacestring);
}



//////////////////////////////
//
// PosixRegularExpression::tr -- translate characters
//

void PosixRegularExpression::tr(Array<char>& inout, const char* inputlist, 
      const char* outputlist) {
   char table[256];
   int i;
   for (i=0; i<256; i++) {
      table[i] = i;
   }

   Array<char> inchars;
   Array<char> outchars;
   inchars.setSize(256);  
   inchars.setGrowth(256);  
   inchars.setSize(0);  
   outchars.setSize(256);  
   outchars.setGrowth(256);  
   outchars.setSize(0);  

   expandList(inchars, inputlist);
   expandList(outchars, outputlist);

   int maxx = inchars.getSize();
   if (outchars.getSize() < maxx) {
      maxx = outchars.getSize();
   }
  
   for (i=0; i<maxx; i++) {
      table[(unsigned char)inchars[i]] = outchars[i];
   }

   for (i=0; i<inout.getSize(); i++) {
      inout[i] = table[(unsigned char)inout[i]];
   }
}



//////////////////////////////
//
// PosixRegularExpression::expandList -- expand a translation string into 
//     individual characters.
//

void PosixRegularExpression::expandList(Array<char>& expandlist, 
     const char* input) {

   char ch;
   int target;
   int source;

   int j;
   int i = 0;
   while (input[i] != '\0') {
      switch (input[i]) {
         case '-': 
            if ((i == 0) || (input[i+1] == '\0')) {
               // treat as a regular character
               ch = '-';
               expandlist.append(ch);
            } else {
               // treat as a character range marker
               // literal markers are not allowed after dash
               // in this implementation of tr
               target = (unsigned char)input[i+1];
               source = (unsigned char)input[i-1];
               for (j=source+1; j<=target; j++) {
                  ch = (char)j;
                  expandlist.append(ch);
               }
               i+= 1;
            }
            break;
         case '\\':                    // deal with \n, \t, \r in the future...
            if (input[i+1] != '\0') {
               ch = input[i+1];
               expandlist.append(ch);
               i++;
            }
            break;
         default:
	    ch = input[i];
            expandlist.append(ch);
      }
      i++;
   }
   ch = '\0';
   expandlist.append(ch);
   expandlist.setSize(expandlist.getSize()-1);
}



//////////////////////////////
//
// PosixRegularExpression::searchAndReplace -- returns the number of times the
//    search string was replaced.
//

int PosixRegularExpression::searchAndReplace(Array<char>& output, const char* input,
      const char* searchstring, const char* replacestring) {
   initializeSearch(searchstring);
   setReplaceString(replacestring);

   return searchAndReplace(output, input);
}



//////////////////////////////
//
// PosixRegularExpression::searchAndReplace -- do a search and replace with
//   the same search and replace from a previous call.
//

int PosixRegularExpression::searchAndReplace(Array<char>& output, 
      const char* input) {

   if (valid == 0) {
      initializeSearch();
   }

   SSTREAM tempdata;
   int counter = 0;
   int i;
   int status = regexec(&re, input, 1, &match, 0);

   while (status == 0) {
      counter++;
      for (i=0; i<match.rm_so; i++) {
         tempdata << input[i];
      }
      tempdata << replace_string.getBase();
      input += match.rm_eo;
      // REG_NOTBOL = start of input is not Beginning Of Line
      status = regexec(&re, input, 1, &match, REG_NOTBOL);
      if (!globalQ) {
         break;
      }
      if (input[0] == '\0') {
         break;
      }
   }

   tempdata << input;   // store the piece of input after last replace.
   tempdata << ends;
   int len = strlen(tempdata.CSTRING);
   output.setSize(len+1);
   strcpy(output.getBase(), tempdata.CSTRING);
   return counter;
}



//////////////////////////////
//
// PosixRegularExpression::search --  returns 0 if match was not found, 
//   otherwise returns the index+1 of the first match found.
//

int PosixRegularExpression::search(const char* input, const char* searchstring) {

   setSearchString(searchstring);
   if (valid == 0) {
      initializeSearch();
   }

   return search(input);
}



//////////////////////////////
//
// PosixRegularExpression::search --  returns 0 if match was not found, 
//   otherwise returns the index+1 of the first match found.
//

int PosixRegularExpression::search(const char* input) {
   if (valid == 0) {
      initializeSearch();
   }
   int status = regexec(&re, input, 1, &match, 0);
   if (status == 0) {
      // successful match, so return position of beginning of match
      // plus one.
      return match.rm_so+1;
   } else {
      return 0;
   }
}



