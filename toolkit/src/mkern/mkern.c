/*******************************************************
   mkern.c : mkern takes data in the stage two
	musedata format distributed by the Center (Centre)
	For Computer Assisted Research in the Humanities,
	or CCARH.

	Tabstops must be set to 3 to read this file.

	Revision history:						Date:
	Written: Kyle Dawkins				June 27th, 1994
	Altered: Kyle Dawkins				July 28th, 1994
		Added -r option to read info from rc file.
	Altered: Kyle Dawkins				July 29th, 1994
		Added -i option to read info from user.
	Altered: Kyle Dawkins				August 2/3, 1994
		Added translation of MuseData repeats and form-
		dictating functions.
		Added -C option to comment unknown directives.
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#ifdef __MSDOS__
#include <alloc.h>
#endif

#ifdef __linux__
#include <malloc.h>
#endif

#ifdef  __APPLE__
#include <sys/malloc.h>
#endif

#include "mkern.h"

/************************************************
   Global variables. Yes, I use them. So what?
************************************************/

int	timebase;
int	generate_script;
char	buffer[960];
char	infilename[80];
char	indirname[1024];
char	outdirname[1024];
char	outfilename[80];
char	rcfilename[80];
char	scriptfilename[80];
char	targetfilename[80];
char	stack[100];
int	stackptr;
int	diatonic=0;
int	chromatic=0;
FILE	*infile;
FILE	*outfile;
FILE	*rcfile;
char	sect = 'A';
int	newsect;
int	qdiv=1;
int	tie_state = 0;
int	comments;
int	double_stop;
long 	int memory;
long	int back;
char	instrument[20];
int	pizz;
int	readrc;
int	interactive;
int	verbose;
int	comment_unknowns;
#ifdef __MSDOS__
unsigned char huge *pool;
unsigned char huge *cur;
#else
unsigned char *pool;
unsigned char *cur;
#endif

/* Necessary prototypes */

void	printform (int , FILE *);

/************************************************
	help : prints out a help page
************************************************/

void	help (void)
{
	printf("\n\nThe MKERN command is used to convert from MuseData to humdrum **kern format.\n\n");
	printf("The following is the valid syntax for invoking the MKERN command:\n\n");
	printf("\tmkern -h\n");
	printf("\tmkern [-c] [-v] [-i] [-r rcfile] inputfile\n");
	printf("\n");
	printf("Here is a brief description of the valid options:\n\n");
	printf("-h : displays this screen\n");
	printf("-c : echoes input lines as global comments before the corresponding **kern line\n");
	printf("-v : verbose; echoes information about the conversion process\n");
	printf("-i : interactive; lets the user interactively choose source and target\n");
	printf("     directories for conversion\n");
	printf("-r rcfile: read rcfile for information on conversion process, then convert files\n");
	printf("\nRefer to reference manual for more details.\n");
}

/************************************************
  dumppool : prints out the data pool
************************************************/

void	dumppool ( FILE *outf )
{
	int 	i;
	char	*ptr;
	char	*halt;

	i = stackptr - 1;
	stack[stackptr] = '\0';

	halt = strstr(pool, "*>A");

	ptr = pool;

	while ( *ptr != '\0')
	{
		if (ptr == halt)
		{
			fprintf(outf, "*>[");
			printform ( i, outf );
			fprintf(outf, "]\n");
		}
		fputc(*ptr, outf);
		ptr++;
	}
}

/************************************************
  crapout: Puts out crap and then craps out
************************************************/

void	crapout ( char *text )
{
	fprintf(stderr, "mkern: %s", text);
	exit (1);
}

/*************************************************
	transpose: transpose a given kern token
*************************************************/

