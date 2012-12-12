//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jun  8 00:38:46 PDT 1998
// Last Modified: Fri May  5 12:18:44 PDT 2000 (added kernToMidiNoteNumber())
// Last Modified: Tue Jun  6 13:51:55 PDT 2000 (durationToKernRhythm() improved)
// Last Modified: Thu Jun 29 20:41:04 PDT 2000 (fixed strange bug in inversion)
// Last Modified: Tue Nov 14 12:18:50 PST 2000 (added kernToDiatonicPitch())
// Last Modified: Wed Jan  2 12:40:12 PST 2002 (added kotoToDuration)
// Last Modified: Sat Mar  9 23:29:28 PST 2002 (durationToKernRhythm() breve)
// Last Modified: Sat Jul 19 19:32:05 PDT 2003 (added some SCORE functions)
// Last Modified: Wed Aug 20 15:47:15 PDT 2003 (allow chords in kotoToDuration)
// Last Modified: Fri Mar 26 16:12:31 PST 2004 (kernToDuration fixed for grace)
// Last Modified: Tue Oct 14 16:56:54 PDT 2008 (added 'Q' groupetto parsing)
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Last Modified: Sun Jun 21 11:35:40 PDT 2009 (updated for GCC 4.3)
// Last Modified: Wed Nov 18 16:40:33 PST 2009 (added base40/trans converts)
// Last Modified: Sat May 22 11:05:59 PDT 2010 (added RationalNumber)
// Last Modified: Sun Dec 26 04:54:46 PST 2010 (added kernClefToBaseline)
// Last Modified: Sat Jan 22 17:13:36 PST 2011 (added kernToDurationNoDots)
// Last Modified: Thu Jan 26 18:10:29 PST 2012 (fixed kotoToDurationR)
// Filename:      ...sig/src/sigInfo/Convert.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/Convert.cpp
// Syntax:        C++ 
//
// Description:   The Convert class is used to store functions
//                in a centeral location which will convert one 
//                type of data into another.
//                

#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "Convert.h"
#include "HumdrumEnumerations.h"
#include "PerlRegularExpression.h"

#ifndef OLDCPP
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #ifdef VISUAL
      #include <strstrea.h>     /* for Windows 95 */
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif
   


EnumerationEI         Convert::exint;
EnumerationCQT        Convert::chordType;
EnumerationCQI        Convert::chordInversion;
EnumerationCQR        Convert::kernPitchClass;
EnumerationMPC        Convert::musePitchClass;
EnumerationInterval   Convert::intervalNames;


///////////////////////////////////////////////////////////////////////////
//
// conversions dealing with humdrum data
//


///////////////////////////////////////////////////////////////////////////
//
// conversions dealing with **kern data
//

//////////////////////////////
//
// Convert::kernToMidiNoteNumber -- -1 means a rest or other negative
//    number; otherwise the notes should end up in the range from 0 to 127.
//

int Convert::kernToMidiNoteNumber(const char* aKernString) {
   int base40 = Convert::kernToBase40(aKernString);
   if (base40 < 0 || base40 == E_unknown) {
      return -1;
   }

   int octave = base40 / 40;
   int pitch = 0;
   switch (base40 % 40) {
      case  0: pitch = -2;     break;   // C--
      case  1: pitch = -1;     break;   // C-
      case  2: pitch = 0;      break;   // C
      case  3: pitch = 1;      break;   // C#
      case  4: pitch = 2;      break;   // C##

      case  6: pitch = 0;      break;   // D--
      case  7: pitch = 1;      break;   // D-
      case  8: pitch = 2;      break;   // D
      case  9: pitch = 3;      break;   // D#
      case 10: pitch = 4;      break;   // D##

      case 12: pitch = 2;      break;   // E--
      case 13: pitch = 3;      break;   // E-
      case 14: pitch = 4;      break;   // E
      case 15: pitch = 5;      break;   // E#
      case 16: pitch = 6;      break;   // E##

      case 17: pitch = 3;      break;   // F--
      case 18: pitch = 4;      break;   // F-
      case 19: pitch = 5;      break;   // F
      case 20: pitch = 6;      break;   // F#
      case 21: pitch = 7;      break;   // F##

      case 23: pitch = 5;      break;   // G--
      case 24: pitch = 6;      break;   // G-
      case 25: pitch = 7;      break;   // G
      case 26: pitch = 8;      break;   // G#
      case 27: pitch = 9;      break;   // G##

      case 29: pitch = 7;      break;   // A--
      case 30: pitch = 8;      break;   // A-
      case 31: pitch = 9;      break;   // A
      case 32: pitch = 10;     break;   // A#
      case 33: pitch = 11;     break;   // A##

      case 35: pitch =  9;     break;   // B--
      case 36: pitch = 10;     break;   // B-
      case 37: pitch = 11;     break;   // B
      case 38: pitch = 12;     break;   // B#
      case 39: pitch = 13;     break;   // B##

      default: 
      cout << "Pitch Unknown: " << base40 % 40 << endl;
      pitch = -1000;
   }
  
   return octave * 12 + pitch + 12;
}



//////////////////////////////
//
// Convert::durationRToKernRhythm -- not allowed to have more than
//	three rhythmic dots.
//	default value: timebase = 1;
//	input is quarter note duration == 1;
//

char* Convert::durationRToKernRhythm(char* output, RationalNumber input, 
      int timebase) {

   output[0] = '\0';

   // dividing by 4 to convert from whole note units to quarter note units.
   RationalNumber newdur = (input/4) * timebase;

   // handle very long notes:
   if (input == 8)  { strcat(output, "0");    return output; }
   if (input == 12) { strcat(output, "0.");   return output; }
   if (input == 16) { strcat(output, "00");   return output; }
   if (input == 24) { strcat(output, "00.");  return output; }
   if (input == 32) { strcat(output, "000");  return output; }
   if (input == 48) { strcat(output, "000."); return output; }

   // handle rhythms which are not dotted
   if (newdur.getNumerator() == 1) {
      sprintf(output, "%d", newdur.getDenominator());
      return output;
   }

   // check for single dot rhythms
   RationalNumber dotless1dur = (newdur*2)/3;
   if (dotless1dur.getNumerator() == 1) {
      sprintf(output, "%d.", dotless1dur.getDenominator());
      return output;
   }

   // check for double dot rhythms
   RationalNumber dotless2dur = (newdur*4)/7;
   if (dotless2dur.getNumerator() == 1) {
      sprintf(output, "%d..", dotless2dur.getDenominator());
      return output;
   }

   // check for triple dot rhythms
   RationalNumber dotless3dur = (newdur*8)/15;
   if (dotless3dur.getNumerator() == 1) {
      sprintf(output, "%d..", dotless3dur.getDenominator());
      return output;
   }

   // unknown rhythm, so output a qqq string to intdicate a grace note...
   strcpy(output, "qqq");
   return output;
}



//////////////////////////////
//
// Convert::durationToKernRhythm -- not allowed to have more than
//	three rhythmic dots
//	default value: timebase = 1;
//

char* Convert::durationToKernRhythm(char* output, double input, int timebase) {

   output[0] = '\0';
   char buffer[32] = {0};

   double testinput = input;
   double basic = 4.0 / input * timebase;
   double diff = basic - (int)basic;

   if (diff > 0.998) {
      diff = 1.0 - diff;
      basic += 0.002;
   }

   if (input == 8.0)  { strcat(output, "0");    return output; }
   if (input == 12.0) { strcat(output, "0.");   return output; }
   if (input == 16.0) { strcat(output, "00");   return output; }
   if (input == 24.0) { strcat(output, "00.");  return output; }
   if (input == 32.0) { strcat(output, "000");  return output; }
   if (input == 48.0) { strcat(output, "000."); return output; }

   // special case for triplet whole notes:
   if (fabs(input - (4.0 * 2.0 / 3.0)) < 0.0001) {
      strcat(output, "3%2");
      return output;
   }

   // special case for triplet breve notes:
   if (fabs(input - (4.0 * 4.0 / 3.0)) < 0.0001) {
      strcat(output, "3%4");
      return output;
   }

   // special case for 9/8 full rests
   if (fabs(input - (4.0 * 9.0 / 8.0)) < 0.0001) {
      strcat(output, "8%9");
      return output;
   }

   // special case for 9/2 full-measure rest
   if (fabs(input - 18.0) < 0.0001) {
      strcat(output, "2%9");
      return output;
   } 

   // handle special rounding cases primarily for SCORE which
   // only stores 4 digits for a duration
   if (input == 0.0833) {
      // triplet 32nd note, which has a real duration of 0.0833333 etc.
      strcat(output, "48");
      return output;
   }
	    
   if (diff < 0.002) {
      sprintf(buffer, "%d", (int)basic);
      strcat(output, buffer);
   } else { 
      testinput = input / 3.0 * 2.0;
      basic = 4.0 / testinput;
      diff = basic - (int)basic;
      if (diff < 0.002) {
         sprintf(buffer, "%d", (int)basic);
         strcat(output, buffer);
         strcat(output, ".");
      } else {
         testinput = input / 7.0 * 4.0;
         basic = 4.0 / testinput;
         diff = basic - (int)basic;
         if (diff < 0.002) {
            sprintf(buffer, "%d", (int)basic);
            strcat(output, buffer);
            strcat(output, "..");
         } else {
            testinput = input / 15.0 * 4.0;
            basic = 2.0 / testinput;
            diff = basic - (int)basic;
            if (diff < 0.002) {
               sprintf(buffer, "%d", (int)basic);
               strcat(output, buffer);
               strcat(output, "...");
            } else {
               sprintf(output, "q%lf", input);
               // strcpy(output, "q");
               // cerr << "Error: Convert::durationToKernRhythm choked on the "
               //      << "duration: " << input << endl; 
               // exit(1);
            }
         }
      }
   }

   return output;
}



//////////////////////////////
//
// Convert::kernToDurationNoDots --
//

double Convert::kernToDurationNoDots(const char* aKernString) {
    RationalNumber value = Convert::kernToDurationNoDotsR(aKernString);
    return value.getFloat();
}



//////////////////////////////
//
// Convert::kernToDurationNoDots --
//

RationalNumber Convert::kernToDurationNoDotsR (const char* aKernString) {
   int dotcount = 0;
   int i = 0;
   while ((aKernString[i] != '\0') && (aKernString[i] != ' ')) {
      if (aKernString[i] == '.') {
         dotcount++;
      }
      i++;
   }
   RationalNumber rn = Convert::kernToDurationR(aKernString);
   if (dotcount > 0) {
      rn = rn * (int)(pow(2.0, dotcount));
      rn = rn / (int)(pow(2.0, dotcount+1) - 1);
   }
   return rn;
}



//////////////////////////////
//
// Convert::kernToDuration -- returns the kern rhythm's duration, using
//	1.0 as the duration of a quarter note (rhythm=4).
// if the kern token has a "q" then assume that it is a grace note
// and return a duration of zero.
//

double Convert::kernToDuration(const char* aKernString) {
    RationalNumber value = Convert::kernToDurationR(aKernString);
    return value.getFloat();
}


RationalNumber Convert::kernToDurationR(const char* aKernString) {
    RationalNumber zero(0,1);

    PerlRegularExpression pre;

   // check for grace notes
   if ((strchr(aKernString, 'q') != NULL) ||
       (strchr(aKernString, 'Q') != NULL)) {
      return zero;
   }

   // check for dots to modify rhythm
   // also add an exit if a space is found, so that dots
   // from multiple notes in chord notes do not get accidentally
   // get counted together (input to this function should be a
   // single note, but chords may accidentally be sent to this
   // function instead).
   int dotcount = 0;
   int index = 0;
   while (aKernString[index] != '\0') {
      if (aKernString[index] == '.') {
         dotcount++;
      }
      if (aKernString[index] == ' ') {
         break;
      }
      index++;
   }

   // parse special rhythms which can't be represented in
   // classical **kern definition.  A non-standard rhythm
   // consists of two numbers separated by any character.
   if (pre.search(aKernString, "(\\d+)[^\\d](\\d+)", "")) {
      int rtop = atoi(pre.getSubmatch(1));
      int rbot = atoi(pre.getSubmatch(2));
      RationalNumber original(rbot, rtop);  // duration is inverse
      RationalNumber output(rbot, rtop);    // duration is inverse
      original *= 4;  // adjust to quarter note count;
      output *= 4;    // adjust to quarter note count;
      for (int i=0; i<dotcount; i++) {
         output += original / (int)(pow(2.0, (double)(i+1)));
      }
      return output;
   }

   index = 0;
   while (aKernString[index] != '\0' && !isdigit(aKernString[index])) {
      index++;
   }
   if (aKernString[index] == '\0') {
      // no rhythm data found
      return zero;
   }
   
   // should now be at start of kern rhythm
   int orhythm = 0;
   while (aKernString[index] != '\0' && isdigit(aKernString[index])) {
      orhythm *= 10;
      orhythm += aKernString[index] - '0';
      index++;
   }

   RationalNumber oduration(0,1);
   if (strchr(aKernString, '0') != NULL && 
       strchr(aKernString, '1') == NULL && 
       strchr(aKernString, '2') == NULL && 
       strchr(aKernString, '3') == NULL && 
       strchr(aKernString, '4') == NULL && 
       strchr(aKernString, '5') == NULL && 
       strchr(aKernString, '6') == NULL && 
       strchr(aKernString, '7') == NULL && 
       strchr(aKernString, '8') == NULL && 
       strchr(aKernString, '9') == NULL    ) {
      if (strstr(aKernString, "000") != NULL) {
         oduration = 32;
      } else if (strstr(aKernString, "00") != NULL) {
         oduration = 16;
      } else {
         oduration = 8;
      }

   } else {

      // now know everything to create a duration
      if (orhythm == 0) {
         oduration = 8;
      } else {
         oduration = 4;
	 oduration /= orhythm;
      }
   }

   RationalNumber duration = oduration;
   for (int i=0; i<dotcount; i++) {
      duration += oduration / (int)(pow(2.0, (double)(i+1)));
   }

   return duration;
}



//////////////////////////////
//
// Convert::kernToDiatonicPitch -- 
//

int Convert::kernToDiatonicPitch(const char* buffer) {
   return Convert::base40ToDiatonic(Convert::kernToBase40(buffer));
}



//////////////////////////////
//
// Convert::kernToDiatonicAlteration --
//

int Convert::kernToDiatonicAlteration(const char* buffer) {
   int i = 0;
   int output = 0;
   while (buffer[i] != '\0') {
      if (buffer[i] == 'n') {
         output = 0;
      } else if (buffer[i] == '#') {
         output++;
      } else if (buffer[i] == '-') {
         output--;
      } else if (buffer[i] == ' ') {
         // only check the first note in the input **kern token
         break;
      }
      i++;
   }
   return output;
}



//////////////////////////////
//
// Convert::base40IntervalToDiatonic -- convert a base40 interval
//    into a diatonic interval (excluding the chromatic alteration)
//

