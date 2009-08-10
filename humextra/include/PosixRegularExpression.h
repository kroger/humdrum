//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun 25 21:54:28 PDT 2009
// Last Modified: Thu Jun 25 21:54:31 PDT 2009
// Filename:      ...sig/src/sig/PosixRegularExpression.h
// Web Address:   http://sig.sapp.org/src/sig/PosixRegularExpression.h
// Syntax:        C++; GNU regular expressions in C
//

#ifndef _REGULAREXPRESSION_H_INCLUDED
#define _REGULAREXPRESSION_H_INCLUDED

#include <regex.h>
#include <iostream>
#include "Array.h"


class PosixRegularExpression {
   public:
           PosixRegularExpression   (void);
	  ~PosixRegularExpression   ();
	    
      int  search                   (const char* input, 
		                     const char* searchstring);
      int  search                   (const char* input);

      int  searchAndReplace         (Array<char>& output, const char* input,
                                     const char* searchstring, 
				     const char* replacestring);
      int  searchAndReplace         (Array<char>& output, 
                                     const char* input);
      int  sar                      (Array<char>& inout,
                                     const char* searchstring,
                                     const char* replacestring,
                                     const char* optionstring = NULL);

      void tr                       (Array<char>& inout, 
		                     const char* inputlist, 
		                     const char* outputlist);

      void initializeSearch         (const char* searchstring);
      void initializeSearch         (void);

      // set regex compile options:
      void setExtendedSyntax        (void);
      void setBasicSyntax           (void);
      void setIgnoreCase            (void);
      void setNoIgnoreCase          (void);
      void setSingle                (void);
      void setGlobal                (void);

      void setSearchString          (const char* searchstring);
      void setReplaceString         (const char* replacestring);

   protected:
      char ignorecaseQ;
      char extendedQ;
      char globalQ;
      int  valid;
      regex_t    re;
      regmatch_t match;

      Array<char> search_string;
      Array<char> replace_string;

   private:
      void expandList               (Array<char>& expandlist, 
                                     const char* input); 

};
	
#endif  /* _REGULAREXPRESSION_H_INCLUDED */



// md5sum: fff68b3c92d5d5d55f2491aa14e0b15c PosixRegularExpression.h [20050403]