void	transpose ( char *token )
{
	int 	i, j;
	char 	*p;
	char	note [20] = "";
	char	acc [20] = "";
	int	pcmap[7] = { 9, 11, 0, 2, 4, 5, 7 };
	int	map[7] = { 5, 6, 0, 1, 2, 3, 4 };
	char	ascmap[8] = "cdefgab";
	char	notechar;
	int	origin, target;
	int	diff;
	int	accidental = 0;
	int	modulus;
	int	octave = 0;
	int	notenum = 0;

	if (strchr(token, 'r') || abs(chromatic) > 19 )
		return;

	p = token;

	i=0;
	j=0;

	while ( (isalpha(*p) || *p == '#' || *p == '-') && *p != '\0')
	{
		if (isalpha(*p) && *p != 'n')
		{
			note[j++] = *p;
		}
		else
		{
			acc[i++] = *p;
		}
		p++;
	}

	note[j] = '\0';
	acc[i] = '\0';

	/* assembling token */

	p = note;

	while (*p!='\0')
	{
		if (isupper(*p))
		{
			octave = octave - 1;
		}
		else
		{
			if (p!=note)
			{
				octave = octave + 1;
			}
		}
		p++;
	}

	notenum = map [((toupper(note[0])-'A') %7)];
	origin = pcmap [((toupper(note[0])-'A') %7)] + 12*octave;

	if ((notenum+diatonic) > 6)
		octave ++;
	else if ((notenum+diatonic) < 0)
		octave --;

	if (diatonic < 0)
		modulus = (7+diatonic)%7;
	else
		modulus = diatonic;

	notenum = map [ ((toupper(note[0])-'A'+modulus) % 7)];
	target = pcmap [((toupper(note[0])-'A'+modulus) % 7)] + 12*octave;

	diff = target - origin;

	notechar = ascmap[notenum];

	if (octave < 0)
	{
		notechar = toupper(notechar);
		octave += 1;
	}

	p = note;

	for (i=abs(octave); i>=0; i--)
	{
		*p = notechar;
		p++;
	}

	*p = '\0';

	/*************************************
	   the token is transposed, so we
		must transpose the accidental
		as well using the chromatic
		information supplied
	*************************************/

	accidental = 0;
	p = acc;

	if (diff!=chromatic || *p!= '\0')
	{
		while (*p!= '\0')
		{
			switch(*p)
			{
				case '#':
					accidental++;
					break;
				case '-':
					accidental--;
					break;
				case 'n':
					break;
				default:
					break;
			}
			p++;
		}

		accidental = accidental - diff + chromatic;

		p = acc;

		if (accidental < 0)
		{
			for (i=accidental;i<0;i++)
			{
				*p++='-';
			}
		}
		else if (accidental > 0)
		{
			for (i=accidental; i>0; i--)
			{
				*p++= '#';
			}
		}
		else
		{
			*p++='n';
		}
	}

	*p = '\0';

	sprintf(token, "%s%s", note, acc);
}

/*************************************************
	getnext: gets the next line of input and
	stores it in "buffer"
*************************************************/

char	*getnext ( FILE *inf )
{
	double_stop = FALSE;
	return fgets (buffer, 960, inf);
}

/*************************************************
	header: given a file pointer, this reads
	through the header and converts it into
	humdrum comments, setting any globals
	if ever required (not yet).  Returns with
	first non-header record.
*************************************************/

void	header ( FILE *infile )
{
	int i;
	int done;

	cur += sprintf(cur, "!! Generated by translation program from file %s\n", infilename);

	for (i=0; i<11; i++)
	{
		getnext(infile);
		if (i==8)
		{
			instrument[0] = '*';
			instrument[1] = '\0';
			strcat(instrument, buffer);
		}
		cur += sprintf(cur, "!! %s", buffer);
	}

	done = FALSE;

	while (!done)
	{
		getnext(infile);

		if ( !strstr( buffer, "part" ) )
			done = TRUE;
		else
			cur += sprintf(cur, "!! %s", buffer);
	}

	cur+= sprintf (cur, "**kern\n");
	if ( strlen (instrument) > 0 )
	{
		cur += sprintf (cur, "%s", instrument );
	}
}

/*************************************************
	process_attributes: sets various variables
	and does various things that are dictated by
	the attributes field in a musedata file
*************************************************/

void	process_attributes (void)
{
	int	i,j,k;
	char	tempstr[80];
	int	tempnum;
	int	sign;
	char	circle[7][2] = { "f", "c", "g", "d", "a", "e", "b" };
	int	pcmap[8] = { 9, 11, 0, 2, 4, 5, 7, 9 };

	i = 0;

	while (buffer[i] != '\0')
	{
		switch (buffer[i])
		{
			case 'Q':
				i+=2;
				tempnum = atoi( &(buffer[i]) );
				if (tempnum > 0)
					qdiv = tempnum*8;
				if (tempnum * 4 > timebase)
					timebase = tempnum * 4;	
				break;
			case 'K':
				/* convert key signature */
				i+=2;
				tempnum = atoi( &(buffer[i]) );
				if (tempnum==0)
				{
					break;
				}
				else if (tempnum > 0)
				{
					k = tempnum;
					tempnum = 0;
					j = 1;
				}
				else
				{
					k = tempnum+6;
					tempnum = 6;
					j = -1;
				}

				strcpy(tempstr, "*k[");

				while (tempnum != k && tempnum >= 0 && tempnum < 7)
				{
					strcat(tempstr, circle[tempnum]);
					if (j>0)
						strcat (tempstr, "#");
					else
						strcat (tempstr, "-");
					tempnum = tempnum + j;
				}
				strcat(tempstr, "]");
				cur += sprintf(cur, "%s\n", tempstr);
				break;
			case 'T':
				/* time signature */
				sscanf( &(buffer[i]), "T:%d/%d", &j, &k);

				/* Convert from musedata special time signatures
					to sensible **kern ones
				*/

				if ( j == 0 && k == 0)
				{
					j=2; k=2;
				}

				if ( j==1 & k==1 )
				{
					j=4; k=4;
				}

				cur += sprintf(cur, "*M%d/%d\n", j, k);
				break;
			case 'C':
				/* convert clef */
				i+=2;
				tempnum = atoi( &buffer[i] );

				switch ( tempnum/10 )
				{
					case 0:
	 					/* G clef */
						strcpy(tempstr, "*clefG");
						break;
					case 1:
						/* C clef */
						strcpy(tempstr, "*clefC");
						break;
					case 2:
						/* F clef */
						strcpy(tempstr, "*clefF");
						break;
					case 3:
						/* Octave transposed G clef */
						strcpy(tempstr, "*clefGv");
						break;
					case 4:
						/* C clef transposed down */
						strcpy(tempstr, "*clefCv");
						break;
					case 5:
						/* F clef transposed down */
						strcpy(tempstr, "*clefFv");
						break;
					case 6:
						/* G clef transposed up */
						strcpy(tempstr, "*clefG^");
						break;
					case 7:
						/* C clef transposed up */
						strcpy(tempstr, "*clefC^");
						break;
					case 8:
						/* F clef transposed up */
						strcpy(tempstr, "*clefF^");
						break;
					default:
						strcpy(tempstr, "*clefX");
						break;
				}

				if (tempnum%10>0 || tempnum%10<6)
				{
					cur+= sprintf(cur, "%s%d\n", tempstr, 6-tempnum%10);
				}
				else
				{
					cur+= sprintf(cur, "%s\n", tempstr);
				}

				break;
			case 'X':
				/* convert transposing part */
				chromatic = 0;
				diatonic = 0;
				i+=2;
				tempnum = atoi( &(buffer[i]) );
				if (tempnum != 0)
				{
					sign = abs(tempnum)/tempnum;
					tempnum = abs(tempnum);
					diatonic = (tempnum-1) / 5;
					if ( diatonic < 8 )
					{
						chromatic=diatonic + tempnum%5;
						if (!(tempnum%5))
							chromatic += 5;
					}
					chromatic = chromatic * sign;
					diatonic = diatonic * sign;
				}
				/* printf("*Trd%dc%do%d\n", diatonic, chromatic, tempnum*sign); */
				break;
			case 'S':
				/* convert number of staves for part */
				break;
			case 'I':
				/* convert number of instruments represented */
				break;
			case 'D':
				/* convert directive */
				break;
			default:
				break;
		}
		i++;
	}
}

