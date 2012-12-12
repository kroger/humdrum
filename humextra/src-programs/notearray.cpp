//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Aug 30 10:51:26 PDT 2011
// Last Modified: Fri Sep  2 18:25:34 PDT 2011
// Last Modified: Tue Sep 13 13:33:52 PDT 2011 added -k option
// Last Modified: Thu Sep 15 01:36:49 PDT 2011 added -D option
// Last Modified: Thu Oct 20 22:23:27 PDT 2011 fixed init bug
// Filename:      ...sig/examples/all/notearray.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/notearray.cpp
// Syntax:        C++; museinfo
// 
// Description:   Generate a two-dimensional numeric array containing
//                notes in the score in base-40, base-12, or base-7 
//                representation.  Each data line represents a sonority
//                with attacked notes in that sonority being positive
//                numbers, and sustained notes from previous sonorities
//                being negative.
//

#include "humdrum.h"

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif
   
#define STYLE_BASE40 40
#define STYLE_BASE12 12
#define STYLE_BASE7   7

#define TYPE_MIN         999
#define TYPE_NOTES	1000
#define TYPE_KERN       1000
#define TYPE_LINE	(2000-1)  /* +1 will be added later to make 2000 */
#define TYPE_MEASURE	3000
#define TYPE_BEAT 	4000
#define TYPE_ABSOLUTE	5000
#define TYPE_LINEDUR    6000
#define TYPE_ATTACK	7100
#define TYPE_LAST    	7200
#define TYPE_NEXT	7300

// function declarations
void getNoteArray           (Array<Array<int> >& notes, Array<int>& measpos, 
                             Array<int>& linenum, HumdrumFile& infile, 
                             int base, int flags);
void printNoteArray         (Array<Array<int> >& notes, Array<int>& measpos, 
                             Array<int>& linenum, HumdrumFile& infile);
void printComments          (HumdrumFile& infile, int startline, int stopline, 
                             int style);
void printExclusiveInterpretations(int basecount);
void printLine              (Array<Array<int> >& notes, 
                             Array<Array<int> >& attacks, 
                             Array<Array<int> >& lasts, 
                             Array<Array<int> >& nexts, Array<int>& measpos, 
                             Array<int>& linenum, HumdrumFile& infile, 
                             int index, int style);
void usage                  (const char* command);
void example                (void);
void checkOptions           (Options& opts, int argc, char* argv[]);
void getNoteAttackIndexes   (Array<Array<int> >& attacks, 
                             Array<Array<int> >& notes, int offst);
void getLastAttackIndexes   (Array<Array<int> >& lasts, 
                             Array<Array<int> >& notes, int offset);
void getNextAttackIndexes   (Array<Array<int> >& lasts, 
                             Array<Array<int> >& notes, int offset);
int  noteStartMarker        (Array<Array<int> >& notes, int line, int column);
int  noteEndMarker          (Array<Array<int> >& notes, int line, int column);
int  noteContinueMarker     (Array<Array<int> >& notes, int line, int column);
int  singleNote             (Array<Array<int> >& notes, int line, int column);


// global variables
Options   options;             // database for command-line arguments
int       humdrumQ  = 0;       // used with -H option
int       base7Q    = 0;       // used with -d option
int       base12Q   = 0;       // used with -m option
int       base40Q   = 1;       // default output type
int       base      = STYLE_BASE40;
int       measureQ  = 1;       // used with -M option
int       beatQ     = 1;       // used with -B option
int       commentQ  = 1;       // used with -C option
int       rationalQ = 0;       // used with -r option
int       fractionQ = 0;       // used with -f option
int       absoluteQ = 0;       // used with -a option
int       linedurQ  = 0;       // used with -D option
int       doubleQ   = 0;       // used with --double option
int       lineQ     = 0;       // used with -l option
int       mathQ     = 0;       // used with --mathematica option
int       susxQ     = 1;       // used with -S option
int       bibQ      = 0;       // used with -b option
int       infoQ     = 1;       // used with -I option
int       octadj    = 0;       // used with -o option
int       endQ      = 0;       // used with -e option
int       typeQ     = 0;       // used with -c option
int       oneQ      = 0;       // used with -1 option
int       sepQ      = 0;       // used with --sep option
int       Offset    = 0;       // used with -1/--offset option
int       OffsetSum = 0;       // used for multiple input files
int       attackQ   = 0;       // used with --attack option
int       nextQ     = 0;       // used with --last option
int       lastQ     = 0;       // used with --next option
int       indexQ    = 0;       // used with -i option
int       saQ       = 0;       // used with --sa option
int       quoteQ    = 0;       // used with --quote option
int       Count     = 0;       // count of how many input files
int       Current   = 0;       // used with --math option
int       moQ       = 0;       // used with --mo option
int       Measure   = 0;       // additive value for measure number
int       Mincrement= 0;       // used to increment between pieces/movements
int       kernQ     = 0;       // used with -k option
int       kerntieQ  = 1;       // used with --no-tie option
int       doubletieQ= 0;       // used with -T option
RationalNumber Absoffset;      // used with --sa option

