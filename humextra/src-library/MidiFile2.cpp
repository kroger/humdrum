//
// Copyright 1997 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Mar  3 22:23:34 GMT-0800 1997
// Last Modified: Mon Mar  3 22:23:34 GMT-0800 1997
// Last Modified: Sun Mar 24 12:10:00 PST 2002 (small changes for visual c++)
// Filename:      ...sig/src/sigInfo/MidiFile2.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/MidiFile2.cpp
// Syntax:        C++ 
//
// Description:   Older version of the MidiFile class.  Different
//                memory management handling.
//

#include "MidiFile2.h"

#include <stdlib.h>

#ifndef OLDCPP
   #include <fstream>
   #include <iomanip>
   using namespace std;
#else
   #include <fstream.h>
   #include <iomanip.h>
#endif

typedef unsigned int uint;



//////////////////////////////
//
// MidiFile2::MidiFile2 --
//

MidiFile2::MidiFile2(void) {
   midiData = NULL;
   dataSize = 0;
   trackStart = NULL;
   trackSize = NULL;
   currTrackIndex = NULL;
   currTrackCommand = NULL;
   trackLoopCount = NULL;
} 


MidiFile2::MidiFile2(const char* filename) {
   midiData = NULL;
   dataSize = 0;
   trackStart = NULL;
   trackSize = NULL;
   currTrackIndex = NULL;
   currTrackCommand = NULL;
   trackLoopCount = NULL;
   input(filename);
}



//////////////////////////////
//
// MidiFile2::~MidiFile2 --
//

MidiFile2::~MidiFile2() {
   if (midiData != NULL)  delete [] midiData;
   midiData = NULL;
   dataSize = 0;
   if (trackStart != NULL)  delete [] trackStart;
   trackStart = NULL;
   if (trackSize != NULL)  delete [] trackSize;
   trackSize = NULL;
   if (currTrackIndex != NULL)  delete [] currTrackIndex;
   currTrackIndex = NULL;
   if (currTrackCommand != NULL)  delete [] currTrackCommand;
   currTrackCommand = NULL;
   if (trackLoopCount != NULL)  delete [] trackLoopCount;
   trackLoopCount = NULL;
}



//////////////////////////////
//
// MidiFile2::backtrack -- subtrack one from a track index pointer
//

void MidiFile2::backtrack(int aTrack) {
   currTrackIndex[aTrack]--;
}



//////////////////////////////
//
// MidiFile2::eot -- returns the loop count, used to check if
//	this is the first end of track.
//

int MidiFile2::eot(int aTrack) {
   return trackLoopCount[aTrack];
}



//////////////////////////////
//
// MidiFile2::extractByte -- extracts a byte from a track
//

uchar MidiFile2::extractByte(int aTrack) {
   uchar output;
   output = midiData[currTrackIndex[aTrack]];
   currTrackIndex[aTrack]++;
   if (currTrackIndex[aTrack] >= trackStart[aTrack] + trackSize[aTrack] + 8) {
      trackLoopCount[aTrack]++;
      currTrackIndex[aTrack] = trackStart[aTrack] + 8;
   }

   return output;
}



//////////////////////////////
//
// MidiFile2::extractVLVtime -- gets the delta time
//	of the specified midi track
//

ulong MidiFile2::extractVLVtime(int aTrack) {
   uchar temp;
   ulong output = 0;

   temp = extractByte(aTrack);
   if (temp < 0x80) {
      output |= temp;
      return output;
   } else {
      output |= 0x7f & temp;
      output = (output << 7);
   }

   temp = extractByte(aTrack);
   if (temp < 0x80) {
      output |= temp;
      return output;
   } else {
      output |= 0x7f & temp;
      output = (output << 7);
   }

   temp = extractByte(aTrack);
   if (temp < 0x80) {
      output |= temp;
      return output;
   } else {
      output |= 0x7f & temp;
      output = (output << 7);
   }

   temp = extractByte(aTrack);
   if (temp < 0x80) {
      output |= temp;
      return output;
   } else {
      output |= 0x7f & temp;
      output = (output << 7);
   }

   cerr << "Error: VLV delta time is too big!" << endl;
   return 0xffffffff;
}



