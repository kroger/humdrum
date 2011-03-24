//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat May 23 21:08:48 PDT 1998
// Last Modified: Fri Jul  3 14:18:04 PDT 1998
// Last Modified: Sat Oct 14 20:26:15 PDT 2000 revised for museinfo 1.0
// Last Modified: Wed Nov 29 12:14:41 PST 2000 use internal analysis
// Last Modified: Tue Apr 21 00:41:11 PDT 2009 fixed spine manip printing
// Last Modified: Thu May 14 20:25:17 PDT 2009 -U option added
// Last Modified: Mon Apr 26 06:21:58 PDT 2010 -n, -s options added
// Last Modified: Thu Mar 10 15:06:00 PST 2011 -i option added
// Last Modified: Wed Mar 16 14:16:01 PDT 2011 added --iv option
// Filename:      ...sig/examples/all/sonority2.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/sonority2.cpp
// Syntax:        C++; museinfo
//
// Description:   Analyzes **kern data for timeslice chord qualities
//

#include "humdrum.h"
#include "Matrix.h"

#include <stdlib.h>
#include <string.h>

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif


// function declarations
void    checkOptions        (Options& opts, int argc, char* argv[]);
void    example             (void);
void    processRecords      (HumdrumFile& infile);
void    usage               (const char* command);
void    fillStringWithNotes (char* string, ChordQuality& quality, 
                             HumdrumFile& infile, int line);
int     identifyBassNote    (SigCollection<int>& notes, 
                             HumdrumFile& infile, int line,
                             Array<int>& sounding);
int     transitionalSonority(ChordQuality& quality, HumdrumFile& infile, 
                             int line);
void    printTriadImage     (HumdrumFile& infile, int rows, int cols);
void    printBarlines       (HumdrumFile& infile, int numberheight, 
                             int numberwidth);
double  getMeasureSize      (HumdrumFile& infile, int width);
void    printLegend         (int legendheight, int legendwidth);
void    printAttackMarker   (HumdrumFile& infile, int line);
void    printAttackMarker   (HumdrumFile& infile, int line);

// global variables
Options      options;            // database for command-line arguments
char         unknown[256] = {0}; // space for unknown chord simplification
int          chordinit;          // for initializing chord detection function
int          explicitQ = 0;      // used with -U option
int          notesQ    = 0;      // used with -n option
int          suppressQ = 0;      // used with -s option
int          parenQ    = 1;      // used with -P option
int          ivQ       = 0;      // used with --iv option
int          forteQ    = 0;      // used with --forte option
int          tnQ       = 0;      // used with --tn option
int          tniQ      = 0;      // used with --tni option
int          attackQ   = 0;      // used with -x option
int          appendQ   = 0;      // used with -a option
int          imageQ    = 0;      // used with -I option
int          imagex    = 800;    // used with -I option
int          imagey    = 20;     // used with -I option
int          octaveVal = -100;   // used with -o option
int          barlinesQ = 0;      // used with -b option
int          legendQ   = 0;      // used with -l option
int          outlineQ  = 1;      // 
const char*  notesep   = " ";    // used with -N option
const char* colorindex[26];


///////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[]) {
   HumdrumFile infile, outfile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   for (int i=0; i<numinputs || i==0; i++) {
      chordinit = 1;
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }

      // analyze the input file according to command-line options
      if (imageQ) {
         printTriadImage(infile, imagey, imagex);
      } else {
         processRecords(infile);
      }
       
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// printTriadImage --
//

