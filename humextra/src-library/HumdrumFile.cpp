//
// Copyright 1998-2010 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May 18 13:43:47 PDT 1998
// Last Modified: Thu Jul  1 16:19:35 PDT 1999
// Last Modified: Thu Apr 13 18:43:34 PDT 2000 (added generalized ex interps)
// Last Modified: Sat May  6 14:52:19 PDT 2000 (added appendCompositeDuration)
// Last Modified: Mon Dec  4 14:23:17 PST 2000 (after many analysis funtions)
// Last Modified: Wed Dec  6 13:22:08 PST 2000 (added analyzeMetricLevel())
// Last Modified: Sat Dec 16 13:37:19 PST 2000 (added analyzeDataIndex())
// Last Modified: Sat Dec 16 14:41:14 PST 2000 (added analyzeCliche())
// Last Modified: Wed Dec 27 20:19:43 PST 2000 (improved combine functions)
// Last Modified: Wed Jan 10 12:21:22 PST 2001 (added analyzeChordProbability())
// Last Modified: Sun Feb 11 16:01:26 PST 2001 (analyzeChordProbabilityDur())
// Last Modified: Fri Apr  6 13:51:44 PDT 2001 (analyzeChordLikelihood())
// Last Modified: Sun May 13 12:42:43 PDT 2001 (getNoteArray())
// Last Modified: Sat Jun  9 15:10:26 PDT 2001 (added getMinTimeBase() functs)
// Last Modified: Mon Nov  5 17:55:54 PST 2001 (added getNextDatum/getLastDatum)
// Last Modified: Mon Nov 19 23:52:13 PST 2001 (made define ROUNDERR)
// Last Modified: Wed Jan  2 12:07:52 PST 2002 (added **koto to analyzeRhythm)
// Last Modified: Sun Mar 24 12:10:00 PST 2002 (small changes for visual c++)
// Last Modified: Mon Apr 29 22:41:32 PDT 2002 (fixed getTiedDuration for 
//                                              spine change (not perfect))
// Last Modified: Wed Jan  1 22:27:59 PST 2003 (extracted Maxwell functions)
// Last Modified: Mon Feb 10 17:56:13 PST 2003 (added getNoteArray2)
// Last Modified: Thu Mar 18 23:11:14 PST 2004 (removd blank lines frm assemble)
// Last Modified: Mon May 17 00:16:55 PDT 2004 (fix multiple part *v in combine)
// Last Modified: Sat Jun  5 01:43:45 PDT 2004 (adjusted metric analysis)
// Last Modified: Wed Jun 16 21:15:06 PDT 2004 (indep. tracks in analyzeKeyKS)
// Last Modified: Thu Jun 17 23:04:17 PDT 2004 (fixed combine termination)
// Last Modified: Sat Jun 26 00:43:48 PDT 2004 (spaceEmptyLines adjusted)
// Last Modified: Sun Jun 27 01:28:56 PDT 2004 (fixed rhythm parsing interrupted
//                                              by spine manipulators)
// Last Modified: Mon Jun  5 06:59:27 PDT 2006 (add fixIrritatingPickupProblem)
// Last Modified: Tue Jan 29 09:05:26 PST 2008 (fixed array bounds bug in 
//                                              fixIrritatingPickupProblem())
// Last Modified: Tue Oct 14 16:56:54 PDT 2008 (added 'Q' groupetto parsing)
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Last Modified: Fri Jun 19 23:24:03 PDT 2009 (fixed malformed meter parsing)
// Last Modified: Sat Sep  5 22:03:28 PDT 2009 (ArrayInt to Array<int>)
// Last Modified: Mon Oct 12 15:49:27 PDT 2009 (fixed "*clef *v *v" type cases)
// Last Modified: Sat May 22 10:52:36 PDT 2010 (added RationalNumber)
// Last Modified: Thu Oct 28 21:22:51 PDT 2010 (some fixing of combine())
// Last Modified: Sat Dec 25 13:07:09 PST 2010 (minrhythm fix with dots)
// Last Modified: Wed Feb  2 17:51:57 PST 2011 (partial fix for breve beat)
// Filename:      ...sig/src/sigInfo/HumdrumFile.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/HumdrumFile.cpp
// Syntax:        C++ 
//
// Description:   Higher-level functions for processing Humdrum files.
//                Inherits HumdrumFileBasic and adds rhythmic and other
//                types of analyses to the HumdrumFile class.
//

#include "HumdrumFile.h"
#include "humdrumfileextras.h"
#include "Convert.h"
#include "PerlRegularExpression.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#ifndef OLDCPP
   #include <fstream>
   #include <iostream>
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #include <fstream.h>
   #include <iostream.h>
   #ifdef VISUAL
      #include <strstrea.h>
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif

// #define ROUNDERR 0.005
// Changed on Tue Mar 23 17:48:42 PST 2004
#define ROUNDERR 0.005



//////////////////////////////
//
// HumdrumFile::HumdrumFile --
//

HumdrumFile::HumdrumFile(void) : HumdrumFileBasic() {
   rhythmcheck = 0;
   minrhythm = 0;
   minrhythmR = 0;
   pickupdur = -1;
   localrhythms.setSize(0);
}


HumdrumFile::HumdrumFile(const HumdrumFile& aHumdrumFile) :
   HumdrumFileBasic(aHumdrumFile) {
   rhythmcheck = 0;
   minrhythm = 0;
   minrhythmR = 0;
   pickupdur = -1;
   localrhythms.setSize(0);
}

HumdrumFile::HumdrumFile(const HumdrumFileBasic& aHumdrumFile) :
   HumdrumFileBasic(aHumdrumFile) {
   rhythmcheck = 0;
   minrhythm = 0;
   minrhythmR = 0;
   pickupdur = -1;
   localrhythms.setSize(0);
}


HumdrumFile::HumdrumFile(const char* filename) : HumdrumFileBasic(filename) {
   rhythmcheck = 0;
   minrhythm = 0;
   minrhythmR = 0;
   pickupdur = -1;
   localrhythms.setSize(0);
}



//////////////////////////////
//
// HumdrumFile::~HumdrumFile
//

HumdrumFile::~HumdrumFile() {
   clear();
}



//////////////////////////////
//
// HumdrumFile::analyzeRhythm --
//     default values: base = "", debug = 0
//

void HumdrumFile::analyzeRhythm(const char* base, int debug) {
   privateRhythmAnalysis(base, debug);
   rhythmcheck = 1;
}



//////////////////////////////
//
// getMinTimeBase --
//

int HumdrumFile::getMinTimeBase(void) {
   // return minrhythm;
   return minrhythmR.getNumerator() * minrhythmR.getDenominator();
}



//////////////////////////////
//
// getMinTimeBaseR -- RationalNumber version of getMinTimeBase().
//

RationalNumber HumdrumFile::getMinTimeBaseR(void) {
   return minrhythmR;
}



//////////////////////////////
//
// getPickupDuration -- returns the duration of any pickup
//     measure as analysed with rhythm function
//

double HumdrumFile::getPickupDuration(void) {
   return pickupdur.getFloat();
}

double HumdrumFile::getPickupDur(void) {
   return pickupdur.getFloat();
}

RationalNumber HumdrumFile::getPickupDurationR(void) {
   return pickupdur;
}

RationalNumber HumdrumFile::getPickupDurR(void) {
   return pickupdur;
}



//////////////////////////////
//
// getStartIndex -- Given an absolute beat, return the first 
//    line starting at that time or just after if there is no
//    items at the specified time.
//
 
int HumdrumFile::getStartIndex(double startbeat) {
   HumdrumFile& score = *this;
   int index = 1;
   while (index < score.getNumLines() - 1) {
      if (score[index+1].getAbsBeat() == score[index].getAbsBeat()) {
         index++;
         continue;
      }
      if (startbeat < score[index].getAbsBeat() + ROUNDERR &&
          startbeat > score[index-1].getAbsBeat() - ROUNDERR) {
         return index;
      }
      if (startbeat > score[index].getAbsBeat() + ROUNDERR &&
          startbeat < score[index+1].getAbsBeat() - ROUNDERR) {
         return index;
      }
      index++;
   }
 
   return score.getNumLines() - 1;
}
 

int HumdrumFile::getStartIndex(RationalNumber startbeat) {
   HumdrumFile& score = *this;
   int index = 1;
   while (index < score.getNumLines() - 1) {
      if (score[index+1].getAbsBeatR() == score[index].getAbsBeatR()) {
         index++;
         continue;
      }
      if ((startbeat < score[index].getAbsBeatR()) &&
          (startbeat > score[index-1].getAbsBeatR() ) ) {
         return index;
      }
      if ((startbeat > score[index].getAbsBeatR()) &&
          (startbeat < score[index+1].getAbsBeatR() ) ) {
         return index;
      }
      index++;
   }
 
   return score.getNumLines() - 1;
}



//////////////////////////////
//
// getStopIndex -- Given an absolute beat, return the last item
//    occuring on that beat.  If there is no items at that time,
//    then return the first item before the specified time.
//

int HumdrumFile::getStopIndex(double stopbeat) {
   HumdrumFile& score = *this;
   int index = 1;
   while (index < score.getNumLines()) {
      if (stopbeat <= score[index].getAbsBeat() + ROUNDERR &&
          stopbeat > score[index-1].getAbsBeat() - ROUNDERR) {
         return index;
      }
      if (stopbeat > score[index].getAbsBeat() + ROUNDERR &&
          stopbeat < score[index+1].getAbsBeat() - ROUNDERR) {
         return index;
      }
      index++;
   }
 
   return score.getNumLines() - 1;
}         



int HumdrumFile::getStopIndex(RationalNumber stopbeat) {
   HumdrumFile& score = *this;
   int index = 1;
   while (index < score.getNumLines()) {
      if ((stopbeat <= score[index].getAbsBeatR() ) &&
          (stopbeat > score[index-1].getAbsBeatR() ) ) {
         return index;
      }
      if ((stopbeat > score[index].getAbsBeatR() ) &&
          (stopbeat < score[index+1].getAbsBeatR() ) ) {
         return index;
      }
      index++;
   }
 
   return score.getNumLines() - 1;
}         



//////////////////////////////
//
// HumdrumFile::appendLine  -- adds a line to a humdrum file
//

void HumdrumFile::appendLine(const char* aLine) {
   HumdrumFileBasic::appendLine(aLine);
   rhythmcheck = 0;
}


void HumdrumFile::appendLine(HumdrumRecord& aRecord) {
   HumdrumFileBasic::appendLine(aRecord);
   rhythmcheck = 0;
}
   

void HumdrumFile::appendLine(HumdrumRecord* aRecord) {
   appendLine(*aRecord);
}

///////////////////////////////////////////////////////////////////////////



//////////////////////////////
//
// HumdrumFile::assemble -- 
//

int HumdrumFile::assemble(HumdrumFile& output, int count, 
      HumdrumFile** pieces) {
   if (count <= 0) {
      return 0;
   }

   int flag = 1;
   HumdrumFile f[2];
   f[0].clear();   
   f[1].clear();   
   f[0] = *(pieces[0]);
   int state = 0;
   int i;
   for (i=1; i<count; i++) {
      f[!state].clear();
      flag &= combine(f[!state], f[state], *(pieces[i]));
      state = !state;
   }

   output = f[state];
   return flag;
}



int HumdrumFile::assemble(HumdrumFile& output, int count, HumdrumFile* pieces) {
   if (count <= 0) {
      return 0;
   } else if (count == 1) {
      output = pieces[0];
      return 1;
   }

   int i;

/*
   for (i=0; i<count; i++) {
      if (pieces[i].rhythmQ() == 0) {
         pieces[i].analyzeRhythm("4");
      }
   }
*/

   int flag = 1;
   HumdrumFile f[2];
   f[0].clear();   
   f[1].clear();   
   f[0] = pieces[0];
   int state = 0;
   for (i=1; i<count; i++) {
      f[!state].clear();
      flag &= combine(f[!state], f[state], pieces[i]);
      state = !state;
   }

   output = f[state];

   return flag;
}



//////////////////////////////
//
// HumdrumFile::getRhythms -- return a list of the rhythms found in the
//      file.


void HumdrumFile::getRhythms(Array<RationalNumber>& rhys) {
   if (rhythmQ() == 0) {
      analyzeRhythm("4");
   }
   int i;
   Array<RationalNumber>& rats = this->localrhythms;

   rhys.setSize(rats.getSize());
   for (i=0; i<rhys.getSize(); i++) {
      rhys[i] = rats[i];
   }
}



//////////////////////////////
//
// HumdrumFile::combine --
//

int HumdrumFile::combine(HumdrumFile& output, HumdrumFile& A, HumdrumFile& B,
      int debug) {
   int flag = 1;

   if (A.rhythmQ() == 0) {
      A.analyzeRhythm("4");
   }

   if (B.rhythmQ() == 0) {
      B.analyzeRhythm("4");
   }

   if (fabs(A.getTotalDuration() - B.getTotalDuration()) > ROUNDERR) {
      cout << "Error: two files are not of same durational length" << endl;
      cout << A.getTotalDuration() << " compared to " << B.getTotalDuration() 
           << endl;

      cout << "The Two files are: " << endl;
      cout << A << endl;
      cout << "====================================================\n" << endl;
      cout << B << endl;
      cout << "====================================================\n" << endl;

      exit(1);
      return 0;
   }

   if (A.getTotalDuration() == 0) {
      cout << "Error: cannot process file with zero duration" << endl;
      exit(1);
      return 0;
   }

   flag = HumdrumFile::processLinesForCombine(output, A, B, debug);

   if (!debug) {
      output.analyzeSpines();
      output.analyzeRhythm("4");
   }
   return flag;
}



//////////////////////////////
//
// HumdrumFile::processLinesForCombine --
//

