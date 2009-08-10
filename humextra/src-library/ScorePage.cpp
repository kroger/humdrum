//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Feb 14 23:40:51 PST 2002
// Last Modified: Mon Feb 25 18:27:30 PST 2002
// Last Modified: Sun Mar 24 12:10:00 PST 2002 (small changes for visual c++)
// Last Modified: Tue Mar 26 00:38:28 PST 2002 (added staff access functions)
// Last Modified: Fri Apr 12 18:09:11 PDT 2002 (added sorting by system)
// Filename:      ...sig/src/sigInfo/ScorePage.h
// Web Address:   http://sig.sapp.org/include/sigInfo/ScorePage.h
// Syntax:        C++ 
//
// Description:   Data structure for a page of SCORE data, with added
//                analysis functions.
//

#include "ScorePage.h"

#ifndef OLDCPP
   using namespace std;
#endif



//////////////////////////////
//
// ScorePage::ScorePage --
//

ScorePage::ScorePage(void) : ScorePageBase() { 
   // do nothing
}



//////////////////////////////
//
// ScorePage::~ScorePage --
//

ScorePage::~ScorePage() { 
   // do nothing
}



//////////////////////////////////////////////////////////////////////////
//
// staff analysis functions
//


//////////////////////////////
//
// ScorePage::getMaxStaff -- return the number of the highest staff
//

int ScorePage::getMaxStaff(void) {
   if (systemAnalysisQ == 0) {
      analyzeSystems();
   }
   return maxStaffNumber;
}



//////////////////////////////
//
// ScorePage::getStaffCount -- return the number of staves on the page.
//

int ScorePage::getStaffCount(void) {
   if (systemAnalysisQ == 0) {
      analyzeSystems();
   }
   return staffCount;
}



//////////////////////////////
//
// ScorePage::getSystemCount -- return the number of systems on the page.
//

int ScorePage::getSystemCount(void) {
   if (systemAnalysisQ == 0) {
      analyzeSystems();
   }
   return systemCount;
}



//////////////////////////////
//
// ScorePage::analyzeSystems -- identify the systems on the page
//

void ScorePage::analyzeSystems(void) {
   staffsystem.setSize(0);
   int i;
   maxStaffNumber = -1;
   staffCount     = -1;
   systemCount    = -1;
   
   Array<int> stavesfound;
   stavesfound.setSize(1024);
   stavesfound.zero();
   int currentstaff;

   for (i=0; i<data.getSize(); i++) {
      currentstaff = (int)(data[i].getPValue(2));
      if (stavesfound[currentstaff] == 0) {
         if (maxStaffNumber < currentstaff) {
            maxStaffNumber = currentstaff;
         }
         stavesfound[currentstaff] = 1;
      }
   }

   // count the number of staves which actually exist:
   staffCount = 0;
   for (i=1; i<=maxStaffNumber; i++) {
      if (stavesfound[i] != 0) {
         staffCount++;
      }
   }

   // now identify the system groupings
   Array<int> measurelength;
   measurelength.setSize(maxStaffNumber+1);
   measurelength.zero();
   for (i=1; i<data.getSize(); i++) {
      if (data[i].isBarlineItem()) {
         if (measurelength[(int)data[i].getPValue(2)] < 
               (int)data[i].getPValue(4)) {
            measurelength[(int)data[i].getPValue(2)] = 
                  (int)data[i].getPValue(4);
         }
      }
   }   

   staffsystem.setSize(maxStaffNumber+1);
   staffsystem.zero();
   track.setSize(maxStaffNumber+1);
   systemCount = 0;
   int currentStaff = 1;
   while (currentStaff <= maxStaffNumber) {
      systemCount++;
      if (measurelength[currentStaff] <= 1) {
         staffsystem[currentStaff] = systemCount;
         track[currentStaff] = 1;
         currentStaff++;
         continue;
      }
      while (currentStaff <= maxStaffNumber && measurelength[currentStaff]>1) {
         if (measurelength[currentStaff] == 0 ||
               measurelength[currentStaff] == 1) {
            track[currentStaff] = 1;
            staffsystem[currentStaff] = systemCount;
         } else {
            for (i=0; i<measurelength[currentStaff]; i++) {
               track[currentStaff+i] = i+1;
               staffsystem[currentStaff+i] = systemCount;
            }
         }
         currentStaff += measurelength[currentStaff];
      }
   }

   systemAnalysisQ = 1;
 

   // store information about the start and lenght of staves on the page.
   if (!sortQ) {
      sortByStaff();
   }
   staffStart.setAll(-1);
   staffSize.setAll(0);
   int oldStaffIndex;
   int staffIndex;
   for (i=0; i<getSize(); i++) {
      staffIndex = (int)data[i].getPValue(2);
      if (staffStart[staffIndex] == -1) {
         // new staff starts here
         staffStart[staffIndex] = i;
         if (i>0) {
            // store the length of the previous staff
            oldStaffIndex = (int)data[i-1].getPValue(2);
            staffSize[oldStaffIndex] = i-1 - staffStart[oldStaffIndex] + 1;
         }
      }
   }
   // add the last staff size information
   staffIndex = (int)data.last().getPValue(2);
   staffSize[staffIndex] = data.getSize() - staffStart[staffIndex];

   buildSystemIndexDatabase();
}