/*************************************************
	print_duration : prints the duration
	into the token specified in the command line
*************************************************/

void	print_duration ( char *durtok, int temp )
{
	int high;
	int low;
	int done;

	high = temp;

	done = FALSE;
	while (high > 0 && !done)
	{
		if ( qdiv % high == 0 || high == 1)
		{
			done = TRUE;
		}
		else if ( high % 3 == 0 && (qdiv % (high/3) == 0) )
		{
				done = TRUE;
		}
		else if ( high%7 == 0 && qdiv%(high/7) == 0 )
		{
				done = TRUE;
		}
		else
		{
			high --;
		}
	}

	if ( (qdiv % high) == 0 )
	{
		sprintf(durtok, "%d\0", qdiv / (high*2));
	}
	else if (high % 7 == 0)
	{
		high = high * 4 / 7;
		print_duration (durtok, high);
		strcat (durtok, "..");
	}
	else if ( high % 3 == 0)
	{
		high = high * 2 / 3;
		print_duration (durtok, high);
		strcat (durtok, ".");
	}
}

/*************************************************
	find_duration : calculates the duration
	from the line, and returns any left-over
	durations from durations whose values
	can not be easily represented with kern's
	recip notation system. This is then re-
	evaluated and tied to produce a note of
	the correct length.
*************************************************/

int	find_duration (char *durtok)
{
	int 	temp;
	int	dotted;
	int	duration;	
	char	dur[10];
	int	quart;
	int	i, j, k;
	int	rc;
	int 	done;
	int	high;
	int	low;

	rc = 0;

	strncpy(dur, &(buffer[5]), 3);
	dur[4] = '\0';
	sscanf (dur, " %d", &temp);

	high = temp;
	low = 0;

	done = FALSE;
	while ( high > 0 && !done )
	{
		if ( qdiv%high == 0 )
		{
			done = TRUE;
		}
		else if ( high%3 == 0 && ( qdiv%(high/3) == 0 ))
		{
				done = TRUE;
		}
		else if ( high%7 == 0 && (qdiv%(high/7) == 0 ))
		{
				done = TRUE;
		}
		else
		{
			high --;
			low ++;
		}
	}

	print_duration (durtok, high);

	if (low != 0)
	{
		rc = low;
	}

	return rc;
}

/*************************************************
	find_note : finds the note as a kern note
*************************************************/

void	find_note(char *notetok)
{
	int 	i, j, k;
	char	notechar;
	char	accidentals[20];
	char	octave[4];
	int	oct;

	/* First split the token up */

	i = 1;
	j = 0;
	k = 0;
	while ( buffer[i] != ' ')
	{
		switch (buffer[i])
		{
			case '#':
				accidentals[j] = '#';
				j++;
				break;
			case 'f':
				accidentals[j] = '-';
				j++;
				break;
			default:
				if (buffer[i] >= '0' && buffer[i] <= '9')
				{
					octave[k] = buffer[i];
					k++;
				}
				break;
		}
		i++;
	}

	octave[k] = '\0';
	accidentals[j] = '\0';

	oct = atoi(octave);

	if (oct > 3)
	{
		notechar = tolower(buffer[0]);
		oct = oct - 3;
	}
	else
	{
		notechar = buffer[0];
		oct = 4 - oct; 
	}

	for (i=0; i<oct; i++)
		notetok[i] = notechar;

	notetok[i] = '\0';
	strcat(notetok, accidentals);	
}

