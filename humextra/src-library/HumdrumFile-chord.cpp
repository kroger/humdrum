//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May 14 12:26:45 PDT 2001
// Last Modified: Tue May 15 11:23:21 PDT 2001
// Last Modified: Sun Mar 24 12:10:00 PST 2002 (small changes for visual c++)
// Filename:      ...sig/src/sigInfo/HumdrumFile-chord.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/HumdrumFile.cpp
// Syntax:        C++ 
//
// Description:   Chord analysis functions for the HumdrumFile class.
//

#include "HumdrumFile.h"
#include "humdrumfileextras.h"
#include "Convert.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifndef OLDCPP
   #include <fstream>
   #include <iostream>
   using namespace std;
#else
   #include <fstream.h>
   #include <iostream.h>
#endif
   


//////////////////////////////
//
// HumdrumFile::measureChordRoot -- measures the chord root
//     of a region of music and returns the most likely root
//     and fills an array of scores for each possible root.
//

int HumdrumFile::measureChordRoot(Array<double>& scores,
      Array<double>& parameters, int startindex, int stopindex, int
      algorithmno, int debug) {

   NoteListArray notelist;
   generateNoteList(notelist, startindex, stopindex);
 
   if (debug) {
      cout << "START INDEX = " << startindex
           << "\tSTOP INDEX = " << stopindex << "\n";
      for (int i=0; i<notelist.getSize(); i++) {
         cout << "i=" << i << "\t" << notelist[i] << "\n";
      }
   }

   int output = 0;

   switch (algorithmno) {
      case 0: output = measureChordRoot0(scores, parameters, notelist); break;
      case 1: output = measureChordRoot1(scores, parameters, notelist); break;
      case 2: output = measureChordRoot2(scores, parameters, notelist); break;
      case 3: output = measureChordRoot3(scores, parameters, notelist); break;
      case 4: output = measureChordRoot4(scores, parameters, notelist); break;
      case 5: output = measureChordRoot5(scores, parameters, notelist); break;
      case 6: output = measureChordRoot6(scores, parameters, notelist); break;
      case 7: output = measureChordRoot7(scores, parameters, notelist); break;
      case 8: output = measureChordRoot8(scores, parameters, notelist); break;
      case 9: output = measureChordRoot9(scores, parameters, notelist); break;
   }

   if (debug) {
      cout << "Root is: " << output << endl;
   }
   
   return output;
}


int HumdrumFile::measureChordRoot(Array<double>& scores,
      Array<double>& parameters, double startbeat, double stopbeat, 
      int algorithmno, int debug) {

   int start = getStartIndex(startbeat);
   int stop = getStopIndex(stopbeat);

// cout << "STARTBEAT = " << startbeat << "\tSTOPBEAT = " << stopbeat << endl;

   return measureChordRoot(scores, parameters, start, stop, algorithmno, debug);
}



//////////////////////////////
//
// static HumdrumFile::measureChordRoot0 -- starting algorithm
//

int HumdrumFile::measureChordRoot0(Array<double>& scores, 
      Array<double>& parameters, NoteListArray& notelist) {
   return measureChordRoot1(scores, parameters, notelist);
}



//////////////////////////////
//
// static HumdrumFile::measureChordRoot1 -- switching sign in
//      algorithm 0
//

