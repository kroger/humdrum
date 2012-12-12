//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun 14 12:18:18 PDT 2012
// Last Modified: Fri Jun 15 16:31:09 PDT 2012
// Filename:      ...sig/examples/all/prettystar.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/prettystar.cpp
// Syntax:        C++; museinfo
//
// Description: Extract all spines from a multi-spine file into
// 		single-column output.
// 

#include "humdrum.h"
#include "SigString.h"
#include "PerlRegularExpression.h"

#ifndef OLDCPP
   using namespace std;
#endif

#define TYPE_USED	 (-100) /* bookkeeping		*/
#define TYPE_UNKNOWN	 (-1)   /* cannot categorize	*/

//                                 EXAMPLES:
#define TYPE_EMPTY	 (0)	/*	*		*/
#define TYPE_STAFF	 (1)	/*	*staff4		*/
#define TYPE_SYSSTAFF	 (2)	/*	*staff:4	*/
#define TYPE_TRANSP	 (3)	/*	*ITrd-4c-7	*/
#define TYPE_ICLASS	 (4)	/*	*ICvox		*/
#define TYPE_IGROUP	 (5)	/*	*IGstrings	*/
#define TYPE_ITYPE	 (6)	/*	*Ivox		*/
#define TYPE_INAME	 (7)	/*	*I"Flute	*/
#define TYPE_IABBR	 (8)	/*	*I'Fl		*/
#define TYPE_INUM	 (9)	/*	*I#2		*/
#define TYPE_EXPAND	 (10)	/*	*>[A,A,B]	*/
#define TYPE_EXPANDVAR	 (11)	/*	*>norep[A,A,B]	*/
#define TYPE_OCLEF	 (12)	/*	*oclefG2	*/
#define TYPE_CLEF	 (13)	/*	*clefF4		*/
#define TYPE_KEYSIG	 (14)	/*	*k[f#]		*/
#define TYPE_KEY	 (15)	/*	*B-:dor		*/
#define TYPE_TIMESIG	 (16)	/*	*M4/4		*/
#define TYPE_METSIG	 (17)	/*	*met(c|)	*/
#define TYPE_TEMPO	 (18)	/*	*MM120		*/
#define TYPE_RSCALE	 (19)	/*	*rscale:1/2	*/
#define TYPE_TACET	 (20)	/*	*xtacet		*/
#define TYPE_HAND	 (21)	/*	*rh, *lh	*/
#define TYPE_TIMEBASE	 (22)	/*	*tb16		*/
#define TYPE_EXPANDLABEL (23)	/*	*>A		*/
#define TYPE_MAX	 (23)

#define NAME_UNKNOWN	 "unknown"	/* cannot categorize	*/
#define NAME_EMPTY	 "empty"	/*	*		*/
#define NAME_STAFF	 "staff"	/*	*staff4		*/
#define NAME_SYSSTAFF	 "sysstaff"	/*	*staff:4	*/
#define NAME_TRANSP	 "transp"	/*	*ITrd-4c-7	*/
#define NAME_ICLASS	 "iclass"	/*	*ICvox		*/
#define NAME_IGROUP	 "igroup"	/*	*IGstrings	*/
#define NAME_ITYPE	 "itype"	/*	*Ivox		*/
#define NAME_INAME	 "iname"	/*	*I"Flut		*/
#define NAME_IABBR	 "iabbr"	/*	*I'Fl		*/
#define NAME_INUM	 "inum"		/*	*I#2		*/
#define NAME_OCLEF	 "oclef"	/*	*oclefG2	*/
#define NAME_CLEF	 "clef"		/*	*clefF4		*/
#define NAME_KEYSIG	 "keysig"	/*	*k[f#]		*/
#define NAME_KEY	 "key"		/*	*B-:dor		*/
#define NAME_TIMESIG	 "timesig"	/*	*M4/4		*/
#define NAME_METSIG	 "metsig"	/*	*met(c|)	*/
#define NAME_TEMPO	 "tempo"	/*	*MM120		*/
#define NAME_RSCALE	 "rscale"	/* 	*rscale:1/2	*/
#define NAME_TACET	 "tacet"	/* 	*tacet		*/
#define NAME_HAND	 "hand"		/*	*rh, *lh	*/
#define NAME_TIMEBASE	 "timebase"	/*	*tb16		*/
#define NAME_EXPAND	 "exp"		/*	*>[A,A,B]	*/
#define NAME_EXPANDVAR	 "expvar"	/*	*>norep[A,A,B]	*/
#define NAME_EXPANDLABEL "expmark"	/*	*>A		*/

// function declarations
void      checkOptions          (Options& opts, int argc, char* argv[]);
void      example               (void);
void      usage                 (const char* command);
void      printBlocks           (HumdrumFile& infile);
void      sortAllBlocks         (HumdrumFile& infile);
int       validline             (HumdrumFile& infile, int i);
int       getInterpretationCount(HumdrumFile& infile, int line);
char*     getInterpName         (char* buffer, HumdrumRecord& line, int index);
int       getInterpNum          (HumdrumRecord& line, int index);
void      printUnknownInterpretations(HumdrumFile& infile);
void      getBlockList          (HumdrumFile& infile, Array<int>& starti, 
                                 Array<int>& endi, int initQ);
void      printSortedInterpretations(HumdrumFile& infile, int starti, int endi,
                                 int blockcount);
