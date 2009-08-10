//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Dec 24 18:25:42 PST 2002
// Last Modified: Tue Dec 24 18:25:44 PST 2002
// Filename:      ...sig/include/sigInfo/RootSpectrum.cpp
// Web Address:   http://sig.sapp.org/include/sigInfo/RootSpectrum.cpp
// Syntax:        C++ 
//
// Description:   Store and calculate chord root calculations.
//

#include "RootSpectrum.h"
#include "PlotFigure.h"
#include "Convert.h"

#include <math.h>
#include <string.h>

#ifndef OLDCPP
   using namespace std;
#endif



//////////////////////////////
//
// RootSpectrum::RootSpectrum --
//

RootSpectrum::RootSpectrum(void) { 
   values.setSize(40);
   values.allowGrowth(0);
   values.setAll(100000.0);
   power = 1.0;
   rhythmOn();
   melodyOn();
   rhythmLinear();
   setMeterBias(3.0);
   setDurationBias(4.0);
   setMelodyScaleFactor(1.0);
   setResolutionFactor(1.0);
   setDurationWeight(0.25);
   setMetricLevelWeight(0.25);
}



//////////////////////////////
//
// RootSpectrum::~RootSpectrum --
//

RootSpectrum::~RootSpectrum() { 
   values.setSize(0);
}



//////////////////////////////
//
// RootSpectrum::bestIndex -- return the index
//

int RootSpectrum::bestIndex(void) {
   int output = 0;
   for (int i=1; i<getSize(); i++) {
      if (values[i] < values[output]) {
         output = i;
      }
   }
   return output;
}



//////////////////////////////
//
// RootSpectrum::getRawScore -- return the chord root score in
//   an unmodified form.
//

double RootSpectrum::getRawScore(int index) { 
   return values[index];
}



//////////////////////////////
//
// RootSpectrum::getInvScore -- return the inverse value of the raw score.
//

double RootSpectrum::getInvScore(int index) {
   double output = 1.0/values[index];
   if (getPower() != 1.0) {
      return pow(output, getPower());
   } else {
      return output;
   }
}



//////////////////////////////
//
// RootSpectrum::getPower -- return the power parameter setting.
//   used to scale the inverse score values.
//

double RootSpectrum::getPower(void) {
   return power;
}



//////////////////////////////
//
// RootSpectrum::setPower -- set the power parameter setting.
//   used to scale the inverse score values.
//

void RootSpectrum::setPower(double aPower) {
   power = aPower;
}



//////////////////////////////
//
// RootSpectrum::getDurationWeight -- 
//

double RootSpectrum::getDurationWeight(void) { 
   return durationWeight;
}



//////////////////////////////
//
// RootSpectrum::setDurationWeight --
//

void RootSpectrum::setDurationWeight(double aWeight) { 
   durationWeight = aWeight;
}



//////////////////////////////
//
// RootSpectrum::getMetricLevelWeight --
//

double RootSpectrum::getMetricLevelWeight(void) { 
   return levelWeight;
}



//////////////////////////////
//
// RootSpectrum::setMetricLevelWeight --
//

void RootSpectrum::setMetricLevelWeight(double aWeight) { 
   levelWeight = aWeight;
}



//////////////////////////////
//
// RootSpectrum::setRhythmWeights --
//

void RootSpectrum::setRhythmWeights(double durWeight, double levWeight) { 
   durationWeight = durWeight;
   levelWeight    = levWeight;
}



//////////////////////////////
//
// RootSpectrum::setDuration --
//

void RootSpectrum::setDuration(int aState) { 
   if (aState == 0) {
      durationOff();
   } else {
      durationOn();
   }
}



//////////////////////////////
//
// RootSpectrum::setMetricLevel --
//


void RootSpectrum::setMetricLevel(int aState) { 
   if (aState == 0) {
      metricLevelOff();
   } else {
      metricLevelOn();
   }
}



//////////////////////////////
//
// RootSpectrum::rhythmOn -- enable duration and metric weightings.
//

void RootSpectrum::rhythmOn(void) { 
   durationOn();
   metricLevelOn();
}



//////////////////////////////
//
// RootSpectrum::rhythmOff -- disable duration and metric weightings.
//

void RootSpectrum::rhythmOff(void) { 
   durationOff();
   metricLevelOff();
}



//////////////////////////////
//
// RootSpectrum::durationOn -- enable duration weighting
//

void RootSpectrum::durationOn(void) { 
   durationQ = 1;
}



