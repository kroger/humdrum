//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed May 19 21:10:47 PDT 2010
// Last Modified: Thu Jan 27 03:49:20 PST 2011 added invert()
// Filename:      ...sig/maint/code/base/RationalNumber/RationalNumber.h
// Web Address:   http://sig.sapp.org/src/sigBase/RationalNumber.h
// Syntax:        C++ 
//
// Description:   Rational number class (fraction with an integer
//                numerator and integer denominator).
//

#ifndef _RATIONALNUMBER_H_INCLUDED
#define _RATIONALNUMBER_H_INCLUDED

#include <cmath>
#include <cassert>
#include <iostream>

using std::ostream;
using std::istream;
using std::swap;
using std::cerr;

void    _error_msg     (const char *str);

class RationalNumber {
   public:
                     RationalNumber   (int _a = 0, int _b = 1);
                    ~RationalNumber   () {};

      RationalNumber operator=   (const RationalNumber &r);
      RationalNumber operator+   (const RationalNumber &r) const;
      RationalNumber operator-   (const RationalNumber &r) const;
      RationalNumber operator*   (const RationalNumber &r) const;
      RationalNumber operator/   (const RationalNumber &r) const;
      RationalNumber operator+=  (const RationalNumber &r);
      RationalNumber operator-=  (const RationalNumber &r);
      RationalNumber operator*=  (const RationalNumber &r);
      RationalNumber operator/=  (const RationalNumber &r);

      int            operator==  (const RationalNumber &r) const;
      int            isEqualTo   (int top, int bot) const;
      int            operator!=  (const RationalNumber &r) const;
      int            operator>   (const RationalNumber &r) const;
      int            operator<   (const RationalNumber &r) const;
      int            operator>=  (const RationalNumber &r) const;
      int            operator<=  (const RationalNumber &r) const;

      RationalNumber  operator++ (void);
      RationalNumber  operator-- (void);
      RationalNumber  operator+  (void) const;
      RationalNumber  operator-  (void) const;
      RationalNumber  operator++ (int n);
      RationalNumber  operator-- (int n);
   
      void            setValue      (int num, int den);
      int             getNumerator  (void) const { return _num; }
      int             getDenominator(void) const { return _den; }
      void            setNumerator  (int val) { _num = val; }
      void            setDenominator(int val) { _den = val; }
      ostream&        printTwoPart  (ostream& out, const char* spacer = "+") 
                                    const;
      void            zero          (void) { _num = 0; _den = 1; }
      double          getFloat      (void) const { return getFloat(*this); }
      RationalNumber  getInversion  (void) const;
      void            invert        (void);
      int             isNegative    (void) const;
      int             isPositive    (void) const;
      int             isZero        (void) const;

      RationalNumber  _abs       (const RationalNumber &r);
      RationalNumber  _min       (const RationalNumber &p, 
                                  const RationalNumber &q);
      RationalNumber  _max       (const RationalNumber &p, 
                                  const RationalNumber &q);
      RationalNumber  convert_to_fraction (char *strNum);
      double          Pow         (const RationalNumber &p, 
                                   const RationalNumber &q);
      double          Log         (const RationalNumber &r);
      double          Log10       (const RationalNumber &r);
      double          Exp         (const RationalNumber &r);
      static int      lcm         (int _x, int _y);
      static int      gcd         (int _x, int _y);
      static void     simplify    (RationalNumber &r);


      static double   getFloat    (const RationalNumber &r);
      static void     verify_number  (char *strNum, int is_fraction);
      static int      isNumber       (char *number);
   private:
      int     _num;
      int     _den;
};

ostream& operator<<(ostream& out, RationalNumber p);
istream& operator>>(istream& input, RationalNumber& p);


#endif  /* _RATIONALNUMBER_H_INCLUDED */


// md5sum: fff68b3c92d5d5d55f2491aa14e0b15c Action.h [20050403]
