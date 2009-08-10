//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 18 08:09:51 PDT 2002
// Last Modified: Fri Jul  5 21:48:42 PDT 2002
// Filename:      ...sig/maint/code/base/PlotFigure/PlotFigure.h
// Web Address:   http://sig.sapp.org/include/sigBase/PlotFigure.h
// Documentation: http://sig.sapp.org/doc/classes/PlotFigure
// Syntax:        C++ 
//
// Description:   Plot data and style options
//

#ifndef _PLOTFIGURE_H_INCLUDED
#define _PLOTFIGURE_H_INCLUDED

#include "Array.h"
#include "PlotData.h"

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif

#define PLOTFIGURE_XFIG     1

#define PSTYLE_CIRCLE       0
#define PSTYLE_OPENCIRCLE   1
#define PSTYLE_BOX          2
#define PSTYLE_OPENBOX      3

class PlotFigure {
   public:
                  PlotFigure      (void);
      void        read            (const char* filename);
      double      readDataInches  (const char* inputdata);
      ostream&    print           (ostream& out);
      ostream&    printXfig       (ostream& out);
      int         addPlot         (PlotData& aPlot);
      static int  commandEqual    (const char* command, const char* string);

      void        allocatePoints  (int aSize);

      double      getWidthIn      (void);
      double      getWidthCm      (void);
      double      getWidthMm      (void);
      void        setWidthIn      (double aWidth);
      void        setWidthCm      (double aWidth);
      void        setWidthMm      (double aWidth);

      double      getHeightIn     (void);
      double      getHeightCm     (void);
      double      getHeightMm     (void);
      void        setHeightIn     (double aWidth);
      void        setHeightCm     (double aWidth);
      void        setHeightMm     (double aWidth);

      double      getLMarginIn    (void);
      double      getLMarginCm    (void);
      double      getLMarginMm    (void);
      void        setLMarginIn    (double aWidth);
      void        setLMarginCm    (double aWidth);
      void        setLMarginMm    (double aWidth);

      double      getTMarginIn    (void);
      double      getTMarginCm    (void);
      double      getTMarginMm    (void);
      void        setTMarginIn    (double aWidth);
      void        setTMarginCm    (double aWidth);
      void        setTMarginMm    (double aWidth);

      void        setXMin         (double aValue);
      void        setXMax         (double aValue);
      void        setYMin         (double aValue);
      void        setYMax         (double aValue);

      double      getXMin         (void);
      double      getXMax         (void);
      double      getYMin         (void);
      double      getYMax         (void);

      void        setXRangeAutoOn (void);
      void        setYRangeAutoOn (void);
      void        setXRangeAutoOff(void);
      void        setYRangeAutoOff(void);
    
      void        xTicksOn        (void);
      void        yTicksOn        (void);
      void        xTicksOff       (void);
      void        yTicksOff       (void);

      void        setXTicksDelta  (double aDelta);
      void        setYTicksDelta  (double aDelta);


      const char* getXLabel       (void);
      const char* getYLabel       (void);
      const char* getTitle        (void);

      void        setXLabel       (const char* string);
      void        setYLabel       (const char* string);
      void        setTitle        (const char* string);

      void        addVLine        (double position, const char* label);
      void        addHLine        (double position, const char* label);
      void        addText         (const char* textstring, double xpos, 
                                   double ypos, double aSize, double angle, 
                                   int orientation);
   
      // point related functions
      void     pointAllocation (int aSize);
      void     addPoint        (double x, double y, double radius, int type=0);

   private:
      Array<PlotData> plotdata; // set of plots for data
      double width;             // width  of plot on page (inches);
      double height;            // height of plot on page (inches);
      double lmargin;           // left margin of plot on page (inches);
      double tmargin;           // top margin of plot on page (inches);

      char *xlabel;             // x-axis label
      char *ylabel;             // y-axis label
      char *figuretitle;        // title of figure

      double xrangeauto;        // boolean: should x scale be auto?
      double yrangeauto;        // boolean: should y scale be auto?

      double xmin;              // minimum x range value
      double xmax;              // maximum x range value
      double ymin;              // minimum y range value
      double ymax;              // maximum y range value

      int    outputType;        // type of data to print()

      Array<double> vline;      // vertical marker lines
      Array<double> hline;      // horizontal marker lines

      // point data
      Array<double> xpoint;     // x-axis point locations
      Array<double> ypoint;     // y-axis point locations
      Array<int>    pstyle;     // point style, see PSTYLE_ defines
      Array<double> pradius;    // point radius

      // text positioning data
      Array< Array<char> > text;// text to position on figure
      Array<double> textsize;   // text font size (in points)
      Array<double> textx;      // x position of text
      Array<double> texty;      // y position of text
      Array<double> textang;    // angle of text
      Array<int>    textjustify;// text justification

      // plot tick information
      int    xticksQ;           // boolean for display of x-axis ticks
      int    yticksQ;           // boolean for display of y-axis ticks
      int    xticksfull;        // boolean for displaying complete lines
      int    yticksfull;        // boolean for displaying complete lines
      double xticksdelta;       // spacing between major x-axis ticks
      double yticksdelta;       // spacing between major y-axis ticks
      double xticksoffset;      // starting point for major ticks on x-axis
      double yticksoffset;      // starting point for major ticks on y-axis
      int    xminorticks;       // number of minor ticks between major ticks
      int    yminorticks;       // number of minor ticks between major ticks

      void   setAutoRange       (void);
      void   setAutoRangeX      (void);
      void   setAutoRangeY      (void);
      void   readPoints         (istream& input);
      void   printXFigTicks     (void);
      void   processTextCommand (const char* buffer);
};



#endif  /* _PLOTFIGURE_H_INCLUDED */



// md5sum: 140aa25957ec9cbfb5d56bc92f1d9564 PlotFigure.h [20050403]
