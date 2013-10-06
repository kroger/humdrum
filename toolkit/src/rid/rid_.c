/****************************************************************************
* 							RID_.C
*
* Programmed by: Tim Racinsky        Date: July, 1994
* Copyright (c) 1994 David Huron
*
* Modifications:
* Date:       Programmer:       	Description:
* May 17 '95  Kyle Dawkins       Fixed a bug in parse_command
* July 16 '97 Mark Dewitt        Fixed a bug in parse_command
*
* This program filters the input stream to remove particular types of
* records. Any type of humdrum file is acceptable as input.  With the 
* exception of the specified record types, all other input lines are passed 
* to the output unhindered.
*	
****************************************************************************/

/****************************************************************************
* Related Files: rid.h - contains function prototypes, type definitions, 
*					 	 and global variables.
*				 makefile - makefile used to compile under GNU compiler (GCC)
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
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

#include "rid.h"

/****************************************************************************
*						Function main
*
* The main function is used only to call other functions
****************************************************************************/

int main (int argc, char *argv[])
	{
	/* Initialize any global variables and data structures */
	init_data();

	/* Parse the command line options */
	parse_command(argc,argv);

	/* Process the given file according to the specified options */
	process_file();

	/* Free any allocated memory and close any open files */
	clean_up();

	exit(0);
	}

/****************************************************************************
*						Function init_data
*
* This function initializes the linked lists used in this program as well as
* the global options structure.
****************************************************************************/

void init_data()
	{
	/* Create a header for a linked list of current interpretations */

	if ((current_interps = (interp_list *) malloc(sizeof(interp_list))) == NULL)
		print_error(MEMORY,"");
	current_interps->next_interp = NULL;

	/* Create a header for a linked list of old interpretations */

	if ((old_interps = (interp_list *) malloc(sizeof(interp_list))) == NULL)
		print_error(MEMORY,"");
	old_interps->next_interp = NULL;

	/* Create a header for a linked list of buffer entries */

	if ((comment_buffer = (buffer *) malloc(sizeof(buffer))) == NULL)
		print_error(MEMORY,"");
	comment_buffer->next_line = NULL;

	/* Initialize the global options structure */

	options.d = FALSE;
	options.g = FALSE;
	options.i = FALSE;
	options.l = FALSE;
	options.D = FALSE;
	options.G = FALSE;
	options.I = FALSE;
	options.L = FALSE;
	options.u = FALSE;
	options.U = FALSE;
	options.T = FALSE;

	/* Two flags used in this program to determine if records are currently */
	/* being stored and if this is the first exclusive interpretation yet	*/

	storing = FALSE;
	first_interpretation_yet = FALSE;
	}

/****************************************************************************
*						Function parse_command
*
* This function parses the command line passed to it to determine if it is
* valid or not.  It should have the form 'rid_ <options> <filename>', where
* <options> is a string of characters deemed valid by the calling shell script,
* and <filename> is the name of a valid file (although this function double
* checks that) or the null string.
****************************************************************************/

void parse_command(int argc, char *argv[])
	{
	char *next_option;
	char filename[FILE_NAME_LENGTH];

	if (argc != 2 && argc != 3)
		print_error(INVOKE,"");
	else
		{
		next_option = argv[1];

		/* Turn on the appropriate options to check for */

		while (*next_option != '\0')
			{
			switch (*next_option) 
				{
				case 'd': options.d = TRUE; break;
				case 'g': options.g = TRUE; break;
				case 'i': options.i = TRUE; break;
				case 'l': options.l = TRUE; break;
				case 'D': options.D = TRUE; break;
				case 'G': options.G = TRUE; break;
				case 'I': options.I = TRUE; break;
				case 'L': options.L = TRUE; break;
				case 'u': options.u = TRUE; break;
				case 'U': options.U = TRUE; break;
				case 'T': options.T = TRUE; break;
				}
			next_option += 1;
			}

		/* Open the filename or standard input if no filename given */

		if (argc == 2 || (strcmp(argv[2],"-") == 0))
			{
			strcpy(filename,"STNDIN");
			stream = stdin;
			}
		else
			{
			if (strcmp(argv[2],"-"))
				{
				strcpy(filename,argv[2]);
				if ((stream = fopen(filename,"r")) == NULL)
					print_error(OPEN_ERROR,filename);
				}
			else
				{
				strcpy(filename, "STNDIN");
				stream = stdin;
				}
			}
		}
	}