//////////////////////////////
//
// RootSpectrum::durationOff -- disable duration weighting
//

void RootSpectrum::durationOff(void) { 
   durationQ = 0;
}



//////////////////////////////
//
// RootSpectrum::metricLevelOn -- enable metric level weighting.
//

void RootSpectrum::metricLevelOn(void) { 
   levelQ = 1;
}



//////////////////////////////
//
// RootSpectrum::metricLevelOff -- disable metric level weighting.
//

void RootSpectrum::metricLevelOff(void) { 
   levelQ = 0;
}



//////////////////////////////
//
// RootSpectrum::melodyOn -- turn on non-harmonic tone estimation.
//

void RootSpectrum::melodyOn(void) {
    melodyQ = 1;

}



//////////////////////////////
//
// RootSpectrum::melodyOff -- turn on non-harmonic tone estimation.
//

void RootSpectrum::melodyOff(void) {
   melodyQ = 0;
}



//////////////////////////////
//
// RootSpectrum::setMelodyScaleFactor -- 
//

void RootSpectrum::setMelodyScaleFactor(double aFactor) {
   melodyScaleFactor = aFactor;
}



//////////////////////////////
//
// RootSpectrum::setMelodyScaleFactor -- 
//

double RootSpectrum::getMelodyScaleFactor(void) {
   return melodyScaleFactor;
}




//////////////////////////////
//
// RootSpectrum::getNormInvScore -- return the inverse value of the
//     raw score, with the maximum value of all of the scores set to
//     1.0.
//

double RootSpectrum::getNormInvScore(int index) {
   double output = values[bestIndex()]/values[index];
   if (getPower() != 1.0) {
      return pow(output, getPower());
   } else {
      return output;
   }
}



//////////////////////////////
//
// RootSpectrum::getSize -- should always return 40 (the base-40 octave)
//

int RootSpectrum::getSize(void) { 
   return values.getSize();
}




//////////////////////////////
//
// RootSpectrum::operator[] -- same as getScore();
//

double& RootSpectrum::operator[](int index) { 
   return values[index];
}



//////////////////////////////
//
// RootSpectrum::printMMA -- print chord spectrum for 2d display in
//   Mathematica.
//   default value: out = cout, functionsQ = 1, normQ = 0
//


