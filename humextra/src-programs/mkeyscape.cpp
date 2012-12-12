//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb  3 16:03:55 PST 2008
// Last Modified: Sat May 30 22:54:41 PDT 2009 added -b option
// Last Modified: Tue Jun  9 00:55:43 PDT 2009 added fill & trim
// Last Modified: Thu Apr 14 15:41:23 PDT 2011 fixed occasional ob1 err with -n
// Last Modified: Sun May  1 10:32:02 PDT 2011 secondary key display
// Last Modified: Wed Nov  9 17:34:49 PST 2011 fixed some irritating problems
// Last Modified: Sun Oct 21 15:33:59 PDT 2012 added -k option
//
// Filename:      ...sig/examples/all/mkeyscape.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/mkeyscape.cpp
// Syntax:        C++; museinfo
//
// Description:   Create Fast Keyscapes using MIDI file or Humdrum file.
//

#include "humdrum.h"
#include "MidiFile.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>             /* for qsort and bsearch functions */

#ifndef OLDCPP
   #include <iostream>
   #include <fstream>
#else
   #include <iostream.h>
   #include <fstream.h>
#endif

#define HISTTYPE double

#define UNKNOWNFILE 0
#define HUMDRUMFILE 1 
#define MIDIFILE    2 

// function declarations:
void     checkOptions           (Options& opts, int argc, char** argv);
void     example                (void);
void     usage                  (const char* command);
double   loadHistogramFromHumdrumFile
                                (Array<Array<HISTTYPE> >& histogram,
                                 HumdrumFile& infile,
                                 const char* filename, int segments);
double   loadHistogramFromMidiFile
                                (Array<Array<HISTTYPE> >& histogram,
                                 const char* filename, int segments);
void     printBaseHistogram     (Array<Array<HISTTYPE> >& histogram);
void     printBaseHistogramHumdrumStyle     
                                (Array<Array<HISTTYPE> >& histogram, 
				 double width);
void     printNormalizedHistogram(Array<Array<HISTTYPE> >& histogram);
void     addToHistogramInteger  (Array<Array<int> >& histogram, int pc,
                                 double start, double dur, double tdur,
                                 int segments);
void     addToHistogramDouble   (Array<Array<double> >& histogram, int pc,
                                 double start, double dur, double tdur,
                                 int segments);
void     fillAllHistograms      (Array<Array<Array<HISTTYPE> > >& histograms);
void     printBest              (Array<Array<Array<HISTTYPE> > >& histogram);
void     calculateBestKeys      (Array<Array<Array<HISTTYPE> > >& histograms);
void     identifyKey            (Array<HISTTYPE>& histogram);
void     displayRawAnalysis     (Array<Array<Array<HISTTYPE> > >& histogram);
void     displayAnalysisHistogram(Array<Array<Array<HISTTYPE> > >& histograms);
void     identifyKeyDouble      (Array<HISTTYPE>& histogram);
void     printPPM               (Array<Array<Array<HISTTYPE> > >& histograms,
                                 HumdrumFile& infile);
double   pearsonCorrelation     (int size, double* x, double* y);
void     setFilterOptions       (Array<int>& channelfilter, 
                                 const char* exclude);
void     processColorFile       (const char* filename, HumdrumFile& cfile);
int      ranksort               (const void* A, const void* B);
void     fillWeightsWithAardenEssen
                                (Array<HISTTYPE>& maj, Array<HISTTYPE>& min);
void     fillWeightsWithBellmanBudge
                                (Array<HISTTYPE>& maj, Array<HISTTYPE>& min);
void     fillWeightsWithKostkaPayne
                                (Array<HISTTYPE>& maj, Array<HISTTYPE>& min);
void     fillWeightsWithKrumhanslKessler
                                (Array<HISTTYPE>& maj, Array<HISTTYPE>& min);
void     fillWeightsWithSimpleValues
                                (Array<HISTTYPE>& maj, Array<HISTTYPE>& min);
void     processWeights         (const char* filename, Array<HISTTYPE>& major, 
                                 Array<HISTTYPE>& minor);
void     printColorMap          (Array<const char*>& colorindex);
void     printWeights           (Array<HISTTYPE>& maj, Array<HISTTYPE>& min);
void     printKeyAnalysisCorr   (Array<Array<Array<HISTTYPE> > >& histograms, 
                                 int level);
void     printKeyCorrelations   (Array<HISTTYPE>& histogram);
void     changeColorMapping     (Array<const char*>& ci, const char* type);
void     fillColorMapping_castel(Array<const char*>& ci);
void     fillColorMapping_newton(Array<const char*>& ci);
void     doBlankAnalysis        (Array<Array<Array<HISTTYPE> > >& histograms);
void     recurseMarkMask        (Array<Array<int> >& mask, 
                                 Array<Array<Array<HISTTYPE> > >& hist, 
                                 int starti, int startj, int mcounter);
void     doFillBlanks           (Array<Array<Array<HISTTYPE> > >& histograms, 
                                 Array<Array<int> >& mask);
void     fillBoundedArea        (double target, int regionid, int line, 
                                 int col, Array<Array<int> >& mask, 
                                 Array<Array<Array<double> > >& histograms);
int      isBounded              (int target, int line, int col, 
                                 Array<Array<char> >& tm, 
                                 Array<Array<int> >& mask);
int      isValidCell            (int line, int col, Array<Array<int> >& mask);
void     printLegend            (int legendheight, int legendwidth);
void     printNumbers           (HumdrumFile& infile, int numberheight, 
                                 int numberwidth);
double   getMeasureSize         (HumdrumFile& infile, int width);
void     doTrim                 (Array<Array<int> >& mask, 
                                 Array<Array<Array<HISTTYPE> > >& histograms);
void     trimRegion             (int start, int end, Array<Array<int> >& mask,
                                 Array<Array<Array<HISTTYPE> > >& histograms,
                                 int color);
void     doRegionID             (Array<Array<int> >& mask, 
                                 Array<Array<Array<HISTTYPE> > >& histograms);
void     fillSurroundedBlanks   (Array<Array<Array<HISTTYPE> > >& histograms, 
                                 Array<Array<int> >& mask, 
				 Array<int> blanksonrow);
void     trimEdges              (Array<Array<Array<HISTTYPE> > >& histograms);
int      hasdigit               (const char* strang);

// User interface variables:
Options   options;
int       segments     = 300;   // used with -s option
int       rawQ         = 0;     // used with -r option
int       histQ        = 0;     // used with --hist option
int       khistQ       = 0;     // used with --khist option
int       transpose    = 0;     // used with -t option
int       rrotate      = 0;     // used with --rotate option
int       corQ         = 0;     // used with --cor option
int       corlevel     = 4;     // used with --cor option
int       blankQ       = 0;     // used with -b option
int       fillQ        = 0;     // used with -f option
int       legendQ      = 0;     // used with -l option
int       outlineQ     = 1;     // used with --no-outline
int       numberQ      = 0;     // used with -n option
int       trimQ        = 0;     // used with --trim option
int       averageQ     = 0;     // used with --average option
int       secondQ      = 0;     // used with --second option
int       keyQ         = 0;     // used with -k option

Array<int> channelfilter;       // used with -x option
Array<const char*> colorindex;  // used with -c option
HumdrumFile colorfile;          // used with -c option (needs to be global)
Array<HISTTYPE> majorweights;
Array<HISTTYPE> minorweights;

Array<HISTTYPE> aamajor;
Array<HISTTYPE> aaminor;
Array<HISTTYPE> bbmajor;
Array<HISTTYPE> bbminor;
Array<HISTTYPE> kpmajor;
Array<HISTTYPE> kpminor;
Array<HISTTYPE> kkmajor;
Array<HISTTYPE> kkminor;
Array<HISTTYPE> ssmajor;
Array<HISTTYPE> ssminor;


//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   channelfilter.setSize(16);
   channelfilter.allowGrowth(0);
   channelfilter.setAll(1);

   majorweights.setSize(12);
   majorweights.allowGrowth(0);
   majorweights.zero(0);

   minorweights.setSize(12);
   minorweights.allowGrowth(0);
   minorweights.zero(0);

   aamajor.setSize(12);
   aamajor.allowGrowth(0);
   aamajor.zero(0);
   aaminor.setSize(12);
   aaminor.allowGrowth(0);
   aaminor.zero(0);

   bbmajor.setSize(12);
   bbmajor.allowGrowth(0);
   bbmajor.zero(0);
   bbminor.setSize(12);
   bbminor.allowGrowth(0);
   bbminor.zero(0);

   kkmajor.setSize(12);
   kkmajor.allowGrowth(0);
   kkmajor.zero(0);
   kkminor.setSize(12);
   kkminor.allowGrowth(0);
   kkminor.zero(0);

   kpmajor.setSize(12);
   kpmajor.allowGrowth(0);
   kpmajor.zero(0);
   kpminor.setSize(12);
   kpminor.allowGrowth(0);
   kpminor.zero(0);

   ssmajor.setSize(12);
   ssmajor.allowGrowth(0);
   ssmajor.zero(0);
   ssminor.setSize(12);
   ssminor.allowGrowth(0);
   ssminor.zero(0);

   fillWeightsWithAardenEssen(aamajor, aaminor);
   fillWeightsWithBellmanBudge(bbmajor, bbminor);
   fillWeightsWithKostkaPayne(kpmajor, kpminor);
   fillWeightsWithSimpleValues(ssmajor, ssminor);
   fillWeightsWithKrumhanslKessler(kkmajor, kkminor);

