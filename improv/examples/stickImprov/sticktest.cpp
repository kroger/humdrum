//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 16 16:16:11 PDT 2000
// Last Modified: Sun Jul 16 17:58:37 PDT 2000
// Filename:      ...sig/doc/examples/all/sticktest/sticktest.cpp
// Syntax:        C++; stickImprov 2.0
//  
// Description: Basic program testing the stick.
//

#include "improv.h"

AdamsStick stick;

void responseFunction(void) {
   cout << "l1 = \t"   << stick.loc1_7()  // FSR 1 Location in 7-bit range
        << "\tf1 = \t" << stick.s1f7()    // FSR 1 Force    in 7-bit range
        << "\tl2 = \t" << stick.loc2_7()  // FSR 2 Location in 7-bit range
        << "\tf2 = \t" << stick.s2f7()    // FSR 2 Force    in 7-bit range
        << "\tl3 = \t" << stick.loc3_7()  // FSR 3 Location in 7-bit range
        << "\tf3 = "   << stick.s3f7()    // FSR 3 Force    in 7-bit range
        << endl;
}


int main(void) {
   stick.response = responseFunction;
   stick.setStreamMode();

   while (1) {
      millisleep(20);
      stick.processIncomingMessages();
   }
}


// md5sum: 31eae0ba2d6b8146ff0351530e63fab7 sticktest.cpp [20050403]
