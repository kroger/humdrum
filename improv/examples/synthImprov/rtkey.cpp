//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun May 16 12:54:28 PDT 1999
// Last Modified: Sun May 31 19:49:53 EDT 2009 (added alternative weights)
// Filename:      .../improv/examples/synthImprov/rtkey/rtkey.cpp
// Syntax:        C++; synthImprov 2.1
//  
// Description:   analyze the key of musical input and output the determined
//                key as a note on the tonic of the key.  The certainty of
//                the measurement is mapped to the loudness on the analysis
//                note.
//
// References:    Key analysis is done using the Krumhansl-Schmuckler
//                key-finding algorithm which measures Pearson correlation
//                of the musical data against prototype pitch distributions
//                of major and minor key profiles. The key which produces
//                the highest correlation is determined to be the key.
//

#include "synthImprov.h"      
#include <math.h>

#ifndef OLDCPP
   #include <iostream>
   #include <iomanip>
   using namespace std;
#else
   #include <iostream.h>
   #include <iomanip.h>
#endif


/*----------------- beginning of improvization algorithms ---------------*/


MidiMessage message;            // for extracting notes from the synthesizer

CircularBuffer<char> notes;     // storage for notes being played
CircularBuffer<long> times;     // storage for note times being played
double occurrences[12] = {0};    // number of notes occurrences
int fadeNote          = 0;      // next to to go out of scope
int fadeTime          = 0;      // next time to go out of scope
int keyoctave         = 7;      // the analysis key performance octave
int displayKey2       = 0;      // display the second key possibility
int octave            = 4;      // used for keyboard keyboard
int freezeQ           = 0;      // used with 'f' keyboard command
SigTimer metronome;             // for display period of key analysis
double analysisDuration;        // duration in seconds of analysis window
double tempo;                   // tempo of the rtkeyalysis

// key profile weights: weights repeated twice to make calculations
// simpler for input to pearsonCorrelation().
// Aarden 2003
double AardenEssenMajor[24] = 
   {17.7661, 0.145624, 14.9265, 0.160186, 19.8049, 11.3587, 
    0.291248, 22.062, 0.145624, 8.15494, 0.232998, 4.95122,
    17.7661, 0.145624, 14.9265, 0.160186, 19.8049, 11.3587, 
    0.291248, 22.062, 0.145624, 8.15494, 0.232998, 4.95122};
double AardenEssenMinor[24] = 
   {18.2648, 0.737619, 14.0499, 16.8599, 0.702494, 14.4362,
    0.702494, 18.6161, 4.56621, 1.93186, 7.37619, 1.75623,
    18.2648, 0.737619, 14.0499, 16.8599, 0.702494, 14.4362,
    0.702494, 18.6161, 4.56621, 1.93186, 7.37619, 1.75623};
// Bellman 2005
double BellmanBudgeMajor[24] = 
   {16.80, 0.86, 12.95, 1.41, 13.49, 11.93,
    1.25, 20.28, 1.80, 8.04, 0.62, 10.57,
    16.80, 0.86, 12.95, 1.41, 13.49, 11.93,
    1.25, 20.28, 1.80, 8.04, 0.62, 10.57};
double BellmanBudgeMinor[24] = 
   {18.16, 0.69, 12.99, 13.34, 1.07, 11.15,
    1.38, 21.07, 7.49, 1.53, 0.92, 10.21,
    18.16, 0.69, 12.99, 13.34, 1.07, 11.15,
    1.38, 21.07, 7.49, 1.53, 0.92, 10.21}; 
// Krumhansl & Kessler (1982) weightings
double KrumhanslKesslerMajor[24] = 
   {6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88,
    6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88};
double KrumhanslKesslerMinor[24] = 
   {6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17,
    6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17};
// Craig 2003
double SimpleMajor[24] = {2, 0, 1, 0, 1, 1, 0, 2, 0, 1, 0, 1,
                          2, 0, 1, 0, 1, 1, 0, 2, 0, 1, 0, 1};
double SimpleMinor[24] = {2, 0, 1, 1, 0, 1, 0, 2, 1, 0, 1, 0,
                          2, 0, 1, 1, 0, 1, 0, 2, 1, 0, 1, 0};
// Temperley 2007
double TemperleyMajor[24] = 
   {0.748, 0.060, 0.488, 0.082, 0.670, 0.460,
    0.096, 0.715, 0.104, 0.366, 0.057, 0.400,
    0.748, 0.060, 0.488, 0.082, 0.670, 0.460,
    0.096, 0.715, 0.104, 0.366, 0.057, 0.400};
