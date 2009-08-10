//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Programmer:    Sachiko Deguchi <deguchi@ccrma.stanford.edu>
// Creation Date: Thu Jun 27 12:17:39 PDT 2002
// Last Modified: Sat Aug 16 13:15:53  2003
// Filename:      ...sig/doc/examples/all/kotomel2.cpp
// Syntax:        C++; batonImprov 2.0
//  
// Description:   Melodic pattern player.  Reads an input file
//		  example syntax given at bottom of file.
//                Tries to read k-pattern.txt in current directory.
//

#define USE_TABLET
#include "batonImprov.h"

#include "Array.h"

#include <ctype.h>

#ifndef OLDCPP
   #include <fstream>
   #include <iostream>
   using namespace std;
#else
   #include <fstream.h>
   #include <iostream.h>
#endif


// function declarations
void readfile(const char* filename);
void processLine(const char* line);
void clearpatterns(Array<Array<Array<Array<int> > > >& dp);
void printpatterns(void);

// variables
Array<int> degrees;
Array<int> positions;
Array<Array<Array<Array<int> > > > degpatterns; // scale degree patterns
Voice voice;
int   transpose = 2;         // transpose value for tonic (0 = C)
int   cpat = 0;              // current pattern
int   csub = 0;              // current subpattern
int   npat = 0;              // new pattern choosen by stick 2
int   nsub = 0;              // new subpattern choosen by stick 2
int   cycle= 0;              // used to chose the next note in pattern
int   csd  = 0;              // current scale degree


///////////////////////////////////////////////////////////////////////////
//
// Improv interface functions
//


//////////////////////////////
//
// description --
//

void description(void) {
   cout <<
   "kotomel2\n"
   " Left Hand: choose melodic pattern\n"
   " Right Hand: choose starting scale degree\n"
   " , = koto  . = chinese  / = german melodic patterns\n"
   " space = print current pattern,   \\ = print all patterns\n"
   " transpose tonic note:   [ = down,   ] = up\n"
   "Keyboard control:\n"
   "\n"
   "      Patterns:    Scale Degrees:\n"
   "       123456          67890\n"
   "       qwerty          yuiop\n"
   "       asdfgh\n"
   "       zxcvbn\n"
   << endl;
} 



//////////////////////////////
//
// initialization --
//

void initialization(void) {
   voice.setPort(synth.getPort());
   voice.setChannel(0);

   degrees.setSize(7);
   degrees.allowGrowth(0);
   // put mode in for default:
   // major: {0, 2, 4, 5, 7, 9, 11}
   // koto: {0, 1, 5, 6, 7,  8, 10}
   degrees[0] = 0;
   degrees[1] = 1;
   degrees[2] = 5;
   degrees[3] = 6;
   degrees[4] = 7;
   degrees[5] = 8;
   degrees[6] = 10;

   positions.setSize(5);
   positions.allowGrowth(0);
   positions[0] = 0;
   positions[1] = 1;
   positions[2] = 2;
   positions[3] = 4;
   positions[4] = 5;

   clearpatterns(degpatterns);
   readfile("k-pattern.txt");
}



//////////////////////////////
//
// stick1trig -- chooses next note to play based
//   on pattern chosen by stick2trig.
//

void stick1trig(void) { 
   static int oldsd = -1;
   int xchoice = midiscale(baton.x1t, 0, 4);
   int ychoice = midiscale(baton.y1t, 0, 1);

   csd = positions[xchoice]; 

   if (csd != oldsd) {
      oldsd = csd;
      cycle = 0;
      cpat = npat;
      csub = nsub;
      cout << "Starting pattern " << cpat << (csub == 0 ? 'a' : 'b')
           << " on scale degree " << csd + 1 << endl;
   }

   if (cycle >= 4) {
      cycle = 0;
   }

   if (cycle == 0) {
      cpat = npat;
      csub = nsub;
   }

   int octave = ychoice + 4;
   int basepitch = degrees[csd] + 12 * octave + transpose; 
   int note = basepitch + degpatterns[csd][cpat][csub][cycle++];

   voice.play(note, 127);
   cout << "playing note: " << note << "\t(based on s"<< csd+1 
        << "-" << cpat << (csub == 0? 'a':'b')
        << "[" << cycle << "])" << endl;
}



//////////////////////////////
//
// stick2trig -- chooses the melodic pattern
//