void printTriadImage(HumdrumFile& infile, int rows, int cols) {

   Array<ChordQuality> cq;
   infile.analyzeSonorityQuality(cq);
   infile.analyzeRhythm("4");

   Array<Array<int> > triads;

   triads.setSize(3);

   triads[0].setSize(cols);
   triads[0].setAll(24);
   triads[1].setSize(cols);
   triads[1].setAll(24);
   triads[2].setSize(cols);
   triads[2].setAll(24);


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
   colorindex[24] = "255 255 255";	// background
   colorindex[25] = "0 0 0";		// silence


   double start;
   double end;
   int    inversion;
   int    starti;
   int    endi;
   int    minQ;
   int    majQ;
   int    i, m, j, ii;
   int    rootindex;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isData()) {
         continue;
      }
      if (strcmp(cq[i].getTypeName(), "min") == 0) {
         minQ = 1;
      } else {
         minQ = 0;
      }
      if (strcmp(cq[i].getTypeName(), "maj") == 0) {
         majQ = 1;
      } else {
         majQ = 0;
      }

      if (!(majQ || minQ)) {
         continue;
      }
      start = infile[i].getAbsBeat();
      end   = start + infile[i].getDuration();
      starti = int(start / infile.getTotalDuration() * cols + 0.5);
      endi   = int(end   / infile.getTotalDuration() * cols + 0.5);
      if (starti < 0) { starti = 0; }
      if (endi   < 0) { endi   = 0; }
      if (starti >= cols) { starti = cols-1; }
      if (endi   >= cols) { endi   = cols-1; }
      rootindex = Convert::base40ToMidiNoteNumber(cq[i].getRoot());
      if (minQ) {
         rootindex += 12;
      }
      inversion = cq[i].getInversion();
      for (ii=starti; ii<=endi; ii++) {
         triads[inversion][ii] = rootindex;
      }
   }

   int barheight = 0;
   int barwidth  = 0;
   if (barlinesQ) {
      barheight = 11;
      barwidth  = cols;
   }

   int legendheight = 0;
   int legendwidth  = 0;
   if (legendQ) {
      legendheight = 100;
      legendwidth  = cols;
   }


   int value = 24;
   Matrix<int> image(rows*2, cols, value);

   for (i=triads.getSize()-1; i>=0; i--) {
      int start = int(i/2.0 * rows);
      for (m=0; m<rows; m++) {
         ii = (int)(m + start);
         if (ii >= image.getRowCount()) {
            ii = image.getRowCount() - 1;
         }
         for (j=0; j<triads[i].getSize(); j++) {
            if (triads[i][j] < 24) {
               image.cell(ii,j) = triads[i][j];
            }
            // cout << colorindex[triads[i][j]] << " ";
            //cout << triads[i][j] << " ";
         }
         //cout << "\n";
      }
   }

   // print Image:
   cout << "P3\n";
   cout << cols  << " " << rows*2 + barheight + legendheight << "\n";
   cout << "255\n";

   for (i=image.getRowCount()-1; i>=0; i--) {
      for (j=0; j<image.getColumnCount(); j++) {
         cout << colorindex[image.cell(i,j)] << ' ';
      }
      cout << "\n";
   }

   if (barlinesQ) {
      printBarlines(infile, barheight, barwidth);
   }

   if (legendQ) {
      printLegend(legendheight, legendwidth);
   }

}



///////////////////////////////
//
// printBarlines -- print Barline numbers as lines underneath
//    the plot.
//

