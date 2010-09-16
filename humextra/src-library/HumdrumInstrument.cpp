//
// Copyright 2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Nov 25 14:18:01 PST 2000
// Last Modified: Sun Mar 24 11:50:53 PST 2002 (fixed some instrument names)
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Filename:      ...sig/src/sigInfo/HumdrumInstrument.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/HumdrumInstrument.cpp
// Syntax:        C++ 
//
// Description:   Special enumeration class for processing Humdrum 
//                instrument names.
//


#include "gminstruments.h"
#include "HumdrumInstrument.h"

#include <string.h>

#ifndef OLDCPP
   using namespace std;
#endif

typedef long TEMP64BITFIX;

// declare static variables
SigCollection< _HumdrumInstrument > HumdrumInstrument::data(0);
int HumdrumInstrument::classcount = 0;


//////////////////////////////
//
// HumdrumInstrument::HumdrumInstrument --
//

HumdrumInstrument::HumdrumInstrument(void) { 
   if (classcount == 0) {
      initialize();
   }
   classcount++;
   index = -1;
}



//////////////////////////////
//
// HumdrumInstrument::HumdrumInstrument --
//

HumdrumInstrument::HumdrumInstrument(const char* Hname) { 
   if (classcount == 0) {
      initialize();
   }

   index = find(Hname);
}



//////////////////////////////
//
// HumdrumInstrument::~HumdrumInstrument --
//

HumdrumInstrument::~HumdrumInstrument() { 
   index = -1;
}



/////////////////////////////
//
// HumdrumInstrument::getGM --
//

int HumdrumInstrument::getGM(void) { 
   if (index > 0) {
      return data[index].gm;
   } else {
      return -1;
   }
}



/////////////////////////////
//
// HumdrumInstrument::getGM --
//

int HumdrumInstrument::getGM(const char* Hname) { 
   if (strncmp(Hname, "*I", 2) == 0) {
      Hname = &Hname[2];
   }

   int tindex = find(Hname);
   if (tindex > 0) {
      return data[tindex].gm;
   } else {
      return -1;
   }
}



//////////////////////////////
//
// HumdrumInstrument::getName --
//

const char* HumdrumInstrument::getName(void) { 
   if (index > 0) {
      return data[index].name;
   } else {
      return "";
   }
}



//////////////////////////////
//
// HumdrumInstrument::getName --
//

const char* HumdrumInstrument::getName(const char* Hname) { 
   if (strncmp(Hname, "*I", 2) == 0) {
      Hname = &Hname[2];
   }
   int tindex = find(Hname);
   if (tindex > 0) {
      return data[tindex].name;
   } else {
      return "";
   }
}



//////////////////////////////
//
// HumdrumInstrument::getHumdrum --
//

const char* HumdrumInstrument::getHumdrum(void) { 
   if (index > 0) {
      return data[index].humdrum;
   } else {
      return "";
   }
}



//////////////////////////////
//
// HumdrumInstrument::setGM --
//

int HumdrumInstrument::setGM(const char* Hname, int aValue) {
   if (aValue < 0 || aValue > 127) {
      return 0;
   }
   int rindex = find(Hname);
   if (rindex > 0) {
      data[rindex].gm = aValue;
   } else {
      // leaky memory for now.
      char *buffer = new char[strlen(Hname) + 1];
      strcpy(buffer, Hname);
      afi(Hname, aValue,Hname);
      sortData();
   }
   return rindex;
}



//////////////////////////////
//
// HumdrumInstrument::setHumdrum --
//

void HumdrumInstrument::setHumdrum(const char* Hname) { 
   if (strncmp(Hname, "*I", 2) == 0) {
      Hname = &Hname[2];
   }
   index = find(Hname);
}



//////////////////////////////////////////////////////////////////////////
//
// private functions
//


//////////////////////////////
//
// HumdrumInstrument::initialize --
//

