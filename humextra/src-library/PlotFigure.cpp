//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 18 08:09:51 PDT 2002
// Last Modified: Fri Jul  5 21:48:42 PDT 2002
// Filename:      ...sig/maint/code/base/PlotFigure/PlotFigure.cpp
// Web Address:   http://sig.sapp.org/include/sigBase/PlotFigure.cpp
// Documentation: http://sig.sapp.org/doc/classes/PlotFigure
// Syntax:        C++ 
//
// Description:   Plot data and style options
//

#include "PlotFigure.h"

#include <string.h>
#include <ctype.h>
#include <math.h>

#ifndef OLDCPP
   #include <fstream>
   using namespace std;
#else
   #include <fstream.h>
#endif



//////////////////////////////
//
// PlotFigure::PlotFigure --
//

PlotFigure::PlotFigure(void) { 
   plotdata.setSize(0);
   width = 6.5;       // width  of plot on page (inches);
   height = 4.0;      // height of plot on page (inches);
   lmargin = 1.0;     // left margin of plot on page (inches);
   tmargin = 2.0;     // top margin of plot on page (inches);

   xrangeauto = 1;    // boolean: should x scale be auto?
   yrangeauto = 1;    // boolean: should y scale be auto?

   xmin = 0;
   xmax = 1;
   ymin = 0;
   ymax = 1;

   outputType = PLOTFIGURE_XFIG;

   xlabel = NULL;
   ylabel = NULL;
   figuretitle = NULL;

   vline.setSize(0);
   hline.setSize(0);

   // point data
   xpoint.setSize(0);
   ypoint.setSize(0);
   pstyle.setSize(0);
   pradius.setSize(0);

   // plot tick information
   xticksQ      = 0;
   yticksQ      = 0;
   xticksdelta  = 10;
   yticksdelta  = 10;
   xticksoffset = 0;
   yticksoffset = 0;
   xminorticks  = 0;
   yminorticks  = 0;
   xticksfull   = 0;
   yticksfull   = 0;

   // text information
   text.setSize(100);
   textsize.setSize(100);
   textx.setSize(100);
   texty.setSize(100);
   textang.setSize(100);
   textjustify.setSize(100);

   text.setSize(0);
   textsize.setSize(0);
   textx.setSize(0);
   texty.setSize(0);
   textang.setSize(0);
   textjustify.setSize(0);
}



//////////////////////////////
//
// PlotFigure::read --
//

void PlotFigure::read(const char* filename) { 
   fstream infile;
 
   #ifndef OLDCPP
      infile.open(filename, ios::in);
   #else
      infile.open(filename, ios::in | ios::nocreate);
   #endif

   if (!infile.is_open()) {
      cout << "Error: cannot open file: " << filename << endl;
      exit(1);
   }

   char buffer[16300] = {0};
   
   int index;
   infile.getline(buffer, 16000, '\n');
   while (!infile.eof()) {
      if (commandEqual("@WIDTH:", buffer)) {
         setWidthIn(readDataInches(&(buffer[7])));
      } else if (commandEqual("@HEIGHT:", buffer)) {
         setHeightIn(readDataInches(&(buffer[8])));
      } else if (commandEqual("@LMARGIN:", buffer)) {
         setLMarginIn(readDataInches(&(buffer[9])));
      } else if (commandEqual("@XLABEL:", buffer)) {
         setXLabel(&(buffer[8]));
      } else if (commandEqual("@YLABEL:", buffer)) {
         setYLabel(&(buffer[8]));
      } else if (commandEqual("@TITLE:", buffer)) {
         setTitle(&(buffer[7]));
      } else if (commandEqual("@TEXT:", buffer)) {
         processTextCommand(buffer);
      } else if (commandEqual("@TMARGIN:", buffer)) {
         setTMarginIn(readDataInches(&(buffer[9])));
      } else if (commandEqual("@VMARKER:", buffer)) {
         char label[1] = {0};
         double position = strtod(&(buffer[9]), (char**)&label);
         addVLine(position, label);
      } else if (commandEqual("@HMARKER:", buffer)) {
         char label[1] = {0};
         double position = strtod(&(buffer[9]), (char**)&label);
         addHLine(position, label);
      } else if (commandEqual("@START:", buffer) ||
            commandEqual("@BEGIN:", buffer)) {
         if (strstr(buffer, "PLOTDATA") != NULL) {
            index = plotdata.getSize();
            plotdata.setSize(index+1);
            plotdata[index].read(infile);
         } else if (strstr(buffer, "POINTS") != NULL) {
            readPoints(infile);
         }
      } else if (commandEqual("@XRANGE:", buffer)) {
         char* cptr = NULL;
         xmin = strtod(&(buffer[8]), &cptr);
         xmax = strtod(cptr, NULL);
         setXRangeAutoOff();
      } else if (commandEqual("@YRANGE:", buffer)) {
         char* cptr = NULL;
         ymin = strtod(&(buffer[8]), &cptr);
         ymax = strtod(cptr, NULL);
         setYRangeAutoOff();
      } else if (commandEqual("@XTICKS:", buffer)) {
         xTicksOn();
         char* cptr = NULL;
         xticksdelta  = strtod(&(buffer[8]), &cptr);
         xticksoffset = strtod(cptr, NULL);
      } else if (commandEqual("@YTICKS:", buffer)) {
         yTicksOn();
         char* cptr = NULL;
         yticksdelta  = strtod(&(buffer[8]), &cptr);
         yticksoffset = strtod(cptr, NULL);
      } else if (commandEqual("@XSUBTICKS:", buffer)) {
         xminorticks = strtol(&(buffer[11]), NULL, 10);
      } else if (commandEqual("@YSUBTICKS:", buffer)) {
         yminorticks = strtol(&(buffer[11]), NULL, 10);
      } else {
         // unknown line
      }
      infile.getline(buffer, 16000, '\n');
   }
}