//////////////////////////////
//
// ScorePage::buildSystemIndexDatabase --
//

void ScorePage::buildSystemIndexDatabase(void) {
   Array<SystemRecord> temprecords;
   temprecords.setSize(data.getSize());

   int i;

   for (i=0; i<temprecords.getSize(); i++) {
      temprecords[i].index  = i;
      temprecords[i].system = getSystem((int)data[i].getPValue(2));
      temprecords[i].ptr    = &data[i];
   }

   qsort(temprecords.getBase(), temprecords.getSize(), sizeof(SystemRecord), 
         ScorePageBase::compareSystem);

   systemind.setSize(data.getSize());
   for (i=0; i<systemind.getSize(); i++) {
      systemind[i] = temprecords[i].index;
   }

   systemStart.setSize(getSystemCount());
   systemSize.setSize(getSystemCount());
   systemStart.setSize(0);
   systemSize.setSize(0);
   int value = 0;
   int lastsystem = 0;
   systemStart.append(value);
   systemSize.append(value);
   for (i=0; i<temprecords.getSize(); i++) {
      if (temprecords[i].system != lastsystem) {
         systemSize.last() = i - systemStart.last() + 1;
         systemStart.append(i);
         systemSize.append(value);
      }
   }
   systemSize.last() = temprecords.getSize() - systemStart.last() + 1;
}



//////////////////////////////
//
// getSystemItem --
//

ScoreRecord&   ScorePage::getSystemItem(int sysno, int index) {
   if (systemAnalysisQ == 0) {
      analyzeSystems();
   }
   int realindex = systemind[systemStart[sysno] + index];
   return data[realindex];
}



//////////////////////////////
//
// getSystemSize -- return the number of items in a particular system.
//

int ScorePage::getSystemSize(int sysno) {
   if (systemAnalysisQ == 0) {
      analyzeSystems();
   }
   return systemSize[sysno];
}



//////////////////////////////
//
// ScorePage::getMaxBarlineLength -- 
//

int ScorePage::getMaxBarlineLength(int staffno) {
   int i;
   int output = 0;
   for (i=0; i<data.getSize(); i++) {
      if (data[i].isBarlineItem() && (int)data[i].getPValue(2) == staffno && 
            (int)data[i].getPValue(4) > output) {
         output = (int)data[i].getPValue(4);
      }
   }

   return output;
}



//////////////////////////////
//
// ScorePage::getSystem -- return the system on the page which
//    contains the specified staff.  The bottom system is label 1.
//    unknown system ownership is labeled 0.

int ScorePage::getSystem(int staffno) {
   if (systemAnalysisQ == 0) {
      analyzeSystems();
   }
   return staffsystem[staffno];
}



//////////////////////////////
//
// ScorePage::getStaff -- returns the ScoreRecord for the given
//     staff number and
//  
// 

ScoreRecord& ScorePage::getStaff(int staffno, int staffItem) {
   if (systemAnalysisQ == 0) {
      analyzeSystems();
   }

   return data[staffStart[staffno]+staffItem];
}



//////////////////////////////
//
// ScorePage::getStaffSize -- returns the number of items on the
//    specified staff.
// 
//

int ScorePage::getStaffSize(int staffno) {
   if (systemAnalysisQ == 0) {
      analyzeSystems();
   }
   return staffSize[staffno];
}



//////////////////////////////
//
// ScorePage::getTrack -- return the system track on the page which
//    contains the specified staff.  The bottom staff on the system is label 1.
//    unknown track assignment is labeled 0.

int ScorePage::getTrack(int staffno) {
   if (systemAnalysisQ == 0) {
      analyzeSystems();
   }
   return track[staffno];
}



//////////////////////////////////////////////////////////////////////////
//
// pitch analysis functions
//


///////////////////////////////
//
// ScorePage::analyzePitch --
//

void ScorePage::analyzePitch(void) {
   if (!sortQ) {
      sortByStaff();
   }
   if (systemAnalysisQ == 0) {
      analyzeSystems();
   }

   int currentposition = 0;
   int i;
   for (i=1; i<=getMaxStaff(); i++) {
      currentposition = assignPitch(i, currentposition);
   }
}