const char* commentStart = "#";
const char* commentStop  = "";
const char* mathvar  = "data"; // used with --mathematica option
const char* beatbase = "";     // used with -t option


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {

   Array<Array<int> > notelist;
   Array<double>      metpos;
   Array<int>         measpos;
   Array<int>         linenum;
 
   HumdrumFile infile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();
   Count = numinputs;

   Absoffset = 0;

   for (int i=0; i<numinputs || i==0; i++) {
      Current = i;
      if (moQ) {
         Measure = (i+1) * Mincrement;
      }
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }
      // analyze the input file according to command-line options
      infile.analyzeRhythm(beatbase);

      getNoteArray(notelist, measpos, linenum, infile, base, doubleQ);
      printNoteArray(notelist, measpos, linenum, infile);
      OffsetSum += notelist.getSize();
    
      if (!saQ) {
         Absoffset += infile.getTotalDurationR();
      }

      if (sepQ && (Count > 1) && (Current < Count - 1)) {
         // add a separate between input file analyses:
         if (mathQ) {
            cout << "(* ********** *)\n";
         } else if (humdrumQ) {
            cout << "!!!!!!!!!!\n";         
         } else {
            cout << "##########\n";
         }
      }
   }
   
   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// getNoteArray -- 
//    style:
//       STYLE_BASE40 40: print as base-40 pitches
//       STYLE_BASE12 12: print as base-12 pitches
//       STYLE_BASE7   7: print as base-7  pitches
//
//    flags:
//    	0: all off:
//    	1: turn on double-barline rest markers.
//

void getNoteArray(Array<Array<int> >& notes, Array<int>& measpos, 
      Array<int>& linenum, HumdrumFile& infile, int style, int flags) {

   notes.setSize(infile.getNumLines());
   notes.setSize(0);

   measpos.setSize(infile.getNumLines());
   measpos.setSize(0);

   linenum.setSize(infile.getNumLines());
   linenum.setSize(0);

   int measnum = 0;
   int rest    = 0;
   int i, j, ii, jj;
   int negQ    = 0;

   // prestorage for note lists so that --no-sustain option can be applied
   Array<int> templist;
   templist.setSize(1000);
   templist.setSize(0);

   Array<int> templistI;
   templistI.setSize(1000);
   templistI.setSize(0);

   int value;
   int firstQ = 1;
   if (typeQ) {
      // store even if lineQ is zero!
      value = TYPE_LINE;
      linenum.append(value);
   }
   if (typeQ) {
      value = TYPE_MEASURE;
      measpos.append(value);
   }

   // increment measure number in case of pickups
   measnum = Measure;
   for (i=0; i<infile.getNumLines(); i++) {

      if (infile[i].isMeasure()) {
         if (doubleQ && (templist.getSize() == 0)) {
            if (strstr(infile[i][0], "||") != NULL) {
               // insert a rest sonority to break music
               // from previous measure
               notes.setSize(notes.getSize() + 1);
               notes.last().setSize(infile[i].getFieldCount());
               notes.last().setSize(0);
               for (j=0; j<infile[i].getFieldCount(); j++) {
                  if (!infile[i].isExInterp(j, "**kern")) {
                     continue;
                  }
                  notes.last().append(rest);
                  linenum.append(i);
               }
               measpos.append(measnum);
            }
         }

         // store new measure number (avoiding double code above)
         sscanf(infile[i][0], "=%d", &measnum);
         measnum += Measure;
      }
      if (!infile[i].isData()) {
         continue;
      }

      templist.setSize(0);


      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (!infile[i].isExInterp(j, "**kern")) {
            continue;
         }
         if (strchr(infile[i].getSpineInfo(j), 'b') != NULL) {
            // ignore notes non-primary tracks
            continue;
         }
         int attack = 1;
         if (strcmp(infile[i][j],".")==0) {
            attack = -1;
            ii = infile[i].getDotLine(j);
            jj = infile[i].getDotSpine(j);
         } else {
            ii = i;
            jj = j;
         }
         int baseval = Convert::kernToBase40(infile[ii][jj]);
         baseval += 40 * octadj;
         if (strchr(infile[ii][jj], 'r') != NULL) {
            baseval = 0;
         } else {
            // now storing base-40, and converting to base-12/base-7 later
            // switch (style) {
            //    case STYLE_BASE12: 
            //       baseval = Convert::base40ToMidiNoteNumber(baseval);
            //       break;
            //    case STYLE_BASE7: 
            //       baseval = Convert::base40ToDiatonic(baseval);
            //       break;
            // }
         }
         baseval = attack * baseval;
         templist.append(baseval);
      }

      negQ = 1;
      if (susxQ) {
         // if all notes are negative, then do not store the line
         for (int m=0; m<templist.getSize(); m++) {
            if (templist[m] >= 0) {
               negQ = 0;
               break;
            }
         }
         if (negQ) {
            continue;
         }
      }

      if (firstQ && typeQ) {
         firstQ = 0;
         templistI.setSize(templist.getSize());
         int value = TYPE_NOTES;
         switch (style) {
            case STYLE_BASE40: value += 40; break;
            case STYLE_BASE12: value += 12; break;
            case STYLE_BASE7:  value +=  7; break;
         }
         templistI.setAll(value);
         notes.increase();
         notes.last() = templistI;
      }

      notes.increase();
      notes.last() = templist;
      measpos.append(measnum);
      linenum.append(i);

   }


   if (endQ) {
      notes.increase();
      notes.last().setSize(notes.last(-1).getSize());
      notes.last().zero();
      // sustains instead of rests (have to copy .last(-1):
      // for (i=0; i<notes.last().getSize(); i++) {
      //    if (notes.last()[i] > 0) {
      //       notes.last()[i] *= -1;
      //    }
      // }
      measpos.append(measpos.last());
      linenum.append(linenum.last());
      // store the data termination line as the line number of the sonority
      for (i=infile.getNumLines()-1; i>=0; i--) {
         if (infile[i].isInterpretation()) {
            linenum.last() = i;
            break;
         }
      }
 
      // increment the measure number if the beat of the end
      // is at 1.  This will work unless the last sonority is a 
      // grace note.
      if (infile[linenum.last()].getBeat() == 1.0) {
         measpos.last()++; 
      }
   }

}



