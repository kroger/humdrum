/****************************************************************************/
/*						DEFINES.H											*/
/* This file contains all #define statements for the humsyn program			*/
/****************************************************************************/

#define TRUE 1
#define FALSE 0
#define NO_ERROR 1
#define ERROR 0
#define FILE_LENGTH 100
#define LINE_LENGTH 1024
#define NAME_LENGTH 80
#define NULL_TOKEN "."
#define SPACE " "
#define CONSECUTIVE_SPACE "  "
#define TAB "\t"
#define CONSECUTIVE_TABS "\t\t"
#define EOL "\n"
#define READ_ONLY "r"
#define TAB_CHAR '\t'
#define SPACE_CHAR ' '
#define NEWLINE '\n'
#define TERMINATE '\0'

#define OPEN_ERROR "humdrum: ERROR: Cannot open file %s\n"
#define BAD_OPTIONS "humdrum: ERROR: Unknown options: %s\n"
#define MEMORY "humdrum: ERROR: Out of Memory\n"
#define USAGE "\n\nUSAGE: humdrum -h|-?              (Help Screen)\n       hu" \
			  "mdrum [-v] [file ...]\n"
#define ERROR1 "humdrum: ERROR 1: Record containing add-spine indicator " \
               "has not been\n                  followed by exclusive " \
               "interpretation for that spine\n                 "
#define ERROR2 "humdrum: ERROR 2: Illegal empty record"
#define ERROR3 "humdrum: ERROR 3: Leading tab"
#define ERROR4 "humdrum: ERROR 4: Trailing tab"
#define ERROR5 "humdrum: ERROR 5: Consecutive tabs"
#define ERROR6 "humdrum: ERROR 6: Missing initial asterisk in " \
               "interpretation\n                  keyword"
#define ERROR7 "humdrum: ERROR 7: Null exclusive interpretation found "
#define ERROR8 "humdrum: ERROR 8: Incorrect number of spines in " \
               "interpretation\n                  record"
#define ERROR9 "humdrum: ERROR 9: Local comment precedes first exclusive " \
               "interpretation\n                  record"
#define ERROR10 "humdrum: ERROR 10: Number of sub-comments in local " \
                "comment does not match the\n                   number " \
                "of currently active spines"
#define ERROR11 "humdrum: ERROR 11: Missing initial exclamation mark in " \
                "local comment\n                  "
#define ERROR12 "humdrum: ERROR 12: Data record appears before first " \
                "exclusive interpretation\n                   record"
#define ERROR13 "humdrum: ERROR 13: Number of tokens in data record does " \
                "not match the\n                   number of currently " \
                "active spines"
#define ERROR14 "humdrum: ERROR 14: All spines have not been properly " \
                "terminated\n                  "
#define ERROR15 "humdrum: ERROR 15: First exclusive interpretation record " \
                "contains a null\n                   interpretation"
#define ERROR16 "humdrum: ERROR 16: First exclusive interpretation record " \
                "contains a spine-path\n                   indicator"
#define ERROR17 "humdrum: ERROR 17: First exclusive interpretation record " \
                "contains a\n                   non-exclusive " \
                "interpretation"
#define ERROR18 "humdrum: ERROR 18: Spine-path indicators mixed with " \
                "\n                   keyword interpretations"
#define ERROR19 "humdrum: ERROR 19: Improper number of exchange-path " \
                "indicators\n                  "
#define ERROR20 "humdrum: ERROR 20: Single join-path indicator found at end " \
                "of\n                   interpretation record"
#define ERROR21 "humdrum: ERROR 21: Join-path indicator is not adjacent to " \
                "another\n                   join-path indicator"
#define ERROR22 "humdrum: ERROR 22: Exclusive interpretations do not match " \
                "for designated\n                   join spines"
#define ERROR23 "humdrum: ERROR 23: Leading spaces in token"
#define ERROR24 "humdrum: ERROR 24: Trailing spaces in token"
#define ERROR25 "humdrum: ERROR 25: Consecutive spaces in token"
#define ERROR26 "humdrum: ERROR 26: Multiple-stop contains null token"
#define WARNING1 "humdrum: WARNING 1: Local comment may be mistaken for " \
                 "global comment\n                   "
#define WARNING2 "humdrum: WARNING 2: Data token may be mistaken for " \
                 "global comment\n                   "
#define WARNING3 "humdrum: WARNING 3: Data token may be mistaken for local " \
                 "comment\n                   "
#define WARNING4 "humdrum: WARNING 4: Data token may be mistaken for " \
                 "exclusive interpretation\n                   "
#define WARNING5 "humdrum: WARNING 5: Data token may be mistaken for tandem " \
                 "interpretation\n                   "
