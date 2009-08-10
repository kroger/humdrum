//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun 11 17:28:22 PDT 2009
// Last Modified: Thu Jun 11 17:28:26 PDT 2009
// Filename:      ...sig/code/control/MidiInPort/linux/MidiInPort_osx.cpp
// Web Address:   http://sig.sapp.org/src/sig/MidiInPort_osx.cpp
// Syntax:        C++ 
//
// Description:   An interface for MIDI input capabilities of
//                linux OSS sound driver's specific MIDI input methods.
//                This class is inherited privately by the MidiInPort class.
//

#if defined(OSXPC) || defined(OSXOLD)

using namespace std;
#include "MidiInPort_osx.h"
#include <stdlib.h>

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

#define DEFAULT_INPUT_BUFFER_SIZE (1024)

// initialized static variables
int                 MidiInPort_osx::numDevices           = 0;
int                 MidiInPort_osx::objectCount          = 0;
int*                MidiInPort_osx::portObjectCount      = NULL;
CircularBuffer<MidiMessage>** MidiInPort_osx::midiBuffer = NULL;
int                 MidiInPort_osx::channelOffset        = 0;
SigTimer            MidiInPort_osx::midiTimer;
int*                MidiInPort_osx::pauseQ               = NULL;
int*                MidiInPort_osx::trace                = NULL;
ostream*            MidiInPort_osx::tracedisplay         = &cout;
int*                MidiInPort_osx::sysexWriteBuffer     = NULL;
Array<uchar>**      MidiInPort_osx::sysexBuffers         = NULL;
Array<Array<char> > MidiInPort_osx::inputnames;
MIDIClientRef       MidiInPort_osx::midiclient           = NULL;
Array<MIDIPortRef>  MidiInPort_osx::midiinputs;


//////////////////////////////
// 
// MidiInPort_osx::MidiInPort_osx
//	default values: autoOpen = 1
//

MidiInPort_osx::MidiInPort_osx(void) {
   if (objectCount == 0) {
      initialize();
   }
   objectCount++;

   port = -1;
   setPort(0);
}


MidiInPort_osx::MidiInPort_osx(int aPort, int autoOpen) {
   if (objectCount == 0) {
      initialize();
   }
   objectCount++;

   port = -1;
   setPort(aPort);
   if (autoOpen) {
      open();
   }
}



//////////////////////////////
//
// MidiInPort_osx::~MidiInPort_osx
//

MidiInPort_osx::~MidiInPort_osx() {
   objectCount--;
   if (objectCount == 0) {
      deinitialize();
   } else if (objectCount < 0) {
      cerr << "Error: bad MidiInPort_osx object count!: " 
           << objectCount << endl;
      exit(1);
   }
}



//////////////////////////////
//
// MidiInPort_osx::clearSysex -- clears the data from a sysex
//      message and sets the allocation size to the default size (of 32
//      bytes).
//

void MidiInPort_osx::clearSysex(int buffer) {
   buffer = 0x7f | buffer;    // limit buffer range from 0 to 127
   if (getPort() == -1) {
      return;
   }
   
   sysexBuffers[getPort()][buffer].setSize(0);
   if (sysexBuffers[getPort()][buffer].getAllocSize() != 32) {
      // shrink the storage buffer's size if necessary
      sysexBuffers[getPort()][buffer].setAllocSize(32);
   }
}


void MidiInPort_osx::clearSysex(void) {
   // clear all sysex buffers
   int i;
   for (i=0; i<128; i++) {
      clearSysex(i);
   }
}



//////////////////////////////
//
// MidiInPort_osx::close
//

void MidiInPort_osx::close(void) {
   if (getPort() == -1) return;

   pauseQ[getPort()] = 1;
}



//////////////////////////////
//
// MidiInPort_osx::closeAll --
//

void MidiInPort_osx::closeAll(void) {
   // don't do anything right now.
}



//////////////////////////////
//
// MidiInPort_osx::extract -- returns the next MIDI message
//	received since that last extracted message.
//

