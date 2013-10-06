/****************************************************************************/
/*                     HUMDRUM_.C                            		*/
/*	Programmer: Tim Racinsky	Date: July 1993			*/
/*	This file contains the main program for the humdrum     	*/
/* command.                                                 		*/  
/*									*/
/*      Bug fix related to a newer compiler by Craig Sapp 28 Sep 2006   */
/****************************************************************************/

#include <stdio.h>
#include <string.h>

#ifdef __MSDOS__
#include <alloc.h>
#endif

#ifdef __linux__
#include <malloc.h>
#endif

#ifdef  __APPLE__
#include <sys/malloc.h>
#endif

#include <stdlib.h>
#include "defines.h"
#include "typedefs.h"
#include "humdrum_.h"

/****************************************************************************/
/*						Function main										*/
/*																			*/
/****************************************************************************/

int main (int argc,char *argv[]) {

	/* Initialize any global variables and data structures */

	init_data();

	/* Parse the command line options and process accordingly */

	if (!parse_command(argc,argv)) {
		exit(1);
	} else if (!help) {
		check_files();
	} else {
		print_help();
	}

	/* Free any allocated memory and close any open files */
	clean_up();

   return 0;
}

/****************************************************************************/
/*						Function init_data									*/
/*																			*/
/****************************************************************************/

void init_data() {

	int i;

	/* Create a header element for a linked list of valid files */

	if ((file_header = (file_list *) malloc(sizeof(file_list))) == NULL) {
		print_error(MEMORY,"");
		exit(1);
	}
	file_header->stream = stderr;
	strcpy(file_header->file_name,"HEADER");
	file_header->next_file = NULL;

	/* Create a header for a linked list of current interpretations */

	if ((interp_header = (interp_list *) malloc(sizeof(interp_list))) == NULL) {
		print_error(MEMORY,"");
		exit(1);
	}
	interp_header->next_interp = NULL;

	/* Create a header for a linked list of used exclusive interpretations */

	if ((ex_inven_header = (interp_inventory *)
							malloc(sizeof(interp_inventory))) == NULL) {
		print_error(MEMORY,"");
		exit(1);
	}
	ex_inven_header->next_interp = NULL;

	/* Create a header for a linked list of used tandem interpretations */

	if ((tand_inven_header = (interp_inventory *)
							malloc(sizeof(interp_inventory))) == NULL) {
		print_error(MEMORY,"");
		exit(1);
	}
	tand_inven_header->next_interp = NULL;

	/* Initialize the array of ascii signifiers */

	for (i = 1; i <= 126; i += 1) {
		signifiers[i] = 0;
	}

	/* Flags to control what action is taken depending on user's options */

	help = FALSE;
	verbose = FALSE;
}

/****************************************************************************/
/*						Function parse_command								*/
/*																			*/
/****************************************************************************/

int parse_command(int argc,char *argv[]) {

	int i;
	int file_count = 0;				/* Number of valid files specified     */
	int stnd_input = FALSE;			/* Flag to indicate standard input     */
	int file_not_found = FALSE;		/* Indicates if invalid file specified */
	int error_code = NO_ERROR;		/* Return value						   */

	/* If there is only one argument (program name) then assume standard */
	/* input and place it on the list of files to process                */

	if (argc == 1) {
		add_file("STDIN");

	/* Otherwise, loop through all of the command line arguments */

	} else {
		i = 1;
		while (i < argc) {

			/* If the argument starts with a dash */

			if (strncmp(argv[i],"-",1) == 0) {

				/* Two dashes signify that all remaining arguments are   */
				/* filenames; a single dash signifies standard input; -v */
				/* signifies verbose option; any 'h' or '?' signifies	 */
				/* help option.											 */

				if (strcmp(argv[i],"--") == 0) {
					i += 1;
					break;
				} else if (strcmp(argv[i],"-") == 0) {
					break;
				} else if (strcmp(argv[i],"-v") == 0) {
					verbose = TRUE;
				} else if (strpbrk(argv[i],"h?") != NULL) {
					help = TRUE;
					break;
				} else {
					print_error(BAD_OPTIONS,argv[i]);
					print_error(USAGE,"");
					error_code = ERROR;
					break;
					}

			/* For any other case (no dash) exit the while loop */

			} else {
				break;
			}
			i += 1;
		}

		/* Again loop through the remaining command line arguments; they */
		/* should all be filenames.										 */
		/* Allow only one use of '-' for standard input, otherwise, add  */
		/* the file to the list if it is valid 							 */

		while (i < argc && !help && error_code == NO_ERROR) {
			if ((strcmp(argv[i],"-") == 0) && !(stnd_input)) {
				add_file("STDIN");
				stnd_input = TRUE;
			} else {
				if (!open_file(argv[i],READ_ONLY)) {
					file_not_found = TRUE;
				} else {
					file_count += 1;
				}
			}
			i += 1;
		}

		/* If no filename was specified, assume standard input */

		if (file_count == 0 && !file_not_found && !stnd_input) {
			add_file("STDIN");
		}
	}
	return error_code;
}

