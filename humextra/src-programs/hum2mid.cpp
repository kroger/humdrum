//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 24 10:39:47 PDT 1999
// Last Modified: Sat Nov 25 20:17:58 PST 2000 added instrument selection
// Last Modified: Tue Feb 20 19:20:09 PST 2001 add articulation interpretation
// Last Modified: Sat Aug 23 08:57:54 PDT 2003 added *free/*strict control
// Last Modified: Wed Mar 24 19:56:04 PST 2004 fixed initial *MM ignore
// Last Modified: Wed Apr  7 16:22:38 PDT 2004 grace-note noteoff hack
// Last Modified: Tue Sep  7 03:31:49 PDT 2004 added extra space at end
// Last Modified: Tue Sep  7 03:43:09 PDT 2004 more grace-note noteoff fixing
// Last Modified: Tue Sep  7 20:58:38 PDT 2004 initial dynamics processing
// Last Modified: Fri Sep 10 02:26:59 PDT 2004 added padding option
// Last Modified: Fri Sep 10 19:46:53 PDT 2004 added cresc. decresc. control
// Last Modified: Sun Sep 12 05:20:44 PDT 2004 added human and metric volume
// Last Modified: Wed Mar 23 00:35:18 PST 2005 added constant volume back
// Last Modified: Sat Dec 17 22:46:11 PST 2005 added **time processing
// Last Modified: Sat Jun  3 10:35:29 PST 2005 added **tempo processing
// Last Modified: Sun Jun  4 19:32:22 PDT 2006 added PerfViz match files
// Last Modified: Tue Sep 12 19:36:08 PDT 2006 added **idyn processing
// Last Modified: Sun Oct  1 21:04:35 PDT 2006 continued work in **idyn
// Last Modified: Thu May  3 22:55:15 PDT 2007 added *pan controls
// Last Modified: Thu Oct 30 12:42:35 PST 2008 added --no-rest option
// Last Modified: Thu Nov 20 08:19:40 PST 2008 added **Dcent interpretation
// Last Modified: Sun Nov 23 22:49:15 PST 2008 added --temperament option
// Last Modified: Tue May 12 12:14:09 PDT 2009 added rhythmic scaling factor
// Last Modified: Tue Feb 22 13:23:24 PST 2011 added --stdout
// Last Modified: Fri Feb 25 13:00:02 PST 2011 added --met
// Last Modified: Fri Feb 25 15:15:09 PST 2011 added --timbres and --autopan
// Last Modified: Wed Oct 12 16:23:02 PDT 2011 fixed --temperament pc 0 prob.
// Last Modified: Fri Aug  3 16:09:29 PDT 2012 added DEFAULT for --timbres
// Last Modified: Tue Oct 16 21:02:56 PDT 2012 added getTitle/song title
// Filename:      ...sig/examples/all/hum2mid.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/hum2mid.cpp
// Syntax:        C++; museinfo
//
// Description:   Converts Humdrum **kern data into MIDI data in a
//                Standard MIDI File format.
//
// Todo:
// 	* Check to make sure input files are not the same as the -o filename
// 	* Allow multiple input / outputs
// 	* Allow multiple inputs one output (already done?)
//

#include "museinfo.h"
#include "PerlRegularExpression.h"
#include "SigString.h"

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

// stringstream, or strstream in older CPP is needed only for PerfViz files:
#ifndef OLDCPP
   #include <fstream>
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #include <fstream.h>
   #ifdef VISUAL
      #include <strstrea.h>    /* for Windows 95 */
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif

#define TICKSPERQUARTERNOTE 120

//  Dynamics to attack velocity conversions:

// Set based on conversions to MIDI files made in Finale:
// (Every 13 increments)
#define DYN_FFFF 127
#define DYN_FFF  114
#define DYN_FF   101
#define DYN_F    88
#define DYN_MF   75
#define DYN_MP   62
#define DYN_P    49
#define DYN_PP   36
#define DYN_PPP  23
#define DYN_PPPP 10


// PerfViz data structure

class PerfVizNote {
   public:   
      int pitch;           // pitch name of note (in base40 notation)
      int beat;            // beat number in measure of note
      int vel;             // MIDI attack velocity of note
      int ontick;          // tick time for note on
      int offtick;         // tick time for note off
      double scoredur;     // score duration of notes in beats
      double absbeat;      // absolute beat position of note in score

      double beatfrac;     // starting fraction of beat
      double beatdur;      // duration of note in beats
    
      static int sid;      // serial number id
      static int bar;      // current measure number
      static int key;      // used to print key signatures
      static int tstop;    // time signature top
      static int tsbottom; // time signature bottom
      static double approxtempo;  // the approximate tempo marking

};

int PerfVizNote::sid         =  0;
int PerfVizNote::bar         =  0;
int PerfVizNote::key         = -1;
int PerfVizNote::tstop       =  0;
int PerfVizNote::tsbottom    =  0;
double PerfVizNote::approxtempo =  0;


// global variables:
const char *outlocation = NULL;
int   trackcount  = 0;           // number of tracks in MIDI file
int   track       = 0;           // track number starting at 0
int   offset      = 0;           // start-time offset in ticks
int   tempo       = 60;          // default tempo

// user interface variables
Options options;
int     storeCommentQ    =   1;    // used with -C option
int     storeTextQ       =   1;    // used with -T option
int     plusQ            =   0;    // used with --plus option
int     defaultvolume    =  64;    // used with -v option
double  tscaling         =   1.0;  // used with -s option
int     multitimbreQ     =   1;    // used with -c option
int     instrumentQ      =   1;    // used with -I option
int     fixedChannel     =   0;    // used with -c option
int     instrumentnumber =  -1;    // used with -f option
int     forcedQ          =   0;    // used with -f option
int     mine             =   0;    // used with -m option
int     shortenQ         =   0;    // used with -s option
int     shortenamount    =   0;    // used with -s option
int     plainQ           =   0;    // used with -p option
int     debugQ           =   0;    // used with --debug option
int     dynamicsQ        =   1;    // used with -D option
int     padQ             =   1;    // used with -P option
int     humanvolumeQ     =   5;    // used with --hv option
int     metricvolumeQ    =   5;    // used with --mv option
int     sforzando        =  20;    // used with --sf option
int     fixedvolumeQ     =   0;    // used with -v option
int     timeQ            =   0;    // used with --time option
int     autopanQ         =   0;    // used with --autopan option
int     tempospineQ      =   0;    // used with --ts option
int     perfvizQ         =   0;    // used with --pvm option
int     idynQ            =   0;    // used with -d option
double  idynoffset       =   0.0;  // used with -d option
int     timeinsecQ       =   0;    // used with --time-in-seconds option
int     norestQ          =   0;    // used with --no-rest option
int     stdoutQ          =   0;    // used with --stdout option
int     starttick        =   0;    // used with --no-rest option
int     bendQ            =   0;    // used with --bend option
int     metQ             =   0;    // used with --met option
int     timbresQ         =   0;    // used with --timbres option
Array<SigString> TimbreName;       // used with --timbres option
Array<int> TimbreValue;            // used with --timbres option
Array<int> TimbreVolume;           // used with --timbres option
Array<int> VolumeMapping;          // used with --timbres option
double  bendamt          = 200;    // used with --bend option
int     bendpcQ          =   0;    // used with --temperament
double  bendbypc[12]     = {0};    // used with --temperament
double  rhysc            = 1.0;    // used with -r option
    // for example, use -r 4.0 to make written sixteenth notes
    // appear as if they were quarter notes in the MIDI file.

SSTREAM *PVIZ = NULL;    // for storing individual notes in PerfViz data file.
double   tickfactor = 960.0 / 1000.0;

// function declarations:
void      assignTracks      (HumdrumFile& infile, Array<int>& trackchannel);
double    checkForTempo     (HumdrumRecord& record);
void      checkOptions      (Options& opts, int argc, char** argv);
void      example           (void);
int       makeVLV           (uchar *buffer, int number);
void      reviseInstrumentMidiNumbers(const char* string);
int       setMidiPlusVolume (const char* kernnote);
void      storeMetaText     (MidiFile& mfile, int track, const char* string, 
                               int tick, int metaType = 1);
void      storeMidiData     (HumdrumFile& infile, MidiFile& outfile);
void      storeInstrument   (int ontick, MidiFile& mfile, HumdrumFile& infile, 
                             int line, int row, int pcQ);
void      usage             (const char* command);
void      storeFreeNote     (Array<Array<int> >& array,int ptrack,int midinote);
void      getDynamics       (HumdrumFile& infile, Array<Array<char> >& dynamics,
                             int defaultdynamic);
void      getDynamicAssignments(HumdrumFile& infile, Array<int>& assignments);
void      getStaffValues    (HumdrumFile& infile, int staffline, 
                             Array<Array<int> >& staffvalues);
void      getNewDynamics    (Array<int>& currentdynamic, 
                             Array<int>& assignments, 
                             HumdrumFile& infile, int line, 
                             Array<Array<char> >& crescendos,
                             Array<Array<char> >& accentuation);
void      processCrescDecresc(HumdrumFile& infile, 
                              Array<Array<char> >& dynamics, 
                              Array<Array<char> >& crescendos);
void      interpolateDynamics(HumdrumFile& infile, Array<char>& dyn, 
                             Array<char>& cresc);
void      generateInterpolation(HumdrumFile& infile, Array<char>& dyn, 
                             Array<char>& cresc, int startline, int stopline, 
                             int direction);
int       findtermination    (Array<char>& dyn, Array<char>& cresc, int start);
char      adjustVolumeHuman  (int startvol, int delta);
char      adjustVolumeMetric (int startvol, int delta, double metricpos);
char      applyAccentuation  (int dynamic, int accent);
int       getMillisecondTime (HumdrumFile& infile, int line);
int       getFileDurationInMilliseconds(HumdrumFile& infile);
int       getMillisecondDuration(HumdrumFile& infile, int row, int col, 
                             int subcol);
void      addTempoTrack      (HumdrumFile& infile, MidiFile& outfile);
void      getBendByPcData    (double* bendbypc, const char* filename);
void      insertBendData     (MidiFile& outfile, double* bendbypc);
void      getKernTracks      (Array<int>& tracks, HumdrumFile& infile);
void      getTitle           (Array<char>& title, HumdrumFile& infile);

// PerfViz related functions:
void      writePerfVizMatchFile(const char* filename, SSTREAM& contents);
ostream& operator<<            (ostream& out, PerfVizNote& note);
void     printPerfVizKey       (int key);
void     printPerfVizTimeSig   (int tstop, int tsbottom);
void     printPerfVizTempo     (double approxtempo);
void     printRational          (ostream& out, double value);
void     storePan              (int ontime, MidiFile& outfile, 
                                HumdrumFile& infile, int row, int column);
void     adjustEventTimes      (MidiFile& outfile, int starttick);
void     checkForBend          (MidiFile& outfile, int notetick, int channel, 
                                HumdrumFile& infile, int row, int col, 
                                double scalefactor);
void     storeTimbres          (Array<SigString>& name, Array<int>& value, 
                                Array<int>& volumes, const char* string);
void     autoPan               (MidiFile& outfile, HumdrumFile& infile);

Array<int> tracknamed;      // for storing boolean if track is named
Array<int> trackchannel;    // channel of each track

#define TICKSPERQUARTERNOTE 120
int tpq = TICKSPERQUARTERNOTE;