int HumdrumFile::processLinesForCombine(HumdrumFile& output, HumdrumFile& A, 
      HumdrumFile& B, int debug) {
   int a = 0;
   int b = 0;
   SSTREAM sout;
   int i;
   int foundStart = 0;   // boolean for finding start of data

   while (a < A.getNumLines() || b < B.getNumLines()) {
      if (debug) {
         cout << "aline = " << a << "\tbline = " << b << endl;
      }

      if (a >= A.getNumLines()) {
         if (debug) {
            sout << "!!CASE AAA" << "\n";
         }
         sout << B[b].getLine() << "\n";
         b++;
         continue;
      } 
      if (b >= B.getNumLines()) {
         if (debug) {
            sout << "!!CASE BBB" << "\n";
         }
         sout << A[a].getLine() << "\n";
         a++;
         continue;
      }
      if (A[a].getType() == E_humrec_empty) {
         if (debug) {
            sout << "!!CASE BBBa" << "\n";
         }
         a++; 
         continue;
      }
      if (B[b].getType() == E_humrec_empty) {
         if (debug) {
            sout << "!!CASE BBBb" << "\n";
         }
         b++; 
         continue;
      }

      if (A[a].getType() == E_humrec_bibliography) {
         for (i=0; i<B.getNumLines(); i++) {
            if ((B[i].getType() == E_humrec_bibliography) &&
                (strcmp(A[a][0], B[i][0]) == 0)) {
               if (debug) {
                  sout << "!!CASE BBBc" << "\n";
               }
               a++;
               continue;
            }
         }
         // Bibliographic record was not found in B.
         if (debug) { sout << "!!CASE CCC" << "\n"; }
         sout << A[a].getLine() << "\n";
         a++;
         continue;
      } else if (A[a].getType() == E_humrec_global_comment) {
         sout << A[a].getLine() << "\n";
         if (B[b].isGlobalComment() && 
             (strcmp(A[a].getLine(), B[b].getLine()) == 0) ) {
            // ignore the global record if a duplicate
            b++;
         }
         a++;
         continue;
      } 

      if (B[b].getType() == E_humrec_bibliography) {
         if (debug) {
            sout << "!!CASE EEE" << "\n";
         }
         sout << B[b].getLine() << "\n";
         b++;
         continue;
      } else if (B[b].getType() == E_humrec_global_comment) {
         if (debug) {
            sout << "!!CASE FFF" << "\n";
         }
         sout << B[b].getLine() << "\n";
         b++;
         continue;
      }

      if ((!foundStart) && (A[a].getType() == E_humrec_interpretation) &&
          (B[b].getType() != E_humrec_interpretation)) {
         if (debug) {
            sout << "!!CASE GGG" << "\n";
         }
         sout << B[b] << "\n";
         b++;
         continue;
      }

      if (B[b].getType() == E_humrec_data_interpretation &&
          A[a].getType() == E_humrec_data) {
         int kk;

         if (A[a].getAbsBeat() < B[b].getAbsBeat()) {
            if (debug) {
               sout << "!!CASE HHH" << "\n";
            }
            sout << A[a];
            sout << "\t";
            for (kk=0; kk<B[b].getFieldCount(); kk++) {
               // sout << ".zz";
               sout << ".";
               if (kk < B[b].getFieldCount()-1) {
                  sout << "\t";
               }
            }
            sout << "\n";
            a++;
            continue;
         }

         for (kk=0; kk<A[a].getFieldCount(); kk++) {
            sout << "*\t";
         }
         sout << B[b] << "\n";
         b++;
         continue;
      }

      if (B[b].getType() == E_humrec_data &&
          A[a].getType() == E_humrec_data_interpretation) {
         int kk;

         if (B[b].getAbsBeat() < A[a].getAbsBeat()) {
            if (debug) {
               sout << "!!CASE III" << "\n";
            }
            for (kk=0; kk<A[a].getFieldCount(); kk++) {
               // sout << ".jj";
               sout << ".";
               if (kk < A[a].getFieldCount()-1) {
                  sout << "\t";
               }
            }
            sout << "\t";
            sout << B[b] << "\n";
            b++;
            continue;

         }

         if (debug) {
            sout << "!!CASE JJJ" << "\n";
         }
         sout << A[a] << "\t";
         for (kk=0; kk<B[b].getFieldCount(); kk++) {
            sout << "*";
            if (kk < B[b].getFieldCount()-1) {
               sout << "\t";
            }
         }

         sout << "\n";
         a++;
         continue;
      }

      if (B[b].getType() == E_humrec_data_interpretation &&
          A[a].getType() == E_humrec_data_measure) {
         if (debug) {
            sout << "!!CASE KKK" << "\n";
         }
         int kk;
         for (kk=0; kk<A[a].getFieldCount(); kk++) {
            sout << "*\t";
         }
         sout<< B[b] << "\n";
         b++;
         continue;
      }

      if ((strncmp(A[a][0], "**", 2) == 0) &&
          (strncmp(B[b][0], "**", 2) == 0)) {
         foundStart = 1;
      }

      if (!foundStart) {
         continue;
      }

      RationalNumber adur = A[a].getAbsBeatR();
      RationalNumber bdur = B[b].getAbsBeatR();
      // cout << "A BEAT = " << adur << "\tB BEAT = " << bdur << endl;
 
      // handle local comments
      if (A[a].getType() == E_humrec_data_comment) {
         if (adur - bdur < 0) {
            if (debug) { sout << "!!CASE: GFD1\n"; }
            // A is earlier than B, print A and wait for B
            sout << A[a] << "\t";
            printConstantTokenFields(sout, B[b], "!");
            sout << "\n";
            a++;
            continue;
         } else if (adur - bdur > 0) {
            if (debug) { sout << "!!CASE: GFD2\n"; }
            // A is after B, print B and wait for A
            if (B[b].isInterpretation()) {
               printConstantTokenFields(sout, A[a], "*");
            } else if (B[b].isLocalComment()) {
               printConstantTokenFields(sout, A[a], "!");
            } else {
               printConstantTokenFields(sout, A[a], ".");
            }
            sout << "\t";
            sout << B[b];
            sout << "\n";
            b++;
            continue;
         } else {
            if (debug) { sout << "!!CASE: GFD3\n"; }
            // A and B are both local comments so print both
            if (B[b].getType() == E_humrec_data_comment) {
               sout << A[a] << "\t" << B[b] << "\n";
               a++; b++;
            } else {
               if (debug) { sout << "!!CASE: GFD4\n"; }
               // B is not a local comment so print A wait for B
               sout << A[a] << "\t";
               printConstantTokenFields(sout, B[b], "!");
               sout << "\n";
               a++;
            }
            continue;
         }
      } else if (B[b].getType() == E_humrec_data_comment) {
         if (adur - bdur < 0) {
            if (debug) { sout << "!!CASE: GFE1\n"; }
            // A is earlier than B so print A and wait for B
            if (B[b].isInterpretation()) {
               printConstantTokenFields(sout, A[a], "*");
            } else if (B[b].isLocalComment()) {
               printConstantTokenFields(sout, A[a], "!");
            } else {
               printConstantTokenFields(sout, A[a], ".");
            }
            sout << "\t";
            sout << B[b];
            sout << "\n";
            b++;
            continue;
         } else if (adur - bdur > 0) {
            if (debug) { sout << "!!CASE: GFE2\n"; }
            // A is after B, print B and wait for A
            printConstantTokenFields(sout, A[a], "!");
            sout << "\t";
            sout << B[b];
            sout << "\n";
            a++;
            continue;
         } else {
            if (debug) { sout << "!!CASE: GFE3\n"; }
            // A and B are both local comments so print both
            if (A[a].getType() == E_humrec_data_comment) {
               sout << A[a] << "\t" << B[b] << "\n";
               a++; b++;
            } else {
               if (debug) { sout << "!!CASE: GFE4\n"; }
               // A is not a local comment, print B; wait for A
               printConstantTokenFields(sout, A[a], "!");
               sout << "\t";
               sout << B[b] << "\n";
               b++;
            }
            continue;
         }
      }

      if ((adur - bdur) < 0) {
         // data lines are supposed to occur at the same time

         if (A[a].isData() && B[b].isData()) {
            if (A[a].getAbsBeatR() < B[b].getAbsBeatR()) {
               // print A and null tokens for B 
               if (debug) {
                  sout << "!!CASE KKKa" << "\n";
               }
               sout << A[a].getLine();
               for (i=0; i<B[b].getFieldCount(); i++) {
                  sout << "\t.";
               }
               sout << "\n";
               a++;
               continue;
            }
            if (A[a].getAbsBeatR() > B[b].getAbsBeatR()) {
               // print null tokens for A and then B
               if (debug) {
                  sout << "!!CASE KKKb" << "\n";
               }
               for (i=0; i<A[a].getFieldCount(); i++) {
                  sout << ".\t";
               }
               sout << B[b].getLine();
               sout << "\n";
               b++;
               continue;
            }
         }

         // handle unequal measures
         if (B[b].getType() == E_humrec_data_measure &&
             A[a].getType() == E_humrec_data) {
            // print the data for A, and empty records for B
            if (debug) {
                sout << "!!CASE LLL" << "\n";
            }
            sout << A[a].getLine();
            for (i=0; i<B[b].getFieldCount(); i++) {
               sout << "\t.";
            }
            sout << "\n";
            a++;
            continue;
         } else if (A[a].getType() == E_humrec_data_measure &&
             B[b].getType() == E_humrec_data) {
            if (debug) {
                sout << "!!CASE MMM" << "\n";
            }
            sout << A[a].getLine() << "\t"; 
            for (i=0; i<B[b].getFieldCount(); i++) {
               sout << "--";
               if (i<B[b].getFieldCount()-1) {
                  sout << "\t";
               }
            }
            sout << "\n";
            a++;
            continue;
         } 

         // if the lines of the two files happen at the same
         // time, but only one of the file lines contains a grace note,
         // then print out that grace note line by itself and defer
         // the other data line until later.


         if ((A[a].getType() == E_humrec_data) &&
             (B[b].getType() == E_humrec_data) ) {

/*
            // handle grace notes by putting them first
            int agrace = 0;
            int bgrace = 0;
            int z;

            for (z=0; z<A[a].getFieldCount(); z++) {
               if (strcmp("**kern", A[a].getExInterp(z)) == 0) {
                  if ((strchr(A[a][z], 'q') != NULL) ||
                      (strchr(A[a][z], 'Q') != NULL)) {
                     agrace = 1;
                  }
               }
            }
            for (z=0; z<B[b].getFieldCount(); z++) {
               if (strcmp("**kern", B[b].getExInterp(z)) == 0) {
                  if ((strchr(B[b][z], 'q') != NULL) ||
                      (strchr(B[b][z], 'Q') != NULL)) {
                     bgrace = 1;
                  }
               }
            }
            
            if (agrace == bgrace) {
               // either both grace notes or neither has grace notes
            } else if (agrace == 1) {
               // A has grace note so display that line first
               sout << A[a].getLine() << "\t";
               int kk;
               for (kk=0; kk<B[b].getFieldCount(); kk++) {
                  sout << ".";
                  if (kk<B[b].getFieldCount()-1) {
                     sout << "\t";
                  }
               }
               sout << "\n";
               a++;
               continue;
            } else if (bgrace == 1) {
               // B has grace note so display that line first
               int kk;
               for (kk=0; kk<A[a].getFieldCount(); kk++) {
                  sout << ".\t";
               }
               sout << B[b].getLine();
               sout << "\n";
               b++;
               continue;
            }
             
*/

            // This section no longer works for grace notes since
            // grace note lines can be give a duration
            if ((A[a].getDuration() == 0.0) && (B[b].getDuration() > 0.0)) {
               // A contains grace note but B does not
               if (debug) {
                   sout << "!!CASE MMM" << "\n";
               }
               sout << A[a].getLine() << "\t";
               int kk;
               for (kk=0; kk<B[b].getFieldCount(); kk++) {
                  sout << ".";
                  if (kk<B[b].getFieldCount()-1) {
                     sout << "\t";
                  }
               }
               sout << "\n";
               a++;
               continue;
            } else if ((B[b].getDuration() == 0.0) && (A[a].getDuration() > 0.0)) {
               // B contains grace note but A does not
               int kk;
               if (debug) {
                   sout << "!!CASE NNN Adur=" << A[a].getDuration()
                        << " Bdur=" << B[b].getDuration()
                        << "\n";
               }
               for (kk=0; kk<A[a].getFieldCount(); kk++) {
                  sout << ".\t";
               }
               sout << B[b].getLine();
               sout << "\n";
               b++;
               continue;
            }
         }

         if ((A[a].getType() != E_humrec_data_interpretation) &&
             (B[b].getType() != E_humrec_data_interpretation) &&
              (A[a].getAbsBeatR() == B[b].getAbsBeatR()) 
               ) {
            if (debug) {
               sout << "!!CASE NNN2" << "\n";
            }
            sout << A[a].getLine() << "\t" << B[b].getLine() << "\n";
            a++;
            b++;
            continue;
         }

         // sort the ordering of notes and interpretations
         if ((A[a].getType() == E_humrec_data_interpretation) &&
             (B[b].getType() == E_humrec_data)) {
            if (debug) {
               sout << "!!CASE OOO" << "\n";
            }
            sout << A[a].getLine() << "\t";
            int kk;
            for (kk=0; kk<B[b].getFieldCount(); kk++) {
               sout << "*";
               if (kk < B[b].getFieldCount()-1) {
                  sout << "\t";
               }
            }
            sout << "\n";
            a++;
            continue;
         }
         if ((A[a].getType() == E_humrec_data) &&
             (B[b].getType() == E_humrec_data_interpretation)) {
            if (debug) {
               sout << "!!CASE PPP" << "\n";
            }
            int kk;
            for (kk=0; kk<A[a].getFieldCount(); kk++) {
               sout << "*\t";
            }
            sout << B[b].getLine() << "\n";
            b++;
            continue;
         }

         // sort the ordering of measure lines and interpretations
         if ((A[a].getType() == E_humrec_data_interpretation) &&
             (B[b].getType() == E_humrec_data_measure)) {
            if (debug) {
               sout << "!!CASE QQQ" << "\n";
            }
            sout << A[a].getLine() << "\t";
            int kk;
            for (kk=0; kk<B[b].getFieldCount(); kk++) {
               sout << "*";
               if (kk < B[b].getFieldCount()-1) {
                  sout << "\t";
               }
            }
            sout << "\n";
            a++;
            continue;
         }
         if ((A[a].getType() == E_humrec_data_measure) &&
             (B[b].getType() == E_humrec_data_interpretation)) {
            if (debug) {
               sout << "!!CASE RRR" << "\n";
            }
            int kk;
            for (kk=0; kk<A[a].getFieldCount(); kk++) {
               sout << "*\t";
            }
            sout << B[b].getLine() << "\n";
            b++;
            continue;
         }

         if ((A[a].getType() == E_humrec_interpretation) &&
             (B[b].getType() == E_humrec_interpretation)    ) {
            if ((strcmp(A[a][0], "*-") == 0) && (strcmp(B[b][0], "*-") != 0)) {
               // if at a terminator in A but none in B, then postpone the
               // printing of A until later.
               if (debug) {
                  sout << "!!CASE SSS" << "\n";
               }
               int kk;
               for (kk=0; kk<A[a].getFieldCount(); kk++) {
                  sout << "*";
                  if (kk < A[a].getFieldCount()-1) {
                     sout << "\t";
                  }
               }
               sout << "\t";
               sout << B[b];
               sout << "\n";
               b++;
               continue;
            }

            if ((strcmp(B[b][0], "*-") == 0) && (strcmp(A[a][0], "*-") != 0)) {
               // if at a terminator in B but none in A, then postpone the
               // printing of B until later.
               int kk;
               if (debug) {
                  sout << "!!CASE TTT" << "\n";
               }
               sout << A[a];
               sout << "\t";
               for (kk=0; kk<B[b].getFieldCount(); kk++) {
                  sout << "*";
                  if (kk < B[b].getFieldCount()-1) {
                     sout << "\t";
                  }
               }
               sout << "\n";
               a++;
               continue;
            }

	    // prevent regular tandem interpretations from aligning
	    // with spine manipulator interpretations
            if ((A[a].isTandem() && B[b].isSpineManipulator())) {
               int ii;
               if (debug) {
                  sout << "!!CASE UUU" << "\n";
               }
               sout << A[a];
               for (ii=0; ii<B[b].getFieldCount(); ii++) {
                  sout << "\t*";
               }
               sout << "\n";
               for (ii=0; ii<A[a].getFieldCount(); ii++) {
                  sout << "*\t";
               }
               sout << B[b];
               sout << '\n';
               a++;
               b++;
               continue;
            } else if ((A[a].isSpineManipulator() && B[b].isTandem())) {
               int ii;
               if (debug) {
                  sout << "!!CASE VVV" << "\n";
               }
               for (ii=0; ii<A[a].getFieldCount(); ii++) {
                  sout << "*\t";
               }
               sout << B[b];
               sout << '\n';

               sout << A[a];
               for (ii=0; ii<B[b].getFieldCount(); ii++) {
                  sout << "\t*";
               }
               sout << "\n";
               a++;
               b++;
               continue;
            } 
         }

         // make sure that both lines do not contain *v spine indicators
         // should also check for *^, but that can be done later ...
         int ahasv = 0;
         int bhasv = 0;
         for (i=0; i<A[a].getFieldCount(); i++) {
            if (strcmp(A[a][i], "*v") == 0) {
               ahasv++;
            }
         }
         for (i=0; i<B[b].getFieldCount(); i++) {
            if (strcmp(B[b][i], "*v") == 0) {
               bhasv++;
            }
         }

         if ((ahasv != 0) && (bhasv != 0)) {
            // the two files have *v markers on the same line, so stagger them.
            if (debug) {
               sout << "!!CASE WWW" << "\n";
            }
            sout << A[a].getLine() << "\t";
            for (i=0; i<B[b].getFieldCount(); i++) {
               sout << "*";
               if (i < B[b].getFieldCount()-1) {
                  sout << "\t";
               }
            }
            sout << "\n";
            // now print line data for second file
            for (i=0; i<A[a].getFieldCount()-ahasv+1; i++) {
               sout << "*" << "\t";
            }
            sout << B[b].getLine() << "\n";
   
            a++;
            b++;
            continue;
         }

         if (debug) {
            sout << "!!CASE XXX xxx" << "\n";
         }
         sout << A[a].getLine() << "\t" << B[b].getLine() << "\n";
         a++;
         b++;
         continue;

      } else if (adur < bdur) {
         // sout << "!! B (" << B[b] << "):" << B[b].getAbsBeat() << " after A: (" 
         // << A[a] << "):" << A[a].getAbsBeat() << "\n";
         // line in a starts before line in b

         if (A[a].getType() == E_humrec_data_measure &&
             B[b].getType() == E_humrec_data) {
            if (debug) {
               sout << "!!CASE YYY" << "\n";
            }
            sout << A[a].getLine() << "\t"; 
            for (i=0; i<B[b].getFieldCount(); i++) {
               sout << "--c";
               if (i<B[b].getFieldCount()-1) {
                  sout << "\t";
               }
            }
            sout << "\n";
            a++;
            continue;
         } 

         if (debug) {
            sout << "!!CASE ZZZ" << "\n";
         }
         sout << A[a].getLine();
         for (i=0; i<B[b].getFieldCount(); i++) {
            sout << "\t.";
         }
         sout << "\n";
         a++;
      } else {
         // sout << "!! B (" << B[b] << "):" << B[b].getAbsBeat() << " before A: (" 
         // << A[a] << "):" << A[a].getAbsBeat() << "\n";
         // line in b starts before line in a

         if (B[b].getType() == E_humrec_data_measure &&
             A[a].getType() == E_humrec_data) {
            if (B[b].getAbsBeat() > A[a].getAbsBeat()) {
               int kk;
               if (debug) {
                  sout << "!!CASE AAAA" << "\n";
               }
               sout<< A[a] << "\t";
               for (kk=0; kk<B[b].getFieldCount(); kk++) {
                  sout << ".y";
                  if (kk < B[b].getFieldCount()-1) {
                     sout << "\t";
                  }
               }
               a++;
               continue;
            }

            if (debug) {
               sout << "!!CASE BBBB" << "\n";
            }
            for (i=0; i<A[a].getFieldCount(); i++) {
               sout << "--d\t";
            }
            sout << B[b].getLine() << "\n";
            b++;
            continue;
         } 

         if (A[a].getType() == E_humrec_data_measure &&
             B[b].getType() == E_humrec_data) {
            if (A[a].getAbsBeat() > B[b].getAbsBeat()) {
               if (debug) {
                  sout << "!!CASE BBBB" << "\n";
               }
               int kk;
               for (kk=0; kk<A[a].getFieldCount(); kk++) {
                  sout << "." << "\t";
                  // sout << ".yy" << "\t";
               }
               //sout << B[b] << "iii\n";
               sout << B[b] << "\n";
               b++;
               continue;
            }

            // Handled by CASE FFFF/GGGG now:
            // if (debug) {
            //    sout << "!!CASE CCCC" << "\n";
            // }
            // sout << A[a].getLine() << "\t";
            // for (i=0; i<B[b].getFieldCount(); i++) {
            //    sout << "-b";
            //    if (i<B[b].getFieldCount()-1) {
            //       sout << "\t";
            //    }
            // }
            // sout << "\n";
            // a++;
            // continue;
         } 

         if (A[a].hasExclusiveQ() && B[b].hasExclusiveQ()) {
            // don't know why this case is necessary...
            if (debug) {
               sout << "!!CASE DDDD" << "\n";
            }
            sout << A[a].getLine() << "\t" << B[b].getLine() << "\n";
            a++;
            b++;
            continue;
         }

         if ((A[a].isMeasure() && B[b].isMeasure()) && 
             (A[a].getAbsBeatR() == B[b].getAbsBeatR()) ) {
            // don't know why this case is necessary...
            // If both current lines have measure on them 
            // and the absolute beat positions are equal, 
            // then print both files' lines at same time.
            if (debug) {
               sout << "!!CASE DDDDa" << "\n";
            }
            sout << A[a].getLine() << "\t" << B[b].getLine() << "\n";
            a++;
            b++;
            continue;
         }

         if (A[a].isInterpretation() && B[b].isInterpretation()) {
            // don't know why this case is necessary...
            // will have bugs if there are tandem interpretations,
            // so check out more carefully...
            if (debug) {
               sout << "!!CASE EEEE" << "\n";
            }
            sout << A[a].getLine() << "\t" << B[b].getLine() << "\n";
            a++;
            b++;
            continue;
         }

         if (A[a].isInterpretation() && B[b].isMeasure()) {
            // Don't pass the barline in B until one is found in A.
            if (debug) {
               sout << "!!CASE FFFF" << "\n";
            }
            sout << A[a].getLine();
            for (i=0; i<B[b].getFieldCount(); i++) {
               sout << "\t*";
            }
            sout << "\n";
            a++;
            continue;
         } else if (A[a].isMeasure() && B[b].isInterpretation()) {
            // Don't pass the barline in A until one is found in B.
            if (debug) {
               sout << "!!CASE GGGG" << "\n";
            }
            for (i=0; i<A[a].getFieldCount(); i++) {
               sout << "*\t";
            }
            sout << B[b].getLine();
            sout << "\n";
            b++;
            continue;
         }

         if (A[a].isData() && B[b].isData()) {
            if (A[a].getAbsBeatR() < B[b].getAbsBeatR()) {
               // print A and null tokens for B 
               if (debug) {
                  sout << "!!CASE GGGGa" << "\n";
               }
               sout << A[a].getLine();
               for (i=0; i<B[b].getFieldCount(); i++) {
                  sout << "\t.";
               }
               sout << "\n";
               a++;
               continue;
            }
            if (A[a].getAbsBeatR() > B[b].getAbsBeatR()) {
               // print null tokens for A and then B
               if (debug) {
                  sout << "!!CASE GGGGb" << "\n";
               }
               for (i=0; i<A[a].getFieldCount(); i++) {
                  sout << ".\t";
               }
               sout << B[b].getLine();
               sout << "\n";
               b++;
               continue;
            }
         }

         if (A[a].isData() && B[b].isMeasure()) {
            // Don't pass the barline in B until one is found in A.
            if (debug) {
               sout << "!!CASE FFFF2" << "\n";
            }
            sout << A[a].getLine();
            for (i=0; i<B[b].getFieldCount(); i++) {
               sout << "\t.";
            }
            sout << "\n";
            a++;
            continue;
         } else if (A[a].isMeasure() && B[b].isData()) {
            // Don't pass the barline in A until one is found in B.
            if (debug) {
               sout << "!!CASE GGGG2" << "\n";
            }
            for (i=0; i<A[a].getFieldCount(); i++) {
               sout << ".\t";
            }
            sout << B[b].getLine();
            sout << "\n";
            b++;
            continue;
         }

         if ((A[a].isData() && B[b].isData()) && 
             (A[a].getAbsBeatR() == B[b].getAbsBeatR()) &&
             (A[a].getDuration() != 0.0) &&
             (B[b].getDuration() != 0.0) ) {
            // Don't know why this case is necessary...
            // If two lines contains notes which are not grace notes
            // only, and they occur at the same time, then print together.
            if (debug) {
               sout << "!!CASE HHHH" << "\n";
            }
            sout << A[a].getLine() << "\t" << B[b].getLine() << "\n";
            a++;
            b++;
            continue;
         }

         if (debug) {
            sout << "!!CASE IIII" << "\n";
         }
         for (i=0; i<A[a].getFieldCount(); i++) {
            sout << ".\t";
         }
         sout << B[b].getLine() << "\n";         
         b++;
         continue;
      }
         
   }

   if (debug) {
      cout << "CONTENTS OF FILE FOR DEBUGGING INFO:" << endl;
      sout << ends;
      sout << "\n";
      cout << sout.CSTRING;
   } 
   output.read(sout);

   // cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
   // cout << output << endl;
   // cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

   return 1;
}


///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// HumdrumFile::clear -- removes all lines from the humdrum file
//

void HumdrumFile::clear(void) {
   HumdrumFileBasic::clear();
   rhythmcheck = 0;
   pickupdur = -1;
   localrhythms.setSize(0);
}



