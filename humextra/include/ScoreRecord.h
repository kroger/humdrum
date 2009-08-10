//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Feb 11 19:48:50 PST 2002
// Last Modified: Tue Feb 26 17:29:02 PST 2002
// Last Modified: Tue Mar 26 09:56:28 PST 2002 (added enum)
// Filename:      ...sig/src/sigInfo/ScoreRecord.h
// Web Address:   http://sig.sapp.org/include/sigInfo/ScoreRecord.h
// Syntax:        C++ 
//
// Description:   A SCORE item parameter class
//

#ifndef _SCORERECORD_H_INCLUDED
#define _SCORERECORD_H_INCLUDED

#include "ScoreParameters.h"

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

typedef enum {
	P1_Note 		= 1,
	P1_Rest			= 2,
	P1_Clef			= 3,
	P1_Line			= 4,
	P1_Slur			= 5,
	P1_Beam			= 6,
	P1_Trill		= 7,
	P1_Staff		= 8,
	P1_Symbol		= 9,
	P1_Number		= 10,
	P1_User			= 11,
	P1_Special		= 12,
	P1_BadLuck		= 13,
	P1_Barline		= 14,
	P1_ImportedEPSGraphic	= 15,
	P1_Text			= 16,
	P1_KeySignature		= 17,
	P1_MeterSignature	= 18
} PARM1VALUE;

typedef enum {
        P1 = 0, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14,
        P15, P16, P17, P18, P19, P20, P21, P22, P23, P24, P25, P26, P27,
        P28, P29, P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P40,
        P41, P42, P43, P44, P45, P46, P47, P48, P49, P50, P51, P52, P53,
        P54, P55, P56, P57, P58, P59, P60, P61, P62, P63, P64, P65, P66,
        P67, P68, P69, P70, P71, P72, P73, P74, P75, P76, P77, P78, P79,
        P80, P81, P82, P83, P84, P85, P86, P87, P88, P89, P90, P91, P92,
        P93, P94, P95, P96, P97, P98, P99
} PARAM;


class ScoreRecord {
   public:
                     ScoreRecord       (void);
                     ScoreRecord       (ScoreRecord& a);
                    ~ScoreRecord       ();

      void           printAscii        (ostream& out, int roundQ = 1, 
		                        int verboseQ = 0);
      void           printAscii        (char* buffer1, char* buffer2, 
                                        int max, int roundQ = 1, 
					int verboseQ = 0);
      void           clear             (void);
      int            writeBinary       (ostream& out);
      void           readBinary        (istream& instream, int pcount);
      void           shrink            (void);
      const char*    getTextData       (void);
      void           setTextData       (const char* buffer);
      const char*    getTextFont       (void);
      void           setTextFont       (const char* buffer);

      // booleans for identifying code items.
      int            isNoteItem        (void);
      int            isRestItem        (void);
      int               hasDurationQ   (void);
      int            isClefItem        (void);
      int            isLineItem        (void);
      int            isSlurItem        (void);
      int            isBeamItem        (void);
      int            isTrillItem       (void);
      int            isStaffItem       (void);
      int            isSymbolItem      (void);
      int            isNumberItem      (void);
      int            isUserItem        (void);
      int            isShapeItem       (void);
      int            isBarlineItem     (void);
      int            isPostscriptItem  (void);
      int            isTextItem        (void);
      int            isKeysigItem      (void);
      int            isTimesigItem     (void);

      // parameter item accessors:
      int            getFixedSize     
	      (void) { return pi.getFixedSize(); };
      float          getValue         
	      (int index) { return pi.getValue(index); };
      void           setValue         
              (int index, float value) { pi.setValue(index, value); };
      float          getPValue         
	      (int index) { return pi.getPValue(index); };
      void           setPValue         
              (int index, float value) { pi.setPValue(index, value); };
      void           setFixedSize    
              (int value) { pi.setFixedSize(value); };
      void           clearKeyParams     
	      (void) { pi.clearKeyParams(); };
      void           setAllocSize     
	      (int asize) { pi.setAllocSize(asize); };

   
      int            getPitch          (void);
      void           setPitch          (int aPitch);
      float          getStartOffset    (void);
      void           setStartOffset    (float aDuration);
      int            getVoice          (void);
      void           setVoice          (int aVoice);
      ScoreRecord&   operator=	       (ScoreRecord& a);

   protected:
      ScoreParameters  pi;               // for storing paramters
      static int       serialcounter;
      int              serialnumber;     // unique id for object
      Array<char>      textdata;         // for storing SCORE text data
      Array<char>      textfont;         // for storing optional font string
      Array<float>     analysisData;     // analysis data: duration and pitch
                                       // P1 = pitch (-1 for rest)
                                       // P2 = duration offset from the start 
                                       //      of the staff
                                       // P3 = voice number 
   private:
      void           writeLittleEndian (ostream& out, float number);
      float          readLittleEndian  (istream& instream);
      void           adjustBuffer      (char *buffer);
};


#endif /* _SCORERECORD_H_INCLUDED */


// md5sum: e640064c77a55b5c7ff784cebe395617 ScoreRecord.h [20050403]
