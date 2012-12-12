//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Feb 11 19:48:50 PST 2002
// Last Modified: Tue Feb 26 17:29:02 PST 2002
// Last Modified: Tue Mar 26 09:56:28 PST 2002 Added parameter enums
// Last Modified: Fri Aug 24 09:52:39 PDT 2012 Renovated
// Filename:      ...sig/src/sigInfo/ScoreRecord.h
// Web Address:   http://sig.sapp.org/include/sigInfo/ScoreRecord.h
// Syntax:        C++ 
//
// Description:   A SCORE item parameter class.  Inherits basic parameter
//                management from ScoreParameters class.  This class adds
//                higher-level functionality to the parameters.
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


class ScoreRecord : public ScoreParameters {
   public:
                     ScoreRecord           (void);
                     ScoreRecord           (ScoreRecord& a);
                    ~ScoreRecord           ();

      // General maintenance functions:
      void           clear                 (void);
      void           clearAnalysisVariables(void);
      ScoreRecord&   operator=	           (ScoreRecord& a);

      // Printing functions:
      int            writeBinary       (ostream& out);
      void           readBinary        (istream& instream, int pcount);
      void           printAscii        (ostream& out, int roundQ = 1, 
		                        int verboseQ = 0, 
                                        const char* indent = "");
      void           printAscii        (char* buffer1, char* buffer2, 
                                        int max, int roundQ = 1, 
					int verboseQ = 0, 
                                        const char* indent = "");
      void           printAsciiExtraParameters(ostream& out, int roundQ, 
                                        int verboseQ,
                                        const char* indent = "");
      void           setTextFont       (const char* aString);

      // booleans for identifying code items.
      int            isNoteItem            (void);
      int            isRestItem            (void);
      int            hasDurationQ          (void);
      int            hasDuration           (void);
      int            isClefItem            (void);
      int            isLineItem            (void);
      int            isSlurItem            (void);
      int            isBeamItem            (void);
      int            isTrillItem           (void);
      int            isStaffItem           (void);
      int            isSymbolItem          (void);
      int            isNumberItem          (void);
      int            isUserItem            (void);
      int            isShapeItem           (void);
      int            isBarlineItem         (void);
      int            isPostscriptItem      (void);
      int            isTextItem            (void);
      int            isKeysigItem          (void);
      int            isTimesigItem         (void);
      int            isMeterItem           (void);

      // General fixed parameter functions:
      int            getStaff              (void);
      int            getStaffNumber        (void) { return getStaff(); }
      int            getPageStaff          (void) { return getStaff(); }
      int            getPageStaffNumber    (void) { return getStaff(); }
      double         getHpos               (void);
      double         getVpos               (void);

      // Note object (P1=1) functions:
      int            getStaffOffset        (void);
      int            getStemDirection      (void);
      int            getPrintedAccidental  (void);
      int            hasPrintedAccidental  (void);
      int            hasPrintedFlat        (void);
      int            hasPrintedSharp       (void);
      int            hasPrintedNatural     (void);
      int            hasPrintedDoubleFlat  (void);
      int            hasPrintedDoubleSharp (void);
      int            hasCautionary         (void);
      double         getAccidental         (void);
      int            hasNatural            (void);
      int            hasEditorialAccidental(void);

      // Functions shared by note and rest items.
      double         getDuration           (void);

      // Line object (P1=4) functions:
      int            isHairpin             (void);
      int            isHairpinDecrescendo  (void);
      int            isHairpinCrescendo    (void);
      int            isWavyLine            (void);
      int            isDashedLine          (void);
      int            isPlainLine           (void);

      // Slur/Tie/Ending object (P1=5) functions:

      // Beam object (P1=6) functions:

      // Staff object (P1=8) functions:
      int            isInvisibleStaff      (void);

      // Trill object (P1=9) functions:

      // Graphic object (P1=10) functions:

      // User object (P1=11) functions:

      // Special object (P1=12) functions:

      // Bad Luck object (P1=13) functions:

      // Barline object (P1=14) functions:
      int            getBarHeight          (void);
      int            getBarlineHeight      (void) { return getBarHeight(); }

      // Imported EPS Graphic  object (P1=15) functions:

      // Text functions (P1=16):
      Array<char>&   getTextDataWithoutFonts(Array<char>& textdata);

      // Lyric functions (P1=16 & p1=4):
      int            getVerseIndex         (void);
      int            getVerseNumber        (void);
      void           setVerseIndex         (double anIndex);
      void           setVerseNumber        (double aNumber);
      int            isHyphen              (void);
      int            isVerse               (void);
      int            isWordEnd             (void);
      int            isWordStart           (void);
      int            isWordMiddle          (void);

      // Key Signature object (P1=17) functions:

      // Time Signature object (P1=18) functions:

      /////////////////////////////////////////////////////////////////////
      //
      // Line/Page/PageSet location functions
      //

      //
      // PageSet-level indexing/location functions.  These are usually set
      // automatically in the ScorePageSet class.
      //

      int            getPageIndex            (void);
      void           setPageIndex            (int pageidx);