void stick2trig(void) { 
   int xchoice = midiscale(baton.x2t, 0, 4);
   int ychoice = midiscale(baton.y2t, 0, 3);

   switch (10 * ychoice + xchoice) {
      case 0:	npat = 4; nsub = 1; break;
      case 1:	npat = 5; nsub = 1; break;
      case 2:	npat = 6; nsub = 1; break;
      case 3:	npat = 7; nsub = 1; break;
      case 4:	voice.off(); break;
      case 10:	npat = 4; nsub = 0; break;
      case 11:	npat = 5; nsub = 0; break;
      case 12:	npat = 6; nsub = 0; break;
      case 13:	npat = 7; nsub = 0; break;
      case 14:	voice.off(); break;
      case 20:	npat = 0; nsub = 1; break;
      case 21:	npat = 1; nsub = 1; break;
      case 22:	npat = 2; nsub = 1; break;
      case 23:	npat = 3; nsub = 1; break;
      case 24:	voice.off(); break;
      case 30:	npat = 0; nsub = 0; break;
      case 31:	npat = 1; nsub = 0; break;
      case 32:	npat = 2; nsub = 0; break;
      case 33:	npat = 3; nsub = 0; break;
      case 34:	voice.off(); break;
   }

   cout << "Next Pattern: " << npat << (nsub == 0? 'a' : 'b') << endl;
}



//////////////////////////////
//
// keyboardchar --
//

void keyboardchar(int key) { 
   int i;
   switch (key) {
      case '\\': printpatterns(); break;
      case ' ': 
         cout << "current pattern:\ts" << csd+1 << ":"
              << cpat << (csub == 0? 'a':'b') << "=[ ";
         for (i=1; i<4; i++) {
            cout << degpatterns[csd][cpat][csub][i] - 
                    degpatterns[csd][cpat][csub][i-1];
            if (i<3) {
               cout << ", ";
            } else { 
               cout << " ";
            }
         }
         cout << "]" << endl;
         break;
      case ',': clearpatterns(degpatterns); readfile("k-pattern.txt"); break;
      case '.': clearpatterns(degpatterns); readfile("c-pattern.txt"); break;
      case '/': clearpatterns(degpatterns); readfile("g-pattern.txt"); break;

      case '[': transpose--; cout << "Tonic: " << transpose << endl; break;
      case ']': transpose++; cout << "Tonic: " << transpose << endl; break;

      case '1': baton.x2t =  12; baton.y2t = 112; stick2trig(); break;
      case '2': baton.x2t =  36; baton.y2t = 112; stick2trig(); break;
      case '3': baton.x2t =  60; baton.y2t = 112; stick2trig(); break;
      case '4': baton.x2t =  84; baton.y2t = 112; stick2trig(); break;
      case '5': baton.x2t = 108; baton.y2t = 112; stick2trig(); break;
      case 'q': baton.x2t =  12; baton.y2t =  80; stick2trig(); break;
      case 'w': baton.x2t =  36; baton.y2t =  80; stick2trig(); break;
      case 'e': baton.x2t =  60; baton.y2t =  80; stick2trig(); break;
      case 'r': baton.x2t =  84; baton.y2t =  80; stick2trig(); break;
      case 't': baton.x2t = 108; baton.y2t =  80; stick2trig(); break;
      case 'a': baton.x2t =  12; baton.y2t =  48; stick2trig(); break;
      case 's': baton.x2t =  36; baton.y2t =  48; stick2trig(); break;
      case 'd': baton.x2t =  60; baton.y2t =  48; stick2trig(); break;
      case 'f': baton.x2t =  84; baton.y2t =  48; stick2trig(); break;
      case 'g': baton.x2t = 108; baton.y2t =  48; stick2trig(); break;
      case 'z': baton.x2t =  12; baton.y2t =  16; stick2trig(); break;
      case 'x': baton.x2t =  36; baton.y2t =  16; stick2trig(); break;
      case 'c': baton.x2t =  60; baton.y2t =  16; stick2trig(); break;
      case 'v': baton.x2t =  84; baton.y2t =  16; stick2trig(); break;
      case 'b': baton.x2t = 108; baton.y2t =  16; stick2trig(); break;

      case '6': baton.x1t =  12; baton.y1t = 112; stick1trig(); break;
      case '7': baton.x1t =  36; baton.y1t = 112; stick1trig(); break;
      case '8': baton.x1t =  60; baton.y1t = 112; stick1trig(); break;
      case '9': baton.x1t =  84; baton.y1t = 112; stick1trig(); break;
      case '0': baton.x1t = 108; baton.y1t = 112; stick1trig(); break;
      case 'y': baton.x1t =  12; baton.y1t =  16; stick1trig(); break;
      case 'u': baton.x1t =  36; baton.y1t =  16; stick1trig(); break;
      case 'i': baton.x1t =  60; baton.y1t =  16; stick1trig(); break;
      case 'o': baton.x1t =  84; baton.y1t =  16; stick1trig(); break;
      case 'p': baton.x1t = 108; baton.y1t =  16; stick1trig(); break;
      
   }
}