//////////////////////////////
//
// PlotFigure::processTextCommand -- 
//

void PlotFigure::processTextCommand(const char* buffer) {
   char tempbuffer[1024] = {0};
   strncpy(tempbuffer, buffer, 1000);
   char* ptr = strtok(tempbuffer, " \n\t");
   // ignore first token

   ptr = strtok(NULL, " \n\t");
   if (ptr == NULL) {
      cout << "LEAVING A" << endl;
      return;
   }
   double xpos = atof(ptr);

   ptr = strtok(NULL, " \n\t");
   if (ptr == NULL) {
      cout << "LEAVING b" << endl;
      return;
   }
   double ypos = atof(ptr);

   ptr = strtok(NULL, " \n\t");
   if (ptr == NULL) {
      cout << "LEAVING c" << endl;
      return;
   }
   double tsize = atof(ptr);

   ptr = strtok(NULL, " \n\t");
   if (ptr == NULL) {
      cout << "LEAVING d" << endl;
      return;
   }
   int orientation = atoi(ptr);

   char textbuffer[1024] = {0};
   ptr = strtok(NULL, " \n\t");
   if (ptr == NULL) {
      cout << "LEAVING e" << endl;
      return;
   }
   while (ptr != NULL) {
      strcat(textbuffer, ptr);
      ptr = strtok(NULL, " \n\t");
      if (ptr != NULL) {
         strcat(textbuffer, " ");
      }
   }

   addText(textbuffer, xpos, ypos, tsize, 0, orientation);

}





//////////////////////////////
//
// PlotFigure::setXRangeAutoOff -- don't do automatic x-axis range detection
//

void PlotFigure::setXRangeAutoOff(void) {
   xrangeauto = 0;
}



//////////////////////////////
//
// PlotFigure::setXRangeAutoOn -- do automatic x-axis range detection
//

void PlotFigure::setXRangeAutoOn(void) {
   xrangeauto = 1;
}



//////////////////////////////
//
// PlotFigure::setYRangeAutoOn -- do automatic y-axis range detection
//

void PlotFigure::setYRangeAutoOn(void) {
   yrangeauto = 1;
}



//////////////////////////////
//
// PlotFigure::setYRangeAutoOff -- don't do automatic y-axis range detection
//

void PlotFigure::setYRangeAutoOff(void) {
   yrangeauto = 0;
}



//////////////////////////////
//
// PlotFigure::setXTicksDelta -- the distance between ticks on x-axis
//

void PlotFigure::setXTicksDelta(double aDelta) { 
   if (aDelta > 0.0) {
      xticksdelta = aDelta;
   }
}



//////////////////////////////
//
// PlotFigure::setYTicksDelta -- the distance between ticks on y-axis
//

void PlotFigure::setYTicksDelta(double aDelta) { 
   if (aDelta > 0.0) {
      yticksdelta = aDelta;
   }
}



//////////////////////////////
//
// PlotFigure::xTicksOn -- Turn on x tick display
//

void PlotFigure::xTicksOn(void) {
   xticksQ = 1;
}



//////////////////////////////
//
// PlotFigure::yTicksOn -- Turn on y tick display
//

void PlotFigure::yTicksOn(void) {
   yticksQ = 1;
}



//////////////////////////////
//
// PlotFigure::xTicksOff -- Turn off x tick display
//

void PlotFigure::xTicksOff(void) {
   xticksQ = 0;
}



//////////////////////////////
//
// PlotFigure::yTicksOff -- Turn off y tick display
//

void PlotFigure::yTicksOff(void) {
   yticksQ = 0;
}



//////////////////////////////
//
// PlotFigure::allocatePoints --
//

void PlotFigure::allocatePoints(int aSize) {
   int oldSize = xpoint.getSize();

   xpoint.setSize(aSize);     // x-axis point locations
   ypoint.setSize(aSize);     // y-axis point locations
   pstyle.setSize(aSize);     // point style, see PSTYLE_ defines
   pradius.setSize(aSize);    // point radius

   xpoint.setSize(oldSize);   // x-axis point locations
   ypoint.setSize(oldSize);   // y-axis point locations
   pstyle.setSize(oldSize);   // point style, see PSTYLE_ defines
   pradius.setSize(oldSize);  // point radius
}



//////////////////////////////
//
// PlotFigure::readPoints --
//

void PlotFigure::readPoints(istream& input) {
   char *ptr = NULL;
   char buffer[16384] = {0};
   input.getline(buffer, 16000, '\n');
   int style = 0;
   double radius = 0.1;
   while (!input.eof()) {
      if (buffer[0] == '@') {
         if (commandEqual("@END", buffer)) {
            // loop exit directive
            break;
         } else if (commandEqual("@RADIUS:", buffer)) {
            // read radius of points
            radius = strtod(&(buffer[8]), NULL);
         } else if (commandEqual("@STYLE:", buffer)) {
            if (strstr(&(buffer[6]), "opencircle") != NULL) {
               style = PSTYLE_OPENCIRCLE;
            } else if (strstr(&(buffer[6]), "circle") != NULL) {
               style = PSTYLE_CIRCLE;
            } else if (strstr(&(buffer[6]), "openbox") != NULL) {
               style = PSTYLE_OPENBOX;
            } else if (strstr(&(buffer[6]), "box") != NULL) {
               style = PSTYLE_BOX;
            } else {
               // unknown style, set to circle
               style = PSTYLE_CIRCLE;
            }
         }
      } else if (buffer[0] == '\0') {
         // do nothing
      } else {
         double xpos = 0;
         double ypos = 0;
         // real data point
         ptr = strtok(buffer, " \t,\n\r");
         if (isdigit(ptr[0]) || ptr[0] == '-' || 
               ptr[0] == '+' || ptr[0] == '.') {
            xpos = strtod(ptr, NULL);
         }
         ptr = strtok(NULL, "\t,\n\r");
         if (isdigit(ptr[0]) || ptr[0] == '-' || 
               ptr[0] == '+' || ptr[0] == '.') {
            ypos = strtod(ptr, NULL);
         }
         addPoint(xpos, ypos, radius, style);
      }
      input.getline(buffer, 16000, '\n');
   }
}



