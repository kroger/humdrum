//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Apr 19 20:03:05 PDT 2009
// Last Modified: Tue May  4 23:11:52 PDT 2009 added -t, -c, -B, --sep options
// Last Modified: Tue May  5 09:54:01 PDT 2009 added -p, -P, --and options
// Last Modified: Mon May 11 20:56:43 PDT 2009 fix initial beat for comments
// Last Modified: Wed Apr 28 18:49:29 PDT 2010 added -T and -D options
// Last Modified: Wed Sep 14 10:40:48 PDT 2011 added -F option
// Filename:      ...sig/examples/all/hgrep.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/hgrep.cpp
// Syntax:        C++; museinfo
//
// Description:   Grep with Humdrum intelligence built into it.
//

#include "humdrum.h"

#include <string.h>
#include <math.h>
#include <regex.h>

#ifndef OLDCPP
   #include <iostream>
   #include <fstream>
   #include <sstream>
   using namespace std;
#else
   #include <iostream.h>
   #include <fstream.h>
   #include <sstream.h>
#endif


//////////////////////////////////////////////////////////////////////////

// function declarations:
void      checkOptions        (Options& opts, int argc, char** argv);
void      example             (void);
void      usage               (const char* command);
void      doSearch            (const char* searchstring, HumdrumFile& infile, 
                               const char* filename);
void      printPreInfo        (const char* filename, HumdrumFile& infile, 
                               double measure, int line, int spine = -1);
char*     searchAndReplace    (char* buffer, const char* searchstring,
                               const char* replacestring, 
                               const char* datastring);
void      displayFraction     (double fraction);
void      fillAndSearches     (Array<regex_t>& relist, 
                               Array<Array<char> >& exlist, 
                               const char* string);
int       tokenSearch         (int& column, HumdrumFile& infile, int line, 
		               regex_t& re);
int       singleTokenSearch   (int& column, HumdrumFile& infile, int line, 
                               regex_t& re, const char* exstring);
void      destroyAndSearches  (Array<regex_t>& relist);
double    getBeatOfNextData   (HumdrumFile& infile, int line);

// User interface variables:
Options     options;
const char* searchstring    = "";    // first argument
int         kernQ           = 0;     // used with -k option
int         dataQ           = 0;     // used with -d option
int         tandemQ         = 0;     // used with -t option
int         commentQ        = 0;     // used with -C option
int         primaryQ        = 0;     // used with -p option
int         nonprimaryQ     = 0;     // used with -P option
int         bibliographicQ  = 0;     // used with -B option
int         absbeatQ        = 0;     // used with -a option
int         fracQ           = 0;     // used with -f option
int         spineQ          = 0;     // used with -s option
int         beatQ           = 0;     // used with -b option
int         measureQ        = 0;     // used with -m option
int         quietQ          = 0;     // used with -q option
int         exinterpQ       = 0;     // used with -x option
int         formQ           = 0;     // used with -F option
int         tokenizeQ       = 0;     // used with -T option
int         datastopQ       = 0;     // used with -D option
const char* exinterps       = "";    // used with -x option
char        separator[1024] = {0};   // used with --sep option
Array<regex_t> Andlist;              // used with --and option
Array<Array<char> > Andexinterp;     // used with --and option

// standard grep option emulations:
int         fileQ           = 0;     // used with -H option
int         matchfilesQ     = 0;     // used with -l option
int         nomatchfilesQ   = 0;     // used with -L option
int         ignorecaseQ     = 0;     // used with -i option
int         basicQ          = 0;     // used with -G option
int         lineQ           = 0;     // used with -n option
int         invertQ         = 0;     // used with -v option

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);
   const char* filename = "";

   int i;
   HumdrumFile infile;
   int numinputs = options.getArgCount() - 1;
   for (i=0; i<numinputs || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         filename = "";
         infile.read(cin);
      } else {
         filename = options.getArg(i+2);
         infile.read(filename);
      }
    
      if (absbeatQ || beatQ || measureQ || fracQ) {
         // need to do this for measureQ because of pickup information
         infile.analyzeRhythm("4");
      }

      doSearch(searchstring, infile, filename);
   }

   destroyAndSearches(Andlist);
   
   return 0;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// doSearch --