//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   VolumeMapping.setSize(0);

   // for humanizing processes 
   #ifndef VISUAL
      srand48(time(NULL));
   # else 
      srand(time(NULL));
   #endif

   SSTREAM *perfviz = NULL;

   HumdrumFile infile;
   MidiFile    outfile;

   // process the command-line options
   checkOptions(options, argc, argv);

   if (perfvizQ) {
      tpq   = 480;
      tempo = 120;
   }
   outfile.setTicksPerQuarterNote(tpq);

   if (timeQ) {
      outfile.setMillisecondDelta();
   }

   // figure out the number of input files to process
   // only the first argument will be processed.  If there are
   // no arguments, then standard input will be used.
   int numinputs = options.getArgCount();

   for (int i=0; i<1 || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }

      // analyze the input file according to command-line options
      infile.analyzeRhythm("4", debugQ);

      if (perfvizQ) {
         perfviz = new SSTREAM[1];
	 const char *filename = NULL;
	 PVIZ = perfviz;
         perfviz[0] << "info(matchFileVersion,2.0).\n";
         if (numinputs < 1) {
            perfviz[0] << "info(scoreFileName,'STDIN').\n";
         } else {
            perfviz[0] << "info(scoreFileName,'";
	    filename = strrchr(options.getArg(i+1), '/');
            if (filename == NULL) {
               filename = options.getArg(i+1);
            } else {
               filename = filename + 1;
            }
            perfviz[0] << filename;
            perfviz[0] << "').\n";
         }
         if (options.getBoolean("output")) {
            perfviz[0] << "info(midiFileName,'";
	    filename = strrchr(options.getString("output"), '/');
            if (filename == NULL) {
               filename = options.getString("output");
            } else {
               filename = filename + 1;
            }
            perfviz[0] << filename;
            perfviz[0] << "').\n";
         } else {
            perfviz[0] << "info(midifileName,'STDOUT').\n";
         }
         perfviz[0] << "info(midiClockUnits,";
         perfviz[0] << tpq << ").\n";
         perfviz[0] << "info(midiClockRate,500000).\n";
      }

      trackcount = infile.getMaxTracks();
      tracknamed.setSize(trackcount + 1);
      trackchannel.setSize(trackcount + 1);
      for (int j=0; j<trackcount+1; j++) {
         tracknamed[j]   = 0;
         trackchannel[j] = fixedChannel;
      }
      if (multitimbreQ) {
         assignTracks(infile, trackchannel);
      }
      outfile.addTrack(trackcount);
      outfile.absoluteTime();

      /*
      // removed this code because of all of the lousy free MIDI 
      // sequencing programs that choke on system exclusive messages.

      // store the "General MIDI activation" system exclusive:
      // don't bother if this is just a Type 0 MIDI file
      if (!options.getBoolean("type0")) {
         Array<uchar> gmsysex;
         gmsysex.setSize(6);
         gmsysex[0] = 0xf0;     // Start of SysEx
         gmsysex[1] = 0x7e;     // Universal (reserved) ID number 
         gmsysex[2] = 0x7f;     // Device ID (general transmission)
         gmsysex[3] = 0x09;     // Means 'This is a message about General MIDI' 
         gmsysex[4] = 0x01;     // Means 'Turn General MIDI On'. 02 means 'Off' 
         gmsysex[5] = 0xf7;     // End of SysEx
         outfile.addEvent(0, 0, gmsysex);
      }
      */ 

      if (bendpcQ) {
         insertBendData(outfile, bendbypc);
      }
      storeMidiData(infile, outfile);

      if (tempospineQ) {
         addTempoTrack(infile, outfile);
      }

      outfile.sortTracks();
      if (norestQ) {
         adjustEventTimes(outfile, starttick);
      }
      if (stdoutQ) {
         outfile.write(cout);
      } else if (outlocation == NULL) {
         cout << outfile;
      } else {
         outfile.write(outlocation);
      }

      if (perfvizQ) {
         // currently you cannot create multiple PerfViz files from
         // multiple inputs.
         writePerfVizMatchFile(options.getString("perfviz"), perfviz[0]);
         delete [] perfviz;
      }
   }

   return 0;
}


//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// insertBendData -- store pitch bend data, one channel per
//      pitch class.  Channel 10 is skipped because it is used
//      as a drum channel on General MIDI synthesizers.
//

void insertBendData(MidiFile& outfile, double* bendbypc) {
   int i;
   int channel;
   int track = 0;
   double bendvalue;
   if (outfile.getNumTracks() > 1) {
      // don't store in first track if a multi-track file.
      track = 1;
   }
   Array<uchar> mididata;
   mididata.setSize(2);
   if (instrumentnumber < 0) {
      mididata[1] = 0;
   } else {
      mididata[1] = instrumentnumber;
   }
   for (i=0; i<12; i++) {
      channel = i;
      if (channel >= 9) {
         channel++;
      }
      bendvalue = bendbypc[i];
      outfile.addPitchBend(track, offset, channel, bendvalue);
      if (forcedQ) {
         mididata[0] = 0xc0 | (0x0f & channel);
         outfile.addEvent(track, offset, mididata);
      }
   }
}



//////////////////////////////
//
// adjustEventTimes -- set the first event time to starttick and
//    also subtrace that value from all events in the MIDI file;
//

void adjustEventTimes(MidiFile& outfile, int starttick) {
   int i, j;
   MFEvent* eventptr;
   int atime;
   int minval = 1000000000;
   for (i=0; i<outfile.getTrackCount(); i++) {
      for (j=0; j<outfile.getNumEvents(i); j++) {
	 eventptr = &outfile.getEvent(i, j);
         if (eventptr->data.getSize() <= 0) {
            continue;
         }
         if ((eventptr->data[0] & 0xf0) == 0x90) {
            if (eventptr->time < minval) {
               minval = eventptr->time;
            }
            break;
         } 
      }
   }

   if (minval > 1000000) {
     // minimum time is ridiculously large, so don't do anything
     return;
   }

   for (i=0; i<outfile.getTrackCount(); i++) {
      for (j=0; j<outfile.getNumEvents(i); j++) {
         eventptr = &outfile.getEvent(i, j);
         atime = eventptr->time;
         atime = atime - minval;
         if (atime < 0) {
            atime = 0;
         }
         eventptr->time = atime;
      }
   }
}



//////////////////////////////
//
// addTempoTrack -- 
//

void addTempoTrack(HumdrumFile& infile, MidiFile& outfile) {
   int i, j;
   double tempo;
   double absbeat;
   int ontick;
   int ttempo;
   Array<uchar> mididata;

   // should erase previous contents of tempo track first...

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_data) {
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (strcmp(infile[i].getExInterp(j), "**tempo") == 0) {
               if (isdigit(infile[i][j][0])) {
                  tempo = strtod(infile[i][j], NULL);
                  //cout << "The tempo value read was " << tempo << endl;
                  tempo = tempo * tscaling;
                  mididata.setSize(6);
                  mididata[0] = 0xff;
                  mididata[1] = 0x51;
                  mididata[2] = 3;
                  ttempo = (int)(60000000.0 / tempo + 0.5);   
                  mididata[3] = (uchar)((ttempo >> 16) & 0xff);
                  mididata[4] = (uchar)((ttempo >> 8) & 0xff);
                  mididata[5] = (uchar)(ttempo & 0xff);
                  absbeat = infile.getAbsBeat(i);
                  ontick = int(absbeat * outfile.getTicksPerQuarterNote());
                  outfile.addEvent(0, ontick + offset, mididata);
               }
            }
         }
      }
   }

}



//////////////////////////////
//
// assignTracks -- give a track number for each **kern spine in the input file
//    trying to place the same instruments into the same channels if
//    there are not enough channels to go around.
//

void assignTracks(HumdrumFile& infile, Array<int>& trackchannel) {
   int i, j;


   Array<int> instruments;        // MIDI instruments indicated in track
   trackchannel.setSize(infile.getMaxTracks() + 1);
   instruments.setSize(trackchannel.getSize());
   for (i=0; i<instruments.getSize(); i++) {
      if (forcedQ) {
         instruments[i] = instrumentnumber;
      } else {
         instruments[i] = -1;
      }
      trackchannel[i] = 0;
   }
   HumdrumInstrument x;
   int inst = -1;
   int ptrack = 0;
   int instcount = 0;
   PerlRegularExpression pre2;

   VolumeMapping.setSize(infile.getMaxTracks()+1);
   VolumeMapping.setAll(64);
   if (idynQ) {
      VolumeMapping.setSize(0);
   }
   for (i=0; i<infile.getNumLines(); i++) {
      if (debugQ) {
         cout << "line " << i+1 << ":\t" << infile[i] << endl;
      }

      if (infile[i].isData()) {
         break;
      }
      if (!infile[i].isInterpretation()) {
         continue;
      }

      for (j=0; j<infile[i].getFieldCount(); j++) {
         track = infile[i].getPrimaryTrack(j);
         if (strncmp(infile[i][j], "*I", 2) == 0) {
            if (timbresQ) {

               PerlRegularExpression pre;
               if (!pre.search(infile[i][j], "^\\*I\"\\s*(.*)\\s*", "")) {
                  inst = -1;
               } else {
                  SigString targetname;
                  targetname = pre.getSubmatch(1);
                  int i;
                  inst = -1;

                  for (i=0; i<TimbreName.getSize(); i++) {
                     if (pre2.search(targetname.getBase(), 
                           TimbreName[i].getBase(), "i")) {
                     // if (TimbreName[i] == targetname) {
                        inst = TimbreValue[i];
                        if (track < VolumeMapping.getSize()) {
                           VolumeMapping[track] = TimbreVolume[i];
                        }
                        break;
                     }
                  }

                  if (inst == -1) {
                     // search for default timbre setting if not otherwise
                     // found.
                     for (i=0; i<TimbreName.getSize(); i++) {
                        if (pre2.search("DEFAULT", TimbreName[i].getBase(), 
                              "i")) {
                        // if (TimbreName[i] == targetname) {
                           inst = TimbreValue[i];
                           if (track < VolumeMapping.getSize()) {
                              VolumeMapping[track] = TimbreVolume[i];
                           }
                           break;
                        }
                     }
                  }

               }

            } else if (!forcedQ) {
               inst = x.getGM(infile[i][j]);
            } else {
               inst = instrumentnumber;
            }
 
            if (inst != -1) {
               ptrack = infile[i].getPrimaryTrack(j);
               if (instruments[ptrack] == -1) {
                  if (!forcedQ) {
                     instruments[ptrack] = inst;
                  } else {
                     instruments[ptrack] = instrumentnumber;
                  }
                  instcount++;
               } 
            }
         }
      }
   }

   int nextChannel = 0;        // next midi channel available to assign

   if (instcount < 15) {
      // have enough space to store each instrument on a separate channel
      // regardless of instrumentation
      nextChannel = 1;        // channel 0 is for undefined instrument spines
      for (i=0; i<instruments.getSize(); i++) {
         if (instruments[i] == -1) {
            trackchannel[i] = 0;
         } else {
            trackchannel[i] = nextChannel++;
         }

         // avoid General MIDI percussion track.
         if (nextChannel == 9) {
            nextChannel++;
         }
      }
   } else {
      // need to conserve channels:  place duplicate instrument tracks on the 
      // same channel.
      nextChannel = 1;     // channel 0 is for undefined instrument spines
      int foundDup = -1;
      for (i=0; i<instruments.getSize(); i++) {
         foundDup = -1;
         for (j=0; j<i; j++) {
            if (instruments[j] == instruments[i]) {
               foundDup = j; 
            }
         }
         if (instruments[i] == -1) {
            trackchannel[i] = 0;
         } else if (foundDup != -1) {
            trackchannel[i] = trackchannel[foundDup];
         } else {
            trackchannel[i] = nextChannel++;
         }

         // avoid General MIDI percussion track.
         if (nextChannel == 9) {
            nextChannel++;
         }
      }

      if (nextChannel > 16) {
         // channel allocation is over quota: squash over-allocations:
         for (i=0; i<trackchannel.getSize(); i++) {
            if (trackchannel[i] > 15) {
               trackchannel[i] = 0;
            }
         }
      }

   }

   Array<int> kerntracks;
   getKernTracks(kerntracks, infile);

   // don't conserve tracks if there is enough to go around
   if (kerntracks.getSize() < 13) { 
      for (i=0; i<kerntracks.getSize(); i++) {
         trackchannel[kerntracks[i]] = i;
         if (i>9) {   // avoid general midi drum track channel
            trackchannel[kerntracks[i]] = i+1;
         }
      }
   }

}



