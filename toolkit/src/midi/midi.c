/*************************************
  midi.c : converts from humdrum
  **kern format to humdrum **MIDI
  format.

  Read it with tabstops set at 3.

      Programmer: Kyle Dawkins
      When:       Summer 1993
**************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "midi.h"

/***************************************
  The following are global variables.
  They are common to every function
  and can be accessed from any of the
  program elements in this file.
***************************************/

static	char 	kern_list[NUM_SPINES]; 			/* for spine i, kern_list[i]
															   contains KERN if spine i is 
																**kern and NOTKERN otherwise */
static	int 	delta_list[NUM_SPINES];			/* Contains the accumulated delta
															   times for each spine */
static	char 	path_list[NUM_SPINES];			/* Used for spine-path indicators:
															   when a line of SPI's is
																encountered, it is processed
															   and ADD, TERMINATE, EXCHANGE,
															   SPLIT, or JOIN is placed
															   in path_list[i] for spine i */
static	char 	chan_list[NUM_SPINES];			/* Contains the MIDI channel
															   for spine i */
static	int	active_spines;						/* The current number of spines */
static	int	kern_spines;						/* The number of kern spines */
static	char	buffer[960];						/* Our input buffer */
static	char	data[NUM_SPINES][120];			/* The data tokens. For each line,
															   "buffer" is broken into spines
															   and then results placed in the
															   data array */
static	MIDI_EVENT	note_list[NUM_SPINES];	/* contains information about
															   queued notes */
static	int	quarter = QUARTER;				/* The default number of clock-
															   ticks per quarter note. */
static	int	defdur;								/* Used for the -r option, this
															   is the specified default
															   duration */
static	int	after;								/* Flag indicating echoing
															   of comments BEFORE */
static	int	before;								/* Flag indicating echoing
															   of comments AFTER */
static	int	doubled;								/* Flag indicating that all
															   note-off commands are to
															   be output */
static	int	recip;								/* Flag indicating that the
															   -r option has been used */
static	FILE	*infile;								/* input file */
static	FILE	*outfile;							/* output file (stdout) */
static	unsigned long int abstime;				/* absolute time */
static	unsigned long int lastout;				/* absolute time at last note
																printing. */
static	int note_on_line;							/* Is there a real note-on or
																note-off command on the line? */
static	int dstop;									/* which double stop are we
																processing? */
static	int grace_note;								/* flag to say that we just read
																a grace-note in */
static	int groupetto;								/* groupetto indicator */


/***************************************
  getnext : reads in next non-comment
  line, echoing comments.
***************************************/

char	*getnext ( int *line )
{
	char	*rc;

	do
	{
		rc = fgets (buffer, 960, infile);
		*line += 1;
		if (rc && buffer[0] == '!')
		{
			if (buffer[1] == '!')
				fprintf(outfile, "%s", buffer);
			else
				fprintf(outfile, "!!%s", buffer);
		}
	} while (buffer[0] == '!' && rc != NULL );

	return rc;
}

/**************************************
  usage : prints a usage message
**************************************/

void	usage (void)
{
	fprintf(stderr,"usage: midi [-cCu] [-q <num>] [-d <num>] [inputfile]\n");
}

/**************************************
  help : prints out the help screen
**************************************/

void	help ( void )
{
	puts("");
	puts("MIDI      : Convert **kern to **MIDI\n");
	puts("     This command is used to convert **kern data to **MIDI data.");
	puts("");
	puts("Syntax:");
	puts("");
	puts("     midi [-c] [-C] [-u] [-d n] [-q n]  inputfile");
	puts("");
	puts("Options:");
	puts("");
	puts("     -c   : echo all data records as global comments BEFORE the corresponding");
	puts("            MIDI events");
	puts("     -C   : echo all data records as global comments AFTER the corresponding");
	puts("            MIDI events");
	puts("     -d n : reassign all note durations to n, where n is a valid **kern or");
	puts("            **recip duration");
	puts("     -q n : set the number of MIDI clockticks per quarter note to n (default");
	puts("            is 72)");
	puts("     -u   : suppress the deletion of duplicate (unison) concurrent note-ons");
	puts("            instructions");
	puts("");
	puts("  Refer to reference manual for further details.");
	puts("");
}

