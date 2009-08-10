//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun May 13 14:00:43 PDT 2001
// Last Modified: Sun May 13 14:00:40 PDT 2001
// Filename:      ...sig/src/museinfo/humdrumfileextras.cpp
// Web Address:   http://sig.sapp.org/src/museinfo/humdrumfileextras.cpp
// Syntax:        C++ 
//
// Description:   Functions useful for use with the HumdrumFile class,
//		  but not necessarily using HumdrumFile class.
//

#include <math.h>

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif



//////////////////////////////////////////////////////////////////////////
//
// Krumhansl-Schmuckler key-finding algorithm functions.
//


//////////////////////////////
//
// analyzeKeyKS -- apply the Krumhansl-Schmuckler
//   key-finding algorithm.  Return value is the base-12 most likely
//   key.  scores 0-11 are the major keys starting on C major. 12-23 are
//   the minor keys starting on C minor.  input array scores must have
//   a size of 24 or greater.  input array pitch must have a size of "size".
//

int analyzeKeyKS(double* scores, double* distribution, int* pitch, 
      double* durations, int size, int rhythmQ, int binaryQ) {
   int i, j;
   int histogram[12] = {0};

   double majorKey[12] = {6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 
         5.19, 2.39, 3.66, 2.29, 2.88};
   double minorKey[12] = {6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 
         4.75, 3.98, 2.69, 3.34, 3.17};

// double majorKey[12] = {24.3279, -10.1283, 2.44009, -4.63092, -2.46215, 
//       7.58629, -8.60338, -11.5317, -24.1048, -7.61466, -26.2269, 7.95787};
// double minorKey[12] = {23.1015, -1.94184, 11.9991, 1.8769, -14.671, 7.08384,
//       6.36015 -3.36218, -9.21259, -25.3657, -25.0074, -1.49362};

   for (i=0; i<24; i++) {
      scores[i] = 0.0;
   }
   if (size == 0) {
      return -1;  // return -1 if not data to analyze
   }

   for (i=0; i<12; i++) {
      distribution[i] = 0.0;
   }

   // generate a histogram of pitches
   for (i=0; i<size; i++) {
      histogram[pitch[i]%12]++;
      if (rhythmQ) {
         distribution[pitch[i]%12] += durations[i]/4.0;  // dur in whole notes
      } else {
         distribution[pitch[i]%12] += 0.25;
      }
   }

   double sum = 0.0;
   double summaj = 0.0;
   double summin = 0.0;
   for (i=0; i<12; i++) {
      sum += distribution[i];
      summaj += majorKey[i];
      summin += minorKey[i];
   }

   double mean = sum/12.0;
   double major_mean = summaj / 12.0;
   double minor_mean = summin / 12.0;

   // Krumhansl-Kessler mean values:
   // double major_mean = 3.4825;
   // double minor_mean = 3.709167;

   double maj_numerator;
   double min_numerator;
   double maj_denominator;
   double min_denominator;
   double maj_temp;
   double min_temp;
   double temp;

   int subscript;
     
/*
   if (binaryQ) {
      double binaryDistribution[12] = {0};
      for (i=0; i<12; i++) {
         if (distribution[i] < mean) {
            binaryDistribution[i] = distribution[i];
         } else {
            binaryDistribution[i] = distribution[i];
         }
      } 

      double binaryMajorKey[12] = {1.75, 0, 1, -3.25, 1.23, 1, 0,  2.8, 0, 1, -0.4, 1};
      double binaryMinorKey[12] =              {2.35, 0, 1, 1, -3.85, 1.75, 0, 1.75, 1, 0.20, .75, .20};

// 100% for wtc2f (wtc1f 10 & 20 don't work [major too strong])
//    double binaryMajorKey[12] = {1.8, 0, 1, -3.25, 1.25, 1, 0,  2.8, 0, 1, -0.4, 1};
//    double binaryMinorKey[12] =              {2.35, 0, 1, 1, -3.85, 1.75, 0, 1.75, 1, 0.20, .75, .20};
// 100% for wtc1f (wtc2 13 doesn't work [minor too strong])
//    double binaryMajorKey[12] = {1.75, 0, 1, -3.25, 1, 1, 0,  2.85, 0, 1, -0.5, 1};
//    double binaryMinorKey[12] =              {2.40, 0, 1, 1, -3.65, 1.5, 0, 1.75, 1,0.20, .75, .25};
//  wtc1f10 and wtc2f13 don't work:
//    double binaryMajorKey[12] = {1.75, 0, 1, -3.25, 1.10, 1, 0,  2.85, 0, 1, -0.4, 1};
//    double binaryMinorKey[12] =              {2.40, 0, 1, 1, -3.65, 1.5, 0, 1.75, 1,0.20, .75, .25};
//    double binaryMajorKey[12] = {1.8, 0, 1, -3.25, 1.23, 1, 0,  2.8, 0, 1, -0.4, 1};
//    double binaryMinorKey[12] =              {2.35, 0, 1, 1, -3.85, 1.75, 0, 1.75, 1, 0.20, .75, .20};
//  wtc1f10 and wtc2f13 don't work:
//    double binaryMajorKey[12] = {1.75, 0, 1, -3.25, 1, 1, 0,  2.85, 0, 1, -0.5, 1};
//    double binaryMinorKey[12] =              {2.40, 0, 1, 1, -3.75, 1.5, 0, 1.75, 1,0.20, .75, .25};
// 100% for wtc2f (wtc1f 10 & 20 don't work [major too strong])
//    double binaryMajorKey[12] = {1.85, 0, 1, -3.25, 1, 1, 0,  2.85, 0, 1, -0.5, 1};
//    double binaryMinorKey[12] =              {2.40, 0, 1, 1, -3.75, 1.5, 0, 1.75, 1,0.20, .75, .25};
// 100% for wtc1f (wtc2 13 doesn't work [minor too strong])
//    double binaryMajorKey[12] = {1.85, 0, 1, -3.25, 1, 1, 0,  2.8, 0, 1, -0.5, 1};
//    double binaryMinorKey[12] =              {2.40, 0, 1, 1, -3.50, 1.5, 0, 1.75, 1,0.20, .75, .25};
// 100% for wtc1f (wtc2 13 doesn't work [minor too strong])
//    double binaryMajorKey[12] = {1.85, 0, 1, -3.25, 1, 1, 0,  2.75, 0, 1, -0.5, 1};
//    double binaryMinorKey[12] =             {2.40,  0, 1, 1, -3.75, 1.5, 0, 1.75, 1,0.20, .75, .25};
// 100% for wtc2f (wtc1f 10 & 20 don't work [major too strong])
//    double binaryMajorKey[12] = {2.0, 0, 1, -3.25, 1, 1, 0,  2.85, 0, 1, -0.5, 1};
//    double binaryMinorKey[12] =             {2.40, 0, 1, 1, -3.75, 1.5, 0, 1.75, 1,0.20, .75, .25};
// 100% for wtc1f: (wtc2f 13 doesn't work [minor too strong])
//    double binaryMajorKey[12] = {2.0, 0, 1, -3.5, 1, 1, 0,  3.0, 0, 1, -0.5, 1};
//    double binaryMinorKey[12] =             {2.40, 0, 1, 1, -3.5, 1.5, 0, 2.0, 1,0.25, .75, .25};
// 100% for wtc2f: (wtc1f 10 & 20 don't work [major too strong])
//    double binaryMajorKey[12] = {2.0, 0, 1, -3.5, 1, 1, 0,  3.0, 0, 1, -0.5, 1};
//    double binaryMinorKey[12] =             {2.25, 0, 1, 1, -3.5, 1.5, 0, 2.0, 1,0.25, .75, .25};
// 100% for wtc1f: (wtc2f 13 & 15 don't work [minor too strong])
//    double binaryMajorKey[12] = {3.5, 0, 1, -3.5, 1, 1, 0,  2.0, 0, 1, 0, 1};
//    double binaryMinorKey[12] =             {3.5, 0, 1, 1, -3.5, 1, 0, 2.0, 1,0.25, .75, .25};

      double* major_scores = scores;
      double* minor_scores = scores + 12;
      for (i=0; i<12; i++) {
         for (j=0; j<12; j++) {
            major_scores[i] += binaryMajorKey[j] * binaryDistribution[(i+j)%12];
            minor_scores[i] += binaryMinorKey[j] * binaryDistribution[(i+j)%12];
         }
      }

   } else {
*/
      double* r_major = scores;
      double* r_minor = scores + 12;

      for (i=0; i<12; i++) {
         maj_numerator = min_numerator = 0;
         maj_denominator = min_denominator = 0;
         maj_temp = min_temp = temp = 0;
   
         // Examine all pitches for each key,
         for (j=0; j<12; j++) {
            subscript = (i+j)%12;
            temp += pow(distribution[subscript] - mean, 2.0);
   
            // for the major keys:
            maj_numerator += (majorKey[j] - major_mean) *
                             (distribution[subscript] - mean);
            maj_temp += pow(majorKey[j] - major_mean, 2.0);
   
            // for the minor keys:
            min_numerator += (minorKey[j] - minor_mean) *
                             (distribution[subscript] - mean);
            min_temp += pow(minorKey[j] - minor_mean, 2.0);
         }
         
         maj_denominator = sqrt(maj_temp * temp);
         min_denominator = sqrt(min_temp * temp);
   
         if (maj_denominator <= 0.0) {
            r_major[i] = 0.0;
         } else {
            r_major[i] = maj_numerator / maj_denominator;
         }
   
         if (min_denominator <= 0.0) {
            r_minor[i] = 0.0;
         } else {
            r_minor[i] = min_numerator / min_denominator;
         }
      }
 //  }

   // now figure out which key is the largest.
   int bestkey = 0;
   for (i=1; i<24; i++) {
      if (scores[i] > scores[bestkey]) {
         bestkey = i;
      }
   }

   return bestkey;
}



