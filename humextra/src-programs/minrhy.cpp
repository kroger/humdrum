//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun  9 14:48:41 PDT 2001
// Last Modified: Sat Jun  9 14:48:45 PDT 2001
// Filename:      ...museinfo/examples/all/minrhy.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/minrhy.cpp
// Syntax:        C++; museinfo
//
// Description:   calculates the minimum timebase which is the least common
//                multiple of all rhythms in the file.
//

#include "humdrum.h"

int findlcm(Array<int>& list);
int GCD(int a, int b);

int main(int argc, char** argv) {
   if (argc < 2) {
      cout << "Usage: " << argv[0] << " input-kern-file" << endl;
      exit(1);
   }
   HumdrumFile hfile;
   Array<int> timebase;
   timebase.setSize(argc-1);
   timebase.setAll(0);
   timebase.allowGrowth(0);

   for (int i=1; i<argc; i++) {
      hfile.clear();
      hfile.read(argv[i]);
      hfile.analyzeRhythm();
      if (argc > 2) {
         cout << argv[i] << ":\t";
      }
      cout << hfile.getMinTimeBase() << "\n";
      timebase[i-1] = hfile.getMinTimeBase();
   }

   if (argc > 2) {
      cout << "all:\t" << findlcm(timebase) << endl;
   }

}



//////////////////////////////
//
// findlcm -- find the least common multiple between rhythms
//

int findlcm(Array<int>& rhythms) {
   if (rhythms.getSize() == 0) {
      return 0;
   }
   int output = rhythms[0];
   for (int i=1; i<rhythms.getSize(); i++) {
      output = output * rhythms[i] / GCD(output, rhythms[i]);
   }

   return output;
}

 

//////////////////////////////
//
// GCD -- greatest common divisor
//
 
int GCD(int a, int b) {
   if (b == 0) {
      return a;
   }
   int z = a % b;
   a = b;
   b = z;
   return GCD(a, b);
}    
// md5sum: 5336efc67af0020221fbeb9eac7b7aa7 minrhy.cpp [20050403]