void HumdrumInstrument::initialize(void) { 
   data.setSize(500);
   data.setSize(0);
   data.allowGrowth();

   afi("accor",	GM_ACCORDION,	"accordion");
   afi("alto",	GM_RECORDER,	"alto");
   afi("archl",	GM_ACOUSTIC_GUITAR_NYLON,	"archlute");
   afi("armon",	GM_HARMONICA,	"harmonica");
   afi("arpa",	GM_ORCHESTRAL_HARP,	"harp");
   afi("bagpI",	GM_BAGPIPE,	"bagpipe (Irish)");
   afi("bagpS",	GM_BAGPIPE,	"bagpipe (Scottish)");
   afi("banjo",	GM_BANJO,	"banjo");
   afi("barit",	GM_CHOIR_AAHS,  	"baritone");
   afi("baset",	GM_CLARINET,	"bassett horn");
   afi("bass",	GM_CHOIR_AAHS,	"bass");
   afi("bdrum",	GM_TAIKO_DRUM,	"bass drum (kit)");
   afi("bguit",	GM_ELECTRIC_BASS_FINGER,	"electric bass guitar");
   afi("biwa",	GM_FLUTE,	"biwa");
   afi("bscan",	GM_CHOIR_AAHS,	"basso cantante");
   afi("bspro",	GM_CHOIR_AAHS,	"basso profondo");
   afi("calam",	GM_OBOE,	"chalumeau");
   afi("calpe",	GM_LEAD_CALLIOPE,	"calliope");
   afi("calto",	GM_CHOIR_AAHS,	"contralto");
   afi("campn",	GM_TUBULAR_BELLS,	"bell");
   afi("cangl",	GM_ENGLISH_HORN,	"english horn");
   afi("caril",	GM_TUBULAR_BELLS,	"carillon");
   afi("castr",	GM_CHOIR_AAHS,	"castrato");
   afi("casts",	GM_WOODBLOCKS,	"castanets");
   afi("cbass",	GM_CONTRABASS,	"contrabass");
   afi("cello",	GM_CELLO,	"violoncello");
   afi("cemba",	GM_HARPSICHORD,	"harpsichord");
   afi("cetra",	GM_VIOLIN,	"cittern");
   afi("chime",	GM_TUBULAR_BELLS,	"chimes");
   afi("chlma",	GM_BASSOON,	"alto shawm");
   afi("chlms",	GM_BASSOON,	"soprano shawm");
   afi("chlmt",	GM_BASSOON,	"tenor shawm");
   afi("clara",	GM_CLARINET,	"alto clarinet (in E-flat)");
   afi("clarb",	GM_CLARINET,	"bass clarinet (in B-flat)");
   afi("clarp",	GM_CLARINET,	"piccolo clarinet");
   afi("clars",	GM_CLARINET,	"soprano clarinet");
   afi("clavi",	GM_CLAVI,	"clavichord");
   afi("clest",	GM_CELESTA,	"celesta");
   afi("colsp",	GM_FLUTE,       	"coloratura soprano");
   afi("cor",	GM_FRENCH_HORN,	"horn");
   afi("cornm",	GM_BAGPIPE,	"French bagpipe");
   afi("corno",	GM_TRUMPET,	"cornett");
   afi("cornt",	GM_TRUMPET,	"cornet");
   afi("crshc",	GM_REVERSE_CYMBAL,	"crash cymbal (kit)");
   afi("ctenor",	GM_CHOIR_AAHS,	"counter-tenor");
   afi("ctina",	GM_ACCORDION,	"concertina");
   afi("drmsp",	GM_FLUTE,	"dramatic soprano");
   afi("dulc",	GM_DULCIMER,	"dulcimer");
   afi("eguit",	GM_ELECTRIC_GUITAR_CLEAN,	"electric guitar");
   afi("fag_c",	GM_BASSOON,	"contrabassoon");
   afi("fagot",	GM_BASSOON,	"bassoon");
   afi("false",	GM_RECORDER,	"falsetto");
   afi("feme",	GM_CHOIR_AAHS,	"female voice");
   afi("fife",	GM_BLOWN_BOTTLE,	"fife");
   afi("fingc",	GM_REVERSE_CYMBAL,	"finger cymbal");
   afi("flt",	GM_FLUTE,	"flute");
   afi("flt_a",	GM_FLUTE,	"alto flute");
   afi("flt_b",	GM_FLUTE,	"bass flute");
   afi("fltda",	GM_RECORDER,	"alto recorder");
   afi("fltdb",	GM_RECORDER,	"bass recorder");
   afi("fltdn",	GM_RECORDER,	"sopranino recorder");
   afi("fltds",	GM_RECORDER,	"soprano recorder");
   afi("fltdt",	GM_RECORDER,	"tenor recorder");
   afi("flugh",	GM_FRENCH_HORN,	"flugelhorn");
   afi("forte",	GM_HONKYTONK_PIANO,	"fortepiano");
   afi("glock",	GM_GLOCKENSPIEL,	"glockenspiel");
   afi("gong", 	GM_STEEL_DRUMS,	"gong");
   afi("guitr",	GM_ACOUSTIC_GUITAR_NYLON,	"guitar");
   afi("hammd",	GM_DRAWBAR_ORGAN,	"Hammond electronic organ");
   afi("heltn",	GM_CHOIR_AAHS,	"Heldentenor");
   afi("hichi",	GM_OBOE,	"hichiriki");
   afi("hurdy",	GM_LEAD_CALLIOPE,	"hurdy-gurdy");
   afi("kit",	GM_SYNTH_DRUM,	"drum kit");
   afi("kokyu",	GM_FIDDLE,	"kokyu (Japanese spike fiddle)");
   afi("komun",	GM_KOTO,	"komun'go (Korean long zither)");
   afi("koto",	GM_KOTO,	"koto (Japanese long zither)");
   afi("kruma",	GM_TRUMPET,	"alto crumhorn");
   afi("krumb",	GM_TRUMPET,	"bass crumhorn");
   afi("krums",	GM_TRUMPET,	"soprano crumhorn");
   afi("krumt",	GM_TRUMPET,	"tenor crumhorn");
   afi("liuto",	GM_ACOUSTIC_GUITAR_NYLON,	"lute");
   afi("lyrsp",	GM_FLUTE,       	"lyric soprano");
   afi("lyrtn",	GM_FRENCH_HORN,	"lyric tenor");
   afi("male",	GM_CHOIR_AAHS,  	"male voice");
   afi("mando",	GM_ACOUSTIC_GUITAR_NYLON,	"mandolin");
   afi("marac",	GM_AGOGO,	"maracas");
   afi("marim",	GM_MARIMBA,	"marimba");
   afi("mezzo",	GM_CHOIR_AAHS,  	"mezzo soprano");
   afi("nfant",	GM_CHOIR_AAHS,  	"child's voice");
   afi("nokan",	GM_SHAKUHACHI,	"nokan (a Japanese flute)");
   afi("oboeD",	GM_ENGLISH_HORN,	"oboe d'amore");
   afi("oboe",	GM_OBOE,	"oboe");
   afi("ocari",	GM_OCARINA,	"ocarina");
   afi("organ",	GM_CHURCH_ORGAN,	"pipe organ");
   afi("panpi",	GM_PAN_FLUTE,	"panpipe");
   afi("piano",	GM_ACOUSTIC_GRAND_PIANO,	"pianoforte");
   afi("piatt",	GM_REVERSE_CYMBAL,	"cymbals");
   afi("picco",	GM_PICCOLO,	"piccolo");
   afi("pipa",	GM_ACOUSTIC_GUITAR_NYLON,	"Chinese lute");
   afi("porta",	GM_TANGO_ACCORDION,	"portative organ");
   afi("psalt",	GM_CLAVI,	"psaltery (box zither)");
   afi("qin",	GM_CLAVI,	"qin, ch'in (Chinese zither)");
   afi("quitr",	GM_ACOUSTIC_GUITAR_NYLON,	"gittern");
   afi("rackt",	GM_TRUMPET,	"racket");
   afi("rebec",	GM_ACOUSTIC_GUITAR_NYLON,	"rebec");
   afi("recit",	GM_CHOIR_AAHS,  	"recitativo");
   afi("reedo",	GM_REED_ORGAN,	"reed organ");
   afi("rhode",	GM_ELECTRIC_PIANO_1,	"Fender-Rhodes electric piano");
   afi("ridec",	GM_REVERSE_CYMBAL,	"ride cymbal (kit)");
   afi("sarod",	GM_SITAR,	"sarod");
   afi("sarus",	GM_TUBA,	"sarrusophone");
   afi("saxA",	GM_ALTO_SAX,	"E-flat alto saxophone");
   afi("saxB",	GM_BARITONE_SAX,	"B-flat bass saxophone");
   afi("saxC",	GM_BARITONE_SAX,	"E-flat contrabass saxophone");
   afi("saxN",	GM_SOPRANO_SAX,	"E-flat sopranino saxophone");
   afi("saxR",	GM_BARITONE_SAX,	"E-flat baritone saxophone");
   afi("saxS",	GM_SOPRANO_SAX,	"B-flat soprano saxophone");
   afi("saxT",	GM_TENOR_SAX,	"B-flat tenor saxophone");
   afi("sdrum",	GM_SYNTH_DRUM,	"snare drum (kit)");
   afi("shaku",	GM_SHAKUHACHI,	"shakuhachi");
   afi("shami",	GM_SHAMISEN,	"shamisen (Japanese fretless lute)");
   afi("sheng",	GM_SHANAI,	"mouth organ (Chinese)");
   afi("sho",	GM_SHANAI,	"mouth organ (Japanese)");
   afi("sitar",	GM_SITAR,	"sitar");
   afi("soprn",	GM_CHOIR_AAHS,  	"soprano");
   afi("spshc",	GM_REVERSE_CYMBAL,	"splash cymbal (kit)");
   afi("steel",	GM_STEEL_DRUMS,	"steel-drum");
   afi("sxhA",	GM_ALTO_SAX,	"E-flat alto saxhorn");
   afi("sxhB",	GM_BARITONE_SAX,	"B-flat bass saxhorn");
   afi("sxhC",	GM_BARITONE_SAX,	"E-flat contrabass saxhorn");
   afi("sxhR",	GM_BARITONE_SAX,	"E-flat baritone saxhorn");
   afi("sxhS",	GM_SOPRANO_SAX,	"B-flat soprano saxhorn");
   afi("sxhT",	GM_TENOR_SAX,	"B-flat tenor saxhorn");
   afi("synth",	GM_ELECTRIC_PIANO_2,	"keyboard synthesizer");
   afi("tabla",	GM_MELODIC_DRUM,	"tabla");
   afi("tambn",	GM_TINKLE_BELL,	"tambourine");
   afi("tambu",	GM_MELODIC_DRUM,	"tambura");
   afi("tanbr",	GM_MELODIC_DRUM,	"tanbur");
   afi("timpa",	GM_MELODIC_DRUM,	"timpani");
   afi("tiorb",	GM_ACOUSTIC_GUITAR_NYLON,	"theorbo");
   afi("tom",	GM_TAIKO_DRUM,	"tom-tom drum");
   afi("trngl",	GM_TINKLE_BELL,	"triangle");
   afi("tromb",	GM_TROMBONE,	"bass trombone");
   afi("tromp",	GM_TRUMPET,	"trumpet");
   afi("tromt",	GM_TROMBONE,	"tenor trombone");
   afi("tuba",	GM_TUBA,	"tuba");
   afi("ud",	GM_ACOUSTIC_GUITAR_NYLON,	"ud");
   afi("ukule",	GM_ACOUSTIC_GUITAR_NYLON,	"ukulele");
   afi("vibra",	GM_VIBRAPHONE,	"vibraphone");
   afi("vina",	GM_SITAR,	"vina");
   afi("viola",	GM_VIOLA,	"viola");
   afi("violb",	GM_CONTRABASS,	"bass viola da gamba");
   afi("viold",	GM_VIOLA,	"viola d'amore");
   afi("violn",	GM_VIOLIN,	"violin");
   afi("violp",	GM_VIOLIN,	"piccolo violin");
   afi("viols",	GM_VIOLIN,	"treble viola da gamba");
   afi("violt",	GM_CELLO,	"tenor viola da gamba");
   afi("vox",	GM_CHOIR_AAHS,  	"generic voice");
   afi("xylo",	GM_XYLOPHONE,	"xylophone");
   afi("zithr",	GM_CLAVI,	"zither");
   afi("zurna",	GM_ACOUSTIC_GUITAR_NYLON,	"zurna");

}



