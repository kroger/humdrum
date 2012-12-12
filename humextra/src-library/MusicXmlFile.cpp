//
// Copyright 2004 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Mar 10 07:49:40 PST 2004
// Last Modified: Thu Mar 18 23:06:15 PST 2004
// Last Modified: Mon Mar 22 12:49:08 PST 2004 started to add voics processing
// Last Modified: Sun Apr  4 23:17:05 PDT 2004 added _NoteState class
// Last Modified: Tue May  4 18:27:40 PDT 2004 some more articulations
// Last Modified: Wed May  5 22:46:52 PDT 2004 suppressed printing of first
//                                              barline when pickup notes
// Last Modified: Sun May 16 14:25:56 PDT 2004 process multi-staff parts
// Last Modified: Thu Jun 17 01:14:04 PDT 2004 fixed rest alignment in voice 1
// Last Modified: Sun Jun 27 16:57:06 PDT 2004 add lyric translation
// Last Modified: Mon Sep 13 02:45:45 PDT 2004 added basic cresc processing
// Last Modified: Fri Jul 25 16:22:10 PDT 2008 adjusted newlineQ case
// Last Modified: Mon Sep 21 15:02:14 PDT 2009 fixed cut-time identification
// Last Modified: Mon Sep 21 15:50:43 PDT 2009 differentiate q and Q graces
// Last Modified: Mon Sep 21 16:27:49 PDT 2009 removed elided slur ident.
// Last Modified: Sat Jun 26 16:53:21 PDT 2010 added syllabic middle parsing
// Last Modified: Sun Apr  3 08:24:08 PDT 2011 added ficta marks from parens
// Last Modified: Tue Jun 19 14:03:03 PDT 2012 added printing of text
// Last Modified: Wed Jun 20 15:43:34 PDT 2012 various updates/enhancements
//
// Filename:      ...sig/include/sigInfo/MusicXmlFile.h
// Web Address:   http://sig.sapp.org/include/sigInfo/MusicXmlFile.h
// Syntax:        C++ 
//
// Description:   A class which parses a MusicXML File.
//
// Todo:          
//                implicit/explicit accidental marks indications
//                logical repeat marks
//                first, second endings 
//                preserve hard-coded system breaks 
//                some more articulations 
//                general solution to multiple voices in a part.
//                generalize **kern data extraction for more than two voices
//                figure out how beaming is indicated with chords.
//                fix dynamic spine syntax when dynamic comes between notes
//                fix assemble code so that grace notes are aligned correctly
//
// Done:	  
//                lyrics (convert to spines to right of music spine)
//                fixed stacattissimo
//                explicit natural sign
//                don't print initial barline when pickup (use fixMeasure)
//                dynamics (only fff, ff, f, mf, mp, p, pp, ppp)
//                diminuendo, crescendo (only start location processed)
//                words dim. cresc. decresc. --> diminuendo and crescendo
//                interpreted rests in lower voice numbers as padding
//                articulation: staccato, accent, fermata
//                technical: up-bow, down-bow
//                grace notes (partially; no grace chords)
//                backup element
//                forward element
//                voices (up to two voices per part -- almost generalized)
//                clefs (check unusual clefs)
//                key signatures
//                monophonic lines (complete)
//                measure (mostly)
//                chords (probably all)
//                ties (all)
//                stem directions (all)
//                beams (all)
//                barlines (repeat indicators)
//                slurs (check elided however -- better to just fix manually)
//

#include "MusicXmlFile.h"
#include "Convert.h"
#include "PerlRegularExpression.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>

using namespace CSL::XML;

/* For debugging partstaves and partoffset... */
void printArray(const char* title, Array<int>& partoffset) {
   cout << title << "\t";
   int i;
   for (i=0; i<partoffset.getSize(); i++) {
      cout << partoffset[i] << " ";
   }
   cout << endl;
}



//////////////////////////////
//
// MusicXmlFile::MusicXmlFile --
//

MusicXmlFile::MusicXmlFile(void) { 
   xmldocument = NULL;
   parts.setSize(100);
   parts.setGrowth(100);
   parts.setSize(0);
   partstaves.setSize(100);
   partstaves.setGrowth(100);
   partstaves.setSize(0);
   partoffset.setSize(100);
   partoffset.setGrowth(100);
   partoffset.setSize(0);
   partdata.setSize(100);
   partdata.setGrowth(100);
   partdata.setSize(0);
   partdynamics.setSize(100);
   partdynamics.setGrowth(100);
   partdynamics.setSize(0);
   parsedivisions = 4;
   parseserialnum = 1;
   partvoiceticktime1 = -1;
   partvoiceticktime2 = -1;
   setOption("stem",    1);
   setOption("beam",    1);
   setOption("dynamic", 1);
   setOption("lyric",   1);
   setOption("renumber",1);
   lastchordhead = NULL;
}


MusicXmlFile::MusicXmlFile(char* aFile) { 
   xmldocument = NULL;
   parts.setSize(100);
   parts.setGrowth(100);
   parts.setSize(0);
   partstaves.setSize(100);
   partstaves.setGrowth(100);
   partstaves.setSize(0);
   partoffset.setSize(100);
   partoffset.setGrowth(100);
   partoffset.setSize(0);
   partdata.setSize(100);
   partdata.setGrowth(100);
   partdata.setSize(0);
   partdynamics.setSize(100);
   partdynamics.setGrowth(100);
   partdynamics.setSize(0);
   parsedivisions = 4;
   parseserialnum = 1;
   partvoiceticktime1 = -1;
   partvoiceticktime2 = -1;
   setOption("stem",    1);
   setOption("beam",    1);
   setOption("dynamic", 1);
   setOption("lyric",   1);
   setOption("renumber",1);
   lastchordhead = NULL;

   read(aFile);   
}



//////////////////////////////
//
// MusicXmlFile::~MusicXmlFile --
//

MusicXmlFile::~MusicXmlFile() { 
   // do nothing
}



//////////////////////////////
//
// MusicXmlFile::getPartCount -- returns the number of parts
//     in the MusicXmlFile.
//

int MusicXmlFile::getPartCount(void) {
   return parts.getSize();
}



//////////////////////////////
//
// MusicXmlFile::getStaffCount -- returns the number of staves
//     in the MusicXmlFile. (Parts can have more than one staff).
//

int MusicXmlFile::getStaffCount(void) {
   return partdata.getSize();
}



//////////////////////////////
//
// MusicXmlFile::getGraceNoteRhythm(ostream& out, 
//

ostream& MusicXmlFile::printGraceNoteRhythm(ostream& out, _MusicXmlItem& item) {
   CXMLObject*        entry   = item.obj;
   CXMLObject*        current = NULL;
   CXMLElement*       element = NULL;
   CXMLCharacterData* cdata   = NULL;
   XMLString string;
   char buffer[1024] = {0};

   // read the elements in the object and search for <type></type> data
   entry = entry->Zoom();
   while (entry != NULL) {
      if (entry->GetType() == xmlElement) {
         element = (CXMLElement*)entry;
         // cout << "ELEMENT NAME: " << element->GetName() << endl;
         if (element->GetName() == "type") {
            current = element;
            current = current->Zoom();
            while (current != NULL) {
               if (current->GetType() == xmlCharacterData) {
                  cdata = (CXMLCharacterData*)current;
                  string = cdata->GetData();
                  strncpy(buffer, string.c_str(), 512);
                  removeBorderSpaces(buffer);
                  if (strcmp(buffer, "128th") == 0) {
                     out << "128";
                     return out;
                  } else if (strcmp(buffer, "64nd") == 0) {
                     out << "64";
                     return out;
                  } else if (strcmp(buffer, "32nd") == 0) {
                     out << "32";
                     return out;
                  } else if (strcmp(buffer, "16th") == 0) {
                     out << "16";
                     return out;
                  } else if (strcmp(buffer, "8th") == 0) {
                     out << "8";
                     return out;
                  } else if (strcmp(buffer, "quarter") == 0) {
                     out << "4";
                     return out;
                  } else if (strcmp(buffer, "half") == 0) {
                     out << "2";
                     return out;
                  } else if (strcmp(buffer, "whole") == 0) {
                     out << "1";
                     return out;
                  } else {
                     return out;
                  }
               }
               current = current->GetNext();
	    }
         } 
      }
      entry = entry->GetNext();
   }
   return out;
}



//////////////////////////////
//
// printGraceMarker -- print "q" if the grace note requires a slash.
//     print "Q" if the grace note does not require a slash.
//

ostream& MusicXmlFile::printGraceMarker(ostream& out, _MusicXmlItem& item) {
   CXMLObject*   entry   = item.obj;
   CXMLElement*  element = NULL;

   entry = entry->Zoom();
   while (entry != NULL) {
      if (entry->GetType() == xmlElement) {
         element = (CXMLElement*)entry;
         for (int i=0; i<element->GetAttributes().GetLength(); i++) {
            if (element->GetAttributes().GetName(i) == "slash") {
               if (element->GetAttributes().GetValue(i) == "yes") {
                  out << "q";
                  return out;
               }
               break;
            }
         }
      }
      entry = entry->GetNext();
   }

   out << "Q";
   return out;
}



//////////////////////////////
//
// MusicXmlFile::getPartStaffCount -- return the number of staves
//   which a part contains.
//

int MusicXmlFile::getPartStaffCount(int index) {
   if (index <0) {
      return 0;
   }
   if (index >= partstaves.getSize()) {
      return 0;
   }
   return partstaves[index];
}



///////////////////////////////
//
// MusicXmlFile::info -- debugging information.
//

ostream& MusicXmlFile::info(ostream& out) {
   out << "There ";
   if (getPartCount() == 1) {
      out << "is ";
   } else {
      out << "are ";
   }
   out << getPartCount() 
       << " part";
   if (getPartCount() != 1) {
      out << "s ";
   } 
   out << " in the MusicXml File:" << endl;
   int i;
   char buffer[1024] = {0};
   for (i=0; i<getPartCount(); i++) {
      cout << "\tPart " << i + 1 << ":\tid=" << getPartId(buffer, i);
      cout << "\tname=" << getPartName(buffer, i);
      cout << "\tstaves=" << getPartStaffCount(i) << endl;
   }

   cout << "PART DATA ================================" << endl;
   int j;
   for (i=0; i<partdata.getSize(); i++) {
      cout << "\nStaff " << i+1 << " ---------------------------------" << endl;
      for (j=0; j<partdata[i].getSize(); j++) {
         cout << "i=" << i << "\tj=" << j 
              << "\ttick=" << partdata[i][j].ticktime  
              << "\ttdur=" << partdata[i][j].tickdur  
              << "\tvoice=" << partdata[i][j].voice  
              << "\tserial=" << partdata[i][j].serialnum  
              << "\tname=" << ((CXMLElement*)(partdata[i][j].obj))->GetName();
         if (((CXMLElement*)(partdata[i][j].obj))->GetName() == "measure") {
            cout << " (" << partdata[i][j].measureno << ")";
         } else if (((CXMLElement*)(partdata[i][j].obj))->GetName() == "note") {
            cout << " (";
            if (partdata[i][j].pitch > 0) {
               cout << Convert::base40ToKern(buffer, partdata[i][j].pitch);
            } else {
               cout << "r";
            }
            cout << ")";
         } else if (((CXMLElement*)(partdata[i][j].obj))->GetName() == "lyric") {
            cout << " (" << partdata[i][j].lyricnum << ":"
                 << partdata[i][j].ownerserial << ")";
         }
         cout << endl;
      }
   }
   cout << "==========================================" << endl;

   return out;
}



//////////////////////////////
//
// MusicXmlFile::getPartName --
//

char* MusicXmlFile::getPartName(char* buffer, int partnum) {
   buffer[0] = '\0';

   if (partnum < 0 || partnum > getPartCount()) {
      return buffer;
   }

   XMLString string;
   CXMLElement* element = NULL;
   CXMLCharacterData* cdata = NULL;
   CXMLObject*  object  = parts[partnum];
   object = object->Zoom();

   do {
      if (object->GetType() == xmlElement) {
         element = (CXMLElement*)object;
         if (element->GetName() == "part-name") {
            object = object->Zoom();
            if (object == NULL) {
               break;
            }
            if (object->GetType() == xmlCharacterData) {
               cdata = (CXMLCharacterData*)object;
               string = cdata->GetData();
               strncpy(buffer, string.c_str(), 512);
               removeBorderSpaces(buffer);
               break;
            } else {
               strcpy(buffer, "Not here");
               break;
            }
         }
      }
      object = object->GetNext();
   } while (object != NULL);

   return buffer;
}



//////////////////////////////
//
// MusicXmlFile::removeBorderSpaces -- remove leading and following spaces
//     from the data.
//

void MusicXmlFile::removeBorderSpaces(char* buffer) {
   int length = strlen(buffer);
   int i;
   for (i=length-1; i>=0; i--) {
      if (isspace(buffer[i])) {
         buffer[i] = '\0';
      } else {
         break;
      }
   }

   int count = 0;
   for (i=0; i<length; i++) {
      if (isspace(buffer[i])) {
         count++;
      } else {
         break;
      }
   }

   if (count == 0) {
      return;
   }

   for (i=0; i<length-count; i++) {
      buffer[i] = buffer[i+count]; 
   }
}



//////////////////////////////
//
// MusicXmlFile::getPartId --
//

char* MusicXmlFile::getPartId(char* buffer, int partnum) {
   buffer[0] = '\0';
   int i;
   if (partnum < 0 || partnum > getPartCount()) {
      return buffer;
   }

   CXMLElement* element = (CXMLElement*)parts[partnum];
   XMLString string;
   for (i=0; i<element->GetAttributes().GetLength(); i++) {
      if (element->GetAttributes().GetName(i) == "id") {
         string = element->GetAttributes().GetValue(i); 
         strncpy(buffer, string.c_str(), 1024);
         break;
      }
   }

   return buffer;
}


//////////////////////////////
//
// MusicXmlFile::print --
//

ostream& MusicXmlFile::print(ostream& out) { 
   CXMLObject* entry = xmldocument;
   printTraverse(out, entry);
   out << "\n";
   return out;
}



//////////////////////////////
//
// MusicXmlFile::printTraverse --
//

ostream& MusicXmlFile::printTraverse(ostream& out, CXMLObject* entry) { 
   while (entry != NULL) {
      printEntry(out, entry);
      entry = entry->GetNext();
   }
   return out;
}



//////////////////////////////
//
// MusicXmlFile::printCloseEvent --
//

ostream& MusicXmlFile::printCloseEvent(ostream& out, 
      CXMLObject* entry) {
   CXMLElement* element = (CXMLElement*)entry;
   if (element->Zoom() != NULL) {
      out << "</"
          << element->GetName()
          << ">";
   }
//   out << "\n";
   return out;
}



//////////////////////////////
//
// MusicXmlFile::printEntry --
//

ostream& MusicXmlFile::printEntry(ostream& out, CXMLObject* entry) {
   switch (entry->GetType()) {
      case xmlObject:
         break;
      case xmlContainer:
         break;
      case xmlAttribute:
         break;
      case xmlElement:
         // out << "ENTRY: xmlElement" << endl;
         printElement(out, entry);
         break;
      case xmlCharacterData:
         // out << "ENTRY: xmlCharacterData" << endl;
         printCharacterData(out, entry);
         break;
      case xmlCDATASection:
         // out << "ENTRY: xmlCDATASection" << endl;
         printCDataSection(out, entry);
         break;
      case xmlComment:
         // out << "ENTRY: xmlComment" << endl;
         printComment(out, entry);
         break;
      case xmlGenericData:
         // out << "ENTRY: xmlGenericData" << endl;
         printGenericData(out, entry);
         break;
      case xmlProcessingInstruction:
         out << "ENTRY: xmlProcessingInstruction" << endl;
         break;
      case xmlExternalEntity:
         out << "ENTRY: xmlExternalEntity" << endl;
         break;
      case xmlNotationDeclaration:
         out << "ENTRY: xmlNotationDeclaration" << endl;
         break;
      case xmlUnparsedEntityDeclaration:
         out << "ENTRY: xmlUnparsedEntityDeclaration" << endl;
         break;
      case xmlDocument:
         // out << "ENTRY: xmlDocument" << endl;
         printDocument(out, entry);
         break;
      default:
         out << "Unknown entry" << endl;
   }

   printTraverse(out, entry->Zoom());

   if (entry->GetType() == xmlElement) {
      printCloseEvent(out, entry);
   }

   return out;
}


