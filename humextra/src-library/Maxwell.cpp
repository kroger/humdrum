//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jan  1 19:54:45 PST 2003 (extracted from HumdrumFile.cpp)
// Last Modified: Wed Jan  1 19:54:59 PST 2003
// Filename:      ...sig/src/sigInfo/Maxwell.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/Maxwell.cpp
// Syntax:        C++ 
//
// Description:   Harmony analysis functions based on John Maxwell's 
//                dissertation.
//

#include "Maxwell.h"

#ifndef OLDCPP
   using namespace std;
#endif

#define MAXWELL_ROUNDERR 0.005



//////////////////////////////
//
// Maxwell::Maxwell --
//

Maxwell::Maxwell(void) {
   // do nothing
}



//////////////////////////////
//
// Maxwell::~Maxwell
//

Maxwell::~Maxwell() {
   // do nothing
}



//////////////////////////////
//
// Maxwell::analyzeVerticalDissonance -- apply the rules 1 & 2 
// from page 337:
//
// Rule 1 & Rule 2.  IF a sonority consists of only one note, OR it
// consists of two notes that form a consonant interval other than a
// perfect fourth, OR it consists of three of more notes forming only
// consonant intervals, THEN it is a consonant vertical, OTHERWISE,
// it is a dissonant vertical.
//

void Maxwell::analyzeVerticalDissonance(HumdrumFile& score, 
      Array<int>& vertdis) {
   Array<int> notes;

   vertdis.setSize(score.getNumLines());
   vertdis.setSize(0);
   vertdis.allowGrowth(1);
   int analysis = 0;

   int line;
   for (line=0; line<score.getNumLines(); line++) {
maxwellverticaldissonance_top:
      if (score[line].getType() != E_humrec_data) {
         analysis = UNDEFINED_VERTICAL;
         vertdis.append(analysis);
         continue;
      }
   
      score.getNoteList(notes, line, NL_FILL | NL_PC | NL_SORT | NL_UNIQ | 
            NL_NORESTS);
      
      // IF sonority consists of only one note, THEN it is a consonant vertical.
      if (notes.getSize() <= 1) {
         analysis = CONSONANT_VERTICAL;
         vertdis.append(analysis);
         continue;
      }
   
      // IF a sonority consists of two notes that form a consonant interval
      // other than a perfect fourth, THEN it is a consonant vertical.
      if (notes.getSize() == 2) {
         int interval = notes[1] - notes[0];
         if (interval < 0) {
            cout << "Error on line " << line+1 << " of file: " 
                 << "problem determing interval" << endl;
            exit(1);
         }
   
         interval = interval % 40;
         if (interval == E_base40_per4) {
            analysis =  DISSONANT_VERTICAL;
            vertdis.append(analysis);
            continue;
         } else if (interval == E_base40_per1 ||
                    interval == E_base40_maj3 ||
                    interval == E_base40_min3 ||
                    interval == E_base40_min6 ||
                    interval == E_base40_maj6 ||
                    interval == E_base40_per5) {
            analysis = CONSONANT_VERTICAL;
            vertdis.append(analysis);
            continue;
         } else {
            analysis = DISSONANT_VERTICAL;
            vertdis.append(analysis);
            continue;
         }
      }
   
      // IF a sonority consists of three or more notes forming only 
      // consonant intervals, THEN it is a consonant vertical.
      int i, j;
      int interval;
      for (i=0; i<notes.getSize()-1; i++) {
         for (j=i+1; j<notes.getSize(); j++) {
            interval = notes[j] - notes[i];
            interval = interval % 40;
            if (interval == E_base40_per1 ||
                interval == E_base40_maj3 || interval == E_base40_min3 ||
                interval == E_base40_min6 || interval == E_base40_maj6 ||
                interval == E_base40_per4 || interval == E_base40_per5) {
               // do nothing
            } else {
               analysis = DISSONANT_VERTICAL;
               vertdis.append(analysis);
               line++;
               goto maxwellverticaldissonance_top;
            }
         }
      }
   
      // the chord notes contain only consonant intervals.
      analysis = CONSONANT_VERTICAL;
      vertdis.append(analysis);
   }

   vertdis.allowGrowth(0);
}



//////////////////////////////
//
// Maxwell::analyzeTertian -- apply the rule 5 on page 337:
//
// Rule 5: IF the pitch classes of a vertical sonority can be arranged
// so that each note is separated from its neighbor(s) by a third or fifth,
// and the pitches all lie within a single octave, THEN it is tertian.
//

