//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jun 10 17:22:37 PDT 2009
// Last Modified: Thu Jun 11 11:36:53 PDT 2009
// Filename:      ...sig/code/control/MidiOutPort/linux/MidiOutPort_osx.cpp
// Web Address:   http://sig.sapp.org/src/sig/MidiOutPort_osx.cpp
// Syntax:        C++ 
//
// Description:   Operating-System specific interface for basic MIDI output 
//                capabilities in Apple Macintosh using the CoreMIDI 
//                interface.  This class is Privately inherited by the
//                MidiOutPort class.
// 

#if defined(OSXPC) || defined(OSXOLD)

// needed for GetMacOSStatusErrorString:
#include "/System/Library/Frameworks/CoreServices.framework/Versions/Current/Frameworks/CarbonCore.framework/Versions/Current/Headers/Debugging.h"

using namespace std;
#include "MidiOutPort_osx.h"

#include <stdlib.h>

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

// initialized static variables
int                 MidiOutPort_osx::initializeQ     = 0;
int                 MidiOutPort_osx::numDevices      = 0;
int*                MidiOutPort_osx::portObjectCount = NULL;
int                 MidiOutPort_osx::channelOffset   = 0;
int*                MidiOutPort_osx::trace           = NULL;
ostream*            MidiOutPort_osx::tracedisplay    = &cout;
MIDIClientRef       MidiOutPort_osx::midiclient      = NULL;
Array<MIDIPortRef>  MidiOutPort_osx::midioutputs; 
Array<Array<char> > MidiOutPort_osx::outputnames(0);


//////////////////////////////
// 
// MidiOutPort_osx::MidiOutPort_osx --
//	default values: autoOpen = 1
//

MidiOutPort_osx::MidiOutPort_osx(void) {
   if (initializeQ == 0) {
      initialize();
   }

   port = -1;
   setPort(0);
}


MidiOutPort_osx::MidiOutPort_osx(int aPort, int autoOpen) {
   if (initializeQ == 0) {
      initialize();
   }

   port = -1;
   setPort(aPort);
   if (autoOpen) {
      open();
   }
}



//////////////////////////////
//
// MidiOutPort_osx::~MidiOutPort_osx --
//

MidiOutPort_osx::~MidiOutPort_osx() {
    // don't do anything when destrying object
}



//////////////////////////////
//
// MidiOutPort_osx::close --
//

void MidiOutPort_osx::close(void) {
   // don't really need to close MIDI outputs in OS X
}



//////////////////////////////
//
// MidiOutPort_osx::closeAll --
//

void MidiOutPort_osx::closeAll(void) {
   // don't really need to close MIDI outputs in OS X
}



//////////////////////////////
//
// MidiOutPort_osx::getChannelOffset -- returns zero if MIDI channel 
//     offset is 0, or 1 if offset is 1.
//

int MidiOutPort_osx::getChannelOffset(void) const {
   return channelOffset;
}



//////////////////////////////
//
// MidiOutPort_osx::getName -- returns the name of the port.
//	returns "" if no name. Name is valid until getName is called again.
//

const char* MidiOutPort_osx::getName(void) {
   return getName(getPort());
}


