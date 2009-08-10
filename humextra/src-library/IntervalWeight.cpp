//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jul 24 12:35:50 PDT 2002
// Last Modified: Wed Jul 24 12:35:55 PDT 2002
// Filename:      ...sig/include/sigInfo/IntervalWeight.cpp
// Web Address:   http://sig.sapp.org/include/sigInfo/IntervalWeight.cpp
// Syntax:        C++ 
//
// Description:   Creates interval weights for chord root calculations.
//

#include "IntervalWeight.h"
#include "HumdrumFile.h"
#include "Convert.h"

#include <math.h>
#include <string.h>

#ifndef OLDCPP
   using namespace std;
#endif

#define MYPI 3.141592653589793238462643383279

#ifndef M_PI
   #define M_PI 3.141592653589793238462643383279
#endif



//////////////////////////////
//
// IntervalWeight::IntervalWeight --
//

IntervalWeight::IntervalWeight(void) { 
   weights.setSize(40);
   weights.allowGrowth(0);
   weights.setAll(1.0);
   wtype = IWTYPE_NONE;
}



//////////////////////////////
//
// IntervalWeight::~IntervalWeight --
//

IntervalWeight::~IntervalWeight() { 
   weights.setSize(0);
}



//////////////////////////////
//
// IntervalWeight::getSize --
//

int IntervalWeight::getSize(void) { 
   return weights.getSize();
}



//////////////////////////////
//
// IntervalWeight::getType --
//

int IntervalWeight::getType(void) { 
   return wtype;
}



//////////////////////////////
//
// IntervalWeight::setAll --
//

void IntervalWeight::setAll(double value) {
   weights.setAll(value);
}



//////////////////////////////
//
// IntervalWeight::operator[] --
//

double& IntervalWeight::operator[](int index) {
   return weights[index];
}

//////////////////////////////
//
// IntervalWeight::getWeight
//   default value: chromatic = 0;
//

double& IntervalWeight::getWeight(int diatonic, int chromatic) {
   int index = 0;
   diatonic = (diatonic + 700) % 7;
   switch (diatonic) {
      case 0: index = 0;  break;
      case 1: index = 6;  break;
      case 2: index = 12; break;
      case 3: index = 17; break;
      case 4: index = 23; break;
      case 5: index = 29; break;
      case 6: index = 35; break;
   }
   index += chromatic;
   index = (index + 400) % 40;
   return weights[index];
}


///////////////////////////////////////////////////////////////////////
//
// Set the weightings
//


//////////////////////////////
//
// IntervalWeight::setFromFile -- read humdrumfile with weights.
// IntervalWeight::readWeights -- read humdrumfile with weights.
//

void IntervalWeight::setFromFile(const char* filename) { 
   readWeights(filename);
}


void IntervalWeight::readWeights(const char* filename) { 
   HumdrumFile infile(filename);
   setAll(100000);
   int i, j;
   int root;
   double weight;
   for (i=0; i<infile.getNumLines(); i++) {
      root = -1;
      weight = 10000;
      if (!infile[i].isData()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp("**kern", infile[i].getExInterp(j)) == 0) {
            root = Convert::kernToBase40(infile[i][j]) % 40;
         } else if (strcmp("**weight", infile[i].getExInterp(j)) == 0) {
            weight = strtod(infile[i][j], NULL);
         }
      }
      if (root >= 0) {
         weights[root] = weight;
      }
   }
}



//////////////////////////////
//
// IntervalWeight::setDiatonic --
//

void IntervalWeight::setDiatonic(void) { 
   setDiatonic(weights);
   wtype = IWTYPE_DIATONIC;
}