/**************************************
  find_note: returns the MIDI
  note value of a kern pitch
**************************************/

int	find_note ( char *p )
{
	int	note;
	int	done;
	int	octave;
	int	accidentals;
	char	*token;
	char	temp;

	static	int	note_vals[7] = { 69, 71, 60, 62, 64, 65, 67 };

	token = p;
	note = 0;
	done = FALSE;

	/***************************************
	  First we find the root pitch
	  ignoring all octave indications,
	  accidentals, etc.
	***************************************/

	while ( *p != 0 && !done )
	{
		temp = tolower (*p);

		if (temp >= 'a' && temp <= 'g')
		{
			if (note == 0)
				note = note_vals[ temp - 'a' ];
		
			done = TRUE;
		}
		p++;
	}	

	/**************************************
	  Now we do octave stuff and
	  accidentals
	**************************************/

	p = token;
	octave = 0;
	accidentals = 0;

	while ( *p != 0 )
	{
		if ( *p >= 'A' && *p <= 'G' )	
		{
			octave = octave - 1;
		}
		else if ( *p >= 'a' && *p <= 'g' )
		{
			octave = octave + 1;
		}
		else if ( *p == '#' )
		{
			accidentals = accidentals + 1;
		}
		else if ( *p == '-' )
		{
			accidentals = accidentals - 1;
		}
		p++;
	}

	if (octave > 0)
		octave = octave - 1;

	note = note + 12 * octave + accidentals;

	/* Is it a rest or the end of a tie? */

	if ( strchr (token, 'r') )
		note = REST;
	else if ( strchr (token, ']') || strchr (token, '_') )
		note = TIE;

	return (note);
}

/****************************************
  find_duration : calculates the
  duration of a given note
****************************************/

int	find_duration	( char *p )
{
	char *durtok;
	int	duration;
	int	dots;
	int	done;

	durtok = p;
	done = FALSE;
	duration = 0;

	while ( !done && *p != 0 && *p != ' ' )
	{
		if ( *p >= '0' && *p <= '9' )
		{
			duration = atoi ( p );
			done = TRUE;
		}
		else
			p ++;
	}

	if ( done )
	{
		if (duration != 0)
			duration = (4*quarter) / duration;
		else
			duration = (8*quarter);

		/*******************************
		  Check if it's a dotted note
		*******************************/

		dots = duration / 2;

		while ( *p >= '0' && *p <= '9' )
			p++;

		while ( *p == '.' )
		{
			duration = duration + dots;
			dots = dots / 2;
			p++;
		}
	}

	if ( strchr(durtok, 'q') )
	{
		duration = quarter / 16;
		grace_note = TRUE;
	}
	else if ( strchr(durtok, 'Q') )
	{
		duration = quarter / 24;
		groupetto = TRUE;
	}

	return duration;
}

/**************************************
  init_arrays : initialises all of
  the arrays concerning the
  translation
**************************************/

void	init_arrays (void)
{
	int i;

	for (i=0; i< NUM_SPINES; i++)
	{
		kern_list[i] = NOTKERN;
		delta_list[i] = quarter;
		path_list[i] = NOTHING;
		chan_list[i] = 1;
		note_list[i].next = NULL;
	}
}

/**************************************
  same_note : looks for other
  occurrences of a given note in the
  note table. Returns TRUE if it finds
  other occurrences of the same note
**************************************/

int	same_note ( int spine, int note )
{
	MIDI_PTR current;
	int	k;
	int rc;
	
	rc = FALSE;

	for (k=0; k<active_spines; k++)
	{
		if (kern_list[k]==1 && chan_list[k]==chan_list[spine] &&
			k != spine)
		{
			current = note_list[k].next;

			while (current != NULL && !rc)
			{
				if (current->note == note)
					rc=TRUE;
				current = current->next;
			}
		}
	}

	if (doubled == TRUE)
		rc = FALSE;

	return rc;
}

