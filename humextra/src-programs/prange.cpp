//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Mar 31 21:51:34 PST 2005
// Last Modified: Thu Feb  9 07:34:18 PST 2012 (SCORE display output by voice)
// Filename:      ...sig/examples/all/range.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/range.cpp
// Syntax:        C++; museinfo
//
// Description:   Calculate pitch histograms in a score of **kern data.
// 

#include "PerlRegularExpression.h"
#include "humdrum.h"

#include <string.h>
#include <math.h>

#define OBJTAB "\t\t\t\t\t\t"
#define SVGTAG "_99%svg%";

// function declarations
void   checkOptions               (Options& opts, int argc, char* argv[]);
void   example                    (void);
void   usage                      (const char* command);
void   generateAnalysis           (HumdrumFile& infile, 
                                   Array<Array<double> >& rdata, 
                                   Array<int>& kernspines);
void   printAnalysis              (Array<double>& rdata, 
                                   HumdrumFile& infile);
void   printPercentile            (Array<double>& midibins, 
                                   double percentile);
double countNotesInRange          (Array<double>& midibins, 
                                   int low, int high);
void   clearHistograms            (Array<Array<double> >& bins, int start = 0);
void   getRange                   (int& rangeL, int& rangeH, 
                                   const char* rangestring);
int    getTessitura               (Array<double>& midibins);
double getMean                    (Array<double>& midibins);
int    getMedian                  (Array<double>& midibins);
void   printScoreVoice            (HumdrumFile& infile, double hpos, 
                                   Array<double>& midibins, int kernspine, 
                                   double maxhist);
void   getVoice                   (char* voicestring, HumdrumFile& infile, 
                                   int kernspine);
int    getMaxPitch                (Array<double>& midibins);
int    getMinPitch                (Array<double>& midibins);
int    getStaffBase12             (int pitch);
double getVpos                    (double pitch, int staff);
double getMaxValue                (Array<double>& bins);
void   printScoreFile             (Array<Array<double> >& midibins, 
                                   HumdrumFile& infile, Array<int>& kernspines);
void   getTitle                   (char* titlestring, HumdrumFile& infile);
void   growHistograms             (Array<Array<double> >& midibins, int voices);
int    getVindex                  (int track, Array<int>& kernspines);
int    getTopQuartile             (Array<double>& midibins);
int    getBottomQuartile          (Array<double>& midibins);
int    getDiatonicInterval        (int note1, int note2);
void   printScoreXmlHeader        (void);
void   printScoreXmlFooter        (void);
void   printFilenameBase          (const char* filename);
void   printXmlEncodedText        (const char* strang);
void   printScoreEncodedText      (const char* strang);
int    getKeySignature            (HumdrumFile& infile);
void   printHTMLStringEncodeSimple(const char* string);
void   printDiatonicPitchName     (int base12);
const char* getTitle           (char* hbuffer, double value, int pitch);

// global variables
Options      options;            // database for command-line arguments
int          durationQ    = 0;   // used with -d option
int          debugQ       = 0;   // used with --debug option
int          percentileQ  = 0;   // used with -p option
int          addfractionQ = 0;   // used with -f option
double       percentile   = 0.0; // used with -p option
int          rangeQ       = 0;   // used with -r option
int          rangeL       = 0;   // used with -r option
int          pitchQ       = 0;   // used with --pitch option 
int          rangeH       = 0;   // used with -r option
int          printQ       = 0;   // used with --print option
int          normQ        = 0;   // used with -N option
int          scoreQ       = 0;   // used with --score option
int          xmlQ         = 0;   // used with --sx option
int          diatonicQ    = 0;   // used with -D option
int          hoverQ       = 0;   // used with --hover option
int          quartileQ    = 0;   // used with --quartile option
int          fillonlyQ    = 0;   // used with --fill option
int          defineQ      = 1;   // used with --no-define option
const char*  FILENAME     = "";

///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile infile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   Array<double> values;
   values.setSize(0);

   Array<Array<double> > midibins;
   midibins.setSize(1);
   clearHistograms(midibins);

   Array<int> kernSpines;

   // can only handle one input if SCORE display is being given.
   if (scoreQ && numinputs > 1) {
      numinputs = 1;
   }

   for (int i=0; i<numinputs || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         FILENAME = options.getArg(i+1);
         infile.read(FILENAME);
      }
      infile.getTracksByExInterp(kernSpines, "**kern");
      growHistograms(midibins, kernSpines.getSize());
      generateAnalysis(infile, midibins, kernSpines);
   }

   if (xmlQ) {
      printScoreXmlHeader();
   }

   if (!scoreQ) {
      printAnalysis(midibins[0], infile);
   } else {
      printScoreFile(midibins, infile, kernSpines);
   }

   if (xmlQ) {
      printScoreXmlFooter();
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// printScoreXmlHeader --
//

void printScoreXmlHeader(void) {
   cout << "<ScoreXML version=\"1.0\">\n";
   cout << "\t<scoreHead>\n";
   cout << "\t\t<info>\n";
   cout << "\t\t\t<description>Pitch range information by voice</description>\n";
   cout << "\t\t</info>\n";
   cout << "\t</scoreHead>\n";
   cout << "\t<scoreData>\n";
}



//////////////////////////////
//
// printScoreXmlFooter --
//

void printScoreXmlFooter(void) {
   cout << "\t</scoreData>\n";
   cout << "</ScoreXML>\n";
}



//////////////////////////////
//
// growHistograms --
//

void growHistograms(Array<Array<double> >& midibins, int voices) {
   int oldsize = midibins.getSize();
   if (voices <= oldsize-1) {
      return;
   }
   midibins.setSize(voices+1);
   clearHistograms(midibins, oldsize);
}



//////////////////////////////
//
// printFilenameBase --
//

void printFilenameBase(const char* filename) {
   PerlRegularExpression pre;
   if (pre.search(filename, "([^/]+)\\.([^.]*)", "")) {
      if (strlen(pre.getSubmatch(1)) <= 8) {
         printXmlEncodedText(pre.getSubmatch(1));
      } else {
         // problem with too long a name (MS-DOS will have problems).
         // optimize to chop off everything after the dash in the
         // name (for Josquin catalog numbers).
         PerlRegularExpression pre2;
         Array<char> shortname;
         shortname.setSize(strlen(pre.getSubmatch())+1);
         strcpy(shortname.getBase(), pre.getSubmatch());
         if (pre2.sar(shortname, "-.*", "", "")) {
            printXmlEncodedText(shortname.getBase());
         } else {
            printXmlEncodedText(shortname.getBase());
         }
      }
   }
}


//////////////////////////////
//
// printReferenceRecords --
//

void printReferenceRecords(HumdrumFile& infile) {
   int i;
   char buffer[1024] = {0};
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      cout <<  "\t\t\t\t\t\t<?Humdrum key=\"";
      printXmlEncodedText(infile[i].getBibKey(buffer));
      cout << "\" value=\"";
      printXmlEncodedText(infile[i].getBibValue(buffer));
      cout << "\"?>\n";
   }
}