void printBarlines(HumdrumFile& infile, int numberheight, int numberwidth) {
   int i, j;
   Array<Array<int> > xaxis;
   xaxis.setSize(numberheight-1);
   xaxis.allowGrowth(0);
   for (i=0; i<xaxis.getSize(); i++) {
      xaxis[i].setSize(numberwidth);
      xaxis[i].allowGrowth(0);
      xaxis[i].setAll(24);
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
      if ((position >= 0) && (size > 0)) {
         for (j=0; j<size; j++) {
            if (style == 0) {
               color = 25;
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
         cout << ' ' << colorindex[24];
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
      legend[i].setAll(24);  // set to background color
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
            legend[i][j] = 25;
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
               legend[i][j] = 25;
            }
	    lastv = v;
	    continue;
         }
	 if (!((v==1)||(v==3)||(v==6)||(v==8)||(v==10))) {
            continue;
         }
	 if ((i==blackend)) {
            legend[i][j] = 25;
         } else if (i > (blackend + startrow)/2) {  // major keys
            legend[i][j] = v;
         } else {                          // minor keys
            legend[i][j] = v+12;
         }
	 lastv = v;
      }
   }

   Array<const char*> transcolor;
   transcolor.setSize(26);
   transcolor.setAll(0);
   transcolor[24] = colorindex[24];
   transcolor[25] = colorindex[25];
   int transpose = 0;
   int rrotate   = 0;
   for (i=0; i<12; i++) {
      transcolor[i] = colorindex[(i+transpose+rrotate) % 12];
      transcolor[i+12] = colorindex[((i+transpose+rrotate) % 12)+12];
   }

   for (i=0; i<legend.getSize(); i++) {
      for (j=0; j<legend[i].getSize(); j++) {
         cout << ' ' << transcolor[legend[i][j]];
      }
      cout << "\n";
   }
}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("t|type=b",          "show only chord type");
   opts.define("i|inversion=b",     "show only chord inversion");
   opts.define("r|root=b",          "show only chord root");
   opts.define("a|assemble|append=b", "append analysis to input data");
   opts.define("f|format=s:t:i:r",  "control display style");
   opts.define("u|unknown=s:X",     "control display of unknowns");
   opts.define("U|unknown-pcs=b",   "print pcs of unknown sonrities");
   opts.define("d|debug=b",         "determine bad input line num");
   opts.define("n|notes=b",         "display pitch classes in sonority");
   opts.define("iv=b",              "print interval vector");
   opts.define("x|sonor|suspension=b",    "print marker if not all start attacks");
   opts.define("forte=b",           "print forte interval vector set name");
   opts.define("Tn|tn=b",           "print forte set with subsets");
   opts.define("Tni|tni=b",         "print forte set with subsets/inversion");
   opts.define("s|suppress=b",      "suppress data if overlapping sonority");
   opts.define("I|image=s:800x20",  "display image of major/minor chords");
   opts.define("P|paren-off=b",     "suppress parentheses for overlapping");
   opts.define("N|separator=s: ",   "characters to separate pitch classes");
   opts.define("o|octave=i:4",      "characters to separate pitch classes");
   opts.define("b|barlines=b",      "display barlines at bottom of image");
   opts.define("l|legend=b",        "display color mapping");

   opts.define("author=b",          "author of program");
   opts.define("version=b",         "compilation info");
   opts.define("example=b",         "example usages");
   opts.define("h|help=b",          "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, May 1998" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: Nov 2000" << endl;
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

   if (opts.getBoolean("root")) {
      ChordQuality::setDisplay("r");
   } else if (opts.getBoolean("type")) {
      ChordQuality::setDisplay("t");
   } else if (opts.getBoolean("inversion")) {
      ChordQuality::setDisplay("i");
   } else {
      ChordQuality::setDisplay(opts.getString("format"));
   }

   Convert::chordType.setNullName(opts.getString("unknown"));
   Convert::chordInversion.setNullName(opts.getString("unknown"));
   Convert::kernPitchClass.setNullName(opts.getString("unknown"));

   strncpy(unknown, opts.getString("unknown"), 64);
   strcat(unknown, ":");
   strncat(unknown, opts.getString("unknown"), 64);
   strcat(unknown, ":");
   strncat(unknown, opts.getString("unknown"), 64);

   explicitQ =  opts.getBoolean("unknown-pcs");
   imageQ    =  opts.getBoolean("image");
   if (imageQ) {
      imagex = 800;
      imagey = 20;
      sscanf(opts.getString("image"), "%dx%d", &imagex, &imagey);
   }

   ivQ       =  opts.getBoolean("iv");
   attackQ   =  opts.getBoolean("suspension");
   forteQ    =  opts.getBoolean("forte");
   tnQ       =  opts.getBoolean("Tn");
   if (tnQ) {
      forteQ = 1;
   }
   tniQ      =  opts.getBoolean("Tni");
   if (tniQ) {
      tnQ = 1;
      forteQ = 1;
   }
   notesQ    =  opts.getBoolean("notes");
   suppressQ =  opts.getBoolean("suppress");
   parenQ    = !opts.getBoolean("paren-off");
   appendQ   =  opts.getBoolean("append");
   barlinesQ =  opts.getBoolean("barlines");
   legendQ   =  opts.getBoolean("legend");
   if (opts.getBoolean("separator")) {
      notesep   =  opts.getString("separator");
   }
   if (opts.getBoolean("octave")) {
      octaveVal =  opts.getInteger("octave");
   }
}