//

void doSearch(const char* searchstring, HumdrumFile& infile, 
      const char* filename) {
   regex_t re;
   int flags = 0;
   double measure = 1;
   if (infile.getPickupDur() != 0.0) {
      measure = 0;
   }

   if (!basicQ) {
      flags = flags | REG_EXTENDED;
   }
   if (ignorecaseQ) {
      flags = flags | REG_ICASE;
   }
   int status; 
   status = regcomp(&re, searchstring, flags);
   if (status != 0) {
      char errstring[1024] = {0};
      regerror(status, &re, errstring, 1000);
      cerr << errstring << endl;
      exit(1);
   }
   int i;
   int matchcount = 0;
   
   for (i=0; i<infile.getNumLines(); i++) {
      if (datastopQ && infile[i].isData()) {
         break;
      }
      if (formQ && !infile[i].isData()) {
         if (!invertQ) {
            cout << infile[i] << endl;
         } else {
            if (strcmp(infile[i][0], "*-") == 0) {
               // handled at marker xyga
            } else if (strncmp(infile[i][0], "**", 2) == 0) {
               // handled at marker xyga
            } else {
               cout << infile[i] << endl;
            }
        }
      }

      if (infile[i].isEmpty()) { continue; }
      if (infile[i].isMeasure()) {
         sscanf(infile[i][0],"=%lf", &measure);
      }
      if (dataQ && tandemQ) {
         if (!(infile[i].isData() || infile[i].isTandem())) { continue; }
      
      } else {
         if (dataQ && (!infile[i].isData())) { continue; }
         if (tandemQ && (!infile[i].isTandem())) { continue; }
      }
      if (commentQ && (!infile[i].isComment())) { continue; }
      if (bibliographicQ && (!infile[i].isBibliographic())) { continue; }

      if (exinterpQ || kernQ) {
         if (!(infile[i].isData() || infile[i].isInterpretation() ||
               infile[i].isMeasure() || infile[i].isLocalComment())) {
            continue;
         }
	 int column = -1;
         status = tokenSearch(column, infile, i, re);
         // status == 0 means a match was found
         // status != 0 means a match was not found

         if (invertQ) {
            status = !status;
         }
         if (status == 0) {
            matchcount++;
            if (matchfilesQ) {
               cout << filename << endl;
               return;
            }
            if (nomatchfilesQ) {
               continue;
            } 
            printPreInfo(filename, infile, measure, i, column);
            if (!quietQ) {
               // marker xyga
               cout << infile[i];
            }
            cout << endl;
         }

      } else { // search entire line as a single unit
         if (tokenizeQ) {
            status = 0;
            for (int ii=0; ii<infile[i].getFieldCount(); ii++) {
               status = tokenSearch(ii, infile, i, re);
               if (status == 0) {
                  break;
               }
            }
         } else {
            status = regexec(&re, infile[i].getLine(), 0, NULL, 0);
         }
         if (Andlist.getSize() > 0) {
            for (int aa=0; aa<Andlist.getSize(); aa++) {
               int newstatus = regexec(&Andlist[aa], infile[i].getLine(), 
			               0, NULL, 0);
	       if (newstatus != 0) {
                  status = 1;
               }
            }
         }
         if (invertQ) {
            status = !status;
         }
         if (status == 0) {
            matchcount++;
            if (matchfilesQ) {
               cout << filename << endl;
               return;
            }
            if (nomatchfilesQ) {
               continue;
            } 
            printPreInfo(filename, infile, measure, i, -1);
            if (!quietQ) {
               cout << infile[i];
            }
            cout << endl;
         }

      }
   }

   regfree(&re);
   if (nomatchfilesQ && matchcount == 0) {
      cout << filename << endl;
   }
}