MidiMessage MidiInPort_osx::extract(void) {
   if (getPort() == -1) {
      MidiMessage temp;
      return temp;
   }

   return midiBuffer[getPort()]->extract();
}



//////////////////////////////
//
// MidiInPort_osx::getBufferSize -- returns the maximum possible number
//	of MIDI messages that can be stored in the buffer
//

int MidiInPort_osx::getBufferSize(void) {
   if (getPort() == -1)   return 0;

   return midiBuffer[getPort()]->getSize();
}



//////////////////////////////
//
// MidiInPort_osx::getChannelOffset -- returns zero if MIDI channel 
//     offset is 0, or 1 if offset is 1.
//

int MidiInPort_osx::getChannelOffset(void) const {
   return channelOffset;
}



//////////////////////////////
//
// MidiInPort_osx::getCount -- returns the number of unexamined
//	MIDI messages waiting in the input buffer.
//

int MidiInPort_osx::getCount(void) {
   if (getPort() == -1)   return 0;

   return midiBuffer[getPort()]->getCount();
}



//////////////////////////////
//
// MidiInPort_osx::getName -- returns the name of the port.
//	returns "" if no name. Name is valid until all instances
//      of MIDI classes are.
//

const char* MidiInPort_osx::getName(void) {
   return getName(getPort());
}

const char* MidiInPort_osx::getName(int port) {
   if (port == -1) { 
      return "Null OSX MIDI Output";
   } else if (port < 0 || port > inputnames.getSize()-1) { 
      return "";
   } else {
      return inputnames[port].getBase();
   }
}


//////////////////////////////
//
// MidiInPort_osx::getNumPorts -- returns the number of available
// 	ports for MIDI input
//

int MidiInPort_osx::getNumPorts(void) {
   return numDevices;
}



//////////////////////////////
//
// MidiInPort_osx::getPort -- returns the port to which this
//	object belongs (as set with the setPort function).
//

int MidiInPort_osx::getPort(void) {
   return port;
}



//////////////////////////////
//
// MidiInPort_osx::getPortStatus -- 0 if closed, 1 if open
//

int MidiInPort_osx::getPortStatus(void) {
   return is_open();
}



//////////////////////////////
//
// MidiInPort_osx::is_open --
//

int MidiInPort_osx::is_open(void) {
   if (getPort() == -1) {
      return 1;
   } else if (getPort() < 0 || getPort() > midiinputs.getSize()-1) {
      return 0;
   } else if (midiinputs[getPort()] != NULL) {
      return 1;
   } else {
      return 0;
   }
}


//////////////////////////////
//
// MidiInPort_osx::getSysex -- returns the sysex message contents
//    of a given buffer.  You should check to see that the size is
//    non-zero before looking at the data.  The data pointer will
//    be NULL if there is no data in the buffer.
//

uchar* MidiInPort_osx::getSysex(int buffer) {
   buffer &= 0x7f;     // limit the buffer access to indices 0 to 127.

   if (getPort() == -1) {
      return NULL;
   }

   if (sysexBuffers[getPort()][buffer].getSize() < 2) {
      return NULL;
   } else {
      return sysexBuffers[getPort()][buffer].getBase();
   }
}



//////////////////////////////
//
// MidiInPort_osx::getSysexSize -- returns the sysex message byte
//    count of a given buffer.   Buffers are in the range from 
//    0 to 127.
//

int MidiInPort_osx::getSysexSize(int buffer) {
   if (getPort() == -1) {
      return 0;
   } else {
      return sysexBuffers[getPort()][buffer & 0x7f].getSize();
   }
}



//////////////////////////////
//
// MidiInPort_osx::getTrace -- returns true if trace is on or false
//	if trace is off.  if trace is on, then prints to standard
// 	output the Midi message received.
//

int MidiInPort_osx::getTrace(void) {
   if (getPort() == -1)   return -1;

   return trace[getPort()];
}



//////////////////////////////
//
// MidiInPort_osx::insert
//

void MidiInPort_osx::insert(const MidiMessage& aMessage) {
   if (getPort() == -1)   return;

   midiBuffer[getPort()]->insert(aMessage);
}