//fillWeightsWithSimpleValues(majorweights, minorweights);
//fillWeightsWithKrumhanslKessler(majorweights, minorweights);
//fillWeightsWithKostkaPayne(majorweights, minorweights);
fillWeightsWithBellmanBudge(majorweights, minorweights);
//fillWeightsWithAardenEssen(majorweights, minorweights);

   colorindex.setSize(26);
   colorindex.allowGrowth(0);
   colorindex[0]  = "0 255 0";		// C major
   colorindex[1]  = "38 255 140";	// C-sharp major
   colorindex[2]  = "63 95 255";	// D major
   colorindex[3]  = "228 19 83";	// E-flat major
   colorindex[4]  = "255 0 0";		// E major
   colorindex[5]  = "255 255 0";	// F major
   colorindex[6]  = "192 255 0";	// F-sharp major
   colorindex[7]  = "93 211 255";	// G major
   colorindex[8]  = "129 50 255";	// A-flat major
   colorindex[9]  = "205 41 255";	// A major
   colorindex[10] = "255 160 0";	// B-flat major
   colorindex[11] = "255 110 10";	// B major
   colorindex[12] = "0 161 0";		// C minor
   colorindex[13] = "15 191 90";	// C-sharp minor
   colorindex[14] = "37 61 181";	// D minor
   colorindex[15] = "184 27 75";	// E-flat minor
   colorindex[16] = "175 0 0";		// E minor
   colorindex[17] = "220 200 0";	// F minor
   colorindex[18] = "140 200 0";	// F-sharp minor
   colorindex[19] = "65 163 181";	// G minor
   colorindex[20] = "100 28 181";	// G-sharp minor
   colorindex[21] = "136 13 181";	// A minor
   colorindex[22] = "181 93 20";	// B-flat minor
   colorindex[23] = "211 107 0";	// B minor
   colorindex[24] = "0 0 0";		// silence
   colorindex[25] = "255 255 255";	// background

   // process the command-line options
   checkOptions(options, argc, argv);

   Array<Array<Array<HISTTYPE> > > histograms;
   histograms.setSize(segments);
   histograms.allowGrowth(0);
   int i;
   int j;
   for (i=0; i<segments; i++) {
      histograms[i].setSize(i+1);
      histograms[i].allowGrowth(0);
      for (j=0; j<i+1; j++) {
         histograms[i][j].setSize(14);   // index 12 is for best key result
                                         // index 13 for second best key result
         histograms[i][j].allowGrowth(0);
         histograms[i][j].zero();
      }
   }

   HumdrumFile infile;
   double totalduration = 0;
   const char* filename = "";
   int   filetype       = UNKNOWNFILE;  
   if (options.getArgCount() > 0) {
      filename = options.getArg(1);
   }
   int fnlength = strlen(filename);
   if (fnlength != 0) {
      if (strcmp(filename + (fnlength - 4), ".mid") == 0) {
         filetype = MIDIFILE;
         totalduration = loadHistogramFromMidiFile(
               histograms[histograms.getSize()-1], filename, segments);
      } else {
         filetype = HUMDRUMFILE;
         totalduration = loadHistogramFromHumdrumFile(
               histograms[histograms.getSize()-1], infile, filename, segments);
      }
   } else {
      filetype = HUMDRUMFILE;
      totalduration = loadHistogramFromHumdrumFile(
               histograms[histograms.getSize()-1], infile, filename, segments);
   }
   if ((filetype != HUMDRUMFILE) && numberQ) {
      // turn off barline numbering axis if input file is not a Humdrum file
      numberQ = 0;
   }

   fillAllHistograms(histograms);

   if (corQ) {
      printKeyAnalysisCorr(histograms, corlevel);
      exit(0);
   }

   if (histQ) {
      // printBaseHistogram(histograms[histograms.getSize()-1]);
      printBaseHistogramHumdrumStyle(histograms[histograms.getSize()-1],
            totalduration);
      exit(0);
   }
   //printNormalizedHistogram(histograms[0]);
   //printBaseHistogram(histograms[1]);
   //cout << "========================" << endl;
   //printBaseHistogram(histograms[histograms.getSize()-1]);

   //identifyKey(histograms[0][0]);
   //exit(0);

   calculateBestKeys(histograms);

   if (blankQ) {
      doBlankAnalysis(histograms);
   }

   //printBest(histograms);

   if (rawQ) {
      displayRawAnalysis(histograms);
      exit(0);
   } else if (khistQ) {
      displayAnalysisHistogram(histograms);
      exit(0);
   } 

   printPPM(histograms, infile);

   return 0;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// doBlankAnalysis -- remove non-plausible key analysis regions.
//

void doBlankAnalysis(Array<Array<Array<HISTTYPE> > >& histograms) {

   Array<Array<int> > mask;
   doRegionID(mask, histograms);

   if (fillQ) {
      doFillBlanks(histograms, mask);
      if (trimQ) {
         trimEdges(histograms);
         doRegionID(mask, histograms);
         doTrim(mask, histograms);
         doRegionID(mask, histograms);
         doFillBlanks(histograms, mask);
      }
   }
}



//////////////////////////////
//
// doRegionID -- Mark each region with a unique serial number.
//  Regions which are not touching the bottom of the plot are
//  assigned to be region 0;
//

void doRegionID(Array<Array<int> >& mask, 
      Array<Array<Array<HISTTYPE> > >& histograms) {

   mask.setSize(histograms.getSize());
   mask.allowGrowth(0);
   int mcounter = 1;
   int i, j;
   for (i=0; i<histograms.getSize(); i++) {
      mask[i].setSize(histograms[i].getSize());
      mask[i].allowGrowth(0);
      mask[i].setAll(0);
   }

   int maxline = histograms.getSize()-1;
   for (i=0; i<histograms[maxline].getSize(); i++) {
      if ((histograms[maxline][i][12] < 0) || 
          (histograms[maxline][i][12] >= 24)) {
         continue;
      }
      if (mask[maxline][i] == 0) {
         mask[maxline][i] = mcounter++;
	 if (i < histograms[maxline-1].getSize()) {
            if ((mask[maxline-1][i] == 0) && 
                (histograms[maxline-1][i][12] == histograms[maxline][i][12])) {
               mask[maxline-1][i] = mcounter;
               recurseMarkMask(mask, histograms, maxline-1, i, mcounter);
            }
	 }
         if (i < histograms[maxline-1].getSize()-1) {
            if ((mask[maxline-1][i+1] == 0) && 
                (histograms[maxline-1][i+1][12]==histograms[maxline][i][12])) {
               mask[maxline-1][i+1] = mcounter;
               recurseMarkMask(mask, histograms, maxline-1, i, mcounter);
            }
         }
      }
   }

   for (i=0; i<histograms.getSize(); i++) {
      for (j=0; j<histograms[i].getSize(); j++) {
         if (mask[i][j] == 0) {
            histograms[i][j][12] = 24; // use silence marker (black)
         }
      }
   }
}



//////////////////////////////
//
// doFillBlanks --
//

void doFillBlanks(Array<Array<Array<HISTTYPE> > >& histograms, 
      Array<Array<int> >& mask) {

   int blankcount;
   int blankendi;
   int colsize;
   int i, j;

   Array<int> blanksonrow;
   blanksonrow.setSize(histograms.getSize());
   blanksonrow.allowGrowth(0);
   blanksonrow.setAll(0);

   for (i=0; i<histograms.getSize(); i++) {
      blankcount = 0;
      blankendi = -1;
      colsize = histograms[i].getSize();

      // traverse top-down / right-left
      for (j=0; j<colsize; j++) {
         if (mask[i][j] != 0) {
            continue;
         }
         // check to the left:
         if (j > 0) {
            if (histograms[i][j][13] == histograms[i][j-1][12]) {
               histograms[i][j][12] = histograms[i][j-1][12];
	       mask[i][j] = mask[i][j-1];
               continue;
            }
         }
         // check above:
         if (i > 0) {
            // check above right:
            if (j < colsize-1) {
               if (histograms[i][j][13] == histograms[i-1][j][12]) {
                  histograms[i][j][12] = histograms[i-1][j][12];
	          mask[i][j] = mask[i-1][j];
                  continue;
               }
            }
            // check above left:
            if (j > 0) {
               if (histograms[i][j][13] == histograms[i-1][j-1][12]) {
                  histograms[i][j][12] = histograms[i-1][j-1][12];
	          mask[i][j] = mask[i-1][j-1];
                  continue;
               }
            }
         }
         // the cell is still blanked, so keep track for revere checking
         blankcount++;
         blankendi = i;
      }

      // check in the other direction
      if (blankcount <= 0) {
         continue;
      }

      // traverse top-down / right-left
      for (j=blankendi; j>=0; j--) {
         if (mask[i][j] != 0) {
            continue;
         }
         // check to the right
	 if (j < colsize - 1) {
            if (histograms[i][j][13] == histograms[i][j+1][12]) {
               histograms[i][j][12] = histograms[i][j+1][12];
	       mask[i][j] = mask[i][j+1];
               continue;
            }
         }
         // check above:
         if (i > 0) {
            // check above right:
            if (j < colsize-1) {
               if (histograms[i][j][13] == histograms[i-1][j][12]) {
                  histograms[i][j][12] = histograms[i-1][j][12];
		  mask[i][j] = mask[i-1][j];
                  continue;
               }
            }
            // check above left:
            if (j > 0) {
               if (histograms[i][j][13] == histograms[i-1][j-1][12]) {
                  histograms[i][j][12] = histograms[i-1][j-1][12];
		  mask[i][j] = mask[i-1][j-1];
                  continue;
               }
            }
         }
         blankcount++;
      }
      blanksonrow[i] = blankcount;
   }


   // now search upwards from left and right...
   for (i=histograms.getSize()-2; i>=0; i--) {
      if (blanksonrow[i] == 0) {
         continue;
      }

      blankcount = 0;
      blankendi = -1;
      colsize = histograms[i].getSize();

      // traverse bottom-up / left-right
      for (j=0; j<colsize; j++) {
         if (mask[i][j] != 0) {
            continue;
         }
         // check to the left:
         if (j > 0) {
            if (histograms[i][j][13] == histograms[i][j-1][12]) {
               histograms[i][j][12] = histograms[i][j-1][12];
	       mask[i][j] = mask[i][j-1];
               continue;
            }
         }
         // check below:
         if (i < histograms.getSize()-1) {
            // check below left:
            if (histograms[i][j][13] == histograms[i+1][j][12]) {
               histograms[i][j][12] = histograms[i+1][j][12];
	       mask[i][j] = mask[i+1][j];
               continue;
            }
            // check below right:
            if (j < histograms[i+1].getSize()-1) {
               if (histograms[i][j][13] == histograms[i+1][j+1][12]) {
                  histograms[i][j][12] = histograms[i+1][j+1][12];
	          mask[i][j] = mask[i+1][j+1];
                  continue;
               }
            }
         }
         // the cell is still blanked, so keep track for revere checking
         blankcount++;
         blankendi = i;
      }
       
      if (blankcount <= 0) {
         continue;
      }
      blankcount = 0;
      // traverse bottom-up / right-left
      for (j=blankendi; j>=0; j--) {
         if (mask[i][j] != 0) {
            continue;
         }
         // check to the right:
         if (j < colsize-1) {
            if (histograms[i][j][13] == histograms[i][j+1][12]) {
               histograms[i][j][12] = histograms[i][j+1][12];
	       mask[i][j] = mask[i][j+1];
               continue;
            }
         }
         // check below:
         if (i < histograms.getSize()-1) {
            // check below left:
            if (histograms[i][j][13] == histograms[i+1][j][12]) {
               histograms[i][j][12] = histograms[i+1][j][12];
	       mask[i][j] = mask[i+1][j];
               continue;
            }
            // check below right:
            if (j < histograms[i+1].getSize()-1) {
               if (histograms[i][j][13] == histograms[i+1][j+1][12]) {
                  histograms[i][j][12] = histograms[i+1][j+1][12];
	          mask[i][j] = mask[i+1][j+1];
                  continue;
               }
            }
         }
         // the cell is still blanked, so keep track for revere checking
         blankcount++;
         blankendi = i;
      }
      blanksonrow[i] = blankcount;
   }
   // do the same process in the opposite direction to catch
   // blanks bounded on the left...


   fillSurroundedBlanks(histograms, mask, blanksonrow);
}



//////////////////////////////
//
// fillSurroundedBlanks --
//

void fillSurroundedBlanks(Array<Array<Array<HISTTYPE> > >& histograms, 
      Array<Array<int> >& mask, Array<int> blanksonrow) {

   // Examine blank spots to see if they are completely
   // surrounded by the same color.  If so then fill the blank
   // spot with the surrounding color.  If a blank spot is found 
   // touching the edge of the triangle, then fill in the blank spot
   // if the other edges are sourrounded by the same color.

   Array<Array<char> > tempmask;
   tempmask.setSize(histograms.getSize());
   tempmask.allowGrowth(0);
   int i, j;
   for (i=0; i<histograms.getSize(); i++) {
      tempmask[i].setSize(histograms[i].getSize());
      tempmask[i].allowGrowth(0);
      tempmask[i].setAll(0);
   }

   int target;
   int regionid;
   for (i=0; i<blanksonrow.getSize(); i++) {
      if (blanksonrow[i] <= 0) {
         continue;
      }
      target = int(histograms[i][0][12]+0.1);
      regionid = mask[i][0];
      for (j=1; j<histograms[i].getSize(); j++) {
         if ((target < 24) && (mask[i][j] == 0)) {
            if (isBounded(regionid, i, j, tempmask, mask)) {
               // isBounded is not working yet...
               fillBoundedArea(target, regionid, i, j, mask, histograms);
            }
         } 
         target = int(histograms[i][j][12]+0.1);
         regionid = mask[i][j];
      }
   }
}