void IntervalWeight::setDiatonic(Array<double>& anArray) {
   anArray.setSize(40);
   anArray[0]  = 0;	// P1	C
   anArray[1]  = 0;	// A1	C#
   anArray[2]  = 0;	// AA1	C##
   anArray[3]  = 10000;	// X
   anArray[4]  = 4;	// d2	D--
   anArray[5]  = 4;	// m2	D--
   anArray[6]  = 4;	// M2	D
   anArray[7]  = 4;	// A2	D#
   anArray[8]  = 4;	// AA2	D##
   anArray[9]  = 10000;	// X
   anArray[10] = 1;	// d3	E--
   anArray[11] = 1;	// m3	E-
   anArray[12] = 1;	// M3	E
   anArray[13] = 1;	// A3	E#
   anArray[14] = 1;	// AA3	E##
   anArray[15] = 5;	// dd4	F--
   anArray[16] = 5;	// d4	F-
   anArray[17] = 5;	// P4	F
   anArray[18] = 5;	// A4	F#
   anArray[19] = 5;	// AA4	F##
   anArray[20] = 10000;	// X
   anArray[21] = 2;	// dd5	G--
   anArray[22] = 2;	// d5	G-
   anArray[23] = 2;	// P5	G
   anArray[24] = 2;	// A5	G#
   anArray[25] = 2;	// AA5	G##
   anArray[26] = 10000;	// X
   anArray[27] = 6;	// d6	A--
   anArray[28] = 6;	// m6	A-
   anArray[29] = 6;	// M6	A
   anArray[30] = 6;	// A6	A#
   anArray[31] = 6;	// AA6	A##
   anArray[32] = 10000;	// X
   anArray[33] = 3;	// d7	B--
   anArray[34] = 3;	// m7	B-
   anArray[35] = 3;	// M7	B
   anArray[36] = 3;	// A7	B#
   anArray[37] = 3;	// AA7	B##
   anArray[38] = 0;	// dd1	C--
   anArray[39] = 0;	// d1	C-


}



//////////////////////////////
//
// IntervalWeight::setChromatic1 --
//

void IntervalWeight::setChromatic1(double theta) { 
   setChromatic1(theta, weights);
   wtype = IWTYPE_CHROMATIC1;
}


void IntervalWeight::setChromatic1(double theta, Array<double>& anArray) { 
   static int xpos[40] = {
        7, /* C-- */  7, /* C-  */  0, /* C   */  7, /* C#  */  7, /* C## */
    10000, /* X   */
        4, /* D-- */  4, /* D-  */  4, /* D   */  4, /* D#  */  4, /* D## */
    10000, /* X   */
        8, /* E-- */  1, /* E-  */  1, /* E   */  8, /* E#  */  8, /* E## */
        5, /* F-- */  5, /* F-  */  5, /* F   */  5, /* F#  */  5, /* F## */
    10000, /* X   */
        9, /* G-- */  2, /* G-  */  2, /* G   */  2, /* G#  */  9, /* G## */
    10000, /* X   */
        6, /* A-- */  6, /* A-  */  6, /* A   */  6, /* A#  */  6, /* A## */
    10000, /* X   */
        3, /* B-- */  3, /* B-  */  3, /* B   */  3, /* B#  */ 10  /* B## */
   };

   static int ypos[40] = {
       -5, /* C-- */ -3, /* C-  */  0, /* C   */  1, /* C#  */  3, /* C## */
    10000, /* X   */
       -4, /* D-- */ -2, /* D-  */  0, /* D   */  2, /* D#  */  4, /* D## */
    10000, /* X   */
       -4, /* E-- */ -1, /* E-  */  1, /* E   */  2, /* E#  */  4, /* E## */
       -5, /* F-- */ -3, /* F-  */ -1, /* F   */  1, /* F#  */  3, /* F## */
    10000, /* X   */
       -5, /* G-- */ -2, /* G-  */  0, /* G   */  2, /* G#  */  3, /* G## */
    10000, /* X   */
       -4, /* A-- */ -2, /* A-  */  0, /* A   */  2, /* A#  */  4, /* A## */
    10000, /* X   */
       -3, /* B-- */ -1, /* B-  */  1, /* B   */  3, /* B#  */  4  /* B## */
   };


   // convert theta to radians:
   theta = theta * MYPI / 180.0;

   anArray.setSize(40);
   double cost2 = cos(theta) * cos(theta);
   double sint2 = sin(theta) * sin(theta);
   int i;
   for (i=0; i<40; i++) {
      anArray[i] = sqrt(cost2*xpos[i]*xpos[i] + sint2*ypos[i]*ypos[i]);
   }

   anArray[5]  = 10000;	// invalid interval
   anArray[11] = 10000;	// invalid interval
   anArray[22] = 10000;	// invalid interval
   anArray[28] = 10000;	// invalid interval
   anArray[34] = 10000;	// invalid interval

   // shift everything by two slots so that they represent interval classes
 
   double pos1 = anArray[0];
   double pos2 = anArray[1];
   for (i=0; i<38; i++) {
      anArray[i] = anArray[i+2];
   }
   anArray[38] = pos1;
   anArray[39] = pos2;

}