/****************************************************************************
*						Function process_file
*
* This function gets each line of text from the input file and processes each
* line according to its type and the options specified.
****************************************************************************/

void process_file()
	{
	tokens fields, terminate;
	char *character;
	char current_line[LINE_LENGTH];
	buffer *next_buf;

	fgets(current_line,LINE_LENGTH,stream);
	if ((character = strrchr(current_line,'\n')) != NULL) *character = '\0';

	/* Loop through all input lines for the current file */
	while (!feof(stream))
		{

		/* Process global comments as an entire line */

		if (strncmp(current_line,"!!",2) == 0)
			process_global(current_line);
		else
			{

			/* Split all other types of lines into fields and process 	*/
			/* as local comments, interpretations, or data records		*/

			split_string(current_line,TAB_CHAR,&fields);
			if (fields.number > 0)
				{
				if (fields.token[0][0] == '!')
					process_local(fields);
				else if (fields.token[0][0] == '*')
					process_interpretation(fields);
				else
					process_data(fields);
				free_tokens(fields);
				}
			}
		fgets(current_line,LINE_LENGTH,stream);
		if ((character = strrchr(current_line,'\n')) != NULL) *character = '\0';
		}

	/* If records were being stored at the end of processing, print them */

	if (storing)
		{
		next_buf = comment_buffer->next_line;
		split_string(next_buf->buf_line,TAB_CHAR,&terminate);
		flush_buffer(terminate);
		free_tokens(terminate);
		}
	}

/****************************************************************************
*						Function clean_up
*
* This function is called to clean up all of the linked lists that were
* created during the course of the program.
****************************************************************************/

void clean_up()
	{
	interp_list *next_interp, *last_interp;
	buffer *next_line, *last_line;

	/* Free the list of current interpretations */

	next_interp = current_interps;
	while (next_interp != NULL)
		{
		last_interp = next_interp;
		next_interp = next_interp->next_interp;
		free(last_interp);
		}

	/* Free the list of old interpretations */

	next_interp = old_interps;
	while (next_interp != NULL)
		{
		last_interp = next_interp;
		next_interp = next_interp->next_interp;
		free(last_interp);
		}

	/* Free the list of stored buffer contents */

	next_line = comment_buffer;
	while (next_line != NULL)
		{
		last_line = next_line;
		next_line = next_line->next_line;
		free(last_line);
		}

	/* Close the opened file */

	fclose(stream);
	}

/****************************************************************************
*						Function process_global
*
* This function processes global comments according to the specified options.
****************************************************************************/

void process_global(char *current_line)
	{
	buffer *new_line, *find_line;

	if (options.G) return;
	else if (options.g && (strcmp(current_line,"!!") == 0)) return;
	else if (storing)
		{
		/* Add this line to the comment_buffer if currently storing records */

		find_line = comment_buffer;
		while (find_line->next_line != NULL) find_line = find_line->next_line;
		if ((new_line = (buffer *) malloc(sizeof(buffer))) == NULL)
			print_error(MEMORY,"");
		strcpy(new_line->buf_line,current_line);
		new_line->next_line = NULL;
		find_line->next_line = new_line;
		}
	else printf("%s\n",current_line);
	}

/****************************************************************************
*						Function process_local
*
* This function processes local comments according to the specified options.
****************************************************************************/

void process_local(tokens fields)
	{
	/* Local comments should not appear after all spines terminate and	*/
	/* before the next exclusive interpretation record					*/

	if (storing)
		print_error(ORDER,"");
	else if (options.L) return;
	else if (options.l && null_record(fields,"!")) return;
	else print_line(fields);
	}

/****************************************************************************
*					Function process_interpretation
*
* This function processes interpretation records according to their type and
* the options specified on the command line.
****************************************************************************/

