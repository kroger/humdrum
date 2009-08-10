//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 23:25:22 PDT 1998
// Last Modified: Tue Jun 30 23:25:27 PDT 1998
// Filename:      ...sig/include/sigInfo/Enum_muserec.h
// Web Address:   http://sig.sapp.org/include/sigInfo/Enum_muserec.h
// Syntax:        C++ 
// Reference:     Beyond Midi, page 410.
//
// Description:   An enumeration of MuseData record types 
//

#ifndef _ENUM_MUSEREC_H_INCLUDED
#define _ENUM_MUSEREC_H_INCLUDED

#include "Enum_basic.h"

#define E_muserec_note_regular       'A'
   //                                'B' --> use type E_muserec_note_regular
   //                                'C' --> use type E_muserec_note_regular
   //                                'D' --> use type E_muserec_note_regular
   //                                'E' --> use type E_muserec_note_regular
   //                                'F' --> use type E_muserec_note_regular
   //                                'G' --> use type E_muserec_note_regular
#define E_muserec_print_suggestion   'P'
#define E_muserec_sound_directives   'S'
#define E_muserec_note_chord         ' '
#define E_muserec_end                '/'
#define E_muserec_append             'a'
#define E_muserec_backspace          'b'
#define E_muserec_note_cue           'c'
#define E_muserec_figured_harmony    'f'
#define E_muserec_note_grace         'g'
#define E_muserec_rest_invisible     'i'
#define E_muserec_measure            'm'
#define E_muserec_rest               'r'
#define E_muserec_musical_attributes '$'
#define E_muserec_comment_toggle     '&'
#define E_muserec_comment_line       '@'
#define E_muserec_musical_directions '*'

#define E_musrec_header               1000

#define E_musrec_footer               2000


#endif  /* _ENUM_MUSEREC_H_INCLUDED */



// md5sum: e7c651e13bf50a5b4c140ac77eda402a Enum_muserec.h [20050403]
