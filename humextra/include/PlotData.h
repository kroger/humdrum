//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 18 08:09:51 PDT 2002
// Last Modified: Fri Jul  5 20:48:00 PDT 2002
// Filename:      ...sig/maint/code/base/PlotData/PlotData.cpp
// Web Address:   http://sig.sapp.org/include/sigBase/PlotData.cpp
// Documentation: http://sig.sapp.org/doc/classes/PlotData
// Syntax:        C++ 
//
// Description:   Plot data and style options for PlotFigure class.
//

#ifndef _PLOTDATA_H_INCLUDED
#define _PLOTDATA_H_INCLUDED

#include "Array.h"

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif


class PlotFigure;

class PlotData {
   public:

                  PlotData        (void);
                  PlotData        (PlotData& aPlot);

      PlotData&   operator=       (PlotData& aPlot);
      void        read            (istream& input);
      static int  commandEqual    (const char* command, const char* string);
      ostream&    print           (ostream& out);
      int         commentQ        (void);
      int         getXFigDepth    (void);
      ostream&    printXfig       (ostream& out, PlotFigure& figure);
      void        getPlotRange    (double& axmin, double& axmax, double& aymin, 
                                   double& aymax);
      void        setData         (Array<double>& xvals, Array<double>& yvals, 
                                   const char* aComment);
      void        addPoint        (double xvalue, double yvalue);
      void        setComment      (const char* aComment);
      const char* getComment      (void);
      int         getPointsInRange(double minx, double maxx, 
                                   double miny, double maxy);

   private:
      Array<Array<double> > data;
      Array<char>           comment;
      int                   layer;


};



#endif /* _PLOTDATA_H_INCLUDED */

// md5sum: ad83e1f3744e5a259554b99ac7c93c31 PlotData.h [20050403]
