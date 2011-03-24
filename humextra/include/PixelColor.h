//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Apr 11 12:33:09 PDT 2002
// Last Modified: Fri Apr 12 18:13:46 PDT 2002
// Last Modified: Sat May 12 17:18:03 PDT 2007 added invert
// Last Modified: Sun Feb 20 18:05:23 PST 2011 added ostream
// Filename:      ...sig/maint/code/base/PixelColor/PixelColor.h
// Web Address:   http://sig.sapp.org/include/sigBase/PixelColor.h
// Documentation: http://sig.sapp.org/doc/classes/PixelColor
// Syntax:        C++ 
//

#ifndef _PIXELCOLOR_H_INCLUDED
#define _PIXELCOLOR_H_INCLUDED

#ifndef OLDCPP
   #include <ostream>
   using namespace std;
#else
   #include <ostream.h>
#endif

typedef unsigned char uchar;
typedef unsigned int  uint;

class PixelColor {
   public:
                   PixelColor     (void);
                   PixelColor     (const char* color);
                   PixelColor     (const PixelColor& color);
                   PixelColor     (int red, int green, int blue);
                   PixelColor     (float red, float green, float blue);
                   PixelColor     (double red, double green, double blue);
                  ~PixelColor     ();

      void         invert         (void);
      PixelColor&  setColor       (const char* colorstring);
      PixelColor&  setColor       (int red, int green, int blue);
      int          getRed         (void);
      int          getGreen       (void);
      int          getBlue        (void);
      void         setRed         (int value);
      void         setGreen       (int value);
      void         setBlue        (int value);
      float        getRedF        (void);
      float        getGreenF      (void);
      float        getBlueF       (void);
      void         setRedF        (float value);
      void         setGreenF      (float value);
      void         setBlueF       (float value);
      void         setColor       (PixelColor color);
      PixelColor&  setHue         (float value);
      PixelColor&  setTriHue      (float value);
      PixelColor&  makeGrey       (void);
      PixelColor&  makeGray       (void);
      PixelColor&  setGrayNormalized(double value);
      PixelColor&  setGreyNormalized(double value);
      int          operator>      (int number);
      int          operator<      (int number);
      int          operator==     (PixelColor color);
      int          operator!=     (PixelColor color);
      PixelColor&  operator=      (PixelColor color);
      PixelColor&  operator=      (int value);
      PixelColor   operator+      (PixelColor color);
      PixelColor&  operator+=     (int number);
      PixelColor   operator-      (PixelColor color);
      PixelColor&  operator*=     (double number);
      PixelColor   operator*      (PixelColor color);
      PixelColor   operator*      (double color);
      PixelColor   operator*      (int color);
      PixelColor   operator/      (double number);
      PixelColor   operator/      (int number);

      static PixelColor getColor (const char* colorstring);

      void         writePpm6      (ostream& out);
      void         writePpm3      (ostream& out);

   public:
      uchar   Red; 
      uchar   Green;
      uchar   Blue;

   private:
      float   charToFloat         (int value);
      int     floatToChar         (float value);
      int     limit               (int value, int min, int max);
};



// for use with P3 ASCII pnm images: print red green blue triplet.
ostream& operator<<(ostream& out, PixelColor apixel);

#endif  /* _PIXELCOLOR_H_INCLUDED */



// md5sum: ae66e4cb52484f07c3515181faa3d14e PixelColor.h [20010708]
