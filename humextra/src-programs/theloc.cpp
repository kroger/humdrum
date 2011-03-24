//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 23 05:24:14 PST 2009
// Last Modified: Tue Nov 23 09:54:40 PST 2010 added more features
// Last Modified: Wed Jan 12 07:00:38 PST 2011 added ending note marking
// Last Modified: Tue Jan 18 11:10:38 PST 2011 added --mstart option
// Last Modified: Sun Feb 20 18:38:07 PST 2011 added --percent option
// Last Modified: Thu Feb 24 17:10:34 PST 2011 added --file option
// Filename:      ...sig/examples/all/theloc.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/theloc.cpp
// Syntax:        C++; museinfo
//
// Description:   Identify the location of an index note in a files as 
//                output from "themax --location".
//
// Todo: If a mark is added with -m or --mchar, the program should
// complain and exit if the mark already exists.  But probably allow
// the -m to have the !!!RDF be overwritten with the same marker.

#include "humdrum.h"
#include "PerlRegularExpression.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifndef OLDCPP
   #include <iostream>
   #include <fstream>
#else
   #include <iostream.h>
   #include <fstream.h>
#endif


// Function declarations:
void      checkOptions          (Options& opts, int argc, char** argv);
void      example               (void);
void      usage                 (const char* command);
void      processData           (istream& input);
void      extractDataFromInputLine(Array<char>& filename, 
                                  Array<char>& voicename, int& track, 
                                  int& subtrack, Array<int>& starts, 
                                  Array<int>& endings, char* inputline);
void      prepareSearchPaths    (Array<Array<char> >& paths, 
                                 const char* pathlist);
int       fileexists            (Array<char>& jointname, Array<char>& filename,
                                 Array<char>& path);
void      getFileAndPath        (Array<char>& fileandpath, 
                                 Array<char>& filename, 
                                 Array<Array<char> >& paths);
int       findNote              (int nth, HumdrumFile& infile, int& cur, 
                                 int& row, int& col, int track, int subtrack,
                                 int& measure);
void      fillMeterInfo         (HumdrumFile& infile, 
                                 Array<RationalNumber>& meterbot, int track);
void      markNotes             (HumdrumFile& infile, int row, int col, 
                                 int track, int subtrack, int matchlen, 
                                 const char* marker);
void      processDataLine       (HumdrumFile& infile, const char* inputline, 
                                 Array<char>& filename, 
                                 Array<char>& lastfilename, 
                                 Array<char>& voicename, int track, 
                                 int subtrack, Array<int>& starts, 
                                 Array<int>& endings);
void      printDash             (void);
void      displayNoteLocationInfo(HumdrumFile& infile, int num, int row, 
                                int col, int measure,
                                Array<RationalNumber>& meterbot);

// User interface variables:
Options   options;
Array<Array<char> > paths;
int         debugQ       = 0;     // used with --debug option
int         percentQ     = 0;     // used with -P option
int         dirdropQ     = 0;     // used with -D option
int         dispLineQ    = 0;     // used with -l option
int         dispColumnQ  = 0;     // used with -c option
int         dispNoteQ    = 1;     // used with -N option
int         dispAbsBeatQ = 0;     // used with -a option
int         dispMeasureQ = 1;     // used with -M option
int         dispQBeatQ   = 0;     // used with -q option
int         dispBeatQ    = 1;     // used with -B option
int         rationalQ    = 0;     // used with -r option
int         markQ        = 0;     // used with --mark option
int         matchlistQ   = 0;     // used with --matchlist option
int         mark2Q       = 0;     // used with --mark2 option
int         tieQ         = 0;     // used with --tie option
int         graceQ       = 1;     // used with -G option
int         doubleQ      = 0;     // used with --mstart option
int         fileQ        = 0;     // used with --file option
const char* Filename     = "";    // used with --file option
int         matchlen     = 1;     // used with --mark option
const char* marker     = "@";     // used with --marker option


