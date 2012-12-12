//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 22:36:03 PDT 1998
// Last Modified: Tue Jun 30 22:36:07 PDT 1998
// Filename:      ...sig/include/SigInfo/MuseRecord.h
// Web Address:   http://sig.sapp.org/include/sigInfo/MuseRecord.h
// Syntax:        C++ 
//
// Description:   A class that stores one line of data for a Musedata file.
//

#ifndef _MUSERECORD_H_INCLUDED
#define _MUSERECORD_H_INCLUDED

#include "MuseRecordBasic.h"

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif

class MuseRecord : public MuseRecordBasic {
   public:
                        MuseRecord                  (void);
                        MuseRecord                  (char* aLine);
                        MuseRecord                  (MuseRecord& aRecord);
                       ~MuseRecord                  ();


   //////////////////////////////
   // functions which work with regular note, cue note, and grace note records
   // (A..G, c, g)

      // columns 1 -- 5: pitch field information
      char*            getNoteField                 (char* output);
      int              getOctave                    (void);
      char*            getOctave                    (char* output);
      int              getPitch                     (void);
      char*            getPitch                     (char* output);
      int              getPitchClass                (void);
      char*            getPitchClass                (char* output);
      int              getAccidental                (void);
      char*            getAccidental                (char* output);
      int              getBase40                    (void);
      void             setPitch                     (int base40, int chordnote = 0, int gracenote = 0);
      void             setPitch                     (const char* pitchname);
      void             setPitchAtIndex              (int index, 
                                                     const char* pitchname);
      void             setChordPitch                (const char* pitchname);
      void             setGracePitch                (const char* pitchname);
      void             setGraceChordPitch           (const char* pitchname);
      void             setCuePitch                  (const char* pitchname);
      void             setStemDown                  (void);
      void             setStemUp                    (void);

      // columns 6 -- 9: duration field information
      char*            getTickDurationField         (char* output);
      char*            getTickDuration              (char* output);
      int              getTickDurationField         (void);
      int              getLineTickDuration          (void);
      int              getNoteTickDuration          (void);
      char*            getTie                       (char* output);
      int              getTie                       (void);
      int              setTie                       (int hidden = 0);
      int              tieQ                         (void);
      int              getTicks                     (void);
      void             setTicks                     (int value);
      void             setBack                      (int value);
      void             setDots                      (int value);
      void             setNoteheadShape             (RationalNumber& duration);
      void             setNoteheadShapeMensural     (RationalNumber& duration);
      void             setNoteheadMaxima            (void);
      void             setNoteheadLong              (void);
      void             setNoteheadBreve             (void);
      void             setNoteheadBreveSquare       (void);
      void             setNoteheadBreveRound        (void);

      void             setNoteheadWhole             (void);
      void             setNoteheadHalf              (void);
      void             setNoteheadQuarter           (void);
      void             setNotehead8th               (void);
      void             setNotehead16th              (void);
      void             setNotehead32nd              (void);
      void             setNotehead64th              (void);
      void             setNotehead128th             (void);
      void             setNotehead256th             (void);

      void             setNoteheadBreveMensural     (void);
      void             setNoteheadWholeMensural     (void);
      void             setNoteheadHalfMensural      (void);
      void             setNoteheadQuarterMensural   (void);
      void             setNotehead8thMensural       (void);
      void             setNotehead16thMensural      (void);
      void             setNotehead32ndMensural      (void);
      void             setNotehead64thMensural      (void);
      void             setNotehead128thMensural     (void);
      void             setNotehead256thMensural     (void);

      // columns 10 -- 12 ---> blank

      // columns 13 -- 80: graphical and interpretive information 

      // column 13: footnote flag
      char*             getFootnoteFlagField        (char* output);
      char*             getFootnoteFlag             (char* output);
      int               getFootnoteFlag             (void);
      int               footnoteFlagQ               (void);

      // column 14: level number
      char*             getLevelField               (char* output);
      char*             getLevel                    (char* output);
      int               getLevel                    (void);
      int               levelQ                      (void);

      // column 15: track number
      char*             getTrackField               (char* output);
      char*             getTrack                    (char* output);
      int               getTrack                    (void);
      int               trackQ                      (void);

      // column 16 ---> blank

      // column 17: graphic note type
      char*             getGraphicNoteTypeField     (char* output);
      char*             getGraphicNoteType          (char* output);
      int               getGraphicNoteType          (void);
      int               getGraphicNoteTypeSize      (void);
      int               graphicNoteTypeQ            (void);

      // column 18: dots of prolongation
      char*             getProlongationField        (char* output);
      char*             getProlongation             (char* output);
      int               getProlongation             (void);
      char*             getStringProlongation       (char* output);
      int               prolongationQ               (void);

      // column 19: actual notated accidentals
      char*            getNotatedAccidentalField    (char* output);
      char*            getNotatedAccidental         (char* output);
      int              getNotatedAccidental         (void);
      int              notatedAccidentalQ           (void);

