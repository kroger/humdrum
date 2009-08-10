//
// Programmer:    Leland Stanford <leland@stanford.edu>
// Creation Date: 29 February 2003
// Last Modified: 29 February 2003
// Filename:      ...sig/doc/examples/all/tabletTemplate.cpp
// Syntax:        C++; tabletImprov 2.0
//  
// Description: Fill in the following functions to create an improv program.
//

#include "tabletImprov.h" 
int p1pold = 0;
int p2pold = 0;
int wold = 0;

/*----------------- beginning of improvization algorithms ---------------*/

/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << "Write the program description here" << endl;
} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     interface once at the start of the program.  Put items
//     here which need to be initialized at the beginning of
//     the program.
//

void initialization(void) { }



//////////////////////////////
//
// finishup -- this function is called by the improv interface
//     whenever the program is exited.  Put items here which
//     need to be taken care of when the program is finished.
//

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

//////////////////////////////
//
// mainloopalgorithms -- this function is called by the improv interface
//   continuously while the program is running.  The global variable t_time
//   which stores the current time is set just before this function is
//   called and remains constant while in this functions.
//

void mainloopalgorithms(void) { 
   if (tablet.p1w() != wold) {
      cout << "Wheel: " << tablet.p1w() << endl;
      wold = tablet.p1w();
   }
   if (tablet.p1p() != p1pold) {
      cout << "pen 1 pressure: " << tablet.p1p() << endl;
      p1pold = tablet.p1p();
   }
   if (tablet.p2p() != p2pold) {
      cout << "pen 1 pressure: " << tablet.p2p() << endl;
      p2pold = tablet.p2p();
   }

}


/*-------------------- triggered algorithms -----------------------------*/

void pen1trigon(void) { 
   cout << "pen1trigon:\tx=" << tablet.p1x()
        << "\ty=" << tablet.p1y() << endl;     
}

void pen1trigoff(void) { 
   cout << "pen1trigoff:\tx=" << tablet.p1x()
        << "\ty=" << tablet.p1y() << endl;    
}

void pen2trigon(void) { 
   cout << "pen2trigon:\tx=" << tablet.p1x()
        << "\ty=" << tablet.p1y() << endl;     
}

void pen2trigoff(void) { 
   cout << "pen2trigoff:\tx=" << tablet.p1x()
        << "\ty=" << tablet.p1y() << endl;    
}

void pen1button1on(void)  { cout << "pen1button1on" << endl;  }
void pen1button1off(void) { cout << "pen1button1off" << endl; }
void pen1button2on(void)  { cout << "pen1button2on" << endl;  }
void pen1button2off(void) { cout << "pen1button2off" << endl; }
void pen2button1on(void)  { cout << "pen2button1on" << endl;  }
void pen2button1off(void) { cout << "pen2button1off" << endl; }
void pen2button2on(void)  { cout << "pen2button2on" << endl;  }
void pen2button2off(void) { cout << "pen2button2off" << endl; }



///////////////////////////////
//
// keyboardchar -- this function is called by the improv interface
//     whenever a key is pressed on the computer keyboard.
//     Put commands here which will be executed when a key is
//     pressed on the computer keyboard.
//

void keyboardchar(int key) { }




// md5sum: af86e414b9e503c7ad1475cbce66ab48 tabletTemplate.cpp [20050403]