double TemperleyMinor[24] = 
   {0.712, 0.084, 0.474, 0.618, 0.049, 0.460,
    0.105, 0.747, 0.404, 0.067, 0.133, 0.330,
    0.712, 0.084, 0.474, 0.618, 0.049, 0.460,
    0.105, 0.747, 0.404, 0.067, 0.133, 0.330};

double* majorKey = TemperleyMajor;  // weights for analysis of major keys
double* minorKey = TemperleyMinor;  // weights for analysis of minor keys

Voice  firstVoice;               // for primary key voice
Voice  secondVoice;              // for secondary key voice
double offFraction = 0.75;       // for turning off next keyvoice.

// function declarations:
void        analyzekey           (void);
void        storeNote            (char storeNote, long storeTime);
void        keyboard             (int key);
const char* name                 (int pitch_class);
double      pearsonCorrelation   (int size, double* x, double* y);


/*--------------------- maintenance algorithms --------------------------*/


//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   printboxtop();
   psl("Keyan -- analyze musical input for key");
   psl("");
   psl(" < = Slow down analysis note tempo      > = Speed up analysis note tempo");
   psl(" @ = Toggle playing of secondary choice - = Clear note memory");
   psl(" A = Use Aarden-Essen key profiles      B = Use Bellman-Budge key profiles");
   psl(" P = Use Simple key profiles            T = Use Temperley key profiles");
   psl(" K = Use Krumhansl-Kessler key profiles");
   psl(" ");
   psl(
"      \"0\"-\"9\" = octave number of computer keyboard notes");
   psl("      Notes:           s   d      g    h   j    ");
   psl("                     z   x   c   v   b   n   m  ");
   printboxbottom();

} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     intervace once at the start of the program.  Put items
//     here which need to be initialized at the beginning of
//     the program.
//

void initialization(void) { 
   analysisDuration = 7.0;          // duration in seconds of analysis window
   tempo = 60.0;                     // tempo of the rtkeyalysis
   metronome.setTempo(tempo);
   firstVoice.setChannel(0);
   secondVoice.setChannel(0);
   notes.setSize(10000);
   times.setSize(10000);

}



//////////////////////////////
//
// finishup -- this function is called by the improv interface
//     whenever the program is exited.  Put items here which
//     need to be taken care of when the program is finished.
//

void finishup(void) { 
   firstVoice.off();
   secondVoice.off();
}


/*-------------------- main loop algorithms -----------------------------*/

//////////////////////////////
//
// mainloopalgorithms -- this function is called by the improv interface
//   continuously while the program is running.  The global variable t_time
//   which stores the current time is set just before this function is
//   called and remains constant while in this functions.
//

void mainloopalgorithms(void) { 
   while (synth.getNoteCount() > 0) {
      message = synth.extractNote();
      if (message.p2() != 0) {
         storeNote(message.p1(), t_time);
      }
   }
   if (metronome.expired()) {
      analyzekey();
      metronome.reset();
   }
   if (metronome.getPeriodCount() > offFraction) {
      firstVoice.off();
      secondVoice.off();
   }

}
      

/*-------------------- triggered algorithms -----------------------------*/


//////////////////////////////
//
// keyboard -- simulate a MIDI keyboard on the computer keyboard.
//

void keyboard(int key) {
   static int keyboardnote = -1;   // computer keyboard note
   if (keyboardnote < -1) {
      synth.play(0, keyboardnote, 0);
   }
   switch (key) {
      case 'z': keyboardnote = 12 * octave + 0;  break;    // C
      case 's': keyboardnote = 12 * octave + 1;  break;    // C#
      case 'x': keyboardnote = 12 * octave + 2;  break;    // D
      case 'd': keyboardnote = 12 * octave + 3;  break;    // D#
      case 'c': keyboardnote = 12 * octave + 4;  break;    // E
      case 'v': keyboardnote = 12 * octave + 5;  break;    // F
      case 'g': keyboardnote = 12 * octave + 6;  break;    // F#
      case 'b': keyboardnote = 12 * octave + 7;  break;    // G
      case 'h': keyboardnote = 12 * octave + 8;  break;    // G#
      case 'n': keyboardnote = 12 * octave + 9;  break;    // A
      case 'j': keyboardnote = 12 * octave + 10;  break;   // A#
      case 'm': keyboardnote = 12 * octave + 11;  break;   // B
      case ',': keyboardnote = 12 * octave + 12;  break;   // C
      default: return;
   }
   if (keyboardnote < 0)  keyboardnote = 0;
   else if (keyboardnote > 127)  keyboardnote = 127;
   synth.play(0, keyboardnote, 100);
   storeNote(keyboardnote, t_time);
}