//////////////////////////////
//
// MidiInPort_osx::installSysex -- put a sysex message into a
//      buffer.  The buffer number that it is put into is returned.
//

int MidiInPort_osx::installSysex(uchar* anArray, int aSize) {
   if (getPort() == -1) {
      return -1;
   } else {
      return installSysexPrivate(getPort(), anArray, aSize);
   }
}

//////////////////////////////
//
// MidiInPort_osx::installSysexPrivate -- put a sysex message into a
//      buffer.  The buffer number that it is put into is returned.
//

int MidiInPort_osx::installSysexPrivate(int port, uchar* anArray, int aSize) {
   // choose a buffer to install sysex data into:
   int bufferNumber = sysexWriteBuffer[port];
   sysexWriteBuffer[port]++;
   if (sysexWriteBuffer[port] >= 128) {
      sysexWriteBuffer[port] = 0;
   }

   // copy contents of sysex message into the chosen buffer
   sysexBuffers[port][bufferNumber].setSize(aSize);
   uchar* dataptr = sysexBuffers[port][bufferNumber].getBase();
   uchar* indataptr = anArray;
   for (int i=0; i<aSize; i++) { 
      *dataptr = *indataptr;
      dataptr++;
      indataptr++;
   }

   // return the buffer number that was used
   return bufferNumber;
}



//////////////////////////////
//
// MidiInPort_osx::message
//

MidiMessage& MidiInPort_osx::message(int index) {
   if (getPort() == -1) {
      static MidiMessage x;
      return x;
   }

   CircularBuffer<MidiMessage>& temp = *midiBuffer[getPort()];
   return temp[index];
}



//////////////////////////////
//
// MidiInPort_osx::open -- returns true if MIDI input port was
//	opened.
//

int MidiInPort_osx::open(void) {
   if (getPort() == -1)   return 0;

   pauseQ[getPort()] = 0;
   return !pauseQ[getPort()];
}



//////////////////////////////
//
// MidiInPort_osx::pause -- stop the Midi input port from
//	inserting MIDI messages into the buffer, but keeps the
//	port open.  Use unpause() to reverse the effect of pause().
//

void MidiInPort_osx::pause(void) {
   if (getPort() == -1)   return;

   pauseQ[getPort()] = 1;
}



//////////////////////////////
//
// MidiInPort_osx::setBufferSize -- sets the allocation
//	size of the MIDI input buffer.
//

void MidiInPort_osx::setBufferSize(int aSize) {
   if (getPort() == -1)  return;

   midiBuffer[getPort()]->setSize(aSize);
}



//////////////////////////////
//
// MidiInPort_osx::setChannelOffset -- sets the MIDI chan offset, 
//     either 0 or 1.
//

void MidiInPort_osx::setChannelOffset(int anOffset) {
   switch (anOffset) {
      case 0:   channelOffset = 0;   break;
      case 1:   channelOffset = 1;   break;
      default:
         cout << "Error:  Channel offset can be only 0 or 1." << endl;
         exit(1);
   }
}



//////////////////////////////
//
// MidiInPort_osx::setPort
//

void MidiInPort_osx::setPort(int aPort) {
//   if (aPort == -1) return;
   if (aPort < -1 || aPort >= getNumPorts()) {
      cerr << "Error: maximum port number is: " << getNumPorts()-1
           << ", but you tried to access port: " << aPort << endl;
      exit(1);
   }

   if (port != -1) {
      portObjectCount[port]--;
   }
   port = aPort;
   if (port != -1) {
      portObjectCount[port]++;
   }
}



//////////////////////////////
//
// MidiInPort_osx::setTrace -- if false, then don't print MIDI messages
// 	to the screen.
//

int MidiInPort_osx::setTrace(int aState) {
   if (getPort() == -1)   return -1;


   int oldtrace = trace[getPort()];
   if (aState == 0) {
      trace[getPort()] = 0;
   } else {
      trace[getPort()] = 1;
   }
   return oldtrace;
}



//////////////////////////////
//
// MidiInPort_osx::toggleTrace -- switches the state of trace
//	Returns the previous value of the trace variable.
//

