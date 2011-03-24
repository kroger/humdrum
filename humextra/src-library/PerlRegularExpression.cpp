// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jun 29 14:25:53 PDT 2009
// Last Modified: Mon Jun 29 14:26:01 PDT 2009
// Filename:      ...sig/src/sig/PerlRegularExpression.cpp
// Web Address:   http://sig.sapp.org/src/sig/PerlRegularExpression.cpp
// Syntax:        C++; Perl Compatible Regular Expressions (http://www.pcre.org)
//
// See also:      http://www.adp-gmbh.ch/cpp/regexp/pcre/functions.html
//                http://regexkit.sourceforge.net/Documentation/pcre
//                http://www.gammon.com.au/pcre/pcreapi.html
//
// Syntax:        http://regexkit.sourceforge.net/Documentation/pcre/pcresyntax.html
//

//////////////////////////////////////////////////
//
// pcre *pcre_compile (
//        const          char  *pattern, 
//                       int    options,
//        const          char **errptr,  
//                       int   *erroffset,
//        const unsigned char  *tableptr);
//
// pattern: regular expression you are going to search with
// options: 
//  PCRE_ANCHORED         Force pattern anchoring
//  PCRE_AUTO_CALLOUT     Compile automatic callouts
//  PCRE_BSR_ANYCRLF      \R matches only CR, LF, or CRLF
//  PCRE_BSR_UNICODE      \R matches all Unicode line endings
//  PCRE_CASELESS         Do caseless matching
//  PCRE_DOLLAR_ENDONLY   $ not to match newline at end
//  PCRE_DOTALL           . matches anything including NL
//  PCRE_DUPNAMES         Allow duplicate names for subpatterns
//  PCRE_EXTENDED         Ignore whitespace and # comments
//  PCRE_EXTRA            PCRE extra features (not much use currently)
//  PCRE_FIRSTLINE        Force matching to be before newline
//  PCRE_MULTILINE        ^ and $ match newlines within data
//  PCRE_NEWLINE_ANY      Recognize any Unicode newline sequence
//  PCRE_NEWLINE_ANYCRLF  Recognize CR, LF, and CRLF as newline sequences
//  PCRE_NEWLINE_CR       Set CR as the newline sequence
//  PCRE_NEWLINE_CRLF     Set CRLF as the newline sequence
//  PCRE_NEWLINE_LF       Set LF as the newline sequence
//  PCRE_NO_AUTO_CAPTURE  Disable numbered capturing parentheses 
//                        (named ones available)
//  PCRE_UNGREEDY         Invert greediness of quantifiers
//  PCRE_UTF8             Run in UTF-8 mode
//  PCRE_NO_UTF8_CHECK    Do not check the pattern for UTF-8 validity 
//                        (only relevant if PCRE_UTF8 is set)
// errptr = pointer to an error string if compile is unsuccessful.
// erroroffset = index of start of error (presumably)
// tableptr = NULL or output of pcre_maketables()
//
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//
// int pcre_exec(
//         const pcre       *code, 
//         const pcre_extra *extra,
//         const char       *subject, 
//               int         length, 
//               int         startoffset,
//               int         options, 
//               int        *ovector, 
//               int         ovecsize);
//
// code:        PCRE structure created with pcre_compile().
// extra:       NULL or analysis created with pcre_study().
// subject:     String to search.
// length:      Length of string to search.
// startoffset: Offset in bytes in the subject at which to start matching.
// options: 
//    PCRE_ANCHORED    == Match only at the first position.
//    PCRE_BSR_ANYCRLF == \R matche only CR, LF, or CRLF.
//    PCRE_BSR_UNICODE == \R matches all Unicoe line endings.
//    PCRE_NEWLINE_ANY == Recognize any Unicode newline sequence.
//    PCRE_NEWLINE_ANYCRLF == Recognize CR, LF, and CRLF as newline sequences
//    PCRE_NEWLINE_CR  == Set CR as the newline sequence.
//    PCRE_NEWLINE_CRLF== Set CRLF as the newline sequence.
//    PCRE_NEWLINE_LF  == Set LF as the newline sequence.
//    PCRE_NOTBOL    == Subject is not at the beginnign of a line
//    PCRE_NOTEOL    == Subject is not at the end of a line
//    PCRE_NOTEMPTY  == An empty string is not a valid match
//    PCRE_NO_UTF8_CHECK == Do not check the subject for UTF-8 vailidity
//                          (only relevant if PCRE_UTF8 set in pcre_compile())
//    PCRE_PARTIAL   == Return PCRE_ERROR_PARTIAL for a partial match.
// ovector:  Vector of ints for matching offsets.
//    [0] = starting index
//    [1] = stopping index
//    [2] = 
// ovecsize: size of ovector divided by three
//
//
// When a match is successful, information about captured substrings is
// returned in pairs of integers, starting at the beginning of ovector, and
// continuing up to two-thirds of its length at the most. The first element
// of a pair is set to the offset of the first character in a substring,
// and the second is set to the offset of the first character after the end
// of a substring. The first pair, ovector[0] and ovector[1], identify the
// portion of the subject string matched by the entire pattern. The next
// pair is used for the first capturing subpattern, and so on. The value
// returned by pcre_exec() is one more than the highest numbered pair that
// has been set. For example, if two substrings have been captured, the
// returned value is 3. If there are no capturing subpatterns, the return
// value from a successful match is 1, indicating that just the first pair
// of offsets has been set.
//
// If the vector is too small to hold all the captured substring offsets,
// it is used as far as possible (up to two-thirds of its length), and the
// function returns a value of zero. In particular, if the substring offsets
// are not of interest, pcre_exec() may be called with ovector passed as NULL
// and ovecsize as zero. However, if the pattern contains back references
// and the ovector is not big enough to remember the related substrings,
// PCRE has to get additional memory for use during matching. Thus it is
// usually advisable to supply an ovector.
//
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//
// pcre_extra* pcre_study(
//        const pcre  *code, 
//        int          options, 
//        const char **errptr);
// 
// If a compiled pattern is going to be used several times, it is worth
// spending more time analyzing it in order to speed up the time taken for
// matching. The function pcre_study() takes a pointer to a compiled pattern
// as its first argument. If studying the pattern produces additional
// information that will help speed up matching, pcre_study() returns a
// pointer to a pcre_extra block, in which the study_data field points to
// the results of the study.
// 
// The returned value from pcre_study() can be passed directly to
// pcre_exec(). However, a pcre_extra block also contains other fields that
// can be set by the caller before the block is passed; these are described
// below in the section on matching a pattern.
// 
// If studying the pattern does not produce any additional information
// pcre_study() returns NULL. In that circumstance, if the calling program
// wants to pass any of the other fields to pcre_exec(), it must set up
// its own pcre_extra block.
// 
// The second argument of pcre_study() contains option bits. At present,
// no options are defined, and this argument should always be zero.
// 
// The third argument for pcre_study() is a pointer for an error message. If
// studying succeeds (even if no data is returned), the variable it points
// to is set to NULL. Otherwise it is set to point to a textual error
// message. This is a static string that is part of the library. You must
// not try to free it. You should test the error pointer for NULL after
// calling pcre_study(), to be sure that it has run successfully.
//
// At present, studying a pattern is useful only for non-anchored patterns
// that do not have a single fixed starting character. A bitmap of possible
// starting bytes is created.
//
//////////////////////////////////////////////////