/*************************************
  remove_notes : traverses list of
  queued notes from a given spine,
  frees the memory associated with
  the spine, and prints out a 
  "note off" command.
*************************************/

int		remove_notes ( int spine, unsigned long delta )
{
	MIDI_PTR	current;
	MIDI_PTR	new;
	int	rc;

	current = note_list[spine].next;

	rc = FALSE;

	while (current != NULL)
	{
		if (current->note != REST)
		{
			if ( !same_note (spine, current->note) )
			{
				if (dstop > 0)
					fprintf(outfile, " ");
				fprintf(outfile, "%lu/-%d/64", delta, current->note);
				dstop++;
				note_on_line = TRUE;
				rc = TRUE;
			}
		}

		new = current->next;
		current->next = NULL;
		free (current);
		current = new;
	}
	note_list[spine].next = NULL;

	return rc;
}

/***************************************
  add_note : queues a note in a list
  for a given spine. This list is used
  by the remove_notes function
  to determine which notes are to
  be turned off.
***************************************/

void	add_note ( int spine, int note )
{
	MIDI_PTR	current;

	current = &(note_list[spine]);

	while (current->next != NULL)
		current = current->next;

	current->next = (MIDI_PTR) malloc (sizeof(MIDI_EVENT));
	current = current->next;
	current->note = note;
	current->next = NULL;
}

/*************************************
  tandem : process tandem
  interpretations. This does NOT
  include spine-path indicators,
  or exclusive interpretations.
*************************************/

void	tandem ( char *token )
{
	int	tempo, num_matched;
	float	low, high;

	num_matched = sscanf(token, "*MM%f-%f", &low, &high);

	if (num_matched == 1)
	{
		tempo = (int) low;
		fprintf(outfile, "*MM%d", tempo);
	}
	else if (num_matched == 2)
	{
		tempo = (int) ( (low+high)/2.0 );
		fprintf(outfile, "*MM%d", tempo);
	}
	else
	{
		fprintf(outfile, "%s", token);
	}
}

/*************************************
  split : splits line into spines
  and places the results in the
  data[][] array.
*************************************/

void split (void)
{
	int i, j, k;

	for (i=0, j=0, k=0; i<active_spines; i++)
	{
		while (buffer[j] != '\t' && buffer[j] != '\n' && buffer[j] != 0)
		{
			data[i][k] = buffer[j];
			j = j + 1;
			k = k + 1;
		}

		data[i][k] = '\0';
		data[i][k+1] = '\0';
		k = 0;
		j++;
	}
}

/*************************************
  read_data : the main loop for the
  conversion utility
**************************************/