int Convert::base40IntervalToDiatonic(int base40interval) {
   int sign = 1;
   if (base40interval < 0) {
      sign = -1;
      base40interval = -base40interval;
   }
   int octave = base40interval / 40;
   base40interval = base40interval % 40;

   int diatonic = 0;
   switch (base40interval) {
      case  0: diatonic = 0; break;  // C
      case  1: diatonic = 0; break;  // C#
      case  2: diatonic = 0; break;  // C##

      case  3: diatonic = 1000; break;  // blank

      case  4: diatonic = 1; break;  // D--
      case  5: diatonic = 1; break;  // D-
      case  6: diatonic = 1; break;  // D
      case  7: diatonic = 1; break;  // D#
      case  8: diatonic = 1; break;  // D##

      case  9: diatonic = 1000; break;  // blank

      case 10: diatonic = 2; break;  // E--
      case 11: diatonic = 2; break;  // E-
      case 12: diatonic = 2; break;  // E
      case 13: diatonic = 2; break;  // E#
      case 14: diatonic = 2; break;  // E##

      case 15: diatonic = 3; break;  // F--
      case 16: diatonic = 3; break;  // F-
      case 17: diatonic = 3; break;  // F
      case 18: diatonic = 3; break;  // F#
      case 19: diatonic = 3; break;  // F##

      case 20: diatonic = 1000; break;  // blank

      case 21: diatonic = 4; break;  // G--
      case 22: diatonic = 4; break;  // G-
      case 23: diatonic = 4; break;  // G
      case 24: diatonic = 4; break;  // G#
      case 25: diatonic = 4; break;  // G##

      case 26: diatonic = 1000; break;  // blank

      case 27: diatonic = 5; break;  // A--
      case 28: diatonic = 5; break;  // A-
      case 29: diatonic = 5; break;  // A
      case 30: diatonic = 5; break;  // A#
      case 31: diatonic = 5; break;  // A##

      case 32: diatonic = 1000; break;  // blank

      case 33: diatonic = 6; break;  // B--
      case 34: diatonic = 6; break;  // B-
      case 35: diatonic = 6; break;  // B
      case 36: diatonic = 6; break;  // B#
      case 37: diatonic = 6; break;  // B##

      case 38: diatonic = 0; break;  // C--
      case 39: diatonic = 0; break;  // C-
   }

   return sign * (diatonic + octave * 7);
}



//////////////////////////////
//
// Convert::kernToDiatonicPitchClass -- converts to lower case a-g, 
//      plus r for rest.
//

int Convert::kernToDiatonicPitchClass(const char* buffer) {
   int i = 0;
   int character;
   while (buffer[i] != '\0') {
      character = tolower(buffer[i]);
      if (character == 'a') {
         return 'a';
      } else if (character == 'b') {
         return 'b';
      } else if (character == 'c') {
         return 'c';
      } else if (character == 'd') {
         return 'd';
      } else if (character == 'e') {
         return 'e';
      } else if (character == 'f') {
         return 'f';
      } else if (character == 'g') {
         return 'g';
      } else if (character == 'r') {
         return 'r';
      }
      i++;
   }

   return 'x';  // no pitch or rest found in data.
}



//////////////////////////////
//
// Convert::kernToDiatonicPitchClassNumeric -- converts to c=0 .. b=6.
//      rest is something negative number.
//

int Convert::kernToDiatonicPitchClassNumeric(const char* buffer) {
   int charval =  Convert::kernToDiatonicPitchClass(buffer);

   switch (charval) {
      case 'a': return  5;
      case 'b': return  6;
      case 'c': return  0;
      case 'd': return  1;
      case 'e': return  2;
      case 'f': return  3;
      case 'g': return  4;
   }

   return -1;
}



//////////////////////////////
//
// Convert::kernTimeSignatureBottom -- returns the rhythm value
//	in the bottom part of a time signature
//

double Convert::kernTimeSignatureBottomToDuration (const char* aKernString) {
   const char* slash = strchr(aKernString, '/');
   if (slash == NULL) {
      cerr << "Error: poorly formed time signature: " << aKernString << endl;
      exit(1);
   } 

   return Convert::kernToDuration(&slash[1]);
}



//////////////////////////////
//
// Convert::kernTimeSignatureTop -- returns the number of beats
//	in the top part of a time signature
// fixed bug Wed May  5 23:56:19 PDT 2004
//

double Convert::kernTimeSignatureTop (const char* aKernString) {
   if (strchr(aKernString, '+') != NULL) {
      cerr << "Error: cannot handle time signature: " << aKernString
           << " yet." << endl;
      exit(1);
   } else {
      int len = strlen(aKernString);
      if (len > 2) {
         return (double)atoi(&aKernString[2]);  // used to be 1
      } else {
         return 0;
      }
   }
}



//////////////////////////////
//
// keyNumberToKern -- reverse of kernKeyToNumber.
//

const char* Convert::keyNumberToKern(int number) {
   switch (number) {
      case -7: return "*k[b-e-a-d-g-c-f-]";
      case -6: return "*k[b-e-a-d-g-c-]";
      case -5: return "*k[b-e-a-d-g-]";
      case -4: return "*k[b-e-a-d-]";
      case -3: return "*k[b-e-a-]";
      case -2: return "*k[b-e-]";
      case -1: return "*k[b-]";
      case  0: return "*k[]";
      case +1: return "*k[f#]";
      case +2: return "*k[f#c#]";
      case +3: return "*k[f#c#g#]";
      case +4: return "*k[f#c#g#d#]";
      case +5: return "*k[f#c#g#d#a#]";
      case +6: return "*k[f#c#g#d#a#e#]";
      case +7: return "*k[f#c#g#d#a#e#b#]";
      default: return "*k[]";
   }
}



//////////////////////////////
//
// kernKeyToNumber -- convert a kern key signature to an integer.
//      For example: *k[f#] == +1, *k[b-e-] == -2, *k[] == 0
//      Input string is expected to be in the form *k[] with the
//      accidentals inside the brackets with no spaces.
//

int Convert::kernKeyToNumber(const char* aKernString) {
   int count = 0;
   int i;
   int length = strlen(aKernString);
   int start = 0;
   int sign = 1;

   if ((strlen(aKernString) == 0)  || (strstr(aKernString, "[]") != NULL)) {
      return 0;
   }

   for (i=0; i<length; i++) {
      if (start) {
         if (aKernString[i] == ']') {
            break;
         } else if (aKernString[i] == '-') {
            sign = -1;
         }
         count++;
      }
      else if (aKernString[i] == '[') {
         start = 1;
      }
   }

   return sign * count/2;
}



//////////////////////////////
//
// Convert::musePitchToKernPitch --
//

char* Convert::musePitchToKernPitch(char* kernOutput, const char* museInput) {
   base40ToKern(kernOutput, museToBase40(museInput));
   return kernOutput;
}



//////////////////////////////
//
// Convert::museClefToKernClef --
//

char* Convert::museClefToKernClef(char* kernOutput, int museInput) {
   switch (museInput) {
      case 0:   strcpy(kernOutput, "GX");       break;
      case 1:   strcpy(kernOutput, "G5");       break;
      case 2:   strcpy(kernOutput, "G4");       break;
      case 3:   strcpy(kernOutput, "G3");       break;
      case 4:   strcpy(kernOutput, "G2");       break;
      case 5:   strcpy(kernOutput, "G1");       break;
      
      case 10:  strcpy(kernOutput, "CX");       break;
      case 11:  strcpy(kernOutput, "C5");       break;
      case 12:  strcpy(kernOutput, "C4");       break;
      case 13:  strcpy(kernOutput, "C3");       break;
      case 14:  strcpy(kernOutput, "C2");       break;
      case 15:  strcpy(kernOutput, "C1");       break;
      
      case 20:  strcpy(kernOutput, "FX");       break;
      case 21:  strcpy(kernOutput, "F5");       break;
      case 22:  strcpy(kernOutput, "F4");       break;
      case 23:  strcpy(kernOutput, "F3");       break;
      case 24:  strcpy(kernOutput, "F2");       break;
      case 25:  strcpy(kernOutput, "F1");       break;
      
      case 30:  strcpy(kernOutput, "GvX");       break;
      case 31:  strcpy(kernOutput, "Gv5");       break;
      case 32:  strcpy(kernOutput, "Gv4");       break;
      case 33:  strcpy(kernOutput, "Gv3");       break;
      case 34:  strcpy(kernOutput, "Gv2");       break;
      case 35:  strcpy(kernOutput, "Gv1");       break;
      
      case 40:  strcpy(kernOutput, "CvX");       break;
      case 41:  strcpy(kernOutput, "Cv5");       break;
      case 42:  strcpy(kernOutput, "Cv4");       break;
      case 43:  strcpy(kernOutput, "Cv3");       break;
      case 44:  strcpy(kernOutput, "Cv2");       break;
      case 45:  strcpy(kernOutput, "Cv1");       break;
      
      case 50:  strcpy(kernOutput, "FvX");       break;
      case 51:  strcpy(kernOutput, "Fv5");       break;
      case 52:  strcpy(kernOutput, "Fv4");       break;
      case 53:  strcpy(kernOutput, "Fv3");       break;
      case 54:  strcpy(kernOutput, "Fv2");       break;
      case 55:  strcpy(kernOutput, "Fv1");       break;
      
      case 60:  strcpy(kernOutput, "G^X");       break;
      case 61:  strcpy(kernOutput, "G^5");       break;
      case 62:  strcpy(kernOutput, "G^4");       break;
      case 63:  strcpy(kernOutput, "G^3");       break;
      case 64:  strcpy(kernOutput, "G^2");       break;
      case 65:  strcpy(kernOutput, "G^1");       break;
      
      case 70:  strcpy(kernOutput, "C^X");       break;
      case 71:  strcpy(kernOutput, "C^5");       break;
      case 72:  strcpy(kernOutput, "C^4");       break;
      case 73:  strcpy(kernOutput, "C^3");       break;
      case 74:  strcpy(kernOutput, "C^2");       break;
      case 75:  strcpy(kernOutput, "C^1");       break;
      
      case 80:  strcpy(kernOutput, "F^X");       break;
      case 81:  strcpy(kernOutput, "F^5");       break;
      case 82:  strcpy(kernOutput, "F^4");       break;
      case 83:  strcpy(kernOutput, "F^3");       break;
      case 84:  strcpy(kernOutput, "F^2");       break;
      case 85:  strcpy(kernOutput, "F^1");       break;
      default:  strcpy(kernOutput, "X");         
   }

   return kernOutput;
}



///////////////////////////////////////////////////////////////////////////
//
// conversions dealing with **qual data
//

//////////////////////////////
//
// Convert::chordQualityStringToValue --
//

ChordQuality Convert::chordQualityStringToValue(const char* aString) {

   ChordQuality output;
   char* temp;
   char* token;
   temp = new char[strlen(aString)+1];
   strcpy(temp, aString);
   token = strtok(temp, ":");
   for (int i=0; i<3; i++) {
      // determine what the current token is
      if (token == NULL) {
         break;
      } else if (Convert::chordType.memberQ(token)) {
         output.setType(token);
      } else if (Convert::chordInversion.memberQ(token)) {
         output.setInversion(token);
      } else if (Convert::kernPitchClass.memberQ(token)) {
         output.setRoot(token);
      } else {
         cerr << "Error: unknown chord value: " << token << endl;
         exit(1);
      }
   }
   delete [] temp;
   return output;
}



//////////////////////////////
//
// Convert::chordQualityToBaseNote --
//

int Convert::chordQualityToBaseNote(const ChordQuality& aQuality) {

   SigCollection<int> notes;
   chordQualityToNoteSet(notes, aQuality);
   return notes[aQuality.getInversion()] % 40;
}



//////////////////////////////
//
// Convert::chordQualityToNoteSet --
//

void Convert::chordQualityToNoteSet(SigCollection<int>& noteSet, 
     const ChordQuality& aQuality) {

   SigCollection<int>& output = noteSet;
   output.setSize(0);
   output.allowGrowth();
   output[0] = 0;
   
   switch (aQuality.getType()) {
      case E_chord_rest:     break;
      case E_chord_note:     break;
      case E_chord_incmin:   output[1] = 11; break;
      case E_chord_incmaj:   output[1] = 12; break;
      case E_chord_secsev:
      case E_chord_dim:      output[1] = 11; output[2] = 22; break;
      case E_chord_min:      output[1] = 11; output[2] = 23; break;
      case E_chord_neopol: 
      case E_chord_secdom:
      case E_chord_maj:      output[1] = 12; output[2] = 23; break;
      case E_chord_aug:      output[1] = 12; output[2] = 24; break;
      case E_chord_minminx5: output[1] = 11; output[2] = 34; break;
      case E_chord_domsevx5: output[1] = 12; output[2] = 34; break;
      
      case E_chord_secsevo:
      case E_chord_fullydim: output[1] = 11; output[2] = 22; output[3] = 33;
                             break;
      case E_chord_secsevc:
      case E_chord_halfdim:  output[1] = 11; output[2] = 22; output[3] = 34;
                             break;
      case E_chord_minmin:   output[1] = 11; output[2] = 23; output[3] = 34;
                             break;
      case E_chord_minmaj:   output[1] = 11; output[2] = 23; output[3] = 35;
                             break;
      case E_chord_secdomsev:
      case E_chord_domsev:   output[1] = 12; output[2] = 23; output[3] = 34;
                             break;
      case E_chord_majmaj:   output[1] = 12; output[2] = 23; output[3] = 35;
                             break;
      case E_chord_frensix:  output[1] = 12; output[2] = 18; output[3] = 30;
                             break;
      case E_chord_germsix:  output[1] = 12; output[2] = 23; output[3] = 30;
                             break;
      case E_chord_italsix:  output[1] = 12; output[2] = 30; break;
      default:
         cerr << "Error: unknown type of chord quality: " << aQuality.getType()
              << endl;
   }

   // add bass offset
   for (int i=0; i<output.getSize(); i++) {
      output[i] += aQuality.getRoot();
      output[i] = (output[i] + 40) % 40;
   }

}



//////////////////////////////
//
// Convert::chordQualityToInversion --
//

int Convert::chordQualityToInversion(const char* aQuality) {

   ChordQuality conversion = chordQualityStringToValue(aQuality);
   return conversion.getInversion();
}



//////////////////////////////
//
// Convert::chordQualityToRoot --
//

int Convert::chordQualityToRoot(const char* aQuality) {

   ChordQuality conversion = chordQualityStringToValue(aQuality);
   return conversion.getRoot();
}



//////////////////////////////
//
// Convert::chordQualityToType --
//

int Convert::chordQualityToType(const char* aQuality) {

   ChordQuality conversion = chordQualityStringToValue(aQuality);
   return conversion.getType();
}



//////////////////////////////
//
// Convert::noteSetToChordQuality --
//

void Convert::noteSetToChordQuality(ChordQuality& cq, 
      const SigCollection<int>& aSet) {

   ChordQuality& output = cq;
   SigCollection<int> newnotes(aSet.getSize());
   int i, k;

   // remove rests and find the lowest note
   int index = 0;
   for (i=0; i<aSet.getSize(); i++) {
      if (aSet[i] > E_root_rest) {
         newnotes[index] = aSet[i];
         index++;
      }
   }
   if (index == 0) {
      output.setType(E_chord_rest);
      output.setInversion(E_blank);
      output.setRoot(E_blank);
      return;
   } else {
      newnotes.setSize(index);
   }

   qsort(newnotes.getBase(), newnotes.getSize(), sizeof(int), intcompare);
   int basenote = BASE40_PITCHCLASS(newnotes[0]);


   // remove octave values
   for (i=0; i<newnotes.getSize(); i++) {
      newnotes[i] = BASE40_PITCHCLASS(newnotes[i]);
   }
   qsort(newnotes.getBase(), newnotes.getSize(), sizeof(int), intcompare);

   // remove redundant notes
   int uniqcount = newnotes.getSize();
   for (i=1; i<uniqcount; i++) {
      while (newnotes[i] == newnotes[i-1] && i < uniqcount) {
         for (k=i-1; k<uniqcount-1; k++) {
            newnotes[k] = newnotes[k+1];
         }
         uniqcount--;
      }
   }
   newnotes.setSize(uniqcount);

   int root = E_unknown;
   int chordFound = E_unknown;
   for (i=0; i<uniqcount; i++) {
      chordFound = checkChord(newnotes);
      if (chordFound != E_unknown) {
         root = newnotes[0];
         break;
      } else {
         rotatechord(newnotes);
      }
   }

   output.setType(chordFound);

   if (chordFound == E_unknown) {
      output.setInversion(E_unknown);
      output.setRoot(E_unknown);
      output.setPitchClasses(newnotes);
   } else {
      output.setInversion(calculateInversion(chordFound, basenote, root));
      output.setRoot(root % 40);
   }
}