int HumdrumFile::measureChordRoot1(Array<double>& scores, 
      Array<double>& parameters, NoteListArray& notelist) {

   int vx[40] = {0, 7, 7, 1000, 4, 4, 4, 4, 4, 1000, 8, 1, 1, 8, 8, 5, 5,
		5, 5, 5, 1000, 9, 2, 2, 2, 9, 1000, 6, 6, 6, 6, 6, 1000, 3,
		3, 3, 3, 10, 7, 7};
   int vy[40] = {0, -2, -4, 1000, 4, 2, 0, -2, -4, 1000, 3, 1, 1, -3, -5, 5,
		3, 1, -1, -3, 1000, 4, 2, 0, -2, -4, 1000, 4, 2, 0, -2, -4,
		1000, 3, 1, -1, -3, -5, 4, 2};

   double& alpha  = parameters[0];
   double& delta  = parameters[1];
   double& lambda = parameters[2];
   scores.setSize(40);
   scores.zero();

   int i, j;
   int p;
   int count = notelist.getSize();
   double asum, bsum;
   double I;
   int max = 0;

// cout << "Notelist size is: " << notelist.getSize() << endl;
// cout << "Scores size is: " << scores.getSize() << endl;

   for (i=0; i<40; i++) {
      asum = bsum = 0.0;
      for (j=0; j<count; j++) {
         p = (notelist[j].getPitch() - i - 2 + 40) % 40;
         I = sqrt(alpha * alpha * vx[p] * vx[p] + vy[p] * vy[p]);
         asum += I * (delta + log(notelist[j].getDur())/log(2.0));
         bsum += I * (lambda + log(notelist[j].getLevel())/log(2.0));
      }
      scores[i] = sqrt(asum * asum + bsum * bsum)/count;
      if (scores[i] < scores[max]) {
         max = i;
      }
   }

   // count the number of major and minor third pitch classes
   // above the identified tonic.  If there are more M3, then 
   // decide that the key is major, otherwise it is minor.
   double M3count = 0.0;
   double m3count = 0.0;
   int majorthird = (max + 6) % 40;
   int minorthird = (max + 5) % 40;
   for (i=0; i<count; i++) {
      if (((notelist[i].getPitch() - max - 2 + 80) % 40) == majorthird) {
         M3count += notelist[i].getDur();
      } else if (((notelist[i].getPitch() - max - 2 + 80) % 40) == minorthird) {
         m3count += notelist[i].getDur();
      }
   }

   // added for distinguishing between major and minor:
   if (m3count > M3count) {
      // indicate minor key by putting up the tonic an octave.
      max = max + 40;
   }

   if (scores[max % 40] == 0.0) {
      return -1;
   } else {
      return max;
   }
}



//////////////////////////////
//
// static HumdrumFile::measureChordRoot2 -- Mix duration
//    and metric level by addition
//

int HumdrumFile::measureChordRoot2(Array<double>& scores, 
      Array<double>& parameters, NoteListArray& notelist) {

   int vx[40] = {0, 7, 7, 1000, 4, 4, 4, 4, 4, 1000, 8, 1, 1, 8, 8, 5, 5,
		5, 5, 5, 1000, 9, 2, 2, 2, 9, 1000, 6, 6, 6, 6, 6, 1000, 3,
		3, 3, 3, 10, 7, 7};
   int vy[40] = {0, -2, -4, 1000, 4, 2, 0, -2, -4, 1000, 3, 1, 1, -3, -5, 5,
		3, 1, -1, -3, 1000, 4, 2, 0, -2, -4, 1000, 4, 2, 0, -2, -4,
		1000, 3, 1, -1, -3, -5, 4, 2};

   double& alpha   = parameters[0];
   double& delta   = parameters[1];
   double& lambda  = parameters[2];
   scores.setSize(40);
   scores.zero();

   int i, j;
   int p;
   int count = notelist.getSize();
   double asum, bsum;
   double I;
   int max = 0;

   double offset = 0.0;
   double testv;
   for (i=0; i<notelist.getSize(); i++) {
      testv = -log(notelist[i].getDur())/log(2.0);
      if (testv > offset) {
         offset = testv;
      }
      testv = -log(notelist[i].getLevel())/log(2.0);
      if (testv > offset) {
         offset = testv;
      }
   }
   offset = offset * 2;

// cout << "Notelist size is: " << notelist.getSize() << endl;
// cout << "Scores size is: " << scores.getSize() << endl;
   for (i=0; i<40; i++) {
      asum = bsum = 0.0;
      for (j=0; j<count; j++) {
         p = (notelist[j].getPitch() - 2 - i + 40) % 40;
         I = sqrt(alpha * alpha * vx[p] * vx[p] + vy[p] * vy[p]);
         asum += I * delta * (offset + log(notelist[j].getDur())/log(2.0));
         bsum += I * lambda * (offset + log(notelist[j].getLevel())/log(2.0));
      }
      scores[i] = (asum + bsum)/count;
      if (scores[i] < scores[max]) {
         max = i;
      }
   }

   if (scores[max] == 0.0) {
      return -1;
   } else {
      return max;
   }

   return 0;
}