///////////////////////////////
//
// keyboardchar -- this function is called by the improv interface
//     whenever a key is pressed on the computer keyboard.
//     Put commands here which will be executed when a key is
//     pressed on the computer keyboard.
//

void keyboardchar(int key) { 
   switch (key) {
      case 'A':            // Use Aarden-Essen key profiles
         majorKey = AardenEssenMajor;
         minorKey = AardenEssenMinor;
         cout << "Aarden-Essen weights" << endl;
         break;
      case 'B':            // Use Bellman-Budge key profiles
         majorKey = BellmanBudgeMajor;
         minorKey = BellmanBudgeMinor;
         cout << "Bellman-Budge weights" << endl;
         break;
      case 'K':            // Use Krumhansl-Kessler key profiles
         majorKey = KrumhanslKesslerMajor;
         minorKey = KrumhanslKesslerMinor;
         cout << "Krumhansl-Kessler weights" << endl;
         break;
      case 'P':            // Use simPle key profiles
         majorKey = SimpleMajor;
         minorKey = SimpleMinor;
         cout << "Simple weights" << endl;
         break;
      case 'T':            // Use Temperley key profiles
         majorKey = TemperleyMajor;
         minorKey = TemperleyMinor;
         cout << "Temperley weights" << endl;
         break;
      case '-':            // Clear note memory
         notes.reset();
         times.reset();
         fadeTime = 0;
         fadeNote = 0;
         { for (int i=0; i<12; i++) { occurrences[i] = 0; } }
         cout << "Note memory cleared" << endl;
         break;
      case 'f':            // freeze histogram (no deleteing of notes in buffer)
         freezeQ = !freezeQ;
         if (freezeQ) {
            cout << "ANALYSIS HISTOGRAM IS FROZEN" << endl;
         } else {
            cout << "ANALYSIS HISTOGRAM IS UNFROZEN" << endl;
         }
         break;
      case '@':            // for playing the secondary key possibility
         displayKey2 = !displayKey2;         
         if (displayKey2) {
            cout << "Secondary key output turned ON" << endl;
         } else {
            cout << "Secondary key output turned OFF" << endl;
         }
         break;

      case '<':            // slow down the tempo which will be
         tempo /= 1.05;    // activated at the next note to be played
         if (tempo < 20) {
            tempo = 20;
         }
         metronome.setTempo(tempo);
         cout << "Tempo is: " << tempo << endl;
         break;

      case '>':            // speed up the tempo which will be
         tempo *= 1.05;    // activated at the next note to be played
         if (tempo < 240) {
            tempo = 240;
         }
         metronome.setTempo(tempo);
         cout << "Tempo is: " << tempo << endl;
         break;
      case '{': case '[':  // shorten analysis window by one second
         analysisDuration -= 1.0;
         if (analysisDuration < 1.0) { analysisDuration = 1.0; }
         cout << "ANALYSIS HISTORY: " << analysisDuration << " sec." << endl;
         break;
      case '}': case ']':  // lengthen analysis window by one second
         analysisDuration += 1.0;
         cout << "ANALYSIS HISTORY: " << analysisDuration << " sec." << endl;
         break;
      default:
         keyboard(key);
   }

   if (isdigit(key)) {
      keyoctave = key - '0';
      cout << "ANALYSIS OCTAVE: " << keyoctave << endl;
   }

}


/*------------------ begining of assistant functions --------------------*/



//////////////////////////////
//
// analyzekey -- figure out the key possibilities and then play the
//   best two choices.
//

