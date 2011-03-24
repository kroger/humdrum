//
// Copyright 2010 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun 17 13:27:39 PDT 2010
// Last Modified: Thu Jun 17 13:27:46 PDT 2010
// Filename:      ...sig/src/sigInfo/MuseDataSet.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/MuseDataSet.cpp
// Syntax:        C++ 
// 
// Description:   A class that stores a collection of MuseDataSet files
//                representing parts for the same score.
//

#include "MuseDataSet.h"
#include "PerlRegularExpression.h"
#include <string.h>

#ifndef OLDCPP
   #include <sstream>
   #include <fstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #ifdef VISUAL
      #include <strstrea.h>     /* for Windows 95 */
   #else
      #include <strstream.h>
   #endif
   #include <fstream.h>
   #define SSTREAM strstream
   #define CSTRING str()
#endif
   

///////////////////////////////////////////////////////////////////////////
//
// MuseDataSet class functions --
//


//////////////////////////////
//
// MuseDataSet::MuseDataSet --
//

MuseDataSet::MuseDataSet (void) { 
   part.setSize(100);
   part.setSize(0);
   part.setGrowth(1000);
}



//////////////////////////////
//
// MuseDataSet::clear -- Remove contents of object.
//

void MuseDataSet::clear(void) { 
   int i;
   for (i=0; i<part.getSize(); i++) {
      delete part[i];
   }

}



//////////////////////////////
//
// MuseDataSet::operator[] --
//

MuseData& MuseDataSet::operator[](int index) {
   return *part[index];
}



//////////////////////////////
//
// MuseDataSet::readPart -- read a single MuseData part, appending it
//      to the current list of parts.
//

int MuseDataSet::readPart(const char* filename) {
   MuseData* md = new MuseData;
   md->read(filename);
   md->setFilename(filename);
   return appendPart(md);
}


int MuseDataSet::readPart(istream& input) {
   MuseData* md = new MuseData;
   md->read(input);
   return appendPart(md);
}



//////////////////////////////
//
// MuseDataSet::read -- read potentially Multiple parts from a single file.
//   First clear the contents of any previous part data.
//

void MuseDataSet::read(const char* filename) {
   MuseDataSet::clear();

   #ifndef OLDCPP
      ifstream infile(filename, ios::in);
   #else
      ifstream infile(filename, ios::in | ios::nocreate);
   #endif

   MuseDataSet::read(infile);
}


void MuseDataSet::read(istream& infile) {
   Array<string> datalines;
   datalines.setSize(1000000);
   datalines.setGrowth(5000000);
   datalines.setSize(0);

   string thing;

   while (!infile.eof()) {
      getline(infile, thing);
      if (infile.eof() && (thing.length() == 0)) {
         // last line was not terminated by a newline character
         break;
      }
      datalines.append(thing);
   }

   Array<int> startindex;
   Array<int> stopindex;
   startindex.setSize(0);
   stopindex.setSize(0);
   analyzePartSegments(startindex, stopindex, datalines);

   int i, j;
   SSTREAM* sstream;
   MuseData* md;
   for (i=0; i<startindex.getSize(); i++) {
      sstream = new SSTREAM;
      for (j=startindex[i]; j<=stopindex[i]; j++) {
          (*sstream) << datalines[j] << '\n';
      }
      md = new MuseData;
      md->read(*sstream);
      appendPart(md);
      delete sstream;
   }
}



//////////////////////////////
//
// MuseDataSet::appendPart -- append a MuseData pointer to the end of the
//   parts list and return the index number of the new part.
//

int MuseDataSet::appendPart(MuseData* musedata) {
   int index = part.getSize();
   part.setSize(part.getSize()+1);
   part[index] = musedata;
   return index;
}



//////////////////////////////
//
// MuseData::analyzePartSegments -- Calculate the starting line index
//    and the ending line index for each part in the input.
//