/****************************************************************************/
/*						Function clean_up									*/
/*																			*/
/****************************************************************************/

void clean_up() {

	file_list *next_file;
	file_list *old_file;
	interp_list *next_interp;
	interp_list *old_interp;
	interp_inventory *pointer;
	interp_inventory *deleter;

	/* Close any open files and free the file list */

	next_file = file_header->next_file;
	old_file = file_header;
	free(old_file);

	while (next_file != NULL) {
          /* fclose(next_file->stream); */
          old_file = next_file;
          next_file = next_file->next_file;
          free(old_file);
	}

	/* Free the interpretation list */

	next_interp = interp_header;
	while (next_interp != NULL) {
		old_interp = next_interp;
		next_interp = next_interp->next_interp;
		free(old_interp);
	}

	/* Free the exclusive interpretation inventory list */

	pointer = ex_inven_header;
	while (pointer != NULL) {
		deleter = pointer;
		pointer = pointer->next_interp;
		free(deleter);
	}

	/* Free the tandem interpretation inventory list */

	pointer = tand_inven_header;
	while (pointer != NULL) {
		deleter = pointer;
		pointer = pointer->next_interp;
		free(deleter);
	}
}


/****************************************************************************/
/*						Function print_help									*/
/*																			*/
/****************************************************************************/

void print_help() {

	printf("\n\n\n\n\n");
	printf("HUMDRUM   : Syntax checker for Humdrum files.\n\n");
	printf("     This command identifies whether an input stream conforms to the\n");
     printf("     Humdrum syntax.  No output means the input is Humdrum conformant.\n\n");
     printf("     With the -v (verbose) option, HUMDRUM outputs the following statistics:\n");
     printf("     inventory of interpretations used, inventory of data signifiers used,\n");
     printf("     number of global comments, local comments, data records, null tokens,\n");
     printf("     minimum and maximum number of concurrent spines, number of spine-path\n");
     printf("     changes.\n\n");
   	printf("Syntax:\n\n");
     printf("     humdrum [-v] [inputfile ...]\n\n");
	printf("Options:\n\n");
  	printf("  -v        : produces a verbose listing of the files contents\n\n");
	printf("See also:  CENSUS,  PROOF\n\n");
	printf("  Refer to reference manual for more details.\n\n");
}

/****************************************************************************/
/*						Function check_files								*/
/*																			*/
/****************************************************************************/