//////////////////////////////
//
// static HumdrumFile::measureChordRoot3 -- 
//

int HumdrumFile::measureChordRoot3(Array<double>& scores, 
      Array<double>& parameters, NoteListArray& notelist) {

   return 0;
}



//////////////////////////////
//
// static HumdrumFile::measureChordRoot4 -- 
//

int HumdrumFile::measureChordRoot4(Array<double>& scores, 
      Array<double>& parameters, NoteListArray& notelist) {

   return 0;
}



//////////////////////////////
//
// static HumdrumFile::measureChordRoot5 -- 
//

int HumdrumFile::measureChordRoot5(Array<double>& scores, 
      Array<double>& parameters, NoteListArray& notelist) {

   return 0;
}



//////////////////////////////
//
// static HumdrumFile::measureChordRoot6 -- 
//

int HumdrumFile::measureChordRoot6(Array<double>& scores, 
      Array<double>& parameters, NoteListArray& notelist) {

   return 0;
}



//////////////////////////////
//
// static HumdrumFile::measureChordRoot7 -- 
//

int HumdrumFile::measureChordRoot7(Array<double>& scores, 
      Array<double>& parameters, NoteListArray& notelist) {

   return 0;
}



//////////////////////////////
//
// static HumdrumFile::measureChordRoot8 -- 
//

int HumdrumFile::measureChordRoot8(Array<double>& scores, 
      Array<double>& parameters, NoteListArray& notelist) {

   return 0;
}



//////////////////////////////
//
// static HumdrumFile::measureChordRoot9 -- 
//

int HumdrumFile::measureChordRoot9(Array<double>& scores, 
      Array<double>& parameters, NoteListArray& notelist) {

   return 0;
}



//////////////////////////////
//
// HumdrumFile::generateNoteList -- 
//     default values:  startLine = 0; endline = 0;
//