//////////////////////////////
//
// HumdrumFile::extract --
//

HumdrumFile HumdrumFile::extract(int aField) {
   return (HumdrumFile)HumdrumFileBasic::extract(aField);
}


//////////////////////////////
//
// HumdrumFile::getDuration -- returns the duration of the current
//      humdrum file line.
//

double HumdrumFile::getDuration(int index) {
   return (*this)[index].getDuration();
}

RationalNumber HumdrumFile::getDurationR(int index) {
   return (*this)[index].getDurationR();
}


//////////////////////////////
//
// HumdrumFile::getLastDatum -- Get previous data in spine.  Tied chord notes
//   other than the first in the tokens are not handled.
//   values for options:  0 = ignore tied notes/ 1 = treat ties notes as
//      separate notes.
//   default value: options = 0
//

const char* HumdrumFile::getLastDatum(int index, int spine, int options) { 
   int nspine = -1;
   int nindex = getLastDatumLine(nspine, index, spine, options);
   if (nspine < 0 || nindex < 0) {
      return "";
   } else {
      return (*this)[nindex][nspine];
   }
}


int HumdrumFile::getLastDatumLine(int& nspine, int index, int spine, 
      int options) { 
   HumdrumFile& file = *this;
   int i = index  - 1;
   int j;
   int lastspine = -1;
   double currenttrack = file[index].getTrack(spine);
   while (i >= 0) {
      if (file[i].getType() != E_humrec_data) {
         i--;
         continue;
      }

      // find matching spine
      lastspine = -1;
      for (j=0; j<file[i].getFieldCount(); j++) {
         if (file[i].getTrack(j) == currenttrack) {
            lastspine = j;
            break;
         }
      }
      if (lastspine == -1) {
         for (j=0; j<file[i].getFieldCount(); j++) {
            if (file[i].getTrack(j) == currenttrack) {
               lastspine = j;
               break;
            }
         }
      }

      if (lastspine == -1) {
         nspine = lastspine;
         return -1;
      }

      if (strcmp(file[i][j], ".") == 0) {
         i--;
         continue;
      } else {
         nspine = j;
         return i;
      }

   }

   // At the beginning of the file, no previous data
   nspine = -1;
   return -1;
}



//////////////////////////////
//
// HumdrumFile::getNextDatum -- Get next data in spine.  Tied chord notes
//   other than the first in the tokens are not handled.
//   values for options:  0 = ignore tied notes/ 1 = treat ties notes as
//      separate notes.
//

const char* HumdrumFile::getNextDatum(int index, int spine, int options) { 
   int nspine = -1;
   int nindex = getNextDatumLine(nspine, index, spine, options);
   if (nspine < 0 || nindex < 0) {
      return "";
   } else {
      return (*this)[nindex][nspine];
   }
}


int HumdrumFile::getNextDatumLine(int& nspine, int index, int spine, 
      int options) { 
   HumdrumFile& file = *this;
   RationalNumber duration(0,1);
   int nextline = -1;
   if (options == 0) {
      // search for next note based on the tied duration of the current note
      duration = getTiedDurationR(index, spine);
   } else {
      // search for next note based including tied notes (ignore ties)
      duration = Convert::kernToDurationR(file[index][spine]);
   }

   if (duration >= getTotalDurationR()) {
      nspine = -1;
      return -1;
   }
   nextline = getStartIndex(getAbsBeatR(index) + duration);
   while (nextline < getNumLines() && getType(nextline) != E_humrec_data ) {
      nextline++;
   }
   if (nextline >= file.getNumLines()) {
      nspine = -1;
      return -1;
   }

   int i;
   double currenttrack = file[index].getTrack(spine);

   for (i=0; i<file[nextline].getFieldCount(); i++) {
      if (file[nextline].getTrack(i) == currenttrack) {
         nspine = i;
         return nextline;
      }
   }

   // didn't find exact track, search for first matching primary track
   for (i=0; i<file[nextline].getFieldCount(); i++) {
      if (file[nextline].getPrimaryTrack(i) == (int)currenttrack) {
         nspine = i;
         return nextline;
      }
   }

   // shouldn't get to this line;
   return -1;

}



//////////////////////////////
//
// HumdrumFile::getBeat -- returns the duration of the current
//      humdrum file line.
//

double HumdrumFile::getBeat(int index) {
   return (*this)[index].getBeat();
}

RationalNumber HumdrumFile::getBeatR(int index) {
   return (*this)[index].getBeatR();
}



//////////////////////////////
//
// HumdrumFile::getAbsBeat -- returns the duration of the current
//      humdrum file line.
//

double HumdrumFile::getAbsBeat(int index) {
   return (*this)[index].getAbsBeat();
}

RationalNumber HumdrumFile::getAbsBeatR(int index) {
   return (*this)[index].getAbsBeatR();
}



//////////////////////////////
//
// HumdrumFile::getNoteList -- Fills the given note list formatted
//   according to the flag values.  Returns the number of notes placed
//   into the list.
//
// Flag Bit    Value              Meaning
// =========================================

//     0       NL_RESTS = 1       include rests or not.
//             NL_NORESTS = 0
//     1       NL_NOEXPAND = 0    expand the meaning of null tokens or not
//             NL_EXPAND = 1 
//     2       NL_MIDI = 0        pitch list output in MIDI note numbers
//             NL_NOMIDI = 1      pitch list output in Base-40 note numbers
//     3       NL_PC = 1          output pitch class values
//             NL_NOPC = 0        output pitches with octave information
//     4       NL_NOSORT = 0      don't sort pitches from lowest to highest
//             NL_SORT = 1        sort pitches from lowest to highest
//     5       NL_NOUNIQ = 0      don't remove redundant pitches
//             NL_UNIQ = 1        remove redundant pitches
//     6       NL_TIED = 0        don't remove tied pitches
//             NL_NOTIED = 1      removed tied pitches
//
//

int HumdrumFile::getNoteList(Array<int>& notes, int line, int flag) {
   // unpack flags:
   int restQ   = flag & (1 << 0);
   int expandQ = flag & (1 << 1);
   int midiQ   = flag & (1 << 2);
   int pcQ     = flag & (1 << 3);
   int sortQ   = flag & (1 << 4);
   int uniqQ   = flag & (1 << 5);
   int tieQ    = !(flag & (1 << 6));

   int i, j;
   int note;
   char tokenbuffer[128] = {0};
   Array<int> rawnotes;
   rawnotes.setSize(32);
   rawnotes.setSize(0);
   rawnotes.allowGrowth();

   HumdrumFile& score = *this;

   notes.setSize(0);
   notes.allowGrowth();

   if (score[line].getType() != E_humrec_data) {
      return 0;
   }

   // store notes found on current line:
   int tokencount = 0;
   for (i=0; i<score[line].getFieldCount(); i++) {
      if (score[line].getExInterpNum(i) != E_KERN_EXINT) {
         continue;
      }

      if (strcmp(score[line][i], ".") == 0 && expandQ) {
         if (score[line].getDotLine(i) < 0) {
            continue;
         }
         HumdrumRecord& dotexpand = score[score[line].getDotLine(i)];
         int spine = score[line].getDotSpine(i);
         if (spine < 0) {
            continue;
         }
         tokencount = dotexpand.getTokenCount(spine);
         for (j=0; j<tokencount; j++) {
            dotexpand.getToken(tokenbuffer, spine, j);
            if ((!tieQ) && (strchr(tokenbuffer, '_') != NULL)) {
               continue;
            }
            if ((!tieQ) && (strchr(tokenbuffer, ']') != NULL)) {
               continue;
            }
            if (midiQ) {
               note = Convert::kernToMidiNoteNumber(tokenbuffer);
               if (pcQ && note >= 0) {
                  note = note % 12;
               }
            } else {
               note = Convert::kernToBase40(tokenbuffer);
               if (pcQ && note != E_base40_rest) {
                  note = note % 40;
               }
            }
            if (note == E_base40_rest) {
               if (restQ) {
                  rawnotes.append(note);
               } 
            } else {
               rawnotes.append(note);
            }
         }
      } else if (strcmp(score[line][i], ".") != 0) {
         tokencount = score[line].getTokenCount(i);
         for (j=0; j<tokencount; j++) {
            score[line].getToken(tokenbuffer, i, j);
            if ((!tieQ) && (strchr(tokenbuffer, '_') != NULL)) {
               continue;
            }
            if ((!tieQ) && (strchr(tokenbuffer, ']') != NULL)) {
               continue;
            }
            if (midiQ) {
               note = Convert::kernToMidiNoteNumber(tokenbuffer);
               if (pcQ && note >= 0) {
                  note = note % 12;
               }
            } else {
               note = Convert::kernToBase40(tokenbuffer);
               if (pcQ && note != E_base40_rest) {
                  note = note % 40;
               }
            }
            if (note == E_base40_rest) {
               if (restQ) {
                  rawnotes.append(note);
               } 
            } else {
               rawnotes.append(note);
            }
         }
      }
   }

   if (rawnotes.getSize() == 0) {
      return 0;
   }
      

   if (rawnotes.getSize() == 1) {
      notes.setSize(1);
      notes[0] = rawnotes[0];
      return 1;
   }
      
   // sort notes if needed:
   if (sortQ) {
      qsort(rawnotes.getBase(), rawnotes.getSize(), sizeof(int), 
            intcompare);
   }

   // uniq notes if needed:
   if (uniqQ && sortQ) {
      int oldnote = rawnotes[0];
      notes.setSize(0);
      notes.append(oldnote);
      for (i=1; i<rawnotes.getSize(); i++) {
         if (rawnotes[i] == oldnote) {
            continue;
         }
         oldnote = rawnotes[i];
         notes.append(oldnote);                  
      }
   } else if (uniqQ && ! sortQ) {
      int foundQ;
      for (i=0; i<rawnotes.getSize(); i++) {
         foundQ = 0;
         for (j=0; j<i; j++) {
            if (rawnotes[i] == rawnotes[j]) {
               foundQ = 1;
               break;
            }
         }
         if (!foundQ) {
            notes.append(rawnotes[i]);
         }
      }
   } else {
      notes = rawnotes;
   }

   return notes.getSize();
}



//////////////////////////////
//
// HumdrumFile::getNoteArray -- 
//     default values:  startLine = 0; endline = 0;
//

void HumdrumFile::getNoteArray(Array<double>& absbeat, 
      Array<int>& pitches, Array<double>& durations, Array<double>& levels,
      int startLine, int endLine, int tracknum) {

   HumdrumFile& score = *this;

   if (endLine <= 0) {
      endLine = score.getNumLines() - 1;
   }
   if (startLine > endLine) {
      int temp = endLine;
      endLine = startLine;
      startLine = temp;
   }
   if (endLine > score.getNumLines() - 1) {
      endLine = score.getNumLines() - 1;
   }
   if (startLine > score.getNumLines() - 1) {
      startLine = score.getNumLines() - 1;
   }

   // estimate the largest amount necessary:
   absbeat.setSize(score.getNumLines() * score.getMaxTracks() * 10);
   pitches.setSize(score.getNumLines() * score.getMaxTracks() * 10);
   durations.setSize(score.getNumLines() * score.getMaxTracks() * 10);
   levels.setSize(score.getNumLines() * score.getMaxTracks() * 10);

   absbeat.setGrowth(score.getNumLines());
   pitches.setGrowth(score.getNumLines());
   durations.setGrowth(score.getNumLines());
   levels.setGrowth(score.getNumLines());

   absbeat.setSize(0);
   pitches.setSize(0);
   durations.setSize(0);
   levels.setSize(0);

   absbeat.allowGrowth(1);
   pitches.allowGrowth(1);
   durations.allowGrowth(1);
   levels.allowGrowth(1);

   Array<int> scorelevels;
   score.analyzeMetricLevel(scorelevels);
   
   int firsttime = 1;
   int i, j, k;
   int ii, jj;
   int ccount;
   static char buffer[1024] = {0};
   int pitch;
   double beatvalue;
   double duration;
   double level;
   for (i=startLine; i<=endLine; i++) {
      if (score[i].getType() != E_humrec_data) {
         // ignore non-note data lines
         continue;
      }
      beatvalue = score.getAbsBeat(i);
      ii = i;
      for (j=0; j<score[i].getFieldCount(); j++) {
         if (score[i].getExInterpNum(j) != E_KERN_EXINT) {
            // ignore non-kern data spines
            continue;
         }
         if ((tracknum != -1) && (score[i].getPrimaryTrack(j) != tracknum)) {
            // ignore all tracks except the one in question
            // tracknum of -1 means get all tracks
            // note that tracks are indexed starting at 1 rather than 0
            continue;
         }
         if (firsttime && strcmp(score[i][j], ".") == 0) {
            // extract the held over note from a previous point in the score
            ii = score[i].getDotLine(j);
            jj = score[i].getDotSpine(j);
            if (ii < 0 || jj < 0) {
               continue;
            }
         } else {
            ii = i;
            jj = j;
         }
 
         if (strcmp(score[ii][jj], ".") != 0) {
            // extract all notes in the region of interest, ignoring
            // tied notes.
            ccount = score[ii].getTokenCount(jj);
            for (k=0; k<ccount; k++) {
               score[ii].getToken(buffer, jj, k, 128);
               if (strchr(buffer, 'r') != NULL) {
                  // skip over rests
                  continue;
               }
               if (strchr(buffer, '_') != NULL) {
                  // skip over doubly tied notes
                  continue;                

               }
               if (!firsttime && strchr(buffer, ']') != NULL) {
                  // skip over tied notes at the ends of ties.
                  continue;
               }
               // have a note so now extract the metric level and the duration
               pitch = Convert::kernToBase40(buffer);
               if (pitch < 0) {
                  // ignore rests
                  continue;
               }
               // pitch = ((int)pitch - 2 + 40) % 40;
               duration = score.getTiedDuration(ii, jj, k);
               if (duration == 0.0) {
                  // ignore grace notes and other zero-dur ornaments
                  continue;
               }
               level = 1.0/pow(2.0, scorelevels[ii]);
               durations.append(duration);
               levels.append(level);
               pitches.append(pitch);
               absbeat.append(beatvalue);
 
            } // end of a chord
         }
      }  // end of a line
      firsttime = 0;
 
   } // end of the music selection   


   absbeat.allowGrowth(0);
   pitches.allowGrowth(0);
   durations.allowGrowth(0);
   levels.allowGrowth(0);

/*
   for (i=0; i<pitches.getSize(); i++) {
      pitches[i] = (pitches[i] - 2 + 40) % 40;
   }
*/
}



//////////////////////////////
//
// HumdrumFile::getNoteArray2 -- 
//     default values:  startLine = 0; endline = 0;
//
// last pitches and next pitches might need fixing for identification
//     of chords.
//

void HumdrumFile::getNoteArray2(Array<double>& absbeat, 
      Array<int>& pitches, Array<double>& durations, Array<double>& levels,
      Array<Array<int> >& lastpitches, Array<Array<int> >& nextpitches,
      int startLine, int endLine) {

   HumdrumFile& score = *this;

   if (endLine <= 0) {
      endLine = score.getNumLines() - 1;
   }
   if (startLine > endLine) {
      int temp = endLine;
      endLine = startLine;
      startLine = temp;
   }

   // estimate the largest amount necessary:
   absbeat.setSize(score.getNumLines() * score.getMaxTracks() * 10);
   pitches.setSize(score.getNumLines() * score.getMaxTracks() * 10);
   durations.setSize(score.getNumLines() * score.getMaxTracks() * 10);
   levels.setSize(score.getNumLines() * score.getMaxTracks() * 10);
   lastpitches.setSize(score.getNumLines() * score.getMaxTracks() * 10);
   nextpitches.setSize(score.getNumLines() * score.getMaxTracks() * 10);

   absbeat.setGrowth(score.getNumLines());
   pitches.setGrowth(score.getNumLines());
   durations.setGrowth(score.getNumLines());
   levels.setGrowth(score.getNumLines());
   lastpitches.setGrowth(score.getNumLines());
   nextpitches.setGrowth(score.getNumLines());

   absbeat.setSize(0);
   pitches.setSize(0);
   durations.setSize(0);
   levels.setSize(0);
   lastpitches.setSize(0);
   nextpitches.setSize(0);

   absbeat.allowGrowth(1);
   pitches.allowGrowth(1);
   durations.allowGrowth(1);
   levels.allowGrowth(1);
   lastpitches.allowGrowth(1);
   nextpitches.allowGrowth(1);

   Array<int> templastpitches;
   Array<int> tempnextpitches;
   templastpitches.setSize(100);
   tempnextpitches.setSize(100);
   templastpitches.setSize(0);
   tempnextpitches.setSize(0);

   Array<int> scorelevels;
   score.analyzeMetricLevel(scorelevels);
   
   int firsttime = 1;
   int i, j, k;
   int ii, jj;
   int ccount;
   static char buffer[1024] = {0};
   int pitch;
   double beatvalue;
   double duration;
   double level;
   for (i=startLine; i<=endLine; i++) {
      if (score[i].getType() != E_humrec_data) {
         // ignore non-note data lines
         continue;
      }
      beatvalue = score.getAbsBeat(i);
      ii = i;
      for (j=0; j<score[i].getFieldCount(); j++) {
         if (score[i].getExInterpNum(j) != E_KERN_EXINT) {
            // ignore non-kern data spines
            continue;
         }
         if (firsttime && strcmp(score[i][j], ".") == 0) {
            // extract the held over note from a previous point in the score
            ii = score[i].getDotLine(j);
            jj = score[i].getDotSpine(j);
            if (ii < 0 || jj < 0) {
               continue;
            }
         } else {
            ii = i;
            jj = j;
         }
 
         if (strcmp(score[ii][jj], ".") != 0) {
            // extract all notes in the region of interest, ignoring
            // tied notes.
            ccount = score[ii].getTokenCount(jj);
            const char* lastptr = "";
            const char* nextptr = "";
            for (k=0; k<ccount; k++) {
               score[ii].getToken(buffer, jj, k, 128);
               if (strchr(buffer, 'r') != NULL) {
                  // skip over rests
                  continue;
               }
               if (strchr(buffer, '_') != NULL) {
                  // skip over doubly tied notes
                  continue;                
               }
               if (!firsttime && strchr(buffer, ']') != NULL) {
                  // skip over tied notes at the ends of ties.
                  continue;
               }
               // have a note so now extract the metric level and the duration
               pitch = Convert::kernToBase40(buffer);
               if (pitch < 0) {
                  // ignore rests
                  continue;
               }
               // pitch = ((int)pitch - 2 + 40) % 40;
               duration = score.getTiedDuration(ii, jj, k);
               if (duration == 0.0) {
                  // ignore grace notes and other zero-dur ornaments
                  continue;
               }
               level = 1.0/pow(2.0, scorelevels[ii]);

               int dummyspine = 0;
               int testline;

               testline = getLastDatumLine(dummyspine, ii, jj, 0);
               if (testline >= startLine) {
                  lastptr = score.getLastDatum(ii, jj);
               } else {
                  lastptr = "";
               }

               testline = getNextDatumLine(dummyspine, ii, jj, 0);
               if (testline <= endLine) {
                  nextptr = score.getNextDatum(ii, jj);
               } else {
                  nextptr = "";
               }

               convertKernStringToArray(templastpitches, lastptr);
               convertKernStringToArray(tempnextpitches, nextptr);
 
               durations.append(duration);
               levels.append(level);
               pitches.append(pitch);
               absbeat.append(beatvalue);
               lastpitches.append(templastpitches);
               nextpitches.append(tempnextpitches);
 
            } // end of a chord
         }
      }  // end of a line
      firsttime = 0;
 
   } // end of the music selection   


   absbeat.allowGrowth(0);
   pitches.allowGrowth(0);
   durations.allowGrowth(0);
   levels.allowGrowth(0);

/*
   for (i=0; i<pitches.getSize(); i++) {
      pitches[i] = (pitches[i] - 2 + 40) % 40;
   }
*/
}



