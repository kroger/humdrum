//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Apr 11 11:43:12 PDT 2002
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 renamed SigCollection class
// Last Modified: Mon Feb 21 08:29:14 PST 2011 added --match
// Last Modified: Sun Feb 27 15:09:29 PST 2011 added fixed vocal colors
// Filename:      ...sig/examples/all/proll.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/proll.cpp
// Syntax:        C++; museinfo
//
// Description:   Generate piano roll plots.
//

#include "humdrum.h"
#include "PerlRegularExpression.h"

#include <string.h>
#include <ctype.h>

typedef SigCollection<PixelColor> PixelRow;


// function declarations
void   checkOptions             (Options& opts, int argc, char* argv[]);
void   example                  (void);
void   usage                    (const char* command);
void   generateBackground       (HumdrumFile& infile, int rfactor, 
                                 Array<PixelRow>& picturedata, 
                                 Array<PixelRow>& background);
int    generatePicture          (HumdrumFile& infile, Array<PixelRow>& picture,
                                 int style);
void   printPicture             (Array<PixelRow>& picturedata, 
                                 Array<PixelRow>& background, int rfactor, 
                                 int cfactor, int minp, int maxp, 
                                 HumdrumFile& infile);
void   placeNote                (Array<PixelRow>& picture, int pitch, 
                                 double start, double duration, int min, 
                                 PixelColor& color, double factor, int match);
PixelColor makeColor            (HumdrumFile& infile, int line, int spine, 
                                 int style, Array<int>& rhylev, int track);
void   getMarkChars             (Array<char>& marks, HumdrumFile& infile);
int    isMatch                  (Array<char>& marks, const char* buffer);
const char* getInstrument       (HumdrumFile& infile, int spine);

// global variables
Options   options;                   // database for command-line arguments
int       debugQ    = 0;             // used with --debug option
int       markQ     = 0;             // used with --mark option
int       maxwidth  = 3000;          // used with -w option
int       maxheight = 400;           // used with -h option
int       rfactor   = 1;
int       cfactor   = 1;
int       gminpitch = 0;
int       gmaxpitch = 127;
int       maxfactor = 5;
int       measureQ  = 1;              // used with the -M option
int       keyboardQ = 1;              // used with the -K option
int       style     = 'H';            // used with the -s option
const char* keyboardcolor = "151515"; // used with the -k option

///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   checkOptions(options, argc, argv);

   Array<PixelRow> picturedata;
   Array<PixelRow> background;
   HumdrumFile infile;

   int numinputs = options.getArgCount();
   if (numinputs > 0) {
      char* filenameIn  = options.getArg(1);
      infile.read(filenameIn);
   } else {
      infile.read(cin);
   }

   int rfactor = generatePicture(infile, picturedata, style);
   generateBackground(infile, rfactor, picturedata, background);
   printPicture(picturedata, background, rfactor, cfactor, 
         gminpitch, gmaxpitch, infile);

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// printPicture --
//

void printPicture(Array<PixelRow>& picturedata, Array<PixelRow>& background,
      int rfactor, int cfactor, int minp, int maxp, HumdrumFile& infile) {


   if (minp > 0) {
      minp--;
   }
   if (maxp < 127) {
      maxp++;
   }
   int i, j;
   int m;
   int width = picturedata[0].getSize();
   int height = (maxp - minp + 1);
   cfactor = (int)(maxheight / height);
   if (cfactor <= 0) {
      cfactor = 1;
   }
   if (cfactor > maxfactor) {
      cfactor = maxfactor;
   }
   PixelColor temp;
   PixelColor black(0,0,0);
   height = cfactor * height;
   cout << "P6\n" << width << " " << height << "\n255\n";
   for (i=maxp; i>=minp; i--) {
      for (m=0; m<cfactor; m++) {
         for (j=0; j<picturedata[i].getSize(); j++) {
            if (picturedata[i][j] == black) {
               background[i][j].writePpm6(cout);
            } else {
               if ((i > 0) && (cfactor > 1) && (m == cfactor-1) && 
                     (picturedata[i-1][j] == picturedata[i][j])) {
                  temp = picturedata[i][j] * 0.667;
                  temp.writePpm6(cout);
               } else {
                  picturedata[i][j].writePpm6(cout);
               }
            }
         }
      }
   }
}



//////////////////////////////
//
// generatePicture -- create the picture.  Returns the number of
//    pixel repetitions for each row's pixel.
//