/*************************************************
	find_junk : assembles the various junky things
	in the columns into kern data
*************************************************/

void	find_junk (char *forejunktok, char *aftjunktok)
{
	int i,j,k;
	char	check[13];
	int	opened_tie;

	if (strlen(buffer) > 32)
	{
		strcpy(check, &(buffer[31]));
	}
	else
	{
		check[0] = '\0';
	}

	/* cur+=sprintf(cur, "Token stuff: %s\n", check); */

	forejunktok[0] = '\0';
	aftjunktok[0] = '\0';
	opened_tie = FALSE;

	if (strchr(check, '\(') || strchr(check, '[') ||
		 strchr(check, '{') || strchr(check, 'z'))
	{
		strcat(forejunktok, "\(");
	}

	/* We check if we have to precede the token with an open-
		or continue-tie marker
	*/

	if ( (buffer[8] == '-' || buffer[8] == '_') )
	{
			switch (tie_state)
			{
				case 0:
					strcat(forejunktok, "[");
					break;
				case 1:
					strcat(aftjunktok, "_");
					break;
				default:
					break;
			}
		opened_tie = TRUE;
	} 

	/* check for pause */

	if (strchr(check, 'F') || strchr(check, 'E'))
	{
		strcat(aftjunktok, ";");
	}

	/* ornaments */

	if (strchr(check, 'M'))
	{
		strcat(aftjunktok, "Mm");
	}

	if (strchr(check, 'r'))
	{
		strcat(aftjunktok, "S");
	}

	if (strchr(check, 't') || strchr(check, '~') ||
		 strchr(check, 'w') || strchr(check, 'c'))
	{
		strcat(aftjunktok, "Tt");
	}

	/* No inverted mordents in MuseData */

	/* Now figure out Appoggiature code */

	/* Now figure out Acciaccatura code */

	/* groupetto? */

	/* Articulation marks: */

	/* sforzando */

	if (strchr(check, 'Z'))
	{
		strcat(aftjunktok, "z");
	}

	/* staccato */

	if (strchr(check, '.'))
	{
		strcat (aftjunktok, "'");
	}

	/* pizzicato, the flag for which is turned on if
	   the keyword 'pizz' is found in a direction line and
	   turned off if 'arco' is found
	*/

	if (pizz)
	{
		strcat(aftjunktok, "\"");
	}

	/* attacca */

	/* Tenuto... what MuseData calls 'legato' */

	if (strchr(check, '_'))
	{
		strcat(aftjunktok, "~");
	}

	/* accent */

	if (strchr(check, 'A') || strchr(check, 'V'))
	{
		strcat(aftjunktok, "^");
	}

	/* arpeggiation would go here */

	if (strchr(check, 'S'))
	{
		strcat(aftjunktok, ":");
	}

	/* bowing marks */

	if (strchr(check, 'v'))
	{
		strcat(aftjunktok, "v");
	}

	if (strchr(check, 'n'))
	{
		strcat(aftjunktok, "u");
	}

	/* down-stem */

	if (buffer[22] == 'd')
	{
		strcat(aftjunktok, "\\");
	}

	/* up-stem */

	if (buffer[22] == 'u')
	{
		strcat(aftjunktok, "/");
	}

	/* Beams */

	j = 1;

	if (strlen(buffer) > 25)
	{
		for (i=25; i<31; i++)
		{
			if (buffer[i] == '[')
			{
				for (; j>0; j--)
				{
					strcat(aftjunktok, "L");
				}
			}
			if (buffer[i] == ']')
			{
				for (; j>0; j--)
				{
					strcat(aftjunktok, "J");
				}
			}
			if (buffer[i] == '\\')
			{
				for (; j>0; j--)
				{
					strcat(aftjunktok, "k");
				}
			}
			if (buffer[i] == '/')
			{
				for (; j>0; j--)
				{
					strcat(aftjunktok, "K");
				}
			}
			j++;
		}
	}

	/* Check if we have to close a tie after the token */

	if (!opened_tie && tie_state == 1)
	{
		strcat(aftjunktok, "]");
	}

	/* Close slur */

	if (strchr(check, ')') || strchr(check, ']') ||
		 strchr(check, '}') || strchr(check, 'x'))
	{
		strcat(aftjunktok, ")");
	}

	/* breath marks */

	if (strchr(check, ','))
	{
		strcat(aftjunktok, ",");
	}
}

/*************************************************
	print_section : prints out the correct
	section and fills in information in the
	stack for the new section
*************************************************/

