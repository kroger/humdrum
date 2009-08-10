//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat May  1 16:20:50 PDT 2004
// Last Modified: Sat May  1 16:20:52 PDT 2004
// Last Modified: Wed May 27 01:00:49 PDT 2009 (fixed out filename in cwd)
// Filename:      ...museinfo/examples/all/thememakerx.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/thememakerx.cpp
// Syntax:        C++; museinfo
//
// Description:   Extract incipits from monophonic Humdrum files which contain
//                phrase markings (preferrably).
//

#include "humdrum.h"

#ifndef OLDCPP
   #include <iostream>
   #include <fstream>
   using namespace std;
#else
   #include <iostream.h>
   #include <fstream.h>
#endif

// includes needed for file/directory processing:
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>


// function declarations:
void      checkOptions           (Options& opts, int argc, char** argv);
void      example                (void);
void      usage                  (const char* command);
void      createIncipit          (HumdrumFile& hfile, const char* filename,
                                  const char* sourcebase, int limit);
void      extractPitchSequence   (Array<int>& pitches, Array<int>& phrase, 
                                  HumdrumFile& hfile);
int       is_directory           (const char* path);
int       is_file                (const char* path);
void      processArgument        (const char* path);
void      createOutputName       (char* outfile, const char* filename, 
                                  const char* target, const char* sourcebase);
void      checkTargetDirectory   (const char* outfile);
void      getPhrasesAndNoteLines (Array<int>& lines, Array<int>& phrase, 
                                  HumdrumFile& hfile);
int       getEndingLine          (Array<int>& lines, Array<int>& phrase, 
                                  HumdrumFile& hfile, int limit);

// User interface variables:
Options   options;
int       debugQ  = 0;         // used with --debug option
int       limitQ  = 0;         // used with -l option
int       limit   = 30;        // used with -l option
int       minval  = 10;        // used with -m option
const char* target = ".";        // used with -b option
char sourcebase[2048] = {0};


//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   checkOptions(options, argc, argv); // process the command-line options

   int i;
   int numinputs = options.getArgCount();
   HumdrumFile hfile;
   strcpy(sourcebase, ".");

   for (i=0; i<numinputs || i==0; i++) {
      hfile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         hfile.read(cin);
         createIncipit(hfile, "", sourcebase, limit);
      } else {
         if (is_directory(options.getArg(i+1))) {
            strcpy(sourcebase, options.getArg(i+1));
         }
         processArgument(options.getArg(i+1));
      }
   }

   return 0;
}


//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// processArgument -- check if the argument is a file or a directory.
//    If a directory, then process all files/subdirectories in it.
//

void processArgument(const char* path) {
   HumdrumFile hfile;
   DIR* dir = NULL;
   char* fullname;
   struct dirent* entry;
   int namelen = 0;
   int valid = 0;

   if (is_file(path)) {
      namelen = strlen(path);
      valid = 0;
      if (strcmp(&(path[namelen-4]), ".krn") == 0) {
         valid = 1;
      } else if (strcmp(&(path[namelen-4]), ".KRN") == 0) {
         valid = 1;
      }
      if (!valid) {
         return;
      }
      hfile.read(path);
      createIncipit(hfile, path, sourcebase, limit);
   } else if (is_directory(path)) {
      dir = opendir(path);
      if (dir == NULL) {
         return;
      }
      entry = readdir(dir);
      while (entry != NULL) {
         if (strncmp(entry->d_name, ".", 1) == 0) {
            entry = readdir(dir);
            continue;
         }

         fullname = new char[strlen(path) + 1 + strlen(entry->d_name) + 1];
         strcpy(fullname, path);
         strcat(fullname, "/");
         strcat(fullname, entry->d_name);
         processArgument(fullname);
         entry = readdir(dir);
      }
   }

   closedir(dir);
}



//////////////////////////////
//
// is_file -- returns true if the string is a file.
//

int is_file(const char* path) {
   struct stat filestat;
   if (stat(path, &filestat)) {
      return 0;
   }
   return S_ISREG(filestat.st_mode);
}



//////////////////////////////
//
// is_directory -- returns true if the string is a directory.
//

int is_directory(const char* path) {
   struct stat filestat;
   if (stat(path, &filestat)) {
      return 0;
   } 
   return S_ISDIR(filestat.st_mode);
}