void Maxwell::analyzeTertian(HumdrumFile& score, Array<int>& tertian) {
   Array<int> notes;
   int analysis;

   tertian.setSize(score.getNumLines());
   tertian.setSize(0);
   tertian.allowGrowth(1);
   
   int line;
   for (line=0; line<score.getNumLines(); line++) {
analyzemaxwelltertian_top:

      if (score[line].getType() != E_humrec_data) {
         analysis = TERTIAN_UNKNOWN;
         tertian.append(analysis);
         continue;
      }
   
      score.getNoteList(notes, line, NL_PC | NL_FILL | NL_SORT | NL_UNIQ | 
            NL_NORESTS);
      
      // if there is one or fewer notes, then cannot be tertian
      if (notes.getSize() <= 1) {
         analysis = TERTIAN_NO;
         tertian.append(analysis);
         continue;
      }
   
      int i, j;
      int foundTertianQ = 0;
      int interval;
      for (i=0; i<notes.getSize(); i++) {
         foundTertianQ = 1;
         for (j=1; j<notes.getSize(); j++) {
            interval = notes[j] - notes[j-1];
            if (interval == E_base40_maj3 ||
                interval == E_base40_min3 ||
                interval == E_base40_dim3 ||
                interval == E_base40_aug3 ||
                interval == E_base40_per5 ||
                interval == E_base40_dim5 ||
                interval == E_base40_aug5) {
               // do nothing
            } else {
               // invalid interval
               foundTertianQ = 0;
               break;
            }
         }
         if (foundTertianQ == 1) {
            analysis = TERTIAN_YES;
            tertian.append(analysis);
            line++;
            goto analyzemaxwelltertian_top;
         }
         rotateNotes(notes);
      }

      analysis = TERTIAN_NO;
      tertian.append(analysis);
   }

   tertian.allowGrowth(0);
}



//////////////////////////////
//
// Maxwell::rotateNotes -- for use with analyzeTertian
//      and analyzeTertianDissonanceLevel.
// 

void Maxwell::rotateNotes(Array<int>& notes) {
   if (notes.getSize() < 2) {
      return;
   }

   int note = notes[0];
   int i;
   for (i=0; i<notes.getSize() - 1; i++) {
      notes[i] = notes[i+1];
   }
   notes[notes.getSize()-1] = note + 40;
}



//////////////////////////////
//
// Maxwell::analyzeAccent -- -- apply the rule 6 on page 337:
//
// Rule 6.  IF a musical event falls on the "primary beat unit" of
//    the meter (as defined by a table relating meters to beat units)
//    THEN it is accented.
//

void Maxwell::analyzeAccent(HumdrumFile& score, Array<int>& accent, int flag) {
   int compoundQ = flag & (0x01<<COMPOUND_METER_BIT);
   int metercount = 4;
   int meterbase = 4;
   int analysis;

   accent.allowGrowth(1);
   accent.setSize(score.getNumLines());
   accent.setSize(0);
   
   if (score.rhythmQ() == 0) {
      score.analyzeRhythm();
   }

   int line, j;
   for (line=0; line<score.getNumLines(); line++) {
      if (score[line].getType() == E_humrec_interpretation) {
         for (j=0; j<score[line].getFieldCount(); j++) {
            if (score[line].getExInterpNum(j) != E_KERN_EXINT) {
               continue;
            }
            if ((strncmp(score[line][j], "*M", 2) == 0) && 
                  isdigit(score[line][j][2]) && 
                  (strchr(score[line][j], '/') != NULL)) {
               sscanf(score[line][j], "*M%d/%d", &metercount, &meterbase);
               break;
            }
         }
      }

      if (score[line].getType() != E_humrec_data) {
         analysis = ACCENT_UNKNOWN;
         accent.append(analysis);
         continue;
      }

      double fraction;
      fraction = score[line].getBeat();
      if (compoundQ && (metercount % 3 == 0)) {
         fraction = fraction/3;
      }
      int beat = (int)fraction;
      fraction = fraction - beat;
      if (fraction < MAXWELL_ROUNDERR) {
         fraction = 0;
      } else if (fraction > (1-MAXWELL_ROUNDERR)) {
         fraction = 0;
         beat++;
      }
   
      if (fraction == 0) {
         analysis = ACCENT_YES;
         accent.append(analysis);
      } else {
         analysis = ACCENT_NO;
         accent.append(analysis);
      }
   }

   accent.allowGrowth(0);
}



