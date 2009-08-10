//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Sep 23 12:28:00 PDT 2000
// Last Modified: Sat Sep 23 12:28:03 PDT 2000
// Filename:      ...sig/doc/examples/all/hciBasic/chanscope.cpp
// Syntax:        C++; hciImprov 2.3
//  
// Description: Display all MIDI messages that come into the computer.
//              via chanscope selection from the comptuter keyboard.    
//

#include "hciImprov.h"      

#define DEC_DISPLAY 0
#define HEX_DISPLAY 1

int channelDisplay = 17;
int oldDisplay = -1;
int commandDisplay = DEC_DISPLAY;

/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   cout << 
   "chanscope -- Displays MIDI message that come into the computer\n"
   "==============================================================\n"
   "  1-9: display channels 1  though 9                    \n"
   "  q-i: display channels 10 though 16                   \n"
   "    o: display system messages                         \n"
   "space: toggle display of messages                      \n"
   "    d: display MIDI commands in decimal format         \n"
   "    h: display MIDI commands in hexadecimal format     \n"
   << endl;
} 

void initialization(void) { 
   description();
}

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

void mainloopalgorithms(void) { }

/*-------------------- triggered algorithms -----------------------------*/


void mididata(int intime, int command, int p1, int p2) { 
   int channel = command & 0x0f;
   if (command & 0xf0 == 0xf0) {   // system messages
      channel = 16;
   }
   if (channel == channelDisplay || channelDisplay < 0) {
      cout << "Time    = " << intime  << "\t";
      if (commandDisplay == DEC_DISPLAY) {
         cout << "Command = " << dec << command << "\t";
      } else {
         cout << "Command = 0x" << hex << command << dec << "\t";
      }
      cout << "P1 = " << p1 << "\t"
           << "P2 = " << p2 
           << endl;
   }
}


void keyboardchar(int key) { 
   switch (key) {
      case '1':  channelDisplay = 0;   break;
      case '2':  channelDisplay = 1;   break;
      case '3':  channelDisplay = 2;   break;
      case '4':  channelDisplay = 3;   break;
      case '5':  channelDisplay = 4;   break;
      case '6':  channelDisplay = 5;   break;
      case '7':  channelDisplay = 6;   break;
      case '8':  channelDisplay = 7;   break;
      case '9':  channelDisplay = 8;   break;
      case 'q':  channelDisplay = 9;   break;
      case 'w':  channelDisplay = 10;  break;
      case 'e':  channelDisplay = 11;  break;
      case 'r':  channelDisplay = 12;  break;
      case 't':  channelDisplay = 13;  break;
      case 'y':  channelDisplay = 14;  break;
      case 'u':  channelDisplay = 15;  break;
      case 'i':  channelDisplay = 16;  break;
      case ' ':  
         if (channelDisplay == 17) {
            channelDisplay = oldDisplay;
         } else {
            oldDisplay = channelDisplay;
            channelDisplay = 17; 
         }
         break;
      case 'd':  commandDisplay = DEC_DISPLAY; return;
      case 'h':  commandDisplay = HEX_DISPLAY; return;
      default:   channelDisplay = -1; 
   }
   if (channelDisplay >= 0 && channelDisplay < 16) {
      cout << "Displaying MIDI channel " << channelDisplay + 1 
           << " data." << endl;
   } else if (channelDisplay < 0) {
      cout << "Displaying all MIDI channels " << endl;
   } else if (channelDisplay == 16) {
      cout << "Displaying system messages" << endl;
   }

}


/*------------------ end improvization algorithms -----------------------*/


// md5sum: 47ebe8e53aaa843c2833307d89472e56 chanscope.cpp [20050403]