void process_interpretation(tokens fields)
	{
	if (options.I)
		return;
	else if (options.i && null_record(fields,"*"))
		return;
	else if ((options.U || options.u) && exclusive_record(fields))
		process_exclusive(fields);
	else if (options.T && tandem_record(fields))
		return;
	else if ((options.U || options.u) && spine_path_record(fields))
		process_spine_path(fields);
	else print_line(fields);
	}

/****************************************************************************
*						Function process_exclusive
*
* This function processes exlusive interpretation records.
****************************************************************************/

void process_exclusive(tokens fields)
	{
	/* If this is the first interpretation record of the file or the first	*/
	/* one after all spines terminated										*/
	if (!first_interpretation_yet)
		{
		first_interpretation_yet = TRUE;

		/* If currently storing records (i.e. all spines terminated), check	*/
		/* to see if these interpretations are repeated and need not be		*/
		/* printed out again.												*/

		if (storing)
			{
			check_past(fields);
			delete_old_interps();
			storing = FALSE;
			}
		else print_line(fields);
		}

	/* If it's not the first exclusive interpretation record, check to see 	*/
	/* if there are interpretations specified that are redundant			*/

	else remove_redundant(fields);

	/* Regardless, store the new interpretations */

	store_new_interps(fields);
	}

/****************************************************************************
*						Function process_spine_path
*
* This function process spine-path records.
****************************************************************************/

void process_spine_path(tokens fields)
	{

	/* These records cannot appear after all spines terminated and before	*/
	/* the next exclusive interpretation record.							*/

	if (storing)
		print_error(ORDER,"");

	/* If all spines terminate in this record, copy the current	*/
	/* interpretations to another linked list and store the 	*/
	/* terminating record in the comment buffer					*/

	if (spine_terminate(fields))
		{
		first_interpretation_yet = FALSE;
		storing = TRUE;
		copy_new_to_old();
		store_terminator(fields);
		}
	else print_line(fields);

	/* Store and process the spine-path indicators */

	store_indicators(fields);
	process_indicators();
	}

/****************************************************************************
*						Function process_data
*
* This function processes the data records according to the options specified.
****************************************************************************/

void process_data(tokens fields)
	{
	/* Data records cannot appear after all spines terminate and	*/
	/* before the next exclusive interpretation record				*/

	if (storing)
		print_error(ORDER,"");
	else if (options.D) return;
	else if (options.d && null_record(fields,".")) return;
	else print_line(fields);
	}

/****************************************************************************
*						Function check_past
*
* This function is called from process_exlusive and is used to determine if
* all of the spines in the previous spine-terminating record really needed
* to be terminated.
****************************************************************************/

void check_past(tokens fields)
	{
	interp_list *next_old;
	int i, terminate_pos;
	buffer *next_buf;
	tokens terminate;

	/* Split the terminating record (which is the first entry in the	*/
	/* comment buffer) into separate fields for processing				*/

	next_buf = comment_buffer->next_line;
	split_string(next_buf->buf_line,TAB_CHAR,&terminate);

	/* If ALL of the interpretations on the current line appear in the	*/
	/* same relative order as when the spines terminated, don't print	*/
	/* them again and modify the spine-terminating record appropriately	*/

	if (all_found(fields))
		{
		terminate_pos = 0;
		next_old = old_interps->next_interp;

		/* Determine which spines didn't need to be terminated */

		for (i = 0; i < fields.number; i += 1)
			{
			while (next_old != NULL)
				{
				if (strcmp(fields.token[i],next_old->name) == 0)
					{
					strcpy(terminate.token[terminate_pos],"*");
					next_old = next_old->next_interp;
					terminate_pos += 1;
					break;
					}
				else
					{
					next_old = next_old->next_interp;
					terminate_pos += 1;
					}
				}
			}

		/* Print the rest of the buffer's contents */

		flush_buffer(terminate);
		}

	/* If all of the interpretations did not exist before the spines	*/
	/* terminated, then print the records as they were found			*/

	else
		{
		flush_buffer(terminate);
		print_line(fields);
		}
	free_tokens(terminate);
	}

/****************************************************************************
*						Function remove_redundant
*
* This function is called from process_exclusive and is used to check
* if any exclusive interpretations specified on the current line are redundant.
****************************************************************************/

