//
// Copyright 1999 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Nov 26 14:12:01 PST 1999
// Last Modified: Fri Dec  2 13:26:29 PST 1999
// Last Modified: Wed Dec 13 10:33:30 PST 2000 (modified sorting routine)
// Last Modified: Tue Jan 22 23:23:37 PST 2002 (allowed reading of meta events)
// Last Modified: Tue Nov  4 23:09:19 PST 2003 (adjust noteoff in eventcompare)
// Last Modified: Tue Jun 29 09:43:10 PDT 2004 (fixed end-of-track problem)
// Last Modified: Sat Dec 17 23:11:57 PST 2005 (added millisecond ticks)
// Last Modified: Thu Sep 14 20:07:45 PDT 2006 (added SMPTE ASCII printing)
// Last Modified: Tue Apr  7 09:23:48 PDT 2009 (added addMetaEvent)
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Filename:      ...sig/src/sigInfo/MidiFile.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/MidiFile.cpp
// Syntax:        C++ 
//
// Description:   A class which can read/write Standard MIDI files.
//                MIDI data is stored by track in an array.  This
//                class is used for example in the MidiPerform class.
//

#include "MidiFile.h"
#include <string.h>

#ifndef OLDCPP
   #include <iomanip>
   using namespace std;
#else
   #include <iomanip.h>
#endif



//////////////////////////////
//
// _MFEvent::_MFEvent --
//

_MFEvent::_MFEvent(void) { 
   time = 0;
   track = 0;
   data.allowGrowth();
   data.setSize(0);
}

_MFEvent::_MFEvent(int command) { 
   time = 0;
   track = 0;
   data.allowGrowth();
   data.setSize(1);
   data[0] = (uchar)command;
}

_MFEvent::_MFEvent(int command, int param1) { 
   time = 0;
   track = 0;
   data.allowGrowth();
   data.setSize(2);
   data[0] = (uchar)command;
   data[1] = (uchar)param1;
}

_MFEvent::_MFEvent(int command, int param1, int param2) { 
   time = 0;
   track = 0;
   data.allowGrowth();
   data.setSize(3);
   data[0] = (uchar)command;
   data[1] = (uchar)param1;
   data[2] = (uchar)param2;
}

_MFEvent::_MFEvent(int aTrack, int command, int param1, int param2) { 
   time = 0;
   track = aTrack;
   data.allowGrowth();
   data.setSize(3);
   data[0] = (uchar)command;
   data[1] = (uchar)param1;
   data[2] = (uchar)param2;
}

_MFEvent::_MFEvent(int aTime, int aTrack, int command, int param1, int param2) {
   time = aTime;
   track = aTrack;
   data.allowGrowth();
   data.setSize(3);
   data[0] = (uchar)command;
   data[1] = (uchar)param1;
   data[2] = (uchar)param2;
}



//////////////////////////////
//
// _MFEvent::~MFEvent
//

_MFEvent::~_MFEvent() { 
   time = -1;
   track = -1;
   data.setSize(0);
}



//////////////////////////////
//
// MidiFile::MidiFile --
//

MidiFile::MidiFile(void) { 
   ticksPerQuarterNote = 48;             // time base of file
   trackCount = 1;                       // # of tracks in file
   theTrackState = TRACK_STATE_SPLIT;    // joined or split
   theTimeState = TIME_STATE_DELTA;      // absolute or delta
   events.setSize(1);
   events[0] = new SigCollection<_MFEvent>;
   events[0]->setSize(0);
   events[0]->allowGrowth(1);
   readFileName = new char[1];
   readFileName[0] = '\0';
}


MidiFile::MidiFile(const char* aFile) { 
   ticksPerQuarterNote = 48;             // time base of file
   trackCount = 1;                       // # of tracks in file
   theTrackState = TRACK_STATE_SPLIT;    // joined or split
   theTimeState = TIME_STATE_DELTA;      // absolute or delta
   events.setSize(1);
   events[0] = new SigCollection<_MFEvent>;
   events[0]->setSize(0);
   events[0]->allowGrowth(1);
   readFileName = new char[1];
   readFileName[0] = '\0';
   read(aFile);
}



//////////////////////////////
//
// MidiFile::~MidiFile --
//

MidiFile::~MidiFile() { 
   if (readFileName != NULL) {
      delete [] readFileName;
      readFileName = NULL;
   }

   erase();

   if (events[0] != NULL) {  
      delete events[0];
      events[0] = NULL;
   }

}



//////////////////////////////
//
// MidiFile::absoluteTime -- convert the time data to
//    absolute time, which means that the time field
//    in the _MFEvent struct represents the exact tick
//    time to play the event rather than the time since
//    the last event to wait untill playing the current
//    event.
//

