//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Oct 15 12:40:15 PDT 1998
// Last Modified: Thu Oct 15 13:07:01 PDT 1998
// Filename:      ...sig/doc/examples/improv/synthImprov/yekyek/yekyek.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: Does the opposite of the keykey program
//     you input a text file and the program will play
//     the synthesizer according to the specified letters
//     You can control the interword time and the speed of 
//     the transmission of the characters.  Try extending
//     the program by assigning pitch patterns to the
//     10 transmission keys, and using different rhythmic
//     patterns.
//

#include "synthImprov.h" 

#define INTERFACE_FUNCTION   /* just a label so you can see req. functions */
#define LOCAL_FUNCTION       /* defined just for this program */

/*----------------- beginning of improvization algorithms ---------------*/

int lastNoteTime = 0;   // time at which to play next note pair.
int notePeriod = 100;   // duration of each note
int restPeriod = 300;   // duration of a non-letter character
int duration = 100;     // duration of the current note.

Voice leftHand;         // Voice for left hand notes.
Voice rightHand;        // Voice for right hand notes.

fstream infile;         // input file stream for characters
int source = 0;         // 0 = keyboard; 1 = file
int sounding = 0;       // 0 = rest, 1 = note on.
int character = 0;      // character to send
int volume = 127;       // volume of MIDI

int note[10] = {48, 50, 52, 53, 55, 60, 62, 64, 65, 67}; // notes to play


// function declarations:
void playNotes(int left, int right);
void playCharacter(int aCharacter);
int getCharacter(void);


/*--------------------- maintenance algorithms --------------------------*/

INTERFACE_FUNCTION void description(void) {
   cout << "\nKeykey:  (Craig Stuart Sapp <craig@ccrma.stanford.edu>)"
           "15 Oct 1998\n"
        << "Use the white midi keys C-G below middle C for the left hand\n"
        << "and middle C-G white keys for the right hand\n"
        << "left hand: C=5, D=4, E=3, F=2, G=1\n"
        << "right hand: C=1, D=4, E=3, F=4, G=5\n"
        << "Here are the key combinations (LH col, RH row):\n\n"
        << "     5 E J O T Y\n"
        << "     4 D I N S X        For example:\n"
        << "     3 C H M R W           LH3+RH2 = F\n"
        << "     2 B G L Q V           LH5+RH3 = O\n"
        << "     1 A F K P U           LH1+RH1 = A\n"
        << "       1 2 3 4 5"
        << " Commands:\n"
        << " 1 = keyboard control, 2 = file control\n"
        << " - = speed up,         = = slow down\n"
        << endl;
} 

INTERFACE_FUNCTION void initialization(void) { 
   leftHand.setPort(synth.getOutputPort());
   rightHand.setPort(synth.getOutputPort());
}

INTERFACE_FUNCTION void finishup(void) { 
   leftHand.off();
   rightHand.off();
}


/*-------------------- main loop algorithms -----------------------------*/

INTERFACE_FUNCTION void mainloopalgorithms(void) { 
   if (source && lastNoteTime + duration <= t_time) {
      character = getCharacter();
      if (character >= 0) {
         lastNoteTime = t_time;
         playCharacter(character);
         if (sounding) {
            duration = notePeriod;
         } else {
            duration = restPeriod;
         }
      }
   }
}


LOCAL_FUNCTION int getCharacter(void) {
   int output;
   if (source == 0) {    
      // keyboard characters are handled elsewhere
      return -1;
   } else {
      output = interfaceKeyboard.getch();
      if (infile.eof()) {
         source = 0;
      }
   }

   if (isalpha(output) || output == '\n') {
      cout << tolower(output) << flush;
   }

   return output;
}



LOCAL_FUNCTION void playCharacter(int aCharacter) {
   switch (tolower(aCharacter)) {
      case 'a':   playNotes(1, 1);   sounding = 1; break;
      case 'b':   playNotes(2, 1);   sounding = 1; break;
      case 'c':   playNotes(3, 1);   sounding = 1; break;
      case 'd':   playNotes(4, 1);   sounding = 1; break;
      case 'e':   playNotes(5, 1);   sounding = 1; break;
      case 'f':   playNotes(1, 2);   sounding = 1; break;
      case 'g':   playNotes(2, 2);   sounding = 1; break;
      case 'h':   playNotes(3, 2);   sounding = 1; break;
      case 'i':   playNotes(4, 2);   sounding = 1; break;
      case 'j':   playNotes(5, 2);   sounding = 1; break;
      case 'k':   playNotes(1, 3);   sounding = 1; break;
      case 'l':   playNotes(2, 3);   sounding = 1; break;
      case 'm':   playNotes(3, 3);   sounding = 1; break;
      case 'n':   playNotes(4, 3);   sounding = 1; break;
      case 'o':   playNotes(5, 3);   sounding = 1; break;
      case 'p':   playNotes(1, 4);   sounding = 1; break;
      case 'q':   playNotes(2, 4);   sounding = 1; break;
      case 'r':   playNotes(3, 4);   sounding = 1; break;
      case 's':   playNotes(4, 4);   sounding = 1; break;
      case 't':   playNotes(5, 4);   sounding = 1; break;
      case 'u':   playNotes(1, 5);   sounding = 1; break;
      case 'v':   playNotes(2, 5);   sounding = 1; break;
      case 'w':   playNotes(3, 5);   sounding = 1; break;
      case 'x':   playNotes(4, 5);   sounding = 1; break;
      case 'y':   playNotes(5, 5);   sounding = 1; break;
      default:    playNotes(0, 0);   sounding = 0; break;
   }
}


LOCAL_FUNCTION void playNotes(int left, int right) {
   switch (left) {
      case 0:   leftHand.off();
      case 1:   leftHand.play(note[0], volume);
      case 2:   leftHand.play(note[1], volume);
      case 3:   leftHand.play(note[2], volume);
      case 4:   leftHand.play(note[3], volume);
      case 5:   leftHand.play(note[4], volume);
   }
   switch (right) {
      case 0:   rightHand.off();
      case 1:   rightHand.play(note[0+5], volume);
      case 2:   rightHand.play(note[1+5], volume);
      case 3:   rightHand.play(note[2+5], volume);
      case 4:   rightHand.play(note[3+5], volume);
      case 5:   rightHand.play(note[4+5], volume);
   }
}



/*-------------------- triggered algorithms -----------------------------*/

INTERFACE_FUNCTION void keyboardchar(int key) { 
   cout << tolower(key) << flush;
   playCharacter(key);
}


/*------------------ end improvization algorithms -----------------------*/


// md5sum: 2141ce2c943938b97a28ebc36319f9cd yekyek.cpp [20050403]