//////////////////////////////
//
// MusicXmlFile::printDocument --
//

ostream& MusicXmlFile::printDocument(ostream& out, CXMLObject* entry) {
   CXMLDocument* document = (CXMLDocument*)entry;

   out << "DOCUMENT NAME = " << document->GetName() << endl;
   out << "DOCUMENT BASE = " << document->GetBase() << endl;

   return out;
}



//////////////////////////////
//
// MusicXmlFile::printCDataSection --
//

ostream& MusicXmlFile::printCDataSection(ostream& out, 
      CXMLObject* entry) {
   CXMLCDATASection* cdata = (CXMLCDATASection*) entry;
   printString(out, cdata);
   return out;
}



//////////////////////////////
//
// printGenericData --
//

ostream& MusicXmlFile::printGenericData(ostream& out, 
      CXMLObject* entry) {
   CXMLGenericData* generic = (CXMLGenericData*)entry;
   printString(out, generic);
   return out;
}



//////////////////////////////
//
// MusicXmlFile::printComment --
//

ostream& MusicXmlFile::printComment(ostream& out, CXMLObject* entry) {
   CXMLComment* comment = (CXMLComment*)entry;
   out << "<!--";
   printString(out, comment);
   out << "-->";
   out << "\n";   // need to print a newline for some reason
   return out;
}


//////////////////////////////
//
// MusicXmlFile::printString --
//

ostream& MusicXmlFile::printString(ostream& out, 
      CXMLCharacterData* characters) {
   unsigned int i;
   XMLString string = characters->GetData();
   for (i=0; i<string.length(); i++) {
      out << string[i];
   }
   return out;
}



//////////////////////////////
//
// MusicXmlFile::printElement --
//

ostream& MusicXmlFile::printElement(ostream& out, CXMLObject* entry) {
   CXMLElement* element = (CXMLElement*)entry;
   out << "<" << element->GetName();
   int i;
   for (i=0; i<element->GetAttributes().GetLength(); i++) {
      out << " "
          << element->GetAttributes().GetName(i) 
          << "=\""
          << element->GetAttributes().GetValue(i)
          << "\"";
   }
   if (element->Zoom() == NULL) {
      out << "/";
   }
   out << ">";
//   out << "\n";

   return out;
}



//////////////////////////////
//
// MusicXmlFile::printCharacterData --
//

ostream& MusicXmlFile::printCharacterData(ostream& out, 
         CXMLObject* entry) {
   CXMLCharacterData* data = (CXMLCharacterData*)entry;
//   out << "(((";
   printString(out, data);
//   out << ")))" << endl;

   return out;
}



//////////////////////////////
//
// MusicXmlFile::read --
//

void MusicXmlFile::read(char* aFile) { 
   CXMLFactory         factory;
   CXMLDocumentBuilder builder;

   xmldocument = factory.CreateDocument("MusicXML Document");
   builder.BuildDocument(aFile, xmldocument, &factory);

   parse();

   int i;
   for (i=0; i<getStaffCount(); i++) {
      sortStaff(i);
   }

   int j;
   for (i=0; i<partdynamics.getSize(); i++) {
      partdynamics[i] = 0;
      for (j=0; j<partdata[i].getSize(); j++) {
         if (partdata[i][j].type == MXI_dynamic) {
            partdynamics[i] = 1;
            break;
         }
      }
   }
}



//////////////////////////////
//
// operator<< -- directions for printing a MusicXmlFile object.
//

ostream& operator<<(ostream& out, MusicXmlFile& aMusicXmlFile) { 
   aMusicXmlFile.print(out);
   return out;
}



//////////////////////////////
//
// MusicXmlFile::parse --
//

void MusicXmlFile::parse(void) {
   if (xmldocument == NULL) {
      cout << "XMLDocument is null" << endl;
      return;
   }

   parseserialnum = 1;
   lastchordhead  = NULL;

   traverse(xmldocument);
   fixPickupBarline();
}



//////////////////////////////
//
// MusicXmlFile::fixMeasureNumbers -- generate a reasonable
//    measure number count by not counting non-controlling
//    barlines.
//

void MusicXmlFile::fixMeasureNumbers(void) {
   int i;
   for (i=0; i<partdata.getSize(); i++) {
      fixMeasureNumbersForPart(i);
   }
}



//////////////////////////////
//
// MusicXmlFile::fixMeasureNumbersForPart --
//

void MusicXmlFile::fixMeasureNumbersForPart(int staffno) {
   int i;
   Array<int> measures;
   measures.setSize(10000);
   measures.setGrowth(10000);
   measures.setSize(0);

   Array<int> timeticks;
   timeticks.setSize(10000);
   timeticks.setGrowth(10000);
   timeticks.setSize(0);
   int mticks = 0;
   int tickinfo = 0;

   if (partdata[staffno].getSize() <= 0) {
      return;  // nothing to do...
   }
   for (i=0; i<partdata[staffno].getSize(); i++) {
      if (partdata[staffno][i].type == MXI_divisions) {
         tickinfo = partdata[staffno][i].divisions;
      } else if (partdata[staffno][i].type == MXI_measure) {   
         measures.append(i);
         timeticks.append(mticks);
      } else if (partdata[staffno][i].type == MXI_time) {   
         mticks = getTimeSignatureTicks(partdata[staffno][i].obj, tickinfo);
      }
   }

   // The timeticks are off by one, so readjust them
   for (i=0; i<timeticks.getSize()-1; i++) {
      timeticks[i] = timeticks[i+1];
   }
   timeticks[timeticks.getSize()-1] = mticks;

   // now mark all barlines which are between two incomplete
   // measure which sum up to the correct size according to
   // the meter.

   Array<int> partial;
   partial.setSize(measures.getSize());
   partial.setAll(0);

   for (i=0; i<measures.getSize(); i++) {
      if (partdata[staffno][measures[i]].tickdur != timeticks[i]) {
         partial[i] = 1;
      }
   }

   for (i=1; i<partial.getSize(); i++) {
      if (partial[i] && partial[i-1]) {

         if ((partdata[staffno][measures[i]].tickdur + 
              partdata[staffno][measures[i-1]].tickdur) == timeticks[i-1]) {
            // current measure is a non-controlling barline so mark it
            partdata[staffno][measures[i]].measureno = 0;
         }
      }
   }

   // now re-adjust the measure numbers in the partdata:
   int measureno = 0;
   for (i=0; i<measures.getSize(); i++) {
      if (partdata[staffno][measures[i]].measureno > 0) {
         measureno++;
         partdata[staffno][measures[i]].measureno = measureno;
      }
   }

}



//////////////////////////////
//
// MusicXmlFile::fixPickupBarline --  remove the first barline if the measure
//     is a pickup measure.
//

void MusicXmlFile::fixPickupBarline(void) {
   int i;
   for (i=0; i<partdata.getSize(); i++) {
      fixPartPickups(i);
   }
}



//////////////////////////////
//
// MusicXmlFile::fixPartPickups --
//

void MusicXmlFile::fixPartPickups(int staffno) {
   int i = 0;
   int meter = -1;
   int metermeasure = 0;
   int firstmeasure = -1;
   int firstnote = -1;
   int tickinfo = 0;
   while (i < partdata[staffno].getSize()) {
      if (partdata[staffno][i].type == MXI_divisions) {
         tickinfo = partdata[staffno][i].divisions;
      } else if (partdata[staffno][i].type == MXI_time) {
         meter = i;
         break;
      } else if (partdata[staffno][i].type == MXI_measure) {
         metermeasure++;
         if (firstmeasure < 0) {
            firstmeasure = i;
         }
      } else if (partdata[staffno][i].type == MXI_note) {
         if (firstnote < 0) {
            firstnote = i;
         }
      }
         
      i++;
   }

   if ((firstnote >= 0) && (firstnote < firstmeasure)) {
      // nothing to fix
      return;
   }

   if (metermeasure != 1) {
      // unusual data, cannot attempt a fix
      return;
   }

   if (tickinfo == 0) {
      // not enough timing information to make a correction
      return;
   }

   // check to see if the duration of the first measure
   // is equivalent to the time signature.  If not, then
   // the first measure is a pickup measure.

   int measureticks = 0;
   int timeticks = 0;
   measureticks = getMeasureTickDuration(partdata[staffno][firstmeasure].obj);
   timeticks = getTimeSignatureTicks(partdata[staffno][meter].obj, tickinfo);

   if (timeticks == measureticks) {
      // the measure contains the expected number of beats; do nothing
      return;
   } else if (timeticks < measureticks) {
      // something weird is happening in the measure; do nothing
      return;
   }

   // the measure does not have the full amount of beats, so
   // mark it as a measure which should not be printed in **kern data.
   partdata[staffno][firstmeasure].measureno = -1;

}



//////////////////////////////
//
// MusicXmlFile::getTimeSignatureTicks -- return the expected number
//     of ticks in a <measure> timespan.
//

int MusicXmlFile::getTimeSignatureTicks(CXMLObject* obj, int tickinfo) {
   int numerator = 0;
   double denominator = 0.0;
   char timebuffer[1024] = {0};
   getKernTimeSig(timebuffer, obj);
   numerator = (int)Convert::kernTimeSignatureTop(timebuffer);
   denominator = Convert::kernTimeSignatureBottomToDuration(timebuffer);
   return (int)(numerator * denominator * tickinfo);
}



//////////////////////////////
//
// MusicXmlFile::traverse --
//

void MusicXmlFile::traverse(CXMLObject* entry) {
   while (entry != NULL) {
      lastchordhead = NULL;
      process(entry);
      entry = entry->GetNext();
   }
}


//////////////////////////////
//
// MusicXmlFile::process --
//

void MusicXmlFile::process(CXMLObject* entry) {
   int zoomQ = 1;
   switch (entry->GetType()) {
      case xmlObject:
         break;
      case xmlContainer:
         break;
      case xmlAttribute:
         break;
      case xmlElement:
         zoomQ = parseElement(entry);
         break;
      case xmlCharacterData:
         break;
      case xmlCDATASection:
         break;
      case xmlComment:
         break;
      case xmlGenericData:
         break;
      case xmlProcessingInstruction:
         break;
      case xmlExternalEntity:
         break;
      case xmlNotationDeclaration:
         break;
      case xmlUnparsedEntityDeclaration:
         break;
      case xmlDocument:
         break;
   }

   if (zoomQ) {
      traverse(entry->Zoom());
   }
}



//////////////////////////////
//
// MusicXmlFile::addStaffData --
//

void MusicXmlFile::addStaffData(void) {
      partdata.setSize(partdata.getSize()+1);
      partdynamics.setSize(partdynamics.getSize()+1);
      partdynamics[partdynamics.getSize()-1] = 0;
      partdata[partdata.getSize()-1].setSize(10000);
      partdata[partdata.getSize()-1].setGrowth(10000);
      partdata[partdata.getSize()-1].setSize(0);
}



//////////////////////////////
//
// MusicXmlFile::getStaffCountFromPart --
//

int MusicXmlFile::getStaffCountFromPart(CXMLObject* object) {
   int output = 1;

   if (object == NULL) {
      return 0;
   }
   if (object->GetType() != xmlElement) {
      return 0;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "part") {
      return 0;
   }

   object = object->Zoom();
   while (object != NULL) {
      if (object->GetType() != xmlElement) {
         object = object->GetNext();
         continue;
      }
      element = (CXMLElement*)object;
      if (element->GetName() != "measure") {
         object = object->GetNext();
         continue;
      }

      // found the first measure, so start reading it for attributes
      object = object->Zoom();
      while (object != NULL) {
         if (object->GetType() != xmlElement) {
            object = object->GetNext();
            continue;
         }
         element = (CXMLElement*)object;
         if (element->GetName() != "attributes") {
            object = object->GetNext();
            continue;
         }
         // found the attributes for the first measure, look for <staves>
         object = object->Zoom();
         while (object != NULL) {
            if (object->GetType() != xmlElement) {
               object = object->GetNext();
               continue;
            }
            element = (CXMLElement*)object;
            if (element->GetName() != "staves") {
               object = object->GetNext();
               continue;
            }
            // found the staves object, get the number from it.
            output = getIntFromCharacterData(element->Zoom());
            break;
         }
         break;
      }
      break;
   }

   return output;
}




//////////////////////////////
//
// parseElement --
//

int MusicXmlFile::parseElement(CXMLObject* entry) {
   CXMLElement* element = (CXMLElement*)entry;
   char buffer[1024] = {0};
   int i;
   int ii;
   XMLString idname;
   int partnum = -1;
   int tval = 1;
   int staves = 0;
   int static partcount = 0;
   int jj;

   if (element->GetName() == "score-part") {
      parts.append(entry);
      tval = 1;
      partstaves.append(tval);

      if (partstaves.getSize() <= 1) {
         tval = partstaves.getSize()-1;
         partoffset.append(tval);
      } else {
         tval = partstaves[partstaves.getSize()-1-1] + 
                partoffset[partoffset.getSize()-1];
         partoffset.append(tval);
      }
      addStaffData();
   } else if (element->GetName() == "part") {
      staves = getStaffCountFromPart(element);
      if (staves > 1) {
         for (ii=0; ii<staves-1; ii++) {
            addStaffData();
            partstaves[partcount]++;
            for (jj=partcount+1; jj<partoffset.getSize(); jj++) {
               partoffset[jj]++;      
            }
            // printArray("partstaves contents: ", partstaves);
            // printArray("partoffset contents: ", partoffset);

         }
      } 
      partcount++;

      for (i=0; i<element->GetAttributes().GetLength(); i++) {
         if (element->GetAttributes().GetName(i) == "id") {
            idname = element->GetAttributes().GetValue(i);
            strncpy(buffer, idname.c_str(), 512);
            partnum = getPartNumberFromId(buffer);
            if (partnum < 0) {
               return 1;
            }
            getInstrumentName(element, partnum, idname);
            parsePart(element->Zoom(), partnum);
            return 0;
            break;
         }

      }
   }

   return 1;
}



//////////////////////////////
//
// MusicXmlFile::getPartNumber -- return the part number for the given
//   ID.  Returns -1 if the part cannot be found.
//

int MusicXmlFile::getPartNumberFromId(const char* buffer) {
   int i;
   char buff2[1024] = {0};

   for (i=0; i<getPartCount(); i++) {
      if (strcmp(buffer, getPartId(buff2, i)) == 0) {
         return i;
      }
   }

   return -1;
}



//////////////////////////////
//
// MusicXmlFile::getInstrumentName --
//

void MusicXmlFile::getInstrumentName(CXMLObject* object, int partnum, 
      XMLString& idname) {

   CXMLElement* scorepartwise = (CXMLElement*)object->GetParent();
   if (scorepartwise->GetName() != "score-partwise") {
      cerr << "Expecting <score-partwise> but found " 
           << scorepartwise->GetName() << " instead." << endl;
      return;
   }
 
   // go through children until <part-list> is found, then <score-part>
   int foundQ = 0;
   object = (CXMLObject*)scorepartwise;
   object = object->Zoom();
   while (object != NULL) {  // part-list search
      if (object->GetType() == xmlElement) {
         if (((CXMLElement*)object)->GetName() == "part-list") {
            object = object->Zoom();
            while (object != NULL) {  // <score-part> search
               if (object->GetType() != xmlElement) { 
                  object = object->GetNext();
                  continue; 
               }
               if (((CXMLElement*)object)->GetName() != "score-part") {
                  object = object->GetNext();
                  continue;
               }
               if (idname != getAttributeValue(object, "id")) {
                  object = object->GetNext();
                  continue;
               }
               foundQ = 1;
               break;
            }
            goto endofloop;
         }
      }
      object = object->GetNext();
   }
   endofloop:

   if (!foundQ) {
      return;
   }

   _MusicXmlItem tempitem;

   tempitem.type      = MXI_instrument;
   tempitem.obj       = object;
   tempitem.ticktime  = 0;
   tempitem.tickdur   = 0;
   tempitem.voice     = 0;
   tempitem.pitch     = 0;
   tempitem.ficta     = 0;
   tempitem.divisions = 0;
   partdata[partoffset[partnum]+0].append(tempitem);
}