ostream& RootSpectrum::printMMA(const char* label, ostream& out, 
      int functionsQ, int normQ) {
   if (functionsQ != 0) {
      out << 
"(* Functions for displaying 2-D root spectra *)\n"
"\n"
"RootPlotDensity[array_?ListQ, repeat_?IntegerQ:2] := Module[\n"
"   {\n"
"      i, x, newarray, labels, longpitches = {}, plotoptions,\n"
"      pitches = {\"C\", \"E\", \"G\", \"B\", \"D\", \"F\", \"A\"},\n"
"      yaxis = {{0.5,\"bb\"},{1.5,\"b\"},{2.5,\"nat.\"},{3.5,\"#\"},{4.5,\"##\"}}\n"
"   },\n"
"   labels = Table[x, {x, 0.5, 6.5 * repeat + repeat/2 , 1}];\n"
"   For[i=0, i<repeat, i++, \n"
"      longpitches = Flatten[Append[longpitches, pitches]]];\n"
"   longpitches = Flatten[longpitches];\n"
"   labels = Transpose[{labels, longpitches}]; \n"
"   newarray = Transpose[Flatten[Table[array, {x,  repeat}], 1]];\n"
"   Show[DensityGraphics[newarray], FrameTicks -> {labels, yaxis}, \n"
"      AspectRatio -> 1/2]\n"
"]\n"
"\n"
"RootPlotContour[array_?ListQ, repeat_?IntegerQ:2, options___] := Module[\n"
"   {\n"
"      x, i, newarray, labels, longpitches = {},\n"
"      pitches = {\"C\", \"E\", \"G\", \"B\", \"D\", \"F\", \"A\"},\n"
"      yaxis = {{1,\"bb\"}, {2,\"b\"}, {3,\"nat.\"}, {4,\"#\"}, {5,\"##\"}}\n"
"   },\n"
"   labels = Table[x, {x, 1, 7 * repeat , 1}];\n"
"   For[i=0, i<repeat, i++,\n"
"      longpitches = Flatten[Append[longpitches, pitches]]];\n"
"   longpitches = Flatten[longpitches];\n"
"   labels = Transpose[{labels, longpitches}]; \n"
"   newarray = Transpose[Flatten[Table[array, {x,  repeat}], 1]];\n"
"   Show[ContourGraphics[newarray], FrameTicks -> {labels, yaxis}, \n"
"      AspectRatio -> 1/2, Contours -> 12, \n"
"      DefaultFont -> {\"Times-Roman\", 14}, PlotRange -> All]\n"
"]\n"
"\n"
"(* Display root spectra in 3-D: \n"
" *    Show[SurfaceGraphics[rspec" << label << "], ViewPoint -> {1, 0, 0}];\n"
" *)\n";
   }

   out.setf(ios::fixed);
   out << "\nrspec" << label << " = {\n";
   out << "\t\t{";                   // C
   if (normQ) {
      out << getNormInvScore(38) << "," 
          << getNormInvScore(39) << ","
          << getNormInvScore(0) << "," << getNormInvScore(1) << ","
          << getNormInvScore(2) << "}, (* C *)\n"
          << "\t\t{"                       // E
          << getNormInvScore(10) << "," << getNormInvScore(11) << ","
          << getNormInvScore(12) << "," << getNormInvScore(13) << ","
          << getNormInvScore(14) << "}, (* E *)\n"
          << "\t\t{";                      // G
      out << getNormInvScore(21) << "," << getNormInvScore(22) << ",";
      out << getNormInvScore(23) << "," << getNormInvScore(24) << ",";
      out << getNormInvScore(25) << "}, (* G *)\n";
      out << "\t\t{";                      // B
      out << getNormInvScore(33) << "," << getNormInvScore(34) << ",";
      out << getNormInvScore(35) << "," << getNormInvScore(36) << ",";
      out << getNormInvScore(37) << "}, (* B *)\n";
      out << "\t\t{";                      // D
      out << getNormInvScore(4) << "," << getNormInvScore(5) << ",";
      out << getNormInvScore(6) << "," << getNormInvScore(7) << ",";
      out << getNormInvScore(8) << "}, (* D *)\n";
      out << "\t\t{";                      // F
      out << getNormInvScore(15) << "," << getNormInvScore(16) << ",";
      out << getNormInvScore(17) << "," << getNormInvScore(18) << ",";
      out << getNormInvScore(19) << "}, (* F *)\n";
      out << "\t\t{";                      // A
      out << getNormInvScore(27) << "," << getNormInvScore(28) << ",";
      out << getNormInvScore(29) << "," << getNormInvScore(30) << ",";
      out << getNormInvScore(31) << "}}; (* A *)\n";
      out << endl;
   } else {
      out << getInvScore(38) << "," << getInvScore(39) << ",";
      out << getInvScore(0) << "," << getInvScore(1) << ",";
      out << getInvScore(2) << "}, (* C *)\n";
      out << "\t\t{";                      // E
      out << getInvScore(10) << "," << getInvScore(11) << ",";
      out << getInvScore(12) << "," << getInvScore(13) << ",";
      out << getInvScore(14) << "}, (* E *)\n";
      out << "\t\t{";                      // G
      out << getInvScore(21) << "," << getInvScore(22) << ",";
      out << getInvScore(23) << "," << getInvScore(24) << ",";
      out << getInvScore(25) << "}, (* G *)\n";
      out << "\t\t{";                      // B
      out << getInvScore(33) << "," << getInvScore(34) << ",";
      out << getInvScore(35) << "," << getInvScore(36) << ",";
      out << getInvScore(37) << "}, (* B *)\n";
      out << "\t\t{";                      // D
      out << getInvScore(4) << "," << getInvScore(5) << ",";
      out << getInvScore(6) << "," << getInvScore(7) << ",";
      out << getInvScore(8) << "}, (* D *)\n";
      out << "\t\t{";                      // F
      out << getInvScore(15) << "," << getInvScore(16) << ",";
      out << getInvScore(17) << "," << getInvScore(18) << ",";
      out << getInvScore(19) << "}, (* F *)\n";
      out << "\t\t{";                      // A
      out << getInvScore(27) << "," << getInvScore(28) << ",";
      out << getInvScore(29) << "," << getInvScore(30) << ",";
      out << getInvScore(31) << "}}; (* A *)\n";
      out << endl;
   }


   return out;
}



//////////////////////////////
//
// RootSpectrum::printHumdrum --
//    default value out = cout, type = DISPLAY_DIATONIC, invQ = 1, normQ = 0
//