//////////////////////////////
//
// Maxwell::analyzeTertianDissonanceLevel -- based on the table 
//     on page 338.  Modified value for Major-minor seventh chords
//
// TERTIAN_UNKNOWN =  -1
// TERTIAN_0       =   0   single notes 
// TERTIAN_1       =   1   consonant intervals, maj/min triads
// TERTIAN_1_5     =   1.5 Mm seventh chords
// TERTIAN_2       =   2   tritone, dim triad, mm
// TERTIAN_2_5     =   2.5   incomplete mm sevenths
// TERTIAN_3       =   3   aug fifth, aug triad
// TERTIAN_4       =   4   1/2 dim 7, dim 7, aug 6
// TERTIAN_5       =   5   MM, M7, every thing else
//

void Maxwell::analyzeTertianDissonanceLevel(HumdrumFile& score, 
      Array<double>& terdis) {

   terdis.setSize(score.getNumLines());
   terdis.setSize(0);
   terdis.allowGrowth(1);

   Array<int> notes;
   double analysis;

   int line;
   for (line=0; line<score.getNumLines(); line++) {
analyzemaxwelltertiandissonancelevel_top:
      if (score[line].getType() != E_humrec_data) {
         analysis = TERTIAN_UNKNOWN;
         terdis.append(analysis);
         continue;
      }
   
      score.getNoteList(notes, line, NL_PC | NL_FILL | NL_SORT | NL_UNIQ | 
            NL_NORESTS);
      
      if (notes.getSize() <= 1) {
         analysis = TERTIAN_0;
         terdis.append(analysis);
         continue;
      }
   
      int i, j;
      int foundTertianQ = 0;
      int interval;
      for (i=0; i<notes.getSize(); i++) {
         foundTertianQ = 1;
         for (j=1; j<notes.getSize(); j++) {
            interval = notes[j] - notes[j-1];
            if (interval == E_base40_maj3 ||
                interval == E_base40_min3 ||
                interval == E_base40_dim3 ||
                interval == E_base40_aug3 ||
                interval == E_base40_per5 ||
                interval == E_base40_dim5 ||
                interval == E_base40_aug5) {
               // do nothing
            } else {
               // invalid interval
               foundTertianQ = 0;
               break;
            }
         }
         if (foundTertianQ == 1) {
            break;
         }
         rotateNotes(notes);
      }
   
      // filter augmented sixth chords:
      for (i=1; i<notes.getSize(); i++) {
         interval = notes[i] - notes[i-1];
         if (interval == E_base40_aug6 || interval == E_base40_dim3) {
            analysis = TERTIAN_4;
            terdis.append(analysis);
            line++;
            goto analyzemaxwelltertiandissonancelevel_top;
         }
      }
   
      // filter out non tertian sonorities:
      if (!foundTertianQ) {
         analysis = TERTIAN_5;
         terdis.append(analysis);
         continue;
      }
   
      // filter tertian chords with major sevenths:
      interval = notes[notes.getSize() - 1] - notes[0];
      if (interval == E_base40_maj7) {
         analysis = TERTIAN_5;
         terdis.append(analysis);
         continue;
      }
   
      // filter out augmented fifth chords:
      if (notes[notes.getSize() - 1] - notes[0] < E_base40_min6) {
         for (i=0; i<notes.getSize() - 1; i++) {
            for (j=i+1; j<notes.getSize(); j++) {
               interval = notes[j] - notes[j-1];
               if (interval == E_base40_aug5) {
                  analysis = TERTIAN_3;
                  terdis.append(analysis);
                  line++;
                  goto analyzemaxwelltertiandissonancelevel_top;
               }
            }
         }
      }
   
      if (notes.getSize() == 2) {
         if (notes[1] - notes[0] == E_base40_dim5) {
            analysis = TERTIAN_2;
            terdis.append(analysis);
            continue;
         } else {
            analysis = TERTIAN_1;
            terdis.append(analysis);
            continue;
         }
      }
   
      int interval1;
      int interval2;
      interval1 = notes[1] - notes[0];
      interval2 = notes[2] - notes[0];
      if (notes.getSize() == 3) {
         if (interval2 == E_base40_per5) {
            if (interval1 == E_base40_maj3 || interval1 == E_base40_min3) {
               analysis = TERTIAN_1;
               terdis.append(analysis);
               continue;
            } else {
               analysis = TERTIAN_5;
               terdis.append(analysis);
               continue;
            }
         } else if (interval2 == E_base40_dim5) {
            if (interval1 == E_base40_min3) {
               analysis = TERTIAN_2;
               terdis.append(analysis);
               continue;
            } else {
               analysis = TERTIAN_5;
               terdis.append(analysis);
               continue;
            }
         } else if (interval2 == E_base40_min7) {
            if (interval1 == E_base40_maj3 ||
                interval1 == E_base40_per5) {
               analysis = TERTIAN_2;
               terdis.append(analysis);
               continue;
            } else if (interval1 == E_base40_min3) {
               analysis = TERTIAN_2_5;
               terdis.append(analysis);
               continue;
            } else {
               analysis = TERTIAN_5;
               terdis.append(analysis);
               continue;
            }
         } else if (interval2 == E_base40_dim7) {
            if (interval1 == E_base40_min3 ||
                interval1 == E_base40_dim5 ||
                interval1 == E_base40_per5) {
               analysis = TERTIAN_4;
               terdis.append(analysis);
               continue;
            } else {
               analysis = TERTIAN_5;
               terdis.append(analysis);
               continue;
            }
         } else {
            analysis = TERTIAN_5;
            terdis.append(analysis);
            continue;
         }
      }
   
      int interval3;
      interval1 = notes[1] - notes[0];
      interval2 = notes[2] - notes[0];
      interval3 = notes[3] - notes[0];
      if (notes.getSize() == 4) {
         if (interval3 == E_base40_maj7) {
            analysis = TERTIAN_5;
            terdis.append(analysis);
            continue;
         } else if (interval3 == E_base40_min7) {
            if (interval2 == E_base40_per5) {
               if (interval1 == E_base40_min3) {
                  analysis = TERTIAN_2;
                  terdis.append(analysis);
                  continue;
               } else if (interval1 == E_base40_maj3) {
                  analysis = TERTIAN_1_5;
                  terdis.append(analysis);
                  continue;
               }
            }
         } else if (interval3 == E_base40_dim7) {
            if (interval2 == E_base40_per5) {
               if (interval1 == E_base40_min3) {
                  analysis = TERTIAN_4;
                  terdis.append(analysis);
                  continue;
               } 
            } else if (interval2 == E_base40_dim5) {
               if (interval1 == E_base40_min3) {
                  analysis = TERTIAN_4;
                  terdis.append(analysis);
                  continue;
               } 
            } 
         } 
      }
   
      // couldn't identify tertian chord
      analysis = TERTIAN_5;
      terdis.append(analysis);
   }

   terdis.allowGrowth(0);
}