//////////////////////////////
//
// doTrim -- If a region is bounded by another region, then
//  trim the first region according to the width of the first
//  region.
//
// Traverse along the bottom of the plot.  If the sequence of
// regions is not in reverse sequential order, then draw
// the trim box.
//

void doTrim(Array<Array<int> >& mask, 
      Array<Array<Array<HISTTYPE> > >& histograms) {

   int i, j;
   int bottom = mask.getSize()-1;
   int size = mask[mask.getSize()-1].getSize();
   int color;
   for (i=size-2; i>=0; i--) {
      if ((mask[bottom][i+1] != 0) && (mask[bottom][i+1] < mask[bottom][i])) {
         for (j=i-1; j>=0; j--) {
            if (mask[bottom][j] == mask[bottom][i+1]) {
               color = int(histograms[bottom][i+1][12]+0.1);
               trimRegion(j, i+1, mask, histograms, color);
               break;
            }
         }
      }
   }

}



//////////////////////////////
//
// trimEdges --  If the top-level color is blocked along the edge
//   by another color, then connect the top-level color to the lower one.
//   This will all trimming to be possible on secondary key regions
//   which touch the leading and trailing edge of the plot.
//

void trimEdges(Array<Array<Array<HISTTYPE> > >& histograms) {
   int i, j;
   int target = int(histograms[0][0][12]+0.1);
   int state = -1;
   int limiter = 3;

   for (i=0; i<histograms.getSize()-limiter; i++) {
      if ((state < 0) && (histograms[i][0][12] != target)) {
         state = i;
      } else if ((state > 0) && (histograms[i][0][12] == target)) {
         for (j=i-1; (j>=state) && (j>=0); j--) {
            histograms[j][0][12] = target;
         }
      }
   }

   state = -1;
   Array<Array<Array<HISTTYPE> > >& h = histograms;

   for (i=0; i<histograms.getSize()-limiter; i++) {
      if ((state < 0) && (histograms[i][h[i].getSize()-1][12] != target)) {
         state = i;
      } else if ((state>0) && (histograms[i][h[i].getSize()-1][12] == target)) {
         for (j=i-1; (j>=state) && (j>=0); j--) {
            histograms[j][h[j].getSize()-1][12] = target;
         }
      }
   }

   // if the bottom quarter of the leading and trailing edge of the plot
   // match each other, but do not match the top-level analysis,
   // then replace the top-level with the majority bottom-half
   // leading and trailing edge value.
   Array<int> bestleading(25);
   Array<int> besttrailing(25);
   bestleading.allowGrowth(0);
   besttrailing.allowGrowth(0);
   bestleading.setAll(0);
   besttrailing.setAll(0);

   int maxlead = 0;
   int value;
   for (i=3*histograms.getSize()/4; i<histograms.getSize(); i++) {
      value = int(histograms[i][0][12]+0.1);
      bestleading[value]++;
      if ((value != maxlead) && (bestleading[value] > bestleading[maxlead])) {
         maxlead = value;
      }
   }

   int maxtrail = 0;
   for (i=3*histograms.getSize()/4; i<histograms.getSize(); i++) {
      value = int(histograms[i][h[i].getSize()-1][12]+0.1);
      besttrailing[value]++;
      if ((value!=maxtrail) && (besttrailing[value]>besttrailing[maxtrail])) {
         maxtrail = value;
      }
   }

   if (!((maxtrail == maxlead) && (maxtrail != target))) {
      // if the majority key of the bottom edges does not match
      // the top-level key, then re-assign the top-level to the 
      // majority lower-level key.
      Array<double> sums(24);
      sums.zero();
      int maxsum = 0;
      sums[0] = besttrailing[0] + bestleading[0];
      for (i=1; i<24; i++) {
         sums[i] = besttrailing[i] + bestleading[i];
         if (sums[i] > sums[maxsum]) {
            maxsum = i;
         }
      }
      if (maxsum != target) {
         maxtrail = maxsum;
      } else {
         return;
      }
   }

   target = maxtrail;
   histograms[0][0][12] = target;

   state = -1;
   for (i=0; i<histograms.getSize()-limiter; i++) {
      if ((state < 0) && (histograms[i][0][12] != target)) {
         state = i;
      } else if ((state > 0) && (histograms[i][0][12] == target)) {
         for (j=i-1; (j>=state) && (j>=0); j--) {
            histograms[j][0][12] = target;
         }
      }
   }

   state = -1;
   for (i=0; i<histograms.getSize()-limiter; i++) {
      if ((state < 0) && (histograms[i][h[i].getSize()-1][12] != target)) {
         state = i;
      } else if ((state>0) && (histograms[i][h[i].getSize()-1][12] == target)) {
         for (j=i-1; (j>=state) && (j>=0); j--) {
            histograms[j][h[j].getSize()-1][12] = target;
         }
      }
   }
}



//////////////////////////////
//
// trimRegion --
//

void trimRegion(int start, int end, Array<Array<int> >& mask,
      Array<Array<Array<HISTTYPE> > >& histograms, int color) {
   double trimratio = 1.1;

//color = 24;

   int height = end - start;
   height = int(height * trimratio + 0.5);  // trim a little higher than 1:1 ratio

   int bottom = mask.getSize()-1;
   int i;
   int jval = start;
     for (i=bottom; (i>=bottom-height) && jval>0; i-=2) {
        if (jval < mask[i].getSize()-1) {
           histograms[i][jval][12] = color;
           if (jval < mask[i-1].getSize()-1) {
              histograms[i-1][jval][12] = color;
           }
        } else {
           break;
        }
        jval--;
     }

   jval = end;
   for (i=bottom; (i>=bottom-height) && jval>=0; i-=2) {
      if (jval < mask[i].getSize()-1) {
         histograms[i][jval][12] = color;
         if (jval < mask[i-1].getSize()-1) {
            histograms[i-1][jval][12] = color;
         }
      } else {
         break;
      }
      jval--;
   }

   int top = bottom-height;
   if (top < 0) {
      top = 0;
   }
	      
   int topstart = start - height/2;
   if (topstart < 0) {
      topstart = 0;
   } 
   if (topstart > mask[top].getSize()-1) {
      topstart = mask[top].getSize()-1;
   } 
   int topend = end - height/2;
   if (topend < 0) {
      topend = 0;
   } 
   if (topend > mask[top].getSize()-1) {
      topend = mask[top].getSize()-1;
   } 

   for (i=topstart; i<=topend; i++) {
      histograms[top][i][12] = color;
   }
   
}



//////////////////////////////
//
// isBounded -- check that all 6 adjacent cells are either blank, 
//      a border, or the target. If so, then do the same check on 
//      all adjacent cells which are blank.
//

// If a neighbor is not blank or the target, then return false
#define CHECKCELL(x, y)                                                 \
   if (isValidCell(x, y, mask)) {                                       \
      if ((mask[x][y] != 0) && (mask[x][y] != target)) {                \
         return 0;                                                      \
      }                                                                 \
   }


// if a neighbor is also blank, then start a search from there
#define CHECKCELL2(x, y)                                                \
   if (isValidCell(x, y, mask)) {                                       \
      if (tm[x][y] == 0) {                                              \
         tm[x][y] = 1;                                                  \
         if (mask[x][y] == 0) {                                         \
            if (isBounded(target, x, y, tm, mask) == 0) {               \
               return 0;                                                \
            }                                                           \
         } else if (mask[x][y] != target) {                             \
            return 0;                                                   \
         }                                                              \
      }                                                                 \
   }


int isBounded(int target, int line, int col, Array<Array<char> >& tm, 
      Array<Array<int> >& mask) {

   CHECKCELL(line  , col+1)       // check to the right
   CHECKCELL(line+1, col  )       // check below left
   CHECKCELL(line+1, col+1)       // check below right
   CHECKCELL(line  , col-1)       // check to the left
   CHECKCELL(line-1, col-1)       // check above left
   CHECKCELL(line-1, col  )       // check above right

   CHECKCELL2(line  , col+1)      // check to the right
   CHECKCELL2(line+1, col  )      // check below left
   CHECKCELL2(line+1, col+1)      // check below right
   CHECKCELL2(line  , col-1)      // check to the left
   CHECKCELL2(line-1, col-1)      // check above left
   CHECKCELL2(line-1, col  )      // check above right

   // all ajacent cells in region are bounded
   return 1;
}



//////////////////////////////
//
// isValidCell -- cell is in data array
//

int isValidCell(int line, int col, Array<Array<int> >& mask) {
   if ((line < 0) || (line > mask.getSize()-1)) {
      return 0;
   }
   if ((col < 0) || (col > mask[line].getSize()-1)) {
      return 0;
   }

   return 1;
}



//////////////////////////////
//
// fillBoundedArea --
//

void fillBoundedArea(double target, int regionid, int line, int col, 
      Array<Array<int> >& mask, Array<Array<Array<double> > >& histograms) {
   if ((line < 0) || (line > mask.getSize()-1)) {
      return;  // out of bounds.
   }
   if ((col < 0) || (col > mask[line].getSize()-1)) {
      return;  // out of bounds.
   }
   if (mask[line][col] != 0) {
      return;
   }

   mask[line][col] = regionid;
   histograms[line][col][12] = target;

   fillBoundedArea(target, regionid, line,   col-1, mask, histograms);
   fillBoundedArea(target, regionid, line,   col+1, mask, histograms);
   fillBoundedArea(target, regionid, line-1, col,   mask, histograms);
   fillBoundedArea(target, regionid, line-1, col-1, mask, histograms);
   fillBoundedArea(target, regionid, line+1, col,   mask, histograms);
   fillBoundedArea(target, regionid, line+1, col+1, mask, histograms);
}



//////////////////////////////
//
// recurseMarkMask --
//

void recurseMarkMask(Array<Array<int> >& mask, 
      Array<Array<Array<HISTTYPE> > >& hist, int starti, int startj,
      int mcounter) {
   // check above
   if (starti > 0) {
      // check above-right
      if (startj < hist[starti-1].getSize()) {
         if (hist[starti][startj][12] == hist[starti-1][startj][12]) {
            if (mask[starti-1][startj] == 0) {
               mask[starti-1][startj] = mcounter;
               recurseMarkMask(mask, hist, starti-1, startj, mcounter);
            }
         }
      }

      // check above-left
      if (startj-1 >= 0) {
         if (hist[starti][startj][12] == hist[starti-1][startj-1][12]) {
            if (mask[starti-1][startj-1] == 0) {
               mask[starti-1][startj-1] = mcounter;
               recurseMarkMask(mask, hist, starti-1, startj-1, mcounter);
            }
         }
      }
   }

   // check left
   if (startj > 0) {
      if (hist[starti][startj][12] == hist[starti][startj-1][12]) {
         if (mask[starti][startj-1] == 0) {
            mask[starti][startj-1] = mcounter;
            recurseMarkMask(mask, hist, starti, startj-1, mcounter);
         }
      }
   }
   // check right
   if (startj < hist[starti].getSize()-1) {
      if (hist[starti][startj][12] == hist[starti][startj+1][12]) {
         if (mask[starti][startj+1] == 0) {
            mask[starti][startj+1] = mcounter;
            recurseMarkMask(mask, hist, starti, startj+1, mcounter);
         }
      }
   }

   // check below
   if (starti < mask.getSize()-1) {  // don't check bottom row
      // check below-left
      if (hist[starti][startj][12] == hist[starti+1][startj][12]) {
         if (mask[starti+1][startj] == 0) {
            mask[starti+1][startj] = mcounter;
            recurseMarkMask(mask, hist, starti+1, startj, mcounter);
         }
      }

      // check below-right
      if (startj < hist[starti].getSize()) {
         if (hist[starti][startj][12] == hist[starti+1][startj+1][12]) {
            if (mask[starti+1][startj+1] == 0) {
               mask[starti+1][startj+1] = mcounter;
               recurseMarkMask(mask, hist, starti+1, startj+1, mcounter);
            }
         }
      }
   }
}