//////////////////////////////
//
// singleTokenSearch -- used in tokenSearch, but only doing one search
//   on a line at a time.  Returns 0 if a match was found, otherwise
//   returns 1 (or a non-zero) if no match.
//

int singleTokenSearch(int& column, HumdrumFile& infile, int line, regex_t& re,
      const char* exstring) {

   int exlen = strlen(exstring);
   int status;

   for (int j=0; j<infile[line].getFieldCount(); j++) {
      if (exlen == 0) {
         // don't filter out based on exclusive interpretation types
      } else if (strstr(exstring, infile[line].getExInterp(j)) == NULL) {
         continue;
      }

      if ((strchr(infile[line].getSpineInfo(j), 'b') != NULL) || 
          (strstr(infile[line].getSpineInfo(j), "((") != NULL)) {
         if (primaryQ) {
            continue;
         }
      } else {
         if (nonprimaryQ) {
            continue;
         }
      }

      status = regexec(&re, infile[line][j], 0, NULL, 0);
      if (status == 0) {
//cout << "AND MATCH FOUND IN TOKEN: " << infile[line][j] << endl;
//cout << "USING REGEX " << (int)(&re) << endl;
         return status;
      }
   }

   return 1; // no match was found on the line
}



//////////////////////////////
//
// tokenSearch -- returns 0 if a match was found, otherwise returns 1
//      if no match was found.
//
//

int tokenSearch(int& column, HumdrumFile& infile, int line, regex_t& re) {
   int matchfound = 0;

   if (kernQ) {
      matchfound = singleTokenSearch(column, infile, line, re, "**kern");
   } else {
      matchfound = singleTokenSearch(column, infile, line, re, exinterps);
   } 

   if (matchfound != 0) {
      return matchfound;
   }

   if (Andlist.getSize() == 0) {
      return matchfound;
   }

//cout << "CHECKING AND DATA" << endl;
   for (int i=0; i<Andlist.getSize(); i++) {
      if (kernQ && (strcmp(Andexinterp[i].getBase(), "") == 0)) {
         matchfound = singleTokenSearch(column, infile, line, 
               Andlist[i], "**kern");
      } else {
         matchfound = singleTokenSearch(column, infile, line, 
               Andlist[i], Andexinterp[i].getBase());
      }
      if (matchfound != 0) {
         column = -1;  // don't identify spine for anded searches
         return 1;
      }
   }

   column = -1;  // don't identify spine for anded searches
   return 0; // all matches were satisfied
}



//////////////////////////////
//
// searchAndReplace --
//

char* searchAndReplace(char* buffer, const char* searchstring,
      const char* replacestring, const char* datastring) {
   buffer[0] = '\0';
   regex_t re;
   regmatch_t match;
   int compflags = 0;
   if (!basicQ) {
      compflags = compflags | REG_EXTENDED;
   }
   if (ignorecaseQ) {
      compflags = compflags | REG_ICASE;
   }
   int status = regcomp(&re, searchstring, compflags);
   if (status !=0) {
      regerror(status, &re, buffer, 1024);
      cerr << buffer << endl;
      exit(1);
   }
   status = regexec(&re, datastring, 1, &match, 0);
   while (status == 0) {
      strncat(buffer, datastring, match.rm_so);
      strcat(buffer, replacestring);
      datastring += match.rm_eo;
      status = regexec(&re, datastring, 1, &match, REG_NOTBOL);
   }
   strcat(buffer, datastring);
   regfree(&re);
   return buffer;
}




//////////////////////////////
//
// printPreInfo --
//    default value: spine = -1
//

void printPreInfo(const char* filename, HumdrumFile& infile, double measure, 
      int line, int spine) {
   if (fileQ) {
      cout << filename << separator;
   }
   if (lineQ) {
      cout << "line " << line+1 << separator;
   }
   if (spineQ && (spine >= 0)) {
      cout << "spine " << spine+1 << separator;
   } /* else if (spineQ) {
      cout << "spine " << 1 << separator;
   } */
	     

   if (measureQ) {
      cout << "measure " << measure << separator;
   }
   if (beatQ) {
      if (infile[line].getBeat() == 0.0) {
         cout << "beat " << getBeatOfNextData(infile, line) << separator;
      } else {
         cout << "beat " << infile[line].getBeat() << separator;
      }
   }
   if (absbeatQ) {
      cout << "absbeat " << infile[line].getAbsBeat() << separator;
   }
   if (fracQ) {
      cout << "frac ";
      displayFraction(infile[line].getAbsBeat()/infile.getTotalDuration());
      cout << separator;
   }
}