#include <string.h>
#include "PerlRegularExpression.h"
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
// PerlRegularExpression::PerlRegularExpression --
//

PerlRegularExpression::PerlRegularExpression(void) {
   valid       = 0;
   globalQ     = 0;
   extendedQ   = 1;  // always extended for PCRE
   ignorecaseQ = 0;
   studyQ      = 0;
   anchorQ     = 0;

   output_substrings.setSize(3 * 100);  // has to be a multiple of 3
   output_substrings.setAll(0);

   pre = NULL;
   pe  = NULL;

   search_string.setSize(1);
   search_string[0]  = '\0';

   replace_string.setSize(1);
   replace_string[0] = '\0';

   substring.setSize(1);
   substring[0] = '\0';

   input_string.setSize(1);
   input_string[0] = '\0';
}



//////////////////////////////
//
// PerlRegularExpression::~PerlRegularExpression --
//

PerlRegularExpression::~PerlRegularExpression() {
   if (pre != NULL) {
      pcre_free(pre);
      pre = NULL;
   }
	 
}



//////////////////////////////
//
// PerlRegularExpression::setReplaceString -- store the search string
//

void PerlRegularExpression::setReplaceString(const char* replacestring) {
   int len = strlen(replacestring);
   replace_string.setSize(len+1);
   strcpy(replace_string.getBase(), replacestring);
}