///////////////////////////////////////////////////////////////////////////
//
// conversions dealing with base 40 system of notation
//


//////////////////////////////
//
// base40ToPerfViz --
//

char* Convert::base40ToPerfViz(char* output, int base40value) {
   int accidental = Convert::base40ToAccidental(base40value);
   int diatonic   = Convert::base40ToDiatonic(base40value);
   int octave     = base40value / 40;


   output[0] = '\0';

   strcat(output, "[");
   switch (diatonic) {
      case 0:  strcat(output, "c"); break;
      case 1:  strcat(output, "d"); break;
      case 2:  strcat(output, "e"); break;
      case 3:  strcat(output, "f"); break;
      case 4:  strcat(output, "g"); break;
      case 5:  strcat(output, "a"); break;
      case 6:  strcat(output, "b"); break;
      default: strcat(output, "X"); break;
   }
   strcat(output, ",");
   if (accidental == 0) {               // natural
      strcat(output, "n");
   } else if (accidental == 1) {        // sharp
      strcat(output, "#");
   } else if (accidental == -1) {       // flat
      strcat(output, "b");
   } else if (accidental == 2) {        // double sharp
      strcat(output, "##");
   } else if (accidental == -2) {       // double flat
      strcat(output, "bb");
   }
   strcat(output, "],");


   char obuf[2] = {0};
   if (octave <= 9  && octave >= 0) {
      obuf[0] = '0' + octave;
      strcat(output, obuf);
   } else {
      strcat(output, "4");
   }

   return output;
}



//////////////////////////////
//
// Convert::base40ToAccidental -- +1 = 1 sharp, +2 = double sharp, 0 = natural
//	-1 = 1 flat, -2 = double flat
//

int Convert::base40ToAccidental(int base40value) {
   if (base40value < 0) {
      return 0;
   }

   switch (base40value % 40) {
      case 0:	return -2;
      case 1:	return -1;
      case 2:	return  0;
      case 3:	return  1;
      case 4:	return  2;
      case 5:	return 1000;
      case 6:	return -2;
      case 7:	return -1;
      case 8:	return  0;
      case 9:	return  1;
      case 10:	return  2;
      case 11:	return 1000;
      case 12:	return -2;
      case 13:	return -1;
      case 14:	return  0;
      case 15:	return  1;
      case 16:	return  2;
      case 17:	return -2;
      case 18:	return -1;
      case 19:	return  0;
      case 20:	return  1;
      case 21:	return  2;
      case 22:	return 1000;
      case 23:	return -2;
      case 24:	return -1;
      case 25:	return  0;
      case 26:	return  1;
      case 27:	return  2;
      case 28:	return 1000;
      case 29:	return -2;
      case 30:	return -1;
      case 31:	return  0;
      case 32:	return  1;
      case 33:	return  2;
      case 34:	return 1000;
      case 35:	return -2;
      case 36:	return -1;
      case 37:	return  0;
      case 38:	return  1;
      case 39:	return  2;
   }

   return 0;
}



//////////////////////////////
//
// Convert::base40IntervalToLineOfFifths -- 0 => 0 (unison), 
//    Perfect Fifth => 1, Major second => 2 (two fifths up), etc.
//

int Convert::base40IntervalToLineOfFifths(int base40interval) {
   base40interval += 4000;
   base40interval = base40interval % 40;

   switch (base40interval) {
      case 0:    return   0;     // C
      case 1:    return   7;     // C#
      case 2:    return  14;     // C##
      case 3:    return 100;     // X
      case 4:    return -12;     // D--
      case 5:    return  -5;     // D-
      case 6:    return   2;     // D
      case 7:    return   9;     // D#
      case 8:    return  16;     // D##
      case 9:    return 100;     // X
      case 10:   return -10;     // E--
      case 11:   return  -3;     // E-
      case 12:   return   4;     // E
      case 13:   return  11;     // E#
      case 14:   return  18;     // E##
      case 15:   return -15;     // F--
      case 16:   return  -8;     // F-
      case 17:   return  -1;     // F
      case 18:   return   6;     // F#
      case 19:   return  13;     // F##
      case 20:   return 100;     // X
      case 21:   return -13;     // G--
      case 22:   return  -6;     // G-
      case 23:   return   1;     // G
      case 24:   return   8;     // G#
      case 25:   return  15;     // G##
      case 26:   return 100;     // X
      case 27:   return -11;     // A--
      case 28:   return  -4;     // A-
      case 29:   return   3;     // A
      case 30:   return  10;     // A#
      case 31:   return  17;     // A##
      case 32:   return 100;     // X
      case 33:   return  -9;     // B--
      case 34:   return  -2;     // B-
      case 35:   return   5;     // B
      case 36:   return  12;     // B#
      case 37:   return  19;     // B##
      case 38:   return -14;     // C--
      case 39:   return  -7;     // C-
      default:   return 100;     // X
   }

   return 100;
}



//////////////////////////////
//
// Convert::base40ToScoreVPos -- get the position of a note on a staff
//     according to the given clef.  The input clef is the SCORE
//     clef number:
//
//     0 = treble
//     1 = bass
//     2 = alto
//
//     for other clefs, add/subtract according to the give default
//     position for G, F, or C clefs.
//
//     The zero position is the space below the first ledger line
//     on the staff.
//

int Convert::base40ToScoreVPos(int pitch, int clef) {
   int octave = pitch / 40;
   int diatonic = Convert::base40ToDiatonic(pitch);
   // make sure diatonic is diatonic pitch class
   diatonic = diatonic % 7;
   switch (clef) {
      case 1:                              // bass clef
         return (octave-2)*7+diatonic-1;
         break;
      case 2:                              // alto clef
         return (octave-3)*7+diatonic;
         break;
      case 0:                              // treble clef
      default:
         return (octave-4)*7+diatonic+1;
         break;
   }
}



//////////////////////////////
//
// base40ToDiatonic -- find the diatonic pitch of the
//   given base 40 note.  0 = C, 1 = D, 2 = E, 3 = F.
//   To get the diatonic pitch class, mod by 7: (% 7).
//

int Convert::base40ToDiatonic(int pitch) {
   int chroma = pitch % 40;
   int octaveoffset = (pitch / 40) * 7;
   if (pitch < 0) { 
      return -1;   // rest;
   }
   switch (chroma) {
//   case 38: case 39: case 0: case 1: case 2:
//      return 0 + octaveoffset;
//   case 4: case 5: case 6: case 7: case 8:
//      return 1 + octaveoffset;
//   case 10: case 11: case 12: case 13: case 14:
//      return 2 + octaveoffset;
//   case 15: case 16: case 17: case 18: case 19:
//      return 3 + octaveoffset;
//   case 21: case 22: case 23: case 24: case 25:
//      return 4 + octaveoffset;
//   case 27: case 28: case 29: case 30: case 31:
//      return 5 + octaveoffset;
//   case 33: case 34: case 35: case 36: case 37:
//      return 6 + octaveoffset;

   case 0: case 1: case 2: case 3: case 4:
     return 0 + octaveoffset;
   case 6: case 7: case 8: case 9: case 10:
     return 1 + octaveoffset;
   case 12: case 13: case 14: case 15: case 16:
     return 2 + octaveoffset;
   case 17: case 18: case 19: case 20: case 21:
     return 3 + octaveoffset;
   case 23: case 24: case 25: case 26: case 27:
     return 4 + octaveoffset;
   case 29: case 30: case 31: case 32: case 33:
     return 5 + octaveoffset;
   case 35: case 36: case 37: case 38: case 39:
     return 6 + octaveoffset;
   }

   return -1;
}



//////////////////////////////
//
// Convert::kernClefToBaseline -- returns the diatonic pitch
//    of the bottom line on the staff.
//

int Convert::kernClefToBaseline(const char* buffer) {
   const char* cptr = "";
   if (strncmp(buffer, "*clef", 5) == 0) {
      cptr = &(buffer[5]);
   } else if (strncmp(buffer, "clef", 4) == 0) {
      cptr = &(buffer[4]);
   } else {
      cerr << "Error in Convert::kernClefToBaseline: " << buffer << endl;
      exit(1);
   }

   if (strcmp(cptr, "G2") == 0) {               // treble clef
      return Convert::kernToDiatonicPitch("e");
   } else if (strcmp(cptr, "F4") == 0) {        // bass clef
      return Convert::kernToDiatonicPitch("GG");
   } else if (strcmp(cptr, "C3") == 0) {        // alto clef
      return Convert::kernToDiatonicPitch("F");
   } else if (strcmp(cptr, "C4") == 0) {        // tenor clef
      return Convert::kernToDiatonicPitch("D"); 
   } else if (strcmp(cptr, "Gv2") == 0) {       // vocal tenor clef
      return Convert::kernToDiatonicPitch("E"); 

   // rest of C clef possibilities:
   } else if (strcmp(cptr, "C1") == 0) {        // soprano clef
      return Convert::kernToDiatonicPitch("c"); 
   } else if (strcmp(cptr, "C2") == 0) {        // mezzo-soprano clef
      return Convert::kernToDiatonicPitch("A"); 
   } else if (strcmp(cptr, "C5") == 0) {        // baritone clef
      return Convert::kernToDiatonicPitch("BB"); 

   // rest of G clef possibilities:
   } else if (strcmp(cptr, "G1") == 0) {        // French-violin clef
      return Convert::kernToDiatonicPitch("g"); 
   } else if (strcmp(cptr, "G3") == 0) {      
      return Convert::kernToDiatonicPitch("c"); 
   } else if (strcmp(cptr, "G4") == 0) {      
      return Convert::kernToDiatonicPitch("A"); 
   } else if (strcmp(cptr, "G5") == 0) {      
      return Convert::kernToDiatonicPitch("F"); 

   // rest of F clef possibilities:
   } else if (strcmp(cptr, "F1") == 0) {      
      return Convert::kernToDiatonicPitch("F"); 
   } else if (strcmp(cptr, "F2") == 0) {      
      return Convert::kernToDiatonicPitch("D"); 
   } else if (strcmp(cptr, "F3") == 0) {      
      return Convert::kernToDiatonicPitch("BB"); 
   } else if (strcmp(cptr, "F5") == 0) {      
      return Convert::kernToDiatonicPitch("EE"); 

   // rest of G clef down an octave possibilities:
   } else if (strcmp(cptr, "Gv1") == 0) {      
      return Convert::kernToDiatonicPitch("G"); 
   } else if (strcmp(cptr, "Gv3") == 0) {      
      return Convert::kernToDiatonicPitch("C"); 
   } else if (strcmp(cptr, "Gv4") == 0) {      
      return Convert::kernToDiatonicPitch("AA"); 
   } else if (strcmp(cptr, "Gv5") == 0) {      
      return Convert::kernToDiatonicPitch("FF"); 

   // F clef down an octave possibilities:
   } else if (strcmp(cptr, "Fv1") == 0) {      
      return Convert::kernToDiatonicPitch("FF"); 
   } else if (strcmp(cptr, "Fv2") == 0) {      
      return Convert::kernToDiatonicPitch("DD"); 
   } else if (strcmp(cptr, "Fv3") == 0) {      
      return Convert::kernToDiatonicPitch("BBB"); 
   } else if (strcmp(cptr, "Fv4") == 0) {      
      return Convert::kernToDiatonicPitch("GGG"); 
   } else if (strcmp(cptr, "Fv5") == 0) {      
      return Convert::kernToDiatonicPitch("EEE"); 

   // C clef down an octave possibilities:
   } else if (strcmp(cptr, "Cv1") == 0) {     
      return Convert::kernToDiatonicPitch("C"); 
   } else if (strcmp(cptr, "Cv2") == 0) {    
      return Convert::kernToDiatonicPitch("AA"); 
   } else if (strcmp(cptr, "Cv3") == 0) {    
      return Convert::kernToDiatonicPitch("FF"); 
   } else if (strcmp(cptr, "Cv4") == 0) {    
      return Convert::kernToDiatonicPitch("DD"); 
   } else if (strcmp(cptr, "Cv5") == 0) {   
      return Convert::kernToDiatonicPitch("BBB"); 

   // G clef up an octave possibilities:
   } else if (strcmp(cptr, "G^1") == 0) {      
      return Convert::kernToDiatonicPitch("gg"); 
   } else if (strcmp(cptr, "G^2") == 0) {      
      return Convert::kernToDiatonicPitch("ee"); 
   } else if (strcmp(cptr, "G^3") == 0) {      
      return Convert::kernToDiatonicPitch("cc"); 
   } else if (strcmp(cptr, "G^4") == 0) {      
      return Convert::kernToDiatonicPitch("a"); 
   } else if (strcmp(cptr, "G^5") == 0) {      
      return Convert::kernToDiatonicPitch("f"); 

   // F clef up an octave possibilities:
   } else if (strcmp(cptr, "F^1") == 0) {      
      return Convert::kernToDiatonicPitch("f"); 
   } else if (strcmp(cptr, "F^2") == 0) {      
      return Convert::kernToDiatonicPitch("d"); 
   } else if (strcmp(cptr, "F^3") == 0) {      
      return Convert::kernToDiatonicPitch("B"); 
   } else if (strcmp(cptr, "F^4") == 0) {      
      return Convert::kernToDiatonicPitch("G"); 
   } else if (strcmp(cptr, "F^5") == 0) {      
      return Convert::kernToDiatonicPitch("E"); 

   // C clef up an octave possibilities:
   } else if (strcmp(cptr, "C^1") == 0) {     
      return Convert::kernToDiatonicPitch("cc"); 
   } else if (strcmp(cptr, "C^2") == 0) {    
      return Convert::kernToDiatonicPitch("a"); 
   } else if (strcmp(cptr, "C^3") == 0) {    
      return Convert::kernToDiatonicPitch("f"); 
   } else if (strcmp(cptr, "C^4") == 0) {    
      return Convert::kernToDiatonicPitch("d"); 
   } else if (strcmp(cptr, "C^5") == 0) {   
      return Convert::kernToDiatonicPitch("B"); 

   // there are also two octave down (*clefGvv2) and two octaves up (*clefG^^2)
   } else {
      // but just use treble clef if don't know what the clef it by this point
      return Convert::kernToDiatonicPitch("e"); 
   }
}



//////////////////////////////
//
// Convert::base40ToMuse -- input is a base40 value which gets converted
// to a diatonic pitch name in the MuseData style.
//