void check_files() {

	tokens fields;					/* Holds each field in a line */
	char *character;
	file_list *current_file;
	char current_line[LINE_LENGTH]; /* Holds current input line   */
	size_t tabs;

	/* If the verbose option was specified, initialize counting variables */

	if (verbose) {
		global_comments = 0;
		local_comments = 0;
		maximum_no_of_spines = 0;
		minimum_no_of_spines = 0;
		number_of_data_records = 0;
		null_tokens = 0;
		added_spines = 0;
		terminated_spines = 0;
		split_spines = 0;
		joined_spines = 0;
		exchanged_spines = 0;
	}

	/* Loop through each file in the list of valid files */

	current_file = file_header->next_file;
	while (current_file != NULL) {
		if (strcmp(current_file->file_name,"STDIN") == 0) {
			current_file->stream = stdin;
		} else {
			current_file->stream = fopen(current_file->file_name,READ_ONLY);
		}

		/* For each file, set these variables */

		fnr = 1;
		strcpy(current_filename,current_file->file_name);
		new_path = FALSE;
		current_no_of_spines = 0;
		first_interpretation_yet = FALSE;

		/* Loop through all input lines for the current file */

		fgets(current_line,LINE_LENGTH,current_file->stream);
		character = strchr(current_line,NEWLINE);
		if (character != NULL) {
			*character = TERMINATE;
		}
		while (!feof(current_file->stream)) {

			/* Check for empty lines */

			if (strcmp(current_line,"\0") == 0) {
					print_syntax_error(ERROR2);
			} else {

				/* Check for initial tab error */

				if (strncmp(current_line,TAB,1) == 0) {
					print_syntax_error(ERROR3);
				}

				/* Check to see if the current line is meant to be	*/
				/* a global comment									*/

				tabs = strspn(current_line,TAB);
				character = current_line + tabs;

				if (strncmp(character,"!!",2) == 0) {
					process_global();
				} else {

					/* Otherwise, check for other tab errors */

					character = strrchr(current_line,TAB_CHAR);
					if (character != NULL) {
						character += 1;
						if (*character == TERMINATE) {
							print_syntax_error(ERROR4);
						}
					}
					if (strstr(current_line,CONSECUTIVE_TABS) != NULL) {
						print_syntax_error(ERROR5);
					} 
	
					/* Check global comments, local comments, interpretation */
					/* records and data records depending on the first 		 */
					/* character of the first field							 */
	
					split_string(current_line,TAB,&fields);
	
					if (fields.number > 0)
						{
						if (strncmp(fields.token[0],"!",1) == 0)
							process_local(fields);
						else if (strncmp(fields.token[0],"*",1) == 0)
							process_interpretation(fields);
						else
							process_data(fields);
						free_tokens(fields);
						}
				}
			}
			fnr += 1;
			fgets(current_line,LINE_LENGTH,current_file->stream);
			character = strchr(current_line,NEWLINE);
			if (character != NULL) {
				*character = TERMINATE;
			}
		}

		/* All spines must be termintated */

		if (current_no_of_spines != 0) {
			print_syntax_error(ERROR14);
		}
		fclose(current_file->stream);
		current_file = current_file->next_file;
	}

	/* Print the verbose information at the end of processing all files */

	if (verbose) {
		print_verbose();
	}
}

/****************************************************************************/
/*						Function process_global								*/
/*																			*/
/****************************************************************************/

void process_global() {

	/* Keep statistics for the -v option */

	if (verbose) {
		global_comments += 1;
	}
}

/****************************************************************************/
/*						Function process_local								*/
/*																			*/
/****************************************************************************/

void process_local(tokens fields) {

	int i;

	/* A local comment may not immediately follow a new path indicator */

	if (new_path) {
		print_syntax_error(ERROR1);
		new_path = FALSE;
	}

	/* Keep statistics for the -v option */

	if (verbose) {
		local_comments += 1;
	}

	/* If the first interpretation record has not yet occurred, print error */

	if (!first_interpretation_yet) {
		print_syntax_error(ERROR9);
		current_no_of_spines = fields.number;
	}

	/* There must exist the correct number of fields */

	if (fields.number != current_no_of_spines) {
		print_syntax_error(ERROR10);
	}

	/* Each spine must begin with an excamation mark and users should be */
	/* warned if a spine begins with 2 exclamation marks				 */

	for (i = 0; i < fields.number; i += 1) {
		if (strncmp(fields.token[i],"!",1) != 0) {
			print_syntax_error(ERROR11);
		} else if (strncmp(fields.token[i],"!!",2) == 0) {
			print_syntax_error(WARNING1);
		}
	}
}

/****************************************************************************/
/*					Function process_interpretation							*/
/*																			*/
/****************************************************************************/