//////////////////////////////
//
// PerlRegularExpression::getSubmatchStart -- returns the character
//    byte offset into the search string for the index-th match.
//

int PerlRegularExpression::getSubmatchStart(int index) {
   int output = -1;
   if (index < 100) {
      output = output_substrings[index*2];
   }
   return output;
}



//////////////////////////////
//
// PerlRegularExpression::getSubmatchEnd -- returns the character
//    byte offset of the character after the last character in search 
//    string for the index-th match.
//

int PerlRegularExpression::getSubmatchEnd(int index) {
   int output = -1;
   if (index < 100) {
      output = output_substrings[index*2+1];
   }
   return output;
}



//////////////////////////////
//
//  PerlRegularExpression::getSubmatch -- Returns the specified 
//      submatch from the last search which was done.  Index 0 
//      contains the total match, and indices 1 and higher contain
//      any substring matches which are specified by placing ()
//      markers around part of the regular expression used in searching.
//      The maximum substring index is currently set to 99.
//

const char* PerlRegularExpression::getSubmatch(int index) {
   int start = 0;
   int stop  = 0;
   int len   = 0;
	 
   if (index < 100) {
      start = output_substrings[index*2];
      stop  = output_substrings[index*2+1];
      len   = stop - start;
   }

   if (len <= 0) {
      substring.setSize(1);
      substring[0] = '\0';
      return substring.getBase();
   }

   if (len+start+1 > input_string.getSize()) {
      // match entry is invalid, so return empty string
      substring.setSize(1);
      substring[0] = '\0';
      return substring.getBase();
   }

   substring.setSize(len+1);
   strncpy(substring.getBase(), input_string.getBase()+start, len);
   substring[substring.getSize()-1] = '\0';
   return substring.getBase();
}


// This version of getSubmatch returns the previouly specified
// submatch in getSubmatch(int index) so that the string does not
// have to be re-copied into the temporary storage string.

const char* PerlRegularExpression::getSubmatch(void) {
   return substring.getBase();
}


//////////////////////////////
//
// PerlRegularExpression::setSearchString -- store the search string
//

void PerlRegularExpression::setSearchString(const char* searchstring) {
   if (strcmp(searchstring, search_string.getBase()) != 0) {
      valid = 0;
      int len = strlen(searchstring);
      search_string.setSize(len+1);
      strcpy(search_string.getBase(), searchstring);
   }
}



///////////////////////////////
//
// PerlRegularExpression::setGlobal -- do a global replace.
//

void PerlRegularExpression::setGlobal(void) {
   globalQ = 1;
}



///////////////////////////////
//
// PerlRegularExpression::setSingle -- do a single replace (not global)
//

void PerlRegularExpression::setSingle(void) {
   globalQ = 0;
}



///////////////////////////////
//
// PerlRegularExpression::setExtendedSyntax --
//

void PerlRegularExpression::setExtendedSyntax(void) {
   if (extendedQ != 1) {
      extendedQ = 1;
      valid = 0;
   }
}



///////////////////////////////
//
// PerlRegularExpression::setBasicSyntax --
//

void PerlRegularExpression::setBasicSyntax(void) {
   if (extendedQ != 0) {
      extendedQ = 0;
      valid = 0;
   }
}



//////////////////////////////
//
// PerlRegularExpression::setAnchor -- set the anchor option which
//    will cause any regular expression search to only match the
//    start of the input string (even if the search pattern does not
//    include ^ at the start of it).
//

void PerlRegularExpression::setAnchor(void) {
     anchorQ = 1;
}



//////////////////////////////
//
// PerlRegularExpression::setNoAnchor -- turn off the anchor option
//     which will require the "^" operator in a regular expression in
//     order to force a match from the start of a line.
//

void PerlRegularExpression::setNoAnchor(void) {
    anchorQ = 0;
}



//////////////////////////////
//
// PerlRegularExpression::setIgnoreCase --
//