int       nameToNumber          (const char* name);
const char* numberToName        (int number);
void      processInterpretationType(int target,  HumdrumFile& infile, 
                                 int starti, Array<Array<int> >& types, 
                                 int blockcount);
void      printInterpretationType(int target, HumdrumFile& infile, int starti, 
                                 Array<Array<int> >& types, int blockcount);
int       hasType                (int target, Array<Array<int> >& types, 
                                  int blocktype);
void      printUnknownLines      (HumdrumFile& infile, int starti, 
                                  Array<Array<int> >& types);
void      checkForWarnings       (HumdrumFile& infile, int starti, int endi, 
                                  Array<Array<int> >& types);
int       checkForMixedInterpretations(Array<int>& types);
void      seachForInterpretation (HumdrumFile& infile, 
                                  const char* SearchString);
void      seachListForInterpretation(HumdrumFile& infile, 
                                  const char* SearchString);

// block parsing functions:
void      fillFieldData          (Array<int>& field, const char* fieldstring, 
                                  HumdrumFile& infile);
void      processFieldEntry      (Array<int>& field, const char* string, 
                                  HumdrumFile& infile, int maxblock);
void      removeDollarsFromString(Array<char>& buffer, int maxblock);

// global variables
Options   options;             // database for command-line arguments
int       listblocksQ = 0;     // used with -l option
int       listunknownQ = 0;    // used with -u option
int       typeQ = 0;           // used with -t option
int       keysigQ = 0;         // used with -k option
int       filenameQ = 0;       // used with -f option
int       searchQ = 0;         // used with -s option
const char* SearchString = ""; // used with -s option
const char* FILENAME = "";     // used with -f option
int       unknownlastQ = 0;    // used with --last option
int       warnQ = 0;           // used with -w option
int       blockQ = 0;          // used with -b option
Array<int> BlockProcess;       // used with -b option
const char* BlockString = "";  // used with -b option
int       debugQ = 0;          // used with --debug option
Array<int>  FirstSort;         // used with -F option
const char* FirstString = "1"; // used with -F option
int       searchlistQ = 0;     // used with -L option
const char* ListString = "";   // used with -L option

///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile infile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   for (int i=0; i<numinputs || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
         FILENAME = "<STDIN>";
      } else {
         FILENAME = options.getArg(i+1);
         infile.read(FILENAME);
      }

      fillFieldData(BlockProcess, BlockString, infile);
      fillFieldData(FirstSort, FirstString, infile);

      if (debugQ) {
         int i;
         cout << "!! BLOCK INFORMATION: ";
         for (i=0; i<BlockProcess.getSize(); i++) {
            cout << BlockProcess[i] << " ";
         }
         cout << endl;

         cout << "!! FIRST INFORMATION: ";
         for (i=0; i<FirstSort.getSize(); i++) {
            cout << FirstSort[i] << " ";
         }
         cout << endl;
      }

      if (listblocksQ) {
         printBlocks(infile);
      } else if (listunknownQ) {
         if (filenameQ) {
            cout << FILENAME << "\n";
         }
         printUnknownInterpretations(infile);
      } else if (searchQ) {
         if (filenameQ) {
            cout << FILENAME << "\n";
         }
         seachForInterpretation(infile, SearchString);
      } else if (searchlistQ) {
         if (filenameQ) {
            cout << FILENAME << "\n";
         }
         seachListForInterpretation(infile, ListString);
      } else {
         sortAllBlocks(infile);
      }
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// printSearchList --
//

void printSearchList(void) {
   int i;
   for (i=1; i<=TYPE_MAX; i++) {
      cout << numberToName(i) << "\n";
   }
}



//////////////////////////////
//
// getInterpType --
// #define NAME_UNKNOWN	 "unknown"	/* cannot categorize	*/
// #define NAME_EMPTY	 "empty"	/*	*		*/
// #define NAME_STAFF	 "staff"	/*	*staff4		*/
// #define NAME_STAFFC	 "sysstaff"	/*	*staff:4	*/
// #define NAME_OCLEF	 "oclef"	/*	*oclefG2	*/
// #define NAME_CLEF	 "clef"		/*	*clefF4		*/
// #define NAME_KEY	 "key"		/*	*B-:dor		*/
// #define NAME_KEYSIG	 "keysig"	/*	*k[f#]		*/
// #define NAME_TIMESIG	 "timesig"	/*	*M4/4		*/
// #define NAME_METSIG	 "metsig"	/*	*met(c|)	*/
// #define NAME_TEMPO	 "tempo"	/*	*MM120		*/
// #define NAME_TRANSP	 "transpose"	/*	*ITrd-4c-7	*/
// #define NAME_ITYPE	 "instrument"	/*	*Ivox		*/
// #define NAME_ICLASS	 "iclass"	/*	*ICvox		*/
// #define NAME_INUM	 "inum"		/*	*I#2		*/
// #define NAME_INAME	 "iname"	/*	*I"Flut		*/
// #define NAME_IABBR	 "iabbr"	/*	*I'Fl		*/
// #define NAME_EXPAND	 "exp"		/*	*>[A,A,B]	*/
// #define NAME_EXPANDVAR "expvar"	/*	*>norep[A,A,B]	*/
// #define NAME_EXPANDLABEL "explevel"	/*	*>A		*/
//