//////////////////////////////
//
// createIncipit --
//

void createIncipit(HumdrumFile& hfile, const char* filename, 
      const char* sourcebase, int limit) {
   char outfile[4096] = {0};
   createOutputName(outfile, filename, target, sourcebase);
   cout << "creating incipit for: " << filename << "\toutput: " << outfile << endl;

   fstream output;

   output.open(outfile, ios::out);
   if (!output.is_open()) {
      cout << "Error opening file to write: " << outfile << endl;
      exit(1);
   }

   Array<int> lines;
   Array<int> phrase;
   getPhrasesAndNoteLines(lines, phrase, hfile);
   int endline = getEndingLine(lines, phrase, hfile, limit);

   int i;
   for (i=0; i<hfile.getNumLines(); i++) {
      switch (hfile[i].getType()) {
         case E_humrec_global_comment:
            if (i < endline) {
               output << hfile[i] << "\n";
            }
            break;
         case E_humrec_bibliography:
            output << hfile[i] << "\n";
            break;
         case E_humrec_data_comment:
            if (i < endline) {
               output << hfile[i][0] << "\n";
            }
            break;
         case E_humrec_data_kern_measure:
            if (i < endline+1) {
               output << hfile[i][0] << "\n";
            }
            break;
         case E_humrec_interpretation:
            if (i < endline) {
               if ((strcmp(hfile[i][0], "*v") != 0) &&
                   (strcmp(hfile[i][0], "*^") != 0) &&
                   (strcmp(hfile[i][0], "*x") != 0)) {
                  output << hfile[i][0] << "\n";
               }
            }
            break;
         case E_humrec_data:
            if (i < endline) {
               output << hfile[i][0] << "\n";
            }
            break;
         case E_humrec_empty:
         case E_humrec_none:
         default:
            break;
      }
      if (i == endline) {
         output << "*-\n";
      }
   }

   output.close();
}



//////////////////////////////
//
// getEndingLine --
//

int getEndingLine(Array<int>& lines, Array<int>& phrase, HumdrumFile& hfile,
      int limit) {
   int i;
   int start = lines.getSize()-1;
   if (start > limit) {
     start = limit;
   }
   int phraseboundary = start;
   if (phrase[phraseboundary] <= 1) {
      if (phraseboundary < minval) {
         phraseboundary = limit;
      }
      if (phraseboundary >= lines.getSize()) {
         phraseboundary = lines.getSize()-1;
      }
      return lines[phraseboundary];
   }

   for (i=start; i>=0; i--) {
      if (i>0 && (phrase[i] != phrase[i-1])) {
         phraseboundary = i; 
         break;
      }
   }

   if (phraseboundary < minval) {
      phraseboundary = limit;
   }
   if (phraseboundary >= lines.getSize()) {
      phraseboundary = lines.getSize()-1;
   }

   return lines[phraseboundary];
}



//////////////////////////////
//
// getPhrasesAndNoteLines --
//

void getPhrasesAndNoteLines(Array<int>& lines, Array<int>& phrase, 
      HumdrumFile& hfile) {

   lines.setSize(hfile.getNumLines());
   lines.setSize(0);
   phrase.setSize(hfile.getNumLines());
   phrase.setSize(0);

   int pcount = 0;

   int i;
   for (i=0; i<hfile.getNumLines(); i++) {
      if (hfile[i].getType() != E_humrec_data) {
         continue;
      }
      if (strcmp(hfile[i][0], ".") == 0) {
         // ignore null tokens
         continue;
      }
      if (strchr(hfile[i][0], '{') != NULL) {
         pcount++;
      }

      if (strchr(hfile[i][0], 'r') != NULL) {
         // ignore rests
         continue;
      }

      if (strchr(hfile[i][0], ']') != NULL) {
         // ignore tie endings
         continue;
      }

      if (strchr(hfile[i][0], '_') != NULL) {
         // ignore tie continuations
         continue;
      }

      lines.append(i);
      phrase.append(pcount);

   }

}



//////////////////////////////
//
// createOutputName --
//