void PerlRegularExpression::setIgnoreCase(void) {
   if (ignorecaseQ != 1) {
      ignorecaseQ = 1;
      valid = 0;
   }
}



///////////////////////////////
//
// PerlRegularExpression::setNoIgnoreCase --
//

void PerlRegularExpression::setNoIgnoreCase(void) {
   if (ignorecaseQ != 0) {
      ignorecaseQ = 0;
      valid = 0;
   }
}



///////////////////////////////
//
// initializeSearch -- set up a search with the current settings.
//

void PerlRegularExpression::initializeSearch(void) {

   int compflags = 0;
   // PCRE always uses extended regular expressions
   if (ignorecaseQ) {
      compflags |= PCRE_CASELESS;
   }
   if (anchorQ) {
      compflags |= PCRE_ANCHORED;
   }

   if (pre != NULL) {
      pcre_free(pre);
      pre = NULL;
   }

   pre = pcre_compile(search_string.getBase(), compflags, &compile_error,
      &error_offset, NULL);
		     
   if (pre == NULL) {
      cerr << "Error trying to understand regular expression: "
           << search_string.getBase() << endl;
      cerr << "Reason: ";
      if (compile_error != NULL) {
         cout << compile_error << endl;
      } else {
         cout << "None." << endl;
      }
      cerr << "Error offset: " << error_offset << endl;
      cerr << endl;
      exit(1);
   }

   valid = 1;
   studyQ = 0;
}



//////////////////////////////
//
// PerlRegularExpression::initializeSearchAndStudy --
//

void PerlRegularExpression::initializeSearchAndStudy(void) {
   initializeSearch();
   studySearch();
}



//////////////////////////////
//
// PerlRegularExpression::initializeSearchAndStudy --
//

void PerlRegularExpression::initializeSearchAndStudy(const char* searchstring) {
   initializeSearch(searchstring);
   studySearch();
}



///////////////////////////////
//
// PerlRegularExpression::initializeSearch -- store the search string,
//    and compile the regular expression.
//

void PerlRegularExpression::initializeSearch(const char* searchstring) {

   setSearchString(searchstring);
   if (valid) {
      return;
   } else {
      initializeSearch();
   }

}



//////////////////////////////
//
// studySearch --
//

void PerlRegularExpression::studySearch() {
   if (pre == NULL) {
      studyQ = 0;
      return;
   }
	     
   if (studyQ == 0) {
      const char* statusMessage = "";
      studyQ = 1;
      // how should any old structure that pe points to be disposed?
      pe = pcre_study(pre, 0, &statusMessage);
      if (statusMessage != NULL) {
         cerr << "WARNING: problem studying regular expression: " << endl;
         cerr << statusMessage << endl;
         studyQ = 0;
      }
      
   }
}



//////////////////////////////
//
// PerlRegularExpression::sar -- search and replace, destructively.
//   returns the number of replacements made in the search string.
//   default value: optionstring = NULL
//

