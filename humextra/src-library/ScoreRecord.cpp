//
// Copyright 2002,2009 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Feb 11 19:48:50 PST 2002
// Last Modified: Sun Mar 24 12:10:00 PST 2002 Small changes for visual C++.
// Last Modified: Sat Mar 21 14:28:55 PST 2009
// Last Modified: Thu Jul 26 20:46:26 PDT 2012 Fixed text object reading.
// Last Modified: Fri Aug 24 09:57:53 PDT 2012 Renovated.
// Filename:      ...sig/src/sigInfo/ScoreRecord.cpp
// Web Address:   http://sig.sapp.org/include/sigInfo/ScoreRecord.cpp
// Syntax:        C++ 
//
// Description:   A SCORE object class.  Inherits basic parameter
//                management from ScoreParameters class.  This class adds
//                higher-level functionality to the parameters.
//

#include "ScoreRecord.h"
#include "PerlRegularExpression.h"
#include "Convert.h"

#include <stdio.h>
#include <string.h>

#ifndef OLDCPP
   #include <iostream>
   #include <iomanip>
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #include <iostream.h>
   #include <iomanip.h>
   #ifdef VISUAL
      #include <strstrea.h>
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif


///////////////////////////////////////////////////////////////////////////
//
// Basic maintenance fuctions.
//
   

//////////////////////////////
//
// ScoreRecord::ScoreRecord -- Constructors
//

ScoreRecord::ScoreRecord(void) { 
   clearAnalysisVariables();
   textFont = NULL;
}


ScoreRecord::ScoreRecord(ScoreRecord& a) {
   pageIndex         = a.pageIndex;
   systemLineIndex   = a.systemLineIndex;
   systemStaffIndex  = a.systemStaffIndex;
   pagePrintPos      = a.pagePrintPos;
   lineStaffPos      = a.lineStaffPos;
   lineSystemPos     = a.lineSystemPos;
   pageStaffPos      = a.pageStaffPos;
   pageSystemPos     = a.pageSystemPos;
   pagesetStaffPos   = a.pagesetStaffPos;
   pagesetSystemPos  = a.pagesetSystemPos;
   lineBeat          = a.lineBeat;
   pageBeat          = a.pageBeat;
   pagesetBeat       = a.pagesetBeat;
   if (a.textFont == NULL) {
      textFont = NULL;
   } else {
      textFont = new char[strlen(a.textFont)+1];
      strcpy(textFont, a.textFont);
   }


   // copy ScoreParameter data?
}



//////////////////////////////
//
// ScoreRecord::~ScoreRecord -- Deconstructor
//

ScoreRecord::~ScoreRecord(void) { 
   clearAnalysisVariables();
}



//////////////////////////////
//
// ScoreRecord::operator= --
//

ScoreRecord& ScoreRecord::operator=(ScoreRecord& a) {
   if (&a== this) {
      return *this;
   }

   ScoreParameters& sp = *this;
   sp = a;

   pageIndex         = a.pageIndex;
   systemLineIndex   = a.systemLineIndex;
   systemStaffIndex  = a.systemStaffIndex;
   pagePrintPos      = a.pagePrintPos;
   lineStaffPos      = a.lineStaffPos;
   lineSystemPos     = a.lineSystemPos;
   pageStaffPos      = a.pageStaffPos;
   pageSystemPos     = a.pageSystemPos;
   pagesetStaffPos   = a.pagesetStaffPos;
   pagesetSystemPos  = a.pagesetSystemPos;
   lineBeat          = a.lineBeat;
   pageBeat          = a.pageBeat;
   pagesetBeat       = a.pagesetBeat;
   if (a.textFont != NULL) {
      if (textFont != NULL) {
         delete [] textFont;
      }
      textFont = new char[strlen(a.textFont)+1];
      strcpy(textFont, a.textFont);
   } else {
      textFont = NULL;
   }

   // copy ScoreParameter data?

   return *this;
}



//////////////////////////////
//
// ScoreRecord::clearAnalysisVariables --
//

void ScoreRecord::clearAnalysisVariables(void) {

   pageIndex         = -1;
   systemLineIndex   = -1;
   systemStaffIndex  = -1;
   pagePrintPos      = -1;
   lineStaffPos      = -1;
   lineSystemPos     = -1;
   pageStaffPos      = -1;
   pageSystemPos     = -1;
   pagesetStaffPos   = -1;
   pagesetSystemPos  = -1;
   lineBeat          = -1.0;
   pageBeat          = -1.0;
   pagesetBeat       = -1.0;
}



//////////////////////////////
//
// ScoreRecord::clear --
//

void ScoreRecord::clear(void) {
   clearAnalysisVariables();
   ScoreParameters::clear();
}


//
// Basic maintenance fuctions.
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
// Printing functions.
//

//////////////////////////////
//
// ScoreRecord::printAscii -- print the parameter data in ASCII text
//     format.
//     default value: roundQ   = 1
//     default value: verboseQ = 0
//