//////////////////////////////
//
// checkForTempo --
//

double checkForTempo(HumdrumRecord& record) {
   if (timeQ) {
      // don't encode tempos if the --time option is set.
      return -1.0;
   }
   int i;
   float tempo = 60.0;
   PerlRegularExpression pre;

   // if (!metQ) {

      for (i=0; i<record.getFieldCount(); i++) {
         if (strncmp(record[i], "*MM", 3) == 0) {
            sscanf(&(record[i][3]), "%f", &tempo);
            // cout << "Found tempo marking: " << record[i] << endl;
            return (double)tempo * tscaling;
         }
      }

   // } else {
   if (metQ) {

      // mensural tempo scalings
      // O           = 58 bpm
      // O.          = 58 bpm
      // C.          = 58 bpm
      // C           = 48 bpm
      // C|          = 72 bpm
      // O2          = 75 bpm
      // C2          = 75 bpm
      // O|          = 76 bpm
      // C|3, 3, 3/2 = 110 bpm
      // C3          = 110 bpm
      // O/3         = 110 bpm
      // C|2, Cr     = 220 bpm
      char mensuration[1024] = {0};
      if (record.isGlobalComment() && pre.search(record[0], 
            "^\\!+primary-mensuration:.*met\\((.*?)\\)\\s*$")) {
         strcpy(mensuration, pre.getSubmatch(1));
      } else if (record.isInterpretation() && record.equalFieldsQ("**kern")) {
         for (i=0; i<record.getFieldCount(); i++) {
            if (record.isExInterp(i, "**kern")) {
               if (pre.search(record[i], "met\\((.*?)\\)")) {
                  strcpy(mensuration, pre.getSubmatch(1));
               }
               break;
            }
         }    
      }

      if (strcmp(mensuration, "O") == 0) {
         return (double)metQ * 1.0;
      } else if (strcmp(mensuration, "C|") == 0) {
         return (double)metQ * 1.241793;
      } else if (strcmp(mensuration, "C.") == 0) {
         return (double)metQ * 1.0;
      } else if (strcmp(mensuration, "O.") == 0) {
         return (double)metQ * 1.0;
      } else if (strcmp(mensuration, "C") == 0) {
         return (double)metQ * 0.8;
      } else if (strcmp(mensuration, "O|") == 0) {
         return (double)metQ * 1.310448;
      } else if (strcmp(mensuration, "C|3") == 0) {
         return (double)metQ * 1.8965517;
      } else if (strcmp(mensuration, "C3") == 0) {
         return (double)metQ * 1.8965517;
      } else if (strcmp(mensuration, "3") == 0) {
         return (double)metQ * 1.8965517;
      } else if (strcmp(mensuration, "3/2") == 0) {
         return (double)metQ * 1.8965517;
      } else if (strcmp(mensuration, "O/3") == 0) {
         return (double)metQ * 1.8965517;
      } else if (strcmp(mensuration, "O2") == 0) {
         return (double)metQ * 1.25;
      } else if (strcmp(mensuration, "C2") == 0) {
         return (double)metQ * 1.25;
      } else if (strcmp(mensuration, "C|2") == 0) {
         return (double)metQ * 3.791034;
      } else if (strcmp(mensuration, "Cr") == 0) {
         return (double)metQ * 3.791034;
      }

   }

   return -1.0;
}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("C|nocomments=b",  "Do not store comments as meta text");
   opts.define("D|nodynamics=b",  "Do not encode dynamics found in file");
   opts.define("showdynamics=b",  "Show the calculated dynamics by input line");
   opts.define("n|note|comment=s","Store a comment line in the file");
   opts.define("T|notext=b",      "Do not non-musical data as meta text");
   opts.define("o|output=s:midi.mid", "Output midifile");
   opts.define("0|O|type0|zero=b","Generate a type 0 MIDI file");
   opts.define("plus=b",          "Create a MIDIPlus compliant MIDI file");
   opts.define("time=b",          "Use timing from a **time spine");
   opts.define("sec|time-in-seconds=b", "Use timing from a **time spine");
   opts.define("v|volume=i:64",   "Default attack velocity");
   opts.define("d|dyn|idyn=d:40.0","Extract attack velocities from **idyn");
   opts.define("t|tempo-scaling=d:1.0", "Tempo scaling");
   opts.define("ts|tempo|tempo-spine=b", "Use tempo markings from tempo spine");
   opts.define("I|noinstrument=b", "Do not store MIDI instrument programs");
   opts.define("i|instruments=s", "Specify MIDI conversions for instruments");
   opts.define("f|forceinstrument=i:0", "MIDI instrument for all tracks");
   opts.define("c|channel=i:-1",   "Channel to store for MIDI data");
   opts.define("m|min=i:0",        "Minimum tick duration of notes");
   opts.define("r|rhythmic-scaling=d:1.0", "Scale all score durations");
   opts.define("s|shorten=i:0",    "Shortening tick value for note durations");
   opts.define("p|plain=b",        "Play with articulation interpretation");
   opts.define("P|nopad=b",        "Do not pad ending with spacer note");
   opts.define("met=d:232",        "Tempo control from metrical symbols");
   opts.define("hv|humanvolume=i:5","Apply a random adjustment to volumes");
   opts.define("mv|metricvolume=i:3","Apply metric accentuation to volumes");
   opts.define("fs|sforzando=i:20","Increase sforzandos by specified amount");
   opts.define("no-rest=b",      "Do not put rests at start of midi");
   opts.define("pvm|perfviz=s:", "Create a PerfViz match file for MIDI output");
   opts.define("debug=b",        "Debugging turned on");
   opts.define("stdout=b",       "Print MIDI file to standard output");
   opts.define("mark=b",         "Handle marked notes somehow");
   opts.define("bend=d:200.0",   "Turn on pitch-bend with given half-depth");
   opts.define("temperament=s:", "Turn on pitch-bend with given data file");
   opts.define("timbres=s",      "Timbral assignments by instrument name");
   opts.define("autopan=b",      "Pan tracks from left to right");

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("h|help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, May 1998" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: April 2002" << endl;
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

   if (opts.getBoolean("instruments")) {
      reviseInstrumentMidiNumbers(opts.getString("instruments"));
   }

   if (opts.getBoolean("nocomments")) {
      storeCommentQ = 0;
   } else {
      storeCommentQ = 1;
   }

   if (opts.getBoolean("met")) {
      metQ = int(opts.getDouble("met")+0.5);
   } else {
      metQ = 0;
   }

   storeTextQ = opts.getBoolean("notext");
   plusQ      = opts.getBoolean("plus");
   plainQ     = opts.getBoolean("plain");

   if (opts.getBoolean("nodynamics")) {
      dynamicsQ = 0;
   } else {
      dynamicsQ = 1;
      if (opts.getBoolean("showdynamics")) {
         dynamicsQ += 1;
      }
   }

   defaultvolume = opts.getInt("volume");
   if (defaultvolume < 1) {
      defaultvolume = 1;
   } else if (defaultvolume > 127) {
      defaultvolume = 127;
   }
   
   tscaling = opts.getDouble("tempo-scaling");
   if (tscaling < 0.001) {
      tscaling = 1.0;
   } else if (tscaling > 1000.0) {
      tscaling = 1.0; 
   }
   // tscaling = 1.0 / tscaling;
   instrumentQ = !opts.getBoolean("noinstrument");

   if (opts.getBoolean("channel")) {
      multitimbreQ = 0;
      fixedChannel = opts.getInteger("channel") - 1;
      if (fixedChannel < 0) {
         fixedChannel = 0;
      } 
      if (fixedChannel > 15) {
         fixedChannel = 15;
      } 
      instrumentQ = 0;
   } else {
      multitimbreQ  = 1;
      fixedChannel = -1;
   }

   if (opts.getBoolean("output")) {
      outlocation = opts.getString("output");
   } else {
      outlocation = NULL;
   }

   forcedQ = opts.getBoolean("forceinstrument");
   if (forcedQ) {
      instrumentnumber = opts.getInteger("forceinstrument");
      if (instrumentnumber < 0 || instrumentnumber > 127) {
         instrumentnumber = 0;
      } 
   } else {
      instrumentnumber = -1;
   }

   mine = opts.getInteger("min");
   if (mine < 0) {
      mine = 0;
   }
   debugQ        =  opts.getBoolean("debug");
   stdoutQ       =  opts.getBoolean("stdout");
   shortenQ      =  opts.getBoolean("shorten");
   shortenamount =  opts.getInteger("shorten");
   padQ          = !opts.getBoolean("nopad");
   humanvolumeQ  =  opts.getInteger("humanvolume");
   fixedvolumeQ  =  opts.getBoolean("volume");
   timeQ         =  opts.getBoolean("time");
   timeinsecQ    =  opts.getBoolean("time-in-seconds");
   tempospineQ   =  opts.getBoolean("tempo-spine");
   perfvizQ      =  opts.getBoolean("perfviz");
   metricvolumeQ =  opts.getInteger("metricvolume");
   sforzando     =  opts.getInteger("sforzando");
   idynQ         =  opts.getBoolean("dyn");
   idynoffset    =  opts.getDouble("dyn");
   norestQ       =  opts.getBoolean("no-rest");
   autopanQ      =  opts.getBoolean("autopan");
   bendQ         =  opts.getBoolean("bend");
   rhysc         = opts.getDouble("rhythmic-scaling");
   if (bendQ) {
      bendamt    =  opts.getDouble("bend");
      if (bendamt <= 0.0) {
         bendamt = 200.0;
      }
   }
   bendpcQ       =  opts.getBoolean("temperament");
   if (bendpcQ) {
      bendQ = 0;   // disable other type of bending (but keep bendamt)
      forcedQ = 1; // force a timber setting for all channels (piano default)
      getBendByPcData(bendbypc, opts.getString("temperament"));
      // for different method, see: http://www.xs4all.nl/~huygensf/scala
   }

   timbresQ = opts.getBoolean("timbres");
   if (timbresQ) {
      storeTimbres(TimbreName, TimbreValue, TimbreVolume, 
            opts.getString("timbres"));
   } else {
      TimbreName.setSize(0);
      TimbreValue.setSize(0);
   }
}



//////////////////////////////
//
// getBendByPcData --
//

