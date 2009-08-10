//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Aug  5 17:29:53 PDT 2003
// Last Modified: Tue Aug  5 17:29:56 PDT 2003
// Filename:      ...sig/code/control/Tablet/visual/Tablet_visual.cpp
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/include/Tablet_visual.cpp
// Syntax:        C++ 
//
// Description:   An interface for WinTab Tablet input capabilities of
// Windows 95/NT/98/ME/2000/XP.  as defined in wintab.lib.  This class
// is inherited privately by the Tablet class.  Uses the standard WinTab
// library from http://www.pointing.com.  Designed specifically for the
// Wacom Intuos2 Tablet.  Only one tablet allowed at a time.
//


#ifdef VISUAL

#include "Tablet_visual.h"

// initialize static variables
int Tablet_visual::classcount   = 0;
int Tablet_visual::openQ        = 0;
int Tablet_visual::tiltsupportQ = 0;


LOGCONTEXT     Tablet_visual::tabletcontext;      // context for the tablet
AXIS           Tablet_visual::tabletx;                  // x-axis sizes of tablet
AXIS           Tablet_visual::tablety;                  // y-axis sizes of tablet
HCTX           Tablet_visual::tablethandle;
HWND           Tablet_visual::windowhandle;
struct tagAXIS Tablet_visual::tiltinfo[3];    // capabilities of tilt
TabletPen      Tablet_visual::pen1;                // information for first pen
TabletPen      Tablet_visual::pen2;                // information for second pen



//////////////////////////////
//
// Tablet_visual::Tablet_visual --
//

Tablet_visual::Tablet_visual(void) { 
   classcount++;
   if (!is_open()) {
      open();
   }

   pen1trigon     = penEmptyBehavior;
   pen1trigoff    = penEmptyBehavior;
   pen2trigon     = penEmptyBehavior;
   pen2trigoff    = penEmptyBehavior;
   pen1button1on  = penEmptyBehavior;
   pen1button1off = penEmptyBehavior;
   pen1button2on  = penEmptyBehavior;
   pen1button2off = penEmptyBehavior;
   pen2button1on  = penEmptyBehavior;
   pen2button1off = penEmptyBehavior;
   pen2button2on  = penEmptyBehavior;
   pen2button2off = penEmptyBehavior;

}



//////////////////////////////
//
// Tablet_visual::~Tablet --
//

Tablet_visual::~Tablet_visual() { 
   classcount--;
   if (classcount <= 0) {
      close();
   }
}



//////////////////////////////
//
// Tablet_visual::is_open -- returns true if the tablet is current open
//   returns false if it is not open.
//

Tablet_visual::is_open(void) { 
   return openQ;
}



//////////////////////////////
//
// Tablet_visual::getName --
//

const char* Tablet_visual::getName(void) { 
   static char devicename[50] = {0};
   WTInfo(WTI_DEVICES, DVC_NAME, devicename);
   return (const char*)&devicename;
}



//////////////////////////////
//
// Tablet_visual::open -- returns true if the tablet was accessible.
//

int Tablet_visual::open(void) { 
   if (is_open()) {
      // already open, so don't try to reopen.
      return 1;
   }

   // open the tablet if there is one present on the computer:
   if (exist()) {
      // get the default context
      WTInfo(WTI_DEFCONTEXT, 0, &tabletcontext);

      // chek for tilt support information
      tiltsupportQ = WTInfo(WTI_DEVICES, DVC_ORIENTATION, &tiltinfo);

      // set the entire tablet as active
      WTInfo(WTI_DEVICES, DVC_X, &tabletx);
      WTInfo(WTI_DEVICES, DVC_Y, &tablety);
      tabletcontext.lcInOrgX = 0;
      tabletcontext.lcInOrgY = 0;
      tabletcontext.lcInExtX = tabletx.axMax;
      tabletcontext.lcInExtY = tablety.axMax;
      tabletcontext.lcOutOrgX = 0;
      tabletcontext.lcOutOrgY = 0;
      tabletcontext.lcOutExtX = 128;  // set default x-axis range 0-127
      tabletcontext.lcOutExtY = 128;  // set default y-axis range 0-127

      // the next line is important for the packet data to 
      // be in the correct order.
      tabletcontext.lcPktData = PACKETDATA;

      // get the window which will be sending messages about 
      // the tablet.  Using the desktop window since this
      // class is not using any particular window.
      windowhandle = GetDesktopWindow();
      if (windowhandle == 0) {
         // could not open window context, so giving up...
         return 0;
      }

      tablethandle = WTOpen(windowhandle, &tabletcontext, TRUE);

      openQ = 1;
      return 1;
   }

   // no tablet so cannot open it.
   return 0;
}