void ScoreRecord::printAscii(ostream& out, int roundQ, int verboseQ, 
         const char* indent) { 

   if (getFixedSize() < 1) {
      return;
   }

   shrink();
   union { float num; unsigned char ch[4]; } u;
   static char buffer[128] = {0};

   double num;
   int i, j;
   int tcount = 0;
   int tcorr  = 0;
   int bufflen;
   for (i=0; i<getFixedSize(); i++) {
      if (isTextItem() && (i==2) && (fabs(getValue(i)) < 0.001)) {
         sprintf(buffer, "%f", 0.001);
         // there is a bug in WinScore with text at position 0.0000...
      } else {
         num = getValue(i);
         if (roundQ) {
            if (num < 0) {
               num = int(num * 1000.0 - 0.5)/1000.0;
            } else {
               num = int(num * 1000.0 + 0.5)/1000.0;
            }
            //if (i == 2) {
            //   num = int(num * 1000.0 + 0.5)/1000.0;
            //} else if ((i == 5) && 
            //      ( (int(getValue(0)) == 4) ||
            //        (int(getValue(0)) == 5) ||
            //        (int(getValue(0)) == 6)
	    //      ) ) {
            //   // round ending of lines, slurs and beams to nearest 1/1000th
            //   num = int(num * 1000.0 + 0.5)/1000.0;
            //}
         }

         sprintf(buffer, "%f", num);

         u.num = (float)atof(buffer);
         if (u.num != num) {
            SSTREAM stream;
            stream.precision(8);
            stream << num;
            stream << ends;
            strcpy(buffer, stream.CSTRING);
         } 
         u.num = (float)atof(buffer);
      }
      // if (verboseQ) {
      //    if (u.num != getValue(i)) {
      //       cout << "XXX:";
      //    }
      // }
      adjustBuffer(buffer);
      if (strchr(buffer, 'e') != NULL) {
         strcpy(buffer, "0.00000");
      }

      bufflen = strlen(buffer);

      switch (i) {
         case P1: 
            if ((int)getValue(0) == 16) {
               out << "t    ";
            } else {
               out << buffer;
	       if (bufflen == 2) {
                  out << ' ';
               }
               out << "  ";
		         
            }
            if (i < getFixedSize()-1) {
               out << ' ';
            }
            break;
         case P2: 
            tcount = 0;
            if (getValue(i) >= 0) {
	       tcount++;	
               if (tcorr > 0) { tcorr--; } else { out << ' '; }
            }
            if (getValue(i) < 10) {
	       tcount++;	
               if (tcorr > 0) { tcorr--; } else { out << ' '; }
            }
            out << buffer;
            if (i < getFixedSize()-1) {
               out << ' ';
            }
            if (i < getFixedSize()-1) {
               for (j=0; j<5 - bufflen - tcount; j++) {
                  out << ' ';
               }
               out << ' ';
            }
	    if (5 - bufflen - tcount < 0) {
               tcorr = -(5-bufflen-tcount);
            }
            break;
         case P3: 
	    tcount = 0;
            cout << setiosflags(ios::left);
            if (getValue(i) < 10.0) {
               out << ' ';
	       tcount++;
            }
            if (getValue(i) < 100.0) {
               out << ' ';
	       tcount++;
            }
            out << buffer;
            if (i<getFixedSize()-1) {
               for (j=0; j<11 - bufflen - tcount; j++) {
                  out << ' ';
               }
               out << ' ';
            }
	    if (11 - bufflen - tcount < 0) {
               tcorr = -(11-bufflen - tcount);
            }
            break;
         default: 
	    tcount = 0;
            if (getValue(i) >= 0) {
	       tcount++;	
               if (tcorr > 0) { tcorr--; } else { out << ' '; }
            }
            if (fabs(getValue(i)) < 10) {
	       tcount++;	
               if (tcorr > 0) { tcorr--; } else { out << ' '; }
            }
            if (fabs(getValue(i)) < 100) {
	       tcount++;	
               if (tcorr > 0) { tcorr--; } else { out << ' '; }
            }
            out << buffer;
            if (i<getFixedSize()-1) {
               for (j=0; j<11 - bufflen - tcount; j++) {
                  out << ' ';
               }
               out << ' ';
            }
	    if (11 - bufflen - tcount < 0) {
               tcorr = -(11-bufflen-tcount);
            }
      }
   }

   if (isTextItem()) {
      out << "\n" << indent << getTextData();
   }

   // leave printing of newline to calling function unless this is a verbose
   // printing:

   if (!verboseQ) {
      return;
   }

   out << endl;
   if (hasExtraParameters()) {
      printAsciiExtraParameters(out, roundQ, verboseQ, indent);
   }
}


void ScoreRecord::printAscii(char* buffer1, char* buffer2, int max, 
      int roundQ, int verboseQ, const char* indent) {

   SSTREAM stream;
   printAscii(stream, roundQ, verboseQ, indent);
   stream << ends;

   int length = strlen(stream.CSTRING);
   if (length > max) {
      cerr << "ERROR: Score Record too long" << endl;
      exit(1);
   }
   strcpy(buffer1, stream.CSTRING);
   char* ptr = strchr(buffer1, '\n');
   if (ptr != NULL) {
      strcpy(buffer2, ptr+1);
      *ptr = '\0';
   } else {
      buffer2[0] = '\0';
   }

   //length = strlen(buffer1);
   //if (!isprint(buffer1[length-1])) {
   //   buffer1[length-1] = '\0';
   //}
   //buffer1[length-1] = '\0';
   //buffer1[length-2] = '\0';

}



//////////////////////////////
//
// ScoreRecord::printAsciiExtraParameters --
//

void ScoreRecord::printAsciiExtraParameters(ostream& out, int roundQ, int verboseQ,
      const char* indent) {
   char buffer[32] ={0};
   int i;
   for (i=0; i<keyParameters.getSize(); i++) {
      if (keyParameters[i].isInvalid()) {
         continue;
      }
      out << indent << "@" << getKeyName(i) << ":\t" << getKeyValue(i);
      if (strcmp(getKeyName(i), "base40") == 0) {
         out << "\t(" << Convert::base40ToMuse(getKeyValue(i), buffer) << ")";
      }
      out << endl;
   }
}




//////////////////////////////
//
// ScoreRecord::setTextFont -- set the font of a SCORE record.  This is 
// stored in a separate location from the P16 text.  When printing, if this 
// font string is set, then print this font and suppress any font at the 
// start of the actual text.
//

void ScoreRecord::setTextFont(const char* aString) {
   if (textFont != NULL) {
      delete [] textFont;
   }
   if (aString == NULL) {
      textFont = NULL;
      return;
   }
   textFont = new char[strlen(aString)+1];
   strcpy(textFont, aString);
}



//////////////////////////////
//
// ScoreRecord::writeBinary -- print the parameter data in Binary
//    format, as found in a SCORE .mus file where the number of numbers
//    to be written is given first.
//

