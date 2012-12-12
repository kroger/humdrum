//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 23:25:22 PDT 1998
// Last Modified: Mon Jun  7 14:27:50 PDT 2010 (added header and empty enums)
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

#define E_muserec_note_regular       'N'
   //                                'A' --> use type E_muserec_note_regular
   //                                'B' --> use type E_muserec_note_regular
   //                                'C' --> use type E_muserec_note_regular
   //                                'D' --> use type E_muserec_note_regular
   //                                'E' --> use type E_muserec_note_regular
   //                                'F' --> use type E_muserec_note_regular
   //                                'G' --> use type E_muserec_note_regular
#define E_muserec_note_chord         'C'
#define E_muserec_note_cue           'c'
#define E_muserec_note_grace         'g'
#define E_muserec_note_grace_chord   'G'
#define E_muserec_print_suggestion   'P'
#define E_muserec_sound_directives   'S'
#define E_muserec_end                '/'
#define E_muserec_endtext            'T'
#define E_muserec_append             'a'
#define E_muserec_backspace          'b'
#define E_muserec_back               'b'
#define E_muserec_backward           'b'
#define E_muserec_figured_harmony    'f'
#define E_muserec_rest_invisible     'i'
#define E_muserec_forward            'i'
#define E_muserec_measure            'm'
#define E_muserec_rest               'r'
#define E_muserec_musical_attributes '$'
#define E_muserec_comment_toggle     '&'
#define E_muserec_comment_line       '@'
#define E_muserec_musical_directions '*'
#define E_muserec_header_1           '1'  // reserved for copyright notice
#define E_muserec_header_2           '2'  // reserved for identification
#define E_muserec_header_3           '3'  // reserved
#define E_muserec_header_4           '4'  // <date> <name of encoder>
#define E_muserec_header_5           '5'  // WK#:<work number> MV#:<mvmt num>
#define E_muserec_header_6           '6'  // <source>
#define E_muserec_header_7           '7'  // <work title>
#define E_muserec_header_8           '8'  // <movement title>
#define E_muserec_header_9           '9'  // <name of part>
#define E_muserec_header_part_name   '9'  // <name of part>
#define E_muserec_header_10          '0'  // misc designations
#define E_muserec_header_11          'A'  // group memberships
#define E_muserec_group_memberships  'A'  // group memberships
// multiple musered_head_12 lines can occur:
#define E_muserec_header_12          'B'  // <name1>: part <x> of <num in group>
#define E_muserec_unknown            'U'  // unknown record type
#define E_muserec_empty              'E'  // nothing on line and not header
                                          // or multi-line comment
#define E_muserec_deleted            'D'  // deleted line
					 
// non-standard record types for MuseDataSet
#define E_muserec_filemarker         '+'
#define E_muserec_filename           'F'


#define E_musrec_header               1000

#define E_musrec_footer               2000


#endif  /* _ENUM_MUSEREC_H_INCLUDED */



// md5sum: e7c651e13bf50a5b4c140ac77eda402a Enum_muserec.h [20050403]