//////////////////////////////
//
// MusicXmlFile::parsePart(CXMLObject* entry, int partnum) {
//

void MusicXmlFile::parsePart(CXMLObject* entry, int partnum) {
   // cout << "Parsing of Part " << partnum + 1 << endl;
   CXMLObject* current = entry;
   CXMLElement* element;
   long ticktime = 0;
   long oldticktime = 0;
   while (current != NULL) {
      if (current->GetType() == xmlElement) {
         element = (CXMLElement*)current;
         if (element->GetName() == "measure") {
            oldticktime = ticktime;
            parseMeasure(element, partnum, ticktime);
            ticktime = oldticktime + getMeasureTickDuration(element);
         }
         // cout << "Part " << partnum + 1 << " Element name = "
         //      << element->GetName() << endl;
      }
      current = current->GetNext();
   }
}



//////////////////////////////
//
// MusicXmlFile::parseMeasure -- read in one measure
//

void MusicXmlFile::parseMeasure(CXMLObject* entry, int partnum, 
      long& ticktime) {

   if (entry == NULL) {
      return;
   }
   if (entry->GetType() != xmlElement) {
      return;
   }
   CXMLElement* element = (CXMLElement*)entry;
   if (element->GetName() != "measure") {
      return;
   }
   
   // insert a barline into data for part
   _MusicXmlItem tempitem;
   tempitem.ticktime = ticktime;
   tempitem.tickdur  = getMeasureTickDuration(entry);
   tempitem.type     = MXI_measure;
   tempitem.obj      = entry;
   tempitem.measureno= getMeasureNumber(entry);
   tempitem.serialnum= parseserialnum++;
   tempitem.divisions = parsedivisions;
   appendAllPartStaves(partnum, tempitem);

   // read the elements in the measure and store them for the part
   entry = entry->Zoom();
   while (entry != NULL) {
      if (entry->GetType() == xmlElement) {
         element = (CXMLElement*)entry;
         // cout << "MEASURE ITEM: " << element->GetName() << endl;
         if (element->GetName() == "note") {
            parseNote(element, partnum, ticktime);
         } else if (element->GetName() == "attributes") {
            parseAttributes(element, partnum, ticktime);
         } else if (element->GetName() == "direction") {
            parseDirection(element, partnum, ticktime);
         } else if (element->GetName() == "barline") {
            parseBarline(element, partnum, ticktime);
         } else if (element->GetName() == "print") {
            parsePrint(element, partnum, ticktime);
         } else if (element->GetName() == "backup") {
            parseBackup(element, partnum, ticktime);
         } else if (element->GetName() == "forward") {
            parseForward(element, partnum, ticktime);
         }
      }
      entry = entry->GetNext();
   }
}



//////////////////////////////
//
// MusicXmlFile::parseDirection -- element location for sound (tempo) and 
//   written dynamics.
//

void MusicXmlFile::parseDirection(CXMLObject* object, int partnum, 
      long& ticktime) {
   if (object == NULL) {
      return;
   }
   if (object->GetType() != xmlElement) {
      return;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "direction") {
      return;
   }

   // check for text placement:
   //  <direction placement="above"> 
   CXMLElement* rootelement = element;

   object = object->Zoom();
   while (object != NULL) {
      if (object->GetType() != xmlElement) {
         object = object->GetNext();
         continue;
      }
      element = (CXMLElement*)object;
      if (element->GetName() == "sound") {
         parseSound(element, partnum, ticktime);
      } else if (element->GetName() == "direction-type") {
         parseDirectionType(element, partnum, ticktime, rootelement);
      }
      object = object->GetNext();
   }

}



//////////////////////////////
//
// MusicXmlFile::parseDirectionType -- extract dynamics from direction type
//

void MusicXmlFile::parseDirectionType(CXMLObject* object, int partnum, 
      long& ticktime, CXMLElement* elementroot) {
   if (object == NULL) {
      return;
   }
   if (object->GetType() != xmlElement) {
      return;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "direction-type") {
      return;
   }

   int staff = getStaffNumber(object) - 1;
   checkPartStaff(partnum, staff+1);

   object = object->Zoom();
   CXMLObject* obj2;
   CXMLElement* elem2;
   _MusicXmlItem tempitem;
   while (object != NULL) {
      if (object->GetType() != xmlElement) {
         object = object->GetNext();
         continue;
      }
      element = (CXMLElement*)object;

      if (element->GetName() == "dynamics") {
         obj2 = element->Zoom();
         while (obj2 != NULL) {
            if (obj2->GetType() != xmlElement) {
               obj2 = obj2->GetNext();
               continue;
            }
            elem2 = (CXMLElement*)obj2;
            if ((elem2->GetName() == "fff") ||
                (elem2->GetName() == "ff") ||
                (elem2->GetName() == "f") ||
                (elem2->GetName() == "mf") ||
                (elem2->GetName() == "mp") ||
                (elem2->GetName() == "p") ||
                (elem2->GetName() == "pp") ||
                (elem2->GetName() == "ppp") ) {
               tempitem.type     = MXI_dynamic;
               tempitem.obj      = element;
               tempitem.ticktime = ticktime;
               tempitem.serialnum= parseserialnum++;
               tempitem.divisions = parsedivisions;
               partdata[partoffset[partnum]+staff].append(tempitem);
               break;
            }
            obj2 = obj2->GetNext();
         }

      } else if (element->GetName() == "wedge") {
         int i;
         for (i=0; i<element->GetAttributes().GetLength(); i++) {
            if (element->GetAttributes().GetName(i) == "type") {
               if (element->GetAttributes().GetValue(i) == "diminuendo") {
                  tempitem.type      = MXI_dynamic;
                  tempitem.obj       = element;
                  tempitem.ticktime  = ticktime;
                  tempitem.serialnum = parseserialnum++;
                  tempitem.divisions = parsedivisions;
                  partdata[partoffset[partnum]+staff].append(tempitem);
               } else if (element->GetAttributes().GetValue(i) == "crescendo") {
                  tempitem.type      = MXI_dynamic;
                  tempitem.obj       = element;
                  tempitem.ticktime  = ticktime;
                  tempitem.serialnum = parseserialnum++;
                  tempitem.divisions = parsedivisions;
                  partdata[partoffset[partnum]+staff].append(tempitem);
               }

            }
         }

      } else if (element->GetName() == "words") {
         obj2 = element->Zoom();
         XMLString string;
         CXMLCharacterData* textdata;
         while (obj2 != NULL) {
            if (obj2->GetType() == xmlCharacterData) {
               textdata = (CXMLCharacterData*)obj2;
               string = textdata->GetData();
               if ((string == "cresc.") || (string == "decresc.") 
                     || (string == "dim.")) {
                  tempitem.type      = MXI_dynamic;
                  tempitem.obj       = element;
                  tempitem.ticktime  = ticktime;
                  tempitem.serialnum = parseserialnum++;
                  tempitem.divisions = parsedivisions;
                  partdata[partoffset[partnum]+staff].append(tempitem);
                  break;  // need or should break?
               } else {
                  textdata = (CXMLCharacterData*)obj2;
                  string = textdata->GetData();
                  tempitem.type = MXI_text;
                  tempitem.obj = element;
                  tempitem.ticktime = ticktime;
                  tempitem.serialnum = parseserialnum++;
                  tempitem.divisions = parsedivisions;
                  partdata[partoffset[partnum]+staff].append(tempitem);
                  break;  // need or should break?
               }
            }
            obj2 = obj2->GetNext();
         }
      }

      object = object->GetNext();
   }

}



//////////////////////////////
//
// MusicXmlFile::parseForward --
//

void MusicXmlFile::parseForward(CXMLObject* object, 
      int partnum, long& ticktime) {
   if (object == NULL) {
      return;
   }
   if (object->GetType() != xmlElement) {
      return;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "forward") {
      return;
   }

   long duration = 0;
   object = object->Zoom();
   while (object != NULL) {
      if (object->GetType() == xmlElement) {
         element = (CXMLElement*)object;
         if (element->GetName() == "duration") {
            duration = getIntFromCharacterData(element->Zoom());
            break;
         }
      }
      object = object->GetNext();
   }

   ticktime += duration;
}



//////////////////////////////
//
// MusicXmlFile::parseBackup --
//

void MusicXmlFile::parseBackup(CXMLObject* object, 
      int partnum, long& ticktime) {
   if (object == NULL) {
      return;
   }
   if (object->GetType() != xmlElement) {
      return;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "backup") {
      return;
   }

   long duration = 0;
   object = object->Zoom();
   while (object != NULL) {
      if (object->GetType() == xmlElement) {
         element = (CXMLElement*)object;
         if (element->GetName() == "duration") {
            duration = getIntFromCharacterData(element->Zoom());
            break;
         }
      }
      object = object->GetNext();
   }

   ticktime -= duration;
}



//////////////////////////////
//
// MusicXmlFile::parsePrint -- Print suggestions.
//

void MusicXmlFile::parsePrint(CXMLObject* entry, int partnum, int ticktime) {
   if (entry == NULL) {
      return;
   }
   if (entry->GetType() != xmlElement) {
      return;
   }
   CXMLElement* element = (CXMLElement*)entry;
   if (element->GetName() != "print") {
      return;
   }

   _MusicXmlItem tempitem;
   tempitem.type     = MXI_print;
   tempitem.obj      = entry;
   tempitem.ticktime = ticktime;
   tempitem.serialnum= parseserialnum++;
   tempitem.divisions = parsedivisions;
   
   appendAllPartStaves(partnum, tempitem);
}




//////////////////////////////
//
// MusicXmlFile::parseBarline --
//

void MusicXmlFile::parseBarline(CXMLObject* entry, int partnum, int ticktime) {
   if (entry == NULL) {
      return;
   }
   if (entry->GetType() != xmlElement) {
      return;
   }
   CXMLElement* element = (CXMLElement*)entry;
   if (element->GetName() != "barline") {
      return;
   }

   _MusicXmlItem tempitem;
   tempitem.type     = MXI_barline;
   tempitem.obj      = entry;
   tempitem.ticktime = ticktime;
   tempitem.serialnum= parseserialnum++;
   tempitem.divisions = parsedivisions;
   
   appendAllPartStaves(partnum, tempitem);
}



//////////////////////////////
//
// MusicXmlFile::getAttributeValue --
//

XMLString MusicXmlFile::getAttributeValue(CXMLObject* object, 
      const char* name) {
   XMLString string;
   CXMLElement* element = (CXMLElement*)object;
   int i; 
   for (i=0; i<element->GetAttributes().GetLength(); i++) {
      if (element->GetAttributes().GetName(i) == name) {
         string = element->GetAttributes().GetValue(i);
         return string;
      }
   }

   return string;
}



//////////////////////////////
//
// MusicXmlFile::parseSound --  
//

void MusicXmlFile::parseSound(CXMLObject* entry, int partnum, long& ticktime) {
   if (entry == NULL) {
      return;
   }
   if (entry->GetType() != xmlElement) {
      return;
   }
   CXMLElement* element = (CXMLElement*)entry;
   if (element->GetName() != "sound") {
      return;
   }

   int type = MXI_unknown;
   XMLString value;
   int i;
   for (i=0; i<element->GetAttributes().GetLength(); i++) {
      // assuming only one or the other of tempo/dynamic for now...
      if (element->GetAttributes().GetName(i) == "tempo") {
         type = MXI_tempo;
      }
   }

   if (type == MXI_unknown) {
      return;
   }

   _MusicXmlItem tempitem;
   tempitem.type     = type;
   tempitem.obj      = entry;
   tempitem.ticktime = ticktime;
   tempitem.serialnum= parseserialnum++;
   tempitem.divisions = parsedivisions;
   appendAllPartStaves(partnum, tempitem);

}



//////////////////////////////
//
// MusicXmlFile::parseNote --
//

void MusicXmlFile::parseNote(CXMLObject* entry, int partnum, long& ticktime) {
   if (entry == NULL) {
      return;
   }
   if (entry->GetType() != xmlElement) {
      return;
   }
   CXMLElement* element = (CXMLElement*)entry;
   if (element->GetName() != "note") {
      return;
   }

   int staff = getStaffNumber(entry) - 1;
   checkPartStaff(partnum, staff+1);

   CXMLObject* current = element;
   CXMLCharacterData* cdata = NULL;
   current = current->Zoom();
   CXMLObject* durcurr;
   char buffer[1024] = {0};
   int duration = 0;
   int voice = 0;
   int pitch = 0;
   int hasFicta = 0;

   int chord = 0;  // duration of the previous note found in the chord.
   int chordQ = 0;  // 1 if <chord> tag is present

   _MusicXmlItem tempitem;
   tempitem.serialnum = parseserialnum++;

   XMLString string;
   while (current != NULL) {
      if (current->GetType() == xmlElement) {
         element = (CXMLElement*)current;
         // cout << "Note Element Name = " << element->GetName() << endl;

         if (element->GetName() == "duration") {
            durcurr = current->Zoom();
            while (durcurr != NULL) {
               if (durcurr->GetType() == xmlCharacterData) {
                  cdata = (CXMLCharacterData*)durcurr;
                  string = cdata->GetData();
                  strncpy(buffer, string.c_str(), 512);
                  removeBorderSpaces(buffer);
                  sscanf(buffer, "%d", &duration);
                  // cout << "NOTE DURATION = " << duration << endl;
               }
               durcurr = durcurr->GetNext();
            }
          
         } else if (element->GetName() == "chord") {
            chordQ = 1;
            // get the duration of the last note in the chord
            // which will be used to adjust the starting time of this note
            // in the score.
            if (partdata[partoffset[partnum]+staff].getSize() > 0) {
               chord = partdata[partoffset[partnum]+staff]
                       [partdata[partoffset[partnum]+staff].getSize()-1]
                       .tickdur;
            } else {
               chord = ticktime;
            }

         } else if (element->GetName() == "voice") {
            durcurr = current->Zoom();
            while (durcurr != NULL) {
               if (durcurr->GetType() == xmlCharacterData) {
                  cdata = (CXMLCharacterData*)durcurr;
                  string = cdata->GetData();
                  strncpy(buffer, string.c_str(), 512);
                  removeBorderSpaces(buffer);
                  sscanf(buffer, "%d", &voice);
                  // cout << "VOICE = " << voice << endl;
               }
               durcurr = durcurr->GetNext();
            }

         } else if (element->GetName() == "pitch") {

            pitch = parsePitch(element);
            // cout << "Base40 NOTE = " << pitch << endl;
            hasFicta = parseFicta(element);

         } else if (element->GetName() == "lyric") {

            int tempnum = 0;
            int i;
            XMLString value;
            for (i=0; i<element->GetAttributes().GetLength(); i++) {
               if (element->GetAttributes().GetName(i) == "number") {
                  value = element->GetAttributes().GetValue(i);
                  strncpy(buffer, value.c_str(), 1024);
                  sscanf(buffer, "%d", &tempnum);
                  break;
               }
            }
            if (tempnum != 0) {
               // cout << "!! Found lyric number " << tempnum << endl;
               _MusicXmlItem templyric;
               templyric.type        = MXI_lyric;
               templyric.obj         = entry;
               templyric.ticktime    = ticktime;
               templyric.tickdur     = duration;
               templyric.divisions   = parsedivisions;
               templyric.serialnum   = parseserialnum++;
               templyric.ownerserial = tempitem.serialnum;
               templyric.lyricnum    = tempnum;
               templyric.obj         = (CXMLObject*)element;
               partdata[partoffset[partnum]+staff].append(templyric);
            }
         }

      }
      current = current->GetNext();
   }

   ticktime = ticktime - chord;
   if (chordQ) {
      tempitem.chordhead = lastchordhead;
   } else {
      lastchordhead = entry;
   }

   tempitem.type      = MXI_note;
   tempitem.obj       = entry;
   tempitem.ticktime  = ticktime;
   tempitem.tickdur   = duration;
   if (voice > 4) {  // temporary code for SharpEye piano parts
      voice -= 4;
   }
   tempitem.voice     = voice;
   tempitem.pitch     = pitch;
   tempitem.ficta     = hasFicta;   hasFicta = 0;
   tempitem.divisions = parsedivisions;
   partdata[partoffset[partnum]+staff].append(tempitem);
   
   ticktime += duration;
}



