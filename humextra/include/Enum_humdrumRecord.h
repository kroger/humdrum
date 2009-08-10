//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun 13 23:27:29 PDT 1998
// Last Modified: Sat Jun 13 23:27:33 PDT 1998
// Filename:      ...sig/include/sigInfo/Enum_humdrumRecord.h
// Web Address:   http://sig.sapp.org/include/sigInfo/Enum_humdrumRecord.h
// Syntax:        C++ 
//


#ifndef _ENUM_HUMDRUM_RECORD_H_INCLUDED
#define _ENUM_HUMDRUM_RECORD_H_INCLUDED

#include "Enum_basic.h"

#define E_humrec_none                (0x00000)
#define E_humrec_empty               (0x00001)
#define E_humrec_global_comment      (0x10000)
#define E_humrec_bibliography        (0x10001)

#define E_humrec_data                (0x20000)

#define E_humrec_data_comment        (0x20001)
#define E_humrec_local_comment       (0x20001)

#define E_humrec_data_measure        (0x20002)
#define E_humrec_data_kern_measure   (0x20002)

#define E_humrec_data_interpretation (0x20003)
#define E_humrec_interpretation      (0x20003)
#define E_humrec_interp              (0x20003)



#endif  /* _ENUM_HUMDRUM_RECORD_H_INCLUDED */



// md5sum: fb1e1f125396a0c00cbfec38dfb6c829 Enum_humdrumRecord.h [20050403]