      int            getPageSetSystemPosition(void);
      int            getPageSetSystemPos     (void);
      int            getPageSetStaffPosition (void);
      int            getPageSetStaffPos      (void);
      int            getPageSetPartPosition  (void);
      int            getPageSetPartPos       (void);

      void           setPageSetSystemPosition(int objidx);
      void           setPageSetSystemPos     (int objidx);
      void           setPageSetStaffPosition (int objidx);
      void           setPageSetStaffPos      (int objidx);
      void           setPageSetPartPosition  (int objidx);
      void           setPageSetPartPos       (int objidx);

      //
      // Page-level indexing/location functions.  These are usually set
      // automatically in the ScorePage class and stitch together systems
      // and staves across multiple line breaks on a page.
      //

      int            getPageSystemPosition   (void);
      int            getPageSystemPos        (void);
      int            getPageStaffPosition    (void);
      int            getPageStaffPos         (void);
      int            getPagePartPosition     (void);
      int            getPagePartPos          (void);

      void           setPageSystemPosition   (int objidx);
      void           setPageSystemPos        (int objidx);
      void           setPageStaffPosition    (int objidx);
      void           setPageStaffPos         (int objidx);
      void           setPagePartPosition     (int objidx);
      void           setPagePartPos          (int objidx);

      int            getSystemStaffIndex     (void);
      void           setSystemStaffIndex     (int anIndex);

      int            getPageSystemIndex      (void);
      void           setPageSystemIndex      (int anIndex);

      // 
      // Line-level indexing/location functions.  These are usually set 
      // automatically in the ScorePage class.
      // 

      int             getLineSystemPosition  (void);
      int             getLineSystemPos       (void);
      void            setLineSystemPosition  (int objidx);
      void            setLineSystemPos       (int objidx);

      int             getLineStaffPosition  (void);
      int             getLineStaffPos       (void);
      void            setLineStaffPosition  (int objidx);
      void            setLineStaffPos       (int objidx);


      /////////////////////////////////////////////////////////////////////
      //
      // Analysis-related functions.  This functions typically access
      //      key parameter values which were set after analyzeContent()
      //      was run in the ScorePage or ScorePageSet classes.
      //

      // Pitch analysis functions:
      void           setPitch                (int aPitch);
      int            getPitch                (void);

      // Rhythm analysis functions:
      void           setSystemBeat           (double beat);
      double         getAbsBeat              (void);
      double         getSystemBeat           (void);
      void           setAbsBeat              (double beat);

      // Tie analysis functions (P1=5):
      void           setTieStart             (int objidx);
      void           setTieEnd               (int objidx);
      void           setTie                  (int leftnote, int rightnote);
      int            hasTieStart             (void);
      int            hasTieEnd               (void);
      int            hasTieContinue          (void);

      ///////////////////////////////
      //
      // static functions
      //

      static int     getDiatonicAccidentalState(Array<int>& states, 
                                        ScoreRecord& arecord);
      static int     getMiddleCVpos    (ScoreRecord& arecord);
      static Array<char>& convertScoreTextToHtmlText(Array<char>& astring);
      static Array<char>& convertScoreTextToPlainText(Array<char>& astring);


   protected:
      void           writeLittleEndian (ostream& out, double number);
      double         readLittleEndian  (istream& instream);
      void           adjustBuffer      (char *buffer);

      // The textFont is used to set the font of an item independent of
      // the font inside of the text string for P1=16 objects.
      // When a text record is printed, and this entry is not NULL or 
      // empty, then it will be printed first on the text line, followed
      // by the text string (which will have any prefixed font removed).
      char*          textFont;

   private:
      int pageIndex;       // Current page to which the item belongs.
      int systemLineIndex; // Broken System to which the item belongs.
      int systemStaffIndex;// Continuous/Broken system to which item belongs.


      // POS variabes: indicies of the objects in various configurations.
      int pagePrintPos;    // printing sequence position on page (order of
                           //    how it was read from input file).


      // single-line position (on staff/system)
      int lineStaffPos;    // Position of item on single staff line on page, 
                           //    such as "P2=4" (the 4th staff on the page).
      int lineSystemPos;   // Position of item on single system line on page,
                           //    such as the 0th system which is the top
                           //    system on the page, the 1st system is the next
                           //    and so on.


      // position of object on the page (continuous staff/part and system)
      int pageStaffPos;    // Position of item in continuous part staff on page.
                           //    A time/space enumeration of all items for a 
                           //    part on a given page.
      int pageSystemPos;   // Postion of item in continuous system on page


      // posistion of object on page set (work/movement)
      int pagesetStaffPos; // Position of item in staff part across multiple 
                           //    pages.
      int pagesetSystemPos;// Position of item in system across multiple pages.


      // duration analysis variables:
      double lineBeat;     // Duration from start of page to current note
      double pageBeat;     // Duration from start of page to current note
      double pagesetBeat;  // Duration from start of first page to current note
};


#endif /* _SCORERECORD_H_INCLUDED */


// md5sum: e640064c77a55b5c7ff784cebe395617 ScoreRecord.h [20050403]