//////////////////////////////
//
// printNoteArray --
//

void printNoteArray(Array<Array<int> >& notes, Array<int>& measpos,
      Array<int>& linenum, HumdrumFile& infile) {

   linenum.allowGrowth(0);
   measpos.allowGrowth(0);
   notes.allowGrowth(0);

   Array<Array<int> > attacks;
   Array<Array<int> > lasts;
   Array<Array<int> > nexts;
   if (attackQ) {
      getNoteAttackIndexes(attacks, notes, Offset + OffsetSum);
   }
   if (lastQ) {
      getLastAttackIndexes(lasts, notes, Offset + OffsetSum);
   }
   if (nextQ) {
      getNextAttackIndexes(nexts, notes, Offset + OffsetSum);
   }

   int i, j;
   for (i=0; i<notes.getSize(); i++) {
      if (i == 0) { 
         if (commentQ && !typeQ) {
            printComments(infile, 0, linenum[0], humdrumQ);
         } else if (commentQ && typeQ) {
            printComments(infile, 0, linenum[1], humdrumQ);
         }
         if (infoQ) {
            printExclusiveInterpretations(notes[0].getSize());
         } else if (humdrumQ) {
            // always print exclusive interpretations if Humdrum output
            printExclusiveInterpretations(notes[0].getSize());
         }
         if (mathQ) {
            if (Count <= 1) {
               cout << mathvar << " = {\n";
            } else if (Current == 0) {
               cout << mathvar << " = {{\n";
            } else {
               cout << "}, {\n";
            }
         }
      } else if (commentQ) {
         printComments(infile, linenum[i-1]+1, linenum[i], humdrumQ);
      }
      if (typeQ && i == 0) {
         printLine(notes, attacks, lasts, nexts, measpos, linenum, infile,i,1);
      } else {
         printLine(notes, attacks, lasts, nexts, measpos, linenum, infile,i,0);
      }
   }

   if (humdrumQ) {
      int width = 1;
      if (attackQ) { width++; }
      if (lastQ)   { width++; }
      if (nextQ)   { width++; }
      if (kernQ)   { width++; }
         
      int counter = notes[0].getSize();
      counter *= width;
      counter += indexQ + lineQ + measureQ + beatQ + absoluteQ + linedurQ;
      // if (attackQ) { counter += notes[0].getSize(); }
      // if (lastQ) { counter += notes[0].getSize(); }
      // if (nextQ) { counter += notes[0].getSize(); }
      for (j=0; j<counter; j++) {
         cout << "*-";
         if (j < counter-1) {
            cout << "\t";
         }
      }
      cout << endl;
   }

   if (mathQ) {
      if (Count <= 1) {
         cout << "};\n";
      } else if (Current <= Count - 2) {
         // cout << "}},\n";
      } else if (Current == Count - 1) {
         cout << "}};\n";
      }
   }

   if (commentQ) {
      printComments(infile, linenum.last(), infile.getNumLines()-1, humdrumQ);
   }
}



//////////////////////////////
//
// getLastAttackIndexes -- return an index of the attack portion
//     of notes (or the first rest in a series of rests) in each voice.
//