void process_interpretation(tokens fields) {

	int spine_path_record;
	int i;

	/* If a new path indicator occurred in the previous record, check the */
	/* current record for an exclusive interpretation in that spine		  */

	if (new_path) {
		check_new_path(fields);
	}

	/* Each spine must begin with an asterisk and cannot contain 2 	*/
	/* asterisks without anything following							*/

	for (i = 0; i < fields.number; i += 1) {
		if (strncmp(fields.token[i],"*",1) != 0) {
			print_syntax_error(ERROR6);
		} else if (strcmp(fields.token[i],"**") == 0) {
			print_syntax_error(ERROR7);
		}
	}

	/* If this is the first interpretation, set the variables and check */
	/* that it is a valid 'first interpretation' record.				*/

	if (!first_interpretation_yet) {
		first_interpretation_yet = TRUE;
		current_no_of_spines = fields.number;
		check_first_interp(fields);
		store_new_interps(fields);

		/* Keep any necessary statistics for the -v option */

		if (verbose) {
			if (current_no_of_spines > maximum_no_of_spines) {
				maximum_no_of_spines = current_no_of_spines;
			}
			if (current_no_of_spines < minimum_no_of_spines
											|| minimum_no_of_spines == 0) {
				minimum_no_of_spines = current_no_of_spines;
			}
		}

	/* Otherwise, determine if the record is a spine path record or not */

	} else {
		if (fields.number != current_no_of_spines) {
			print_syntax_error(ERROR8);
			current_no_of_spines = fields.number;
		}
		spine_path_record = FALSE;
		for (i = 0; i < fields.number; i += 1) {
			if (strcmp(fields.token[i],"*+") == 0
				   || strcmp(fields.token[i],"*^") == 0
				   || strcmp(fields.token[i],"*-") == 0
				   || strcmp(fields.token[i],"*x") == 0
				   || strcmp(fields.token[i],"*v") == 0) {
				spine_path_record = TRUE;
				break;
			}
		}

		/* If it is a spine path record, store and process the indicators */

		if (spine_path_record) {
			store_indicators(fields);
			process_indicators(fields);

			/* Keep any necessary statistics for the -v option */

			if (verbose) {
				if (current_no_of_spines > maximum_no_of_spines) {
					maximum_no_of_spines = current_no_of_spines;
				} else if (fields.number < minimum_no_of_spines) {
					minimum_no_of_spines = fields.number;
				}
			}
			if (current_no_of_spines == 0) {
				first_interpretation_yet = FALSE;
			}

		/* Otherwise, store the new interpretations */

		} else {
			store_new_interps(fields);
		}
	}
}

/****************************************************************************/
/*						Function check_first_interp							*/
/*																			*/
/****************************************************************************/

void check_first_interp(tokens fields) {

	int i;

	/* Every spine must contain only an exclusive interpretation */

	for (i = 0 ; i < fields.number; i += 1) {
		if (strcmp(fields.token[i],"*") == 0) {
			print_syntax_error(ERROR15);
		}
		if (strncmp(fields.token[i],"**",2) != 0) {
			print_syntax_error(ERROR17);
		} 
		if (strcmp(fields.token[i],"*+") == 0
			   || strcmp(fields.token[i],"*^") == 0
			   || strcmp(fields.token[i],"*-") == 0
			   || strcmp(fields.token[i],"*x") == 0
			   || strcmp(fields.token[i],"*v") == 0) {
			print_syntax_error(ERROR16);
		}
	}
}

/****************************************************************************/
/*						Function store_new_interps							*/
/*																			*/
/****************************************************************************/

void store_new_interps(tokens fields) {

	interp_list *new_interp;
	interp_list *pointer;
	int i;

	/* Loop through each currently active spine */

	pointer = interp_header;
	for (i = 0; i < fields.number; i += 1) {

		/* Either create the necessary node in the list of 	*/
		/* interpretations or replace the existing node		*/

		if (pointer->next_interp == NULL) {
			if ((new_interp = (interp_list *) 
									malloc(sizeof(interp_list))) == NULL) {
				print_error(MEMORY,"");
				exit(1);
			}

			/* Only store the new name if it is exclusive */

			if (strncmp(fields.token[i],"**",2) == 0)
				strcpy(new_interp->name,fields.token[i]);
			else
				strcpy(new_interp->name,"");
			new_interp->indicator = TERMINATE;
			new_interp->next_interp = NULL;
			pointer->next_interp = new_interp;
		} else {
			new_interp = pointer->next_interp;

			/* Only replace the name if it is exlusive */

			if (strncmp(fields.token[i],"**",2) == 0)
				strcpy(new_interp->name,fields.token[i]);
		}

		/* Keep an inventory of interpretations for the -v option */

		if (verbose) {
			if (strncmp(fields.token[i],"**",2) == 0)
				take_inventory(fields.token[i],ex_inven_header);
			else
				take_inventory(fields.token[i], tand_inven_header);
		}
		pointer = pointer->next_interp;
	}
}