void remove_redundant(tokens fields)
	{
	interp_list *interp;
	int i;

	/* Loop through the current fields and check the interpretations with	*/
	/* the ones stored in current_interps									*/

	interp = current_interps->next_interp;
	for (i = 0; i < fields.number; i += 1)
		{
		if (interp == NULL) break;
		if (strcmp(interp->name,fields.token[i]) == 0)
			strcpy(fields.token[i],"*");
		interp = interp->next_interp;
		}
	if (!null_record(fields,"*")) print_line(fields);
	}

/****************************************************************************
*						Function all_found
*
* This function is called from check_past and is used to determine if the
* interpretations specified on the current line were active, in the same
* order, before all of the spines terminated.
****************************************************************************/

boolean all_found(tokens fields)
	{
	int i;
	interp_list *next_old;
	boolean found = TRUE;

	/* Loop through the current fields and check against the */
	/* interpretations specified in old_interps				*/

	next_old = old_interps->next_interp;
	for (i = 0; i < fields.number; i += 1)
		{
		while (next_old != NULL)
			{
			if (strcmp(fields.token[i],next_old->name) == 0) break;
			next_old = next_old->next_interp;
			}
		if (next_old == NULL)
			{
			found = FALSE;
			break;
			}
		next_old = next_old->next_interp;
		}
	return found;
	}

/****************************************************************************
*						Function copy_old_to_new
*
* This function is called from process_spine_path when all of the spines
* are to terminate so that the current interpretations can be saved to 
* compare with a possible future exclusive interpretation record.
****************************************************************************/

void copy_new_to_old()
	{
	interp_list *current, *new_old, *prev_old;

	/* Loop through the list of current interpretations and copy them	*/
	/* to a list of old interpretations									*/

	current = current_interps->next_interp;
	prev_old = old_interps;
	while (current != NULL)
		{
		if ((new_old = (interp_list *) malloc(sizeof(interp_list))) == NULL)
			print_error(MEMORY,"");
		strcpy(new_old->name,current->name);
		new_old->indicator = '\0';
		new_old->next_interp = NULL;
		prev_old->next_interp = new_old;
		prev_old = prev_old->next_interp;
		current = current->next_interp;
		}
	}

/****************************************************************************
*						Function store_terminator
*
* This function stores the current spine-path terminating record as the first
* entry in the comment buffer.
****************************************************************************/

void store_terminator(tokens fields)
	{
	char current_line[LINE_LENGTH];
	buffer *new_line;
	int i;

	/* The separate fields containing the spine-path terminators must	*/
	/* be re-built into a record to be stored in the comment buffer		*/

	strcpy(current_line,fields.token[0]);
	for (i = 1; i < fields.number; i += 1)
		{
		strcat(current_line,TAB);
		strcat(current_line,fields.token[i]);
		}
	if ((new_line = (buffer *) malloc(sizeof(buffer))) == NULL)
		print_error(MEMORY,"");
	strcpy(new_line->buf_line,current_line);
	new_line->next_line = NULL;
	comment_buffer->next_line = new_line;
	}

/****************************************************************************
*						Function flush_buffer
*
* This function is used to print out the contents of the comment buffer.  The
* exception is that the first entry (the spine-terminating record) is not
* printed but rather it is passed to the function as separate fields and it
* is only printed if all of the fields are not null interpretations.
****************************************************************************/

void flush_buffer(tokens terminate)
	{
	buffer *next_buf, *del_buf;

	/* Check if the spine-terminating record should be printed */

	if (!null_record(terminate,"*")) print_line(terminate);

	del_buf = comment_buffer->next_line;
	next_buf = del_buf->next_line;
	comment_buffer->next_line = NULL;
	free(del_buf);

	/* Print the rest of the buffer and delete the contents of the buffer	*/

	while(next_buf != NULL)
		{
		printf("%s\n",next_buf->buf_line);
		del_buf = next_buf;
		next_buf = next_buf->next_line;
		free(del_buf);
		}
	}

/****************************************************************************
*						Function print_line
*
* This function is used to print a record that has been split into 
* separate fields by the split_string function.
****************************************************************************/