//////////////////////////////
//
// Maxwell::analyzeDissonantInContext -- determine dissonance 
//     in context value using definition by John Maxwell:
//
// Rule 7: IF [a sonority is not tertian] OR [it is accented AND dissonant AND
// the next sonority is tertian AND the next sonority has a lower 
// tertian-dissonance level] OR [it is unaccented AND dissonant AND the last 
// sonority is tertian AND the last sonority has a lower tertian-dissonance 
// level], THEN the sonority is dissonant in context.
//
// Default values: flag = AFLAG_COMPOUND_METER
//

void Maxwell::analyzeDissonantInContext(HumdrumFile& score, Array<int>& dissic, 
      int flag) {
   Array<int> vertdis(0);
   Array<int> tertian(0);
   Array<double> terdis(0);
   Array<int> accent(0);

   analyzeDissonantInContext(score, dissic, vertdis, tertian, 
         terdis, accent, flag);
}


void Maxwell::analyzeDissonantInContext(HumdrumFile& score, 
      Array<int>& dissic, Array<int>& vertdis, Array<int>& tertian, 
      Array<double>& terdis, Array<int>& accent, int flag) {

   int analysis;

   dissic.setSize(score.getNumLines());
   dissic.setSize(0);
   dissic.allowGrowth(1);

   if (score.rhythmQ() == 0) {
      score.analyzeRhythm();
   } 

   // make sure dependent analyses have been done
   if (vertdis.getSize() != score.getNumLines()) {
      analyzeVerticalDissonance(score, vertdis);
   }
   if (tertian.getSize() != score.getNumLines()) {
      analyzeTertian(score, tertian);
   }
   if (terdis.getSize() != score.getNumLines()) {
      analyzeTertianDissonanceLevel(score, terdis);
   }
   if (accent.getSize() != score.getNumLines()) {
      analyzeAccent(score, accent, flag);
   }

   int i;
   int line;
   for (line=0; line<score.getNumLines(); line++) {

      if (score[line].getType() != E_humrec_data) {
         analysis = DISSIC_UNDEFINED;
         dissic.append(analysis);
         continue;
      }

      // case 1: IF a sonority is not tertian, THEN the sonority is dissonant
      // in context.
      if (tertian[line] == TERTIAN_NO) {
         analysis = DISSIC_YES;
         dissic.append(analysis);
         continue;
      }
   
      // case 2: IF a sonority is accented AND dissonant AND the next 
      // sonority is tertian AND the next sonority has a lower 
      // tertian-dissonance level, THEN the sonority is dissonant in context.
      double nextdislevel = -1;
      int nexttertian = -1;
      for (i=line+1; i<tertian.getSize(); i++) {
         if (tertian[i] != TERTIAN_UNKNOWN) {
            nextdislevel = terdis[i];
            nexttertian  = tertian[i];
            break;
         } 
      }
      if ((accent[line] == ACCENT_YES) && 
            (vertdis[line] == DISSONANT_VERTICAL) &&
            (nexttertian == TERTIAN_YES) && (nextdislevel < terdis[line])) {
         analysis = DISSIC_YES;
         dissic.append(analysis);
         continue;
      }
   
      // case 3: IF a sonority is unaccented AND dissonant AND the last sonority
      // is tertian AND the last sonority has a lower tertian-dissonance level,
      // THEN the sonority is dissonant in context.
      int lasttertian = -1;
      double lastterdis = -1;
      for (i=line-1; i>0; i--) {
         if (tertian[line] != TERTIAN_UNKNOWN) {
            lastterdis = terdis[i];
            lasttertian  = tertian[i];
            break;
         } 
      }
      if ((accent[line] == ACCENT_NO) && 
          (vertdis[line] == DISSONANT_VERTICAL) &&
          (lasttertian == TERTIAN_YES) && (lastterdis < terdis[line])) {
         analysis = DISSIC_YES;
         dissic.append(analysis);
         continue;
      }
   
      analysis = DISSIC_NO;
      dissic.append(analysis);
   }

   dissic.allowGrowth(0);
}



