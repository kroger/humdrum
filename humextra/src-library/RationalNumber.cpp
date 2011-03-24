//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed May 19 21:10:47 PDT 2010
// Last Modified: Wed May 19 21:10:51 PDT 2010
// Filename:      ...sig/maint/code/base/RationalNumber/RationalNumber.cpp
// Web Address:   http://sig.sapp.org/src/sigBase/RationalNumber.cpp
// Syntax:        C++ 
//
// Description:   Rational number class (fraction with an integer
//                numerator and integer denominator).
//

#include "RationalNumber.h"

#include <stdlib.h>
#include <string.h>

//////////////////////////////
//
// RationalNumber::RationalNumber -- constructor
//     default values: _a = 0, _b = 1
//

RationalNumber::RationalNumber(int _a, int _b) {
   _num = _a;
   _den = _b;
   if (_den == 0) {
      _error_msg("denominator can't be zero.");
      assert(_den != 0);
   }
   simplify(*this);
}



//////////////////////////////
//
// RationalNumber::operator= --
//

RationalNumber RationalNumber::operator=(const RationalNumber &r) {
   this->_num = r._num;
   this->_den = r._den;
   return *this;
}



//////////////////////////////
//
// RationalNumber::operator+ --
//

RationalNumber RationalNumber::operator+(void) const {
   return *this;
}



//////////////////////////////
//
// RationalNumber::operator- --
//

RationalNumber RationalNumber::operator-(void) const {
   RationalNumber temp = *this;
   temp._num = -(this->_num);
   return temp;
}



//////////////////////////////
//
// RationalNumber::operator+ --
//

RationalNumber RationalNumber::operator+(const RationalNumber &r) const {
   if (r.getNumerator() == 0) {
      return *this;
   }
   if (this->getNumerator() == 0) {
      return r;
   }

   RationalNumber temp;
   int lcm_val = lcm( this->_den, r._den );
   int a = this->_num * ( lcm_val / this->_den );
   int c = r._num * ( lcm_val / r._den );
   // int b = lcm_val; 
   // int d = b;
   temp._num = a + c;
   temp._den = lcm_val;
   simplify(temp);
   return temp;
}



//////////////////////////////
//
// RationalNumber::operator- --
//

RationalNumber RationalNumber::operator-(const RationalNumber &r) const {
   RationalNumber tempval = -r;
   return (*this) + tempval;
}



//////////////////////////////
//
// RationalNumber::operator* --
//

RationalNumber RationalNumber::operator*(const RationalNumber &r) const {
   RationalNumber temp;

   if (r.getNumerator() == 0) {
      temp.setValue(0,1);
      return temp;
   }
   if (getNumerator() == 0) {
      temp.setValue(0,1);
      return temp;
   }


   int a = this->_num;
   int b = this->_den;
   int c = r._num;
   int d = r._den;
   int gcd_val = gcd( a, d );
   int gcd_val2 = gcd( b, c );
   a /= gcd_val;
   d /= gcd_val;
   b /= gcd_val2;
   c /= gcd_val2;
   temp._num = a * c;
   temp._den = b * d;
   return temp;
}



//////////////////////////////
//
// RationalNumber::operator/ --
//

RationalNumber RationalNumber::operator/(const RationalNumber &r) const {
   RationalNumber temp;
   /*int a = this->_num;
   int b = this->_den;
   int c = r._num;
   int d = r._den;
   int gcd_val = gcd( a, c );
   int gcd_val2 = gcd( b, d );
   a /= gcd_val;
   c /= gcd_val;
   b /= gcd_val2;
   d /= gcd_val2;
   temp._num = a * d;
   temp._den = b * c;
   simplify(temp);*/
   temp._num = r._den;
   temp._den = r._num;
   temp *= *this;
   return temp;
}



//////////////////////////////
//
// RationalNumber::operator+= --
//

RationalNumber RationalNumber::operator+=(const RationalNumber &r) {
   *this = *this + r;
   return *this;
}



//////////////////////////////
//
// RationalNumber::operator-= --
//

RationalNumber RationalNumber::operator-=(const RationalNumber &r) {
   *this = *this - r;
   return *this;
}



//////////////////////////////
//
// RationalNumber::operator*= --
//

RationalNumber RationalNumber::operator*=(const RationalNumber &r) {
   *this = *this * r;
   return *this;
}



//////////////////////////////
//
// RationalNumber::operator/= --
//