void print_line(tokens fields)
	{
	int i;

	printf("%s",fields.token[0]);
	for (i = 1; i < fields.number; i += 1)
		printf("\t%s",fields.token[i]);
	printf("\n");
	}

/****************************************************************************
*						Function store_new_interps
*
* This function stores the current record of exclusive interpretation records
* in the current_interps linked list.
****************************************************************************/

void store_new_interps(tokens fields)
	{
	interp_list *new_interp, *pointer, *del_interp;
	int i;

	pointer = current_interps;

	/* Loop through each of the fields on the current line */

	for (i = 0; i < fields.number; i += 1)
		{

		/* If there is no entry in current_interps to store this		*/
		/* interpretation, then make a new entry						*/

		if (pointer->next_interp == NULL)
			{
			if ((new_interp = (interp_list *) 
									malloc(sizeof(interp_list))) == NULL)
				print_error(MEMORY,"");

			/* If this field is an exclusive interpretation, then store its	*/
			/* name in the entry in the linked list							*/

			if (strncmp(fields.token[i],"**",2) == 0)
				strcpy(new_interp->name,fields.token[i]);

			/* Otherwise, store no name (this is non-humdrum but must	*/
			/* still be taken care of for consistency)					*/

			else
				strcpy(new_interp->name,"");
			new_interp->indicator = '\0';
			new_interp->next_interp = NULL;
			pointer->next_interp = new_interp;
			}

		/* If there was already an entry in the linked list, then modify it */
		/* only if the current field is an exclusive interpretation			*/

		else if (strncmp(fields.token[i],"**",2) == 0)
			{
			new_interp = pointer->next_interp;
			strcpy(new_interp->name,fields.token[i]);
			}
		pointer = pointer->next_interp;
		}

	/* If there are entries still in the linked list after the last field, 	*/
	/* then delete them. (Again, this is not a valid humdrum file but 		*/
	/* must be taken care of to keep the integrity of the linked lists		*/

	if (pointer->next_interp != NULL)
		{
		del_interp = pointer->next_interp;
		pointer->next_interp = NULL;
		pointer = del_interp;
		while (pointer != NULL)
			{
			pointer = pointer->next_interp;
			free(del_interp);
			del_interp = pointer;
			}
		}
	}

/****************************************************************************
*						Function store_indicators
*
* This function stores the spine-path indicators in the current line in the
* linked list current_interps.
****************************************************************************/

void store_indicators(tokens fields)
	{
	int i;
	interp_list *pointer;

	/* Loop through the fields of the current record */

	pointer = current_interps->next_interp;
	for (i = 0; i < fields.number; i += 1)
		{

		/* Quit the loop if there are more fields in the current line	*/
		/* than there should be (i.e. non-humdrum)						*/

		if (pointer == NULL) break;
		else
			{

			/* If there is no indicator, store a null interpretation */
			/* Otherwise, store the indicator alone.				*/

			if (strcmp(fields.token[i],"*") == 0) pointer->indicator = '*';
			else pointer->indicator = fields.token[i][1];
			}
		pointer = pointer->next_interp;
		}
	}

/****************************************************************************
*						Function process_indicators
*
* Process the indicators stored in current_interps.  These indicators will
* necessarily modify the linked list current_interps itself.
****************************************************************************/