ostream& RootSpectrum::printHumdrum(ostream& out, int type, 
      int invQ, int normQ) { 
   int i;
   double value = 0.0;
   int findex;
   char buffer[64] = {0};
   Array<int> bestorder;
   getBestOrdering(bestorder);
   if (durationQ) {
      out << "!! duration weighting: " << getDurationWeight() << "\n";
   }
   if (levelQ) {
      out << "!! metric weighting:   " << getMetricLevelWeight() << "\n";
   }
   out << "**kern\t**rootscore\n";
   for (i=0; i<40; i++) {
      switch (type) {
         case DISPLAY_CHROMATIC:
            findex = (15 + i * 23) % 40;
            break;
         case DISPLAY_BEST:
            findex = bestorder[i];
            break;
         case DISPLAY_DIATONIC:
         default:
            findex = (i-2+40)%40;
      }
      Convert::base40ToKern(buffer, (findex+2)%40+4*40);
      if (buffer[0] == '\0') {
         continue;
      }

      if (normQ && invQ) {
         value = getNormInvScore(findex);
      } else if (invQ) {
         value = getInvScore(findex);
      } else {
         value = values[findex];
      }
      if (value > 0.0001) {
         out << buffer << "\t" << value << "\n";
      } else {
         out.setf(ios::fixed);
         out << buffer << "\t" << value << "\n";
         out.unsetf(ios::fixed);
      }
   }
   out << "*-\t*-\n";
   return out;
}



//////////////////////////////
//
// RootSpectrum::getBestOrdering -- returns the ordering
//     of the root scores from best to worst.
//

class _index_value { public: double value; int index; };

void RootSpectrum::getBestOrdering(Array<int>& bestordering) {
   Array<_index_value> iv(40);
   int i;
   for (i=0; i<iv.getSize(); i++) {
      iv[i].index = i;
      iv[i].value = values[i];
   }
   qsort(iv.getBase(), iv.getSize(), sizeof(_index_value), minfloat);
   bestordering.setSize(40);
   for (i=0; i<bestordering.getSize(); i++) {
      bestordering[i] = iv[i].index;
   }
}



//////////////////////////////
//
// RootSpectrum::minfloat -- sorting function from low to high.
//

