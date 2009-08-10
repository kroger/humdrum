#include "batonImprov.h"

/* <Jane Rivera><Music120><FinalProject><March1996>
   -----------------------------------------------------------------------------
   12BarRadioBatonBlues.c -- 3 channels worth of Blues: a ground bassline,
        "comp" chords (stick 1 trigger)in three inversions (horizontal axis)
        through three registers (vertical axis), and blues scale (stick 2
        pollresponse) mapped horizontally with three vertical register shifts.
        Bass & blues scale velocity mapped to pots 1 & 2.  Velocity of comp
        chords controlled by whack. User may choose key/transposition with
        b15+/keyboard input.

    three voices:
       chann1--chords on piano
               stick1 triggers notes, whack->loudness, yt1-chord,xt1->inversion
       chann2--walking bass on pizz bass
               starts after 1st chord, d1p->loudness
       chann3--melody on brass
               stick2 poll plays notes, yt2->octive (3 octs),
               x2p->blues scale d2p->loudness
       b15+ ->allows transposition typed at keyboard
*/

#define BASS_TONES     48
#define CHORD_TONES     5
#define CHORD_CHANGES   9
#define BEAT          500

int  xc, yc;             // current x,y coordinates
int  xcOld, ycOld;       // previous x,y coordinates
int  progressionChord;
int  chordIndex = 0;
int  transpose = 0;
long nextBassNotetime;
int  bluesnote = 0;
int  tempIn;
int  trmptnoteon;
int  keyvel,i,z;

// blues scale degrees/intervals are: 1,m3,4,d5,5,m7
int BluesScale[6] = {0, 3, 5, 6, 7, 10};

// 12bar bassline for progression of:I,IV,I,I,IV,IV,I,I,V,IV,I,V
int bassline[BASS_TONES] = {0,0,7,6,  5,5,9,10,   12,12,10,9, 7,0,3,4,  
                           5,5,12,11, 12,11,10,3, 0,0,7,6,    7,4,5,6,  
                           7,7,14,6,  5,5,12,1,   0,4,10,9,   7,-5,-2,-1};
int roots[BASS_TONES] =    {0,0,0,0,  5,5,5,5,    0,0,0,0,    0,0,0,0,  
                            5,5,5,5,  5,5,5,5,    0,0,0,0,    0,0,0,0,  
                            7,7,7,7,  5,5,5,5,    0,0,0,0,   7,7,7,7};
int bassIndex  = 0;
int BassNote   = 0;
int PlayStatus = 0;

// Change chord nine times during 12 bars

// Intervals in three different comp chords (each have 5 notes):
int chords[3][CHORD_TONES]={{12,10,16,21,26},
                            {04,10,21,26,31},
                            {04,10,14,31,36}};

// oldChord store the keynumbers of the old chord to turn off later...
int oldChord[CHORD_TONES] = {0, 0, 0, 0, 0};

/*--------------------beginning improvization algorithms------------------*/
void description(void){
   printf("3 voice blues piece\n");
   printf("  ch1-piano chords;  ch2-pizz bass; ch3-trumpet\n");
   printf("  loudness------whack    pt1            pt2    \n");
   printf(" bass start = b15+    bass stop =  b14+  \n");
}

/*---------------------initialization algorithms--------------------------*/

void initialization(void) {
   program_change(1, 0);    // channel 1--piano
   program_change(2,46 );   // channel 2--pizz base
   program_change(3,62);    // channel 3--trumpet

   control_change(1,7,127);control_change(1,11,127);control_change(1,10,64);
   control_change(2,7,127);control_change(2,11,127);control_change(2,10,64);
   control_change(3,7,127);control_change(3,11,127);control_change(3,10,64);
}

void localinit(void){
   nextBassNotetime = 0;
}

/*--------------------main loop algorithms -------------------------------*/

// mainloop: make sure chords and bass change together after correct amount of
//          time...also, make sure user can input a transposition before playing
//          starts, i.e. make an on/off switch...

void mainloopalgorithms(void){
  if (PlayStatus) {
      if (t_time > nextBassNotetime) {
         nextBassNotetime = t_time+500;
         note_on(2, BassNote, 0);
         BassNote = bassline[bassIndex] + transpose + 48;
         progressionChord=roots[bassIndex++];
         note_on(2, BassNote, pt1);
         bassIndex %= BASS_TONES;
      }
   } else {
      nextBassNotetime = t_time;
      bassIndex = 0;
      chordIndex = 0;
   }
   if (zt2>80) {
      xcOld=xc;ycOld=yc;xc=xt2/22;yc=yt2/43;
      if (xc != xcOld || yc != ycOld) {
         note_on(3, bluesnote, 0);
         bluesnote=48+12*yc+BluesScale[xc]+transpose;
         note_on(3, bluesnote, pt2);trmptnoteon=1;
      }
   } else if (trmptnoteon) {
      trmptnoteon=0;note_on(3,bluesnote,0);
   }

}

/*--------------------triggered algorithms--------------------------------*/

void stick1trig(void) {
   if (PlayStatus) {
      for (i=0; i<CHORD_TONES; i++) {
         // turn off old chord tone:
         note_on(1, oldChord[i], 0);
   
         //start at middleC, add entered interval & play appropriate comp chord.
         //also,replace the oldChord note with a new chord note.
         //"progressionChord" controls how chord will change over time.
         //"ScaleMap" will control how the inversion and register is chosen.
   
         z = 48 + progressionChord + transpose + chords[xw1/43][i] + 
               ((yw1/43-1) * 12);
         keyvel = wh1+ 50 ;                //ensure chords are sounding...
         note_on(1,z,keyvel);
         oldChord[i] = z;
      }
   }
}

void stick2trig(void) { }

void b14plustrig(void){
   int i;
   PlayStatus=0;
   // turn off currently playing notes:
   note_on(1, BassNote, 0);
   for (i=0; i< CHORD_TONES; i++) {
      note_on(1, oldChord[i], 0);
   }
}

void b15plustrig(void){
   PlayStatus=1;
   localinit();
}

void b14minusuptrig(void){}
void b14minusdowntrig(void){}
void b15minusuptrig(void){}
void b15minusdowntrig(void){}
void keyboardchar(int c){}
void finishup(void){}

/*---------------------------end improvization algorithms------------------*/


// md5sum: 638041a726f4eed3f79c65c5f61b007b blues.cpp [20050403]
