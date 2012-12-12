//
// Copyright 2004 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Mar 10 07:49:40 PST 2004
// Last Modified: Sun Apr  4 23:17:36 PDT 2004
// Filename:      ...sig/include/sigInfo/MusicXmlFile.h
// Web Address:   http://sig.sapp.org/include/sigInfo/MusicXmlFile.h
// Syntax:        C++ 
//
// Description:   A class which parses a MusicXML File.
//

#ifndef _MUSICXMLFILE_H_INCLUDED
#define _MUSICXMLFILE_H_INCLUDED

// include Centerpoint XML classes (http://www.cpointc.com/XML):
#include "XMLDocument.h"
#include "XMLDocumentBuilder.h"
#include "XMLUtils.h"
#include "XMLException.h"

#include "Array.h"
#include "HumdrumFile.h"

#ifndef OLDCPP
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #ifdef VISUAL
      #include <strstrea.h>    /* for Windows 95 */
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif

// MusicXmlItem allowed types:

#define MXI_unknown     0
#define MXI_print       1  /* printing suggestions */
#define MXI_divisions   2
#define MXI_measure     3
#define MXI_barline     4
#define MXI_clef        5
#define MXI_key         6  /* key signatures  */
#define MXI_timemet     7  /* metrical display for time signatures */
#define MXI_time        8  /* time signatures */
#define MXI_tempo       9  /* tempo markings  */
#define MXI_forward    10
#define MXI_backup     11
#define MXI_instrument 12 /* instrument name */
#define MXI_text       13  /* text to print in score */
#define MXI_note       14  /* notes & rests */
#define MXI_dynamic    15  /* dynamic marking */
#define MXI_lyric      16  /* text underlay of notes */

#define MAXLYRIC      32  /* maximum lyric number allowed */

typedef struct {
   int line;
   int serial;
} _SerialLine;

class _MusicXmlItem {
   public:
      long   ticktime;
      long   tickdur;
      long   divisions;
      
      int    humline;       // line in humdrum output which item is placed
      int    measureno;
      int    serialnum;
      int    ownerserial;   // for attaching lyrics to later
      int    lyricnum;
      int    voice;
      int    type;
      int    pitch;
      int    ficta;
      CSL::XML::CXMLObject* obj;
      CSL::XML::CXMLObject* chordhead;

      void clear(void)    { divisions = tickdur = ticktime = 0; 
                            ficta = humline = ownerserial = lyricnum = 0;
                            chordhead = obj = NULL; 
                            measureno = serialnum = pitch = voice = type = 0; }
      _MusicXmlItem(void) { clear(); }
     ~_MusicXmlItem()     { clear(); }
};



class _NoteState {  // used in MusicXmlFile::printKernNote()
   public:

   int gracenote;
   int chordnote;
   // lines:
   int natural;
   int tiestart;
   int tieend;
   int slurstart;
   int slurend;
   int elidedslurstart;
   int elidedslurend;
   int phrasestart;
   int phraseend;
   int elidedphrasestart;
   int elidedphraseend;
   // beams:
   int stemdir;		// 1 = up, -1 = down, 0 = unknown/none
   int startbeam;
   int endbeam;
   int hookleft;
   int hookright;
   // articulations:
   int staccato;
   int tenuto;
   int ficta;
   int accent;
   int upbow;
   int downbow;
   int staccatissimo;
   int fermata;

   _NoteState(void) { clear(); }
   
   void clear(void) {
      chordnote         = 0;
      tiestart          = 0;
      tieend            = 0;
      natural           = 0;
      stemdir           = 0;
      startbeam         = 0;
      slurstart         = 0;
      slurend           = 0;
      elidedslurstart   = 0;
      elidedslurend     = 0;
      phrasestart       = 0;
      phraseend         = 0;
      elidedphrasestart = 0;
      elidedphraseend   = 0;
      endbeam           = 0;
      hookleft          = 0;
      hookright         = 0;
      gracenote         = 0;
      ficta             = 0;
      staccato          = 0;
      tenuto            = 0;
      upbow             = 0;
      downbow           = 0;
      accent            = 0;
      staccatissimo     = 0;
      fermata           = 0;
   }
   
   void incorporate(_NoteState& ns) {
      if (stemdir == 0) { stemdir = ns.stemdir; }
      staccato         |= ns.staccato;
      tenuto           |= ns.tenuto;
      staccatissimo    |= ns.staccatissimo;
      accent           |= ns.accent;
      fermata          |= ns.fermata;
      upbow            |= ns.upbow;
      downbow          |= ns.downbow;
   }

};