//////////////////////////////
//
// convertKernStringToArray --  extract the base 40 notes from kern data.
//

void HumdrumFile::convertKernStringToArray(Array<int>& array, 
      const char* string) {
   array.setSize(0);
   int note;
   char* buffer;
   int size = strlen(string);
   if (size == 0) {
      return;
   }
   buffer = new char[size+1];
   strcpy(buffer, string);
   char* ptr;
   ptr = strtok(buffer, " \t\n");
   while (ptr != NULL) {
      note = Convert::kernToBase40(ptr);
      array.append(note);
      ptr = strtok(NULL, " \t\n");
   }
   delete [] buffer;
   buffer = NULL;
}



//////////////////////////////
//
// HumdrumFile::getTiedDuration -- returns the total duration of
//   a tied note if the first note is the beginning of a tie.
//   Returns the duration of the note if not a tied note, or
//   zero if the specified field is not a note.
//     default value: token = 0;
//

double HumdrumFile::getTiedDuration(int linenum, int field, int token) {
   RationalNumber anum;
   anum = getTiedDurationR(linenum, field, token);
   return anum.getFloat();
}


RationalNumber HumdrumFile::getTiedDurationR(int linenum, int field, 
      int token) {
   HumdrumFile& file = *this;
   int length = file.getNumLines();
   char buffer[128] = {0};
   RationalNumber duration(0,1); // total duration of tied notes.
   int done = 0;                 // true when end of tied note is found
   int startpitch = 0;           // starting pitch of the tie
   int matchpitch = 0;           // current matching pitch of the tie
   
   file[linenum].getToken(buffer, field, token);
   if (strchr(buffer, '[')) {
      duration = Convert::kernToDurationR(buffer);
      // allow for enharmonic ties:
      startpitch = Convert::kernToMidiNoteNumber(buffer);
   } else {
      return Convert::kernToDurationR(buffer);
   }

// not quite perfect: if two primary tracks with common ties, will have prob:

   int m;
   int ptrack = file[linenum].getPrimaryTrack(field);
   int currentLine = linenum + 1;
   while (!done && currentLine < length) {
      if (file[currentLine].getType() != E_humrec_data) {
         currentLine++;
         continue;
      }

      for (m=0; m<file[currentLine].getFieldCount(); m++) {
         if (ptrack != file[currentLine].getPrimaryTrack(m)) {
            continue;
         }

         if (strchr(file[currentLine][m], '_')) {
            matchpitch = Convert::kernToMidiNoteNumber(file[currentLine][m]);
            if (startpitch == matchpitch) {
               duration += Convert::kernToDurationR(file[currentLine][m]);
            } else {
               done = 1;
            }
            break;
         } else if (strchr(file[currentLine][m], ']')) {
            matchpitch = Convert::kernToMidiNoteNumber(file[currentLine][m]);
            if (startpitch == matchpitch) {
               duration += Convert::kernToDurationR(file[currentLine][m]);
               done = 1;
            } else {
               done = 1;
            }
            break;
         }
      }
      currentLine++;
   }

   return duration;
}



//////////////////////////////
//
// HumdrumFile::getTotalTiedDuration -- return the duration of a tied
// group of notes, even if the the current note is not the first
// note in the tied group (i.e., go fine the first note in the
// group and then run the getTiedDurationR() function
//

RationalNumber HumdrumFile::getTotalTiedDurationR(int linenum, int field, 
      int token) {
   char buffer[128] = {0};
   (*this)[linenum].getToken(buffer, field, token);

   if ((strchr(buffer, '_') != NULL) || (strchr(buffer, ']') != NULL)) {
      int tline;
      int tcol;
      int ttok;
      getTiedStartLocation(linenum, field, token, tline, tcol, ttok);
      if ((tline < 0) || (tcol < 0) || (ttok < 0)) {
         return getTiedDurationR(linenum, field, token);
      } else{
         return getTiedDurationR(tline, tcol, ttok);
      }
   } else {
      return getTiedDurationR(linenum, field, token);
   }
}



//////////////////////////////
//
// HumdrumFile::getTiedStartLocation --  Need to generalize to chords.
//

void HumdrumFile::getTiedStartLocation(int linenum, int field, int token, 
      int& tline, int& tcol, int& ttok) {

   RationalNumber startbeat = -1;
   HumdrumFile& file = *this;
   char buffer[128] = {0};
   RationalNumber duration = 0;   // total duration of tied notes.
   int done = 0;                  // true when end of tied note is found
   int startpitch = 0;            // starting pitch of the tie
   int matchpitch = 0;            // current matching pitch of the tie
   
   file[linenum].getToken(buffer, field, token);
   if ((strchr(buffer, ']') != NULL) || (strchr(buffer, '_') != NULL)) {
      duration = Convert::kernToDurationR(buffer);
      // allow for enharmonic ties:
      startpitch = Convert::kernToMidiNoteNumber(buffer);
   } else {
      // nothing to do, at start of tie group or no tie
      tline = linenum; 
      tcol = field;
      ttok = token;
      return;
   }

   // search back through the music for the starting point of the tie.
   int m = field;
   int ptrack = file[linenum].getPrimaryTrack(field);
   int currentLine = linenum - 1;
   while (!done && currentLine >= 0) {
      if (file[currentLine].getType() != E_humrec_data) {
         currentLine--;
         continue;
      }

      for (m=0; m<file[currentLine].getFieldCount(); m++) {
         if (ptrack != file[currentLine].getPrimaryTrack(m)) {
            continue;
         }

         if (strchr(file[currentLine][m], '_') != NULL) {
            matchpitch = Convert::kernToMidiNoteNumber(file[currentLine][m]);
            if (startpitch == matchpitch) {
               break;
               // continue searching backwards in file
            } else {
               done = 1;
            }
            break;
         } else if (strchr(file[currentLine][m], ']') != NULL) {
            matchpitch = Convert::kernToMidiNoteNumber(file[currentLine][m]);
            if (startpitch == matchpitch) {
               break;
               // continue searching backwards in file
            } else {
               done = 1;
            }
            break;
         } else if (strchr(file[currentLine][m], '[') != NULL) {
            tline = currentLine;
            tcol = m;
            ttok = 0;  // not bothering with chords yet...
            return;
         }
      }
      currentLine--;
   }

   tline = currentLine;
   tcol = m;
   ttok = 0;   // not bothering with chords yet...
}



//////////////////////////////
//
// HumdrumFile::getTiedStartBeat --
//     default value: token = 0;
//

double HumdrumFile::getTiedStartBeat(int linenum, int field, int token) {
   RationalNumber anum;
   anum = getTiedStartBeatR(linenum, field, token);
   return anum.getFloat();
}



RationalNumber HumdrumFile::getTiedStartBeatR(int linenum, int field, 
      int token) {
   RationalNumber startbeat = -1;
   HumdrumFile& file = *this;
   int length = file.getNumLines();
   char buffer[128] = {0};
   RationalNumber duration = 0;   // total duration of tied notes.
   int done = 0;                  // true when end of tied note is found
   int startpitch = 0;            // starting pitch of the tie
   int matchpitch = 0;            // current matching pitch of the tie
   
   file[linenum].getToken(buffer, field, token);
   // should be == NULL and not != NULL? [20110218]
   if (strchr(buffer, '[') == NULL) {
      duration = Convert::kernToDurationR(buffer);
      // allow for enharmonic ties:
      startpitch = Convert::kernToMidiNoteNumber(buffer);
   } else {
      startbeat = file[linenum].getAbsBeatR();
      return startbeat;
   }

   int m;
   int ptrack = file[linenum].getPrimaryTrack(field);
   int currentLine = linenum + 1;
   while (!done && currentLine < length) {
      if (file[currentLine].getType() != E_humrec_data) {
         currentLine++;
         continue;
      }

      for (m=0; m<file[currentLine].getFieldCount(); m++) {
         if (ptrack != file[currentLine].getPrimaryTrack(m)) {
            continue;
         }

         if (strchr(file[currentLine][m], '_')) {
            matchpitch = Convert::kernToMidiNoteNumber(file[currentLine][m]);
            if (startpitch == matchpitch) {
               duration += Convert::kernToDurationR(file[currentLine][m]);
            } else {
               done = 1;
            }
            break;
         } else if (strchr(file[currentLine][m], ']')) {
            matchpitch = Convert::kernToMidiNoteNumber(file[currentLine][m]);
            if (startpitch == matchpitch) {
               duration += Convert::kernToDurationR(file[currentLine][m]);
               done = 1;
            } else {
               done = 1;
            }
            break;
         }
      }
      currentLine++;
   }

   return startbeat;
}



//////////////////////////////
//
// HumdrumFile::getTotalDuration -- returns the total beat count of
//     the file.
//

double HumdrumFile::getTotalDuration(void) {
   RationalNumber anum;
   anum = getTotalDurationR();
   double output = ((int)(anum.getFloat() * 10000 + 0.5))/10000.0;
   return output;
}

RationalNumber HumdrumFile::getTotalDurationR(void) {
   HumdrumFile& score = *this;
   RationalNumber output = score[score.getNumLines()-1].getAbsBeatR();
   return output;
}



//////////////////////////////
//
// HumdrumFile::operator=
//

HumdrumFile& HumdrumFile::operator=(const HumdrumFile& aFile) {
   // don't copy onto self
   if (&aFile == this) {
      return *this;
   }

   // delete any current contents
   int i;
   for (i=0; i<records.getSize(); i++) {
      delete records[i];
      records[i] = NULL;
   }

   records.setSize(aFile.records.getSize());
   for (i=0; i<aFile.records.getSize(); i++) {
      records[i] = new HumdrumRecord;
      *(records[i]) = *(aFile.records[i]);
   }

   rhythmcheck = aFile.rhythmcheck;
   maxtracks = aFile.maxtracks;
   localrhythms = aFile.localrhythms;
   return *this;
}



//////////////////////////////
//
// HumdrumFile::read -- read in a humdrum file.
//

void HumdrumFile::read(const char* filename) {
   HumdrumFileBasic::read(filename);
   rhythmcheck = 0;
}


void HumdrumFile::read(istream& inStream) {
   HumdrumFileBasic::read(inStream);
   rhythmcheck = 0;
}



//////////////////////////////////////////////////////////////////////////
//
// functions related to rhythm analysis
//

//////////////////////////////
//
// HumdrumFile::rhythmQ -- returns 1 if the rhythm analysis has been
//     done, otherwise should return 0.  If a line is appended to
//     the HumdrumFile then rhythmQ is turned off.  If you read a new
//     file, then rhythmQ is turned off.
//

int HumdrumFile::rhythmQ(void) {
   return rhythmcheck;
}



//////////////////////////////
//
// HumdrumFile::privateRhythmAnalysis --
//     default value: base = "", debug = 0
//

void HumdrumFile::privateRhythmAnalysis(const char* base, int debug) {
   int init = 0;                   // marker indicating when the data starts
   int datainit = 0;               // marker indicating when the data starts

   minrhythm = 0;                  // keeping track of the min timebase 
   minrhythmR = 0;
   Array<int> rhythms;
   Array<RationalNumber> rhythmsR;

   rhythms.setSize(32);
   rhythms.setSize(0);
   rhythms.allowGrowth(1);

   rhythmsR.setSize(32);
   rhythmsR.setSize(0);
   rhythmsR.allowGrowth(1);

   HumdrumFile& infile = *this;
   RationalNumber summation(0,1);  // for summing measure duration
   RationalNumber duration;
   HumdrumRecord tempRecord;       // for *beat: interpretation
   // const char* slash;              // for metronome marking
   RationalNumber measureBeats(0,1);
   // for fixing meter locations:

   SigCollection<RationalNumber> meterbeats; 
   SigCollection<RationalNumber> timebaseC;

   meterbeats.setSize(getNumLines());
   timebaseC.setSize(getNumLines());
   meterbeats.allowGrowth(0);
   timebaseC.allowGrowth(0);

   Array<int> ignore;              // for avoiding free rhythm spines
   ignore.setSize(infile.getMaxTracks());
   ignore.setAll(0);

   // for analyzing record durations:
   SigCollection<RationalNumber> lastdurations;
   SigCollection<RationalNumber> runningstatus;

   int fixedTimebase = 0;
   RationalNumber timebase = 4;
   if (strcmp(base, "") != 0) {
      fixedTimebase = 1;
      int tempval;
      sscanf(base, "%d", &tempval);
      timebase = tempval;
 
      // check for prolongation dot.  Ignore any double dots
      if (strchr(base, '.') != NULL) {
         timebase = (timebase*2)/3;
      }
   }

   HumdrumRecord currRecord;
   int measurecount = 0;
   int ii, jj;
   int nonblank = 0;
   int foundstart = 0;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (debug != 0) {
         cout << "processing line " << (i+1) << " of input ..." << endl;
         cout << infile[i] << endl;
      }

      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_bibliography:
         case E_humrec_global_comment:
         case E_humrec_data_comment:
            infile[i].setDurationR(0,1);
            if (i+1 < infile.getNumLines()) {
               infile[i+1].setAbsBeatR(infile[i].getAbsBeatR());
               infile[i+1].setBeatR(infile[i].getBeatR());
            }
            if (datainit == 0) {
               infile[i].setBeatR(0,1);
            }
            break;

         case E_humrec_data_interpretation:
            if (debug) {
               cout << "line is an interpretation" << endl;
            }
            infile[i].setDurationR(0,1);
            if (i+1 < infile.getNumLines()) {
               infile[i+1].setAbsBeatR(infile[i].getAbsBeatR());
               infile[i+1].setBeatR(infile[i].getBeatR());
            }
            if (debug) {
               cout << "Beat position of line is " 
                    << infile[i].getBeat() << endl;
            }

            if (!foundstart && infile[i].hasExclusiveQ()) {
               foundstart = 1;
               initializeTracers(lastdurations, runningstatus, infile[i]);
            } else {
               // check for time signature
               PerlRegularExpression pre;
               if (pre.search(infile[i][0], "^\\*M(\\d+)/(\\d+)", "")) {
                  int top = atoi(pre.getSubmatch(1));
                  int bot = atoi(pre.getSubmatch(2));
                  if (bot == 0) {
                     bot = 1;
                     top *= 2;
                  }
                  measureBeats = top;
                  measureBeats /= bot;
               }

               /* 20110202
               slash = strchr(infile[i][0], '/');
               if ((infile[i][0][1] == 'M') && isdigit(infile[i][0][2]) &&
                     (slash != NULL)) {
                  measureBeats = atoi(&infile[i][0][2]);
                  if (!fixedTimebase) {
                     timebase = atoi(&slash[1]);
                  } else {
                     measureBeats *= 4;
                     measureBeats /= atoi(&slash[1]);
                  }
               }
               */
   
            }
            for (ii=0; ii<infile[i].getFieldCount(); ii++) {
               if (strcmp(infile[i][ii], "*free") == 0) {
                  adjustForRhythmMarker(infile[i], 1, ii, lastdurations, 
                     runningstatus, init, datainit, ignore);
               } else if (strcmp(infile[i][ii], "*strict") == 0) {
                  adjustForRhythmMarker(infile[i], 0, ii, lastdurations, 
                     runningstatus, init, datainit, ignore);
               } 
            }
            if (infile[i].hasPathQ()) {
               adjustForSpinePaths(infile[i], lastdurations, runningstatus, 
                     init, datainit, ignore);
            }
            //if (datainit == 0) {
            //   infile[i].setBeatR(0,1);
            //   if (debug) {
            //      cout << "The music needs to be initialized " << endl;
            //   }
            //}
            break;

         case E_humrec_data_measure:
            summation.setValue(0,1);
            infile[i].setDurationR(0,1);
            if (i+1 < infile.getNumLines()) {
               infile[i+1].setAbsBeatR(infile[i].getAbsBeatR());
               // infile[i+1].setBeatR(summation);
               infile[i+1].setBeatR(summation + 1);
               // infile[i].setBeatR(0,1);
            }
	    measurecount++;
            break;

         case E_humrec_data:
            datainit = 1;  // data has been seen, so indicate so

            // handle null fields

            nonblank = 0;
            for (jj=0; jj<infile[i].getFieldCount(); jj++) {
               if (ignore[infile[i].getPrimaryTrack(jj)-1] != 0) {
                  continue;
               }
               if ((strcmp(infile[i].getExInterp(jj), "**kern") != 0) &&
                   (strcmp(infile[i].getExInterp(jj), "**koto") != 0)) {
                  continue;                  
               }
               if (strcmp(infile[i][jj], ".") != 0) {
                  nonblank = 1;
                  break;
               }
            }
            if (nonblank == 0) {
               infile[i].setDurationR(0,1);
               if (i+1 < infile.getNumLines()) {
                  infile[i+1].setAbsBeatR(infile[i].getAbsBeatR());
                  infile[i+1].setBeatR(infile[i].getBeatR());
               }
               break;
            }

            if (infile[i].equalFieldsQ("**kern", ".") &&
               infile[i].equalFieldsQ("**koto", ".")) {
            }

//            duration = (determineDurationR(infile[i], init,
//               lastdurations, runningstatus, rhythms, ignore) * timebase) / 4;
            duration = (determineDurationR2(infile[i], init,
               lastdurations, runningstatus, rhythmsR, ignore) * timebase) / 4;

            infile[i].setDurationR(duration);
            if (datainit && i+1 < infile.getNumLines()) {
               infile[i+1].setAbsBeatR(infile[i].getAbsBeatR() + duration);
               infile[i+1].setBeatR(infile[i].getBeatR() + duration);
            } else if (datainit == 0) {
               datainit = 1;
               infile[i+1].setAbsBeatR(infile[i].getAbsBeatR() + duration);
               infile[i].setBeatR(1);
               infile[i+1].setBeatR(infile[i].getBeatR() + duration);
            }

            break;
         default:
            cerr << "Error on line " << (i+1) << " of input" << endl;
            exit(1);
      }

      meterbeats[i] = measureBeats;
      timebaseC[i] = timebase;
   }

   fixIncompleteBarMeterR(meterbeats, timebaseC);
   fixIrritatingPickupProblem();
   rhythms.setSize(rhythmsR.getSize());
   for (i=0; i<rhythms.getSize(); i++) {
      // cout << "XRHYTHM = " << rhythmsR[i] << endl;

      if (ispoweroftwo(rhythmsR[i].getNumerator())) {
         // since the numerator of the rhythmic value is a power of
         // two, the denominator can be ignored since the numerator
         // alone controls the minimum rhythmic unit with respect to
         // the whole note.
         rhythms[i] = rhythmsR[i].getNumerator();
      } else {
         // this will have to be thought about some more
         rhythms[i] = rhythmsR[i].getNumerator() * rhythmsR[i].getDenominator();
      }

      rhythmsR[i] = rhythms[i];
   } 
   minrhythm = findlcm(rhythms);
   localrhythms.setSize(rhythmsR.getSize());
   for (i=0; i<rhythmsR.getSize(); i++) {
      localrhythms[i] = rhythmsR[i];
   }
   // spaceEmptyLines();

   // add offset of +1 if there are no barlines present in the file
   if (measurecount == 0) {
      for (i=0; i<infile.getNumLines(); i++) {
         if (infile[i].getType() != E_humrec_data) {
            continue;
         }
         infile[i].setBeatR(infile[i].getBeatR()+1);
      }
   }

   // this will eventually replace minrhythm:
   minrhythmR = getMinimumRationalRhythm(rhythmsR);
}


///////////////////////////////
//
// ispoweroftwo --
//

int HumdrumFile::ispoweroftwo(int value) {
   if (value < 0) {
      return 0;
   }
   return !(value & (value - 1));
}



//////////////////////////////
//
// HumdrumFile::getMinimumRationalRhythm -- Examine the minimum
//    time difference between successive lines in a Humdrum file
//    and then return the inversion of that duration.  If the 
//    minimum duration is 0, then return -1 (or maybe 0 as currently).
//