char* getInterpName(char* b, HumdrumRecord& line, int index) {
   if (strcmp(line[index], "*") == 0) { strcpy(b, NAME_EMPTY); return b; }
   if (line.isClef(index))		{ strcpy(b, NAME_CLEF); return b; }
   if (line.isOriginalClef(index))	{ strcpy(b, NAME_OCLEF); return b; }
   if (line.isKey(index))		{ strcpy(b, NAME_KEY); return b; }
   if (line.isKeySig(index))		{ strcpy(b, NAME_KEYSIG); return b; }
   if (line.isTempo(index))		{ strcpy(b, NAME_TEMPO); return b; }
   if (line.isTimeSig(index))		{ strcpy(b, NAME_TIMESIG); return b; }
   if (line.isMetSig(index))		{ strcpy(b, NAME_METSIG); return b; }
   if (line.isTranspose(index))		{ strcpy(b, NAME_TRANSP); return b; }
   if (line.isInstrumentType(index))	{ strcpy(b, NAME_ITYPE); return b; }
   if (line.isInstrumentClass(index))	{ strcpy(b, NAME_ICLASS); return b; }
   if (line.isInstrumentName(index))	{ strcpy(b, NAME_INAME); return b; }
   if (line.isInstrumentAbbr(index))	{ strcpy(b, NAME_IABBR); return b; }
   if (line.isInstrumentNum(index))	{ strcpy(b, NAME_INUM); return b; }
   if (line.isLabelExpansion(index))	{ strcpy(b, NAME_EXPAND); return b; }
   if (line.isLabelVariant(index))	{ strcpy(b, NAME_EXPANDVAR); return b; }
   if (line.isLabelMarker(index))  { strcpy(b, NAME_EXPANDLABEL); return b; }
   if (line.isStaffNumber(index))	{ strcpy(b, NAME_STAFF); return b; }
   if (line.isSysStaffNumber(index))	{ strcpy(b, NAME_SYSSTAFF); return b; }

   PerlRegularExpression pre;

   if (pre.search(line[index], "^\\*x?tacet$")) {
      strcpy(b, NAME_TACET); return b;
   }

   if (pre.search(line[index], "^\\*[lr]h$", "i")) {
      strcpy(b, NAME_HAND); return b;
   }

   if (pre.search(line[index], "^\\*tb\\d", "")) {
      strcpy(b, NAME_TIMEBASE); return b;
   }

   if (pre.search(line[index], "^\\*IG", "")) {
      strcpy(b, NAME_IGROUP); return b;
   }

   if (pre.search(line[index], "^\\*x?rscale:\\d+/?\\d*$")) {
      strcpy(b, NAME_RSCALE); return b;
   }

   strcpy(b, NAME_UNKNOWN); return b;
}



//////////////////////////////
//
// getInterpNum --
//

int getInterpNum(HumdrumRecord& line, int index) {
   if (strcmp(line[index], "*") == 0)	{ return TYPE_EMPTY; }
   if (line.isClef(index))		{ return TYPE_CLEF; }
   if (line.isOriginalClef(index))	{ return TYPE_OCLEF; }
   if (line.isKey(index))		{ return TYPE_KEY; }
   if (line.isKeySig(index))		{ return TYPE_KEYSIG; }
   if (line.isTempo(index))		{ return TYPE_TEMPO; }
   if (line.isTimeSig(index))		{ return TYPE_TIMESIG; }
   if (line.isMetSig(index))		{ return TYPE_METSIG; }
   if (line.isTranspose(index))		{ return TYPE_TRANSP; }
   if (line.isInstrumentType(index))	{ return TYPE_ITYPE; }
   if (line.isInstrumentClass(index))	{ return TYPE_ICLASS; }
   if (line.isInstrumentName(index))	{ return TYPE_INAME; }
   if (line.isInstrumentAbbr(index))	{ return TYPE_IABBR; }
   if (line.isInstrumentNum(index))	{ return TYPE_INUM; }
   if (line.isLabelExpansion(index))	{ return TYPE_EXPAND; }
   if (line.isLabelVariant(index))	{ return TYPE_EXPANDVAR; }
   if (line.isLabelMarker(index))	{ return TYPE_EXPANDLABEL; }
   if (line.isStaffNumber(index))	{ return TYPE_STAFF; }
   if (line.isSysStaffNumber(index))	{ return TYPE_SYSSTAFF; }

   PerlRegularExpression pre;

   if (pre.search(line[index], "^\\*x?tacet$")) {
      return TYPE_TACET;
   }

   if (pre.search(line[index], "^\\*[lr]h$", "i")) {
      return TYPE_HAND;
   }

   if (pre.search(line[index], "^\\*tb\\d", "")) {
      return TYPE_TIMEBASE;
   }

   if (pre.search(line[index], "^\\*IG", "")) {
      return TYPE_IGROUP;
   }

   if (pre.search(line[index], "^\\*x?rscale:\\d+/?\\d*$")) {
      return TYPE_RSCALE;
   }
   return TYPE_UNKNOWN;
}



//////////////////////////////
//
// numberToName --
//

