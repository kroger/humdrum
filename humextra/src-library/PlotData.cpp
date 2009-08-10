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
// Description:   Plot data and style options
//

#include "PlotData.h"
#include "PlotFigure.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef OLDCPP
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #ifdef VISUAL
      #include <strstrea.h>
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif


   
//////////////////////////////
//
// PlotData::PlotData --
//

PlotData::PlotData(void) { 
   data.setSize(1000);
   data.setGrowth(1000);
   data.setSize(0);
   comment.setSize(0);
   layer = 500;
}


PlotData::PlotData(PlotData& aPlot) { 
   data    = aPlot.data;
   comment = aPlot.comment;
   layer   = aPlot.layer;
}



//////////////////////////////
//
// PlotData::operator= --
//

PlotData& PlotData::operator=(PlotData& aPlot) {
   if (&aPlot == this) {
      return *this;
   }
   data    = aPlot.data;
   comment = aPlot.comment;
   layer   = aPlot.layer;

   return *this;
}



//////////////////////////////
//
// PlotData::read --
//

void PlotData::read(istream& input) { 
   char *ptr = NULL;
   char buffer[16384] = {0};
   input.getline(buffer, 16000, '\n');
   double datum = 0.0;
   data.setSize(0);
   layer = 500;
   int index = 0;
   double xscale = 1.0;
   double yscale = 1.0;
   while (!input.eof()) {
      if (buffer[0] == '@') {
         // directive
         if (commandEqual("@END", buffer)) {
            break;
         } else if (commandEqual("@LAYER:", buffer)) {
            layer = strtol(&(buffer[7]), NULL, 10);
         } else if (commandEqual("@XSCALE:", buffer)) {
            xscale = strtod(&(buffer[8]), NULL);
         } else if (commandEqual("@YSCALE:", buffer)) {
            yscale = strtod(&(buffer[8]), NULL);
         }
      } else if (buffer[0] == '\0') {
         // do nothing
      } else {
         index = data.getSize();
         data.setSize(index+1);
         data[index].setSize(0);
         // real data point
         ptr = strtok(buffer, " \t,:\n\r");
         while (ptr != NULL) {
            if (isdigit(ptr[0]) || ptr[0] == '-' || 
                  ptr[0] == '+' || ptr[0] == '.') {
               datum = strtod(ptr, NULL);
               data[index].append(datum);
            }
            ptr = strtok(NULL, "\t,:\n\r");
         }
         if (data[index].getSize() == 1) {
            data[index][0] *= yscale;
         } else if (data[index].getSize() == 2) {
            data[index][0] *= xscale;
            data[index][1] *= yscale;
         } 
      }
      input.getline(buffer, 16000, '\n');
   }

}



//////////////////////////////
//
// PlotData::commandEqual --
//

int PlotData::commandEqual(const char* command, const char* string) {
   int i = 0;
   while ((command[i] != '\0') && (string[i] != '\0') && 
         (tolower(command[i]) == tolower(string[i]))) {
      i++;
   }
   if (command[i] == '\0') {
      return 1;
   } else {
      return 0;
   }
}




//////////////////////////////
//
// PlotData::print --
//

ostream& PlotData::print(ostream& out) { 
   int i;
   int j;
   for (i=0; i<data.getSize(); i++) {
      out << i << ":\t";
      for (j=0; j<data[i].getSize(); j++) {
         out << data[i][j];
         if (j < data[i].getSize()-1) {
            out << '\t';
         }
      }
      out << '\n';
   }

   return out;
}



//////////////////////////////
//
// PlotData::commentQ -- return true if there is a comment.
//