//////////////////////////////
//
// MusicXmlFile::parseFicta -- returns true if there is a ficta mark
//    which is indicated in the MusicXML as an accidental in parentheses:
//
//      <note default-x="131">
//        <pitch>
//          <step>F</step>
//          <alter>1</alter>
//          <octave>4</octave>
//        </pitch>
//        <duration>16</duration>
//        <voice>1</voice>
//        <type>half</type>
//        <accidental parentheses="yes">sharp</accidental>
//

int MusicXmlFile::parseFicta(CXMLObject* entry) {

   CXMLObject* current;
   CXMLElement* element;

   int i;
   while (entry != NULL) {
      current = entry;

      if (current->GetType() == xmlElement) {
         element = (CXMLElement*)current;

         if (element->GetName() == "accidental") {

            for (i=0; i<element->GetAttributes().GetLength(); i++) {
               if (element->GetAttributes().GetName(i) == "parentheses") {
                  if (element->GetAttributes().GetValue(i) == "yes") {
                     return 1;
                  } 
               }
            }

         }
      }

      entry = entry->GetNext();
   }

   return 0;
}



//////////////////////////////
//
// MusicXmlFile::parsePitch -- convert a MusicXML pitch into base40.
//

int MusicXmlFile::parsePitch(CXMLObject* entry) {
   entry = entry->Zoom();
   int chroma = 2;
   int octave = 4;
   int alter = 0;

   CXMLObject* current;
   CXMLCharacterData* cdata;
   CXMLElement* element;
   char buffer[1024] = {0};
   XMLString string;

   while (entry != NULL) {
      current = entry;
      if (current->GetType() == xmlElement) {
         element = (CXMLElement*)current;

         if (element->GetName() == "step") {
            current = current->Zoom();
            while (current != NULL) {
               if (current->GetType() == xmlCharacterData) {
                  cdata = (CXMLCharacterData*)current;
                  string = cdata->GetData();
                  strncpy(buffer, string.c_str(), 512);
                  removeBorderSpaces(buffer);
                  if (isalpha(buffer[0])) {
                     switch (toupper(buffer[0])) {
                        case 'C': chroma =  2; break;
                        case 'D': chroma =  8; break;
                        case 'E': chroma = 14; break;
                        case 'F': chroma = 19; break;
                        case 'G': chroma = 25; break;
                        case 'A': chroma = 31; break;
                        case 'B': chroma = 37; break;
                     }
                  }
                  break;
               }
               current = current->GetNext();
            }

         } else if (element->GetName() == "alter") {
            current = current->Zoom();
            while (current != NULL) {
               if (current->GetType() == xmlCharacterData) {
                  cdata = (CXMLCharacterData*)current;
                  string = cdata->GetData();
                  strncpy(buffer, string.c_str(), 512);
                  removeBorderSpaces(buffer);
                  sscanf(buffer, "%d", &alter);
                  break;
               }
               current = current->GetNext();
            }

         } else if (element->GetName() == "octave") {
            current = current->Zoom();
            while (current != NULL) {
               if (current->GetType() == xmlCharacterData) {
                  cdata = (CXMLCharacterData*)current;
                  string = cdata->GetData();
                  strncpy(buffer, string.c_str(), 512);
                  removeBorderSpaces(buffer);
                  sscanf(buffer, "%d", &octave);
                  break;
               }
               current = current->GetNext();
            }

         }
      }

      entry = entry->GetNext();
   }

   if ((alter < -2) || (alter > 2)) {
      cout << "!! error processing chromatic alteration of: " << alter << endl;
   }

   return chroma + alter + 40 * octave;
}



//////////////////////////////
//
// MusicXmlFile::appendAllPartStaves --  Append an item to all
//   staves for a part (used to add key signatures to the parts, for example).
//

void MusicXmlFile::appendAllPartStaves(int partnum, _MusicXmlItem& tempitem) {
   int i;
   for (i=0; i<partstaves[partnum]; i++) {
      partdata[partoffset[partnum]+i].append(tempitem);
   }
}



//////////////////////////////
//
// MusicXmlFile::getClefStaffNumber --
//

int MusicXmlFile::getClefStaffNumber(CXMLObject* object) {
   int output = 1;
   if (object == NULL) {
      return output;
   }
   if (object->GetType() != xmlElement) {
      return output;
   }

   CXMLElement* element = (CXMLElement*)object;

   XMLString value;
   int i;
   char buffer[1024] = {0};
   for (i=0; i<element->GetAttributes().GetLength(); i++) {
      if (element->GetAttributes().GetName(i) == "number") {
         value = element->GetAttributes().GetValue(i);
         strncpy(buffer, value.c_str(), 1024);
         sscanf(buffer, "%d", &output);
         break;
      }
   }

   return output;
}



//////////////////////////////
//
// MusicXmlFile::getStaffNumber -- return the staff number of the 
//      object, or 1 by default.
// 

int MusicXmlFile::getStaffNumber(CXMLObject* object) {
   int output = 1;
   if (object == NULL) {
      return output;
   }

   CXMLElement* element;
   if (object->GetType() == xmlElement) {
      element = (CXMLElement*)object;
      if (element->GetName() == "clef") {
         return getClefStaffNumber(object);
      }
   }

   object = object->Zoom();
   if (object == NULL) {
      return output;
   }

   while (object != NULL) {
      if (object->GetType() != xmlElement) {
         object = object->GetNext();
         continue;
      }
      element = (CXMLElement*)object;
      if (element->GetName() != "staff") {
         object = object->GetNext();
         continue;
      }
      output = getIntFromCharacterData(object->Zoom());

      break;
   }

   return output;
}



//////////////////////////////
//
// MusicXmlFile::checkPartStaff --
//

void MusicXmlFile::checkPartStaff(int partnum, int staff) {
   if (staff <= 0) {
      cout << "Error: staff number is too small: " << staff
           << " for part " << partnum << endl;
      exit(1);
   }

   if (staff > partstaves[partnum]) {
      cout << "Error: staff number is too large: " << staff
           << " for part " << partnum+1 << endl;
      cout << "Maximum staff number is: " << partstaves[partnum] << endl;
      exit(1);
   }
}



//////////////////////////////
//
// MusicXmlFile::parseAttributes --
//

void MusicXmlFile::parseAttributes(CXMLObject* entry, int partnum, 
      long& ticktime) {
   if (entry == NULL) {
      return;
   }
   if (entry->GetType() != xmlElement) {
      return;
   }
   CXMLElement* element = (CXMLElement*)entry;
   if (element->GetName() != "attributes") {
      return;
   }
   
   CXMLObject* current = element;
   current = current->Zoom();
   _MusicXmlItem tempitem;

   int i;
   int staff = 0;

   while (current != NULL) {
      if (current->GetType() == xmlElement) {
         element = (CXMLElement*)current;
         // cout << "Attributes Element Name = " << element->GetName() << endl;

         tempitem.obj       = current;
         tempitem.divisions = parsedivisions;
         tempitem.ticktime  = ticktime;
     
         if (element->GetName() == "divisions") {
            tempitem.type = MXI_divisions;
            parsedivisions = getDivisions(element);
            tempitem.divisions = parsedivisions;
            tempitem.serialnum = parseserialnum++;
            tempitem.divisions = parsedivisions;
            appendAllPartStaves(partnum, tempitem);
         } else if (element->GetName() == "clef") {
            tempitem.type = MXI_clef;
            staff = getStaffNumber(element) - 1;
            checkPartStaff(partnum, staff+1);
            tempitem.serialnum = parseserialnum++;
            tempitem.divisions = parsedivisions;
            partdata[partoffset[partnum]+staff].append(tempitem);
         } else if (element->GetName() == "key") {
            tempitem.type = MXI_key;
            tempitem.serialnum = parseserialnum++;
            tempitem.divisions = parsedivisions;
            appendAllPartStaves(partnum, tempitem);
         } else if (element->GetName() == "time") {
            tempitem.obj  = element;
            tempitem.type = MXI_time;

            for (i=0; i<element->GetAttributes().GetLength(); i++) {
               if (element->GetAttributes().GetName(i) == "symbol") {
                  tempitem.type = MXI_timemet;
                  tempitem.serialnum = parseserialnum++;
                  tempitem.divisions = parsedivisions;
                  appendAllPartStaves(partnum, tempitem);
               }
            }

            tempitem.type = MXI_time;
            tempitem.serialnum = parseserialnum++;
            tempitem.divisions = parsedivisions;
            appendAllPartStaves(partnum, tempitem);
         } 

      }
      current = current->GetNext();
   }
}



//////////////////////////////
//
// MusicXmlFile::sortStaff --
//

void MusicXmlFile::sortStaff(int staffno) {
   qsort(partdata[staffno].getBase(), partdata[staffno].getSize(),
         sizeof(_MusicXmlItem), staffcompare);
}



//////////////////////////////
//
// MusicXmlFile::staffcompare -- sorting function for items in a single part.
//

int MusicXmlFile::staffcompare(const void* A, const void* B) {
   _MusicXmlItem& a = *((_MusicXmlItem*)A);
   _MusicXmlItem& b = *((_MusicXmlItem*)B);

   if (a.ticktime < b.ticktime) {
      return -1;
   } else if (a.ticktime > b.ticktime) {
      return +1;
   }


   if ((a.type >= MXI_note) || (b.type >= MXI_note)) {
      // sort items that go after notes according to their type
      if (a.type < b.type) { return -1; }
      if (b.type < a.type) { return +1; }
   } else if ((a.type != MXI_note) || (b.type != MXI_note)) {
      // events are happening at the same time:
      // now sort by event type (measure, note, keysignature, etc.)

      // put clef marks before time and key signatures
      // put key signatures before time signatures
      // put metronome markings after time signatures.
      if ((a.type == MXI_measure) && (b.type == MXI_print)) return -1;
      if ((a.type == MXI_print) && (b.type == MXI_measure)) return +1;

      if ((a.type == MXI_print) && (b.type != MXI_print)) return -1;
      if ((a.type != MXI_print) && (b.type == MXI_print)) return +1;
      
      if ((a.type == MXI_clef) && (b.type == MXI_time)) return -1;
      if ((a.type == MXI_time) && (b.type == MXI_clef)) return +1;

      if ((a.type == MXI_clef) && (b.type == MXI_timemet)) return -1;
      if ((a.type == MXI_timemet) && (b.type == MXI_clef)) return +1;

      if ((a.type == MXI_clef) && (b.type == MXI_key)) return -1;
      if ((a.type == MXI_key) && (b.type == MXI_clef)) return +1;

      if ((a.type == MXI_key) && (b.type == MXI_time)) return -1;
      if ((a.type == MXI_time) && (b.type == MXI_key)) return +1;

      if ((a.type == MXI_key) && (b.type == MXI_timemet)) return -1;
      if ((a.type == MXI_timemet) && (b.type == MXI_key)) return +1;

      if ((a.type == MXI_tempo) && (b.type == MXI_time)) return +1;
      if ((a.type == MXI_time) && (b.type == MXI_tempo)) return -1;

      if ((a.type == MXI_tempo) && (b.type == MXI_timemet)) return +1;
      if ((a.type == MXI_timemet) && (b.type == MXI_tempo)) return -1;

      if ((a.type == MXI_tempo) && (b.type == MXI_clef)) return +1;
      if ((a.type == MXI_clef) && (b.type == MXI_tempo)) return -1;

      if ((a.type == MXI_tempo) && (b.type == MXI_key)) return +1;
      if ((a.type == MXI_key) && (b.type == MXI_tempo)) return -1;

      if ((a.type == MXI_timemet) && (b.type == MXI_time)) return -1;
      if ((a.type == MXI_time) && (b.type == MXI_timemet)) return +1;

      // otherwise, for non-durational items, sort by their original position
      // in the MusicXML File:
      if (a.type == MXI_note) {   // notes always go last when at same time
         return +1; 
      } else if (b.type == MXI_note) {
         return -1;
      } else if (a.serialnum < b.serialnum) {
         return -1;
      } else if (a.serialnum > b.serialnum) {
         return +1;
      } else {
         if (a.type < b.type) {
            return -1;
         } else if (a.type > b.type) {
            return +1;
         } else {
            // the objects are incredibly equivalent to each other!
            return 0;
         }
      }

   }

   // sorting exactly two notes after this point.

   // both events are notes which are occuring at the same time.
   // sort the notes by voice.

   if (a.voice < b.voice) {
      return -1;
   } else if (a.voice > b.voice) {
      return 1;
   }

   // notes are in the same voice, sort by grace note/not grace note.
   // If both notes are grace notes, then sort by their order in the file.
   if (a.tickdur == 0 && b.tickdur > 0) {
      return -1;
   } else if (a.tickdur > 0 && b.tickdur == 0) {
      return +1;
   } else if (a.tickdur == 0 && b.tickdur == 0) {
      if (a.serialnum < b.serialnum) {
         return -1;
      } else if (a.serialnum > b.serialnum) {
         return +1;
      } else {
         // something strange happended
         return 0;
      }
   }

   // notes are in the same voice, sort by pitch
   if (a.pitch < b.pitch) {
      return -1;
   } else if (a.pitch > b.pitch) {
      return 1;
   }

   // seem to be the same/duplicate note item
   // if the rhythms are different, then we have a problem...
   // so address that later if necessary.

   return 0;
}



//////////////////////////////
//
// MusicXmlFile::humdrumPart --
//    default value: debug = 0
//

