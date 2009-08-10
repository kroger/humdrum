//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul  2 23:05:34 PDT 1999
// Last Modified: Thu Jul  8 15:11:58 PDT 1999
// Filename:      ...sig/maint/code/info/Performance/Performance.cpp
// Syntax:        C++
//

#include "Performance.h"


//////////////////////////////
//
// Performance::Performance --
//

Performance::Performance(void) { 
   ticksPerQuarter = 72;
   tempoMultiplier = 1.0;
   zeroNoteStates(); 
   playingQ = 0;
   nextActionTime = 0;
   default_tempo = 80;
   current_tempo = 80;
   current_measure = 0;
}



//////////////////////////////
//
// Performance::~Performance --
//

Performance::~Performance() { 
   // do nothing
}



//////////////////////////////
//
// Performance::getBar -- return the current measure number.
//

int Performance::getBar(void) {
   return getMeasure();
}



//////////////////////////////
//
// Performance::getMeasure -- return the current measure number.
//

int Performance::getMeasure(void) {
   return current_measure;
}



//////////////////////////////
//
// Performance::getTempo -- returns the current tempo of the
//   of the performance.
//

double Performance::getTempo(void) {
   return current_tempo;
}



//////////////////////////////
//
// Performance::getTempoMultiplier -- returns the current tempo multiplier
//   of the of the performance.
//

double Performance::getTempoMultiplier(void) {
   return timeFormat;
}



//////////////////////////////
//
// Performance::getTextEcho -- returns the echo state of the text 
//    in the performance data.
//

int Performance::getTextEcho(void) {
   return echoTextQ;
}



//////////////////////////////
//
// Performance::getTicksPerQuarterNote -- set the number of ticks per
//   quarter notes.
//

double Performance::getTicksPerQuarterNote(int ticks) { 
   return ticksPerQuarter;
}



//////////////////////////////
//
// Performance::gotoBar -- go to the specified bar.  If the new bar is
//     less than or equal to zero, then just set us up at the start;
//     otherwise, search through the list in whatever direction for 
//     the barline.
//

void Performance::gotoBar(int aBar) {
   stop();
   if (aBar <= 0) {
      currentIndex = 0;
      current_measure = 0;
      current_tempo = default_tempo;
   }

   int direction = aBar < current_measure ? -1 : 1;
   current_measure = -1;
   while ((getType() != PERFORM_TYPE_BEGIN) && (getType() != PERFORM_TYPE_END) 
         && (current_measure != aBar)) {
      switch (getType()) {
         case PERFORM_TYPE_TEMPO:
            current_tempo = getTempo();
            break;
         case PERFORM_TYPE_MEASURE:
            current_measure = getBar();
            break;
         default:
            break;
      }

      if (direction > 0) {
         next();
      } else {
         back();
      }
   }
   start();
}



//////////////////////////////
//
// Performance::pause -- pause the performance
//

void Performance::pause(void) {
   playingQ = 0;
}



//////////////////////////////
//
// Performance::perform -- check the time and perform any data
//    that needs to be performed
//

void Performance::perform(void) { 
   if (playingQ != 1 && getType() != PERFORM_TYPE_END) {
      return;
   }

   while (timer.getPeriodCount() >= nextActionTime) {
      play();
      next();
      nextActionTime += getTime();
      if (getType() == PERFORM_TYPE_END) {
         stop();
         return;
      }
   }
}



//////////////////////////////
//
// Performance::play -- performs the current element in the data.
//

void Performance::play(void) {
   switch (getType()) {
      case PERFORM_TYPE_MEASURE:
         current_measure = PerformData::getMeasure();
         break;
      case PERFORM_TYPE_TEMPO:
         current_tempo = PerformData::getTempo() * getTempoMultiplier();
         break;
      case PERFORM_TYPE_MIDI:
         {
         uchar* mididata = (uchar*)PerformData::getData();
         int size = PerformData::getLength();
         MidiOutput::rawsend(mididata, size);
         int channel = mididata[0] & 0x0f;
         if ((mididata[0] & 0xf0) ==  0x90) {
            if (mididata[2] == 0) {
               markOff(channel, mididata[1]);
            } else {
               markOn(channel, mididata[1]);
            }
         } else if ((mididata[0] & 0xf0) == 0x80) {
            markOff(channel, mididata[1]);
         }
         }
         break;
      case PERFORM_TYPE_TEXT:
         if (getTextEcho()) {
            cout << getData();
         }
         break;
      default:
         break;
   }
}