//////////////////////////////
//
// HumdrumInstrument::afi --
//

void HumdrumInstrument::afi(const char* humdrum_name, int midinum,
      const char* EN_name) {
   _HumdrumInstrument x;
   x.name = EN_name;
   x.humdrum = humdrum_name;
   x.gm = midinum;

   data.append(x);
}



//////////////////////////////
//
// HumdrumInstrument::find --
//

int HumdrumInstrument::find(const char* Hname) { 
   void* searchResult;
   _HumdrumInstrument key;
   key.humdrum = Hname;
   key.name = "";
   key.gm = 0;

   searchResult = bsearch(&key, data.getBase(),
         data.getSize(), sizeof(_HumdrumInstrument),
         &data_compare_by_humdrum_name);

   if (searchResult == NULL) {
      return -1;
   } else {
      return (((TEMP64BITFIX)(searchResult)) - ((TEMP64BITFIX)(data.getBase())))/
         sizeof(_HumdrumInstrument);
   }
}


//////////////////////////////
//
// HumdrumInstrument::data_compare_by_humdrum_name --
//

int HumdrumInstrument::data_compare_by_humdrum_name(const void* a,
      const void* b) {
   _HumdrumInstrument& valuea = *((_HumdrumInstrument*)a);
   _HumdrumInstrument& valueb = *((_HumdrumInstrument*)b);
   return strcmp(valuea.humdrum, valueb.humdrum);
}



//////////////////////////////
//
// HumdrumInstrument::sortData --
//

void HumdrumInstrument::sortData(void) {
   qsort(data.getBase(), data.getSize(), sizeof(_HumdrumInstrument), 
      &HumdrumInstrument::data_compare_by_humdrum_name);
}



// md5sum: 01b4e4b69617317d6ad08f45fef73b6b HumdrumInstrument.cpp [20050403]
