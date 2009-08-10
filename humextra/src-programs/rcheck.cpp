//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Dec  4 17:28:06 PST 2000
// Last Modified: Mon Dec  4 17:28:09 PST 2000
// Last Modified: Tue Dec 19 14:33:57 PST 2000
// Filename:      ...sig/examples/all/rcheck.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/rcheck.cpp
// Syntax:        C++; museinfo
//
// Description:   Print the rhythmic information for a Humdrum file
//                containing at least one **kern spine.
//

#include "humdrum.h"

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif

#define DEBUG 0

int main(int argc, char** argv) {
   if (argc > 2) {
      cout << "Usage: " << argv[0] << " input-file" << endl;
      exit(1);
   }

   HumdrumFile hfile;
   if (argc == 2) {
      hfile.read(argv[1]);
   } else if (argc == 1) {
      hfile.read(cin);
   }
   
   hfile.analyzeRhythm("4", DEBUG);

   cout << "absbeat\tdur\tbeat\t::\tdata\n";
   cout << ":::::::::::::::::::::::::::::::::::::::::::::::::::::\n";
   for (int i=0; i<hfile.getNumLines(); i++) {
      cout << hfile.getAbsBeat(i) << '\t'
           << hfile.getDuration(i) << '\t'
           << hfile.getBeat(i) << "\t::\t"
           << hfile.getLine(i) << endl;
   }

   return 0;
}



// md5sum: e85e2c07edf50cbd0282a11f206cf797 rcheck.cpp [20050403]