void createOutputName(char* outfile, const char* filename, 
      const char* target, const char* sourcebase) {
   strcpy(outfile, target);
   strcat(outfile, "/");
   char buffer[1024] = {0};
   strcpy(buffer, filename);
   int len2 = strlen(buffer);
   char* dot = strrchr(buffer, '.');
   if (len2 - (int)(dot - buffer) == 4) {
      strcpy(dot, ".thm");
   } else {
      strcat(buffer, ".thm");
   }
   char* ptr;
   ptr = strrchr(buffer, '/');
   if (ptr) {
      strcat(outfile, ptr+1);
   } else {
      strcat(outfile, buffer);
   }
   checkTargetDirectory(outfile);
}



//////////////////////////////
//
// checkTargetDirectory -- create the target directory if it does not
//    exist.
//

void checkTargetDirectory(const char* outfile) {
   char buffer[4096] = {0};
   strncpy(buffer, outfile, 4000);
   char* end = strrchr(buffer, '/');
   if (end == NULL) {
      return;
   }
   end[0] = '\0';
   if (is_directory(buffer)) {
      return;
   }

   char fulldir[4096] = {0};
   if (buffer[0] == '/') {
      strcpy(fulldir, "/");
   }

   char* ptr = strtok(buffer, "/");
   while (ptr != NULL) {
      strcat(fulldir, ptr);
      if (!is_directory(fulldir)) {
         cout << "Creating the directory: " << fulldir << endl;
         if (mkdir(fulldir, 0755)) {
            cout << "ERROR creating the directory: "<< fulldir << endl;
            exit(1);
         }
      }
      strcat(fulldir, "/");      
      ptr = strtok(NULL, "/");
   }
}



//////////////////////////////
//
// extractPitchSequence --
// restrictions:
//   (1) **kern data must be first column in file.
//   (2) chords will be ignored, only first note in chord will be processed.
//

void extractPitchSequence(Array<int>& pitches, HumdrumFile& hfile) {
   pitches.setSize(10000);
   pitches.setGrowth(10000);
   pitches.setSize(0);
   pitches.allowGrowth();
   int pitch = 0;
   int i;

   for (i=0; i<hfile.getNumLines(); i++) {
      switch (hfile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_bibliography:
         case E_humrec_global_comment:
         case E_humrec_data_comment:
         case E_humrec_interpretation:
         case E_humrec_data_kern_measure:
            break;
         case E_humrec_data:
            if (strcmp(hfile[i][0], ".") == 0) {
               // ignore null tokens
               break;
            }
            if (strchr(hfile[i][0], '_') != NULL) {
               // ignore continuing ties
               break;
            }
            if (strchr(hfile[i][0], ']') != NULL) {
               // ignore ending ties
               break;
            }
            if (strchr(hfile[i][0], 'r') != NULL) {
               // ignore rests
               break;
            }
            pitch = Convert::kernToBase40(hfile[i][0]);
            if ((pitch < 0) || (pitch > 10000)) {
               // ignore rests and other strange things
               break;
            }
            pitches.append(pitch); 
            if (limitQ) {
               if (pitches.getSize() >= limit) {
                  return;
               }
            }
            break;
         default:
            break;
      }
   }

}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("debug=b",        "print debug information"); 
   opts.define("l|limit=i:30",   "upper limit the number of extracted notes");
   opts.define("m|min=i:10",     "lower limit on number of extracted notes");
   opts.define("t|target=s:.",   "filename target base for recursive output");

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("h|help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, May 2004" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 1 May 2004" << endl;
      cout << "compiled: " << __DATE__ << endl;
      cout << MUSEINFO_VERSION << endl;
      exit(0);
   } else if (opts.getBoolean("help")) {
      usage(opts.getCommand());
      exit(0);
   } else if (opts.getBoolean("example")) {
      example();
      exit(0);
   }
   
   debugQ      = opts.getBoolean("debug");
   limitQ      = opts.getBoolean("limit");
   limit       = opts.getInteger("limit");
   minval      = opts.getInteger("min");
   target      = opts.getString("target");

   if (!is_directory(target)) {
      cout << "Error: target directory does not exist: " << target << endl;
      exit(1);
   }

}



//////////////////////////////
//
// example --
//

void example(void) {


}



//////////////////////////////
//
// usage --
//

void usage(const char* command) {

}



// md5sum: 607dbdf6817a432762d577e6e551c1c6 thememakerx.cpp [20090527]
