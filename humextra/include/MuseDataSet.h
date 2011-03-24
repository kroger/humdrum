//
// Copyright 2010 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun 17 13:17:50 PDT 2010
// Last Modified: Thu Jun 17 13:17:57 PDT 2010
// Filename:      ...sig/include/SigInfo/MuseDataSetSet.h
// Web Address:   http://sig.sapp.org/include/sigInfo/MuseDataSetSet.h
// Syntax:        C++ 
//
// Description:   A class that stores a collection of MuseDataSet files
//                representing parts for the same score.
//

#ifndef _MUSEDATASET_H_INCLUDED
#define _MUSEDATASET_H_INCLUDED

#include "MuseData.h"
#include "Array.h"

class MuseDataSet {
   public:
                        MuseDataSet         (void);
                        MuseDataSet         (MuseDataSet& input);
                       ~MuseDataSet         () { clear(); }

      void              clear               (void);
      int               readPart            (const char* filename);
      int               readPart            (istream& input);
      void              read                (const char* filename);
      void              read                (istream& input);
      MuseData&         operator[]          (int index);
      int               getPartCount        (void);
      void              deletePart          (int index);
      void              cleanLineEndings    (void);

   private:
      Array<MuseData*>  part;

   protected:
      int               appendPart          (MuseData* musedata);
      void              analyzeSetType      (Array<int>& types, 
                                             Array<string>& lines);
      void              analyzePartSegments (Array<int>& startindex, 
                                             Array<int>& stopindex, 
                                             Array<string> lines);

};
   

ostream& operator<<(ostream& out, MuseDataSet& musedata);


#endif  /* _MUSEDATASET_H_INCLUDED */


// md5sum: 051e558c8e271327a795fcad656d760b MuseDataSet.h [20050403]