RationalNumber RationalNumber::operator/=(const RationalNumber &r) {
   *this = *this / r;
   return *this;
}



//////////////////////////////
//
// RationalNumber::operator++ --
//

RationalNumber RationalNumber::operator++(void) {
   this->_num += this->_den;
   return *this;
}



//////////////////////////////
//
// RationalNumber::operator-- --
//

RationalNumber RationalNumber::operator--(void) {
   this->_num -= this->_den;
   return *this;
}



//////////////////////////////
//
// RationalNumber::operator++ --
//

RationalNumber RationalNumber::operator++(int n) {
   if (n == 0) {
      this->_num += this->_den;
   }
   this->_num += n * this->_den;
   return *this;
}



//////////////////////////////
//
// RationalNumber::operator-- --
//

RationalNumber RationalNumber::operator--(int n) {
   if (n == 0) {
      this->_num -= this->_den;
   }
   this->_num -= n * this->_den;
   return *this;
}



//////////////////////////////
//
// RationalNumber::operator== --
//

int RationalNumber::operator ==(const RationalNumber &r) const {
   if ((this->getNumerator() == 0) && (r.getNumerator() == 0)) {
      return 1;
   } else {
      return ((this->getNumerator() == r.getNumerator()) && 
            (this->getDenominator() == r.getDenominator() ));
   }
}



//////////////////////////////
//
// RationalNumber::operator!= --
//

int RationalNumber::operator!=(const RationalNumber &r) const {
   return ((this->_num != r._num) || (this->_den != r._den));
}



//////////////////////////////
//
// RationalNumber::operator> --
//

int RationalNumber::operator>(const RationalNumber &r) const {
   return this->getFloat() > r.getFloat();
}



//////////////////////////////
//
// RationalNumber::operator< -- 
//

int RationalNumber::operator<(const RationalNumber &r) const {
   return this->getFloat() < r.getFloat();
}



//////////////////////////////
//
// RationalNumber::operator>= --
//

int RationalNumber::operator>=(const RationalNumber &r) const {
   return this->getFloat() >= r.getFloat();
}



//////////////////////////////
//
// RationalNumber::operator<= -- ???
//

int RationalNumber::operator<=(const RationalNumber &r) const {
   return this->getFloat() <= r.getFloat();
}



//////////////////////////////
//
// RationalNumber::setValue --
//

void RationalNumber::setValue(int num, int den) {
   _num = num;
   _den = den;
   simplify(*this);
}



//////////////////////////////
//
// RationalNumber::gcd -- Greatest common denominator. (static function)
//

int RationalNumber::gcd(int _x, int _y) {
   int x = _x;
   int y = _y;
   if (x < 0) x = -x;
   if (y < 0) y = -y;
   if (_x > _y) swap(x, y);
   int i;
   for (i=x; i>1; i--) {
      if (!(x%i) && !(y%i)) break;
   }
   if (i==0) {
      i++;
   }
   return i;
}



//////////////////////////////
//
// RationalNumber::lcm -- Least Common Multiple. (static function)
//

int RationalNumber::lcm(int _x, int _y) {
   int gcd_val = gcd(_x, _y);
   int prod = _x * _y;
   if (prod < 0) {
      prod = -prod;
   }
   return (prod / gcd_val);
}



//////////////////////////////
//
// RationalNumber::simplify -- Removed redundant factors between
//     numerator and denominator. (static function)
//

void RationalNumber::simplify(RationalNumber &r) {
   int gcd_val = gcd(r._num, r._den);
   if (r._den < 0) {
      r._num = -r._num;
      r._den = -r._den;
   }
   r._num /= gcd_val;
   r._den /= gcd_val;
}



//////////////////////////////
//
// RationalNumber::_abs --
//

RationalNumber RationalNumber::_abs(const RationalNumber &r) {
   if (r._num < 0 ) { 
      this->_num = -r._num;
   }
   if (r._den < 0 ) { 
      this->_den = -r._den;
   }
   return *this;
}



//////////////////////////////
//
// RationalNumber::_min --
//

RationalNumber RationalNumber::_min(const RationalNumber &p, 
      const RationalNumber &q ) {
   return (p < q ? p : q);
}



//////////////////////////////
//
// RationalNumber::_max --
//

RationalNumber RationalNumber::_max(const RationalNumber &p, 
      const RationalNumber &q ) {
   return (p > q ? p : q);
}



//////////////////////////////
//
// RationalNumber::convert_to_fraction --
//