void	read_data (void)
{
	char	*ptr;				/* pointer into the current data token */
	char	note_temp[60];	/* array into which the current stop in the 
								   current spine is copied for analysis. */
	int	note; 			/* holds the MIDI note value for a token */
	int	duration;		/* holds the MIDI duration in ticks for a token */
	int	last_delta;		/* is the temporal interval between successive
								   data records, calculated after each line is
								   read in. */
	int	min_delta; 		/* The smallest delta-time between successive
								   data records. Used to calculate last_delta */
	int	decrease;		/* Flag indicating a JOIN or TERMINATE spine path
								   indicator has been encountered */
	int	increase;		/* Flag indicating an ADD or SPLIT spine path
								   indicator has been encountered */
	int	checked;			/* Used in double stops, since double stops all
								   have the same duration, after the first one is
								   checked, the delta-time for the other can
								   be set to zero */
	int	done;				/* We are done if we are at the end of the line */
	int	line; 			/* line number into the file */
	int	tab;				/* Flag set when first tab has been output on a line */
	int	barnumber;		/* Guess. */
	int	exchange;		/* Temporary variable used for swapping 2 variables */
	MIDI_EVENT	nexchange;			/* same as above but for MIDI_EVENTs */
	int	score_data;		/* Flag set when a Kern data record is read in */
	int	join;				/* Flag set when a join is encountered and unset
								   when the first non-join spine is encountered */
	int	exclusive;		/* Set when exclusive interpretations are read in.
								   This was unfortunately necessary because **kern
								   maps to TWO interpretations: **MIDI and *Ch1 */
	int	freed;			/* Flag is set when notes have been freed from the
								   note queue */
	int	header;			/* Set when we're reading a header */
	int	suppress;		/* Set when we want to suppress printing */
	int	nulls;			/* Set when we have a line of null data records. */
	char	*rc2;				/* Generic char return code */
	int	rc;				/* Generic return code, but used here to indicate if
								   a function has echoed any output or not */
	int	i, j, k;			/* counters */
	int	barlines;		/* flag if a barline has been encountered in any
									of the kern spines */

	/************************************
	  First we read through the file
	  ignoring comments until we come to
	  the first valid line and use it
	  to figure out how many active
	  spine we have to begin with
	************************************/

	rc2 = getnext ( &line );

	if (rc2 == NULL)
		return;

	lastout = 0;
	abstime = quarter;

	do
	{
		init_arrays ();
		active_spines = 0;
		last_delta = quarter;
		min_delta = 0;
		barnumber = 0;
		header = TRUE;
		kern_spines = 0;

		line = 1;
	
		for (i=0; buffer[i] != '\n' ; i++)
		{
			if (buffer[i] == '\t')
				active_spines = active_spines + 1;
		}
	
		active_spines += 1;

		for (i=0; i< active_spines; i++)
			note_list[i].next = NULL;
	
		/*************************************
		  Our main loop. Here we take one
		  line at a time, break it into
		  spines, and then break each spine
		  up into note information, which
		  we then store in the buffer. Also,
		  if we encounter any add-spine
		  or terminate-spine representations,
		  we act accordingly.
		**************************************/
		  
		do 
		{
			note_on_line = FALSE;

			if (before)
			{
				fprintf(outfile, "!!%s", buffer);
			}

			/**********************************
			  Break line into separate pieces
			  of data, one for each spine.
			**********************************/
	
			exclusive = FALSE;
			join = FALSE;
			score_data = FALSE;
			suppress = FALSE;
			barlines = FALSE;

			split ();

			/**********************************
			  Now that we've broken the line
			  up, we can look at the tokens
			**********************************/
	
			increase = FALSE;
			decrease = FALSE;

			/*****************************************************
			  This is tricky: we first assume that whatever line 
			  we have read in is all null tokens. We have three 
			  cases where we change our tune: 
			  1) If ANY kern spines have non null tokens.
			  2) If we are reading a header with interpretations
			  3) If the number of kern spines is zero
			*****************************************************/

			nulls = TRUE;
			for ( i=0; i < active_spines && nulls==TRUE; i++)
			{
				if ( (kern_list[i] == KERN && data[i][0] != '.') || header ||
						kern_spines == 0)
					nulls = FALSE;
			}

			/**************************************************
			  If the nulls flag is negative, then we have
			  found data to process, so we process it.
			  Otherwise we just continue on to the next line.
			**************************************************/

			grace_note = FALSE;
			groupetto = FALSE;

			if (!nulls)
			{
				for (i=0; i<active_spines; i++)
				{
					freed = FALSE;
					rc = FALSE;
					dstop = 0;
					
					if (!strcmp(data[i], "*+"))
					{
						increase = TRUE;
						suppress = TRUE;
						path_list[i] = ADD;
						join = FALSE;
					}
					else if (!strcmp(data[i], "*^"))
					{
						increase = TRUE;
						suppress = TRUE;
						path_list[i] = SPLIT;
						join = FALSE;
					}
					else if ( !strcmp(data[i], "*-") )
					{
						decrease = TRUE;
						path_list[i] = TERMINATE;
						suppress = TRUE;
						join = FALSE;
					}
					else if (!strcmp(data[i], "*v"))
					{
						suppress = TRUE;
						if (join)
						{
							decrease = TRUE;
							path_list[i] = TERMINATE;
						}
						else
						{
							join = TRUE;
						}
					}	
					else if (!strcmp(data[i], "*x"))
					{
						path_list[i] = EXCHANGE;
						increase = TRUE;
						suppress = TRUE;
						join = FALSE;
					}
					else if (!strcmp(data[i], "**kern"))
					{
						if (kern_list[i] != 1)
							kern_spines+=1;	
						kern_list[i] = KERN;
						fprintf(outfile, "**MIDI");
						exclusive = TRUE;
						join = FALSE;
					}
					else if ( !strncmp(data[i], "*Ch", 3) )
					{
						sscanf(data[i], "*Ch%d", &chan_list[i]);
						fprintf(outfile, "%s", data[i]);
						join = FALSE;
					}
					else if ( !strncmp(data[i], "**", 2) )
					{
						fprintf(outfile, "%s", data[i]);
						join = FALSE;
					}
					else if ( data[i][0] == '*' )
					{
						if (!strstr(buffer, "\t**"))
						{
							suppress = TRUE;
							if (data[i][1] == '\0')
							{
								join = FALSE;
							}
						}
						else
							fprintf(outfile, "*");
					}
					else if (data[i][0] == '.')
					{
						/*******************************************************
						  If we read in a null token, we know that the whole
						  line is not nulls because we could not be here if it
						  were, so we echo the null and decrement the delta_list
						  by the correct amount.
						*******************************************************/

						fprintf(outfile, ".");
						score_data = TRUE;
	
						if (kern_list[i]==KERN)
							delta_list[i] = delta_list[i] - last_delta;
	
						if (delta_list[i] < 0 && kern_list[i] == KERN)
						{
							delta_list[i] = 0;
							fprintf(stderr, 
									"Timing error at line %d, spine %d.\n", line, i);
							fprintf(stderr, "%s", buffer);
							exit (1);
						}
					}
					else if (data[i][0] == '=')
					{
						/*******************************************************
						  We read in a bar line. We therefore scan in the bar-
						  number, echo it, and reset the delta_list, so that
						  if any are slightly out of synch due to integer
						  division, they are equalised here
						*******************************************************/

						sscanf (data[i], "=%d", &barnumber);
						fprintf(outfile, "%s", data[i]);
						barlines = TRUE;

						/* Equalising deltas: */

						for (j=0;j<active_spines;j++)
							delta_list[j] = last_delta;
					}
					else if ( kern_list[i] == NOTKERN )
					{
						fprintf (outfile, "%s", data[i]);
					}
					else
					{
						/**************************************
					  	  Here, the line has been split into
					  	  spines, each stored in data[i].
					  	  Now we process each data[i] in turn
					  	  and split it into multiple stops.
					 	**************************************/
		
						done = FALSE;
						checked = FALSE;
						ptr = data[i];
		
						while ( !done )
						{
							/****************************************
							  Copy each stop into note_temp and
							  process it in turn. 
							****************************************/

							for (j=0;ptr[j]!=' '&&ptr[j]!='\0';j++)
								note_temp[j] = ptr[j];

							note_temp[j] = '\0';

							/***********************************************
							  First we find the MIDI note number corresp-
							  onding to the kern pitch token
							***********************************************/

							note = find_note ( note_temp );
							if (strchr(data[i],']') || strchr(data[i],'_'))
								note = TIE;

							/***********************************************
							  Then we find the duration. If the recip flag
							  is set then the user has specified a duration
							  override, and we set the duration to this.
							************************************************/

							if (recip)
								duration = defdur;
							else
								duration = find_duration ( note_temp );

							/************************************************
							  If the note is a rest and is NOT the last
							  stop of the token, we must suppress the echo-
							  ing of the space between stops because another
							  routine will do it for us.
							************************************************/

							if ( note == REST && strchr(ptr,' ') )
							{
								/* Do nothing... there was some code here but I deleted it */
							}
							else
							{
								/***************************************
						  	  	  If we just read in a bona-fide note,
						  	  	  and we haven't freed all of the queued
						  	  	  notes, we free them and mark them as
						  	  	  freed
								***************************************/
			
								if ( note != 0 )
								{
									if (note != TIE)
									{
										if (!freed)
										{
											rc = remove_notes ( i, abstime-lastout );
											freed = TRUE;
										}
									}
									else
									{
										if (dstop == 0)
											fprintf(outfile, ".");
										rc = TRUE;
									}
									score_data = TRUE;
								}
			
								/******************************************
						  	  	  If we haven't already done it for this
						  	  	  spine (only in the case of multiple stops
						  	  	  do we reach this line more than once) we
						  	  	  calculate the delta differential based
						  	  	  on the last delta time, and the duration
						  	  	  of the note we just read in.
								*******************************************/
			
								if (!checked)
								{
									delta_list[i] = delta_list[i]+
													duration-last_delta;
									checked = TRUE;
								}
			
								/*******************************************
							  	  delta_list[i] should never become
							  	  negative, but if it does, in the case
							  	  of faulty input data, set it to zero and
							  	  continue.	
								*******************************************/
			
								if ( delta_list[i] < 0)
								{
									delta_list[i] = 0;
								}
			
								/*********************************************
						  	  	  Finally, after all of the checks and
						  	  	  queues and so forth, we can now insert
						  	  	  the note into the queue, and print out a
						  	  	  note-on command as long as it's not a 
						  	  	  rest or a tied note
								*********************************************/
			
								if (note != 0)
								{
									if (note != TIE)
										add_note ( i, note );
	
									if (note != REST && note != TIE)
									{
										if (dstop > 0)
											fprintf(outfile, " ");
										fprintf(outfile,"%lu/%d/64",abstime-lastout,
												  note);
										dstop++;
										note_on_line = TRUE;
									}
									else
									{
										if (note == REST)
										{
											if (dstop == 0)
												fprintf(outfile, ".");
										}
										else if (dstop == 0)
										{
											fprintf(outfile, ".");
										}
									}
								}
								else
								{
									done = TRUE;
									if (dstop == 0)
										fprintf(outfile, ".");
								}
							}
	
							/********************************
						  	 Move to the next field in the
						  	 spine if there is one,
						  	 otherwise, exit from loop.
							*********************************/

							while ( *ptr != ' ' && *ptr != 0 && *ptr != '\n' )
							ptr ++;
			
							switch ( *ptr )
							{	
								case '\n' :
								case '\0' :
									done = TRUE;
									break;
								case ' ' :
									ptr ++;
									break;
								default:
									break;
							}
						}
					}
	
					if (suppress != TRUE)
					{	
						if ( i < active_spines - 1)
						{
							fprintf (outfile, "\t");
						}
						else if (i != active_spines)
						{
							fprintf (outfile, "\n");	
						}
					}
				}
					
				if (note_on_line)
				{
					lastout = abstime;
				}

				/***************************************
			  	  Calculate the smallest time
			  	  differential from the new line of
			  	  note information. The smallest one
			  	  will be the delta-time for the next
			  	  MIDI event.
				***************************************/
		
				if (grace_note || groupetto)
				{

					/****************************************************
					   if we had a line with a grace_note or a groupetto
						on it, we have to adjust the timings of the
						other voices so that the overall synchronisation
						is not all screwed up.
					*****************************************************/

					for (i=0; i<active_spines; i++)
					{
						/*
						if (delta_list[i] == 0)
						{
							delta_list[i] = 16*quarter;
						}
						*/
						if (groupetto && delta_list[i] != quarter/8)
						{
							delta_list[i] = delta_list[i] + quarter/8;
						}
						else if (grace_note && delta_list[i] != quarter/16)
						{
							delta_list[i] = delta_list[i] + quarter/16;
						}
					}
				}

				min_delta = 16 * quarter;

				/* printf("!! "); */

				for (i=0; i<active_spines; i++)
				{
					/* printf("%d ", delta_list[i]); */

					if ( kern_list[i] == KERN && delta_list[i] < min_delta)
						min_delta = delta_list[i];
				}
				/* printf(" Grace note : %d Quarter: %d\n", grace_note, quarter); */
		
				if ( score_data )
				{ 
					if ( min_delta != 16 * quarter )
					{
						last_delta = min_delta;
					}

					if (buffer[0] != '*' && !barlines)
						abstime += last_delta;

					header=FALSE;
				}


				/****************************************************
				  If decrease is set then we have received a
				  spine path indicator which involves terminating
				  at least one spine. So, we pop off all of the
				  queued notes in those spines BEFORE we terminate
				  them.
				****************************************************/

				if (decrease)
				{
					for (i=0; i< active_spines; i++)
					{
						dstop = 0;
						if (path_list[i] == TERMINATE && kern_list[i] == 1)
						{
							rc = remove_notes ( i, abstime-lastout );
							if (rc)
								freed = TRUE;
							else
								fprintf(outfile, ".");
						} 
						else
						{
							fprintf(outfile, ".");
						}
	
						if (i< active_spines-1)
							fprintf(outfile, "\t");
						else
							fprintf(outfile, "\n");
					}
	
					if (freed)
					{
						for (i=0; i< active_spines; i++)
						{
							if (kern_list[i] == 1)
								delta_list[i] = delta_list[i] - last_delta;
						}

						abstime = abstime - last_delta;
						last_delta = 0;
					}
				} 

				/*******************************************************
				  If either increase or decrease is set, we are doing
				  something with spine path indicators.
				********************************************************/

				if (increase || decrease)
				{
					for ( i=0; i<active_spines;) 
					{
						switch ( path_list[i] )
						{
							case ADD:
								path_list[i] = NOTHING;
								for (j = active_spines-1; j >= i; j--)
								{
									kern_list[j+1] = kern_list[j];
									delta_list[j+1] = delta_list[j];
									path_list[j+1] = path_list[j];
									memcpy( &(note_list[j+1]), &(note_list[j]),
											sizeof(MIDI_EVENT));
								}
								active_spines = active_spines + 1;
								i++;
								path_list[i] = NOTHING;
								kern_list[i] = NOTKERN;
								if (kern_list[i-1] == 1)
									delta_list[i] = delta_list[i-1];
								else
									delta_list[i] = 0;
								note_list[i].next = NULL;
								i++;
								break;
							case SPLIT:
								path_list[i] = NOTHING;
								for (j = active_spines-1; j >= i; j--)
								{
									kern_list[j+1] = kern_list[j];
									delta_list[j+1] = delta_list[j];
									path_list[j+1] = path_list[j];
									memcpy( &(note_list[j+1]), &(note_list[j]),
											sizeof(MIDI_EVENT));
								}
								active_spines = active_spines + 1;
								if (kern_list[i] == 1)
									kern_spines++;
								i++;
								path_list[i] = NOTHING;
								kern_list[i] = kern_list[i-1];
								delta_list[i] = delta_list[i-1];
								note_list[i].next = NULL;
								i++;
								break;
							case TERMINATE:
								path_list[i] = NOTHING;
								if (kern_list[i] == 1)
									kern_spines--;
								for (j = i; j < active_spines-1; j++) 
								{
									kern_list[j] = kern_list[j+1];
									delta_list[j] = delta_list[j+1];
									path_list[j] = path_list[j+1];
									memcpy( &(note_list[j]), &(note_list[j+1]),
											sizeof(MIDI_EVENT));
								}
								active_spines = active_spines - 1; 
								kern_list[active_spines] = NOTKERN;
								path_list[active_spines] = NOTHING;
								note_list[active_spines].next = NULL;
								break;
							case EXCHANGE:
								path_list[i] = NOTHING;
								for (k=i+1;k<active_spines && 
									path_list[k]!=EXCHANGE; k++ );
	
								if (k<active_spines)
								{
									path_list[k] = NOTHING;
									exchange = kern_list[k];
									kern_list[k] = kern_list[i];
									kern_list[i] = exchange;
									exchange = delta_list[k];
									delta_list[k] = delta_list[i];
									delta_list[i] = exchange;
									exchange = chan_list[k];
									chan_list[k] = chan_list[i];
									chan_list[i] = exchange;
									memcpy( &nexchange, &(note_list[k]),
											sizeof(MIDI_EVENT));
									memcpy( &(note_list[k]), &(note_list[i]),
											sizeof(MIDI_EVENT));
									memcpy( &(note_list[i]), &nexchange,
											sizeof(MIDI_EVENT));
								}
								i++;
								break;
							default:
								i++;
								break;
						}
					}	
					fprintf(outfile, "%s", buffer);
					suppress = FALSE;
				}

				if (suppress)
				{
					/***********************************************
					  If the suppress flag was set, then we are now
					  required to print out the line, because we
					  have already suppressed printing above.
					************************************************/

					tab = FALSE;

					for (i=0; i< active_spines; i++)
					{
						/************************************************
						  For each token in the line, we call the 
						  tandem(char *) function which looks at the
						  string we pass it and echoes back whatever it
						  is supposed to. Currently it translates tempo
						  markings, but other things can be added
						************************************************/

						if (tab)
						{
							fprintf(outfile, "\t");
							tandem(data[i]);
						}
						else
						{
							tandem(data[i]);
							tab = TRUE;
						}
					}
					fprintf(outfile, "\n");
				}
		
				/********************************************
			  	  I admit it, this is a hack. If we just
			  	  read in a **kern exclusive interpretation,
			  	  we assign all of the spines to MIDI
			  	  channel 1. However, if another channel
				  indicator comes along (explicitly placed
				  by the user), that will supercede this.
				********************************************/
		
				if (exclusive)
				{
					for (i=0; i< active_spines; i++)
					{
						if ( kern_list[i] != NOTKERN )
							fprintf (outfile, "*Ch1");
						else
							fprintf(outfile, "*");
		
						if (i < active_spines-1)
							fprintf(outfile, "\t");
					}
					fprintf(outfile, "\n");
				}
	
				if (after)
				{
					fprintf(outfile, "!!%s", buffer);
				}
			} /* if */
			/* printf("!!abstime: %lu, last printed: %lu, note_on_line: %d\n", 
						abstime, lastout, note_on_line);
			*/
		} while ( active_spines > 0 && getnext ( &line ) );
	} while ( getnext (&line ) );
}