const char* MidiOutPort_osx::getName(int port) {
   static char buffer[1024] = {0};
   buffer[0] = '\0';
   if (port == -1) { 
      return "Null OSX MIDI Output";
   } else if (port < 0 || port > getNumOutputs()-1) { 
      return "ERROR: Port out of range";
   } else {
      // store the names of the devices.
      MIDIEndpointRef destination;
      CFStringRef pname;
      CFStringRef pmanu;
      CFStringRef pmodel;
      char name[128]   = {0};
      char manu[128]   = {0};
      char model[128]  = {0};
      
      destination = ::MIDIGetDestination(port);
      if (destination == NULL) {
         strcpy(buffer, "ERROR");
         return buffer;
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
      strcpy(buffer, name);
      strcat(buffer, ":");
      strcat(buffer, manu);
      strcat(buffer, ":");
      strcat(buffer, model);
   }

   return buffer;
}



//////////////////////////////
//
// MidiOutPort_osx::getNumPorts -- returns the number of available
// 	ports for MIDI output.  
//

int MidiOutPort_osx::getNumPorts(void) {
   return numDevices;
}



//////////////////////////////
//
// MidiOutPort_osx::getPort -- returns the port to which this
//	object belongs (as set with the setPort function).
//

int MidiOutPort_osx::getPort(void) {
   return port;
}



//////////////////////////////
//
// MidiOutPort_osx::getPortStatus -- 0 if closed, 1 if open
//

int MidiOutPort_osx::getPortStatus(void) {
   return is_open();
}



//////////////////////////////
//
// MidiOutPort_osx::getTrace -- returns true if trace is on or
//	false if off.  If trace is on, then prints to standard output
//	the Midi message being sent.
//

int MidiOutPort_osx::getTrace(void) {
   if (getPort() == -1) return -1;

   return trace[getPort()];
}



//////////////////////////////
//
// MidiOutPort_osx::rawsend -- send the Midi command and its parameters
//

int MidiOutPort_osx::rawsend(int command, int p1, int p2) {

   if (getPort() == -1) return 1;
   if (getPort() < 0 || getPort() > numDevices-1) {
      return 0;
   }

   int status;
   MIDIEndpointRef dest;
   dest = ::MIDIGetDestination(getPort());
   Byte packetbuffer[128];
   MIDIPacketList *packetlist = (MIDIPacketList*)packetbuffer;
   MIDIPacket     *packet     = ::MIDIPacketListInit(packetlist);
   Byte mdata[3] = {(uchar)command, (uchar)p1, (uchar)p2};
   packet = ::MIDIPacketListAdd(packetlist, sizeof(packetbuffer),
      packet, 0, 3, mdata);
   status = ::MIDISend(midioutputs[getPort()], dest, packetlist);
   status = !status;

   if (getTrace()) {
      if (status == 1) {
         cout << "(" << hex << (int)mdata[0] << dec << ":"
              << (int)mdata[1] << "," << (int)mdata[2] << ")";
         cout.flush();
      } else {
         cout << "(" << hex << (int)mdata[0] << dec << "X"
              << (int)mdata[1] << "," << (int)mdata[2] << ")";
         cout.flush();
      }
   }      

   return status;
}


int MidiOutPort_osx::rawsend(int command, int p1) {
   if (getPort() == -1) return 1;
   if (getPort() < 0 || getPort() > numDevices-1) {
      return 0;
   }

   int status;
   MIDIEndpointRef dest;
   dest = ::MIDIGetDestination(getPort());
   Byte packetbuffer[128];
   MIDIPacketList *packetlist = (MIDIPacketList*)packetbuffer;
   MIDIPacket     *packet     = ::MIDIPacketListInit(packetlist);
   Byte mdata[2] = {(uchar)command, (uchar)p1};
   packet = ::MIDIPacketListAdd(packetlist, sizeof(packetbuffer),
      packet, 0, 2, mdata);
   status = ::MIDISend(midioutputs[getPort()], dest, packetlist);
   status = !status;

   if (getTrace()) {
      if (status == 1) {
         cout << "(" << hex << (int)mdata[0] << dec << ":"
              << (int)mdata[1] << ")";
         cout.flush();
      } else {
         cout << "(" << hex << (int)mdata[0] << dec << "X"
              << (int)mdata[1] << "," << (int)mdata[2] << ")";
         cout.flush();
      }
   }
 
   return status;
}


int MidiOutPort_osx::rawsend(int command) {
   if (getPort() == -1) return 1;
   if (getPort() < 0 || getPort() > numDevices-1) {
      return 0;
   }

   int status;
   MIDIEndpointRef dest;
   dest = ::MIDIGetDestination(getPort());
   Byte packetbuffer[128];
   MIDIPacketList *packetlist = (MIDIPacketList*)packetbuffer;
   MIDIPacket     *packet     = ::MIDIPacketListInit(packetlist);
   Byte mdata[1] = {(uchar)command};
   packet = ::MIDIPacketListAdd(packetlist, sizeof(packetbuffer),
      packet, 0, 1, mdata);
   status = ::MIDISend(midioutputs[getPort()], dest, packetlist);
   status = !status;

   if (getTrace()) {
      if (status == 1) {
         cout << "(" << hex << (int)mdata[0] << ")";
         cout.flush();
      } else {
         cout << "(" << hex << (int)mdata[0] << ")";
         cout.flush();
      }
   }

   return status;
}


int MidiOutPort_osx::rawsend(uchar* array, int size) {
//  probably only works with complete messages.
//  probably does not allow for system exclusive messages
//  which have to be done a different way in CoreMIDI.

   if (getPort() == -1) return 1;
   if (getPort() < 0 || getPort() > numDevices-1) {
      return 0;
   }

   int status;
   MIDIEndpointRef dest;
   dest = ::MIDIGetDestination(getPort());
   Array<Byte> packetbuffer(size+100);
   MIDIPacketList *packetlist = (MIDIPacketList*)packetbuffer.getBase();
   MIDIPacket     *packet     = ::MIDIPacketListInit(packetlist);
   packet = ::MIDIPacketListAdd(packetlist, size+100, packet, 0, size, array);
   status = ::MIDISend(midioutputs[getPort()], dest, packetlist);
   status = !status;

   if (getTrace()) {
      if (status == 1) {
         cout << "(array)";
         cout.flush();
      } else {
         cout << "(XarrayX)";
         cout.flush();
      }
   }

   return status;
}



//////////////////////////////
//
// MidiOutPort_osx::open -- returns true if MIDI output port was
//	opened.
//

int MidiOutPort_osx::open(void) {
   if (getPort() == -1) {
      return 1;
   } else if (getPort() < 0 || getPort() > numDevices-1) {
      return 0;
   }

   if (midioutputs[getPort()] != NULL) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// MidiOutPort_osx::setChannelOffset -- sets the MIDI channel offset, 
//     either 0 or 1.
//

void MidiOutPort_osx::setChannelOffset(int anOffset) {
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
// MidiOutPort_osx::setPort --
//

void MidiOutPort_osx::setPort(int aPort) {
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
// MidiOutPort_osx::setTrace -- if false, then won't print
//      Midi messages to standard output.
//

int MidiOutPort_osx::setTrace(int aState) {
   if (getPort() == -1) return -1;

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
// MidiOutPort_osx::sysex -- send a system exclusive message.
//     The message must start with a 0xf0 byte and end with
//     a 0xf7 byte.
//

int MidiOutPort_osx::sysex(uchar* array, int size) {
   if (size == 0 || array[0] != 0xf0) {
      cout << "Error: invalid sysex message" << endl;
      exit(1);
   }

   // will have to fix this probably for OS X CoreMIDI
   return rawsend(array,size);
}



//////////////////////////////
//
// MidiOutPort_osx::toggleTrace
//

void MidiOutPort_osx::toggleTrace(void) {
   if (getPort() == -1) return;

   trace[getPort()] = !trace[getPort()];
}



///////////////////////////////////////////////////////////////////////////
//
// Private functions
//



//////////////////////////////
//
// MidiOutPort_osx::deinitialize -- sets up storage if necessary
//	This function should be called if the current object is
//	the first object to be created.
//

void MidiOutPort_osx::deinitialize(void) {
   closeAll();
   if (portObjectCount != NULL) delete [] portObjectCount;
   portObjectCount = NULL;
   if (trace != NULL) delete [] trace;
   trace = NULL;

   int i;
   for (i=0; i<midioutputs.getSize(); i++) {
      if (midioutputs[i] != NULL) {
         ::MIDIPortDispose(midioutputs[i]);
         midioutputs[i] = NULL;
      }
   }
   midioutputs.setSize(0);

   ::MIDIClientDispose(midiclient); 
   midiclient = NULL;
}



//////////////////////////////
//
// MidiOutPort_osx::initialize -- sets up storage if necessary
//	This function should be called if the current object is
//	the first object to be created.
//

void MidiOutPort_osx::initialize(void) {
   if (initializeQ != 0) {
      deinitialize();
   }
   // get the number of ports
   numDevices = getNumOutputs();
   int i;

   if  (getNumPorts() <= 0) {
      cerr << "Warning: no MIDI output devices" << endl;
      portObjectCount = NULL;
      trace = NULL;
   } else {
      OSStatus status;
      if ((status = ::MIDIClientCreate(CFSTR("ImprovMIDIout"), NULL, NULL, 
            &midiclient)) != 0) {
         cout << "Error trying to create MIDI Client structure: " 
              << status << "\n";
         cout << GetMacOSStatusErrorString(status) << endl;
         exit(status);
      }
      // For MIDI output in OSX, there is no reason to wait to open
      // the ports, so just open all of them now.

      midioutputs.setSize(numDevices);
      midioutputs.setAll(0);
      midioutputs.allowGrowth(0);
      for (i=0; i<midioutputs.getSize(); i++) {
         if ((status = ::MIDIOutputPortCreate(midiclient, CFSTR("ImprovOut"), 
               &midioutputs[i])) != 0) {
            // opening output port was not successful
            midioutputs[i] = NULL;
         }
      }

      // store the names of the devices.
      MIDIEndpointRef destination;
      CFStringRef pname;
      CFStringRef pmanu;
      CFStringRef pmodel;
      char name[128]         = {0};
      char manu[128] = {0};
      char model[128]        = {0};
      MidiOutPort_osx::outputnames.setSize(numDevices);
      MidiOutPort_osx::outputnames.allowGrowth(0);
      for (i=0; i<MidiOutPort_osx::outputnames.getSize(); i++) {
         destination = ::MIDIGetDestination(i);
         if (destination == NULL) {
            MidiOutPort_osx::outputnames[i].setSize(strlen("ERROR")+1);
            strcpy(MidiOutPort_osx::outputnames[i].getBase(), "ERROR");
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
         MidiOutPort_osx::outputnames[i].setSize(strlen(name)+strlen(manu)+strlen(model)+3+100);
         strcpy(MidiOutPort_osx::outputnames[i].getBase(), name);
         strcat(MidiOutPort_osx::outputnames[i].getBase(), ":");
         strcat(MidiOutPort_osx::outputnames[i].getBase(), manu);
         strcat(MidiOutPort_osx::outputnames[i].getBase(), ":");
         strcat(MidiOutPort_osx::outputnames[i].getBase(), model);
      }

      // allocate space for object count on each port:
      if (portObjectCount != NULL) delete [] portObjectCount;
      portObjectCount = new int[numDevices];
   
      // allocate space for trace variable for each port:
      if (trace != NULL) delete [] trace;
      trace = new int[numDevices];
   
      // initialize the static arrays
      for (int i=0; i<getNumPorts(); i++) {
         portObjectCount[i] = 0;
         trace[i] = 0;
      }
   }
}



//////////////////////////////
//
// MidiOutPort_osx::setPortStatus --
//

void MidiOutPort_osx::setPortStatus(int aStatus) {
   // not used in OSX implementation
}



//////////////////////////////
//
// MidiOutPort_osx::getNumOutputs -- returns the number of available
// 	ports for MIDI output.  Calls the CoreMIDI function
//      MIDIGetNumberOfDestinations() which answers the question.
//

int MidiOutPort_osx::getNumOutputs(void) {
   return ::MIDIGetNumberOfDestinations();
}



//////////////////////////////
//
// MidiOutPort_osx::is_open --
//

int MidiOutPort_osx::is_open(void) {
   if (initializeQ == 0) {
      initialize();
   }
   if (getPort() == -1) {
      return 1;
   } else if (getPort() < 0 || getPort() > midioutputs.getSize()-1) {
      return 0;
   } else if (midioutputs[getPort()] != NULL) {
      return 1;
   } else {
      return 0;
   }
}


#endif  /* OSXPC || OSXOLD */

// md5sum: a427b7ac36a4652dbaa4f915dda9bcba MidiOutPort_osx.cpp [20030102]