void	print_section (int secttype)
{
	switch (secttype)
	{
		case LEFT:
			cur += sprintf(cur, "*>%c\n", sect);
			stack[stackptr++] = sect;
			sect++;
			break;
		case RIGHT:
			cur += sprintf(cur, "*>%c\n", sect);
			stack[stackptr++] = '(';
			stack[stackptr++] = sect;
			sect++;
			break;
		case LEFTRIGHT:
			cur += sprintf(cur, "*>%c\n", sect);
			stack [stackptr++] = ')';
			stack [stackptr++] = '(';
			stack [stackptr++] = sect;
			sect++;
			break;
		default:
			break;
	}
	newsect = FALSE;
}

/*************************************************
	check_repeats: checks the repeat status of
	barlines and generates labels for the
	'thru' command
*************************************************/

void	check_repeats (void)
{
	int i;

	if ( strstr(buffer, ":|:") )
	{
		newsect = LEFTRIGHT;
	}
	else if ( strstr(buffer, "|:") )
	{
		newsect = RIGHT;
	}
	else if (strstr (buffer, ":|"))
	{
		newsect = LEFT;
		stack[stackptr++] = ')';
	}
}

/*************************************************
	process: given a line, this function processes
	it into its various parts and figures out
	what to do with it
*************************************************/

void	process (void)
{
	int	num_matched;
	int	i, j, k;
	int	rc;
	char	mtype[20];
	char	duration[20];
	char	note[20];
	char	forejunk[20];
	char	aftjunk[20];
	char	token[80];
	char	scanline[960];

	switch (buffer[0])
	{
		case '$':
			/* Controlling music attributes */
			process_attributes();
			break;
		case '&':
			/* Comment mode has been turned on */
			getnext(infile);
			while (buffer[0] != '&')
			{
				cur += sprintf(cur, "!! %s", buffer);
				getnext(infile);
			}
			break;
		case '*':
			/* musical direction */
			if (strstr(buffer, "pizz"))
			{
				pizz = TRUE;
			}
			else if (strstr(buffer, "arco"))
			{
				pizz = FALSE;
			}
			else if (comment_unknowns)
			{
				cur+=sprintf(cur, "!!%s", buffer);
			}
			break;
		case ' ':
			for (j=0; j<5; j++)
				buffer[j] = buffer[j+1];

			double_stop = TRUE;

			/* There is no break here */

		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
			/* note value */
			if (newsect)
				print_section(newsect);
			strncpy ( duration, buffer+5, 3 );
			duration[4] = '\0';
			sscanf(duration, " %d", &i);

			rc = find_duration ( duration );
			find_note ( note );

			if (diatonic || chromatic)
				transpose ( note );

			find_junk ( forejunk, aftjunk );

			if (rc)
			{
				strcpy(scanline, buffer);
				/* open a tie */

				if (back > 0)
					cur += sprintf(cur, "!");

				cur += sprintf(cur, "[%s%s%s%s\n", forejunk, duration, note, aftjunk);

				/* rewrite record */

				sprintf(buffer+5, "%3d", rc);

				/* Loop through until we finish the tie */

				while ( (rc = find_duration(duration)) != 0 )
				{
					if (back > 0)
						cur += sprintf(cur, "!");

					cur += sprintf(cur, "_%s%s%s%s\n", forejunk, duration, note, aftjunk);
					sprintf(buffer+5, "%3d", rc);
				}

				if (back > 0)
					cur+=sprintf(cur, "!");

				cur += sprintf (cur, "]%s%s%s%s", forejunk, duration, note, aftjunk );
				strcpy(buffer, scanline);
			}
			else
			{
				if (back > 0)
					cur+=sprintf(cur, "!");

				cur += sprintf(cur, "%s%s%s%s", forejunk, duration, note, aftjunk);
			}

			while ( (j=getc(infile)) == 'P' || j=='S')
			{
				ungetc(j, infile);
				fgets(buffer, 960, infile);
			}

			if ( j != ' ')
			{
				/* No more double stops so we can do some
					end-of-line processing
				*/

				cur += sprintf(cur, "\n");

				if (strchr(forejunk, '['))
				{
					tie_state = 1;
				}

				if (strchr(aftjunk, ']'))
				{
					tie_state = 0;
				}

				if (back > 0)
				{
					if (back-i <= 0)
					{
						cur += sprintf(cur, "*endback\n");
					}
					back = back - i;
				}
			}
			else if (back <= 0)
			{
				cur += sprintf(cur, " ");
			}
			ungetc(j, infile);

			memset (duration, 0, 20);
			memset (note, 0, 20);
			memset (forejunk, 0, 20);
			memset (aftjunk, 0, 20);
			break;
		case '/':
			/* End of music or end of file */
			cur += sprintf(cur, "*-\n");
			break;
		case '@':
			/* Single line comment */
			cur += sprintf(cur, "!! %s", &(buffer[1]));
			break;
		case 'a':
			/* Append to previous line */
			if (comment_unknowns)
				cur+=sprintf(cur, "!!%s", buffer);
			break;
		case 'b':
			/* Backspace */
			cur += sprintf(cur, "*back");
			strcpy(scanline, buffer);

			do
			{
				rc = find_duration (duration);
				cur += sprintf(cur, "-%s", duration);

				sprintf(buffer+5, "%3d", rc);
			} while ( rc );

			cur += sprintf(cur, "\n");
			strcpy(buffer, scanline);

			strncpy(duration, buffer+5, 3);
			duration[4] = '\0';
			sscanf(duration, " %d", &back);
			break;
		case 'c':
			/* Cue note size */
			if (newsect)
				print_section(newsect);
			if (comment_unknowns)
				cur+=sprintf(cur, "!!%s", buffer);
			break;
		case 'f':
			/* Figured harmony */
			if (newsect)
				print_section(newsect);
			if (comment_unknowns)
				cur+=sprintf(cur, "!!%s", buffer);
			break;
		case 'g':
			/* grace notes */
			if (newsect)
				print_section(newsect);
			if (comment_unknowns)
				cur+=sprintf(cur, "!!%s", buffer);
			break;
		case 'm':
			/* barlines */
			if (newsect)
				print_section(newsect);
			num_matched = sscanf (buffer, "%s %d", mtype, &i);

			if (!strcmp(mtype, "measure"))
			{
				cur += sprintf(cur, "=");
				if (num_matched == 2)
				{
					cur += sprintf(cur,"%d\n", i);
				}
				else
				{
					cur += sprintf(cur,"\n");
				}
			}
			else if (!strncmp(mtype, "mheavy", 6))
			{
				cur += sprintf(cur, "==\n");
			}
			else if (!strncmp(mtype, "mdouble", 7))
			{
				cur += sprintf(cur, "=\n");
			}
			else if (comment_unknowns)
			{
				cur+=sprintf(cur, "!!%s", buffer);
			}
			break;
		case 'r':
			/* Regular rest */
		case 'i':
			/* invisible rest */
			if (newsect)
				print_section(newsect);
			strncpy ( duration, buffer+5, 3 );
			duration[4] = '\0';
			sscanf(duration, " %d", &i);

			rc = find_duration ( duration );

			if (rc)
			{
				strcpy(scanline, buffer);
				/* open a tie */

				if (back > 0)
					cur += sprintf(cur, "!");

				cur += sprintf(cur, "%sr\n", duration);

				/* rewrite record */

				sprintf(buffer+5, "%3d", rc);

				/* Loop through until we have broken up rests */

				while ( (rc = find_duration(duration)) != 0 )
				{
					if (back > 0)
						cur += sprintf(cur, "!");

					cur += sprintf(cur, "%sr\n", duration);
					sprintf(buffer+5, "%3d", rc);
				}

				if (back > 0)
					cur+=sprintf(cur, "!");

				cur += sprintf (cur, "%sr", duration);
				strcpy(buffer, scanline);
			}
			else
			{
				if (back > 0)
					cur+=sprintf(cur, "!");

				cur += sprintf(cur, "%sr", duration);
			}
			cur+=sprintf(cur, "\n");

			if (back > 0)
			{
				if (back-i <= 0)
				{
					cur += sprintf(cur, "*endback\n");
				}
				back = back - i;
			}
			memset (duration, 0, 20);
			break;
		case 's':
			/* Sound directions */
			if (comment_unknowns)
			{
				cur+=sprintf(cur, "!!%s", buffer );
			}
			break;
		case 'P':
			/* Print suggestions */
			if (comment_unknowns)
			{
				cur+=sprintf(cur, "!!%s", buffer );
			}
			break;
		default:
			break;
	}
	if (sect == 'A')
	{
		/* cur+=sprintf(cur, "*>A\n"); */
		/* stack[stackptr++] = sect++; */
		print_section (RIGHT);
	}
	check_repeats();
}