void getLastAttackIndexes(Array<Array<int> >& lasts, 
      Array<Array<int> >& notes, int offset) {

   int start = 0;
   lasts.setSize(notes.getSize());
   if (typeQ) {
      lasts[0].setAll(TYPE_LAST);
      start = 1;
   }

   int i, j;

   lasts[start].setAll(-1);

   if (notes.getSize() == start+1) {
      return;
   }

   Array<int> states;
   states.setSize(notes[0].getSize());
   if (typeQ) {
      states.setAll(offset+1);
   } else {
      states.setAll(offset);
   }

   for (i=0; i<notes.getSize(); i++) {
      lasts[i].setSize(notes[i].getSize());
      if (i <= start) {
         lasts[i].setAll(-1);
         if (typeQ && (i==0)) {
            lasts[i].setAll(TYPE_LAST);
         }
         continue;
      }

      for (j=0; j<notes[i].getSize(); j++) {
         if (notes[i][j] > 0) {
            // a new note attack, store the note attack index in
            // the states array after recording the index of the previous note
            lasts[i][j] = states[j];
            states[j] = i + offset;
         } else if (notes[i][j] < 0) {
           // note is sustaining, so copy the index of the last attack
           // from the previous line
           if (i > start) {
              lasts[i][j] = lasts[i-1][j];
           } else {
              lasts[i][j] = -1;
           }
         } else {
           // rest: if last line was a rest then this is a rest sustain:
           if (i > start) {
              if (notes[i-1][j] == 0) {
                 // rest sustain
                 lasts[i][j] = lasts[i-1][j];
              } else {
                 // rest attack
                 lasts[i][j] = states[j];
                 states[j] = i + offset;
              }  
           } else {
              lasts[i][j] = -1;
           }
         }
      }
   }

}



//////////////////////////////
//
// getNextAttackIndexes -- return an index of the attack portion
//     of notes (or the first rest in a series of rests) in each voice.
//

void getNextAttackIndexes(Array<Array<int> >& nexts, 
      Array<Array<int> >& notes, int offset) {

   int start = 0;
   nexts.setSize(notes.getSize());
   if (typeQ) {
      nexts[0].setSize(notes[0].getSize());
      nexts[0].setAll(TYPE_NEXT);
      start = 1;
   }

   int i, j;

   nexts.last().setSize(notes.last().getSize());
   nexts.last().setAll(-1);

   if (notes.getSize() == start+1) {
      return;
   }

   for (i=notes.getSize()-2; i>=start; i--) {
      nexts[i].setSize(notes[i].getSize());
      for (j=0; j<notes[i].getSize(); j++) {
         if ((notes[i][j] != 0) && (notes[i+1][j] == 0)) {
            // next note is a "rest attack"
            nexts[i][j] = i + 1 + offset;
         } else if (notes[i+1][j] > 0) {
            // a new note attack, store the note attack index in
            // the states array after recording the index of the previous note
            nexts[i][j] = i + 1 + offset;
         } else {
            nexts[i][j] = nexts[i+1][j];
         }

      }
   }

}



//////////////////////////////
//
// getNoteAttackIndexes -- return an index of the attack portion
//     of notes (or the first rest in a series of rests) in each voice.
//

void getNoteAttackIndexes(Array<Array<int> >& attacks, 
      Array<Array<int> >& notes, int offset) {

   attacks.setSize(notes.getSize());
   int i, j;

   attacks[0].setSize(notes[0].getSize());
   attacks[0].setAll(offset);

   for (i=1; i<attacks.getSize(); i++) {
      attacks[i].setSize(notes[i].getSize());
      attacks[i].allowGrowth(0);
      for (j=0; j<attacks[i].getSize(); j++) {
         if (notes[i][j] < 0) {
            // a sustained note, so store index of attack note
            attacks[i][j] = attacks[i-1][j];
         } else if (notes[i][j] > 0) {
            // note being attacked, so store its index
            attacks[i][j] = i + offset;
         } else {
            // a rest: check to see if last position was a rest.
            // if so, then this is a "tied rest"; otherwise it is
            // a "attacked rest".
            if (notes[i-1][j] == 0) {
               attacks[i][j] = attacks[i-1][j];
            } else {
               attacks[i][j] = i + offset;
            }
         }
      }
   }

   if (typeQ) {
      attacks[0].setAll(TYPE_ATTACK);
   }
}



//////////////////////////////
//
// printLine -- print a line of the note array.
//    style == 0: regular line
//    style == 1: index line (don't extract beat or absbeat from infile)
//