void MusicXmlFile::humdrumPart(HumdrumFile& hfile, int staffno, int debugQ) {
   hfile.clear();
   humline = 0;   // reset humline to start of file

   if ((staffno < 0) || (staffno >= getStaffCount())) {
      return;
   }

   partvoiceticktime2 = -1;
   partvoiceticktime1 = -1;
   
   int newlineQ = 0;   // for printing dynamics and lyrics after notes.
   SSTREAM* tempstream = new SSTREAM;

   (*tempstream) << "**kern";
   if (humdrumDynamics && partdynamics[staffno]) {
      (*tempstream) << "\t**dynam";
   }
   (*tempstream) << "\n"; humline++;

   (*tempstream) << "*staff" << staffno+1;
   if (humdrumDynamics && partdynamics[staffno]) {
      (*tempstream) << "\t*staff" << staffno+1;
   }
   (*tempstream) << "\n"; humline++;

   int noteinit = 0;
   int i;
   int divisions = 4;
   int measureno;
   char buffer[1024] = {0};
   char buffer2[1024] = {0};
   for (i=0; i<partdata[staffno].getSize(); i++) {
      switch (partdata[staffno][i].type) {
         case MXI_divisions:
            divisions = getDivisions(partdata[staffno][i].obj);
            break;
         case MXI_measure:
            measureno = partdata[staffno][i].measureno;
            // don't print pickup-measures which are -1 measureno
            if (measureno >= 0) {
               strcpy(buffer, "=");
               if (measureno > 0) {
                  sprintf(buffer2, "%d", measureno);
                  strcat(buffer, buffer2);
               }
               if (!noteinit) {
                  strcat(buffer, "-");  // first barline does not print
               }
               printMeasureStyle(buffer2, staffno, i);
               strcat(buffer, buffer2);
               (*tempstream) << buffer;
               if (humdrumDynamics && partdynamics[staffno]) {
                  (*tempstream) << "\t" << buffer;
               }
               (*tempstream) << "\n"; humline++;
            }
            break;
         case MXI_print:
	    humline += printSystemBreak((*tempstream), partdata[staffno][i].obj);
	    humline += printPageBreak((*tempstream), partdata[staffno][i].obj);
            break;
         case MXI_barline:
            if (printMeasureStyle(buffer, staffno, i)) {
               (*tempstream) << buffer;
               if (humdrumDynamics && partdynamics[staffno]) {
                  (*tempstream) << "\t" << buffer;
               }
               (*tempstream) << "\n"; humline++;
            }
            break;
         case MXI_clef:
            (*tempstream) << printKernClef(buffer, partdata[staffno][i].obj);
            if (partvoiceticktime1 > partdata[staffno][i].ticktime) {
               // this if statement is needed if there are two voices
               // (or more) and the first part is being interrupted
               // by the clef sign.  If it is being interrupted, then
               // there should be two clef markers: one for the fist
               // voice in the spine, and one for the second voice
               // in the spine.  If the spine are combined to
               // have only one clef marker, then the rhythm cannot be
               // parsed correctly since the duration of the first 
               // voice will be lost most likely in the merging of the 
               // two voices.
               (*tempstream) << "\t";
               (*tempstream) << printKernClef(buffer, partdata[staffno][i].obj);
            }
            if (humdrumDynamics && partdynamics[staffno]) {
               (*tempstream) << "\t" << buffer;
            }
            (*tempstream) << "\n"; humline++;
            break;
         case MXI_key:
            getKernKey(buffer, partdata[staffno][i].obj); 
            if (buffer[0] != '\0') {
               (*tempstream) << buffer;
               if (humdrumDynamics && partdynamics[staffno]) {
                  (*tempstream) << "\t" << buffer;
               }
               (*tempstream) << "\n"; humline++;
            }
            break;
         case MXI_timemet:
            getKernTimeMet(buffer, partdata[staffno][i].obj);
            if (buffer[0] != '\0') {
               (*tempstream) << buffer; 
               if (humdrumDynamics && partdynamics[staffno]) {
                  (*tempstream) << "\t" << buffer;
               }
               (*tempstream) << "\n"; humline++;
            }
            break;
         case MXI_time:
            (*tempstream) << getKernTimeSig(buffer, partdata[staffno][i].obj);
            if (humdrumDynamics && partdynamics[staffno]) {
               (*tempstream) << "\t" << buffer;
            }
            (*tempstream) << "\n"; humline++;
            break;
         case MXI_tempo:
            getKernMetronome(buffer, partdata[staffno][i].obj);
            if (buffer[0] != '\0') {
               (*tempstream) << buffer;
               if (humdrumDynamics && partdynamics[staffno]) {
                  (*tempstream) << "\t" << buffer;
               }
               (*tempstream) << "\n"; humline++;
            }
            break;
         case MXI_forward:
            break;
         case MXI_backup:
            break;
         case MXI_text:
            printText(*tempstream, staffno, i);
            (*tempstream) << "\n"; humline++;
            break;
         case MXI_instrument:
            printInstrument(*tempstream, staffno, i);
            (*tempstream) << "\n"; humline++;
            break;
         case MXI_dynamic:
            if (humdrumDynamics && partdynamics[staffno]) {
               printDynamic(*tempstream, staffno, i);
               (*tempstream) << "\n"; humline++;
            }
            break;
         case MXI_note:
            partdata[staffno][i].humline = humline;
            noteinit = 1;
            newlineQ = printKernNote((*tempstream), staffno, i, divisions);
            if (newlineQ) {
               if (humdrumDynamics && partdynamics[staffno]) {
                  if (i < partdata[staffno].getSize()-1) {
                     if (partdata[staffno][i].ticktime != 
                         partdata[staffno][i+1].ticktime) {
                        // time for something else, but no dynamic/lyric found
                        (*tempstream) << "\t.\n"; humline++;
                     } else if (isGraceNote(partdata[staffno][i].obj)) {
                        (*tempstream) << "\t.\n"; humline++;
                     } else {
                        // new case added 2008Jul25
                        if (partdata[staffno][i+1].type == MXI_dynamic) {
                           if (humdrumDynamics && partdynamics[staffno]) {
                              printDynamic(*tempstream, staffno, ++i);
                              (*tempstream) << "\n"; humline++;
                           } else {
	                      (*tempstream) << "\t.\n"; humline++;
                           }
                        } else {
	                   (*tempstream) << "\t.\n"; humline++;
                        }
	             }
                  } else {
                     (*tempstream) << "\t.\n"; humline++;
                  }
               } else {
                  (*tempstream) << "\n"; humline++;
               }
            } 
            break;
      }
   }
   (*tempstream) << "*-";
   if (humdrumDynamics && partdynamics[staffno]) {
      (*tempstream) << "\t*-";
   }
   (*tempstream) << ends;

   SSTREAM* newtempstream = new SSTREAM;
   processSpineChanges(*newtempstream, *tempstream, staffno);
   delete tempstream;
   tempstream = newtempstream;
   (*tempstream) << ends;
   if (debugQ) {
      cout << tempstream->CSTRING << endl;
   }
   hfile.read(*tempstream);
   delete tempstream;

   if (getOption("lyric")) {
      addLyrics(hfile, staffno);
   }
}



//////////////////////////////
//
// MusicXmlFile::addLyrics -- add lyrics to converted humdrum file
//

void MusicXmlFile::addLyrics(HumdrumFile& hfile, int staffno) {
   int i;

   // find the maximum verse number
   int lyriccount = 0;
   for (i=0; i<partdata[staffno].getSize(); i++) {
      if (partdata[staffno][i].type == MXI_lyric) {
         if (lyriccount < partdata[staffno][i].lyricnum) {
            lyriccount = partdata[staffno][i].lyricnum;
         }
      }
   }
   // cout << "!! Adding Lyrics to " << staffno << " total = " 
   //      << lyriccount << endl;
   if (lyriccount > MAXLYRIC) {
      // don't want to add wierd number of lyrics.
      return;
   }
   int m;
   for (m=0; m<lyriccount; m++) {
      for (i=0; i<hfile.getNumLines(); i++) {
         // cout << "!! Index = " << i << endl;
         // cout << ">>> Input line is" << hfile[i] << endl;
         switch (hfile[i].getType()) {
            case E_humrec_none:
            case E_humrec_empty:
            case E_humrec_global_comment:
            case E_humrec_bibliography:
               break;
            case E_humrec_data_comment:
               // add a "!" token
               // cout << "Adding a local comment" << endl;
               hfile[i].appendField("!", E_unknown, "1");
               break;
            case E_humrec_data_kern_measure:
               // add hfile[i][0]
               // cout << "Adding a measure element" << endl;
               hfile[i].appendField(hfile[i][0], E_unknown, "1");
               break;
            case E_humrec_interpretation:
               // add "*", "**text", or "*-" token
               // cout << "Adding a interpretation element" << endl;
               if (strncmp(hfile[i][0], "**", 2) == 0) {
               hfile[i].appendField("**silbe", E_unknown, "1");
               } else if (strncmp(hfile[i][0], "*-", 2) == 0) {
                  hfile[i].appendField("*-", E_unknown, "1");
               } else if (strncmp(hfile[i][0], "*staff", 6) == 0) {
                  // transfer staff interpretations
                  hfile[i].appendField(hfile[i][0], E_unknown, "1");
               } else {
                  hfile[i].appendField("*", E_unknown, "1");
               }
               break;
            case E_humrec_data:
               // add "." token
               // cout << "Adding a null element" << endl;
               hfile[i].appendField(".", E_unknown, "1");
               break;
            default:
               break;
         }
      }
   }

   // now build a table of humdrum lines for each note serial number
   // so that the text can be inserted onto the proper line.
   Array<_SerialLine> seriallines;
   seriallines.setSize(partdata[staffno].getSize());
   seriallines.setSize(0);
   _SerialLine serialline;

   for (i=0; i<partdata[staffno].getSize(); i++) {
      if (partdata[staffno][i].type == MXI_note) {
         serialline.line = partdata[staffno][i].humline;
         serialline.serial = partdata[staffno][i].serialnum;
         seriallines.append(serialline); 
      }
   }

   qsort(seriallines.getBase(), seriallines.getSize(), 
         sizeof(_SerialLine), seriallinesort);

   // now that a database of lines for each serial number of notes
   // has been created and sorted by serial number, go through
   // the partdata for the staffno and insert text for each
   // lyric found in the data.

   char* buffer[4096] = {0};
   int lyricnum = 0;
   int ownerserial = 0;
   _SerialLine* match = NULL;
   for (i=0; i<partdata[staffno].getSize(); i++) {
      if (partdata[staffno][i].type != MXI_lyric) {
         continue;
      }

      // have a lyric, check to see if the number is in range
      if (partdata[staffno][i].lyricnum > MAXLYRIC) {
         continue;
      }

      lyricnum = partdata[staffno][i].lyricnum;
      ownerserial = partdata[staffno][i].ownerserial;

      // have a lyric to insert, find out which line to put it on
      serialline.serial = ownerserial;

      match = (_SerialLine*) bsearch(&serialline, seriallines.getBase(), 
                      seriallines.getSize(), sizeof(_SerialLine), 
                      seriallinesort);

//      void *bsearch(const void *key, const void *base, size_t nmemb,
//             size_t size, int (*compar)(const void *, const void *));


      if (match == NULL) {
         cout << "!! Lyric placement serial not found :" 
              << serialline.serial << endl;
         continue;
      }


      getLyricText((char*)buffer, (CXMLObject*)partdata[staffno][i].obj);
      if (hfile[match->line].getType() == E_humrec_data_measure) {
         // temporary bug fix from unusual line error
         hfile[match->line-1].changeField(
           hfile[match->line].getFieldCount()-lyriccount+lyricnum-1,
           (char*)buffer);
      } else {
         hfile[match->line].changeField(
           hfile[match->line].getFieldCount()-lyriccount+lyricnum-1,
           (char*)buffer);
      }

   }


}



//////////////////////////////
//
// MusicXmlFile::getLyricText -- extract lyric text.
//

char* MusicXmlFile::getLyricText(char* buffer, CXMLObject* object) {
   buffer[0] = '\0';
   if (object == NULL) {
      return buffer;
   }
   if (object->GetType() != xmlElement) {
      return buffer;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "lyric") {
      return buffer;
   }

   CXMLCharacterData* textdata;
   CXMLCharacterData* formatdata;
   XMLString string;
   XMLString formatstring;
   object = object->Zoom();
   CXMLObject* obj2;
   while (object != NULL) {
      if (object->GetType() != xmlElement) {
         object = object->GetNext();
         continue;
      }
      element = (CXMLElement*)object;

      if (element->GetName() == "text") {
         obj2 = object->Zoom();
         while (obj2 != NULL) {
            if (obj2->GetType() == xmlCharacterData) {
               textdata = (CXMLCharacterData*)obj2;
               string = textdata->GetData();
               break;
            }
            obj2 = obj2->GetNext();
         }
      } else if (element->GetName() == "syllabic") {
         obj2 = object->Zoom();
         while (obj2 != NULL) {
            if (obj2->GetType() == xmlCharacterData) {
               formatdata = (CXMLCharacterData*)obj2;
               formatstring = formatdata->GetData();
               break;
            }
            obj2 = obj2->GetNext();
         }
      }
      object = object->GetNext();
   }

   buffer[0] = '\0';
   if (strcmp(formatstring.c_str(), "end") == 0) {
      strcat(buffer, "-");
   }
   if (strcmp(formatstring.c_str(), "middle") == 0) {
      strcat(buffer, "-");
   }
   strncat(buffer, string.c_str(), 1024);
   if (strcmp(formatstring.c_str(), "begin") == 0) {
      strcat(buffer, "-");
   }
   if (strcmp(formatstring.c_str(), "middle") == 0) {
      strcat(buffer, "-");
   }

   // <extend/> is used to place underscore starting on current
   // syllable.
   
   return buffer;
}



//////////////////////////////
//
// seriallinesort --
//

