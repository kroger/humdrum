//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Feb 14 07:42:21 PST 2011
// Last Modified: Mon Feb 14 07:42:29 PST 2011
// Filename:      ...sig/maint/code/sigBase/CheckSum.cpp
// Web Address:   http://sig.sapp.org/src/sigBase/CheckSum.cpp
// Syntax:        C++ 
//
// Description:   Handles calculating checksums in various formats.
// 		  Currently CRC32 (cksum command)
// 		  Currently MD5   (md5sum command)
//

#include "CheckSum.h"
#include <string.h>
#include <assert.h>

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
   

//////////////////////////////
//
// CheckSum::CheckSum -- Constructor
//

CheckSum::CheckSum(void) {
   // do nothing
}


//////////////////////////////
//
// CheckSum::~CheckSum -- Destructor
//

CheckSum::~CheckSum(void) {
   // do nothing
}




///////////////////////////////
//
// CheckSum::crc32 -- returns the same as the command-line cksum program
// (just the first number, not the size in bytes of the input string).
//

unsigned long CheckSum::crc32(const char* buf, int length) {
   static long int const crctab[256] = {
      0x00000000,
      0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
      0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6,
      0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
      0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac,
      0x5bd4b01b, 0x569796c2, 0x52568b75, 0x6a1936c8, 0x6ed82b7f,
      0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a,
      0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
      0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58,
      0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033,
      0xa4ad16ea, 0xa06c0b5d, 0xd4326d90, 0xd0f37027, 0xddb056fe,
      0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
      0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4,
      0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
      0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5,
      0x2ac12072, 0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
      0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca, 0x7897ab07,
      0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c,
      0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1,
      0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
      0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b,
      0xbb60adfc, 0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698,
      0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d,
      0x94ea7b2a, 0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
      0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2, 0xc6bcf05f,
      0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
      0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80,
      0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
      0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a,
      0x58c1663d, 0x558240e4, 0x51435d53, 0x251d3b9e, 0x21dc2629,
      0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c,
      0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
      0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e,
      0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65,
      0xeba91bbc, 0xef68060b, 0xd727bbb6, 0xd3e6a601, 0xdea580d8,
      0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
      0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2,
      0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
      0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74,
      0x857130c3, 0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
      0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c, 0x7b827d21,
      0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a,
      0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e, 0x18197087,
      0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
      0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d,
      0x2056cd3a, 0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce,
      0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb,
      0xdbee767c, 0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
      0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4, 0x89b8fd09,
      0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
      0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf,
      0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
   };

   unsigned long crc = 0;
   for (int i=0; i<length; i++) {
      crc = (crc << 8) ^ crctab[((crc >> 24) ^ *buf++) & 0xFF];
   }
   for (; length; length >>= 8) {
      crc = (crc << 8) ^ crctab[((crc >> 24) ^ length) & 0xFF];
   }
   crc = ~crc & 0xFFFFFFFF;
   return crc;

/*
int main (void) {
   char* mystring = "This is a test.\n";
   int length = strlen(mystring);
	   
   long value = string_cksum(mystring, length);
   printf("Checksum = %u\n", value);

   exit(0);
}
*/

}


///////////////////////////////////////////////////////////////////////////////
//
// MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
//
// Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All rights 
// reserved.
//
// License to copy and use this software is granted provided that it is 
// identified as the "RSA Data Security, Inc. MD5 Message-Digest Algorithm" in 
// all material mentioning or referencing this software or this function. 
//
// License is also granted to make and use derivative works provided that such 
// works are identified as "derived from the RSA Data Security, Inc. MD5 
// Message-Digest Algorithm" in all material mentioning or referencing the 
// derived work. 
// 
// RSA Data Security, Inc. makes no representations concerning either the 
// merchantability of this software or the suitability of this software for 
// any particular purpose. It is provided "as is" without express or implied 
// warranty of any kind. 
// 
// These notices must be retained in any copies of any part of this 
// documentation and/or software. 
//

// #include <cassert>
// #include <cstring>
// using namespace std;

// Constants for MD5Transform routine.
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

// F, G, H and I are basic MD5 functions.  
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