//////////////////////////////
//
// PlotFigure::readDataInches -- read input string number and units, setting
//    the output units to inches.
//

double PlotFigure::readDataInches(const char* inputdata) {
   char* endp = NULL;
   double number = strtod(inputdata, &endp);
   if (strstr(endp, "in")) {
      // do nothing
   } else if (strstr(endp, "cm")) {
      number /= 2.54;
   } else if (strstr(endp, "mm")) {
      number /= 25.4;
   }

   return number;
}



//////////////////////////////
//
// PlotFigure::print --
//

ostream& PlotFigure::print(ostream& out) { 
   switch (outputType) {
      case PLOTFIGURE_XFIG:
      default:
         return printXfig(out);
   }
   return out;
}


//////////////////////////////
//
// PlotFigure::printXfig -- print plot in xfig format.
//

ostream& PlotFigure::printXfig(ostream& out) {
   setAutoRange();

   int i;
   out << "#FIG 3.2\n";   // marker for XFIG file format 3.2
   out << "Portrait\n";   // orientation: Portrait or Landscape
   out << "Center\n";     // justification: "Center" or "Flush Left"
   out << "Inches\n";     // units: "Inches", or "Metric"
   out << "Letter\n";     // papersize: "Letter", "Legal", "Ledger", 
                          //  "Tabloid", "A4", "B3", "C3", "D3", etc.
   out << "100.00\n";     // magnification percentage
   out << "Single\n";     // multiple-page: "Single" or "Multiple"
   out << "-2\n";         // transparent color: -2=none, -3=background
                          //  -1=default 0-31, 32+
   out << "#Width:\t"     << getWidthIn()   << " in" << endl; 
   out << "#Height:\t"    << getHeightIn()  << " in" << endl; 
   out << "#LMargin:\t"   << getLMarginIn() << " in" << endl; 
   out << "#TMargin:\t"   << getTMarginIn() << " in" << endl; 
   out << "#PlotCount:\t" << plotdata.getSize() << endl;
   out << "#XMin:\t"      << getXMin() << endl;
   out << "#XMax:\t"      << getXMax() << endl;
   out << "#YMin:\t"      << getYMin() << endl;
   out << "#YMax:\t"      << getYMax() << endl;
   out << "1200 2\n";     // resolution, coordinates: 1=lower left (not used)
                          //  2 = upper left (always used)

   double xorigin = getLMarginIn() * 1200;
   double yorigin = getTMarginIn() * 1200 + getHeightIn() * 1200;
   double xdelta  = getWidthIn()   * 1200;
   double ydelta  = getHeightIn()  * 1200;

   // print plot box
   out << "# Plot box\n";
   out << "2 2 0 1 0 7 100 0 -1 0.000 0 0 -1 0 0 5\n";
   out << "\t";
   out << xorigin << " "   << yorigin << " ";  // bottom left
   out << xorigin + xdelta << " "     << yorigin << " ";
   out << xorigin + xdelta << " "     << yorigin - ydelta << " ";
   out << xorigin << " "   << yorigin - ydelta << " ";
   out << xorigin << " "   << yorigin << "\n";  // bottom left again

   printXFigTicks();

   // print title
   if (getTitle()[0] != '\0') {
      out << "# Plot title\n";
      out << "4 1 0 100 0 0 ";
      out << sqrt(xdelta * ydelta) * .09 / 1200 * 72; 
      out << " 0.0000 4 105 645 ";
      out << (int)(xorigin + xdelta/2.0 + 0.5) << " ";
      out << (int)(yorigin + 0.5 - ydelta - 1200/72 * 25) << " ";
      out << getTitle() << "\\001\n";
   }

   // print x-axis label
   if (getXLabel()[0] != '\0') {
      out << "# X-axis label\n";
      out << "4 1 0 100 0 0 ";
      out << sqrt(xdelta*ydelta) * .06 / 1200 * 72;
      out << " 0.0000 4 105 645 ";
      out << (int)(xorigin + xdelta/2.0 + 0.5) << " ";
      out << (int)(yorigin + 0.5 + 1200/72 * 50) << " ";
      out << getXLabel() << "\\001\n";
   }

   // print y-axis label
   if (getYLabel()[0] != '\0') {
      out << "# Y-axis label\n";
      out << "4 1 0 100 0 0 ";
      out << sqrt(xdelta*ydelta) * .06 / 1200 * 72;
      out << " 1.5708 4 105 645 ";  // 1.5708 = 90 degrees
      out << (int)(xorigin - 1200/72 * 35 + 0.5) << " ";
      out << (int)(yorigin - ydelta/2.0 + 0.5)   << " ";
      out << getYLabel() << "\\001\n";
   }

   // print marker lines
   for (i=0; i<vline.getSize(); i++) {
      out << "# Vertical Marker at x=" << vline[i] << "\n";
      out << "2 1 0 1 0 7 100 0 -1 0.000 0 0 -1 0 0 2 \n";
      out << (int)(xorigin + xdelta * (vline[i] - xmin)/(xmax - xmin) + 0.5);
      out << " ";
      out << (int)(yorigin + 0.5);
      out << " ";
      out << (int)(xorigin + xdelta * (vline[i] - xmin)/(xmax - xmin) + 0.5);
      out << " ";
      out << (int)(yorigin - ydelta + 0.5);
      out << "\n";
   }
   for (i=0; i<hline.getSize(); i++) {
      out << "# Horizontal Marker at x=" << hline[i] << "\n";
      out << "2 1 0 1 0 7 100 0 -1 0.000 0 0 -1 0 0 2 \n";
      out << (int)(xorigin + xdelta + 0.5);
      out << " ";
      out << (int)(yorigin - ydelta * (hline[i] - ymin)/(ymax - ymin) + 0.5);
      out << " ";
      out << (int)(xorigin + 0.5);
      out << " ";
      out << (int)(yorigin - ydelta * (hline[i] - ymin)/(ymax - ymin) + 0.5);
      out << "\n";
   }


   // print individual plots
   for (i=0; i<plotdata.getSize(); i++) {
      out << "#Plot:\t" << i+1 << endl;
      plotdata[i].printXfig(out, *this);
   }

   // print points
   if (xpoint.getSize() > 0) {
      out << "# points ====================================================\n";
   }
   int ifigx = 0;
   int ifigy = 0;
   int ifigr = 0;
   double figx = 0.0;
   double figy = 0.0;
   double figr = 0.0;
   for (i=0; i<xpoint.getSize(); i++) {
      figx = (xpoint[i] - xmin)/(xmax - xmin) * xdelta + xorigin;
      figy = yorigin - (ypoint[i] - ymin)/(ymax - ymin) * ydelta;
      figr = pradius[i]/(ymax - ymin) * xdelta;
      ifigx = (int)(figx + 0.5);
      ifigy = (int)(figy + 0.5);
      ifigr = (int)(figr + 0.5);
      
      out << "1 ";    // object_code (1 = ellipse)
      out << "3 ";    // sub_type (3 = circle defined by radius)
      out << "0 ";    // line_style
      out << "1 ";    // thickness
      out << "0 ";    // pen_color
      if (pstyle[i] == PSTYLE_CIRCLE) {
         out << "0 ";    // fill color
      } else if (pstyle[i] == PSTYLE_OPENCIRCLE) {
         out << "7 ";    // fill color
      } else {
         out << "0 ";    // fill color
      }
      out << "50 ";   // depth
      out << "0 ";    // pen_style (not used)
      if (pstyle[i] == PSTYLE_CIRCLE) {
         out << "20 ";    // area_fill
      } else if (pstyle[i] == PSTYLE_OPENCIRCLE) {
         out << "-1 ";    // area_fill
      } else {
         out << "20 ";    // area_fill
      }
      out << "0.000 ";// style_val
      out << "1 ";    // direction (always 1)
      out << "0.0000 "; // angle (radians, angle of the x-axis)
      out << ifigx << " " << ifigy << " ";  // coordinates of circle center
      out << ifigr << " " << ifigr << " ";  // x-y radius of circle
      out << ifigx << " " << ifigy << " ";  // starting point of circle def.
      // ending point of figure (at a 45 degree angle):
      out << (int)(figx + 0.70710678119 * figr + 0.5) << " ";
      out << (int)(figy + 0.70710678119 * figr + 0.5) << "\n";
   }

   // print text labels
   if (xpoint.getSize() > 0) {
      out << "# text labels ===============================================\n";
   }
   for (i=0; i<text.getSize(); i++) {
      out << "4 ";   // object_code (4 = text)
      switch (textjustify[i]) {
         case -1: out << "0 ";  break; // left   justified
         case  0: out << "1 ";  break; // center justified
         case  1: out << "2 ";  break; // right  justified
         default: out << "0 ";         // left   justified
      }
      out << "0 ";   // color (0=black)
      out << "40 ";  // depth
      out << "0 ";   // pen style (not used)
      out << "1 ";   // font 0 = Times Roman
      out << textsize[i] << " "; // fontsize
      out << textang[i]  << " "; // angle
      out << "2 ";    // font flags: 2 = PostScript font
      out << "0 ";    // height in fig units
      out << "0 ";    // width in fig units

      figx = (textx[i] - xmin)/(xmax - xmin) * xdelta + xorigin;
      figy = yorigin - (texty[i] - ymin)/(ymax - ymin) * ydelta;
      ifigx = (int)(figx + 0.5);
      ifigy = (int)(figy + 0.5);
      out << ifigx << " " << ifigy << " ";  // coordinates of text
      out << text[i].getBase() << "\\001";  // string for text 
      out << "\n";
   }

   return out;
}



