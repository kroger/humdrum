//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Dec 30 16:44:21 PST 2002
// Last Modified: Wed Jan  1 11:05:56 PST 2003
// Filename:      ...sig/doc/examples/improv/synthImprov/rp.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: Test performance program for "Responsoria prolixa"
//

#include "synthImprov.h"      
#include <string.h>

class NoteData { public: double dur; double start; int voice; int vel; };
typedef Array<Array<Array<NoteData> > > NoteScore;

// function declarations:
int  midilimit         (int input);
void readScore         (NoteScore& score);
void printScore        (NoteScore& score);
void readScoreFile     (const char* filename, Array<NoteData>& data);
void playNextPattern   (EventBuffer& eventbuffer, NoteScore& score, 
                        int pattern, int subpattern, double tempo);

// global variables:
double      tempo           = 80.0;    // tempo for the performance
int         nextpattern     =  1;      // next pattern group to play
int         nextsubpattern  =  0;      // next subpattern to play in group
int         nextpatterntime =  0;      // time to play the next (sub)pattern
int         patternbeats    =  0;      // number of beats in pattern
EventBuffer eventbuffer;               // storage for notes to play in pattern
Array<int>  voicetokey;                // MIDI note number for each voice
NoteScore   rpscore;                   // Score for reading during performance
Array<int>  activevoice;               // used to turn voices on/off


//////////////////////////////////////////////////////////////////////////

void description(void) {
   cout << "Test performance program for Responsoria prolixa" << endl;
   cout << "Type 0-6 to control pattern level               " << endl;
   cout << "     a-d to control pattern sublevel            " << endl;
   cout << "     [ to slow tempo, and ] to speed up tempo   " << endl;
   cout << "     space to restart cycle                     " << endl;
   cout << "     q, w, e, r, t and then - or + for timbre   " << endl;
   cout << "     y, u, i, o, p to turn voices on/off        " << endl;
   cout << "     / to print score                           " << endl;
} 

void initialization(void) { 
   eventbuffer.setPort(synth.getInputPort());
   nextpatterntime = t_time;
   voicetokey.setSize(5);
   readScore(rpscore);
   voicetokey[0] = GM_CLAVES;
   voicetokey[1] = GM_COWBELL;
   voicetokey[2] = GM_HIGH_AGOGO;
   voicetokey[3] = GM_LOW_AGOGO;
   voicetokey[4] = GM_HI_BONGO;
   activevoice.setSize(5);
   activevoice.setAll(1);
}

void finishup(void) { }

void mainloopalgorithms(void) { 
   eventbuffer.checkPoll();
   if (nextpatterntime <= t_time) {
      patternbeats = nextpattern + 2;
      if (nextpattern == 0) {
         patternbeats = 3;
      }
      nextpatterntime = t_time + (int)(60.0/tempo*1000*patternbeats + 0.5);
      playNextPattern(eventbuffer, rpscore, nextpattern, nextsubpattern, tempo);
   }
}