int PlotData::commentQ(void) {
   if (comment.getSize() > 0) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// PlotData::getXFigDepth --
//

int PlotData::getXFigDepth(void) {
   if (layer < 0) {
      return 0;
   } else if (layer >= 1000) { 
      return 999;
   } else {
      return layer;
   }
}



//////////////////////////////
//
// PlotData::printXfig -- plot as a ployline in XFIG format
//

ostream& PlotData::printXfig(ostream& out, PlotFigure& figure) { 
   int i;
 
   if (commentQ()) {
      out << "# " << getComment() << "\n";
   }

   out << "2 ";   // object code (2 = POLYLINE)
   out << "1 ";   // subtype (1 = polyline)
   out << "0 ";   // line_style
   out << "1 ";   // line thickness: 1/80 of inch; 1/160 of inch in PostScript
   out << "0 ";   // pen color
   out << "7 ";   // fill color
   out << getXFigDepth() << " "; // depth 
   out << "0 ";   // pen_style 
   out << "-1 ";  // area_fill, -1=no fill
   out << "0.000 "; // style_val (float)
   out << "0 ";   // join_style
   out << "0 ";   // cap_style
   out << "-1 ";  // radius
   out << "0 ";   // forward_arrow, 0=off, 1=on
   out << "0 ";   // backward_arrow, 0=off, 1=on

   double xorigin = figure.getLMarginIn() * 1200;
   double yorigin = figure.getTMarginIn() * 1200 + figure.getHeightIn() * 1200;
   double xdelta  = figure.getWidthIn() * 1200;
   double ydelta  = figure.getHeightIn() * 1200;
   double outx;
   double outy;
   double outx2;
   double outy2;
   double noutx;
   double nouty;
   double slope;
   double b;
   double xmin = figure.getXMin();
   double xmax = figure.getXMax();
   double ymin = figure.getYMin();
   double ymax = figure.getYMax();

   int pcount = 0;   // number of points in plot

   SSTREAM ptemp;   // print points to temporary location

   for (i=0; i<data.getSize(); i++) {
      if (data[0].getSize() == 1) {
         outx = xorigin + (i - xmin) / (xmax - xmin) * xdelta;
         outy = yorigin - (data[i][0] - ymin) / (ymax - ymin) * ydelta;
         if ((outx < xorigin) || (outx > xorigin + xdelta)) {
            continue;
         } else if ((outy < yorigin - xdelta) || (outy > yorigin)) {
            continue;
         }
      } else {
         outx = xorigin + (data[i][0] - xmin) / (xmax - xmin) * xdelta;
         outy = yorigin - (data[i][1] - ymin) / (ymax - ymin) * ydelta;
         if (outx < xorigin) {  // point is too small

            if (i<data.getSize() && data[i+1][0] > xmin) {
               outx2 = xorigin + (data[i+1][0] - xmin) / (xmax - xmin) * xdelta;
               outy2 = yorigin - (data[i+1][1] - ymin) / (ymax - ymin) * ydelta;
               slope = (outy - outy2)/(outx - outx2);
               b     = outy - slope * outx;
               noutx = xorigin;
               nouty = slope * noutx + b;
               pcount++;
               pcount++;
               ptemp << "\t" << (int)outx2 << "\t" << (int)outy2 << "\n";
               ptemp << "\t" << (int)noutx << "\t" << (int)nouty << "\n";
               continue;
            }
            continue;
         } else if (outx > xorigin + xdelta) { // point is too large

            if (i>0 && data[i-1][0] < xmax) {
               outx2 = xorigin + (data[i-1][0] - xmin) / (xmax - xmin) * xdelta;
               outy2 = yorigin - (data[i-1][1] - ymin) / (ymax - ymin) * ydelta;
               slope = (outy - outy2)/(outx - outx2);
               b     = outy - slope * outx;
               noutx = xorigin + xdelta;
               nouty = slope * noutx + b;
               pcount++;
               ptemp << "\t" << (int)noutx << "\t" << (int)nouty << "\n";
               continue;
            }

            continue;
         } else if ((outy < yorigin - xdelta) || (outy > yorigin)) {
            continue;
         }
      }
      pcount++;
      ptemp << "\t" << (int)outx << "\t" << (int)outy << "\n";
      continue;
   }
  
   
   // print number of points to follow
   // out << data.getSize(); // npoints = number of points in line
   out << pcount; // npoints = number of points in line
   out << "\n";

   ptemp << ends;
   out << ptemp.CSTRING;

   return out;
}



//////////////////////////////
//
// PlotData::getPointsInRange -- count the number of points inside the
//     range which will be plotted in that range.
//     Assumes a linear sequence of points on the x-axis, and 
//     assumes that the y-axis points do not go outside of the y range.
//

int PlotData::getPointsInRange(double minx, double maxx, 
      double miny, double maxy) {
   int outlow  = 0;
   int outhigh = 0;

   int pointcount = 0;
  
   int i;
   for (i=0; i<data.getSize(); i++) {
      if (data[i].getSize() == 1) {
         if (i < minx) {
            outlow  = 1;
         } else if (i > maxx) {
            outhigh = 1;
         } else {
            pointcount++;
         }
      } else {
         if (data[i][0] < minx) {
            outlow  = 1;
         } else if (data[i][0] > maxx) {
            outhigh = 1;
         } else {
            pointcount++;
         }
      }
   }
   if (outlow) {
      pointcount++;
   } else if (outhigh) {
      pointcount++;
   }

   return pointcount;
}



//////////////////////////////
//
// PlotData::getPlotRange --
//

void PlotData::getPlotRange(double& axmin, double& axmax, double& aymin, 
      double& aymax) {
   if (data.getSize() == 0) {
      axmin = 0;
      axmax = 1;
      aymin = 0;
      aymax = 1;
      return;
   }
   double txmin = 0.0;
   double txmax = 1.0;
   double tymin = 0.0;
   double tymax = 1.0;
   int init = 0;

   int i;
   if (data[0].getSize() == 1) {
      txmin = 0;
      txmax = data.getSize();
      for (i=0; i<data.getSize(); i++) {
         if (init == 0) {
            tymin = data[i][0];
            tymax = data[i][0];
            init = 1;
         } else {
            if (data[i][0] < tymin) tymin = data[i][0];
            if (data[i][0] > tymax) tymax = data[i][0];
         }
      }
   } else {
      for (i=0; i<data.getSize(); i++) {
         if (init == 0) {
            txmin = data[i][0];
            txmax = data[i][0];
            tymin = data[i][1];
            tymax = data[i][1];
            init = 1;
         } else {
            if (data[i][0] < txmin) txmin = data[i][0];
            if (data[i][0] > txmax) txmax = data[i][0];
            if (data[i][1] < tymin) tymin = data[i][1];
            if (data[i][1] > tymax) tymax = data[i][1];
         }
      }
   }

   axmin = txmin;
   axmax = txmax;
   aymin = tymin;
   aymax = tymax;
}



//////////////////////////////
//
// PlotData::addPoint --
//

void PlotData::addPoint(double xvalue, double yvalue) {
   Array<double> xy;
   xy.setSize(2);
   xy[0] = xvalue;
   xy[1] = yvalue;
   data.append(xy);
}



//////////////////////////////
//
// PlotData::setData -- set the points for the plot.
//

void PlotData::setData(Array<double>& xvals, Array<double>& yvals,
      const char* aComment) {
   data.setSize(xvals.getSize());
   for (int i=0; i<xvals.getSize(); i++) {
      data[i].setSize(2);
      data[i][0] = xvals[i];
      data[i][1] = yvals[i];
   }

   setComment(aComment);
}



//////////////////////////////
//
// PlotData::setComment --
//

void PlotData::setComment(const char* aComment) {
   if (aComment != NULL) {
      int csize = strlen(aComment);
      comment.setSize(csize+1);
      strcpy(comment.getBase(), aComment);
   } else {
      comment.setSize(0);
   }
}



//////////////////////////////
//
// PlotData::getComment --
//

const char* PlotData::getComment(void) {
   if (comment.getSize() == 0) {
      return "";
   } else {
      return comment.getBase();
   }
}



// md5sum: 20d7be4128134ce3214ad4d58bdc82bc PlotData.cpp [20050403]