//////////////////////////////
//
// PlotFigure::printXFigTicks --
//

void PlotFigure::printXFigTicks(void) {
   double xorigin = getLMarginIn() * 1200;
   double yorigin = getTMarginIn() * 1200 + getHeightIn() * 1200;
   double xdelta  = getWidthIn()   * 1200;
   double ydelta  = getHeightIn()  * 1200;

   double majorticklength = ydelta * 0.04; // 4% of plot height
   double minorticklength = majorticklength / 2.0;

   double startx    = 0.0;
   double endx      = 0.0;
   double minxdelta = 0.0;

   double starty    = 0.0;
   double endy      = 0.0;
   double minydelta = 0.0;

   int xtickcount = (int)((xmax - xmin) / xticksdelta);
   int ytickcount = (int)((ymax - ymin) / yticksdelta);

   double tickx;
   double tickylo;
   double tickyhi;

   double ticky;
   double tickxlo;
   double tickxhi;

   int i;

   // print x-axis ticks
   if (xticksQ && xtickcount < 100) {
      cout << "# X-axis ticks:\n";

      startx = (int)(xmin/xticksdelta + xticksoffset / xticksdelta)
               * xticksdelta + xticksoffset;
      if (startx < xmin) {
         startx += xticksdelta;
      }
      endx = (int)(xmax/xticksdelta + xticksoffset/xticksdelta + 0.5)
               * xticksdelta + xticksoffset;
      if (endx < xmax) {
         endx += xticksdelta;
      }
      if (endx > xmax) {
         endx -= xticksdelta;
      }
      minxdelta = xticksdelta/(xminorticks+1.0);

      double currentx = startx - xticksdelta;

      while (currentx <= endx) {

         if (currentx >= startx) {
            cout << "2 ";   // object code (2 = POLYLINE)
            cout << "1 ";   // subtype (1 = polyline)
            cout << "0 ";   // line_style
            cout << "1 ";   // line thickness: 1/80 in; 1/160 in in PostScript
            cout << "0 ";   // pen color
            cout << "7 ";   // fill color
            cout << "900 "; // depth 
            cout << "0 ";   // pen_style 
            cout << "-1 ";  // area_fill, -1=no fill
            cout << "0.000 "; // style_val (float)
            cout << "0 ";   // join_style
            cout << "0 ";   // cap_style
            cout << "-1 ";  // radius
            cout << "0 ";   // forward_arrow, 0=off, 1=on
            cout << "0 ";   // backward_arrow, 0=off, 1=on
            cout << "2 \n\t";   // npoints = number of points in line

            tickx   = xorigin + (currentx - xmin)/(xmax - xmin) * xdelta;
            tickylo = yorigin;
            if (xticksfull) {
               tickyhi = yorigin - ydelta;
            } else {
               tickyhi = tickylo - majorticklength;
            }
            cout << (int)(tickx + 0.5)   << " ";
            cout << (int)(tickylo + 0.5) << " ";
            cout << (int)(tickx + 0.5)   << " ";
            cout << (int)(tickyhi + 0.5);
            cout << "\n";

            if (!xticksfull) {
               cout << "2 ";   // object code (2 = POLYLINE)
               cout << "1 ";   // subtype (1 = polyline)
               cout << "0 ";   // line_style
               cout << "1 ";   // line thickness: 1/80 in; 1/160 in PostScript
               cout << "0 ";   // pen color
               cout << "7 ";   // fill color
               cout << "900 "; // depth 
               cout << "0 ";   // pen_style 
               cout << "-1 ";  // area_fill, -1=no fill
               cout << "0.000 "; // style_val (float)
               cout << "0 ";   // join_style
               cout << "0 ";   // cap_style
               cout << "-1 ";  // radius
               cout << "0 ";   // forward_arrow, 0=off, 1=on
               cout << "0 ";   // backward_arrow, 0=off, 1=on
               cout << "2 \n\t";   // npoints = number of points in line

               cout << (int)(tickx + 0.5)   << " ";
               cout << (int)(yorigin - ydelta) << " ";
               cout << (int)(tickx + 0.5)   << " ";
               cout << (int)(yorigin - ydelta + majorticklength);
               cout << "\n";
            }


            // print x-axis tick label centered below current tick
            cout << "4 1 0 901 0 0 ";
            cout << majorticklength/1200.0*72.0;  // font size in points
            cout << " 0.0000 4 105 99 ";
            cout << (int)(tickx + 0.5) << " ";
            cout << (int)(yorigin + 0.5 + majorticklength * 1.1) << " ";
            cout << currentx << "\\001\n";
         }

         // print minor ticks
         for (i=1; i<=xminorticks; i++) {
            if (currentx + i*minxdelta < xmin) {
               continue;
            }
            if (currentx + i*minxdelta > xmax) {
               break;
            }
            cout << "2 ";   // object code (2 = POLYLINE)
            cout << "1 ";   // subtype (1 = polyline)
            cout << "0 ";   // line_style
            cout << "1 ";   // line thickness: 1/80 in; 1/160 in in PostScript
            cout << "0 ";   // pen color
            cout << "7 ";   // fill color
            cout << "900 "; // depth 
            cout << "0 ";   // pen_style 
            cout << "-1 ";  // area_fill, -1=no fill
            cout << "0.000 "; // style_val (float)
            cout << "0 ";   // join_style
            cout << "0 ";   // cap_style
            cout << "-1 ";  // radius
            cout << "0 ";   // forward_arrow, 0=off, 1=on
            cout << "0 ";   // backward_arrow, 0=off, 1=on
            cout << "2 \n\t";   // npoints = number of points in line

            tickx   = xorigin + (currentx + (i)*minxdelta - xmin) / 
                        (xmax - xmin) * xdelta;
            tickylo = yorigin;
            tickyhi = tickylo - minorticklength;

            cout << (int)(tickx + 0.5)   << " ";
            cout << (int)(tickylo + 0.5) << " ";
            cout << (int)(tickx + 0.5)   << " ";
            cout << (int)(tickyhi + 0.5);
            cout << "\n";

            // print ticks on opposite side of plot
            cout << "2 ";   // object code (2 = POLYLINE)
            cout << "1 ";   // subtype (1 = polyline)
            cout << "0 ";   // line_style
            cout << "1 ";   // line thickness: 1/80 in; 1/160 in PostScript
            cout << "0 ";   // pen color
            cout << "7 ";   // fill color
            cout << "903 "; // depth 
            cout << "0 ";   // pen_style 
            cout << "-1 ";  // area_fill, -1=no fill
            cout << "0.000 "; // style_val (float)
            cout << "0 ";   // join_style
            cout << "0 ";   // cap_style
            cout << "-1 ";  // radius
            cout << "0 ";   // forward_arrow, 0=off, 1=on
            cout << "0 ";   // backward_arrow, 0=off, 1=on
            cout << "2 \n\t";   // npoints = number of points in line

            cout << (int)(tickx + 0.5)   << " ";
            cout << (int)(yorigin - ydelta) << " ";
            cout << (int)(tickx + 0.5)   << " ";
            cout << (int)(yorigin - ydelta + minorticklength);
            cout << "\n";
         }
    
         currentx += xticksdelta;
      }
    
   }

   // print y-axis ticks
   if (yticksQ && ytickcount < 100) {
      cout << "# Y-axis ticks:\n";
    
      starty = (int)(ymin/yticksdelta + yticksoffset / yticksdelta)
               * yticksdelta + yticksoffset;
      if (starty < ymin) {
         starty += yticksdelta;
      }
      endy = (int)(ymax/yticksdelta + yticksoffset/yticksdelta + 0.5)
               * yticksdelta + yticksoffset;
      if (endy < ymax) {
         endy += yticksdelta;
      }
      if (endy > ymax) {
         endy -= yticksdelta;
      }
      minydelta = yticksdelta/(yminorticks+1.0);
      
      double currenty = starty - yticksdelta;
    
      while (currenty-yticksdelta/1.001 <= endy) {
    
         if (currenty >= starty) {
            cout << "2 ";   // object code (2 = POLYLINE)
            cout << "1 ";   // subtype (1 = polyline)
            cout << "0 ";   // line_style
            cout << "1 ";   // line thickness: 1/80 in; 1/160 in in PostScript
            cout << "0 ";   // pen color
            cout << "7 ";   // fill color
            cout << "900 "; // depth 
            cout << "0 ";   // pen_style 
            cout << "-1 ";  // area_fill, -1=no fill
            cout << "0.000 "; // style_val (float)
            cout << "0 ";   // join_style
            cout << "0 ";   // cap_style
            cout << "-1 ";  // radius
            cout << "0 ";   // forward_arrow, 0=off, 1=on
            cout << "0 ";   // backward_arrow, 0=off, 1=on
            cout << "2 \n\t";   // npoints = number of points in line
    
            ticky   = yorigin - (currenty - ymin)/(ymax - ymin) * ydelta;
            tickxlo = xorigin;
            tickxhi = tickxlo + majorticklength;

            if (xticksfull) {
               tickxhi = xorigin + xdelta;
            } else {
               tickxhi = tickxlo + majorticklength;
            }

            cout << (int)(tickxlo + 0.5)   << " ";
            cout << (int)(ticky + 0.5) << " ";
            cout << (int)(tickxhi + 0.5)   << " ";
            cout << (int)(ticky + 0.5);
            cout << "\n";

            if (!yticksfull) {
               cout << "2 ";   // object code (2 = POLYLINE)
               cout << "1 ";   // subtype (1 = polyline)
               cout << "0 ";   // line_style
               cout << "1 ";   // line thickness: 1/80 in; 1/160 in PostScript
               cout << "0 ";   // pen color
               cout << "7 ";   // fill color
               cout << "900 "; // depth 
               cout << "0 ";   // pen_style 
               cout << "-1 ";  // area_fill, -1=no fill
               cout << "0.000 "; // style_val (float)
               cout << "0 ";   // join_style
               cout << "0 ";   // cap_style
               cout << "-1 ";  // radius
               cout << "0 ";   // forward_arrow, 0=off, 1=on
               cout << "0 ";   // backward_arrow, 0=off, 1=on
               cout << "2 \n\t";   // npoints = number of points in line

               cout << (int)(xorigin + xdelta + 0.5)   << " ";
               cout << (int)(ticky) << " ";
               cout << (int)(xorigin + xdelta + 0.5 - majorticklength) << " ";
               cout << (int)(ticky);
               cout << "\n";
            }

            // print y-axis tick label centered to the left of the current tick
            cout << "4 2 0 902 0 0 ";
            cout << majorticklength/1200.0*72.0;  // font size in points
            cout << " 0.0000 4 105 99 ";
            cout << (int)(tickxlo + 0.5 - majorticklength * 0.25) << " ";
            cout << (int)(ticky + 0.5 + majorticklength * 0.4) << " ";
            cout << currenty << "\\001\n";
         }
    
         // print minor ticks
         for (i=1; i<=yminorticks; i++) {
            if (currenty + i*minydelta < ymin) {
               continue;
            }
            if (currenty + i*minydelta > ymax) {
               break;
            }
            cout << "2 ";   // object code (2 = POLYLINE)
            cout << "1 ";   // subtype (1 = polyline)
            cout << "0 ";   // line_style
            cout << "1 ";   // line thickness: 1/80 in; 1/160 in in PostScript
            cout << "0 ";   // pen color
            cout << "7 ";   // fill color
            cout << "900 "; // depth 
            cout << "0 ";   // pen_style 
            cout << "-1 ";  // area_fill, -1=no fill
            cout << "0.000 "; // style_val (float)
            cout << "0 ";   // join_style
            cout << "0 ";   // cap_style
            cout << "-1 ";  // radius
            cout << "0 ";   // forward_arrow, 0=off, 1=on
            cout << "0 ";   // backward_arrow, 0=off, 1=on
            cout << "2 \n\t";   // npoints = number of points in line
    
            ticky   = yorigin - (currenty + (i)*minydelta - ymin) / 
                        (ymax - ymin) * ydelta;
            tickxlo = xorigin;
            tickxhi = tickxlo + minorticklength;
    
            cout << (int)(tickxlo + 0.5)   << " ";
            cout << (int)(ticky + 0.5) << " ";
            cout << (int)(tickxhi + 0.5)   << " ";
            cout << (int)(ticky + 0.5);
            cout << "\n";

            // minor ticks on far left of plot
            cout << "2 ";   // object code (2 = POLYLINE)
            cout << "1 ";   // subtype (1 = polyline)
            cout << "0 ";   // line_style
            cout << "1 ";   // line thickness: 1/80 in; 1/160 in PostScript
            cout << "0 ";   // pen color
            cout << "7 ";   // fill color
            cout << "900 "; // depth 
            cout << "0 ";   // pen_style 
            cout << "-1 ";  // area_fill, -1=no fill
            cout << "0.000 "; // style_val (float)
            cout << "0 ";   // join_style
            cout << "0 ";   // cap_style
            cout << "-1 ";  // radius
            cout << "0 ";   // forward_arrow, 0=off, 1=on
            cout << "0 ";   // backward_arrow, 0=off, 1=on
            cout << "2 \n\t";   // npoints = number of points in line

            cout << (int)(xorigin + xdelta + 0.5)   << " ";
            cout << (int)(ticky) << " ";
            cout << (int)(xorigin + xdelta + 0.5 - minorticklength) << " ";
            cout << (int)(ticky);
            cout << "\n";
         }
    
         currenty += yticksdelta;
      }
   }

}