int ScoreRecord::writeBinary(ostream& out) { 
   if (getFixedSize() < 1) {
      return 0;
   }

   shrink();

   // WinScore can't understand data fields smaller than 3
   if (getFixedSize() < 3) {
      setPValue(3, 0.0);
   }

   int i;
   if (getValue(0) == P1_Text) {
      // set the length of the text in P12
      int textlen = strlen(getTextData());
      int totaltextlen = textlen;

      setPValue(12, totaltextlen);
      if (getPValue(13) == 0.0) {
         setPValue(13, 0.0); // needed for .MUS files to parse in WinScore
      }
      int spaces = totaltextlen % 4;
      int charbytes = totaltextlen / 4;
      if (spaces != 0) {
         spaces = 4 - spaces;
         charbytes++;
      }

      writeLittleEndian(out, (float)(getFixedSize() + charbytes));
      for (i=0; i<getFixedSize(); i++) {
         writeLittleEndian(out, getValue(i));
      }

      const char* ptr;
      ptr = getTextData();
      for (i=0; i<textlen; i++) {
         out << ptr[i];
      }

      for (i=0; i<spaces; i++) {
         out << ' ';
      }

      return getFixedSize() + charbytes + 1;
   } else {
      writeLittleEndian(out, (float)getFixedSize());
      for (i=0; i<getFixedSize(); i++) {
         writeLittleEndian(out, getValue(i));
      }
      return getFixedSize() + 1;
   }

}



//////////////////////////////
//
// ScoreRecord::readBinary -- input parameter values from the given
//     input stream.
//

void ScoreRecord::readBinary(istream& instream, int pcount) {
   int i;
   setFixedSize(pcount);
   if (pcount < 1) {
      cout << "Error: invalid parameter count: " << pcount << endl;
      exit(1);
   }
   setValue(0, readLittleEndian(instream));

   if ((int)getValue(0) == 16) {
      setFixedSize(13);
      for (i=1; i<13; i++) {
         setValue(i, 
               ScoreParameters::roundFractionDigits(readLittleEndian(instream), 
               3));
      }
      int count = (int)getPValue(12);
      // char buffer[count+1+16];  // can't be done in MS Visual C++
      static char buffer[1024] = {0}; 
      instream.read(buffer, count);
      buffer[count] = '\0';
      setTextData(buffer);
      int extra = count % 4;
      if (extra > 0) {
         instream.read(buffer, 4-extra);
         buffer[4-extra] = '\0';
         // extra string should be all spaces, but occasionally
         // this is not true.
         //for (i=0; i<4-extra; i++) {
         // if (buffer[i] != 0x20) {
         //   cout << "Error: non-space character found" << endl;
         //   cout << "Found character: \"" << buffer[i] << "\"" << endl;
         //   exit(1);
         // }
         //}
      }
   } else {
      // non-text data parameters
      for (i=1; i<pcount; i++) {
         setValue(i, readLittleEndian(instream));
      }
   }
}



///////////////////////////////
//
// ScoreRecord::writeLittleEndian --
//

void ScoreRecord::writeLittleEndian(ostream& out, double number) {
   float fnumber = (float)number;
   union { float f; unsigned int i; } num;
   num.f = fnumber;
   char byteinfo[4];
   byteinfo[0] = (char)( num.i        & 0xff);
   byteinfo[1] = (char)((num.i >> 8)  & 0xff);
   byteinfo[2] = (char)((num.i >> 16) & 0xff);
   byteinfo[3] = (char)((num.i >> 24) & 0xff);
   out.write(byteinfo, 4);
}



///////////////////////////////
//
// ScoreRecord::readLittleEndian --
//

double ScoreRecord::readLittleEndian(istream& instream) {
   unsigned char byteinfo[4];
   instream.read((char*)byteinfo, 4);
   union { float f; unsigned int i; } num;
   num.i = 0;
   num.i = byteinfo[3];
   num.i = (num.i << 8) | byteinfo[2];
   num.i = (num.i << 8) | byteinfo[1];
   num.i = (num.i << 8) | byteinfo[0];

   return (double)num.f;
}



//////////////////////////////
//
// ScoreRecord::adjustBuffer -- remove all right hand zeros
//

void ScoreRecord::adjustBuffer(char *buffer) {
   int len = strlen(buffer);
   int i;
   for (i=len-1; i>0; i--) {
      if (buffer[i] == '0') {
         buffer[i] = '\0';
         len--;
      } else {
         break;
      }
   }

   // remove trailing decimal point as well for integers:
   if (buffer[len-1] == '.') {
      buffer[len-1] = ' ';
   }
}


//
// Printing functions.
//
///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// ScoreRecord::is... -- boolean for code Item values.
//

int ScoreRecord::isNoteItem   (void){ return (int)getPValue(1) == P1_Note ? 1 : 0;   }
int ScoreRecord::isRestItem   (void){ return (int)getPValue(1) == P1_Rest ? 1 : 0;   }
int ScoreRecord::hasDurationQ (void){ return isNoteItem() || isRestItem(); }
int ScoreRecord::isClefItem   (void){ return (int)getPValue(1) == P1_Clef ? 1 : 0;   }
int ScoreRecord::isLineItem   (void){ return (int)getPValue(1) == P1_Line ? 1 : 0;   }
int ScoreRecord::isSlurItem   (void){ return (int)getPValue(1) == P1_Slur ? 1 : 0;   }
int ScoreRecord::isBeamItem   (void){ return (int)getPValue(1) == P1_Beam ? 1 : 0;   }
int ScoreRecord::isTrillItem  (void){ return (int)getPValue(1) == P1_Trill ? 1 : 0;  }
int ScoreRecord::isStaffItem  (void){ return (int)getPValue(1) == P1_Staff ? 1 : 0;  }
int ScoreRecord::isSymbolItem (void){ return (int)getPValue(1) == P1_Symbol ? 1 : 0; }
int ScoreRecord::isNumberItem (void){ return (int)getPValue(1) == P1_Number? 1 : 0;  }
int ScoreRecord::isUserItem   (void){ return (int)getPValue(1) == P1_User? 1 : 0;    }
int ScoreRecord::isShapeItem  (void){ return (int)getPValue(1) == P1_Special? 1 : 0; }
int ScoreRecord::isBarlineItem(void){ return (int)getPValue(1) == P1_Barline? 1 : 0; }
int ScoreRecord::isPostscriptItem(void) 
      { return (int)getPValue(1) == P1_ImportedEPSGraphic? 1 : 0;}
int ScoreRecord::isTextItem(void)   { return (int)getPValue(1) == P1_Text? 1 : 0;}
int ScoreRecord::isKeysigItem(void) 
      { return (int)getPValue(1) == P1_KeySignature? 1 : 0;}
int ScoreRecord::isTimesigItem(void)
      { return (int)getPValue(1) == P1_MeterSignature? 1 : 0;}
