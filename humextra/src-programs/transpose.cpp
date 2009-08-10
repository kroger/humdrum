//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Apr 12 23:22:19 PDT 2004
// Last Modified: Mon Apr 12 23:22:22 PDT 2004
// Last Modified: Thu Feb 24 22:43:17 PST 2005 (added -k option)
// Last Modified: Wed Jun 24 15:39:58 PDT 2009 (updated for GCC 4.4)
// Filename:      ...sig/examples/all/transpose.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/transpose.cpp
// Syntax:        C++; museinfo
//
// Description:   Transpose **kern musical data.
//

#include "humdrum.h"

#include <string.h>
#include <stdio.h>

#ifndef OLDCPP
   #include <iostream>
   #include <fstream>
#else
   #include <iostream.h>
   #include <fstream.h>
#endif


// function declarations:
void      checkOptions          (Options& opts, int argc, char** argv);
void      example               (void);
void      usage                 (const char* command);
void      printFile             (HumdrumFile& infile);
int       generateFieldList     (Array<int>& fieldlist,const char* fieldstring);
void      processFile           (HumdrumFile& infile);
void      printNewKernString    (const char* string);
void      printHumdrumKernToken (HumdrumRecord& record, int index);
void      printHumdrumDataRecord(HumdrumRecord& record, 
                                 Array<int>& spineprocess);
int       getBase40ValueFromInterval(const char* string);
void      printNewKeyInterpretation(HumdrumRecord& aRecord, int index);

// User interface variables:
Options     options;
int         transval    = 0;     // used with -b option
const char* fieldstring = "";    // used with -f option
int         fieldQ      = 0;     // used with -f option
int         ssetkeyQ     = 0;    // used with -k option
int         ssetkey      = 0;    // used with -k option
int         currentkey   = 0;


//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);

   HumdrumFile infile;
   if (options.getArgCount() > 0) {
      infile.read(options.getArg(1));
   } else {
      infile.read(cin);
   }

   processFile(infile);

   return 0;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile) {
   int i;
   Array<int> fieldlist;
   Array<int> spineprocess(infile.getMaxTracks());
   if (fieldQ) {
      spineprocess.setAll(0);
      generateFieldList(fieldlist, fieldstring);
      for (i=0; i<fieldlist.getSize(); i++) {
         if (fieldlist[i] < 1) {
            continue;
         }
         if (fieldlist[i]-1 < spineprocess.getSize()) {
            spineprocess[fieldlist[i]-1] = 1;
         }
      }
   } else {
      spineprocess.setAll(1);
   }

   int length;
   int diatonic;
   int j;
   const char* ptr;
   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_data:
            printHumdrumDataRecord(infile[i], spineprocess);
            cout << "\n";
            break;
         case E_humrec_interpretation:
            // adjust *k[] values here.
            for (j=0; j<infile[i].getFieldCount(); j++) {
               length = strlen(infile[i][j]);
               ptr = strrchr(infile[i][j], ':');
               if ((length < 3) || (ptr == NULL) || (ptr - infile[i][j] > 4)) {
                  cout << infile[i][j];
                  if (j<infile[i].getFieldCount()-1) {
                     cout << "\t";
                  }
                  continue;
               }
               diatonic = tolower(infile[i][j][1]) - 'a';
               if (diatonic >= 0 && diatonic <= 6) {
                  printNewKeyInterpretation(infile[i], j);
                  if (j<infile[i].getFieldCount()-1) {
                     cout << "\t";
                  }
                  continue;
               }
               
               cout << infile[i][j];
               if (j<infile[i].getFieldCount()-1) {
                  cout << "\t";
               }
            }
            cout << "\n";
            break;
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
         case E_humrec_data_comment:
         case E_humrec_data_kern_measure:
         default:
            cout << infile[i] << "\n";
      }
   }
}



//////////////////////////////
//
// printNewKeyInterpretation -- 
//