void MuseDataSet::analyzePartSegments(Array<int>& startindex, 
      Array<int>& stopindex, Array<string> lines) {
 
   startindex.setSize(1000);
   startindex.setSize(0);
   startindex.setGrowth(1000);

   stopindex.setSize(1000);
   stopindex.setSize(0);
   stopindex.setGrowth(1000);

   Array<int> types;
   // MuseData& thing = *this;
   int i, j;

   types.setSize(lines.getSize());
   types.allowGrowth(0);
   types.setAll(E_muserec_unknown);

   // first identify lines which are multi-line comments so they will
   // not cause confusion in the next step
   int commentstate = 0;
   for (i=0; i<lines.getSize(); i++) {
      if (lines[i].c_str()[0] == '&') {
         types[i] = E_muserec_comment_toggle;
	 commentstate = !commentstate;
         continue;
      }
      if (commentstate) {
         types[i] = E_muserec_comment_line;
      }
   }

   // search the data for "Group memberships:" lines which are required
   // to be in the header of each part.
   Array<int> groupmemberships(1000);
   groupmemberships.setSize(0);
   groupmemberships.setGrowth(1000);
   int len = strlen("Group memberships:");
   for (i=0; i<lines.getSize(); i++) {
      if (strncmp("Group memberships:", lines[i].c_str(), len) == 0) {
         if (types[i] != E_muserec_comment_line) {
            groupmemberships.append(i);
         }
      }
   }

   // search backwards from "Group memberships:" until the end of the
   // header, sucking any comments which occurs just before the start
   // of the header. (currently only multi-line comments, but also need
   // to add single-line comments)
   int value;
   int headerline;
   int ii;
   int found = 0;
   for (ii=0; ii<groupmemberships.getSize(); ii++) {
      i = groupmemberships[ii];
      types[i] = E_muserec_group_memberships;
      found = 0;
      headerline = 11;
      for (j=i-1; j>=0; j--) {
         if (j < 0) {
            break;
         }
         if ((types[j] == E_muserec_comment_line) || 
             (types[j] == E_muserec_comment_toggle)) {
            j--;
            continue;
         }
         if (j < 0) {
            break;
         }
         headerline--;

	 if (headerline == 0) {
            while ((j>= 0) && (lines[j][0] == '@')) {
               j--;
            }
            value = j+1;
            //value = j+2;
            found = 1;
            startindex.append(value);
            break;
         }

         if ((j >= 0) && (headerline == 0)) {
            value = j+1;
            found = 1;
            startindex.append(value);
            break;
         }
         if (j<0) {
            value = 0;
            found = 1;
            startindex.append(value);
            continue;
         }
         switch (headerline) {
            case 11: types[j] = E_muserec_header_11; break;
            case 10: types[j] = E_muserec_header_10; break;
            case  9: types[j] = E_muserec_header_9; break;
            case  8: types[j] = E_muserec_header_8; break;
            case  7: types[j] = E_muserec_header_7; break;
            case  6: types[j] = E_muserec_header_6; break;
            case  5: types[j] = E_muserec_header_5; break;
            case  4: types[j] = E_muserec_header_4; break;
            case  3: types[j] = E_muserec_header_3; break;
            case  2: types[j] = E_muserec_header_2; break;
            case  1: types[j] = E_muserec_header_1; break;
         }
      }
      if (!found) {
         value = 0;
         startindex.append(value);
      }
   }

   // now calculate the stopindexes:
   stopindex.setSize(startindex.getSize());
   stopindex[stopindex.getSize()-1] = lines.getSize()-1;
   for (i=0; i<startindex.getSize()-1; i++) {
      stopindex[i] = startindex[i+1]-1;
   }


//for (i=0; i<lines.getSize(); i++) {
//   cout << (char)types[i] << "\t" << lines[i] << endl;
//}

}



//////////////////////////////
//
// MuseDataSet::getPartCount -- return the number of parts found
//      in the MuseDataSet
//

int MuseDataSet::getPartCount(void) {
   return part.getSize();
}



//////////////////////////////
//
// MuseDataSet::deletePart -- remove a particular part from the data set.
//

void MuseDataSet::deletePart(int index) {
   if (index < 0 || index > part.getSize()-1) {
      cerr << "Trying to delete a non-existent part" << endl;
      exit(1);
   }

   delete part[index];
   int i;
   for (i=index+1; i<part.getSize(); i++) {
      part[i-1] = part[i];
   }
   part.setSize(part.getSize()-1);
}



//////////////////////////////
//
// MuseDataSet::cleanLineEndings -- remove spaces for ends of lines.
//

void MuseDataSet::cleanLineEndings(void) {
   int i;
   for (i=0; i<part.getSize(); i++) {
      part[i]->cleanLineEndings();
   }
}



///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// operator<< -- print out all parts in sequential order
//

ostream& operator<<(ostream& out, MuseDataSet& musedataset) {
   int i, j;
   for (i=0; i<musedataset.getPartCount(); i++) {
      for (j=0; j<musedataset[i].getNumLines(); j++) {
         out << musedataset[i][j] << '\n';
      }
   }
   return out;
}



// md5sum: 05e1e67705ebcb2bfa1afad9e504582c MuseDataSet.cpp [20050403]