// ROTATE_LEFT rotates x left n bits.  
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.

#define FF(a, b, c, d, x, s, ac) { \
     (a) += F ((b), (c), (d)) + (x) + (unsigned long)(ac); \
     (a) = ROTATE_LEFT ((a), (s)); \
     (a) += (b); \
     }
#define GG(a, b, c, d, x, s, ac) { \
     (a) += G ((b), (c), (d)) + (x) + (unsigned long)(ac); \
     (a) = ROTATE_LEFT ((a), (s)); \
     (a) += (b); \
     }
#define HH(a, b, c, d, x, s, ac) { \
     (a) += H ((b), (c), (d)) + (x) + (unsigned long)(ac); \
     (a) = ROTATE_LEFT ((a), (s)); \
     (a) += (b); \
     }
#define II(a, b, c, d, x, s, ac) { \
     (a) += I ((b), (c), (d)) + (x) + (unsigned long)(ac); \
     (a) = ROTATE_LEFT ((a), (s)); \
     (a) += (b); \
     }



//////////////////////////////
//
// CheckSum::MD5Init -- MD5 initialization. Begins an MD5 operation, writing a 
//            new context.
//

void CheckSum::MD5Init (MD5_CTX *context) {
   context->count[0] = context->count[1] = 0;

   // Load magic initialization constants.
   context->state[0] = 0x67452301;
   context->state[1] = 0xefcdab89;
   context->state[2] = 0x98badcfe;
   context->state[3] = 0x10325476;
}



//////////////////////////////
//
// CheckSum::MD5Update -- MD5 block update operation. Continues an MD5 
//   message-digest operation, processing another message block, 
//   and updating the context.
//

void CheckSum::MD5Update(MD5_CTX *context, unsigned char *input, 
      unsigned int inputLen) {
   unsigned int i, index, partLen;

   // Compute number of bytes mod 64
   index = (unsigned int)((context->count[0] >> 3) & 0x3F);

   // Update number of bits 
   if ((context->count[0] += ((unsigned long)inputLen << 3))
            < ((unsigned long)inputLen << 3)) {
      context->count[1]++;
   }
   context->count[1] += ((unsigned long)inputLen >> 29);

   partLen = 64 - index;

   // Transform as many times as possible. 

   if (inputLen >= partLen) {
      memcpy(&context->buffer[index], input, partLen);
      MD5Transform (context->state, context->buffer);

      for (i=partLen; i+63 < inputLen; i+=64) {
         MD5Transform (context->state, &input[i]);
      }

      index = 0;
   } else {
      i = 0;
   }

   // Buffer remaining input 
   memcpy (&context->buffer[index], &input[i], inputLen-i);
}



//////////////////////////////
//
// CheckSum::MD5Final -- MD5 finalization. Ends an MD5 message-digest 
//        operation, writing the the message digest and zeroizing the context. 
//

void CheckSum::MD5Final(unsigned char digest[16], MD5_CTX *context) {
   unsigned char bits[8];
   unsigned int index, padLen;

   unsigned char PADDING[64] = {
      0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0
   };

   // Save number of bits
   Encode(bits, context->count, 8);

   // Pad out to 56 mod 64. 
   index  = (unsigned int)((context->count[0] >> 3) & 0x3f);
   padLen = (index < 56) ? (56 - index) : (120 - index);
   MD5Update(context, PADDING, padLen);

   // Append length (before padding) 
   MD5Update(context, bits, 8);
   // Store state in digest
   Encode(digest, context->state, 16);

   // Zeroize sensitive information. 
   assert(sizeof(*context) == sizeof(MD5_CTX));
   memset(context, 0, sizeof(*context));
}



//////////////////////////////
//
// CheckSu::MD5Transform -- MD5 basic transformation. Transforms state 
//      based on block.
//

