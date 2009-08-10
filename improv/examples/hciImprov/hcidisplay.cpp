//
// Programmer:    Leland Stanford, Jr. <leland@stanford.edu>
// Creation Date: 29 February 2001
// Last Modified: 29 February 2001
// Filename:      ...sig/doc/examples/improv/hciImprov/hciTemplate.cpp
// Syntax:        C++; hciImprov 2.3
//  
// Description: This is a template which you fill with algorithms 
//              in the following functions to create an hciImprov program.
//

#include "hciImprovGUI.h"      

/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   cout << "Write the program description here" << endl;
} 

void initialization(void) { }
void finishup(void) { }

/*-------------------- main loop algorithms -----------------------------*/

void mainloopalgorithms(void) { }

/*-------------------- triggered algorithms -----------------------------*/

void mididata(int intime, int command, int p1, int p2) { 
   if (command == 0xa0 && p1 == 10) {
      canvas.point(p2);
   }
}

void keyboardchar(int key) { }


/*------------------ end improvization algorithms -----------------------*/


// md5sum: 702237fb1abe260465e9fb837da3dfba hcidisplay.cpp [20050403]