void getBendByPcData(double* bendbypc, const char* filename) {
   int i, j;
   for (i=0; i<12; i++) {
      bendbypc[i] = 0.0;
   }

   HumdrumFile temperamentdata;
   temperamentdata.read(filename);
   HumdrumFile& td = temperamentdata;

   int pc;
   double dcent;
   for (i=0; i<td.getNumLines(); i++) {
      if (td[i].getType() != E_humrec_data) {
         continue;
      }
      pc = -1;
      dcent = 0;
      for (j=0; j<td[i].getFieldCount(); j++) {
         if (strcmp(td[i].getExInterp(j), "**kern") == 0) {
            if (strcmp(td[i][j], ".") == 0) {
               // ignore null tokens
               continue;
            }
            if (strchr(td[i][j], 'r') != NULL) {
               // ignore rests
               continue;
            }
            pc = Convert::kernToMidiNoteNumber(td[i][j]) % 12;
         } else if (strcmp(td[i].getExInterp(j), "**Dcent") == 0) {
            if (strcmp(td[i][j], ".") == 0) {
               // determine value of null tokens
	       sscanf(td.getDotValue(i,j), "%lf", &dcent);
            } else {
	       sscanf(td[i][j], "%lf", &dcent);
            }
         }
      }
      if (pc >= 0 && pc < 12) {
         bendbypc[pc] = dcent / bendamt;
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
// storeMetaText --
//

void storeMetaText(MidiFile& mfile, int track, const char* string, int tick,
      int metaType) {
   int i;
   int length = strlen(string);
   Array<uchar> metadata;
   uchar size[23] = {0};
   int lengthsize =  makeVLV(size, length);

   metadata.setSize(2+lengthsize+length);
   metadata[0] = 0xff;
   metadata[1] = metaType;
   for (i=0; i<lengthsize; i++) {
      metadata[2+i] = size[i];
   }
   for (i=0; i<length; i++) {
      metadata[2+lengthsize+i] = string[i];
   }

   mfile.addEvent(track, tick + offset, metadata);
}



//////////////////////////////
//
// makeVLV -- 
//

int makeVLV(uchar *buffer, int number) {

   unsigned long value = (unsigned long)number;

   if (value >= (1 << 28)) {
      cout << "Error: number too large to handle" << endl; 
      exit(1);
   }
  
   buffer[0] = (value >> 21) & 0x7f;
   buffer[1] = (value >> 14) & 0x7f;
   buffer[2] = (value >>  7) & 0x7f;
   buffer[3] = (value >>  0) & 0x7f;

   int i;
   int flag = 0;
   int length = -1;
   for (i=0; i<3; i++) {
      if (buffer[i] != 0) {
         flag = 1;
      }
      if (flag) {
         buffer[i] |= 0x80;
      }
      if (length == -1 && buffer[i] >= 0x80) {
         length = 4-i;
      }
   }

   if (length == -1) {
      length = 1;
   }

   if (length < 4) {
      for (i=0; i<length; i++) {
         buffer[i] = buffer[4-length+i];
      }
   }

   return length;
}



//////////////////////////////
//
// getIdynDynamics -- extracts **idyn amplitude values for notes.
//

void getIdynDynamics(HumdrumFile& infile, Array<Array<char> >& dynamics, 
      double idynoffset) {
   int i, j, k;
   int intdyn;
   char cdyn;
   double dyn;
   int notecount;
   char buffer[1024] = {0};
   dynamics.setSize(infile.getNumLines());
   for (i=0; i<dynamics.getSize(); i++) {
      dynamics[i].setSize(8);
      dynamics[i].setSize(0);
   }

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_data) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp(infile[i].getExInterp(j), "**idyn") != 0)  {
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            continue;
         }

         notecount = infile[i].getTokenCount(j);
         for (k=0; k<notecount; k++) {
            dyn = 0.0;
            infile[i].getToken(buffer, j, k);
            if (sscanf(buffer, "%lf", &dyn)) {
               dyn += idynoffset;
               // dyn *= 2.0;
            }
            intdyn = int(dyn + 0.5);
            if (intdyn < 1) {
               intdyn = 1;
            }
            if (intdyn > 127) {
               intdyn = 127;
            }
	    cdyn = char(intdyn);
            dynamics[i].append(cdyn);
         }
      }
   }


   if (debugQ) {
      for (i=0; i<dynamics.getSize(); i++) {
         cout << i << ":";
         for (j=0; j<dynamics[i].getSize(); j++) {
            cout << "\t" << (int)dynamics[i][j];
         }
         cout << "\n";
      }
   }

}
            


//////////////////////////////
//
// storeMidiData --
//