int ScoreRecord::isMeterItem(void)  { return isTimesigItem(); }




///////////////////////////////////////////////////////////////////////////
//
// Function for all P1 object types.
//


//////////////////////////////
//
// ScoreRecord::getStaff -- return int value of P2 value of the 
//     object (horizontal position).
//

int ScoreRecord::getStaff(void) {
   return (int)getValue(P2);
}



//////////////////////////////
//
// ScoreRecord::getHpos -- return P3 value of the object (horizontal position).
//     When an object (such as a slur) has two Hposes, only the left side's Hpos
//     is given by this function (P6 usually has the right Hpos in this case).
//

double ScoreRecord::getHpos(void) {
   return getValue(P3);
}



//////////////////////////////
//
// ScoreRecord::getVpos -- return P4 value of the object (vertical position).
//     But remove grace note indication (for notes) which is the 
//     +100, -100 digit.  (also remove anything above 100 or below -100).
//     In other words, limit vpos to +/- 100.  When an object (such as a slur)
//     has a left and right end, this function returns only the left Vpos
//     of the object (P5 usually has the right Vpos in this case).
//

double ScoreRecord::getVpos(void) {
   double vpos = getValue(P4);
   double sign = +1.0;
   if (vpos < 0.0) {
      sign = -1.0;
      vpos = -vpos;
   }
   int hundreds = ((int)(vpos/100))*100;
   vpos = vpos - hundreds;
   return sign * vpos;
}


//
// Function for all P1 object types.
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
// Note functions (P1=1).
//


//////////////////////////////
//
// getStaffOffset -- Return the P12 parameter of P1=1 (notes).  The P12
//     value is mapped to actual staff offsets:
//        0 =  0
//        1 = +1
//        2 = -1
//

int  ScoreRecord::getStaffOffset(void) {
   if (!isNoteItem()) {
      return 0;
   }
   int offset = (int)getValue(P12);
   switch (offset) {
      case 0:  return  0;
      case 1:  return +1;
      case 2:  return -1;
      default: return 0;
   }
}



//////////////////////////////
//
// ScoreRecord::getStemDirection -- Return +1 if note has stem up, -1 if 
//       stem down, and 0 otherwise.
//

int ScoreRecord::getStemDirection(void) {
   if (!isNoteItem()) {
      return 0;
   }

   int value = getValue(P5, 1);
   if (value == 1) {
      return 1;  // stem up
   } else if (value == 2) {
      return -1;
   }
  
   return 0;
}



//////////////////////////////
//
// ScoreRecord::getPrintedAccidental -- returns the 1's digit of P5
//    0 = no printed accidental 
//    1 = printed flat accidental 
//    2 = printed sharp accidental 
//    3 = printed natural accidental 
//    4 = printed double flat accidental 
//    5 = printed double sharp accidental 
//    6-9 = quarter-sharp/flat accidentals
//

int ScoreRecord::getPrintedAccidental(void) {
   if (!isNoteItem()) { 
      return 0;
   }
   int value = getValue(P5);
   if (value < 0) {
      value = -value;
   }
   return (int)value % 10;
}



//////////////////////////////
//
// ScoreRecord::hasPrinted* -- Returns true for various types of printed 
//      accidentals.
//
 
int ScoreRecord::hasPrintedAccidental(void)  
      { return getPrintedAccidental() ? 1 : 0; }
int ScoreRecord::hasPrintedFlat(void)        
      { return getPrintedAccidental() == 1 ? 1 : 0; }
int ScoreRecord::hasPrintedSharp(void)       
      { return getPrintedAccidental() == 2 ? 1 : 0; }
int ScoreRecord::hasPrintedNatural(void)     
      { return getPrintedAccidental() == 3 ? 1 : 0; }
int ScoreRecord::hasPrintedDoubleFlat(void)  
      { return getPrintedAccidental() == 4 ? 1 : 0; }
int ScoreRecord::hasPrintedDoubleSharp(void) 
      { return getPrintedAccidental() == 5 ? 1 : 0; }



//////////////////////////////
//
// ScoreRecord::hasCautionary -- Returns if the pitchAnalysis() function
//      marked the note has having a cautionary accidental.
//

int ScoreRecord::hasCautionary(void) {
   return (int)getValue("cautionary");
}



//////////////////////////////
//
// ScoreRecord::getAccidental -- return the 1's digit of P5
//    0 ==  0 (no accidental)
//    1 == -1 (flat)
//    2 == -1 (sharp)
//    3 ==  0 (natural)
//    4 == -2 (double flat)
//    5 == +2 (double sharp)
//    deal with quarter-tones later (notice the double return value)...
//

double ScoreRecord::getAccidental(void) {
   int ones = (int)(getValue(P5)) % 10;
   switch (ones) {
      case 0: return  0.0;
      case 1: return -1.0;
      case 2: return +1.0;
      case 3: return  0.0;
      case 4: return -2.0;
      case 5: return +2.0;
   }

   return 0.0;
}



//////////////////////////////
//
// ScoreRecord::editorialAccidental --
//    True if there is an editorial accidental. 
//    1 = flat
//    2 = sharp
//    3 = natural
//

int ScoreRecord::hasEditorialAccidental(void) {
   int artic = (int)(getValue(P11));
   switch (artic) {
      case 1:  return 1;
      case 2:  return 2;
      case 3:  return 3;
      default: return 0;
   }
}



//////////////////////////////
//
// ScoreRecord::hasNatural -- returns true if there is a natural
//    sign on the note.  The natural sign may be displayed as an
//    editorial marking.
//