char*  Convert::base40ToMuse(int base40, char* buffer) {
   buffer[0] = '\0';

   char diatonic[2] = {0};
   switch (Convert::base40ToDiatonic(base40) % 7) {
      case 0:  diatonic[0] = 'C'; break;
      case 1:  diatonic[0] = 'D'; break;
      case 2:  diatonic[0] = 'E'; break;
      case 3:  diatonic[0] = 'F'; break;
      case 4:  diatonic[0] = 'G'; break;
      case 5:  diatonic[0] = 'A'; break;
      case 6:  diatonic[0] = 'B'; break;
      default: diatonic[0] = 'X'; 
   }

   char octave[2] = {0};
   octave[0]   = '0' + base40 / 40; 

   char accidental[4] = {0};
   int acc = Convert::base40ToAccidental(base40);
   switch (acc) {
      case -2:   strcpy(accidental, "ff"); break;
      case -1:   strcpy(accidental, "f");  break;
      case +1:   strcpy(accidental, "#");  break;
      case +2:   strcpy(accidental, "##"); break;
   }

   strcpy(buffer, diatonic);
   strcat(buffer, accidental);
   strcat(buffer, octave);

   return buffer;
}



//////////////////////////////
//
// Convert::base40ToKern --
//

char* Convert::base40ToKern(char* output, int aPitch) {
   int octave = aPitch / 40;
   if (octave > 12 || octave < -1) {
      cerr << "Error: unreasonable octave value: " << octave << endl;
      exit(1);
   }
   int chroma = aPitch % 40;

   strcpy(output, kernPitchClass.getName(chroma));
   if (octave >= 4) {
      output[0] = tolower(output[0]);
   } else {
      output[0] = toupper(output[0]);
   }
   int repeat = 0;
   switch (octave) {
      case 4:  repeat = 0; break;
      case 5:  repeat = 1; break;
      case 6:  repeat = 2; break;
      case 7:  repeat = 3; break;
      case 8:  repeat = 4; break;
      case 9:  repeat = 5; break;
      case 3:  repeat = 0; break;
      case 2:  repeat = 1; break;
      case 1:  repeat = 2; break;
      case 0:  repeat = 3; break;
      case -1: repeat = 4; break;
      default:
         cerr << "Error: unknown octave value: " << octave << endl;
         cerr << "for base-40 pitch: " << aPitch << endl;
         exit(1);
   }
   if (repeat == 0) {
      return output;
   }

   int length = strlen(output);
   output[length + repeat] = '\0';
   int i; 
   for (i=length-1; i>=0; i--) {
      output[i+repeat] = output[i];
   }
   for (i=0; i<repeat; i++) {
      output[i+1] = output[0];
   }

   return output;
}



//////////////////////////////
//
// Convert::base40ToIntervalAbbr --
//

char* Convert::base40ToIntervalAbbr(char* output, int base40value) {
   base40value += 400;
   if (base40value < 0) {
      strcpy(output, "r");
      return output;
   }

   output[0] = '\0';
   switch (base40value % 40) {
      case  0: strcpy(output, "p1"   ); break;  // C
      case  1: strcpy(output, "a1"   ); break;  // C#
      case  2: strcpy(output, "aa1"  ); break;  // C##
      case  3: strcpy(output, "X1"   ); break;  // X
      case  4: strcpy(output, "d2"   ); break;  // D--
      case  5: strcpy(output, "m2"   ); break;  // D-
      case  6: strcpy(output, "M2"   ); break;  // D
      case  7: strcpy(output, "a2"   ); break;  // D#
      case  8: strcpy(output, "aa2"  ); break;  // D##
      case  9: strcpy(output, "X2"   ); break;  // X
      case 10: strcpy(output, "d3"   ); break;  // E--
      case 11: strcpy(output, "m3"   ); break;  // E-
      case 12: strcpy(output, "M3"   ); break;  // E
      case 13: strcpy(output, "a3"   ); break;  // E#
      case 14: strcpy(output, "aa3"  ); break;  // E##
      case 15: strcpy(output, "dd4"  ); break;  // F--
      case 16: strcpy(output, "d4"   ); break;  // F-
      case 17: strcpy(output, "p4"   ); break;  // F
      case 18: strcpy(output, "a4"   ); break;  // F#
      case 19: strcpy(output, "aa4"  ); break;  // F##
      case 20: strcpy(output, "X3"   ); break;  // X
      case 21: strcpy(output, "dd5"  ); break;  // G--
      case 22: strcpy(output, "d5"   ); break;  // G-
      case 23: strcpy(output, "p5"   ); break;  // G
      case 24: strcpy(output, "a5"   ); break;  // G#
      case 25: strcpy(output, "aa5"  ); break;  // G##
      case 26: strcpy(output, "X4"   ); break;  // X
      case 27: strcpy(output, "d6"   ); break;  // A--
      case 28: strcpy(output, "m6"   ); break;  // A-
      case 29: strcpy(output, "M6"   ); break;  // A
      case 30: strcpy(output, "a6"   ); break;  // A#
      case 31: strcpy(output, "aa6"  ); break;  // A##
      case 32: strcpy(output, "X5"   ); break;  // X
      case 33: strcpy(output, "d7"   ); break;  // B--
      case 34: strcpy(output, "m7"   ); break;  // B-
      case 35: strcpy(output, "M7"   ); break;  // B
      case 36: strcpy(output, "a7"   ); break;  // B#
      case 37: strcpy(output, "aa7"  ); break;  // B##
      case 38: strcpy(output, "dd1"  ); break;  // C--
      case 39: strcpy(output, "d1"   ); break;  // C-
   }

   return output;
}



//////////////////////////////
//
// Convert::base40ToIntervalAbbr2 -- same as above but
//   d = diminished, D = doubly diminished, a = augmented, A = doubly augmented
//

char* Convert::base40ToIntervalAbbr2(char* output, int base40value) {
   base40value += 400;
   if (base40value < 0) {
      strcpy(output, "r");
      return output;
   }

   output[0] = '\0';
   switch (base40value % 40) {
      case  0: strcpy(output, "p1"   ); break;  // C
      case  1: strcpy(output, "a1"   ); break;  // C#
      case  2: strcpy(output, "A1"   ); break;  // C##
      case  3: strcpy(output, "X1"   ); break;  // X
      case  4: strcpy(output, "d2"   ); break;  // D--
      case  5: strcpy(output, "m2"   ); break;  // D-
      case  6: strcpy(output, "M2"   ); break;  // D
      case  7: strcpy(output, "a2"   ); break;  // D#
      case  8: strcpy(output, "A2"   ); break;  // D##
      case  9: strcpy(output, "X2"   ); break;  // X
      case 10: strcpy(output, "d3"   ); break;  // E--
      case 11: strcpy(output, "m3"   ); break;  // E-
      case 12: strcpy(output, "M3"   ); break;  // E
      case 13: strcpy(output, "a3"   ); break;  // E#
      case 14: strcpy(output, "A3"   ); break;  // E##
      case 15: strcpy(output, "D4"   ); break;  // F--
      case 16: strcpy(output, "d4"   ); break;  // F-
      case 17: strcpy(output, "p4"   ); break;  // F
      case 18: strcpy(output, "a4"   ); break;  // F#
      case 19: strcpy(output, "A4"   ); break;  // F##
      case 20: strcpy(output, "X3"   ); break;  // X
      case 21: strcpy(output, "D5"   ); break;  // G--
      case 22: strcpy(output, "d5"   ); break;  // G-
      case 23: strcpy(output, "p5"   ); break;  // G
      case 24: strcpy(output, "a5"   ); break;  // G#
      case 25: strcpy(output, "A5"   ); break;  // G##
      case 26: strcpy(output, "X4"   ); break;  // X
      case 27: strcpy(output, "d6"   ); break;  // A--
      case 28: strcpy(output, "m6"   ); break;  // A-
      case 29: strcpy(output, "M6"   ); break;  // A
      case 30: strcpy(output, "a6"   ); break;  // A#
      case 31: strcpy(output, "A6"   ); break;  // A##
      case 32: strcpy(output, "X5"   ); break;  // X
      case 33: strcpy(output, "d7"   ); break;  // B--
      case 34: strcpy(output, "m7"   ); break;  // B-
      case 35: strcpy(output, "M7"   ); break;  // B
      case 36: strcpy(output, "a7"   ); break;  // B#
      case 37: strcpy(output, "A7"   ); break;  // B##
      case 38: strcpy(output, "D1"   ); break;  // C--
      case 39: strcpy(output, "d1"   ); break;  // C-
   }

   return output;
}



//////////////////////////////
//
// Convert::base40ToKernTranspose -- returns the transpose
//	string value (c.f. p221 of Humdrum Reference (1994)
//

char* Convert::base40ToKernTranspose(char* output, int transpose, 
      int keysignature) {
   int keyacc;
   int keydia;
   int oldbase;
   int newacc;
   int newdia;

   switch (keysignature) {
      case  0:  keyacc = 0;   keydia = 0;  oldbase = 0;  break;  // C 
      case  1:  keyacc = 0;   keydia = 4;  oldbase = 23; break;  // G 
      case  2:  keyacc = 0;   keydia = 1;  oldbase = 6;  break;  // D 
      case  3:  keyacc = 0;   keydia = 5;  oldbase = 29; break;  // A 
      case  4:  keyacc = 0;   keydia = 2;  oldbase = 12; break;  // E 
      case  5:  keyacc = 0;   keydia = 6;  oldbase = 35; break;  // B 
      case  6:  keyacc = +1;  keydia = 3;  oldbase = 18; break;  // F#
      case  7:  keyacc = +1;  keydia = 0;  oldbase = 1;  break;  // C#
      case -1:  keyacc = 0;   keydia = 3;  oldbase = 17; break;  // F 
      case -2:  keyacc = -1;  keydia = 6;  oldbase = 34; break;  // Bb
      case -3:  keyacc = -1;  keydia = 2;  oldbase = 11; break;  // Eb
      case -4:  keyacc = -1;  keydia = 5;  oldbase = 28; break;  // Ab
      case -5:  keyacc = -1;  keydia = 1;  oldbase = 5;  break;  // Db
      case -6:  keyacc = -1;  keydia = 4;  oldbase = 22; break;  // Gb
      case -7:  keyacc = -1;  keydia = 0;  oldbase = -1; break;  // Cb
      default:  keyacc = 0;   keydia = 0;  oldbase = 0;  break;  
   }
  
   int newbase = (oldbase + transpose + 400) % 40;

   switch (newbase) {
      case  0:  newacc = 0;   newdia = 0;  break;  // C 
      case 23:  newacc = 0;   newdia = 4;  break;  // G 
      case  6:  newacc = 0;   newdia = 1;  break;  // D 
      case 29:  newacc = 0;   newdia = 5;  break;  // A 
      case 12:  newacc = 0;   newdia = 2;  break;  // E 
      case 35:  newacc = 0;   newdia = 6;  break;  // B 
      case 18:  newacc = +1;  newdia = 3;  break;  // F#
      case  1:  newacc = +1;  newdia = 0;  break;  // C#
      case 17:  newacc = 0;   newdia = 3;  break;  // F 
      case 34:  newacc = -1;  newdia = 6;  break;  // Bb
      case 11:  newacc = -1;  newdia = 2;  break;  // Eb
      case 28:  newacc = -1;  newdia = 5;  break;  // Ab
      case  5:  newacc = -1;  newdia = 1;  break;  // Db
      case 22:  newacc = -1;  newdia = 4;  break;  // Gb
      case -1:  newacc = -1;  newdia = 0;  break;  // Cb
      default:  newacc = 0;   newdia = 0;  break;  
   }

   int chrom = newacc - keyacc;
   int diaton = newdia - keydia;
   if (diaton < 0) {
      diaton += 7;
   }
   if (transpose < 0) {
      diaton -= 7;
//      diaton -= 7 * (abs(transpose) % 40);
   } else {
//      diaton += 7 * (abs(transpose) % 40);
   }
    
   SSTREAM tempoutput;
   tempoutput << "d" << diaton << "c" << chrom << ends;
   strcpy(output, tempoutput.CSTRING);
    
   return output; 
}


//////////////////////////////
//
// Convert::base40ToMidiNoteNumber --
//

int Convert::base40ToMidiNoteNumber(int base40value) {
   if (base40value < 0) {
      return -100;
   }

   // plus one needed for octave value since middle C is 60 
   // and 60 / 12 = 5 rather than 4.
   int octave = base40value / 40 + 1;
   int pc = -1;
   switch (base40value % 40) {
      case  0:   pc = -2;   break;    // C--
      case  1:   pc = -1;   break;    // C-
      case  2:   pc =  0;   break;    // C
      case  3:   pc =  1;   break;    // C#
      case  4:   pc =  2;   break;    // C##
      case  5:   pc = -100;   break;    // X
      case  6:   pc =  0;   break;    // D--
      case  7:   pc =  1;   break;    // D-
      case  8:   pc =  2;   break;    // D
      case  9:   pc =  3;   break;    // D#
      case 10:   pc =  4;   break;    // D##
      case 11:   pc = -100;   break;    // X
      case 12:   pc =  2;   break;    // E--
      case 13:   pc =  3;   break;    // E-
      case 14:   pc =  4;   break;    // E
      case 15:   pc =  5;   break;    // E#
      case 16:   pc =  6;   break;    // E##
      case 17:   pc =  3;   break;    // F--
      case 18:   pc =  4;   break;    // F-
      case 19:   pc =  5;   break;    // F
      case 20:   pc =  6;   break;    // F#
      case 21:   pc =  7;   break;    // F##
      case 22:   pc = -100;   break;    // X
      case 23:   pc =  5;   break;    // G--
      case 24:   pc =  6;   break;    // G-
      case 25:   pc =  7;   break;    // G
      case 26:   pc =  8;   break;    // G#
      case 27:   pc =  9;   break;    // G##
      case 28:   pc = -100;   break;    // X
      case 29:   pc =  7;   break;    // A--
      case 30:   pc =  8;   break;    // A-
      case 31:   pc =  9;   break;    // A
      case 32:   pc = 10;   break;    // A#
      case 33:   pc = 11;   break;    // A##
      case 34:   pc = -100;   break;    // X
      case 35:   pc =  9;   break;    // B--
      case 36:   pc = 10;   break;    // B-
      case 37:   pc = 11;   break;    // B
      case 38:   pc = 12;   break;    // B#
      case 39:   pc = 13;   break;    // B##
   }

   if (pc <= -100) {
      return -100;
   }

   int output = pc + octave * 12;
   if (output < 0) {
      output += 12;
   }
   if (output < 0) {
      return -100;
   }
   return output;
}



//////////////////////////////
//
// Convert::kernToBase40 -- extracts the pitch from a kern data field.
//      returns -1 if a null token.
//

int Convert::kernToBase40(const char* kernfield) {

   if (kernfield[0] == '.') {
      return -1;
   }

   int note = E_unknown;
   int i = 0;
   while (kernfield[i] != '\0') {
      if (isalpha(kernfield[i])) {
         switch (toupper(kernfield[i])) {
            case 'A': case 'B': case 'C': case 'D':
            case 'E': case 'F': case 'G':
               note = kernNoteToBase40(&kernfield[i]);
               goto finishup;
               break;
            case 'R':
               return E_base40_rest;
               break;
         }
      }
      i++;
   }

finishup:

   return note;
}



//////////////////////////////
//
// Convert::kernToBase40Class -- extracts the pitch from a kern data field.
//      Pitch class of the note.  Returns -1 if a null token
//