//////////////////////////////
//
// Maxwell::analyzeDissonantNotes -- determine if notes are 
//     dissonant using rules by John Maxwell.
//
// Rule 8: IF a note is in a vertical that is both "dissonant" and 
// "dissonant in context," AND the note formas a dissonant interval with 
// another note in the vertical, AND the vertical is accented, AND the
// note is note continued or repeated in the next vertical, THEN the note
// is a dissonant note.
//
// Rule 9: IF a note is in a vertical that is both "dissonant" and
// "dissonant in context", AND the note forms a dissonant interval
// with another note in the vertical, AND the vertical is unaccented, AND
// the note is not continued or repeated from the previous vertical, THEN
// the note is a dissonant note.
//

void Maxwell::analyzeDissonantNotes(HumdrumFile& score, 
      Array<ArrayInt>& notediss) {
   Array<int> vertdis(0);
   Array<int> tertian(0);
   Array<double> terdis(0);
   Array<int> accent(0);
   Array<int> dissic(0);

   analyzeDissonantNotes(score, notediss, vertdis, tertian, terdis, accent, 
         dissic);
}


void Maxwell::analyzeDissonantNotes(HumdrumFile& score, 
      Array<ArrayInt>& notediss, Array<int>& vertdis, Array<int>& tertian, 
      Array<double>& terdis, Array<int>& accent, Array<int>& dissic) {

   int i;
   Array<int> currentnotes;

   if (score.rhythmQ() == 0) {
      score.analyzeRhythm();
   }

   notediss.setSize(score.getNumLines());
   for (i=0; i<notediss.getSize(); i++) {
      notediss[i].setSize(0); 
   }

   // make sure dependent analyses have been done
   if (dissic.getSize() != score.getNumLines()) {
      analyzeDissonantInContext(score, dissic, vertdis, tertian, 
         terdis, accent);
   }
   if (vertdis.getSize() != score.getNumLines()) {
      analyzeVerticalDissonance(score, vertdis);
   }
   if (tertian.getSize() != score.getNumLines()) {
      analyzeTertian(score, tertian);
   }
   if (terdis.getSize() != score.getNumLines()) {
      analyzeTertianDissonanceLevel(score, terdis);
   }
   if (accent.getSize() != score.getNumLines()) {
      analyzeAccent(score, accent);
   }


   int line;
   for (line=0; line<score.getNumLines(); line++) {
      score.getNoteList(currentnotes, line, NL_NOFILL | NL_NOSORT |
            NL_NOUNIQ | NL_NORESTS);
      for (i=0; i<currentnotes.getSize(); i++) {
         currentnotes[i] = measureNoteDissonance(score, line, currentnotes[i], 
               vertdis, accent, dissic);
      }

      notediss[line] = currentnotes;
   }

   notediss.allowGrowth(0);
}