const char* numberToName(int number) {

   switch (number) {
      case TYPE_EMPTY:		return NAME_EMPTY;
      case TYPE_STAFF:		return NAME_STAFF;
      case TYPE_SYSSTAFF:	return NAME_SYSSTAFF;
      case TYPE_TRANSP:		return NAME_TRANSP;
      case TYPE_ICLASS:		return NAME_ICLASS;
      case TYPE_IGROUP:		return NAME_IGROUP;
      case TYPE_ITYPE:		return NAME_ITYPE;
      case TYPE_INAME:		return NAME_INAME;
      case TYPE_IABBR:		return NAME_IABBR;
      case TYPE_INUM:		return NAME_INUM;
      case TYPE_EXPAND:		return NAME_EXPAND;
      case TYPE_EXPANDVAR:	return NAME_EXPANDVAR;
      case TYPE_OCLEF:		return NAME_OCLEF;
      case TYPE_CLEF:		return NAME_CLEF;
      case TYPE_KEYSIG:		return NAME_KEYSIG;
      case TYPE_KEY:		return NAME_KEY;
      case TYPE_TIMESIG:	return NAME_TIMESIG;
      case TYPE_METSIG:		return NAME_METSIG;
      case TYPE_TEMPO:		return NAME_TEMPO;
      case TYPE_RSCALE:		return NAME_RSCALE;
      case TYPE_TACET:		return NAME_TACET;
      case TYPE_HAND:		return NAME_HAND;
      case TYPE_TIMEBASE:	return NAME_TIMEBASE;
      case TYPE_EXPANDLABEL:	return NAME_EXPANDLABEL;
   }

   return NAME_UNKNOWN;
}



//////////////////////////////
//
// searchForInterpretation --
//

void seachForInterpretation(HumdrumFile& infile, const char* SearchString) {

   Array<int> starti;
   Array<int> endi;
   getBlockList(infile, starti, endi, 0);

   int i, j;
   int testval;
   int b;
   int target;
   int count = 0;

   PerlRegularExpression pre;
   if (pre.search(SearchString, "^/")) {
      Array<char> searchstring;
      searchstring.setSize(strlen(SearchString)+1);
      strcpy(searchstring.getBase(), SearchString);
      pre.sar(searchstring, "^/", "");
      pre.sar(searchstring, "/$", "");

      for (b=0; b<starti.getSize(); b++) {
         if (starti[b] < 0) {
            continue;
         }
         for (i=starti[b]; i<=endi[b]; i++) {
            for (j=0; j<infile[i].getFieldCount(); j++) {
               if (pre.search(infile[i][j], searchstring.getBase())) {
                  if (count++ != 0) {
                     cout << ",";
                  }
                  cout << b+1;
                  break;
               }
            }
         }
      }
   } else {
      target = nameToNumber(SearchString);
      for (b=0; b<starti.getSize(); b++) {
         if (starti[b] < 0) {
            continue;
         }
         for (i=starti[b]; i<=endi[b]; i++) {
            for (j=0; j<infile[i].getFieldCount(); j++) {
               testval = getInterpNum(infile[i], j);            
               if (testval == target) {
                  if (count++ != 0) {
                     cout << ",";
                  }
                  cout << b+1;
                  break;
               }
            }
         }
      }
   }
   if (count > 0) {
      cout << endl;
   }
}



//////////////////////////////
//
// searchListForInterpretation --
//

void seachListForInterpretation(HumdrumFile& infile, const char* SearchString) {

   Array<int> starti;
   Array<int> endi;
   getBlockList(infile, starti, endi, 0);

   int i, j;
   int testval;
   int b;
   int target;

   PerlRegularExpression pre;
   if (pre.search(SearchString, "^/")) {
      Array<char> searchstring;
      searchstring.setSize(strlen(SearchString)+1);
      strcpy(searchstring.getBase(), SearchString);
      pre.sar(searchstring, "^/", "");
      pre.sar(searchstring, "/$", "");

      for (b=0; b<starti.getSize(); b++) {
         if (starti[b] < 0) {
            continue;
         }
         for (i=starti[b]; i<=endi[b]; i++) {
            for (j=0; j<infile[i].getFieldCount(); j++) {
               if (pre.search(infile[i][j], searchstring.getBase())) {
                  cout << infile[i][j] << "\n";
               }
            }
         }
      }
   } else {
      target = nameToNumber(SearchString);
      for (b=0; b<starti.getSize(); b++) {
         if (starti[b] < 0) {
            continue;
         }
         for (i=starti[b]; i<=endi[b]; i++) {
            for (j=0; j<infile[i].getFieldCount(); j++) {
               testval = getInterpNum(infile[i], j);            
               if (testval == target) {
                  cout << infile[i][j] << "\n";
               }
            }
         }
      }
   }

}



//////////////////////////////
//
// nameToNumber --
//