//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);
 
   ifstream input;
   int numinputs = options.getArgumentCount();
   for (int i=0; i<numinputs || i==0; i++) {
      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         processData(cin);
      } else {
         input.open(options.getArg(i+1));
         processData(input);
         input.close();
      }
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// processData --
//

void processData(istream& input) {
   #define LINESIZE 100000
   Array<char> filename;
   Array<char> lastfilename;
   lastfilename.setSize(1);
   lastfilename[0] = '\0';
   Array<int> starts;
   Array<int> endings;
   Array<char> voicename;
   voicename.setSize(1);
   voicename[0] = '\0';
   int track;
   int subtrack;
   HumdrumFile infile;
   char inputline[LINESIZE] = {0};

   do {
      input.getline(inputline, LINESIZE);
      if (input.eof()) {
         break;
      }

      if (strncmp(inputline, "#NOGRACE", strlen("#NOGRACE")) == 0) {
         graceQ = 0;
         continue;
      } else if (strncmp(inputline, "#GRACE", strlen("#GRACE")) == 0) {
         graceQ = 1;
         continue;
      } else if (strncmp(inputline, "#", strlen("#")) == 0) {
         // unknown control message, just ignore and don't try to process.
         continue;
      }

      extractDataFromInputLine(filename, voicename, track, subtrack, starts, 
            endings, inputline);
      if (track > 0) {
         processDataLine(infile, inputline, filename, lastfilename, voicename,
               track, subtrack, starts, endings);
         lastfilename = filename;
      } else {
         // echo input lines which are not understood (comments?)
         cout << inputline << endl; 
      }

   } while (!input.eof());


   while (!input.eof()) {

   }

   // flush any data needing to be printed (such as for markQ):
   filename.setSize(1);
   filename[0] = '\0';
   processDataLine(infile, "", filename, lastfilename, voicename, track, 
         subtrack, starts, endings);
}



//////////////////////////////
//
// processDataLine --
//