//////////////////////////////
//
// MidiFile2::getNumTracks -- returns the number of tracks in
//	the midi file.  The number of tracks is stored in
//	the 4th field of the midi file header in two bytes
//	from bytes 10-11 (offset 0).
//

int MidiFile2::getNumTracks(void) {
   int output = flip2bytes(*((unsigned short*)&midiData[10]));
   return output;
}



//////////////////////////////
//
// MidiFile2::getTicksPerQuarterNote -- returns the number of
//    clock ticks per quarternote in the MIDI file. The
//    number of ticks is stored in the 5th field of the
//    MIDI file header in two bytes from bytes 12-13 (offset 0)
//

int MidiFile2::getTicksPerQuarterNote(void) {
   int output = flip2bytes(*((unsigned short*)&midiData[12]));
   return output;
}


//////////////////////////////
//
// MidiFile2::getNextMessage -- returns the next midi message
//	for a given track the time information is the
//	number of clock ticks for the delta time of the
//	message.
//

MidiMessage MidiFile2::getNextMessage(int aTrack) {
   MidiMessage output;
   output.time = 0;

marker:
   int timevalue;
   timevalue = extractVLVtime(aTrack);
   output.time += timevalue;
   output.command() = extractByte(aTrack);

   // make sure not in running status
   if (output.command() < 0x80) {
      backtrack(aTrack);
      output.command() = currTrackCommand[aTrack];
   } else {
      currTrackCommand[aTrack] = output.command();
   }

   // ignore meta messages
   // might want to not ignore end of track meta, depends on MidiFile2 creation
   if (output.command() == 0xff) {
      extractByte(aTrack);   // the meta type
      int metaSize = extractVLVtime(aTrack);
      for (int i=0; i<metaSize; i++) {
         extractByte(aTrack);  // the meta data
      }
      goto marker;
   }

   switch (output.command() & 0xf0) {
      // two parameter commands
      case 0x80:                        // a note off command
      case 0x90:                        // a note on command
      case 0xa0:                        // an aftertouch command
      case 0xb0:                        // a continuous controller command
      case 0xe0:                        // a pitch wheel command
         output.p1() = extractByte(aTrack);  
         output.p2() = extractByte(aTrack);
         break;
      // one parameter commands
      case 0xc0:                        // a patch change command
      case 0xd0:                        // a channel pressure command
         output.p1() = extractByte(aTrack);  
         break;
      default:
         cerr << "I am a stupid program, I do no know what the command "
              << "0x" << hex << (int)output.command() << " is." << endl;
         cerr << "Error occurred at midi file index " 
              << currTrackIndex[aTrack]-1 << endl;
         exit(1);
   }
            
   return output;
}



//////////////////////////////
//
// MidiFile2::input -- returns true if successful
//

int MidiFile2::input(const char* filename) {

   // first load the file into an array

   #ifndef OLDCPP
      ifstream infile(filename, ios::in | ios::binary);
   #else
      ifstream infile(filename, ios::in | ios::nocreate | ios::binary);
   #endif

   if (!infile)  {
      cerr << "Could no open file: " << filename << endl;
      exit(1);
   }

   infile.seekg(0, ios::end);    // go to end of file
   dataSize = infile.tellg();    // return location in file
   infile.seekg(0, ios::beg);    // go to beginning of file
   if (midiData != NULL)  delete [] midiData;
   midiData = new uchar[dataSize];

   infile.read((char*)midiData, dataSize);

   infile.close();   
   if (midiData[0] != 'M' || midiData[1] != 'T' || 
       midiData[2] != 'h' || midiData[3] != 'd'    ) {
      cerr << "File is not a MIDI file : " << filename << endl;
      exit(1);
   } 

   // now set up the track variables
   numTracks = getNumTracks();
   // store track locations
   if (trackStart != NULL)  delete [] trackStart;
   trackStart = new int[numTracks];
   if (trackSize != NULL)  delete [] trackSize;
   trackSize = new int[numTracks];
   if (currTrackIndex != NULL)  delete [] currTrackIndex;
   currTrackIndex = new int[numTracks];
   if (currTrackCommand != NULL)  delete [] currTrackCommand;
   currTrackCommand = new int[numTracks];
   if (trackLoopCount != NULL)  delete [] trackLoopCount;
   trackLoopCount = new int[numTracks];
   setupTrackStart();


// Test to see if working:  printout the track information
   cout << "Track statistics for " << filename << ':' << endl;
   for (int i=0; i<numTracks; i++) {
      cout << "Track " << i << " start = " << trackStart[i] 
           << " size = " << trackSize[i] << endl;
   }


   return 1;

}