//////////////////////////////
//
// printKeyAnalysisCorr -- print the correlation weights for a particular
//   level of analysis.
//

void printKeyAnalysisCorr(Array<Array<Array<HISTTYPE> > >& histograms, 
      int level) {
   int i;

   cout << "% TARGETLEVEL = " << level << "\n";
   cout << "% Meaning of columns below:\n";
   cout << "% COLUMN 1:     Absolute beat index\n";
   cout << "% COLUMN 2:     Analysis window size in beats (centered)\n";
   cout << "% COLUMN 3:     Best key (argmax of data on rest of row)\n";
   cout << "%               A value of 0 means max is in column 4\n";
   cout << "%                  which represents the value for C major, etc.\n";
   cout << "%               A value of 24 means not enough data to do an analysis.\n";
   cout << "% COLUMN 4-15:  Major key Pearson correlation values\n";
   cout << "%           4:  C       major Pearson correlation value (best=0)\n";
   cout << "%           5:  C-sharp major Pearson correlation value (best=1)\n";
   cout << "%           6:  D       major Pearson correlation value (best=2)\n";
   cout << "%           7:  E-flat  major Pearson correlation value (best=3)\n";
   cout << "%           8:  E       major Pearson correlation value (best=4)\n";
   cout << "%           9:  F       major Pearson correlation value (best=5)\n";
   cout << "%          10:  F-sharp major Pearson correlation value (best=6)\n";
   cout << "%          11:  G       major Pearson correlation value (best=7)\n";
   cout << "%          12:  A-flat  major Pearson correlation value (best=8)\n";
   cout << "%          13:  A       major Pearson correlation value (best=9)\n";
   cout << "%          14:  B-flat  major Pearson correlation value (best=10)\n";
   cout << "%          15:  B       major Pearson correlation value (best=11)\n";
   cout << "% COLUMN 16-27: Minor key Pearson correlation values\n";
   cout << "%          16:  C       minor Pearson correlation value (best=12)\n";
   cout << "%          17:  C-sharp minor Pearson correlation value (best=13)\n";
   cout << "%          18:  D       minor Pearson correlation value (best=14)\n";
   cout << "%          19:  E-flat  minor Pearson correlation value (best=15)\n";
   cout << "%          20:  E       minor Pearson correlation value (best=16)\n";
   cout << "%          21:  F       minor Pearson correlation value (best=17)\n";
   cout << "%          22:  F-sharp minor Pearson correlation value (best=18)\n";
   cout << "%          23:  G       minor Pearson correlation value (best=19)\n";
   cout << "%          24:  A-flat  minor Pearson correlation value (best=20)\n";
   cout << "%          25:  A       minor Pearson correlation value (best=21)\n";
   cout << "%          26:  B-flat  minor Pearson correlation value (best=22)\n";
   cout << "%          27:  B       minor Pearson correlation value (best=23)\n";
   int size = histograms.getSize();
   int last = size - 1;
   int counter = 0;

   // ramp up from a lower level
   for (i=0; i<level; i+=2) {
      cout << counter++ << "\t";
      cout << i+1;
      printKeyCorrelations(histograms[last-i][0]);
      cout << "\n";
   }
   for (i=1; i<histograms[last-level+1].getSize()-1; i++) {
      cout << counter++     << "\t";
      cout << level;
      printKeyCorrelations(histograms[last-level+1][i]);
      cout << "\n";
   }
   int ii;
   for (i=0; i<level; i+=2) {
      ii = last - level + i + 1;
      cout << counter++ << "\t";
      cout << level-i;
      printKeyCorrelations(histograms[ii][histograms[ii].getSize()-1]);
      cout << "\n";
   }
}



//////////////////////////////
//
// printKeyCorrelations --
//

void printKeyCorrelations(Array<HISTTYPE>& histogram) {
   int i;

   double h[24];
   for (i=0; i<12; i++) {
      h[i] = histogram[i];
      h[i+12] = h[i];
   }

   double keysum[24];

   double testsum = 0.0;
   for (i=0; i<12; i++) {
      testsum += h[i];
      keysum[i]    = pearsonCorrelation(12, majorweights.getBase(), h+i);
      keysum[i+12] = pearsonCorrelation(12, minorweights.getBase(), h+i);
   }

   int bestkey;
   if (testsum == 0.0) {
      bestkey = 24;
   } else {
      // find max value
      int besti = 0;
      HISTTYPE bestsum = keysum[0];
      for (i=1; i<24; i++) {
         if (keysum[i] > bestsum) {
            besti = i;
            bestsum = keysum[i];
         }
      }
      bestkey = besti;
   }
   
   cout << "\t" << bestkey;
   if (bestkey == 24) {
      for (i=0; i<24; i++) {
         cout << "\t" << 0;
      }
   } else {
      for (i=0; i<24; i++) {
         cout << "\t" << keysum[i];
      }
   }
}
  


//////////////////////////////
//
// printWeights --
//

void printWeights(Array<HISTTYPE>& maj, Array<HISTTYPE>& min) {
   cout << "**kern	**weight\n";
   cout << "!!major weights (using C as the tonic)\n";
   cout << "C	" << maj[0]  << "\n";
   cout << "C#	" << maj[1]  << "\n";
   cout << "D	" << maj[2]  << "\n";
   cout << "E-	" << maj[3]  << "\n";
   cout << "E	" << maj[4]  << "\n";
   cout << "F	" << maj[5]  << "\n";
   cout << "F#	" << maj[6]  << "\n";
   cout << "G	" << maj[7]  << "\n";
   cout << "G#	" << maj[8]  << "\n";
   cout << "A	" << maj[9]  << "\n";
   cout << "B-	" << maj[10] << "\n";
   cout << "B	" << maj[11] << "\n";
   cout << "!!minor weights (using c as the tonic)\n";
   cout << "c	" << min[0]  << "\n";
   cout << "c#	" << min[1]  << "\n";
   cout << "d	" << min[2]  << "\n";
   cout << "e-	" << min[3]  << "\n";
   cout << "e	" << min[4]  << "\n";
   cout << "f	" << min[5]  << "\n";
   cout << "f#	" << min[6]  << "\n";
   cout << "g	" << min[7]  << "\n";
   cout << "g#	" << min[8]  << "\n";
   cout << "a	" << min[9]  << "\n";
   cout << "b-	" << min[10] << "\n";
   cout << "b	" << min[11] << "\n";
   cout << "*-	*-\n";
}



//////////////////////////////
//
// printColorMap --
//

void printColorMap(Array<const char*>& ci) {
   cout << "**index	**pixel	**comment\n";
   cout << "!! Minor Keys\n";
   cout << "0	" << ci[0]	<< "\tC major\n";
   cout << "1	" << ci[1]	<< "\tC-sharp major\n";
   cout << "2	" << ci[2]	<< "\tD major\n";
   cout << "3	" << ci[3]	<< "\tE-flat major\n";
   cout << "4	" << ci[4]	<< "\tE major\n";
   cout << "5	" << ci[5]	<< "\tF major\n";
   cout << "6	" << ci[6]	<< "\tF-sharp major\n";
   cout << "7	" << ci[7]	<< "\tG major\n";
   cout << "8	" << ci[8]	<< "\tA-flat major\n";
   cout << "9	" << ci[9]	<< "\tA major\n";
   cout << "10	" << ci[10]	<< "\tB-flat major\n";
   cout << "11	" << ci[11]	<< "\tB major\n";
   cout << "!! Minor Keys\n";
   cout << "12	" << ci[12]	<< "\tC minor\n";
   cout << "13	" << ci[13]	<< "\tC-sharp minor\n";
   cout << "14	" << ci[14]	<< "\tD minor\n";
   cout << "15	" << ci[15]	<< "\tE-flat minor\n";
   cout << "16	" << ci[16]	<< "\tE minor\n";
   cout << "17	" << ci[17]	<< "\tF minor\n";
   cout << "18	" << ci[18]	<< "\tF-sharp minor\n";
   cout << "19	" << ci[19]	<< "\tG minor\n";
   cout << "20	" << ci[20]	<< "\tG-sharp minor\n";
   cout << "21	" << ci[21]	<< "\tA minor\n";
   cout << "22	" << ci[22]	<< "\tB-flat minor\n";
   cout << "23	" << ci[23]	<< "\tB minor\n";
   cout << "!! Other Colors\n";
   cout << "24	" << ci[24]	<< "\tsilence\n";
   cout << "25	" << ci[25]	<< "\tbackground\n";
   cout << "*-	*-	*-\n";
}



//////////////////////////////
//
// printPPM --
//

void printPPM(Array<Array<Array<HISTTYPE> > >& histograms,
      HumdrumFile& infile) {

   if (keyQ) {
      // print the top-level key and exit
      switch (int(histograms[0][0][12])) {
         case  0: cout << "C Major"  << endl;  break;
         case  1: cout << "D- Major" << endl;  break;
         case  2: cout << "D Major"  << endl;  break;
         case  3: cout << "E- Major" << endl;  break;
         case  4: cout << "E Major"  << endl;  break;
         case  5: cout << "F Major"  << endl;  break;
         case  6: cout << "F# Major" << endl;  break;
         case  7: cout << "G Major"  << endl;  break;
         case  8: cout << "A- Major" << endl;  break;
         case  9: cout << "A Major"  << endl;  break;
         case 10: cout << "B- Major" << endl;  break;
         case 11: cout << "B Major"  << endl;  break;
         case 12: cout << "C Minor"  << endl;  break;
         case 13: cout << "C# Minor" << endl;  break;
         case 14: cout << "D Minor"  << endl;  break;
         case 15: cout << "E- Minor" << endl;  break;
         case 16: cout << "E Minor"  << endl;  break;
         case 17: cout << "F Minor"  << endl;  break;
         case 18: cout << "F# Minor" << endl;  break;
         case 19: cout << "G Minor"  << endl;  break;
         case 20: cout << "A- Minor" << endl;  break;
         case 21: cout << "A Minor"  << endl;  break;
         case 22: cout << "B- Minor" << endl;  break;
         case 23: cout << "B Minor"  << endl;  break;
      }
      return;
   }

   // Pitch to Color translations

   int scapeheight = histograms.getSize();
   int scapewidth  = scapeheight * 2;

   int numberheight = 0;
   int numberwidth  = 0;
   if (numberQ) {
      numberheight = 11;
      numberwidth  = scapewidth;
   }

   int legendheight = 0;
   int legendwidth  = 0;
   if (legendQ) {
      legendheight = scapeheight / 2;
      legendwidth  = scapewidth;
   }

   #define BGCOLOR colorindex[25]

   cout << "P3\n";
   cout << scapewidth  << " " 
        << scapeheight + legendheight + numberheight<< "\n";
   cout << "255\n";

   int blankcells;
   int i;
   int j;
   const char* color;
   for (i=0; i<scapeheight; i++) {
      blankcells = scapeheight - histograms[i].getSize();
      for (j=0; j<blankcells; j++) {
         cout << ' ' << BGCOLOR;
      }
      for (j=0; j<histograms[i].getSize(); j++) {
         color = colorindex[(int)histograms[i][j][12]];
         cout << ' ' << color << ' ' << color;
      }
      for (j=0; j<blankcells; j++) {
         cout << ' ' << BGCOLOR;
      }
      cout << "\n";
   }

   if (numberQ) {
      printNumbers(infile, numberheight, numberwidth);
   }

   if (legendQ) {
      printLegend(legendheight, legendwidth);
   }
}