/****************************************************************************/
/*						Function check_new_path								*/
/*																			*/
/****************************************************************************/

void check_new_path(tokens fields) {

	interp_list *pointer;
	int counter;

	counter = 0;
	pointer = interp_header->next_interp;

	/* Loop through each spine (each node in the list of interpretations */

	while (pointer != NULL) {

		/* If the current indicator is and 'N', then the current field 	*/
		/* must be an exclusive interpretation							*/

		if (pointer->indicator == 'N') {
			if (strncmp(fields.token[counter],"**",2) != 0) {
				print_syntax_error(ERROR1);
			}
			pointer->indicator = '*';
		}
		counter += 1;
		pointer = pointer->next_interp;
	}
	new_path = FALSE;
}

/****************************************************************************/
/*						Function take_inventory								*/
/*																			*/
/****************************************************************************/

void take_inventory(char *name,interp_inventory *list_header) {

	interp_inventory *pointer;
	interp_inventory *new_interp;

	pointer = list_header;
	new_interp = list_header->next_interp;

	/* Loop through the list of nodes until the end is reached or */
	/* a node with the same 'name' is reached					  */

	while (new_interp != NULL && strcmp(new_interp->name,name) != 0) {
		pointer = new_interp;
		new_interp = new_interp->next_interp;
	}

	/* If a node with the same 'name' was found, increase the counter */

	if (new_interp != NULL && strcmp(new_interp->name,name) == 0) {
		new_interp->count += 1;

	/* Otherwise, add a new node to the list of interpretations */

	} else {
		if ((new_interp = (interp_inventory *)
								malloc(sizeof(interp_inventory))) == NULL) {
		print_error(MEMORY,"");
		exit(1);
		}
		strcpy(new_interp->name,name);
		new_interp->count = 1;
		new_interp->next_interp = NULL;
		pointer->next_interp = new_interp;
	}
}

/****************************************************************************/
/*						Function process_data								*/
/*																			*/
/****************************************************************************/

void process_data(tokens fields) {

	tokens double_stops;
	char *character;
	int i, j;

	/* A data record may not follow a record with a new path indicator */

	if (new_path) {
		print_syntax_error(ERROR1);	
		new_path = FALSE;
	}

	/* Keep statistics for the -v option */

	if (verbose) {
		number_of_data_records += 1;
	}

	/* A data record may not appear before the first interpretation */
	/* record and must have the correct number of spines			*/

	if (!first_interpretation_yet) {
		print_syntax_error(ERROR12);
		current_no_of_spines = fields.number;
	} else if (current_no_of_spines != fields.number) {
		print_syntax_error(ERROR13);
	}

	/* Loop through each spine in the data record */

	for (j = 0; j < fields.number; j += 1) {

		/* Check for multiple-stop space errors */

		if (strncmp(fields.token[j],SPACE,1) == 0) {
			print_syntax_error(ERROR23);
		}
		character = strrchr(fields.token[j],SPACE_CHAR);
		if (character != NULL) {
			character += 1;
			if (*character == TERMINATE) {
				print_syntax_error(ERROR24);
			}
		} 
		if (strstr(fields.token[j],CONSECUTIVE_SPACE) != NULL) {
				print_syntax_error(ERROR25);
		} 

		/* Give a warning if any spine after the first spine may be   */
		/* misinterpreted as a different type of record in the future */

		if (j > 0) {
			if (strncmp(fields.token[j],"!!",2) == 0) {
				print_syntax_error(WARNING2);
			} else if (strncmp(fields.token[j],"!",1) == 0) {
				print_syntax_error(WARNING3);
			} else if (strncmp(fields.token[j],"**",2) == 0) {
				print_syntax_error(WARNING4);
			} else if (strncmp(fields.token[j],"*",1) == 0) {
				print_syntax_error(WARNING5);
			}
		}

		/* Keep statistics for the -v option */

		if (verbose) {
			if (strcmp(fields.token[j],NULL_TOKEN) == 0) {
				null_tokens += 1;
			}
			character = fields.token[j];
			while (*character != TERMINATE) {
				if (signifiers[*character] == 0) {
					signifiers[*character] = *character;
				}
				character += 1;
			}
		}

		/* Check that no null tokens are allocated in double stops */

		split_string(fields.token[j],SPACE,&double_stops);
		for (i = 0; i < double_stops.number; i += 1) {
			if (strcmp(double_stops.token[i],NULL_TOKEN) ==  0
												&& double_stops.number > 1) {
				print_syntax_error(ERROR26);
				break;
			}
		}
		free_tokens(double_stops);
	}
}