void process_indicators()
	{
	interp_list *current, *previous;

	current = current_interps->next_interp;
	previous = current_interps;

	/* Loop through the current_interps and act according to the indicator */

	while (current != NULL)
		{
		switch(current->indicator)
			{
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

/****************************************************************************
*						Function do_split
*
* Split the given linked list entry into two entries with the same name.
****************************************************************************/

interp_list *do_split(interp_list *current)
	{
	interp_list *pointer;

	/* Create a new node in the list of interpretations with */
	/* the same value as the current node (spine)			 */

	current = insert_new(current);
	pointer = current->next_interp;
	strcpy(pointer->name,current->name);
	pointer->indicator = '*';
	current->indicator = '*';

	return current;
	}

/****************************************************************************
*						Function do_terminate
*
* Delete the given linked list entry.
****************************************************************************/

interp_list *do_terminate(interp_list *current,interp_list *previous)
	{
	current = delete_old(current,previous);
	return current;
	}

/****************************************************************************
*						Function do_exchange
*
* This function is used to exchange the positions of two entries (exchanging
* their names is just as good!).
****************************************************************************/

interp_list *do_exchange(interp_list *current)
	{
	char temp[NAME_LENGTH];
	boolean found = FALSE;
	interp_list *next_exchange, *pointer;
	next_exchange = 0;

	pointer = current->next_interp;
	current->indicator = '*';

	/* There must always be two entries to exchange (and only two) 	*/
	/* If there are more than two in the current list, ignore then	*/

	while (pointer != NULL)
		{
		if (pointer->indicator == 'x')
			{

			/* Delete any extra exchange indicators */

			if (found) pointer->indicator = '*';
			else
				{
				next_exchange = pointer;
				found = TRUE;
				pointer->indicator = '*';
				}
			}
		pointer = pointer->next_interp;
		}

	/* Only exchange spines if two were found */

	if (found)
		{
		strcpy(temp,current->name);
		strcpy(current->name,next_exchange->name);
		strcpy(next_exchange->name,temp);
		}

	return current;
	}

/****************************************************************************
*						Function do_join
*
* This spines joins two or more spines together (it is assumed that it is a
* valid join -- i.e. all spines have the same exclusive interpretation).
****************************************************************************/

interp_list *do_join(interp_list *current)
	{
	interp_list *pointer;	

	pointer = current->next_interp;
	current->indicator = '*';

	/* Loop through and delete the joined entries */

	while (pointer != NULL && pointer->indicator == 'v')
		{
		pointer = delete_old(pointer,current);
		pointer = pointer->next_interp;
		}

	return current;
	}

/****************************************************************************
*						Function do_add
*
* This function is used to add a new spine to the current list of 
* interpretations and it is given no name since it should be defined in
* the next record.
****************************************************************************/

interp_list *do_add(interp_list *current)
	{
	interp_list *pointer;

	current = insert_new(current);
	pointer = current->next_interp;
	strcpy(pointer->name,"");
	pointer->indicator = '*';
	current->indicator = '*';

	return current;
	}

/****************************************************************************
*						Function insert_new
*
* This function does the actual insertion of entries into the linked list.
****************************************************************************/

interp_list *insert_new(interp_list *current)
	{
	interp_list *new;

	if ((new = (interp_list *) malloc(sizeof(interp_list))) == NULL)
		print_error(MEMORY,"");
	new->next_interp = current->next_interp;
	current->next_interp = new;
	return current;
	}

/****************************************************************************
*						Function delete_old
*
* This function does the actual removal of entries from the linked list.
****************************************************************************/

interp_list *delete_old(interp_list *current,interp_list *previous)
	{
	interp_list *deleter;

	previous->next_interp = current->next_interp;
	deleter = current;
	current = previous;
	free(deleter);

	return current;
	}

/****************************************************************************
*						Function delete_old_interps
*
* This function is called from process_exclusive and is used to delete the
* list of old interpretations that were stored to check for redundancy.
****************************************************************************/

void delete_old_interps()
	{
	interp_list *old_deleter, *old_pointer;

	old_deleter = old_interps->next_interp;
	while (old_deleter != NULL)
		{
		old_pointer = old_deleter->next_interp;
		free(old_deleter);
		old_deleter = old_pointer;
		}
	}

/****************************************************************************
*						Function null_record
*
* This function is used to determine if the current record is a null record
* of some sort (data, interpretation, or local comment).
****************************************************************************/

boolean null_record(tokens fields,char *type)
	{
	int i;
	boolean null_only = TRUE;

	for (i = 0; i < fields.number; i += 1)
		{
		if (strcmp(fields.token[i],type) != 0)
			{
			null_only = FALSE;
			break;
			}
		}
	return null_only;
	}

/****************************************************************************
*						Function exclusive_record
*
* This function is used to determine if the current record contains an
* exclusive interpretation.
****************************************************************************/

boolean exclusive_record(tokens fields)
	{
	int i;
	boolean exclusive = FALSE;

	for (i = 0; i < fields.number; i += 1)
		{
		if (strncmp(fields.token[i],"**",2) == 0)
			{
			exclusive = TRUE;
			break;
			}
		}
	return exclusive;
	}

/****************************************************************************
*						Function tandem_record
*
* This function is used to determine if the current record is a tandem
* interpretation record (i.e. not exclusive and not spine-path).
****************************************************************************/

boolean tandem_record(tokens fields)
	{
	int i;
	boolean tandem = FALSE;

	for (i = 0; i < fields.number; i += 1)
		{
		if (strncmp(fields.token[i],"**",2) != 0 
							&& strcmp(fields.token[i],"*+") != 0
			   				&& strcmp(fields.token[i],"*^") != 0
						   	&& strcmp(fields.token[i],"*-") != 0
						   	&& strcmp(fields.token[i],"*x") != 0
			   				&& strcmp(fields.token[i],"*v") != 0
							&& strcmp(fields.token[i],"*") != 0)
			{
			tandem = TRUE;
			break;
			}
		}
	return tandem;
	}

/****************************************************************************
*						Function spine_path_record
*
* This function is used to determine if the current record is a
* spine-path record.
****************************************************************************/

boolean spine_path_record(tokens fields)
	{
	int i;
	boolean spine_path = FALSE;

	for (i = 0; i < fields.number; i += 1)
		{
		if (strcmp(fields.token[i],"*+") == 0
			   				|| strcmp(fields.token[i],"*^") == 0
						   	|| strcmp(fields.token[i],"*-") == 0
						   	|| strcmp(fields.token[i],"*x") == 0
			   				|| strcmp(fields.token[i],"*v") == 0)
	   		{
			spine_path = TRUE;
			break;
			}
		}

	return spine_path;
	}

/****************************************************************************
*						Function spine_terminate
*
* This function is used to determine if this record contains only
* spine-path terminators.
****************************************************************************/

boolean spine_terminate(tokens fields)
	{
	int i;
	boolean terminate = TRUE;

	for (i = 0; i < fields.number; i += 1)
		{
		if (strcmp(fields.token[i],"*-") != 0)
			{
			terminate = FALSE;
			break;
			}
		}
	return terminate;
	}

/****************************************************************************
*						Function split_string
*
* This function takes the current record and splits it into separate fields
* according to the paramter 'delim' and stores the fields in 'this_line' as
* well as the number of fields found.
****************************************************************************/

void split_string(char *string,char delim,tokens *this_line)
	{
	char temp[LINE_LENGTH];
	char *next_char;
	int index, sub_index;

	this_line->number = 0;
	next_char = string;

	/* Determine the number of fields in the current line */

	while (*next_char != '\0')
		{
		while (*next_char == delim)
			next_char += 1;
		if (*next_char != '\0')
			this_line->number += 1;
		while (*next_char != delim && *next_char != '\0')
			next_char += 1;
		}

	/* Actually store the strings if there are any to store */

	if (this_line->number != 0)
		{
		if ((this_line->token = (char **) 
							malloc(this_line->number * sizeof(char *))) == NULL)
			print_error(MEMORY,"");

		index = 0;
		next_char = string;
		while (*next_char != '\0')
			{
			sub_index = 0;
			temp[sub_index] = '\0';
			while (*next_char == delim)
				next_char += 1;
			while (*next_char != delim && *next_char != '\0')
				temp[sub_index++] = *next_char++;
			temp[sub_index] = '\0';
			if ((this_line->token[index] = 
										(char *) malloc(LINE_LENGTH)) == NULL)
				print_error(MEMORY,"");
			strcpy(this_line->token[index],temp);
			index += 1;
			}
		}
	}

/****************************************************************************
*						Function free_tokens
*
* This function is used to free the tokens allocated by split_string
****************************************************************************/

void free_tokens(tokens fields)
	{
	int i;

	for (i = 0; i < fields.number; i += 1)
		free(fields.token[i]);
	free(fields.token);
	}

/****************************************************************************
*						Function print_error
*
* This function is used to print a terminating error to standard error and
* exit from the program with a non-zero exit code.
****************************************************************************/

void print_error(char *message,char *args)
	{
	fprintf(stderr,message,args);
	exit(1);
	}