void printLine(Array<Array<int> >& notes, Array<Array<int> >& attacks, 
      Array<Array<int> >& lasts, Array<Array<int> >& nexts, 
      Array<int>& measpos, Array<int>& linenum, HumdrumFile& infile, 
      int index, int style) {

   int& i = index;
   int j;

   if (mathQ) {
      cout << "{";
   }

   if (indexQ) { 
      cout << i + Offset + OffsetSum;
   }

   if (lineQ) { 
      if (indexQ) {
         if (mathQ) {
            cout << ",";
         }
         cout << "\t";
      }
      cout << linenum[i]+1;
   }

   if (measureQ) { 
      if (indexQ || lineQ) {
         if (mathQ) {
            cout << ",";
         }
         cout << "\t";
      }
      cout << measpos[i];
   }

   if (beatQ) {
      if (indexQ || lineQ || measureQ) {
         if (mathQ) {
            cout << ",";
         }
         cout << "\t";
      }
      if (style == 1) {
         cout << TYPE_BEAT;
      } else if (rationalQ) {
         if (fractionQ) {
            cout << infile[linenum[i]].getBeatR();
         } else {
            infile[linenum[i]].getBeatR().printTwoPart(cout);
         }
      } else {
         // print beat position as a floating-point number:
         cout << infile[linenum[i]].getBeat();
      }
   }

   if (absoluteQ) {
      if (indexQ || lineQ || measureQ || beatQ) {
         if (mathQ) {
            cout << ",";
         }
         cout << "\t";
      }
      if (style == 1) {
         cout << TYPE_ABSOLUTE;
      } else if (rationalQ) {
         if (fractionQ) {
            cout << (Absoffset + infile[linenum[i]].getAbsBeatR());
         } else {
            RationalNumber sum = Absoffset + infile[linenum[i]].getAbsBeatR();
            sum.printTwoPart(cout);
         }
      } else {
         // print beat position as a floating-point number:
         cout << infile[linenum[i]].getAbsBeat() + Absoffset.getFloat();
      }
 
   }

   if (linedurQ) {
      if (indexQ || lineQ || measureQ || beatQ || absoluteQ) {
         if (mathQ) {
            cout << ",";
         }
         cout << "\t";
      }
      if (style == 1) {
         cout << TYPE_LINEDUR;
      } else if (rationalQ) {
         if (fractionQ) {
            cout << (Absoffset + infile[linenum[i]].getDurationR());
         } else {
            RationalNumber num = infile[linenum[i]].getDurationR();
            num.printTwoPart(cout);
         }
      } else {
         // print beat position as a floating-point number:
         cout << infile[linenum[i]].getDuration();
      }
 
   }
   if (indexQ || lineQ || measureQ || beatQ || absoluteQ || linedurQ) {
      if (mathQ) {
         cout << ",";
      }
   }


   Array<int> values;
   values.setSize(notes.getSize() * 10);
   values.setSize(0);

   int vv;
   int sign;
   for (j=0; j<notes[i].getSize(); j++) {
      vv = notes[i][j];
      //if (kernQ && (style == 1) && typeQ) {
      // int ww = TYPE_KERN;
      //   values.append(ww);
      //} 
      if (vv < TYPE_NOTES) {
         if (vv < 0) {
            sign = -1;
            vv = -vv;
         } else {
            sign = +1;
         }
         if (vv != 0) { 
            switch (style) {
               case STYLE_BASE12: 
                  vv = Convert::base40ToMidiNoteNumber(vv);
                break;
               case STYLE_BASE7: 
                  vv = Convert::base40ToDiatonic(vv);
                  break;
            }
         }
         vv *= sign;
         values.append(vv);
      } else {
         values.append(vv);
      }

      if (attackQ) {
         values.append(attacks[i][j]);
      }
      if (lastQ) {
         values.append(lasts[i][j]);
      }
      if (nextQ) {
         values.append(nexts[i][j]);
      }
   }

   int width = 1;  // notes
   if (attackQ) { width++; }
   if (lastQ)   { width++; }
   if (nextQ)   { width++; }

   char buffer[1024] = {0};

   for (j=0; j<values.getSize(); j++) {
      if (kernQ && ((j % width) == 0)) {
         //if (mathQ) {
         //   cout << ",";
         //}
         cout << "\t";
         if ((style == 1) && (i == 0)) {
            cout << TYPE_KERN;         
         } else {
            if (mathQ || quoteQ) {
               cout << "\"";
            }

            if (kerntieQ && noteStartMarker(notes, i, j / width)) {
               cout << "[";
            } 
            if (doubletieQ && singleNote(notes, i, j / width)) {
               cout << "["; 
            }

            if (values[j] == 0) {
               cout << "r";
            } else {
               cout << Convert::base40ToKern(buffer, abs(values[j]));
            }

            if (kerntieQ && noteContinueMarker(notes, i, j / width)) {
               cout << "_";
            } else if (kerntieQ && noteEndMarker(notes, i, j / width)) {
               cout << "]";
            } 
            if (doubletieQ && singleNote(notes, i, j / width)) {
               cout << "]"; 
            }

            if (mathQ || quoteQ) {
               cout << "\"";
            }
         }
         if (mathQ) {
            cout << ",";
         }
      }
      cout << "\t" << values[j];

      if (j < values.getSize()-1) {
         if (mathQ) {
            cout << ",";
         }
         //cout << "\t";
      }
   }
   if (mathQ) {
      cout << "}";
      if (i < linenum.getSize() - 1) {
         cout << ",";
      }
   }
   cout << endl;
}



