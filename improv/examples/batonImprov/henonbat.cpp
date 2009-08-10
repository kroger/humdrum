//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Oct 19 22:44:27  2002
// Last Modified: Sun Oct 20 21:40:26  2002
// Filename:      ...sig/doc/examples/all/henonbat.cpp
// Syntax:        C++; batonImprov 2.0
//  
// Description:   Radio Baton control of a melody based on the Henon map:
//                x[n] = 1 + a * x[n-1] * x[n-1] + b * x[n-2]
//

#include "batonImprov.h" 

// function declarations:
int  nextHenon    (double alpha, double beta, double& x,
                   double& xx);
int  midiLimit    (int value);
void startHenon   (int prealpha, int prebeta);
void initSequence (double& x, double& xx, double& alpha, double& beta, 
                   int xpos, int ypos);

// variables:
int    duration =  100;  // duration of each note
int    meter    =  2;    // current meter;
int    nextmeter=  2;    // meter of next measure;
int    curbeat  =  0;    // current beat in measure;
int    loudness =  64;   // MIDI note attack velocity
int    emphasis =  0;    // used for metric control of loudness
int    key      =  0;    // output MIDI note from Henon sequence
int    transpose=  0;    // transposition of melody
int    nextnotetime;     // next time to try to play a note (in millisec)
int    minnote  =  A0;   // minimum MIDI note to play
int    maxnote  =  C7;   // maximum MIDI note to play
int    z1level  =  80;   // baton z1p below this level plays the sequence
int    z2level  =  80;   // baton z2p below this level activates extra controls
int    seqstate =  0;    // 0 = sequence not playing, 1 = sequence playing
double initx    =  0.0;  // starting value of x in Henon sequence
double initxx   =  0.0;  // starting value of xx in Henon sequence
double x;                // current output of Henon sequence
double xx;               // previous output of Henon sequence
double alpha;            // alpha parameter of Henon map
double beta;             // beta parameter of Henon map
double alphamin = -2.0;  // minimum value for alpha on baton surface (x-axis)
double alphamax = -1.0;  // maximum value for alpha on baton surface (x-axis)
double betamin  = -0.25; // minimum value for beta on baton surface (y-axis)
double betamax  =  0.25; // maximum value for beta on baton surface (y-axis)
Voice  voice;            // for playing melody

/*--------------------- maintenance algorithms --------------------------*/

void description(void) { 
   cout << 
      " Henon Map Melody -- Craig Sapp 19 October 2002\n"
      " Stick 1:   trigger = alpha, beta values and start of sequence\n"
      "            z1p = turn off sequence\n"
      "            x1p = loudness, y1p = tempo\n"
      " Stick 2:   z1p = turn stick 2 controls on/off\n"
      "            x2p = meter\n"
      "            y2p = transposition\n"
      " Dials:     d1p = transposition amount\n"
      "            d2p = metric emphasis\n"
      << endl;
} 

void stick1pollresponse(void);
void stick2pollresponse(void);
void initialization(void) {
   baton.stick1position = stick1pollresponse;
   baton.stick2position = stick2pollresponse;
   voice.setPort(synth.getPort());
}

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

void mainloopalgorithms(void) {
   if (seqstate && nextnotetime <= t_time) {
      nextnotetime += duration;
     
      if (curbeat >= meter*2 || curbeat == 0) {
         curbeat = 0;
         emphasis = midiscale(baton.d2p, 0, 20);
         meter = nextmeter;
      } else {
         if (curbeat % 2 == 0) {
            emphasis = 7;
         } else {
            emphasis = 0;
         }
      }
      curbeat++;
      if (baton.z2p < z2level) {
         emphasis = 0;
      }
      key = nextHenon(alpha, beta, x, xx) + transpose;
      if (key >= minnote && key <= maxnote) {
         cout << "\tPlaying note: " << key << "\tvel="
              << loudness + emphasis
              << "\tdur=" << duration 
              << "\tbeat=" << curbeat/2.0+0.5  << "  "
              << "\tmeter=" << meter << endl;
         voice.play(key, loudness + emphasis);
      } else {
         voice.off();
      }
   }
}


/*-------------------- triggered algorithms -----------------------------*/

void stick1trig(void) { 
   seqstate = 1;           // enable playing of sequence
   nextnotetime = t_time;
   initSequence(x, xx, alpha, beta, baton.x1t, baton.y1t);
   nextHenon(alpha, beta, x, xx); // ignore first value
}

void stick2trig(void) { 
   curbeat = 0;
}

void b14plustrig(void) { }
void b15plustrig(void) { }
void b14minusuptrig(void) { }
void b14minusdowntrig(void) { }
void b15minusuptrig(void) { }
void b15minusdowntrig(void) { }
void keyboardchar(int key) { }

void stick1pollresponse(void) {
   duration = midiscale(127-baton.y1p, 70, 300);
   loudness = midiscale(baton.x1p, 20, 100);
   if (baton.z1p < z1level) {
      seqstate = 0;  // turn off seq if baton too high
      voice.off();
   }
}

void stick2pollresponse(void) {
   if (baton.z2p < z2level) return;

   transpose = midiscale(baton.y2p, -1, 1) * midiscale(baton.d1p, 0, 12);
   switch (midiscale(baton.x2p, 0, 2)) {
      case 0: nextmeter = 2; break;
      case 1: nextmeter = 5; break;
      case 2: nextmeter = 3; break;
   }
}

/*------------------ end improvization algorithms -----------------------*/


void initSequence(double& x, double& xx, double& alpha, double& beta, 
      int xpos, int ypos) {
   x     = initx;
   xx    = initxx;
   alpha = xpos/127.0 * (alphamax - alphamin) + alphamin;
   beta  = ypos/127.0 * (betamax  - betamin)  + betamin;
   cout << "Starting position: (" << xpos << ", " << ypos << ")" 
        << "\twith (alpha, beta) = ("
        << alpha << ", " <<  beta << ")"
        << endl;
}


int nextHenon(double alpha, double beta, double& x, double& xx) {
   double xxx = xx;
   xx  = x;
   x   = 1 + alpha * xx * xx + beta * xxx;
   return midiLimit((int)((x + 1.0)/2.0 * 127.0 + 0.5));
}


int midiLimit(int value) {
   if (value < 0)        return 0;
   else if (value > 127) return 127;
   else                  return value;
}


// md5sum: 2551356d8669998fdeb5c68dbf036fb2 henonbat.cpp [20050403]