void MidiFile::absoluteTime(void) { 
   if (getTimeState() == TIME_STATE_ABSOLUTE) {
      return;
   }
   int i, j;
   int length = getNumTracks();
   int* timedata = new int[length];
   for (i=0; i<length; i++) {
      timedata[i] = 0;
      if (events[i]->getSize() > 0) {
         timedata[i] = (*events[i])[0].time;
      } else {
         continue;
      }
      for (j=1; j<events[i]->getSize(); j++) {
         timedata[i] += (*events[i])[j].time;
         (*events[i])[j].time = timedata[i];
      }
   }
   theTimeState = TIME_STATE_ABSOLUTE;
   delete [] timedata;
}



//////////////////////////////
//
// MidiFile::addEvent -- 
//

int MidiFile::addEvent(int aTrack, int aTime, Array<uchar>& midiData) {
   _MFEvent anEvent;
   anEvent.time = aTime;
   anEvent.track = aTrack;
   anEvent.data = midiData;

   events[aTrack]->append(anEvent);
   return events[aTrack]->getSize() - 1;
}



///////////////////////////////
//
// MidiFile::addMetaEvent --
//

int MidiFile::addMetaEvent(int aTrack, int aTime, int aType,
      Array<uchar>& metaData) {

   int i;
   int length = metaData.getSize();
   Array<uchar> fulldata;
   uchar size[23] = {0};
   int lengthsize =  makeVLV(size, length);

   fulldata.setSize(2+lengthsize+length);
   fulldata[0] = 0xff;
   fulldata[1] = aType & 0x7F;
   for (i=0; i<lengthsize; i++) {
      fulldata[2+i] = size[i];
   }
   for (i=0; i<length; i++) {
      fulldata[2+lengthsize+i] = metaData[i];
   }

   return addEvent(aTrack, aTime, fulldata);
}


int MidiFile::addMetaEvent(int aTrack, int aTime, int aType, 
      const char* metaData) {

   int length = strlen(metaData);
   Array<uchar> buffer;
   buffer.setSize(length);
   int i;
   for (i=0; i<length; i++) {
      buffer[i] = (uchar)metaData[i];
   }
   return addMetaEvent(aTrack, aTime, aType, buffer);
}



//////////////////////////////
//
// MidiFile::makeVLV -- 
//