class MusicXmlFile {
   public:
                 MusicXmlFile             (void);
                 MusicXmlFile             (char* aFile);
                ~MusicXmlFile             ();

      int        getPartCount             (void);
      int        getStaffCount            (void);
      int        getPartStaffCount        (int index);
      char*      getPartId                (char* buffer, int partnum);
      char*      getPartName              (char* buffer, int partnum);
      int        getPartNumberFromId      (const char* buffer);
      ostream&   print                    (ostream& out = cout);
      ostream&   info                     (ostream& out = cout);
      void       read                     (char* aFile);
      static int staffcompare             (const void* A, const void* B);
      static int getMeasureNumber         (CSL::XML::CXMLObject* object);
      static int isGraceNote              (CSL::XML::CXMLObject* object);
      static int getDivisions             (CSL::XML::CXMLObject* object);
      static int getNoteTickDuration      (CSL::XML::CXMLObject* object);
      static int getBackupTickDuration    (CSL::XML::CXMLObject* object);
      static int getForwardTickDuration   (CSL::XML::CXMLObject* object);
      static int getMeasureTickDuration   (CSL::XML::CXMLObject* object);
      static int getStaffCountFromPart    (CSL::XML::CXMLObject* object);
      static int getClefStaffNumber       (CSL::XML::CXMLObject* object);
      static ostream& printGraceNoteRhythm(ostream& out, _MusicXmlItem& item);
      static ostream& printGraceMarker    (ostream& out, _MusicXmlItem& item);
      static char* getLyricText           (char* buffer, 
                                           CSL::XML::CXMLObject* object);
      int        getStaffNumber           (CSL::XML::CXMLObject* object);
      void       fixMidiAccidentals       (void);
      void       humdrumPart              (HumdrumFile& hfile, int staffno,
                                           int debugQ = 0);
      HumdrumFile& createHumdrumFile      (HumdrumFile& hfile);
      void      setOption                 (const char* key, int value);
      int       getOption                 (const char* key);
      void      fixMeasureNumbers         (void);

   protected:
      void        addLyrics               (HumdrumFile& hfile, int staffno);
      void        fixMeasureNumbersForPart(int staffno);
      int         getTimeSignatureTicks   (CSL::XML::CXMLObject* obj, 
                                           int tickinfo);
      static void removeBorderSpaces      (char* buffer);
      static int  getIntFromCharacterData (CSL::XML::CXMLObject* object);
      void        checkPartStaff          (int partnum, int staff);
      void        processSpineChanges     (SSTREAM& newstream, 
                                           SSTREAM& oldstream, int staffno);
      void        fixPickupBarline        (void);
      void        fixPartPickups          (int partno);
      void        addStaffData            (void);
      void        appendAllPartStaves     (int partnum, 
                                           _MusicXmlItem& tempitem);
      void        getInstrumentName       (CSL::XML::CXMLObject* object, 
                                           int partnum, 
                                           CSL::XML::XMLString& idname);
      char*       getCharacterData        (char* buffer, 
                                           CSL::XML::CXMLObject* object);

      // parsing functions
      void      parse                     (void);
      void      traverse                  (CSL::XML::CXMLObject* entry);
      void      process                   (CSL::XML::CXMLObject* entry);
      int       parseElement              (CSL::XML::CXMLObject* entry);
      void      parsePart                 (CSL::XML::CXMLObject* entry, 
                                           int partnum);
      void      parseBackup               (CSL::XML::CXMLObject* object, 
                                           int partnum, long& ticktime);
      void      parseForward              (CSL::XML::CXMLObject* object, 
                                           int partnum, long& ticktime);
      void      parseMeasure              (CSL::XML::CXMLObject* entry, 
                                           int partnum, long& ticktime);
      void      parsePrint                (CSL::XML::CXMLObject* entry, 
                                           int partnum, int ticktime);
      void      parseBarline              (CSL::XML::CXMLObject* entry, 
                                           int partnum, int ticktime);
      void      parseNote                 (CSL::XML::CXMLObject* entry, 
                                           int partnum, long& ticktime);
      void      parseDirection            (CSL::XML::CXMLObject* object, 
                                           int partnum, long& ticktime);
      void      parseSound                (CSL::XML::CXMLObject* entry, 
                                           int partnum, long& ticktime);
      int       parsePitch                (CSL::XML::CXMLObject* entry);
      int       parseFicta                (CSL::XML::CXMLObject* entry);
      void      parseAttributes           (CSL::XML::CXMLObject* entry, 
                                           int partnum, long& ticktime);
      void      sortStaff                 (int partnum);
      void      parseDirectionType        (CSL::XML::CXMLObject* object, 
                                           int partnum, long& ticktime,
                                           CSL::XML::CXMLElement* rootelement);
      void      printDynamic              (ostream& out, int staffno, 
                                           int index);
      void      printInstrument           (ostream& out, int staffno, 
                                           int index);
      void      printText                 (ostream& out, int staffno, 
                                           int index);
      int       printSystemBreak          (ostream& out, 
                                           CSL::XML::CXMLObject* object);
      int       printPageBreak            (ostream& out, 
                                           CSL::XML::CXMLObject* object);

