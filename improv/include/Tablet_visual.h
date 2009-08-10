//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Aug  5 17:29:53 PDT 2003
// Last Modified: Tue Aug  5 17:29:56 PDT 2003
// Filename:      ...sig/code/control/Tablet/visual/Tablet_visual.h
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/include/Tablet_visual.h
// Syntax:        C++ 
//
// Description:   An interface for WinTab Tablet input capabilities of
// Windows 95/NT/98/ME/2000/XP.  as defined in wintab.lib.  This class
// is inherited privately by the Tablet class.  Uses the standard WinTab
// library from http://www.pointing.com.  Designed specifically for the
// Wacom Intuos2 Tablet.  Only one tablet allowed at a time.
//

#ifndef _TABLET_VISUAL_H_INCLUDED
#define _TABLET_VISUAL_H_INCLUDED

#ifdef VISUAL

#include <windows.h>
#include <wintab.h> /* from the WinTab 1.2.6 library */
// packetdata must be defined before including pktdef.h.  This define
// determines the packet data messages which can be received by the
// program.
#define PACKETDATA (PK_CONTEXT | PK_CURSOR | PK_BUTTONS | PK_X | \
                    PK_Y | PK_TANGENT_PRESSURE | PK_NORMAL_PRESSURE | \
                    PK_ORIENTATION)
// don't know what the next define is for, but it makes things work.
#define PACKETMODE (PK_BUTTONS)
#include <pktdef.h>   /* from the WinTab 1.2.6 library */


class TabletPen {
   public:
      int x;  // x-axis position
      int y;  // y-axis position
      int p;  // pressure
      int w;  // wheel (for airbrush)
      int b0; // boolean for if pen is touching tablet.
      int b1; // boolean for if button 1 is on/off
      int b2; // boolean for if button 2 is on/off (none on airbrush);
      // int serial = serial number for pen
};
   

class Tablet_visual {
   public:
                     Tablet_visual          (void);
                     Tablet_visual          (int aPort, int autoOpen = 1);
                    ~Tablet_visual          ();

        void         close                  (void);
        int          exist                  (void);
        void         flush                  (void);
        const char*  getName                (void);
        int          is_open                (void);
        int          open                   (void);
        void         processIncomingMessages(void);

        int          p1x                    (void);     // pen 1 x position
        int          p1y                    (void);     // pen 1 y position
        int          p2x                    (void);     // pen 2 x position
        int          p2y                    (void);     // pen 2 y position
        int          p1p                    (void);     // pen 1 pressure
        int          p2p                    (void);     // pen 2 pressure
        int          p1w                    (void);     // pen 1 wheel
        int          p2w                    (void);     // pen 2 wheel

         
      // button behavior functions: These functions are to be defined by the 
      // user, or creator, or a tabletImprov or similar program.
      // These functions are called by a radio baton object when
      // a certain event such as a trigger or position update has occurred.

      void (*pen1trigon)(void);
      void (*pen1trigoff)(void);
      void (*pen2trigon)(void);
      void (*pen2trigoff)(void);
      void (*pen1button1on)(void);
      void (*pen1button1off)(void);
      void (*pen1button2on)(void);
      void (*pen1button2off)(void);
      void (*pen2button1on)(void);
      void (*pen2button1off)(void);
      void (*pen2button2on)(void);
      void (*pen2button2off)(void);


   protected:
      static int classcount;                // number of instances
      static LOGCONTEXT tabletcontext;      // context for the tablet
      static AXIS tabletx;                  // x-axis sizes of tablet
      static AXIS tablety;                  // y-axis sizes of tablet
      static HCTX tablethandle;
      static HWND windowhandle;
      static struct tagAXIS tiltinfo[3];    // capabilities of tilt
      static int tiltsupportQ;
      static int openQ;

      // pen state information:
      static TabletPen pen1;                  // information for first pen
      static TabletPen pen2;                  // information for second pen

   private:
      void processPacket(TabletPen& pen, PACKET& packet, int number);

};


void penEmptyBehavior(void);


  
#endif  /* VISUAL */

#endif  /* _TABLET_VISUAL_H_INCLUDED */


// md5sum: a34dd466ad07fb740ee9613de0aa0a53 Tablet_visual.h [20030102]