//////////////////////////////
//
// example -- example usage of the sonority program
//

void example(void) {
   cout <<
   "                                                                         \n"
   "# example usage of the sonority program.                                 \n"
   "# analyze a Bach chorale for chord qualities:                            \n"
   "     sonority chor217.krn                                                \n"
   "                                                                         \n"
   "# display the chord analysis with original data:                         \n"
   "     sonority -a chor217.krn                                             \n"
   "                                                                         \n"
   "# display only the roots of chords:                                      \n"
   "     sonority -r chor217.krn                                             \n"
   "                                                                         \n"
   << endl;
}



//////////////////////////////
//
// processRecords -- looks at humdrum records and determines chord
//	sonority quality;
//

void processRecords(HumdrumFile& infile) {
   Array<ChordQuality> cq;
   infile.analyzeSonorityQuality(cq);
   ChordQuality quality;

   int foundstart = 0;
   char aString[512] = {0};

   for (int i=0; i<infile.getNumLines(); i++) {
      if (options.getBoolean("debug")) {
         cout << "processing line " << (i+1) << " of input ..." << endl;
	 cout << "LINE IS: " << infile[i] << endl;
      }
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_bibliography:
         case E_humrec_global_comment:
            cout << infile[i] << endl;
            break;
         case E_humrec_data_comment:
            if (appendQ) {
	       cout << infile[i] << "\t";
            } 
            if (infile[i].equalFieldsQ("**kern")) {
               cout << infile[i][0];
            } else {
               cout << "!";
            }
            cout << endl;
            break;
         case E_humrec_data_interpretation:
            if (appendQ) {
               cout << infile[i] << "\t";
            }
            if (!foundstart && infile[i].hasExclusiveQ()) {
               foundstart = 1;
               if (tniQ) {
                  cout << "**Tni";
               } else if (tnQ) {
                  cout << "**Tn";
               } else if (forteQ) {
                  cout << "**forte";
               } else {
                  cout << "**qual";
               }
            } else {
               if (infile[i].equalFieldsQ("**kern") && 
                   (!infile[i].isSpineManipulator(0))) {
                  cout << infile[i][0];
               } else {
                  cout << "*";
               }
            }
	    cout << endl;
            break;
         case E_humrec_data_kern_measure:
            if (appendQ) {
               cout << infile[i] << "\t";
            }
            cout << infile[i][0];
            cout << endl;
            break;
         case E_humrec_data:
            if (appendQ) {
               cout << infile[i] << "\t";
            }
            // handle null fields
            if (infile[i].equalFieldsQ("**kern", ".")) {
	       cout << "." << endl;
               break;
            }
            if (ivQ) {
               Array<int> iv;
               infile.getIntervalVector(iv, i);
               cout << "<";
               for (int ii=0; ii<iv.getSize(); ii++) {
                  if (iv[ii] < 9) {
                     cout << iv[ii];
                     continue;
                  }
                  if (iv[ii] < 36) {
                     cout << char(iv[ii]-10+'A');
                     continue;
                  }
                  if (ii > 0) {
                     cout << ",";
                  }
                  cout << iv[ii];
                  if (ii < 5) {
                     cout << ",";
                  }
               }
               cout << ">" << endl;

            } else if (forteQ) {
               const char* name = infile.getForteSetName(i);
               cout << name;
               if (tnQ) {
                  if (strcmp(name, "3-11") == 0) {
                     if (strcmp(cq[i].getTypeName(), "min") == 0) {
                        cout << "A";
                     } else if (strcmp(cq[i].getTypeName(), "maj") == 0) {
                        cout << "B";
                     }
                  }
               }
               if (tniQ) {
                  int inversion = -1;
                  if (strcmp(name, "3-11") == 0) {
                     if ((strcmp(cq[i].getTypeName(), "min") == 0) || 
                        (strcmp(cq[i].getTypeName(), "maj") == 0)) {
                        inversion = cq[i].getInversion();
                     }
                     if (inversion >= 0) {
                        cout << char('a'+inversion);
                     }
                  }
               }
               if (attackQ) {
                  printAttackMarker(infile, i);
               }
               cout << endl;

            } else if (notesQ == 0) {
               quality = cq[i];
               quality.makeString(aString, explicitQ);
               if (strcmp(aString, "") != 0) {
                  if (strcmp(aString, unknown) == 0 || (
                        quality.getType()==E_chord_note && 
                              options.getBoolean("root"))
                        ) {
                     char tempbuffer[128] = {0};
	             strcpy(tempbuffer, aString);
                     strcpy(aString, options.getString("unknown"));
	             strcat(aString, tempbuffer);
                  }
		  if (suppressQ && transitionalSonority(quality, infile, i)) {
                     strcpy(aString, ".");
                  }
               } else {
                  strcpy(aString, "rest");
               }
	       cout << aString << endl;
            } else {
               quality = cq[i];
               fillStringWithNotes(aString, quality, infile, i);
	       cout << aString << endl;
            }

            break;
         default:
            cerr << "Error on line " << (i+1) << " of input" << endl;
            cerr << "record type = " << infile[i].getType() << endl;
            exit(1);
      }
   }

}



