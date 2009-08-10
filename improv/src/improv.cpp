//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu> (from 18Dec1997)
// Creation Date: 4 January 1997
// Last Modified: Wed Feb 11 22:57:46 GMT-0800 1998
// Last Modified: Sun Jun 21 10:53:47 PDT 2009 (updated for GCC 4.3)
//
// Filename:      ...sig/code/control/improv/improv.cpp
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/src/improv.cpp
// Syntax:        C++
//
// Description:   Basic helper functions for use in all improv environments.
//

#include "improv.h"
#include <stdlib.h>
#include <string.h>

#ifndef OLDCPP
   #include <iostream>
   #include <iomanip>
   using namespace std;
#else
   #include <iostream.h>
   #include <iomanip.h>
#endif

#ifndef VISUAL
   #define BOX_UL '+'
   #define BOX_UR '+'
   #define BOX_BL '+'  
   #define BOX_BR '+'  
   #define BOX_HL '-'
   #define BOX_DHL '='
   #define BOX_VL '|'
   #define BOX_DVL '|'
   #define BOX_JL '+'
   #define BOX_JR '+'
#else
   #define BOX_UL '\311'
   #define BOX_UR '\273'
   #define BOX_BL '\310'  
   #define BOX_BR '\274'  
   #define BOX_HL '\304'
   #define BOX_DHL '\315'
   #define BOX_VL '|'
   #define BOX_DVL '\272'
   #define BOX_JL '\307'
   #define BOX_JR '\266'
#endif
 

//////////////////////////////
//
// displayInputDevices -- show the user the MIDI input device to 
//	communicate with the specified object.
//

void displayInputPorts(void) {
   MidiInPort tempmidi;
   for (int i=0; i<tempmidi.getNumPorts(); i++) {
      tempmidi.setPort(i);
      cout << "   " << i << ": " << tempmidi.getName() << endl;
   }
}



//////////////////////////////
//
// displayOutputDevices -- show the user the MIDI ouptut device to 
//	communicate with the specified object.
//

void displayOutputPorts(void) {
   MidiOutPort tempmidi;
   for (int i=0; i<tempmidi.getNumPorts(); i++) {
      tempmidi.setPort(i);
      cout << "   " << i << ": " << tempmidi.getName() << endl;
   }
}



//////////////////////////////
//
// printboxtop -- prints a text-graphic line forming the top
//	of a box
//

void printboxtop(void) {
   cout << BOX_UL;
   for (int i=0; i<77; i++) {
      cout << BOX_DHL;
   }
   cout << BOX_UR << endl;
}


void printtopline(void) {
   printboxtop();
}


//////////////////////////////
//
// printboxbottom -- prints a text-graphic line forming the 
//	bottom of a box.
//

void printboxbottom(void) {
   cout << BOX_BL;
   for (int i=0; i<77; i++) {
      cout << BOX_DHL;
   }
   cout << BOX_BR << endl;
}


void printbottomline(void) {
   printboxbottom();
}



//////////////////////////////
//
// printintermediateline -- prints a text-graphic line forming
//	a single horizontal line connecting two sides of a
//	double-lined box.
//

void printintermediateline(void) {
   cout << BOX_JL;
   for (int i=0; i<77; i++) {
      cout << BOX_HL;
   }
   cout << BOX_JR << endl;
}



//////////////////////////////
//
// printstringline -- prints text inside of an 80-character
//	wide double-lined box.
//

void printstringline(const char* aString) {
   int spaceCount = 77 - strlen(aString);
   cout << BOX_DVL;
   cout << aString;
   for (int i=0; i<spaceCount; i++) {
      cout << ' ';
   }
   cout << BOX_DVL << endl;
}


void psl(const char* aString) {
   printstringline(aString);
}



//////////////////////////////
//
// printcenterline -- centers text inside of an 80-character
//	wide double-lined box.
//

void printcenterline(const char* aString) {
   int spaceCount = 77 - strlen(aString);
   int leftCount = spaceCount/2;
   int rightCount = spaceCount - leftCount;
   int i;
   cout << BOX_DVL;
   for (i=0; i<leftCount; i++) {
      cout << ' ';
   }
   cout << aString;
   for (i=0; i<rightCount; i++) {
      cout << ' ';
   }
   cout << BOX_DVL;
   cout << endl;
}


void pcl(const char* aString) {
   printcenterline(aString);
}



//////////////////////////////
//
// midiscale -- converts a value in the range from 0 to 127
//      into a number in a new range.  For example the value
//      64 scaled to the range from 0 to 2 would be 1.
//

int midiscale(int value, int min, int max) {
   return value >= 127 ? max : (int)(value/127.0*(max-min+1)+min);
}



//////////////////////////////
//
// GeneralMidiName -- returns the instrument name of the
//     index value.
//