int seriallinesort(const void* a, const void* b) {
   _SerialLine A = *((_SerialLine*)a);
   _SerialLine B = *((_SerialLine*)b);
   if (A.serial < B.serial) {
      return -1;
   } else if (A.serial > B.serial) {
      return +1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// MusicXmlFile::isGraceNote -- 
//

int MusicXmlFile::isGraceNote(CXMLObject* object) {
   if (object == NULL) {
      return 0;
   }
   if (object->GetType() != xmlElement) {
      return 0;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "note") {
      return 0;
   }

   object = object->Zoom();
   while (object != NULL) {
      if (object->GetType() != xmlElement) {
         object = object->GetNext();
         continue;
      }
      element = (CXMLElement*)object;
      if (element->GetName() == "grace") {
         return 1;
      }
      object = object->GetNext();
   }

   return 0;
}



//////////////////////////////
//
// MusicXmlFile::printInstrument --
//

void MusicXmlFile::printInstrument(ostream& out, int staffno, int index) {
   // start with <score-part> element for the correct part:
   CXMLObject* object = partdata[staffno][index].obj;

   char buffer[1024] = {0};

   // now look for <part-name> child element and extract text content.
   object = object->Zoom();

   while (object != NULL) {
      if (object->GetType() != xmlElement) {
         object = object->GetNext();
         continue;
      }
      if (((CXMLElement*)object)->GetName() == "part-name") {
         getCharacterData(buffer, object);
         out << "*I\"" << buffer;
         return;
      }
   }
}



//////////////////////////////
//
// MusicXmlFile::getCharcterData -- Given an element, return the first
//     charcter data inside of the element.  Only get up to the first 512 chars.
//

char*  MusicXmlFile::getCharacterData(char* buffer, CXMLObject* object) {
   XMLString string;
   CXMLCharacterData* cdata;
   object = object->Zoom();
   while (object != NULL) {
      if (object->GetType() == xmlCharacterData) {
         cdata = (CXMLCharacterData*)object;
         string = cdata->GetData();
         strncpy(buffer, string.c_str(), 512);
         removeBorderSpaces(buffer);
         break;
      }
      object = object->GetNext();
   }

   return buffer;
}



//////////////////////////////
//
// MusicXmlFile::printText -- 
//

void MusicXmlFile::printText(ostream& out, int staffno, int index) {
   CXMLObject* object = partdata[staffno][index].obj;
   if (object == NULL) {
      out << "!";
      return;
   }

   if (object->GetType() != xmlElement) {
      out << "!";
      return;
   }

   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "words") {
      out << "!";
      return;
   }

   // find grandparent and check for attribute:
   // <direction placement="above">
   CXMLElement *grandparent = (CXMLElement*)element->GetParent();
   grandparent = (CXMLElement*)grandparent->GetParent();
   int aboveQ = 0;
   int i;
   for (i=0; i<grandparent->GetAttributes().GetLength(); i++) {
      if (grandparent->GetAttributes().GetName(i) == "placement") {
         if (grandparent->GetAttributes().GetValue(i) == "above") {
            aboveQ = 1;
         }
         break;
      }
   }

   char buffer[12345] = {0};
   int italicQ = 0;
   int boldQ = 0;
   int yval = 0;
   XMLString string;
   if (element->GetName() == "words") {
      for (i=0; i<element->GetAttributes().GetLength(); i++) {

         if (element->GetAttributes().GetName(i) == "font-style") {
            if (element->GetAttributes().GetValue(i) == "italic") {
               italicQ = 1;
            } else if (element->GetAttributes().GetValue(i) == "bold") {
               boldQ = 1;
            }
         } else if (element->GetAttributes().GetName(i) == "default-y") {
            string = element->GetAttributes().GetValue(i);
            sscanf(string.c_str(), "%d", &yval);
         }
      }

      CXMLObject* current = element;
      CXMLCharacterData* cdata;
      current = current->Zoom();
      while (current != NULL) {
         if (current->GetType() == xmlCharacterData) {
            cdata = (CXMLCharacterData*)current;
            string = cdata->GetData();
            strncpy(buffer, string.c_str(), 512);
            removeBorderSpaces(buffer);
            break;
         }
         current = current->GetNext();
      }
   }

   out << "!LO:TX:";
   if (italicQ && boldQ) {
      out << "bi:";
   } else if (italicQ) {
      out << "i:";
   } else if (boldQ) {
      out << "b:";
   }
   if (aboveQ && yval) {
      out << "Z=" << yval << ":";
   } else {
      out << "Y=" << -yval << ":";
   }
   out << "t=";
   Array<char> tbuffer;
   tbuffer.setSize(strlen(buffer)+1);
   strcpy(tbuffer.getBase(), buffer);
   PerlRegularExpression pre;
   pre.sar(tbuffer, ":", "&colon;", "g");
   out << tbuffer;
}



//////////////////////////////
//
// MusicXmlFile::printDynamic --
//

void MusicXmlFile::printDynamic(ostream& out, int staffno, int index) {
   if (index > 0) {
      if (partdata[staffno][index-1].type == MXI_dynamic) {
         // can not understand two dynamics in a row!  Ignore all but first.
         return;
      }
   }
   CXMLObject* object = partdata[staffno][index].obj;
   if (object == NULL) {
      out << "\t.";
      return;
   }
   if (object->GetType() != xmlElement) {
      out << "\t.";
      return;
   }
   CXMLElement* element = (CXMLElement*)object;
   if ((element->GetName() != "dynamics") && 
       (element->GetName() != "wedge") &&
       (element->GetName() != "words") ) {
      out << "\t.";
      return;
   }

   char buffer[1024] = {0};
   strcpy(buffer, ".");

   if (element->GetName() == "dynamics") {
      object = object->Zoom();
      while (object != NULL) {
         if (object->GetType() != xmlElement) {
            object = object->GetNext();
            continue;
         }
         element = (CXMLElement*)object;
         if (element->GetName() == "fff") {
            strcpy(buffer, "fff");
            break;
         } else if (element->GetName() == "ff") {
            strcpy(buffer, "ff");
            break;
         } else if (element->GetName() == "f") {
            strcpy(buffer, "f");
            break;
         } else if (element->GetName() == "mf") {
            strcpy(buffer, "mf");
            break;
         } else if (element->GetName() == "mp") {
            strcpy(buffer, "mp");
            break;
         } else if (element->GetName() == "p") {
            strcpy(buffer, "p");
            break;
         } else if (element->GetName() == "pp") {
            strcpy(buffer, "pp");
            break;
         } else if (element->GetName() == "ppp") {
            strcpy(buffer, "ppp");
            break;
         }
         object = object->GetNext();
      }
   } 

   else if (element->GetName() == "wedge") {
      int i;
      for (i=0; i<element->GetAttributes().GetLength(); i++) {
         if (element->GetAttributes().GetName(i) == "type") {
            if (element->GetAttributes().GetValue(i) == "diminuendo") {
               strcpy(buffer, ">");
            } else if (element->GetAttributes().GetValue(i) == "crescendo") {
               strcpy(buffer, "<");
            }
         }
      }
   }

   // text versions of crescendos and dimuendos:
   else if (element->GetName() == "words") {
      CXMLObject* obj2 = element->Zoom();
      XMLString string;
      CXMLCharacterData* textdata;
      while (obj2 != NULL) {
         if (obj2->GetType() == xmlCharacterData) {
            textdata = (CXMLCharacterData*)obj2;
            string = textdata->GetData();
            if ((string == "cresc.")) {
               strcpy(buffer, "<");
            } else if ((string == "decresc.") || (string == "dim.")) {
               strcpy(buffer, ">");
            }
            break;
         }
         obj2 = obj2->GetNext();
      }
   }

   // out << " [" << partdata[staffno][index-1].ticktime <<  "]";

   out << "\t";
   out << buffer;
}



//////////////////////////////
//
// MusicXmlFile::processSpineChanges --
//

void MusicXmlFile::processSpineChanges(SSTREAM& newstream, SSTREAM& oldstream,
      int staffno) {
   char buffer[4096] = {0};

   int length;
   Array<char*> strings;
   Array<char*> dynamics;
   char* ptr = NULL;
   char* ptr2 = NULL;
   strings.setSize(50000);
   strings.setGrowth(50000);
   strings.setSize(0);
   
   dynamics.setSize(50000);
   dynamics.setGrowth(50000);
   dynamics.setSize(0);

   while (!oldstream.eof()) {
      buffer[0] = '\0';
      oldstream.getline(buffer, 4000, '\n');
      if (oldstream.eof() && (buffer[0] == '\0')) {
         break;
      }
      if (humdrumDynamics && partdynamics[staffno]) {
         ptr2 = strrchr(buffer, '\t');
         if (ptr2 == NULL) {
            ptr = new char[1];
            ptr[0] = '\0';
            dynamics.append(ptr);
         } else {
            length = strlen(ptr2);
            ptr = new char[length+1];
            strcpy(ptr, ptr2);
            dynamics.append(ptr);
            ptr2[0] = '\0';
            length = strlen(buffer);
         }
      } 
      length = strlen(buffer);
      ptr = new char[length+1];
      strcpy(ptr, buffer);
      strings.append(ptr);
   }

   int i;
   int j;
   Array<int> tabcount;
   tabcount.setSize(strings.getSize());
   tabcount.setAll(0);
   int count;
   for (i=0; i<strings.getSize(); i++) {
      j = 0;
      count = 0;
      while (strings[i][j] != '\0') {
         if (strings[i][j] == '\t') {
            count++;
         }
         j++;
      }
      tabcount[i] = count;
   }

   // double the barlines when two voices continue over barlines
   for (i=1; i<strings.getSize()-1; i++) {
      if ((strings[i][0] == '=') && (tabcount[i] == 0)) {
         if ((tabcount[i-1] == 1) && (tabcount[i+1] == 1)) {
            strcpy(buffer, strings[i]);
            strcat(buffer, "\t");
            strcat(buffer, strings[i]);
            length = strlen(buffer);
            delete [] strings[i];
            strings[i] = new char[length+1];
            strcpy(strings[i], buffer);
            tabcount[i]++;
         }
      }
   }

   // re-assemble the data with spine manipulations added.
   for (i=0; i<strings.getSize(); i++) {
      if (i > 0) {
         if ((tabcount[i-1] == 0) && (tabcount[i] == 1)) {
            newstream << "*^";
            if (humdrumDynamics && partdynamics[staffno]) {
               newstream << "\t*";
            }
            newstream << "\n";
            // newstream << "iii";
         } else if ((tabcount[i-1] == 1) && (tabcount[i] == 0)) {
            if (strstr(strings[i], "clef") != NULL) {
               newstream << strings[i] << '\t';
	       tabcount[i] = 1;
            } else {
               newstream << "*v\t*v";
               if (humdrumDynamics && partdynamics[staffno]) {
                  newstream << "\t*";
               }
               newstream << "\n";
            }
         } 

         // process multiple spine changes:
         else if ((tabcount[i-1] == 0) && (tabcount[i] == 2)) {
            newstream << "*^";
            if (humdrumDynamics && partdynamics[staffno]) {
               newstream << "\t*";
            }
            newstream << "\n";

            newstream << "*\t*^";
            if (humdrumDynamics && partdynamics[staffno]) {
               newstream << "\t*";
            }
            newstream << "\n";
         } else if ((tabcount[i-1] == 2) && (tabcount[i] == 0)) {
            newstream << "*v\t*v\t*v";
            if (humdrumDynamics && partdynamics[staffno]) {
               newstream << "\t*";
            }
            newstream << "\n";
         } else if ((tabcount[i-1] == 1) && (tabcount[i] == 2)) {
            // assume a split in the secondary part
            newstream << "*\t*^";
            if (humdrumDynamics && partdynamics[staffno]) {
               newstream << "\t*";
            }
            newstream << "\n";
         } else if ((tabcount[i-1] == 2) && (tabcount[i] == 1)) {
            // assume a join of the secondary part
            newstream << "*\t*v\t*v";
            if (humdrumDynamics && partdynamics[staffno]) {
               newstream << "\t*";
            }
            newstream << "\n";
         } else if ((tabcount[i-1] == 3) && (tabcount[i] == 0)) {
            newstream << "*v\t*v\t*v\t*v";
            if (humdrumDynamics && partdynamics[staffno]) {
               newstream << "\t*";
            }
            newstream << "\n";
         } else if ((tabcount[i-1] == 0) && (tabcount[i] == 3)) {
            newstream << "*^";
            if (humdrumDynamics && partdynamics[staffno]) {
               newstream << "\t*";
            }
            newstream << "\n";

            newstream << "*\t*^";
            if (humdrumDynamics && partdynamics[staffno]) {
               newstream << "\t*";
            }
            newstream << "\n";

            newstream << "*\t*\t*^";
            if (humdrumDynamics && partdynamics[staffno]) {
               newstream << "\t*";
            }
            newstream << "\n";
         } 

      }
      newstream << strings[i];
      if (dynamics.getSize() > 0) {
         newstream << dynamics[i];
      }
      newstream << '\n';
   }

   // delete the temporary strings:

   for (i=0; i<strings.getSize(); i++) {
      delete [] strings[i];
      strings[i] = NULL;
   }

   for (i=0; i<dynamics.getSize(); i++) {
      delete [] dynamics[i];
      dynamics[i] = NULL;
   }

}



///////////////////////////////
//
// MusicXmlFile::getKernKey --
//

char* MusicXmlFile::getKernKey(char* buffer, CXMLObject* object) {
   int fifths= 0;
   CXMLElement* element;
   object = object->Zoom();
   while (object != NULL) {
      if (object->GetType() == xmlElement) {
         element = (CXMLElement*)object;
         if (element->GetName() == "fifths") {
            fifths = getIntFromCharacterData(element->Zoom());
         }
      }
      object = object->GetNext();
   }
   
   strcpy(buffer, Convert::keyNumberToKern(fifths));
   
   return buffer;
}



//////////////////////////////
//
// MusicXmlFile::printSystemBreak -- search for an attribute called
// new-system="yes"
//

int MusicXmlFile::printSystemBreak(ostream& out, CXMLObject* object) {
   int output = 0;
   if (object == NULL) {
      return output;
   }
   if (object->GetType() != xmlElement) {
      return output;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "print") {
      return output;
   }   

   int i;
   for (i=0; i<element->GetAttributes().GetLength(); i++) {
      if (element->GetAttributes().GetName(i) == "new-system") {
         if (element->GetAttributes().GetValue(i) == 
               "yes") {
            out << "!!linebreak:original\n";
	    output++;
         }
      }
   }
   return output;
}



//////////////////////////////
//
// MusicXmlFile::printPageBreak -- search for an attribute called
// new-system="yes"
//

int MusicXmlFile::printPageBreak(ostream& out, CXMLObject* object) {
   int output = 0;
   if (object == NULL) {
      return output;
   }
   if (object->GetType() != xmlElement) {
      return output;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "print") {
      return output;
   }   

   int i;
   for (i=0; i<element->GetAttributes().GetLength(); i++) {
      if (element->GetAttributes().GetName(i) == "new-page") {
         if (element->GetAttributes().GetValue(i) == 
               "yes") {
            out << "!!pagebreak:original\n";
	    output++;
         }
      }
   }
   return output;
}

//////////////////////////////
//
// MusicXmlFile::printKernClef --
//

char* MusicXmlFile::printKernClef(char* buffer, CXMLObject* object) {
   strcpy(buffer, "*");
   
   if (object == NULL) {
      return buffer;
   }
   if (object->GetType() != xmlElement) {
      return buffer;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "clef") {
      return buffer;
   }   

   int type = -1;   // 0 = g clef, 1 = f clef, 2 = c clef
   int line = 0;
   int octavechange = 0;

   object = object->Zoom();
   XMLString string;
   CXMLObject* obj2;
   CXMLCharacterData* cdata;
   while (object != NULL) {
      if (object->GetType() == xmlElement) {
         element = (CXMLElement*)object;
         if (element->GetName() == "sign") {
           obj2 = object->Zoom();
           while (obj2 != NULL) {
              if (obj2->GetType() == xmlCharacterData) {
                 cdata = (CXMLCharacterData*)obj2;
                 string = cdata->GetData();
                 if (strchr(string.c_str(), 'G') != NULL) {
                    type = 0;
                 } else if (strchr(string.c_str(), 'F') != NULL) {
                    type = 1;
                 } else if (strchr(string.c_str(), 'C') != NULL) {
                    type = 2;
                 }
                 break;
              }
              obj2 = obj2->GetNext();
           }
         } else if (element->GetName() == "line") {
            line = getIntFromCharacterData(object->Zoom());
         } else if (element->GetName() == "clef-octave-change") {
            octavechange = getIntFromCharacterData(object->Zoom());
         } 

      }
      object = object->GetNext();
   }

   strcpy(buffer, "*clef");
   switch (type) {
      case 0: // G clef
         strcat(buffer, "G");
         break;
      case 1: // F clef
         strcat(buffer, "F");
         break;
      case 2: // C clef
         strcat(buffer, "C");
         break;
      default:
         strcat(buffer, "X");
         break;
   }
   if ((octavechange == -1) && (type == 0)) {
      strcat(buffer, "v");
   }

   char buf[32] = {0};
   sprintf(buf, "%d", line);
   
   strcat(buffer, buf);

   return buffer;
}



//////////////////////////////
//
// MusicXmlFile::getIntFromCharacterData -- gets the integer from the
//    first character data field found in the list of given objects.
//

int MusicXmlFile::getIntFromCharacterData(CXMLObject* object) {
   int output = 0;
   char buf[1024] = {0};
   XMLString string;
   CXMLCharacterData* cdata;
   while (object != NULL) {
      if (object->GetType() == xmlCharacterData) {
         cdata = (CXMLCharacterData*)object;
         string = cdata->GetData();
         strncpy(buf, string.c_str(), 512);
         removeBorderSpaces(buf);
         sscanf(buf, "%d", &output);
      }
         
      object = object->GetNext();
   }
   
   return output;
}



//////////////////////////////
//
// MusicXmlFile::printMeasureStyle -- print the orthographic nature
//    of a barline, such as repeats, dashed, invisible, etc.
//

int MusicXmlFile::printMeasureStyle(char* buffer, int staffno, int index) {
   int repeatback    = 0;
   int repeatforward = 0;
   int measureQ      = 0;   // true if barline is attached to a measure
   int iammeasure    = 0;
   int finalbar      = 0;
   int thinthin      = 0;
   int ending        = 0;
   CXMLElement*        element;
   CXMLObject*         object;
   CXMLCharacterData*  cdata;
   CXMLObject*         obj2;
   XMLString           string;
   int i;

   buffer[0] = '\0';

   if (partdata[staffno][index].type == MXI_measure) {
      measureQ = 1;
      iammeasure = 1;
   }

   // check the current item if a barline rather than a measure
   if (!iammeasure) {
      if (partdata[staffno][index].type == MXI_barline) {
         object = partdata[staffno][index].obj;
         object = object->Zoom();
         while (object != NULL) {
            if (object->GetType() == xmlElement) {
               element = (CXMLElement*)object;
               if (element->GetName() == "repeat") {
                  for (i=0; i<element->GetAttributes().GetLength(); i++) {
                     if (element->GetAttributes().GetName(i) == "direction") {
                        if (element->GetAttributes().GetValue(i) == 
                              "backward") {
                           repeatback = 1;
                        }
                     }
                  }
               } else if (element->GetName() == "bar-style") {
                  obj2 = object->Zoom();
                  while (obj2 != NULL) {
                     if (obj2->GetType() == xmlCharacterData) {
                        cdata = (CXMLCharacterData*)obj2;
                        string = cdata->GetData();
                        if (strstr(string.c_str(), "light-heavy") != NULL) {
                           finalbar = 1;
                        } else if (strstr(string.c_str(), "light-thin") 
                              != NULL) {
                           thinthin = 1;
                        }
                        break;
                     }
                     obj2 = obj2->GetNext();
                  }
               }
            }
            object = object->GetNext();
         }
      }
   }

   // check the previous item if a barline
   if (index - 1 >= 0) {
      if (partdata[staffno][index-1].type == MXI_measure) {
         measureQ = 1;
      }
      if (partdata[staffno][index-1].type == MXI_barline) {
         object = partdata[staffno][index-1].obj;
         object = object->Zoom();
         while (object != NULL) {
            if (object->GetType() == xmlElement) {
               element = (CXMLElement*)object;
               if (element->GetName() == "repeat") {
                  for (i=0; i<element->GetAttributes().GetLength(); i++) {
                     if (element->GetAttributes().GetName(i) == "direction") {
                        if (element->GetAttributes().GetValue(i) == 
                              "backward") {
                           repeatback = 1;
                        }
                     }
                  }
               } else if (element->GetName() == "bar-style") {
                  obj2 = object->Zoom();
                  while (obj2 != NULL) {
                     if (obj2->GetType() == xmlCharacterData) {
                        cdata = (CXMLCharacterData*)obj2;
                        string = cdata->GetData();
                        if (strstr(string.c_str(), "light-heavy") != NULL) {
                           finalbar = 1;
                        } else if (strstr(string.c_str(), "light-light") 
                              != NULL) {
                           thinthin = 1;
                        }
                        break;
                     }
                     obj2 = obj2->GetNext();
                  }
               }

            }
            object = object->GetNext();
         }
      }
   }

   // check the next item if a barline
   if (index + 1 < partdata[staffno].getSize()) {
      if (partdata[staffno][index+1].type == MXI_measure) {
         measureQ = 1;
      }
      if (partdata[staffno][index+1].type == MXI_barline) {
         object = partdata[staffno][index+1].obj;
         object = object->Zoom();
         while (object != NULL) {
            if (object->GetType() == xmlElement) {
               element = (CXMLElement*)object;
               if (element->GetName() == "repeat") {
                  for (i=0; i<element->GetAttributes().GetLength(); i++) {
                     if (element->GetAttributes().GetName(i) == "direction") {
                        if (element->GetAttributes().GetValue(i) == "forward") {
                           repeatforward = 1;
                        }
                     }
                  }
               }
            }
            object = object->GetNext();
         }
      }
   } else {
      ending = 1;
   }

   // if a measure is present, and this particular item is not a measure
   // then return without doing anything.
   if (!iammeasure) {
      if (measureQ == 0) {
         // continue on and print the non-controlling barline
      } else {
         return 0;
      }
   }

   if (!measureQ) { // a non-controlling barline needs to be printed
      strcat(buffer, "=");
   } 

   if (finalbar && ending) {
      strcat(buffer, "=");
   }

   if (thinthin) {
      strcat(buffer, "||");
   }

   if (repeatforward && repeatback) {
      strcat(buffer, ":|!|:");
      return !measureQ;
   }

   if (repeatforward) {
      strcat(buffer, "!|:");
      return !measureQ;
   }

   if (repeatback) {
      strcat(buffer, ":|!");
      return !measureQ;
   }

   return !measureQ;
}



//////////////////////////////
//
// MusicXmlFile::getKernMetronome --
//

char* MusicXmlFile::getKernMetronome(char* buffer, CXMLObject* object) {
   buffer[0] = '\0';
   if (object == NULL) {
      return buffer;
   }
   if (object->GetType() != xmlElement) {
      return buffer;
   }

   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "sound") {
      return buffer;
   }

   int i;
   XMLString string;
   int tempo = 120;
   for (i=0; i<element->GetAttributes().GetLength(); i++) {
      if (element->GetAttributes().GetName(i) == "tempo") {
         string = element->GetAttributes().GetValue(i);
         sscanf(string.c_str(), "%d", &tempo);
         break;
      }
   }
   
   sprintf(buffer, "*MM%d", tempo);

   return buffer;
}