//////////////////////////////
//
// getBeatOfNextData -- used to fix the beat position of comments
//      at the start of measures which would be labeled as
//      beat 0 instead of beat 1.
//

double getBeatOfNextData(HumdrumFile& infile, int line) {
   int i;
   double output = infile[line].getBeat();
   for (i=line; i<infile.getNumLines(); i++) {
      if ((!infile[i].isData()) && (!infile[i].isMeasure())) {
         continue;
      }
      output = infile[i].getBeat();
      break;
   }

   return output;
}



//////////////////////////////
//
// displayFraction
//

void displayFraction(double fraction) {
   int value; 
   if (fraction == 0.0) {
      cout << "0.000";
   } else if (fraction == 1.0) {
      cout << "1.000";
   } else if (fraction > 0.0 && fraction < 1.0) {
      value = int(fraction * 1000.0 + 0.5);
      cout << "0.";
      if (value < 100) { cout << "0"; }
      if (value < 10) { cout << "0"; }
      cout << value;
   } else {
      cout << fraction;
   }
}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {

   opts.define("a|absbeat=b", "display the absolute beat postion of a match");
   opts.define("b|beat=b",     "display the metrical beat postion of a match");
   opts.define("B|bibliographic=b", "search only bibliographic records");
   opts.define("C|comment=b",       "search only comment records");
   opts.define("d|data=b",          "search only data records");
   opts.define("f|fraction|frac=b", "disp. pos. as fraction of total length");
   opts.define("F|form=b",          "preserve Humdrum file formattting");
   opts.define("k|kern=b",          "search only **kern records");
   opts.define("p|primary=b",       "search only primary spines");
   opts.define("P|nonprimary=b",    "search only non-primary spines");
   opts.define("q|quiet=b", "don't display matching lines (just pre markers)");
   opts.define("t|tandem=b",        "search only data records");
   opts.define("s|spine=b",         "display spine number of match");
   opts.define("m|measure=b",       "display measure number of match");
   opts.define("x|exinterp=s",      "search only listed exinterps");
   opts.define("T|tokenize=b",      "search tokens independently");
   opts.define("D|data-stop=b",     "stop search at first data record");
   opts.define("sep|separator=s::", "data separator string");
   opts.define("and=s:",            "anded search strings");

   // options which mimic regular grep program:
   opts.define("G|basic-regexp=b",  "use basic regular expression syntax"); 
   opts.define("H|with-filename=b",  "display filename at start match line"); 
   opts.define("h|no-filename=b","do not display filename on match line"); 
   opts.define("L|files-without-match=b", "list files without match");
   opts.define("l|files-with-match=b", "list files with match(es)");
   opts.define("i|ignore-case=b", "ignore case in matches");
   opts.define("n|line-number=b", "display line number of match");
   opts.define("v|invert-match=b", "invert the matching criteria");

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, April 2009" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 5 May 2009" << endl;
      cout << "compiled: " << __DATE__ << endl;
      cout << MUSEINFO_VERSION << endl;
      exit(0);
   } else if (opts.getBoolean("help")) {
      usage(opts.getCommand());
      exit(0);
   } else if (opts.getBoolean("example")) {
      example();
      exit(0);
   }
   
   if (opts.getArgCount() < 1) {
      cerr << "Error: you must supply a search string\n";
      exit(1);
   }
   searchstring  = opts.getArg(1);
   basicQ        = opts.getBoolean("basic-regexp");
   fileQ         = opts.getBoolean("with-filename");
   matchfilesQ   = opts.getBoolean("files-with-match");
   nomatchfilesQ = opts.getBoolean("files-without-match");
   ignorecaseQ   = opts.getBoolean("ignore-case");
   lineQ         = opts.getBoolean("line-number");
   invertQ       = opts.getBoolean("invert-match");

   absbeatQ      = opts.getBoolean("absbeat");
   beatQ         = opts.getBoolean("beat");
   quietQ        = opts.getBoolean("quiet");
   kernQ         = opts.getBoolean("kern");
   fracQ         = opts.getBoolean("fraction");
   dataQ         = opts.getBoolean("data");
   tandemQ       = opts.getBoolean("tandem");
   primaryQ      = opts.getBoolean("primary");
   commentQ      = opts.getBoolean("comment");
   nonprimaryQ   = opts.getBoolean("nonprimary");
   bibliographicQ= opts.getBoolean("bibliographic");
   spineQ        = opts.getBoolean("spine");
   formQ         = opts.getBoolean("form");
   measureQ      = opts.getBoolean("measure");
   tokenizeQ     = opts.getBoolean("tokenize");
   datastopQ     = opts.getBoolean("data-stop");
   exinterpQ     = opts.getBoolean("exinterp");
   exinterps     = opts.getString("exinterp");
   char tempbuffer[1024] = {0};
   searchAndReplace(tempbuffer, "[\\]t", "	", opts.getString("separator"));
   searchAndReplace(separator, "[\\]n", "\n", tempbuffer);

   if (matchfilesQ && nomatchfilesQ) {
      nomatchfilesQ = 0;
   }

   Andlist.setSize(0);
   Andexinterp.setSize(0);
   if (opts.getBoolean("and")) {
      fillAndSearches(Andlist, Andexinterp, opts.getString("and"));
   }
}