int Convert::kernToBase40Class(const char* kernfield) {

   int absolute = kernToBase40(kernfield);
   if (absolute >= 0) {
      return absolute % 40;
   } else {
      return absolute;
   }
}



//////////////////////////////
//
// Convert::kernNoteToBase40 -- returns the absolute base 40 pitch number of
//      a kern pitch.  Returns -1 on error or null token.
//

int Convert::kernNoteToBase40(const char* name) {

   int output;

   if (name[0] == '.') {
      return -1;
   }

   switch (name[0]) {
      case 'a': output = 4 * 40 + E_root_a;     break;
      case 'b': output = 4 * 40 + E_root_b;     break;
      case 'c': output = 4 * 40 + E_root_c;     break;
      case 'd': output = 4 * 40 + E_root_d;     break;
      case 'e': output = 4 * 40 + E_root_e;     break;
      case 'f': output = 4 * 40 + E_root_f;     break;
      case 'g': output = 4 * 40 + E_root_g;     break;
      case 'A': output = 3 * 40 + E_root_a;     break;
      case 'B': output = 3 * 40 + E_root_b;     break;
      case 'C': output = 3 * 40 + E_root_c;     break;
      case 'D': output = 3 * 40 + E_root_d;     break;
      case 'E': output = 3 * 40 + E_root_e;     break;
      case 'F': output = 3 * 40 + E_root_f;     break;
      case 'G': output = 3 * 40 + E_root_g;     break;
      case 'R': return E_root_rest;             break;
      default:  return -1;
   }

   int octave=0;
   while (name[octave] != '\0' && name[octave] == name[0]) {
      octave++;
   }
   if (islower(name[0])) {
      output += (octave - 1) * 40;
   } else {
      output -= (octave - 1) * 40;
   }

   // check for first accidental sign
   int accidental = octave;
   if (name[accidental] != '\0' && name[accidental] == '-') {
      output--;
   } else if (name[accidental] != '\0' && name[accidental] == '#') {
      output++;
   }

   // chek for second accidental sign
   if (name[accidental] == '\0') {
      ; // nothing
   } else { // check for double sharp or double flat
      accidental++;
      if (name[accidental] != '\0' && name[accidental] == '-') {
         output--;
      } else if (name[accidental] != '\0' && name[accidental] == '#') {
         output++;
      }
   }

   if (output >= 0) {
      return output;
   } else {
      cerr << "Error: pitch \"" << name << "\" is too low." << endl;
      exit(1);
   }
}



//////////////////////////////
//
// Convert::transToBase40 -- convert the Humdrum Toolkit program
//     trans's binomial notation for intervals into base-40.
//  The input can be in three formats:
//     d1c2      == no prepended text on information
//     *Trd1c2   == Transposition interpretation marker prefixed
//     *ITrd1c2  == Instrumental transposition marker prefixed
//

int Convert::transToBase40(const char* buffer) {
   int dval = 0;
   int cval = 0;
   if (sscanf(buffer, "d%dc%d", &dval, &cval) != 2) {
      if (sscanf(buffer, "*Trd%dc%d", &dval, &cval) != 2) {
         if (sscanf(buffer, "*ITrd%dc%d", &dval, &cval) != 2) {
            // cerr << "Cannot find correct information" << endl;
            return 0;
         }
      }
   }

   int dsign = 1;
   int csign = 1;
   if (dval < 0) {
      dsign = -1;
   } 
   if (cval < 0) {
      csign = -1;
   } 

   int doctave = dsign * dval / 7;
   // int coctave = csign * cval / 12;

   int base = 0;

        if ((dval==0)  && (cval==0))   { base =	 0; }
   else if ((dval==0)  && (cval==1))   { base =	 1; }
   else if ((dval==0)  && (cval==2))   { base =	 2; }
   else if ((dval==1)  && (cval==0))   { base =	 4; }
   else if ((dval==1)  && (cval==1))   { base =	 5; }
   else if ((dval==1)  && (cval==2))   { base =	 6; }
   else if ((dval==1)  && (cval==3))   { base =	 7; }
   else if ((dval==1)  && (cval==4))   { base =	 8; }
   else if ((dval==2)  && (cval==2))   { base =	 10; }
   else if ((dval==2)  && (cval==3))   { base =	 11; }
   else if ((dval==2)  && (cval==4))   { base =	 12; }
   else if ((dval==2)  && (cval==5))   { base =	 13; }
   else if ((dval==2)  && (cval==6))   { base =	 14; }
   else if ((dval==3)  && (cval==3))   { base =	 15; }
   else if ((dval==3)  && (cval==4))   { base =	 16; }
   else if ((dval==3)  && (cval==5))   { base =	 17; }
   else if ((dval==3)  && (cval==6))   { base =	 18; }
   else if ((dval==3)  && (cval==7))   { base =	 19; }
   else if ((dval==4)  && (cval==5))   { base =	 21; }
   else if ((dval==4)  && (cval==6))   { base =	 22; }
   else if ((dval==4)  && (cval==7))   { base =	 23; }
   else if ((dval==4)  && (cval==8))   { base =	 24; }
   else if ((dval==4)  && (cval==9))   { base =	 25; }
   else if ((dval==5)  && (cval==7))   { base =	 27; }
   else if ((dval==5)  && (cval==8))   { base =	 28; }
   else if ((dval==5)  && (cval==9))   { base =	 29; }
   else if ((dval==5)  && (cval==10))  { base =	 30; }
   else if ((dval==5)  && (cval==11))  { base =	 31; }
   else if ((dval==6)  && (cval==9))   { base =	 33; }
   else if ((dval==6)  && (cval==10))  { base =	 34; }
   else if ((dval==6)  && (cval==11))  { base =	 35; }
   else if ((dval==6)  && (cval==12))  { base =	 36; }
   else if ((dval==6)  && (cval==13))  { base =	 37; }
   else if ((dval==7)  && (cval==10))  { base =	 38; }
   else if ((dval==7)  && (cval==11))  { base =	 38; }
   else if ((dval==-0) && (cval==-0))  { base =	 -0; }
   else if ((dval==-0) && (cval==-1))  { base =	 -1; }
   else if ((dval==-0) && (cval==-2))  { base =	 -2; }
   else if ((dval==-1) && (cval==1))   { base =	 -3; }
   else if ((dval==-1) && (cval==-0))  { base =	 -4; }
   else if ((dval==-1) && (cval==-1))  { base =	 -5; }
   else if ((dval==-1) && (cval==-2))  { base =	 -6; }
   else if ((dval==-1) && (cval==-3))  { base =	 -7; }
   else if ((dval==-2) && (cval==-1))  { base =	 -9; }
   else if ((dval==-2) && (cval==-2))  { base =	-10; }
   else if ((dval==-2) && (cval==-3))  { base =	-11; }
   else if ((dval==-2) && (cval==-4))  { base =	-12; }
   else if ((dval==-2) && (cval==-5))  { base =	-13; }
   else if ((dval==-3) && (cval==-3))  { base =	-15; }
   else if ((dval==-3) && (cval==-4))  { base =	-16; }
   else if ((dval==-3) && (cval==-5))  { base =	-17; }
   else if ((dval==-3) && (cval==-6))  { base =	-18; }
   else if ((dval==-3) && (cval==-7))  { base =	-19; }
   else if ((dval==-4) && (cval==-5))  { base =	-21; }
   else if ((dval==-4) && (cval==-6))  { base =	-22; }
   else if ((dval==-4) && (cval==-7))  { base =	-23; }
   else if ((dval==-4) && (cval==-8))  { base =	-24; }
   else if ((dval==-4) && (cval==-9))  { base =	-25; }
   else if ((dval==-5) && (cval==-6))  { base =	-26; }
   else if ((dval==-5) && (cval==-7))  { base =	-27; }
   else if ((dval==-5) && (cval==-8))  { base =	-28; }
   else if ((dval==-5) && (cval==-9))  { base =	-29; }
   else if ((dval==-5) && (cval==-10)) { base =	-30; }
   else if ((dval==-6) && (cval==-8))  { base =	-32; }
   else if ((dval==-6) && (cval==-9))  { base =	-33; }
   else if ((dval==-6) && (cval==-10)) { base =	-34; }
   else if ((dval==-6) && (cval==-11)) { base =	-35; }
   else if ((dval==-6) && (cval==-12)) { base =	-36; }
   else if ((dval==-7) && (cval==-10)) { base =	-38; }
   else if ((dval==-7) && (cval==-11)) { base =	-39; }
   else { // some error occurred or accidentals out of range
      // cerr << "Problem occured in transToBase40()" << endl;
      base = 0; 
   }  
	     
   base += 40 * doctave * dsign;
   
   return base;
}



//////////////////////////////
//
// Convert::base40ToTrans -- convert a base 40 interval into
//    a trans program's diatonic/chromatic alteration marker
//


char* Convert::base40ToTrans(char* buffer, int base40) {
   int sign = 1;
   int chroma;
   int octave;
   if (base40 < 0) {
      sign = -1;
      chroma = -base40 % 40;
      octave = -base40 / 40;
   } else {
      sign = +1;
      chroma = base40 % 40;
      octave = base40 / 40;
   }

   int cval = 0;
   int dval = 0;
   
   switch (chroma * sign) {
      case   0:	dval=0;  cval=0;	break; //	C -> C
      case   1:	dval=0;  cval=1;	break; //	C -> C#
      case   2:	dval=0;  cval=2;	break; //	C -> C##
      case   4:	dval=1;  cval=0;	break; //	C -> D--
      case   5:	dval=1;  cval=1;	break; //	C -> D-
      case   6:	dval=1;  cval=2;	break; //	C -> D
      case   7:	dval=1;  cval=3;	break; //	C -> D#
      case   8:	dval=1;  cval=4;	break; //	C -> D##
      case  10:	dval=2;  cval=2;	break; //	C -> E--
      case  11:	dval=2;  cval=3;	break; //	C -> E-
      case  12:	dval=2;  cval=4;	break; //	C -> E
      case  13:	dval=2;  cval=5;	break; //	C -> E#
      case  14:	dval=2;  cval=6;	break; //	C -> E##
      case  15:	dval=3;  cval=3;	break; //	C -> F--
      case  16:	dval=3;  cval=4;	break; //	C -> F-
      case  17:	dval=3;  cval=5;	break; //	C -> F
      case  18:	dval=3;  cval=6;	break; //	C -> F#
      case  19:	dval=3;  cval=7;	break; //	C -> F##
      case  21:	dval=4;  cval=5;	break; //	C -> G--
      case  22:	dval=4;  cval=6;	break; //	C -> G-
      case  23:	dval=4;  cval=7;	break; //	C -> G
      case  24:	dval=4;  cval=8;	break; //	C -> G#
      case  25:	dval=4;  cval=9;	break; //	C -> G##
      case  27:	dval=5;  cval=7;	break; //	C -> A--
      case  28:	dval=5;  cval=8;	break; //	C -> A-
      case  29:	dval=5;  cval=9;	break; //	C -> A
      case  30:	dval=5;  cval=10;	break; //	C -> A#
      case  31:	dval=5;  cval=11;	break; //	C -> A##
      case  33:	dval=6;  cval=9;	break; //	C -> B--
      case  34:	dval=6;  cval=10;	break; //	C -> B-
      case  35:	dval=6;  cval=11;	break; //	C -> B
      case  36:	dval=6;  cval=12;	break; //	C -> B#
      case  37:	dval=6;  cval=13;	break; //	C -> B##
      case  38:	dval=7;  cval=10;	break; //	C -> c--
      case  39:	dval=7;  cval=11;	break; //	C -> c-
      case  -1:	dval=-0; cval=-1;	break; //	c -> c-
      case  -2:	dval=-0; cval=-2;	break; //	c -> c--
      case  -3:	dval=-1; cval=1;	break; //	c -> B##
      case  -4:	dval=-1; cval=-0;	break; //	c -> B#
      case  -5:	dval=-1; cval=-1;	break; //	c -> B
      case  -6:	dval=-1; cval=-2;	break; //	c -> B-
      case  -7:	dval=-1; cval=-3;	break; //	c -> B--
      case  -9:	dval=-2; cval=-1;	break; //	c -> A##
      case -10:	dval=-2; cval=-2;	break; //	c -> A#
      case -11:	dval=-2; cval=-3;	break; //	c -> A
      case -12:	dval=-2; cval=-4;	break; //	c -> A-
      case -13:	dval=-2; cval=-5;	break; //	c -> A-
      case -15:	dval=-3; cval=-3;	break; //	c -> G##
      case -16:	dval=-3; cval=-4;	break; //	c -> G#
      case -17:	dval=-3; cval=-5;	break; //	c -> G
      case -18:	dval=-3; cval=-6;	break; //	c -> G-
      case -19:	dval=-3; cval=-7;	break; //	c -> G--
      case -21:	dval=-4; cval=-5;	break; //	c -> F##
      case -22:	dval=-4; cval=-6;	break; //	c -> F#
      case -23:	dval=-4; cval=-7;	break; //	c -> F
      case -24:	dval=-4; cval=-8;	break; //	c -> F-
      case -25:	dval=-4; cval=-9;	break; //	c -> F--
      case -26:	dval=-5; cval=-6;	break; //	c -> E##
      case -27:	dval=-5; cval=-7;	break; //	c -> E#
      case -28:	dval=-5; cval=-8;	break; //	c -> E
      case -29:	dval=-5; cval=-9;	break; //	c -> E-
      case -30:	dval=-5; cval=-10;	break; //	c -> E--
      case -32:	dval=-6; cval=-8;	break; //	c -> D##
      case -33:	dval=-6; cval=-9;	break; //	c -> D#
      case -34:	dval=-6; cval=-10;	break; //	c -> D
      case -35:	dval=-6; cval=-11;	break; //	c -> D-
      case -36:	dval=-6; cval=-12;	break; //	c -> D--
      case -38:	dval=-7; cval=-10;	break; //	c -> C##
      case -39:	dval=-7; cval=-11;	break; //	c -> C#
      default:
         dval=0; cval=0;
   }

   if (octave > 0) {
      dval = dval + sign * octave * 7;
      cval = cval + sign * octave * 12;
   }

   sprintf(buffer, "d%dc%d", dval, cval);

   return buffer;
}




//////////////////////////////
//
// Convert::keyToScaleDegrees --
//

SigCollection<int> Convert::keyToScaleDegrees(int aKey, int aMode) {
   SigCollection<int> output;
   
   switch (aMode) {
      case E_mode_major:
      case E_mode_ionian:
         output.setSize(7);
         output[0] = aKey;
         output[1] = aKey + E_base40_maj2;
         output[2] = aKey + E_base40_maj3;
         output[3] = aKey + E_base40_per4;
         output[4] = aKey + E_base40_per5;
         output[5] = aKey + E_base40_maj6;
         output[6] = aKey + E_base40_maj7;
         break;
      case E_mode_minor:
      case E_mode_aeolian:
         output[0] = aKey;
         output[1] = aKey + E_base40_maj2;
         output[2] = aKey + E_base40_min3;
         output[3] = aKey + E_base40_per4;
         output[4] = aKey + E_base40_per5;
         output[5] = aKey + E_base40_min6;
         output[6] = aKey + E_base40_min7;
         break;
      case E_mode_harm_minor:
         output[0] = aKey;
         output[1] = aKey + E_base40_maj2;
         output[2] = aKey + E_base40_min3;
         output[3] = aKey + E_base40_per4;
         output[4] = aKey + E_base40_per5;
         output[5] = aKey + E_base40_min6;
         output[6] = aKey + E_base40_maj7;
         break;
      default:
         cerr << "unknown mode: " << aMode << endl;
   }

   // keep the pitches in the primary octave
   for (int i=0; i<output.getSize(); i++) {
      output[i] = output[i] % 40;
   }

   return output;
}



