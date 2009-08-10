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

class ScoreParameters {
   public:
                          ScoreParameters    (void);
                          ScoreParameters    (ScoreParameters& item);

      void                clear            (void);
      float               getValue         (int index);
      float               getPValue        (int index);
      void                setValue         (int index, float value);
      void                setPValue        (int index, float value);
      void                setAllocSize     (int aSize);
      int                 getFixedSize     (void);
      void                setFixedSize     (int value);
      int                 getKeySize       (void);
      ScoreParameters&    operator=        (ScoreParameters& anItem);
      void                print            (void);
      void                clearKeyParams     (void);

   protected:
      void                appendKeyParameter (const char* string);

   private:
      Array<float>        fixedParameters;
      Array<Array<char> > keyParameters;

};


#endif /* _SCOREPARAMETERS_H_INCLUDED */
// md5sum: 23bb8fd811be6b25c9dff0a99b58faae ScoreParameters.h [20050403]