void printNewKeyInterpretation(HumdrumRecord& aRecord, int index) {

   int mode = 0;
   if (islower(aRecord[index][1])) {
      mode = 1;
   }
   int base40 = Convert::kernToBase40(aRecord[index]);
   currentkey = base40;
   base40 = base40 + transval;
   base40 = base40 + 4000;
   base40 = base40 % 40;
   base40 = base40 + (3 + mode) * 40;
   char buffer[128] = {0};

   if (ssetkeyQ) {
      transval = (ssetkey%40 - currentkey%40);
      base40 = ssetkey + (3 + mode) * 40;
      if (transval > 40) {
         transval -= 40;
      } 
      if (transval > 20) {
         transval = 40 - transval;
         transval = -transval;
      }
      if (transval < -40) {
         transval += 40;
      }
      if (transval < -20) {
         transval = -40 - transval;
         transval = -transval;
      }
      // cout << "TRANSVAL = "<< transval << endl;
   }

   cout << "*" << Convert::base40ToKern(buffer, base40) << ":";
}



//////////////////////////////
//
// printHumdrumDataRecord --
//

void printHumdrumDataRecord(HumdrumRecord& record, Array<int>& spineprocess) {
   int i;
   for (i=0; i<record.getFieldCount(); i++) {
      if (!spineprocess[record.getPrimaryTrack(i)-1]) {
         // don't try to transpose spines which were not indicated.
         cout << record[i];
         if (i<record.getFieldCount()-1) {
            cout << "\t";
         }
         continue;
      }
      if (record.getExInterpNum(i) != E_KERN_EXINT) {
         // don't try to transpose non-kern spines
         cout << record[i];
         if (i<record.getFieldCount()-1) {
            cout << "\t";
         }
         continue;
      }

      printHumdrumKernToken(record, i);
      if (i<record.getFieldCount()-1) {
         cout << "\t";
      }
      continue;
   }
   
}



//////////////////////////////
//
// printHumdrumKernToken --
//

void printHumdrumKernToken(HumdrumRecord& record, int index) {
   if (strcmp(record[index], ".") == 0) {
      // null record element (no pitch).
      cout << ".";
      return;
   }

   int k;
   static char buffer[1024] = {0};
   int tokencount = record.getTokenCount(index);

   for (k=0; k<tokencount; k++) {
      record.getToken(buffer, index, k);
      printNewKernString(buffer);
      if (k<tokencount-1) {
         cout << " ";
      }         
   }
}



//////////////////////////////
//
// printNewKernString --
//

void printNewKernString(const char* string) {
   if (strchr(string, 'r') != NULL) {
      cout << string;
      return;
   }

   char buffer[1024] = {0};
   char buffer2[1024] = {0};
   strcpy(buffer, string);
   int base40 = Convert::kernToBase40(string);
   char* ptr1 = strtok(buffer, "ABCDEFGabcdefg#-n");
   char* ptr2 = strtok(NULL, "ABCDEFGabcdefg#-n");
   char* ptr3 = strtok(NULL, "ABCDEFGabcdefg#-n");
   
   if (ptr3 != NULL) {
      cout << "Error in **kern pitch token: " << string << endl;
      exit(1);
   }

   if (ptr1 != NULL) {
      cout << ptr1;
   }
   cout << Convert::base40ToKern(buffer2, base40 + transval);
   if (ptr2 != NULL) {
      cout << ptr2;
   }
}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("b|base40=i:0", "The base-40 transposition value");
   opts.define("o|octave=i:0", "The octave addition to tranpose value");
   opts.define("f|field=s", "The base-40 transposition value");
   opts.define("t|transpose=s", "musical interval transposition value");
   opts.define("k|setkey=s", "Transpose to the given key");

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, 12 Apr 2004" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 12 Apr 2004" << endl;
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

   transval    = opts.getInteger("base40");
   fieldstring = opts.getString("field");
   fieldQ      = opts.getBoolean("field");
   ssetkeyQ    = opts.getBoolean("setkey");
   ssetkey     = Convert::kernToBase40(opts.getString("setkey"));

   ssetkey = ssetkey % 40;


   if (opts.getBoolean("transpose")) {
      transval = getBase40ValueFromInterval(opts.getString("transpose"));
   }

   transval += 40 * opts.getInteger("octave");
}


//////////////////////////////
//
// getBase40ValueFromInterval -- note: only ninth interval range allowed
//