//////////////////////////////
//
// Maxwell::measureNoteDissonance -- helper function used by the 
//   analyzeDissonantNotes function.
//

int Maxwell::measureNoteDissonance(HumdrumFile& score, int line, int note, 
      Array<int>& vertdis, Array<int>& accent, Array<int>& dissic) {

   if (score[line].getType() != E_humrec_data) {
      return NOTEDISSONANT_UNKNOWN;
   }

   if (note == E_base40_rest) {
      return NOTEDISSONANT_UNKNOWN;
   }

   // remove octave information from note:
   note = note % 40;

   int dissintervalQ = 0;
   int interval;
   Array<int> notes;
   score.getNoteList(notes, line, NL_PC | NL_FILL | NL_SORT | NL_UNIQ | 
         NL_NORESTS);
   int i, j;
   for (i=0; i<notes.getSize()-1; i++) {
      for (j=i+1; j<notes.getSize(); j++) {
         interval = notes[j] - notes[i];
         if (interval == E_base40_per1 ||
             interval == E_base40_min3 ||
             interval == E_base40_maj3 ||
             interval == E_base40_per4 ||
             interval == E_base40_per5 ||
             interval == E_base40_min6 ||
             interval == E_base40_maj6) {
            // do nothing
         } else {
            dissintervalQ = 1;
            break;
         }
      }
   }

   Array<int> nextnotes;
   nextnotes.setSize(0);
   int nextline = line+1;
   while ((nextline < score.getNumLines()) && 
         (score[nextline].getType() != E_humrec_data)) {
      nextline++;
   }
   int noteinnextQ = 0;
   if (nextline < score.getNumLines()) {
      score.getNoteList(nextnotes, nextline, NL_PC | NL_FILL | NL_SORT | 
            NL_UNIQ | NL_NORESTS);
      noteinnextQ = 0;
      for (i=0; i<nextnotes.getSize(); i++) {
         if (note == nextnotes[i]) {
            noteinnextQ = 1;
            break;
         }
      }
   } else {
      noteinnextQ = 0;
   }

   // Rule 8:
   if ((vertdis[line] == DISSONANT_VERTICAL) &&
       (dissic[line] == DISSIC_YES) && dissintervalQ &&
       (accent[line] == ACCENT_YES) && !noteinnextQ) {
      return NOTEDISSONANT_YES;
   }

   int noteinlastQ = 0;
   Array<int> lastnotes;
   lastnotes.setSize(0);
   int lastline = line-1;
   while ((lastline > 0) && (score[lastline].getType() != E_humrec_data)) {
      lastline--;
   }
   if (lastline > 0) {
      score.getNoteList(lastnotes, lastline, NL_PC | NL_FILL | NL_SORT | 
            NL_UNIQ | NL_NORESTS);
      noteinlastQ = 0;
      for (i=0; i<lastnotes.getSize(); i++) {
         if (note == lastnotes[i]) {
            noteinlastQ = 1;
            break;
         }
      }
   } else {
      noteinlastQ = 0;
   }

   // Rule 9:
   if ((vertdis[line] == DISSONANT_VERTICAL) &&
       (dissic[line] == DISSIC_YES) && dissintervalQ &&
       (accent[line] == ACCENT_NO) && !noteinlastQ) {
      return NOTEDISSONANT_YES;
   }

   return NOTEDISSONANT_NO;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// Maxwell::analyzeSonorityRelations --
//    Default value: flag = AFLAG_COMPOUND_METER
//

void Maxwell::analyzeSonorityRelations(HumdrumFile& score, Array<int>&sonrel, 
      int flag) {

   Array<int> vertdis;
   Array<int> tertian;
   Array<double> terdis;
   Array<int> accent;
   Array<int> dissic;
   Array<double> beatdur;
   Array<ChordQuality> cq;

   analyzeSonorityRelations(score, sonrel, vertdis, tertian, terdis,
         accent, dissic, beatdur, cq, flag);
}


void Maxwell::analyzeSonorityRelations(HumdrumFile& score, Array<int>&sonrel,
      Array<int>& vertdis, Array<int>& tertian, Array<double>& terdis,
      Array<int>& accent, Array<int>& dissic, Array<double>& beatdur,
      Array<ChordQuality>& cq, int flag) {

   int debugQ = flag & (1<<DEBUG_BIT);

   if (score.rhythmQ() == 0) {
      score.analyzeRhythm();
   }

   sonrel.setSize(score.getNumLines());
   sonrel.setSize(0);
   sonrel.allowGrowth(1);

   // make sure dependent analyses have been done

   if (dissic.getSize() != score.getNumLines()) {
      analyzeDissonantInContext(score, dissic, vertdis, tertian,
         terdis, accent, flag);
   }
   if (vertdis.getSize() != score.getNumLines()) {
      analyzeVerticalDissonance(score, vertdis);
   }
   if (tertian.getSize() != score.getNumLines()) {
      analyzeTertian(score, tertian);
   }
   if (terdis.getSize() != score.getNumLines()) {
      analyzeTertianDissonanceLevel(score, terdis);
   }
   if (accent.getSize() != score.getNumLines()) {
      analyzeAccent(score, accent, flag);
   }
   if (cq.getSize() != score.getNumLines()) {
      score.analyzeSonorityQuality(cq);
   }                                     

   if (beatdur.getSize() != score.getNumLines()) {
      score.analyzeBeatDuration(beatdur, flag);
   }

   int line;
   int analysis1;
   for (line=0; line<score.getNumLines(); line++) {
      if (debugQ) {
         cout << "Processing line " << line+1 
              << " in analyzeSonorityRelations" << endl;
      }

      if (score[line].getType() != E_humrec_data) {
         analysis1 = CHORD_UNKNOWN;
         sonrel.append(analysis1);
         continue;
      }

      analysis1 = measureChordFunction1(score, line, vertdis, tertian, terdis,
            accent, dissic, beatdur, cq);
      sonrel.append(analysis1);
   }

   sonrel.allowGrowth(0);
}



//////////////////////////////
//
// Maxwell::measureChordFunction1 -- apply rules 20 through 28 to identify
//    chord and non-chord sonorities.
//

int Maxwell::measureChordFunction1(HumdrumFile& score, int line, 
      Array<int>& vertdis, Array<int>& tertian, Array<double>& terdis, 
      Array<int>& accent, Array<int>& dissic, Array<double>& beatdur, 
      Array<ChordQuality>& cq) {

   //
   // preliminaries 
   //

   int lastline = line - 1;
   while (lastline > 0 && score[lastline].getType() != E_humrec_data) {
      lastline--;
   }

   int nextline = line + 1;
   while (nextline < score.getNumLines() && 
         score[nextline].getType() != E_humrec_data) {
      nextline++;
   }
   if (nextline >= score.getNumLines()) {
      nextline = -1;
   }


   ////////////////////

   // Rule 20: IF a vertical is tertian AND its duration is at least as long
   // as the primary beat of the meter, THEN it is a chord.

   if ((tertian[line] == TERTIAN_YES) && 
       (score[line].getDuration() >= beatdur[line])) {
      return CHORD_CHORD;
   }

   // Rule 21: IF a vertical is tertian AND it is accented AND it is not
   // "dissonant in context," THEN it is a chord.

   if ((tertian[line] == TERTIAN_YES) && (accent[line] == ACCENT_YES) &&
       (dissic[line] == DISSIC_NO)) {
      return CHORD_CHORD;
   }

   // Rule 22: IF a vertical is unaccented AND it is tertian AND the previous
   // vertical is tertian AND they both have the same root, THEN the vertical
   // is subordinate to the previous vertical.

   if ((accent[line] == ACCENT_NO) && (tertian[line] == TERTIAN_YES) &&
       (lastline > 0) && (tertian[lastline] == TERTIAN_YES) &&
       (cq[line].getRoot() >= 0) &&
       (cq[line].getRoot() == cq[lastline].getRoot())) {
      return CHORD_PREVSUB;
   }

   // Rule 23: IF a vertical is accented AND it is dissonant in context AND
   // the next vertical is tertian, THEN it is subordinate to the next vertical,
   // and is not an independent chord.

   if ((accent[line] == ACCENT_YES) && (dissic[line] == DISSIC_YES) &&
       (nextline > 0) && (tertian[nextline] == TERTIAN_YES)) {
      return CHORD_NEXTSUB;
   }

   // Rule 24: IF a vertical is unaccented AND it is not dissonant in context
   // AND the last vertical is dissonant in context, THEN the vertical is a 
   // chord.

   if ((accent[line] == ACCENT_NO) && (dissic[line] == DISSIC_NO) &&
      (lastline > 0) && (dissic[lastline] == DISSIC_YES)) {
      return CHORD_CHORD;
   }

   // Rule 25: IF a vertical is tertian AND the previous vertical is
   // tertian AND the previous vertical is not dissonant in context AND the
   // two verticals do not have the same root, THEN the vertical is a chord,
   // but is marked passing.

   if ((tertian[line] == TERTIAN_YES) && (lastline > 0) &&
       (tertian[lastline] == TERTIAN_YES) && 
       (dissic[lastline] == DISSIC_NO) && 
       (cq[line].getRoot() >= 0) && (cq[lastline].getRoot() >= 0) &&
       (cq[line].getRoot() != cq[lastline].getRoot())) {
      return CHORD_PASSING;
   }

   // Rule 26: IF a vertical is not tertian AND it is accented, THEN
   // it is subordinate to whatever chord follows.

   if ((tertian[line] == TERTIAN_NO) && (accent[line] == ACCENT_YES)) {
      return CHORD_NEXTSUB;
   }

   // Rule 27: IF a vertical is not tertian AND it is unaccented, THEN
   // it is subordinate to whatever chord precedes it.

   if ((tertian[line] == TERTIAN_NO) && (accent[line] == ACCENT_NO)) {
      return CHORD_PREVSUB;
   }

   // Rule 28: IF a vertical contains a suspension AND the resolution
   // note of the suspension can be substituted for the suspension note to
   // form an implied tertian vertical AND the implied tertian vertical is
   // more consonant than the original vertical, THEN the implied vertical
   // should be substituted for the suspension vertical, and is made a
   // chord.

   // Rule 29: IF a vertical is unaccented AND it is not dissonant in
   // context AND it is a two-voice sonority AND the previous vertical is a
   // two-voice sonority AND the notes of the vertical can be combined with
   // the notes of the previous vertical producing an implied tertian
   // sonority with the same root as the previous vertical, THEN the
   // unaccented vertical should be subordinate to the previous vertical,
   // and its notes can be included with those of the previous vertical.

   // Rule 30: IF a vertical consists of a single note from a single
   // voice, THEN the vertical is subordinate to the previous chord.

   return CHORD_UNKNOWN;
}



//////////////////////////////
//
// Maxwell::measureChordFunction2 -- apply rules 31 through 36 to identify
//    chord and non-chord sonorities.  These rules must be applied after
//    the key has been determined.
//

int Maxwell::measureChordFunction2(HumdrumFile& score, int line, 
      Array<int>& vertdis, Array<int>& tertian, Array<double>& terdis, 
      Array<int>& accent, Array<int>& dissic, Array<ChordQuality>& cq) {

   // Rule 31: IF a chord is in the same key as the previous chord AND
   // the chord has the same harmonic function as the previous chord AND
   // they have the same inversion, THEN the chord can be subordinated to
   // the previous chord.

   // Rule 32: IF a chord is accented AND it is followed by a chord
   // with a weaker harmonic strength, which is followed by a chord with the
   // same function as the first chord, THEN the second two chrods can be
   // subordinated to the first chord.

   // Rule 33: IF a chord is accented AND it is followed by a chord
   // that is unaccented AND the second chord has a weaker harmonic function
   // than the first, THEN the second chord is subordinate to the first.

   // Rule 34: IF a chord contains a suspension AND the resolution of
   // the suspension note can be substituted for the suspension note to
   // produce a chord with a greater function strength that the original
   // chord, THEN the new function should be substituted for the old
   // function. (This rule does note combine chords, but can cause other
   // rules to do so.).

   // Rule 35: IF a chord is accented AND it is on a strong beat AND
   // it is a six-four chord AND the next chord has the same bass note AND
   // the next chord is in root position, THEN the first chord is
   // subordinate to the second.

   // Rule 36: IF a chord is a two-voice chrod AND the previous chord
   // is accented AND the two chords are in the same measure AND the notes
   // of the chord can be combined with those of the previous chord without
   // changing the root of the previous chord, THEN the chord is subordinate
   // to the previous chord.

   return CHORD_UNKNOWN;
}



// md5sum: 88665f25708fe42976fcc0653f777991 Maxwell.cpp [20050403]