//////////////////////////////
//
// printScorEncodedText -- print SCORE text string
//    See SCORE 3.1 manual additions (page 19) for more.
//

void printScoreEncodedText(const char* strang) {
   Array<char> newstring;
   newstring.setSize(strlen(strang)+1);
   strcpy(newstring.getBase(), strang);
   PerlRegularExpression pre;
   // pre.sar(newstring, "&(.)acute;", "<<$1", "g");
   pre.sar(newstring, "&aacute;", "<<a", "g");
   pre.sar(newstring, "&eacute;", "<<e", "g");
   pre.sar(newstring, "&iacute;", "<<i", "g");
   pre.sar(newstring, "&oacute;", "<<o", "g");
   pre.sar(newstring, "&uacute;", "<<u", "g");
   pre.sar(newstring, "&Aacute;", "<<A", "g");
   pre.sar(newstring, "&Eacute;", "<<E", "g");
   pre.sar(newstring, "&Iacute;", "<<I", "g");
   pre.sar(newstring, "&Oacute;", "<<O", "g");
   pre.sar(newstring, "&Uacute;", "<<U", "g");

   // pre.sar(newstring, "&(.)grave;", ">>$1", "g");
   pre.sar(newstring, "&agrave;", ">>a", "g");
   pre.sar(newstring, "&egrave;", ">>e", "g");
   pre.sar(newstring, "&igrave;", ">>i", "g");
   pre.sar(newstring, "&ograve;", ">>o", "g");
   pre.sar(newstring, "&ugrave;", ">>u", "g");
   pre.sar(newstring, "&Agrave;", ">>A", "g");
   pre.sar(newstring, "&Egrave;", ">>E", "g");
   pre.sar(newstring, "&Igrave;", ">>I", "g");
   pre.sar(newstring, "&Ograve;", ">>O", "g");
   pre.sar(newstring, "&Ugrave;", ">>U", "g");

   // pre.sar(newstring, "&(.)uml;",   "%%$1", "g");
   pre.sar(newstring, "&auml;",   "%%a", "g");
   pre.sar(newstring, "&euml;",   "%%e", "g");
   pre.sar(newstring, "&iuml;",   "%%i", "g");
   pre.sar(newstring, "&ouml;",   "%%o", "g");
   pre.sar(newstring, "&uuml;",   "%%u", "g");
   pre.sar(newstring, "&Auml;",   "%%A", "g");
   pre.sar(newstring, "&Euml;",   "%%E", "g");
   pre.sar(newstring, "&Iuml;",   "%%I", "g");
   pre.sar(newstring, "&Ouml;",   "%%O", "g");
   pre.sar(newstring, "&Uuml;",   "%%U", "g");

   // pre.sar(newstring, "&(.)circ;",  "^^$1", "g");
   pre.sar(newstring, "&acirc;",  "^^a", "g");
   pre.sar(newstring, "&ecirc;",  "^^e", "g");
   pre.sar(newstring, "&icirc;",  "^^i", "g");
   pre.sar(newstring, "&ocirc;",  "^^o", "g");
   pre.sar(newstring, "&ucirc;",  "^^u", "g");
   pre.sar(newstring, "&Acirc;",  "^^A", "g");
   pre.sar(newstring, "&Ecirc;",  "^^E", "g");
   pre.sar(newstring, "&Icirc;",  "^^I", "g");
   pre.sar(newstring, "&Ocirc;",  "^^O", "g");
   pre.sar(newstring, "&Ucirc;",  "^^U", "g");

   // pre.sar(newstring, "&(.)cedil;", "##$1", "g");
   pre.sar(newstring, "&ccedil;", "##c", "g");
   pre.sar(newstring, "&Ccedil;", "##C", "g");

   pre.sar(newstring, "\\|",        "?|",   "g");
   pre.sar(newstring, "\\\\",       "?\\",  "g");
   pre.sar(newstring, "---",        "?m",   "g");
   pre.sar(newstring, "--",         "?n",   "g");
   pre.sar(newstring, "-sharp",     "?2",   "g");
   pre.sar(newstring, "-flat",      "?1",   "g");
   pre.sar(newstring, "-natural",   "?3",   "g");
   pre.sar(newstring, "/",          "\\",   "g");
   pre.sar(newstring, "\\[",        "?[",   "g");
   pre.sar(newstring, "\\]",        "?]",   "g");

   if (xmlQ) {
      printXmlEncodedText(newstring.getBase());
   } else {
      cout << newstring;
   }
}



//////////////////////////////
//
// printXmlEncodedText -- convert 
//    & to &amp;
//    " to &quot;
//    ' to &spos;
//    < to &lt;
//    > to &gt;
//

void printXmlEncodedText(const char* strang) {
   PerlRegularExpression pre;
   Array<char> astring;
   astring.setSize(strlen(strang)+128);
   astring.setGrowth(12345);
   astring.setSize(strlen(strang)+1);
   strcpy(astring.getBase(), strang);
  
   pre.sar(astring, "&",  "&amp;",  "g");
   pre.sar(astring, "'",  "&apos;", "g");
   pre.sar(astring, "\"", "&quot;", "g");
   pre.sar(astring, "<",  "&lt;",   "g");
   pre.sar(astring, ">",  "&gt;",   "g");

   cout << astring;
}



//////////////////////////////
//
// printScoreFile --
//

