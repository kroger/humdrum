//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 5 January 1998
// Last Modified: Sun Jan 25 18:39:32 GMT-0800 1998
// Filename:      ...sig/code/control/Synthesizer/Synthesizer.h
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/include/Synthesizer.h
// Syntax:        C++
//
// Description:   A class for handling Synthesizer Input and Output
//	  	  especially the input, the MidiOutput class is
//		  sufficient for synthesizer output alone.
//
//

#ifndef _SYNTHESIZER_H_INCLUDED
#define _SYNTHESIZER_H_INCLUDED

#include "MidiIO.h"

#define DEFAULT_CONT_SIZE (10)


class Synthesizer : public MidiIO {
   public:
                   Synthesizer              (void);
                   Synthesizer              (int outputPort, int inputPort);
                  ~Synthesizer              ();

      int          controller               (int controlNumber, int channel = 0, 
                                               int index = 0);
      MidiMessage  extractNote              (void);
      int          getNoteCount             (void) const;        
      MidiMessage& operator[]               (int index);
      void         processIncomingMessages  (void);
      void         zeroControllers          (void);

   protected:

      // state variables
      CircularBuffer<MidiMessage> note;
      CircularBuffer<uchar> Controller[16][128];   // [channel][controller]
   
      void        interpretMessage          (MidiMessage aMessage);

};


#endif  /* _SYNTHESIZER_H_INCLUDED */


// md5sum: db136ccd09e77121c6a1a6bc71e895eb Synthesizer.h [20050403]