RationalNumber HumdrumFile::getMinimumRationalRhythm(
      Array<RationalNumber>& rhythms) {

   // RationalNumber zeroR(0,1);
   // RationalNumber maxval  = zeroR;  
   // RationalNumber output  = zeroR;
   int i;

   // for (i=0; i<rhythms.getSize(); i++) {
   //    if (maxval < rhythms[i].getNumerator()) {
   //       maxval = rhythms[i].getNumerator();
   //    }
   // }

   Array<int> singler;
   singler.setSize(rhythms.getSize());
   for (i=0; i<rhythms.getSize(); i++) {
      // cout << "YRHYTHM = " << rhythms[i] << endl;
      singler[i] = rhythms[i].getNumerator() * rhythms[i].getDenominator();
   }

   int output = findlcm(singler);

   // if (maxval == zeroR) {
   //    return zeroR;
   // } else {
   //    return maxval;
   // }
   
   return output;
}



//////////////////////////////
//
// fixIrritatingPickupProblem -- some pickup beats are not assigned
//     the correct beat value, so fix here.
//

void HumdrumFile::fixIrritatingPickupProblem(void) {
   int bari;

   int numerator;
   int denominator;
   RationalNumber beatsperbar(0,1);
   int j;

   for (bari=0; bari<getNumLines(); bari++) {
      if (getType(bari) == E_humrec_data_interpretation) {
         for (j=0; j<(*this)[bari].getFieldCount(); j++) {
            if (sscanf((*this)[bari][j], "*M%d/%d", &numerator, &denominator) 
                  == 2) {
               if (denominator == 0) {
                  // allow for breve beats 20110202
                  numerator *=  2;
                  denominator = 1;
               }
               beatsperbar.setValue(numerator * 4, denominator);
               break;
            }
         }
      }

      if (getType(bari) == E_humrec_data_measure) {
         break;
      } 
   }

   if (bari < getNumLines()) {

      if (getType(bari) != E_humrec_data_measure) {
         return;
      }

      if (getBeatR(bari).isPositive()) {
         return;
      }

      if (getBeatR(bari).isZero() && getAbsBeatR(bari).isZero()) {
         return;
      }

      if (beatsperbar.isZero()) {
         return;
      }
   }
   int i;
   RationalNumber sum = 0;
   for (i=bari-1; i>=0; i--) {
      sum += getDurationR(i);      
      (*this)[i].setBeatR(beatsperbar - sum + 1);
   }

}



//////////////////////////////
//
// HumdrumFile::spaceEmptyLines -- if a line has zero duration,
//    and there is no grace note on the line, then steal
//    time from the previous line equally for each following 
//    empty line (which does not have a measure line).
//

void HumdrumFile::spaceEmptyLines(void) {
   Array<int> index;
   index.setSize(getNumLines());
   index.setSize(0);

   int i;
   int q;
   int valid;
   for (i=0; i<getNumLines(); i++) {
      if (getType(i) == E_humrec_data) {
         // if the line has grace notes, ignore it.
         valid = 1;
         for (q=0; q<(*this)[i].getFieldCount(); q++) {
            if (!(*this)[i].isExInterp(q, "**kern")) {
               continue;
            }
            if (strchr((*this)[i][q], 'q') != NULL) {
               valid = 0;
               break;
            }
         }
         if (valid != 0) {
            index.append(i);
         }
      } else if (getType(i) == E_humrec_data_measure) {
         // keep track of barlines so that the spacing does not occur 
         // across barlines
         index.append(i);
      }
   }

   int j;
   int count = 0;
   RationalNumber newduration;
   newduration.zero();
   RationalNumber startbeat(0,1);
   RationalNumber basebeat(0,1);
   index.allowGrowth(0);
   for (i=0; i<index.getSize(); i++) {
      if ((i > 0) && ((*this)[index[i]].getDuration() == 0)
                  && ((*this)[index[i]].getType() != E_humrec_data_measure)) {
         count = 1;
         j = i+1;
         while ((j<index.getSize()) && ((*this)[index[j]].getDuration()==0)
                && ((*this)[index[j]].getType() != E_humrec_data_measure)
               ) {

            count++;
            j++;
         }
//         if (valid == 0) {
//            continue;
//         }
         // newduration = (*this)[index[i-1]].getDuration() / (count+1.0);
         newduration = (*this)[index[i-1]].getDurationR() / (count+1);
         basebeat = (*this)[index[i-1]].getBeatR();
         for (j=0; j<count+1; j++) {
            (*this)[index[i-1+j]].setDurationR(newduration);
            // adjust the metric position
            RationalNumber value = basebeat;  
            value += newduration * j;

            (*this)[index[i-1+j]].setBeatR(basebeat + newduration * j);
         }

         startbeat = (*this)[index[i-1]].getAbsBeatR();
         for (j=0; j<count; j++) {
            RationalNumber value = startbeat + newduration*(j+1);
            (*this)[index[i+j]].setAbsBeatR(value);
         }

      }
   }

   // adjust other elements which may be out of absbeat order now
   RationalNumber lastpos = (*this)[getNumLines()-1].getAbsBeatR();
   RationalNumber lastbeat = (*this)[getNumLines()-1].getBeatR();
   for (i=getNumLines()-1; i>=0; i--) {
      if ((*this)[i].getAbsBeatR() > lastpos) {
         (*this)[i].setAbsBeatR(lastpos);
         // adjust the metric position
         (*this)[i].setBeatR(lastbeat);
      } else {
         lastpos = (*this)[i].getAbsBeatR();
         lastbeat = (*this)[i].getBeatR();
      }

      // adjust the number of beats found in the measure
      if (i < getNumLines() - 1) {
         if ((*this)[i+1].getType() == E_humrec_data_measure) {
            (*this)[i+1].setBeatR(lastbeat + (*this)[i].getDurationR() - 1);
         }
      }
   }

   // adjust the duration markers for each measure
   lastbeat = (*this)[getNumLines()-1].getAbsBeatR();
   RationalNumber curbeat(0,1);
   for (i=getNumLines()-1; i>=0; i--) {
      if ((*this)[i].getType() == E_humrec_data_measure) {
         curbeat = (*this)[i].getAbsBeatR(); 
         (*this)[i].setBeatR(lastbeat - curbeat);
         lastbeat = curbeat;
         // mark pickup-beats with a negative duration
         if ((*this)[i].getAbsBeatR() < (*this)[i].getBeatR()) {
            if (!(*this)[i].getAbsBeatR().isZero()) {
               (*this)[i].setBeatR((*this)[i].getBeatR() * -1);
            }
         }
      }

   }
}



//////////////////////////////
//
// findlcm -- find the least common multiple between rhythms
//

int HumdrumFile::findlcm(Array<int>& rhythms) {
   if (rhythms.getSize() == 0) {
      return 0;
   }
   int output = rhythms[0];
   for (int i=1; i<rhythms.getSize(); i++) {
      output = output * rhythms[i] / GCD(output, rhythms[i]);
   }

   return output;
}

 

//////////////////////////////
//
// GCD -- greatest common divisor
//
 
int HumdrumFile::GCD(int a, int b) {
   if (b == 0) {
      return a;
   }
   int z = a % b;
   a = b;
   b = z;
   return GCD(a, b);
}    



//////////////////////////////
//
// HumdrumFile:initializeTracers -- set up the duration checking
//     functions.
//

void HumdrumFile::initializeTracers(
      SigCollection<RationalNumber>& lastdurations,
      SigCollection<RationalNumber>& runningstatus, HumdrumRecord& currRecord) {
   lastdurations.allowGrowth(1);
   runningstatus.allowGrowth(1);
   lastdurations.setSize(0);
   runningstatus.setSize(0);
   RationalNumber zero(0,1);
   int i;
   for (i=0; i<currRecord.getFieldCount(); i++) {
      if (currRecord.getExInterpNum(i) == E_KERN_EXINT ||
          strcmp(currRecord.getExInterp(i), "**koto") == 0) {
         lastdurations.append(zero);
         runningstatus.append(zero);
      }
   }
   lastdurations.allowGrowth(0);
   runningstatus.allowGrowth(0);
}



//////////////////////////////
//
// HumdrumFile::fixIncompleteBarMeterR -- resolve when incomplete bars are
//    supposed to be the ends of measures rather than the
//    beginnings of measures.
//

void HumdrumFile::fixIncompleteBarMeterR(
      SigCollection<RationalNumber>& meterbeats,
      SigCollection<RationalNumber>& timebase) {

   int sumstatus = 0;
   int lasti = 0;
   int init = 0;
   int k;
   int i;
   RationalNumber mb(0,1);
   
   for (i=0; i<getNumLines(); i++) {

      // at each measure line determine one of three cases:
      // (1) all ok -- the summation of durations in the measure
      //     matches the current time signature.
      // (2) a partial measure -- the measure durations do not
      //     add up to the time signature, but the measure is
      //     at the start/end of a musical section such as the
      //     beginning of a piece, end of a piece, or between
      //     repeat bar dividing a full measure.
      // (3) the sum of the durations does not match the 
      //     time signature because the durations are incorrectly
      //     given.
      //

      if (getType(i) != E_humrec_data_kern_measure) {
         continue;
      }

      // case 2a: start of piece
      RationalNumber difference;
      if (init == 0) {
         init = 1;
         lasti = i;
         (*this)[i].setBeatR(getBeatR(i)-1);
         if (pickupdur.isNegative()) {
            pickupdur = getAbsBeatR(i);
         }
	 if (pickupdur == meterbeats[i]) {
            pickupdur.zero();
         }
         if (pickupdur.isPositive()) {
            k = i-1;
	    RationalNumber sum(0,1);
            while (k >= 0) {
               if (mb.isZero()) {
                  mb = meterbeats[k];
               }
	       sum += getDurationR(k);
               ((*this)[k]).setBeatR(mb - sum + 1);
               k--;
            }
         } else if (pickupdur.isZero()) {
            // fix offset from zero to offset from one
            k=i-1;
            while (k >= 0)  {
               if (getType(k) == E_humrec_data) {
                  (*this)[k].setBeatR((*this)[k].getBeatR()+1);
               }
               k--;
            }
         }
         continue;
      }

      // case 1: measure sum is complete
      if (getBeatR(i) == meterbeats[i] + 1) {
         if (pickupdur.isNegative()) {
            pickupdur = 0;
         }
         lasti = i;
         ((*this)[i]).setBeatR(getBeatR(i)-1);
         sumstatus = 0;
         continue;
      }
 
      // case 2b: repeat bar or something splitting up a regular bar
      if (getBeatR(i) < meterbeats[i] + 1) {
         ((*this)[i]).setBeatR(getBeatR(i)-1);
         if (pickupdur.isNegative()) {
            pickupdur = getBeatR(i);
         }
         if (sumstatus == 1 && (getBeatR(i) + 
				 getBeatR(lasti) == meterbeats[i])) {
            for (k=i-1; k>lasti; k--) {
               ((*this)[k]).setBeatR(getBeatR(lasti) + getBeatR(k));
            }
         } 
         lasti = i;
         sumstatus = 1;
      }

      // case 3: incorrect measure duration: ignore this error 

   }

   if (pickupdur.isZero()) {
      for (i=0; i<getNumLines(); i++) {
         if (!(((*this)[i].getType() == E_humrec_data) 
               || ((*this)[i].getType() == E_humrec_data_measure))) {
            (*this)[i].setBeatR(0,1);
         } else {
            break;
         }
      }
   }

}



//////////////////////////////
//
// HumdrumFile::adjustForRhythmMarker --
//

void HumdrumFile::adjustForRhythmMarker(HumdrumRecord& aRecord,
      int state, int spine, SigCollection<RationalNumber>& lastdurations, 
      SigCollection<RationalNumber>& runningstatus, int& init, int& datastart,
      Array<int>& ignore) {

   SigCollection<RationalNumber> newdurations;
   SigCollection<RationalNumber> newstatus;
   newdurations.setSize(lastdurations.getSize() + 4);
   newstatus.setSize(runningstatus.getSize() + 4);
   newdurations.setGrowth(newdurations.getSize());
   newstatus.setGrowth(newstatus.getSize());
   newdurations.setSize(0);
   newstatus.setSize(0);
   newdurations.allowGrowth();
   newstatus.allowGrowth();

   if (aRecord.getExInterpNum(spine) != E_KERN_EXINT &&
      strcmp(aRecord.getExInterp(spine), "**koto") != 0) {
      return;
   }

   int i;
   int ii = 0;
   for (i=0; i<aRecord.getFieldCount(); i++) {
      if (i == spine) {
         if (state) {
            // start ignoring
            ignore[aRecord.getPrimaryTrack(i)-1] = 1;
            ii++;
            continue; 
         } else {
            // stop ignoring
            ignore[aRecord.getPrimaryTrack(i)-1] = 0;
            RationalNumber zero(0,1);
            lastdurations.append(zero);
            runningstatus.append(zero);
         }
      } else if (ignore[aRecord.getPrimaryTrack(i) - 1] != 0) {
         continue;
      } else if (aRecord.getExInterpNum(i) != E_KERN_EXINT &&
          strcmp(aRecord.getExInterp(i), "**koto") != 0) {
         continue;
      }
      
      newdurations.append(lastdurations[ii]);
      newstatus.append(runningstatus[ii]);
      ii++;
   }

   lastdurations.setSize(newdurations.getSize());
   runningstatus.setSize(newstatus.getSize());

   for (i=0; i<newdurations.getSize(); i++) {
      lastdurations[i] = newdurations[i];
   }

   for (i=0; i<newstatus.getSize(); i++) {
      runningstatus[i] = newstatus[i];
   }

}



//////////////////////////////
//
// HumdrumFile::adjustForSpinePaths -- Take the previous spine information
//    and generate a new spine information set based on the HumdrumRecord
//    line that has at least one path indicator
//

void HumdrumFile::adjustForSpinePaths(HumdrumRecord& aRecord, 
      SigCollection<RationalNumber>& lastdurations, 
      SigCollection<RationalNumber>& runningstatus,
      int& init, int& datastart, Array<int>& ignore) {

   int spinecount = aRecord.getFieldCount();
   int subcount;
   int inindex = 0;

   SigCollection<RationalNumber> newdurations;
   SigCollection<RationalNumber> newstatus;
   newdurations.allowGrowth();
   newstatus.allowGrowth();
   newstatus.setSize(runningstatus.getSize() + 4);
   newdurations.setSize(lastdurations.getSize() + 4);
   newdurations.setGrowth(newdurations.getSize());
   newstatus.setGrowth(newstatus.getSize());
   newdurations.setSize(0);
   newstatus.setSize(0);

   int i, j;
   for (i=0; i<spinecount; i++) {
      if (ignore[aRecord.getPrimaryTrack(i)-1] != 0) {
         continue;
      }
      if ((aRecord.getExInterpNum(i) != E_KERN_EXINT) &&
          (strcmp(aRecord.getExInterp(i), "**koto") != 0)) {
         continue;
      }

      if (strcmp("*^", aRecord[i]) == 0) {
         newdurations.append(lastdurations[inindex]);
         newdurations.append(lastdurations[inindex]);
         newstatus.append(runningstatus[inindex]);
         newstatus.append(runningstatus[inindex]);
         inindex++;
      } else if (strcmp("*-", aRecord[i]) == 0) {
         inindex++;
      } else if (strcmp("*+", aRecord[i]) == 0) {
         // do nothing: wait and see if a new kern spine is added.
         inindex++;
      } else if (strcmp("*x", aRecord[i]) == 0) {
         if (lastdurations.getSize() < inindex+1) {
            // this code needed for cases when *x is swapping
            // the order of a rhythmic and non-rhythmic spine.
            // might need to be more fixing (only tested
            // on two rhythmic spines.
            newdurations.append(lastdurations[inindex+1]);
         }
         newdurations.append(lastdurations[inindex]);
         if (runningstatus.getSize() < inindex+1) {
            // like the previous comment
            newstatus.append(runningstatus[inindex+1]);
         }
         newstatus.append(runningstatus[inindex]);
         inindex+= 2;
         i++;
      } else if (strcmp("*v", aRecord[i]) == 0) {
         subcount = 1;
         for (j=i+1; j<spinecount; j++) {
            if (strcmp("*v", aRecord[j]) == 0) {
               subcount++;
            } else {
               // non-adjacent *v spines are not related
               break;
            }
         }
         if (subcount == 1) {
            cout << "Error: single *v path indicator: " << aRecord.getLine()
                 << endl;
            exit(1);
         } else {
            newdurations.append(lastdurations[inindex]);
            newstatus.append(runningstatus[inindex]);
            // note: the following line had an insidious off-by-one error
            // the follownig line CANNOT be: inindex += subcount-1;
            inindex += subcount;   
            i += subcount-1;   
         }
      } else if (strncmp("**", aRecord[inindex], 2) == 0) {
         newdurations.setSize(newdurations.getSize()+1);
         newdurations[newdurations.getSize()+1] = 0;
         newstatus.setSize(newdurations.getSize()+1);
         newstatus[newdurations.getSize()+1] = 0;
      } else {
         newdurations.append(lastdurations[inindex]);
         newstatus.append(runningstatus[inindex]);
         inindex++;
      }
   }

   // now replace the old arrays with the new values:

   lastdurations.setSize(newdurations.getSize());
   for (i=0; i<newdurations.getSize(); i++) {
      lastdurations[i] = newdurations[i];
   }

   runningstatus.setSize(newstatus.getSize());
   for (i=0; i<newstatus.getSize(); i++) {
      runningstatus[i] = newstatus[i];
   }

   if (aRecord.equalDataQ("*-")) {
      init = 0;
      datastart = 0;
   }

}



//////////////////////////////
//
// HumdrumFile::determineDurationR2 -- determines the duration of the **kern
//	entries before a new **kern entry.  Also works on **koto spines.
//
//	This function will eventually replace determineDurationR
//	It changes Array<int>& rhythms to
//	           Array<RationalNumber>& rhythms.
//