int nameToNumber(const char* name) {
   if (strcmp(NAME_UNKNOWN, name) == 0)		{ return TYPE_UNKNOWN; }
   if (strcmp(NAME_EMPTY, name) == 0)		{ return TYPE_EMPTY; }
   if (strcmp(NAME_STAFF, name) == 0)		{ return TYPE_STAFF; }
   if (strcmp(NAME_SYSSTAFF, name) == 0)	{ return TYPE_SYSSTAFF; }
   if (strcmp(NAME_TRANSP, name) == 0)	{ return TYPE_TRANSP; }
   if (strcmp(NAME_ICLASS, name) == 0)		{ return TYPE_ICLASS; }
   if (strcmp(NAME_ITYPE, name) == 0)		{ return TYPE_ITYPE; }
   if (strcmp(NAME_INAME, name) == 0)		{ return TYPE_INAME; }
   if (strcmp(NAME_IABBR, name) == 0)		{ return TYPE_IABBR; }
   if (strcmp(NAME_INUM, name) == 0)		{ return TYPE_INUM; }
   if (strcmp(NAME_OCLEF, name) == 0)		{ return TYPE_OCLEF; }
   if (strcmp(NAME_CLEF, name) == 0)		{ return TYPE_CLEF; }
   if (strcmp(NAME_KEY, name) == 0)		{ return TYPE_KEY; }
   if (strcmp(NAME_KEYSIG, name) == 0)		{ return TYPE_KEYSIG; }
   if (strcmp(NAME_TIMESIG, name) == 0)		{ return TYPE_TIMESIG; }
   if (strcmp(NAME_METSIG, name) == 0)		{ return TYPE_METSIG; }
   if (strcmp(NAME_TEMPO, name) == 0)		{ return TYPE_TEMPO; }
   if (strcmp(NAME_EXPAND, name) == 0)		{ return TYPE_EXPAND; }
   if (strcmp(NAME_EXPANDVAR, name) == 0)	{ return TYPE_EXPANDVAR; }
   if (strcmp(NAME_EXPANDLABEL, name) == 0)	{ return TYPE_EXPANDLABEL; }
   
   if (strcmp(NAME_TACET, name) == 0)		{ return TYPE_TACET; }
   if (strcmp(NAME_HAND, name) == 0)		{ return TYPE_HAND; }
   if (strcmp(NAME_TIMEBASE, name) == 0)	{ return TYPE_TIMEBASE; }
   if (strcmp(NAME_IGROUP, name) == 0)		{ return TYPE_IGROUP; }
   if (strcmp(NAME_RSCALE, name) == 0)		{ return TYPE_RSCALE; }

   return TYPE_UNKNOWN;
}



//////////////////////////////
//
// printUnknownInterpretations --
//

void printUnknownInterpretations(HumdrumFile& infile) {
   Array<SigString> unknowns;
   unknowns.setSize(0);

   char buffer[1024] = {0};
   int i, j;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!validline(infile, i)) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (keysigQ && (!infile[i].isExInterp(j, "**kern"))) {
            continue;
         }
         getInterpName(buffer, infile[i], j);
         if (strcmp(buffer, "unknown") != 0) {
            continue;
         }
         cout << infile[i][j] << endl;
      }
   }
}



//////////////////////////////
//
// sortAllBlocks --
//

void sortAllBlocks(HumdrumFile& infile) {
   Array<SigString> unknowns;
   unknowns.setSize(0);

   int i, j;
   int count;
   int starti;
   int endi;
   int blockcount = 0;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!validline(infile, i)) {
         if (!warnQ) {
            cout << infile[i] << "\n";
         }
         continue;
      }
      starti = i;
      count = getInterpretationCount(infile, i);

      if (BlockProcess[blockcount] == 0) {
         // this block is not supposed to be altered
         for (j=0; j<count; j++) {
            cout << infile[i+j] << "\n";
         }
         i += count - 1;
         blockcount++;
         continue;
      }

      i += count - 1;
      endi = i;
      printSortedInterpretations(infile, starti, endi, blockcount++);
   }
}



//////////////////////////////
//
// printSortedInterpretations --
//

void printSortedInterpretations(HumdrumFile& infile, int starti, int endi,
      int blockcount) {

   Array<Array<int> > types;
   int rows = endi - starti + 1;
   int cols = infile[starti].getFieldCount();
   if (cols != infile[endi].getFieldCount()) {
      cerr << "Funny error" << endl;
      exit(1);
   }

   types.setSize(rows);
   types.allowGrowth(0);
   int i, j;
   for (i=0; i<types.getSize(); i++) {
      types[i].setSize(cols);
      types[i].allowGrowth(0);
      for (j=0; j<types[i].getSize(); j++) {
         types[i][j] = getInterpNum(infile[starti+i], j);
      }
   }

   if (warnQ) {
      checkForWarnings(infile, starti, endi, types);
      return;
   }

   Array<int> sortOrder;
   sortOrder.setSize(TYPE_MAX*2);
   sortOrder.setSize(0);

   int value;
   if (FirstSort[blockcount] == 0) {
      // force expandlabel to be sorted on top of block
      // if the block is not a first-type.
      value = TYPE_EXPANDLABEL;
      sortOrder.append(value);
   }


   // Add other interpretations
   for (i=0; i<TYPE_MAX; i++) {
      value = i+1;
      if ((FirstSort[blockcount] == 0) && (value == TYPE_EXPANDLABEL)) {
         // do nothing
      } else {
         sortOrder.append(value);
      }
   }
   sortOrder.allowGrowth(0);

   if (FirstSort[blockcount] == 0) {
      processInterpretationType(TYPE_EXPANDLABEL, infile, starti, types, 
            !FirstSort[blockcount]);
   }

   if (!unknownlastQ) {
      printUnknownLines(infile, starti, types);
   }

   for (i=0; i<sortOrder.getSize(); i++) {
      processInterpretationType(sortOrder[i], infile, starti, types, 
            !FirstSort[blockcount]);
   }

   if (unknownlastQ) {
      printUnknownLines(infile, starti, types);
   }
}



//////////////////////////////
//
// checkForWarnings -- 
//