//////////////////////////////
//
// fillAndSearches -- exinterp strings are sticky.
//

void fillAndSearches(Array<regex_t>& relist, Array<Array<char> >& exlist, 
      const char* string) {
   int len = strlen(string);
   Array<char> mybuffer(len*2+128);
   mybuffer.setAll('\0');
   char *buffer = mybuffer.getBase();
   relist.setSize(100);
   relist.setSize(0);

   char exbuff[1024] = {0};

   int status;
   int compflags = 0;
   if (!basicQ) {
      compflags = compflags | REG_EXTENDED;
   }
   if (ignorecaseQ) {
      compflags = compflags | REG_ICASE;
   }

   searchAndReplace(buffer, "[\\]n", "\n", string);
   char* ptr = strtok(buffer, "\n");
   while (ptr != NULL) {
      
      if (strncmp("**", ptr, 2) == 0) {
         strcpy(exbuff, ptr);
         ptr = strtok(NULL, "\n");
         continue;
      }
      relist.setSize(relist.getSize()+1);
//cout << "SEARCH STRING ADDED: " << ptr << endl;
//cout << "ADDRESS IS " << (int)(&(relist[relist.getSize()-1])) << endl;
      status = regcomp(&(relist[relist.getSize()-1]), ptr, compflags);
      if (status !=0) {
         regerror(status, &(relist[relist.getSize()-1]), buffer, 1024);
         cerr << buffer << endl;
         exit(1);
      }
      exlist.setSize(exlist.getSize()+1);
      exlist[exlist.getSize()-1].setSize(strlen(exbuff)+1);
      strcpy(exlist[exlist.getSize()-1].getBase(), exbuff);
      ptr = strtok(NULL, "\n");
   }
}



//////////////////////////////
//
// destroyAndSearches --
//

void destroyAndSearches(Array<regex_t>& relist) {
   for (int i=0; i<relist.getSize(); i++) {
      regfree(&(relist[i]));
   }
   relist.setSize(0);
}



//////////////////////////////
//
// example --
//

void example(void) {


}



//////////////////////////////
//
// usage --
//

void usage(const char* command) {

}



// md5sum: 27df66509a3c37fff6fbc9de02f90239 hgrep.cpp [20110918]
