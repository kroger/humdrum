//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun May 13 14:00:43 PDT 2001
// Last Modified: Sun May 13 14:00:40 PDT 2001
// Filename:      ...sig/src/sigInfo/HumdrumFile.cpp
// Web Address:   http://sig.sapp.org/include/museinfo/humdrumfileextras.h
// Syntax:        C++ 
//
// Description:   Functions useful for use with the HumdrumFile class,
//		  but not necessarily using HumdrumFile class.
//


//////////////////////////////////////////////////////////////////////////
//
// Krumhansl-Schmuckler key-finding algorithm functions.
//

int analyzeKeyKS(double* scores, double* distribution, int* pitch, 
      double* durations, int size, int rhythmQ = 1, int binaryQ = 0);

int analyzeKeyKS2(double* scores, double* distribution, int* pitch, 
      double* durations, int size, int rhythmQ, double *majorKey,
      double* minorKey);




// md5sum: 66f8d07ebb1f7c36e603feb044de1c18 humdrumfileextras.h [20050403]