int MidiFile::makeVLV(uchar *buffer, int number) {

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
// addPitchBend -- convert  number in the range from -1 to +1 
//     into two 7-bit numbers (smallest piece first)
//
//   -1.0 maps to 0 (0x0000) 
//    0.0 maps to 8192 (0x2000 --> 0x40 0x00) 
//   +1.0 maps to 16383 (0x3FFF --> 0x7F 0x7F)
//

int MidiFile::addPitchBend(int aTrack, int aTime, int aChannel, double amount) {
   amount += 1.0;
   int value = int(amount * 8192 + 0.5);

   // prevent any wrap-around in case of round-off errors
   if (value > 0x3fff) {
      value = 0x3fff;
   }
   if (value < 0) {
      value = 0;
   }

   int lsbint = 0x7f & value;
   int msbint = 0x7f & (value  >> 7);

   Array<uchar> mididata;
   mididata.setSize(3);
   if (aChannel < 0) {
      aChannel = 0;
   } else if (aChannel > 15) {
      aChannel = 15;
   }
   mididata[0] = uchar(0xe0 | aChannel);
   mididata[1] = uchar(lsbint);
   mididata[2] = uchar(msbint);

   return addEvent(aTrack, aTime, mididata);
}



//////////////////////////////
//
// MidiFile::addTrack -- adds a blank track at end of the 
//    track list.  Returns the track number of the added
//    track.
//

int MidiFile::addTrack(void) { 
   int length = getNumTracks();
   events.setSize(length+1);
   events[length] = new SigCollection<_MFEvent>;
   events[length]->setSize(10000);
   events[length]->setSize(0);
   events[length]->allowGrowth(1);
   return length;
}

int MidiFile::addTrack(int count) {
   int length = getNumTracks();
   events.setSize(length+count);
   int i;
   for (i=0; i<count; i++) {
      events[length + i] = new SigCollection<_MFEvent>;
      events[length + i]->setSize(10000);
      events[length + i]->setSize(0);
      events[length + i]->allowGrowth(1);
   }
   return length + count - 1;
}


//////////////////////////////
//
// MidiFile::allocateEvents --
//

void MidiFile::allocateEvents(int track, int aSize) {
   int oldsize = events[track]->getSize();
   if (oldsize < aSize) {
      events[track]->setSize(aSize);
      events[track]->setSize(oldsize);
   } 
}



//////////////////////////////
//
// MidiFile::deleteTrack -- remove a track from the MidiFile.
//   Tracks are numbered starting at track 0.
//

void MidiFile::deleteTrack(int aTrack) { 
   int length = getNumTracks();
   if (aTrack < 0 || aTrack >= length) {
      return;
   }
   if (length == 1) {
      return;
   }
   delete events[aTrack];
   for (int i=aTrack; i<length-1; i++) {
      events[i] = events[i+1];
   }

   events[length] = NULL;
   events.setSize(length-1);
}



//////////////////////////////
//
// MidiFile::deltaTime -- convert the time data to
//     delta time, which means that the time field
//     in the _MFEvent struct represents the time
//     since the last event was played. When a MIDI file
//     is read from a file, this is the default setting.
//

void MidiFile::deltaTime(void) { 
   if (getTimeState() == TIME_STATE_DELTA) {
      return;
   }
   int i, j;
   int temp;
   int length = getNumTracks();
   int *timedata = new int[length];
   for (i=0; i<length; i++) {
      timedata[i] = 0;
      if (events[i]->getSize() > 0) {
         timedata[i] = (*events[i])[0].time;
      } else {
         continue;
      }
      for (j=1; j<events[i]->getSize(); j++) {
         temp = (*events[i])[j].time;
         (*events[i])[j].time = temp - timedata[i];
         timedata[i] = temp;
      }
   }
   theTimeState = TIME_STATE_DELTA;
   delete [] timedata;
}



//////////////////////////////
//
// MidiFile::erase -- make the MIDI file empty with one
//     track with no data in it.
//

void MidiFile::erase(void) { 
   int length = getNumTracks();
   for (int i=0; i<length; i++) {
      delete events[i];
      events[i] = NULL;
   }
   events.setSize(1);
   events[0] = new SigCollection<_MFEvent>;
   events[0]->setSize(0);
   events[0]->allowGrowth(1);
}


void MidiFile::clear(void) {
   MidiFile::erase();
}



//////////////////////////////
//
// MidiFile::getEvent -- return the event at the given index in the
//    specified track.
//

_MFEvent& MidiFile::getEvent(int aTrack, int anIndex) {
   return (*events[aTrack])[anIndex];
}



//////////////////////////////
//
// MidiFile::getTicksPerQuarterNote -- returns the number of
//   time units that are supposed to occur during a quarternote.
//

int MidiFile::getTicksPerQuarterNote(void) { 
   return ticksPerQuarterNote;
}



//////////////////////////////
//
// MidiFile::getTrackCount -- return the number of tracks in 
//   the Midi File.
//

int MidiFile::getTrackCount(void) { 
   return events.getSize();
}

int MidiFile::getNumTracks(void) { 
   return events.getSize();
}



//////////////////////////////
//
// MidiFile::getNumEvents -- returns the number of events
//   in a given track.
//

int MidiFile::getNumEvents(int aTrack) { 
   return events[aTrack]->getSize();
}



//////////////////////////////
//
// MidiFile::joinTracks -- merge the data from all tracks,
//   but keeping the identity of the tracks unique so that
//   the function splitTracks can be called to split the 
//   tracks into separate units again.  The style of the 
//   MidiFile when read from a file is with tracks split.
//

void MidiFile::joinTracks(void) { 
   if (getTrackState() == TRACK_STATE_JOINED) {
      return;
   }
   if (getNumTracks() == 1) {
      return;
   }

   SigCollection <_MFEvent>* joinedTrack;
   joinedTrack = new SigCollection<_MFEvent>;
   joinedTrack->setSize(200000);
   joinedTrack->setSize(0);
   int oldTimeState = getTimeState();
   if (oldTimeState == TIME_STATE_DELTA) {
      absoluteTime();
   }
   int i, j;
   int length = getNumTracks();
   for (i=0; i<length; i++) {
      for (j=0; j<events[i]->getSize(); j++) {
         joinedTrack->append((*events[i])[j]);
      }
   }

   erase(); 

   delete events[0];
   events[0] = joinedTrack;
   sortTracks();
   if (oldTimeState == TIME_STATE_DELTA) {
      deltaTime();
   }
}



//////////////////////////////
//
// MidiFile::mergeTracks -- combine the data from two
//   tracks into one.  Placing the data in the first
//   track location listed, and Moving the other tracks
//   in the file around to fill in the spot where Track2
//   used to be.  The results of this function call cannot
//   be reversed.
//

void MidiFile::mergeTracks(int aTrack1, int aTrack2) { 
   SigCollection <_MFEvent>* mergedTrack;
   mergedTrack = new SigCollection<_MFEvent>;
   mergedTrack->setSize(0);
   int oldTimeState = getTimeState();
   if (oldTimeState == TIME_STATE_DELTA) {
      absoluteTime();
   }
   int i, j;
   int length = getNumTracks();
   for (i=0; i<events[aTrack1]->getSize(); i++) {
      mergedTrack->append((*events[aTrack1])[i]);
   }
   for (j=0; j<events[aTrack2]->getSize(); i++) {
      (*events[aTrack2])[i].track = aTrack1;
      mergedTrack->append((*events[aTrack2])[i]);
   }

   sortTrack(*mergedTrack);

   delete events[aTrack1];

   events[aTrack1] = mergedTrack;

   for (i=aTrack2; i<length-1; i++) {
      events[i] = events[i+1];
   }

   events[length] = NULL;
   events.setSize(length-1);

   if (oldTimeState == TIME_STATE_DELTA) {
      deltaTime();
   }
}



//////////////////////////////
//
// MidiFile::read -- read a MIDI file and store its contents.
//

int MidiFile::read(const char* aFile) { 

   #ifndef OLDCPP
      #ifdef VISUAL
         FileIO inputfile(aFile, ios::in | ios::binary);
      #else
         FileIO inputfile(aFile, ios::in);
      #endif 
   #else
      #ifdef VISUAL
         FileIO inputfile(aFile, ios::in | ios::nocreate | ios::binary);
      #else
         FileIO inputfile(aFile, ios::in | ios::nocreate);
      #endif 
   #endif

   if (!inputfile.is_open()) {
      return 0;
   }

   // Read the MIDI header (4 bytes of ID, 4 byte data size, 6 bytes
   // of data.

   ulong longdata;
   uchar chardata;
   ushort shortdata;
   
   inputfile.readBigEndian(chardata);
   if (chardata != 'M') {
      cout << "File: " << aFile << " is not a MIDI file" << endl;
      cout << "Chara data is" << chardata << endl;
      return 0;
   }

   inputfile.readBigEndian(chardata);
   if (chardata != 'T') {
      cout << "File: " << aFile << " is not a MIDI file" << endl;
      return 0;
   }

   inputfile.readBigEndian(chardata);
   if (chardata != 'h') {
      cout << "File: " << aFile << " is not a MIDI file" << endl;
      return 0;
   }

   inputfile.readBigEndian(chardata);
   if (chardata != 'd') {
      cout << "File: " << aFile << " is not a MIDI file" << endl;
      return 0;
   }

   // read header size
   inputfile.readBigEndian(longdata);
   if (longdata != 6) {
      cout << "File: " << aFile 
           << " is not a MIDI 1.0 Standard MIDI file." << endl;
      cout << "The header size is: " << longdata << endl;
      return 0;
   }

   // read file type
   int type;
   inputfile.readBigEndian(shortdata);
   switch (shortdata) {
      case 0:
         type = 0;
         break;
      case 1:
         type = 1;
         break;
      default:
         cout << "Error: cannot handle type " << shortdata 
              << " MIDI file" << endl;
         return 0;
   }
 
   // read number of tracks
   int tracks;
   inputfile.readBigEndian(shortdata);
   if (type == 0 && shortdata != 1) {
      cout << "Error: Type 0 MIDI file can only contain one track" << endl;
      return 0;
   } else {
      tracks = shortdata;
   }
   // cout << "Track count is: " << tracks << endl;
   erase();
   if (events[0] != NULL) {
      delete events[0];
   }
   events.setSize(tracks);
   for (int z=0; z<tracks; z++) {
      events[z] = new SigCollection<_MFEvent>;
      events[z]->setAllocSize(10000);
      events[z]->setSize(0);
      events[z]->allowGrowth(1);
   }

   // read ticks per quarter note
   short signeddata;
   inputfile.readBigEndian(signeddata);
   if (signeddata <= 0) {
      int framespersecond = (signeddata >> 8) & 0x00ff;
      int resolution      = signeddata & 0x00ff;
      switch (framespersecond) {
         case 232:  framespersecond = 24; break;
         case 231:  framespersecond = 25; break;
         case 227:  framespersecond = 29; break;
         case 226:  framespersecond = 30; break;
         default: 
               cerr << "Warning: unknown FPS: " << framespersecond << endl;
               framespersecond = 255 - framespersecond + 1;
               cerr << "Setting FPS to " << framespersecond << endl;
      }
       
      signeddata = framespersecond * resolution;

      cout << "SMPTE ticks: " << signeddata << " tpq" << endl;
      cout << "SMPTE frames per second: " << framespersecond << endl;
      cout << "SMPTE frame resolution: " << resolution << endl;
   } 
   ticksPerQuarterNote = signeddata;

   //////////////////////////////////////////////////
   //
   // now read individual tracks:
   //

   uchar runningCommand = 0;
   _MFEvent event;
   int absticks;
   int barline;
   
   for (int i=0; i<tracks; i++) {
      // cout << "\nReading Track: " << i + 1 << flush;
      // read track header...
     
      inputfile.readBigEndian(chardata);
      if (chardata != 'M') {
         cout << "File: " << aFile << " has bad track info" << endl;
         cout << "character 1 is: " << (int)chardata << endl;
         inputfile.readBigEndian(chardata);
         if (inputfile.eof()) {
            cout << "End of file reached" << endl;
         }
         return 0;
      }

      inputfile.readBigEndian(chardata);
      if (chardata != 'T') {
         cout << "File: " << aFile << " has bad track info" << endl;
         cout << "character 2 is: " << (int)chardata << endl;
         return 0;
      }

      inputfile.readBigEndian(chardata);
      if (chardata != 'r') {
         cout << "File: " << aFile << " has bad track info" << endl;
         cout << "character 3 is: " << (int)chardata << endl;
         return 0;
      }

      inputfile.readBigEndian(chardata);
      if (chardata != 'k') {
         cout << "File: " << aFile << " has bad track info" << endl;
         cout << "character 4 is: " << (int)chardata << endl;
         return 0;
      }

      // Now read track chunk size and throw it away because it is
      // not really necessary since the track MUST end with an
      // end of track meta event, and 50% of Midi files or so
      // do not correctly give the track size.
      inputfile.readBigEndian(longdata);

      // set the size of the track allocation so that it might
      // approximately fit the data.
      events[i]->setSize(longdata/2);
      events[i]->setSize(0);

      // process the track
      absticks = 0;
      barline = 1;
      while (!inputfile.eof()) {
         longdata = extractVlvTime(inputfile);
//cout << "ticks = " << longdata << endl;
         absticks += longdata;
         extractMidiData(inputfile, event.data, runningCommand);
//cout << "command = " << hex << (int)event.data[0] << dec << endl;
         if (event.data[0] == 0xff && (event.data[1] == 1 || 
             event.data[1] == 2 || event.data[1] == 3 || event.data[1] == 4)) {
           // mididata.append('\0');
           // cout << '\t';
           // for (int m=0; m<event.data[2]; m++) {
           //    cout << event.data[m+3];
           // }
           // cout.flush();
         } else if (event.data[0] == 0xff && event.data[1] == 0x2f) {
            break;
         }

         if (event.data[0] != 0xff && event.data[0] != 0xf0) {
            event.time = absticks;
            if ((event.data[0] & 0xf0) == 0x90) {
               if (event.data[1] < 12) {
                  event.data[0] = event.data[1];
                  switch (event.data[0]) {
                     case 2: event.data[2] = barline++;    // barline
                             break; 
                     case 0: break;                        // beat
                  }
               }
            }
            event.track = i;
            events[i]->append(event);
         } else {
            event.time = absticks;
            event.track = i;
            events[i]->append(event);
         }

      }

   }
   // cout << endl;

   theTimeState = TIME_STATE_ABSOLUTE;
   return 1;
}



//////////////////////////////
//
// MidiFile::setTicksPerQuarterNote -- 
//

void MidiFile::setTicksPerQuarterNote(int ticks) {
   ticksPerQuarterNote = ticks;
}



//////////////////////////////
//
// MidiFile::setMilliseconds -- set the ticks per quarter note
//   value to milliseconds.  The format for this specification is
//   highest 8-bits: SMPTE Frame rate (as a negative 2's compliment value).
//   lowest 8-bits: divisions per frame (as a positive number).
//   for millisecond resolution, the SMPTE value is -25, and the
//   frame rate is 40 frame per division.  In hexadecimal, these
//   values are: -25 = 1110,0111 = 0xE7 and 40 = 0010,1000 = 0x28
//   So setting the ticks per quarter note value to 0xE728 will cause
//   delta times in the MIDI file to represent milliseconds.
//

void MidiFile::setMillisecondDelta(void) {
   ticksPerQuarterNote = 0xE728;
}



//////////////////////////////
//
// MidiFile::sortTrack -- 
//

void MidiFile::sortTrack(SigCollection<_MFEvent>& trackData) { 
   qsort(trackData.getBase(), trackData.getSize(), 
      sizeof(_MFEvent), eventcompare);
}



//////////////////////////////
//
// MidiFile::sortTracks -- sort all tracks in the MidiFile.
//

void MidiFile::sortTracks(void) { 
   for (int i=0; i<getTrackCount(); i++) {
      sortTrack(*events[i]);
   }
}



//////////////////////////////
//
// MidiFile::splitTracks -- take the joined tracks and split them
//   back into their separate track identities.
//

void MidiFile::splitTracks(void) { 
   if (getTrackState() == TRACK_STATE_SPLIT) {
      return;
   }

   int oldTimeState = getTimeState();
   if (oldTimeState == TIME_STATE_DELTA) {
      absoluteTime();
   }

   int maxTrack = 0;
   int i;
   int length = events[0]->getSize();
   for (i=0; i<length; i++) {
      if ((*events[0])[i].track > maxTrack) {
          maxTrack = (*events[0])[i].track;
      }
   }

   SigCollection<_MFEvent>* olddata = events[0];
   events[0] = NULL;
   events.setSize(maxTrack);
   for (i=0; i<maxTrack; i++) {
      events[i] = new SigCollection<_MFEvent>;
      events[i]->setSize(0);
      events[i]->allowGrowth();
   }

   int trackValue = 0;
   for (i=0; length; i++) {
      trackValue = (*olddata)[i].track;
      events[trackValue]->append((*olddata)[i]);
   }

   delete olddata;

   if (oldTimeState == TIME_STATE_DELTA) {
      deltaTime();
   }
}



//////////////////////////////
//
// MidiFile::timeState -- returns what type of time method is
//   being used: either TIME_STATE_ABSOLUTE or TIME_STATE_DELTA.
//

int MidiFile::getTimeState(void) { 
   return theTimeState;
}



//////////////////////////////
//
// MidiFile::getTrackState -- returns what type of track method
//     is being used: either TRACK_STATE_JOINED or TRACK_STATE_SPLIT.
//

int MidiFile::getTrackState(void) { 
   return theTrackState;
}



//////////////////////////////
//
// MidiFile::write -- write a standard MIDI file from data.
//

int MidiFile::write(const char* aFile) { 
   int oldTimeState = getTimeState();
   if (oldTimeState == TIME_STATE_ABSOLUTE) {
      deltaTime();
   }

   #ifndef OLDCPP
      #ifdef VISUAL
         FileIO outputfile(aFile, ios::out | ios::binary);
      #else
         FileIO outputfile(aFile, ios::out);
      #endif 
   #else
      #ifdef VISUAL
         FileIO outputfile(aFile, ios::out | ios::noreplace | ios::binary);
      #else
         FileIO outputfile(aFile, ios::out | ios::noreplace);
      #endif 
   #endif 

   if (!outputfile.is_open()) {
      cout << "Error: could not write: " << aFile << endl;
      exit(1);
   }

   // write the header of the Standard MIDI File
   
   char ch;
   // 1. The characters "MThd"
   ch = 'M';
   outputfile.writeBigEndian(ch);
   ch = 'T';
   outputfile.writeBigEndian(ch);
   ch = 'h';
   outputfile.writeBigEndian(ch);
   ch = 'd';
   outputfile.writeBigEndian(ch);

   // 2. write the size of the header (alwas a "6" stored in unsigned long
   ulong longdata = 6;
   outputfile.writeBigEndian(longdata);

   // 3. MIDI file format, type 0, 1, or 2
   ushort shortdata;
   if (getNumTracks() == 1) {
      shortdata = 0;
   } else {
      shortdata = 1;
   }
   outputfile.writeBigEndian(shortdata);
      
   // 4. write out the number of tracks.
   shortdata = getNumTracks();
   outputfile.writeBigEndian(shortdata);

   // 5. write out the number of ticks per quarternote. (avoiding SMTPE for now)
   shortdata = getTicksPerQuarterNote();
   outputfile.writeBigEndian(shortdata);

   // now write each track.
   Array<uchar> trackdata;
   uchar endoftrack[4] = {0, 0xff, 0x2f, 0x00};
   int i, j, k;
   int size;
   for (i=0; i<getNumTracks(); i++) {
      trackdata.setSize(1000000);   // make the track data larger than
                                    // expected data input
      trackdata.setGrowth(1000000);
      trackdata.setSize(0);
      trackdata.allowGrowth();
      for (j=0; j<events[i]->getSize(); j++) {
         writeVLValue((*events[i])[j].time, trackdata);
         for (k=0; k<(*events[i])[j].data.getSize(); k++) {
            trackdata.append((*events[i])[j].data[k]);
         }
      }
      size = trackdata.getSize();
      if (!((trackdata[size-3] == 0xff) && (trackdata[size-2] == 0x2f))) {
         trackdata.append(endoftrack[0]);
         trackdata.append(endoftrack[1]);
         trackdata.append(endoftrack[2]);
         trackdata.append(endoftrack[3]);
      }

      // now ready to write to MIDI file.
   
      // first write the track ID marker "MTrk":
      ch = 'M';
      outputfile.writeBigEndian(ch);
      ch = 'T';
      outputfile.writeBigEndian(ch);
      ch = 'r';
      outputfile.writeBigEndian(ch);
      ch = 'k';
      outputfile.writeBigEndian(ch);

      // A. write the size of the MIDI data to follow:
      longdata = trackdata.getSize();
      outputfile.writeBigEndian(longdata);

      // B. write the actual data
      outputfile.write((char*)trackdata.getBase(), trackdata.getSize());
   }

   if (oldTimeState == TIME_STATE_ABSOLUTE) {
      absoluteTime();
   }
 
   outputfile.close();

   return 1;
}



///////////////////////////////////////////////////////////////////////////
//
// private functions
//
   

//////////////////////////////
//
// MidiF::extractMidiData --
//

void MidiFile::extractMidiData(FileIO& inputfile, Array<uchar>& array, 
   uchar& runningCommand) {

   uchar byte;
   array.setSize(0);
   array.allowGrowth();
   int runningQ;

   inputfile.readBigEndian(byte);

   if (byte < 0x80) {
      runningQ = 1;
      if (runningCommand == 0) {
         cout << "Error: running command with no previous command" << endl;
         exit(1);
      }
   } else {
      runningCommand = byte;
      runningQ = 0;
   }

   array.append(runningCommand);
   if (runningQ) {
      array.append(byte);
   }

   uchar metai;
   switch (runningCommand & 0xf0) {
      case 0x80:        // note off (2 more bytes)
      case 0x90:        // note on (2 more bytes)
      case 0xA0:        // aftertouch (2 more bytes)
      case 0xB0:        // cont. controller (2 more bytes)
      case 0xE0:        // pitch wheel (2 more bytes)
         inputfile.readBigEndian(byte);
         array.append(byte);
         if (!runningQ) {
            inputfile.readBigEndian(byte);
            array.append(byte);
         }
         break;
      case 0xC0:        // patch change (1 more byte)
      case 0xD0:        // channel pressure (1 more byte)
         if (!runningQ) {
            inputfile.readBigEndian(byte);
            array.append(byte);
         }
         break;
      case 0xF0:
         switch (runningCommand) {
            case 0xff:                 // meta event
               {
               if (!runningQ) {
                  inputfile.readBigEndian(byte);  // meta type
               array.append(byte);
               }
               inputfile.readBigEndian(metai);  // meta size
               array.append(metai);
               for (uchar j=0; j<metai; j++) {
                  inputfile.readBigEndian(byte);  // meta data
                  array.append(byte);
               }
               }
               break;
            case 0xf0:                // sysex
               // read until you find a 0xf7 character
               byte = 0;
               while (byte != 0xf7 && !inputfile.eof()) {
                  inputfile.readBigEndian(byte);  // meta data
               }
               break;
         }
         break; 
      default:
         cout << "Error reading midifile" << endl;
         cout << "Command byte was " << (int)runningCommand << endl;
         exit(1);
   }
}



//////////////////////////////
//
// MidiF::extractVlvTime -- 
//

ulong MidiFile::extractVlvTime(FileIO& inputfile) {
   uchar b[5] = {0};
 
   for (int i=0; i<5; i++) {
      inputfile.readBigEndian(b[i]);
      if (b[i] < 0x80) {
         break;
      }
   }
  
   return unpackVLV(b[0], b[1], b[2], b[3], b[4]);
}



//////////////////////////////
//
// MidiF::unpackVLV -- converts a VLV value to pure unsigned long value.
// default values: a = b = c = d = e = 0;
//

ulong MidiFile::unpackVLV(uchar a, uchar b, uchar c, uchar d, uchar e) {
   if (e > 0x7f) {
      cout << "Error: VLV value was too long" << endl;
      exit(1);
   }

   uchar bytes[5] = {a, b, c, d, e};
   int count = 0;
   while (bytes[count] > 0x7f && count < 5) {
      count++;
   }
   count++;

   ulong output = 0;
   for (int i=0; i<count; i++) {
      output = output << 7;
      output = output | (bytes[i] & 0x7f);
   }
   
   return output;
}
   

//////////////////////////////
//
// MidiFileWrite::writeVLValue -- write a number to the midifile
//    as a variable length value which segments a file into 7-bit
//    values.  Maximum size of aValue is 0x7fffffff
//

void MidiFile::writeVLValue(long aValue, Array<uchar>& outdata) {
   uchar bytes[5] = {0};
   bytes[0] = (uchar)(((ulong)aValue >> 28) & 0x7f);  // most significant 5 bits
   bytes[1] = (uchar)(((ulong)aValue >> 21) & 0x7f);  // next largest 7 bits
   bytes[2] = (uchar)(((ulong)aValue >> 14) & 0x7f);
   bytes[3] = (uchar)(((ulong)aValue >> 7)  & 0x7f);
   bytes[4] = (uchar)(((ulong)aValue)       & 0x7f);  // least significant 7 bits

   int start = 0;
   while (start<5 && bytes[start] == 0)  start++;

   for (int i=start; i<4; i++) {
      bytes[i] = bytes[i] | 0x80;    
      outdata.append(bytes[i]);
   }
   outdata.append(bytes[4]);
}

          
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// external functions
//


//////////////////////////////
//
// eventcompare -- for sorting the tracks
//

int eventcompare(const void* a, const void* b) {
   _MFEvent& aevent = *((_MFEvent*)a);
   _MFEvent& bevent = *((_MFEvent*)b);

   if (aevent.time > bevent.time) {
      return 1;
   } else if (aevent.time < bevent.time) {
      return -1;
   } else if (aevent.data[0] == 0xff && bevent.data[0] != 0xff) {
      return 1;
   } else if (bevent.data[0] == 0xff && aevent.data[0] != 0xff) {
      return -1;
   } else if (bevent.data[0] == 0xff && bevent.data[1] == 0x2f) {
      return -1;
   } else if (aevent.data[0] == 0xff && aevent.data[1] == 0x2f) {
      return 1;
   } else if (((aevent.data[0] & 0xf0) == 0xe0) && 
              ((bevent.data[0] & 0xf0) == 0x90)) {
      // pitch bend placed before note on messages
      return -1;
   } else if (((aevent.data[0] & 0xf0) == 0x90) && 
              ((bevent.data[0] & 0xf0) == 0xe0)) {
      // pitch bend placed before note on messages
      return +1;
   } else if (((aevent.data[0] & 0xf0) == 0x90) && 
              ((bevent.data[0] & 0xf0) == 0x80)) {
      return 1;
   } else if (((aevent.data[0] & 0xf0) == 0x80) && 
              ((bevent.data[0] & 0xf0) == 0x90)) {
      return -1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// operator<< -- for printing an ASCII version of the MIDI file
//

ostream& operator<<(ostream& out, MidiFile& aMidiFile) {
   int i, j, k;
   out << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
   out << "Number of Tracks: " << aMidiFile.getTrackCount() << "\n";
   out << "Time method: " << aMidiFile.getTimeState();
   if (aMidiFile.getTimeState() == TIME_STATE_DELTA) {
      out << " (Delta timing)";
   } else if (aMidiFile.getTimeState() == TIME_STATE_ABSOLUTE) {
      out << " (Absolute timing)";
   } else {
      out << " (unknown method)";
   } 
   out << "\n";
    
   out << "Divisions per Quarter Note: " << dec << aMidiFile.getTicksPerQuarterNote() << "\n";
   for (i=0; i<aMidiFile.getNumTracks(); i++) {
      out << "\nTrack " << i 
          << "   +++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
      for (j=0; j<aMidiFile.getNumEvents(i); j++) {
         out << dec << aMidiFile.getEvent(i, j).time << "\t"
             << "0x" << hex << (int)aMidiFile.getEvent(i, j).data[0] << " ";
         if (aMidiFile.getEvent(i, j).data[0] == 0xff) {

            if (aMidiFile.getEvent(i, j).data[1] == 0x01) {
               out << "TEXT [";
               for (k=3; k<aMidiFile.getEvent(i, j).data.getSize(); k++) {
                  out << (char)aMidiFile.getEvent(i, j).data[k];
               }
               out << "]";

            } else if (aMidiFile.getEvent(i, j).data[1] == 0x02) {
               out << "COPY [";
               for (k=3; k<aMidiFile.getEvent(i, j).data.getSize(); k++) {
                  out << (char)aMidiFile.getEvent(i, j).data[k];
               }
               out << "]";

            } else if (aMidiFile.getEvent(i, j).data[1] == 0x03) {
               out << "TRACK [";
               for (k=3; k<aMidiFile.getEvent(i, j).data.getSize(); k++) {
                  out << (char)aMidiFile.getEvent(i, j).data[k];
               }
               out << "]";

            } else if (aMidiFile.getEvent(i, j).data[1] == 0x04) {
               out << "INSTR [";
               for (k=3; k<aMidiFile.getEvent(i, j).data.getSize(); k++) {
                  out << (char)aMidiFile.getEvent(i, j).data[k];
               }
               out << "]";

            } else if (aMidiFile.getEvent(i, j).data[1] == 0x05) {
               out << "LYRIC [";
               for (k=3; k<aMidiFile.getEvent(i, j).data.getSize(); k++) {
                  out << (char)aMidiFile.getEvent(i, j).data[k];
               }
               out << "]";

            } else {
               for (k=1; k<aMidiFile.getEvent(i, j).data.getSize(); k++) {
                  out << dec << (int)aMidiFile.getEvent(i, j).data[k] << " ";
               }
            }

         } else {
            for (k=1; k<aMidiFile.getEvent(i, j).data.getSize(); k++) {
               out << dec << (int)aMidiFile.getEvent(i, j).data[k] << " ";
            }
         }
         out << "\n";
      }
   }
   out << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
   return out;
}



// md5sum: dbc169503f646840c182ce6f3b910feb MidiFile.cpp [20030102]