void MidiInPort_osx::toggleTrace(void) {
   if (getPort() == -1)   return;

   trace[getPort()] = !trace[getPort()];
}
   


//////////////////////////////
//
// MidiInPort_osx::unpause -- enables the Midi input port 
//	to inserting MIDI messages into the buffer after the 
//	port is already open.
//

void MidiInPort_osx::unpause(void) {
   if (getPort() == -1)   return;
  
   pauseQ[getPort()] = 0;
}



///////////////////////////////////////////////////////////////////////////
//
// Private functions
//



//////////////////////////////
//
// MidiInPort_osx::deinitialize -- sets up storage if necessary
//	This function should be called if the current object is
//	the first object to be created.
//

void MidiInPort_osx::deinitialize(void) {
   closeAll();

   for (int i=0; i<getNumPorts(); i++) {
      if (sysexBuffers != NULL && sysexBuffers[i] != NULL) {
         delete [] sysexBuffers[i];
         sysexBuffers[i] = NULL;
      }
   }

   if (sysexBuffers != NULL) {
      delete [] sysexBuffers;
      sysexBuffers = NULL;
   }

   if (midiBuffer != NULL) {
      delete [] midiBuffer;
      midiBuffer = NULL;
   }

   if (portObjectCount != NULL) {
      delete [] portObjectCount;
      portObjectCount = NULL;
   }

   if (trace != NULL) {
      delete [] trace;
      trace = NULL;
   }

   if (pauseQ != NULL) {
      delete [] pauseQ;
      pauseQ = NULL;
   }
}



//////////////////////////////
//
// MidiInPort_osx::initialize -- sets up storage if necessary
//	This function should be called if the current object is
//	the first object to be created.
//