void processDataLine(HumdrumFile& infile, const char* inputline, 
      Array<char>& filename, Array<char>& lastfilename, 
      Array<char>& voicename, int track, int subtrack, Array<int>& starts, 
      Array<int>& endings) {

   PerlRegularExpression pre;
   PerlRegularExpression pre2;
   PerlRegularExpression pre3;

   Array<char> fileandpath;
   if (fileQ) {
      filename.setSize(strlen(Filename)+1);
      strcpy(filename.getBase(), Filename);
   }
   if (strcmp(filename.getBase(), "") != 0) {
      getFileAndPath(fileandpath, filename, paths);
      if (debugQ) {
         cout << "FOUND FILE: " << fileandpath << endl;
      }
      if (strlen(fileandpath.getBase()) == 0) {
         // print original line without processing content since file not found
         cout << inputline << endl;
         return;
      }
   }

   Array<char> tempstr;
   tempstr.setSize(1);
   tempstr[0] = '\0';
   if (strcmp(filename.getBase(), lastfilename.getBase()) != 0) {
      if (strcmp(lastfilename.getBase(), "") != 0) {
         if (markQ) {
            cout << infile;
            if (pre.search(marker, "^\\s*([^\\s])\\s+", "") ||
                pre.search(marker, "^\\s*([^\\s])$", "")
                     ) {
               cout << "!!!RDF**kern: " << pre.getSubmatch(1);
               cout << "= matched note";
               if (pre.search(marker, 
                     "color\\s*=\\s*\"?([^\\s\"\\)\\(,;]+)\"?", "")) {

                  tempstr.setSize(strlen(marker) + 1);
                  strcpy(tempstr.getBase(), marker);
                  pre3.sar(tempstr, "^\\s*[^\\s]\\s+", "", "");
                  pre3.sar(tempstr, "color\\s*=\\s*\"?[^\\s\"\\)\\(,;]+\"?", "", "g");
                  pre3.sar(tempstr, "^[\\s:,;-=]+", "", "");
                  pre3.sar(tempstr, "[\\s,:;-=]+$", "", "");

                  if (!pre2.search(pre.getSubmatch(1), "#", "")) {
                     if (pre2.search(pre.getSubmatch(1), "^[0-9a-f]+$", "i")) {
                        if (strlen(pre.getSubmatch(1)) == 6) {
                           cout << ", color=\"#" << pre.getSubmatch(1) << "\"";
                        } else {
                           cout << ", color=\"" << pre.getSubmatch(1) << "\"";
                        }
                     } else {
                        cout << ", color=\"" << pre.getSubmatch(1) << "\"";
                     }
                  } else {
                     cout << ", color=\"" << pre.getSubmatch(1) << "\"";
                  }
               }
               if (strlen(tempstr.getBase()) > 0) {
                  cout << " " << tempstr;
               }
               cout << endl;
            }
            if (!mark2Q) {
               cout << "!!!MATCHLEN:\t" << matchlen << endl;
            }
            if (strcmp(filename.getBase(), "") == 0) {
               // empty filename is a dummy to force last line of 
               // input to be processed correctly if markQ or similar is used.
               return;
            }
         }
      }
      if (strcmp(filename.getBase(), "") != 0) {
         infile.clear();
         infile.read(fileandpath.getBase());
         infile.analyzeRhythm("4"); // only by quarter-note beats for now
      }
   }

   if (strcmp(filename.getBase(), "") == 0) {
      // empty filename is a dummy, but shouldn't ever get here.
      return;
   }

   if (matchlistQ && markQ) {
      cout << "!!MATCHES:\t";
   }

   if (fileQ) {
      fileandpath.setSize(strlen(Filename)+1);
      strcpy(fileandpath.getBase(), Filename);
   }

   if (matchlistQ) {
      cout << fileandpath;
      cout << ":";
      cout << voicename;
      cout << ":";
      cout << track;
      cout << "\t";
   }
   
   int row = 0;
   int col = 0;
   int cur = 0;   // current nth numbered note in the given track

   int erow = 0;
   int ecol = 0;
   int ecur = 0;   // current nth numbered note in the given track

   int measure = 1;
   int emeasure = 1;
   if (infile.getPickupDuration() > 0.0) {
      measure = 0;
      emeasure = 0;
   }

   Array<RationalNumber> meterbot;
   meterbot.setSize(0);
   if (dispBeatQ) {
      fillMeterInfo(infile, meterbot, track);
   }

   int i;
   int state;
   int estate;
   for (i=0; i<starts.getSize(); i++) {
      state = findNote(starts[i], infile, cur, row, col, track, 
            subtrack, measure);
      if (state == 0) {
         continue;
      }
      if ((endings.getSize() >0) && (endings[i] >= 0)) {
         estate = findNote(endings[i], infile, ecur, erow, ecol, 
         track, subtrack, emeasure);
      } else {
         estate = 0;
      }
      if (markQ) {
         if ((endings.getSize() > 0) && (endings[i] >= 0)) {
            markNotes(infile, row, col, track, subtrack, 
                  endings[i]-starts[i]+1, marker);
         } else {
            markNotes(infile, row, col, track, subtrack, matchlen, marker);
         }
      }
      if (matchlistQ) {
         displayNoteLocationInfo(infile, starts[i], row, col, measure, meterbot);
         if ((endings.getSize() > 0) && (endings[i] >= 0) && estate) {
            printDash();
            displayNoteLocationInfo(infile, endings[i], erow, ecol, emeasure, 
                  meterbot);
         }
      }

      if (matchlistQ) {
         if (i < starts.getSize()-1) {
            cout << " ";
         }
      }
   }
   if (matchlistQ) {
      cout << endl;
   }

}



//////////////////////////////
//
// printDash -- have to do a complicated system, since markQ would not
// involve a dash being printed.
//