void HumdrumFile::generateNoteList(NoteListArray& notelist, 
      int startLine, int endLine) {

   HumdrumFile& score = *this;

// cout << "start index: " << startLine << "\tstopindex: " << endLine << endl;
// int z;
// for (z=startLine; z<=endLine; z++) {
// cout << "\t" << score[z] << endl;
// }

   if (endLine <= 0) {
      endLine = score.getNumLines() - 1;
   }
   if (startLine > endLine) {
      int temp = endLine;
      endLine = startLine;
      startLine = temp;
   }

   // estimate the largest amount necessary:
   notelist.setSize(score.getNumLines() * score.getMaxTracks() * 10);
   notelist.setGrowth(score.getNumLines());
   notelist.setSize(0);
   notelist.allowGrowth(1);

   NoteList currentlist;
   currentlist.clear();

   Array<int> scorelevels;
   score.analyzeMetricLevel(scorelevels);
   
   int firsttime = 1;
   int i, j, k;
   int ii, jj;
   int ccount;
   static char buffer[1024] = {0};
   int pitch;
   int token;
   int spine;
   double beatvalue;
   double track;
   double duration;
   double firstdur;
   for (i=startLine; i<=endLine; i++) {
      if (score[i].getType() != E_humrec_data) {
         // ignore non-note data lines
         continue;
      }
      beatvalue = score.getAbsBeat(i);
      ii = i;
      for (j=0; j<score[i].getFieldCount(); j++) {
         spine = j;
         if (score[i].getExInterpNum(j) != E_KERN_EXINT) {
            // ignore non-kern data spines
            continue;
         }
         if (firsttime && strcmp(score[i][j], ".") == 0) {
            // extract the held over note from a previous point in the score
            ii = score[i].getDotLine(j);
            jj = score[i].getDotSpine(j);
         } else {
            ii = i;
            jj = j;
         }
 
         if (strcmp(score[ii][jj], ".") != 0) {
            // extract all notes in the region of interest, ignoring
            // tied notes.
            ccount = score[ii].getTokenCount(jj);
            track = score[ii].getTrack(jj);
            for (k=0; k<ccount; k++) {
               token = k;
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
// cout << "read pitch: " << pitch % 40 << endl;
               if (pitch < 0) {
                  // ignore rests
                  continue;
               }
               duration = score.getTiedDuration(ii, jj, k);
               firstdur = Convert::kernToDuration(score[ii].getToken(buffer, jj, k));
               if (duration == 0.0) {
                  // ignore grace notes and other zero-dur ornaments
                  continue;
               }
               currentlist.setLevel(1.0/pow(2.0, scorelevels[ii]));
               currentlist.setFirstDur(firstdur);
               currentlist.setDur(duration);
               currentlist.setPitch(pitch);
               currentlist.setAbsBeat(beatvalue);
               currentlist.setLine(i);
               currentlist.setTrack(track);
               currentlist.setSpine(spine);
               currentlist.setToken(token);
               notelist.append(currentlist);
               currentlist.clear();
 
            } // end of a chord
         }
      }  // end of a line
      firsttime = 0;
 
   } // end of the music selection   

   notelist.allowGrowth(0);

// cout << "Notes: " << endl;
// char xxx[128] = {0};
// for (int z=0; z<notelist.getSize(); z++) {
// cout << "\t(" << notelist[z].getPitch() << " - " <<
// notelist[z].getPitch()%40 << ") " << Convert::base40ToKern(xxx,
// ((notelist[z].getPitch()) % 40)+3*40) << endl;
// }

}



//////////////////////////////////////////////////////////////////////////
//
// old functions -- should get rid of them, but they are used by ckey
//     example program.
//

//////////////////////////////
//
// HumdrumFile::analyzeChordProbabilityDurNorm -- gives start and stop
//       points in terms of absolute beats.
//

int HumdrumFile::analyzeChordProbabilityDurNorm(Array<double>& coef, 
      double startbeat, double stopbeat, Array<int>& scorelevels, 
      double empirical1, double empirical2, double sx, double sy) {

   HumdrumFile& score = *this;

   int start = 0;
   int stop = score.getNumLines() - 1;

   while (start < score.getNumLines()-1 && 
         score[start].getAbsBeat() < startbeat) {
      start++;
   }
   while (stop > 0 && score[stop].getAbsBeat() > stopbeat) {
      stop--;
   }


   return analyzeChordProbabilityDurNorm(coef, start, stop, scorelevels, 
        empirical1, empirical2, sx, sy);
}



//////////////////////////////
//
// HumdrumFile::analyzeChordProbabilityDur -- gives start and stop
//       points in terms of absolute beats.
//

int HumdrumFile::analyzeChordProbabilityDur(Array<double>& coef, 
      double startbeat, double stopbeat, Array<int>& scorelevels, 
      double empirical1, double empirical2, double sx, double sy) {

   HumdrumFile& score = *this;

   int start = 0;
   int stop = score.getNumLines() - 1;

   while (start < score.getNumLines()-1 && 
         score[start].getAbsBeat() < startbeat) {
      if (fabs(score[start].getAbsBeat() - startbeat) < 0.001) {
         break;
      }
      start++;
   }
   while (stop > 0 && score[stop].getAbsBeat() > stopbeat) {
      if (fabs(score[stop].getAbsBeat() - stopbeat) < 0.001) {
         break;
      }
      stop--;
   }

   // if subdividing a rhythm:
   if (stop < start) {
      start = stop;
   }

// cout << "Start line: " << start+1 << "\tEnd line: " << stop+1 << endl;

   return analyzeChordProbability(coef, start, stop, scorelevels, 
        empirical1, empirical2, sx, sy);
}