//////////////////////////////
//
// analyzeKeyKS -- apply the Krumhansl-Schmuckler
//   key-finding algorithm.  Return value is the base-12 most likely
//   key.  scores 0-11 are the major keys starting on C major. 12-23 are
//   the minor keys starting on C minor.  input array scores must have
//   a size of 24 or greater.  input array pitch must have a size of "size".
//

int analyzeKeyKS2(double* scores, double* distribution, int* pitch, 
      double* durations, int size, int rhythmQ, double* majorKey,
      double* minorKey) {
   int i, j;
   int histogram[12] = {0};

   // double majorKey[12] = {6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 
   //       5.19, 2.39, 3.66, 2.29, 2.88};
   // double minorKey[12] = {6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 
   //       4.75, 3.98, 2.69, 3.34, 3.17};

   for (i=0; i<24; i++) {
      scores[i] = 0.0;
   }
   if (size == 0) {
      return -1;  // return -1 if no data to analyze
   }

   for (i=0; i<12; i++) {
      distribution[i] = 0.0;
   }

   // generate a histogram of pitches
   for (i=0; i<size; i++) {
      histogram[pitch[i]%12]++;
      if (rhythmQ) {
         distribution[pitch[i]%12] += durations[i]/4.0;  // dur in whole notes
      } else {
         distribution[pitch[i]%12] += 0.25;
      }
   }

//cout << "FREQENCYTABLE:\n";
//for (i=0; i<12; i++) {
//cout << i << "\t" << distribution[i] << endl;
//}
//cout << "\n\n\n";

   double sum = 0.0;
   double summaj = 0.0;
   double summin = 0.0;
   for (i=0; i<12; i++) {
      sum    += distribution[i];
      summaj += majorKey[i];
      summin += minorKey[i];
   }

   double mean = sum/12.0;
   double major_mean = summaj/12.0;
   double minor_mean = summin/12.0;


   // Krumhansl-Kessler weight means
   // double major_mean = 3.4825;
   // double minor_mean = 3.709167;

   double maj_numerator;
   double min_numerator;
   double maj_denominator;
   double min_denominator;
   double maj_temp;
   double min_temp;
   double temp;

   int subscript;
     
   double* r_major = scores;
   double* r_minor = scores + 12;

   for (i=0; i<12; i++) {
      maj_numerator = min_numerator = 0;
      maj_denominator = min_denominator = 0;
      maj_temp = min_temp = temp = 0;
   
      // Examine all pitches for each key,
      for (j=0; j<12; j++) {
         subscript = (i+j)%12;
         temp += pow(distribution[subscript] - mean, 2.0);
  
         // for the major keys:
         maj_numerator += (majorKey[j] - major_mean) *
                          (distribution[subscript] - mean);
         maj_temp += pow(majorKey[j] - major_mean, 2.0);
 
         // for the minor keys:
         min_numerator += (minorKey[j] - minor_mean) *
                          (distribution[subscript] - mean);
         min_temp += pow(minorKey[j] - minor_mean, 2.0);
      }
      
      maj_denominator = sqrt(maj_temp * temp);
      min_denominator = sqrt(min_temp * temp);

      if (maj_denominator <= 0.0) {
         r_major[i] = 0.0;
      } else {
         r_major[i] = maj_numerator / maj_denominator;
      }

      if (min_denominator <= 0.0) {
         r_minor[i] = 0.0;
      } else {
         r_minor[i] = min_numerator / min_denominator;
      }
   }

   // now figure out which key is the largest.
   int bestkey = 0;
   for (i=1; i<24; i++) {
      if (scores[i] > scores[bestkey]) {
         bestkey = i;
      }
   }

   return bestkey;
}



// md5sum: 09796ef2403c60dd1810761cafd4df11 humdrumfileextras.cpp [20050403]