void checkForWarnings(HumdrumFile& infile, int starti, int endi, 
      Array<Array<int> >& types) {
   int i, j;
   int mixed = 0;
   static const char* LASTFILENAME = "XAEASDGASGAEG";


   for (i=0; i<types.getSize(); i++) {
      mixed = checkForMixedInterpretations(types[i]);
      if (mixed) {
         if (filenameQ && (strcmp(LASTFILENAME, FILENAME) != 0)) {
            cout << FILENAME << "\t=================" << "\n";
            LASTFILENAME = FILENAME;
         }
         cout << "!!WARNING: mixed interpretation types on line " 
              << starti+i+1 << "\n";
         cout << infile[starti+i] << "\n";
      }
      for (j=0; j<types[i].getSize(); j++) {
         if (types[i][j] == TYPE_UNKNOWN) {
            if (filenameQ && (strcmp(LASTFILENAME, FILENAME) != 0)) {
               cout << FILENAME << "\t=================" << "\n";
               LASTFILENAME = FILENAME;
            }
            cout << "!!UNKNOWN interpretation on line " << starti+i+1 << ":\t"
                 << infile[starti+i][j] << "\n";
         }
      }
   }
}



//////////////////////////////
//
// checkForMixedInterpretations --
//

int checkForMixedInterpretations(Array<int>& types) {
   double found = 123456;
   int i;
   for (i=0; i<types.getSize(); i++) {
      if (types[i] == TYPE_USED) {
         // shouldn't happen, but just in case
         continue;
      }
      if (types[i] == TYPE_EMPTY) {  
         continue;
      }
      if (found == 123456) {
         found = types[i];
         continue;
      }
      if (found != types[i]) {
         return 1;
      }
   }
   return 0;
}



//////////////////////////////
//
// printUnknownLines --
//

void printUnknownLines(HumdrumFile& infile, int starti, 
      Array<Array<int> >& types) {

   // first print all lines which consist only of 
   // unknowns, used, or empty interpretations.

   int i, j;
   int hasunknown;
   for (i=0; i<types.getSize(); i++) {
      hasunknown = 0;
      for (j=0; j<types[i].getSize(); j++) {
         if (types[i][j] == TYPE_UNKNOWN) {
            hasunknown = 1;
            break;
         }
      }
      if (hasunknown) {
         for (j=0; j<types[i].getSize(); j++) {
            if (types[i][j] == TYPE_UNKNOWN) {
               cout << infile[starti+i][j];
            } else {
               cout << "*";
            }
            if (j<types[i].getSize()-1) {
               cout << "\t";
            }
         }
         cout << "\n";
      }
   }
}



//////////////////////////////
//
// processInterpretationType --
//

void processInterpretationType(int target,  HumdrumFile& infile, 
      int starti, Array<Array<int> >& types, int blockcount) {
   int hastype = hasType(target, types, blockcount);
   while (hastype) {
      printInterpretationType(target, infile, starti, types, blockcount);
      if (target == TYPE_KEYSIG) {
         blockcount++;  // prevent recursive auto keysig printing
      }
      hastype = hasType(target, types, blockcount);
      if ((hastype > 1) && blockcount) {
         hastype = 0;
      }
   }
}



//////////////////////////////
//
// hasType --
//

int hasType(int target, Array<Array<int> >& types, int blockcount) {
   int i, j;

   if ((blockcount == 0) && keysigQ && (target == TYPE_KEYSIG)) {
      return 2;
   }

   for (i=0; i<types.getSize(); i++) {
      for (j=0; j<types[i].getSize(); j++) {
         if (types[i][j] == target) {
            return 1;
         }
      }
   }
   return 0;
}



//////////////////////////////
//
// printInterpretationType --
//

void printInterpretationType(int target, HumdrumFile& infile, int starti, 
      Array<Array<int> >& types, int blockcount) {
   int i, j;
   int found;
   for (j=0; j<types[0].getSize(); j++) {
      found = -1;
      for (i=0; i<types.getSize(); i++) {
         if (target == types[i][j]) {
            found = i;
            types[i][j] = TYPE_USED;
            break;
         }
      }
      if (found >= 0) {
         cout << infile[starti+found][j];
      } else {
         if (keysigQ && (blockcount == 0) && (target == TYPE_KEYSIG)) {
            cout << "*k[]";
         } else {
            cout << "*";
         }
      }
      if (j<types[0].getSize()-1) {
         cout << "\t";
      }
   }
   cout << "\n";
}



//////////////////////////////
//
// getInterpretationCount --
//

int getInterpretationCount(HumdrumFile& infile, int line) {
   int i;
   int count = 0;
   for (i=line; i<infile.getNumLines(); i++) {
      if (validline(infile, i)) {
         count++;
      } else {
         break;
      }
   }
   return count;
}



//////////////////////////////
//
// getBlockList --
//

void getBlockList(HumdrumFile& infile, Array<int>& starti, Array<int>& endi, 
   int initQ) {

   starti.setSize(10000);
   starti.setGrowth(100000);
   starti.setSize(0);

   endi.setSize(10000);
   endi.setGrowth(100000);
   endi.setSize(0);

   int minusone = -1;
   int i;
   int count;
   int blockcount = 0;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!validline(infile, i)) {
         continue;
      }
      if (!initQ) {
         if (BlockProcess[blockcount] == 0) {
            // ignore this interpretation block
            count = getInterpretationCount(infile, i);
            i += count - 1;
            blockcount++;
            starti.append(minusone);
            endi.append(minusone);
            continue;
         }
      }
      starti.append(i);
      count = getInterpretationCount(infile, i);
      i += count - 1;
      endi.append(i);
      blockcount++;
   }
}
   