//////////////////////////////
//
// IntervalWeight::setChromatic2 --
//

void IntervalWeight::setChromatic2(double theta1, double theta2) {
   setChromatic2(theta1, theta2, weights);
   wtype = IWTYPE_CHROMATIC2;
}

void IntervalWeight::setChromatic2(double theta1, double theta2,
      Array<double>& anArray) {
   static int xpos[40] = {
        7, /* C-- */  7, /* C-  */  0, /* C   */  7, /* C#  */  7, /* C## */
    10000, /* X   */
        4, /* D-- */  4, /* D-  */  4, /* D   */  4, /* D#  */  4, /* D## */
    10000, /* X   */
        8, /* E-- */  1, /* E-  */  1, /* E   */  8, /* E#  */  8, /* E## */
        5, /* F-- */  5, /* F-  */  5, /* F   */  5, /* F#  */  5, /* F## */
    10000, /* X   */
        9, /* G-- */  2, /* G-  */  2, /* G   */  2, /* G#  */  9, /* G## */
    10000, /* X   */
        6, /* A-- */  6, /* A-  */  6, /* A   */  6, /* A#  */  6, /* A## */
    10000, /* X   */
        3, /* B-- */  3, /* B-  */  3, /* B   */  3, /* B#  */ 10  /* B## */
   };

   static int ypos[40] = {
       -5, /* C-- */ -3, /* C-  */  0, /* C   */  1, /* C#  */  3, /* C## */
    10000, /* X   */
       -4, /* D-- */ -2, /* D-  */  0, /* D   */  2, /* D#  */  4, /* D## */
    10000, /* X   */
       -4, /* E-- */ -1, /* E-  */  1, /* E   */  2, /* E#  */  4, /* E## */
       -5, /* F-- */ -3, /* F-  */ -1, /* F   */  1, /* F#  */  3, /* F## */
    10000, /* X   */
       -5, /* G-- */ -2, /* G-  */  0, /* G   */  2, /* G#  */  3, /* G## */
    10000, /* X   */
       -4, /* A-- */ -2, /* A-  */  0, /* A   */  2, /* A#  */  4, /* A## */
    10000, /* X   */
       -3, /* B-- */ -1, /* B-  */  1, /* B   */  3, /* B#  */  4  /* B## */
   };

   anArray.setSize(40);
   theta1 = theta1 * MYPI / 180.0;
   theta2 = theta2 * MYPI / 180.0;

   int i;
   double x, y, value;
   for (i=0; i<40; i++) {
      x = xpos[i] * cos(theta1);
      y = ypos[i] * sin(theta1);
      value = y*y + x*x + 2.0 * x * y * sin(theta2);
      if (value > 0) {
         anArray[i] = sqrt(value);
      } else {
         anArray[i] = 0;
      }
   }

   anArray[5]  = 10000;	// invalid interval
   anArray[11] = 10000;	// invalid interval
   anArray[22] = 10000;	// invalid interval
   anArray[28] = 10000;	// invalid interval
   anArray[34] = 10000;	// invalid interval
}



//////////////////////////////
//
// IntervalWeight::setChromatic2b --
//

void IntervalWeight::setChromatic2b(double theta1, double theta2) {
   setChromatic2b(theta1, theta2, weights);
   wtype = IWTYPE_CHROMATIC2B;
}