int generatePicture(HumdrumFile& infile, Array<PixelRow>& picture, int
      style) {

   Array<char> marks;
   getMarkChars(marks, infile);
   PixelColor matchcolor(255,255,255);

   infile.analyzeRhythm("4");
   int min = infile.getMinTimeBase();
   double totaldur = infile.getTotalDuration();
   
   int columns = (int)(totaldur * min / 4.0 + 0.5) + 5;
   if (columns > 50000) {
      cout << "Error: picture will be too big to generate" << endl;
      exit(1);
   }
   int factor = (int)(maxwidth / columns);
   if (factor <= 0) {
      factor = 1;
   }
   if (factor > maxfactor) {
      factor = maxfactor;
   }

   // set picture to black first.  Black regions will be filled in
   // with the background later.
   picture.setSize(128);
   int i, j, k;
   for (i=0; i<picture.getSize(); i++) {
      picture[i].setSize(columns * factor);
      for (j=0; j<picture[i].getSize(); j++) {
         picture[i][j].setRed(0);
         picture[i][j].setGreen(0);
         picture[i][j].setBlue(0);
      }
   }

   // examine metric levels for metric coloration
   Array<int>rhylev;
   infile.analyzeMetricLevel(rhylev);
   for (i=0; i<rhylev.getSize(); i++) {
      // reverse sign so that long notes are positive.
      rhylev[i] = -rhylev[i];
   }
   
   PixelColor color;
   int minpitch = 128;
   int maxpitch = -1;
   int pitch = 0;
   double duration = 0;
   double start = 0;
   char buffer[1024] = {0};
   for (i=0; i<infile.getNumLines(); i++) {
      if (debugQ) {
         cout << "Processing input line " << i + 1 << '\t' << infile[i] << endl;
      }
      if (infile[i].isData()) {
         start = infile[i].getAbsBeat();
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (strcmp(infile[i].getExInterp(j), "**kern") != 0) {
               continue;
            }
            // duration = Convert::kernToDuration(infile[i][j]);
            duration = infile.getTiedDuration(i, j);
            color = makeColor(infile, i, j, style, rhylev, 
                  infile[i].getPrimaryTrack(j));
            for (k=0; k<infile[i].getTokenCount(j); k++) {
               infile[i].getToken(buffer, j, k);
               if (strchr(buffer, '_') != NULL) {
                  continue;
               }
               if (strchr(buffer, ']') != NULL) {
                  continue;
               }

               pitch = Convert::kernToMidiNoteNumber(buffer);
               if (pitch < 0) {
                  // ignore rests
                  continue;
               }
               if (pitch < minpitch) {
                  minpitch = pitch;
               }
               if (pitch > maxpitch) {
                  maxpitch = pitch;
               }
               if (isMatch(marks, buffer)) {
                  placeNote(picture, pitch, start, duration, min, 
                        color, factor, 1);
               } else {
                  placeNote(picture, pitch, start, duration, min, 
                        color, factor, 0);
               }
            }
         }
      }

   }

   gmaxpitch = maxpitch;
   gminpitch = minpitch;
   return factor;
}



//////////////////////////////
//
// isMatch -- returns true if the string has a match character in it
//

int isMatch(Array<char>& marks, const char* buffer) {
   int i;
   for (i=0; i<marks.getSize(); i++) {
      if (strchr(buffer, marks[i]) != NULL) {
         return 1;
      }
   }
   return 0;
}



//////////////////////////////
//
// makeColor --
//

PixelColor makeColor(HumdrumFile& infile, int line, int spine, int style,
      Array<int>& rhylev, int track) {
   PixelColor output;
   int trackCount;
   PerlRegularExpression pre;
   const char* instrument = "";

   PixelColor purple     (225, 121, 255);
   PixelColor yellowgreen(150, 200,   0);

   switch (toupper(style)) {
      case 'M':    // color by metric position
         if (rhylev[line] >= 2) {
            output.setColor("red");
         } else if (rhylev[line] == 1) {
            output.setColor("lightorange");
         } else if (rhylev[line] == 0) {
            output.setColor("yellow");
         } else if (rhylev[line] == -1) {
            output.setColor("green");
         } else if (rhylev[line] == -2) {
            output.setColor("blue");
         } else if (rhylev[line] <= -3) {
            output.setColor("violet");
         } else {
            output.setColor("silver");
         }
         break;

      case 'V':    // color spines by voice
         instrument = getInstrument(infile, track);
         if (pre.search(instrument, "Bassus", "i")) {
            output.setColor("red");
         } else if (pre.search(instrument, "Contra", "i")) {
            output.setColor("darkorange");
         } else if (pre.search(instrument, "Tenor", "i")) {
            output.setColor("blue");
         } else if (pre.search(instrument, "Altus", "i")) {
            output = purple;
         } else if (pre.search(instrument, "Superius", "i")) {
            output.setColor("limegreen");
         } else if (pre.search(instrument, "Cantus", "i")) {
            output.setColor("limegreen");
         } else if (pre.search(instrument, "Discantus", "i")) {
            output = yellowgreen;
         } else {
            output.setColor("black");
         }
         break;
         
      case 'H':    // color spines by hue
      default:
         trackCount = infile.getMaxTracks();
         output.setHue(((int)infile[line].getTrack(spine))/(double)trackCount);
   }

   return output;
}



//////////////////////////////
//
// getInstrument --
//

const char* getInstrument(HumdrumFile& infile, int track) {
   int i, j;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isInterpretation()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (track != infile[i].getPrimaryTrack(j)) {
            continue;
         }
         if (strncmp(infile[i][j], "*I\"", 3) == 0) {
            return &infile[i][j][3];
         }
      }
   }
   return "";
}