void printScoreFile(Array<Array<double> >& midibins, HumdrumFile& infile,
   Array<int>& kernspines) {

   char titlestring[1024] = {0};
   getTitle(titlestring, infile);

   if (xmlQ) {
      // print file start info;
      cout << "\t\t<page>\n";
      cout << "\t\t\t<pageData>\n";
      cout << "\t\t\t\t<scoreFile>\n";
      cout << "\t\t\t\t\t<fileHead>\n";
      cout << "\t\t\t\t\t\t<name>";
      printFilenameBase(FILENAME);
      cout << "</name>\n";
      cout << "\t\t\t\t\t\t<pmxExt>pmx";
      cout << "</pmxExt>\n";
      printReferenceRecords(infile);
      cout << "\t\t\t\t\t</fileHead>\n";
      cout << "\t\t\t\t\t<fileObjects>\n";
   }

   if ((!xmlQ) && defineQ) {
      cout << "#define SVG t 1 1 \\n_99%svg%\n";
   }

   // print CSS style information if requested
   if (hoverQ) {
      if (!xmlQ) {
         if (defineQ) {
            cout << "SVG ";
         } else {
            cout << "t 1 1\n";
            cout << SVGTAG;
         }
         if (fillonlyQ) {
            printScoreEncodedText("<style type=\"text/css\">.bar:hover path&#123;fill:red&#125;</style>");
         } else {
            printScoreEncodedText("<style type=\"text/css\">.bar:hover &#123;color:red;stroke:red&#125;</style>");
         }
         cout << "\n";
      } else {
         cout << OBJTAB << "<obj p1=\"16\" p2=\"1\"";
         cout << " p3=\"2\" p4=\"20\" p5=\"1\" p6=\"1\" p7=\"0\"";
         cout << " p8=\"0\" p9=\"0\" p10=\"0\" p11=\"-1\"";
         cout << " text=\"" << SVGTAG;
         if (fillonlyQ) {
            printScoreEncodedText("<style type=\"text/css\">.bar:hover path&#123;fill:red&#125;</style>");
         } else {
            printScoreEncodedText("<style type=\"text/css\">.bar:hover &#123;color:red&#125;stroke:red;</style>");
         }
         cout << "\">\n";
      }
   }

   // print title
   if (!xmlQ) {
      cout << "t 2 10 14 1 1 0 0 0 0 -1.35\n";
      // cout << "_03";
      printScoreEncodedText(titlestring);
      cout << "\n";
   } else {
      cout << OBJTAB << "<obj p1=\"16\" p2=\"2\" p3=\"10\""
           <<    " p4=\"14\" p5=\"1\" p6=\"1\" p11=\"-1.35\" "
           <<    "text=\""; 
      printScoreEncodedText(titlestring);
      cout << "\"/>\n";
   }

   // print duration label if duration weighting is being used
   if (durationQ) {
      if (!xmlQ) {
         cout << "t 2 180.075 14 1 0.738 0 0 0 0 0\n";
         cout << "_01(durations)\n";
      } else {
         cout << OBJTAB << "<obj p1=\"16\" p2=\"2\" p3=\"180.075\""
              <<    " p4=\"14\" p5=\"1\""
              <<    " p6=\"0.738\" text=\"_01(durations)\"/>\n";
      }
   }

   // print staff lines
   if (!xmlQ) {
      cout << "8 1 0 0 0 200\n";   // staff 1
      cout << "8 2 0 -6 0 200\n";   // staff 2
   } else {
      cout << OBJTAB << "<obj p1=\"8\" p2=\"1\" p6=\"200\"/>\n";
      cout << OBJTAB << "<obj p1=\"8\" p2=\"2\" p4=\"-6\" "
           <<    "p6=\"200\"/>\n";
   }

   int keysig = getKeySignature(infile);
   // print key signature
   if (keysig) {
      if (!xmlQ) {
         cout << "17 1 10 0 " << keysig << " 1.0\n"; 
         cout << "17 2 10 0 " << keysig << "\n";    
      } else {
         cout << OBJTAB << "<obj p1=\"17\" p2=\"1\" p3=\"10\" p4=\"0\"";
         cout << " p5=\"" << keysig << "\" p6=\"1\"/>\n";
         cout << OBJTAB << "<obj p1=\"17\" p2=\"2\" p3=\"10\" p4=\"0\"";
         cout << " p5=\"" << keysig << "\"/>\n";
      }
   }

   // print barlines
   if (!xmlQ) {
      cout << "14 1 0 2\n";         // starting barline
      cout << "14 1 200 2\n";       // ending barline
      cout << "14 1 0 2 8\n";       // curly brace at start
   } else {
      cout << OBJTAB << "<obj p1=\"14\" p2=\"1\" p4=\"2\"/>\n";
      cout << OBJTAB << "<obj p1=\"14\" p2=\"1\" p3=\"200\" "
           <<    "p4=\"2\"/>\n";
      cout << OBJTAB << "<obj p1=\"14\" p2=\"1\" p4=\"2\" "
           <<    "p5=\"8\"/>\n";
   }

   // print clefs
   if (!xmlQ) {
      cout << "3 2 2\n";            // treble clef
      cout << "3 1 2 0 1\n";        // bass clef
   } else {
      cout << OBJTAB << "<obj p1=\"3\" p2=\"2\" p3=\"2\"/>\n";
      cout << OBJTAB << "<obj p1=\"3\" p2=\"1\" p3=\"2\" p5=\"1\"/>\n";
   }

   int ii;
   // calculate the locations for each voice.
   Array<double> hpos;
   double minn = 25;
   double maxx = 170.0;
   hpos.setSize(kernspines.getSize());
   hpos.last() = minn;
   hpos[0] = maxx;
   int i;
   if (hpos.getSize() > 2) {
      for (i=1; i<hpos.getSize()-1; i++) {
         ii = hpos.getSize() - i - 1;
         hpos[i] = (double)ii / (hpos.getSize()-1) * (maxx - minn) + minn;
      }
   }

   for (i=hpos.getSize()-1; i>=0; i--) {
      printScoreVoice(infile, hpos[i], midibins[i+1], kernspines[i], 17.6);
   }


   if (xmlQ) {
      cout << "\t\t\t\t\t</fileObjects>\n";
      cout << "\t\t\t\t</scoreFile>\n";
      cout << "\t\t\t</pageData>\n";
      cout << "\t\t</page>\n";
   }
}



//////////////////////////////
//
// getKeySignature -- find first key signature in file.
//

int getKeySignature(HumdrumFile& infile) {
   int i, j;
   PerlRegularExpression pre;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isInterpretation()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (pre.search(infile[i][j], "^\\*k\\[(.*)\\]", "")) {
            return Convert::kernKeyToNumber(infile[i][j]);
         }
      }
   }

   return 0; // C major key signature
}



//////////////////////////////
//
// printScoreVoice -- print the range information for a particular voice.
//