void IntervalWeight::setChromatic2b(double theta1, double theta2,
      Array<double>& anArray) {

   // p1 = perfect 5th distance
   static int p1[40] = {
        6, /* C-- */  5, /* C-  */  0, /* C   */  4, /* C#  */  5, /* C## */
    10000, /* X   */
        4, /* D-- */  3, /* D-  */  2, /* D   */  3, /* D#  */  4, /* D## */
    10000, /* X   */
        6, /* E-- */  1, /* E-  */  1, /* E   */  5, /* E#  */  6, /* E## */
        5, /* F-- */  4, /* F-  */  3, /* F   */  3, /* F#  */  4, /* F## */
    10000, /* X   */
        7, /* G-- */  2, /* G-  */  1, /* G   */  2, /* G#  */  6, /* G## */
    10000, /* X   */
        5, /* A-- */  4, /* A-  */  3, /* A   */  4, /* A#  */  5, /* A## */
    10000, /* X   */
        3, /* B-- */  2, /* B-  */  2, /* B   */  3, /* B#  */  7  /* B## */
   };

   // p2 = major or minor third distance; negative are for majors
   static int p2[40] = {
       -5, /* C-- */ -3, /* C-  */  0, /* C   */  1, /* C#  */  3, /* C## */
    10000, /* X   */
       -4, /* D-- */ -2, /* D-  */  0, /* D   */  2, /* D#  */  4, /* D## */
    10000, /* X   */
       -4, /* E-- */ -1, /* E-  */  1, /* E   */  2, /* E#  */  4, /* E## */
       -5, /* F-- */ -3, /* F-  */ -1, /* F   */  1, /* F#  */  3, /* F## */
    10000, /* X   */
       -5, /* G-- */ -2, /* G-  */  0, /* G   */  2, /* G#  */  3, /* G## */
    10000, /* X   */
       -4, /* A-- */ -2, /* A-  */  0, /* A   */  2, /* A#  */  4, /* A## */
    10000, /* X   */
       -3, /* B-- */ -1, /* B-  */  1, /* B   */  3, /* B#  */  4  /* B## */
   };

   // p3 = theta 1 or theta 2
   static double p3[40] = {
        1, /* C-- */  1, /* C-  */  0, /* C   */  2, /* C#  */  2, /* C## */
    10000, /* X   */
        1, /* D-- */  1, /* D-  */  0, /* D   */  2, /* D#  */  2, /* D## */
    10000, /* X   */
        1, /* E-- */  1, /* E-  */  2, /* E   */  2, /* E#  */  2, /* E## */
        1, /* F-- */  1, /* F-  */  1, /* F   */  2, /* F#  */  2, /* F## */
    10000, /* X   */
        1, /* G-- */  1, /* G-  */  0, /* G   */  2, /* G#  */  2, /* G## */
    10000, /* X   */
        1, /* A-- */  1, /* A-  */  0, /* A   */  2, /* A#  */  2, /* A## */
    10000, /* X   */
        1, /* B-- */  1, /* B-  */  2, /* B   */  2, /* B#  */  2  /* B## */
   };

   // convert thetas to radians:
   theta1 = theta1 * MYPI / 180.0;
   theta2 = theta2 * MYPI / 180.0;
   int i;
   double theta;
   double avalue;
   double bvalue = 0.0;
   anArray.setSize(40);
   for (i=0; i<40; i++) {
      avalue = p1[i] * sin(MYPI - theta1 - theta2)/sin(theta2);
      if (p2[i] < 0) {
         bvalue = -p2[i];
      } else {
         bvalue = p2[i] * sin(theta1)/sin(theta2);
      }
      if (p3[i] == 1) {
         theta = theta1;
      } else if (p3[i] == 2) {
         theta = theta2;
      } else if (p3[i] == 0) {
         theta = 0;
      } else {
         theta = MYPI;
      }
      anArray[i] = sqrt(avalue * avalue + bvalue * bvalue -
            2.0 * avalue * bvalue * cos(theta));
   }

   anArray[5]  = 10000;	// invalid interval
   anArray[11] = 10000;	// invalid interval
   anArray[22] = 10000;	// invalid interval
   anArray[28] = 10000;	// invalid interval
   anArray[34] = 10000;	// invalid interval

}



///////////////////////////////
//
// IntervalWeight::setCircular --
//

void IntervalWeight::setCircular(double theta, double ascale) {
   setCircular(theta, ascale, weights);
   wtype = IWTYPE_CIRCULAR;
}