//////////////////////////////
//
// MusicXmlFile::getKernTimeMet -- get the display format of the time signature.
//

char* MusicXmlFile::getKernTimeMet(char* buffer, CXMLObject* object) {
   buffer[0]= '\0';
   if (object == NULL) { 
      return buffer;
   }
   if (object->GetType() != xmlElement) {
      return buffer;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "time") {
      return buffer;
   }

   int i;
   for (i=0; i<element->GetAttributes().GetLength(); i++) {
      if (element->GetAttributes().GetName(i) == "symbol") {
         if (element->GetAttributes().GetValue(i) == "common") {
            strcpy(buffer, "*met(c)");
            return buffer;
         } else if (element->GetAttributes().GetValue(i) == "cut") {
            strcpy(buffer, "*met(c|)");
            return buffer;
         } 
      }
   }

   // strcpy(buffer, "Unknown met");
   return buffer;
}



//////////////////////////////
//
// MusicXmlFile::getKernTimeSig --
//

char* MusicXmlFile::getKernTimeSig(char* buffer, CXMLObject* object) {
   int top = 5;
   int bottom = 5;
   if (object == NULL) {
      strcpy(buffer, "*");
      return buffer;
   }

   if (object->GetType() != xmlElement) {
      strcpy(buffer, "*");
      return buffer;
   }

   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "time") {
      strcpy(buffer, "*");
      return buffer;
   }

   object = object->Zoom();
   CXMLObject* obj2;
   XMLString string;
   CXMLCharacterData* cdata;
   char buf[1024] = {0};
   while (object != NULL) {
      if (object->GetType() == xmlElement) {
         element = (CXMLElement*)object;

         if (element->GetName() == "beats") {
            // cout << "FOUND TIME SIGNATURE BEATS ENTRY" << endl;
            obj2 = object->Zoom();
            while (obj2 != NULL) {
               if (obj2->GetType() == xmlCharacterData) {
                  cdata = (CXMLCharacterData*)obj2;
                  string = cdata->GetData();
                  strncpy(buf, string.c_str(), 512);
                  removeBorderSpaces(buf);
                  sscanf(buf, "%d", &top);
               }
               obj2 = obj2->GetNext();
            }
         } 

         if (element->GetName() == "beat-type") {
            obj2 = object->Zoom();
            while (obj2 != NULL) {
               if (obj2->GetType() == xmlCharacterData) {
                  cdata = (CXMLCharacterData*)obj2;
                  string = cdata->GetData();
                  strncpy(buf, string.c_str(), 512);
                  removeBorderSpaces(buf);
                  sscanf(buf, "%d", &bottom);
                  // cout << "TimeSig Bottom: " << bottom << endl;
               }
               obj2 = obj2->GetNext();
            }
         } 

      }
      object = object->GetNext();
   }

   sprintf(buffer, "*M%d/%d", top, bottom);
   return buffer;
}



//////////////////////////////
//
// MusicXmlFile::getNextVoiceTime -- Get the absolute tick time of the
//    next note item in this voice.  Return the measure boundary
//    if no other note for this voice left in the measure.
//

int MusicXmlFile::getNextVoiceTime(int staffno, int index, int voice) {
   int output = 0;
   index++;
   int pvoice = partdata[staffno][index].voice;
   if (pvoice > 4) {  // temporary code for SharpEye piano parts.
      pvoice -= 4;
   }
   while (index < partdata[staffno].getSize()) {
      if (partdata[staffno][index].type == MXI_note) {
         if (pvoice == voice) {
            return partdata[staffno][index].ticktime;
         }
      } else if (partdata[staffno][index].type == MXI_measure) {
         return partdata[staffno][index].ticktime;
      } else if (partdata[staffno][index].type == MXI_barline) {
         return partdata[staffno][index].ticktime;
      }
      index++;
   }

   int size = partdata[staffno].getSize();
   if (output == 0) {
      if (partdata[staffno][size-1].type == MXI_note) {
         return partdata[staffno][size-1].ticktime + 
                partdata[staffno][size-1].tickdur;
      } else {
         return partdata[staffno][size-1].ticktime;
      }
   }

   return output;
}



//////////////////////////////
//
// MusicXmlFile::interpextenttick -- get the interpreted (padded) rest
//    for the voice (1) and return the amount of ticked used from the
//    input in the final decision of what duration of rest to use.
//
// Note: the algorithm will probably need to be made more sophisicated.
//

int MusicXmlFile::getInterpRestDuration(char* interpbuffer, int interptick,
      int divisions) {
    // try to create a rhythm out of the entire duration:
    Convert::durationToKernRhythm(interpbuffer, 
          (double)interptick/divisions);

    // tests now need to be done in order to verify that the interpreted
    // rhythm is correct. Perhaps this rule:  if the duration is not
    // printable, then check to see if the duration is the same length
    // or longer that the duration of the voice 2 note which is being
    // currently processed.  If so, then borrow that rhythm, and return
    // the amount of the total need ticks to the calling function...

    return interptick;
}



//////////////////////////////
//
// MusicXmlFile::getKernNoteProperties --
//

void MusicXmlFile::getKernNoteProperties(_NoteState& ns, CXMLObject* object, 
      int tickdur) {
   ns.clear();
   if (object == NULL) {
      return;
   }
   if (object->GetType() != xmlElement) {
      return;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "note") {
      return;
   }

   int i;

   if (tickdur == 0) {
      ns.gracenote = 1;
   }

   char buffer[1024] = {0};
   object = object->Zoom();
   CXMLObject* obj2 = NULL;
   CXMLObject* obj3 = NULL;
   CXMLElement* elem2 = NULL;
   CXMLElement* elem3 = NULL;
   CXMLCharacterData* cdata;
   XMLString string;
   while (object != NULL) {
      if (object->GetType() != xmlElement) {
         object = object->GetNext();
         continue;
      }
      element = (CXMLElement*)object;

      if (element->GetName() == "chord") {
         ns.chordnote = 1;
      }

      if (element->GetName() == "tie") {
         for (i=0; i<element->GetAttributes().GetLength(); i++) {
            if (element->GetAttributes().GetName(i) == "type") {
               if (element->GetAttributes().GetValue(i) == "start") {
                  ns.tiestart = 1;
               } else if (element->GetAttributes().GetValue(i) == "stop") {
                  ns.tieend = 1;
               }
            }
         }
      }

      // check for natural sign:
      if (element->GetName() == "accidental") {

         for (i=0; i<element->GetAttributes().GetLength(); i++) {
            if (element->GetAttributes().GetName(i) == "parentheses") {
               if (element->GetAttributes().GetValue(i) == "yes") {
                  ns.ficta = 1;
               } 
            }
         }

         obj2 = element->Zoom();
         while (obj2 != NULL) {
            if (obj2->GetType() == xmlCharacterData) {
               cdata = (CXMLCharacterData*)obj2;
               string = cdata->GetData();
               strncpy(buffer, string.c_str(), 512);
               removeBorderSpaces(buffer);
               if (strcmp(buffer, "natural") == 0) {
                  ns.natural = 1;
                  break;
               }
            }
            obj2 = obj2->GetNext();
         }
      }

      if (element->GetName() == "notations") {
         // could be a slur or an articulation/technical
         int type = -1;   // 0 = slur start, 1 = slur stop
         int number = 0;  // the slur level
         obj2 = object->Zoom();
         while (obj2 != NULL) {
            if (obj2->GetType() != xmlElement) {
               obj2 = obj2->GetNext();
               continue;
            }
            elem2 = (CXMLElement*)obj2;
            if (elem2->GetName() == "fermata") {
               ns.fermata = 1;
            }
            if (elem2->GetName() == "slur") {
               for (i=0; i<elem2->GetAttributes().GetLength(); i++) {
                  if (elem2->GetAttributes().GetName(i) == "type") {
                     if (elem2->GetAttributes().GetValue(i) == "start") {
                        type = 0;
                     } else if (elem2->GetAttributes().GetValue(i) == "stop") {
                        type = 1;
                     }
                  } else if (elem2->GetAttributes().GetName(i) == "number") {
                     if (elem2->GetAttributes().GetValue(i) == "1") {
                        number = 1;
                     } else if (elem2->GetAttributes().GetValue(i) == "2") {
                        number = 2;
                     }
                     // ignoring slurs of more than level 2 for now...
                  }
               }
               if (type == 0) {
                  ns.slurstart = 1;
               }
               if (type == 1) {
                  ns.slurend = 1;
               }
	       /*
               if ((type == 0) && (number == 1)) {
                  ns.slurstart = 1;
               }
               if ((type == 1) && (number == 1)) {
                  ns.slurend = 1;
               }
               if ((type == 1) && (number == 2)) {
		  // doesn't work so well: 2 could just mean a separate voice
                  // ns.elidedslurstart = 1;
               }
               if ((type == 1) && (number == 2)) {
		  // doesn't work so well: 2 could just mean a separate voice
                  // ns.elidedslurend = 1;
               }
	       */
            } else if (elem2->GetName() == "articulations") {
               obj3 = elem2->Zoom();
               while (obj3 != NULL) {
                  if (obj3->GetType() != xmlElement) {
                     obj3 = obj3->GetNext();
                     continue;
                  }
                  elem3 = (CXMLElement*)obj3;
                  if (elem3->GetName() == "staccato") {
                     ns.staccato = 1;
                  } else if (elem3->GetName() == "tenuto") {
                     ns.tenuto = 1;
                  } else if (elem3->GetName() == "staccatissimo") {
                     ns.staccatissimo = 1;
                  } else if (elem3->GetName() == "accent") {
                     ns.accent = 1;
                  }
                  obj3 = obj3->GetNext();
               }
            } else if (elem2->GetName() == "technical") {
               obj3 = elem2->Zoom();
               while (obj3 != NULL) {
                  if (obj3->GetType() != xmlElement) {
                     obj3 = obj3->GetNext();
                     continue;
                  }
                  elem3 = (CXMLElement*)obj3;
                  if (elem3->GetName() == "up-bow") {
                     ns.upbow = 1;
                  } else if (elem3->GetName() == "down-bow") {
                     // ns.downbow = 1;
                  }
                  obj3 = obj3->GetNext();
               }
            }
            obj2 = obj2->GetNext();
         }
      } // end of if "notations"

      if (element->GetName() == "beam") {
         obj2 = object->Zoom();
         while (obj2 != NULL) {
            if (obj2->GetType() == xmlCharacterData) {
               cdata = (CXMLCharacterData*)obj2;
               string = cdata->GetData();
               if (strstr(string.c_str(), "begin") != NULL) {
                  ns.startbeam++;
               }
               if (strstr(string.c_str(), "end") != NULL) {
                  ns.endbeam++;
               }
               if (strstr(string.c_str(), "backward hook") != NULL) {
                  ns.hookleft++;
               }
               if (strstr(string.c_str(), "forward hook") != NULL) {
                  ns.hookright++;
               }
            }
            obj2 = obj2->GetNext();
         }
      }  // end of if "beam"


      if (element->GetName() == "stem") {
         obj2 = object->Zoom();
         while (obj2 != NULL) {
            if (obj2->GetType() == xmlCharacterData) {
               cdata = (CXMLCharacterData*)obj2;
               string = cdata->GetData();
               if (string == "down") {
                  ns.stemdir = -1;
               } else if (string == "up") {
                  ns.stemdir = +1;
               }
               break;
            } 
            obj2 = obj2->GetNext();
         }
      } // end of if "stem"

      object = object->GetNext();
   } // close of: while (object != NULL)

}



/* Canonical Kern token signifier ordering:

	   signified			signifier(s)	comments
	1. open phrase elision indicator	&	must precede {
	2. open phrase mark			{
	3. open slur elision indicator		&	must precede (
	4. open slur				(
	5. open tie				[
	6. duration		0123456789	any combination; signifiers
						may be repeated
	7. augmentation dot(s)		.	signifier may be repeated
	8. pitch or rest	abcdefgrABCDEFG	only one of; signifier may
						be repeated
	9. accidental		- or # or n	- and # may be repeated
	10. glissando		h or H
	11. harmonic		o
	12. pause		;
	13. ornament		MmS$TtWwR or O	O precludes others; no
						repetition of a given signifier;
						must appear in order given
	14. appoggiatura designator	p or P
	15. acciaccatura designator	q
	16. groupetto designator	Q
	17. articulation	Usz'"`~^: or I	I precludes others; no
						repetition of a given signifier;
						must appear in order given
	18. bowing		u or v	only one of
	19. stem-direction	/ or \ 	only one of
	20. beaming		L or J	signifiers may be repeated
	21. partial beaming	k or K	signifiers may be repeated
	22. user-defined marks	ijl		one or more of;
				NVZ		may be repeated but
				@ % + < >	must be in order given
	23. closed or continuing tie		] or _
	24. closed slur elision indicator	&	must precede )
	25. closed slur	)
	26. closed phrase elision indicator	&	must precede }
	27. closed phrase mark	}
	28. breath mark	,
	29. editorial marks	xx or XX
*/



//////////////////////////////
//
// MusicXmlFile::printKernNote --
//