const char* GeneralMidiName(int index) {
   static const char* names[] = {
   "Acoustic Grand Piano",     /* 000 */     /* Pianos */
   "Bright Acoustic Piano",    /* 001 */
   "Electric grand Piano",     /* 002 */
   "Honky Tonk Piano",         /* 003 */
   "Electric Piano 1",         /* 004 */
   "Electric Piano 2",         /* 005 */
   "Harpsichord",              /* 006 */
   "Clavinet",                 /* 007 */
   "Celesta",                  /* 008 */     /* Chromatic Percussion */
   "Glockenspiel",             /* 009 */
   "Music Box",                /* 010 */
   "Vibraphone",               /* 011 */
   "Marimba",                  /* 012 */
   "Xylophone",                /* 013 */
   "Tubular bells",            /* 014 */
   "Dulcimer",                 /* 015 */
   "Drawbar Organ",            /* 016 */     /* Organs */
   "Percussive Organ",         /* 017 */
   "Rock Organ",               /* 018 */
   "Church Organ",             /* 019 */
   "Reed Organ",               /* 020 */
   "Accordion",                /* 021 */
   "Harmonica",                /* 022 */
   "Tango Accordion",          /* 023 */
   "Nylo n Accustic Guitar",   /* 024 */     /* Guitars */
   "Steel Acoustic Guitar",    /* 025 */
   "Jazz Electric Guitar",     /* 026 */
   "Clean Electric Guitar",    /* 027 */
   "Muted Electric Guitar",    /* 028 */
   "Overdrive Guitar",         /* 029 */
   "Distorted Guitar",         /* 030 */
   "Guitar Harmonics",         /* 031 */
   "Acoustic Bass",            /* 032 */     /* Basses */
   "Electric Fingered Bass",   /* 033 */
   "Electric Picked Bass",     /* 034 */
   "Fretless Bass",            /* 035 */
   "Slap Bass 1",              /* 036 */
   "Slap Bass 2",              /* 037 */
   "Syn Bass 1",               /* 038 */
   "Syn Bass 2",               /* 039 */
   "Violin",                   /* 040 */     /* Strings/Orchestra */
   "Viola",                    /* 041 */
   "Cello",                    /* 042 */
   "Contrabass",               /* 043 */
   "Tremolo Strings",          /* 044 */
   "Pizzicato Strings",        /* 045 */
   "Orchestral Harp",          /* 046 */
   "Timpani",                  /* 047 */
   "String Ensemble 1",        /* 048 */     /* Ensembles */
   "String Ensemble 2 (Slow)", /* 049 */
   "Syn Strings 1",            /* 050 */
   "Syn Strings 2",            /* 051 */
   "Choir Aahs",               /* 052 */
   "Voice Oohs",               /* 053 */
   "Syn Choir",                /* 054 */
   "Orchestral Hit",           /* 055 */
   "Trumpet",                  /* 056 */     /* Brass */
   "Trombone",                 /* 057 */
   "Tuba",                     /* 058 */
   "Muted Trumpet",            /* 059 */
   "French Horn",              /* 060 */
   "Brass Section",            /* 061 */
   "Syn Brass 1",              /* 062 */
   "Syn Brass 2",              /* 063 */
   "Soprano Sax",              /* 064 */     /* Reeds */
   "Alto Sax",                 /* 065 */
   "Tenor Sax",                /* 066 */
   "Baritone Sax",             /* 067 */
   "Oboe",                     /* 068 */
   "English Horn",             /* 069 */
   "Bassoon",                  /* 070 */
   "Clarinet",                 /* 071 */
   "Piccolo",                  /* 072 */     /* Pipes */
   "Flute",                    /* 073 */
   "Recorder",                 /* 074 */
   "Pan Flute",                /* 075 */
   "Bottle Blow",              /* 076 */
   "Shakuhachi",               /* 077 */
   "Whistle",                  /* 078 */
   "Ocarina",                  /* 079 */
   "Syn Square Wave",          /* 080 */     /* Synth Leads */
   "Syn Sawtooth Wave",        /* 081 */
   "Syn Calliope",             /* 082 */
   "Syn Chiff",                /* 083 */
   "Syn Charang",              /* 084 */
   "Syn Voice",                /* 085 */
   "Syn Fifths Sawtooth Wave", /* 086 */
   "Syn Brass & Lead",         /* 087 */
   "New Age Syn Pad",          /* 088 */     /* Synth Pads */
   "Warm Syn Pad",             /* 089 */
   "Polysynth Syn Pad",        /* 090 */
   "Choir Syn Pad",            /* 091 */
   "Bowed Syn Pad",            /* 092 */
   "Metal Syn Pad",            /* 093 */
   "Halo Syn Pad",             /* 094 */
   "Sweep Syn Pad",            /* 095 */
   "SFX Rain",                 /* 096 */     /* Synth Effects */
   "SFX Soundtrack",           /* 097 */
   "SFX Crystal",              /* 098 */
   "SFX Atmosphere",           /* 099 */
   "SFX Brightness",           /* 100 */
   "SFX Goblins",              /* 101 */
   "SFX Echoes",               /* 102 */
   "SFX Sci-fi",               /* 103 */
   "Sitar",                    /* 104 */     /* Ethnic */
   "Banjo",                    /* 105 */
   "Shamisen",                 /* 106 */
   "Koto",                     /* 107 */
   "Kalimba",                  /* 108 */
   "Bag Pipe",                 /* 109 */
   "Fiddle",                   /* 110 */
   "Shanai",                   /* 111 */
   "Tinkle Bell",              /* 112 */     /* Percussive */
   "Agogo",                    /* 113 */
   "Steel Drums",              /* 114 */
   "Woodblock",                /* 115 */
   "Taiko Drum",               /* 116 */
   "Melodic Tom",              /* 117 */
   "Syn Drum",                 /* 118 */
   "Reverse Cymbal",           /* 119 */
   "Guitar Fret Noise",        /* 120 */     /* Sound Effects */
   "Breath Noise",             /* 121 */
   "Seashore",                 /* 122 */
   "Bird Tweet",               /* 123 */
   "Telephone Ring",           /* 124 */
   "Helicopter",               /* 125 */
   "Applause",                 /* 126 */
   "Gun Shot"};                /* 127 */

   return names[index % 128];
}


// md5sum: 6909f61b4685ad926f9b43cbfc4e9d0f improv.cpp [20050403]