void CheckSum::MD5Transform(unsigned long state[4], unsigned char block[64]) {
   unsigned long a = state[0], b = state[1], c = state[2], d = state[3], x[16];
   Decode (x, block, 64);

   // Round 1
   FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
   FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
   FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
   FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
   FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
   FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
   FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
   FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
   FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
   FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
   FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
   FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
   FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
   FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
   FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
   FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

   // Round 2
   GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
   GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
   GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
   GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
   GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
   GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
   GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
   GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
   GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
   GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
   GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
   GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
   GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
   GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
   GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
   GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

   // Round 3 
   HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
   HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
   HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
   HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
   HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
   HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
   HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
   HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
   HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
   HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
   HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
   HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
   HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
   HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
   HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
   HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

   // Round 4 
   II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
   II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
   II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
   II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
   II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
   II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
   II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
   II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
   II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
   II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
   II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
   II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
   II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
   II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
   II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
   II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

   state[0] += a;
   state[1] += b;
   state[2] += c;
   state[3] += d;

   // Zeroize sensitive information.
   memset(x, 0, sizeof(x));
}



//////////////////////////////
//
// CheckSum::Encode -- Encodes input (unsigned long) into output 
//       (unsigned char).  Assumes len is a multiple of 4.
//

void CheckSum::Encode(unsigned char *output, unsigned long *input, 
      unsigned int len) {
   unsigned int i, j;
   for (i=0, j=0; j<len; i++, j+= 4) {
      output[j]   = (unsigned char)( input[i]        & 0xff);
      output[j+1] = (unsigned char)((input[i] >>  8) & 0xff);
      output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
      output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
   }
}



//////////////////////////////
//
// CheckSum::Decode -- Decodes input (unsigned char) into 
//      output (unsigned long).  Assumes len is a multiple of 4.

void CheckSum::Decode (unsigned long *output, unsigned char *input, 
      unsigned int len) {
   unsigned int i, j;
   for (i=0, j=0; j<len; i++, j+=4) {
      output[i] = ((unsigned long)input[j])           | 
                  (((unsigned long)input[j+1]) << 8)  |
                  (((unsigned long)input[j+2]) << 16) | 
                  (((unsigned long)input[j+3]) << 24);
   }
}


//
// MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
//
///////////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// CheckSum::getMD5Sum -- interface to the previous functions.
//

void CheckSum::getMD5Sum(Array<char>& md5sum, Array<char>& data) {
   MD5_CTX context;
   MD5Init(&context);

   int i;
   int count    = (data.getSize()-1) / 64;
   int leftover = (data.getSize()-1) % 64;

   for (i=0; i<count; i++) {
      MD5Update(&context, (unsigned char*)data.getBase() + i * 64, 64);

   }
   if (leftover > 0) {
      MD5Update(&context, (unsigned char*)data.getBase() + count*64, leftover);
   }

   SSTREAM outvalue;
   unsigned char digest[16] = {0};
   MD5Final(digest, &context);
   for (i=0; i<16; i++) {
      if ((int)digest[i] < 16) {
         outvalue << "0";
      }
      outvalue << hex << (int)digest[i] << dec;
   }
   outvalue << ends;
   md5sum.setSize(strlen(outvalue.CSTRING) + 1);
   strcpy(md5sum.getBase(), outvalue.CSTRING);
}



//////////////////////////////
//
// CheckSum::getMD5Sum -- interface to the previous functions.
//

void CheckSum::getMD5Sum(ostream& out, SSTREAM& data) {
   Array<unsigned char> buffer;
   buffer.setSize(data.str().length()+1);
   buffer[buffer.getSize()-1] = '\0';
   strncpy((char*)buffer.getBase(), data.CSTRING, data.str().length());

   MD5_CTX context;
   MD5Init(&context);

   int i;
   int count    = (buffer.getSize()-1) / 64;
   int leftover = (buffer.getSize()-1) % 64;

   for (i=0; i<count; i++) {
      MD5Update(&context, buffer.getBase() + i * 64, 64);

   }
   if (leftover > 0) {
      MD5Update(&context, buffer.getBase() + count * 64, leftover);
   }

   unsigned char digest[16] = {0};
   MD5Final(digest, &context);
   for (i=0; i<16; i++) {
      if (digest[i] < 16) {
         out << "0";
      }
      out << hex << (int)digest[i] << dec;
   }
}


// md5sum: 5a09b3a7a4a44da97fcbbd1b4f2c3464 CheckSum.cpp [20050403]