//////////////////////////////
//
// Tablet_visual::exist -- return true if there is a tablet present on the 
//     computer.  Return false otherwise.
//

int Tablet_visual::exist(void) { 
   return WTInfo(0, 0, NULL);
}



//////////////////////////////
//
// Tablet_visual::close -- closes access to tablet hardware
//

void Tablet_visual::close(void) { 
   if (is_open()) {
      WTClose(tablethandle);
      openQ = 0;
   }
}



//////////////////////////////
//
// Tablet_visual::flush -- remove all waiting information packets.
//      useful to clear if there is a backlog of data waiting
//      to be processed.
//

void Tablet_visual::flush(void) { 
   if (is_open()) {
      WTPacketsGet(tablethandle, 0, NULL);
   }
}



//////////////////////////////
//
// Tablet_visual::processIncomingMessages -- read incoming packet
//    information.  Currently only read one at a time. 
//    Use flush() to clear too old data.
//

void Tablet_visual::processIncomingMessages(void) {
   PACKET packet;
   while (WTPacketsGet(tablethandle, 1, &packet)) {
      // pen 1 is defined as being in normal position
      // pkCursor == 1 (for first pen) or 4 (for secondary pen)
      if ((packet.pkCursor == 1) || (packet.pkCursor == 4)) {
         processPacket(pen1, packet, 1);
      }
      // pen 2 is defined as being in eraser position
      // pkCursor == 2 (for first pen) or 5 (for secondary pen)
      else if ((packet.pkCursor == 2) || (packet.pkCursor == 5)) {
         processPacket(pen2, packet, 2);
      }
   }
}



//////////////////////////////
//
// Tablet_visual::processPacket -- store new data for a pen.
//

void Tablet_visual::processPacket(TabletPen& pen, PACKET& packet, int number) {
   int b0old = pen.b0;
   int b1old = pen.b1;
   int b2old = pen.b2;

   pen.x  = packet.pkX;
   pen.y  = packet.pkY;
   pen.p  = packet.pkNormalPressure;
   pen.w  = packet.pkTangentPressure;
   pen.b0 = packet.pkButtons & 0x01;
   pen.b1 = packet.pkButtons & 0x02;
   pen.b2 = packet.pkButtons & 0x04;


   // callback functions for changes in button states.
   if (number == 1) {
      if (b0old != pen.b0) { 
         if (pen.b0) { pen1trigon();  }
         else        { pen1trigoff(); }
      }
      if (b1old != pen.b1) { 
         if (pen.b1) { pen1button1on();  }
         else        { pen1button1off(); }
      }
      if (b2old != pen.b2) { 
         if (pen.b2) { pen1button2on();  }
         else        { pen1button2off(); }
      }
   } else if (number == 2) {
      if (b0old != pen.b0) { 
         if (pen.b0) { pen2trigon();  }
         else        { pen2trigoff(); }
      }
      if (b1old != pen.b1) { 
         if (pen.b1) { pen2button1on();  }
         else        { pen2button1off(); }
      }
      if (b2old != pen.b2) { 
         if (pen.b2) { pen2button2on();  }
         else        { pen2button2off(); }
      }
   }
}



//////////////////////////////
//
// Tablet_visual::p1x, p1y, p2x, p2y, p1p, p2p --
//

int Tablet_visual::p1x(void) { return pen1.x; }
int Tablet_visual::p1y(void) { return pen1.y; }
int Tablet_visual::p2x(void) { return pen2.x; }
int Tablet_visual::p2y(void) { return pen2.y; }
int Tablet_visual::p1p(void) { return pen1.p; }
int Tablet_visual::p2p(void) { return pen2.p; }
int Tablet_visual::p1w(void) { return pen1.w; }
int Tablet_visual::p2w(void) { return pen2.w; }



//////////////////////////////
//
// penEmptyBehavior -- this function is the default function
//     to which the button behavior functions point to.
// 

void penEmptyBehavior(void) {
   // nothing to do since it is empty
}




#endif  /* VISUAL */


// md5sum: 6835442959ec5a062496fa001624d09b Tablet_visual.cpp [20050403]