void analyzekey(void) { 
   int i;

   // first, adjust the number of notes being analyzed
   if (!freezeQ) {
      while ((times.getCount() > 0) 
             && (fadeTime < t_time - analysisDuration * 1000)) {
         if (fadeTime > 0) {
            occurrences[fadeNote%12]--;
         }
         fadeTime = times.extract();
         fadeNote = notes.extract();
      }
      if ((fadeTime > 0) && (fadeTime < t_time - analysisDuration * 1000)) {
         occurrences[fadeNote%12]--;
         fadeTime = 0;
         fadeNote = 0;
      }
   }
      
   // now analyze the keys
   double total = 0;
   cout << "Count:";
   for (i=0; i<12; i++) {
      cout.width(3);
      cout << occurrences[i];
      total += occurrences[i];
   }
   cout << flush;

   if (total <= 0) {
      cout << endl;
      return;
   }
      
   double r_major[12];
   double r_minor[12];
   for (i=0; i<12; i++) {
      r_major[i] = pearsonCorrelation(12, occurrences, majorKey + 12 - i);
      r_minor[i] = pearsonCorrelation(12, occurrences, minorKey + 12 - i);
   }

   // Now determine which correlation is the greatest.
   // Start off with the assumption that C major is the best key.
   double best_key = 0.0; 
   const char* mode = "unknown";
   int pitch_class = 0;            // tonic note of best key
   
   // Compare all the remaining key correlations.
   for (i=0; i<12; i++) {
      if (r_major[i] > best_key) {
         best_key = r_major[i]; 
         mode = "major"; 
         pitch_class = i;
      }
      if (r_minor[i] > best_key) {
         best_key = r_minor[i]; 
         mode = "minor"; 
         pitch_class = i;
      }
   }

   // A confidence measure can be determined by taking the difference
   // between the correlation for the "best key" and subtracting the
   // correlation for the "second best key".  The maximum confidence
   // score is 100; the minimum is zero.
   // First, having found the "best key", find the "second best key."
   double second_best_key = 0;
   const char* secondmode = "unknown";
   int sec_pitch_class = 0;
   for (i=0; i<12; i++) {
      if (r_major[i] != best_key && r_major[i] > second_best_key) {
         second_best_key = r_major[i];
         secondmode = "major";
         sec_pitch_class = i;
      }
      if (r_minor[i] != best_key && r_minor[i] > second_best_key) {
         second_best_key = r_minor[i];
         secondmode = "minor";
         sec_pitch_class = i;
      }
   }

   // The value 3.0 below is a scaling factor.
   double confidence = (best_key - second_best_key) * 100 * 3.0;
   if (confidence > 100.0) confidence = 100.0;
   
   // Print the analysis results:
   cout << "   Key: " << name(pitch_class) << " " << mode 
        << " (";
   cout.width(3);
   cout << (int)confidence << "%), or " 
        << name(sec_pitch_class) << " " << secondmode;
   cout << endl;

   // finally, play the determined keys according to confidence
   int velocity = (int)(127.0 * confidence/100.0);
   
   firstVoice.play(pitch_class + 12 * keyoctave, velocity);
   if (displayKey2) {
      secondVoice.play(sec_pitch_class+12*keyoctave, int(velocity*0.75 + 0.5));
   }

}



//////////////////////////////
//
// pearsonCorrelation --
//

double pearsonCorrelation(int size, double* x, double* y) {

   double sumx  = 0.0;
   double sumy  = 0.0;
   double sumco = 0.0;
   double meanx = x[0];
   double meany = y[0];
   double sweep;
   double deltax;
   double deltay;

   int i;
   for (i=2; i<=size; i++) {
      sweep = (i-1.0) / i;
      deltax = x[i-1] - meanx;
      deltay = y[i-1] - meany;
      sumx  += deltax * deltax * sweep;
      sumy  += deltay * deltay * sweep;
      sumco += deltax * deltay * sweep;
      meanx += deltax / i;
      meany += deltay / i;
   }

   double popsdx = sqrt(sumx / size);
   double popsdy = sqrt(sumy / size);
   double covxy  = sumco / size;

   return covxy / (popsdx * popsdy);
}



//////////////////////////////
//
// name -- return the name of the input pitch class
//

const char* name(int pitch_class) {
   pitch_class = pitch_class % 12;
   switch (pitch_class) {
      case 0:   return "C ";
      case 1:   return "C#";
      case 2:   return "D ";
      case 3:   return "Eb";
      case 4:   return "E ";
      case 5:   return "F ";
      case 6:   return "F#";
      case 7:   return "G ";
      case 8:   return "Ab";
      case 9:   return "A ";
      case 10:  return "Bb";
      case 11:  return "B ";
   }
   
   return "unknown";
}
   




//////////////////////////////
//
// storeNote -- store the next note
//

void storeNote(char storeNote, long storeTime) {
   notes.insert(storeNote);
   times.insert(storeTime);
   occurrences[storeNote%12]++;
}



/*------------------ end improvization algorithms -----------------------*/



// md5sum: 37e8b26f4e2bda279991a5247715ff18 rtkey.cpp [20090626]