//////////////////////////////
//
// ScorePage::assignPitch --
//

int ScorePage::assignPitch(int staff, int currentposition) {
   int i = currentposition;
   int clef = 0;        // treble clef by default
   int clefoffset = 0;  // base position by default
   int keysig = 0;      // C major by default
   int p12clef       = 0;
   int p12clefoffset = 0;
   int p12keysig     = 0;
   ScoreRecord* clefptr = NULL;
   ScoreRecord* keyptr  = NULL;
   Array<int> accidentals(7);
   Array<int> p12accidentals(7);
   accidentals.zero();
   accidentals.allowGrowth(0);
   p12accidentals.zero();
   p12accidentals.allowGrowth(0);

   while (data[i].getPValue(2) == staff) {
      if (data[i].isKeysigItem()) {
         keysig = (int)data[i].getPValue(5);
         resetAccidentals(accidentals, keysig);
      } else if (data[i].isBarlineItem()) {
         resetAccidentals(accidentals, keysig);
      } else if (data[i].isNoteItem()) {
         switch ((int)data[i].getPValue(12)) { 
            case 1:          // place on staff above
               // does not handle accidental alterations in affected staff
               clefptr = &getClefAtLocation(staff+1, 
                     data[currentposition].getPValue(3));
               keyptr  = &getKeysigAtLocation(staff+1, 
                     data[currentposition].getPValue(3));
               p12clef = (int)clefptr->getPValue(5);
               p12clefoffset = (int)clefptr->getPValue(4);
               p12keysig = (int)keyptr->getPValue(5);
               resetAccidentals(p12accidentals, p12keysig);
               data[i].setPitch(convertPitchToBase40(
                     (int)(data[i].getPValue(4) + 0.5) % 100, 
                     ((int)data[i].getPValue(5)) % 10, 
                     p12clef, p12clefoffset, p12accidentals));
               break;
            case 2:          // place on staff below
               // does not handle accidental alterations in affected staff
               clefptr = &getClefAtLocation(staff-1, 
                     data[currentposition].getPValue(3));
               keyptr  = &getKeysigAtLocation(staff-1, 
                     data[currentposition].getPValue(3));
               p12clef = (int)clefptr->getPValue(5);
               p12clefoffset = (int)clefptr->getPValue(4);
               p12keysig = (int)keyptr->getPValue(5);
               resetAccidentals(p12accidentals, p12keysig);
               data[i].setPitch(convertPitchToBase40(
                     (int)(data[i].getPValue(4) + 0.5) % 100, 
                     ((int)data[i].getPValue(5)) % 10, 
                     p12clef, p12clefoffset, p12accidentals));
               break;
            case 0:          // place on current staff
            default:
               data[i].setPitch(convertPitchToBase40(
                     (int)(data[i].getPValue(4) + 0.5) % 100, 
                     ((int)data[i].getPValue(5)) % 10, 
                     clef, clefoffset, accidentals));
               break;
         }

      } else if (data[i].isRestItem()) {
         data[i].setPitch(-1000);
      } else if (data[i].isClefItem()) {
         clef = (int)data[i].getPValue(5);
         clefoffset = (int)data[i].getPValue(4) % 100;
         if (data[i].getPValue(5) == 0.8) {         // Vocal tenor clef
            clef = 0;
            clefoffset += 8;
         }
      }
      
      i++;
   }
   return i;
}




//////////////////////////////
//
// ScorePage::getClefAtLocation -- returns the currently active clef
//    on the specified staff at the given position.  If not clef was
//    found, then will return a treble clef.
//

ScoreRecord& ScorePage::getClefAtLocation(int staffno, float position) {
   if (!sortQ) {
      sortByStaff();
   }
   static ScoreRecord defaultclef;
   static int init = 0;
   if (!init) {
      init = 1;
      defaultclef.setPValue(1, 3.0);
      defaultclef.setPValue(2, 1.0);
      defaultclef.setPValue(3, 0.0);
      defaultclef.setPValue(4, 0.0);
      defaultclef.setPValue(5, 0.0);
   }

   int i = findStaff(staffno);
   if (i < 0) {
      defaultclef.setPValue(2, (float)staffno);
      return defaultclef;
   }

   int clefindex = -1;
   while (i < data.getSize() && (int)data[i].getPValue(2) == staffno &&
         data[i].getPValue(3) < position) {
      if (data[i].isClefItem()) {
         clefindex = i;
      }
      i++;
   }

   if (clefindex >= 0) {
      return data[clefindex];
   } 

   defaultclef.setPValue(2, (float)staffno);
   return defaultclef;
}



//////////////////////////////
//
// ScorePage::getKeysigAtLocation -- returns the currently active
//    key signature at the current location on the given staff.
//    If no key signature has been found, then use the key of C major.
//

