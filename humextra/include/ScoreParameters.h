//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb 10 19:42:45 PST 2002
// Last Modified: Sun Feb 10 19:42:48 PST 2002
// Filename:      ...sig/src/sigInfo/ScoreParameters.h
// Web Address:   http://sig.sapp.org/include/sigInfo/ScoreParameters.h
// Syntax:        C++ 
//
// Description:   Base class for SCORE musical objects.
//

#ifndef _SCOREPARAMETERS_H_INCLUDED
#define _SCOREPARAMETERS_H_INCLUDED

#include "Array.h"

#include <iostream>

class ScoreNamedParameter {
   public:
                    ScoreNamedParameter   (void);
                    ScoreNamedParameter   (ScoreNamedParameter& aKeyParam);
                   ~ScoreNamedParameter   ();
      void         clear                  (void);
      const char*  getName                (void);
      double       getValue               (void);
      void         setNameAndValue        (const char* aName, double aValue);
      void         setName                (const char* aName);
      void         setValue               (double aValue);
      int          isValid                (void);
      int          isInvalid              (void);
      int          isNamed                (const char* aName);
      ScoreNamedParameter& operator=      (ScoreNamedParameter& aKeyParam);

   private:
      Array<char> name;    // name of the key parameter
      double      value;   // value of the key parameter
};



class ScoreParameters {
   public:
                          ScoreParameters     (void);
                          ScoreParameters     (ScoreParameters& item);
                         ~ScoreParameters     ();

      void                clear               (void);

      const char*         getText             (void);
      const char*         getTextData         (void);
      void                setText             (const char* aString);
      void                setTextData         (const char* aString);

      double              getValue            (int index);
      double              getValue            (const char* keyName);
      int                 getValue            (int index, int digit);
      double              getPValue           (int number);
      double              getPValue           (const char* keyName);
      int                 getPValue           (int number, int digit);

      const char*         getKeyName          (int index);
      double              getKeyValue         (int index);

      void                setValue            (int index, double value);
      void                setValue            (const char* keyName, 
                                                 double keyValue);
      void                setPValue           (int number, double value);
      void                setPValue           (const char* keyName, 
                                                 double keyValue);
     
      ScoreParameters&    operator=           (ScoreParameters& anItem);
      void                setAllocSize        (int aSize);
      int                 getFixedSize        (void);
      void                setFixedSize        (int aSize);
      int                 getKeySize          (void);
      void                clearKeyParameters  (void);
      int                 hasExtraParameters  (void);
      int                 hasKeyParameters    (void);
      int                 hasKey              (const char* aName);
      int                 hasValue            (const char* aName);
      void                shrink              (void);

      ostream&            print               (ostream& out);
      ostream&            printFixedParameters(ostream& out);
      ostream&            printKeyParameters  (ostream& out);

      static double       roundFractionDigits(double number, int digits);

   protected:
      Array<double>        fixedParameters;     // SCORE fixed parameter list
      Array<char>          text;                // Used for text when P1=16
      int                  textFont;            // for default font of text

      Array<ScoreNamedParameter> keyParameters; // Extended SCORE key parameters

};


#endif /* _SCOREPARAMETERS_H_INCLUDED */
// md5sum: 23bb8fd811be6b25c9dff0a99b58faae ScoreParameters.h [20050403]
