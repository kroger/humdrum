//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct 26 15:58:30 PST 1998
// Last Modified: Mon Oct 26 15:58:34 PST 1998
// Last Modified: 20 Nov 1998
// Filename:      ...sig/maint/code/control/Nidaq/Nidaq.cpp
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/src/Nidaq.cpp
// Syntax:        C++; LabWindows/CVI libraries
//
// Description:   This class can handle continuous sampling from the NIDAQ.
//                Will work with up to 16 channels possible on the card.
//                There is probably a driver bug in that the multiple channels
//                must be consecutive starting at channel 0.
// 

#ifdef VISUAL
#ifdef NIDAQ

#include "Nidaq.h"

#include <easyio.h>    /* for AI... sampling functions */

#include <string.h>
#include <stdio.h>

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif


///////////////////////////////
//
// Nidaq::Nidaq
//

Nidaq::Nidaq(void) {
   device = 1;
   targetSampleRate = 200;
   actualSampleRate = 0;
   taskID = 0xffffffff;
   error = 0;
   samplingState = 0;
   channelString[0] = '\0';
   frameSize = 0;
   tempFrameSize = 8;
   frameData = NULL;
   numRunningChannels = 0;
   mode = CONSECUTIVE_MODE;
   defaultMaxVoltage = 10.0;
   defaultMinVoltage = 0;     // or could go down to -10.0 volts

   for (int i=0; i<16; i++) {
      channelState[i] = 0;
      channelIndex[i] = -1;
      minvolt[i] = getDefaultMinVoltage();
      maxvolt[i] = getDefaultMaxVoltage();
   }
}



///////////////////////////////
//
// Nidaq::~Nidaq
//

Nidaq::~Nidaq() {
   stop();

   if (frameData != NULL) {
      delete [] frameData;
   }
}



///////////////////////////////
//
// Nidaq::activateAllChannels -- select all channels for sampling
//

void Nidaq::activateAllChannels(void) {
   if (is_running()) {
      cerr << "Warning: cannot change channel status while sampling." << endl;
   } else {
      for (int i=0; i<16; i++) {
         channelState[i] = 1;
      }
   }
}

     

///////////////////////////////
//
// Nidaq::acquireData -- transfer a full frame if is ready from
//     the NIDAQ card.  Returns the frame number.  If a full frame
//     is not ready, then it will not do anything, and the frameCount
//     will be the same as the last call to acquireData
//

int Nidaq::aquireData(void) {
   if (!is_running()) {
      // not continuously sampling
      return -1;
   }
  
   ulong samplesWaiting;
   error = AICheckAcquisition(taskID, &samplesWaiting);

   if ((int)samplesWaiting >= getFrameSize()) {
      error = AIReadAcquisition(taskID, getFrameSize(), getMode(), 
         &samplesWaiting, GROUP_BY_CHANNEL, frameData);
      frameCount++;
   }

   return frameCount;
}



///////////////////////////////
//
// Nidaq::checkPoll -- see if it is time to check for new data
//      frame.  If so, then call aquireData.
//

int Nidaq::checkPoll(void) {
   if (pollTimer.expired()) {
      pollTimer.reset();
      return aquireData();
   } else {
      return 0;
   }
}



///////////////////////////////
//
// Nidaq::getBufferSize -- returns the size of the NIDAQ's internal
//     circular buffer.
//

int Nidaq::getBufferSize(void) const {
   return internalBufferSize;
}



///////////////////////////////
//
// Nidaq::getChannelState -- 
//

int Nidaq::getChannelState(int index) const {
   verifyChannelIndex(index);
   return channelState[index];
}



///////////////////////////////
//
// Nidaq::getChannelString --
//

const char* Nidaq::getChannelString(void) const {
   return channelString;
}



///////////////////////////////
//
// Nidaq::getDefaultMaxVoltage --
//

double Nidaq::getDefaultMaxVoltage(void) const {
   return defaultMaxVoltage;
}



///////////////////////////////
//
// Nidaq::getDefaultMinVoltage --
//

double Nidaq::getDefaultMinVoltage(void) const {
   return defaultMinVoltage;
}