int RootSpectrum::minfloat(const void* a, const void* b) {
   _index_value& A = *((_index_value*)a);   
   _index_value& B = *((_index_value*)b);   
   if (A.value < B.value) {
      return -1;
   } else if (A.value > B.value) {
      return +1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// RootSpectrum::printXfig -- print spectrum in xfig format
//     default value: out = cout, type = 0 (diatonic)
//          title: "Root Spectrum"       1 (by fifths)
//                                       2 (best first)
//

ostream& RootSpectrum::printXfig(ostream& out, const char* title, int type) {
   PlotFigure figure;
   PlotData   data;
   figure.allocatePoints(40);
   int i;
   double yvalue;
   double angle = 90.0 * 3.14159624/180.0;
   double max = getInvScore(bestIndex());
   double pointsize = 0.007 * max;
   int justify = -1;   // left
   Array<int> bestorder;
   getBestOrdering(bestorder);
   int count = 0;
   double fontsize = 14.0;
   char buffer[64] = {0};
   int nextindex = -1;
   int lastindex = -1;
   int findex    = -1;
   for (i=0; i<40; i++) {
      lastindex = findex;     
      switch (type) {
         case DISPLAY_CHROMATIC:
            findex = (15 + i * 23) % 40;
            if (i+1<40) {
               nextindex = (15 + (i+1) * 23) % 40;
            } else {
               nextindex = -1;
            }
            break;
         case DISPLAY_BEST:
            findex = bestorder[i];
            if (i+1 < 40) {
               nextindex = bestorder[i+1];
            } else {
               nextindex = -1;
            }
            break;
         case DISPLAY_DIATONIC:
         default:
            findex = (i-2+40)%40;
            if (i+1<40) {
               nextindex = (i+1-2+40)%40;
            } else {
               nextindex = -1;
            }
      }
      Convert::base40ToKern(buffer, ((findex+2)%40) + 4*40);
      if (buffer[0] == '\0') {
         continue;
      }
      yvalue = getInvScore(findex);
      figure.addText(buffer, count+0.2, -0.080 * max, fontsize, angle, justify);
      if (i > 0 && i < 39 && lastindex >=0 && lastindex < 40 && 
            nextindex >= 0 && nextindex < 40) {
         if (type == DISPLAY_CHROMATIC && (strcmp(buffer, "b##") == 0)) {
            // do not display B## as a peak
         } else if ((values[findex] < values[lastindex]) && 
             (values[findex] < values[nextindex]) ) {
            figure.addText(buffer, count, 0.02 * max + yvalue, 
               fontsize - fontsize/8.0, 0, 0);
         }
      }
      figure.addPoint(count, yvalue, pointsize);
      data.addPoint(count, yvalue);


      count++;
   }
   figure.setYMin(0.0);
   figure.setXRangeAutoOff();
   figure.setXMin(-0.5);
   figure.setXMax(34.5);
   figure.addPlot(data);
   figure.setYLabel("Inverse Score");
   figure.setXLabel("Root Pitch");
   figure.setTitle(title);
   figure.yTicksOn();
   figure.setYTicksDelta(0.1);

   return figure.printXfig(out);
}



//////////////////////////////
//
// RootSpectrum::printPlot -- print spectrum in pre xfig plot format
//     default value: out = cout, type = DISPLAY_DIATONIC
//

ostream& RootSpectrum::printPlot(ostream& out, int type) {
   // not implemented yet, just output xfig code instead
   return printXfig(out);
}



/////////////////////////////////////////////////////////////////////////////
//
// Analysis Functions
//

//////////////////////////////
//
// RootSpectrum::calculate -- returns the best root found for chord data.
//     default value: debugQ = 0
//


int RootSpectrum::calculate(IntervalWeight& distances, HumdrumFile& infile, 
      int startline, int stopline, int debugQ) {

   int i;
   int j;

   Array<double>& rootscores = values;

   double chordstartbeat = infile[startline].getAbsBeat();
   double chordendbeat   = infile[stopline].getAbsBeat();

   // extract note data
   Array<double> absbeat;
   Array<int>    pitches;
   Array<double> durations;
   Array<double> levels;
   Array<Array<int> > lastpitches;   // pitches which approach note
   Array<Array<int> > nextpitches;   // pitches which leave note


   char buffer[64] = {0};
   if (!melodyQ) {
      infile.getNoteArray(absbeat, pitches, durations, levels, startline,
         stopline);

      if (debugQ) {
         cout << "!! Note information from line " << startline + 1
           << " to line " << stopline + 1 << "\n"
           << "!! **dur     = duration of the note in quarter note units.\n"
           << "!! **absbeat = absolute beat position of note from start (beat 0)\n"
           << "!! **level   = metric level of the note on linear scale\n";
         cout << "**kern\t**dur\t**level\t**absbeat\n";
         for (i=0; i<pitches.getSize(); i++) {
            cout << Convert::base40ToKern(buffer, pitches[i]) << "\t";
            cout << durations[i] << "\t";
            cout << levels[i]    << "\t";
            cout << absbeat[i]   << "\n";
         }
         cout << "*-\t*-\t*-\t*-\n";
      }
   } else {
      infile.getNoteArray2(absbeat, pitches, durations, levels, 
         lastpitches, nextpitches, startline, stopline);

      if (debugQ) {
         cout << "!! Note information from line " << startline + 1
           << " to line " << stopline + 1 << "\n"
           << "!! **dur     = duration of the note in quarter note units.\n"
           << "!! **absbeat = absolute beat position of note from start (beat 0)\n"
           << "!! **level   = metric level of the note on linear scale\n";
         cout << "**kern\t**last\t**next\t**dur\t**level\t**absbeat\n";
         for (i=0; i<pitches.getSize(); i++) {
            cout << Convert::base40ToKern(buffer, pitches[i]) << "\t";
   
            if (lastpitches[i].getSize() == 0) {
               cout << "." << "\t";
            } else {
               for (j=0; j<lastpitches[i].getSize(); j++) {
                  cout << Convert::base40ToKern(buffer, lastpitches[i][j]); 
                  if (j<lastpitches[i].getSize() - 1) {
                     cout << " ";
                  }
               }
               cout << "\t";
            }
   
            if (nextpitches[i].getSize() == 0) {
               cout << "." << "\t";
            } else {
               for (j=0; j<nextpitches[i].getSize(); j++) {
                  cout << Convert::base40ToKern(buffer, nextpitches[i][j]); 
                  if (j<nextpitches[i].getSize() - 1) {
                     cout << " ";
                  }
               }
               cout << "\t";
            }
   
            cout << durations[i] << "\t";
            cout << levels[i]    << "\t";
            cout << absbeat[i]   << "\n";
         }
         cout << "*-\t*-\t*-\t*-\n";
      }
   }

   if (pitches.getSize() == 0) {
      // give up if there are no notes in the specified region
      values.setAll(100000.0);
      return -1;
   }

   rootscores.setSize(40);
   rootscores.setAll(0.0);
   double melodyscaling;

   for (i=0; i<rootscores.getSize(); i++) {
      for (j=0; j<pitches.getSize(); j++) {
         if (melodyQ) {
            melodyscaling = getMelodicScaling(i, pitches[j], 
               lastpitches[j], nextpitches[j], distances, 
               absbeat[j], chordstartbeat, chordendbeat, durations[j]);
         } else {
            melodyscaling = 1.0;
         }
         rootscores[i] += distances[(pitches[j]-i+400)%40] *
            durationscaling(durations[j]) * 
            metricscaling(levels[j]) *
            melodyscaling;
      }
   }

   return bestIndex();
}


/////////////////////////////////////////////////////////////////////////////
//
// Private Functions
//


//////////////////////////////
//
// RootSpectrum::nonchordtoneQ -- returns true if a non-chord tone.
//    with respect to the given root (not a root, 3rd, 5th, or 7th).
//

int RootSpectrum::nonchordtoneQ(int root, int note) {
   int interval = ((note-2) - root + 400) % 40;
   switch (interval) {
      case 0:   // note is tonic
      case 11:  // note is 3rd
      case 12:  // note is 3rd
      case 22:  // note is 5th
      case 23:  // note is 5th
      case 24:  // note is 5th
      case 33:  // note is 7th
      case 34:  // note is 7th
      case 35:  // note is 7th
      case 36:  // note is 7th
         return 0;
         break;
      default:
         return 1;  // note is a chord tone
   }
   return 1;
}

int RootSpectrum::nonchordtoneQno7th(int root, int note) {
   int interval = ((note-2) - root + 400) % 40;
   switch (interval) {
      case 0:   // note is tonic
      case 11:  // note is 3rd
      case 12:  // note is 3rd
      case 22:  // note is 5th
      case 23:  // note is 5th
      case 24:  // note is 5th
//      case 33:  // note is 7th
//      case 34:  // note is 7th
//      case 35:  // note is 7th
//      case 36:  // note is 7th
         return 0;
         break;
      default:
         return 1;  // note is a chord tone
   }
   return 1;
}



//////////////////////////////
//
// RootSpectrum::getMelodicScaling -- get the scaling value
//     by taking into account non-harmonic tones which will be given
//     a different interval weight.
//
//

double RootSpectrum::getMelodicScaling(int root, int note, 
      Array<int>& lastpitches, Array<int>& nextpitches, 
      IntervalWeight& distances, double absbeat, double chordstartbeat, 
      double chordendbeat, double duration) {


   if (!melodyQ) {
      return 1.0;
   }

   double scalefactor = getMelodyScaleFactor();

   if (lastpitches.getSize() == 0 && nextpitches.getSize() == 0) {
      // check to see if the note is non-harmonic with respect to root
      if (nonchordtoneQ(root, note)) {
         return nonresolutionscaling;
      } else {
         return 1.0;
      }
   }

   int i;
   int testchordn;
   int testnonharmn;

   // check to see if there is a non-harmonic candidate in lastpitches.
   for (i=0; i<lastpitches.getSize(); i++) {
        testnonharmn = ((note-2) - root + 400) % 40;
        testchordn = ((lastpitches[i]-2) - root + 400) % 40;

      // ascending non-harmonic tones
      if (testchordn == 11 && testnonharmn == 5)  { return scalefactor; }
      if (testchordn == 12 && testnonharmn == 5)  { return scalefactor; }
      if (testchordn == 11 && testnonharmn == 6)  { return scalefactor; }
      if (testchordn == 12 && testnonharmn == 6)  { return scalefactor; }
      if (testchordn == 22 && testnonharmn == 16) { return scalefactor; }
      if (testchordn == 23 && testnonharmn == 16) { return scalefactor; }
      if (testchordn == 22 && testnonharmn == 17) { return scalefactor; }
      if (testchordn == 23 && testnonharmn == 17) { return scalefactor; }
      if (testchordn == 24 && testnonharmn == 17) { return scalefactor; }
      if (testchordn == 23 && testnonharmn == 18) { return scalefactor; }
      if (testchordn == 24 && testnonharmn == 18) { return scalefactor; }
      if (testchordn == 33 && testnonharmn == 27) { return scalefactor; }
      if (testchordn == 34 && testnonharmn == 27) { return scalefactor; }
      if (testchordn == 33 && testnonharmn == 28) { return scalefactor; }
      if (testchordn == 34 && testnonharmn == 28) { return scalefactor; }
      if (testchordn == 35 && testnonharmn == 28) { return scalefactor; }
      if (testchordn == 34 && testnonharmn == 29) { return scalefactor; }
      if (testchordn == 35 && testnonharmn == 29) { return scalefactor; }
      if (testchordn == 36 && testnonharmn == 29) { return scalefactor; }
      if (testchordn == 35 && testnonharmn == 30) { return scalefactor; }
      if (testchordn == 36 && testnonharmn == 30) { return scalefactor; }
 
      // decending non-harmonic tones
      if (testchordn ==  0 && testnonharmn ==  5) { return scalefactor; }
      if (testchordn ==  0 && testnonharmn ==  6) { return scalefactor; }
      if (testchordn == 11 && testnonharmn == 16) { return scalefactor; }
      if (testchordn == 11 && testnonharmn == 17) { return scalefactor; }
      if (testchordn == 12 && testnonharmn == 17) { return scalefactor; }
      if (testchordn == 12 && testnonharmn == 18) { return scalefactor; }
      if (testchordn == 22 && testnonharmn == 27) { return scalefactor; }
      if (testchordn == 22 && testnonharmn == 28) { return scalefactor; }
      if (testchordn == 23 && testnonharmn == 28) { return 0.05 * scalefactor; }
      if (testchordn == 23 && testnonharmn == 29) { return 0.05 * scalefactor; }
      if (testchordn == 24 && testnonharmn == 29) { return scalefactor; }
      if (testchordn == 24 && testnonharmn == 30) { return scalefactor; }

   }

   // check to see if there is a non-harmonic candidate in nextpitches.
   for (i=0; i<nextpitches.getSize(); i++) {
      testnonharmn = ((note-2) - root + 400) % 40;
      testchordn = ((nextpitches[i]-2) - root + 400) % 40;

      // ascending non-harmonic tones
      if (testchordn == 11 && testnonharmn == 5)  { return scalefactor; }
      if (testchordn == 12 && testnonharmn == 5)  { return scalefactor; }
      if (testchordn == 11 && testnonharmn == 6)  { return scalefactor; }
      if (testchordn == 12 && testnonharmn == 6)  { return scalefactor; }
      if (testchordn == 22 && testnonharmn == 16) { return scalefactor; }
      if (testchordn == 23 && testnonharmn == 16) { return scalefactor; }
      if (testchordn == 22 && testnonharmn == 17) { return scalefactor; }
      if (testchordn == 23 && testnonharmn == 17) { return scalefactor; }
      if (testchordn == 24 && testnonharmn == 17) { return scalefactor; }
      if (testchordn == 23 && testnonharmn == 18) { return scalefactor; }
      if (testchordn == 24 && testnonharmn == 18) { return scalefactor; }
      if (testchordn == 33 && testnonharmn == 27) { return scalefactor; }
      if (testchordn == 34 && testnonharmn == 27) { return scalefactor; }
      if (testchordn == 33 && testnonharmn == 28) { return scalefactor; }
      if (testchordn == 34 && testnonharmn == 28) { return scalefactor; }
      if (testchordn == 35 && testnonharmn == 28) { return scalefactor; }
      if (testchordn == 34 && testnonharmn == 29) { return scalefactor; }
      if (testchordn == 35 && testnonharmn == 29) { return scalefactor; }
      if (testchordn == 36 && testnonharmn == 29) { return scalefactor; }
      if (testchordn == 35 && testnonharmn == 30) { return scalefactor; }
      if (testchordn == 36 && testnonharmn == 30) { return scalefactor; }

      // decending non-harmonic tones
      if (testchordn ==  0 && testnonharmn ==  5)   { return scalefactor; }
      if (testchordn ==  0 && testnonharmn ==  6) { return scalefactor; }
      if (testchordn == 11 && testnonharmn == 16) { return scalefactor; }
      if (testchordn == 11 && testnonharmn == 17) { return scalefactor; }
      if (testchordn == 12 && testnonharmn == 17) { return scalefactor; }
      if (testchordn == 12 && testnonharmn == 18) { return scalefactor; }
      if (testchordn == 22 && testnonharmn == 27) { return scalefactor; }
      if (testchordn == 22 && testnonharmn == 28) { return scalefactor; }
      if (testchordn == 23 && testnonharmn == 28) { return scalefactor; }
      if (testchordn == 23 && testnonharmn == 29) {
         // 6-5 suspensions
         return 0.8 * scalefactor;
      }
      if (testchordn == 24 && testnonharmn == 29)  { return scalefactor; } 
      if (testchordn == 24 && testnonharmn == 30)  { return scalefactor; }

   }

   // check for pedal tone
   if (pedaltone(root, note, absbeat, chordstartbeat, chordendbeat, duration)) {
      return scalefactor;
   }

   if (nonchordtoneQno7th(root, note)) {
      return nonresolutionscaling;
   } 

   return 1.0;
}


//////////////////////////////
//
// pedaltone -- return true if the note is probably a pedal tone.
//

int RootSpectrum::pedaltone(int root, int note, double absbeat, 
      double chordstartbeat, double chordendbeat, double duration) {
   // int rootinterval = ((note-2) - root + 400) % 40;
   // For now, just return true if the note starts before the chord
   // starts and ends after the chord ends.

   if (absbeat < chordstartbeat && (absbeat + duration > chordendbeat)) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// RootSpectrum::durationLinear -- set the duration calculation
//    to be linear.
//

void RootSpectrum::durationLinear(void) {
   durlinear = 1;
}



//////////////////////////////
//
// RootSpectrum::durationLog -- set the duration calculation
//    to be logarithmic.
//

void RootSpectrum::durationLog(void) {
   durlinear = 0;
}



//////////////////////////////
//
// RootSpectrum::meterLinear -- set the metric calculation
//    to be linear.
//

void RootSpectrum::meterLinear(void) {
   metlinear = 1;
}



//////////////////////////////
//
// RootSpectrum::meterLog -- set the metric calculation
//    to be linear.
//

void RootSpectrum::meterLog(void) {
   metlinear = 0;
}



//////////////////////////////
//
// RootSpectrum::rhythmLinear -- set the rhythmic calculations
//    to be linear.
//

void RootSpectrum::rhythmLinear(void) {
   durationLinear();
   meterLinear();
}



//////////////////////////////
//
// RootSpectrum::rhythmLog -- set the rhythmic calculations
//    to be logarithmic.
//

void RootSpectrum::rhythmLog(void) {
   durationLog();
   meterLog();
}



//////////////////////////////
//
// RootSpectrum::setMeterBias -- set the bias for metric log calculations
//

void RootSpectrum::setMeterBias(double aBias) {
   meterBias = aBias;
}



//////////////////////////////
//
// RootSpectrum::setDurationBias -- set the bias for duration log calculations
//

void RootSpectrum::setDurationBias(double aBias) {
   durationBias = aBias;
}



//////////////////////////////
//
// RootSpectrum::getDurationBias -- get the current logarithmic
//     duration bias.
//

double RootSpectrum::getDurationBias(void) {
   return durationBias;
}



//////////////////////////////
//
// RootSpectrum::getMeterBias -- get the current logarithmic
//     meter bias.
//

double RootSpectrum::getMeterBias(void) {
   return meterBias;
}



//////////////////////////////
//
// RootSpectrum::durationscaling -- scaling factor for duration of note.
//

double RootSpectrum::durationscaling(double duration) {
   if (durationQ == 0) {
      return 1.0;
   }
   if (durlinear) {
      return pow(duration, getDurationWeight());
   } else {
      return durationBias + log(duration);
   }
}



//////////////////////////////
//
// RootSpectrum::metricscaling -- scaling factor for metric level of note.
//

double RootSpectrum::metricscaling(double level) {
   if (levelQ == 0) {
      return 1.0;
   }
   if (metlinear) {
      return pow(level, getMetricLevelWeight());
   } else {
      return meterBias + log(level);
   }
}



//////////////////////////////
//
// RootSpectrum::setResolutionFactor -- set the scaling factor for notes
//    which appear to be non-harmonic tones for a given root, but which
//    do not resolve to a chord-tone.
//


void RootSpectrum::setResolutionFactor(double aFactor) {
   nonresolutionscaling = aFactor;   
}



//////////////////////////////
//
// RootSpectrum::getResolutionFactor --
//

double RootSpectrum::getResolutionFactor(void) {
   return nonresolutionscaling;
}




// md5sum: 4fdb177212e4320f83b3e2f5b8bf9a2f RootSpectrum.cpp [20050403]