//////////////////////////////
//
// PlotFigure::getWidth** -- get the width of the main figure.
//

double PlotFigure::getWidthIn(void) { 
   return width;
}
double PlotFigure::getWidthCm(void) { 
   return width * 2.54;
}
double PlotFigure::getWidthMm(void) { 
   return width * 25.4;
}



//////////////////////////////
//
// PlotFigure::setWidth** -- set the width of the main figure.
//

void PlotFigure::setWidthIn(double aWidth) { 
   width = aWidth;
}
void PlotFigure::setWidthCm(double aWidth) { 
   width = aWidth / 2.54;
}
void PlotFigure::setWidthMm(double aWidth) { 
   width = aWidth / 25.4;
}



//////////////////////////////
//
// PlotFigure::getHeight** -- get the height of the main figure.
//

double PlotFigure::getHeightIn(void) { 
   return height;
}
double PlotFigure::getHeightCm(void) { 
   return height * 2.54;
}
double PlotFigure::getHeightMm(void) { 
   return height * 25.4;
}



//////////////////////////////
//
// PlotFigure::setHeight** -- set the height of the main figure.
//

void PlotFigure::setHeightIn(double aHeight) { 
   height = aHeight; 
}
void PlotFigure::setHeightCm(double aHeight) { 
   height = aHeight / 2.54; 
}
void PlotFigure::setHeightMm(double aHeight) { 
   height = aHeight / 25.4; 
}