int getBase40ValueFromInterval(const char* string) {
   int sign = 1;
   if (strchr(string, '-') != NULL) {
      sign = -1;
   }

   int length = strlen(string);
   char* buffer = new char[length+1];
   strcpy(buffer, string);
   int i;
   for (i=0; i<length; i++) {
      if (buffer[i] == 'p') { buffer[i] = 'P'; }
      if (buffer[i] == 'a') { buffer[i] = 'A'; }
      if (buffer[i] == 'D') { buffer[i] = 'd'; }
   }

   int output = 0;

   if (strstr(buffer, "dd1") != NULL)      { output = -2; }
   else if (strstr(buffer, "d1") != NULL)  { output = -1; }
   else if (strstr(buffer, "P1") != NULL)  { output =  0; }
   else if (strstr(buffer, "AA1") != NULL) { output =  2; }
   else if (strstr(buffer, "A1") != NULL)  { output =  1; }

   else if (strstr(buffer, "dd2") != NULL) { output =  3; }
   else if (strstr(buffer, "d2") != NULL)  { output =  4; }
   else if (strstr(buffer, "m2") != NULL)  { output =  5; }
   else if (strstr(buffer, "M2") != NULL)  { output =  6; }
   else if (strstr(buffer, "AA2") != NULL) { output =  8; }
   else if (strstr(buffer, "A2") != NULL)  { output =  7; }

   else if (strstr(buffer, "dd3") != NULL) { output =  9; }
   else if (strstr(buffer, "d3") != NULL)  { output = 10; }
   else if (strstr(buffer, "m3") != NULL)  { output = 11; }
   else if (strstr(buffer, "M3") != NULL)  { output = 12; }
   else if (strstr(buffer, "AA3") != NULL) { output = 14; }
   else if (strstr(buffer, "A3") != NULL)  { output = 13; }

   else if (strstr(buffer, "dd4") != NULL) { output = 15; }
   else if (strstr(buffer, "d4") != NULL)  { output = 16; }
   else if (strstr(buffer, "P4") != NULL)  { output = 17; }
   else if (strstr(buffer, "AA4") != NULL) { output = 19; }
   else if (strstr(buffer, "A4") != NULL)  { output = 18; }

   else if (strstr(buffer, "dd5") != NULL) { output = 21; }
   else if (strstr(buffer, "d5") != NULL)  { output = 22; }
   else if (strstr(buffer, "P5") != NULL)  { output = 23; }
   else if (strstr(buffer, "AA5") != NULL) { output = 25; }
   else if (strstr(buffer, "A5") != NULL)  { output = 24; }

   else if (strstr(buffer, "dd6") != NULL) { output = 26; }
   else if (strstr(buffer, "d6") != NULL)  { output = 27; }
   else if (strstr(buffer, "m6") != NULL)  { output = 28; }
   else if (strstr(buffer, "M6") != NULL)  { output = 29; }
   else if (strstr(buffer, "AA6") != NULL) { output = 31; }
   else if (strstr(buffer, "A6") != NULL)  { output = 30; }

   else if (strstr(buffer, "dd7") != NULL) { output = 32; }
   else if (strstr(buffer, "d7") != NULL)  { output = 33; }
   else if (strstr(buffer, "m7") != NULL)  { output = 34; }
   else if (strstr(buffer, "M7") != NULL)  { output = 35; }
   else if (strstr(buffer, "AA7") != NULL) { output = 37; }
   else if (strstr(buffer, "A7") != NULL)  { output = 36; }

   else if (strstr(buffer, "dd8") != NULL) { output = 38; }
   else if (strstr(buffer, "d8") != NULL)  { output = 39; }
   else if (strstr(buffer, "P8") != NULL)  { output = 40; }
   else if (strstr(buffer, "AA8") != NULL) { output = 42; }
   else if (strstr(buffer, "A8") != NULL)  { output = 41; }

   else if (strstr(buffer, "dd9") != NULL) { output = 43; }
   else if (strstr(buffer, "d9") != NULL)  { output = 44; }
   else if (strstr(buffer, "m9") != NULL)  { output = 45; }
   else if (strstr(buffer, "M9") != NULL)  { output = 46; }
   else if (strstr(buffer, "AA9") != NULL) { output = 48; }
   else if (strstr(buffer, "A9") != NULL)  { output = 47; }

   return output * sign;
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




//////////////////////////////
//
// generateFieldList --
//

int generateFieldList(Array<int>& fieldlist, const char* fieldstring) {
cout << "ENTERING GENERATEFIELD LIST" << endl;
   int maxfield = 0;
   int length = strlen(fieldstring);
   char* buffer = new char[length+1];
   strcpy(buffer, fieldstring);
   int starti, stopi;
   int temp;
   int num;
   char* ptr = strtok(buffer, " ,;:");
   while (ptr != NULL) {
      if (strchr(ptr, '-') != NULL) {
         sscanf(ptr, "%d-%d", &starti, &stopi);
         if (starti > stopi) {
            temp = starti;
            starti=stopi;
            stopi = temp;
         } 
         for (num=starti; num<=stopi; num++) {
            fieldlist.append(num);
         }
         if (num > maxfield) {
            maxfield = num;
         }
      } else {
         sscanf(ptr, "%d", &num);
         fieldlist.append(num);
         if (num > maxfield) {
            maxfield = num;
         }
      }
      ptr = strtok(NULL, " ,;:");
   }
cout << "LEAVING GENERATEFIELD LIST" << endl;
   return maxfield;
}



/* BRIEF DOCUMENTATION


transpose options:

   -t interval = transpose music by the specified interval, where 
                 interval is of the form:
                    P1 = perfect unison (no transposition)
                    m2 = up a minor second
                   -m2 = down a minor second
                    M3 = up a major third
                   -A4 = down an augmented fourth
                    d5 = up a diminished fifth

   -b interval = transpose by the base-40 equivalent to the -t option interval
                     0 = perfect unison (no transposition)
                     6 = up a minor second
                    -6 = down a minor second
                    12 = up a major third
                   -18 = down an augmented fourth
                    22 = up a diminished fifth


   -o octave  = transpose (additionally by given octave)
		transpose -t m3 -o 1 = transpose up by an octave and a minor
			               third.


   -f fieldstring = transpose only the given list of data spines.
		    example:
 			transpose -f1-2,4 -t P4 fourpart.krn
 			transpose -f3     -t P4 fourpart.krn
		   Note: does not work yet with the -k option

                    

##########################################################################
##
## EXAMPLES
##



input: file.krn  -- an example with the key being a minor:

**kern
*a:
4A
4B
4c
4d
4e
4f
4g
4a
*-


#####################################################################
#
# Transpose the file up a minor third (so that it is in C Minor):
#

tranpose -t m3 file.krn

**kern
*c:
4c
4d
4e-
4f
4g
4a-
4b-
4cc
*-

#####################################################################
#
# Transpose the file down a minor third (so that it is in F# Minor):
#

tranpose -t -m3 file.krn

**kern
*f#:
4F#
4G#
4A
4B
4c#
4d
4e
4f#
*-


#####################################################################
#
# Transpose the file up a perfect octave:
#

tranpose -t P8 file.krn

**kern
*a:
4A
4B
4cc
4dd
4ee
4ff
4gg
4aa
*-




#####################################################################
#
# Force the file to a tonic on C rather than a:
#

transpose -k c file.krn

**kern
*c:
4c
4d
4e-
4f
4g
4a-
4b-
4cc
*-


# case -k option value is irrelevant:

transpose -k C file.krn

**kern
*c:
4c
4d
4e-
4f
4g
4a-
4b-
4cc
*-


# Transpose from A Minor to G# Minor:

transpose -k G# file.krn

**kern
*g#:
4G#
4A#
4B
4c#
4d#
4e
4f#
4g#
*-


# Changing input files to:

**kern
*C:
4c
4d
4e
4f
*G:
4g
4a
4b
4cc
*-


# Using -k option will convert all keys to same in output:

transpose -k e file.krn

**kern
*E:
4e
4f#
4g#
4a
*E:
4e
4f#
4g#
4a
*-



##############################
##
## octave transpositions
##


# Back to original data file, transposing up a minor tenth:

transpose -o 1 -t m3 file.krn

**kern
*E-:
8ee-
8ff
8gg
8aa-
8bb-
8ccc
8ddd
8eee-
*-

# transpose down two octaves:

transpose -o -2 file.krn

**kern
*a:
4AAA
4BBB
4CC
4DD
4EE
4FF
4GG
4AA
*-


####################
##
## base 40 -b option instead of -t option
##


# Transpose down two octaves:
transpose -b -80 file.krn

**kern
*a:
4AAA
4BBB
4CC
4DD
4EE
4FF
4GG
4AA
*-

*/


// md5sum: 28a5b69adb369d26bcba3835299da2a8 transpose.cpp [20090626]