void printDash(void) {
   if (dispNoteQ || dispLineQ || dispColumnQ || dispAbsBeatQ || dispMeasureQ ||
         dispBeatQ || percentQ || dispQBeatQ) {
      cout << "-";  
   }
}



//////////////////////////////
//
// displayNoteLocationInfo --
//

void displayNoteLocationInfo(HumdrumFile& infile, int num, int row, int col,
      int measure, Array<RationalNumber>& meterbot) {

   RationalNumber four(4, 1);

   if (dispNoteQ) {
      cout << num;
   }
   if (dispLineQ) {
      cout << "L" << row+1;
   }
   if (dispColumnQ) {
      cout << "C" << col+1;
   }
   if (dispAbsBeatQ) {
      if (rationalQ) {
         cout << "A" << infile[row].getAbsBeatR();
      } else {
         cout << "A" << infile[row].getAbsBeat();
      }
   }
   if (percentQ) {
      double percent = infile.getTotalDuration();
      if (percent > 0.0) {
         percent = int(infile[row].getAbsBeat() / percent * 1000.0 
               + 0.5) / 10.0;
      }
      cout << "P" << percent;
   }
   if (dispMeasureQ) {
      cout << "=" << measure;
   }
   if (dispBeatQ) {
      if (rationalQ) {
         RationalNumber tval = (infile[row].getBeatR()-1) * 
                                  (meterbot[row] / four) + 1;
         cout << "B";
         tval.printTwoPart(cout);
      } else {
         cout << "B" << (infile[row].getBeat()-1) * 
                         (meterbot[row].getFloat() / 4.0) + 1;
      }
   }
   if (dispQBeatQ) {
      if (rationalQ) {
         cout << "Q" << infile[row].getBeatR();
      } else {
         cout << "Q" << infile[row].getBeat();
      }
   }
}



//////////////////////////////
//
// markNotes -- mark notes in match sequence (for possible later output as HTML
//     with colors).
//

void markNotes(HumdrumFile& infile, int row, int col, int track, int subtrack,
      int matchlen, const char* marker) {

   PerlRegularExpression pre;
   char markchar[2] = {0};
   if (pre.search(marker, "^\\s*([^\\s])\\s+", "") ||
         pre.search(marker, "^\\s*([^\\s])$", "")) {
      markchar[0] = pre.getSubmatch(1)[0];
   }

   int tiestate = 0;
   Array<char> newdata;
   newdata.setSize(1000);
   newdata.setGrowth(1000);
   newdata.setSize(0);
   int foundcount = 0;
   int i, j;
   int scount = 0;
   for (i=row; i<infile.getNumLines(); i++) {
      if (!infile[i].isData()) {
         continue;
      }
      if (foundcount >= matchlen) {
         break;
      }
      scount = 0;
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (track != infile[i].getPrimaryTrack(j)) {
            // ignore the current spine if not the correct one
            continue;
         }
         scount++;
         if (subtrack != scount) {
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            // don't count null tokens
            break;
         }
         if (strchr(infile[i][j], 'r') != NULL) {
            // don't count rests
            break;
         }
         if ((!graceQ) && ((strchr(infile[i][j], 'q') != NULL) ||
                              (strchr(infile[i][j], 'Q') != NULL))
               ) {
            // ignore grace notes if not supposed to count
            break;
         }

         if (strchr(infile[i][j], '[') != NULL) {
            if (tieQ) {
               foundcount--;  // suppress from count later on
               tiestate = 1;
            } 
         } else if (strchr(infile[i][j], ']') != NULL) {
            if (tieQ) {
               // don't subtract one from foundcount (this is the last note)
               tiestate = 1;
            } else {
               // don't color tied note unless asked to
               break;
            }
         } else if (strchr(infile[i][j], '_') != NULL) {
            if (tieQ) {
               foundcount--;  // suppress from count later on
               if (tiestate == 0) {
                  // a weird case where the tie starts with a medial
                  // tie.  This is can legally occur when there is a
                  // multiple ending which a slur crosses.
                  foundcount++;
               }
               tiestate = 1;
            } else {
               // don't color tied note unless asked to
               continue;
            }
         } else {
            tiestate = 0;
         }

         foundcount++;

         if (strstr(infile[i][j], markchar) != NULL) {
            // already marked (perhaps overlapping match)
            break;
         }
         newdata.setSize(strlen(infile[i][j]) + strlen(markchar) + 1);
         strcpy(newdata.getBase(), infile[i][j]);
         strcat(newdata.getBase(), markchar);
         if (doubleQ && (foundcount == 1)) {
            strcat(newdata.getBase(), markchar);
         }
         infile[i].changeField(j, newdata.getBase());
         break;

         if (foundcount >= matchlen) {
            goto veryend;
            // if (tieQ && (strchr(infile[i][j], '[') != NULL)) {
            //    foundcount--;
            // } else if (tieQ && (strchr(infile[i][j], '_') != NULL)) {
            //    foundcount--;
            // }
         } 
      }
   }