//
// Other improv interface functions
//

void b14plustrig(void) { }
void b15plustrig(void) { }
void b14minusuptrig(void) { }
void b14minusdowntrig(void) { }
void b15minusuptrig(void) { }
void b15minusdowntrig(void) { }
void finishup(void) { }
void mainloopalgorithms(void) { }


//
//
///////////////////////////////////////////////////////////////////////////



//////////////////////////////
//
// clearpatterns --
//

void clearpatterns(Array<Array<Array<Array<int> > > >& dp) {
   int sd, pat, sub, i;

   dp.setSize(7);
   dp.allowGrowth(0);
   for (sd=0; sd<dp.getSize(); sd++) {
      dp[sd].setSize(8);
      dp[sd].allowGrowth(0);
      for (pat=0; pat<dp[sd].getSize(); pat++) {
         dp[sd][pat].setSize(2);
         dp[sd][pat].allowGrowth(0);
         for (sub=0; sub<dp[sd][pat].getSize(); sub++) {
            dp[sd][pat][sub].setSize(4);
            dp[sd][pat][sub].allowGrowth(0);
            for (i=0; i<dp[sd][pat][sub].getSize(); i++) {
               dp[sd][pat][sub][i] = 0;
            }
         }
      }
   }
}



//////////////////////////////
//
// readfile --
//

void readfile(const char* filename) {

   ifstream infile(filename);
   if (!infile.is_open()) {
      cout << "Cannot open file: " << filename << endl;
      return;
   }

   char buffer[2048] = {0};
   infile.getline(buffer, 256, '\n'); 
   while (!infile.eof()) {
      processLine(buffer);
      infile.getline(buffer, 256, '\n'); 
   }
}



//////////////////////////////
//
// processLine --
//

void processLine(const char* line) {
   int length = strlen(line);
   if (length > 256) {
      return;
   }
   char buffer[2048] = {0};
   strcpy(buffer, line);
   char* ptr = strtok(buffer, " \t\n");
   if (ptr == NULL) {
      // cout << "nothing on line" << endl;
      return;
   }

   if (ptr[0] == '#') {
      // print comment line
      cout << line << endl;
      return;
   }
 
   // first data field must start with an 's' or 'p':
   if (ptr[0] == 'S') {
      ptr[0] = 's';
   }
   if (ptr[0] == 'P') {
      ptr[0] = 'p';
   }
   if ((ptr[0] != 's') && (ptr[0] != 'p')) {
      return;
   }

   int scaledegree = -1;
   int position = -1;
   int pattern = -1;
   int count = 0;
   int tpose;

   if (strchr(ptr, '=') != NULL) {
      if (ptr[0] == 'p') {
         count = sscanf(ptr, "p%d=s%d", &position, &scaledegree);
         if (count != 2) {
            return;
         }
         if (count != 2) {
            cout << "First token invalid" << endl;
            return;
         }
         position--;
         scaledegree--;
         positions[position] = scaledegree;
         return;
      } else {
         count = sscanf(ptr, "s%d=%d", &scaledegree, &tpose);
         if (count != 2) {
            return;
         }
   //    cout << "SCALE DEGREE " << scaledegree << " is " << tpose
   //         << " half-steps above the tonic." << endl;
   
         if (scaledegree < 1 || scaledegree > 7) {
            cout << "Scale degree out of range: " << scaledegree << endl;
         }
         scaledegree = scaledegree - 1;   // offset from zero
    
         if (scaledegree != 0) {
            degrees[scaledegree] = tpose;
         } else {
            transpose = tpose;
         }
         return;
      }
   } else {
      count = sscanf(ptr, "s%d:%d", &scaledegree, &pattern);
      if (count != 2) {
         cout << "First token invalid" << endl;
         return;
      }
   }

   char subpattern = ' ';
   int len2 = strlen(ptr);
   if (isalpha(ptr[len2-1])) {
      subpattern = tolower(ptr[len2-1]);
   }

// cout << "S=" << scaledegree << "\tP=" << pattern 
//      << "\tV=" << subpattern;


   scaledegree = scaledegree - 1;   // offset from zero
   if (scaledegree < 0 || scaledegree > 6) {
      cout << "Scale degree is out of range:" << scaledegree << endl;
      return;
   }

   if (pattern < 0 || pattern > 7) {
      cout << "Pattern is out of range:" << pattern << endl;
      return;
   }

   if (subpattern != ' ' && subpattern != 'a' && subpattern != 'b') {
      cout << "Sub pattern is out of range:" << subpattern << endl;
      return;
   }

   ptr = strtok(NULL, " ,:\t\n;");
   int degree;
   int icount = 0;
   Array<int> intervals;
   intervals.setSize(3);
   intervals.setAll(0);
   intervals.allowGrowth(0);
   while (ptr != NULL) {
      count = sscanf(ptr, "%d", &degree);
      if (count == 1) {
         // cout << "\t" << degree << "\t";
         intervals[icount++] = degree;
      }
      ptr = strtok(NULL, " ,:\t\n;");
      if (icount >= 3) {  // only 3 intervals allowed
         break;    
      }
   }
   // cout << endl;

   int i;
   if (subpattern == 'a' || subpattern == ' ') {
      degpatterns[scaledegree][pattern][0][0] = 0;
      for (i=1; i<4; i++) {
         degpatterns[scaledegree][pattern][0][i] = 
               degpatterns[scaledegree][pattern][0][i-1] + intervals[i-1];
      }
   }
   if (subpattern == 'b' || subpattern == ' ') {
      degpatterns[scaledegree][pattern][1][0] = 0;
      for (i=1; i<4; i++) {
         degpatterns[scaledegree][pattern][1][i] = 
               degpatterns[scaledegree][pattern][1][i-1] + intervals[i-1];
      }
   }
}