//////////////////////////////
//
// printAttackMarker -- print an "x" if the there is any note at
//    the start of the region which is not attacked (i.e., suspended
//    from a previous sonority.
//

void printAttackMarker(HumdrumFile& infile, int line) {
   int j, ii, jj;
   int& i = line;
   int dotQ;
   for (j=0; j<infile[i].getFieldCount(); j++) {
      if (!infile[i].isExInterp(j, "**kern")) {
         continue;
      }
      if (strcmp(infile[i][j], ".") == 0) {
        ii = infile[i].getDotLine(j);
        jj = infile[i].getDotSpine(j);
        dotQ = 1;
      } else {
         ii = i;
         jj = j;
         dotQ = 0;
      }
      if (strchr(infile[ii][jj], 'r') != NULL) {
         continue;
      } else if (dotQ) {
         cout << "x";
         return;
      }
      if (strchr(infile[ii][jj], '_') != NULL) {
         cout << "x";
         return;
      }
      if (strchr(infile[ii][jj], ']') != NULL) {
         cout << "x";
         return;
      }
   }

}



//////////////////////////////
//
// transitionalSonority --
//

int transitionalSonority(ChordQuality& quality, HumdrumFile& infile, int line) {
   SigCollection<int> notes;
   quality.getNotesInChord(notes);

   Array<int> octave;
   octave.setSize(notes.getSize());
   octave.allowGrowth(0);
   octave.setAll(0);

   Array<int> sounding;
   sounding.setSize(notes.getSize());
   sounding.allowGrowth(0);
   sounding.setAll(0);

   // int bassindex = identifyBassNote(notes, infile, line, sounding);

   int i;
   for (i=0; i<sounding.getSize(); i++) {
      if (sounding[i] == 0) {
         return 1;
      }
   }

   return 0;
}



//////////////////////////////
//
// fillStringWithNotes --
//

void fillStringWithNotes(char* string, ChordQuality& quality, 
      HumdrumFile& infile, int line) {

   string[0] = '\0';

   SigCollection<int> notes;
   quality.getNotesInChord(notes);

   Array<int> octave;
   octave.setSize(notes.getSize());
   octave.allowGrowth(0);
   octave.setAll(4);

   Array<int> sounding;
   sounding.setSize(notes.getSize());
   sounding.allowGrowth(0);
   sounding.setAll(0);

   int bassindex = identifyBassNote(notes, infile, line, sounding);
   if (bassindex >= 0) {
      octave[bassindex] = 3;
      //if (notes[bassindex] >= 40) {
      //   octave[bassindex] += -2;
      //}
   }

   int i;
   if (suppressQ) {
      for (i=0; i<sounding.getSize(); i++) {
         if (sounding[i] == 0) {
            strcpy(string, ".");
            return;
         }
      }
   }

   string[0] = '\0';
   char buffer[32] = {0};
   for (i=0; i<notes.getSize(); i++) {
      //if (octaveVal >= 0) {
      //   Convert::base40ToKern(buffer, (notes[i]%40) + octaveVal * 40);
      //} else {
      //   Convert::base40ToKern(buffer, notes[i] + ((octave[i]+4) * 40));
      //}
      Convert::base40ToKern(buffer, notes[i]%40 + (octave[i] * 40));
      if (parenQ && (sounding[i] == 0)) {
         strcat(string, "(");
      }
      strcat(string, buffer);
      if (parenQ && (sounding[i] == 0)) {
         strcat(string, ")");
      }
      if (i < notes.getSize() - 1) {
         strcat(string, notesep);
      }
   }

}