//////////////////////////////
//
// printBlocks --
//

void printBlocks(HumdrumFile& infile) {
   int i, j;
   int k;
   int count;
   int blocknum = 0;
   char buffer[1024] = {0};
   for (i=0; i<infile.getNumLines(); i++) {
      if (!validline(infile, i)) {
         continue;
      }
      count = getInterpretationCount(infile, i);
      if (BlockProcess[blocknum] == 0) {
         i += count - 1;
         blocknum++;
         continue;
      }
      blocknum++;
      cout << "!!Blocknum=" << blocknum
           << "\tstart=" << i+1
           << "\tend=" << i+count
           << endl;
      for (j=i; j<i+count; j++) {
         cout << infile[j] << endl;
         if (typeQ) {
            for (k=0; k<infile[j].getFieldCount(); k++) {
               cout << "!" << getInterpName(buffer, infile[j], k);
               if (k < infile[j].getFieldCount()-1) {
                  cout << "\t";
               }
            }
            cout << "\n";
         }
      }
      i += count - 1;
   }
}



//////////////////////////////
//
// validline -- true if an interpretation line with no exclusive
//      interpretations and no spine manipulators.
//

int validline(HumdrumFile& infile, int i) {
   if (!infile[i].isInterpretation()) {
      return 0;
   }
   if (strncmp(infile[i][0], "**", 2) == 0) {
      return 0;
   }
   if (infile[i].isSpineManipulator()) {
      // *^ *v *+ *x ** interpreations are not allowed
      // to mix with tandem interpreatations

      if (warnQ) {
         int j;
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (strcmp(infile[i][j], "*") == 0) {
               continue;
            }
            if (strcmp(infile[i][j], "*v") == 0) {
               continue;
            }
            if (strcmp(infile[i][j], "*^") == 0) {
               continue;
            }
            if (strcmp(infile[i][j], "*x") == 0) {
               continue;
            }
            if (strcmp(infile[i][j], "*+") == 0) {
               continue;
            }
            if (strcmp(infile[i][j], "*-") == 0) {
               continue;
            }
            if (strncmp(infile[i][j], "**", 2) == 0) {
               continue;
            }
            cout << "!!ERROR: mixing tandem and spine "
                 << "manipulator interpretations on line " << i+1 << ":\n";
            cout << infile[i] << "\n";
            break;
         }
      }

      return 0;
   }
     
   return 1;
}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("t|type=b",     "Display the interpretation type");
   opts.define("l|list=b",     "Display a list of the interpretation blocks");
   opts.define("u|unknown=b",  "Display a list unknown interpretations");
   opts.define("last=b",       "Place unknown interpreations last");
   opts.define("k|keysig=b",   "Add key signatures if not present");
   opts.define("f|filename=b", "Print filename (used with -u)");
   opts.define("w|warn=b",     "Warning about mismatched or unknown interps");
   opts.define("b|block=s:1-$","Process only specified blocks");
   opts.define("s|search=s:",  "Search for a partcular interpretation");
   opts.define("S|search-list=b","Display search list");
   opts.define("F|first=s:1",   "Treat the block list as sorting by first");
   opts.define("L|list-search=s", "List interpretations of a particular type");

   opts.define("debug=b");              // determine bad input line num
   opts.define("author=b");             // author of program
   opts.define("version=b");            // compilation info
   opts.define("example=b");            // example usages
   opts.define("h|help=b");             // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, June 2012" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: June 2012" << endl;
      cout << "compiled: " << __DATE__ << endl;
      cout << MUSEINFO_VERSION << endl;
      exit(0);
   } else if (opts.getBoolean("help")) {
      usage(opts.getCommand());
      exit(0);
   } else if (opts.getBoolean("example")) {
      example();
      exit(0);
   }

   if (opts.getBoolean("search-list")) {
      printSearchList();
      exit(0);
   }

   blockQ       = opts.getBoolean("block");
   BlockString  = opts.getString("block");
   typeQ        = opts.getBoolean("type");
   listblocksQ  = opts.getBoolean("list");
   listunknownQ = opts.getBoolean("unknown");
   keysigQ      = opts.getBoolean("keysig");
   filenameQ    = opts.getBoolean("filename");
   unknownlastQ = opts.getBoolean("last");
   warnQ        = opts.getBoolean("warn");
   debugQ       = opts.getBoolean("debug");
   searchQ      = opts.getBoolean("search");
   SearchString = opts.getString("search");
   FirstString  = opts.getString("first");
   searchlistQ  = opts.getBoolean("list-search");
   ListString   = opts.getString("list-search");

}



//////////////////////////////
//
// example -- example usage of the quality program
//

void example(void) {
   cout <<
   "                                                                         \n"
   "                                                                         \n"
   << endl;
}



//////////////////////////////
//
// usage -- gives the usage statement for the meter program
//

void usage(const char* command) {
   cout <<
   "                                                                         \n"
   "                                                                         \n"
   << endl;
}


///////////////////////////////////////////////////////////////////////////
//
// Spine field list extraction functions
//

//////////////////////////////
//
// fillFieldData --
//

