// The game of hot and cold for the radio batons.
//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Mar 19 13:35:34 PST 1996
// Last Modified: Mon Apr  8 21:35:04 PDT 1996
// Last Modified: Tue Apr 18 11:20:26 PDT 2000 (syntax updated)
// Filename:      ...sig/doc/examples/all/hotcold/hotcold.cpp
// Syntax:        C++; batonImprov
//
// Description: Hit the drum surface with a baton to start a search for a 
//   random point.  Notes stop playing when the 3-D point is found.
//   You are hot when the pitches rise; cold when pitch drops.
//   Each axis has a voice associated with itself.
//

#include "batonImprov.h"
#include <math.h>

/***************** Beginning of user function *****************************/

Voice  xVoice(CH_1);           // voice for x-axis
Voice  yVoice(CH_2);           // voice for y-axis
Voice  zVoice(CH_3);           // voice for z-axis

int  xrand, yrand, zrand;      // random point in baton space
int  status = 0;               // test whether to play notes
SigTimer timer;                // time how long it takes to find point
long start_time = 0;           // time that new point was chosen

CircularBuffer<int> xpos(16);  // history of x positions
CircularBuffer<int> ypos(16);  // history of y positions
CircularBuffer<int> zpos(16);  // history of y positions

/*----------------- beginning of improvization algorithms ---------------*/

/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   cout
   << "______________________________________________________________ \n"
   << "| Description:                                                |\n"
   << "|    Hit the drum with a baton to start a search for a random |\n"
   << "| point.  Notes stop playing when the 3-D point is found.     |\n"
   << "| You are hot when the pitches rise; cold when pitch drops.   |\n"
   << "| Each axis has a voice associated with itself.               |\n"
   << "|                          -- Craig Stuart Sapp, 19 Mar 1996  |\n"
   << "---------------------------------------------------------------\n"
   ;
}


void programhelp(void) {
   cout
   << ">                                                              \n"
   << "> Baton 1:                                                     \n"
   << ">   trig: Start the hot/cold game                              \n"
   << ">    x1p: X-axis note -- higher if warmer, lower if colder     \n"
   << ">    y1p: Y-axis note -- higher if warmer, lower if colder     \n"
   << ">    z1p: Z-axis note -- higher if warmer, lower if colder     \n"
   << "> Baton 2:                                                     \n"
   << ">   trig: Stop the hot/cold game                               \n"
   << ">                                                              \n"
   << ">    p1p: Attack velocity of x direction voice                 \n"
   << ">    p2p: Attack velocity of y direction voice                 \n"
   << ">    p3p: Attack velocity of z direction voice                 \n"
   << ">   +b14: Command mode                                         \n"
   << ">   +b15: This help screen                                     \n"
   << ">                                                              \n"
   ;
}

void initialization(void) { 
   description();
   xVoice.pc(GM_ACOUSTIC_GRAND_PIANO);
   yVoice.pc(GM_MUSIC_BOX);
   zVoice.pc(GM_MARIMBA);
   timer.setPeriod(200);
   timer.reset();
}


void finishup(void) { }

/*-------------------- main loop algorithms -----------------------------*/


int change(CircularBuffer<int>& buffer, int min, int max) {
   int valueold, value;
   value = buffer[0] >= 127 ? max : (int)(buffer[0]/127.0*(max-min+1)+min);
   valueold = buffer[1] >= 127 ? max : (int)(buffer[1]/127.0*(max-min+1)+min);
   return value - valueold;
}


void mainloopalgorithms(void) { 
   if (status == 1 && timer.expired()) {
      xpos.insert(baton.x1p);
      ypos.insert(baton.y1p);
      zpos.insert(baton.z1p);
      timer.reset();

      // play the notes for each axis if a new note space is entered:

      if (change(xpos, A0, C7)) {
         xVoice.play(C7-abs(xrand-baton.x1ps(A0, C7)), 64);
         cout << " x = "   << C7-abs(xrand-baton.x1ps(A0, C7));
      } 
      if (change(ypos, A0, C7)) {
         yVoice.play(C7-abs(yrand-baton.y1ps(A0, C7)), 64);
         cout << " y = "   << C7-abs(yrand-baton.y1ps(A0, C7));
      } 
      if (change(zpos, A0, C7)) {
         zVoice.play(C7-abs(zrand-baton.z1ps(A0, C7)), 64);
         cout << " z = "   << C7-abs(zrand-baton.z1ps(A0, C7));
      } 

      cout << endl;


      // if the 3-D point was found, turn off the notes and print time:
      if (   abs(baton.x1ps(A0, C7) - xrand) < 10  &&
             abs(baton.y1ps(A0, C7) - yrand) < 10  &&
             abs(baton.z1ps(A0, C7) - zrand) < 10       ) {
         status = 0;
         xVoice.off();
         yVoice.off();
         zVoice.off();
         cout << "Time to find point: " << (t_time-start_time)/1000.0 
              << " sec.\n";
      }
   }
}


/*---------------- Trigger response functions ----------------------------*/

void stick1trig(void) { 
   // if not already searching for a point, choose a new point and start timer:
   if (status == 0) {
      status = 1;
      xrand = rand() % (C7-A0) + A0;
      yrand = rand() % (C7-A0) + A0;
      zrand = (rand() % 44) + 44;        // limit z-axis close to baton surface
      start_time = t_time;
      cout << "random point chosen\n"; 
   }
}

void stick2trig(void) { 
   if (status == 1) {
      status = 0;
      xVoice.off();
      yVoice.off();
      zVoice.off();
   }
}

void b14plustrig(void) { 
   status = 0;
   xVoice.off();
   yVoice.off();
   zVoice.off();
   cout << "Game ended" << endl;
}

void b14minupuptrig(void) { programhelp(); }
void b14minusdowntrig(void) { programhelp(); }
void b14minusuptrig(void) { programhelp(); }
void b15minudownstrig(void) { programhelp(); }
void b15minusdowntrig(void) { programhelp(); }
void b15minusuptrig(void) { programhelp(); }
void b15plustrig(void) { programhelp(); }


/*---------------- Poll response algorithms ------------------------------*/

void stick1pollresponse(void) { }
void stick2pollresponse(void) { }
void potspollresponse(void) { }



void keyboardchar(int key) { }

/***************** End of user functions **********************************/

// md5sum: 705d01d6987e9f6e0e3639309e413146 hotcold.cpp [20050403]