//////////////////////////////
//
// placeNote -- draw a note in the picture area
//

void placeNote(Array<PixelRow>& picture, int pitch, double start, 
      double duration, int min, PixelColor& color, double factor, int match) {
   int startindex = (int)(start * min / 4.0 * factor);
   int endindex = (int)((start + duration) * min / 4.0 * factor) - 1;

   PixelColor zcolor = color;
   if (match) {
      zcolor.Red   = (zcolor.Red   + 4*255)/5;
      zcolor.Green = (zcolor.Green + 4*255)/5;
      zcolor.Blue  = (zcolor.Blue  + 4*255)/5;
   }
   PixelColor black(0,0,0);
   if (startindex-1 >= 0) {
      if (picture[pitch][startindex-1] == color) {
         picture[pitch][startindex-1] *= 0.667;
      }
   }
   for (int i=startindex; i<=endindex; i++) {
      if (picture[pitch][i] == black) {
         picture[pitch][i] = zcolor;
      } else {
         if (match) {
            picture[pitch][i].Red   = (2*zcolor.Red 
                  + picture[pitch][i].Red)/3;
            picture[pitch][i].Green = (2*zcolor.Green 
                  + picture[pitch][i].Green)/3;
            picture[pitch][i].Blue  = (2*zcolor.Blue 
                  + picture[pitch][i].Blue)/3;
         } else {
            picture[pitch][i].Red   = (color.Red 
                  + picture[pitch][i].Red)/2;
            picture[pitch][i].Green = (color.Green 
                  + picture[pitch][i].Green)/2;
            picture[pitch][i].Blue  = (color.Blue 
                  + picture[pitch][i].Blue)/2;
         }
      }
   }
}



//////////////////////////////
//
// generateBackground -- create the picture.
//

void generateBackground(HumdrumFile& infile, int rfactor, 
      Array<PixelRow>& picturedata, Array<PixelRow>& background) {

   background.setSize(picturedata.getSize());
   int i, j;

   for (i=0; i<picturedata.getSize(); i++) {
      background[i].setSize(picturedata[i].getSize());
      for (j=0; j<picturedata[i].getSize(); j++) {
         background[i][j] = 0;
      }
   }

   PixelColor whitekeys(keyboardcolor);
   if (keyboardQ) {
      for (i=0; i<background.getSize(); i++) {
         switch (i % 12) {
            case 0: case 2: case 4: case 5: case 7: case 9: case 11:
               for (j=0; j<background[i].getSize(); j++) {
                  background[i][j] = whitekeys;
               }
               break;
         }
      }
   }


   int index;
   int min = infile.getMinTimeBase();
   PixelColor measureColor;
   measureColor.setColor(25, 25, 25);
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isMeasure()) {
         index = (int)(infile[i].getAbsBeat() * min / 4.0 * rfactor);
         for (j=0; j<background.getSize(); j++) {
            background[j][index] = measureColor;
         }
      }
   }

}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("w|width=i:3000",      "maximum width allowable for image");   
   opts.define("h|height=i:400",      "maximum height allowable for image");   
   opts.define("M|no-measure=b",      "do not display measure lines on image");
   opts.define("K|no-keyboard=b",     "do not display keyboard in background");
   opts.define("k|keyboard=s:151515", "keyboard white keys color");
   opts.define("s|style=s:H",         "Coloring style");
   opts.define("mark=b",              "Highlight marked/matched notes");

   opts.define("debug=b",          "trace input parsing");   
   opts.define("author=b",         "author of the program");   
   opts.define("version=b",        "compilation information"); 
   opts.define("example=b",        "example usage"); 
   opts.define("help=b",           "short description"); 
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, April 2002" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 8 April 2002" << endl;
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


   debugQ    =  opts.getBoolean("debug");
   markQ     =  opts.getBoolean("mark");
   maxwidth  =  opts.getInteger("width");
   maxheight =  opts.getInteger("height");
   measureQ  = !opts.getBoolean("no-measure");
   keyboardQ = !opts.getBoolean("no-keyboard");
   keyboardcolor = opts.getString("keyboard");
   style     = opts.getString("style")[0];
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
// getMarkChars --
//

void getMarkChars(Array<char>& marks, HumdrumFile& infile) {
   PerlRegularExpression pre;
   Array<char>& colorchar = marks;

   colorchar.setSize(0);
   char value;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      // !!!RDF**kern: N= mark color="#ff0000", root
      if (pre.search(infile[i].getLine(), 
            "^!!!RDF\\*\\*kern:\\s*([^\\s])\\s*=\\s*match", "i") ||
          pre.search(infile[i].getLine(), 
            "^!!!RDF\\*\\*kern:\\s*([^\\s])\\s*=\\s*mark", "i")
         ) {
         value = pre.getSubmatch(1)[0];
         colorchar.append(value);
      }
   }
}



// md5sum: 3d3af5f512f6fef7c9c890dd52011719 proll.cpp [20120805]
