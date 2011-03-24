//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jun 29 14:25:53 PDT 2009
// Last Modified: Mon Jun 29 14:26:01 PDT 2009
// Filename:      ...sig/src/sig/PerlRegularExpression.h
// Web Address:   http://sig.sapp.org/src/sig/PerlRegularExpression.h
// Syntax:        C++; Perl Compatible Regular Expressions (http://www.pcre.org)
//
// See also:      http://www.adp-gmbh.ch/cpp/regexp/pcre/functions.html
//                http://regexkit.sourceforge.net/Documentation/pcre
//                http://www.gammon.com.au/pcre/pcreapi.html
//
// Syntax:        http://regexkit.sourceforge.net/Documentation/pcre/pcresyntax.html
//

#ifndef _PERLREGULAREXPRESSION_H_INCLUDED
#define _PERLREGULAREXPRESSION_H_INCLUDED

#include <iostream>
#include "Array.h"
#include <pcre.h>


class PerlRegularExpression {
   public:
           PerlRegularExpression    (void);
	  ~PerlRegularExpression    ();
	    
      int  search                   (Array<char>& input, 
		                     const char* searchstring,
				     const char* optionstring = NULL);
      int  search                   (const char* input, 
		                     const char* searchstring,
				     const char* optionstring = NULL);
      int  search                   (Array<char>& input);
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
      static int getTokens          (Array<Array<char> >& output, 
                                     const char* separator, 
                                     const char* input);
      static int getTokensWithEmpties(Array<Array<char> >& output, 
                                     const char* separator, const char* input);

      void initializeSearch         (void);
      void initializeSearch         (const char* searchstring);

      void studySearch              (void);
      void initializeSearchAndStudy (void);
      void initializeSearchAndStudy (const char* searchstring);

      const char* getSubmatch       (int index);
      const char* getSubmatch       (void);
      int  getSubmatchStart         (int index);
      int  getSubmatchEnd           (int index);

      // set regex compile options:
      void setExtendedSyntax        (void);
      void setBasicSyntax           (void);
      void setIgnoreCase            (void);
      void setNoIgnoreCase          (void);
      void setSingle                (void);
      void setGlobal                (void);
      void setAnchor                (void);
      void setNoAnchor              (void);

      void setSearchString          (const char* searchstring);
      void setReplaceString         (const char* replacestring);

   protected:
      char  ignorecaseQ;
      char  extendedQ;
      char  globalQ;
      char  anchorQ;                // true if anchored search
      int   valid;
      int   studyQ;

      pcre* pre;                    // Perl-Compatible RegEx compile structure
      pcre_extra* pe;               // Extra data structure for analyzing 
                                    // pcre* information for faster searches.
      const char* compile_error;
      int   error_offset;

      Array<int> output_substrings; // list of substring matches
      Array<char> substring;        // storage for a substring match

      Array<char> input_string;     // storage for input string 
      Array<char> search_string;    // regular expression
      Array<char> replace_string;

   private:
      void expandList               (Array<char>& expandlist, 
                                     const char* input); 

};
	
#endif  /* _PERLREGULAREXPRESSION_H_INCLUDED */



// md5sum: fff68b3c92d5d5d55f2491aa14e0b15c PerlRegularExpression.h [20050403]