RationalNumber HumdrumFile::determineDurationR2(HumdrumRecord& aRecord,
      int& init, SigCollection<RationalNumber>& lastdurations, 
      SigCollection<RationalNumber>& runningstatus,
      Array<RationalNumber>& rhythms, Array<int>& ignore) {
   int i;
   // initialization:
   if (init) {
      init = 0;
      int size = aRecord.getFieldCount("**kern");
      size += aRecord.getFieldCount("**koto");
      lastdurations.setSize(size);
      runningstatus.setSize(size);
      for (i=0; i<size; i++) {
         lastdurations[i].zero();
         runningstatus[i].zero();
      }
   }

   // Step (1): if lastdurations == runningstatus, then zero running
   // status.
   RationalNumber zero(0,1);
   for (i=0; i<runningstatus.getSize(); i++) {
      if ((runningstatus[i] - lastdurations[i]) == zero) {
         runningstatus[i].zero();
      } else {
      }
   }
 
   // Step (2): input new durations into the lastdurations array
   int q;
   int count = 0;
   int stype = 0;
   // static char rbuff[32] = {0};
   for (i=0; i<aRecord.getFieldCount(); i++) {
      if (ignore[aRecord.getPrimaryTrack(i)-1] != 0) {
         stype = 0;
      } else if (aRecord.getExInterpNum(i) == E_KERN_EXINT) {
         stype = 1;
      } else if (strcmp(aRecord.getExInterp(i), "**koto") == 0) {
         stype = 2;
      } else {
         stype = 0;
      }
      if (stype) {
         if (strcmp(aRecord[i], ".") != 0) {
            switch (stype) {
               case 1:
                  lastdurations[count] = Convert::kernToDurationR(aRecord[i]);
                  if (strchr(aRecord[i], 'P') != NULL) {
                     // remove appogiatura durations for summations
                     // lastdurations[count] = 0;
                  } else if ((strchr(aRecord[i], 'q') != NULL) ||
                             (strchr(aRecord[i], 'Q') != NULL)) {
                     // remove gracenote durations from summations
                     lastdurations[count] = 0;
                  } 
                  break;
               case 2:
                  lastdurations[count] = Convert::kotoToDurationR(aRecord[i]);
                  if ((strchr(aRecord[i], 'q') != NULL) ||
                      (strchr(aRecord[i], 'Q') != NULL)) {
                     // remove gracenote durations from summations
                     lastdurations[count] = 0;
                  } 
                  break;
            }

            if (lastdurations[count] != 0) {
               // have a legitimate rhythm, store it in the rhythms array.
               RationalNumber sss = lastdurations[count];
               // Convert::durationRToKernRhythm(rbuff, sss);
               RationalNumber rbase = sss.getInversion() * 4;
               // int rbase  = atoi(rbuff);
               // int length = strlen(rbuff);
               int z;
               // for (z=length-1; z>0; z--) {
               //    if (rbuff[z] == '.') {
               //       rbase = 2 * rbase;
               //    }
               // }
               int done = 0;
               RationalNumber value;
               for (z=0; z<rhythms.getSize(); z++) {
                  if (rbase == 0) {
                     done = 1;
                     break;
                  }
                  value = rhythms[z] / rbase;
                  if (value.getDenominator() == 1) {
                     // if the duration of rbase is an integer
                     // multiple of a particular rhythm, then stop
                     // processing, since the minimum rhythm calculation
                     // will not need to know anything about rbase.
                     done = 1;
                     break;
                  }
               }
               if (!done) {
                  rhythms.append(rbase);
               }

            }

            if (strstr(aRecord[i], "--") != NULL && runningstatus[count] != 0) {
               cout << "Error in rhythm on line: " << aRecord.getLineNum()
                    << endl;
 
               cout << "Error on line: " << aRecord.getLineNum() 
                    << ": problem with rhythm in spine " << i+1 << endl;
 
               cout << "\n\t" << aRecord.getLine() << endl;
               cout << "D";
               for (q=0; q<lastdurations.getSize(); q++) {
                  cout << "\t" << lastdurations[q]; 
               }
               cout << endl;
               cout << "RT";
               for (q=0; q<runningstatus.getSize(); q++) {
                  cout << "\t" << runningstatus[q]; 
               }
               cout << endl;
               exit(1);
            }
        } else {
           // do nothing
        }
         count++;
      }
   }
   if (count != runningstatus.getSize()) {
      cerr << "Error: spine count has changed" << endl;
   }

   // Step (3): find minimum duration by subtracting last from running
   RationalNumber min(99999999,1);
   RationalNumber testval;

   for (i=0; i<lastdurations.getSize(); i++) {
      testval = lastdurations[i] - runningstatus[i];
      if (testval.isNegative()) {   
         cout << "Error on line: " << aRecord.getLineNum() 
              << ": problem with rhythm in **kern spine " 
              << i+1 << endl;
         cout << "Line min duration is measured to be: " << testval << endl;
  
         cout << "Durations on this line: " << endl;
         for (q=0; q<lastdurations.getSize(); q++) {
            cout << "\t" << lastdurations[q]; 
         }
         cout << endl;
         cout << "Running total of durations from previous rhythm: " << endl;
         for (q=0; q<runningstatus.getSize(); q++) {
            cout << "\t" << runningstatus[q]; 
         }
         cout << endl;
         cout << "Line of data that failed:\n";
         cout << aRecord << endl;
  
         exit(1);
      }
      if (testval < min) {
         min = testval;
      }
   }

   // Step (4): add the duration to the running values and to meter position
   for (i=0; i<runningstatus.getSize(); i++) {
      runningstatus[i] += min;
   }

   return min;
}



//////////////////////////////
//
// HumdrumFile::determineDurationR -- determines the duration of the **kern
//	entries before a new **kern entry.  Also works on **koto spines.
//

RationalNumber HumdrumFile::determineDurationR(HumdrumRecord& aRecord,
      int& init, SigCollection<RationalNumber>& lastdurations, 
      SigCollection<RationalNumber>& runningstatus,
      Array<int>& rhythms, Array<int>& ignore) {
   int i;
   // initialization:
   if (init) {
      init = 0;
      int size = aRecord.getFieldCount("**kern");
      size += aRecord.getFieldCount("**koto");
      lastdurations.setSize(size);
      runningstatus.setSize(size);
      for (i=0; i<size; i++) {
         lastdurations[i].zero();
         runningstatus[i].zero();
      }
   }

   // Step (1): if lastdurations == runningstatus, then zero running
   // status.
   RationalNumber zero(0,1);
   for (i=0; i<runningstatus.getSize(); i++) {
      if ((runningstatus[i] - lastdurations[i]) == zero) {
         runningstatus[i].zero();
      } else {
      }
   }
 
   // Step (2): input new durations into the lastdurations array
   int q;
   int count = 0;
   int stype = 0;
   static char rbuff[32] = {0};
   for (i=0; i<aRecord.getFieldCount(); i++) {
      if (ignore[aRecord.getPrimaryTrack(i)-1] != 0) {
         stype = 0;
      } else if (aRecord.getExInterpNum(i) == E_KERN_EXINT) {
         stype = 1;
      } else if (strcmp(aRecord.getExInterp(i), "**koto") == 0) {
         stype = 2;
      } else {
         stype = 0;
      }
      if (stype) {
         if (strcmp(aRecord[i], ".") != 0) {
            switch (stype) {
               case 1:
                  lastdurations[count] = Convert::kernToDurationR(aRecord[i]);
                  if (strchr(aRecord[i], 'P') != NULL) {
                     // remove appogiatura durations for summations
                     // lastdurations[count] = 0;
                  } else if ((strchr(aRecord[i], 'q') != NULL) ||
                             (strchr(aRecord[i], 'Q') != NULL)) {
                     // remove gracenote durations from summations
                     lastdurations[count] = 0;
                  } 
                  break;
               case 2:
                  lastdurations[count] = Convert::kotoToDurationR(aRecord[i]);
                  if ((strchr(aRecord[i], 'q') != NULL) ||
                      (strchr(aRecord[i], 'Q') != NULL)) {
                     // remove gracenote durations from summations
                     lastdurations[count] = 0;
                  } 
                  break;
            }

            if (lastdurations[count] != 0) {
               // have a legitimate rhythm, store it in the rhythms array.
               RationalNumber sss = lastdurations[count];
               Convert::durationRToKernRhythm(rbuff, sss);
               int rbase  = atoi(rbuff);
               int length = strlen(rbuff);
               int z;
               for (z=length-1; z>0; z--) {
                  if (rbuff[z] == '.') {
                     rbase = 2 * rbase;
                  }
               }
               int done = 0;
               for (z=0; z<rhythms.getSize(); z++) {
                  if (rbase == 0) {
                     done = 1;
                     break;
                  }
                  if (rhythms[z] % rbase == 0) {
                     done = 1;
                     break;
                  }
               }
               if (!done) {
                  rhythms.append(rbase);
               }

            }

            if (strstr(aRecord[i], "--") != NULL && runningstatus[count] != 0) {
               cout << "Error in rhythm on line: " << aRecord.getLineNum()
                    << endl;
 
               cout << "Error on line: " << aRecord.getLineNum() 
                    << ": problem with rhythm in spine " << i+1 << endl;
 
               cout << "\n\t" << aRecord.getLine() << endl;
               cout << "D";
               for (q=0; q<lastdurations.getSize(); q++) {
                  cout << "\t" << lastdurations[q]; 
               }
               cout << endl;
               cout << "RT";
               for (q=0; q<runningstatus.getSize(); q++) {
                  cout << "\t" << runningstatus[q]; 
               }
               cout << endl;
               exit(1);
            }
        } else {
           // do nothing
        }
         count++;
      }
   }
   if (count != runningstatus.getSize()) {
      cerr << "Error: spine count has changed" << endl;
   }

   // Step (3): find minimum duration by subtracting last from running
   RationalNumber min(99999999,1);
   RationalNumber testval;

   for (i=0; i<lastdurations.getSize(); i++) {
      testval = lastdurations[i] - runningstatus[i];
      if (testval.isNegative()) {   
         cout << "Error on line: " << aRecord.getLineNum() 
              << ": problem with rhythm in **kern spine " 
              << i+1 << endl;
         cout << "Line min duration is measured to be: " << testval << endl;
  
         cout << "Durations on this line: " << endl;
         for (q=0; q<lastdurations.getSize(); q++) {
            cout << "\t" << lastdurations[q]; 
         }
         cout << endl;
         cout << "Running total of durations from previous rhythm: " << endl;
         for (q=0; q<runningstatus.getSize(); q++) {
            cout << "\t" << runningstatus[q]; 
         }
         cout << endl;
         cout << "Line of data that failed:\n";
         cout << aRecord << endl;
  
         exit(1);
      }
      if (testval < min) {
         min = testval;
      }
   }

   // Step (4): add the duration to the running values and to meter position
   for (i=0; i<runningstatus.getSize(); i++) {
      runningstatus[i] += min;
   }

   return min;
}



//////////////////////////////////////////////////////////////////////////
//
// other analyses that generate external data
//

//////////////////////////////
//
// HumdrumFile::analyzeDataIndex -- generate a list of the index
//     values for the data in a HumdrumFile which can then be
//     iterated through.
//     Default value: segment = 0;
//

void HumdrumFile::analyzeDataIndex(Array<int>& indices, int segment) {
   HumdrumFile& score = *this;
   indices.setSize(score.getNumLines());
   indices.setSize(0);
   indices.allowGrowth(1);

   int count = score.getSegmentCount();
   if (segment < -1 || segment >= count) {
      indices.allowGrowth(0);
      return;
   }

   int start = 0;
   int scount = 0;
   if (segment >= 0) {
      while (start < score.getNumLines()) {
         if (score[start].getType() == E_humrec_interpretation &&
              strncmp(score[start][0], "**", 2) == 0) {
            scount++;
            if (scount == segment) {
               break;
            }
         }
         start++;
      }
   }

   int i;
   for (i=start; i<score.getNumLines(); i++) {
      if (score[i].getType() == E_humrec_data) {
         indices.append(i);
      } else if (segment < 0 && 
                 score[i].getType() == E_humrec_interpretation && 
                 strcmp(score[i][0], "*-") == 0) {

         break;
      }
   }

   indices.allowGrowth(0);
}



//////////////////////////////
//
// HumdrumFile::analyzeCliche -- returns the count of cliches found
//    in the score.
//     Default values: start = -1, stop = -1;
//

int HumdrumFile::analyzeCliche(Array<int>& cliche, double duration, 
      int minimumcount, double start, double stop) {
   HumdrumFile& score = *this;
   int  nlflag = NL_SORT | NL_UNIQ;
   // later flags to add: NL_PC; NL_FILL;

   cliche.setSize(score.getNumLines());
   cliche.zero();

   Array<int> di;  // data index list for score data lines
   score.analyzeDataIndex(di);

   Array<Array<int> > pitchset;
   int starti = 0;
// cout << "di get size = " << di.getSize() << endl;

   if (start > 0) {
      while (score[di[starti]].getAbsBeat() < start && starti < di.getSize()) {
         starti++;
      }
   }

   int endi = di.getSize() - 1;
   if (stop > 0) {
      while (score[di[endi]].getAbsBeat() < stop && endi > 1) {
         endi--;
      }
   }

   int i, j;
   Array<Array<int> > allnotes;
   allnotes.setSize(endi-starti+1);
   for (i=0; i<endi-starti+1; i++) {
      score.getNoteList(allnotes[i], di[i], nlflag);
   }

   int match = 0;
   int found = 0;
   int count = 0;
   int tcount = 0;
   for (i=starti; i<endi; i++) {
      if (cliche[di[i]] != 0) {
         continue;
      }
      found = 0;
      match = 0;
      tcount = 0;
      for (j=i+1; j<=endi; j++) {
// cout << "Matching line " << di[i] << " to " << di[j] << " : "
//      << score[di[i]][0] << " \t" << score[di[j]][0] << endl;
         match = attemptMatch(allnotes, di, starti, i, j, duration);
         if (match && !found) {
            count++;
            cliche[di[i]] = -count;
            found = 1;
         }
         if (match) {
            cliche[di[j]] = count;
            tcount++;
         }
      }
      if (tcount < minimumcount) {
         for (j=i+1; j<=endi; j++) {
            if (cliche[di[j]] == -cliche[di[i]]) {
               cliche[di[j]] = 0;
            }
         }
         cliche[di[i]] = 0;
         count--;
      }
   }

   return count;
}



//////////////////////////////
//
// private: HumdrumFile::attemptMatch -- for use with analyzeCliche
//

int HumdrumFile::attemptMatch(Array<Array<int> >& allnotes, Array<int>& di, 
      int starti, int i, int j, double duration) {
   HumdrumFile& score = *this;

   double sumduration = 0.0;

   int delta = 0;
   int k;
   while (sumduration < duration) {
      if (score[di[i+delta]].getDuration() != 
            score[di[j+delta]].getDuration()) {
         return 0;
      } else {
         sumduration += duration;
      }
      
      if (allnotes[i+delta].getSize() != allnotes[j+delta].getSize()) {
         return 0;
      }

      for (k=0; k<allnotes[i+delta].getSize(); k++) {
         if (allnotes[i+delta][k] != allnotes[j+delta][k]) {
            return 0;
         }
      }
      delta++;
   }
   
   return 1;
}



//////////////////////////////
//
// HumdrumFile::analyzeTempoMarkings --
//     default value: tdefault = 60.0;
//

void HumdrumFile::analyzeTempoMarkings(Array<double>& tempo, double tdefault) {
   HumdrumFile& score = *this;
   tempo.setSize(score.getNumLines());
   tempo.zero();
 
   int i, j;
   double lasttempo = tdefault;
   for (i=0; i<score.getNumLines(); i++) {
      if (score[i].getType() != E_humrec_interpretation) {
         tempo[i] = lasttempo;
         continue;
      }

      for (j=0; j<score[i].getFieldCount(); j++) {
         if (score[i].getExInterpNum(j) != E_KERN_EXINT) {
            continue;
         }

         if (strncmp(score[i][j], "*MM", 3) == 0 && isdigit(score[i][j][3])) {
            sscanf(score[i][j], "*MM%lf", &lasttempo);
            break;
         }
      }

      tempo[i] = lasttempo;
   }
}



//////////////////////////////
//
// HumdrumFile::analyzeMeter -- extract the prevalent time signature
//   for each position in the score.
//   Default value: flag = 0;
//

void HumdrumFile::analyzeMeter(Array<double>& top, Array<double>& bottom,
      int flag) {
   int    compoundQ   = flag & (0x01<<COMPOUND_METER_BIT);
   double goodtop     = -1.0;
   double goodbottom  = -1.0;
   int    testtop     = -1;
   int    testbottom  = -1;

   HumdrumFile& score = *this;
   top.setSize(score.getNumLines());
   bottom.setSize(score.getNumLines());

   int j;
   int line;
   int count;
   for (line=0; line<score.getNumLines(); line++) {
      if (score[line].getType() == E_humrec_interpretation) {
         for (j=0; j<score[line].getFieldCount(); j++) {
            if (score[line].getExInterpNum(j) != E_KERN_EXINT) {
               continue;
            }
            if ((strncmp(score[line][j], "*M", 2) == 0) &&
                  isdigit(score[line][j][2]) &&
                  (strchr(score[line][j], '/') != NULL)) {
               count = sscanf(score[line][j], "*M%d/%d", &testtop,
                     &testbottom);
               if (goodbottom == 0.0) {
                  // cannot handle 00 (long which is 1/4 rhythm)
                  goodbottom = 0.5;
               }
               if (count != 2) {
                  continue;
               } else {          
                  goodtop = testtop;
                  goodbottom = testbottom;
               }

               if (compoundQ && (testtop % 3 == 0) && (testtop != 3)) {
                  goodtop = goodtop / 3.0;
                  goodbottom = 4.0/goodbottom * 3.0;
               } else {
                  goodbottom = 4.0/goodbottom;
               }
               break;
            }
         }
      }

      top[line] = goodtop;
      bottom[line] = goodbottom;
   }
}



//////////////////////////////
//
// HumdrumFile::analyzeBeatDuration -- determine the duration of a beat
//   given the meter
//     Default value: flag = AFLAG_COMPOUND_METER
//

void HumdrumFile::analyzeBeatDuration(Array<double>& beatdur, int flag) {
   Array<double> top;
   analyzeMeter(top, beatdur, flag);
}



//////////////////////////////
//
// HumdrumFile::analyzeAttackAccentuation -- determine the attack
//   accentuation
//

void HumdrumFile::analyzeAttackAccentuation(Array<int>& atakcent) {
   HumdrumFile& score = *this;
   int allnotes;
   int attacknotes;
   int sustainnotes;
   int analysis;
   Array<int> notes;

   atakcent.setSize(score.getNumLines());
   atakcent.setSize(0);
   atakcent.allowGrowth(1);

   for (int i=0; i<score.getNumLines(); i++) {
      if (score[i].getType() != E_humrec_data) {
         analysis = 0;      
         atakcent.append(analysis);
         continue;
      }
      score.getNoteList(notes, i, NL_NOPC | NL_FILL | NL_NOSORT |
         NL_NOUNIQ | NL_NORESTS);
      allnotes = notes.getSize();
      score.getNoteList(notes, i, NL_NOPC | NL_NOFILL | NL_NOSORT |
         NL_NOUNIQ | NL_NOTIED | NL_NORESTS);
      attacknotes = notes.getSize();
      sustainnotes = allnotes - attacknotes;
      analysis = attacknotes - sustainnotes;
      atakcent.append(analysis);
   }

   atakcent.allowGrowth(0);
}



//////////////////////////////
//
// HumdrumFile::analyzeMetricLevel -- determine the metric level of
//    the data.
//

void HumdrumFile::analyzeMetricLevel(Array<int>& metlev) {
   HumdrumFile& score = *this;
   int i;
   metlev.setSize(score.getNumLines());
   metlev.zero();

   Array<int> iscompound;
   Array<double> msigtop; 
   Array<double> msigbottom; 

   analyzeMeter(msigtop, msigbottom);
   iscompound.setSize(msigtop.getSize());
   iscompound.zero();

   int mval;
   int ltop = 0;
   int lbottom = 0;
   for (i=0; i<iscompound.getSize(); i++) {
      if ((i > 0) && (ltop == msigtop[i]) && (lbottom == msigbottom[i])) {
         iscompound[i] = iscompound[i-1];
      } else {
         mval = (int)(msigtop[i] * msigbottom[i] + 0.45);
         if ((mval % 3) == 0) {
            iscompound[i] = 1;
            // Prevent 6/4 meters from being interpreted as compound here?
         } else {
            iscompound[i] = 0;
         }
      }
   }

   if (score.rhythmQ() == 0) {
      score.analyzeRhythm();
   }

   int level = 0;
   double metloc;
   double fraction;
   double testtriplet;
   double testhigher;
   double highertriplet;
   for (i=0; i<score.getNumLines(); i++) {
      if (!score[i].isData()) {
         continue;
      }
      level = 0;
      metloc = score[i].getBeat() - 1.0;   // put beat 1 at 0
      if (iscompound[i]) {  // adjust metric position for compound meters
         metloc = metloc / 3.0;
      }
      fraction = metloc - (int)metloc;
      if (fraction < ROUNDERR || fraction > (1-ROUNDERR)) {
         // check for higher levels of metrical grouping
         if (metloc == 0.0) {
            if (iscompound[i]) {
               metlev[i] = (int)(-1.0*log10(msigtop[i]*msigbottom[i]*2.0/3.0)/log10(2.0));
            } else {
               metlev[i] = (int)(-1.0 * log10(msigtop[i]*msigbottom[i])/log10(2.0));
            }
             
            continue;
         }

         testhigher = metloc;
         while (level > -5) {
            testhigher = testhigher / 2.0;
            level--;
            fraction = testhigher - (int)testhigher;
            if (fraction < ROUNDERR || fraction > (1-ROUNDERR)) {
               metlev[i] = level;
               break;
            }
            if (iscompound[i]) {
               highertriplet = metloc * 2.0/3.0; // check for triplets on level
               fraction = highertriplet - (int)highertriplet;
               if (fraction < ROUNDERR || fraction > (1-ROUNDERR)) {
                  metlev[i] = level;
                  break;
               }
            }
         }
         
         continue;
      }

      // inside of subdivision of a beat if getting to this point
      level = 0;
      while (level < 10) {
         if (iscompound[i]) {
            metloc = metloc * 2.0; // just base 2 checking for now
            level++;
            testtriplet = metloc * 3.0/2.0; // check for triplets on level
            fraction = testtriplet - (int)testtriplet;
            if (fraction < ROUNDERR || fraction > (1-ROUNDERR)) {
               metlev[i] = level;
               break;
            }
         } else {
            metloc = metloc * 2.0; // just base 2 checking for now
            level++;
            fraction = metloc - (int)metloc;
            if (fraction < ROUNDERR || fraction > (1-ROUNDERR)) {
               metlev[i] = level;
               break;
            } else {
               testtriplet = metloc * 3.0/2.0; // check for triplets on level
               fraction = testtriplet - (int)testtriplet;
               if (fraction < ROUNDERR || fraction > (1-ROUNDERR)) {
                  metlev[i] = level;
                  break;
               }
            }
         }

      }
      if (level >= 10) {
         metlev[i] = level;
      }
   }
}