/*************************************************
	printform : prints the form of the piece
*************************************************/

void	printform (int pos, FILE *outf)
{
	int i,j;
	int printed;

	/* scan backwards and match brackets */

	i = 1;
	j = pos;

	while (i>0 && j>0)
	{
		j--;
		if (stack[j] == ')')
			i++;
		if (stack[j] == '(')
			i--;
	}

	/* scan forwards and echo output */

	printed = FALSE;

	while (j<pos)
	{
		if ( isalpha(stack[j]) )
		{
			if (printed)
			{
				fputc(',', outf);
			}
			fputc(stack[j], outf);
			printed = TRUE;
		}
		else if ( stack[j] == ')' )
		{
			if (printed)
			{
				fputc(',', outf);
			}
			printform (j, outf);
		}

		j++;
	}
}

/*************************************************
	convert: takes a file pointer and converts
	the contents of that file into kern, writing
	the output to standard output.
*************************************************/

void	convert ( FILE *infile )
{
	int i,j,k;
	char token[80];
	char temp[80];
	int	skip;

	memset(stack, 0, 100);
	stackptr = 0;
	sect = 'A';
	newsect = FALSE;

	header (infile);

	skip = FALSE;

	do
	{
		/* printf("%s", buffer); */

		if ( !strncmp(buffer, "/FINE", 5) )
		{
			skip = TRUE;
		}

		if (!skip)
		{
			process ();
		}

		if (comments)
		{
			cur += sprintf(cur, "!!%s", buffer);
		}
	} while (getnext(infile));
	/* send the pointer to the beginning by adding another bracket */
	stack[stackptr++] = ';';

	stack[stackptr] = '\0';
	/* cur+=sprintf(cur, "%s\n", stack); */
	*cur++ = '\0';
}