veryend: ;
}



//////////////////////////////
//
// fillMeterInfo --
//

void fillMeterInfo(HumdrumFile& infile, Array<RationalNumber>& meterbot, 
      int track) {

   int top;
   int bot;

   meterbot.setSize(infile.getNumLines());

   RationalNumber current(4, 1);
   RationalNumber compound(3, 2);
   int i, j;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isInterpretation()) {
         meterbot[i] = current;         
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (track != infile[i].getPrimaryTrack(j)) {
            continue;
         }
         if (sscanf(infile[i][j], "*M%d/%d", &top, &bot) == 2) {
            current = bot;
            if ((top != 3) && ((top % 3) == 0)) {
               current *= compound;
            }
         }
         meterbot[i] = current;
         break;
      }
   }
}



//////////////////////////////
//
// findNote -- Search for the row and column of the nth note in the
//     track.  The current position of row and col is on the cur'th
//     note in the track (if all are 0, then haven't started looking).
//     returns 0 if nth note cannot be found in track.
//

int findNote(int nth, HumdrumFile& infile, int& cur, int& row, int& col, 
      int track, int subtrack, int& measure) {
   int direction = 1;

   if (nth > cur) {
      direction = 1;
   } else if (nth < cur) {
      direction = -1;
   } else {
      // Already have the match (for some strange reason) so just return.
      return 1;
   }

   int scount;
   int mval;
   int i, j;
   for (i=row+direction; (i<infile.getNumLines()) && (i>=0); i+=direction) {
      if (infile[i].isMeasure()) {
         if (sscanf(infile[i][0], "=%d", &mval)) {
            measure = mval;
            if (direction < 0) {
               measure--;
            }
         }
      }
      if (!infile[i].isData()) {
         continue;
      }
      scount = 0;
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (track != infile[i].getPrimaryTrack(j)) {
            continue;
         }
         scount++;
         if (subtrack == scount) {
            if (strcmp(infile[i][j], ".") == 0) { 
               // skip null tokens (could make search faster
               // if null token references were utilized).
               break;
            }
            // currently only considering tracks to be **kern data,
            // but should be generalized later (so don't exit from "r"
            // or "]" or "_" for non **kern data.
            if (strchr(infile[i][j], 'r') != NULL) { 
               // skip null tokens (could make search faster
               // if null token references were utilized).
               break;
            }
            if ((!graceQ) && ((strchr(infile[i][j], 'q') != NULL) ||
                              (strchr(infile[i][j], 'Q') != NULL))
                  ) {
               // ignore grace notes if requested
               break;
            }
            // the following statements are not quite right (consider
            // chords with only some notes being tied?)
            // but this will be dependent on tindex's behavior.
            if (strchr(infile[i][j], ']') != NULL) { 
               // skip endings of ties.
               break;
            }
            if (strchr(infile[i][j], '_') != NULL) { 
               // skip continuation ties.
               break;
            }
            // now have a note which is to be counted:
            cur += direction;
            if (cur == nth) {
               row = i;
               col = j;
               return 1;
            } 
            break;
         }
      }
   }

   // if get here, then note not found:
   row = 0;
   col = 0;
   cur = 0;
   return 0;
}