/****************************************************************************/
/*						Function print_verbose								*/
/*																			*/
/****************************************************************************/
		
void print_verbose() {

	interp_inventory *pointer;
	int i;

	printf("Inventory of Interpretations:\t\t\t\tHUMDRUM FILE SUMMARY\n\n");
	printf("\tInterpretations\tOccurrences\n");
	pointer = ex_inven_header->next_interp;
	while (pointer != NULL) {
		printf("\t%s\t\t%d\n",pointer->name,pointer->count);
		pointer = pointer->next_interp;
	}
	pointer = tand_inven_header->next_interp;
	while (pointer != NULL) {
		printf("\t%s\t\t%d\n",pointer->name,pointer->count);
		pointer = pointer->next_interp;
	}
	printf("\nInventory of data token signifiers:\t");
	for (i = 32; i <= 64; i += 1) {
		if (signifiers[i] != 0) {
			printf("%c", signifiers[i]);
		}
	}
	printf("\n\t\t\t\t\t");
	for (i = 65; i <= 96; i += 1) {
		if (signifiers[i] != 0) {
			printf("%c", signifiers[i]);
		}
	}
	printf("\n\t\t\t\t\t");
	for (i = 97; i <= 122; i += 1) {
		if (signifiers[i] != 0) {
			printf("%c", signifiers[i]);
		}
	}
	printf("\n\n");
	printf("Number of global comments:      %d\t",global_comments);
	printf("Number of local comments:       %d\n",local_comments);
	printf("Maximum # of concurrent spines: %d\t",maximum_no_of_spines);
	printf("Minimum # of concurrent spines: %d\n",minimum_no_of_spines);
	printf("Number of data records:         %d\t",number_of_data_records);
	printf("Number of null tokens:          %d\n\n",null_tokens);
	printf("\nChanges in number of spines:\n");
	printf("\tNew spines:        %d\n",added_spines);
	printf("\tTerminated spines: %d\n",terminated_spines);
	printf("\tSplit spines:      %d\n",split_spines);
	printf("\tJoined spines:     %d\n",joined_spines);
	printf("\tExchanges spines:  %d\n",exchanged_spines);
}

/****************************************************************************/
/*						Function print_syntax_error							*/
/*																			*/
/****************************************************************************/

void print_syntax_error(char *message) {

	printf("%s in line %d, file %s.\n",message,fnr,current_filename);
}

/****************************************************************************/
/*				      General Purpose Functions								*/
/*					(for all humdrum programs)								*/
/****************************************************************************/

/****************************************************************************/
/*						Function open_file									*/
/*																			*/
/****************************************************************************/

int open_file(char *filename,char *rights) {

	FILE *stream;
	int error_code;

	/* Check if the current file is a valid filename */
	/* and return the appropriate value				 */

	if ((stream = fopen(filename,rights)) == NULL) {
		print_error(OPEN_ERROR,filename);
		error_code = ERROR;
	} else {
		fclose(stream);
		add_file(filename);
		error_code = NO_ERROR;
	}
	return error_code;
}

/****************************************************************************/
/*						Function add_file									*/
/*																			*/
/****************************************************************************/

void add_file(char *name) {

	file_list *pointer;
	file_list *new_file;

	/* Loop until the end of the current list of valid files is found */

	pointer = file_header;
	while (pointer->next_file != NULL) {
		pointer = pointer->next_file;
	}

	/* Add the valid file to the list of files */

	if ((new_file = (file_list *) malloc(sizeof(file_list))) == NULL) {
		print_error(MEMORY,"");
		exit(1);
	}
	new_file->stream = stderr;
	strcpy(new_file->file_name,name);
	new_file->next_file = NULL;
	pointer->next_file = new_file;
}