/*************************************************
	scandirect: given a string, which is a directory
	pathname, this function lists the contents
	of that directory
*************************************************/

void	scandirect ( char *dirname )
{
	DIR *dir;
	struct dirent *ent;
	int i;
	int	chrs;

	if ( (dir=opendir(dirname)) == NULL )
		return;

	i = 0;
	chrs = 0;

	while ( (ent = readdir (dir)) != NULL )
	{
		printf("%s", ent->d_name);
		chrs += strlen (ent->d_name);

		if ( i%5 == 0 )
		{
			printf("\n");
			chrs = 0;
		}
		else
		{
			while ( chrs%14 != 0)
			{
				printf(" ");
				chrs++;
			}
		}

		i++;
	}

	if (closedir(dir) != 0)
	{
		perror("mkern: Error closing directory.\n");
		exit (1);
	}
}

/*************************************************
	getinputdata : prompts the user for input
	about conversion. At present we only have
	information about source and destination
	locations
*************************************************/

void	getinputdata (void)
{
	char arg[1024];
	int	done;

	printf("MuseData-Kern Conversion Utility\n");
	printf("--------------------------------\n");

	done = FALSE;

	while (!done)
	{
		printf("\nEnter directory of source (MuseData) files:\n");
		fgets(arg, 1024, stdin);
		if (strlen(arg) > 0 )
		{
			sscanf(arg, "%s", indirname);
			if (indirname[strlen(indirname)-1] == '/')
			{
				scandirect (indirname);
			}
			else
			{
				done = TRUE;
			}
		}
		/* printf("Source directory set to %s\n", indirname); */
		arg[0] = '\0';
	}

	printf("\nEnter directory for target (Kern) files:\n");
	fgets(arg, 1024, stdin);
	if (strlen(arg) > 0 )
	{
		sscanf(arg, "%s", outdirname);
	}
	/* printf("Target directory set to %s\n", outdirname); */
	arg[0] = '\0';

	printf("\nEnter filename for conversion script file that will be created\nby this program:\n");
	fgets(arg, 1024, stdin);
	sscanf(arg, "%s", scriptfilename);
	/* printf("Script filename set to %s\n", scriptfilename); */
	if (strlen(scriptfilename) > 0)
		generate_script = TRUE;
	arg[0] = '\0';

	printf("\nEnter filename for final assembled (kern) datafile:\n");
	fgets(arg, 1024, stdin);
	sscanf(arg, "%s", targetfilename);
	/* printf("target filename set to %s\n", targetfilename); */
	arg[0] = '\0';
}

/*************************************************
	read_rc : reads the rc file for information
	about conversion. At present we only have
	information about source and destination
	locations
*************************************************/

void	read_rc (void)
{
	char command[16];
	char arg[1024];

	rcfile = fopen(rcfilename, "r");

	if (rcfile)
	{
		/* read it */

		while (getnext(rcfile))
		{
			if (buffer[0] != '#')
			{
				/* printf("processing line %s", buffer); */

				command[0] = '\0';
				arg[0] = '\0';

				sscanf(buffer, "%s %s", command, arg);

				if ( !strncmp(command, "DEST", 4) || !strncmp(command, "dest", 4))
				{
					strcpy(outdirname, arg);
					arg[0] = '\0';
				}
				else if ( !strncmp(command, "SOUR", 4) || !strncmp(command, "sour", 4))
				{
					strcpy(indirname, arg);
					arg[0] = '\0';
				}
				else if ( !strncmp(command, "SCRI", 4) || !strncmp(command, "scri", 4))
				{
					strcpy(scriptfilename, arg);
					generate_script = TRUE;
					arg[0] = '\0';
				}
				else if ( !strncmp(command, "TARG", 4) || !strncmp(command, "targ", 4))
				{
					strcpy(targetfilename, arg);
					arg[0] = '\0';
				}
			}
		}
		/* printf("%s\n%s\n%s\n%s\n", indirname, outdirname, scriptfilename, */
					/* targetfilename); */

		fclose(rcfile);
	}
	else
	{
		crapout("No such rcfile.\n");
	}
}

/*************************************************
	do_file : processes the next file in the
	directory
*************************************************/

