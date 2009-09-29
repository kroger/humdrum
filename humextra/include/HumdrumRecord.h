//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May 18 13:52:59 PDT 1998
// Last Modified: Fri May  5 13:13:32 PDT 2000 (added sub-spine access)
// Last Modified: Fri Oct 13 12:12:15 PDT 2000 (added spine path tracing)
// Last Modified: Fri Dec 10 00:03:59 PST 2004 (added isSpineManipulator)
// Last Modified: Tue Apr 28 14:34:13 PDT 2009 (added isTandem)
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Last Modified: Sat Aug  8 23:50:10 PDT 2009 (added isExInterp)
// Filename:      ...sig/include/sigInfo/HumdrumRecord.h
// Webpage:       http://sig.sapp.org/include/sigInfo/HumdrumRecord.h
// Syntax:        C++ 
//
// Description:   Stores the data for one line in a HumdrumFile and
//                segments data into spines.
//

#ifndef _HUMDRUMRECORD_H_INCLUDED
#define _HUMDRUMRECORD_H_INCLUDED

#include "SigCollection.h"
#include "Array.h"
#include "EnumerationEI.h"
#include "Enum_humdrumRecord.h"

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif

class HumdrumRecord {
   public:
                        HumdrumRecord      (void);
                        HumdrumRecord      (const char* aLine, 
                                              int aLineNum = -1);
                        HumdrumRecord      (const HumdrumRecord& aRecord);
                       ~HumdrumRecord      ();
  
      void              appendField        (const char* aField, 
                                              int anInterp = E_unknown,
                                              const char* spinetrace = "");
      void              appendFieldEI      (const char* aField, 
                                             const char* anInterp = "**unknown",
                                             const char* spinetrace = "");
      void              changeField        (int aField, const char* aString);
      void              copySpineInfo      (char** info, int size,
                                              int line = 0);
      void              copySpineInfo      (SigCollection<char*>& aCollection,
                                              int line = 0);
      void              copySpineInfo      (HumdrumRecord& aRecord,
                                              int line = 0);

      int               equalDataQ         (const char* aValue);
      int               equalFieldsQ       (void);
      int               equalFieldsQ       (int anInterp);
      int               equalFieldsQ       (const char* anInterp);
      int               equalFieldsQ       (int anInterp, const char*
                                              compareString);
      int               equalFieldsQ       (const char* anInterp,
                                              const char* compareString);
      double            getAbsBeat         (void) const;
      double            getBeat            (void) const;
      int               getDotLine         (int index);
      int               getDotSpine        (int index);
      double            getDuration        (void) const;
      int               getExInterpNum     (int fieldIndex) const;
      const char*       getExInterp        (int fieldIndex) const;
      int               getFieldCount      (void) const;
      int               getFieldCount      (const char* exinterp) const;
      int               getFieldCount      (int exinterp) const;
      char*             getBibKey          (char* buffer, int maxsize = 0);
      char*             getBibValue        (char* buffer, int maxsize = 0);
      const char*       getBibLangIso639_2 (const char* string = NULL);
      static const char*getLanguageName    (const char* code);
      static const char*getBibliographicMeaning(Array<char>& output, 
                                            const char* code);
      const char*       getLine            (void); 
      int               getLineNum         (void) const; 
      int               getPrimaryTrack    (int spineNumber);
      double            getTrack           (int spineNumber);
      const char*       getSpineInfo       (int index) const;
      int               getSpinePrediction (void);
      int               getSpineWidth      (void);
      void              changeToken        (int spineIndex, int tokenIndex,  
                                            const char* newtoken,
                                            char separator = ' ');
      int               getTokenCount      (int fieldIndex, 
                                              char separator = ' ');
      char*             getToken           (char* buffer, int fieldIndex, 
                                              int tokenIndex, 
                                              int buffersize = -1, 
                                              char separator = ' ');
      int               getType            (void) const;

      int               isData             (void) const;
      int               isEmpty            (void) const;
      int               isInterpretation   (void) const;
      int               isGlobalComment    (void) const;
      int               isBibliographic    (void) const;
      int               isMeasure          (void) const;
      int               isLocalComment     (void) const;
      int               isComment          (void) const;
      int               isTandem           (void) const;
      int               isSpineManipulator (int index);
      int               isSpineManipulator (void);
      int               isExInterp         (int index, const char* string);
      int               hasSpines          (void) const;

      int               dataQ              (void) const;
      int               measureQ           (void) const;
      int               localCommentQ      (void) const;
      int               globalCommentQ     (void) const;
      int               bibRecordQ         (void) const;
      int               interpretationQ    (void) const;
      int               nullQ              (void) const;
      int               hasExclusiveQ      (void) const;
      int               hasPathQ           (void) const;

      void              insertField        (int index, const char* aField,
                                              int anInterp = E_unknown,
                                              const char* spinetrace = "");
      void              insertField        (int index, const char* aField,
                                              const char* anInterp,
                                              const char* spinetrace = "");
      HumdrumRecord&    operator=          (const HumdrumRecord& aRecord);
      HumdrumRecord&    operator=          (const HumdrumRecord* aRecord);
      HumdrumRecord&    operator=          (const char* aRecord);
      const char*       operator[]         (int index) const;
      void              setAbsBeat         (double aValue);
      void              setBeat            (double aValue);
      void              setDotLine         (int index, int value);
      void              setDotSpine        (int index, int value);
      void              setDuration        (double aValue);
      void              setExInterp        (int fieldIndex, int interpretation);
      void              setExInterp        (int fieldIndex, 
                                              const char* interpretation);
      void              setLine            (const char* aString); 
      void              setLineNum         (int aLine);
      void              setSpineID         (int index, const char* anID);
      void              setSpineWidth      (int aSize);

   protected:
      int                  lineno;         // line number of record in a file
      int                  type;           // category of humdrum record
      int                  spinewidth;     // for size of spines in comments
      char*                recordString;   // record string
      int                  modifiedQ;      // boolen for if need to make Rstring
      SigCollection<char*> recordFields;   // data for humdrum text record
      SigCollection<char*> spineids;       // spine tracing ids
      SigCollection<int>   interpretation; // exclusive interpretation of data

      SigCollection<int>   dotline;        // for resolving meaning of "."'s
      SigCollection<int>   dotspine;       // for resolving meaning of "."'s


      // data storage for rhythmic analysis in relation to entire Humdrum File.
      float             duration;       // duration of the record 
      float             meterloc;       // metric position of the record
      float             absloc;         // absolute beat location of the record
      
      // private functions
      int               determineFieldCount(const char* aLine) const;
      int               determineType      (const char* aLine) const;
      void              makeRecordString   (void);
      void              storeRecordFields  (void);


};
   

ostream& operator<<(ostream& out, HumdrumRecord& aRecord);


#endif  /* _HUMDRUMRECORD_H_INCLUDED */



// md5sum: a51a3ff3765fbd6308f3334a7ceb8b6e HumdrumRecord.h [20001204]