void printScoreVoice(HumdrumFile& infile, double hpos, Array<double>& midibins,
      int kernspine, double maxhist) {
   
   int minpitchbase12, maxpitchbase12;
   int mini = getMinPitch(midibins);
   int maxi = getMaxPitch(midibins);
   
   if (diatonicQ) {
      minpitchbase12 = Convert::base7ToBase12(mini);
      maxpitchbase12 = Convert::base7ToBase12(maxi);
   } else {
      minpitchbase12 = mini;
      maxpitchbase12 = maxi;
   }
   if ((minpitchbase12 <= 0) && (maxpitchbase12 <= 0)) {
      return;
   }

   int    staff;
   double vpos;
   char voicestring[1024] = {0};
   getVoice(voicestring, infile, kernspine);
   int voicevpos = -3;
   staff = getStaffBase12(minpitchbase12);
   int lowestvpos = getVpos(minpitchbase12, staff);
   if ((staff == 1) && (lowestvpos <= 0)) {
      voicevpos += lowestvpos;
      voicevpos -= 1;
   }

   double maxvalue = getMaxValue(midibins);
   double value;
   double hoffset = 2.3333;
   int i;
   int base12;
   char hbuffer[1024] = {0};
   for (i=mini; i<=maxi; i++) {
      if (midibins[i] == 0.0) {
         continue;
      }
      base12 = i;
      if (diatonicQ) {
         base12 = Convert::base7ToBase12(base12);
      }
      staff = getStaffBase12(base12);
      vpos  = getVpos(base12, staff);
      value = midibins[i] / maxvalue * maxhist + hoffset;
      if (!xmlQ) {
         if (hoverQ) {
            if (defineQ) {
               cout << "SVG ";
            } else {
               cout << "t 1 1\n";
               cout << SVGTAG;
            }
            getTitle(hbuffer, (int)midibins[i], i);
            printScoreEncodedText(hbuffer);
            cout << "\n";
         }
         cout << "1 " << staff << " " << hpos << " " << vpos;
         cout << " 0 -1 4 0 0 0 99 0 0 ";
         cout << value << "\n";      
         if (hoverQ) {
            if (defineQ) {
               cout << "SVG ";
            } else {
               cout << "t 1 1\n";
               cout << SVGTAG;
            }
            printScoreEncodedText("</g>");
            cout << "\n";
         }
      } else {
         if (hoverQ) {
            cout << OBJTAB << "<obj p1=\"16\" p2=\"1\"";
            cout << " p3=\"2\" p4=\"20\" p5=\"1\" p6=\"1\" p7=\"0\"";
            cout << " p8=\"0\" p9=\"0\" p10=\"0\" p11=\"-1\"";
            cout << " text=\"" << SVGTAG;
            getTitle(hbuffer, (int)midibins[i], i);
            printScoreEncodedText(hbuffer);
            cout << "\"/>\n";
         }
         cout << OBJTAB << "<obj p1=\"1\" p2=\"" << staff << "\" p3=\""
              << hpos << "\" p4=\"" << vpos << "\" p6=\"-1\" p7=\"4\""
              << " p11=\"99\" p14=\"" << value << "\"/>\n";
         if (hoverQ) {
            cout << OBJTAB << "<obj p1=\"16\" p2=\"1\"";
            cout << " p3=\"2\" p4=\"20\" p5=\"1\" p6=\"1\" p7=\"0\"";
            cout << " p8=\"0\" p9=\"0\" p10=\"0\" p11=\"-1\"";
            cout << " text=\"" << SVGTAG;
            printScoreEncodedText("</g>");
            cout << "\"/>\n";
         }
      }
   }

   if (strlen(voicestring) > 0) {
      // print voice name
      double tvoffset = -2.0;
      if (!xmlQ) {
         cout << "t 1 " << hpos << " " << voicevpos 
              << " 1 1 0 0 0 0 " << tvoffset;
         cout << "\n";
         cout << "_00";
         printScoreEncodedText(voicestring);
         cout << "\n";
      } else {
         cout << OBJTAB << "<obj p1=\"16\" p2=\"1\" p3=\"" << hpos
              <<   "\" p4=\"" << voicevpos << "\" p5=\"1\" p6=\"1\" "
              <<   "p11=\"" << tvoffset << "\" text=\"" << "_00"; 
         printScoreEncodedText(voicestring);
         cout << "\"/>\n";
      }
   }

   // print the lowest pitch in range
   staff = getStaffBase12(minpitchbase12);
   vpos = getVpos(minpitchbase12, staff);
   if (!xmlQ) {
      // if (hoverQ) {
      //    if (defineQ) {
      //       cout << "SVG ";
      //    } else {
      //       cout << "t 1 1\n";
      //       cout << SVGTAG;
      //    }
      //    printScoreEncodedText("<g><title>");
      //    printDiatonicPitchName(minpitchbase12);
      //    cout << ": lowest note";
      //    if (strlen(voicestring) > 0) {
      //       cout <<  " of " << voicestring << "\'s range";
      //    }
      //    printScoreEncodedText("</title>\n");
      // }
      cout << "1 " << staff << " " << hpos << " " << vpos
           << " 0 0 4 0 0 -2\n";
      //if (hoverQ) {
      //   if (defineQ) {
      //      cout << "SVG ";
      //   } else {
      //      cout << "t 1 1\n";
      //      cout << SVGTAG;
      //   }
      //   printScoreEncodedText("</g>\n");
      //}
   } else {
      cout << OBJTAB << "<obj p1=\"1\" p2=\"" << staff << "\" p3=\""
           << hpos << "\" p4=\"" << vpos << "\" p7=\"4\" p10=\"-2\"/>\n";
   }

   // print the highest pitch in range
   staff = getStaffBase12(maxpitchbase12);
   vpos = getVpos(maxpitchbase12, staff);
   if (!xmlQ) {
      // if (hoverQ) {
      //    if (defineQ) {
      //       cout << "SVG ";
      //    } else {
      //       cout << "t 1 1\n";
      //       cout << SVGTAG;
      //    }
      //    printScoreEncodedText("<g><title>");
      //    printDiatonicPitchName(maxpitchbase12);
      //    cout << ": highest note";
      //    if (strlen(voicestring) > 0) {
      //       cout <<  " of " << voicestring << "\'s range";
      //    }
      //    printScoreEncodedText("</title>\n");
      // }
      cout << "1 " << staff << " " << hpos << " " << vpos
           << " 0 0 4 0 0 -2\n";
      //if (hoverQ) {
      //   if (defineQ) {
      //      cout << "SVG ";
      //   } else {
      //      cout << "t 1 1\n";
      //      cout << SVGTAG;
      //   }
      //   printScoreEncodedText("</g>\n");
      //}
   } else {
      cout << OBJTAB << "<obj p1=\"1\" p2=\"" << staff << "\" p3=\""
           << hpos << "\" p4=\"" << vpos << "\" p7=\"4\" p10=\"-2\"/>\n";
   }

   /*
   double mean = getMean(midibins);
   // print the mean note
   staff = getStaffBase12(mean);
   vpos = getVpos(mean, staff);
   cout << "1.0 " << staff << ".0 " << hpos << " ";
   if (vpos > 0) {
      cout << vpos + 100;
   } else {
      cout << vpos - 100;
   }
   cout << " 0.0 0.0 4.0 0.0 0.0 -5.0\n";
   */

   double goffset = -1.66;
   double toffset = 1.5;

   double median = getMedian(midibins);
   if (diatonicQ) {
      median = Convert::base7ToBase12(median);
   }
   staff = getStaffBase12(median);
   vpos = getVpos(median, staff);

   // these offsets are useful when the quartile pitches are not shown...
   int vvpos = getDiatonicInterval(median, maxpitchbase12);
   int vvpos2 = getDiatonicInterval(median, minpitchbase12);
   double offset = goffset;
   if (vvpos <= 2) { 
      offset += toffset;
   } else if (vvpos2 <= 2) {
      offset -= toffset;
   }

   // print the median note
   if (!xmlQ) {
      //if (hoverQ) {
      //   if (defineQ) {
      //      cout << "SVG ";
      //   } else {
      //      cout << "t 1 1\n";
      //      cout << SVGTAG;
      //   }
      //   printScoreEncodedText("<g><title>");
      //   printDiatonicPitchName(median);
      //   cout << ": median note";
      //   if (strlen(voicestring) > 0) {
      //      cout <<  " of " << voicestring << "\'s range";
      //   }
      //   printScoreEncodedText("</title>\n");
      //}
      cout << "1 " << staff << " " << hpos << " ";
      if (vpos > 0) {
         cout << vpos + 100;
      } else {
         cout << vpos - 100;
      }
      cout << " 0 1 4 0 0 " << offset << "\n";
      //if (hoverQ) {
      //   if (defineQ) {
      //      cout << "SVG ";
      //   } else {
      //      cout << "t 1 1\n";
      //      cout << SVGTAG;
      //   }
      //   printScoreEncodedText("</g>\n");
      //}
   } else {
      if (hoverQ) {
         cout << OBJTAB << "<obj p1=\"16\" p2=\"1\" p3=\"1\" ";
         cout <<    "text=\"";
         printHTMLStringEncodeSimple("_99%svg%<g><title>: median note");
         if (strlen(voicestring) > 0) {
            cout <<  " for " << voicestring;
         }
         printHTMLStringEncodeSimple("<\\title>\n");
      }
      cout << OBJTAB << "<obj p1=\"1\" p2=\"" << staff << "\""
           << " p3=\"" << hpos << "\"" << " p4=\"";
      if (vpos > 0) {
         cout << vpos + 100;
      } else {
         cout << vpos - 100;
      }
      cout << "\" p6=\"1\" p7=\"4\" p10=\"" << offset << "\"/>\n";
      if (hoverQ) {
         cout << OBJTAB << "<obj p1=\"16\" p2=\"1\" p3=\"1\" text=\"";
         printHTMLStringEncodeSimple("_99%svg <\\g>");
         cout << "\"/>\n";
      }
   }


   int topquartile;
   if (quartileQ) { 
      // print top quartile
      topquartile = getTopQuartile(midibins);
      if (diatonicQ) {
         topquartile = Convert::base7ToBase12(topquartile);
      }
      staff = getStaffBase12(topquartile);
      vpos = getVpos(topquartile, staff);
      vvpos = getDiatonicInterval(median, topquartile);
      if (vvpos <= 2) {
         offset = goffset + toffset;
      } else {
         offset = goffset;
      }
      vvpos = getDiatonicInterval(maxpitchbase12, topquartile);
      if (vvpos <= 2) {
         offset = goffset + toffset;
      }
      if (!xmlQ) {
         if (hoverQ) {
            if (defineQ) {
               cout << "SVG ";
            } else {
               cout << "t 1 1\n";
               cout << SVGTAG;
            }
            printScoreEncodedText("<g><title>");
            printDiatonicPitchName(topquartile);
            cout << ": top quartile note";
            if (strlen(voicestring) > 0) {
               cout <<  " of " << voicestring << "\'s range";
            }
            printScoreEncodedText("</title>\n");
         }
         cout << "1 " << staff << " " << hpos << " ";
         if (vpos > 0) {
            cout << vpos + 100;
         } else {
            cout << vpos - 100;
         }
         cout << " 0 0 4 0 0 " << offset << "\n";
         if (hoverQ) {
            if (defineQ) {
               cout << "SVG ";
            } else {
               cout << "t 1 1\n";
               cout << SVGTAG;
            }
            printScoreEncodedText("</g>\n");
         }
      } else {
         cout << OBJTAB << "<obj p1=\"1\" p2=\"" << staff << "\""
              << " p3=\"" << hpos << "\"" << " p4=\"";
         if (vpos > 0) {
            cout << vpos + 100;
         } else {
            cout << vpos - 100;
         }
         cout << "\" p7=\"4\" p10=\"" << offset << "\"/>\n";
      }
   }
   
   
   // print bottom quartile
   if (quartileQ) {
      int bottomquartile = getBottomQuartile(midibins);
      if (diatonicQ) {
         bottomquartile = Convert::base7ToBase12(bottomquartile);
      }
      staff = getStaffBase12(bottomquartile);
      vpos = getVpos(bottomquartile, staff);
      vvpos = getDiatonicInterval(median, bottomquartile);
      if (vvpos <= 2) {
         offset = goffset + toffset;
      } else {
         offset = goffset;
      }
      vvpos = getDiatonicInterval(minpitchbase12, bottomquartile);
      if (vvpos <= 2) {
         offset = goffset - toffset;
      }
      if (!xmlQ) {
         if (hoverQ) {
            if (defineQ) {
               cout << "SVG ";
            } else {
               cout << "t 1 1\n";
               cout << SVGTAG;
            }
            printScoreEncodedText("<g><title>");
            printDiatonicPitchName(bottomquartile);
            cout << ": bottom quartile note";
            if (strlen(voicestring) > 0) {
               cout <<  " of " << voicestring << "\'s range";
            }
            printScoreEncodedText("</title>\n");
         }
         cout << "1.0 " << staff << ".0 " << hpos << " ";
         if (vpos > 0) {
            cout << vpos + 100;
         } else {
            cout << vpos - 100;
         }
         cout << " 0 0 4 0 0 " << offset << "\n";
         if (hoverQ) {
            if (defineQ) {
               cout << "SVG ";
            } else {
               cout << "t 1 1\n";
               cout << SVGTAG;
            }
            printScoreEncodedText("</g>\n");
         }
      } else {
         cout << OBJTAB << "<obj p1=\"1\" p2=\"" << staff << "\""
              << " p3=\"" << hpos << "\"" << " p4=\"";
         if (vpos > 0) {
            cout << vpos + 100;
         } else {
            cout << vpos - 100;
         }
         cout << "\" p7=\"4\" p10=\"" << offset << "\"/>\n";
      }
   }
}