///////////////////////////////
//
// Nidaq::getDevice -- the device number of the NIDAQ card being
//     used to sample data.
//

int Nidaq::getDevice(void) const {
   return device;
}



///////////////////////////////
//
// getFrameCount -- 
//

int Nidaq::getFrameCount(void) const {
   return frameCount;
}



///////////////////////////////
//
// Nidaq::getFrameSize -- the size of the object's buffer which
//     is used to transfer data from the card.  This is the number
//     of samples used for each channel.
//

int Nidaq::getFrameSize(void) const {
   return frameSize;
}



///////////////////////////////
//
// getMode --
//

int Nidaq::getMode(void) const {
   return mode;
}



///////////////////////////////
//
// Nidaq::getPollPeriod -- returns the poll period time used
//     by checkPoll function.
//

double Nidaq::getPollPeriod(void) const {
   return pollTimer.getPeriod();
}



///////////////////////////////
//
// Nidaq::getSrate -- returns the sampling rate begin used
//    for measuring data on the NIDAQ.
//

double Nidaq::getSrate(void) const {
   return actualSampleRate;
}



///////////////////////////////
//
// Nidaq::is_running -- return 0 if not aquiring data, otherwise
//    will return 1 when sampling data
//

int Nidaq::is_running(void) const {
   return samplingState;
}



///////////////////////////////
//
// Nidaq::initialize -- the initialization of the NIDAQ stuff
//    for accessing the library functions for LabWindows/CVI.
//

void Nidaq::initialize(char** argv) {
   if (InitCVIRTE(0, argv, 0) == 0) {
      // out of memory
      exit(-1);
   }
}



///////////////////////////////
//
// Nidaq::operator[] -- accesses the frame data for the specified 
//     channel.  Program will die if an inactive channel is accessed.
//

double* Nidaq::operator[](int index) {
   verifyChannelIndex(index);
   
   if (channelState[index] == 0) {
      cerr << "Error: channel " << index << " is not actively sampling" << endl;
      exit(1);
   }

   return &frameData[indexmap[index]];
}



//////////////////////////////
//
// Nidaq::selectChannel -- activates the sampling of a channel
//     when start() function is called. (selection is ignored 
//     while sampling.
//

void Nidaq::selectChannel(int aChannel) {
   if (!is_running()) {
      verifyChannelIndex(aChannel);
      channelState[aChannel] = 1;
   }
}



//////////////////////////////
//
// Nidaq::setFrameSize -- 
//

void Nidaq::setFrameSize(int aSize) {
   if (aSize < 0) {
      cerr << "Error: ridiculous size for nidaq buffer: " << aSize << endl;
      exit(1);
   }

   if (internalBufferSize > aSize) {
      cerr << "Warning: frame size is larger than buffer size" << endl;
   } 
   tempFrameSize = aSize;
}



///////////////////////////////
//
// Nidaq::setDevice -- sets the NIDAQ device being used for
//     data aquisition.
//

void Nidaq::setDevice(int aDevice) {
   if (is_running()) {
      stop();
      device = aDevice;
      start();
   } else {
      device = aDevice;
   }
}



//////////////////////////////
//
// Nidaq::setModeLatest --
//

void Nidaq::setModeLatest(void) {
   setMode(LATEST_MODE);
}



//////////////////////////////
//
// Nidaq::setModeContinuous --
//

void Nidaq::setModeContinuous(void) {
   setMode(CONSECUTIVE_MODE);
}



//////////////////////////////
//
// Nidaq::setMode --
//

void Nidaq::setMode(int aMode) {
   switch (aMode) {
      case CONSECUTIVE_MODE:   mode = CONSECUTIVE_MODE;   break;
      case LATEST_MODE:        mode = LATEST_MODE;       break;
      default:
         // don't do anything
         break;
   }
}



//////////////////////////////
//
// Nidaq::setFrameCount --
//

void Nidaq::setFrameCount(int aCount) {
   frameCount = aCount;
}



//////////////////////////////
//
// Nidaq::setPollPeriod -- sets the polling period of the
//   class.
//