void storeMidiData(HumdrumFile& infile, MidiFile& outfile) {
   double duration = 0.0;
   int midinote = 0;
   int base40note = 0;
   double absbeat = 0.0;
   int ontick = 0;
   int idyncounter = 0;   // used for individual note volumes
   int offtick = 0;
   Array<uchar> mididata;
   int i, j, k;
   int ii;
   int tokencount = 0;
   char buffer1[1024] = {0};
   double pvscoredur = 0.0;
   int staccatoQ = 0;
   int accentQ = 0;
   int sforzandoQ = 0;
   int volume = defaultvolume;
   int ttempo;
   Array<int> freeQ;
   freeQ.setSize(infile.getMaxTracks());
   freeQ.setAll(0);
   Array<Array<int> > freenotestate;
   freenotestate.setSize(freeQ.getSize());
   for (i=0; i<freenotestate.getSize(); i++) {
      freenotestate[i].setSize(0);
      freenotestate[i].allowGrowth();
   }
   int ptrack = 0;

   Array<Array<char> > dynamics;
   if (idynQ) {
      getIdynDynamics(infile, dynamics, idynoffset);
   } else {
      getDynamics(infile, dynamics, defaultvolume);
   }

   // store a default tempo marking if the tempo scaling option 
   // was used.
   if (tscaling != 1.0) {
      ttempo = (int)(100 * tscaling);
      if (ttempo > 0) {
         mididata.setSize(6);
         mididata[0] = 0xff;
         mididata[1] = 0x51;
         mididata[2] = 3;
         ttempo = (int)(60000000.0 / ttempo + 0.5);   
         mididata[3] = (uchar)((ttempo >> 16) & 0xff);
         mididata[4] = (uchar)((ttempo >> 8) & 0xff);
         mididata[5] = (uchar)(ttempo & 0xff);
         outfile.addEvent(0, 0 + offset, mididata);
      }
   }

   if (options.getBoolean("comment")) {
      storeMetaText(outfile, 0, options.getString("comment"), 0);
   }

   if (perfvizQ) {
      // set the tempo to MM 120.0 at tick time 0.
      mididata.setSize(6);
      mididata[0] = 0xff;
      mididata[1] = 0x51;
      mididata[2] = 3;
      ttempo = (int)(60000000.0 / 120.0 + 0.5);   
      mididata[3] = (uchar)((ttempo >> 16) & 0xff);
      mididata[4] = (uchar)((ttempo >> 8) & 0xff);
      mididata[5] = (uchar)(ttempo & 0xff);
      outfile.addEvent(0, 0, mididata);
   }

   if (autopanQ) {
      autoPan(outfile, infile);
   }

   if (storeTextQ) {
      // store the title
      Array<char> title;
      title.setSize(0);
      getTitle(title, infile);
      if (title.getSize() > 0) {
         storeMetaText(outfile, 0, title.getBase(), 0, 3);
      }
   }

   for (i=0; i<infile.getNumLines(); i++) {
      if (debugQ) {
         cout << "Line " << i+1 << "::\t" << infile[i] << endl;
      }

      if (storeCommentQ && (infile[i].getType() == E_humrec_global_comment)) {
         if (timeQ || perfvizQ) {
            ontick = getMillisecondTime(infile, i);
            if (perfvizQ) {
               ontick = int(ontick * tickfactor + 0.5);
            }
         } else {
            absbeat = infile.getAbsBeat(i);
            ontick = int(absbeat * outfile.getTicksPerQuarterNote());
         }
         storeMetaText(outfile, 0, infile[i].getLine(), ontick + offset);
      } else if (storeTextQ && (infile[i].getType() == E_humrec_bibliography)) {
         if (timeQ || perfvizQ) {
            ontick = getMillisecondTime(infile, i);
            if (perfvizQ) {
               ontick = int(ontick * tickfactor + 0.5);
            }
         } else {
            absbeat = infile.getAbsBeat(i);
            ontick = int(absbeat * outfile.getTicksPerQuarterNote());
         }
         if (strncmp(&(infile[i].getLine()[3]), "YEC", 3) == 0) {
            storeMetaText(outfile, 0, infile[i].getLine(), ontick+offset, 2);
         // store OTL as regular text, creating sequence name separately
         //} else if (strncmp(&(infile[i].getLine()[3]), "OTL", 3) == 0) {
         //   storeMetaText(outfile, 0, infile[i].getLine(), ontick+offset, 3);
         } else if (storeCommentQ) {
            storeMetaText(outfile, 0, infile[i].getLine(), ontick + offset);
         }
      } 
      if (infile[i].isInterpretation() || infile[i].isGlobalComment()) {

         tempo = (int)checkForTempo(infile[i]);
         if (tempo > 0 && !tempospineQ && !perfvizQ) {
            // cout << "The tempo read was " <<  tempo << endl;
            ttempo = tempo;  // scaling already applied.
            mididata.setSize(6);
            mididata[0] = 0xff;
            mididata[1] = 0x51;
            mididata[2] = 3;
            ttempo = (int)(60000000.0 / ttempo + 0.5);   
            mididata[3] = (uchar)((ttempo >> 16) & 0xff);
            mididata[4] = (uchar)((ttempo >> 8) & 0xff);
            mididata[5] = (uchar)(ttempo & 0xff);
            if (timeQ || perfvizQ) {
               ontick = getMillisecondTime(infile, i);
               if (perfvizQ) {
                  ontick = int(ontick * tickfactor + 0.5);
               }
            } else {
               absbeat = infile.getAbsBeat(i);
               ontick = int(absbeat * outfile.getTicksPerQuarterNote());
            }
            outfile.addEvent(0, ontick + offset, mididata);
            // outfile.addEvent(0, 10 + offset, mididata);
            tempo = -1;
         }


         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (timeQ || perfvizQ) {
               ontick = getMillisecondTime(infile, i);
               if (perfvizQ) {
                  ontick = int(ontick * tickfactor + 0.5);
               }
            } else {
               absbeat = infile.getAbsBeat(i);
               ontick = int(absbeat * outfile.getTicksPerQuarterNote());
            }
            int track = infile[i].getPrimaryTrack(j);

            if (strcmp(infile[i][j], "**kern") == 0 && forcedQ && !bendpcQ) {
               Array<uchar> mididata;
               mididata.setSize(2);
               mididata[0] = 0xc0 | (0x0f & trackchannel[track]);
               mididata[1] = instrumentnumber;
               outfile.addEvent(track, ontick + offset, mididata);
               continue;
            }

            if (strncmp(infile[i][j], "*I", 2) == 0) {
               storeInstrument(ontick + offset, outfile, infile, i, j, 
                     instrumentQ);
               continue;
            } 

            if ((!autopanQ) && (strncmp(infile[i][j], "*pan=", 5) == 0)) {
               storePan(ontick + offset, outfile, infile, i, j);
               continue;
            } 
	    
	    // capture info data for PerfViz:
            int length = strlen(infile[i][j]);
            int key;
            if (infile[i][j][length-1] == ':') {
               key = Convert::kernToBase40(infile[i][j]);
               if (key != PerfVizNote::key) {
                  printPerfVizKey(key);
                  PerfVizNote::key = key;
               }
            }
            int tstop;
            int tsbottom;
            if (sscanf(infile[i][j], "*M%d/%d", &tstop, &tsbottom) == 2) {
               if (tstop != PerfVizNote::tstop || 
                   tsbottom != PerfVizNote::tsbottom) {
                  printPerfVizTimeSig(tstop, tsbottom);
                  PerfVizNote::tstop = tstop;
                  PerfVizNote::tsbottom = tsbottom;
               }
            }
            double approxtempo;
            if (sscanf(infile[i][j], "*MM%lf", &approxtempo) == 1) {
               if (approxtempo != PerfVizNote::approxtempo) {
                  printPerfVizTempo(approxtempo);
                  PerfVizNote::approxtempo = approxtempo;
               }
            }
            // info fields to be addressed in the future:
            // info(beatSubdivisions,[3]).
            // info(tempoIndication,[andante]).
            // info(subtitle,[]).

            
            if (strcmp(infile[i][j], "*free") == 0) {
               freeQ[infile[i].getPrimaryTrack(j)-1] = 1;
            } else if (strcmp(infile[i][j], "*strict") == 0) {
               freeQ[infile[i].getPrimaryTrack(j)-1] = 0;

               // turn off any previous notes in the track
               // started during a free rhythm section
               ptrack = infile[i].getPrimaryTrack(j) - 1;
               for (ii=0; ii<freenotestate[ptrack].getSize(); ii++) {
                  // turn off the note if it is zero or above
                  if (freenotestate[ptrack][ii] >= 0) {
                     mididata.setSize(3);
                     if (bendpcQ) {
                        int pcchan = freenotestate[ptrack][ii] % 12;
                        if (pcchan >= 9) { 
                           pcchan++;
                        }
                        mididata[0] = 0x80 | pcchan;
                     } else {
                        mididata[0] = 0x80 | trackchannel[track];
                     }
                     mididata[1] = (uchar)freenotestate[ptrack][ii];
                     mididata[2] = 0;
                     outfile.addEvent(track, ontick + offset + 1, mididata);
                     // added 1 to the previous line for grace notes 7Apr2004
                     freenotestate[ptrack][ii] = -1;
                     if (ii == freenotestate[ptrack].getSize() - 1) {
                        // shrink-wrap the free note off array
                        freenotestate[ptrack].setSize(ii);
                        break;
                     }
                  }
               }
            }

         }
      } else if (infile[i].getType() == E_humrec_data_measure) {
         PerfVizNote::bar += 1;
      } else if (infile[i].getType() == E_humrec_data) {
         idyncounter = 0;
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (strcmp(infile[i][j], ".") == 0) {
               continue;
            }
            if (!infile[i].isExInterp(j, "**kern")) {
               continue;
            }

            if (idynQ) {
              if (dynamics[i].getSize() > idyncounter) {
                 volume = dynamics[i][idyncounter]; 
              } else {
                //  cout << "Warning: bad volume data on line " << i+1 << endl;
		//  cout << "Size of dynamics array is: " 
                //       << dynamics[i].getSize() << endl;
                //  cout << "Note counter on line: " << idyncounter << endl;
                //  volume = 64;
              }
            } else {
               volume = dynamics[infile[i].getPrimaryTrack(j)-1][i];
            }

            if (strcmp(infile[i][j], ".") == 0) {
               continue;
            }

            if (infile[i].getExInterpNum(j) != E_KERN_EXINT) {
               if (timeQ || perfvizQ) {
                  ontick = getMillisecondTime(infile, i);
                  if (perfvizQ) {
                     ontick = int(ontick * tickfactor + 0.5);
                  }
               } else {
                  absbeat = infile.getAbsBeat(i);
                  ontick = int(absbeat * outfile.getTicksPerQuarterNote());
               }
               track = infile[i].getPrimaryTrack(j);
               if (storeTextQ) {
                  storeMetaText(outfile, track, infile[i][j], ontick+offset);
               }
               continue;
            } else {
               // process **kern note events

               track      = infile[i].getPrimaryTrack(j);
               tokencount = infile[i].getTokenCount(j);
               ptrack     = infile[i].getPrimaryTrack(j) - 1;

               if (freeQ[ptrack] != 0) {
                  // turn off any previous notes in the track
                  // during a free rhythm section
                  for (ii=0; ii<freenotestate[ptrack].getSize(); ii++) {
                     // turn off the note if it is zero or above
                     if (freenotestate[ptrack][ii] >= 0) {
                        mididata.setSize(3);
                        if (bendpcQ) {
                           int pcchan = freenotestate[ptrack][ii] % 12;
                           if (pcchan >= 9) { 
                              pcchan++;
                           }
                           mididata[0] = 0x90 | pcchan;
                        } else {
                           mididata[0] = 0x80 | trackchannel[track];
                        }
                        mididata[1] = (uchar)freenotestate[ptrack][ii];
                        mididata[2] = 0;
                        outfile.addEvent(track, ontick + offset, mididata);
                        freenotestate[ptrack][ii] = -1;
                        if (ii == freenotestate[ptrack].getSize() - 1) {
                           // shrink-wrap the free note off array
                           freenotestate[ptrack].setSize(ii);
                           break;
                        }
                     }
                  }
               }
               if (VolumeMapping.getSize() > 0) {
                  volume = VolumeMapping[infile[i].getPrimaryTrack(j)];
               }
               for (k=0; k<tokencount; k++) {
                  infile[i].getToken(buffer1, j, k); 

                  // skip tied notes
                  if (strchr(buffer1, '_') || strchr(buffer1, ']')) {
                     continue;
                  }

                  if (timeQ) { 
                     duration = getMillisecondDuration(infile, i, j, k);
                  } else if (perfvizQ) {
                     duration = getMillisecondDuration(infile, i, j, k);
                     // perfviz duration shortened later

                     // store the score duration of the note for perfviz
                     if (strchr(buffer1, '[')) {
                        // total tied note durations
                        pvscoredur = infile.getTiedDuration(i, j, k);
                     } else {
                        pvscoredur = rhysc * Convert::kernToDuration(buffer1);
                     }
                  } else {
                     if (strchr(buffer1, '[')) {
                        // total tied note durations
                        duration = infile.getTiedDuration(i, j, k);
                     } else {
                        duration = rhysc * Convert::kernToDuration(buffer1);
                     }
                  }
                  midinote = Convert::kernToMidiNoteNumber(buffer1); 
                  base40note = Convert::kernToBase40(buffer1);
                  // skip rests 
                  if (midinote < 0) {
                     continue;
                  }

                  if (!plainQ) {
                     accentQ    = strchr(buffer1, '^')  == NULL ? 0 : 1;
                     sforzandoQ = strchr(buffer1, 'z')  == NULL ? 0 : 1;
                     staccatoQ  = strchr(buffer1, '\'') == NULL ? 0 : 1;
                     // treat attacas/staccatissimos as staccatos
                     staccatoQ |= strchr(buffer1, '`') == NULL ? 0 : 1;
                     // treat pizzicatos as staccatos
                     staccatoQ |= strchr(buffer1, '"') == NULL ? 0 : 1;
                     // treat spiccatos as staccatos (maybe shorten later?)
                     staccatoQ |= strchr(buffer1, 's') == NULL ? 0 : 1;
                  }
   
                  if (shortenQ) {
                     duration -= shortenamount;
                     if (duration < mine) {
                        duration = mine;
                     }
                  }

                  if (staccatoQ) {
                     duration = duration * 0.5;
                  }
                  if (accentQ) {
                     volume = (int)(volume * 1.3 + 0.5);
                  }
                  if (sforzandoQ) {
                     volume = (int)(volume * 1.5 + 0.5);
                  }
                  if (volume > 127) {
                     volume = 127;
                  }
                  if (volume < 1) {
                     volume = 1;
                  }
   
                  if (plusQ) {
                     volume = setMidiPlusVolume(buffer1);
                  }
                  if (timeQ) {
                     ontick  = getMillisecondTime(infile, i);
                     offtick = (int)(ontick + duration + 0.5);
                  } else if (perfvizQ) {
                     ontick  = int(getMillisecondTime(infile,i)*tickfactor+0.5);
                     offtick = int(getMillisecondTime(infile,i)*tickfactor + 
                                duration*tickfactor + 0.5);
                  } else {
                     absbeat = infile.getAbsBeat(i);
                     ontick  = int(absbeat * outfile.getTicksPerQuarterNote());
                     offtick = int(duration * 
                           outfile.getTicksPerQuarterNote()) + ontick;
                  }
                  if (shortenQ) {
                     offtick -= shortenamount;
                     if (offtick - ontick < mine) {
                        offtick = ontick + mine;
                     }
                  }

                  // fix for grace note noteoffs (7 Sep 2004):
                  if (timeQ) {
                     if (offtick <= ontick) {
                        offtick = ontick + 100;
                     }
                  } else if (perfvizQ) {
                     if (offtick <= ontick) {
                        offtick = int(ontick + 100 * tickfactor + 0.5);
                     }
                  } else {
                     if (offtick <= ontick) {
                        offtick = ontick + (int)(infile[i].getDuration() 
                                   * outfile.getTicksPerQuarterNote()+0.5);
                     }
                     // in case the duration of the line is 0:
                     if (offtick <= ontick) {
                        offtick = ontick + 12; 
                     }
                  }

                  if (volume < 0) {
                     volume = 1;
                  }
                  if (volume > 127) {
                     volume = 127;
                  }
                  mididata.setSize(3);
                  if (bendpcQ) {
                     int pcchan = midinote % 12;
                     if (pcchan >= 9) { 
                        pcchan++;
                     }
                     mididata[0] = 0x90 | pcchan;
                  } else {
                     mididata[0] = 0x90 | trackchannel[track];
                  }
                  mididata[1] = midinote;
                  mididata[2] = volume;
                  if (fixedvolumeQ) {
                     mididata[2] = defaultvolume;
                  }
                  outfile.addEvent(track, ontick + offset, mididata);
                  if (bendQ) {
                     checkForBend(outfile, ontick + offset, trackchannel[track],
                           infile, i, j, bendamt);
                  }
                  idyncounter++;
                  if (perfvizQ && PVIZ != NULL) {
                     PerfVizNote pvnote;
                     pvnote.pitch	= base40note;
                     pvnote.scoredur    = pvscoredur;
                     pvnote.absbeat     = infile[i].getAbsBeat();
                     pvnote.beat	= int(infile[i].getBeat());
                     pvnote.beatfrac    = infile[i].getBeat() - pvnote.beat;
                     pvnote.beatdur     = pvscoredur;
                     pvnote.vel		= volume;
                     pvnote.ontick      = ontick;
                     pvnote.offtick     = offtick;
                     PVIZ[0] << pvnote;
                  }
                  if (freeQ[ptrack] == 0) {
                     // don't specify the note off duration when rhythm is free.
                     if (bendpcQ) {
                        int pcchan = midinote % 12;
                        if (pcchan >= 9) { 
                           pcchan++;
                        }
                        mididata[0] = 0x80 | pcchan;
                     } else {
                        mididata[0] = 0x80 | trackchannel[track];
                     }
                     outfile.addEvent(track, offtick + offset, mididata);
                  } else {
                     // store the notes to be turned off later
                     storeFreeNote(freenotestate, ptrack, midinote);
                  }
               }
            }
         }
      }
   }

   // now add the end-of-track message to all tracks so that they
   // end at the same time.

   mididata.setSize(3);
   
   if (timeQ) {
      ontick = getFileDurationInMilliseconds(infile);
      ontick += 1000;
   } else if (perfvizQ) {
      ontick = getFileDurationInMilliseconds(infile);
      ontick += 1000;
      ontick = int(ontick * tickfactor + 0.5);
   } else {
      absbeat = infile.getAbsBeat(infile.getNumLines()-1);
      ontick = int(absbeat * outfile.getTicksPerQuarterNote());
      // note that extra time is added so that the last note will not get 
      // cut off by the MIDI player.  
      ontick += 120;
   }

   // stupid Microsoft Media player (et al.) will still cut off early, 
   // so add a dummy note-off at end as well...

   if (options.getBoolean("type0")) {
      outfile.joinTracks();
      if (padQ) {
         mididata[0] = 0x90;
         mididata[1] = 0x00;
         mididata[2] = 0x00;
         outfile.addEvent(0, ontick-1+offset, mididata);
      }
      mididata[0] = 0xff;
      mididata[1] = 0x2f;
      mididata[2] = 0x00;
      outfile.addEvent(0, ontick+offset, mididata);
   } else {  // type 1 MIDI file
      int trackcount = outfile.getNumTracks();
      for (i=0; i<trackcount; i++) {
         // Skip padding the first track, because Windows Media Player 
	 // won't play the MIDI file otherwise.
         if (i > 0 && padQ) {
            mididata[0] = 0x90;
            mididata[1] = 0x00;
            mididata[2] = 0x00;
            outfile.addEvent(i, ontick-1+offset, mididata);
         }
         mididata[0] = 0xff;
         mididata[1] = 0x2f;
         mididata[2] = 0x00;
         outfile.addEvent(i, ontick+offset, mididata);
      }
   }
}



//////////////////////////////
//
// getTitle -- return the title of the work.  If OPR is present, then
//    include that first, then OTL
//

