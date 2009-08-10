//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Apr  9 21:27:50 PDT 1996
// Last Modified: Tue Apr  9 21:27:59 PDT 1996
// Last Modified: Tue Apr 18 13:53:31 PDT 2000 (updated syntax)
// Filename:      ...sig/doc/examples/gopen/gopen.cpp
// Syntax:        C++; batonImprov 2.0
//
// Description: Basic idea for a radio baton guitar.  Only open strings 
//              on this example. 
//

#include <math.h>				 // for abs function
#include "batonImprov.h"

enum strings {string1 = 3, string2 = 5, string3 = 7, string4 = 9, 
              string5 = 11, string6 = 13};


/***************** Beginning of user function *****************************/

Voice st1;  // Bass   E string
Voice st2;  //        A string
Voice st3;  //        D string
Voice st4;  //        G string
Voice st5;  //        B string
Voice st6;  // Treble E string

int xpos1 = 0;		// position of baton 1 in the y-axis
int ypos2 = 0;		// position of baton 2 in the y-axis
int new1Q = 0;		// boolean for if baton 1 in new box
int new2Q = 0;		// boolean for if baton 2 in new box
int test = 0;

void stick1pollresponse(void);
void stick2pollresponse(void);

/*---------------- Documentation functions -------------------------------*/

void description(void) {
   cout
   << "______________________________________________________________ \n"
   << "| Description:                                                |\n"
   << "|    Basic idea for a radio baton guitar.  Only open strings  |\n"
   << "| on this example.                                            |\n"
   << "|                          -- Craig Stuart Sapp, 10 Apr 1996  |\n"
   << "---------------------------------------------------------------\n"
   ;
}


void programhelp(void) {
   cout
   << ">                                                              \n"
   << "> Baton 1:                                                     \n"
   << ">    x1p:                                                      \n"
   << ">    y1p:                                                      \n"
   << ">    z1p:                                                      \n"
   << "> Baton 2:                                                     \n"
   << ">   trig:                                                      \n"
   << ">                                                              \n"
   << ">    p1p:                                                      \n"
   << ">    p2p:                                                      \n"
   << ">    p3p:                                                      \n"
   << ">   +b14: Command mode.                                        \n"
   << ">   +b15: This help screen.                                    \n"
   << ">                                                              \n"
   ;
}

/*---------------- Initialization algorithms -----------------------------*/

void initialization(void) { 
   description();

   st1.setChan(CH_1);        // Bass   E string
   st2.setChan(CH_2);        //        A string
   st3.setChan(CH_3);        //        D string
   st4.setChan(CH_4);        //        G string
   st5.setChan(CH_5);        //        B string
   st6.setChan(CH_6);        // Treble E string

   st1.pc(GM_ACOUSTIC_GUITAR_NYLON);        // Bass   E string
   st2.pc(GM_ACOUSTIC_GUITAR_NYLON);        //        A string
   st3.pc(GM_ACOUSTIC_GUITAR_NYLON);        //        D string
   st4.pc(GM_ACOUSTIC_GUITAR_NYLON);        //        G string
   st5.pc(GM_ACOUSTIC_GUITAR_NYLON);        //        B string
   st6.pc(GM_ACOUSTIC_GUITAR_NYLON);        // Treble E string

   st1.setKey(E2);
   st2.setKey(A2);
   st3.setKey(D3);
   st4.setKey(G3);
   st5.setKey(B3);
   st6.setKey(E4);

   synth.cont(CH_1, 10, 10);         // pan left
   synth.cont(CH_2, 10, 30);
   synth.cont(CH_3, 10, 50);
   synth.cont(CH_4, 10, 75);
   synth.cont(CH_5, 10, 100);
   synth.cont(CH_6, 10, 120);       // pan right 

   baton.stick1position = stick1pollresponse;
   baton.stick2position = stick2pollresponse;

}


/*---------------- Main loop algorithms ----------------------------------*/

void mainloopalgorithms(void) { }

void stick1trig(void) { }
void stick2trig(void) { }

void b14plustrig(void) { 
   st1.off(); st2.off(); st3.off(); st4.off(); st5.off(); st6.off();
}

void b15plustrig(void) { programhelp(); }
void b14minusuptrig(void) { programhelp(); }
void b14minusdowntrig(void) { programhelp(); }
void b15minusuptrig(void) { programhelp(); }
void b15minusdowntrig(void) { programhelp(); }

void keyboardchar(int key) { 
   st1.pc(GM_ACOUSTIC_GUITAR_NYLON);        // Bass   E string
   st2.pc(GM_ACOUSTIC_GUITAR_NYLON);        //        A string
   st3.pc(GM_ACOUSTIC_GUITAR_NYLON);        //        D string
   st4.pc(GM_ACOUSTIC_GUITAR_NYLON);        //        G string
   st5.pc(GM_ACOUSTIC_GUITAR_NYLON);        //        B string
   st6.pc(GM_ACOUSTIC_GUITAR_NYLON);        // Treble E string
}

/*---------------- Poll response algorithms ------------------------------*/