//////////////////////////////
//
// PlotFigure::getLMargin** -- get the left margin.
//

double PlotFigure::getLMarginIn(void) { 
   return lmargin;
}
double PlotFigure::getLMarginCm(void) { 
   return lmargin * 2.54;
}
double PlotFigure::getLMarginMm(void) { 
   return lmargin * 25.4;
}



//////////////////////////////
//
// PlotFigure::setLMargin -- set the left margin.
//

void PlotFigure::setLMarginIn(double aMargin) { 
   lmargin = aMargin;
}
void PlotFigure::setLMarginCm(double aMargin) { 
   lmargin = aMargin / 2.54;
}
void PlotFigure::setLMarginMm(double aMargin) { 
   lmargin = aMargin / 25.4;
}



//////////////////////////////
//
// PlotFigure::getTMargin** -- get top margin.
//

double PlotFigure::getTMarginIn(void) { 
   return tmargin;
}
double PlotFigure::getTMarginCm(void) { 
   return tmargin * 2.54;
}
double PlotFigure::getTMarginMm(void) { 
   return tmargin * 25.4;
}



//////////////////////////////
//
// PlotFigure::setTMargin** -- set the top margin.
//

void PlotFigure::setTMarginIn(double aMargin) { 
   tmargin = aMargin;
}
void PlotFigure::setTMarginCm(double aMargin) { 
   tmargin = aMargin / 2.54;
}
void PlotFigure::setTMarginMm(double aMargin) { 
   tmargin = aMargin / 25.4;
}