///////////////////////////////
//
// printNumbers -- print Barline numbers as ticks underneath
//    the plot.
//

void printNumbers(HumdrumFile& infile, int numberheight, int numberwidth) {

   int i, j;
   Array<Array<int> > xaxis;
   xaxis.setSize(numberheight-1);
   xaxis.allowGrowth(0);
   for (i=0; i<xaxis.getSize(); i++) {
      xaxis[i].setSize(numberwidth);
      xaxis[i].allowGrowth(0);
      xaxis[i].setAll(25);
   }

   // bar counter keeps track of multiple repeats of the same
   // music.
   Array<int> barcount(10000);
   barcount.allowGrowth(0);
   barcount.setAll(0);

   double measuresize = getMeasureSize(infile, numberwidth);
   int size;
   int style = 0;
   int position;
   int number;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isMeasure()) {
         continue;
      }
      size = 0;
      position = -1;
      if (sscanf(infile[i][0], "=%d", &number)) {
         if ((number >= 0) && (number < barcount.getSize())) {
            style = barcount[number];
            barcount[number]++;
         } else {
            style = 0;
         }
         position = int(numberwidth * infile[i].getAbsBeat() / 
               infile.getTotalDuration() + 0.5);
         if ((number % 100) == 0) {
            size = 10;
         } else if ((number % 50) == 0) {
            size = 8;
         } else if ((number % 10) == 0) {
            size = 6;
         } else if ((number % 5) == 0) {
            size = 4;
         } else {
            size = 2;
            if (measuresize < 3) {
               // don't display single measure ticks if they
               // are too closely spaced
               size = 0; 
            }
         }
      }

      int color;
      if ((position >= 0) && (position < numberwidth) && (size > 0)) {
         for (j=0; j<size; j++) {
            if (style == 0) {
               color = 24;
            } else if (style == 1) {
               color = 4;  // red (in default color)
            } else if (style == 2) {
               color = 2;  // blue (in default color)
            } else if (style == 3) {
               color = 0;  // green (in default color)
            } else {
               color = 11;    // orange (in default color)
            }
            xaxis[j][position] = color;
	    if (j==9) {
               if (position>0) {
                  xaxis[j][position-1] = color;
               }
               if (position<xaxis[0].getSize()-1) {
                  xaxis[j][position+1] = color;
               }
            }
         }
      } 
   }

   // print a empty line so that small measure markers can be seen
   for (i=0; i<xaxis[0].getSize(); i++) {
         cout << ' ' << colorindex[25];
   }
   cout << '\n';

   for (i=0; i<xaxis.getSize(); i++) {  
      for (j=0; j<xaxis[i].getSize(); j++) {  
         cout << ' ' << colorindex[xaxis[i][j]];
      }
      cout << '\n';
   }


}



//////////////////////////////
//
// getMeasureSize -- return the pixel size of a single measure.
//

double getMeasureSize(HumdrumFile& infile, int width) {
   int i;
   int bar = -100;
   int lastbar = -1000;
   int line = -100;
   int lastline = -1000;

   int number;

   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isMeasure()) {
         continue;
      }
      if (sscanf(infile[i][0], "=%d", &number)) {
         bar = number;
         line = i;
         if (bar == lastbar + 1) {
            return 
            (infile[line].getAbsBeat() - infile[lastline].getAbsBeat()) 
            / infile.getTotalDuration() * width;
         } else {
            lastbar = bar;
            lastline = line;
         }
      }
      
   }

   return 5;
}



///////////////////////////////
//
// printLegend -- print key color mappings as a keyboard 
//

void printLegend(int legendheight, int legendwidth) {
   Array<Array<int> > legend;
   legend.setSize(legendheight);
   legend.allowGrowth(0);
   int i, j;
   for (i=0; i<legend.getSize(); i++) {
      legend[i].setSize(legendwidth);
      legend[i].allowGrowth(0);
      legend[i].setAll(25);  // set to background color
   }


   int startrow = legendheight / 5;
   int endrow   = legendheight - 1;
   int startcol = legendwidth / 5;
   int endcol   = legendwidth - startcol - 1;

   int dooutline = outlineQ;
   if (legendwidth < 100) {   // don't print outline on a small legend
      dooutline = 0;
   }

   Array<int> diatonic(8);
   diatonic.allowGrowth(0);
   diatonic[0] = 0;
   diatonic[1] = 2;
   diatonic[2] = 4;
   diatonic[3] = 5;
   diatonic[4] = 7;
   diatonic[5] = 9;
   diatonic[6] = 11;

   int v, lastv = -1;
   for (i=startrow; i<=endrow; i++) {
      for (j=startcol; j<=endcol; j++) {
         v = diatonic[int((double)(j-startcol)/(endcol-startcol+1)*7)]; 
         if (dooutline && ((v != lastv) || 
               (j == endcol) || (i==startrow) || (i==endrow))) {
            legend[i][j] = 24;
         } else if (i > (endrow + startrow)/2) {  // major keys
            legend[i][j] = v;
         } else {                          // minor keys
            legend[i][j] = v+12;
         }
         lastv = v;
      }
   }

   int blackkeyheight = 2 * (endrow - startrow) / 3;
   int blackend = startrow + blackkeyheight;

   int blackstartcol = startcol;
   int blackendcol   = endcol;
   blackstartcol = blackstartcol + (endcol - startcol) / 96;

   lastv = 0;
   for (i=startrow; i<=blackend; i++) {
      for (j=blackstartcol; j<=blackendcol; j++) {
	 v = int((double)(j-blackstartcol)/(blackendcol-blackstartcol+1)*12);
	 if ((v != lastv) || (i==startrow)) {
            if ((v != 0) && (v != 5)) {
               legend[i][j] = 24;
            }
	    lastv = v;
	    continue;
         }
	 if (!((v==1)||(v==3)||(v==6)||(v==8)||(v==10))) {
            continue;
         }
	 if ((i==blackend)) {
            legend[i][j] = 24;
         } else if (i > (blackend + startrow)/2) {  // major keys
            legend[i][j] = v;
         } else {                                   // minor keys
            legend[i][j] = v+12;
         }
	 lastv = v;
      }
   }

   for (i=0; i<legend.getSize(); i++) {
      for (j=0; j<legend[i].getSize(); j++) {
         if (legend[i][j] == 24) {
            cout << ' ' << colorindex[24];
         } else if (legend[i][j] == 25) {
            cout << ' ' << colorindex[25];
         } else {
            cout << ' ';
            if (legend[i][j] < 12) {
               cout << colorindex[(legend[i][j] + transpose + rrotate + 144) % 12];
            } else {
               cout << colorindex[(legend[i][j] + transpose + rrotate + 144) % 12+12];
            }
         }
      }
      cout << "\n";
   }
}



//////////////////////////////
//
// calculateBestKeys --
//

void calculateBestKeys(Array<Array<Array<HISTTYPE> > >& histograms) {
   int i;
   int j;

   for (i=0; i<histograms.getSize(); i++) {
      for (j=0; j<histograms[i].getSize(); j++) {
          identifyKeyDouble(histograms[i][j]);
      }
   }
}



////////////////////////////////////////
//
// identifyKeyDouble --
//

void identifyKeyDouble(Array<HISTTYPE>& histogram) {
   int i;

   double h[24];
   for (i=0; i<12; i++) {
      h[i] = histogram[i];
      h[i+12] = h[i];
   }

   double keysum[24];

   double testsum = 0.0;

   for (i=0; i<12; i++) {
      testsum += h[i];
      keysum[i]    = 0;
      keysum[i+12] = 0;
   }

   if (testsum == 0.0) {
      histogram[12] = 24;  // empty histogram, so going to display black
      histogram[13] = 24;  // empty histogram, so going to display black
      return;
   }

   if (!averageQ) {
      for (i=0; i<12; i++) {
         keysum[i]    = pearsonCorrelation(12, majorweights.getBase(), h+i);
         keysum[i+12] = pearsonCorrelation(12, minorweights.getBase(), h+i);
      }
   } else {
      for (i=0; i<12; i++) {
         keysum[i]    = pearsonCorrelation(12, majorweights.getBase(), h+i);
         keysum[i+12] = pearsonCorrelation(12, minorweights.getBase(), h+i);

/*
         keysum[i]    = 3 * pearsonCorrelation(12, aamajor.getBase(), h+i);
         keysum[i+12] = 3 * pearsonCorrelation(12, aaminor.getBase(), h+i);

         //keysum[i]    += 2 * pearsonCorrelation(12, bbmajor.getBase(), h+i);
         //keysum[i+12] += 2 * pearsonCorrelation(12, bbminor.getBase(), h+i);

         keysum[i]    += pearsonCorrelation(12, kpmajor.getBase(), h+i);
         keysum[i+12] += pearsonCorrelation(12, kpminor.getBase(), h+i);

         keysum[i]    += 8 * pearsonCorrelation(12, ssmajor.getBase(), h+i);
         keysum[i+12] += 8 * pearsonCorrelation(12, ssminor.getBase(), h+i);

         //keysum[i]    += pearsonCorrelation(12, kkmajor.getBase(), h+i);
         //keysum[i+12] += pearsonCorrelation(12, kkminor.getBase(), h+i);

         keysum[i]    /= 12.0;
         keysum[i+12] /= 12.0;
*/
      }
   }
         

   // find max value
   int besti = 0;
   for (i=1; i<24; i++) {
      if (keysum[i] > keysum[besti]) {
         besti = i;
      }
   }

   // find second best key (maybe add an if statement : only used with -f opt)
   int secondbesti = 0;
   if (besti == 0) {
      secondbesti = 1;
   }
   for (i=1; i<24; i++) {
      if (i == besti) {
         continue;
      }
      if (keysum[i] > keysum[secondbesti]) {
         secondbesti = i;
      }
   }

   histogram[12] = besti;
   histogram[13] = secondbesti;

   // if second-best key being displayed, switch order of values:
   if (secondQ) {
      int temp;
      temp = histogram[12];
      histogram[12] = histogram[13];
      histogram[13] = temp;
   }

}



////////////////////////////////////////
//
// identifyKey --
//