//////////////////////////////
//
// singleNote -- true if the note in the column/line is uniq
//

int singleNote(Array<Array<int> >& notes, int line, int column) {
   int start = 0;
   if (typeQ) {
      start = 1;
   }
   if (line < start) {
      return 0;
   }

   int pitch = notes[line][column];
   int nextpitch = -1;
   int lastpitch = -1;

   if ((line > start) && (notes.getSize() > 1+start)) {
      lastpitch = notes[line-1][column];
   }
   if ((notes.getSize() > start+1) && (line < notes.getSize() - 1)) {
      nextpitch = notes[line+1][column];
   }
 
   if (pitch > TYPE_MIN) {
      return 0;
   }

   if (pitch < 0) {
      // note is a sustain, so not considered
      return 0;
   } else if (pitch == 0) {
      // if the rest is surrounded by non-rests, then mark
      if ((pitch != lastpitch) && (pitch != nextpitch)) {
         return 1;
      } else {
         return 0;
      }
   } else {
      if (pitch != -nextpitch) {
         // next sonority does not contain a sustain of the note
         return 1;
      } else {
         return 0;
      }
   }
}



//////////////////////////////
//
// noteStartMarker --
//

int noteStartMarker(Array<Array<int> >& notes, int line, int column) {
   int start = 0;
   if (typeQ) {
      start = 1;
   }
   if (line < start) {
      return 0;
   }

   int pitch = notes[line][column];
   int nextpitch = -1;
   int lastpitch = -1;

   if ((line > start) && (notes.getSize() > 1+start)) {
      lastpitch = notes[line-1][column];
   }
   if ((notes.getSize() > start+1) && (line < notes.getSize() - 1)) {
      nextpitch = notes[line+1][column];
   }

   if (pitch == 0) {
      // if the first rest in a row then true
      if ((lastpitch != 0) && (nextpitch == 0)) {
         // don't include rests which start and stop on the same line
         return 1;
      } else {
         return 0;
      }
   } else if (pitch < 0) {
      return 0;
   } else {
      if ((nextpitch < 0) && (nextpitch != -1)) {
         return 1;
      } else {
         return 0;
      }
   }
}



//////////////////////////////
//
// noteContinueMarker --
//

int noteContinueMarker(Array<Array<int> >& notes, int line, int column) {
   int start = 0;
   if (typeQ) {
      start = 1;
   }
   if (line < start) {
      return 0;
   }

   int pitch = notes[line][column];
   int nextpitch = -1;
   int lastpitch = -1;

   if ((line > start) && (notes.getSize() > 1+start)) {
      lastpitch = notes[line-1][column];
   }
   if ((notes.getSize() > start+1) && (line < notes.getSize() - 1)) {
      nextpitch = notes[line+1][column];
   }

   if (pitch == 0) {
      // if not the first or the last zero than a continue
      if ((lastpitch == 0) && (nextpitch == 0)) {
         return 1;
      } else {
         return 0;
      }
   } else if (pitch > 0) {
      return 0;
   } else {
      if (pitch == nextpitch ) {
         return 1;
      } else {
         return 0;
      }
   }
}



//////////////////////////////
//
// noteEndMarker --
//

int noteEndMarker(Array<Array<int> >& notes, int line, int column) {
   int start = 0;
   if (typeQ) {
      start = 1;
   }
   if (line < start) {
      return 0;
   }

   int pitch = notes[line][column];
   int nextpitch = -1;
   int lastpitch = -1;

   if ((line > start) && (notes.getSize() > 1+start)) {
      lastpitch = notes[line-1][column];
   }
   if ((notes.getSize() > start+1) && (line < notes.getSize() - 1)) {
      nextpitch = notes[line+1][column];
   }

   if (pitch == 0) {
      // if not the first or the last zero than a continue
      if (nextpitch != 0) {
         return 1;
      } else {
         return 0;
      }
   } else if (pitch > 0) {
      return 0;
   } else {
      if (pitch != nextpitch ) {
         return 1;
      } else {
         return 0;
      }
   }
}



//////////////////////////////
//
// printExclusiveInterpretations -- print ** markers at the start of 
//      each column of data.
//      
//      Order of optional prefix columns:
//      lineQ
//      measureQ
//      beatQ
//      absoluteQ
//      linedurQ
//

