//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Nov 27 14:10:32 PST 1999
// Last Modified: Wed Dec  1 11:35:37 PST 1999
// Filename:      ...sig/maint/code/info/MidiPerform/MidiPerform.cpp
// Syntax:        C++ 
//
// Description:   A class which performs a MIDI file with various
//                types of tempo and volume controls built in.
//                Works similar to a sequencer program, but not designed
//                for editing of the data.
//

#include "MidiPerform.h"


//////////////////////////////
//
// MidiPerform::MidiPerform --
//

MidiPerform::MidiPerform(void) { 
   tempo = 120.0;
   performanceTimer.setTempo(tempo);
   beatTimer.setTempo(tempo);
   pauseLocation = 0.0;
   beatTimes.setSize(100);
   beatTimes.reset();
   readIndex = new int[1];
   readIndex = 0;
   tempoMethod = TEMPO_METHOD_AUTOMATIC;
   amp = 1.0;
}


MidiPerform::MidiPerform(char* aFile) { 
   tempo = 120.0;
   performanceTimer.setTempo(tempo);
   beatTimer.setTempo(tempo);
   pauseLocation = 0.0;
   beatTimes.setSize(100);
   beatTimes.reset();
   readIndex = new int[1];
   readIndex = 0;
   tempoMethod = TEMPO_METHOD_AUTOMATIC;
   amp = 1.0;
  
   read(aFile);
}


//////////////////////////////
//
// MidiPerform::~MidiPerform --
//

MidiPerform::~MidiPerform() { 
   // do nothing
}



//////////////////////////////
//
// MidiPerform::beat -- 
//

void MidiPerform::beat(void) { 
   double aNewTime = millisecTimer.getTime();
   beatTimes.insert(aNewTime);

   switch (getTempoMethod()) {
      case TEMPO_METHOD_AUTOMATIC:
         beat_automatic();
         break;
      case TEMPO_METHOD_CONSTANT:
         beat_constant();
         break;
      case TEMPO_METHOD_ONEBACK:
         if (beatTimes.getCount() < 2) {
            beat_constant();
         } else {
            beat_tracktempo(1);
         }
         break;
      case TEMPO_METHOD_TWOBACK:
         if (beatTimes.getCount() < 2) {
            beat_constant();
         } else {
            beat_tracktempo(2);
         }
         break;
      case TEMPO_METHOD_THREEBACK:
         if (beatTimes.getCount() < 2) {
            beat_constant();
         } else {
            beat_tracktempo(3);
         }
         break;
      case TEMPO_METHOD_FOURBACK:
         if (beatTimes.getCount() < 2) {
            beat_constant();
         } else {
            beat_tracktempo(4);
         }
         break;
      default:
         beat_automatic();
   }

   cout << " Current Tempo: " << tempo << "\t\t Current Beat: " 
        << performanceTimer.getPeriodCount() << endl;
}



//////////////////////////////
//
// MidiPerform::beat_automatic -- ignores the beat indicators.
//

void MidiPerform::beat_automatic(void) {
cout << "Beat AUTOMATIC" << endl;
   // do nothing
}
   

//////////////////////////////
//
// MidiPerform::beat_constant -- follow the beat but keep
//    old tempo.
//

void MidiPerform::beat_constant(void) {
cout << "Beat CONSTANT" << endl;
   double beatPosition = beatTimer.getPeriodCount();
   double padjust = 1.0 - beatPosition;
   if (padjust < -1.0) {
      padjust = padjust - (int)padjust;
   }
   performanceTimer.adjustPeriod(padjust);
   beatTimer.adjustPeriod(-beatPosition);
}



//////////////////////////////
//
// MidiPerform::beat_tracktempo -- averages the past
//    x number of beat's tempos as the current tempo 
//    of the next beat to be performed.
//

void MidiPerform::beat_tracktempo(int watchhistory) {
   if (beatTimes.getCount() > 2) {
      tempo = getAverageTempo(watchhistory);
   } else {
      return;
   }

   double beatPosition = beatTimer.getPeriodCount();
   double padjust = 1.0 - beatPosition;
   if (padjust < -1.0) {
      padjust = padjust - (int)padjust;
   }
   performanceTimer.adjustPeriod(padjust);
   beatTimer.adjustPeriod(-beatPosition);

   performanceTimer.setTempo(tempo);
   beatTimer.setTempo(tempo);
}



//////////////////////////////
//
// MidiPerform::getAmp --
//

double MidiPerform::getAmp(void) {
   return amp;
}



//////////////////////////////
//
// MidiPerform::getMaxAmp --
//

int MidiPerform::getMaxAmp(void) {
   return maxamp;
}



//////////////////////////////
//
// MidiPerform::getAverageTempo -- 
//

double MidiPerform::getAverageTempo(int count) {
   double sum = 0.0;
   int readamount;
   for (readamount = 0; readamount < count && readamount<beatTimes.getCount()-1;
       readamount++) {
      sum += (beatTimes[readamount] - beatTimes[readamount+1]);
   }

   if (readamount == 0 || sum == 0.0) {
      return tempo;
   } else {
      return readamount/sum * 60.0 * 1000.0;
   }
}



//////////////////////////////
//
// MidiPerform::channelCollapse -- 
//