void fillFieldData(Array<int>& field, const char* fieldstring, 
      HumdrumFile& infile) {

   Array<int> starti;
   Array<int> endi;

   getBlockList(infile, starti, endi, 1);
   int maxblock = starti.getSize();
   if (debugQ) {
      cout << "BLOCK COUNT = " << maxblock << endl;
   }

   field.setSize(maxblock);
   field.setGrowth(0);
   field.setAll(0);

   Array<int> tempfield;
   tempfield.setSize(maxblock);
   tempfield.setSize(0);

   PerlRegularExpression pre;
   Array<char> buffer;
   buffer.setSize(strlen(fieldstring)+1);
   strcpy(buffer.getBase(), fieldstring);
   pre.sar(buffer, "\\s", "", "gs");
   int start = 0;
   int value = 0;
   value = pre.search(buffer.getBase(), "^([^,]+,?)");
   while (value != 0) {
      start += value - 1;
      start += strlen(pre.getSubmatch(1));
      processFieldEntry(tempfield, pre.getSubmatch(), infile, maxblock);
      value = pre.search(buffer.getBase() + start, "^([^,]+,?)");
   }

   int i;
   for (i=0; i<tempfield.getSize(); i++) {
     field[tempfield[i]-1] = 1; 
   }
}



//////////////////////////////
//
// processFieldEntry -- 
//   3-6 expands to 3 4 5 6
//   $   expands to maximum block number
//   $-1 expands to maximum spine track minus 1, etc.
//

void processFieldEntry(Array<int>& field, const char* string, 
      HumdrumFile& infile, int maxblock) {

   PerlRegularExpression pre;
   Array<char> buffer;
   buffer.setSize(strlen(string)+1);
   strcpy(buffer.getBase(), string);

   // remove any comma left at end of input string (or anywhere else)
   pre.sar(buffer, ",", "", "g");

   if (debugQ) {
      cout << "MAXBLOCK = " << maxblock << endl;
      cout << "INPUT BLOCK STRING TO    DOLLAR: " << buffer << endl;
   }
   // first remove $ symbols and replace with the correct values
   removeDollarsFromString(buffer, maxblock);
   if (debugQ) {
      cout << "OUTPUT BLOCK STRING FROM DOLLAR: " << buffer << endl;
   }

   if (pre.search(buffer.getBase(), "^(\\d+)-(\\d+)$")) {
      int firstone = strtol(pre.getSubmatch(1), NULL, 10);
      int lastone  = strtol(pre.getSubmatch(2), NULL, 10);

      if ((firstone < 1) && (firstone != 0)) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains too small a number at start: " << firstone << endl;
         cerr << "Minimum number allowed is " << 1 << endl;
         exit(1);
      }
      if ((lastone < 1) && (lastone != 0)) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains too small a number at end: " << lastone << endl;
         cerr << "Minimum number allowed is " << 1 << endl;
         exit(1);
      }
      if (firstone > maxblock) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains number too large at start: " << firstone << endl;
         cerr << "Maximum number allowed is " << maxblock << endl;
         exit(1);
      }
      if (lastone > maxblock) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains number too large at end: " << lastone << endl;
         cerr << "Maximum number allowed is " << maxblock << endl;
         exit(1);
      }

      int i;
      if (firstone > lastone) {
         for (i=firstone; i>=lastone; i--) {
            field.append(i);
         }
      } else {
         for (i=firstone; i<=lastone; i++) {
            field.append(i);
         }
      }
   } else if (pre.search(buffer.getBase(), "^(\\d+)")) {
      int value = strtol(pre.getSubmatch(1), NULL, 10);
      if ((value < 1) && (value != 0)) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains too small a number at end: " << value << endl;
         cerr << "Minimum number allowed is " << 1 << endl;
         exit(1);
      }
      if (value > maxblock) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains number too large at start: " << value << endl;
         cerr << "Maximum number allowed is " << maxblock << endl;
         exit(1);
      }
      field.append(value);
   }
}



//////////////////////////////
//
// removeDollarsFromString -- substitute $ sign for maximum track count.
//

void removeDollarsFromString(Array<char>& buffer, int maxblock) {
   PerlRegularExpression pre;
   char buf2[128] = {0};
   int value2;
   if (debugQ) {
      cout << "IN DOLLAR STRING MAXBLOCK = " << maxblock << endl; 
   }

   if (pre.search(buffer.getBase(), "\\$$")) {
      sprintf(buf2, "%d", maxblock);
      pre.sar(buffer, "\\$$", buf2);
   }

   if (debugQ) {
      cout << "IN DOLLAR STRING = " << buffer << endl; 
   }

   if (pre.search(buffer.getBase(), "\\$(?![\\d-])")) {
      // don't know how this case could happen, however...
      sprintf(buf2, "%d", maxblock);
      pre.sar(buffer, "\\$(?![\\d-])", buf2, "g");
   }

   if (pre.search(buffer.getBase(), "\\$0")) {
      // replace $0 with maxblock (used for reverse orderings)
      sprintf(buf2, "%d", maxblock);
      pre.sar(buffer, "\\$0", buf2, "g");
   }

   while (pre.search(buffer.getBase(), "\\$(-?\\d+)")) {
      value2 = maxblock - (int)fabs(strtol(pre.getSubmatch(1), NULL, 10));
      sprintf(buf2, "%d", value2);
      pre.sar(buffer, "\\$-?\\d+", buf2);
   }

}


//
// Spine field list extraction functions
//
///////////////////////////////////////////////////////////////////////////



// md5sum: 0e713a16488534ca9d497bf76e394a27 prettystar.cpp [20120615]