RationalNumber RationalNumber::convert_to_fraction(char *strNum) {
   if (!isNumber(strNum)) {
      _error_msg("input string is not understandable fraction value.");
      assert(isNumber(strNum));
   }
   int counter = 0; 
   // int len = 0; 
   int pos = -1;
   int i = 0;
   int isdecimal = false;
   char *str = strNum;
   char *temp = new char[20];
   while (*str) {
      if (*str == '.') { 
         isdecimal = true; 
         pos = counter; 
      } else {
         *(temp + i++) = *str;
      }
      str++; 
      counter++;
   }
   temp[i] = 0;
   if (isdecimal) {
      this->_num = atol(temp);
      this->_den = (int)pow(10, counter - pos - 1 );
   } else {
      this->_num = atol(temp);
      this->_den = 1;
   }
   delete temp;
   return *this;
}



//////////////////////////////
//
// RationalNumber::getFloat -- 
//

double RationalNumber::getFloat(const RationalNumber &r ) {
   return ((double)r._num/r._den);
}



//////////////////////////////
//
// RationalNumber::getInversion --
//

RationalNumber RationalNumber::getInversion(void) const {
   RationalNumber anumber(getDenominator(), getNumerator());
   return anumber;
}


//////////////////////////////
//
// RationalNumber::invert -- create the reciprocal of the number.
//    But don't invert if the value is zero.
//

void RationalNumber::invert(void) {
   int top = getNumerator();
   int bot = getDenominator();

   if (top == 0) {
      return;
   }

   setValue(bot, top);
}



//////////////////////////////
//
// RationalNumber::isNegative -- returns true if less than zero, 
//       false otherwise.
//