void stick1pollresponse(void) { 
   int xpos1 = midiscale(baton.x1pb[0], 0, 15); 
   int xpos1old = midiscale(baton.x1pb[1], 0, 15);
   static int new1Q = 0;
   new1Q += xpos1 - xpos1old;
   if (baton.z1p < 40) {
      new1Q = 0;
   }

   // if baton 1 moves onto a new string:
   if (abs(new1Q) == 2 && (xpos1 % 2) == 1) {
      switch (xpos1) {
         case string1:  st1.setVelocity(baton.z1p); st1.play(); break;
         case string2:  st2.setVelocity(baton.z1p); st2.play(); break;
         case string3:  st3.setVelocity(baton.z1p); st3.play(); break;
         case string4:  st4.setVelocity(baton.z1p); st4.play(); break;
         case string5:  st5.setVelocity(baton.z1p); st5.play(); break;
         case string6:  st6.setVelocity(baton.z1p); st6.play(); break;
      }
      new1Q = 0;
   }


   // cover the cases when the baton is moving too fast:

   if (new1Q > 2) {
      if (xpos1old < string1 && xpos1 >= string1) {
         st1.setVelocity(baton.z1p); st1.play(); 
      }
      if (xpos1old < string2 && xpos1 >= string2) {
         st2.setVelocity(baton.z1p); st2.play(); 
      }
      if (xpos1old < string3 && xpos1 >= string3) {
         st3.setVelocity(baton.z1p); st3.play(); 
      }
      if (xpos1old < string4 && xpos1 >= string4) {
         st4.setVelocity(baton.z1p); st4.play(); 
      }
      if (xpos1old < string5 && xpos1 >= string5) {
         st5.setVelocity(baton.z1p); st5.play(); 
      }
      if (xpos1old < string6 && xpos1 >= string6) {
         st6.setVelocity(baton.z1p); st6.play(); 
      }
      new1Q = 0;
   }

   if (new1Q < -2) {
      if (xpos1old > string1 && xpos1 <= string1) {
         st1.setVelocity(baton.z1p); st1.play(); 
      }
      if (xpos1old > string2 && xpos1 <= string2) {
         st2.setVelocity(baton.z1p); st2.play(); 
      }
      if (xpos1old > string3 && xpos1 <= string3) {
         st3.setVelocity(baton.z1p); st3.play(); 
      }
      if (xpos1old > string4 && xpos1 <= string4) {
         st4.setVelocity(baton.z1p); st4.play(); 
      }
      if (xpos1old > string5 && xpos1 <= string5) {
         st5.setVelocity(baton.z1p); st5.play(); 
      }
      if (xpos1old > string6 && xpos1 <= string6) {
         st6.setVelocity(baton.z1p); st6.play(); 
      }
      new1Q = 0;
   }

}

void stick2pollresponse(void) { 
   int xpos2 = midiscale(baton.x2pb[0], 0, 15); 
   int xpos2old = midiscale(baton.x2pb[1], 0, 15);
   static int new2Q = 0;
   new2Q += xpos2 - xpos2old;
   if (baton.z2p < 40) {
      new2Q = 0;
   }

   // if baton 2 moves onto a new string:
   if (abs(new2Q) == 2 && (xpos2 % 2) == 1) {
      switch (xpos2) {
         case string1:  st1.off(); break;
         case string2:  st2.off(); break;
         case string3:  st3.off(); break;
         case string4:  st4.off(); break;
         case string5:  st5.off(); break;
         case string6:  st6.off(); break;
      }
      new1Q = 0;
   }


   // cover the cases when the baton is moving too fast:

   if (new2Q > 2) {
      if (xpos2old < string1 && xpos2 >= string1) {
         st1.off(); 
      }
      if (xpos2old < string2 && xpos2 >= string2) {
         st2.off(); 
      }
      if (xpos2old < string3 && xpos2 >= string3) {
         st3.off(); 
      }
      if (xpos2old < string4 && xpos2 >= string4) {
         st4.off(); 
      }
      if (xpos2old < string5 && xpos2 >= string5) {
         st5.off(); 
      }
      if (xpos2old < string6 && xpos2 >= string6) {
         st6.off(); 
      }
      new2Q = 0;
   }

   if (new2Q < -2) {
      if (xpos2old > string2 && xpos2 <= string2) {
         st1.off(); 
      }
      if (xpos2old > string2 && xpos2 <= string2) {
         st2.off(); 
      }
      if (xpos2old > string3 && xpos2 <= string3) {
         st3.off(); 
      }
      if (xpos2old > string4 && xpos2 <= string4) {
         st4.off(); 
      }
      if (xpos2old > string5 && xpos2 <= string5) {
         st5.off(); 
      }
      if (xpos2old > string6 && xpos2 <= string6) {
         st6.off(); 
      }
      new2Q = 0;
   }

}



/*---------------- Actions to take just before leaving program -----------*/

void finishup(void) { }

/***************** End of user functions **********************************/


// md5sum: 63a6e2b8f79254eb6232768724f739fe gopen.cpp [20050403]
