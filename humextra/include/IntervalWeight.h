//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jul 24 12:35:50 PDT 2002
// Last Modified: Wed Jul 24 12:35:55 PDT 2002
// Filename:      ...sig/include/sigInfo/IntervalWeight.h
// Web Address:   http://sig.sapp.org/include/sigInfo/IntervalWeight.h
// Syntax:        C++ 
//
// Description:   Creates interval weights for chord root calculations.
//


#ifndef _INTERVALWEIGHT_H_INCLUDED
#define _INTERVALWEIGHT_H_INCLUDED

#include "Array.h"

#define IWTYPE_NONE          0 /* unknown type */
#define IWTYPE_DIATONIC      1
#define IWTYPE_CHROMATIC1    2
#define IWTYPE_CHROMATIC2    3
#define IWTYPE_CHROMATIC2B   4
#define IWTYPE_CIRCULAR      5
#define IWTYPE_SPIRAL        6

class IntervalWeight {
   public: 
                  IntervalWeight   (void);
                 ~IntervalWeight   ();

      int         getType          (void);
      double&     getWeight        (int diatonic, int chromatic = 0);
      int         getSize          (void);
      void        setAll           (double value);
      double&     operator[]       (int index);

      void        setFromFile      (const char* filename);
      void        readWeights      (const char* filename);
      void        setDiatonic      (void);
      static void setDiatonic      (Array<double>& anArray);
      void        setChromatic1    (double theta);
      void        setChromatic     (double theta) {setChromatic1(theta); }
      static void setChromatic1    (double theta, Array<double>& anArray);
      void        setChromatic2    (double theta1, double theta2);
      void        setChromatic     (double theta1, double theta2) {
                                    setChromatic2(theta1, theta2); }
      static void setChromatic2    (double theta1, double theta2,
                                    Array<double>& anArray);
      void        setChromatic2b   (double theta1, double theta2);
      static void setChromatic2b   (double theta1, double theta2,
                                    Array<double>& anArray);
      void        setCircular      (double theta, double ascale);
      static void setCircular      (double theta, double ascale,
                                    Array<double>& anArray);
      void        setSpiral        (double radius, double height);
      static void setSpiral        (double radius, double height, 
                                    Array<double>& anArray);

      ostream&    newprint         (ostream& out);

 
   private:
      Array <double>   weights;   
      int              wtype;       // type of weighting being used

};

ostream&    operator<<(ostream& out, IntervalWeight& weights);

#endif /* _INTERVALWEIGHT_H_INCLUDED */



// md5sum: 93db400756cabb6dfd26caad4d76cb47 IntervalWeight.h [20050403]