void getTitle(Array<char>& title, HumdrumFile& infile) {
   title.setSize(1000);
   title.setSize(0);
   int opr = -1;
   int otl = -1;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      if ((opr < 0) && (strncmp(infile[i][0], "!!!OPR", 6) == 0)) {
         opr = i;
      }
      if ((otl < 0) && (strncmp(infile[i][0], "!!!OTL", 6) == 0)) {
         otl = i;
      }
   }

   char bufferopr[512] = {0};
   char bufferotl[512] = {0};
   if (opr >= 0) {
      infile[opr].getBibValue(bufferopr);
   }
   if (otl >= 0) {
      infile[otl].getBibValue(bufferotl);
   }
   char buffer[1024] = {0};
   strcat(buffer, bufferopr);
   if (otl >= 0) {
      if (opr >= 0) {
         strcat(buffer, "  ");
      }
   }
   strcat(buffer, bufferotl);
   int len = strlen(buffer);
   if (len == 0) {
      return;
   }
   title.setSize(len+1);
   strcpy(title.getBase(), buffer);
}



//////////////////////////////
//
// checkForBend -- 
//

void checkForBend(MidiFile& outfile, int notetick, int channel, 
      HumdrumFile& infile, int row, int col, double scalefactor) {
   double bendvalue = 0;

   int i;
   for (i=col+1; i<infile[row].getFieldCount(); i++) {
      if (strcmp(infile[row].getExInterp(i), "**Dcent") != 0) {
         continue;
      }
      if (strcmp(infile[row][i], ".") == 0) {
         // consider in the future to resolve "." to previous value
         // in case the **Dcent value is specified before a note attack
         break;
      }
      if (sscanf(infile[row][i], "%lf", &bendvalue)) {
         bendvalue = bendvalue / scalefactor;
         // store one tick before the expected note location
         // but this can cause an audible pitch aberation...
	 //notetick = notetick - 1;
	 //if (notetick < 0) {
         //   notetick = 0;
         // }
         outfile.addPitchBend(track, notetick, channel, bendvalue);
      }
      break;
   }
}



//////////////////////////////
//
// storeFreeNote -- store a midi note in the freenotestate array.
//

void storeFreeNote(Array<Array<int> >& array, int ptrack, int midinote) {
   int i;
   int loc = -1;
   for (i=0; i<array[ptrack].getSize(); i++) {
      if (array[ptrack][i] < 0) {
         loc = i;
         break;
      }
   }

   if (loc >= 0) {
      array[ptrack][loc] = midinote;
   } else {
      array[ptrack].append(midinote);
   }
}



//////////////////////////////
//
// reviseInstrumentMidiNumbers --
//

void reviseInstrumentMidiNumbers(const char* string) {
   const char* spaces = " \t\n,:;";
   char* buffer = new char[strlen(string) + 1];
   strcpy(buffer, string);
   HumdrumInstrument x;

   char* pointer = buffer;
   pointer = strtok(buffer, spaces);
   int instnum = 0;
   while (pointer != NULL) {
      char* humdrumname = pointer;
      pointer = strtok(NULL, spaces);
      if (pointer == NULL) {
         break;
      }
      instnum = 0;
      sscanf(pointer, "%d", &instnum);
      
      if (instnum < 0 || instnum > 127) {
         continue;
      }
      x.setGM(humdrumname, instnum);
      pointer = strtok(NULL, spaces);
   }
}



//////////////////////////////
//
// setMidiPlusVolume -- store slur and enharmonic spelling information.
//    based on definition in the book:
//       Beyond MIDI: The Handbook of Musical Codes. pp. 99-104.
//

int setMidiPlusVolume(const char* kernnote) {
   int output = 1 << 6;

   // check for slurs, but do not worry about separating multiple slurs
   if (strchr(kernnote, '(') != NULL) {
      // start of slur A
      output |= (1 << 2);
   } else if (strchr(kernnote, ')') != NULL) {
      // end of slur A
      output |= (1 << 4);
   }

   // set the accidental marking
   int base40class = Convert::kernToBase40(kernnote) % 40;
   int midinoteclass = Convert::kernToMidiNoteNumber(kernnote) % 12;

   int acheck = 0;
   switch (midinoteclass) {
      case 0:                           // C/Dbb/B#
         switch (base40class) {
            case  4: acheck = 1; break;  // Dbb
            case  0: acheck = 2; break;  // C
            case 36: acheck = 3; break;  // B#
            default: acheck = 0;
         }
         break;
      case 1:                           // C#/Db/B##
         switch (base40class) {
            case  5: acheck = 1; break;  // Db
            case  1: acheck = 2; break;  // C#
            case 37: acheck = 3; break;  // B##
            default: acheck = 0;
         }
         break;
      case 2:                           // D/C##/Ebb
         switch (base40class) {
            case 10: acheck = 1; break;  // Ebb
            case  6: acheck = 2; break;  // D
            case  2: acheck = 3; break;  // C##
            default: acheck = 0;
         }
         break;
      case 3:                           // D#/Eb/Fbb
         switch (base40class) {
            case 15: acheck = 1; break;  // Fbb
            case 11: acheck = 2; break;  // Eb
            case  7: acheck = 3; break;  // D#
            default: acheck = 0;
         }
         break;
      case 4:                           // E/Fb/D##
         switch (base40class) {
            case 16: acheck = 1; break;  // Fb
            case 12: acheck = 2; break;  // E
            case  8: acheck = 3; break;  // D##
            default: acheck = 0;
         }
         break;
      case 5:                           // F/E#/Gbb
         switch (base40class) {
            case 21: acheck = 1; break;  // Gbb
            case 17: acheck = 2; break;  // F
            case 13: acheck = 3; break;  // E#
            default: acheck = 0;
         }
         break;
      case 6:                           // F#/Gb/E##
         switch (base40class) {
            case 22: acheck = 1; break;  // Gb
            case 18: acheck = 2; break;  // F#
            case 14: acheck = 3; break;  // E##
            default: acheck = 0;
         }
         break;
      case 7:                           // G/Abb/F##
         switch (base40class) {
            case 27: acheck = 1; break;  // Abb
            case 23: acheck = 2; break;  // G
            case 19: acheck = 3; break;  // F##
            default: acheck = 0;
         }
         break;
      case 8:                           // G#/Ab/F###
         switch (base40class) {
            case 28: acheck = 1; break;  // Ab
            case 24: acheck = 2; break;  // G#
            default: acheck = 0;         // F###
         }
         break;
      case 9:                           // A/Bbb/G##
         switch (base40class) {
            case 33: acheck = 1; break;  // Bbb
            case 29: acheck = 2; break;  // A
            case 25: acheck = 3; break;  // G##
            default: acheck = 0;
         }
         break;
      case 10:                           // Bb/A#/Cbb
         switch (base40class) {
            case 38: acheck = 1; break;  // Cbb
            case 34: acheck = 2; break;  // Bb
            case 30: acheck = 3; break;  // A#
            default: acheck = 0;
         }
         break;
      case 11:                           // B/Cf/A##
         switch (base40class) {
            case 39: acheck = 1; break;  // Cb
            case 35: acheck = 2; break;  // B
            case 31: acheck = 3; break;  // A##
            default: acheck = 0;
         }
         break;
   }
     
   output |= acheck;
   return output; 
}



//////////////////////////////
//
// storePan --
//

void storePan(int ontime, MidiFile& outfile, HumdrumFile& infile, 
   int row, int column) {
   double value = 0.5;
   int mvalue = 64;
   sscanf(infile[row][column], "*pan=%lf", &value);
   if (value <= 1.0) {
      mvalue = int (value * 128.0);
   } else {
      mvalue = int(value + 0.5);
   }
   if (mvalue > 127) {
      mvalue = 127;
   } else if (mvalue < 0) {
      mvalue = 0;
   }
      
   int track = infile[row].getPrimaryTrack(column);
   int channel = 0x0f & trackchannel[track];   

   //ontime = ontime - 1;
   //if (ontime < 0) {
   //   ontime = 0;
   //}

   Array<uchar> mididata;
   mididata.setSize(3);
   mididata[0] = 0xb0 | channel;
   mididata[1] = 10;
   mididata[2] = mvalue;
   outfile.addEvent(track, ontime, mididata);
}


//////////////////////////////
//
// autoPan -- presuming multi-track MIDI file.
//

void autoPan(MidiFile& outfile, HumdrumFile& infile) {

   Array<int> kerntracks;
   getKernTracks(kerntracks, infile);

   double value = 0.0;
   int    mval  = 0;
   // Array<int> trackchannel;    // channel of each track
   
   Array<uchar> mididata;
   mididata.setSize(3);
   
   long ontime = 0;
   int i;
   int channel;
   int track;
   for (i=0; i<kerntracks.getSize(); i++) {
      track = kerntracks[i];
      channel = trackchannel[track];
      value = 127.0 * i/(kerntracks.getSize()-1);
      if (value < 0.0) { value = 0.0; }
      if (value > 127.0) { value = 127.0; }
      mval = (int)value;
      
      mididata[0] = 0xb0 | channel;
      mididata[1] = 10;
      mididata[2] = (char)mval;
      outfile.addEvent(track, ontime, mididata);
   }

}



//////////////////////////////
//
// storeInstrument --
//

void storeInstrument(int ontick, MidiFile& mfile, HumdrumFile& infile, 
      int line, int row, int pcQ) {

   PerlRegularExpression pre;
   PerlRegularExpression pre2;

   if (timbresQ && !forcedQ) {

      if (!pre.search(infile[line][row], "^\\*I\"\\s*(.*)\\s*", "")) {
         return;
      }
      SigString targetname;
      targetname = pre.getSubmatch(1);
      int i;
      int pc = -1;
      for (i=0; i<TimbreName.getSize(); i++) {
         if (pre2.search(targetname.getBase(), TimbreName[i].getBase(), "i")) {
            pc = TimbreValue[i];
            break;
         }
      }
      if (pc < 0) {
         for (i=0; i<TimbreName.getSize(); i++) {
            if (pre2.search("DEFAULT", TimbreName[i].getBase(), "i")) {
               pc = TimbreValue[i];
               break;
            }
         }
      }
      if (pc >= 0) {
         int track = -1;
         track = infile[line].getPrimaryTrack(row);
         int channel = 0x0f & trackchannel[track];   

         Array<uchar> mididata;
         mididata.setSize(2);
         mididata[0] = 0xc0 | channel;
         mididata[1] = (uchar)pc;
         mfile.addEvent(track, ontick + offset, mididata);
      }

   } else {
      static HumdrumInstrument huminst;
      int track = infile[line].getPrimaryTrack(row);
      const char* trackname = huminst.getName(infile[line][row]);
      int pc = huminst.getGM(infile[line][row]);
      if (forcedQ) {
         pc = instrumentnumber;
      }
      int channel = 0x0f & trackchannel[track];   
   
      // store the program change if requested:
      Array<uchar> mididata;
      mididata.setSize(2);
      mididata[0] = 0xc0 | channel;
      mididata[1] = (uchar)pc;
      if (pcQ && pc >= 0 && !forcedQ) {
         mfile.addEvent(track, ontick + offset, mididata);
      }

      if (!tracknamed[track]) {
         tracknamed[track] = 1;
         storeMetaText(mfile, track, trackname, 0, 3);    // Track Name
      }
      storeMetaText(mfile, track, trackname + offset, ontick, 4);  // Inst. Name
   }

}



//////////////////////////////
//
// usage --
//

void usage(const char* command) {

}


//////////////////////////////
//
// getDynamics --
//