//////////////////////////////
//
// PlotFigure::set Min/Max range --
//

void PlotFigure::setXMin(double aValue) { xmin = aValue; }
void PlotFigure::setXMax(double aValue) { xmax = aValue; }
void PlotFigure::setYMin(double aValue) { ymin = aValue; }
void PlotFigure::setYMax(double aValue) { ymax = aValue; }



//////////////////////////////
//
// PlotFigure::get Min/Max Range --
//

double PlotFigure::getXMin(void) { return xmin; }
double PlotFigure::getXMax(void) { return xmax; }
double PlotFigure::getYMin(void) { return ymin; }
double PlotFigure::getYMax(void) { return ymax; }



//////////////////////////////
//
// PlotFigure::setAutoRange -- find the x and y range from the data.
//

void PlotFigure::setAutoRangeX(void) {
   int i; 
   double txmin = 0; double txmax = 0; double tymin = 0; double tymax = 0;
   double fxmin = 0; double fxmax = 0; 
   int init = 0;
   for (i=0; i<plotdata.getSize(); i++) {
      plotdata[i].getPlotRange(txmin, txmax, tymin, tymax);
      if (!init) {
         fxmin = txmin;
         fxmax = txmax;
         init = 1;
      } else {
         if (txmin < fxmin) fxmin = txmin;
         if (txmax > fxmax) fxmax = txmax;
      }
   }
   setXMin(fxmin);
   setXMax(fxmax);
}