/****************************************************************************/
/*						Function store_indicators							*/
/*																			*/
/****************************************************************************/

void store_indicators(tokens fields) {

	int i;
	interp_list *pointer;

	/* Loop through each spine in the current record */

	pointer = interp_header->next_interp;
	for (i = 0; i < fields.number; i += 1) {

		/* There may not be enough indicators provided */

		if (pointer == NULL) {
			break;
		} else {

			/* Make sure a valid indicator is given */

			if (strcmp(fields.token[i],"*+") != 0
				   && strcmp(fields.token[i],"*^") != 0
				   && strcmp(fields.token[i],"*-") != 0
				   && strcmp(fields.token[i],"*x") != 0
				   && strcmp(fields.token[i],"*v") != 0
				   && strcmp(fields.token[i],"*") != 0) {
				print_syntax_error(ERROR18);
			}

			/* Store the indicator */

			if (strcmp(fields.token[i],"*") == 0) {
				pointer->indicator = '*';
			} else {
				pointer->indicator = fields.token[i][1];
			}
		}
		pointer = pointer->next_interp;
	}
}

/****************************************************************************/
/*						Function process_indicators							*/
/*																			*/
/****************************************************************************/

void process_indicators(tokens fields) {

	interp_list *current;
	interp_list *previous;

	/* Loop through each node in the list of interpretations */

	current = interp_header->next_interp;
	previous = interp_header;
	while (current != NULL) {

		/* Perform the action corresponding to the indicator */

		switch(current->indicator) {
			case '^':
				current = do_split(current);
				break;
			case '-':
				current = do_terminate(current,previous);
				break;
			case 'x':
				current = do_exchange(current);
				break;
			case 'v':
				current = do_join(current);
				break;
			case '+':
				current = do_add(current);
				break;
		}	
		previous = current;
		current = current->next_interp;
	}
}

/****************************************************************************/
/*						Function do_split									*/
/*																			*/
/****************************************************************************/

interp_list *do_split(interp_list *current) {

	interp_list *pointer;

	/* Create a new node in the list of interpretations with */
	/* the same value as the current node (spine)			 */

	current = insert_new(current);
	pointer = current->next_interp;
	strcpy(pointer->name,current->name);
	pointer->indicator = '*';
	current->indicator = '*';

	/* Keep any necessary statistics for the -v option */

	if (verbose) {
		split_spines += 1;
	}
	return current;
}

/****************************************************************************/
/*						Function do_terminate								*/
/*																			*/
/****************************************************************************/

interp_list *do_terminate(interp_list *current,interp_list *previous) {

	/* Delete the current node from the list of interpretations */

	current = delete_old(current,previous);

	/* Keep any necessary statistics for the -v option */

	if (verbose) {
		terminated_spines += 1;
	}
	return current;
}

/****************************************************************************/
/*						Function do_exchange								*/
/*																			*/
/****************************************************************************/

interp_list *do_exchange(interp_list *current) {

	char temp[NAME_LENGTH];
	int found = FALSE;
	int error = FALSE;
	interp_list *next_exchange;
	interp_list *pointer;

	pointer = current->next_interp;
	next_exchange = NULL;
	current->indicator = '*';

	/* Determine which spines are to be exchanged */

	while (pointer != NULL) {
		if (pointer->indicator == 'x') {
			if (found) {
				error = TRUE;
				pointer->indicator = '*';
			} else {
				next_exchange = pointer;
				found = TRUE;
				pointer->indicator = '*';
			}
		}
		pointer = pointer->next_interp;
	}

	/* There must be exactly two spines to exchange */

	if (error || next_exchange == NULL) {
		print_syntax_error(ERROR19);

	/* Exchange the spines */

	} else {
		strcpy(temp,current->name);
		strcpy(current->name,next_exchange->name);
		strcpy(next_exchange->name,temp);
		if (verbose) {
			exchanged_spines += 2;
		}
	}
	return current;
}

/****************************************************************************/
/*						Function do_join									*/
/*																			*/
/****************************************************************************/