//////////////////////////////
//
// printpatterns --
//

void printpatterns(void) {
   int deg, pat, i;

   for (deg = 0; deg < 7; deg++) {
      for (pat = 0; pat < 8; pat++) {
         cout << "s" << deg+1 << ":" << pat << " :: a = ";
         for (i=1; i<4; i++) {
            cout << degpatterns[deg][pat][0][i] -
                    degpatterns[deg][pat][0][i-1] << " ";
         }
         cout << "\tb =";
         for (i=1; i<4; i++) {
            cout << degpatterns[deg][pat][1][i] -
                    degpatterns[deg][pat][1][i-1] << " ";
         }
         cout << endl;
      }
   }
}



/*  sample pattern file:

# Chinese melodic patterns

p1=s1	# position 1 = scale degree 1
p2=s2	# position 2 = scale degree 2
p3=s3	# position 3 = scale degree 3
p4=s5	# position 4 = scale degree 5
p5=s6	# position 5 = scale degree 6

s1=2    # scale degree 1 is 2 half-steps above C (i.e.: tonic is on D)
s2=2	# scale degree 2 is 2 half-step  above the tonic (s1)
s3=4	# scale degree 3 is 4 half-steps above the tonic (s1)
s5=7	# scale degree 5 is 7 half-steps above the tonic (s1)
s6=9	# scale degree 6 is 9 half-steps above the tonic (s1)

s1:0	-3 -2 -3
s1:1a	-3 -2 2
s1:1b	-3 -2 5
s1:2	-3 3 -3
s1:3	-3 3 2
s1:4	2 -2 -3
s1:5	2 -2 2
s1:6	2 2 -2
s1:7	2 2 3

s2:0	-2 -3 -2
s2:1a	-2 -3 3
s2:1b	-2 -3 5
s2:2	-2 2 -2
s2:3	-2 2 2
s2:4a	2 -2 -2
s2:4b	5 -3 -2
s2:5a	2 -2 2
s2:5b	2 -4 2
s2:6a	2 3 -3
s2:6b	2 3 -5
s2:7	2 3 2

s3:0	-2 -2 -3
s3:1	-2 -2 2
s3:2	-2 2 -2
s3:3	-2 2 3
s3:4	3 -3 -2
s3:5a	3 -3 3
s3:5b	3 -5 2
s3:6	3 2 -2
s3:7	3 2 3

s5:0	-3 -2 -2
s5:1a	-3 -2 2
s5:1b	-3 -2 5
s5:2	-3 3 -3
s5:3	-3 3 2
s5:4a	2 -2 -3
s5:4b	5 -3 -2
s5:5	2 -2 2
s5:6	2 3 -5
s5:7	2 3 2

s6:0	-2 -3 -2
s6:1a	-2 -3 3
s6:1b	-2 -3 5
s6:2	-2 2 -2
s6:3	-2 2 3
s6:4	3 -3 -2
s6:5a	3 -3 3
s6:5b	3 -5 2
s6:6	3 2 -2
s6:7	3 2 2

*/



// md5sum: 47cb15d6c3221eef0686ba7e65938192 kotomel2.cpp [20050403]
