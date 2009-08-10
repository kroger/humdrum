//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jan 15 17:34:02 PST 2009
// Last Modified: Sun Jan 18 21:14:59 PST 2009
// Filename:      ...sig/doc/examples/improv/synthImprov/jazzchord.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: Chord recognition.
//

#include "humdrum.h"
#include "synthImprov.h"      // includes the default Win95 console interface
                              // for the synthImprov environment

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

Array<int>          keystates;
Array<int>          currentchord;
Array<int>          tempchord;
Array<Array<char> > names;
Array<Array<int> >  chordset;
MidiMessage         message;
long lastnotetime = 0;
long timedelta    = 60;
char lastprintchord[1024] = {0};

// function declarations:
void getNewChordInfo       (Array<int>& tempchord, Array<int>& keystates);
int  equalChord            (Array<int>& tempchord, Array<int>& currentchord);
void storeChordData        (Array<int>& chordinfo, HumdrumFile& infile, 
                            int index);
void prepareChordSet       (Array<Array<char> >& names, 
                            Array<Array<int> >& chordset);
int  compareChords         (int* chorda, int* chordb, int count);
void printNewChord         (Array<int>& tempchord, 
                            Array<Array<int> >& chordset, 
                            Array<Array<char> >& names);


//////////////////////////////
//
// printNewChord --
//

void printNewChord(Array<int>& tempchord, Array<Array<int> >& chordset, 
      Array<Array<char> >& names) {

   SSTREAM chordstream;

   int i, j, k;
   int length;
   Array<int> testset(24);
   testset.allowGrowth(0);
   for (i=0; i<12; i++) {
      testset[i] = tempchord[i];
      testset[i+12] = tempchord[i];
   }

   int count = 0;
   int equal;
   char buffer[1024] = {0};
   for (i=0; i<chordset.getSize(); i++) {
      for (j=0; j<12; j++) {
         equal = compareChords(chordset[i].getBase(), testset.getBase()+j, 12);
         if (equal) {
            if (count > 0) {
               chordstream << " or ";
            }
            count++;
            Convert::base12ToKern(buffer, j+4*12);
	    length = strlen(buffer);
	    for (k=0; k<length; k++) {
               if (buffer[k] == '-') {
                  buffer[k] = 'b';
               }
            }
            if (strcmp(buffer, "G#") == 0) {
               strcpy(buffer, "Ab");
            }
            chordstream << buffer << names[i].getBase(); 
         }
      }
   }

   if (count > 0) {
      chordstream << ends;
      strcpy(lastprintchord, chordstream.CSTRING);
      cout << lastprintchord;
      cout << endl;
   }
}



/////////////////////////////
//
// compareChords --
//

int compareChords(int* chorda, int* chordb, int count) {
   int i;
   for (i=0; i<count; i++) {
      if (chorda[i] != chordb[i]) {
         return 0;
      }
   }

   return 1;
}



//////////////////////////////
//
// getNewChordInfo --
//

void getNewChordInfo(Array<int>& tempchord, Array<int>& keystates) {
   int i;
   tempchord.setAll(0);
   for (i=0; i<keystates.getSize(); i++) {
      if (keystates[i] != 0) {
         tempchord[i%12] = 1;
      }
   }
}


//////////////////////////////
//
// equalChord --
//

int equalChord(Array<int>& tempchord, Array<int>& currentchord) {
   int i;
   for (i=0; i<tempchord.getSize(); i++) {
      if (currentchord[i] != tempchord[i]) {
         return 0;
      }
   }
   return 1;
}



//////////////////////////////
//
// printNewChord --
//

void printNewChord(Array<int>& chord) {
   int i;
   cout << "[";
   for (i=0; i<chord.getSize(); i++) {
      cout << chord[i]; 
      if (i < 12-1) {
         cout << " ";
      }
   }
   cout << "]" << endl;
}



//////////////////////////////
//
// prepareChordSet --
//