//////////////////////////////
//
// getFileAndPath -- given a particular filename and a list of directory
//    paths to search, return the first file which is found which matches
//    the filename in the list of directory paths.  First search using
//    the complete filename.  Then if the filename with any attached 
//    directory information is not found, then remove the directory
//    information and search again.  
//

void getFileAndPath(Array<char>& fileandpath, Array<char>& filename, 
   Array<Array<char> >& paths) {
   PerlRegularExpression pre;

   int i;
   for (i=0; i<paths.getSize(); i++) {
      if (fileexists(fileandpath, filename, paths[i])) {
         return;
      }
   }

   if (!pre.search(filename, "/")) {
      fileandpath.setSize(1);
      fileandpath[0] = '\0';
   }

   // check to see if removing the directory name already attached 
   // to the filename helps:

   Array<char> tempfilename = filename;
   pre.sar(tempfilename, ".*/", "", "");

   for (i=0; i<paths.getSize(); i++) {
      if (fileexists(fileandpath, tempfilename, paths[i])) {
         return;
      }
   }

   fileandpath.setSize(1);
   fileandpath[0] = '\0';
}



//////////////////////////////
//
// JoinDirToPath -- The temporary buffer used to
//    create the filename is given back to the calling function.
//

void JoinDirToPath(Array<char>& jointname, Array<char>& path, 
      Array<char>& filename) {

   PerlRegularExpression pre;
   int len = strlen(filename.getBase());
   if (pre.search(path, "^\\./*$")) {
      // if searching in the current directory, then don't
      // add the current directory marker.
      jointname.setSize(len+1);
      strcpy(jointname.getBase(), filename.getBase());
   } else {
      // append "/" to directory name and then filename
      len += strlen(path.getBase());
      if (pre.search(path, "/$", "")) {
         // don't need to add "/" to separate dir and file names.
         jointname.setSize(len+1);
         strcpy(jointname.getBase(), path.getBase());
         strcat(jointname.getBase(), filename.getBase());
      } else {
         // need to add "/" to separate dir and file names.
         jointname.setSize(len+2);
         strcpy(jointname.getBase(), path.getBase());
         strcat(jointname.getBase(), "/");
         strcat(jointname.getBase(), filename.getBase());
      }
   }
}



//////////////////////////////
//
// fileexists --
//

int fileexists(Array<char>& jointname, Array<char>& filename, 
      Array<char>& path) {
   JoinDirToPath(jointname, path, filename);
   if (access(jointname.getBase(), F_OK) != -1) {
      return 1;
   } else {
      return 0;
   }

   // or another way:
   //    struct stat buffer;
   //    return stat(filename, &buffer) == 0;
}



//////////////////////////////
//
// extractDataFromInputLine --
//