//////////////////////////////
//
// printDiatonicPitchName --
//

void printDiatonicPitchName(int base12) {
   char buffer[16] = {0};
   Convert::base12ToKern(buffer, base12);
   buffer[1] = '\0';
   buffer[0] = toupper(buffer[0]);
   cout << buffer;
   int octave = base12 / 12 - 1;
   sprintf(buffer, "%d", octave);
   cout << buffer;
}



//////////////////////////////
//
// printHTMLStringEncodeSimple --
//

void printHTMLStringEncodeSimple(const char* string) {
   Array<char> newstring;
   newstring.setSize(strlen(string)+1);
   strcpy(newstring.getBase(), string);
   PerlRegularExpression pre;
   pre.sar(newstring, "&", "&amp;", "g");
   pre.sar(newstring, "<", "&lt;", "g");
   pre.sar(newstring, ">", "&lt;", "g");
   cout << newstring;
}



//////////////////////////////
//
// getTitle -- return the title of the histogram bar.
//

const char* getTitle(char* hbuffer, double value, int pitch) {
   strcpy(hbuffer, "<g class=\"bar\"><title>");
   char tempbuf[128] = {0};
   char pitchstring[128] = {0};
   int base12 = pitch;
   if (diatonicQ) {
      base12 = Convert::base7ToBase12(pitch);
   }

   Convert::base12ToKern(pitchstring, base12);
   pitchstring[1] = '\0';
   pitchstring[0] = toupper(pitchstring[0]);
   int octave = base12 / 12 - 1;
   char obuf[16] = {0};
   sprintf(obuf, "%d", octave);

   if (durationQ) {
      sprintf(tempbuf, "%lf", value/8.0);
      if (value/8.0 == 1.0) {
         strcat(tempbuf, " long on ");
      } else {
         strcat(tempbuf, " longs on ");
      }
      strcat(tempbuf, pitchstring);
   } else {
      sprintf(tempbuf, "%d ", (int)value);
      strcat(tempbuf, pitchstring);               
      strcat(tempbuf, obuf);
      if (value != 1.0) {
         strcat(tempbuf, "s");
      } 
   }
   strcat(hbuffer, tempbuf);
   strcat(hbuffer, "</title>");

   return hbuffer;
}