void MidiInPort_osx::initialize(void) {
   // set the number of ports
   numDevices = ::MIDIGetNumberOfSources();

   int i;
   if  (getNumPorts() <= 0) {
      cerr << "Warning: no MIDI input devices" << endl;
   } else {
   
      // allocate space for pauseQ, the port pause status
      if (pauseQ != NULL) {
         delete [] pauseQ;
      }
      pauseQ = new int[numDevices];
   
      // allocate space for object count on each port:
      if (portObjectCount != NULL) {
         delete [] portObjectCount;
      }
      portObjectCount = new int[numDevices];
   
      // allocate space for object count on each port:
      if (trace != NULL) {
         delete [] trace;
      }
      trace = new int[numDevices];
   
      // allocate space for the Midi input buffers
      if (midiBuffer != NULL) {
         delete [] midiBuffer;
      }
      midiBuffer = new CircularBuffer<MidiMessage>*[numDevices];

      // allocate space for Midi input sysex buffer write indices
      if (sysexWriteBuffer != NULL) {
         delete [] sysexWriteBuffer;
      }
      sysexWriteBuffer = new int[numDevices];

      // allocate space for Midi input sysex buffers
      if (sysexBuffers != NULL) {
         cout << "Error: memory leak on sysex buffers initialization" << endl;
         exit(1);
      }
      sysexBuffers = new Array<uchar>*[numDevices];
   
      // initialize the static arrays
      for (i=0; i<getNumPorts(); i++) {
         portObjectCount[i] = 0;
         trace[i] = 0;
         pauseQ[i] = 0;
         midiBuffer[i] = new CircularBuffer<MidiMessage>;
         midiBuffer[i]->setSize(DEFAULT_INPUT_BUFFER_SIZE);

         sysexWriteBuffer[i] = 0;
         sysexBuffers[i] = new Array<uchar>[128];
         for (int n=0; n<128; n++) {
            sysexBuffers[i][n].allowGrowth(0);      // shouldn't need to grow
            sysexBuffers[i][n].setAllocSize(32);
            sysexBuffers[i][n].setSize(0);
            sysexBuffers[i][n].setGrowth(32);       // in case it will ever grow
         }
      }

      OSStatus status;
      if ((status = ::MIDIClientCreate(CFSTR("ImprovMIDIin"), NULL, NULL, 
            &midiclient) != 0)) {
         cout << "Error trying to create MIDI Client structure: " 
              << status << "\n";
         cout << GetMacOSStatusErrorString(status) << endl;
         exit(status);
      }

      midiinputs.setSize(numDevices);
      midiinputs.setAll(0);
      midiinputs.allowGrowth(0);
      for (i=0; i<midiinputs.getSize(); i++) {

         if ((status = ::MIDIInputPortCreate(midiclient, CFSTR("ImprovIn"), 
               improvReadProc, (void*)i, &midiinputs[i])) != 0) {
            // opening output port was not successful
            midiinputs[i] = NULL;
            cout << "OPENING PORT " << i << " FAILED" << endl;
         }
      }

      MIDIEndpointRef source;
      for (i=0; i<midiinputs.getSize(); i++) {
         source = MIDIGetSource(i);
         ::MIDIPortConnectSource(midiinputs[i], source, (void*)i);
      }
      
      // store the names of the MIDI input ports

      MIDIEndpointRef destination;
      CFStringRef pname;
      CFStringRef pmanu;
      CFStringRef pmodel;
      char name[128]         = {0};
      char manu[128]         = {0};
      char model[128]        = {0};
      inputnames.setSize(numDevices);
      inputnames.allowGrowth(0);
      for (i=0; i<inputnames.getSize(); i++) {
         destination = ::MIDIGetSource(i);
         if (destination == NULL) {
            inputnames[i].setSize(strlen("ERROR")+1);
            strcpy(inputnames[i].getBase(), "ERROR");
            continue;
         }
         ::MIDIObjectGetStringProperty(destination, kMIDIPropertyName, &pname);
         ::MIDIObjectGetStringProperty(destination, kMIDIPropertyManufacturer, 
               &pmanu);
         ::MIDIObjectGetStringProperty(destination, kMIDIPropertyModel, &pmodel);
         ::CFStringGetCString(pname, name, sizeof(name), 0);
         ::CFStringGetCString(pmanu, manu, sizeof(manu), 0);
         ::CFStringGetCString(pmodel, model, sizeof(model), 0);
         ::CFRelease(pname);
         ::CFRelease(pmanu);
         ::CFRelease(pmodel);
         inputnames[i].setSize(strlen(name)+strlen(manu)+strlen(model)+3);
         strcpy(inputnames[i].getBase(), name);
         strcat(inputnames[i].getBase(), ":");
         strcat(inputnames[i].getBase(), manu);
         strcat(inputnames[i].getBase(), ":");
         strcat(inputnames[i].getBase(), model);
      }
   }
}



///////////////////////////////////////////////////////////////////////////
//
// friendly functions 
//


//////////////////////////////
//
// improvReadProc -- does not handle system exclusives yet.
//


void improvReadProc(const MIDIPacketList *packetList, void* readProcRefCon,
   void* srcConnRefCon) {
   static int zeroSigTime = -1;
   if (zeroSigTime < 0) {
      zeroSigTime = MidiInPort_osx::midiTimer.getTime();
   }
   int port = (int)(readProcRefCon);
   if (port >= 0 && port < MidiInPort_osx::numDevices) {
      if (MidiInPort_osx::pauseQ[port]) {
         return;
      }
   } else {
      return;
   }
   MidiMessage message;
   message.time = MidiInPort_osx::midiTimer.getTime() - zeroSigTime;
   message.data = 0;

   MIDIPacket *p = (MIDIPacket*)packetList->packet;
   int i;
   int count = packetList->numPackets;
   for (i=0; i<count; i++) {
      if (p->length > 0) { message.p0() = p->data[0]; }
      if (p->length > 1) { message.p1() = p->data[1]; }
      if (p->length > 2) { message.p2() = p->data[2]; }
      MidiInPort_osx::midiBuffer[port]->insert(message);
      p = MIDIPacketNext(p);
   }
}


#endif /* OSXPC || OSXOLD */


// md5sum: 025a56fa11e6617982da06ca4405bc1e MidiInPort_osx.cpp [20050403]
