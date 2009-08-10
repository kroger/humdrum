//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jan 14 16:39:15 PST 1999
// Last Modified: Thu Jan 14 16:56:37 PST 1999
// Filename:      ...improv/examples/byteTable/bytetable.cpp
// Syntax:        C++; improv
// $Smake:        g++ -O3 -o %b -Iinclude/sig %f -Llib -lsig && strip %b
//
// Description:   Prints a table of the number-base equivalents
//                for one byte of decimal, hexadecimal and binary
//                number systems.  The negative version will print
//                the two's compliment values for negative numbers
//                which are the common representation for computers
//                to use when dealing with negative numbers.
//

#include "Options.h"
#include <stdlib.h>
#include <ctype.h>

#ifndef OLDCPP
   #include <iostream>
   #include <iomanip>
   using namespace std;
#else
   #include <iostream.h>
   #include <iomanip.h>
#endif

typedef unsigned char uchar;

#define PLAIN_STYLE 0
#define COMMA_STYLE 1
#define ZERO_STYLE  2

// Global variables for command-line options.
Options  options;                   // for command-line processing
int      binaryStyle = PLAIN_STYLE; // for binary number display style
int      asciiQ = 0;                // for printing ASCII eqivalents


// function declarations:
void checkOptions(Options& opts);
void example(void);
void printbinarybyte(int number, ostream& out, int style=0);
void usage(const char* command);


///////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv) {
   options.setOptions(argc, argv);
   checkOptions(options);

   if (options.getBoolean("negative")) {
      char i = -128;
      for (int j=-128; j<128; j++, i++) {
         cout << dec << (int)i << '\t';
         if ((binaryStyle & ZERO_STYLE) && (i>0 && i<0x10)) {
            cout << '0';
         }
         cout << hex << ((int)(uchar)i) << '\t';
         printbinarybyte((uchar)i, cout, binaryStyle);
         if (asciiQ && isprint(i)) {
            cout << "\t'" << (char)i << "'";
         }
         cout << endl;
      }
   } else {
      for (int k=0; k<256; k++) {
         cout << dec << k << '\t';
         if ((binaryStyle & ZERO_STYLE) && k<0x10) {
            cout << '0';
         }
         cout << hex << k << '\t';
         printbinarybyte(k, cout, binaryStyle);
         if (asciiQ && isprint(k)) {
            cout << "\t'" << (char)k << "'";
         }
         cout << endl;
      }
   }
   

   return 0;
}

///////////////////////////////////////////////////////////////////////////
   

//////////////////////////////
//
// checkOptions --
//

void checkOptions(Options& opts) {   // options are:
   opts.define("n|negative=b");      //    boolean for twos complement negative
   opts.define("c|comma=b");         //    boolean for adding comma to binary
   opts.define("z|leading-zeros=b"); //    boolean for adding leading zeros
   opts.define("a|ascii=b");         //    boolean for adding ASCII chars
   opts.define("author=b");
   opts.define("version=b");
   opts.define("example=b");
   opts.define("h|help=b");
   opts.process();

   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
              "craig@ccrma.stanford.edu, January 1999" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << "bytetable, version 1.0 (14 Jan 1999)\n"
              "compiled: " << __DATE__ << endl;
      exit(0);
   } else if (opts.getBoolean("help")) {
      usage(opts.getCommand());
      exit(0);
   } else if (opts.getBoolean("example")) {
      example();
      exit(0);
   }

   // determine output style variables
   if (opts.getBoolean("comma")) {
      binaryStyle |= COMMA_STYLE;
   }
   if (opts.getBoolean("leading-zeros")) {
      binaryStyle |= ZERO_STYLE;
   }
   if (opts.getBoolean("ascii")) {
      asciiQ = 1;
   }
}



//////////////////////////////
//
// example -- shows various command-line option calls to program.
//

void example(void) {
   cout << 
   "\n"
   "# display a table of dec/hex/binary values for one byte\n"
   "bytetable\n"
   "\n"
   "# display a table of dec/hex/binary using the signed range\n"
   "bytetable -n\n"
   "\n"
   "# display a table of dec/hex/binary filling in empty digits\n"
   "bytetable -z\n"
   "\n"
   "# display a table of dec/hex/binary adding a comma in the binaries\n"
   "bytetable -c\n"
   "\n"
   << endl;
}



//////////////////////////////
//
// printbinarybyte -- prints the number in binary form with no
//    leading zeros.
//

void printbinarybyte(int number, ostream& out, int style) {
   unsigned long value = (unsigned long) number;
   int flag = 0;
   if (binaryStyle & ZERO_STYLE) {
      flag = 1;
   }

   for (int i=0; i<8; i++) {
      if (value & (1 << (7-i))) {
         flag = 1;
      }
      
      if (flag) {
         if (value & (1 << (7-i))) {
            out << '1';
         } else {
            out << '0';
         }
         if ((i==3) && (binaryStyle & COMMA_STYLE)) {
            out << ',';
         }
      }
   }

   if (flag == 0 && !(binaryStyle & ZERO_STYLE)) {
      out << '0';
   }
}




//////////////////////////////
//
// usage -- how to run this program from the command-line.
//

void usage(const char* command) {
   cout << 
   "\n"
   "Print a number-conversion table for one byte\n"
   "\n"
   "Usage: " << command << " [-a][-c][-n][-z] number(s)\n"
   "\n"
   "Options:\n"
   "   -a = add a column of printable ASCII character equivalents\n"
   "   -c = add a comma in the middle of binary numbers\n"
   "   -n = print a table which includes the negative numbers\n"
   "   -z = fill in empty digits for binary and hex numbers\n"
   "   --options = list all options, default values, and aliases\n"
   "\n"
   << endl;
}



// md5sum: 8c1d55bfcbc2bc3608d9838536d515ae bytetable.cpp [20050403]
