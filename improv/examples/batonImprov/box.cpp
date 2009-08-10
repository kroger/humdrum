//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 26 December 1997
// Last Modified: Tue Apr 18 17:51:22 PDT 2000 (updated syntax)
// Filename:      ...sig/doc/examples/all/box/box.cpp
// Syntax:        C++; batonImprov 2.0
//  
// Description: Demonstrates how to create boxes on the radio drum.
//	Notice that the edged of the boxes are very narrow,
//	and the notes can oscillate at the edges.
//

#include "batonImprov.h" 

/*----------------- beginning of improvization algorithms ---------------*/

int boxlevel;    // the height of the four boxes on the surface of the drum

void stick1position(void);
void stick2position(void);

/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();  
   printstringline(
" Description: Demonstrates how to create boxes    +---------+---------+ ");
   psl(
" on the  radio drum. Notice that the edged of     | note 1  | note 2  | ");
   psl(
" the boxes are very narrow, and the notes can     |         |         | ");
   psl(
" oscillate at the edges.  There are four boxes    | (dial1) | (dial2) | ");
   psl(
" on the drum surface, one for each quadrant.      +---------+---------+ ");
   psl(
" When a stick enters a box, it will generate a    |  note 3 |  note 4 | ");
   psl(
" note.                                            |         |         | ");
   psl(
"    The volume of each box is controlled by       | (dial3) | (dial4) | ");
   psl(
"  each of the four dials as indicated to the      +---------+---------+ ");
   psl(
"  right.  The b14+ raises the height of the boxes, and b15+ lowers      ");
   psl(
"  the height of the box on the surface of the radio drum.               ");
   printboxbottom();
} 

void initialization(void) { 
   boxlevel = 100;
   baton.stick1position = stick1position;
   baton.stick2position = stick2position;
}

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

int newbox(int batonNumber) {
   // this function returns true if a new box was entered
   switch (batonNumber) {
      case 1:
         // check the z-axis
         if (baton.z1pb[0] > boxlevel && baton.z1pb[1] <= boxlevel) {
            return 1;
         }
         // first check to see if the x-axis border was crossed:
         if (baton.x1pc(0, 1)) {
            return 1;
         }
         // now check to see if the y-axis boarder was crossed:
         if (baton.y1pc(0, 1)) {
            return 1;
         }
         break;
      case 2:
         // check the z-axis
         if (baton.z2pb[0] > boxlevel && baton.z2pb[-1] <= boxlevel) {
            return 1;
         }
         // first check to see if the x-axis border was crossed:
         if (baton.x2pc(0, 1)) {
            return 1;
         }
         // now check to see if the y-axis boarder was crossed:
         if (baton.y2pc(0, 1)) {
            return 1;
         }
         break;
   } // end of switch

   // no new box 
   return 0;
}

void playbox(int boxnumber) {
   switch (boxnumber) {
      case 11:  
         synth.play(CH_10, GM_LOW_MID_TOM, baton.d1p);
         break;
      case 12:
         synth.play(CH_10, GM_HIGH_MID_TOM, baton.d2p);
         break;
      case 21:
         synth.play(CH_10, GM_LOW_TOM, baton.d3p);
         break;
      case 22:
         synth.play(CH_10, GM_HIGH_TOM, baton.d4p);
         break;
   } // end switch
}

void playboxnote1(void) {
   int boxnumber = 10 * baton.x1ps(1, 2) + baton.y1ps(1, 2);
   playbox(boxnumber);
}

void playboxnote2(void) {
   int boxnumber = 10 * baton.x2ps(1, 2) + baton.y2ps(1, 2);
   playbox(boxnumber);
}
      

void mainloopalgorithms(void) { }


/*-------------------- triggered algorithms -----------------------------*/


void stick1position(void) {
   if (newbox(1)) {
      playboxnote1();
   }
}

void stick2position(void) {
   if (newbox(2)) {
      playboxnote2();
   }
}


void stick1trig(void) { }
void stick2trig(void) { }

void b14plustrig(void) { 
   boxlevel--;
   if (boxlevel < 0)  boxlevel = 0;
   cout << "boxlevel = " << boxlevel << endl;
}

void b15plustrig(void) { 
   boxlevel++;  
   if (boxlevel > 127)  boxlevel = 127;
   cout << "boxlevel = " << boxlevel << endl;
}

void b14minusuptrig(void) { cout << "b14 pedal released" << endl; }
void b14minusdowntrig(void) { cout << "b14 pedal released" << endl; }
void b15minusuptrig(void) { cout << "b15 pedal released" << endl; }
void b15minusdowntrig(void) { cout << "b15 pedal depressed" << endl; }
void keyboardchar(int key) { charsynth(key); }


/*------------------ end improvization algorithms -----------------------*/


// md5sum: 77cd37ef3f92e819522ad91250656d3a box.cpp [20050403]