      // columns 20 -- 22: time modification
      char*            getTimeModificationField     (char* output);
      char*            getTimeModification          (char* output);
      char*            getTimeModificationLeftField (char* output);
      char*            getTimeModificationLeft      (char* output);
      int              getTimeModificationLeft      (void);
      char*            getTimeModificationRightField(char* output);
      char*            getTimeModificationRight     (char* output);
      int              getTimeModificationRight     (void);
      int              timeModificationQ            (void);
      int              timeModificationLeftQ        (void);
      int              timeModificationRightQ       (void);

      // column 23
      char*            getStemDirectionField        (char* output);
      char*            getStemDirection             (char* output);
      int              getStemDirection             (void);
      int              stemDirectionQ               (void);

      // column 24
      char*            getStaffField                (char* output);
      char*            getStaff                     (char* output);
      int              getStaff                     (void);
      int              staffQ                       (void);
      
      // column 25 ---> blank

      // columns 26 - 31: beam codes
      char*            getBeamField                 (char* output);
      int              beamQ                        (void);
      char             getBeam8                     (void);
      char             getBeam16                    (void);
      char             getBeam32                    (void);
      char             getBeam64                    (void);
      char             getBeam128                   (void);
      char             getBeam256                   (void);
      int              beam8Q                       (void);
      int              beam16Q                      (void);
      int              beam32Q                      (void);
      int              beam64Q                      (void);
      int              beam128Q                     (void);
      int              beam256Q                     (void);
      char*            getKernBeamStyle             (char* output);
      void             setBeamInfo                  (Array<char>& strang);

      // columns 32 -- 43: additional notation
      char*            getAdditionalNotationsField  (char* output);
      int              additionalNotationsQ         (void);
      int              getAddCount                  (void);
      char*            getAddItem                   (int elementIndex, 
                                                       char* output);
      int              addAdditionalNotation        (char symbol);
      int              addAdditionalNotation        (const char*  symbol);
      int              getAddItemLevel              (int elementIndex);
      char*            getEditorialLevels           (char* output);
      int              addEditorialLevelQ           (void);
      //  protected:   getAddElementIndex
      int              findField                    (const char* key);
      int              findField                    (char key, int mincol, 
                                                     int maxcol);
      // int              getNotationLevel
      int              getSlurStartColumn           (void);

      // columns 44 -- 80: text underlay
      char*            getTextUnderlayField         (char* output);
      int              textUnderlayQ                (void);
      int              getVerseCount                (void);
      char*            getVerse                     (int index, char* output);

      // general functions for note records:
      char*            getKernNoteStyle             (char* output, 
                                                       int beams = 0,
                                                       int stems = 0);
      char*            getKernNoteAccents           (char* output);


   //////////////////////////////
   // functions which work with basso continuo figuration records ('f'):

      // column 2: number of figure fields
      char*            getFigureCountField          (char* output);
      char*            getFigureCount               (char* output);
      int              getFigureCount               (void);

      // columns 3 -- 5 ---> blank
      
      // columns 6 -- 8: figure division pointer advancement (duration)
      char*            getFigurePointerField        (char* output);
      int              figurePointerQ               (void);
      // same as note records: getDuration

      // columns 9 -- 12 ---> blank

      // columns 13 -- 15: footnote and level information
      // column 13 --> footnote: uses same functions as note records in col 13.
      // column 14 --> level: uses same functions as note records on column 14.
      // column 15 ---> blank

      // columns 17 -- 80: figure fields
      char*            getFigureFields              (char* output);
      int              figureFieldsQ                (void);
      char*            getFigure                    (int index, char* output);
 

   //////////////////////////////
   // functions which work with combined records ('b', 'i'):


   //////////////////////////////
   // functions which work with measure records ('m'):

      // columns 1 -- 7: measure style information
      char*            getMeasureTypeField          (char* output);

      // columns 9 -- 12: measure number (left justified)
      char*            getMeasureNumberField        (char* output);
      char*            getMeasureNumber             (char* output);
      int              getMeasureNumber             (void);
      int              measureNumberQ               (void);

      // columns 17 -- 80: measure flags
      int              measureFermataQ              (void);
      int              measureFlagQ                 (const char* key);
      void             addMeasureFlag               (const char* strang);

      // general functions for measure records:
      char*            getKernMeasureStyle          (char* output);


   //////////////////////////////
   // functions which work with musical attributes records ('$'):

      char*            getAttributeList             (char* output);
      int              attributeQ                   (const char* attribute);
      int              getAttributeInt              (char attribute);
      int              getAttributeString           (char* output, 
                                                       const char* attribute);


   //
   //////////////////////////////

      char*            getKernRestStyle             (char* output, 
                                                       int quarter = 16);

   protected:
      void             allowNotesOnly               (const char* functioName);
      void             allowMeasuresOnly            (const char* functioName);
      void             allowFigurationOnly          (const char* functioName);
      void             allowFigurationAndNotesOnly  (const char* functioName);
      int              getAddElementIndex           (int& index, char* output,
                                                       char* input);
      char*            zerase                       (char* output, int num);

};
   


#endif  /* _MUSERECORD_H_INCLUDED */


// md5sum: 051e558c8e271327a795fcad656d760b MuseRecord.h [20050403]