int PerlRegularExpression::sar(Array<char>& inout, const char* searchstring,
      const char* replacestring, const char* optionstring)  {

   int i;
   if (optionstring != NULL) {
      i = 0; 
      int g_found = 0;
      int i_found = 0;
      while (optionstring[i] != '\0')  {
         switch (optionstring[i]) {
            case 'g': setGlobal();     g_found = 1; break;
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
// PerlRegularExpression::tr -- translate characters
//

void PerlRegularExpression::tr(Array<char>& inout, const char* inputlist, 
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

   // cout  << "INPUTLIST  " << inchars.getBase() << endl;
   // cout  << "OUTPUTLIST " << outchars.getBase() << endl;
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
// PerlRegularExpression::expandList -- expand a translation string into 
//     individual characters.
//

void PerlRegularExpression::expandList(Array<char>& expandlist, 
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
// PerlRegularExpression::searchAndReplace -- returns the number of times the
//    search string was replaced.
//

int PerlRegularExpression::searchAndReplace(Array<char>& output, 
      const char* input, const char* searchstring, const char* replacestring) {
   initializeSearch(searchstring);
   setReplaceString(replacestring);

   return searchAndReplace(output, input);
}



//////////////////////////////
//
// PerlRegularExpression::searchAndReplace -- do a search and replace with
//   the same search and replace from a previous call.
//

int PerlRegularExpression::searchAndReplace(Array<char>& output, 
      const char* input) {

   const char* ptr = input;
   if (valid == 0) {
      initializeSearch();
   }

   SSTREAM tempdata;
   int counter = 0;
   int i;
   int len    = strlen(ptr);
   int status;
   if (studyQ) {
      status = pcre_exec(pre, pe, ptr, len, 0, 0, 
            output_substrings.getBase(), output_substrings.getSize()/3);
   } else {
      status = pcre_exec(pre, NULL, ptr, len, 0, 0, 
            output_substrings.getBase(), output_substrings.getSize()/3);
   }
   
   while (status >= 0) {
      counter++;
      for (i=0; i<output_substrings[0]; i++) {
         tempdata << ptr[i];
      }
      tempdata << replace_string.getBase();
      ptr   += output_substrings[1];
      len   -= output_substrings[1];
      // REG_NOTBOL = start of ptr is not Beginning Of Line
      if (studyQ) {
         status = pcre_exec(pre, pe, ptr, len, 0, PCRE_NOTBOL, 
            output_substrings.getBase(), output_substrings.getSize()/3);
      } else {
         status = pcre_exec(pre, NULL, ptr, len, 0, PCRE_NOTBOL, 
            output_substrings.getBase(), output_substrings.getSize()/3);
      }
      if (!globalQ) {
         break;
      }
      if (ptr[0] == '\0') {
         break;
      }
   }

   if (ptr[0] != '\0') {
      tempdata << ptr;   // store the piece of input after last replace.
   }
   tempdata << ends;
   len = strlen(tempdata.CSTRING);
   output.setSize(len+1);
   strcpy(output.getBase(), tempdata.CSTRING);
   return counter;
}



//////////////////////////////
//
// PerlRegularExpression::search --  returns 0 if match was not found, 
//   otherwise returns the index+1 of the first match found.
//     default value: optionstring = NULL
//

int PerlRegularExpression::search(Array<char>& input, const char* searchstring,
      const char* optionstring) {
   return PerlRegularExpression::search(input.getBase(), searchstring,
         optionstring);
}


int PerlRegularExpression::search(const char* input, const char* searchstring,
      const char* optionstring) {

   setSearchString(searchstring);
   if (valid == 0) {
      initializeSearch();
   }

   int i;
   if (optionstring != NULL) {
      i = 0; 
      int g_found = 0;
      int i_found = 0;
      while (optionstring[i] != '\0')  {
         switch (optionstring[i]) {
            case 'g': setGlobal();     g_found = 1; break;
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

   return search(input);
}



//////////////////////////////
//
// PerlRegularExpression::search --  returns 0 if match was not found, 
//   otherwise returns the index+1 of the first match found.  Uses the
//   previous used/specified search pattern and options.
//

int PerlRegularExpression::search(Array<char>& input) {
   return PerlRegularExpression::search(input.getBase());
}

int PerlRegularExpression::search(const char* input) {
   if (valid == 0) {
      initializeSearch();
   }

   int len = strlen(input);
   input_string.setSize(len+1);
   strcpy(input_string.getBase(), input);

   int status;

   if (studyQ) {
      status = pcre_exec(pre, pe, input_string.getBase(), len, 0, 0, 
         output_substrings.getBase(), output_substrings.getSize());
   } else {
      status = pcre_exec(pre, NULL, input_string.getBase(), len, 0, 0, 
         output_substrings.getBase(), output_substrings.getSize());
   }

   if (status >= 0) {
      // successful match, so return position of beginning of first match
      // plus one.
      return output_substrings[0]+1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// PerlRegularExpression::getTokens -- returns an array of strings
//     which are separated by the separator regular expression
//     in the input string.
// static function.
//
// Example getTokens(tokens, "\\s+", " a b   c   d e ")
// will return an array with 5 elements: a b c d e.
// Leading and trailing matched separators will not generate null
// output tokens, like is done in PERL.
//

int PerlRegularExpression::getTokens(Array<Array<char> >& output, 
      const char* separator, const char* input) {

   output.setSize(0);
   if (output.getGrowth() == 4) {
      // if there are going to be more than 4 elements in the
      // output array, and the person who called this function
      // did not explictly set the growth size for the array,
      // then set it automatically to 1000 (but don't allocate
      // any more space to the array until it is needed).
      output.setGrowth(1000);
   }

   PerlRegularExpression pre;
   int flag;
   pre.setAnchor();
   const char* ptr = input;

   // skip over any initial separator 
   flag = pre.search(ptr, separator);
   if (flag) {
      ptr = ptr + pre.getSubmatchEnd(0);
   }

   Array<char> spat;
   spat.setSize(strlen("(..*?)()") + strlen(separator) + 1);
   strcpy(spat.getBase(), "(..*?)(");
   strcat(spat.getBase(), separator);
   strcat(spat.getBase(), ")");

   int oindex;
   int stepsize;
   int i;
   int strsize;
   char* cptr;
   // remember: anchor is still active
   while (pre.search(ptr, spat.getBase())) {
      oindex = output.getSize();
      output.setSize(oindex+1);
      strsize = pre.getSubmatchEnd(1) - pre.getSubmatchStart(1);
      output[oindex].setSize(strsize + 1);
      cptr = output[oindex].getBase();
      for (i=0; i<strsize; i++) {
         cptr[i] = ptr[i];
      }
      cptr[strsize] = '\0';
      stepsize = pre.getSubmatchEnd(2);
      if (stepsize > 0) {
         ptr += stepsize;
      }
      if (stepsize <= 0) {
         break;
      }
   }

   if (ptr[0] != '\0') {
      oindex = output.getSize();
      output.setSize(oindex+1);
      strsize = strlen(ptr);
      output[oindex].setSize(strsize + 1);
      strcpy(output[oindex].getBase(), ptr);
   }

   return output.getSize();
}



int PerlRegularExpression::getTokensWithEmpties(Array<Array<char> >& output, 
      const char* separator, const char* input) {

   output.setSize(0);
   if (output.getGrowth() == 4) {
      // if there are going to be more than 4 elements in the
      // output array, and the person who called this function
      // did not explictly set the growth size for the array,
      // then set it automatically to 1000 (but don't allocate
      // any more space to the array until it is needed).
      output.setGrowth(1000);
   }

   PerlRegularExpression pre;
   int flag;
   pre.setAnchor();
   const char* ptr = input;

   // skip over any initial separator 
   flag = pre.search(ptr, separator);
   if (flag) {
      ptr = ptr + pre.getSubmatchEnd(0);
   }

   Array<char> spat;
   spat.setSize(strlen("(.*?)()") + strlen(separator) + 1);
   strcpy(spat.getBase(), "(.*?)(");
   strcat(spat.getBase(), separator);
   strcat(spat.getBase(), ")");

   int oindex;
   int stepsize;
   int i;
   int strsize;
   char* cptr;
   // remember: anchor is still active
   while (pre.search(ptr, spat.getBase())) {
      oindex = output.getSize();
      output.setSize(oindex+1);
      strsize = pre.getSubmatchEnd(1) - pre.getSubmatchStart(1);
      output[oindex].setSize(strsize + 1);
      cptr = output[oindex].getBase();
      for (i=0; i<strsize; i++) {
         cptr[i] = ptr[i];
      }
      cptr[strsize] = '\0';
      stepsize = pre.getSubmatchEnd(2);
      if (stepsize > 0) {
         ptr += stepsize;
      }
      if (stepsize <= 0) {
         break;
      }
   }

   if (ptr[0] != '\0') {
      oindex = output.getSize();
      output.setSize(oindex+1);
      strsize = strlen(ptr);
      output[oindex].setSize(strsize + 1);
      strcpy(output[oindex].getBase(), ptr);
   }

   return output.getSize();
}



/**************************************************************************
 *
 * http://regexkit.sourceforge.net/Documentation/pcre/pcresyntax.html
 *

PCRE REGULAR EXPRESSION SYNTAX SUMMARY

The full syntax and semantics of the regular expressions that are
supported by PCRE are described in the pcrepattern documentation. This
document contains just a quick-reference summary of the syntax.

QUOTING

  \x         where x is non-alphanumeric is a literal x
  \Q...\E    treat enclosed characters as literal


CHARACTERS

  \a         alarm, that is, the BEL character (hex 07)
  \cx        "control-x", where x is any character
  \e         escape (hex 1B)
  \f         formfeed (hex 0C)
  \n         newline (hex 0A)
  \r         carriage return (hex 0D)
  \t         tab (hex 09)
  \ddd       character with octal code ddd, or backreference
  \xhh       character with hex code hh
  \x{hhh..}  character with hex code hhh..


CHARACTER TYPES

  .          any character except newline;
               in dotall mode, any character whatsoever
  \C         one byte, even in UTF-8 mode (best avoided)
  \d         a decimal digit
  \D         a character that is not a decimal digit
  \h         a horizontal whitespace character
  \H         a character that is not a horizontal whitespace character
  \p{xx}     a character with the xx property
  \P{xx}     a character without the xx property
  \R         a newline sequence
  \s         a whitespace character
  \S         a character that is not a whitespace character
  \v         a vertical whitespace character
  \V         a character that is not a vertical whitespace character
  \w         a "word" character
  \W         a "non-word" character
  \X         an extended Unicode sequence

In PCRE, \d, \D, \s, \S, \w, and \W recognize only ASCII characters.


GENERAL CATEGORY PROPERTY CODES FOR \p and \P

  C          Other
  Cc         Control
  Cf         Format
  Cn         Unassigned
  Co         Private use
  Cs         Surrogate

  L          Letter
  Ll         Lower case letter
  Lm         Modifier letter
  Lo         Other letter
  Lt         Title case letter
  Lu         Upper case letter
  L&         Ll, Lu, or Lt

  M          Mark
  Mc         Spacing mark
  Me         Enclosing mark
  Mn         Non-spacing mark

  N          Number
  Nd         Decimal number
  Nl         Letter number
  No         Other number

  P          Punctuation
  Pc         Connector punctuation
  Pd         Dash punctuation
  Pe         Close punctuation
  Pf         Final punctuation
  Pi         Initial punctuation
  Po         Other punctuation
  Ps         Open punctuation

  S          Symbol
  Sc         Currency symbol
  Sk         Modifier symbol
  Sm         Mathematical symbol
  So         Other symbol

  Z          Separator
  Zl         Line separator
  Zp         Paragraph separator
  Zs         Space separator


SCRIPT NAMES FOR \p AND \P

   Arabic, Armenian, Balinese, Bengali, Bopomofo, Braille, Buginese, Buhid,
   Canadian_Aboriginal, Cherokee, Common, Coptic, Cuneiform, Cypriot,
   Cyrillic, Deseret, Devanagari, Ethiopic, Georgian, Glagolitic, Gothic,
   Greek, Gujarati, Gurmukhi, Han, Hangul, Hanunoo, Hebrew, Hiragana,
   Inherited, Kannada, Katakana, Kharoshthi, Khmer, Lao, Latin, Limbu,
   Linear_B, Malayalam, Mongolian, Myanmar, New_Tai_Lue, Nko, Ogham,
   Old_Italic, Old_Persian, Oriya, Osmanya, Phags_Pa, Phoenician, Runic,
   Shavian, Sinhala, Syloti_Nagri, Syriac, Tagalog, Tagbanwa, Tai_Le, Tamil,
   Telugu, Thaana, Thai, Tibetan, Tifinagh, Ugaritic, Yi.

CHARACTER CLASSES

  [...]       positive character class
  [^...]      negative character class
  [x-y]       range (can be used for hex characters)
  [[:xxx:]]   positive POSIX named set
  [[:^xxx:]]  negative POSIX named set

  alnum       alphanumeric
  alpha       alphabetic
  ascii       0-127
  blank       space or tab
  cntrl       control character
  digit       decimal digit
  graph       printing, excluding space
  lower       lower case letter
  print       printing, including space
  punct       printing, excluding alphanumeric
  space       whitespace
  upper       upper case letter
  word        same as \w
  xdigit      hexadecimal digit

In PCRE, POSIX character set names recognize only ASCII characters. You
can use \Q...\E inside a character class.


QUANTIFIERS

  ?           0 or 1, greedy
  ?+          0 or 1, possessive
  ??          0 or 1, lazy
  *           0 or more, greedy
  *+          0 or more, possessive
  *?          0 or more, lazy
  +           1 or more, greedy
  ++          1 or more, possessive
  +?          1 or more, lazy
  {n}         exactly n
  {n,m}       at least n, no more than m, greedy
  {n,m}+      at least n, no more than m, possessive
  {n,m}?      at least n, no more than m, lazy
  {n,}        n or more, greedy
  {n,}+       n or more, possessive
  {n,}?       n or more, lazy


ANCHORS AND SIMPLE ASSERTIONS

  \b          word boundary
  \B          not a word boundary
  ^           start of subject
               also after internal newline in multiline mode
  \A          start of subject
  $           end of subject
               also before newline at end of subject
               also before internal newline in multiline mode
  \Z          end of subject
               also before newline at end of subject
  \z          end of subject
  \G          first matching position in subject


MATCH POINT RESET

  \K          reset start of match


ALTERNATION

  expr|expr|expr...


CAPTURING

  (...)          capturing group
  (?<name>...)   named capturing group (Perl)
  (?'name'...)   named capturing group (Perl)
  (?P<name>...)  named capturing group (Python)
  (?:...)        non-capturing group
  (?|...)        non-capturing group; reset group numbers for
                  capturing groups in each alternative


ATOMIC GROUPS

  (?>...)        atomic, non-capturing group


COMMENT

  (?#....)       comment (not nestable)


OPTION SETTING

  (?i)           caseless
  (?J)           allow duplicate names
  (?m)           multiline
  (?s)           single line (dotall)
  (?U)           default ungreedy (lazy)
  (?x)           extended (ignore white space)
  (?-...)        unset option(s)


LOOKAHEAD AND LOOKBEHIND ASSERTIONS

  (?=...)        positive look ahead
  (?!...)        negative look ahead
  (?<=...)       positive look behind
  (?<!...)       negative look behind

Each top-level branch of a look behind must be of a fixed length.


BACKREFERENCES

  \n             reference by number (can be ambiguous)
  \gn            reference by number
  \g{n}          reference by number
  \g{-n}         relative reference by number
  \k<name>       reference by name (Perl)
  \k'name'       reference by name (Perl)
  \g{name}       reference by name (Perl)
  \k{name}       reference by name (.NET)
  (?P=name)      reference by name (Python)


SUBROUTINE REFERENCES (POSSIBLY RECURSIVE)

  (?R)           recurse whole pattern
  (?n)           call subpattern by absolute number
  (?+n)          call subpattern by relative number
  (?-n)          call subpattern by relative number
  (?&name)       call subpattern by name (Perl)
  (?P>name)      call subpattern by name (Python)


CONDITIONAL PATTERNS

  (?(condition)yes-pattern)
  (?(condition)yes-pattern|no-pattern)

  (?(n)...       absolute reference condition
  (?(+n)...      relative reference condition
  (?(-n)...      relative reference condition
  (?(<name>)...  named reference condition (Perl)
  (?('name')...  named reference condition (Perl)
  (?(name)...    named reference condition (PCRE)
  (?(R)...       overall recursion condition
  (?(Rn)...      specific group recursion condition
  (?(R&name)...  specific recursion condition
  (?(DEFINE)...  define subpattern for reference
  (?(assert)...  assertion condition


BACKTRACKING CONTROL

The following act immediately they are reached:

  (*ACCEPT)      force successful match
  (*FAIL)        force backtrack; synonym (*F)

The following act only when a subsequent match failure causes a backtrack to reach them. They all force a match failure, but they differ in what happens afterwards. Those that advance the start-of-match point do so only if the pattern is not anchored.

  (*COMMIT)      overall failure, no advance of starting point
  (*PRUNE)       advance to next starting character
  (*SKIP)        advance start to current matching position
  (*THEN)        local failure, backtrack to next alternation


NEWLINE CONVENTIONS

These are recognized only at the very start of the pattern or after a (*BSR_...) option.

  (*CR)
  (*LF)
  (*CRLF)
  (*ANYCRLF)
  (*ANY)


WHAT \R MATCHES

These are recognized only at the very start of the pattern or after a (*...) option that sets the newline convention.

  (*BSR_ANYCRLF)
  (*BSR_UNICODE)


CALLOUTS

  (?C)      callout
  (?Cn)     callout with data n

******************************************************************************/