//////////////////////////////
//
// identifyBassnote --
//

int identifyBassNote(SigCollection<int>& notes, HumdrumFile& infile, 
      int line, Array<int>& sounding) {
   int j, k;
   int output = -1;
   int minval = 1000000;
   int value;
   int tcount;
   char buffer[128] = {0};

   Array<int> soundQ(40);
   soundQ.setAll(0);

   sounding.setSize(notes.getSize());
   sounding.setAll(0);

   int pline;
   int pspine;

   int dotQ = 0;

   if (notes.getSize() == 0) {
      return -1;
   }

   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (!infile[line].isExInterp(j, "**kern")) {
         continue;
      }
      dotQ = 0;
      if (strcmp(infile[line][j], ".") == 0) {
         pline  = infile[line].getDotLine(j);
         pspine = infile[line].getDotSpine(j);
         dotQ = 1;
      } else {
         pline = line;
         pspine = j;
      }
      tcount = infile[pline].getTokenCount(pspine);
      for (k=0; k<tcount; k++) {
         infile[pline].getToken(buffer, pspine, k);
         if (strchr(buffer, 'r') != NULL) {
            continue;
         }
	 if (strcmp(buffer, ".") == 0) {
            // shouldn't get here...
            continue;
         }
	 value = Convert::kernToMidiNoteNumber(buffer);
         if (value < 0) {
            continue;
         }
         if (value < minval) {
            minval = value;
         }
         if (dotQ) {
            continue;
         }
         if (strchr(buffer, '_') != NULL) {
            continue;
         }
         if (strchr(buffer, ']') != NULL) {
            continue;
         }
	 value = Convert::kernToBase40(buffer);
         if (value < 0) {
            continue;
         }
         soundQ[value % 40] = 1;
      }
   }

   if (minval > 100000) {
      return -1;
   }

   minval = minval % 12;
   int i;
   int tval;
   for (i=0; i<notes.getSize(); i++) {
      if (notes[i] >= 0) {
         if (soundQ[notes[i]%40]) {
            sounding[i] = 1;
         }
      }
      tval = Convert::base40ToMidiNoteNumber(notes[i]);
      if (tval < 0) {
         continue;
      }
      tval = tval % 12;
      if (tval == minval) {
         output = i;
         // break;  need to supress this because of sounding tests
      }
   }
   return output;
}



//////////////////////////////
//
// usage -- gives the usage statement for the sonority program
//

void usage(const char* command) {
   cout <<
   "                                                                         \n"
   "Analyzes **kern data and generates a **qual spine which gives the chord  \n"
   "quality of the **kern spines.  Currently, input spines cannot split or   \n"
   "join.                                                                    \n"
   "                                                                         \n"
   "Usage: " << command << " [-a][-i|-r|-t] [input1 [input2 ...]]            \n"
   "                                                                         \n"
   "Options:                                                                 \n"
   "   -a = assemble the **qual analysis spine with input data               \n"
   "   -i = displays the **qual chord inversion only                         \n"
   "   -r = displays the **qual chord root only                              \n"
   "   -t = displays the **qual chord type only                              \n"
   "   --options = list of all options, aliases and default values           \n"
   "                                                                         \n"
   << endl;
}



// md5sum: c1cc61fb24d51daddeaf667365673f20 sonority.cpp [20110317]