//////////////////////////////
//
// HumdrumFile::analyzeSonorityQuality -- try to find the root and
//   inversion of the given note sets.
//

void HumdrumFile::analyzeSonorityQuality(Array<ChordQuality>& cq) {
   Array<int> notes;
   HumdrumFile& score = *this;
   cq.setSize(score.getNumLines());

   int line;
   for (line=0; line<score.getNumLines(); line++) {
      score.getNoteList(notes, line, NL_NOPC | NL_FILL | NL_NOSORT |
         NL_NOUNIQ | NL_NORESTS);
      Convert::noteSetToChordQuality(cq[line], notes);
   }

   cq.allowGrowth(0);
}



//////////////////////////////
//
// HumdrumFile::analyzeSonorityRoot -- determine the roots of the
//   sonorities in the score.
//   Default value: flag = AFLAG_BASE40_PITCH
//

void HumdrumFile::analyzeSonorityRoot(Array<int>& roots, int flag) {
   int base12Q = flag & (0x01 << PITCH_BASE_BIT);
   Array<int> notes;
   HumdrumFile& score = *this;
   roots.setSize(score.getNumLines());
   ChordQuality cq;

   int line;
   for (line=0; line<score.getNumLines(); line++) {
      score.getNoteList(notes, line, NL_NOPC | NL_FILL | NL_NOSORT |
         NL_NOUNIQ | NL_NORESTS);
      Convert::noteSetToChordQuality(cq, notes);
      roots[line] = cq.getRoot();
      if (base12Q) {
         roots[line] = Convert::base40ToMidiNoteNumber(roots[line]);
      }
   }

   roots.allowGrowth(0);
}



//////////////////////////////
//
// HumdrumFile::analyzeKeyKS -- measure the key of a particular
//      region of the score using the Krumhansl-Schmuckler key-finding
//      algorithm.
//      default value: rhythmQ = 1
//      default value: binaryQ = 0
//      default value: tracknum = -1 (examine all spines of music)

int HumdrumFile::analyzeKeyKS(Array<double>& scores, int startindex, 
      int stopindex, int rhythmQ, int binaryQ, int tracknum) {

   Array<double> absbeat;
   Array<int>    pitches;
   Array<double> durations;
   Array<double> levels;
   getNoteArray(absbeat, pitches, durations, levels, startindex, stopindex,
      tracknum);

   scores.setSize(24);
   Array<double> distribution(12);

   int i;
   for (i=0; i<pitches.getSize(); i++) {
      pitches[i] = Convert::base40ToMidiNoteNumber(pitches[i]);
   }

   return ::analyzeKeyKS(scores.getBase(), distribution.getBase(), 
      pitches.getBase(), durations.getBase(), pitches.getSize(),
            rhythmQ, binaryQ);
}


int HumdrumFile::analyzeKeyKS2(Array<double>& scores, int startindex, 
      int stopindex, double* majorprofile, double* minorprofile, int rhythmQ,
      int binaryQ, int tracknum) {

   Array<double> absbeat;
   Array<int>    pitches;
   Array<double> durations;
   Array<double> levels;
   getNoteArray(absbeat, pitches, durations, levels, startindex, stopindex,
      tracknum);

   scores.setSize(24);
   Array<double> distribution(12);

   int i;
   for (i=0; i<pitches.getSize(); i++) {
      pitches[i] = Convert::base40ToMidiNoteNumber(pitches[i]);
   }

   return ::analyzeKeyKS2(scores.getBase(), distribution.getBase(), 
      pitches.getBase(), durations.getBase(), pitches.getSize(),
            rhythmQ, majorprofile, minorprofile);
}



//////////////////////////////
//
// HumdrumFile::getNormalForm --
//

void HumdrumFile::getNormalForm(Array<int>& norm, int line) {
   Array<int> base12;
   this->getBase12PitchList(base12, line);
   Convert::base12ToNormalForm(norm, base12);
}



//////////////////////////////
//
// getBase12PitchList --  Returns a list of the MIDI note numbers
//    for all pitches sounding in **kern spines on the given line.
//    Both notes which are started on the current line and ones which
//    are sustained from a previous line are included.
//

void HumdrumFile::getBase12PitchList(Array<int>& list, int line) {
   HumdrumRecord& arecord = (*this)[line];
   list.setSize(arecord.getFieldCount());
   list.setSize(0);
   int j, k;
   int ii, jj;
   int tcount;
   char buffer[128] = {0};
   int value;

   for (j=0; j<arecord.getFieldCount(); j++) {
      if (!arecord.isExInterp(j, "**kern")) {
         continue;
      }
      ii = line;
      jj = j;
      if (strcmp(arecord[j], ".") == 0) {
         ii = arecord.getDotLine(j);
         jj = arecord.getDotSpine(j);
         if (ii < 0 || jj < 0) {
            continue;
         }
      }
      if (strchr((*this)[ii][jj], 'r') != NULL) {
         continue;
      }
      tcount = (*this)[ii].getTokenCount(jj);
      for (k=0; k<tcount; k++) {
         (*this)[ii].getToken(buffer, jj, k);
         value = Convert::kernToMidiNoteNumber(buffer);
         list.append(value);
      }
   }
}



//////////////////////////////
//
// HumdrumFile::getIntervalVector --
//

void HumdrumFile::getIntervalVector(Array<int>& iv, int line) {
   Array<int> base12;
   this->getBase12PitchList(base12, line);
   Convert::base12ToIntervalVector(iv, base12);
}



//////////////////////////////
//
// HumdrumFile::getTnSetName --
//

const char* HumdrumFile::getTnSetName(int line) {
   Array<int> base12;
   this->getBase12PitchList(base12, line);
   return Convert::base12ToTnSetName(base12);
}



//////////////////////////////
//
// HumdrumFile::getTnNormalForm -- 0-transposed normal form.
//

void HumdrumFile::getTnNormalForm(Array<int>& tnorm, int line) {
   Array<int> base12;
   this->getBase12PitchList(base12, line);
   Convert::base12ToTnNormalForm(tnorm, base12);
}



//////////////////////////////
//
// HumdrumFile::getForteSetName --
//