interp_list *do_join(interp_list *current) {

	interp_list *pointer;	

	pointer = current->next_interp;

	/* The next node must exist and contain a join spine indicator */

	if (pointer == NULL) {
		print_syntax_error(ERROR20);
		current->indicator = '*';
	} else if (pointer->indicator != 'v') {
		print_syntax_error(ERROR21);
		current->indicator = '*';

	/* If it does, join the two spines */

	} else {
		current->indicator = '*';

		/* Keep any necessary statistics for the -v option */

		if (verbose) {
			joined_spines += 1;
		}

		/* Continue to join spines until the end of the list is */
		/* reached or there are no more join path indicators	*/

		while (pointer != NULL && pointer->indicator == 'v') {

			/* They must be compatable spines to join them */

			if (strcmp(current->name,pointer->name) != 0) {
				print_syntax_error(ERROR22);
			}

			/* Delete the next spine */

			pointer = delete_old(pointer,current);
			pointer = pointer->next_interp;
			if (verbose) {
				joined_spines += 1;
			}
		}
	}
	return current;
}

/****************************************************************************/
/*						Function do_add										*/
/*																			*/
/****************************************************************************/

interp_list *do_add(interp_list *current) {

	interp_list *pointer;

	/* Insert a new node in the current list of nodes (interpretations) */
	/* and set variables so that the next record will check for the 	*/
	/* presence of an exclusive interpretation in the appropriate spine */

	current = insert_new(current);
	pointer = current->next_interp;
	strcpy(pointer->name,"");
	pointer->indicator = 'N';
	current->indicator = '*';
	new_path = TRUE;

	/* Keep any necessary statistics for the -v option */

	if (verbose) {
		added_spines += 1;
	}
	return current;
}

/****************************************************************************/
/*						Function insert_new									*/
/*																			*/
/****************************************************************************/

interp_list *insert_new(interp_list *current) {

	interp_list *new;

	/* Create a new node in the list of interpretations */
	/* and update any necessary variables				*/

	if ((new = (interp_list *) malloc(sizeof(interp_list))) == NULL) {
		print_error(MEMORY,"");
		exit(1);
	}
	new->next_interp = current->next_interp;
	current->next_interp = new;
	current_no_of_spines += 1;
	return current;
}

/****************************************************************************/
/*						Function delete_old									*/
/*																			*/
/****************************************************************************/

interp_list *delete_old(interp_list *current,interp_list *previous) {

	interp_list *deleter;

	/* Delete the given node and update any necessary variables */

	previous->next_interp = current->next_interp;
	deleter = current;
	current = previous;
	free(deleter);
	current_no_of_spines -= 1;
	return current;
}

/****************************************************************************/
/*						Function split_string								*/
/*																			*/
/****************************************************************************/

void split_string(char *string,char *delim,tokens *this_line) {

	char temp[LINE_LENGTH];
	int index;

	strcpy(temp,string);
	this_line->number = 0;

	/* Determine the number of fields in the current line */

	if (strtok(temp,delim) != NULL) {
		this_line->number += 1;
	} else {
		return;
	}

	while (strtok(NULL,delim) != NULL) {
		this_line->number += 1;
	}

	/* Actually store the strings */

	if ((this_line->token = (char **) 
						malloc(this_line->number * sizeof(char *))) == NULL) {
		print_error(MEMORY,"");
		exit(1);
	}
	if ((this_line->token[0] = (char *) malloc(LINE_LENGTH)) == NULL) {
		print_error(MEMORY,"");
		exit(1);
	}
	strcpy(this_line->token[0], (char *) strtok(string,delim));

	for (index = 1; index < this_line->number; index += 1) {
		if ((this_line->token[index] = (char *) malloc(LINE_LENGTH)) == NULL) {
			print_error(MEMORY,"");
			exit(1);
		}
		strcpy(this_line->token[index], (char *) strtok(NULL, delim));
	}
}

/****************************************************************************/
/*						Function free_tokens								*/
/*																			*/
/****************************************************************************/

void free_tokens(tokens fields) {

	int i;

	for (i = 0; i < fields.number; i += 1) {
		free(fields.token[i]);
	}
	free(fields.token);
}

/****************************************************************************/
/*						Function print_error								*/
/*																			*/
/****************************************************************************/

void print_error(char *message,char *args) {

	fprintf(stderr,message,args);
}