//////////////////////////////
//
// getDiatonicInterval --
//

int getDiatonicInterval(int note1, int note2) {
   int vpos1 = getVpos(note1, 0);
   int vpos2 = getVpos(note2, 0);
   return abs(vpos1 - vpos2) + 1;
}



//////////////////////////////
//
// getTopQuartile --
//

int getTopQuartile(Array<double>& midibins) {
   double sum = midibins.sum();

   double cumsum = 0.0;
   int i;
   for (i=midibins.getSize()-1; i>=0; i--) {
      if (midibins[i] <= 0.0) {
         continue;
      }      
      cumsum += midibins[i]/sum;
      if (cumsum >= 0.25) {
         return i;
      }
   }

   return -1;
}



//////////////////////////////
//
// getBottomQuartile --
//

int getBottomQuartile(Array<double>& midibins) {
   double sum = midibins.sum();

   double cumsum = 0.0;
   int i;
   for (i=0; i<midibins.getSize(); i++) {
      if (midibins[i] <= 0.0) {
         continue;
      }      
      cumsum += midibins[i]/sum;
      if (cumsum >= 0.25) {
         return i;
      }
   }

   return -1;
}




//////////////////////////////
//
// getMaxValue --
//

double getMaxValue(Array<double>& bins) {
   int i;
   double maxi = 0;
   for (i=1; i<bins.getSize(); i++) {
      if (bins[i] > bins[maxi]) {
         maxi = i;
      } 
   }
   return bins[maxi];
}


//////////////////////////////
//
// getVpos == return the position on the staff given the MIDI pitch
//     and the staff. 1=bass, 2=treble.
//

double getVpos(double pitch, int staff) {
   int base40;

   base40 = Convert::base12ToBase40(int(pitch));

   if (staff == 2) {
      return (Convert::base40ToScoreVPos(base40, 0) + (pitch - int(pitch)));
   } else {
      return (Convert::base40ToScoreVPos(base40, 1) + (pitch - int(pitch)));
   }
}



//////////////////////////////
//
// getStaffBase12 -- return 1 if less than middle C; otherwise return 2.
//

int getStaffBase12(int pitch) {
   if (pitch < 60) {
      return 1;
   } else {
      return 2;
   }
}


//////////////////////////////
//
// getMaxPitch -- return the highest non-zero content.
//

int getMaxPitch(Array<double>& midibins) {
   int i;
   for (i=midibins.getSize()-1; i>=0; i--) {
      if (midibins[i] != 0.0) {
         return i;
      }
   }
   return -1;
}



//////////////////////////////
//
// getMinPitch -- return the lowest non-zero content.
//

int getMinPitch(Array<double>& midibins) {
   int i;
   for (i=0; i<midibins.getSize(); i++) {
      if (midibins[i] != 0.0) {
         return i;
      }
   }
   return -1;
}



//////////////////////////////
//
// getVoice --
//

void getVoice(char* voicestring, HumdrumFile& infile, int kernspine) {
   int i,j;
   strcpy(voicestring, "");
   Array<char> tempstring;
   PerlRegularExpression pre;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isInterpretation()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (kernspine != infile[i].getPrimaryTrack(j)) {
            continue;
         }
         if (strncmp(infile[i][j], "*I\"", strlen("*I\"")) == 0) {
            strcpy(voicestring, &(infile[i][j][3]));
            tempstring.setSize(strlen(voicestring)+1);
            strcpy(tempstring.getBase(), voicestring);
            pre.sar(tempstring, "\\s+", " ", "g");
            strcpy(voicestring, tempstring.getBase());
         }
      }
   }


   // don't print editorial marks for voice names
   if (strchr(voicestring, '[') == NULL) {
      return;
   }

   Array<char> buffer;
   buffer.setSize(strlen(voicestring)+1);
   strcpy(buffer.getBase(), voicestring);
   pre.sar(buffer, "[\\[\\]]", "", "g");
   strcpy(voicestring, buffer.getBase());
}



