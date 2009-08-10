//
// Copyright 2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Nov 25 14:18:01 PST 2000
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Filename:      ...sig/include/sigInfo/HumdrumInstrument.h
// Web Address:   http://museinfo.sapp.org/include/sigInfo/HumdrumInstrument.h
// Syntax:        C++ 
//
// Description:   Special enumeration class for processing Humdrum 
//                instrument names.
//

#ifndef _HUMDRUMINSTRUMENT_H_INCLUDED
#define _HUMDRUMINSTRUMENT_H_INCLUDED


#include "SigCollection.h"


class _HumdrumInstrument {
   public:
      _HumdrumInstrument(void) { humdrum = ""; name = ""; gm = 0; }
     ~_HumdrumInstrument() { humdrum = ""; name = ""; gm = 0; }

      const char* humdrum;
      const char* name;
      int   gm;
};

class HumdrumInstrument {
   public:
                      HumdrumInstrument  (void);
                      HumdrumInstrument  (const char* Hname);
                     ~HumdrumInstrument  ();

      const char*     getName 		 (void);
      const char*     getName 	  	 (const char* Hname);
      const char*     getHumdrum 	 (void);
      int             getGM	 	 (void);
      int             getGM	 	 (const char* Hname);
      void            setHumdrum 	 (const char* Hname);
      int             setGM	 	 (const char* Hname, int aValue);

   private:
      int                                      index;
      static SigCollection<_HumdrumInstrument> data;
      static int                               classcount;

   protected:
      void   initialize   (void);
      void   afi          (const char* humdrum_name, int midinum,
                             const char* EN_name);
      int    find         (const char* Hname);
      void   sortData     (void);
      static int data_compare_by_humdrum_name(const void* a, const void* b);
};


#endif /* _HUMDRUMINSTRUMENT_H_INCLUDED */



// md5sum: 4ac776c00b38e596a922618e626c890d HumdrumInstrument.h [20001204]