//////////////////////////////
//
// MidiFile2::printnext --
//

void MidiFile2::printnext(int aTrack) {
   cout << hex << (uint)midiData[currTrackIndex[aTrack]]   << " ";
   cout << hex << (uint)midiData[currTrackIndex[aTrack]+1] << " ";
   cout << hex << (uint)midiData[currTrackIndex[aTrack]+2] << " ";
   cout << hex << (uint)midiData[currTrackIndex[aTrack]+3] << " ";
   cout << hex << (uint)midiData[currTrackIndex[aTrack]+4] << " ";
   cout << hex << (uint)midiData[currTrackIndex[aTrack]+5] << " ";
   cout << hex << (uint)midiData[currTrackIndex[aTrack]+6] << " ";
   cout << hex << (uint)midiData[currTrackIndex[aTrack]+7] << " ";
   cout << hex << (uint)midiData[currTrackIndex[aTrack]+8] << " ";
   cout << dec << endl;
}
   


//////////////////////////////
//
// MidiFile2::reset -- put the buffer read to the beginning
//	of a track and set the loop count to 0
//

void MidiFile2::reset(int aTrack) {
   currTrackIndex[aTrack] = trackStart[aTrack] + 8;
   trackLoopCount[aTrack] = 0;
}


//////////////////////////////
//
// MidiFile2::resetAllTracks -- calls reset for all tracks
//

void MidiFile2::resetAllTracks(void) {
   for (int i=0; i<getNumTracks(); i++) {
      reset(i);
   }
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// MidiFile2::flip2bytes -- the fun of Intel processors: they are
//	little endian and bytes in a midi file are big endian.
//

ushort MidiFile2::flip2bytes(ushort aNumber) {
   ushort output = (aNumber >> 8) | (aNumber << 8);
   return output;
}



//////////////////////////////
//
// MidiFile2::flip4bytes -- the fun of Intel processors: they are
//	little endian and bytes in a midi file are big endian.
//

ulong MidiFile2::flip4bytes(ulong aNumber) {
   ushort output = 0;
   output |= (aNumber << 24) & 0xff000000;
   output |= (aNumber << 8)  & 0x00ff0000;
   output |= (aNumber >> 8)  & 0x0000ff00;
   output |= (aNumber >> 24) & 0x000000ff;
   return output;
}



//////////////////////////////
//
// MidiFile2::setupTrackStart --
//

void MidiFile2::setupTrackStart(void) {
   // get the first track location which is always the same
   if (midiData[14] == 'M' && midiData[15] == 'T' &&
       midiData[16] == 'r' && midiData[17] == 'k'    ) {
      trackStart[0] = 14;
      trackSize[0] = flip4bytes( *((ulong*)&midiData[18]) ); 
      currTrackIndex[0] = trackStart[0] + 8;
      trackLoopCount[0] = 0;
   } else {
      cerr << "Error: invalid track: 0" << endl;
      exit(1);
   }

   int counter = trackStart[0];
   int i;
   for (i=1; i<numTracks; i++) {
      counter += 8 + trackSize[i-i];
      if (midiData[counter] == 'M' && midiData[counter+1] == 'T' &&
          midiData[counter+2] == 'h' && midiData[counter+3] == 'd'    ) {
         trackStart[i] = counter;
         trackSize[i] = flip4bytes( *((ulong*)&midiData[counter+4]) ); 
         currTrackIndex[i] = trackStart[i] + 8;
         trackLoopCount[i] = 0;
      } else {
         cerr << "Error: invalid track: " << i << endl;
      }
   }

   if (counter + 8 + trackSize[i-1] != dataSize) {
      cerr << "unexpected end of file!" << endl;
   }
}
      

 
// md5sum: 95d42a2e7b7f72f020f3f2cdb0233381 MidiFile2.cpp [20050403]