int MusicXmlFile::printKernNote(ostream& out, int staffno, int index, 
      int divisions) {

   _NoteState cs;

   getKernNoteProperties(cs, partdata[staffno][index].obj, 
      partdata[staffno][index].tickdur);

   if (cs.chordnote) {
      _NoteState ls;
      getKernNoteProperties(ls, partdata[staffno][index].chordhead, 0);
      cs.incorporate(ls);
   }

   ///////////////////////////////////////////////////////////////////////
   // 
   // found out all of the information about the kern note, now print
   // the components according to the canonical order.
   //

   int interprestQ = 0;
   char interpbuffer[128] = {0};
   int interpextenttick = 0;
   int interptick = 0;

   // first, check if appending null tokens is necessary for multiple voices.
   // can only handle two voices right now...
   int voice;
   int m;
   int lastnoteindex = 0;
   if (index > 0) {
   // out << "{" << partdata[staffno][index].ticktime << "}";
      voice = partdata[staffno][index].voice;
      if (voice > 4) {   // temporary method for SharpEye piano part parsing
         voice -= 4;
      }
      lastnoteindex = -1;
      for (m=index-1; m>=0; m--) {
         if (partdata[staffno][m].type == MXI_note) {
            lastnoteindex = m;
            break;
         }
      }
      
      if ((voice > 1) && ((lastnoteindex < 0) || 
                           (partdata[staffno][lastnoteindex].ticktime != 
                            partdata[staffno][index].ticktime))) {

         if (voice == 2) {
            // print an interpreted rest as a place holder if
            // the first voice is not sounding at this time
            if (partdata[staffno][index].ticktime >=
                partvoiceticktime1) {
               interptick = getNextVoiceTime(staffno, index, 1) - 
                            partdata[staffno][index].ticktime;
               interpextenttick = getInterpRestDuration(interpbuffer, 
                                   interptick, 
                                   partdata[staffno][index].divisions);
               partvoiceticktime1 = partdata[staffno][index].ticktime +
                                    interpextenttick;
               out << interpbuffer << "ryy";
               interprestQ = 1;
            }
         } else {
            out << ".{" << partdata[staffno][index].voice << "}";
         }
         if (interprestQ) {
            out << "\t";
         } else {
            out << ".\t";
         }
      } 
   }

   // canonical item #1: open elided phrase marker
   if (cs.elidedphrasestart) {    
      out << "&{";
   }

   // canonical item #2: open phrase marker
   if (cs.phrasestart) {    
      out << "{";
   }

   // canonical item #3: open elided slur marker
   if (cs.elidedslurstart) {    
      out << "&(";
   }

   // canonical item #4: open slur marker
   if (cs.slurstart) {
      out << "(";
   }

   // canonical item #5: open tie marker
   if (cs.tiestart && !cs.tieend) {
      out << "[";
   }

   // canonical item #6: duration marker
   // canonical item #7: augmentation dots
   double duration = (double)partdata[staffno][index].tickdur/divisions;
   char buffer[1024] = {0};
   if (duration > 0) {
      // print duration only if not a gracenote.
      out << Convert::durationToKernRhythm(buffer, duration);
   } else {
       // print duration even if a grace note (used for visual display)
       printGraceNoteRhythm(out, partdata[staffno][index]);
   }

   // canonical item #8: pitch or rest marker
   // canonical item #9: accidental
   if (partdata[staffno][index].pitch <= 0) {
      out << "r";
   } else {
      out << Convert::base40ToKern(buffer, partdata[staffno][index].pitch);
   }

   // print any explicit natural signs
   if (cs.natural) {
      out << "n";
   }

   // print any ficta marking
   if (cs.ficta) {
      out << "i";
   }

   // canonical item #10: glissando marker

   // canonical item #11: harmonic marker

   // canonical item #12: pause marker

   // canonical item #13: ornament marker
   if (cs.staccato) {
      out << "\'";
   }
   if (cs.tenuto) {
      out << "~";
   }
   if (cs.staccatissimo) {
      out << "`";
   }
   if (cs.accent) {
      out << "^";
   }
   if (cs.fermata) {
      out << ";";
   }

   // canonical item #14: appogiatura marker

   // canonical item #15: acciaccatura marker
   if (cs.gracenote) {
      printGraceMarker(out, partdata[staffno][index]);
   }

   // canonical item #16: groupetto marker

   // canonical item #17: articulation marker

   // canonical item #18: bowing marker
   if (cs.upbow) {
      out << "v";
   }
   if (cs.downbow) {
      out << "u";
   }

   // canonical item #19: stem direction marker (if desired)
   if (cs.stemdir && getOption("stem")) {
      if (cs.stemdir < 0) {
         out << "\\";
      } else if (cs.stemdir > 0) {
         out << "/";
      }
   }

   // canonical item #20: beaming marker
   if (getOption("beam")) {
      int ii;
      for (ii=0; ii<cs.startbeam; ii++) {
         out << "L";
      }
      for (ii=0; ii<cs.endbeam; ii++) {
         out << "J";
      }
      // canonical item #21: partial beaming marker
      for (ii=0; ii<cs.hookleft; ii++) {
         out << "k";
      }
      for (ii=0; ii<cs.hookright; ii++) {
         out << "K";
      }
   }

   // canonical item #22: user-defined marks


   // canonical item #23: closed or continuing tie mark
   if (cs.tiestart && cs.tieend) {
      out << "_";
   } else if (cs.tieend && !cs.tiestart) {
      out << "]";
   }

   // canonical item #24: closed elision slur marker
   if (cs.elidedslurend) {
      out << "&)";
   }

   // canonical item #25: closed slur
   if (cs.slurend) {
      out << ")";
   }

   // canonical item #26: closed elision phrase marker
   if (cs.phraseend) {
      out << "&}";
   }

   // canonical item #27: closed phrase marker
   if (cs.elidedphraseend) {
      out << "}";
   }


   // 
   //  end of canonical kern markers.
   // 
   ////////////////////////////////////////////////////////////////////////

   // determine what to print after note: space, tab, null token, or newline.
   int newlineQ = 1;
   if (cs.gracenote) {
      newlineQ = 1;  // no grace chords allowed at this time.
                     // allow in multiple voices later ...
   } else if (index+1 < partdata[staffno].getSize()) {
      if ((partdata[staffno][index+1].type == MXI_note)) {
         if (partdata[staffno][index+1].ticktime == 
               partdata[staffno][index].ticktime) {
            if (partdata[staffno][index+1].voice == 
                partdata[staffno][index].voice) {
               // the next note is a chord note
               out << " ";
	       newlineQ = 0;
            } else {
               // the next note is in another voice in part
               out << "\t";
	       newlineQ = 0;
            }
         } else {
            newlineQ = 1;
         }
      } else {
         newlineQ = 1;
      }
   } else {
      newlineQ = 1;
   }

   // print a newline if necessary, but print a null token for the 
   // second voice if it is currently active.
   if (newlineQ) {
      if (partdata[staffno][index].voice == 1) {
         if (partdata[staffno][index].ticktime < partvoiceticktime2) {
            out << "\t.";
         }
      } 

      // out << "\n";  // print newline in calling function...
   }

   if (partdata[staffno][index].voice == 2) {
      partvoiceticktime2 = partdata[staffno][index].ticktime + 
                           partdata[staffno][index].tickdur;
   } else if (partdata[staffno][index].voice == 1) {
      partvoiceticktime1 = partdata[staffno][index].ticktime + 
                           partdata[staffno][index].tickdur;
   }

   return newlineQ;
}



//////////////////////////////
//
// MusicXmlFile::getDivisions -- 
//

int MusicXmlFile::getDivisions(CXMLObject* object) {
   int output = 0;
   if (object == NULL) {
      return output;
   }
   if (object->GetType() != xmlElement) {
      return output;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "divisions") {
      return output;
   }

   object = object->Zoom();

   XMLString string;
   CXMLCharacterData* cdata;
   char buffer[1024] = {0};
   while (object != NULL) {
      if (object->GetType() == xmlCharacterData) {
         cdata = (CXMLCharacterData*)object;
         string = cdata->GetData();
         strncpy(buffer, string.c_str(), 512);
         removeBorderSpaces(buffer);
         sscanf(buffer, "%d", &output);
         // cout << "Found divisions: " << output << endl;
         break;
      } 
      object = object->GetNext();
   }

   return output;
}



//////////////////////////////
//
// MusicXmlFile::getMeasureNumber -- get the Measure number of a
//   measure entry in an XML file.
//

int MusicXmlFile::getMeasureNumber(CXMLObject* object) {
   int output = 0;
   if (object == NULL) {
      return output;
   }
   if (object->GetType() != xmlElement) {
      return output;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "measure") {
      return output;
   }

   int i;
   XMLString string;
   char buffer[1024] = {0};
   for (i=0; i<element->GetAttributes().GetLength(); i++) {
      if (element->GetAttributes().GetName(i) == "number") {
         string = element->GetAttributes().GetValue(i); 
         strncpy(buffer, string.c_str(), 1024);
         sscanf(buffer, "%d", &output);
         break;
      }
   }

   return output;
}



//////////////////////////////
//
// MusicXmlFile::fixMidiAccidentals -- fix accidentals so that
//    voice leading melodic lines look reasonably spelled.
//

void MusicXmlFile::fixMidiAccidentals(void) {
   int i;
   int j;

   int note = -1;
   int lnote = -1;
   int llnote = -1;
   int index = 0;
   int lindex = 0;
   int llindex = 0;
   
   for (i=0; i<partdata.getSize(); i++) {
      note = lnote = llnote = -1;
      index = lindex = llindex = 0;
      for (j=0; j<partdata[i].getSize(); j++) {
         if (partdata[i][j].type == MXI_note) {
            if (partdata[i][j].pitch <= 0) {
               continue;
            }
            llnote = lnote;   llindex = lindex;
            lnote = note;     lindex  = index;
            note = partdata[i][j].pitch; index = j;
            if ((note - lnote == 1) && (lnote - llnote == -1)) {
               // e.g.: D-Db-D => D-C#-D
               partdata[i][lindex].pitch += -4;
            } else if ((note - lnote == -10) && (lnote - llnote == -1)) {
               // e.g.: D-Db-B => D-C#-B
               partdata[i][lindex].pitch += -4;
            } else if ((note - lnote == 1) && (lnote - llnote == 10)) {
               // e.g.: B-Db-D => B-C#-D
               partdata[i][lindex].pitch += -4;
            } else if ((note - lnote == 7) && (lnote - llnote == -1)) {
               // e.g.: D-Db-E => D-C#-E
               partdata[i][lindex].pitch += -4;
            } else if ((note - lnote == 1) && (lnote - llnote == 16)) {
               // e.g.: A-Db-D => A-C#-D
               partdata[i][lindex].pitch += -4;
            } else if ((note - lnote == 1) && (lnote - llnote == -7)) {
               // e.g.: E-Db-D => E-C#-D
               partdata[i][lindex].pitch += -4;
            }

         }
      }
   }
}



//////////////////////////////
//
// MusicXmlFile::createHumdrumFile --
//

HumdrumFile& MusicXmlFile::createHumdrumFile(HumdrumFile& hfile) {
   hfile.clear();

   int count = getStaffCount();
   if (count <= 0) {
      return hfile;
   }

   HumdrumFile* partfiles;
   partfiles = new HumdrumFile[count];

   int i;
   // most MusicXML files start with highest part first, and then
   // progress towards the lowest part, so assemble the parts
   // in reverse order to follow the Humdrum convention of 
   // encoding the scores from lowest to hightest pitch ranges
   // in the score.
   for (i=0; i<count; i++) {
      humdrumPart(partfiles[count-i-1], i);
   }
   HumdrumFile::assemble(hfile, count, partfiles);

   delete [] partfiles;
   return hfile;
}



//////////////////////////////
//
// MusicXmlFile::setOption --
//

void MusicXmlFile::setOption(const char* key, int value) {
   if (strncmp(key, "stem", 4) == 0) {
      humdrumStem = value;
   } else if (strncmp(key, "beam", 4) == 0) {
      humdrumBeam = value;
   } else if (strncmp(key, "dynamic", 7) == 0) {
      humdrumDynamics = value;
   } else if (strncmp(key, "lyric", 5) == 0) {
      humdrumLyrics = value;
   }
}



//////////////////////////////
//
// MusicXmlFile::getOption --
//

int MusicXmlFile::getOption(const char* key) {
   if (strncmp(key, "stem", 4) == 0) {
      return humdrumStem;
   }

   if (strncmp(key, "beam", 4) == 0) {
      return humdrumBeam;
   }

   if (strncmp(key, "dynamic", 7) == 0) {
      return humdrumDynamics;
   }

   if (strncmp(key, "lyric", 5) == 0) {
      return humdrumLyrics;
   }

   return 0;
}



//////////////////////////////
//
// MusicXmlFile::getMeasureTickDuration -- determine the tick duration
//    of a measure according to this algorithm:
// (1) store the absolute tick position of the current measure starting point.
// (2) as each note is processed, keep track of the maximum tick duration
//     from the starting point of the measure which each note duration
//     produces. (The starting tick position of all notes in the measure
//     can still be calculated by the previous algorithm).
// (3) when the next measure is encountered add the value stored in step (1)
//     to the maximum measure duration value determined in step (2).
//     This sum is equal to the absolute tick position of the starting
//     point of the next measure.
//

int MusicXmlFile::getMeasureTickDuration(CXMLObject* object) {
   if (object == NULL) {
      return 0;
   }
   if (object->GetType() != xmlElement) {
      return 0;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "measure") {
      return 0;
   }
  
   int maxtick  = 0;
   int tickpos  = 0;
   int duration = 0;

   object = object->Zoom();
   while (object != NULL) {
      if (object->GetType() != xmlElement) {
         object = object->GetNext();
         continue;
      }
      element = (CXMLElement*)object;
      if (element->GetName() == "note") {
         duration = getNoteTickDuration(element);
      } else if (element->GetName() == "backup") {
         duration = getBackupTickDuration(element);
         duration = -duration;
      } else if (element->GetName() == "forward") {
         duration = getForwardTickDuration(element);
      } else {
         duration = 0;  // a non-durational item must be here
      }

      tickpos += duration;
      if (tickpos > maxtick) {
        maxtick = tickpos;
      }

      object = object->GetNext();
   }

   return maxtick;
}



//////////////////////////////
//
// MusicXmlFile::getBackupTickDuration --
//

int MusicXmlFile::getBackupTickDuration(CXMLObject* object) {
   if (object == NULL) {
      return 0;
   }
   if (object->GetType() != xmlElement) {
      return 0;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "backup") {
      return 0;
   }

   int output = 0;
   object = object->Zoom();
   while (object != NULL) {
      if (object->GetType() != xmlElement) {
         object = object->GetNext();
         continue;
      }
      element = (CXMLElement*)object;
      if (element->GetName() == "duration") {
         output = getIntFromCharacterData(element->Zoom());
         break;
      }
      object = object->GetNext();
   }

   return output;
}



//////////////////////////////
//
// MusicXmlFile::getForwardTickDuration --
//

int MusicXmlFile::getForwardTickDuration(CXMLObject* object) {
   if (object == NULL) {
      return 0;
   }
   if (object->GetType() != xmlElement) {
      return 0;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "forward") {
      return 0;
   }

   int output = 0;
   object = object->Zoom();
   while (object != NULL) {
      if (object->GetType() != xmlElement) {
         object = object->GetNext();
         continue;
      }
      element = (CXMLElement*)object;
      if (element->GetName() == "duration") {
         output = getIntFromCharacterData(element->Zoom());
         break;
      }
      object = object->GetNext();
   }

   return output;
}



//////////////////////////////
//
// MusicXmlFile::getNoteTickDuration -- return 0 duration if the note 
//    contains a <chord> element.  <note> with <chord> elements
//    do not increment the timeline.  All <note> containing a
//    <chord> element must have the same duration as the
//    first note to which the other chord notes are attached.
//    Otherwise, funny things will start to happen...
//    Also, if a note contains a <grace> element, the note is
//    automatically assumed to have no duration.
//

int MusicXmlFile::getNoteTickDuration(CXMLObject* object) {
   if (object == NULL) {
      return 0;
   }
   if (object->GetType() != xmlElement) {
      return 0;
   }
   CXMLElement* element = (CXMLElement*)object;
   if (element->GetName() != "note") {
      return 0;
   }
   int output = 0;

   object = object->Zoom();
   while (object != NULL) {
      if (object->GetType() != xmlElement) {
         object = object->GetNext();
         continue;
      }
      element = (CXMLElement*)object;
      if (element->GetName() == "duration") {
         output = getIntFromCharacterData(element->Zoom());
      } else if (element->GetName() == "chord") {
         output = 0;
         break;
      } else if (element->GetName() == "grace") {
         output = 0;
         break;
      }
      object = object->GetNext();
   }

   return output;
}


// md5sum: cc71b3485439d292a48dbc45ac8bec89 MusicXmlFile.cpp [20030102]