int ScoreRecord::hasNatural(void) {
   int artic = (int)(getValue(P11));
   if (artic == 3) {
      return 1; 
   } else if (artic == 1) {
      return 0;
   } else if (artic == 2) {
      return 0;
   }
   int ones  = (int)(getValue(P5)) % 10;
   if (ones == 3) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// ScoreRecord::getPitch --  Get the analytic pitch as Base-40 number.
//

int ScoreRecord::getPitch(void) { 
   return (int)getValue("base40");
}



//////////////////////////////
//
// ScoreRecord::setPitch -- Set the analytic pitch as a Base-40 number.
//    In general this function should not be used, since the pitch
//    will be set automatically by analyzePitch() in ScorePage/ScorePageSet 
//    classes.
//

void ScoreRecord::setPitch(int aPitch) { 
   setValue("base40", aPitch);
}


//
// Note functions (P1=1).
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
// Functions shared by Notes and Rests (P1 = 1 or 2)
//

//////////////////////////////
//
// ScoreRecord::getDuration --  Return the P7 value of a note/rest.  If the 
//     P7 value is greater than 60 or negative, then the note/rest is
//     considered a grace note/rest.
//

double ScoreRecord::getDuration(void) {
   if (!hasDuration()) {
      return 0.0;
   }
   double p7value  = getValue(P7);
   if (p7value < 0.0) {
      return 0.0;
   }

   // handle gracenotes which have a duration of 0.0 by P7=65:
   if (p7value > 60.0) {
      return 0.0;
   }

   return p7value;
}



//////////////////////////////
//
// ScoreRecord::hasDuration -- Returns true if the item posseses duration.
//    Such as it is a note or a rest.  The duration can be zero (such
//    as for gracenotes).
//

int ScoreRecord::hasDuration(void) {
   if (isNoteItem() || isRestItem()) {
      return 1;
   } else {
      return 0;
   }
}


//
// Functions shared by Notes and Rests (P1 = 1 or 2)
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
// Line functions (P1=4).
//


//////////////////////////////
//
// ScoreRecord::isHairpin -- Return true if a hairpin crescendo/descrescendo.
//

int ScoreRecord::isHairpin(void) {
   if (!isLineItem()) {
      return 0;
   }
   // hairpins have P5=999
   if ((int)getValue(P5) != 999) {
      return 0;
   }
   if (((int)getValue(P7) == 0) || ((int)getValue(P7) == -1)) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// ScoreRecord::isHairpinDescrescendo --
//

int ScoreRecord::isHairpinDecrescendo(void) {
   if (!isLineItem()) {
      return 0;
   }

   // hairpins have P5=999
   if ((int)getValue(P5) != 999) {
      return 0;
   }

   // decrescendo has P7 = -1
   if ((int)getValue(P7) == -1) {
      return 1;
   } else {
      return 0;
   }

}



//////////////////////////////
//
// ScoreRecord::isHairpinCrescendo --
//

int ScoreRecord::isHairpinCrescendo(void) {
   if (!isLineItem()) {
      return 0;
   }

   // hairpins have P5=999
   if ((int)getValue(P5) != 999) {
      return 0;
   }
   // crescendo has P7 = 0
   if ((int)getValue(P7) == 0) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// ScoreRecord::isWavyLine --
//

int ScoreRecord::isWavyLine(void) {
   if (!isLineItem()) {
      return 0;
   }

   // only hairpins have P5=999
   if ((int)getValue(P5) == 999) {
      return 0;
   }

   // wavy line has P7 = -1
   if ((int)getValue(P7) == -1) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// ScoreRecord::isDashedLine -- True if line is dashed.
//

int ScoreRecord::isDashedLine(void) {
   if (!isLineItem()) {
      return 0;
   }

   // only hairpins have P5=999
   if ((int)getValue(P5) == 999) {
      return 0;
   }

   // dashed line has P7 = 1
   if ((int)getValue(P7) == 1) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// ScoreRecord::isPlainLine -- True if just a normal solid line.
//

int ScoreRecord::isPlainLine(void) {
   if (!isLineItem()) {
      return 0;
   }

   // only hairpins have P5=999
   if ((int)getValue(P5) == 999) {
      return 0;
   }

   // dashed line has P7 = 0
   if ((int)getValue(P7) == 0) {
      return 1;
   } else {
      return 0;
   }
}


//
// Line functions (P1=4).
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
// Functions for Slur items (P1=5)
//


//////////////////////////////
//
// ScoreRecord::setTieStart --
//

void ScoreRecord::setTieStart(int objidx) {
   setValue("tiestart", objidx);
}



//////////////////////////////
//
// ScoreRecord::setTieEnd --
//

void ScoreRecord::setTieEnd(int objidx) {
   setValue("tieend", objidx);
}



//////////////////////////////
//
// ScoreRecord::setTie -- Mark a slur item as a tie and store the two 
//    notes that it ties together (as index numbers for the page).
//

void ScoreRecord::setTie(int leftnote, int rightnote) {
   setValue("tie", 1);
   setValue("tiestart", leftnote);
   setValue("tieend",   rightnote);
}



//////////////////////////////
//
// ScoreRecord::hasTieStart --
//

int ScoreRecord::hasTieStart(void) {
   if ((getValue("tiestart") > 0) && (!getValue("tieend"))) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// ScoreRecord::hasTieEnd --
//

int ScoreRecord::hasTieEnd(void) {
   if ((getValue("tieend") > 0) && (!getValue("tiestart"))) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// ScoreRecord::hasTieContinue --
//

int ScoreRecord::hasTieContinue(void) {
   if ((getValue("tieend") > 0) && (getValue("tiestart") > 0)) {
      return 1;
   } else {
      return 0;
   }
}


//
// Functions for Slur items (P1=5)
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
// Staff functions (P1=8).
//


//////////////////////////////
//
// ScoreRecord::isInvisibleStaff --  Returns true if P7 is -1 and a staff item.
//

int ScoreRecord::isInvisibleStaff(void) {
   if (!isStaffItem()) {
      return 0;
   }
   if (getValue(P7) == -1) {
      return 1;
   } else {
      return 0;
   }
}

//
// Staff functions (P1=8).
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//
// barline functions (P1=14).
//

//////////////////////////////
//
// ScoreReord::getBarHeight -- Return how many staves the staff connects
//    to.  This is the P4 value.  If P4=0 then the output is 1.
//
int ScoreRecord::getBarHeight(void) {
   if (!isBarlineItem()) {
      return 0;
   }
   int output = (int)getValue(P4);
   if (output == 0) {
      output = 1;
   }
   return output;
}

//
// barline functions (P1=14).
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//
// Lyrics functions.  Related to analyzeLyrics() in ScorePage/ScorePageSet classes.
//


//////////////////////////////
//
// ScoreRecord::getVerseIndex -- Return the "verse" number parameter, offset 
//    from 0 rather than 1. return -1 if there is no "verse" field.
//

int ScoreRecord::getVerseIndex(void) {
   return (int)getValue("verse") - 1;
}



//////////////////////////////
//
// ScoreRecord::getVerseNumber -- Return the "verse" number parameter, offset 
//    from 1 rather than 0. return 0 if there is no "verse" field.
//

int ScoreRecord::getVerseNumber(void) {
   return (int)getValue("verse");
}



//////////////////////////////
//
// ScoreRecord::setVerseIndex -- Set the "verse" number parameter, offset 
//    from 0 for the first verse rather than 1.
//

void ScoreRecord::setVerseIndex(double anIndex) {
   setValue("verse", anIndex);
}



//////////////////////////////
//
// ScoreRecord::setVerseNumber -- Set the "verse" number parameter, offset 
//    from 1 for the first verse rather than 0.
//

void ScoreRecord::setVerseNumber(double aNumber) {
   setValue("verse", (aNumber - 1.0));
}



//////////////////////////////
//
// ScoreRecord::isHyphen -- returns true if a line and previously given non-zero 
//       "hyphen" parameter.
//

int ScoreRecord::isHyphen(void) {
   if (!isLineItem()) {
      return 0;
   }
   return (int)getValue("hyphen");
}



//////////////////////////////
//
// ScoreRecord::isVerse --  Return true if part of a verse.  Could be a text item
//     or a line item which serves as a hyphen or a word extension line.
//

int ScoreRecord::isVerse(void) {
   return hasValue("verse");
}



//////////////////////////////
//
// ScoreRecord::isWordEnd -- Returns true if the ending syllable of a word in lyrics.
//

int ScoreRecord::isWordEnd(void) { 
   return getValue("hyphenafter") ? 0 : 1;
}



//////////////////////////////
//
// ScoreRecord::isWordStart -- Returns true
//

int ScoreRecord::isWordStart(void) { 
   return getValue("hyphenbefore") ? 0 : 1;
}



//////////////////////////////
//
// ScoreRecord::isWordMiddle --
//

int ScoreRecord::isWordMiddle(void) { 
   return getValue("hyphenbefore") && getValue("hyphenafter") ? 1 : 0;
}


//
// Lyrics functions.  Related to analyzeLyrics() in ScorePage/ScorePageSet classes.
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
// Text functions (P1=16).
//


//////////////////////////////
//
// ScoreRecord::getTextDataWithoutFonts -- Remove font codes from a SCORE
//      text string.  This is a static function which can be called without and 
//      class instantiation.
//

Array<char>& ScoreRecord::getTextDataWithoutFonts(Array<char>& textdata) {
   PerlRegularExpression pre;
   textdata.setSize(strlen(getTextData())+1);
   strcpy(textdata.getBase(), getTextData());
   pre.sar(textdata, "_\\d\\d", "", "g");
   return textdata;
}



//////////////////////////////
//
// ScoreRecord::convertScoreTextToHtmlText -- Convert SCORE special characters
//    such as accents into HTML character entities.  Static function.
//

Array<char>& ScoreRecord::convertScoreTextToHtmlText(Array<char>& astring) {
   PerlRegularExpression pre;

   pre.sar(astring, "<<a", "&aacute;", "g");
   pre.sar(astring, "<<e", "&eacute;", "g");
   pre.sar(astring, "<<i", "&iacute;", "g");
   pre.sar(astring, "<<o", "&oacute;", "g");
   pre.sar(astring, "<<u", "&uacute;", "g");
   pre.sar(astring, "<<A", "&Aacute;", "g");
   pre.sar(astring, "<<E", "&Eacute;", "g");
   pre.sar(astring, "<<I", "&Iacute;", "g");
   pre.sar(astring, "<<O", "&Oacute;", "g");
   pre.sar(astring, "<<U", "&Uacute;", "g");

   pre.sar(astring, ">>a", "&agrave;", "g");
   pre.sar(astring, ">>e", "&egrave;", "g");
   pre.sar(astring, ">>i", "&igrave;", "g");
   pre.sar(astring, ">>o", "&ograve;", "g");
   pre.sar(astring, ">>u", "&ugrave;", "g");
   pre.sar(astring, ">>A", "&Agrave;", "g");
   pre.sar(astring, ">>E", "&Egrave;", "g");
   pre.sar(astring, ">>I", "&Igrave;", "g");
   pre.sar(astring, ">>O", "&Ograve;", "g");
   pre.sar(astring, ">>U", "&Ugrave;", "g");

   pre.sar(astring, "%%a", "&auml;", "g");
   pre.sar(astring, "%%e", "&euml;", "g");
   pre.sar(astring, "%%i", "&iuml;", "g");
   pre.sar(astring, "%%o", "&ouml;", "g");
   pre.sar(astring, "%%u", "&uuml;", "g");
   pre.sar(astring, "%%A", "&Auml;", "g");
   pre.sar(astring, "%%E", "&Euml;", "g");
   pre.sar(astring, "%%I", "&Iuml;", "g");
   pre.sar(astring, "%%O", "&Ouml;", "g");
   pre.sar(astring, "%%U", "&Uuml;", "g");

   pre.sar(astring, "##c", "&ccedil;", "g");
   pre.sar(astring, "##C", "&Ccedil;", "g");

   pre.sar(astring, "\\?\\\\", "\\", "g");
   pre.sar(astring, "\\?\\|", "|", "g");
   pre.sar(astring, "\\?\\[", "[", "g");
   pre.sar(astring, "\\?\\]", "]", "g");
   pre.sar(astring, "\\?a", "&aelig;", "g");
   pre.sar(astring, "\\?A", "&AElig;", "g");
   pre.sar(astring, "\\?e", "&oelig;", "g");
   pre.sar(astring, "\\?E", "&OElig;", "g");
   pre.sar(astring, "!a", "&aring;", "g");
   pre.sar(astring, "!A", "&Aring;", "g");
   pre.sar(astring, "~n", "&ntilde;", "g");
   pre.sar(astring, "~N", "&Ntilde;", "g");

   return astring;
}




//////////////////////////////
//
// ScoreRecord::convertScoreTextToPlainText -- Remove special characters are
//    removed to give just simple text string.
//

Array<char>& ScoreRecord::convertScoreTextToPlainText(Array<char>& astring) {
   PerlRegularExpression pre;

   pre.sar(astring, "<<a", "a", "g");
   pre.sar(astring, "<<e", "e", "g");
   pre.sar(astring, "<<i", "i", "g");
   pre.sar(astring, "<<o", "o", "g");
   pre.sar(astring, "<<u", "u", "g");
   pre.sar(astring, "<<A", "A", "g");
   pre.sar(astring, "<<E", "E", "g");
   pre.sar(astring, "<<I", "I", "g");
   pre.sar(astring, "<<O", "O", "g");
   pre.sar(astring, "<<U", "U", "g");

   pre.sar(astring, ">>a", "a", "g");
   pre.sar(astring, ">>e", "e", "g");
   pre.sar(astring, ">>i", "i", "g");
   pre.sar(astring, ">>o", "o", "g");
   pre.sar(astring, ">>u", "u", "g");
   pre.sar(astring, ">>A", "A", "g");
   pre.sar(astring, ">>E", "E", "g");
   pre.sar(astring, ">>I", "I", "g");
   pre.sar(astring, ">>O", "O", "g");
   pre.sar(astring, ">>U", "U", "g");

   pre.sar(astring, "%%a", "ae", "g");
   pre.sar(astring, "%%e", "ee", "g");
   pre.sar(astring, "%%i", "ie", "g");
   pre.sar(astring, "%%o", "oe", "g");
   pre.sar(astring, "%%u", "ue", "g");
   pre.sar(astring, "%%A", "Ae", "g");
   pre.sar(astring, "%%E", "Ee", "g");
   pre.sar(astring, "%%I", "Ie", "g");
   pre.sar(astring, "%%O", "Oe", "g");
   pre.sar(astring, "%%U", "Ue", "g");

   pre.sar(astring, "##c", "c", "g");
   pre.sar(astring, "##C", "C", "g");

   pre.sar(astring, "\\?\\\\", "\\", "g");
   pre.sar(astring, "\\?\\|", "|", "g");
   pre.sar(astring, "\\?\\[", "[", "g");
   pre.sar(astring, "\\?\\]", "]", "g");
   pre.sar(astring, "\\?a", "ae", "g");
   pre.sar(astring, "\\?A", "AE", "g");
   pre.sar(astring, "\\?e", "oe", "g");
   pre.sar(astring, "\\?E", "OE", "g");
   pre.sar(astring, "!a", "a", "g");
   pre.sar(astring, "!A", "A", "g");
   pre.sar(astring, "~n", "n", "g");
   pre.sar(astring, "~N", "N", "g");

   return astring;
}


//
// Text functions (P1=16).
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
// Indexing functions
//


//
// PageSet-level indexing functions
//


//////////////////////////////
//
// ScoreRecord::setPageIndex -- set the page index number of the ScoreRecord
//    on the page to which it belongs.  This is usually handled automatically
//    in the ScorePageSet class when reading a set of pages.  Pages are indexed
//    starting at 0.
//

void ScoreRecord::setPageIndex(int pageidx) {
   pageIndex = pageidx;
}



//////////////////////////////
//
// ScoreRecord::getPageIndex -- Get the page index number of the ScoreRecord
//    for the page to which it belongs.  Returns -1 if the page index is 
//    undefined.
//

int ScoreRecord::getPageIndex(void) {
   return pageIndex;
}



//////////////////////////////
//
// ScoreRecord::get/setPageSetSystemPosition -- Return the index position of the
//    item within the continuous system sequence over multiple pages.  The starting
//    position is the left-most item on the top system on the first page, and the
//    last position is the right-most item on the bottom system of the last page.
//    All staves in the system are interleaved by horizontal position, and
//    then secondary sort by vertical position (from low to high) and then
//    tertiary sort by P1 code item type.
//

int ScoreRecord::getPageSetSystemPosition(void) {
   return pagesetSystemPos;
}


int ScoreRecord::getPageSetSystemPos(void) {
   return getPageSetSystemPosition();
}


void ScoreRecord::setPageSetSystemPosition(int objidx) {
   pagesetSystemPos = objidx;
}


void ScoreRecord::setPageSetSystemPos(int objidx) {
   setPageSetSystemPosition(objidx);
}



//////////////////////////////
//
// ScoreRecord::getPageSetStaffPosition --  Similar to getPageSetSystemPosition(), but
//      index only a single staff.
//

int ScoreRecord::getPageSetStaffPosition(void) {
   return pagesetStaffPos;
}


int ScoreRecord::getPageSetStaffPos(void) {
   return getPageSetStaffPosition();
}


void ScoreRecord::setPageSetStaffPosition(int objidx) {
   pagesetStaffPos = objidx;
}


void ScoreRecord::setPageSetStaffPos(int objidx) {
   setPageSetStaffPosition(objidx);
}



//////////////////////////////
//
// ScoreRecord::getPageSetPartPosition -- An alias for getPagePartStaffPosition().  A
// continuous index for a part across multiple system line breaks on the page.
// When a part is represeted by a single staff, then this is equivalent to
// a list of SCORE items for a particular part on page across multiple line breaks.
//

int ScoreRecord::getPageSetPartPosition(void) {
   return getPageSetStaffPosition();
}


int ScoreRecord::getPageSetPartPos(void) {
   return getPageSetStaffPosition();
}


//
// Page-level indexing functions.  These functions treat multiple lines
// of systems on the page as a single continuous function.
//


//////////////////////////////
//
// ScoreRecord::getPageSystemPosition -- Return the index position of the
//    item within the continuous system sequence on the page.  The starting
//    position is the left-most item on the top system on the page, and the
//    last position is the right-most item on the bottom system of the page.
//    All staves in the system are interleaved by horizontal position, and
//    then secondary sort by vertical position (from low to high) and then
//    tertiary sort by P1 code item type.
//

int ScoreRecord::getPageSystemPosition(void) {
   return pageSystemPos;
}


int ScoreRecord::getPageSystemPos(void) {
   return getPageSystemPosition();
}


void ScoreRecord::setPageSystemPosition(int objidx) {
   pageSystemPos = objidx;
}


void ScoreRecord::setPageSystemPos(int objidx) {
   setPageSystemPosition(objidx);
}



//////////////////////////////
//
// ScoreRecord::getPageStaffPosition --  Similar to getPageSystemPosition(), but
//      index only a single staff.
//

int ScoreRecord::getPageStaffPosition(void) {
   return pageStaffPos;
}


int ScoreRecord::getPageStaffPos(void) {
   return getPageStaffPosition();
}


void ScoreRecord::setPageStaffPosition(int objidx) {
   pageStaffPos = objidx;
}


void ScoreRecord::setPageStaffPos(int objidx) {
   setPageStaffPosition(objidx);
}



//////////////////////////////
//
// ScoreRecord::getPagePartPosition -- An alias for getPageStaffPosition().  A
// continuous index for a part across multiple system line breaks on the page.
// When a part is represeted by a single staff, then this is equivalent to
// a list of SCORE items for a particular part on page across multiple line breaks.
//

int ScoreRecord::getPagePartPosition(void) {
   return getPageStaffPosition();
}


int ScoreRecord::getPagePartPos(void) {
   return getPageStaffPosition();
}


//////////////////////////////
//
// ScoreRecord::getSystemStaffIndex -- Returns the staff index on the system.  The
//     system staff index works for the system on a line, the system on a page and
//     the system in a pageset.
//

int ScoreRecord::getSystemStaffIndex(void) {
   return systemStaffIndex;
}


void ScoreRecord::setSystemStaffIndex(int anIndex) {
   systemStaffIndex = anIndex;
}


//
// Line-level indexing functions.  Single staff lines on the page are indexed
// as they are in SCORE: starting at the bottom of the page with #1, and 
// going upwards on the page (typically to a maximum allowed of 32 in
// DOS SCORE, or 99 otherwise.  This information is stored intrinsically
// in each SCORE item record.  Use getStaff() to access the staff number 
// (also getValue(P4) with a caveat of removing the 100's digits).
//

//
// System-Lines are indexed from 0, starting at the top of the page so 
// that they are causal.
//


//////////////////////////////
//
// ScoreRecord::getLineSystemPosition -- Return the position in the list of 
//    objects on a particular system line.
//

int ScoreRecord::getLineSystemPosition(void) {
   return lineSystemPos;
}


int ScoreRecord::getLineSystemPos(void) {
   return getLineSystemPosition();
}


void ScoreRecord::setLineSystemPosition(int objidx) {
   lineSystemPos = objidx;
}


void ScoreRecord::setLineSystemPos(int objidx) {
   setLineSystemPosition(objidx);
}



//////////////////////////////
//
// ScoreRecord::getLineStaffPosition -- Return the position in the list of 
//    objects on a particular system line.
//

int ScoreRecord::getLineStaffPosition(void) {
   return lineSystemPos;
}


int ScoreRecord::getLineStaffPos(void) {
   return getLineStaffPosition();
}


void ScoreRecord::setLineStaffPosition(int objidx) {
   lineSystemPos = objidx;
}


void ScoreRecord::setLineStaffPos(int objidx) {
   setLineStaffPosition(objidx);
}


///////////////////////////////////////////////////////////////////////////
//
// Static functions
//


//////////////////////////////
//
// ScoreRecord::getDiatonicAccidentalState -- Static function which returns 
//     an array of seven integers representing the accidental state of each
//     diatonic pitch in this order:  C, D, E, F, G, A, B.
//     Return value is the key signature count, such as -4 for four flats.
//

int ScoreRecord::getDiatonicAccidentalState(Array<int>& states, 
      ScoreRecord& arecord) {
   if (!arecord.isKeysigItem()) {
      return -1000;
   }
   states.setSize(7);
   states.allowGrowth(0);
   states.setAll(0);

   int keyval = (int)arecord.getValue(P5);

   // If 100's digit of P5 is 1, then the key signature
   // is a cancellation signature, with all values natural
   if ((keyval >= 100) && (keyval < 200)) {
      keyval = 0;
   } else if ((keyval <= -100) && (keyval > -200)) {
      keyval = 0;
   }

   if (keyval == 0) {
      // nothing else left to do
      return keyval;
   }

   if (keyval < 0) {
      if (keyval <= -1) { states[6] = -1; } // B-flat
      if (keyval <= -2) { states[2] = -1; } // E-flat
      if (keyval <= -3) { states[5] = -1; } // A-flat
      if (keyval <= -4) { states[1] = -1; } // D-flat
      if (keyval <= -5) { states[4] = -1; } // G-flat
      if (keyval <= -6) { states[0] = -1; } // C-flat
      if (keyval <= -7) { states[3] = -1; } // F-flat
   } else if (keyval > 0) {
      if (keyval >= +1) { states[3] = +1; } // F-sharp
      if (keyval >= +2) { states[0] = +1; } // C-sharp
      if (keyval >= +3) { states[4] = +1; } // G-sharp
      if (keyval >= +4) { states[1] = +1; } // D-sharp
      if (keyval >= +5) { states[5] = +1; } // A-sharp
      if (keyval >= +6) { states[2] = +1; } // E-sharp
      if (keyval >= +7) { states[6] = +1; } // B-sharp
   }

   return keyval;
}



//////////////////////////////
//
// ScoreRecord::getMiddleCVpos -- return the vertical position of middle C given
//     a particular clef.  For treble clef, middle C is at vpos 1; for bass
//     clef middle C is at vpos 13, etc.
//

int ScoreRecord::getMiddleCVpos(ScoreRecord& arecord) {
   int clefval = (int)arecord.getValue(P5);
   int p4value = (int)arecord.getValue(P4);
   float cleffrac = arecord.getValue(P5) - clefval;
   int fractens = int(cleffrac*10.0 + 0.5);

   int output = 1;  // default for treble clef

   if (clefval == 0) {          // treble clef
      output += p4value;
   } else if (clefval == 1)  {  // bass clef
      output = 13;
      output += p4value;
   } else if (clefval == 2) {   // alto clef
      output = 7;
      output += p4value;
   } else if (clefval == 3) {   // tenor clef
      output = 9;
      output += p4value;
   }

   // deal with soprano clef, mezzo-soprano clef, baritone clef, etc here.

   // handle clefs (such as vocal tenor clef) which transpose
   // their music and octave lower.
   if (fractens == 8) {
      output += 7;
   }

   return output;
}


// md5sum: 9316e26ed432152361bd0ff02f6e024c ScoreRecord.cpp [20050403]