void extractDataFromInputLine(Array<char>& filename, 
      Array<char>& voicename, int& track, int& subtrack, Array<int>& starts, 
      Array<int>& endings, char* inputline) {
   filename.setSize(1);
   filename[0] = '\0';
   track = 0;
   subtrack = 1;

   starts.setSize(1000);
   starts.setGrowth(1000);
   starts.setSize(0);

   endings.setSize(1000);
   endings.setGrowth(1000);
   endings.setSize(0);

   int negone = -1;

   char* ptr = inputline;
   int len;
   int value;
   PerlRegularExpression pre;
   PerlRegularExpression pre2;

   if (pre.search(ptr, "^([^\\t:]+)[^\\t]*:(\\d+)\\.?(\\d+)?\\t(\\d+)([^\\s]*\\s*)")) {
      if (fileQ) {
         len = strlen(Filename);
         filename.setSize(len + 1);
         strcpy(filename.getBase(), Filename);
      } else {
         len = strlen(pre.getSubmatch(1));
         filename.setSize(len + 1);
         strcpy(filename.getBase(), pre.getSubmatch());
      }
      track = atoi(pre.getSubmatch(2));
      if (strlen(pre.getSubmatch(3)) > 0) {
         subtrack = atoi(pre.getSubmatch(3));
      } else {
         subtrack = 1;
      }
      value = atoi(pre.getSubmatch(4));
      starts.append(value);
      if (pre2.search(pre.getSubmatch(5), "-(\\d+)", "")) {
         value = atoi(pre2.getSubmatch(1));
         endings.append(value);
      } else {
         endings.append(negone);
      }
      ptr = ptr + pre.getSubmatchEnd(5);
      while (pre.search(ptr, "^(\\d+)([^\\s]*\\s*)")) { 
         value = atoi(pre.getSubmatch(1));
         starts.append(value);
         if (pre2.search(pre.getSubmatch(2), "-(\\d+)", "")) {
            value = atoi(pre2.getSubmatch(1));
            endings.append(value);
         } else {
            endings.append(negone);
         }
         ptr = ptr + pre.getSubmatchEnd(2);
      }
      if (pre.search(inputline, "^[^\\t:]*:([^\\t:]*):")) {
         voicename.setSize(strlen(pre.getSubmatch(1))+1);
         strcpy(voicename.getBase(), pre.getSubmatch());
      }
   } else if (pre.search(ptr, "^([^\\t:]+)\\t(\\d+)([^\\s]*\\s*)")) {
      // monophonic label (no spine information available).
      // search only on the first **kern column in the file.
      if (fileQ) {
         len = strlen(Filename);
         filename.setSize(len + 1);
         strcpy(filename.getBase(), Filename);
      } else {
         len = strlen(pre.getSubmatch(1));
         filename.setSize(len + 1);
         strcpy(filename.getBase(), pre.getSubmatch());
      }
      track = 1;
      value = atoi(pre.getSubmatch(2));
      starts.append(value);
      if (pre2.search(pre.getSubmatch(3), "-(\\d+)", "")) {
         value = atoi(pre2.getSubmatch(1));
         endings.append(value);
      } else {
         endings.append(negone);
      }
      ptr = ptr + pre.getSubmatchEnd(3);
      while (pre.search(ptr, "^(\\d+)([^\\s]*\\s*)")) { 
         value = atoi(pre.getSubmatch(1));
         starts.append(value);
         if (pre2.search(pre.getSubmatch(2), "-(\\d+)", "")) {
            value = atoi(pre2.getSubmatch(1));
            endings.append(value);
         } else {
            endings.append(negone);
         }
         ptr = ptr + pre.getSubmatchEnd(2);
      }
      if (pre.search(inputline, "^[^\\t:]*:([^\\t:]*):")) {
         // won't occur in this case
         voicename.setSize(strlen(pre.getSubmatch(1))+1);
         strcpy(voicename.getBase(), pre.getSubmatch());
      }
   }

   if (dirdropQ) {
      // remove directory names from filename if the -D option was used.
      pre.sar(filename, ".*/", "", "");
   }

}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("p|path=s:.", "colon-separated search patch for data files");
   opts.define("D|nodir=b", "remove directory information from input data");
   opts.define("l|L|line=b", "display line in file on which match found");
   opts.define("c|col=b", "display column in file on which match found");
   opts.define("N|nonth=b", "don't display nth note number in file");
   opts.define("P|percent=b", "display percentage into file");
   opts.define("M|nomeasure=b", "don't display measure number in file");
   opts.define("a|abs=b", "display absolute beat number of note in file");
   opts.define("q|qbeat=b", "display quarter note duration for start of bar");
   opts.define("B|nobeat=b", "don't display beat of start of match");
   opts.define("r|rational=b", "display metric info as rational numbers");
   opts.define("m|mark=b", "mark matches in first input file");
   opts.define("mstart|double=b", "double-mark first note in match");
   opts.define("G|no-grace|nograce=b", "do not count grace notes");
   opts.define("fixedmark=i:1", "mark matches in first input file");
   opts.define("matchlist=b", "list matches in output Humdurm file");
   opts.define("file=s:", "filename to use as basis for search information");
   opts.define("mchar|markchar=s:@", "character to mark matches with");
   opts.define("all=b", "display all location formats");
   opts.define("tie|ties=b", "display search markers on tie middle/end notes");

   opts.define("debug=b",  "author of program"); 
   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Nov 2010" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 10 Nov 2010" << endl;
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

   dispLineQ   = opts.getBoolean("line");
   dispAbsBeatQ= opts.getBoolean("abs");
   dispColumnQ = opts.getBoolean("col");
   dispNoteQ   =!opts.getBoolean("nonth");
   dispMeasureQ=!opts.getBoolean("nomeasure");
   dispQBeatQ  = opts.getBoolean("qbeat");
   dispBeatQ   =!opts.getBoolean("nobeat");
   rationalQ   = opts.getBoolean("rational");
   dirdropQ    = opts.getBoolean("nodir");
   debugQ      = opts.getBoolean("debug");
   percentQ    = opts.getBoolean("percent");
   tieQ        = opts.getBoolean("tie");
   doubleQ     = opts.getBoolean("mstart");
   fileQ       = opts.getBoolean("file");
   if (fileQ) {
      Filename = opts.getString("file");
   }
   markQ       = opts.getBoolean("fixedmark");
   matchlen    = opts.getInteger("fixedmark");
   matchlistQ  = opts.getBoolean("matchlist");
   if (opts.getBoolean("mark")) {
      markQ = 1;
      matchlen = 1;
      mark2Q = 1;
   } else {
      mark2Q = 0;
   }

   if (!markQ) {
      matchlistQ = 1;
   }
   marker =  opts.getString("markchar");
   graceQ = !opts.getString("no-grace");

   if (opts.getBoolean("all")) {
      dispLineQ    = 1;  // used with -l option
      dispColumnQ  = 1;  // used with -c option
      dispNoteQ    = 1;  // used with -N option
      dispAbsBeatQ = 1;  // used with -a option
      percentQ     = 1;  // used with -P option
      dispMeasureQ = 1;  // used with -M option
      dispQBeatQ   = 1;  // used with -q option
      dispBeatQ    = 1;  // used with -B option
   }
   
   prepareSearchPaths(paths, opts.getString("path"));
}



//////////////////////////////
//
// prepareSearchPaths -- get a list of search paths for looking for data files.
//     The list of paths is colon separated.
//

void prepareSearchPaths(Array<Array<char> >& paths, const char* pathlist) {
   paths.setSize(100);
   paths.setSize(0);
   paths.setGrowth(1000);
   PerlRegularExpression pre;
   int len;
   const char* ptr = pathlist;
   while (pre.search(ptr, "([^:]+):?")) {
      len = strlen(pre.getSubmatch(1));
      paths.setSize(paths.getSize()+1);
      paths.last().setSize(len+1);
      strcpy(paths[paths.getSize()-1].getBase(), pre.getSubmatch());
      ptr = ptr + pre.getSubmatchEnd(1);
   }

   if (debugQ) {
      cout << "Search Paths:" << endl;
      for (int i=0; i<paths.getSize(); i++) {
         cout << "search path " << i + 1 << ":\t" << paths[i] << endl;
      }
   }


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



// md5sum: d603b124e68fe6022b7b8005542fec04 theloc.cpp [20110304]
