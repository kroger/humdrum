//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 25 13:03:35 PST 1998
// Last Modified: Mon Nov 23 15:22:13 PST 1998
// Filename:      ...sig/maint/code/misc/sigCVI.h
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/include/sigCVI.h
// Syntax:        C++; Visual C++ 6.0 interface to LabWindows CVI libraries
//
// Description:   Include this file as an interface between
//                LabWindows/CVI libraries and Visual C++ compiler.
//                See chapter 3 of LabWindows/CVI Programmer Ref Manual (v4)
//                This file is used for Improv programs which use the 
//                National Instruments AT-MIO-16-E digital acquisition card.
//                This file probably only useful for Windows 95/NT.
//

#ifndef _SIGCVI_H_INCLUDED
#define _SIGCVI_H_INCLUDED

#ifdef VISUAL

// The following define sets up the National Instruments header
// include options:
#define _NI_mswi32_

// include the following libraries in the visual c++ project:
//    cvisupp.lib
//    cvirt.lib
//
// optionally include these libraries:
//    analysis.lib -- analysis library
//    gpib.lib     -- GPIB/GPIB 488.2 Library
//    dataacq.lib  -- data aquisition library
//    easyio.lib   -- Easy I/O for DAQ Library
//    visa.lib     -- VISA Transition Library
//    nivxi.lib    -- VXI Library 
//

// Add the following path to the include search path:
//   cvi\include


// Some useful LabWindows/CVI include files:
//
#include <cvirte.h>       // Primary LabWindows/CVI library interface
// #include <userint.h>   // User Interface for LabWindows/CVI
// #include <dataacq.h>   // Data Acquisition Interface
// #include <easyio.h>    // Easy I/O for Data Acquisition
// #include <utility.h>   // Various utility function definitions


#endif  /* VISUAL */

#endif  /* _SIGCVI_H_INCLUDED */


// md5sum: 067686a3bd6e291cdedcf308016cb71d sigCVI.h [20020518]