void printExclusiveInterpretations(int basecount) {

   char basename[1024] = {0};

   const char* prefix = "##";
   if (humdrumQ || mathQ) {
      prefix = "**";
   } 

   if (kernQ) {
      strcat(basename, "kern");
      strcat(basename, "\t");
   }

   if (base7Q) {
      // strcat(basename, prefix);
      strcat(basename, "b7");
   } else if (base12Q) {
      // strcat(basename, prefix);
      strcat(basename, "b12");
   } else {
      // strcat(basename, prefix);
      strcat(basename, "b40");
   }

   if (attackQ) {
      strcat(basename, "\t");
      strcat(basename, prefix);
      strcat(basename, "attk");
   }
   if (lastQ) {
      strcat(basename, "\t");
      strcat(basename, prefix);
      strcat(basename, "last");
   }
   if (nextQ) {
      strcat(basename, "\t");
      strcat(basename, prefix);
      strcat(basename, "next");
   }

   int startmark = 0;

   if (indexQ) {
      if ((startmark == 0) && mathQ) {
         cout << "(*";
         startmark++;
      } else {
         cout << prefix;
      }
      cout << "idx\t"; 
   }
   if (lineQ) { 
      if ((startmark == 0) && mathQ) {
         cout << "(*";
         startmark++;
      } else {
         cout << prefix;
      }
      cout << "line\t"; 
   }
   if (measureQ) { 
      if ((startmark == 0) && mathQ) {
         cout << "(*";
         startmark++;
      } else {
         cout << prefix;
      }
      cout << "bar\t"; 
   }

   if (beatQ) { 
      if ((startmark == 0) && mathQ) {
         cout << "(*";
         startmark++;
      } else {
         cout << prefix;
      }
      cout << "beat\t"; 
   }

   if (absoluteQ) { 
      if ((startmark == 0) && mathQ) {
         cout << "(*";
         startmark++;
      } else {
         cout << prefix;
      }
      cout << "abs\t"; 
   }

   if (linedurQ) { 
      if ((startmark == 0) && mathQ) {
         cout << "(*";
         startmark++;
      } else {
         cout << prefix;
      }
      cout << "ldur\t"; 
   }

   int i;
   for (i=0; i<basecount; i++) {
      if ((startmark == 0) && mathQ) {
         cout << "(*";
         startmark++;
      } else {
         cout << prefix;
      }
      cout << basename;
      if (i < basecount - 1) {
         cout << "\t";
      }
   }
   if (mathQ) {
      cout << " *)";
   }
   cout << "\n";

}



//////////////////////////////
//
//  
// printComments -- print any comments
//