void Nidaq::setPollPeriod(double aPeriod) {
   pollTimer.setPeriod(aPeriod);
}



//////////////////////////////
//
// Nidaq::setScaleRange -- sets the channel scale on the user interface 
//     to the specified values.
//

void Nidaq::setScaleRange(int aChannel, double min, double max) {
   verifyChannelIndex(aChannel);

   if (min < -10.0)   min = -10.0;
   if (min >  10.0)   min =  10.0;
   if (max < -10.0)   max = -10.0;
   if (max >  10.0)   max =  10.0;
   
   if (min < max) {
      minvolt[aChannel] = min;
      minvolt[aChannel] = max;
   } else {
      minvolt[aChannel] = max;
      minvolt[aChannel] = min;
   }
}

  

//////////////////////////////
//
// Nidaq::setSrate -- sets the sampling rate of the data
//   aquisition.
//

void Nidaq::setSrate(double anSrate) {
   if (is_running()) {
      stop();
      targetSampleRate = anSrate;
      start();
   } else {
      targetSampleRate = anSrate;
   }
}



//////////////////////////////
//
// Nidaq::start -- starts the aquisition of data.
//

void Nidaq::start(void) {
   if (!is_running()) {

      numRunningChannels = createChannelString(channelString, 
         channelState, minvolt, maxvolt);

      // adjust the size of the frame data storage
      if (frameData != NULL) {
         delete [] frameData;
      }

      frameSize = tempFrameSize;
      frameData = new double[frameSize * numRunningChannels];
      frameCount = 0;
      internalBufferSize = 2 * frameSize;

      error = AIStartAcquisition(getDevice(), channelString, 
            getBufferSize(), targetSampleRate, getDefaultMinVoltage(), 
            getDefaultMaxVoltage(), &actualSampleRate, &taskID);

      samplingState = 1;
   }
}
       


///////////////////////////////
//
// Nidaq::stop -- stops the aquisition of data.
//

void Nidaq::stop(void) {
   if (is_running()) {
      error = AIClearAcquisition(taskID);
      taskID = 0xffffffff;
      samplingState = 0;
   }
}



//////////////////////////////
//
// Nidaq::unselectChannel -- deactivates a display of a particular
//     channel.
//

void Nidaq::unselectChannel(int aChannel) {
   if (!is_running()) {
      verifyChannelIndex(aChannel);
      channelState[aChannel] = 0;
   }
}



///////////////////////////////////////////////////////////////////////////
//
// Private functions:
//


//////////////////////////////
//
// createChannelString -- creates the channel initialization for
//     AIStartAcquisition function.
//

int Nidaq::createChannelString(char* string, int* chanStates,
      double* min, double* max) {
   int count = 0;
   char temp[128] = {0};
   string[0] = '\0';
   int i;
   for (i=0; i<16; i++) {
      if (chanStates[i] == 1) {
         channelIndex[count++] = i;
         strcat(string, " cmd hi ");
         sprintf(temp, "%f", max[i]);
         strcat(string, temp);
         strcat(string, " low ");
         sprintf(temp, "%f", min[i]);
         strcat(string, temp);
         strcat(string, ";");
         sprintf(temp, "%d", i);
         strcat(string, temp);
         strcat(string, ";");
      }
   }

   int output = count;
   for (i=count; i<16; i++) {
      channelIndex[i] = -1;
   }

   for (i=0; i<16; i++) {
      indexmap[i] = -1;
   }
   i = 0;
   while (channelIndex[i] != -1 && i < 16) {
      indexmap[channelIndex[i]] = i;
      i++;
   }

   return output;
}



//////////////////////////////
//
// Nidaq::verifyChannelIndex -- makes sure that the channel index is in
//    the range from 0 to 15.
//

void Nidaq::verifyChannelIndex(int anIndex) const {
   if (anIndex < 0 || anIndex > 15) {
      cerr << "Error: invalid channel: " << anIndex << endl;
      exit(1);
   }
}


#endif // NIDAQ
#endif // VISUAL
// md5sum: a2530adfd69cb0c8c70452dd9f854f1f Nidaq.cpp [20050403]