//////////////////////////////
//
// Performance::reset -- position performance to start of
//    data.  do start to start at that position.
//

void Performance::reset(void) { 
   tacet();
   currentIndex = 0;
   nextActionTime = 0;
   current_tempo = default_tempo;
   current_measure = 0;
}



//////////////////////////////
//
// Performance::search -- look for a string in a certain direction
//   and start playing from there.
//

void Performance::search(const char* regexpression, int dir) { 
   tacet();
   if (dir == 0) {
      cout << "Error: search direction cannot be zero" << endl;
      exit(1);
   }
   int result = 0;
   while (!result && !eof() && !bof()) {
      switch (getType()) {
         case PERFORM_TYPE_TEXT:
            result = match(regexpression);
            break;
         case PERFORM_TYPE_MIDI:
            // not searching MIDI data for now
            break;
         case PERFORM_TYPE_MEASURE:
            current_measure = PerformData::getMeasure();
            result = match(regexpression);
            break;
         case PERFORM_TYPE_TEMPO:
            current_tempo = PerformData::getTempo();
            result = match(regexpression);
            break;
         case PERFORM_TYPE_BEGIN:
         case PERFORM_TYPE_END:
            return;
            break;
      }
      if (dir > 0) {
         next();
      } else {
         back();
      }
   }
}



//////////////////////////////
//
// Performance::setTempo -- set the current tempo of the performance.
//

void Performance::setTempo(double aTempo) {
   timer.setTempo(aTempo);
}



//////////////////////////////
//
// Performance::setTempoMultiplier -- set the current tempo multiplier
//     of the performance.
//

void Performance::setTempoMultiplier(double aMultiplier) {
   if (aMultiplier > 0.01) {
      tempoMultiplier = aMultiplier;
   }
}



//////////////////////////////
//
// Performance::setTicksPerQuarterNote -- set the number of ticks
//   per quarter note.
//

void Performance::setTicksPerQuarterNote(int ticks) { 
   if (ticks > 1) {
      ticksPerQuarter = ticks;
   }
}



//////////////////////////////
//
// Performance::start -- start playing the data.
//

void Performance::start(void) { 
   playingQ = 1;     // tell perform() that it can play notes.
   nextActionTime = 0;
   timer.setTempo(current_tempo);
   timer.reset();
}



//////////////////////////////
//
// Performance::stop -- stop playing the data.
//

void Performance::stop(void) { 
   tacet();          // turn off all playing notes
   playingQ = 0;     // tell perform() that it should not do anything.
}



//////////////////////////////
//
// Performance::tacet -- turn off all notes which are registered 
//    as on.
//

void Performance::tacet(void) {
   int i, j;
   for (i=0; i<16; i++) {
      for (j=0; j<128; j++) {
         if (noteState[i][j] != 0) {
            MidiOutput::play(i, j, 0);
            noteState[i][j] = 0;
         }
      }
   }
}



//////////////////////////////
//
// Performance::unpause -- unpause the performance
//

void Performance::unpause(void) {
   nextActionTime = 0;
   timer.reset();
}



///////////////////////////////////////////////////////////////////////////
//
// private functions
//


//////////////////////////////
//
// Performance::markOff -- make a note state off
//

void Performance::markOff(int channel, int key) {
   noteState[channel][key] = 0;
}
   


//////////////////////////////
//
// Performance::markOn -- make a note state on
//

void Performance::markOn(int channel, int key) {
   noteState[channel][key] = 1;
}
   


//////////////////////////////
//
// Performance::zeroNoteStates -- set all note states to off.
//

void Performance::zeroNoteStates(void) {
   int i, j;
   for (i=0; i<16; i++) {
      for (j=0; j<128; j++) {
         noteState[i][j] = 0;
      }
   }
}



// md5sum: 462e53a5c99bdc47e3dfcea87c2ac28a Performance.cpp [20020518]