void keyboardchar(int key) { 
   static int controlvoice = 0;

   switch (key) {
      case '0':   nextpattern = 0;     break;  // select level 0
      case '1':   nextpattern = 1;     break;  // select level 1
      case '2':   nextpattern = 2;     break;  // select level 2
      case '3':   nextpattern = 3;     break;  // select level 3
      case '4':   nextpattern = 4;     break;  // select level 4
      case '5':   nextpattern = 5;     break;  // select level 5
      case '6':   nextpattern = 6;     break;  // select level 6 (cadences)
      case 'a':   nextsubpattern = 0;  break;  // select subgroup a
      case 'b':   nextsubpattern = 1;  break;  // select subgroup b
      case 'c':   nextsubpattern = 2;  break;  // select subgroup c
      case 'd':   nextsubpattern = 3;  break;  // select subgroup d
      case '/':   printScore(rpscore); break;  // for debugging
      case '[':   // slow down the tempo
         tempo *= 0.99;
         cout << "Tempo = " << tempo << endl;
         break;
      case ']':   // speed up the tempo
         tempo *= 1.01;
         cout << "Tempo = " << tempo << endl;
         break;
      case '-':   // lower the key number for the given voice
         voicetokey[controlvoice]--;
         voicetokey[controlvoice] = midilimit(voicetokey[controlvoice]);
         cout << "Voice: " << controlvoice << "\tkey: " 
              << voicetokey[controlvoice] << endl;
         break;
      case '+':   // raise the key number for the given voice
      case '=':   // same as '+' without the shift key
         voicetokey[controlvoice]++;
         voicetokey[controlvoice] = midilimit(voicetokey[controlvoice]);
         cout << "Voice: " << controlvoice << "\tkey: " 
              << voicetokey[controlvoice] << endl;
         break;
      case ' ':   // restart the pattern cycle
         nextpatterntime = t_time;
         break;
      case 'q':  controlvoice = 0; break; // control timber of voice 0 with -/=
      case 'w':  controlvoice = 1; break; // control timber of voice 1 with -/=
      case 'e':  controlvoice = 2; break; // control timber of voice 2 with -/=
      case 'r':  controlvoice = 3; break; // control timber of voice 3 with -/=
      case 't':  controlvoice = 4; break; // control timber of voice 4 with -/=
      case 'y':   // turn voice 0 on/off
         activevoice[0] = !activevoice[0];
         cout << "Voice 0 = " << activevoice[0] << endl;
         break;
      case 'u':   // turn voice 1 on/off
         activevoice[1] = !activevoice[1];
         cout << "Voice 1 = " << activevoice[1] << endl;
         break;
      case 'i':   // turn voice 2 on/off
         activevoice[2] = !activevoice[2];
         cout << "Voice 2 = " << activevoice[2] << endl;
         break;
      case 'o':   // turn voice 3 on/off
         activevoice[3] = !activevoice[3];
         cout << "Voice 3 = " << activevoice[3] << endl;
         break;
      case 'p':   // turn voice 4 on/off
         activevoice[4] = !activevoice[4];
         cout << "Voice 4 = " << activevoice[4] << endl;
         break;
   }
}


//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// midilimit -- limit an integer into the range from 0 to 127.
//

int midilimit(int input) {
   if (input < 0) {
      return 0;
   } else if (input > 127) {
      return 127;
   } else {
      return input;
   }
}



//////////////////////////////
//
// playNextPattern -- prepare the next measure of music.
//

void playNextPattern(EventBuffer& eventbuffer, NoteScore& score, 
      int pattern, int subpattern, double tempo) {
   if (pattern == 0 && subpattern == 3) {
      subpattern = 2;
   }
   if (pattern == 6 && subpattern == 3) {
      subpattern = 2;
   }
   int p  = pattern;
   int sp = subpattern;
   cout << "playing pattern " << pattern << (char)('a' + subpattern) << endl;

   NoteEvent note;   // temporary note for placing in buffer
   int i;
   for (i=0; i<score[pattern][subpattern].getSize(); i++) {
      if (!activevoice[score[pattern][subpattern][i].voice]) {
         continue;
      }
      note.setOnDur((int)(t_time + score[p][sp][i].start * 1000 * 60 / tempo + 
            0.5), (int)(score[p][sp][i].dur * 1000 * 60 / tempo + 0.5));
      note.setChan(CH_10);
      note.setKey(voicetokey[score[p][sp][i].voice]);
      note.setVel(score[p][sp][i].vel);
      note.activate();
      if (score[p][sp][i].start == 0.0) {
         note.action(eventbuffer);   // play starting notes now, avoiding delay
      }
      eventbuffer.insert(note);
   }
}



//////////////////////////////
//
// readScore -- read in the Responsoria Prolixa score
//