void identifyKey(Array<HISTTYPE>& histogram) {
   int i;

   int h[24];
   int scaled[24];
   int mean = 0;
   for (i=0; i<12; i++) {
      h[i] = (int)histogram[i];
      h[i+12] = h[i];
      scaled[i] = int(h[i] * 0.75 + 0.5);
      scaled[i+12] = scaled[i];
      mean += h[i];
   }
   mean = (int)(mean / 12.0 + 0.5);
   int keysum[24] = {0};

   //double majorw[12] = {2,0,1,0,1,1,0,2,0,1,0,1};
   //double minorw[12] = {2,0,1,1,0,1,0,2,1,0,1,0};

   for (i=0; i<12; i++) {
      // keysums[i]    = pearsonCorrelation(12, majorw, h+i);
      // keysums[i+12] = pearsonCorrelation(12, minorw, h+i);

      keysum[i] += ((h[i]-mean) << 1) - scaled[i];
      keysum[i] += -scaled[i+1];
      keysum[i] += h[i+2] - mean - scaled[i+2];
      keysum[i] += -scaled[i+3];
      keysum[i] += h[i+4] - mean - scaled[i+4];
      keysum[i] += h[i+5] - mean - scaled[i+5];
      keysum[i] += -scaled[i+6];
      keysum[i] += ((h[i]-mean) << 1) - scaled[i+7];
      keysum[i] += -scaled[i+8];
      keysum[i] += h[i+9] - mean - scaled[i+9];
      keysum[i] += -scaled[i+10];
      keysum[i] += h[i+11] - mean - scaled[i+11];

      keysum[i+12] += ((h[i]-mean) << 1) - scaled[i];
      keysum[i+12] += -scaled[i+1];
      keysum[i+12] += h[i+2] - mean - scaled[i+2];
      keysum[i+12] += h[i+3] - mean - scaled[i+3];
      keysum[i+12] += -scaled[i+4];
      keysum[i+12] += h[i+5] - mean - scaled[i+5];
      keysum[i+12] += -scaled[i+6];
      keysum[i+12] += ((h[i]-mean) << 1) - scaled[i+7];
      keysum[i+12] += h[i+8] - mean - scaled[i+8];
      keysum[i+12] += -scaled[i+9];
      keysum[i+12] += h[i+10] - mean - scaled[i+10];
      keysum[i+12] += -scaled[i+11];
   }

   // find max value
   int besti = 0;
   HISTTYPE bestsum = keysum[0];
   for (i=1; i<24; i++) {
// cout << i << ":\t" << keysum[i] << endl;
      if (keysum[i] > bestsum) {
         besti = i;
         bestsum = keysum[i];
      }
   }

   histogram[12] = besti;
}



//////////////////////////////
//
// fillAllHistograms --
//

void fillAllHistograms(Array<Array<Array<HISTTYPE> > >& histograms) {
   int size = histograms.getSize();
   int i, j, k;
   for (i=size-2; i>=0; i--) {
      for (j=0; j<histograms[i].getSize(); j++) {
         for (k=0; k<12; k++) {
            histograms[i][j][k] = histograms[i+1][j][k] +
                                  histograms[size-1][size-1-i+j][k];
         }
      }
   }
}



//////////////////////////////
//
// displayAnalysisHistogram --
//

void displayAnalysisHistogram(Array<Array<Array<HISTTYPE> > >& histograms) {
   int i, j;
   int key = 0;
   int size = histograms.getSize();
   int total = size * (size + 1) / 2;   // triangle number
   Array<int> counts;
   counts.setSize(25);
   counts.allowGrowth(0);
   counts.zero();
   for (i=0; i<histograms.getSize(); i++) {
      for (j=0; j<histograms[i].getSize(); j++) {
         key = (int)histograms[i][j][12];
         counts[key]++;
      }
   }

   Array<int*> rank;
   rank.setSize(25);
   rank.allowGrowth(0);
   Array<int> cc;
   cc.setSize(25);
   for (i=0; i<rank.getSize(); i++) {
      cc[i] = counts[i];
      rank[i] = &(cc[i]);
     
   }
   qsort(rank.getBase(), rank.getSize(), sizeof(int*), ranksort);
   for (i=0; i<cc.getSize(); i++) {
      *(rank[i]) = i+1;
   }

   cout << "!! Count Total = " << total << "\n";
   cout << "**key\t**rank\t**count\t**fraction\n";
   for (i=0; i<counts.getSize(); i++) {
      key = i;
      if (key < 12) {
         key = key+1;
      } else if (key < 24) {
         key = -(key - 11);
      } else {
         key = 0;
      }
      cout << key   << "\t";
      cout << cc[i] << "\t";
      cout << counts[i] << "\t";
      cout << (double)counts[i] / total << "\n";
   }

   int majorsum = 0;
   int minorsum = 0;
   for (i=0; i<12; i++) {
      majorsum += counts[i];
   }
   for (i=12; i<24; i++) {
      minorsum += counts[i];
   }
   cout << "*-\t*-\t*-\t*-\n";
   cout << "!! Major fraction: " << (double)majorsum / total << "\n";
   cout << "!! Minor fraction: " << (double)minorsum / total << "\n";
   if (majorsum > minorsum) {
      cout << "!! The music is happy\n";
   } else {
      cout << "!! The music is sad\n";
   }

}



//////////////////////////////
//
// ranksort -- sort counts by largest first
//

int ranksort(const void* A, const void* B) {
   int& a = *(*((int**)A));
   int& b = *(*((int**)B));
   if (a < b) {
      return +1;
   } else if (a > b) {
      return -1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// displayRawAnalysis --
//

void displayRawAnalysis(Array<Array<Array<HISTTYPE> > >& histogram) {
   int i, j;
   int key;
   for (i=0; i<histogram.getSize(); i++) {
      for (j=0; j<histogram[i].getSize(); j++) {
         if (j > 0) {
            cout << '\t';
         } 
         key = (int)histogram[i][j][12];
         if (key < 12) {
            cout << key+1;             // major key C=1 C#=2 D=3, etc.
         } else if (key < 24) {
            cout << -(key - 11);       // minor key C=-1 C#=-2 D=-3, etc.
         } else {
            cout << 0;                 // silence
         }
      }
      cout << '\n';
   }
}



//////////////////////////////
//
// printBaseHistogramHumdrumStyle --
//

void printBaseHistogramHumdrumStyle(Array<Array<HISTTYPE> >& histogram,
      double totalduration) {
   int i;
   int j;
   cout << "!! Total Duration of Music:\t" << totalduration << endl;
   cout << "!! Duration Units per Frame:\t" 
	<< totalduration / histogram.getSize() << endl;
   cout << "**frame\t**bin00\t**bin01\t**bin02\t**bin03"
        << "\t**bin04\t**bin05\t**bin06\t**bin07\t**bin08\t**bin09"
        << "\t**bin10\t**bin11\n";
   cout << "!\t!C\t!C#\t!D\t!D#\t!E\t!F\t!F#\t!G\t!G#\t!A\t!A#\t!B\n";
   for (i=0; i<histogram.getSize(); i++) {
      cout << i << ':';
      for (j=0; j<histogram[i].getSize()-1; j++) {
         // subtracting one from limit becuase last item
         // is storage for key analysis
         cout << '\t' << histogram[i][j];
      }
      cout << '\n';
   }
   cout << "*-\t*-\t*-\t*-\t*-\t*-\t*-\t*-\t*-\t*-\t*-\t*-\t*-\n";
}

//////////////////////////////
//
// printBaseHistogram --
//

void printBaseHistogram(Array<Array<HISTTYPE> >& histogram) {
   int i;
   int j;
   for (i=0; i<histogram.getSize(); i++) {
      cout << i << ':';
      for (j=0; j<histogram[i].getSize(); j++) {
         cout << '\t' << histogram[i][j];
      }
      cout << '\n';
   }
}



//////////////////////////////
//
// printNormalizedHistogram --
//

void printNormalizedHistogram(Array<Array<HISTTYPE> >& histogram) {
   int i;
   int j;
   double sum = 0;
   for (i=0; i<histogram.getSize(); i++) {
      cout << i << ':';
      sum = 0;
      for (j=0; j<histogram[i].getSize(); j++) {
         sum += histogram[i][j];
      }
      for (j=0; j<histogram[i].getSize(); j++) {
         cout << '\t' << (double)histogram[i][j] / sum;
      }
      cout << '\n';
   }
}



//////////////////////////////
//
// printBest --
//

void printBest(Array<Array<Array<HISTTYPE> > >& histogram) {
   int i;
   int j;
   for (i=0; i<histogram.getSize(); i++) {
      cout << i << ':';
      for (j=0; j<histogram[i].getSize(); j++) {
         cout << '\t' << histogram[i][j][12];
      }
      cout << '\n';
   }
}



//////////////////////////////
//
// loadHistogramFromHumdrumFile --
//

double loadHistogramFromHumdrumFile(Array<Array<HISTTYPE> >& histograms,
   HumdrumFile& infile, const char* filename, int segments) {

   if (strcmp(filename, "") == 0) {
      infile.read(cin);
   } else {
      infile.read(filename);
   }
   infile.analyzeRhythm("4");
   double totalduration = infile.getTotalDuration();

   double duration;
   int i;
   int j;
   int k;
   char buffer[10000] = {0};
   int pitch;
   double start;
   int tokencount;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() !=  E_humrec_data) {
         continue;
      }
      start = infile[i].getAbsBeat();
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp(infile[i].getExInterp(j), "**kern") != 0) {
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            continue; // ignore null tokens
         }
         tokencount = infile[i].getTokenCount(j);
         for (k=0; k<tokencount; k++) {
            infile[i].getToken(buffer, j, k);
            if (strcmp(buffer, ".") == 0) {
               continue;  // ignore illegal inline null tokens
            }
            pitch = Convert::kernToMidiNoteNumber(buffer);
            if (pitch < 0) {
               continue;  // ignore rests or strange objects
            }
            pitch = pitch % 12;  // convert to chromatic pitch-class
            duration = Convert::kernToDuration(buffer);
            if (duration <= 0.0) {
               continue;   // ignore grace notes and strange objects
            }
            
            addToHistogramDouble(histograms, pitch,
                  start, duration, totalduration, segments);
         }
      }
   }

   return totalduration;
}



//////////////////////////////
//
// addToHistogramDouble -- fill the histogram in the right spots.
//

void addToHistogramDouble(Array<Array<double> >& histogram, int pc, 
      double start, double dur, double tdur, int segments) {

   pc = (pc + transpose + 144) % 12;

   double startseg = start / tdur * segments;
   double startfrac = startseg - (int)startseg;

   double segdur = dur / tdur * segments;

   if (segdur <= 1.0 - startfrac) {
      histogram[(int)startseg][pc] += segdur;
      return;
   } else if (1.0 - startfrac > 0.0) {
      histogram[(int)startseg][pc] += (1.0 - startfrac);
      segdur -= (1.0 - startfrac);
   }

   int i = (int)(startseg + 1);
   while (segdur > 0.0 && i < histogram.getSize()) {
      if (segdur < 1.0) {
         histogram[i][pc] += segdur;
         segdur = 0.0;
      } else {
         histogram[i][pc] += 1.0;
         segdur -= 1.0;
      }
      i++;
   }
}



//////////////////////////////
//
// addToHistogramInteger -- fill the histogram in the right spots.
//

#define WID 100000

void addToHistogramInteger(Array<Array<int> >& histogram, int pc, 
      double start, double dur, double tdur, int segments) {

   double startseg = start / tdur * segments;
   double startfrac = startseg - (int)startseg;

   double segdur = dur / tdur * segments;

   if (segdur <= 1.0 - startfrac) {
      histogram[(int)startseg][pc] += int(segdur * WID + 0.5);
      return;
   } else if (1.0 - startfrac > 0.0) {
      histogram[(int)startseg][pc] += int((1.0 - startfrac) * WID + 0.5);
      segdur -= (1.0 - startfrac);
   }

   int i = (int)(startseg + 1);
   while (segdur > 0.0 && i < histogram.getSize()) {
      if (segdur < 1.0) {
         histogram[i][pc] += int(segdur * WID + 0.5);
         segdur = 0.0;
      } else {
         histogram[i][pc] += WID;
         segdur -= 1.0;
      }
      i++;
   }
}



//////////////////////////////
//
// loadHistogramFromMidiFile --
//