int MidiPerform::channelCollapse(int aSetting) {
   if (aSetting == 1) {
      channelcollapseQ = 1;
   } else if (aSetting == 0) {
      channelcollapseQ = 0;
   }
   return channelcollapseQ;
}


//////////////////////////////
//
// MidiPerform::xcheck -- 
//

void MidiPerform::xcheck(void) {
   int quitQ = 0;
   MFEvent* event = NULL;
   if (beatTimer.expired()) {   // waiting for the next beat, so don't continue
      if (getTempoMethod() != TEMPO_METHOD_AUTOMATIC) {
         return;
      }
   }
   int currentTime = (int)(performanceTimer.getPeriodCount() *
         midifile.getTicksPerQuarterNote());
   int i;
   for (i=0; i<midifile.getTrackCount(); i++) {
      if (readIndex[i] >= midifile.getNumEvents(i)) {
         quitQ++;
         continue;
      }
      while (midifile.getEvent(i, readIndex[i]).time <= currentTime) {
         event = &midifile.getEvent(i, readIndex[i]);
         if (((event->data[0] & 0xf0) == 0x90) ||
             ((event->data[0] & 0xf0) == 0x80)) {
            if (channelCollapse()) {
               event->data[0] = event->data[0] & (uchar)0xf0;
            }
            if (event->data[2] != 0) {
               int amplitude = (int)(event->data[2] * getAmp());
               if (amplitude < 0) {
                  amplitude = 0;
               } else if (amplitude > getMaxAmp()) {
                  amplitude = getMaxAmp();
               }
               event->data[2] = (uchar)amplitude;
            }
         }
         rawsend(event->data.getBase(), event->data.getSize());
         readIndex[i] = readIndex[i] + 1;
         if (readIndex[i] >= midifile.getNumEvents(i)) {
            quitQ++;
            break;
         }
      }
   }

   if (quitQ == midifile.getTrackCount()) {
      exit(0);
   }
}



//////////////////////////////
//
// MidiPerform::getBeatFraction -- if it is above 1.0 then
//   waiting for next beat to occur.
//

double MidiPerform::getBeatFraction(void) { 
   return beatTimer.getPeriod();
}



//////////////////////////////
//
// MidiPerform::getBeatLocation -- the integer count of the
//   beats.
//  
//

int MidiPerform::getBeatLocation(void) { 
   return performanceTimer.expired();
}



//////////////////////////////
//
// MidiPerform::getTempo -- return current performance
//    tempo.
//

double MidiPerform::getTempo(void) {
   return tempo;
}



//////////////////////////////
//
// MidiPerform::getTempoMethod -- return the performance
//    type.
//

int MidiPerform::getTempoMethod(void) {
   return tempoMethod;
}



//////////////////////////////
//
// MidiPerform::pause --
//

void MidiPerform::pause(void) { 
   pauseLocation = performanceTimer.getTime();
   playingQ = 0;
}



//////////////////////////////
//
// MidiPerform::play --
//

void MidiPerform::play(void) { 
   playingQ = 1;
   performanceTimer.reset();
   beatTimer.reset();
   beatTimer.sync(performanceTimer);
}



//////////////////////////////
//
// MidiPerform::read --
//

void MidiPerform::read(char* aFile) { 
   int status = midifile.read(aFile);
   if (status == 0) {
      cout << "Error: midifile " << aFile << " is bad." << endl;
   }
   
   if (readIndex != NULL) {
      delete readIndex;
   }
   readIndex = new int[midifile.getTrackCount()];
   for (int i=0; i<midifile.getTrackCount(); i++) {
      readIndex[i] = 0;
   }

   performanceTimer.setTempo(tempo);
   beatTimer.setTempo(tempo);

}



//////////////////////////////
//
// MidiPerform::rewind --
//

void MidiPerform::rewind(void) { 
   pauseLocation = 0.0;
}



//////////////////////////////
//
// MidiPerform::setAmp --
//

void MidiPerform::setAmp(double anAmp) {
   if (amp < 0) {
      return;
   }
   amp = anAmp;
}



//////////////////////////////
//
// MidiPerform::setMaxAmp --
//

void MidiPerform::setMaxAmp(int aMax) {
   if (aMax < 0) {
      maxamp = 0;
   } else if (aMax > 127) {
      maxamp = 127;
   } else {
      maxamp = aMax;
   }
}



//////////////////////////////
//
// MidiPerform::setBeatLocation --
//

void MidiPerform::setBeatLocation(double aLocation) { 
   // xxx
}



//////////////////////////////
//
// MidiPerform::setTempo --
//

void MidiPerform::setTempo(double aTempo) { 
   tempo = aTempo;
   performanceTimer.setTempo(tempo);
   beatTimer.setTempo(tempo);
}



//////////////////////////////
//
// MidiPerform::setTempo --
//

void MidiPerform::setTempoMethod(int aMethod) { 
   if (aMethod == TEMPO_METHOD_AUTOMATIC) {
      beatTimes.reset();
   }
   tempoMethod = aMethod;
}



//////////////////////////////
//
// MidiPerform::stop --
//

void MidiPerform::stop(void) { 
   pauseLocation = performanceTimer.getTime();
}



// md5sum: 901e82cd86464be403679222b09a3940 MidiPerform.cpp [20020518]
