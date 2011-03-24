//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Feb 14 07:40:03 PST 2011
// Last Modified: Mon Feb 14 07:40:09 PST 2011
// Filename:      ...sig/maint/code/base/CheckSum/CheckSum.h
// Web Address:   http://sig.sapp.org/include/sigBase/CheckSum.h
// Syntax:        C++ 
//
// Description:   Handles calculating checksums in various formats.
// 		  Currently CRC32 (cksum command)
// 		  Currently MD5   (md5sum command)
//

#ifndef _CHECKSUM_H_INCLUDED
#define _CHECKSUM_H_INCLUDED

#ifndef OLDCPP
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #ifdef VISUAL
      #include <strstrea.h>     /* for Windows 95 */
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif
   

#include "Array.h"

struct MD5_CTX {              // MD5 context
   unsigned long state[4];    // state (ABCD) 
   unsigned long count[2];    // number of bits, modulo 2^64 (lsb first) 
   unsigned char buffer[64];  // input buffer 
};

class CheckSum {
   public:
                    CheckSum             (void);
                   ~CheckSum             ();

      // equivalent to the checksum output by "cksum" command:
      static unsigned long crc32     (const char* buf, int length);

      // equivalent to the md5sum output by "md5sum" command:
      static void          getMD5Sum (Array<char>& md5sum, Array<char>& data);
      static void          getMD5Sum (ostream& out, SSTREAM& data);

   protected:

      // md5sum calculation functions
      static void MD5Init      (MD5_CTX *context);
      static void MD5Update    (MD5_CTX *context, unsigned char *input, 
                                unsigned int inputLen);
      static void MD5Final     (unsigned char digest[16], MD5_CTX *context);
      static void MD5Transform (unsigned long state[4], 
                                unsigned char block[64]);
      static void Encode       (unsigned char *output, unsigned long *input, 
                                unsigned int len);
      static void Decode       (unsigned long *output, unsigned char *input, 
                                unsigned int len);

};


#endif  /* _CHECKSUM_H_INCLUDED */



// md5sum: c59d297a8081cb48f61b534484819f48 CheckSum.h [20110214]