//////////////////////////////
//
// Convert::museToBase40 --
//

int Convert::museToBase40(const char* pitchString) {
   char *temp;
   int length = strlen(pitchString);
   temp = new char[length+1];
   strcpy(temp, pitchString);
   int octave;
   int i = length;
   while (i >= 0 && !isdigit(temp[i])) {
      i--;
   }
  
   if (i <= 0) {
      cerr << "Error: could not find octave in string: " << pitchString << endl;
      exit(1);
   }

   octave = temp[i] - '0';
   temp[i] = '\0';

   return musePitchClass.getValue(temp) + 40 * (octave);
}


///////////////////////////////////////////////////////////////////////////
//
// conversions dealing with base 40 system of notation
//

//////////////////////////////
//
// Convert::base7ToBase12 -- convert diatonic pitch class with optional 
//    chromatic alteration into Base-12 (MIDI note number).
//

int Convert::base7ToBase12(int aPitch, int alter) {
   if (aPitch <= 0) {
      return -1;
   }

   int octave = aPitch / 7;
   int chroma = aPitch % 7;
   int output = 0;
   switch (chroma) {
      case 0:  output =  0;  break;   // 0 = C -> 0
      case 1:  output =  2;  break;   // 1 = D -> 2
      case 2:  output =  4;  break;   // 2 = E -> 4
      case 3:  output =  5;  break;   // 3 = F -> 5
      case 4:  output =  7;  break;   // 4 = G -> 7
      case 5:  output =  9;  break;   // 5 = A -> 9
      case 6:  output = 11;  break;   // 6 = B -> 11
      default: output = 0;
   }
   // need to add 1 to octave since C4 is MIDI note 60 which is 5 * 12:
   return output + 12 * (octave+1) + alter;
}



//////////////////////////////
//
//  Convert::base7ToBase40 --
//

int Convert::base7ToBase40(int aPitch, int alter) {
   return base12ToBase40(base7ToBase12(aPitch, alter));
}



//////////////////////////////
//
// Convert::base12ToBase40 -- assume fixed accidentals.
//

int Convert::base12ToBase40(int aPitch) {
   int octave = aPitch / 12 - 1;
   int chroma = aPitch % 12;
 
   int output = 0;
   switch (chroma) {
      case 0: output = E_muse_c   ;  break;   // 0  = C
      case 1: output = E_muse_cs  ;  break;   // 1  = C#
      case 2: output = E_muse_d   ;  break;   // 2  = D
      case 3: output = E_muse_ef  ;  break;   // 3  = E-
      case 4: output = E_muse_e   ;  break;   // 4  = E
      case 5: output = E_muse_f   ;  break;   // 5  = F
      case 6: output = E_muse_fs  ;  break;   // 6  = F#
      case 7: output = E_muse_g   ;  break;   // 7  = G
      case 8: output = E_muse_af  ;  break;   // 8  = A-
      case 9: output = E_muse_a   ;  break;   // 9  = A
      case 10: output = E_muse_bf ;  break;   // 10 = B-
      case 11: output = E_muse_b  ;  break;   // 11 = B
      default: output = E_muse_c  ;  break;   // other = C
   }
    
   output = output + 40 * octave;
   return output;
}



//////////////////////////////
//
// Convert::base12ToKern -- Convert MIDI note numbers to Kern pitches.
//     It might be nice to also add a reference key to minimize
//     diatonic pitch errors (for example 61 in A-flat major is probably
//     a D-flat, but in B-major it is probably a C-sharp.
//

char* Convert::base12ToKern(char* output, int aPitch) {
   int octave = aPitch / 12 - 1;  // possible bug fix or bug creation
   if (octave > 12 || octave < -1) {
      cerr << "Error: unreasonable octave value: " << octave << endl;
      exit(1);
   }
   int chroma = aPitch % 12;

   switch (chroma) {
      case 0:  strcpy(output, "c");  break;
      case 1:  strcpy(output, "c#"); break;
      case 2:  strcpy(output, "d");  break;
      case 3:  strcpy(output, "e-"); break;
      case 4:  strcpy(output, "e");  break;
      case 5:  strcpy(output, "f");  break;
      case 6:  strcpy(output, "f#"); break;
      case 7:  strcpy(output, "g");  break;
      case 8:  strcpy(output, "g#"); break;
      case 9:  strcpy(output, "a");  break;
      case 10: strcpy(output, "b-"); break;
      case 11: strcpy(output, "b");  break;
   }

   if (octave >= 4) {
      output[0] = tolower(output[0]);
   } else {
      output[0] = toupper(output[0]);
   }
   int repeat = 0;
   switch (octave) {
      case 4:  repeat = 0; break;
      case 5:  repeat = 1; break;
      case 6:  repeat = 2; break;
      case 7:  repeat = 3; break;
      case 8:  repeat = 4; break;
      case 9:  repeat = 5; break;
      case 3:  repeat = 0; break;
      case 2:  repeat = 1; break;
      case 1:  repeat = 2; break;
      case 0:  repeat = 3; break;
      case -1: repeat = 4; break;
      default:
         cerr << "Error: unknown octave value: " << octave << endl;
         cerr << "for base-12 pitch: " << aPitch << endl;
         exit(1);
   }
   if (repeat == 0) {
      return output;
   }

   int length = strlen(output);
   output[length + repeat] = '\0';
   int i; 
   for (i=length-1; i>=0; i--) {
      output[i+repeat] = output[i];
   }
   for (i=0; i<repeat; i++) {
      output[i+1] = output[0];
   }

   return output;
}



//////////////////////////////
//
// Convert::base12ToPitch -- Convert MIDI note numbers to **pitch format.
//     This is a diatonic pitch class followed by an octave number.
//     It might be nice to also add a reference key to minimize
//     diatonic pitch errors (for example 61 in A-flat major is probably
//     a D-flat, but in B-major it is probably a C-sharp.
//

char* Convert::base12ToPitch(char* output, int aPitch) {
   int octave = aPitch / 12 - 1;  // possible bug fix or bug creation
   if (octave > 12 || octave < -1) {
      cerr << "Error: unreasonable octave value: " << octave << endl;
      exit(1);
   }
   int chroma = aPitch % 12;

   switch (chroma) {
      case 0:  strcpy(output, "C");  break;
      case 1:  strcpy(output, "C#"); break;
      case 2:  strcpy(output, "D");  break;
      case 3:  strcpy(output, "E-"); break;
      case 4:  strcpy(output, "E");  break;
      case 5:  strcpy(output, "F");  break;
      case 6:  strcpy(output, "F#"); break;
      case 7:  strcpy(output, "G");  break;
      case 8:  strcpy(output, "G#"); break;
      case 9:  strcpy(output, "A");  break;
      case 10: strcpy(output, "B-"); break;
      case 11: strcpy(output, "B");  break;
   }
   char buffer[12] = {0};
   sprintf(buffer, "%d", octave);
   strcat(output, buffer);

   return output;
}



///////////////////////////////////////////////////////////////////////////
//
// conversions dealing with frequency in hertz
//


//////////////////////////////
//
// Convert::freq2midi --
//     default value: a440 = 440.0
//

int Convert::freq2midi(double freq, double a440) {
   return (int)(69.5 + 12 * log10(freq/a440)/log10(2.0));
}


//////////////////////////////
//
// kotoToDuration -- convert a **koto value into a duration
//   with 1.0 being a quarter note.
//

double Convert::kotoToDuration(const char* aKotoString) {
   int length = strlen(aKotoString);
   int bars   = 0;
   int dashes = 0;
   int dots   = 0;
   int i;

   for (i=0; i<length; i++) {
      if (aKotoString[i] == '|') {
         bars++;
      }
      if (aKotoString[i] == '.') {
         dots++;
      }
      if (aKotoString[i] == '-') {
         dashes++;
      }
      if (aKotoString[i] == ' ') {
         break;   // only look at first note in a chord or arpeggio
      }
   }
   // now know everything to create a duration

   double duration = 1;
   for (i=0; i<bars; i++) {
      duration = duration / 2.0;
   }
   
   double baseduration = duration;
   for (i=0; i<dots; i++) {
      duration += baseduration * pow(2.0, -(i+1));
   }

   return duration;
}



//////////////////////////////
//
// kotoToDurationR -- convert a **koto value into a duration
//   with 1.0 being a quarter note.
//

RationalNumber Convert::kotoToDurationR(const char* aKotoString) {
   int length = strlen(aKotoString);
   int bars   = 0;
   int dashes = 0;
   int dots   = 0;
   int i;

   for (i=0; i<length; i++) {
      if (aKotoString[i] == '|') {
         bars++;
      }
      if (aKotoString[i] == '.') {
         dots++;
      }
      if (aKotoString[i] == '-') {
         dashes++;
      }
      if (aKotoString[i] == ' ') {
         break;   // only look at first note in a chord or arpeggio
      }
   }

   // now know everything to create a duration

   RationalNumber duration = 1;
   for (i=0; i<bars; i++) {
      duration /= 2;
   }
   
   RationalNumber baseduration = duration;
   for (i=0; i<dots; i++) {
      duration += baseduration / (int)(pow(2.0, (double)(i+1)));
   }

   return duration;
}




///////////////////////////////////////////////////////////////////////////
//
// Protected functions
//


//////////////////////////////
//
// calculateInversion --
//

int Convert::calculateInversion(int aType, int bassNote, int root) {

   if (aType == E_chord_note) {
      return -1;
   }

   ChordQuality cq(aType, 0, root);
   SigCollection<int> notes;
   chordQualityToNoteSet(notes, cq);
   bassNote = bassNote % 40;
   int i;
   for (i=0; i<notes.getSize(); i++) {
      if (notes[i] == bassNote) {
         return i;
      }
   }
   
   cerr << "Error: Note: " 
        << Convert::kernPitchClass.getName(bassNote) << " is not in chord " 
        << Convert::chordType.getName(aType) << " with root "
        << Convert::kernPitchClass.getName(root) << endl;

   exit(1);
}


//////////////////////////////
//
// checkChord --
//

int Convert::checkChord(const SigCollection<int>& aSet) {

   int output;
   SigCollection<int> inval(aSet.getSize()-1);
   for (int i=0; i<inval.getSize(); i++) {
      inval[i] = aSet[i+1] - aSet[0];
   }

   switch (inval.getSize()) {
      case 0:
         output = E_chord_note;
         break;
      case 1:
         if (inval[0] == 11) {
            output = E_chord_incmin;
         } else if (inval[0] == 12) {
            output = E_chord_incmaj;
         } else {
            output = E_unknown;
         }
         break;
      case 2:
         if (inval[0] == 11 && inval[1] == 22) {
            output = E_chord_dim;
         } else if (inval[0] == 11 && inval[1] == 23) {
            output = E_chord_min;
         } else if (inval[0] == 12 && inval[1] == 23) {
            output = E_chord_maj;
         } else if (inval[0] == 12 && inval[1] == 24) {
            output = E_chord_aug;
         } else if (inval[0] == 11 && inval[1] == 34) {
            output = E_chord_minminx5;
         } else if (inval[0] == 12 && inval[1] == 34) {
            output = E_chord_domsevx5;
         } else if (inval[0] == 12 && inval[1] == 30) {
            output = E_chord_italsix;
         } else {
            output = E_unknown;
         }
         break;
      case 3:
         if (inval[0] == 11 && inval[1] == 22 && inval[2] == 33) {
            output = E_chord_fullydim;
         } else if (inval[0] == 11 && inval[1] == 22 && inval[2] == 34) {
            output = E_chord_halfdim;
         } else if (inval[0] == 11 && inval[1] == 23 && inval[2] == 34) {
            output = E_chord_minmin;
         } else if (inval[0] == 11 && inval[1] == 23 && inval[2] == 35) {
            output = E_chord_minmaj;
         } else if (inval[0] == 12 && inval[1] == 23 && inval[2] == 34) {
            output = E_chord_domsev;
         } else if (inval[0] == 12 && inval[1] == 23 && inval[2] == 35) {
            output = E_chord_majmaj;
         } else if (inval[0] == 12 && inval[1] == 18 && inval[2] == 30) {
            output = E_chord_frensix;
         } else if (inval[0] == 12 && inval[1] == 23 && inval[2] == 30) {
            output = E_chord_germsix;
         } else {
            output = E_unknown;
         }
         break;
      default:
         output = E_unknown;
   }

   return output;
}
   


//////////////////////////////
//
// intcompare -- compare two integers for ordering
//