double loadHistogramFromMidiFile(Array<Array<HISTTYPE> >& histogram,
   const char* filename, int segments) {

   MidiFile midifile(filename);
   midifile.absoluteTime();
   midifile.joinTracks();

   Array<int> ontimes(128*16);
   Array<int> onvelocities(128*16);
   ontimes.setAll(-1);
   onvelocities.zero();

   int i;
   int command = 0;
   int totalduration = midifile.getEvent(0,midifile.getNumEvents(0)-1).time;
   // using the last item as the total duration is not so great because
   // some MIDI files have some junk messages long after the music 
   // has stopped (2_ase.mid is an example), so search backwards
   // through the midifile for the last note-on or note-off message.
   for (i=midifile.getNumEvents(0)-1; i>=0; i--) {
      command = midifile.getEvent(0,i).data[0] & 0xf0;
      if (command == 0x90 || command == 0x80) {
         totalduration = midifile.getEvent(0, i).time;
         break;
      }
   }
   
   int key;
   int channel;
   int duration;
   int ontime;
   for (i=0; i<midifile.getNumEvents(0); i++) {
      command = midifile.getEvent(0,i).data[0] & 0xf0;
      channel = midifile.getEvent(0,i).data[0] & 0x0f;
      if (channelfilter[channel] == 0) {
         // ignore events on this channel
         continue;
      }
      if (command == 0x90 && midifile.getEvent(0,i).data[2] != 0) {
         // store note-on velocity and time.
         key = midifile.getEvent(0,i).data[1];
         ontime = midifile.getEvent(0,i).time;
         if (ontimes[key * channel] > -1) {
            // the previous note was not turned off, to turn
            // it off now and store that note in the histogram
            duration = ontime - ontimes[key * channel];
            addToHistogramDouble(histogram, key % 12, ontimes[key*channel], 
                  duration, totalduration, segments);
            ontimes[key * channel] = ontime;
         } else {
            // no note exists in the slot, to store for later
            ontimes[key * channel] = ontime;
         }
      } else if (command == 0x90 || command == 0x80) {
         // process a note-off command
         key = midifile.getEvent(0,i).data[1];
         ontime = midifile.getEvent(0,i).time;
         if (ontimes[key * channel] > -1) {
            // process the note which has been waiting
            duration = ontime - ontimes[key * channel];
            addToHistogramDouble(histogram, key % 12, ontimes[key*channel], 
                  duration, totalduration, segments);
         }
         ontimes[key * channel] = -1;
      }
   }

   return totalduration;
}



//////////////////////////////
//
// pearsonCorrelation --
//

double pearsonCorrelation(int size, double* x, double* y) {

   double sumx  = 0.0;
   double sumy  = 0.0;
   double sumco = 0.0;
   double meanx = x[0];
   double meany = y[0];
   double sweep;
   double deltax;
   double deltay;

   int i;
   for (i=2; i<=size; i++) {
      sweep = (i-1.0) / i;
      deltax = x[i-1] - meanx;
      deltay = y[i-1] - meany;
      sumx  += deltax * deltax * sweep;
      sumy  += deltay * deltay * sweep;
      sumco += deltax * deltay * sweep;
      meanx += deltax / i;
      meany += deltay / i;
   }

   double popsdx = sqrt(sumx / size);
   double popsdy = sqrt(sumy / size);
   double covxy  = sumco / size;

   return covxy / (popsdx * popsdy);
}



//////////////////////////////
//
// checkOptions --
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("b|blank=b",        "blank out non-plausible key regions");
   opts.define("trim=b",           "blank out non-plausible key regions");
   opts.define("f|fill=b", "fill in blanked regions with second best key");
   opts.define("average=b",        "average multiple weightings results");
   opts.define("t|transpose=i:0",  "Transposition by half-steps");
   opts.define("rotate=s:",        "Rotate color map (external transpose)");
   opts.define("s|segments=i:300", "The height in pixel of the output plot");
   opts.define("sec|second=b",     "Display the second-best key region");
   opts.define("r|raw=b",          "Display the analyzed keys");
   opts.define("c|colorfile=s",    "key to color mapping specification");
   opts.define("l|legend=b",       "print color legend beneath plot");
   opts.define("n|number=b",       "print barnumber markers");
   opts.define("no-outline=b",     "do not outline keys in legend");
   opts.define("m|mapping=s:newton", "predefined color mapping");
   opts.define("C|printcolors=b",  "print key to color mapping");
   opts.define("D|no-drum=b",      "Don't analyze the General MIDI drum track");
   opts.define("x|exclude=s",      "exclude MIDI channel notes");
   opts.define("hist|histograms|histogram=b", "display baseline histograms");
   //opts.define("beat=b", "segment by beat");
   opts.define("cor=i:4", "Print correlation values for key analysis");
   opts.define("w|weights=s", "arbitrary set of pitch weights");
   opts.define("W|printweights=b", "display weights which will be used");
   opts.define("k|key=b", "display top-level key anaysis");

   opts.define("aa|aarden=b",        "load Aarden-Essen weights");
   opts.define("bb|bellman|budge=b", "load Bellman-Budge weights");
   opts.define("kk|krumhansl=b",     "load Krumhansl-Kessler weights");
   opts.define("kp|kostkapayne=b",   "load Kostka-Payne weights");
   opts.define("ss|simple|sapp=b",   "load Simple weights");

   opts.define("khist=b", "display the analysis key histogram");
   opts.define("author=b",  "author of program");
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");
   opts.define("help=b",  "short description");
   opts.process(argc, argv);

   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Jan 2008" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 30 Jan 2008" << endl;
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

   legendQ   =  opts.getBoolean("legend");
   numberQ   =  opts.getBoolean("number");
   blankQ    =  opts.getBoolean("blank");
   fillQ     =  opts.getBoolean("fill");
   averageQ  = !opts.getBoolean("average");
   secondQ   =  opts.getBoolean("second");
   keyQ      =  opts.getBoolean("key");
	    
   trimQ     =  opts.getBoolean("trim");
   segments  =  opts.getInteger("segments");
   histQ     =  opts.getBoolean("histogram");
   khistQ    =  opts.getBoolean("khist");
   rawQ      =  opts.getBoolean("raw");
   outlineQ  = !opts.getBoolean("no-outline");
   transpose =  opts.getInteger("transpose");
   if (transpose < -12 || transpose > 12) {
      cerr << "Error: transposition is out of range: " << transpose << endl;
      exit(1);
   } 

   rrotate = 0;
   if (opts.getInteger("rotate")) {
      const char* strang = opts.getString("rotate");
      if (hasdigit(strang)) {
         rrotate = atol(strang);
      } else {
         rrotate = Convert::kernToMidiNoteNumber(strang) % 12;
      }
      rrotate = (rrotate + 1200) % 12;
      if (rrotate < 0) {
         cerr << "Error: funny value for rotation: " << strang << endl;
      }
   }

   if (trimQ) {  // turn on filling if trim is specified
      fillQ = 1;
   }
   if (fillQ) {  // turn on blanking if fill is specified
      blankQ = 1;
   }

   setFilterOptions(channelfilter, opts.getString("exclude"));
   if (opts.getBoolean("no-drum")) {
      // turn off MIDI channel 10
      channelfilter[9] = 0;
   }

   if (opts.getBoolean("colorfile")) {
      processColorFile(opts.getString("colorfile"), colorfile);
   }
   if (opts.getBoolean("printcolors")) {
      printColorMap(colorindex);
      exit(0);
   }
	    
   
   if (opts.getBoolean("aarden")) {
      fillWeightsWithAardenEssen(majorweights, minorweights);
   } else if (opts.getBoolean("bellman")) {
      fillWeightsWithBellmanBudge(majorweights, minorweights);
   } else if (opts.getBoolean("krumhansl")) {
      fillWeightsWithKrumhanslKessler(majorweights, minorweights);
   } else if (opts.getBoolean("kostkapayne")) {
      fillWeightsWithKostkaPayne(majorweights, minorweights);
   } else if (opts.getBoolean("simple")) {
      fillWeightsWithSimpleValues(majorweights, minorweights);
   }

   if (opts.getBoolean("weights")) {
      processWeights(opts.getString("weights"), majorweights, minorweights);
   }

   if (opts.getBoolean("printweights")) {
      printWeights(majorweights, minorweights);  
      exit(0);
   }

   corQ     = opts.getBoolean("cor");
   corlevel = opts.getInteger("cor");

   if (opts.getBoolean("mapping")) {
      changeColorMapping(colorindex, opts.getString("mapping"));
   }
}



//////////////////////////////
//
// hasdigit --
//

int hasdigit(const char* strang) {
   int i=0;
   while (strang[i] != '\0') {
      if (isdigit(strang[i++])) {
         return 1;
      }
   }
   return 0;
}



//////////////////////////////
//
// changeColorMapping --
//

void changeColorMapping(Array<const char*>& ci, const char* type) {
   if (strcmp(type, "castel") == 0) {
      fillColorMapping_castel(ci);
   } else if (strcmp(type, "newton") == 0) {
      fillColorMapping_newton(ci);
   }
}



//////////////////////////////
//
// fillColorMapping_castel --
//
// http://homepage.eircom.net/~musima/visualmusic/visualmusic.htm
//
//Louis Bertrand Castel - CLAVECIN OCULAIRE
//
// The French Jesuit monk Louis Bertrand Castel, the well-known
// mathematician and physicist, was a firm advocate of there being direct
// solid relationships between the seven colors and the seven units of
// the scale, as per Newton's Optics. Around 1742, Castel proposed the
// construction of a clavecin oculaire, a light-organ, as a new musical
// instrument which would simultaneously produce both sound and the "correct"
// associated color for each note.
// 
// B (dark) violet Bb agate A violet Ab crimson G red F# orange
// F golden yellow E yellow Eb olive green D green C# pale green C blue
//
// "The Jesuit, Father Louis Bertrand Castel, built an Ocular Harpsichord
// around 1730, which consisted of a 6-foot square frame above a normal
// harpsichord; the frame contained 60 small windows each with a different
// colored-glass pane and a small curtain attached by pullies to one specific
// key, so that each time that key would be struck, that curtain would lift
// briefly to show a flash of corresponding color. Enlightenment society
// was dazzled and fascinated by this invention, and flocked to his Paris
// studio for demonstrations. The German composer Telemann traveled to France
// to see it, composed some pieces to be played on the Ocular Harpsichord,
// and wrote a German-language book about it. But a second, improved model
// in 1754 used some 500 candles with reflecting mirrors to provide enough
// light for a larger audience, and must have been hot, smelly and awkward,
// with considerable chance of noise and malfunction between the pullies,
// curtains and candles....
// 
// Castel predicted that every home in Paris would one day have an Ocular
// Harpsichord for recreation, and dreamed of a factory making some 800,000
// of them. But the clumsy technology did not really outlive the inventor
// himself, and no physical relic of it survives.
//
// http://rhythmiclight.com/archives/ideas/colorscales.html
// 
// http://rhythmiclight.com/articles/InstrumentsToPerformColor.pdf

void fillColorMapping_castel(Array<const char*>& ci) {

   ci[0]  = "51 102 255";	// C major		blue
   ci[1]  = "153 255 153";	// C-sharp major	pale green
   ci[2]  = "0 204 0";		// D major		green
   ci[3]  = "153 153 0";	// E-flat major		olive green
   ci[4]  = "255 255 0";	// E major		yellow
   ci[5]  = "255 204 0";	// F major		golden yellow
   ci[6]  = "255 102 0";	// F-sharp major	orange
   ci[7]  = "255 0 0";		// G major		red
   ci[8]  = "153 0 51";		// A-flat major		crimson
   ci[9]  = "255 153 255";	// A major		violet
   ci[10] = "204 153 153";	// B-flat major		agate
   ci[11] = "153 51 153";	// B major		dark violet

   ci[12] = "38 77 191";	// C minor
   ci[13] = "115 191 115";	// C-sharp minor
   ci[14] = "0 153 0";		// D minor
   ci[15] = "115 115 0";	// E-flat minor
   ci[16] = "191 191 0";	// E minor
   ci[17] = "191 153 0";	// F minor
   ci[18] = "191 77 0";		// F-sharp minor
   ci[19] = "191 0 0";		// G minor
   ci[20] = "115 0 38";		// G-sharp minor
   ci[21] = "191 115 191";	// A minor
   ci[22] = "153 115 115";	// B-flat minor
   ci[23] = "115 38 115";	// B minor

   //colorindex[24] = "0 0 0";		// silence
   //colorindex[25] = "255 255 255";	// background
}