void getDynamics(HumdrumFile& infile, Array<Array<char> >& dynamics, 
      int defaultdynamic) {
   int maxtracks = infile.getMaxTracks();
   Array<int> currentdynamic;
   currentdynamic.setSize(maxtracks);
   currentdynamic.setAll(defaultdynamic);

   Array<int> metlev;
   infile.analyzeMetricLevel(metlev);

   Array<Array<char> > crescendos;  // -1=decrescendo, +1=crescendo, 0=none
   crescendos.setSize(maxtracks);

   Array<Array<char> > accentuation;  // v = sf, sfz, fz, sffz
   accentuation.setSize(maxtracks);

   dynamics.setSize(maxtracks);

   int i; 
   int j;
   for (i=0; i<dynamics.getSize(); i++) {
      dynamics[i].setSize(infile.getNumLines());
      crescendos[i].setSize(infile.getNumLines());
      crescendos[i].setAll(0);
      accentuation[i].setSize(infile.getNumLines());
      accentuation[i].setAll(0);
   }

   Array<int> assignments;  // dynamic data which controls kern data
   getDynamicAssignments(infile, assignments);


   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_data) {
         getNewDynamics(currentdynamic, assignments, infile, i, crescendos, 
              accentuation);
      }
      for (j=0; j<currentdynamic.getSize(); j++) {
         dynamics[j][i] = currentdynamic[j];
         // remove new dynamic marker:
         if (currentdynamic[j] < 0) {
            currentdynamic[j] = -currentdynamic[j];
         }
      }
   }

   processCrescDecresc(infile, dynamics, crescendos);

   // convert negative dynamics back into positive ones.
   if (dynamicsQ > 1) {
      cout << "----------------------------------------------" << endl;
      cout << "Dynamics profile of file:" << endl;
   }
   for (i=0; i<dynamics[0].getSize(); i++) {
      for (j=0; j<dynamics.getSize(); j++) {
         if (dynamics[j][i] < 0) {
            dynamics[j][i] = -dynamics[j][i];
         } 
         if (humanvolumeQ) {
            dynamics[j][i] = adjustVolumeHuman(dynamics[j][i], humanvolumeQ);
         }
         if (metricvolumeQ) {
            dynamics[j][i] = adjustVolumeMetric(dynamics[j][i], 
                              metricvolumeQ, -metlev[i]);
         }
         if (dynamicsQ) {
            // probably should change accentuation of this type
            // into a continuous controller....
            dynamics[j][i] = applyAccentuation(dynamics[j][i], 
                                                     accentuation[j][i]);
         }
         if (dynamicsQ > 1) {
            cout << (int)dynamics[j][i] << "\t";
         }
      }
      if (dynamicsQ > 1) {
         cout << endl;
      }
   }
   if (dynamicsQ > 1) {
      cout << "----------------------------------------------" << endl;
   }

}



//////////////////////////////
//
// applyAccentuation -- 
//

char applyAccentuation(int dynamic, int accent) {
   switch (accent) {
      case 'v':
         dynamic += sforzando;
         break;
   }

   if (dynamic > 127) {
      dynamic = 127;
   } else if (dynamic <= 0) {
      dynamic = 1; 
   }

   return (char)dynamic;
}



///////////////////////////////
//
// adjustVolumeMetric -- adjust the attack volume based on the
//  metric position of the note (on the beat, off the beat, on 
//  a metrically strong beat).
//

char adjustVolumeMetric(int startvol, int delta, double metricpos) {
   if (metricpos > 0.0) {
      startvol += delta;
   } else if (metricpos < 0) {
      startvol -= delta;
   }
 
   if (startvol <= 0) {
      startvol = 1;
   } else if (startvol > 127) {
      startvol = 127;
   }
   
   return (char)startvol;
}



////////////////////////////////
//
// adjustVolumeHuman -- add a randomness to the volume
//

char adjustVolumeHuman(int startvol, int delta) {
   int randval;
   #ifndef VISUAL
      randval = lrand48() % (delta * 2 + 1) - delta;
   #else
      randval = rand() % (delta * 2 + 1) - delta;
   #endif

   startvol += randval;
   if (startvol <= 0) {
      startvol = 1;
   }
   if (startvol > 127) {
      startvol = 127;
   }

   return (char)startvol;
}



//////////////////////////////
//
// processCrescDecresc -- adjust attack velocities based on the
//    crescendos and decrescendos found in the file.
//

void processCrescDecresc(HumdrumFile& infile, Array<Array<char> >& dynamics, 
   Array<Array<char> >& crescendos) {

   int i;
   for (i=0; i<dynamics.getSize(); i++) {
      interpolateDynamics(infile, dynamics[i], crescendos[i]);
   }
}



//////////////////////////////
//
// interpolateDynamics --
//

void interpolateDynamics(HumdrumFile& infile, Array<char>& dyn, 
      Array<char>& cresc) {
   int direction = 0;
   int i;
   int ii;
  
   for (i=0; i<dyn.getSize(); i++) {
      if (cresc[i] != 0) {
         if (cresc[i] > 0) {
            direction = +1;
         } else if (cresc[i] < 0) {
            direction = -1;
         } else {
            direction = 0;
         }
         ii = findtermination(dyn, cresc, i);
         generateInterpolation(infile, dyn, cresc, i, ii, direction);

         // skip over calm water:
         i = ii - 1;
      }

   }
}



//////////////////////////////
//
// generateInterpolation -- do the actual interpolation work.
//

void generateInterpolation(HumdrumFile& infile, Array<char>& dyn, 
      Array<char>& cresc, int startline, int stopline, int direction) {

   double startbeat = infile[startline].getAbsBeat();  
   double stopbeat  = infile[stopline].getAbsBeat();

   if (startbeat == stopbeat) {
      // nothing to do
      return;
   }

   int startvel = dyn[startline];
   int stopvel  = dyn[stopline];
   if (startvel < 0)   startvel = -startvel;
   if (stopvel  < 0)   stopvel  = -stopvel;

   if (stopvel == startvel) {
      if (direction > 0) {
         stopvel += 10;
      } else {
         stopvel -= 10;
      }
   } else if (stopvel>startvel && direction<0) {
      stopvel = startvel - 10;
   } else if (stopvel<startvel && direction>0) {
      stopvel = startvel + 10;
   }

   int i;
   double slope = (double)(stopvel-startvel)/(double)(stopbeat-startbeat);
   double currvel = 0.0;
   double currbeat = 0.0;
   for (i=startline+1; i<stopline && i<dyn.getSize(); i++) { 
      currbeat = infile[i].getAbsBeat();
      currvel  = slope * (currbeat - startbeat) + startvel;
      if (currvel > 127.0) {
         currvel = 127.0;
      }
      if (currvel < 0.0) {
         currvel = 0.0;
      }
      dyn[i] = (char)(currvel+0.5);
   }

}



//////////////////////////////
//
// findtermination --
//

int findtermination(Array<char>& dyn, Array<char>& cresc, int start) {
   int i;
   for (i=start+1; i<dyn.getSize(); i++) {
      if (cresc[i] != 0) {
         break;
      } else if (dyn[i] < 0) {
         break;
      }
   }
   
   if (i>=dyn.getSize()) {
      i = dyn.getSize()-1;
   }
   return i;
}



//////////////////////////////
//
// getNewDynamics --
//

void getNewDynamics(Array<int>& currentdynamic, Array<int>& assignments, 
      HumdrumFile& infile, int line, Array<Array<char> >& crescendos,
      Array<Array<char> >& accentuation) {

   if (infile[line].getType() != E_humrec_data) {
      return;
   }

   int i;
   int j;
   int k;
   int length;
   int track = -1;
   int dval = -1;
   int accent = 0;
   int cresval = 0;
   char buffer[2048] = {0};
   for (i=0; i<infile[line].getFieldCount(); i++) {
      dval = -1;
      cresval = 0;
      accent = 0;
      if (strcmp(infile[line][i], ".") == 0) {
         continue;
      }
      if (strcmp(infile[line].getExInterp(i), "**dynam") != 0) {
         continue;
      }

      // copy the **dynam value, removing any X information:
      length = strlen(infile[line][i]);
      k = 0;
      for (j=0; j<length && j<1024; j++) {
         if (infile[line][i][j] == 'm' ||
             infile[line][i][j] == 'p' ||
             infile[line][i][j] == 'f'
             ) {
            buffer[k++] = infile[line][i][j];
         } else {
            // do not store character in dynamic string
         }
      }
      buffer[k] = '\0';

      track = infile[line].getPrimaryTrack(i) - 1;

      if (strstr(buffer, "ffff") != NULL) {
         dval = DYN_FFFF;
      } else if (strcmp(buffer, "fff") == 0) {
         dval = DYN_FFF;
      } else if (strcmp(buffer, "ff") == 0) {
         dval = DYN_FF;
      } else if (strcmp(buffer, "f") == 0) {
         dval = DYN_F;
      } else if (strcmp(buffer, "mf") == 0) {
         dval = DYN_MF;
      } else if (strcmp(buffer, "mp") == 0) {
         dval = DYN_MP;
      } else if (strcmp(buffer, "p") == 0) {
         dval = DYN_P;
      } else if (strcmp(buffer, "pp") == 0) {
         dval = DYN_PP;
      } else if (strcmp(buffer, "ppp") == 0) {
         dval = DYN_PPP;
      } else if (strstr(buffer, "pppp") != NULL) {
         dval = DYN_PPPP;
      }

      // cannot have both > and < on the same line.
      if (strchr(infile[line][i], '<') != NULL) {
         cresval = +1;
      } else if (strchr(infile[line][i], '>') != NULL) {
         cresval = -1;
      }

      // look for accentuatnon
      if (strchr(infile[line][i], 'v') != NULL) {
         accent = 'v';
      }
      
      if (dval > 0 || cresval !=0) {
         for (j=0; j<assignments.getSize(); j++) {
            if (assignments[j] == track) {
               // mark new dynamics with a minus sign 
               // which will be removed after cresc/decresc processing
               if (dval > 0) {
                  currentdynamic[j] = -dval;
               }
               if (cresval != 0) {
                  crescendos[j][line] = cresval;
               }
               if (accent != 0) {
                  accentuation[j][line] = accent;
               }
            }
         }
      }
   }

}



//////////////////////////////
//
// getDynamicAssignments --
//

void getDynamicAssignments(HumdrumFile& infile, Array<int>& assignments) {
   assignments.setSize(infile.getMaxTracks());
   assignments.setAll(-1);

   // *staff assignment assumed to be all on one line, and before
   // any note data.

   int staffline = -1;
   int exinterp = -1;
   int i;
   int j;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_data) {
         break;
      }
      if (infile[i].getType() == E_humrec_interpretation) {
         if (strncmp(infile[i][0], "**", 2) == 0) {
            exinterp = i;
            continue;
         }
         if (strstr(infile[i][0], "staff") != NULL) {
            staffline = i;
            break;
         }
      }
   }

   // first assume that there are no *staff assignments and setup
   // the default values.

   int currdyn = -1;
   for (i=infile[exinterp].getFieldCount()-1; i>=0; i--) {
      if (strcmp(infile[exinterp][i], "**dynam") == 0) {
         currdyn = i;
      }
      if (currdyn >= 0) {
         assignments[i] = infile[exinterp].getPrimaryTrack(currdyn)-1;
      } else {
         assignments[i] = -1;
      }
   }

   if (staffline == -1) {
      // there is no staff assignments in the file, so attach any
      // dynamics to the first **kern spine on the left.

      return;
   }

   // there is a *staff assignment line, so follow the directions found there

   Array<Array<int> > staffvalues;
   getStaffValues(infile, staffline, staffvalues);

   int k;
   // assignments.setSize(infile.getMaxTracks());
   for (i=0; i<infile[exinterp].getFieldCount(); i++) {
      if (strcmp(infile[exinterp][i], "**kern") != 0) {
         continue;
      }
      for (j=0; j<infile[exinterp].getFieldCount(); j++) {
         if (strcmp(infile[exinterp][j], "**dynam") != 0) {
            continue;
         }
         for (k=0; k<staffvalues[j].getSize(); k++) {
            if (staffvalues[i][0] == staffvalues[j][k]) {
               assignments[infile[exinterp].getPrimaryTrack(i)-1] = 
                  infile[exinterp].getPrimaryTrack(j)-1;
            }
         }
      }
   }

   // for (i=0;i<assignments.getSize();i++) {
   //    cout << "ASSIGNMENT " << i << " = " << assignments[i] << endl;
   // }

}




