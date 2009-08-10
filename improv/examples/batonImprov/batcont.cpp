//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Last Modified: Sat Jan 16 10:03:46 PST 1999
// Last Modified: Sat Jan 16 11:03:46 PST 1999
// Filename:      ...improv/doc/examples/batonImprov/batcont/batcont.cpp
// Syntax:        C++; batonImprov
//  
// Description: Program for Jun and DMP11
//   x- and y-axes of two batons are mapped to arbitrary controllers.
//

#include "batonImprov.h" 


/*----------------- beginning of improvization algorithms ---------------*/


int cont1x = 10;     // stick 1 x-axis controller number
int cont1y = 11;     // stick 1 y-axis controller number
int cont2x = 12;     // stick 2 x-axis controller number
int cont2y = 13;     // stick 2 y-axis controller number

#define CONTROLTIME 50
int nextcontroltime = 0;

int control1 = 0;
int control2 = 0;


/*----------------- FUNCTIONS -------------------------------------------*/

void contup(int& controller) {
   controller++;
   if (controller > 127) {
      controller = 127;
   }
}

void contdown(int& controller) {
   controller--;
   if (controller < 0) {
      controller = 0;
   }
}

/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   cout 
   << "\nThis program demonstrates how to play notes with baton triggers.\n"
   << "Only one pitch is played at a time.\n"
   << "\nThe b14+ and b15+ buttons control the instrument being played.\n"
   << "\nPress the key \"X\" to turn on the MIDI out trace and see what\n"
   << "MIDI commands are being sent to the synthesizer.\n"
   << endl;
}

void initialization(void) { }

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

void mainloopalgorithms(void) { 
   if (nextcontroltime <= t_time) {
      if (control1) {
         synth.cont(0, cont1x, baton.x1p);
         synth.cont(0, cont1y, baton.y1p);
      }
      if (control2) {
         synth.cont(0, cont2x, baton.x2p);
         synth.cont(0, cont2y, baton.y2p);
      }
      nextcontroltime = t_time + CONTROLTIME;
   }
}


/*-------------------- triggered algorithms -----------------------------*/

void stick1trig(void) { }

void stick2trig(void) { }

void b14plustrig(void) { }

void b15plustrig(void) { }

void b14minusuptrig(void) { }

void b14minusdowntrig(void) { }

void b15minusuptrig(void) { }

void b15minusdowntrig(void) { }

void keyboardchar(int key) { 
   switch (key) {
      case '1': contup(cont1x); cout << "cont1x = " << cont1x << endl;
         break;
      case 'q': contdown(cont1x); cout << "cont1x = " << cont1x << endl;
         break;
      case '2': contup(cont1y); cout << "cont1y = " << cont1y << endl;
         break;
      case 'w': contdown(cont1y); cout << "cont1y = " << cont1y << endl;
         break;
      case '3': contup(cont2x); cout << "cont2x = " << cont2x << endl;
         break;
      case 'e': contdown(cont2x); cout << "cont2x = " << cont2x << endl;
         break;
      case '4': contup(cont2y); cout << "cont2y = " << cont2y << endl;
         break;
      case 'r': contdown(cont2y); cout << "cont2y = " << cont2y << endl;
         break;
      case 'z': control1 = !control1; cout << "control1 = " << control1 << endl;
         break;
      case 'x': control2 = !control2; cout << "control2 = " << control2 << endl;
         break;
   }
}


/*------------------ end improvization algorithms -----------------------*/



// md5sum: 3bdfae0f93133305440a599df0418ef5 batcont.cpp [20050403]