//////////////////////////////
//
// fillColorMapping_newton --
//
// http://homepage.eircom.net/~musima/visualmusic/visualmusic.htm
// http://rhythmiclight.com/archives/ideas/colorscales.html
//
// Isaac Newton colors
//

void fillColorMapping_newton(Array<const char*>& ci) {

   ci[0]  = "255 0 0";		// C major		red
   ci[1]  = "250 118 86";	// C-sharp major	red-orange
   ci[2]  = "255 102 0";	// D major		orange
   ci[3]  = "245 205 97";	// E-flat major		yellow-orange
   ci[4]  = "255 255 0";	// E major		yellow
   ci[5]  = "0 255 0";		// F major		green
   ci[6]  = "60 251 213";	// F-sharp major	blue-green
   ci[7]  = "63 95 255";	// G major		blue
   ci[8]  = "144 133 248";	// A-flat major		blue-indigo
   ci[9]  = "108 50 255";	// A major		indigo
   ci[10] = "183 115 247";	// B-flat major		indigo-violet
   ci[11] = "162 0 216";	// B major		violet

   ci[12] = "191 0 0";		// C minor
   ci[13] = "183 106 74";	// C-sharp minor
   ci[14] = "191 77 0";		// D minor
   ci[15] = "170 137 9";	// E-flat minor
   ci[16] = "191 191 0";	// E minor
   ci[17] = "0 161 0";		// F minor
   ci[18] = "38 183 154";	// F-sharp minor
   ci[19] = "37 61 181";	// G minor
   ci[20] = "119 114 179";	// G-sharp minor
   ci[21] = "85 28 181";	// A minor
   ci[22] = "125 88 173";	// B-flat minor
   ci[23] = "87 0 136";		// B minor

   //colorindex[24] = "0 0 0";		// silence
   //colorindex[25] = "255 255 255";	// background
}



//////////////////////////////
//
// fillWeightsWith* -- Set key weights to specified default values.
//

void fillWeightsWithAardenEssen(Array<HISTTYPE>& maj, Array<HISTTYPE>& min) {
   // as found in Bret Aarden's dissertation
   maj[0]  = 17.7661  ;  // C major weights
   maj[1]  =  0.145624;  // C#
   maj[2]  = 14.9265  ;  // D
   maj[3]  =  0.160186;  // D#
   maj[4]  = 19.8049  ;  // E
   maj[5]  = 11.3587  ;  // F
   maj[6]  =  0.291248;  // F#
   maj[7]  = 22.062   ;  // G
   maj[8]  =  0.145624;  // G#
   maj[9]  =  8.15494 ;  // A
   maj[10] =  0.232998;  // A#
   maj[11] =  4.95122 ;  // B
   min[0]  = 18.2648  ;  // c minor weights
   min[1]  =  0.737619;  // c#
   min[2]  = 14.0499  ;  // d
   min[3]  = 16.8599  ;  // d#
   min[4]  =  0.702494;  // e
   min[5]  = 14.4362  ;  // f
   min[6]  =  0.702494;  // f#
   min[7]  = 18.6161  ;  // g
   min[8]  =  4.56621 ;  // g#
   min[9]  =  1.93186 ;  // a
   min[10] =  7.37619 ;  // a#
   min[11] =  1.75623 ;  // b
} 


void fillWeightsWithBellmanBudge(Array<HISTTYPE>& maj, Array<HISTTYPE>& min) {
   // as found in Bellman CMMR 2005 paper, derived from Budge 1943 dissertation
   maj[0]  = 16.80;	// C major weights
   maj[1]  =  0.86;	// C#
   maj[2]  = 12.95;	// D
   maj[3]  =  1.41;	// D#
   maj[4]  = 13.49;	// E
   maj[5]  = 11.93;	// F
   maj[6]  =  1.25;	// F#
   maj[7]  = 20.28;	// G
   maj[8]  =  1.80;	// G#
   maj[9]  =  8.04;	// A
   maj[10] =  0.62;	// A#
   maj[11] = 10.57;	// B
   min[0]  = 18.16;	// c minor weights
   min[1]  =  0.69;	// c#
   min[2]  = 12.99;	// d
   min[3]  = 13.34;	// d#
   min[4]  =  1.07;	// e
   min[5]  = 11.15;	// f
   min[6]  =  1.38;	// f#
   min[7]  = 21.07;	// g
   min[8]  =  7.49;	// g#
   min[9]  =  1.53;	// a
   min[10] =  0.92;	// a#
   min[11] = 10.21;	// b
}


void fillWeightsWithKrumhanslKessler(Array<HISTTYPE>& maj, Array<HISTTYPE>& min) {
   // as found in Carol Krumhansl's 1990 book
   maj[0]  = 6.35; 	// C major weights
   maj[1]  = 2.23; 	// C#
   maj[2]  = 3.48; 	// D
   maj[3]  = 2.33; 	// D#
   maj[4]  = 4.38; 	// E
   maj[5]  = 4.09; 	// F
   maj[6]  = 2.52; 	// F#
   maj[7]  = 5.19; 	// G
   maj[8]  = 2.39; 	// G#
   maj[9]  = 3.66; 	// A
   maj[10] = 2.29; 	// A#
   maj[11] = 2.88;	// B
   min[0]  = 6.33; 	// c minor weights
   min[1]  = 2.68; 	// c#
   min[2]  = 3.52; 	// d
   min[3]  = 5.38; 	// d#
   min[4]  = 2.60; 	// e
   min[5]  = 3.53; 	// f
   min[6]  = 2.54; 	// f#
   min[7]  = 4.75; 	// g
   min[8]  = 3.98; 	// g#
   min[9]  = 2.69; 	// a
   min[10] = 3.34; 	// a#
   min[11] = 3.17;	// b
}


void fillWeightsWithKostkaPayne(Array<HISTTYPE>& maj, Array<HISTTYPE>& min) {
   // found in David Temperley: Music and Probability 2006
   maj[0]  = 0.748;	// C major weights
   maj[1]  = 0.060;	// C#
   maj[2]  = 0.488;	// D
   maj[3]  = 0.082;	// D#
   maj[4]  = 0.670;	// E
   maj[5]  = 0.460;	// F
   maj[6]  = 0.096;	// F#
   maj[7]  = 0.715;	// G
   maj[8]  = 0.104;	// G#
   maj[9]  = 0.366;	// A
   maj[10] = 0.057;	// A#
   maj[11] = 0.400;	// B
   min[0]  = 0.712;	// c minor weights
   min[1]  = 0.084;	// c#
   min[2]  = 0.474;	// d
   min[3]  = 0.618;	// d#
   min[4]  = 0.049;	// e
   min[5]  = 0.460;	// f
   min[6]  = 0.105;	// f#
   min[7]  = 0.747;	// g
   min[8]  = 0.404;	// g#
   min[9]  = 0.067;	// a
   min[10] = 0.133;	// a#
   min[11] = 0.330;	// b
}


void fillWeightsWithSimpleValues(Array<HISTTYPE>& maj, Array<HISTTYPE>& min) {
   maj[0]  = 2;		// C major weights
   maj[1]  = 0;		// C#
   maj[2]  = 1;		// D
   maj[3]  = 0;		// D#
   maj[4]  = 1;		// E
   maj[5]  = 1;		// F
   maj[6]  = 0;		// F#
   maj[7]  = 2;		// G
   maj[8]  = 0;		// G#
   maj[9]  = 1;		// A
   maj[10] = 0;		// A#
   maj[11] = 1;		// B
   min[0]  = 2;		// c minor weights
   min[1]  = 0;		// c#
   min[2]  = 1;		// d
   min[3]  = 1;		// d#
   min[4]  = 0;		// e
   min[5]  = 1;		// f
   min[6]  = 0;		// f#
   min[7]  = 2;		// g
   min[8]  = 1;		// g#
   min[9]  = 0;		// a
   min[10] = 1;		// a#
   min[11] = 0;		// b
}



//////////////////////////////
//
// processWeights --
//

void processWeights(const char* filename, Array<HISTTYPE>& major, 
      Array<HISTTYPE>& minor) {

   HumdrumFile wfile;
   wfile.read(filename);
   int i, j;
   int key;
   double value;
   for (i=0; i<wfile.getNumLines(); i++) {
      if (wfile[i].getType() != E_humrec_data) {
         continue;
      }
      key = -1;
      value = -1.0;
      for (j=0; j<wfile[i].getFieldCount(); j++) {
         if (strcmp(wfile[i].getExInterp(j), "**kern") == 0) {
            key = Convert::kernToMidiNoteNumber(wfile[i][j]) % 12;
            if (islower(wfile[i][j][0])) {
               key += 12;
            }
         } else if (strcmp(wfile[i].getExInterp(j), "**weight") == 0) {
            sscanf(wfile[i][j], "%lf", &value);
         }
      }
      if ((key >= 0) && (key < 24) && (value != -1.0)) {
         if (key < 12) {
            major[key] = value;
         } else {
            minor[key-12] = value;
         }
      }
   }
}



//////////////////////////////
//
// processColorFile --
//

void processColorFile(const char* filename, HumdrumFile& cfile) {
   cfile.read(filename);
   int i;
   int j;
   int index;
   int test1;
   int test2;
   int test3;
   const char* string;

   for (i=0; i<cfile.getNumLines(); i++) {
      if (cfile[i].getType() != E_humrec_data) {
         continue;
      }
      index = -1;
      string = NULL;
      for (j=0; j<cfile[i].getFieldCount(); j++) {
         if (strcmp(cfile[i].getExInterp(j), "**index") == 0) {
            sscanf(cfile[i][j], "%d", &index);
            if (index < 0 || index > 25) {
               index = -1;  // ignore out-of range indices
            }
         }
         if (strcmp(cfile[i].getExInterp(j), "**pixel") == 0) {
            if (sscanf(cfile[i][j], "%d %d %d", &test1, &test2, &test3) == 3) {
               string = cfile[i][j]; 
            } else {
               cerr << "Error in color file on line " << i + 1 
                    << ":" << cfile[i] << endl;
               exit(1);
            }
         }
      }
      if (index >= 0) {
         colorindex[index] = string;
         // note that the "string" can not be deallocated until the
         // program is finished.
      }
   }
}



//////////////////////////////
//
// setFilterOptions --
//

void setFilterOptions(Array<int>& channelfilter, const char* exclude) {
   int length = strlen(exclude);
   int character;
   int i;
   int value;

   for (i=0; i<length; i++) {
      character = toupper(exclude[i]);
      if (!isxdigit(character)) {
         continue;
      }
      if (isdigit(character)) {
         value = character - '0';
      } else {
         value = character - 'A' + 10;
      }
      if (value >= 0 && value <= 15) {
         channelfilter[value] = 0;
      }
   }
}



//////////////////////////////
//
// example --
//

void example(void) {


}



//////////////////////////////
//
// usage --
//

void usage(const char* command) {

}



// md5sum: d8c4f68a5d05606c86be8c29a54728f8 mkeyscape.cpp [20111105]