//////////////////////////////
//
// getStaffValues --
//

void getStaffValues(HumdrumFile& infile, int staffline, 
      Array<Array<int> >& staffvalues) {

   staffvalues.setSize(infile[staffline].getFieldCount());

   int i;
   for (i=0; i<staffvalues.getSize(); i++) {
      staffvalues[i].setSize(0);
   }

   int value;
   const char* cptr;
   char* newcptr;
   for (i=0; i<infile[staffline].getFieldCount(); i++) {
      if (strncmp(infile[staffline][i], "*staff", 6) == 0) {
         if (strchr(infile[staffline][i], '/') == NULL) {
            if (sscanf(infile[staffline][i], "*staff%d", &value) == 1) {
               staffvalues[i].append(value);
            }
         } else {
            // more than one number in the staff assignment
            cptr = &(infile[staffline][i][6]);
            while (strlen(cptr) > 0) {
               if (!isdigit(cptr[0])) {
                  break;
               }
               value = strtol(cptr, &newcptr, 10);
               staffvalues[i].append(value);
               cptr = newcptr;
               if (cptr[0] != '/') {
                  break;
               } else {
                  cptr = cptr + 1;
               }
            }
         }
      }
   }
}



//////////////////////////////
//
// getMillisecondTime -- return the time in milliseconds found
//    on the current line in the file.
//

int getMillisecondTime(HumdrumFile& infile, int line) {
   double output = -100;
   int flag;
   int i;

   while ((line < infile.getNumLines()) && 
          (infile[line].getType() != E_humrec_data)) {
      line++;
   }
   if (line >= infile.getNumLines() - 1) {
      return getFileDurationInMilliseconds(infile);
   }

   for (i=0; i<infile[line].getFieldCount(); i++) {
      if (strcmp(infile[line].getExInterp(i), "**time") == 0) {
         if (strcmp(infile[line][i], ".") == 0) {
            cout << "Error on line " << line + 1 << ": no time value" << endl;
            exit(1);
         }         
         //if (strcmp(infile[line][i], ".") == 0) {
         //   output = -1.0;
         //} else {
         //   flag = sscanf(infile[line][i], "%lf", &output);
         //}
         flag = sscanf(infile[line][i], "%lf", &output);
         break;
      }
   }

   if (timeinsecQ) {
      output *= 1000.0;
   }

   if (output < 0.0) {
      return -1;
   } else {
      return (int)(output + 0.5);
   }
}



//////////////////////////////
//
// getMillisecondDuration --
//

int getMillisecondDuration(HumdrumFile& infile, int row, int col, int subcol) {
   char buffer1[1024] = {0};

   int startime = getMillisecondTime(infile, row);

   double output = -100.0;

   double duration = 0.0;

   if (strchr(buffer1, '[')) {
      // total tied note durations
      duration = infile.getTiedDuration(row, col, subcol);
   } else {
      infile[row].getToken(buffer1, col, subcol); 
      duration = rhysc * Convert::kernToDuration(buffer1);
   }

   double stopbeat = duration + infile[row].getAbsBeat();

   int i, j;
   for (i=row+1; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_data) {
         continue;
      }
      if (infile[i].getAbsBeat() >= (stopbeat-0.0002)) {
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (strcmp(infile[i].getExInterp(j), "**time") == 0) {
               sscanf(infile[i][j], "%lf", &output);
               break;
            }
         }
         break;
      }
   }

   if (timeinsecQ) {
      output *= 1000.0;
   }


   if (output - startime < 0.0) {
      return 0;
   } else {
      return (int)(output - startime + 0.5);
   }
}



//////////////////////////////
//
// getFileDurationInMilliseconds --
//

int getFileDurationInMilliseconds(HumdrumFile& infile) {

   int lastdataline = infile.getNumLines() - 1;
   while ((lastdataline > 0) && 
          (infile[lastdataline].getType() != E_humrec_data)) {
      lastdataline--;
   } 

   double ctime = getMillisecondTime(infile, lastdataline);
   double cbeat = infile[lastdataline].getAbsBeat();
   double nbeat = infile[infile.getNumLines()-1].getAbsBeat();

   int preindex = -1;

   int i;
   for (i=lastdataline-1; i>=0; i--) {
      if (infile[i].getType() != E_humrec_data) {
         continue;
      }

      preindex = i;
      break;
   }

   if (preindex < 0) {
       return -1;
   }

   double pbeat = infile[preindex].getAbsBeat();
   double ptime = -1.0;

   for (i=0; i<infile[preindex].getFieldCount(); i++) {
      if (strcmp("**time", infile[preindex].getExInterp(i)) != 0) {
         continue;
      }

      sscanf(infile[preindex][i], "%lf", &ptime);
      break;
   }

   if (ptime < 0.0) {
      return -1;
   }

   if (timeinsecQ) {
      ptime *= 1000.0;
   }

   double db2 = nbeat - cbeat;
   double db1 = cbeat - pbeat;
   double dt1 = ctime - ptime;

   // cout << "<< DB1 = " << db1 << " >> ";
   // cout << "<< DB2 = " << db2 << " >> ";
   // cout << "<< DT1 = " << dt1 << " >> ";

   double result = ctime + db2 * dt1 / db1;

   return (int)(result + 0.5);
}


///////////////////////////////////////////////////////////////////////////
//
// PerfViz related functions
//

ostream& operator<<(ostream& out, PerfVizNote& note) {

   int anchor		= ++note.sid;
   int measure 		= note.bar;
   int beat   		= note.beat;
   int velocity		= note.vel;
   int onset		= note.ontick;   // tick on time for note
   int offset		= note.offtick;  // tick off time for note
   int adjoffset	= offset;        // not taking pedalling into account
   double absbeaton     = note.absbeat;
   double absbeatoff	= note.absbeat + note.scoredur;
   char pitchname[1024] = {0};
   Convert::base40ToPerfViz(pitchname, note.pitch);
   double soffset	= note.beatfrac;
   double dur		= note.beatdur;
   
   out << "snote(n"
       << anchor	<< ","
       << pitchname	<< ","
       << measure	<< ":"
       << beat		<< ",";

   printRational(out, soffset);
   out << ",";
   printRational(out, dur);
   out << ",";

   out << absbeaton	<< ","
       << absbeatoff	<< ","
       << "[])-";
   out << "note("
       << anchor	<< ","
       << pitchname	<< ","
       << onset		<< ","
       << offset	<< ","
       << adjoffset	<< ","
       << velocity	
       << ").\n";

   return out;
}



//////////////////////////////
//
// printRational --
//

void printRational(ostream& out, double value) {
   if (value <= 0.0002) {
      out << "0";
      return;
   }


   if (fabs(value-0.5) < 0.0002) {
      out << "1/2";
   } else if (fabs(value - 0.25) < 0.0002) {
      out << "1/16";
   } else if (fabs(value - 1.5) < 0.0002) {
      out << "3/2";
   } else if (fabs(value - 2.5) < 0.0002) {
      out << "5/2";
   } else if (fabs(value - 0.75) < 0.0002) {
      out << "3/16";
   } else if (fabs(value - 0.16667) < 0.0002) {
      out << "1/8/3";
   } else if (fabs(value - int(value)) <= 0.0002) {
      out << int(value);
   } else if (fabs(value - int(value)) >= 0.9998) {
      out << int(value)+1;
   } else if (value == 1.0) {
      out << "1";
   } else if (fabs(value - 0.3333) < 0.0002) {
      out << "1/4/3";
   } else if (fabs(value - 0.6667) < 0.0002) {
      out << "2/4/3";
   } else {
      out << "{{" << value << "}}";
   }

}



//////////////////////////////
//
// writePerfVizMatchFile --
//

void writePerfVizMatchFile(const char* filename, SSTREAM& contents) {
   ofstream outputfile(filename);
   contents   << ends;
   outputfile << contents.CSTRING;
   outputfile.close();
}


//////////////////////////////
//
// printPerfVizKey --
//

void printPerfVizKey(int key) {
   int octave = key / 40;
   int diatonic = Convert::base40ToDiatonic(key) % 7;
   int accidental = Convert::base40ToAccidental(key);

   if (PVIZ == NULL) {
      return;
   }

   PVIZ[0] << "info(keySignature,[";
   switch (diatonic) {
      case 0:  PVIZ[0] << "c"; break;
      case 1:  PVIZ[0] << "d"; break;
      case 2:  PVIZ[0] << "e"; break;
      case 3:  PVIZ[0] << "f"; break;
      case 4:  PVIZ[0] << "g"; break;
      case 5:  PVIZ[0] << "a"; break;
      case 6:  PVIZ[0] << "b"; break;
      default: PVIZ[0] << "X";
   }
   switch (accidental) {
      case -2: PVIZ[0] << "bb"; break;
      case -1: PVIZ[0] << "b"; break;
      case  0: PVIZ[0] << "n"; break;
      case  1: PVIZ[0] << "#"; break;
      case  2: PVIZ[0] << "##"; break;
   }
   PVIZ[0] << ",";
   if (octave == 3) {
      PVIZ[0] << "major";
   } else if (octave == 4) {
      PVIZ[0] << "minor";
   } else {
      PVIZ[0] << "locrian";
   }
   PVIZ[0] << "]).\n";
}



//////////////////////////////
//
// printPerfVizTimeSig --
//

void printPerfVizTimeSig(int tstop, int tsbottom) {
   if (PVIZ == NULL) {
      return;
   }

   PVIZ[0] << "info(timeSignature," << tstop << "/" << tsbottom << ").\n";

}



//////////////////////////////
//
// printPerfVizTempo --
//

void printPerfVizTempo(double approxtempo) {
   if (PVIZ == NULL) {
      return;
   }
   
   PVIZ[0] << "info(approximateTempo," << approxtempo << ").\n";

}



//////////////////////////////
//
// storeTimbres --
//

void storeTimbres(Array<SigString>& name, Array<int>& value, 
      Array<int>& volumes, const char* string) {
   PerlRegularExpression pre;
   Array<Array<char> > tokens;
   pre.getTokens(tokens, "\\s*;\\s*", string);
   name.setSize(tokens.getSize());
   value.setSize(tokens.getSize());
   volumes.setSize(tokens.getSize());
   name.setSize(0);
   value.setSize(0);
   volumes.setSize(0);

   SigString tempstr;
   int temp;
   int i;
   for (i=0; i<tokens.getSize(); i++) {

      if (pre.search(tokens[i], "(.*)\\s*:\\s*i?(\\d+),v(\\d+)", "")) {
         temp = atoi(pre.getSubmatch(2));
         value.append(temp);
         temp = atoi(pre.getSubmatch(3));
         volumes.append(temp);
         tempstr = pre.getSubmatch(1);
         name.append(tempstr);

      } else if (pre.search(tokens[i], "(.*)\\s*:\\s*(\\d+)", "")) {
         temp = atoi(pre.getSubmatch(2));
         value.append(temp);
         tempstr = pre.getSubmatch(1);
         name.append(tempstr);
      }

   }
}



//////////////////////////////
//
// getKernTracks --  Return a list of the **kern primary tracks found
//     in the Humdrum data.  Currently all tracks are independent parts.
//     No grand staff parts are considered if the staves are separated 
//     into two separate spines.
//
//

void getKernTracks(Array<int>& tracks, HumdrumFile& infile) {
   tracks.setSize(infile.getMaxTracks());
   tracks.setSize(0);
   int i;
   for (i=1; i<=infile.getMaxTracks(); i++) {
      if (strcmp(infile.getTrackExInterp(i), "**kern") == 0) {
         tracks.append(i);
      }
   }
}



// md5sum: e9a407df34d2637d3f9f4a991e694888 hum2mid.cpp [20121112]