void printComments(HumdrumFile& infile, int startline, int stopline, 
      int style) {

   int i;
   for (i=startline; i<=stopline; i++) {
      if (!infile[i].isComment()) {
         continue;
      }
      if (bibQ && infile[i].isGlobalComment()) {
         continue;
      }
      if (infile[i].isLocalComment()) {
         // don't know how to store local comments, ignore for now.
         continue;
      }
      if (style) {
         // print in Humdrum format:
         cout << infile[i] << "\n";
      } else {
         // print in Matlab format:
         cout << commentStart << infile[i] << commentStop << "\n";
      }
   }
}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("a|absolute=b",    "print absolute beat numbers");
   opts.define("b|bib|bibliographic|reference=b", "display only bib record");
   opts.define("c|type=b",        "add a numeric index at start of data array");
   opts.define("d|diatonic=b",    "print output in absolute diatonic");
   opts.define("f|fraction=b",    "display rational number as fraction");
   opts.define("e|end|end-rest=b","store ending rest");
   opts.define("i|index=b",       "print a column with the index value");
   opts.define("j|josquin=b",     "default settings for josquin project");
   opts.define("k|kern=b",        "display kern spine");
   opts.define("no-tie|no-ties=b", "don't display kern tie information");
   opts.define("l|line=b",        "print original line of sonority in data");
   opts.define("m|midi=b",        "print output as MIDI key numbers");
   opts.define("o|octave=i:0",    "octave adjustment value");
   opts.define("r|rational=b",    "display metric position as rational number");
   opts.define("t|beat=s:",       "metric base for constant beat analysis");
   opts.define("1|one=b",         "offset index values by one");
   opts.define("M|no-measures=b", "don't print measure number column");
   opts.define("B|no-beats=b",    "don't print beat value column");
   opts.define("C|no-comments=b", "don't print comments in file");
   opts.define("D|linedur=b",     "display duration of line");
   opts.define("A|all=b",         "display all options prefix columns");
   opts.define("H|humdrum=b",     "print output in Humdrum format");
   opts.define("I|no-info=b",     "do not display information header");
   opts.define("S|no-sustain=b",  "suppress sonorities that are only sustains");
   opts.define("T|all-tie|all-ties=b",  "start/stop tie marks on all notes");
   opts.define("N|no-cols=b",     "turn off all information columns");
   opts.define("attack=b",        "display note-attack index values");
   opts.define("double=b",        "add rests at double barlines");
   opts.define("last=b",          "display previous note-attack index values");
   opts.define("math|mathematica=s:data", "print output data as Matlab array");
   opts.define("mel|melodic=b",    "display melodic note index columns");
   opts.define("mo|measure-offset=i:0", "bar num increase per file");
   opts.define("next=b",          "display following note-attack index values");
   opts.define("offset=i:0",      "starting index for first row");
   opts.define("sa|separate-absolute=b", "single absolute beat positions");
   opts.define("sep|separator=b",  "print a separator between input analyses");
   opts.define("quote=b",          "print quotes around kern names");

   opts.define("debug=b");        // determine bad input line num
   opts.define("author=b");       // author of program
   opts.define("version=b");      // compilation info
   opts.define("example=b");      // example usages
   opts.define("h|help=b");       // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Aug 2011" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 30 August 2011" << endl;
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

   if (opts.getBoolean("josquin")) {
      beatbase  = "1";  // beat is the whole note.
      doubleQ   = 1;
   } else {
      beatbase  = opts.getString("beat");
      doubleQ   = opts.getBoolean("double");
   }

   humdrumQ = opts.getBoolean("humdrum");
   base7Q   = opts.getBoolean("diatonic");
   base12Q  = opts.getBoolean("midi");
   if (base7Q) {
      base7Q  = 1;
      base12Q = 0;
      base40Q = 0;
   } else if (base12Q) {
      base7Q  = 0;
      base12Q = 1;
      base40Q = 0;
   } else {
      base7Q  = 0;
      base12Q = 0;
      base40Q = 1;
   }

   if (base7Q) {
      base = STYLE_BASE7;
   } else if (base12Q) {
      base = STYLE_BASE12;
   } else {
      base = STYLE_BASE40;
   }

   mathQ     =  opts.getBoolean("mathematica");
   mathvar   =  opts.getString("mathematica");
   commentStart = "(* ";
   commentStop  = " *)";

   if (!mathQ) {
      commentStart = "# ";
      commentStop  = "";
   }

   lineQ     =  opts.getBoolean("line");
   measureQ  = !opts.getBoolean("no-measures");
   beatQ     = !opts.getBoolean("no-beats");
   absoluteQ =  opts.getBoolean("absolute");
   linedurQ  =  opts.getBoolean("linedur");
   commentQ  = !opts.getBoolean("no-comments");
   rationalQ =  opts.getBoolean("rational");
   fractionQ =  opts.getBoolean("fraction");
   susxQ     =  opts.getBoolean("no-sustain");
   kernQ     =  opts.getBoolean("kern");
   kerntieQ  = !opts.getBoolean("no-tie");
   bibQ      =  opts.getBoolean("bibliographic");
   infoQ     = !opts.getBoolean("no-info");
   endQ      =  opts.getBoolean("end-rest");
   octadj    =  opts.getInteger("octave");
   typeQ     =  opts.getBoolean("type");
   attackQ   =  opts.getBoolean("attack");
   nextQ     =  opts.getBoolean("last");
   lastQ     =  opts.getBoolean("next");
   indexQ    =  opts.getBoolean("index");
   sepQ      =  opts.getBoolean("sep");
   saQ       =  opts.getBoolean("separate-absolute");
   Mincrement=  opts.getInteger("measure-offset");
   moQ       =  opts.getBoolean("measure-offset");
   Offset    =  opts.getInteger("offset");
   quoteQ    =  opts.getBoolean("quote");
   doubletieQ=  opts.getBoolean("all-tie");
   if (doubletieQ) {
      kerntieQ = 1;
   }
   if (Offset < 0) {
      Offset = 0;
   }
   if (opts.getBoolean("mel")) {
      attackQ = 1;
      nextQ   = 1;
      lastQ   = 1;
   }

   if (opts.getBoolean("1")) {
      Offset = 1;
   }

   if (fractionQ) {
      rationalQ = 1;
   }

   if (opts.getBoolean("no-cols")) {
      measureQ = 0;
      beatQ    = 0;
   }

   if (opts.getBoolean("all")) {
      lineQ = measureQ = beatQ = absoluteQ = linedurQ = 1;
   }

}



//////////////////////////////
//
// example -- example usage of the quality program
//

void example(void) {
   cout <<
   "                                                                         \n"
   << endl;
}



//////////////////////////////
//
// usage -- gives the usage statement for the meter program
//

void usage(const char* command) {
   cout <<
   "                                                                         \n"
   << endl;
}


// md5sum: cae22ba3e86ede60cf34a672bb901dbd notearray.cpp [20111105]