const char* HumdrumFile::getForteSetName(int line) {
   Array<int> iv;
   Array<int> base12;
   HumdrumRecord& arecord = (*this)[line];
   base12.setSize(arecord.getFieldCount());
   base12.setSize(0);
   int j, k;
   int ii, jj;
   int tcount;
   char buffer[128] = {0};
   int value;
   for (j=0; j<arecord.getFieldCount(); j++) {
      if (!arecord.isExInterp(j, "**kern")) {
         continue;
      }
      ii = line;
      jj = j;
      if (strcmp(arecord[j], ".") == 0) {
         ii = arecord.getDotLine(j);
         jj = arecord.getDotSpine(j);
         if (ii < 0 || jj < 0) {
            continue;
         }
      }
      if (strchr((*this)[ii][jj], 'r') != NULL) {
         continue;
      }
      tcount = (*this)[ii].getTokenCount(jj);
      for (k=0; k<tcount; k++) {
         (*this)[ii].getToken(buffer, jj, k);
         value = Convert::kernToMidiNoteNumber(buffer);
         base12.append(value);
      }
   }

   if (base12.getSize() == 0) {
      return "0-0";
   } else if (base12.getSize() == 1) {
      return "1-0";
   }

   Convert::base12ToIntervalVector(iv, base12);

   switch (iv[0]) {
      case 0:
         if (iv[1]==0 && iv[2]==0 && iv[3]==0 && iv[4]==0 && iv[5]==0)	return "0-0";
         if (iv[1]==0 && iv[2]==0 && iv[3]==0 && iv[4]==0 && iv[5]==0)	return "1-1";
         if (iv[1]==0 && iv[2]==0 && iv[3]==0 && iv[4]==0 && iv[5]==1)	return "2-6";
         if (iv[1]==0 && iv[2]==0 && iv[3]==0 && iv[4]==1 && iv[5]==0)	return "2-5";
         if (iv[1]==0 && iv[2]==0 && iv[3]==1 && iv[4]==0 && iv[5]==0)	return "2-4";
         if (iv[1]==0 && iv[2]==0 && iv[3]==3 && iv[4]==0 && iv[5]==0)	return "3-12";
         if (iv[1]==0 && iv[2]==1 && iv[3]==0 && iv[4]==0 && iv[5]==0)	return "2-3";
         if (iv[1]==0 && iv[2]==1 && iv[3]==1 && iv[4]==1 && iv[5]==0)	return "3-11";
         if (iv[1]==0 && iv[2]==2 && iv[3]==0 && iv[4]==0 && iv[5]==1)	return "3-10";
         if (iv[1]==0 && iv[2]==4 && iv[3]==0 && iv[4]==0 && iv[5]==2)	return "4-28";
         if (iv[1]==1 && iv[2]==0 && iv[3]==0 && iv[4]==0 && iv[5]==0)	return "2-2";
         if (iv[1]==1 && iv[2]==0 && iv[3]==0 && iv[4]==2 && iv[5]==0)	return "3-9";
         if (iv[1]==1 && iv[2]==0 && iv[3]==1 && iv[4]==0 && iv[5]==1)	return "3-8";
         if (iv[1]==1 && iv[2]==1 && iv[3]==0 && iv[4]==1 && iv[5]==0)	return "3-7";
         if (iv[1]==1 && iv[2]==2 && iv[3]==1 && iv[4]==1 && iv[5]==1)	return "4-27";
         if (iv[1]==1 && iv[2]==2 && iv[3]==1 && iv[4]==2 && iv[5]==0)	return "4-26";
         if (iv[1]==2 && iv[2]==0 && iv[3]==1 && iv[4]==0 && iv[5]==0)	return "3-6";
         if (iv[1]==2 && iv[2]==0 && iv[3]==2 && iv[4]==0 && iv[5]==2)	return "4-25";
         if (iv[1]==2 && iv[2]==0 && iv[3]==3 && iv[4]==0 && iv[5]==1)	return "4-24";
         if (iv[1]==2 && iv[2]==1 && iv[3]==0 && iv[4]==3 && iv[5]==0)	return "4-23";
         if (iv[1]==2 && iv[2]==1 && iv[3]==1 && iv[4]==2 && iv[5]==0)	return "4-22";
         if (iv[1]==3 && iv[2]==0 && iv[3]==2 && iv[4]==0 && iv[5]==1)	return "4-21";
         if (iv[1]==3 && iv[2]==2 && iv[3]==1 && iv[4]==4 && iv[5]==0)	return "5-35";
         if (iv[1]==3 && iv[2]==2 && iv[3]==2 && iv[4]==2 && iv[5]==1)	return "5-34";
         if (iv[1]==4 && iv[2]==0 && iv[3]==4 && iv[4]==0 && iv[5]==2)	return "5-33";
         if (iv[1]==6 && iv[2]==0 && iv[3]==6 && iv[4]==0 && iv[5]==3)	return "6-35";

      case 1:

          if (iv[1]==0 && iv[2]==0 && iv[3]==0 && iv[4]==0 && iv[5]==0)	return "2-1";
          if (iv[1]==0 && iv[2]==0 && iv[3]==0 && iv[4]==1 && iv[5]==1)	return "3-5";
          if (iv[1]==0 && iv[2]==0 && iv[3]==1 && iv[4]==1 && iv[5]==0)	return "3-4";
          if (iv[1]==0 && iv[2]==1 && iv[3]==1 && iv[4]==0 && iv[5]==0)	return "3-3";
          if (iv[1]==0 && iv[2]==1 && iv[3]==2 && iv[4]==2 && iv[5]==0)	return "4-20";
          if (iv[1]==0 && iv[2]==1 && iv[3]==3 && iv[4]==1 && iv[5]==0)	return "4-19";
          if (iv[1]==0 && iv[2]==2 && iv[3]==1 && iv[4]==1 && iv[5]==1)	return "4-18";
          if (iv[1]==0 && iv[2]==2 && iv[3]==2 && iv[4]==1 && iv[5]==0)	return "4-17";
          if (iv[1]==1 && iv[2]==0 && iv[3]==1 && iv[4]==2 && iv[5]==1)	return "4-16";
          if (iv[1]==1 && iv[2]==1 && iv[3]==0 && iv[4]==0 && iv[5]==0)	return "3-2";
          if (iv[1]==1 && iv[2]==1 && iv[3]==1 && iv[4]==1 && iv[5]==1)	return "4-Z15";
          if (iv[1]==1 && iv[2]==1 && iv[3]==1 && iv[4]==1 && iv[5]==1)	return "4-Z29";
          if (iv[1]==1 && iv[2]==1 && iv[3]==1 && iv[4]==2 && iv[5]==0)	return "4-14";
          if (iv[1]==1 && iv[2]==2 && iv[3]==0 && iv[4]==1 && iv[5]==1)	return "4-13";
          if (iv[1]==1 && iv[2]==2 && iv[3]==1 && iv[4]==0 && iv[5]==1)	return "4-12";
          if (iv[1]==1 && iv[2]==3 && iv[3]==2 && iv[4]==2 && iv[5]==1)	return "5-32";
          if (iv[1]==1 && iv[2]==4 && iv[3]==1 && iv[4]==1 && iv[5]==2)	return "5-31";
          if (iv[1]==2 && iv[2]==1 && iv[3]==1 && iv[4]==1 && iv[5]==0)	return "4-11";
          if (iv[1]==2 && iv[2]==1 && iv[3]==3 && iv[4]==2 && iv[5]==1)	return "5-30";
          if (iv[1]==2 && iv[2]==2 && iv[3]==0 && iv[4]==1 && iv[5]==0)	return "4-10";
          if (iv[1]==2 && iv[2]==2 && iv[3]==1 && iv[4]==3 && iv[5]==1)	return "5-29";
          if (iv[1]==2 && iv[2]==2 && iv[3]==2 && iv[4]==1 && iv[5]==2)	return "5-28";
          if (iv[1]==2 && iv[2]==2 && iv[3]==2 && iv[4]==3 && iv[5]==0)	return "5-27";
          if (iv[1]==2 && iv[2]==2 && iv[3]==3 && iv[4]==1 && iv[5]==1)	return "5-26";
          if (iv[1]==2 && iv[2]==3 && iv[3]==1 && iv[4]==2 && iv[5]==1)	return "5-25";
          if (iv[1]==3 && iv[2]==1 && iv[3]==2 && iv[4]==2 && iv[5]==1)	return "5-24";
          if (iv[1]==3 && iv[2]==2 && iv[3]==1 && iv[4]==3 && iv[5]==0)	return "5-23";
          if (iv[1]==4 && iv[2]==2 && iv[3]==4 && iv[4]==2 && iv[5]==2)	return "6-34";
          if (iv[1]==4 && iv[2]==3 && iv[3]==2 && iv[4]==4 && iv[5]==1)	return "6-33";
          if (iv[1]==4 && iv[2]==3 && iv[3]==2 && iv[4]==5 && iv[5]==0)	return "6-32";

      case 2:

          if (iv[1]==0 && iv[2]==0 && iv[3]==0 && iv[4]==2 && iv[5]==2)	return "4-9";
          if (iv[1]==0 && iv[2]==0 && iv[3]==1 && iv[4]==2 && iv[5]==1)	return "4-8";
          if (iv[1]==0 && iv[2]==1 && iv[3]==2 && iv[4]==1 && iv[5]==0)	return "4-7";
          if (iv[1]==0 && iv[2]==2 && iv[3]==3 && iv[4]==2 && iv[5]==1)	return "5-22";
          if (iv[1]==0 && iv[2]==2 && iv[3]==4 && iv[4]==2 && iv[5]==0)	return "5-21";
          if (iv[1]==1 && iv[2]==0 && iv[3]==0 && iv[4]==0 && iv[5]==0)	return "3-1";
          if (iv[1]==1 && iv[2]==0 && iv[3]==0 && iv[4]==2 && iv[5]==1)	return "4-6";
          if (iv[1]==1 && iv[2]==0 && iv[3]==1 && iv[4]==1 && iv[5]==1)	return "4-5";
          if (iv[1]==1 && iv[2]==1 && iv[3]==1 && iv[4]==1 && iv[5]==0)	return "4-4";
          if (iv[1]==1 && iv[2]==1 && iv[3]==2 && iv[4]==3 && iv[5]==1)	return "5-20";
          if (iv[1]==1 && iv[2]==2 && iv[3]==1 && iv[4]==0 && iv[5]==0)	return "4-3";
          if (iv[1]==1 && iv[2]==2 && iv[3]==1 && iv[4]==2 && iv[5]==2)	return "5-19";
          if (iv[1]==1 && iv[2]==2 && iv[3]==2 && iv[4]==2 && iv[5]==1)	return "5-Z18";
          if (iv[1]==1 && iv[2]==2 && iv[3]==2 && iv[4]==2 && iv[5]==1)	return "5-Z38";
          if (iv[1]==1 && iv[2]==2 && iv[3]==3 && iv[4]==2 && iv[5]==0)	return "5-Z17";
          if (iv[1]==1 && iv[2]==2 && iv[3]==3 && iv[4]==2 && iv[5]==0)	return "5-Z37";
          if (iv[1]==1 && iv[2]==3 && iv[3]==2 && iv[4]==1 && iv[5]==1)	return "5-16";
          if (iv[1]==2 && iv[2]==0 && iv[3]==2 && iv[4]==2 && iv[5]==2)	return "5-15";
          if (iv[1]==2 && iv[2]==1 && iv[3]==1 && iv[4]==0 && iv[5]==0)	return "4-2";
          if (iv[1]==2 && iv[2]==1 && iv[3]==1 && iv[4]==3 && iv[5]==1)	return "5-14";
          if (iv[1]==2 && iv[2]==1 && iv[3]==3 && iv[4]==1 && iv[5]==1)	return "5-13";
          if (iv[1]==2 && iv[2]==2 && iv[3]==1 && iv[4]==2 && iv[5]==1)	return "5-Z12";
          if (iv[1]==2 && iv[2]==2 && iv[3]==1 && iv[4]==2 && iv[5]==1)	return "5-Z36";
          if (iv[1]==2 && iv[2]==2 && iv[3]==2 && iv[4]==2 && iv[5]==0)	return "5-11";
          if (iv[1]==2 && iv[2]==3 && iv[3]==1 && iv[4]==1 && iv[5]==1)	return "5-10";
          if (iv[1]==2 && iv[2]==3 && iv[3]==4 && iv[4]==3 && iv[5]==1)	return "6-31";
          if (iv[1]==2 && iv[2]==4 && iv[3]==2 && iv[4]==2 && iv[5]==3)	return "6-30";
          if (iv[1]==2 && iv[2]==4 && iv[3]==2 && iv[4]==3 && iv[5]==2)	return "6-Z29";
          if (iv[1]==2 && iv[2]==4 && iv[3]==2 && iv[4]==3 && iv[5]==2)	return "6-Z50";
          if (iv[1]==2 && iv[2]==4 && iv[3]==3 && iv[4]==2 && iv[5]==2)	return "6-Z28";
          if (iv[1]==2 && iv[2]==4 && iv[3]==3 && iv[4]==2 && iv[5]==2)	return "6-Z49";
          if (iv[1]==2 && iv[2]==5 && iv[3]==2 && iv[4]==2 && iv[5]==2)	return "6-27";
          if (iv[1]==3 && iv[2]==1 && iv[3]==2 && iv[4]==1 && iv[5]==1)	return "5-9";
          if (iv[1]==3 && iv[2]==2 && iv[3]==2 && iv[4]==0 && iv[5]==1)	return "5-8";
          if (iv[1]==3 && iv[2]==2 && iv[3]==3 && iv[4]==4 && iv[5]==1)	return "6-Z26";
          if (iv[1]==3 && iv[2]==2 && iv[3]==3 && iv[4]==4 && iv[5]==1)	return "6-Z48";
          if (iv[1]==3 && iv[2]==3 && iv[3]==2 && iv[4]==4 && iv[5]==1)	return "6-Z25";
          if (iv[1]==3 && iv[2]==3 && iv[3]==2 && iv[4]==4 && iv[5]==1)	return "6-Z47";
          if (iv[1]==3 && iv[2]==3 && iv[3]==3 && iv[4]==3 && iv[5]==1)	return "6-Z24";
          if (iv[1]==3 && iv[2]==3 && iv[3]==3 && iv[4]==3 && iv[5]==1)	return "6-Z46";
          if (iv[1]==3 && iv[2]==4 && iv[3]==2 && iv[4]==2 && iv[5]==2)	return "6-Z23";
          if (iv[1]==3 && iv[2]==4 && iv[3]==2 && iv[4]==2 && iv[5]==2)	return "6-Z45";
          if (iv[1]==4 && iv[2]==1 && iv[3]==4 && iv[4]==2 && iv[5]==2)	return "6-22";
          if (iv[1]==4 && iv[2]==2 && iv[3]==4 && iv[4]==1 && iv[5]==2)	return "6-21";
          if (iv[1]==5 && iv[2]==4 && iv[3]==3 && iv[4]==6 && iv[5]==1)	return "7-35";
          if (iv[1]==5 && iv[2]==4 && iv[3]==4 && iv[4]==4 && iv[5]==2)	return "7-34";
          if (iv[1]==6 && iv[2]==2 && iv[3]==6 && iv[4]==2 && iv[5]==3)	return "7-33";

      case 3:

          if (iv[1]==0 && iv[2]==3 && iv[3]==6 && iv[4]==3 && iv[5]==0)	return "6-20";
          if (iv[1]==1 && iv[2]==0 && iv[3]==1 && iv[4]==3 && iv[5]==2)	return "5-7";
          if (iv[1]==1 && iv[2]==1 && iv[3]==2 && iv[4]==2 && iv[5]==1)	return "5-6";
          if (iv[1]==1 && iv[2]==3 && iv[3]==4 && iv[4]==3 && iv[5]==1)	return "6-Z19";
          if (iv[1]==1 && iv[2]==3 && iv[3]==4 && iv[4]==3 && iv[5]==1)	return "6-Z44";
          if (iv[1]==2 && iv[2]==1 && iv[3]==0 && iv[4]==0 && iv[5]==0)	return "4-1";
          if (iv[1]==2 && iv[2]==1 && iv[3]==1 && iv[4]==2 && iv[5]==1)	return "5-5";
          if (iv[1]==2 && iv[2]==2 && iv[3]==1 && iv[4]==1 && iv[5]==1)	return "5-4";
          if (iv[1]==2 && iv[2]==2 && iv[3]==2 && iv[4]==1 && iv[5]==0)	return "5-3";
          if (iv[1]==2 && iv[2]==2 && iv[3]==2 && iv[4]==4 && iv[5]==2)	return "6-18";
          if (iv[1]==2 && iv[2]==2 && iv[3]==3 && iv[4]==3 && iv[5]==2)	return "6-Z17";
          if (iv[1]==2 && iv[2]==2 && iv[3]==3 && iv[4]==3 && iv[5]==2)	return "6-Z43";
          if (iv[1]==2 && iv[2]==2 && iv[3]==4 && iv[4]==3 && iv[5]==1)	return "6-16";
          if (iv[1]==2 && iv[2]==3 && iv[3]==4 && iv[4]==2 && iv[5]==1)	return "6-15";
          if (iv[1]==2 && iv[2]==3 && iv[3]==4 && iv[4]==3 && iv[5]==0)	return "6-14";
          if (iv[1]==2 && iv[2]==4 && iv[3]==2 && iv[4]==2 && iv[5]==2)	return "6-Z13";
          if (iv[1]==2 && iv[2]==4 && iv[3]==2 && iv[4]==2 && iv[5]==2)	return "6-Z42";
          if (iv[1]==3 && iv[2]==2 && iv[3]==1 && iv[4]==1 && iv[5]==0)	return "5-2";
          if (iv[1]==3 && iv[2]==2 && iv[3]==2 && iv[4]==3 && iv[5]==2)	return "6-Z12";
          if (iv[1]==3 && iv[2]==2 && iv[3]==2 && iv[4]==3 && iv[5]==2)	return "6-Z41";
          if (iv[1]==3 && iv[2]==3 && iv[3]==2 && iv[4]==3 && iv[5]==1)	return "6-Z11";
          if (iv[1]==3 && iv[2]==3 && iv[3]==2 && iv[4]==3 && iv[5]==1)	return "6-Z40";
          if (iv[1]==3 && iv[2]==3 && iv[3]==3 && iv[4]==2 && iv[5]==1)	return "6-Z10";
          if (iv[1]==3 && iv[2]==3 && iv[3]==3 && iv[4]==2 && iv[5]==1)	return "6-Z39";
          if (iv[1]==3 && iv[2]==5 && iv[3]==4 && iv[4]==4 && iv[5]==2)	return "7-32";
          if (iv[1]==3 && iv[2]==6 && iv[3]==3 && iv[4]==3 && iv[5]==3)	return "7-31";
          if (iv[1]==4 && iv[2]==2 && iv[3]==2 && iv[4]==3 && iv[5]==1)	return "6-9";
          if (iv[1]==4 && iv[2]==3 && iv[3]==2 && iv[4]==3 && iv[5]==0)	return "6-8";
          if (iv[1]==4 && iv[2]==3 && iv[3]==5 && iv[4]==4 && iv[5]==2)	return "7-30";
          if (iv[1]==4 && iv[2]==4 && iv[3]==3 && iv[4]==5 && iv[5]==2)	return "7-29";
          if (iv[1]==4 && iv[2]==4 && iv[3]==4 && iv[4]==3 && iv[5]==3)	return "7-28";
          if (iv[1]==4 && iv[2]==4 && iv[3]==4 && iv[4]==5 && iv[5]==1)	return "7-27";
          if (iv[1]==4 && iv[2]==4 && iv[3]==5 && iv[4]==3 && iv[5]==2)	return "7-26";
          if (iv[1]==4 && iv[2]==5 && iv[3]==3 && iv[4]==4 && iv[5]==2)	return "7-25";
          if (iv[1]==5 && iv[2]==3 && iv[3]==4 && iv[4]==4 && iv[5]==2)	return "7-24";
          if (iv[1]==5 && iv[2]==4 && iv[3]==3 && iv[4]==5 && iv[5]==1)	return "7-23";

      case 4:

          if (iv[1]==2 && iv[2]==0 && iv[3]==2 && iv[4]==4 && iv[5]==3)	return "6-7";
          if (iv[1]==2 && iv[2]==1 && iv[3]==2 && iv[4]==4 && iv[5]==2)	return "6-Z38";
          if (iv[1]==2 && iv[2]==1 && iv[3]==2 && iv[4]==4 && iv[5]==2)	return "6-Z6";
          if (iv[1]==2 && iv[2]==2 && iv[3]==2 && iv[4]==3 && iv[5]==2)	return "6-5";
          if (iv[1]==2 && iv[2]==4 && iv[3]==5 && iv[4]==4 && iv[5]==2)	return "7-22";
          if (iv[1]==2 && iv[2]==4 && iv[3]==6 && iv[4]==4 && iv[5]==1)	return "7-21";
          if (iv[1]==3 && iv[2]==2 && iv[3]==1 && iv[4]==0 && iv[5]==0)	return "5-1";
          if (iv[1]==3 && iv[2]==2 && iv[3]==3 && iv[4]==2 && iv[5]==1)	return "6-Z37";
          if (iv[1]==3 && iv[2]==2 && iv[3]==3 && iv[4]==2 && iv[5]==1)	return "6-Z4";
          if (iv[1]==3 && iv[2]==3 && iv[3]==2 && iv[4]==2 && iv[5]==1)	return "6-Z3";
          if (iv[1]==3 && iv[2]==3 && iv[3]==2 && iv[4]==2 && iv[5]==1)	return "6-Z36";
          if (iv[1]==3 && iv[2]==3 && iv[3]==4 && iv[4]==5 && iv[5]==2)	return "7-20";
          if (iv[1]==3 && iv[2]==4 && iv[3]==3 && iv[4]==4 && iv[5]==3)	return "7-19";
          if (iv[1]==3 && iv[2]==4 && iv[3]==4 && iv[4]==4 && iv[5]==2)	return "7-Z18";
          if (iv[1]==3 && iv[2]==4 && iv[3]==4 && iv[4]==4 && iv[5]==2)	return "7-Z38";
          if (iv[1]==3 && iv[2]==4 && iv[3]==5 && iv[4]==4 && iv[5]==1)	return "7-Z17";
          if (iv[1]==3 && iv[2]==4 && iv[3]==5 && iv[4]==4 && iv[5]==1)	return "7-Z37";
          if (iv[1]==3 && iv[2]==5 && iv[3]==4 && iv[4]==3 && iv[5]==2)	return "7-16";
          if (iv[1]==4 && iv[2]==2 && iv[3]==4 && iv[4]==4 && iv[5]==3)	return "7-15";
          if (iv[1]==4 && iv[2]==3 && iv[3]==2 && iv[4]==1 && iv[5]==1)	return "6-2";
          if (iv[1]==4 && iv[2]==3 && iv[3]==3 && iv[4]==5 && iv[5]==2)	return "7-14";
          if (iv[1]==4 && iv[2]==3 && iv[3]==5 && iv[4]==3 && iv[5]==2)	return "7-13";
          if (iv[1]==4 && iv[2]==4 && iv[3]==3 && iv[4]==4 && iv[5]==2)	return "7-Z12";
          if (iv[1]==4 && iv[2]==4 && iv[3]==3 && iv[4]==4 && iv[5]==2)	return "7-Z36";
          if (iv[1]==4 && iv[2]==4 && iv[3]==4 && iv[4]==4 && iv[5]==1)	return "7-11";
          if (iv[1]==4 && iv[2]==5 && iv[3]==3 && iv[4]==3 && iv[5]==2)	return "7-10";
          if (iv[1]==4 && iv[2]==8 && iv[3]==4 && iv[4]==4 && iv[5]==4)	return "8-28";
          if (iv[1]==5 && iv[2]==3 && iv[3]==4 && iv[4]==3 && iv[5]==2)	return "7-9";
          if (iv[1]==5 && iv[2]==4 && iv[3]==4 && iv[4]==2 && iv[5]==2)	return "7-8";
          if (iv[1]==5 && iv[2]==6 && iv[3]==5 && iv[4]==5 && iv[5]==3)	return "8-27";
          if (iv[1]==5 && iv[2]==6 && iv[3]==5 && iv[4]==6 && iv[5]==2)	return "8-26";
          if (iv[1]==6 && iv[2]==4 && iv[3]==6 && iv[4]==4 && iv[5]==4)	return "8-25";
          if (iv[1]==6 && iv[2]==4 && iv[3]==7 && iv[4]==4 && iv[5]==3)	return "8-24";
          if (iv[1]==6 && iv[2]==5 && iv[3]==4 && iv[4]==7 && iv[5]==2)	return "8-23";
          if (iv[1]==6 && iv[2]==5 && iv[3]==5 && iv[4]==6 && iv[5]==2)	return "8-22";
          if (iv[1]==7 && iv[2]==4 && iv[3]==6 && iv[4]==4 && iv[5]==3)	return "8-21";

      case 5:

          if (iv[1]==3 && iv[2]==2 && iv[3]==3 && iv[4]==5 && iv[5]==3)	return "7-7";
          if (iv[1]==3 && iv[2]==3 && iv[3]==4 && iv[4]==4 && iv[5]==2)	return "7-6";
          if (iv[1]==4 && iv[2]==3 && iv[3]==2 && iv[4]==1 && iv[5]==0)	return "6-1";
          if (iv[1]==4 && iv[2]==3 && iv[3]==3 && iv[4]==4 && iv[5]==2)	return "7-5";
          if (iv[1]==4 && iv[2]==4 && iv[3]==3 && iv[4]==3 && iv[5]==2)	return "7-4";
          if (iv[1]==4 && iv[2]==4 && iv[3]==4 && iv[4]==3 && iv[5]==1)	return "7-3";
          if (iv[1]==4 && iv[2]==5 && iv[3]==6 && iv[4]==6 && iv[5]==2)	return "8-20";
          if (iv[1]==4 && iv[2]==5 && iv[3]==7 && iv[4]==5 && iv[5]==2)	return "8-19";
          if (iv[1]==4 && iv[2]==6 && iv[3]==5 && iv[4]==5 && iv[5]==3)	return "8-18";
          if (iv[1]==4 && iv[2]==6 && iv[3]==6 && iv[4]==5 && iv[5]==2)	return "8-17";
          if (iv[1]==5 && iv[2]==4 && iv[3]==3 && iv[4]==3 && iv[5]==1)	return "7-2";
          if (iv[1]==5 && iv[2]==4 && iv[3]==5 && iv[4]==6 && iv[5]==3)	return "8-16";
          if (iv[1]==5 && iv[2]==5 && iv[3]==5 && iv[4]==5 && iv[5]==3)	return "8-Z15";
          if (iv[1]==5 && iv[2]==5 && iv[3]==5 && iv[4]==5 && iv[5]==3)	return "8-Z29";
          if (iv[1]==5 && iv[2]==5 && iv[3]==5 && iv[4]==6 && iv[5]==2)	return "8-14";
          if (iv[1]==5 && iv[2]==6 && iv[3]==4 && iv[4]==5 && iv[5]==3)	return "8-13";
          if (iv[1]==5 && iv[2]==6 && iv[3]==5 && iv[4]==4 && iv[5]==3)	return "8-12";
          if (iv[1]==6 && iv[2]==5 && iv[3]==5 && iv[4]==5 && iv[5]==2)	return "8-11";
          if (iv[1]==6 && iv[2]==6 && iv[3]==4 && iv[4]==5 && iv[5]==2)	return "8-10";

      case 6:

          if (iv[1]==4 && iv[2]==4 && iv[3]==4 && iv[4]==6 && iv[5]==4)	return "8-9";
          if (iv[1]==4 && iv[2]==4 && iv[3]==5 && iv[4]==6 && iv[5]==3)	return "8-8";
          if (iv[1]==4 && iv[2]==5 && iv[3]==6 && iv[4]==5 && iv[5]==2)	return "8-7";
          if (iv[1]==5 && iv[2]==4 && iv[3]==3 && iv[4]==2 && iv[5]==1)	return "7-1";
          if (iv[1]==5 && iv[2]==4 && iv[3]==4 && iv[4]==6 && iv[5]==3)	return "8-6";
          if (iv[1]==5 && iv[2]==4 && iv[3]==5 && iv[4]==5 && iv[5]==3)	return "8-5";
          if (iv[1]==5 && iv[2]==5 && iv[3]==5 && iv[4]==5 && iv[5]==2)	return "8-4";
          if (iv[1]==5 && iv[2]==6 && iv[3]==5 && iv[4]==4 && iv[5]==2)	return "8-3";
          if (iv[1]==6 && iv[2]==5 && iv[3]==5 && iv[4]==4 && iv[5]==2)	return "8-2";
          if (iv[1]==6 && iv[2]==6 && iv[3]==9 && iv[4]==6 && iv[5]==3)	return "9-12";
          if (iv[1]==6 && iv[2]==7 && iv[3]==7 && iv[4]==7 && iv[5]==3)	return "9-11";
          if (iv[1]==6 && iv[2]==8 && iv[3]==6 && iv[4]==6 && iv[5]==4)	return "9-10";
          if (iv[1]==7 && iv[2]==6 && iv[3]==6 && iv[4]==8 && iv[5]==3)	return "9-9";
          if (iv[1]==7 && iv[2]==6 && iv[3]==7 && iv[4]==6 && iv[5]==4)	return "9-8";
          if (iv[1]==7 && iv[2]==7 && iv[3]==6 && iv[4]==7 && iv[5]==3)	return "9-7";
          if (iv[1]==8 && iv[2]==6 && iv[3]==7 && iv[4]==6 && iv[5]==3)	return "9-6";

      case 7:

          if (iv[1]==6 && iv[2]==5 && iv[3]==4 && iv[4]==4 && iv[5]==2)	return "8-1";
          if (iv[1]==6 && iv[2]==6 && iv[3]==6 && iv[4]==7 && iv[5]==4)	return "9-5";
          if (iv[1]==6 && iv[2]==6 && iv[3]==7 && iv[4]==7 && iv[5]==3)	return "9-4";
          if (iv[1]==6 && iv[2]==7 && iv[3]==7 && iv[4]==6 && iv[5]==3)	return "9-3";
          if (iv[1]==7 && iv[2]==7 && iv[3]==6 && iv[4]==6 && iv[5]==3)	return "9-2";

      case 8:

          if (iv[1]==7 && iv[2]==6 && iv[3]==6 && iv[4]==6 && iv[5]==3)	return "9-1";
          if (iv[1]==8 && iv[2]==8 && iv[3]==8 && iv[4]==8 && iv[5]==9)	return "10-6";
          if (iv[1]==8 && iv[2]==8 && iv[3]==8 && iv[4]==9 && iv[5]==8)	return "10-5";
          if (iv[1]==8 && iv[2]==8 && iv[3]==9 && iv[4]==8 && iv[5]==8)	return "10-4";
          if (iv[1]==8 && iv[2]==9 && iv[3]==8 && iv[4]==8 && iv[5]==8)	return "10-3";
          if (iv[1]==9 && iv[2]==8 && iv[3]==8 && iv[4]==8 && iv[5]==8)	return "10-2";

      case 9:
          if (iv[1]==8 && iv[2]==8 && iv[3]==8 && iv[4]==8 && iv[5]==8)	return "10-1";

      case 10:
          if (iv[1]==10 && iv[2]==10 && iv[3]==10 && iv[4]==10 && iv[5]==10)	return "11-1";

      case 12:
          if (iv[1]==12 && iv[2]==12 && iv[3]==12 && iv[4]==12 && iv[5]==12)	return "12-1";

   }

   return "unknown";
}



//////////////////////////////////////////////////////////////////////////
//
// other private functions
//

//////////////////////////////
//
// intcompare -- compare two integers for ordering
//

int HumdrumFile::intcompare(const void* a, const void* b) {
   if (*((int*)a) < *((int*)b)) {
      return -1;
   } else if (*((int*)a) > *((int*)b)) {
      return 1;
   } else {
      return 0;
   }
}


//////////////////////////////
//
// HumdrumFile::printConstantTokenFields --
//

ostream& HumdrumFile::printConstantTokenFields(ostream& out, 
      HumdrumRecord& aRecord, const char* token) {
   int i;
   for (i=0; i<aRecord.getFieldCount(); i++) {
      out << token;
      if (i < aRecord.getFieldCount() - 1) {
         out << "\t";
      }
   }
   return out;
}



// md5sum: 3997e8805220a29acf9afd717bf8dd20 HumdrumFile.cpp [20001204]