int Convert::intcompare(const void* a, const void* b) {

   if (*((int*)a) < *((int*)b)) {
      return -1;
   } else if (*((int*)a) > *((int*)b)) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// Convert::rotatechord -- put the bottom note in the chord
//     on the top
//

void Convert::rotatechord(SigCollection<int>& aChord) {

   int temp = aChord[0];
   for (int i=1; i<aChord.getSize(); i++) {
      aChord[i-1] = aChord[i];
   }
   aChord[aChord.getSize()-1] = temp + 40;
}



//////////////////////////////
//
// Convert::base40ToIntervalVector -- convert a list of base 40 pitches into an
//      interval vector.
//

void Convert::base40ToIntervalVector(Array<int>& iv, Array<int>& base40) {
   iv.setSize(6);
   iv.setAll(0);
   if (base40.getSize() <= 1) {
      return;
   }
   Array<int> base12;
   base12.setSize(base40.getSize());
   int i;
   for (i=0; i<base12.getSize(); i++) {
      base12[i] = Convert::base40ToMidiNoteNumber(base40[i]);
   }
   Convert::base12ToIntervalVector(iv, base12);
}



//////////////////////////////
//
// Convert::base12ToIntervalVector -- convert a list of MIDI note numbers
//     into an interval vector.
//

void Convert::base12ToIntervalVector(Array<int>& iv, Array<int>& base12) {
   iv.setSize(6);
   iv.setAll(0);
   if (base12.getSize() <= 1) {
      return;
   }

   Array<int> pitchclass(12);
   pitchclass.setAll(0);
   int i, j;
   for (i=0; i<base12.getSize(); i++) {
      pitchclass[base12[i] % 12] = 1;
   }
   int value;
   for (i=0; i<12; i++) {
      for (j=i+1; j<12; j++) {
         if (pitchclass[i] + pitchclass[j] < 2) {
            continue;
         }
         value = j - i;
         if (value > 6) {
            // inversion of interval
            value = 12 - value;
         }
         iv[value-1]++;
      }
   }
}



//////////////////////////////
//
// Convert::base12ToTnNormalForm --
//

void Convert::base12ToTnNormalForm(Array<int>& tnorm, Array<int>& base12) {
   if (base12.getSize() == 0) {
      tnorm.setSize(0);
      return;
   } else if (base12.getSize() == 0) {
      tnorm.setSize(1);
      tnorm[0] = 0;
      return;
   }
   Convert::base12ToNormalForm(tnorm, base12);
   int i;
   int reference = tnorm[0];
   for (i=0; i<tnorm.getSize(); i++) {
      tnorm[i] = (tnorm[i] - reference + 144) % 12;
   }
}



//////////////////////////////
//
// Convert::base12ToTnSetName -- given an input of base12 pitches, return 
// the Tn set name for that unsorted collection (which may contain repeats)
//

const char* Convert::base12ToTnSetName(Array<int>& base12) {
   Array<int> tnorm;
   Convert::base12ToTnNormalForm(tnorm, base12);
   int pcount = tnorm.getSize();
   Array<int>& x = tnorm;

   switch (pcount) {
      case 0:
         return "0-1";

      case 1:
         return "1-1";

      case 2:
         if (x[1]==1)	return "2-1*";
         if (x[1]==2)	return "2-2*";
         if (x[1]==3)	return "2-3*";
         if (x[1]==4)	return "2-4*";
         if (x[1]==5)	return "2-5*";
         if (x[1]==6)	return "2-6*S6";

      case 3:
         if (x[1]==1 && x[2]==2)	return "3-1*";
         if (x[1]==1 && x[2]==3)	return "3-2A";
         if (x[1]==2 && x[2]==3)	return "3-2B";
         if (x[1]==1 && x[2]==4)	return "3-3A";
         if (x[1]==3 && x[2]==4)	return "3-3B";
         if (x[1]==1 && x[2]==5)	return "3-4A";
         if (x[1]==4 && x[2]==5)	return "3-4B";
         if (x[1]==1 && x[2]==6)	return "3-5A";
         if (x[1]==5 && x[2]==6)	return "3-5B";
         if (x[1]==2 && x[2]==4)	return "3-6*";
         if (x[1]==2 && x[2]==5)	return "3-7A";
         if (x[1]==3 && x[2]==5)	return "3-7B";
         if (x[1]==2 && x[2]==6)	return "3-8A";
         if (x[1]==4 && x[2]==6)	return "3-8B";
         if (x[1]==2 && x[2]==7)	return "3-9*";
         if (x[1]==3 && x[2]==6)	return "3-10*";
         if (x[1]==3 && x[2]==7)	return "3-11A";
         if (x[1]==4 && x[2]==7)	return "3-11B";
         if (x[1]==4 && x[2]==8)	return "3-12*S4";

      case 4:
         if (x[1]==1 && x[2]==2 && x[3]==3)	return "4-1*";
         if (x[1]==1 && x[2]==2 && x[3]==4)	return "4-2A";
         if (x[1]==2 && x[2]==3 && x[3]==4)	return "4-2B";
         if (x[1]==1 && x[2]==3 && x[3]==4)	return "4-3*";
         if (x[1]==1 && x[2]==2 && x[3]==5)	return "4-4A";
         if (x[1]==3 && x[2]==4 && x[3]==5)	return "4-4B";
         if (x[1]==1 && x[2]==2 && x[3]==6)	return "4-5A";
         if (x[1]==4 && x[2]==5 && x[3]==6)	return "4-5B";
         if (x[1]==1 && x[2]==2 && x[3]==7)	return "4-6*";
         if (x[1]==1 && x[2]==4 && x[3]==5)	return "4-7*";
         if (x[1]==1 && x[2]==5 && x[3]==6)	return "4-8*";
         if (x[1]==1 && x[2]==6 && x[3]==7)	return "4-9*S6";
         if (x[1]==2 && x[2]==3 && x[3]==5)	return "4-10*";
         if (x[1]==1 && x[2]==3 && x[3]==5)	return "4-11A";
         if (x[1]==2 && x[2]==4 && x[3]==5)	return "4-11B";
         if (x[1]==2 && x[2]==3 && x[3]==6)	return "4-12A<";
         if (x[1]==3 && x[2]==4 && x[3]==6)	return "4-12B<";
         if (x[1]==1 && x[2]==3 && x[3]==6)	return "4-13A";
         if (x[1]==3 && x[2]==5 && x[3]==6)	return "4-13B";
         if (x[1]==2 && x[2]==3 && x[3]==7)	return "4-14A<";
         if (x[1]==4 && x[2]==5 && x[3]==7)	return "4-14B<";
         if (x[1]==1 && x[2]==4 && x[3]==6)	return "4-Z15A..29";
         if (x[1]==2 && x[2]==5 && x[3]==6)	return "4-Z15B..29";
         if (x[1]==1 && x[2]==5 && x[3]==7)	return "4-16A";
         if (x[1]==2 && x[2]==6 && x[3]==7)	return "4-16B";
         if (x[1]==3 && x[2]==4 && x[3]==7)	return "4-17*";
         if (x[1]==1 && x[2]==4 && x[3]==7)	return "4-18A";
         if (x[1]==3 && x[2]==6 && x[3]==7)	return "4-18B";
         if (x[1]==1 && x[2]==4 && x[3]==8)	return "4-19A";
         if (x[1]==3 && x[2]==4 && x[3]==8)	return "4-19B";
         if (x[1]==1 && x[2]==5 && x[3]==8)	return "4-20*";
         if (x[1]==2 && x[2]==4 && x[3]==6)	return "4-21*";
         if (x[1]==2 && x[2]==4 && x[3]==7)	return "4-22A";
         if (x[1]==3 && x[2]==5 && x[3]==7)	return "4-22B";
         if (x[1]==2 && x[2]==5 && x[3]==7)	return "4-23*";
         if (x[1]==2 && x[2]==4 && x[3]==8)	return "4-24*";
         if (x[1]==2 && x[2]==6 && x[3]==8)	return "4-25*S6";
         if (x[1]==3 && x[2]==5 && x[3]==8)	return "4-26*";
         if (x[1]==2 && x[2]==5 && x[3]==8)	return "4-27A";
         if (x[1]==3 && x[2]==6 && x[3]==8)	return "4-27B";
         if (x[1]==3 && x[2]==6 && x[3]==9)	return "4-28*S3";
         if (x[1]==1 && x[2]==3 && x[3]==7)	return "4-Z29A..15";
         if (x[1]==4 && x[2]==6 && x[3]==7)	return "4-Z29B..15";

      case 5:
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4)	return "5-1*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5)	return "5-2A";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5)	return "5-2B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5)	return "5-3A";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5)	return "5-3B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==6)	return "5-4A";
         if (x[1]==3 && x[2]==4 && x[3]==5 && x[4]==6)	return "5-4B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==7)	return "5-5A";
         if (x[1]==4 && x[2]==5 && x[3]==6 && x[4]==7)	return "5-5B";
         if (x[1]==1 && x[2]==2 && x[3]==5 && x[4]==6)	return "5-6A";
         if (x[1]==1 && x[2]==4 && x[3]==5 && x[4]==6)	return "5-6B";
         if (x[1]==1 && x[2]==2 && x[3]==6 && x[4]==7)	return "5-7A";
         if (x[1]==1 && x[2]==5 && x[3]==6 && x[4]==7)	return "5-7B";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==6)	return "5-8*";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==6)	return "5-9A";
         if (x[1]==2 && x[2]==4 && x[3]==5 && x[4]==6)	return "5-9B";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==6)	return "5-10A";
         if (x[1]==2 && x[2]==3 && x[3]==5 && x[4]==6)	return "5-10B";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==7)	return "5-11A";
         if (x[1]==3 && x[2]==4 && x[3]==5 && x[4]==7)	return "5-11B";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==6)	return "5-Z12*..36";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==8)	return "5-13A";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==8)	return "5-13B";
         if (x[1]==1 && x[2]==2 && x[3]==5 && x[4]==7)	return "5-14A";
         if (x[1]==2 && x[2]==5 && x[3]==6 && x[4]==7)	return "5-14B";
         if (x[1]==1 && x[2]==2 && x[3]==6 && x[4]==8)	return "5-15*";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==7)	return "5-16A";
         if (x[1]==3 && x[2]==4 && x[3]==6 && x[4]==7)	return "5-16B";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==8)	return "5-Z17*..37";
         if (x[1]==1 && x[2]==4 && x[3]==5 && x[4]==7)	return "5-Z18A<..38";
         if (x[1]==2 && x[2]==3 && x[3]==6 && x[4]==7)	return "5-Z18B<..38";
         if (x[1]==1 && x[2]==3 && x[3]==6 && x[4]==7)	return "5-19A";
         if (x[1]==1 && x[2]==4 && x[3]==6 && x[4]==7)	return "5-19B";
         if (x[1]==1 && x[2]==3 && x[3]==7 && x[4]==8)	return "5-20A";
         if (x[1]==1 && x[2]==5 && x[3]==7 && x[4]==8)	return "5-20B";
         if (x[1]==1 && x[2]==4 && x[3]==5 && x[4]==8)	return "5-21A";
         if (x[1]==3 && x[2]==4 && x[3]==7 && x[4]==8)	return "5-21B";
         if (x[1]==1 && x[2]==4 && x[3]==7 && x[4]==8)	return "5-22*";
         if (x[1]==2 && x[2]==3 && x[3]==5 && x[4]==7)	return "5-23A";
         if (x[1]==2 && x[2]==4 && x[3]==5 && x[4]==7)	return "5-23B";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==7)	return "5-24A";
         if (x[1]==2 && x[2]==4 && x[3]==6 && x[4]==7)	return "5-24B";
         if (x[1]==2 && x[2]==3 && x[3]==5 && x[4]==8)	return "5-25A";
         if (x[1]==3 && x[2]==5 && x[3]==6 && x[4]==8)	return "5-25B";
         if (x[1]==2 && x[2]==4 && x[3]==5 && x[4]==8)	return "5-26A<";
         if (x[1]==3 && x[2]==4 && x[3]==6 && x[4]==8)	return "5-26B<";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==8)	return "5-27A";
         if (x[1]==3 && x[2]==5 && x[3]==7 && x[4]==8)	return "5-27B";
         if (x[1]==2 && x[2]==3 && x[3]==6 && x[4]==8)	return "5-28A<";
         if (x[1]==2 && x[2]==5 && x[3]==6 && x[4]==8)	return "5-28B<";
         if (x[1]==1 && x[2]==3 && x[3]==6 && x[4]==8)	return "5-29A";
         if (x[1]==2 && x[2]==5 && x[3]==7 && x[4]==8)	return "5-29B";
         if (x[1]==1 && x[2]==4 && x[3]==6 && x[4]==8)	return "5-30A";
         if (x[1]==2 && x[2]==4 && x[3]==7 && x[4]==8)	return "5-30B";
         if (x[1]==1 && x[2]==3 && x[3]==6 && x[4]==9)	return "5-31A";
         if (x[1]==2 && x[2]==3 && x[3]==6 && x[4]==9)	return "5-31B";
         if (x[1]==1 && x[2]==4 && x[3]==6 && x[4]==9)	return "5-32A";
         if (x[1]==1 && x[2]==4 && x[3]==7 && x[4]==9)	return "5-32B";
         if (x[1]==2 && x[2]==4 && x[3]==6 && x[4]==8)	return "5-33*";
         if (x[1]==2 && x[2]==4 && x[3]==6 && x[4]==9)	return "5-34*";
         if (x[1]==2 && x[2]==4 && x[3]==7 && x[4]==9)	return "5-35*";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==7)	return "5-Z36A..12";
         if (x[1]==3 && x[2]==5 && x[3]==6 && x[4]==7)	return "5-Z36B..12";
         if (x[1]==3 && x[2]==4 && x[3]==5 && x[4]==8)	return "5-Z37*..17";
         if (x[1]==1 && x[2]==2 && x[3]==5 && x[4]==8)	return "5-Z38A..18";
         if (x[1]==3 && x[2]==6 && x[3]==7 && x[4]==8)	return "5-Z38B..18";

      case 6:
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5)	return "6-1*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==6)	return "6-2A";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6)	return "6-2BA";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==6)	return "6-Z3A..36B";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6)	return "6-Z3B..36";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==6)	return "6-Z4*..37";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==6 && x[5]==7)	return "6-5A";
         if (x[1]==1 && x[2]==4 && x[3]==5 && x[4]==6 && x[5]==7)	return "6-5B";
         if (x[1]==1 && x[2]==2 && x[3]==5 && x[4]==6 && x[5]==7)	return "6-Z6*..38";
         if (x[1]==1 && x[2]==2 && x[3]==6 && x[4]==7 && x[5]==8)	return "6-7*S6";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==7)	return "6-8*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==7)	return "6-9A";
         if (x[1]==2 && x[2]==4 && x[3]==5 && x[4]==6 && x[5]==7)	return "6-9B";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==7)	return "6-Z10..39";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==7)	return "6-Z10B..39B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==7)	return "6-Z11..40B";
         if (x[1]==2 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==7)	return "6-Z11B..40";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==6 && x[5]==7)	return "6-Z12..41B";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==7)	return "6-Z12B..41";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==7)	return "6-Z13*..42";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==8)	return "6-14..14";
         if (x[1]==3 && x[2]==4 && x[3]==5 && x[4]==7 && x[5]==8)	return "6-14B..14B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==8)	return "6-15A";
         if (x[1]==3 && x[2]==4 && x[3]==6 && x[4]==7 && x[5]==8)	return "6-15B";
         if (x[1]==1 && x[2]==4 && x[3]==5 && x[4]==6 && x[5]==8)	return "6-16A";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==7 && x[5]==8)	return "6-16B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==7 && x[5]==8)	return "6-Z17..43B";
         if (x[1]==1 && x[2]==4 && x[3]==6 && x[4]==7 && x[5]==8)	return "6-Z17B..43";
         if (x[1]==1 && x[2]==2 && x[3]==5 && x[4]==7 && x[5]==8)	return "6-18A";
         if (x[1]==1 && x[2]==3 && x[3]==6 && x[4]==7 && x[5]==8)	return "6-18B";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==7 && x[5]==8)	return "6-Z19..44B";
         if (x[1]==1 && x[2]==4 && x[3]==5 && x[4]==7 && x[5]==8)	return "6-Z19B..44";
         if (x[1]==1 && x[2]==4 && x[3]==5 && x[4]==8 && x[5]==9)	return "6-20*S4";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==8)	return "6-21A";
         if (x[1]==2 && x[2]==4 && x[3]==5 && x[4]==6 && x[5]==8)	return "6-21B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==6 && x[5]==8)	return "6-22A";
         if (x[1]==2 && x[2]==4 && x[3]==6 && x[4]==7 && x[5]==8)	return "6-22B";
         if (x[1]==2 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==8)	return "6-Z23*..45";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==8)	return "6-Z24..46B";
         if (x[1]==2 && x[2]==4 && x[3]==5 && x[4]==7 && x[5]==8)	return "6-Z24B..46";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==8)	return "6-Z25..47B";
         if (x[1]==2 && x[2]==3 && x[3]==5 && x[4]==7 && x[5]==8)	return "6-Z25B..47";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==7 && x[5]==8)	return "6-Z26*..48";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==9)	return "6-27A";
         if (x[1]==2 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==9)	return "6-27B";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==9)	return "6-Z28*..49";
         if (x[1]==1 && x[2]==3 && x[3]==6 && x[4]==8 && x[5]==9)	return "6-Z29*..50";
         if (x[1]==1 && x[2]==3 && x[3]==6 && x[4]==7 && x[5]==9)	return "6-30AS6";
         if (x[1]==2 && x[2]==3 && x[3]==6 && x[4]==8 && x[5]==9)	return "6-30BS6";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==8 && x[5]==9)	return "6-31A";
         if (x[1]==1 && x[2]==4 && x[3]==6 && x[4]==8 && x[5]==9)	return "6-31B";
         if (x[1]==2 && x[2]==4 && x[3]==5 && x[4]==7 && x[5]==9)	return "6-32*";
         if (x[1]==2 && x[2]==3 && x[3]==5 && x[4]==7 && x[5]==9)	return "6-33A";
         if (x[1]==2 && x[2]==4 && x[3]==6 && x[4]==7 && x[5]==9)	return "6-33B";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==7 && x[5]==9)	return "6-34A";
         if (x[1]==2 && x[2]==4 && x[3]==6 && x[4]==8 && x[5]==9)	return "6-34B";
         if (x[1]==2 && x[2]==4 && x[3]==6 && x[4]==8 && x[5]==10)	return "6-35*S2";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==7)	return "6-Z36..3B";
         if (x[1]==3 && x[2]==4 && x[3]==5 && x[4]==6 && x[5]==7)	return "6-Z36B..3";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==8)	return "6-Z37*..4";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==7 && x[5]==8)	return "6-Z38*..6";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==8)	return "6-Z39..10";
         if (x[1]==3 && x[2]==4 && x[3]==5 && x[4]==6 && x[5]==8)	return "6-Z39B..10B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==8)	return "6-Z40..11B";
         if (x[1]==3 && x[2]==5 && x[3]==6 && x[4]==7 && x[5]==8)	return "6-Z40B..11";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==6 && x[5]==8)	return "6-Z41..12B";
         if (x[1]==2 && x[2]==5 && x[3]==6 && x[4]==7 && x[5]==8)	return "6-Z41B..12";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==6 && x[5]==9)	return "6-Z42*..13";
         if (x[1]==1 && x[2]==2 && x[3]==5 && x[4]==6 && x[5]==8)	return "6-Z43..17B";
         if (x[1]==2 && x[2]==3 && x[3]==6 && x[4]==7 && x[5]==8)	return "6-Z43B..17";
         if (x[1]==1 && x[2]==2 && x[3]==5 && x[4]==6 && x[5]==9)	return "6-Z44..19B";
         if (x[1]==1 && x[2]==2 && x[3]==5 && x[4]==8 && x[5]==9)	return "6-Z44B..19";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==9)	return "6-Z45*..23";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==6 && x[5]==9)	return "6-Z46..24B";
         if (x[1]==2 && x[2]==4 && x[3]==5 && x[4]==6 && x[5]==9)	return "6-Z46B..24";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==7 && x[5]==9)	return "6-Z47..25B";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==7 && x[5]==9)	return "6-Z47B..25";
         if (x[1]==1 && x[2]==2 && x[3]==5 && x[4]==7 && x[5]==9)	return "6-Z48*..26";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==7 && x[5]==9)	return "6-Z49*..28";
         if (x[1]==1 && x[2]==4 && x[3]==6 && x[4]==7 && x[5]==9)	return "6-Z50*..29";

      case 7:
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6)	return "7-1*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==7)	return "7-2A";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==7)	return "7-2B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==8)	return "7-3A";
         if (x[1]==3 && x[2]==4 && x[3]==5 && x[4]==6 && x[5]==7 && x[6]==8)	return "7-3B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==6 && x[6]==7)	return "7-4A";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==7)	return "7-4B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==6 && x[6]==7)	return "7-5A";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==7)	return "7-5B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==7 && x[6]==8)	return "7-6A";
         if (x[1]==1 && x[2]==4 && x[3]==5 && x[4]==6 && x[5]==7 && x[6]==8)	return "7-6B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==6 && x[5]==7 && x[6]==8)	return "7-7A";
         if (x[1]==1 && x[2]==2 && x[3]==5 && x[4]==6 && x[5]==7 && x[6]==8)	return "7-7B";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==8)	return "7-8*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==6 && x[6]==8)	return "7-9A";
         if (x[1]==2 && x[2]==4 && x[3]==5 && x[4]==6 && x[5]==7 && x[6]==8)	return "7-9B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==6 && x[6]==9)	return "7-10A";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==9)	return "7-10B";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==8)	return "7-11A";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==7 && x[6]==8)	return "7-11B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==7 && x[6]==9)	return "7-Z12*..36";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==8)	return "7-13A";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==7 && x[6]==8)	return "7-13B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==7 && x[6]==8)	return "7-14A";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==7 && x[6]==8)	return "7-14B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==6 && x[5]==7 && x[6]==8)	return "7-15*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==6 && x[6]==9)	return "7-16A";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==9)	return "7-16B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==9)	return "7-Z17*..37";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==8 && x[6]==9)	return "7-Z18A<..38";
         if (x[1]==1 && x[2]==4 && x[3]==6 && x[4]==7 && x[5]==8 && x[6]==9)	return "7-Z18B<..38";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==6 && x[5]==7 && x[6]==9)	return "7-19A";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==6 && x[5]==8 && x[6]==9)	return "7-19B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==7 && x[5]==8 && x[6]==9)	return "7-20A";
         if (x[1]==1 && x[2]==2 && x[3]==5 && x[4]==7 && x[5]==8 && x[6]==9)	return "7-20B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==8 && x[6]==9)	return "7-21A";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==8 && x[6]==9)	return "7-21B";
         if (x[1]==1 && x[2]==2 && x[3]==5 && x[4]==6 && x[5]==8 && x[6]==9)	return "7-22*";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==7 && x[6]==9)	return "7-23A";
         if (x[1]==2 && x[2]==4 && x[3]==5 && x[4]==6 && x[5]==7 && x[6]==9)	return "7-23B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==7 && x[6]==9)	return "7-24A";
         if (x[1]==2 && x[2]==4 && x[3]==6 && x[4]==7 && x[5]==8 && x[6]==9)	return "7-24B";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==7 && x[6]==9)	return "7-25A";
         if (x[1]==2 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==7 && x[6]==9)	return "7-25B";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==7 && x[6]==9)	return "7-26A<";
         if (x[1]==2 && x[2]==4 && x[3]==5 && x[4]==6 && x[5]==8 && x[6]==9)	return "7-26B<";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==7 && x[6]==9)	return "7-27A";
         if (x[1]==2 && x[2]==4 && x[3]==5 && x[4]==7 && x[5]==8 && x[6]==9)	return "7-27B";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==7 && x[6]==9)	return "7-28A<";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==8 && x[6]==9)	return "7-28B<";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==6 && x[5]==7 && x[6]==9)	return "7-29A";
         if (x[1]==2 && x[2]==3 && x[3]==5 && x[4]==7 && x[5]==8 && x[6]==9)	return "7-29B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==6 && x[5]==8 && x[6]==9)	return "7-30A";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==7 && x[5]==8 && x[6]==9)	return "7-30B";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==7 && x[6]==9)	return "7-31A";
         if (x[1]==2 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==8 && x[6]==9)	return "7-31B";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==8 && x[6]==9)	return "7-32A";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==8 && x[6]==9)	return "7-32B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==6 && x[5]==8 && x[6]==10)	return "7-33*";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==8 && x[6]==10)	return "7-34*";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==8 && x[6]==10)	return "7-35*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==6 && x[6]==8)	return "7-Z36A..12";
         if (x[1]==2 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==7 && x[6]==8)	return "7-Z36B..12";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==7 && x[6]==8)	return "7-Z37*..17";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==7 && x[6]==8)	return "7-Z38A..18";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==7 && x[6]==8)	return "7-Z38B..18";

      case 8:
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6 && x[7]==7)	return "8-1*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6 && x[7]==8)	return "8-2A";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==8)	return "8-2B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6 && x[7]==9)	return "8-3*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==7 && x[7]==8)	return "8-4A";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==8)	return "8-4B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==6 && x[6]==7 && x[7]==8)	return "8-5A";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==8)	return "8-5B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==8)	return "8-6*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==8 && x[7]==9)	return "8-7*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==7 && x[6]==8 && x[7]==9)	return "8-8*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==6 && x[5]==7 && x[6]==8 && x[7]==9)	return "8-9*S6";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==9)	return "8-10*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==7 && x[7]==9)	return "8-11A";
         if (x[1]==2 && x[2]==4 && x[3]==5 && x[4]==6 && x[5]==7 && x[6]==8 && x[7]==9)	return "8-11B";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==9)	return "8-12A<";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==8 && x[7]==9)	return "8-12B<";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==6 && x[6]==7 && x[7]==9)	return "8-13A";
         if (x[1]==2 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==7 && x[6]==8 && x[7]==9)	return "8-13B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==9)	return "8-14A<";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==7 && x[6]==8 && x[7]==9)	return "8-14B<";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==6 && x[6]==8 && x[7]==9)	return "8-Z15A..29";
         if (x[1]==1 && x[2]==3 && x[3]==5 && x[4]==6 && x[5]==7 && x[6]==8 && x[7]==9)	return "8-Z15B..29";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==7 && x[6]==8 && x[7]==9)	return "8-16A";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==6 && x[5]==7 && x[6]==8 && x[7]==9)	return "8-16B";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==8 && x[7]==9)	return "8-17*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==6 && x[6]==8 && x[7]==9)	return "8-18A";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==7 && x[6]==8 && x[7]==9)	return "8-18B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==8 && x[7]==9)	return "8-19A";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==7 && x[6]==8 && x[7]==9)	return "8-19B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==7 && x[6]==8 && x[7]==9)	return "8-20*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==6 && x[6]==8 && x[7]==10)	return "8-21*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==6 && x[6]==8 && x[7]==10)	return "8-22A";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==7 && x[6]==9 && x[7]==10)	return "8-22B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==7 && x[6]==8 && x[7]==10)	return "8-23*";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==8 && x[7]==10)	return "8-24*";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==6 && x[5]==7 && x[6]==8 && x[7]==10)	return "8-25*S6";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==7 && x[6]==9 && x[7]==10)	return "8-26*";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==7 && x[6]==8 && x[7]==10)	return "8-27A";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==6 && x[5]==7 && x[6]==9 && x[7]==10)	return "8-27B";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==7 && x[6]==9 && x[7]==10)	return "8-28*S3";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==9)	return "8-Z29A..15";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==6 && x[5]==7 && x[6]==8 && x[7]==9)	return "8-Z29B..15";

      case 9:
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6 && x[7]==7 && x[8]==8)	return "9-1*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6 && x[7]==7 && x[8]==9)	return "9-2A";
         if (x[1]==2 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==8 && x[8]==9)	return "9-2B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6 && x[7]==8 && x[8]==9)	return "9-3A";
         if (x[1]==1 && x[2]==3 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==8 && x[8]==9)	return "9-3B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==7 && x[7]==8 && x[8]==9)	return "9-4A";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==8 && x[8]==9)	return "9-4B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==6 && x[6]==7 && x[7]==8 && x[8]==9)	return "9-5A";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==8 && x[8]==9)	return "9-5B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6 && x[7]==8 && x[8]==10)	return "9-6*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==7 && x[7]==8 && x[8]==10)	return "9-7A";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==7 && x[7]==9 && x[8]==10)	return "9-7B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==6 && x[6]==7 && x[7]==8 && x[8]==10)	return "9-8A";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==6 && x[6]==8 && x[7]==9 && x[8]==10)	return "9-8B";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==8 && x[8]==10)	return "9-9*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==6 && x[6]==7 && x[7]==9 && x[8]==10)	return "9-10*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==6 && x[6]==7 && x[7]==9 && x[8]==10)	return "9-11A";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==5 && x[5]==6 && x[6]==8 && x[7]==9 && x[8]==10)	return "9-11B";
         if (x[1]==1 && x[2]==2 && x[3]==4 && x[4]==5 && x[5]==6 && x[6]==8 && x[7]==9 && x[8]==10)	return "9-12*S4";

      case 10:
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6 && x[7]==7 && x[8]==8 && x[9]==9)	return "10-1*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6 && x[7]==7 && x[8]==8 && x[9]==10)	return "10-2*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6 && x[7]==7 && x[8]==9 && x[9]==10)	return "10-3*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6 && x[7]==8 && x[8]==9 && x[9]==10)	return "10-4*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==7 && x[7]==8 && x[8]==9 && x[9]==10)	return "10-5*";
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==6 && x[6]==7 && x[7]==8 && x[8]==9 && x[9]==10)	return "10-6*S6";

      case 11:
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6 && x[7]==7 && x[8]==8 && x[9]==9 && x[10]==10)	return "11-1*";

      case 12:
         if (x[1]==1 && x[2]==2 && x[3]==3 && x[4]==4 && x[5]==5 && x[6]==6 && x[7]==7 && x[8]==8 && x[9]==9 && x[10]==10 && x[11]==11)	return "12-1*S1";
   }

   return "unknown";
}