void IntervalWeight::setCircular(double theta, double ascale,
      Array<double>& anArray) {

   static int xpos[40] = {
        7, /* C-- */  7, /* C-  */  0, /* C   */  7, /* C#  */  7, /* C## */
    10000, /* X   */
        4, /* D-- */  4, /* D-  */  4, /* D   */  4, /* D#  */  4, /* D## */
    10000, /* X   */
        8, /* E-- */  1, /* E-  */  1, /* E   */  8, /* E#  */  8, /* E## */
        5, /* F-- */  5, /* F-  */  5, /* F   */  5, /* F#  */  5, /* F## */
    10000, /* X   */
        9, /* G-- */  2, /* G-  */  2, /* G   */  2, /* G#  */  9, /* G## */
    10000, /* X   */
        6, /* A-- */  6, /* A-  */  6, /* A   */  6, /* A#  */  6, /* A## */
    10000, /* X   */
        3, /* B-- */  3, /* B-  */  3, /* B   */  3, /* B#  */ 10  /* B## */
   };

   static int ypos[40] = {
       -5, /* C-- */ -3, /* C-  */  0, /* C   */  1, /* C#  */  3, /* C## */
    10000, /* X   */
       -4, /* D-- */ -2, /* D-  */  0, /* D   */  2, /* D#  */  4, /* D## */
    10000, /* X   */
       -4, /* E-- */ -1, /* E-  */  1, /* E   */  2, /* E#  */  4, /* E## */
       -5, /* F-- */ -3, /* F-  */ -1, /* F   */  1, /* F#  */  3, /* F## */
    10000, /* X   */
       -5, /* G-- */ -2, /* G-  */  0, /* G   */  2, /* G#  */  3, /* G## */
    10000, /* X   */
       -4, /* A-- */ -2, /* A-  */  0, /* A   */  2, /* A#  */  4, /* A## */
    10000, /* X   */
       -3, /* B-- */ -1, /* B-  */  1, /* B   */  3, /* B#  */  4  /* B## */
   };


   // convert theta to radians:
   theta = theta * MYPI / 180.0;

   anArray.setSize(40);
   double y1;
   double value;
   int i;
   for (i=0; i<40; i++) {
      y1 = ascale * ypos[i] + 1.0;
      value = y1*y1 + 1.0 - 2.0 * y1 * cos(theta * xpos[i]);
      if (value > 0) {
         anArray[i] = sqrt(value);
      } else {
         anArray[i] = 0.0;
      }
   }

   anArray[5]  = 10000;	// invalid interval
   anArray[11] = 10000;	// invalid interval
   anArray[22] = 10000;	// invalid interval
   anArray[28] = 10000;	// invalid interval
   anArray[34] = 10000;	// invalid interval

}



//////////////////////////////
//
// IntervalWeight::setSpiral --
//

void IntervalWeight::setSpiral(double radius, double height) {
   setSpiral(radius, height, weights);
   wtype = IWTYPE_SPIRAL;
}


void IntervalWeight::setSpiral(double radius, double height, 
      Array<double>& anArray) {

   int i;
   int ii;

   double valx;
   double valy;
   double valz;

   double rvalx = radius * sin(0 * M_PI / 2.0);
   double rvaly = radius * cos(0 * M_PI / 2.0);
   double rvalz = height * 0;

   for (i=0; i<20; i++) {
      ii = (i * 23) % 40;
      valx = radius * sin(i  * M_PI / 2.0);
      valy = radius * cos(i  * M_PI / 2.0);
      valz = height * i;

      valx = valx - rvalx;
      valy = valy - rvaly;
      valz = valz - rvalz;

      anArray[ii] = sqrt(valx*valx + valy*valy + valz*valz);
 
      if (i < 16) {
         anArray[(-i * 23 + 40000) % 40] = anArray[ii];
      }
   }

   anArray[5-2]  = 10000;	// invalid interval
   anArray[11-2] = 10000;	// invalid interval
   anArray[22-2] = 10000;	// invalid interval
   anArray[28-2] = 10000;	// invalid interval
   anArray[34-2] = 10000;	// invalid interval

}



//////////////////////////////
//
// IntervalWeight::newprint --
//

ostream& IntervalWeight::newprint(ostream& out) {
   out << "**kern\t**weight\n";
   int i;
   char buffer[64] = {0};
   for (i=0; i<40; i++) {
      Convert::base40ToKern(buffer, i + 4*40);
      if (buffer[0] == '\0') {
         continue;
      }
      out << buffer << "\t" << weights[(i-2+40)%40] << "\n";
   }
   out << "*-\t*-" << endl;
   return out;
}




//////////////////////////////
//
// operator<< -- print out interval weights.
//

ostream& operator<<(ostream& out, IntervalWeight& weights) {
   out << "**kern\t**weight\n";
   int i;
   char buffer[64] = {0};
   for (i=0; i<40; i++) {
      Convert::base40ToKern(buffer, i + 4*40);
      if (buffer[0] == '\0') {
         continue;
      }
      out << buffer << "\t" << weights[i] << "\n";
   }
   out << "*-\t*-" << endl;
   return out;
}


// md5sum: 8917fd046eb9f2836eb6c926820a78ee IntervalWeight.cpp [20050403]