      // kern conversion functions
      int       printKernNote             (ostream& out, int staffno, 
                                           int index, int divisions);
      void      getKernNoteProperties     (_NoteState& ns, 
                                           CSL::XML::CXMLObject* object, 
                                           int tickdur);
      char*     getKernKey                (char* buffer, 
                                           CSL::XML::CXMLObject* object);
      char*     printKernClef             (char* buffer, 
                                           CSL::XML::CXMLObject* object);
      char*     getKernTimeSig            (char* buffer, 
                                           CSL::XML::CXMLObject* object);
      char*     getKernTimeMet            (char* buffer, 
                                           CSL::XML::CXMLObject* object);
      char*     getKernMetronome          (char* buffer, 
                                           CSL::XML::CXMLObject* object);
      int       printMeasureStyle         (char* buffer, int partnum, 
                                           int index);
      int       getNextVoiceTime          (int staffno, int index, int voice);
      int       getInterpRestDuration     (char* interpbuffer, int interptick,
                                           int divisions);

      // printing functions
      ostream&  printTraverse             (ostream& out, 
                                           CSL::XML::CXMLObject* entry);
      ostream&  printEntry                (ostream& out, 
                                           CSL::XML::CXMLObject* entry);
      ostream&  printElement              (ostream& out, 
                                           CSL::XML::CXMLObject* entry);
      ostream&  printCloseEvent           (ostream& out, 
                                           CSL::XML::CXMLObject* entry);
      ostream&  printCharacterData        (ostream& out, 
                                           CSL::XML::CXMLObject* entry);
      ostream&  printComment              (ostream& out, 
                                           CSL::XML::CXMLObject* entry);
      ostream&  printGenericData          (ostream& out, 
                                           CSL::XML::CXMLObject* entry);
      ostream&  printCDataSection         (ostream& out, 
                                           CSL::XML::CXMLObject* entry);
      ostream&  printDocument             (ostream& out, 
                                           CSL::XML::CXMLObject* entry);
      ostream&  printString               (ostream& out, 
                                           CSL::XML::CXMLCharacterData* 
                                           characters);
   private:
      CSL::XML::CXMLDocument*      xmldocument;
      Array<CSL::XML::CXMLObject*> parts;      // pointer to part data in file
      Array<int>                   partstaves; // list of staves in each part
      Array<int>                   partoffset; // starting staff of each part
      Array<Array<_MusicXmlItem> > partdata;   // part data by staff
      int parsedivisions;     // used for global divisions in parsing input.
      int parseserialnum;     // used for sorting items in parts.
      int partvoiceticktime2; // used for adding null tokens to second
                              // voice in spine.
      int partvoiceticktime1; // used for adding interpreted rests in spine 1.
      CSL::XML::CXMLObject* lastchordhead;
      Array<int> partdynamics;  // used to identify if a part contains dynamics
      int humline;              // used to print lyrics

      CSL::XML::XMLString getAttributeValue(CSL::XML::CXMLObject* object, 
                      const char* name);

      // Humdrum Conversion options:
      int humdrumStem;      // 0 = don't encode stems, 1 = encode stems
      int humdrumBeam;      // 0 = don't encode beams, 1 = encode beams
      int humdrumDynamics;  // 0 = don't encode **dynam spines, 1 = do
      int humdrumLyrics;    // 0 = don't encode **text spines,  1 = do
};


ostream& operator<<(ostream& out, MusicXmlFile& aMusicXmlFile);

int seriallinesort(const void* a, const void* b);

#endif /* _MUSICXMLFILE_H_INCLUDED */


// md5sum: 2646aedfc2dbd8b5d642977cd3a4ca9a MusicXmlFile.h [20050403]