void readScore(NoteScore& score) {
   score.setSize(7);
   score[0].setSize(3); score[1].setSize(4);
   score[2].setSize(4); score[3].setSize(4);
   score[4].setSize(4); score[5].setSize(4);
   score[6].setSize(3);
   Array<NoteData> data;
   data.setSize(100);
   data.setGrowth(100);
   data.setSize(0);
   readScoreFile("rp0a.txt", data); score[0][0] = data;
   readScoreFile("rp0b.txt", data); score[0][1] = data;
   readScoreFile("rp0c.txt", data); score[0][2] = data;
   readScoreFile("rp1a.txt", data); score[1][0] = data;
   readScoreFile("rp1b.txt", data); score[1][1] = data;
   readScoreFile("rp1c.txt", data); score[1][2] = data;
   readScoreFile("rp1d.txt", data); score[1][3] = data;
   readScoreFile("rp2a.txt", data); score[2][0] = data;
   readScoreFile("rp2b.txt", data); score[2][1] = data;
   readScoreFile("rp2c.txt", data); score[2][2] = data;
   readScoreFile("rp2d.txt", data); score[2][3] = data;
   readScoreFile("rp3a.txt", data); score[3][0] = data;
   readScoreFile("rp3b.txt", data); score[3][1] = data;
   readScoreFile("rp3c.txt", data); score[3][2] = data;
   readScoreFile("rp3d.txt", data); score[3][3] = data;
   readScoreFile("rp4a.txt", data); score[4][0] = data;
   readScoreFile("rp4b.txt", data); score[4][1] = data;
   readScoreFile("rp4c.txt", data); score[4][2] = data;
   readScoreFile("rp4d.txt", data); score[4][3] = data;
   readScoreFile("rp5a.txt", data); score[5][0] = data;
   readScoreFile("rp5b.txt", data); score[5][1] = data;
   readScoreFile("rp5c.txt", data); score[5][2] = data;
   readScoreFile("rp5d.txt", data); score[5][3] = data;
   readScoreFile("rp6a.txt", data); score[6][0] = data;
   readScoreFile("rp6a.txt", data); score[6][1] = data;
   readScoreFile("rp6a.txt", data); score[6][2] = data;
}



//////////////////////////////
//
// readscorefile -- read a score file.
//

/*  Here is a sample score file.
 *  ordering of data:
 *     note  start-beat duration gm-channel10-key velocity

; Leader
note	0.0	0.5	0	84
note	3.0	1.0	1	64

; Left
note	2.5	0.5	2	64
note	3.5	0.5	2	64
note	4.0	0.75	2	64

*/


void readScoreFile(const char* filename, Array<NoteData>& data) {
   data.setSize(0);
   NoteData n;
   #ifndef OLDCPP
      ifstream input(filename, ios::in);
   #else
      ifstream input(filename, ios::in | ios::nocreate);
   #endif
   if (!input.is_open()) {
      cout << "Error: could not read file: " << filename << endl;
      exit(1);
   }
   char buffer[1024] = {0};
   int count;
   while (!input.eof()) {
      input.getline(buffer, 1000, '\n'); 
      if (strncmp(buffer, "note\t", 5) == 0) {
         count = sscanf(buffer, "note\t%lf\t%lf\t%d\t%d", &n.start, &n.dur, 
               &n.voice, &n.vel);
         if (count == 4) {
            data.append(n);
         }
      }
   }
}



//////////////////////////////
//
// printScore -- print the input score (for debugging purposes).
//

void printScore(NoteScore& score) {
   int level;
   int sublevel;
   int note;
   for (level=0; level<score.getSize(); level++) {
      for (sublevel=0; sublevel<score[level].getSize(); sublevel++) {
         for (note=0; note<score[level][sublevel].getSize(); note++) {
            cout << level << (char)('a' + sublevel) << "\t"
                 << score[level][sublevel][note].start << "\t"
                 << score[level][sublevel][note].dur   << "\t"
                 << score[level][sublevel][note].voice << "\t"
                 << score[level][sublevel][note].vel   << "\n";
         }
      }
   }
}



// md5sum: bba113ece30546dbbee31665fc6bbf9a rp.cpp [20050403]