int	do_file ( int filenum )
{
	int success;

#ifdef __MSDOS__
	sprintf(infilename, "%s\\%02d\0", indirname, filenum);
	sprintf(outfilename, "%s\\%02d.krn\0", outdirname, filenum);
#else
	sprintf(infilename, "%s/%02d\0", indirname, filenum);
	sprintf(outfilename, "%s/%02d.krn\0", outdirname, filenum);
#endif

	/* printf("in do_file with files %s and %s\n", infilename, outfilename); */

	infile = fopen(infilename, "r");

	if (infile)
		outfile = fopen(outfilename, "w");

	if (infile && outfile)
		success = TRUE;
	else
		success = FALSE;

	if ( success )
	{
		if (verbose)
		{
			printf("Converting file %02d to file %02d.krn\n", filenum, filenum);
		}
		convert (infile);
		dumppool (outfile);

		fclose(infile);
		fclose(outfile);
	}
	else
	{
		/* printf("File not found... %s\n", infilename); */
	}
	return !success;
}

/*************************************************
	generate: generates a shell script
	to timebase and assemble the files that
	have just been converted
*************************************************/

void	generate (int k)
{
	int i;
	char	script[1104];
	FILE	*scriptf;

	if (k<2)
		return;

#ifdef __MSDOS__
	sprintf(script, "%s\\%s\0", outdirname, scriptfilename);
#else
	sprintf(script, "%s/%s\0", outdirname, scriptfilename);
#endif

	scriptf = fopen(script, "w");

	if (scriptf)
	{
		fprintf(scriptf, "##############################################\n");
		fprintf(scriptf, "#  Scriptfile generated by the mkern program. \n");
		fprintf(scriptf, "#  Files converted from directory\n");
		fprintf(scriptf, "#  %s\n", indirname);
		fprintf(scriptf, "##############################################\n");
		fprintf(scriptf, "\n");

		fprintf(scriptf, "echo Converting files originally found in directory\n");
		fprintf(scriptf, "echo     %s\n\n", indirname);

		for (i=1; i<k-1; i++)
		{
			fprintf(scriptf, "echo Timebasing file %02d.krn\n", i);
			fprintf(scriptf, "timebase -t %d %02d.krn > %02d.tb\n", 
					  timebase, i, i);
		}

		fprintf (scriptf, "\necho Assembling files and removing null tokens...\n\n");

		fprintf (scriptf, "assemble ");

		for (i=k-2; i>0; i--)
		{
			fprintf(scriptf, "%02d.tb ", i);
		}

		if (targetfilename[0] != '\0')
		{
			fprintf(scriptf, "| rid -d > %s\n", targetfilename);
		}
		else
		{
			fprintf(scriptf, "| rid -d > output.krn\n");
		}

		fclose (scriptf);

		if (verbose)
		{
			printf("Conversion script %s created in destination directory\n",
					  scriptfilename);
		}
	}
}

/*************************************************
   program mainline: processes command line
	arguments and then calls whatever processing
	function is necessary
*************************************************/

int	main ( int argc, char *argv[] )
{
	int i,j,k;
	int done;

	pizz = FALSE;
	infile = stdin;
	comments = FALSE;
	readrc = FALSE;
	interactive = FALSE;
	comment_unknowns = FALSE;
	done = FALSE;
	timebase = 1;

	/* Set up filenames */
	targetfilename[0] = '\0';
	outfilename[0] = '\0';
	infilename[0] = '\0';
	rcfilename[0] = '\0';
	strcpy (indirname, ".");
	strcpy (outdirname, ".");

	if (argc > 1)
	{
		for (j=1; j<argc; j++)
		{
			if (argv[j][0] == '-')
			{
				i = 1;

				while (argv[j][i] != '\0')
				{
					switch (argv[j][i])
					{
						case 'h':
							help ();
							exit (0);
							break;
						case 's':
							if (argc < 3)
								crapout("No filename specified.\n");
						
							strcpy(infilename, argv[j+1]);
							break;
						case 'c':
							comments = TRUE;
							break;
						case 'r':
							readrc = TRUE;

							if (argc < 3)
								crapout("No rcfile specified.\n");

							strcpy(rcfilename, argv[j+1]);
							break;
						case 'i':
							interactive = TRUE;
							readrc = TRUE;
							break;
						case 'v':
							verbose = TRUE;
							break;
						case 'C':
							comment_unknowns = TRUE;
							break;
						default:
							break;
					}
					i++;
				}
			}
			else
			{
				strcpy(infilename, argv[j]);
			}
		}

		if (!readrc)
		{
			infile = fopen(infilename, "r");

			if (!infile)
				crapout("No such file\n");
		}
	}

#ifdef __MSDOS__
	memory = farcoreleft() - 40960;
	pool = (unsigned char huge *) farmalloc ( memory );
#else
	memory = 307200;
	pool = (unsigned char *) malloc (memory);
#endif


	if (pool)
	{
		if (!readrc)
		{
			cur = pool;
			convert (infile);
			dumppool (stdout);
		}
		else
		{
			if (interactive)
				getinputdata();
			else
				read_rc();

			k = 1;
			while (!done)
			{
				cur = pool;
				memset(cur, 0, memory);
				done = do_file(k);
				k++;
			}
			if (generate_script)
			{
				generate(k);
			}
		}
		free(pool);
	}

	fclose (infile);
}