//////////////////////////////
//
// Convert::base12ToNormalForm -- convert a list of MIDI note numbers
//     into a normal form.
//

void Convert::base12ToNormalForm(Array<int>& nform, Array<int>& base12) {
   int i;
   if (base12.getSize() == 0) {
      nform.setSize(0);
      return;
   } else if (base12.getSize() == 1) {
      nform.setSize(1);
      nform[0] = base12[0] % 12;
      return;
   }

   // first sort pitch classes into normal form
   Array<int> pcs(12);
   pcs.setAll(0);
   for (i=0; i<base12.getSize(); i++) {
      pcs[base12[i]%12] = 1;
   }
   Array<int> values(12);
   values.setSize(0);
   for (i=0; i<pcs.getSize(); i++) {
      if (pcs[i]) {
         values.append(i);
      }
   }

   // now find the best rotation
   Array<int> bestanswer(12);
   bestanswer.setSize(1);
   bestanswer[0] = 0;
   int test;
   int min = values.last() - values[0];
   int sizem1 = values.getSize() - 1;
   int asize = sizem1 + 1;
   for (i=1; i<values.getSize(); i++) {
      test = ((values[(i+sizem1)%asize] - values[i])+144)%12;
      if (test < min) {
         bestanswer.setSize(0);
         bestanswer.append(i);
         min = test;
      } else if (test == min) {
         bestanswer.append(i);
      }
   }

   int bestone;
   if (bestanswer.getSize() == 1) {
      bestone = bestanswer[0];
   } else {
      bestone = findBestNormalRotation(values, values.getSize()-1, bestanswer);
   }

   // store the final result
   nform.setSize(values.getSize());
   int thesize = values.getSize();
   for (i=0; i<nform.getSize(); i++) {
      nform[i] = values[(i+bestone)%thesize];
   }
}



//////////////////////////////
//
// Convert::findBestNormalRotation -- used with base12ToNormalForm().
//

int Convert::findBestNormalRotation(Array<int>& input, int asize, 
      Array<int>& choices) {
   Array<int> newchoices(choices.getSize());
   newchoices.setSize(0);

   int test;
   newchoices.setSize(1);
   newchoices[0] = choices[0];
   int sizem1 = asize - 1;
   int thesize = input.getSize();
   int min = (input[(choices[0]+sizem1)%thesize] - input[choices[0]] + 144)%12;

   int i;
   for (i=1; i<choices.getSize(); i++ ) {
      test = (input[(choices[i]+sizem1)%thesize] - input[choices[i]] + 144)%12;
      if (test < min) {
         newchoices.setSize(0);
         newchoices.append(choices[i]);
         min = test;
      } else if (test == min) {
         newchoices.append(choices[i]);
      }
   }

   if (newchoices.getSize() == 1) {
      return newchoices[0];
   }

   if (asize <= 2) {
      // Case which occurs only when there is tie due to
      // rotataional symmetry, so just choose the first one 
      return newchoices[0];
   }

   return Convert::findBestNormalRotation(input, asize-1, newchoices);
}




// md5sum: 8536ad8281c4b63bd8433449c6400194 Convert.cpp [20050403]