//////////////////////////////
//
// getTitle --
//

void getTitle(char* titlestring, HumdrumFile& infile) {
   strcpy(titlestring, "_00");

   char buffer[1024] = {0};
   infile.getBibValue(buffer, "SCT");
   if (strlen(buffer) > 0) {
      strcat(titlestring, buffer);
      strcat(titlestring, " ");
      infile.getBibValue(buffer, "OPR");
      if (strlen(buffer) > 0) {
         strcat(titlestring, buffer);
         strcat(titlestring, " _02");
      }
      infile.getBibValue(buffer, "OTL");
      if (strlen(buffer) > 0) {
         strcat(titlestring, buffer);
      }
      
   }
}



//////////////////////////////
//
// clearHistograms --
//

void clearHistograms(Array<Array<double> >& bins, int start) {
   int i;
   for (i=start; i<bins.getSize(); i++) {
      bins[i].setSize(128);
      bins[i].setAll(0);
      bins[i].allowGrowth(0);
   }
   
}




//////////////////////////////
//
// printAnalysis --
//

void printAnalysis(Array<double>& midibins, HumdrumFile& infile) {
   if (percentileQ) {
      printPercentile(midibins, percentile);
      return;
   }  else if (rangeQ) {
      double notesinrange = countNotesInRange(midibins, rangeL, rangeH);
      cout << notesinrange << endl;
      return;
   }

   int i;
   double normval = 1.0;

   // print the pitch histogram

   double fracL = 0.0;
   double fracH = 0.0;
   double fracA = 0.0;
   double sum = midibins.sum();
   if (normQ) {
      normval = sum;
   }
   double runningtotal = 0.0;


   cout << "**keyno\t";
   if (pitchQ) {
      cout << "**pitch";
   } else {
      cout << "**kern";
   }
   cout << "\t**count";
   if (addfractionQ) {
      cout << "\t**fracL";
      cout << "\t**fracA";
      cout << "\t**fracH";
   }
   cout << "\n";


   int base12;
   char buffer[1024] = {0};
   for (i=0; i<midibins.getSize(); i++) {
      if (midibins[i] <= 0.0) {
         continue;
      }      
      if (diatonicQ) {
         base12 = Convert::base7ToBase12(i);
      } else {
         base12 = i;
      }
      cout << base12 << "\t";
      if (pitchQ) {
         cout << Convert::base12ToPitch(buffer, base12);
      } else {
         cout << Convert::base12ToKern(buffer, base12);
      }
      cout << "\t";
      cout << midibins[i] / normval;
      fracL = runningtotal/sum;
      runningtotal += midibins[i];
      fracH = runningtotal/sum;
      fracA = (fracH + fracL)/2.0;
      fracL = (int)(fracL * 10000.0 + 0.5)/10000.0;
      fracH = (int)(fracH * 10000.0 + 0.5)/10000.0;
      fracA = (int)(fracA * 10000.0 + 0.5)/10000.0;
      if (addfractionQ) {
         cout << "\t" << fracL;
         cout << "\t" << fracA;
         cout << "\t" << fracH;
      }
      cout << "\n";
   }
   cout << "*-\t*-\t*-";
   if (addfractionQ) {
      cout << "\t*-";
      cout << "\t*-";
      cout << "\t*-";
   }
   cout << "\n";

   cout << "!!tessitura:\t" << getTessitura(midibins) << " semitones\n";

   double mean = getMean(midibins);
   if (diatonicQ) {
      mean = Convert::base7ToBase12(mean);
   }
   cout << "!!mean:\t" << mean;
   cout << " (";
   cout << Convert::base12ToKern(buffer, int(mean+0.5));
   cout << ")" << "\n";

   int median = getMedian(midibins);
   if (diatonicQ) {
      median = Convert::base7ToBase12(median);
   }
   cout << "!!median:\t" << median;
   cout << " (";
   cout << Convert::base12ToKern(buffer, median);
   cout << ")" << "\n";


}


//////////////////////////////
//
// getMedian -- return the pitch on which half of pitches are above
//     and half are below.
//

int getMedian(Array<double>& midibins) {
   double sum = midibins.sum();

   double cumsum = 0.0;
   int i;
   for (i=0; i<midibins.getSize(); i++) {
      if (midibins[i] <= 0.0) {
         continue;
      }      
      cumsum += midibins[i]/sum;
      if (cumsum >= 0.50) {
         return i;
      }
   }

   return -1;
}


//////////////////////////////
//
// getMean -- return the interval between the highest and lowest
//     pitch in terms if semitones.
//

double getMean(Array<double>& midibins) {
   double top    = 0.0;
   double bottom = 0.0;

   int i;
   for (i=0; i<midibins.getSize(); i++) {
      if (midibins[i] <= 0.0) {
         continue;
      }      
      top += midibins[i] * i;
      bottom += midibins[i];
     
   }

   return top / bottom;
}



//////////////////////////////
//
// getTessitura -- return the interval between the highest and lowest
//     pitch in terms if semitones.
//

int getTessitura(Array<double>& midibins) {
   int minn = -1000;
   int maxx = -1000;
   int i;

   for (i=0; i<midibins.getSize(); i++) {
      if (midibins[i] <= 0.0) {
         continue;
      }      
      if (minn < 0) {
         minn = i;
      }
      if (maxx < 0) {
         maxx = i;
      }
      if (minn > i) {
         minn = i;
      }
      if (maxx < i) {
         maxx = i;
      }
   }
   if (diatonicQ) {
      maxx = Convert::base7ToBase12(maxx);
      minn = Convert::base7ToBase12(minn);
   }

   return maxx - minn;
}



//////////////////////////////
//
// countNotesInRange --
//

double countNotesInRange(Array<double>& midibins, int low, int high) {
   int i;
   double sum = 0;
   for (i=low; i<=high; i++) {
      sum += midibins[i];
   }
   return sum;
}



//////////////////////////////
//
// printPercentile --
//