ScoreRecord& ScorePage::getKeysigAtLocation(int staffno, float position) {
   if (!sortQ) {
      sortByStaff();
   }
   static ScoreRecord defaultkeysig;
   static int init = 0;
   if (!init) {
      init = 1;
      defaultkeysig.setPValue(1, 17.0);
      defaultkeysig.setPValue(2, 1.0);
      defaultkeysig.setPValue(3, 0.0);
      defaultkeysig.setPValue(4, 0.0);
      defaultkeysig.setPValue(5, 0.0);
      defaultkeysig.setPValue(6, 0.0);
   }
   defaultkeysig.setPValue(2, (float)staffno);

   int i = findStaff(staffno);
   if (i < 0) {
      defaultkeysig.setPValue(2, (float)staffno);
      return defaultkeysig;
   }

   int keyindex = -1;
   while (i < data.getSize() && (int)data[i].getPValue(2) == staffno &&
         data[i].getPValue(3) < position) {
      if (data[i].isKeysigItem()) {
         keyindex = i;
      }
      i++;
   }

   if (keyindex >= 0) {
      return data[keyindex];
   } 

   defaultkeysig.setPValue(2, (float)staffno);
   return defaultkeysig;
}



//////////////////////////////
//
// ScorePage::convertPitchToBase40 -- figure out what the @#$% pitch is.
//

int ScorePage::convertPitchToBase40(int line, int acc, int clef, int
   clefoffset, Array<int>& accidentals) {
   int octave = 0;
   int diatonic = 0;
   int basepitch[7] = {0, 6, 12, 17, 23, 29, 35};

   switch (clef) {
      case 1:       // bass clef
         octave = 2;
         diatonic = line + 1 - clefoffset;
         break;
      case 2:       // alto clef
         octave = 3;
         diatonic = line - clefoffset;
         break;
      case 3:       // tenor clef
         octave = 3;
         diatonic = line - 2 - clefoffset;
         break;
      case 0:       // treble clef
      default:      // handle other clefs later, assign to treble clef for now
         octave = 4;
         diatonic = line - 1 - clefoffset;
         break;
   }
   if (diatonic < 0) {
      while (diatonic < 0) {
         diatonic += 7;
         octave -= 1;
      }
   }
   if (diatonic > 6) {
      octave += diatonic / 7;
      diatonic = diatonic % 7;
   } 

   int output = basepitch[diatonic] + 40 * octave + 2;

   switch (acc) {
      case 1:      // flat found on note
         output--;
         accidentals[diatonic] = -1;
         break;
      case 2:      // sharp found on note
         output++;
         accidentals[diatonic] = +1;
         break;
      case 3:      // natural found on note
         accidentals[diatonic] = 0;
         break;
      case 4:      // double flat found on note
         output-=2;
         accidentals[diatonic] = -2;
         break;
      case 5:      // double sharp found on note
         output+=2;
         accidentals[diatonic] = +2;
         break;
      case 0:      // no accidental specified explicitly on the note
      default:
         output += accidentals[diatonic];
         break;
   }
if (output < 0) {
   cout << "ERROR: acc=" << accidentals[diatonic]   
        << " octave=" << octave << " diatonic=" << diatonic << endl;
}

   return output;
}



//////////////////////////////
//
// resetAccidentals -- set the accidentals to the current key signature
//

void ScorePage::resetAccidentals(Array<int>& accidentals, int keysig) {
   if (keysig == 0) {
      accidentals.zero();
      return;
   }

   if (keysig > 0) {
      if (keysig > 0) accidentals[3] = +1;  // F-sharp
      if (keysig > 1) accidentals[0] = +1;  // C-sharp
      if (keysig > 2) accidentals[4] = +1;  // G-sharp
      if (keysig > 3) accidentals[1] = +1;  // D-sharp
      if (keysig > 4) accidentals[5] = +1;  // A-sharp
      if (keysig > 5) accidentals[2] = +1;  // E-sharp
      if (keysig > 6) accidentals[6] = +1;  // B-sharp
      return;
   }

   if (keysig < 0)  accidentals[6] = -1;  // B-flat
   if (keysig < -1) accidentals[2] = -1;  // E-flat
   if (keysig < -2) accidentals[5] = -1;  // A-flat
   if (keysig < -3) accidentals[1] = -1;  // D-flat
   if (keysig < -4) accidentals[4] = -1;  // G-flat
   if (keysig < -5) accidentals[0] = -1;  // C-flat
   if (keysig < -6) accidentals[3] = -1;  // F-flat
}



// md5sum: 097b1076b87b8b25465be795f377816f ScorePage.cpp [20050403]