/***************************************
  Program mainline.
***************************************/

int	main 	(int argc, char *argv[])
{
	int	i,j;
	int	done;

	quarter = QUARTER;
	infile = stdin;
	outfile = stdout;
	before = FALSE;
	after = FALSE;
	recip = FALSE;
	doubled = FALSE;

	/********************************************
	  Command line argument processing.
	  Currently available options:
	  -c : echoes every line as a global comment
	  -C : echoes every line as global comment
		    AFTER the corresponding MIDI line
	  -d : enables note off commands for all
	       notes, even doubled ones (by default
	       this is disabled
	  -q <num> : sets the value of a quarter
	       note to <num> clockticks (default 72)
	  -r <num> : overrides all duration
	       and sets every duration to <num>
	********************************************/

	if (argc > 9)
	{
		usage();
		printf("num args\n");
		exit(1);
	}
	else
	{
		for (i=1; i< argc; i++)
		{
			if (argv[i][0] == '-')
			{
				done = FALSE;
				for (j=1; j<strlen(argv[i]) && !done ; j++)
				{
					switch (argv[i][j])
					{
						case 'h':
							help();
							exit(1);
							break;
						case 'c':
							before = TRUE;
							break;
						case 'C':
							after = TRUE;
							break;
						case 'u':
							doubled = TRUE;
							break;
						case 'q':
							if (i<argc)
							{
								quarter = atoi ( argv[i+1] );
								i++;
								done = TRUE;
							}
							else
							{
								usage();
								exit(1);
							}
							break;
						case 'd':
							if (i<argc)
							{
								defdur=find_duration(argv[i+1]);
								recip = TRUE;
								i++;
								done = TRUE;
							}
							else
							{
								usage ();
								printf("duration\n");
								exit(1);
							}
							break;
						default:
							usage();
							printf("%d default\n", i);
							exit(1);
							break;
					}
				}
			}
			else
			{
				infile = fopen(argv[i], "r");
				if (infile == NULL)
				{
					fprintf(stderr, "midi: file not found: %s\n", argv[i]);
					exit(1);
				}
			} /* if */
		} /* for */
	}
	read_data ();
}