void PlotFigure::setAutoRangeY(void) {
   int i; 
   double txmin = 0; double txmax = 0; double tymin = 0; double tymax = 0;
   double fymin = 0; double fymax = 0;
   int init = 0;
   for (i=0; i<plotdata.getSize(); i++) {
      plotdata[i].getPlotRange(txmin, txmax, tymin, tymax);
      if (!init) {
         fymin = tymin;
         fymax = tymax;
         init = 1;
      } else {
         if (tymin < fymin) fymin = tymin;
         if (tymax > fymax) fymax = tymax;
      }
   }
   setYMin(fymin);
   setYMax(fymax);
}


void PlotFigure::setAutoRange(void) {
   if (xrangeauto && !yrangeauto) {
      setAutoRangeX();
      return;
   } else if (yrangeauto && !xrangeauto) {
      setAutoRangeY();
      return;
   } else if (!xrangeauto && !yrangeauto) {
      return;
   }

   int i; 
   double txmin = 0; double txmax = 0; double tymin = 0; double tymax = 0;
   double fxmin = 0; double fxmax = 0; double fymin = 0; double fymax = 0;
   int init = 0;
   for (i=0; i<plotdata.getSize(); i++) {
      plotdata[i].getPlotRange(txmin, txmax, tymin, tymax);
      if (!init) {
         fxmin = txmin;
         fxmax = txmax;
         fymin = tymin;
         fymax = tymax;
         init = 1;
      } else {
         if (txmin < fxmin) fxmin = txmin;
         if (txmax > fxmax) fxmax = txmax;
         if (tymin < fymin) fymin = tymin;
         if (tymax > fymax) fymax = tymax;
      }
   }
   setXMin(fxmin);
   setXMax(fxmax);
   setYMin(fymin);
   setYMax(fymax);
}



//////////////////////////////
//
// PlotFigure::getXLabel -- 
//

const char* PlotFigure::getXLabel(void) {
   if (xlabel == NULL) {
      return "";
   } else {
      return xlabel;
   }
}



//////////////////////////////
//
// PlotFigure::getYLabel -- 
//

const char* PlotFigure::getYLabel(void) {
   if (ylabel == NULL) {
      return "";
   } else {
      return ylabel;
   }
}



//////////////////////////////
//
// PlotFigure::getTitle -- 
//

const char* PlotFigure::getTitle(void) {
   if (figuretitle == NULL) {
      return "";
   } else {
      return figuretitle;
   }
}



//////////////////////////////
//
// PlotFigure::setXLabel --
//

void PlotFigure::setXLabel(const char* string) {
   if (xlabel != NULL) {
      delete [] xlabel;
      xlabel = NULL;
   }
   int length = strlen(string);
   xlabel = new char[length+1];
   strcpy(xlabel, string);
}



//////////////////////////////
//
// PlotFigure::setYLabel --
//

void PlotFigure::setYLabel(const char* string) {
   if (ylabel != NULL) {
      delete [] ylabel;
      ylabel = NULL;
   }
   int length = strlen(string);
   ylabel = new char[length+1];
   strcpy(ylabel, string);
}



//////////////////////////////
//
// PlotFigure::setTitle --
//

void PlotFigure::setTitle(const char* string) {
   if (figuretitle != NULL) {
      delete [] figuretitle;
      figuretitle = NULL;
   }
   int length = strlen(string);
   figuretitle = new char[length+1];
   strcpy(figuretitle, string);
}



//////////////////////////////
//
// PlotFigure::addPlot --
//

int PlotFigure::addPlot(PlotData& aPlot) {
   plotdata.setSize(plotdata.getSize()+1);
   plotdata.last() = aPlot;
   return plotdata.getSize() - 1;
}




//////////////////////////////
//
// PlotFigure::addVLine -- 
//

void PlotFigure::addVLine(double position, const char* label) {
   vline.append(position);
   // label will be stored here
}



//////////////////////////////
//
// PlotFigure::addHLine -- 
//

void PlotFigure::addHLine(double position, const char* label) {
   hline.append(position);
   // label will be stored here
}



//////////////////////////////
//
// PlotFigure::addText --
//

void PlotFigure::addText(const char* textstring, double xpos, double ypos, 
      double aSize, double angle, int orientation) {
   Array<char> teststring;
   int length = strlen(textstring);
   teststring.setSize(length+1);
   strcpy(teststring.getBase(), textstring);
   text.append(teststring);
   textsize.append(aSize);
   textx.append(xpos);
   texty.append(ypos);
   textang.append(angle);
   textjustify.append(orientation);
}



//////////////////////////////
//
// pointAllocation -- allocation empty spots for points in figure
//

void PlotFigure::pointAllocation(int aSize) {
   int psize = xpoint.getSize();
   if (psize >= aSize) {
      return;
   }

   xpoint.setSize(aSize);
   ypoint.setSize(aSize);
   pstyle.setSize(aSize);
   pradius.setSize(aSize);

   xpoint.setSize(psize);
   ypoint.setSize(psize);
   pstyle.setSize(psize);
   pradius.setSize(psize);

}



//////////////////////////////
//
// PlotFigure::addPoint -- add a point to the figure
//    default value: radius = 0.1 
//    default value: type = 0 (filled circle)
//

void PlotFigure::addPoint(double x, double y, double radius, int type) {
   xpoint.append(x);
   ypoint.append(y);
   pstyle.append(type);
   pradius.append(radius);
}



//////////////////////////////
//
// PlotFigure::commandEqual --
//

int PlotFigure::commandEqual(const char* command, const char* string) {
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


// md5sum: 8949f37fe4f272df575985989917bc5c PlotFigure.cpp [20050403]
