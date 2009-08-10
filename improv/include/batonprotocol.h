//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 21 December 1997
// Last Modified: Sun Feb 15 21:45:16 GMT-0800 1998
// Last Modified: Sun Oct  1 15:11:11 PDT 2000 (updated for firmware "AE")
// Filename:      ...sig/code/control/RadioBaton/batonprotocol.h
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/include/batonprotocol.h
// Syntax:        C++
//
// Description:   Various defines for the Radio Baton communication
//                protocol.
//

#ifndef _BATONPROTOCOL_H_INCLUDED
#define _BATONPROTOCOL_H_INCLUDED


/////////////////////////////////////////////////////////
// A Baton message consists of three parts:            //
//    command byte -- usually 0xa0                     // 
//    message byte -- a general behavior               //
//    data byte    -- a qualifier of the message byte  //
/////////////////////////////////////////////////////////


//////////////////////////////
//
// Baton command bytes
//

#define BAT_MIDI_COMMAND        (0xa0)  /* baton messages start with this */

//
//  0xa5 = highest nibble of buff array being sent (shift 12)
//	message byte = calib array index (from 0 to 15)
//	data byte    = highest nibble of calib data for index
//  0xa6 = next lower nibble of buff array being sent (shift 8)
//	message byte = calib array index (from 0 to 15)
//	data byte    = highest nibble of calib data for index
//  0xa7 = next lower nibble of buff array being sent (shift 4)
//	message byte = calib array index (from 0 to 15)
//	data byte    = highest nibble of calib data for index
//  0xa8 = lowest nibble of buff array being sent (no shift)
//	message byte = calib array index (from 0 to 15)
//	data byte    = highest nibble of calib data for index
//


//////////////////////////////
//
// Baton message bytes
//

// messages from the radio drum to the PC
#define BAT_VERSION             (0)     /* version number */
#define BAT_STICK1_TRIGGER      (1)     /* stick 1 got triggered */
#define BAT_STICK2_TRIGGER      (2)     /* stick 2 got triggered */
#define BAT_BUTTON_FOOT_TRIGGER (3)     /* either button or foot triggered */
#define BAT_POT1_RESPONSE       (4)     /* dial 1 position response */
#define BAT_POT2_RESPONSE       (5)     /* dial 2 position response */
#define BAT_POT3_RESPONSE       (6)     /* dial 3 position response */
#define BAT_POT4_RESPONSE       (7)     /* dial 4 position response */
#define BAT_STICK1_RESPONSE_X   (8)     /* stick 1 x-axis position response */
#define BAT_STICK1_RESPONSE_Y   (9)     /* stick 1 y-axis position response */
#define BAT_STICK1_RESPONSE_Z   (10)    /* stick 1 z-axis position response */
#define BAT_STICK2_RESPONSE_X   (11)    /* stick 2 x-axis position response */
#define BAT_STICK2_RESPONSE_Y   (12)    /* stick 2 y-axis position response */
#define BAT_STICK2_RESPONSE_Z   (13)    /* stick 2 z-axis position response */
#define BAT_SEND                (14)    /* for parameterless commands */
#define BAT_STICK1_TRIG_X       (15)    /* stick 1 x-axis trigger position */
#define BAT_STICK1_TRIG_Y       (16)    /* stick 1 y-axis trigger position */
#define BAT_STICK2_TRIG_X       (17)    /* stick 2 x-axis trigger position */
#define BAT_STICK2_TRIG_Y       (18)    /* stick 2 y-axis trigger position */

// messages from the PC to the radio drum
#define REPORT_POS              (17)    /* toggle for reporting */


//////////////////////////////
//
// Baton data bytes
//

// data for message BAT_BUTTON_FOOT_TRIGGER (3) message:
#define BAT_B14p_TRIGGER        (0)     /* button 14+ got triggered */
#define BAT_B15p_TRIGGER        (1)     /* button 15+ got triggered */
#define BAT_B14m_DOWN_TRIGGER   (2)     /* button 14- down got triggered */
#define BAT_B14m_UP_TRIGGER     (3)     /* button 14- up got triggered */
#define BAT_B15m_DOWN_TRIGGER   (4)     /* button 15- down got triggered */
#define BAT_B15m_UP_TRIGGER     (5)     /* button 15- up got triggered */

// data for REPORT_POS (17) message:
#define POS_REPORT_ON           (127)   /* turn pos. reporting on */
#define POS_REPORT_OFF          (0)     /* turn pos. reporting off */


#endif  /* _BATONPROTOCOL_H_INCLUDED */

// md5sum: b34b828e7c95aeae4951e99e3e894c6a batonprotocol.h [20020518]