void prepareChordSet(Array<Array<char> >& names, Array<Array<int> >& chordset) {
   SSTREAM stream;
   stream << "!! Column 1: Root of the chord\n";
   stream << "!! Column 2: Other notes in the chord\n";
   stream << "!! Column 3: Notated chord symbol\n";
   stream << "!! Column 4: Full chord name\n";
   stream << "!!  substitutes with pitch in first column\n";
   stream << "**kern\t**kern\t**label\t**name\n";
   stream << "!root\t!others\t!\t!\n";
   stream << "4C\t4e 4g\t.\tMajor\n";
   stream << "4C\t4e- 4g\tm\tMinor\n";
   stream << "4C\t4e 4g#\t+\tAugmented Triad\n";
   stream << "4C\t4f 4g\tsus4\tSuspended 4th\n";
   stream << "4C\t4e- 4g-\to\tDiminished Triad\n";
   stream << "4C\t4e 4g 4a\t6\tMajor 6th\n";
   stream << "4C\t4e- 4g 4a\tm6\tMinor 6th\n";
   stream << "4C\t4e 4g 4b\tmaj7\tMajor 7th\n";
   stream << "4C\t4e- 4g 4b-\tm7\tMinor 7th\n";
   stream << "4C\t4e 4g 4b-\t7\tDominant 7th\n";
   stream << "4C\t4f 4g 4b-\t7sus4\tDominant 7th sus4\n";
   stream << "4C\t4e 4g# 4b-\t7+5\tDominant 7th Augmented 5th\n";
   stream << "4C\t4e 4g- 4b-\t7-5\tDominant 7th Flattened 5th\n";
   stream << "4C\t4e- 4g- 4b--\tdim7\tDiminished 7th\n";
   stream << "4C\t4e- 4g- 4b-\tm7-5\tMinor 7th Flattened 5th\n";
   stream << "4C\t4e- 4g 4b\tmmaj7\tMinor-Major 7th\n";
   stream << "4C\t4e 4g 4b 4dd\tmaj9\tMajor 9th\n";
   stream << "4C\t4e- 4g 4b- 4dd\tm9\tMinor 9th\n";
   stream << "4C\t4e 4g 4b- 4dd\t9\tDominant 9th\n";
   stream << "4C\t4e 4g# 4b- 4dd\t9+5\t9th Augmented 5th\n";
   stream << "4C\t4e 4g- 4b- 4dd\t9-5\t9th Flattened 5th\n";
   stream << "4C\t4e 4g 4a 4b- 4dd\t9/6\t9th Add 6th\n";
   stream << "4C\t4e 4g 4b 4dd 4ff\tmaj11\tMajor 11th\n";
   stream << "4C\t4e- 4g 4b- 4dd 4ff\tm11\tMinor 11th\n";
   stream << "4C\t4e 4g 4b- 4dd 4ff\t11\tDominant 11th\n";
   stream << "4C\t4e 4g 4b- 4dd- 4ff\t11-9\t11th Flattened 9th\n";
   stream << "4C\t4e 4g 4b 4dd 4ff 4aa\tmaj13\tMajor 13th\n";
   stream << "4C\t4e- 4g 4b- 4dd 4ff 4aa\tm13\tMinor 13th\n";
   stream << "4C\t4e 4g 4b- 4dd 4ff 4aa\t13\tDominant 13th\n";
   stream << "4C\t4e 4g 4b- 4dd- 4ff 4aa\t13-9\t13th Flattened 9th\n";
   stream << "*-\t*-\t*-\t*-\n";

   stream << ends;

   HumdrumFile infile;
   infile.read(stream);
   chordset.setSize(infile.getNumLines());
   chordset.setSize(0);

   names.setSize(infile.getNumLines());
   names.setSize(0);

   int index;
   int length;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_data) {
         continue;
      }
      index = chordset.getSize();
      chordset.setSize(chordset.getSize() + 1);
      names.setSize(names.getSize() + 1);
      length = strlen(infile[i][2]);
      names[index].setSize(length+1);
      strcpy(names[index].getBase(), infile[i][2]);
      if (names[index][0] == '.') {
         names[index][0] = '\0';	        
      }
      storeChordData(chordset[index], infile, i);
      

   }
   chordset.allowGrowth(0);
}




//////////////////////////////
//
// storeChordData --
//

void storeChordData(Array<int>& chordinfo, HumdrumFile& infile, int index) {
   int i;
   int note;

   chordinfo.setSize(12);
   chordinfo.allowGrowth(0);
   chordinfo.setAll(0);

   note = Convert::kernToMidiNoteNumber(infile[index][0]) % 12;
   if (note != 0) {
      cout << "Error reading chord: " << infile[index] << endl;
   }

   chordinfo[0] = 1;

   char buffer[1024] = {0};
   int count = infile[index].getTokenCount(1);
   for (i=0; i<count; i++) {
      infile[index].getToken(buffer, 1, i, 1000);
      note = Convert::kernToMidiNoteNumber(buffer) % 12;
      if (note < 0) {
         continue;
      }
      chordinfo[note] = 1;
   }
}





/*----------------- beginning of improvization algorithms ---------------*/

/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   printboxtop();
   psl("Identify chords played on the piano");
   psl("");
   psl(" space = mark last chord");
   psl(" enter = add a blank line to display");
   printboxbottom();
} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     interface once at the start of the program.  Put items
//     here which need to be initialized at the beginning of
//     the program.
//

void initialization(void) { 
   keystates.setSize(128);
   keystates.allowGrowth(0);
   keystates.setAll(0);

   currentchord.setSize(12);
   currentchord.allowGrowth(0);
   currentchord.setAll(0);

   tempchord.setSize(12);
   tempchord.allowGrowth(0);
   tempchord.setAll(0);

   prepareChordSet(names, chordset);
    
   lastnotetime = 0;
}



//////////////////////////////
//
// finishup -- this function is called by the improv interface
//     whenever the program is exited.  Put items here which
//     need to be taken care of when the program is finished.
//

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

//////////////////////////////
//
// mainloopalgorithms -- this function is called by the improv interface
//   continuously while the program is running.  The global variable t_time
//   which stores the current time is set just before this function is
//   called and remains constant while in this functions.
//

void mainloopalgorithms(void) { 
   if (synth.getNoteCount() > 0) {
      while (synth.getNoteCount() > 0) {
         message = synth.extractNote();
         if ((message.p2() == 0) || ((message.p0() & 0xf0) == 0x80)) {
            keystates[message.p1()] = 0;
         } else if (message.p2() != 0) {
            keystates[message.p1()] = 1;
         }
      }
      getNewChordInfo(tempchord, keystates);

      lastnotetime = t_time;
   }

   if (t_time > lastnotetime + timedelta) {
      if (!equalChord(tempchord, currentchord)) {
         printNewChord(tempchord, chordset, names);
         currentchord = tempchord;
      }
   }
}


/*-------------------- triggered algorithms -----------------------------*/

///////////////////////////////
//
// keyboardchar -- this function is called by the improv interface
//     whenever a key is pressed on the computer keyboard.
//     Put commands here which will be executed when a key is
//     pressed on the computer keyboard.
//

void keyboardchar(int key) { 
   switch (key) {
      case '\n':
         cout << endl;
	 break;
      case ' ':
         if (strcmp(lastprintchord, "") != 0) {
            cout << "\t\t\t" << lastprintchord << endl;
         }
   }
}


/*------------------ end improvization algorithms -----------------------*/


// md5sum: 163c49a5ec5e0bd41298728add43c609 jazzchord.cpp [20090129]