void printPercentile(Array<double>& midibins, double percentile) {
   double sum = midibins.sum();
   double runningtotal = 0.0;
   int i;
   for (i=0; i<midibins.getSize(); i++) {
      if (midibins[i] <= 0) {
         continue;
      }
      runningtotal += midibins[i] / sum;
      if (runningtotal >= percentile) {
         cout << i << endl;
         return;
      }
   }

   cout << "unknown" << endl;
}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("c|range|count=s:60-71", "count notes in a particular range");
   opts.define("d|duration=b",      "weight pitches by duration");   
   opts.define("f|fraction=b",      "display histogram fractions");   
   opts.define("fill=b",            "change color of fill only");
   opts.define("p|percentile=d:0.0","display the xth percentile pitch");   
   opts.define("print=b",           "count printed notes rather than sounding");
   opts.define("pitch=b",           "display pitch info in **pitch format");
   opts.define("N|norm=b",          "normalize pitch counts");
   opts.define("score=b",           "convert range info to SCORE");
   opts.define("q|quartile=b",      "display quartile notes");
   opts.define("sx|scorexml|score-xml|ScoreXML|scoreXML=b", 
                                    "output ScoreXML format");
   opts.define("hover=b",           "include svg hover capabilities");
   opts.define("D|diatonic=b",      
         "diatonic counts ignore chormatic alteration");
   opts.define("no-define=b", "Do not use defines in output SCORE data");

   opts.define("debug=b",       "trace input parsing");   
   opts.define("author=b",      "author of the program");   
   opts.define("version=b",     "compilation information"); 
   opts.define("example=b",     "example usage"); 
   opts.define("h|help=b",      "short description"); 
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Mar 2005" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 31 Mar 2005" << endl;
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

   scoreQ       = opts.getBoolean("score");
   fillonlyQ    = opts.getBoolean("fill");
   xmlQ         = opts.getBoolean("score-xml");
   if (xmlQ) {
      scoreQ = 1;
   }
   quartileQ    = opts.getBoolean("quartile");
   debugQ       = opts.getBoolean("debug");
   normQ        = opts.getBoolean("norm");
   printQ       = opts.getBoolean("print");
   pitchQ       = opts.getBoolean("pitch");
   durationQ    = opts.getBoolean("duration");
   percentileQ  = opts.getBoolean("percentile");
   rangeQ       = opts.getBoolean("range");
   getRange(rangeL, rangeH, opts.getString("range"));
   addfractionQ = opts.getBoolean("fraction");
   percentile   = opts.getDouble("percentile");
   hoverQ       = opts.getBoolean("hover");
   diatonicQ    = opts.getBoolean("diatonic");

   // the percentile is a fraction from 0.0 to 1.0.
   // if the percentile is above 1.0, then it is assumed
   // to be a percentage, in which case the value will be
   // divided by 100 to get it in the range from 0 to 1.
   if (percentile > 1) {
      percentile = percentile / 100.0;
   }

}



//////////////////////////////
//
// getRange --
//

void getRange(int& rangeL, int& rangeH, const char* rangestring) {
   rangeL = -1; rangeH = -1;
   if (rangestring == NULL) {
      return;
   }
   if (rangestring[0] == '\0') {
      return;
   }
   int length = strlen(rangestring);
   char* buffer = new char[length+1];
   strcpy(buffer, rangestring);
   char* ptr;
   if (isdigit(buffer[0])) {
      ptr = strtok(buffer, " \t\n:-");
      sscanf(ptr, "%d", &rangeL);
      ptr = strtok(NULL, " \t\n:-");
      if (ptr != NULL) {
         sscanf(ptr, "%d", &rangeH);
      }
   } else {
      ptr = strtok(buffer, " :");
      if (ptr != NULL) {
         rangeL = Convert::kernToMidiNoteNumber(ptr);
         ptr = strtok(NULL, " :");
         if (ptr != NULL) {
            rangeH = Convert::kernToMidiNoteNumber(ptr);
         }
      }
   }

   if (rangeH < 0) {
      rangeH = rangeL;
   }

   if (rangeL <   0) { rangeL =   0; }
   if (rangeH <   0) { rangeH =   0; }
   if (rangeL > 127) { rangeL = 127; }
   if (rangeH > 127) { rangeH = 127; }
   if (rangeL > rangeH) {
      int temp = rangeL;
      rangeL = rangeH;
      rangeH = temp;
   }

}



//////////////////////////////
//
// example -- example usage of the maxent program
//

void example(void) {
   cout <<
   "                                                                        \n"
   << endl;
}




//////////////////////////////
//
// usage -- gives the usage statement for the quality program
//

void usage(const char* command) {
   cout <<
   "                                                                        \n"
   << endl;
}



//////////////////////////////
//
// generateAnalysis --
//

void generateAnalysis(HumdrumFile& infile, Array<Array<double> >& midibins,
      Array<int>& kernspines) {
   int i, j, k;

   char buffer[1024] = {0};
   int tokencount;
   int keynum;
   double duration;

   int vindex;

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_data) {
         continue;
      } 
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp(infile[i].getExInterp(j), "**kern") != 0) {
            continue;
         }
         vindex = getVindex(infile[i].getPrimaryTrack(j), kernspines);
         if (strcmp(infile[i][j], ".") == 0) {  // ignore null tokens
            continue;
         }
         tokencount = infile[i].getTokenCount(j);
         for (k=0; k<tokencount; k++) {
            infile[i].getToken(buffer, j, k);         
            if (strcmp(buffer, ".") == 0) {    // ignore strange null tokens
               continue;
            }
            if (strchr(buffer, 'r') != NULL) { // ignore rests
               continue;
            }
            if (!printQ && !durationQ) {
               // filter out middle and ending tie notes if counting
               // by sounding pitch
               if (strchr(buffer, '_') != NULL) {
                  continue;
               }
               if (strchr(buffer, ']') != NULL) {
                  continue;
               }
            }
            if (!diatonicQ) {
               keynum = Convert::kernToMidiNoteNumber(buffer);
            } else {
               keynum = Convert::kernToDiatonicPitch(buffer);
            }
            if (keynum > 127) {
               cout << "ERROR: Funny pitch: " << keynum 
                    << " = " << buffer << endl;
            } else if (durationQ) {
               duration = Convert::kernToDuration(buffer);
               midibins[0][keynum] += duration;
               midibins[vindex+1][keynum] += duration;
            } else {
               midibins[0][keynum] += 1.0;
               midibins[vindex+1][keynum] += 1.0;
            }
         }
      }
   }
}



//////////////////////////////
//
// getVindex -- return the voice index of the primary track.
//

int getVindex(int track, Array<int>& kernspines) {
   int i;
   for (i=0; i<kernspines.getSize(); i++) {
      if (kernspines[i] == track) {
         return i;
      }
   }
   return -1;
}




// md5sum: 5878776ce2985003a8d9cc3bb76103b4 prange.cpp [20121112]