//////////////////////////////
//
// HumdrumFile::analyzeChordProbability -- analyze a set of notes in
//     the score for the most likely chord that fits those notes
//     using Craig's thirds-space plus rhythmic information algorithm.
//     returns the Base-40 pitch class number of the most likely chord 
//     possibility.  The coef array size is set to 40 and the chord scorings 
//     for each root are generated in the function.
//
// Default value: empirical1 = -3.0
// Default value: empirical2 = -3.0
//

int HumdrumFile::analyzeChordProbability(Array<double>& coef, 
      int start, int stop, Array<int>& scorelevels, double empirical1,
      double empirical2, double sx, double sy) {

   int vectorx[40] = {
		0,	//  Cn	0	
		7,	//  Cs	1	
		7,	//  Css	2	
		1000,	//  x       3    
		4,	//  Dff	4	
		4,	//  Df	5	
		4,	//  Dn	6	
		4,	//  Ds	7	
		4,	//  Dss	8	
		1000,	//  x       9    
		8,	//  Eff	10	
		1,	//  Ef	11	
		1,	//  En	12	
		8,	//  Es	13	
		8,	//  Ess	14	
		5,	//  Fff	15	
		5,	//  Ff	16	
		5,	//  Fn	17	
		5,	//  Fs	18	
		5,	//  Fss	19	
		1000,	//  x       20    
		9,	//  Gff	21	
		2,	//  Gf	22	
		2,	//  Gn	23	
		2,	//  Gs	24	
		9,	//  Gss	25	
		1000,	//  x       26    
		6,	//  Aff	27	
		6,	//  Af	28	
		6,	//  An	29	
		6,	//  As	30	
		6,	//  Ass	31	
		1000,	//  x       32    
		3,	//  Bff	33	
		3,	//  Bf	34	
		3,	//  Bn	35	
		3,	//  Bs	36	
		10,	//  Bss	37	
		7,	//  Cff	38	
		7	//  Cf	39	
	};

   int vectory[40] = {
		0,	//  Cn	0	
		-2,	//  Cs	1	
		-4,	//  Css	2	
		1000,	//  x       3    
		4,	//  Dff	4	
		2,	//  Df	5	
		0,	//  Dn	6	
		-2,	//  Ds	7	
		-4,	//  Dss	8	
		1000,	//  x       9    
		3,	//  Eff	10	
		1,	//  Ef	11	
		1,	//  En	12	
		-3,	//  Es	13	
		-5,	//  Ess	14	
		5,	//  Fff	15	
		3,	//  Ff	16	
		1,	//  Fn	17	
		-1,	//  Fs	18	
		-3,	//  Fss	19	
		1000,	//  x       20    
		4,	//  Gff	21	
		2,	//  Gf	22	
		0,	//  Gn	23	
		-2,	//  Gs	24	
		-4,	//  Gss	25	
		1000,	//  x       26    
		4,	//  Aff	27	
		2,	//  Af	28	
		0,	//  An	29	
		-2,	//  As	30	
		-4,	//  Ass	31	
		1000,	//  x       32    
		3,	//  Bff	33	
		1,	//  Bf	34	
		-1,	//  Bn	35	
		-3,	//  Bs	36	
		-5,	//  Bss	37	
		4,	//  Cff	38	
		2	//  Cf	39	
	};

   HumdrumFile& score = *this;

   if (scorelevels.getSize() != score.getNumLines()) {
      scorelevels.setSize(0);
      score.analyzeMetricLevel(scorelevels);
   }

   Array<double> pitches;
   Array<double> durs;
   Array<double> levels;

   int linecount = stop - start + 1;
   pitches.setSize(linecount * 32);
   durs.setSize(linecount * 32);
   levels.setSize(linecount * 32);
   pitches.setSize(0);
   durs.setSize(0);
   levels.setSize(0);
   pitches.allowGrowth();
   durs.allowGrowth();
   levels.allowGrowth();

   double pitch;
   double duration;
   double level;

   char buffer[128] = {0};

   int i, ii;          // line index
   int j, jj;          // spine index
   int k;              // chord note index
   int ccount;         // chord note count
   int firsttime = 1;  // collect tied note from previous point in score
   // first build array of notes used to calculate the chord statistics
   for (i=start; i<=stop; i++) {
      if (score[i].getType() != E_humrec_data) {
         // ignore non-note data lines
         continue;
      }
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
// cout << "i=" << ii << "\tj=" << jj << "\tk=" << k << "\t: " << buffer << "\t";
               if (strchr(buffer, 'r') != NULL) {
                  // skip over rests
// cout << "rest" << endl;
                  continue;
               }
               if (strchr(buffer, '_') != NULL) {
                  // skip over doubly tied notes
// cout << "tie" << endl;
                  continue;
               }
               if (!firsttime && strchr(buffer, ']') != NULL) {
                  // skip over tied notes at the ends of ties.
// cout << "tie" << endl;
                  continue;
               }
               // have a note so now extract the metric level and the duration
               pitch = Convert::kernToBase40(buffer);
               if (pitch < 0) {
                  // ignore rests
                  continue;
               }
//             if (buffer[0] == '\0') {
//                continue;
//             }
               pitch = ((int)pitch - 2 + 40) % 40;
               duration = score.getTiedDuration(ii, jj, k);
               if (duration == 0.0) {
                  // ignore grace notes and other zero-dur ornaments
                  continue;
               }
               level = scorelevels[ii];
// cout << "Pitch= " << buffer << " (" << pitch << ")\t";
// cout << "Dur=" << duration << "\t";
// cout << "Lev=" << level << "\n";
               durs.append(duration);
               levels.append(level);
               pitches.append(pitch);

            } // end of a chord
         }
      }  // end of a line
      firsttime = 0;

   } // end of the music selection

   // now process the measured data to get it in a good form:
   // double empirical = -3;
   for (i=0; i<pitches.getSize(); i++) {
      if (pitches[i] >= 40) {
         cout << "Error in pitch " <<pitches[i] <<" inside chord anal" <<endl;
         exit(1);
      }
//      pitches[i] = chordnorm707[(int)pitches[i]];
      durs[i] = -log(durs[i])/log(2.0) + empirical1;
      levels[i] = levels[i] + empirical2;           // already log2()
//      levels[i] = log(levels[i])/log(2) + empirical;
   }
     
   // exit if there are no notes to analyze
   if (pitches.getSize() == 0) {
      return -1;
   }

   // now measure the chord probablility for all roots 
   double suma;
   double sumb;
   coef.setSize(40);
   int rootindex = 0;
   double normvalue;
   for (i=0; i<coef.getSize(); i++) {
      suma = 0.0;
      sumb = 0.0;
      for (j=0; j<pitches.getSize(); j++) {
         rootindex = ((int)pitches[j] - i + 40) % 40;
         normvalue = sqrt(vectorx[rootindex] * vectorx[rootindex] * sx * sx +
                     vectory[rootindex] * vectory[rootindex] * sy * sy);
         suma += normvalue * durs[j];
         sumb += normvalue * levels[j];
      }
      coef[i] = sqrt(suma * suma + sumb * sumb) / (double)pitches.getSize();
   }

   return 0;
}


// md5sum: ddaf85ce49c3144ab9ab9f110e9ec9ba HumdrumFile-chord.cpp [20050403]