int RationalNumber::isNegative(void) const {
   if (_num < 0) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// RationalNumber::isPositive -- returns true if greater than zero, 
//       false otherwise.
//

int RationalNumber::isPositive(void) const {
   if (_num > 0) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// RationalNumber::isZero -- returns true if value is zero.
//

int RationalNumber::isZero(void) const {
   if (_num == 0) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// RationalNumber::Log -- 
//

double RationalNumber::Log(const RationalNumber &r) {
   if ((r._num <= 0) || (r._den <= 0)) {
      _error_msg("numerator or denominator can't be negative values or zero.");
      assert((r._num > 0) && (r._den > 0));
   }
   return (log(r._num) - log(r._den));
}



//////////////////////////////
//
// RationalNumber::Log10 -- 
//

double RationalNumber::Log10(const RationalNumber &r) {
   if ((r._num <= 0) || (r._den <= 0)) {
      _error_msg("numerator and denominator can't be negative values or zero.");
      assert(r._num > 0 && r._den > 0);
   }
   return (log10(r._num) - log10(r._den));
}



//////////////////////////////
//
// RationalNumber::Exp -- 
//

double RationalNumber::Exp(const RationalNumber &r) {
   return exp(((double)r._num / r._den));
}



//////////////////////////////
//
// RationalNumber::Pow -- 
//

double RationalNumber::Pow(const RationalNumber &p, const RationalNumber &q) {
   return pow(((double)p._num/p._den), ((double)q._num/q._den));
}



//////////////////////////////
//
// operator<< --
//

ostream& operator<<(ostream& out, RationalNumber p) {
   out << p.getNumerator();
   if (p.getNumerator() != 0) {
      if (p.getDenominator() != 1) {
         out << "/" << p.getDenominator();
      }
   }
   return out;
}



//////////////////////////////
//
// RationalNumber::printTwoPart --
//     default value: spacer = "+"
//

ostream& RationalNumber::printTwoPart(ostream& out, const char* spacer) const {
   int tnum = _num;
   int tden = _den;
   int sign = 1;
   if (tnum < 0) {
      tnum = -tnum;
      sign = -sign;
   }
   if (tden < 0) {
      tden = -tden;
      sign = -sign;
   }

   if (tnum < tden) {
      out << *this;
      return out;
   }

   int integ = tnum / tden;
   tnum = tnum - tden * integ;

   if (sign < 0) {
      out << '-';
   }
   if (integ > 0) {
      out << integ;
      if (tnum > 0) {
         out << spacer;
         RationalNumber newone(tnum, tden);
         out << newone;
      }
   } else {
      RationalNumber newone(tnum, tden);
      out << newone;
   }

   return out;
}



//////////////////////////////
//
// operator>> --
//

istream& operator>>(istream& input, RationalNumber &p ) {
   char *buff = new char[20];
   char *tmp  = new char[20];
   input >> buff;
   char *str = buff;
   int is_fraction = false;
   int is_rational = true;
   int i = 0;
   // search "/" operator in "str" and retriving numerator
   while (*str) {
      if (*str == '/') { 
         is_fraction = true; 
         break; 
      }
      *(tmp + i) = *str;
      str++; 
      i++;
   }
   tmp[i] = 0;
   if (!RationalNumber::isNumber(tmp)) {
      is_rational = false;
   }
   RationalNumber::verify_number(tmp, is_fraction);
   RationalNumber P, Q(1,1); 
   i = 0; 
   P.convert_to_fraction(tmp);
   if (is_fraction) {
      str++;
      // retriving denominator
      while (is_fraction && *str) {
         *(tmp + i) = *str;
         str++; 
         i++;
      }
      tmp[i] = 0;
      RationalNumber::verify_number(tmp, is_fraction);
      Q.convert_to_fraction(tmp);
   }
   p = P/Q;
   delete tmp; delete buff;
   return input;
} 



//////////////////////////////
//
// RationalNumber::verify_number -- static function
//

void RationalNumber::verify_number(char *strNum, int is_fraction) { 
   int len = strlen(strNum); 
   if (is_fraction && (len == 0 || !isNumber(strNum))) { 
      _error_msg("this is not a rational number."); 
      assert(isNumber(strNum)); 
   } 
   if (len > 9) {
      _error_msg("this number is too big.");
      assert(len < 10);
   }
}



//////////////////////////////
//
// RationalNumber::isNumber -- static function
//

int RationalNumber::isNumber(char *number) {
   int len = strlen(number);
   int isnumber = true;
   int i = 0;
   
   if (len == 0) {
      return !isnumber;
   }

   while (i<len && isnumber) {
       if (isdigit(number[i]) == 0) {
           if (number[i] == '.') {
            if (i+1 > len-1) {
               isnumber = false;
               break;
            }
         }

         if (number[i] == '+' || number[i] == '-' ) {
            if ((i+1 > len-1) ||(i-1 >= 0)) {
               isnumber = false;
               break;
            }
         }

         if ((number[i] != '+') && (number[i] != '-') && (number[i] != '.')) {
            isnumber = false;
            break;
         }
      }
      i++;
   }
   return isnumber;
}



//////////////////////////////
//
// _error_msg --
//

void _error_msg(const char *str) {
   cerr << str << "\n";
}


/////////////////////////////////////////////////////////////////////////

/* TEST PROGRAM

#include "RationalNumber.h"
using std::cout;
using std::cin;
using std::endl;


int main() {

   RationalNumber value(1,2);
   RationalNumber summation;
   summation = value * value;
   summation *= 4;
   
   cout << summation << endl;

   RationalNumber R[3], R1;
   cout << "Please enter 3 rational numbers" << endl << endl;
   int i;

   for (i=0; i<3; i++) {
      cout << "number[" << i << "] = ";
      cin >> R[i];
   }
   cout << endl;
   for ( i = 0; i < 3; i++ ) {
      cout << "number[" << i << "] = ";
      cout << R[i] << endl;
   }
   cout << endl;
   cout << R[0] << " + " << R[1] << " = " << R[0] + R[1] << endl;
   cout << R[0] << " - " << R[1] << " = " << R[0] - R[1] << endl;
   cout << R[0] << " * " << R[1] << " = " << R[0] * R[1] << endl;
   cout << R[0] << " / " << R[1] << " = " << R[0] / R[1] << endl;

   if (R[0] > R[2]) {
      cout << R[0] << " > " << R[2] << endl;
   }

   if (R[0] < R[1]) {
      cout << R[0] << " < " << R[1] << endl;
   }

   R1 = R[0];
   cout << "++" << R1 << " = " << ++R[0] << endl;

   R1 = R[0];
   cout << "--" << R1 << " = " << --R[0] << endl;

   R1 = R[2];
   cout << "_abs("  << R1 << ") = " << R[2]._abs(R[2])  << endl;
   cout << "Pow("   << R[0] << "," << R[1] << ") = " 
        << R1.Pow( R[0], R[1] ) << endl;
   cout << "Log("   << R[1] << ") = " << R1.Log(R[1])   << endl;
   cout << "Log10(" << R[1] << ") = " << R1.Log10(R[1]) << endl;
   cout << "Exp("   << R[2] << ") = " << R1.Exp(R[2])   << endl;

   return 0;
}

*/



