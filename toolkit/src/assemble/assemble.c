/*************************************
  Assemble : amalgamates humdrum
  files
**************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "assemble.h"

/***************************************
  The following are global variables.
  They are common to every function
  and can be accessed from any of the
  program elements in this file.
***************************************/

static	int	numfiles;						/* The current number of spines */
static	char	buffer[960];						/* Our input buffer */
static	char	bufline[960];
static 	char	tokens[NUM_FILES][960];
static	char	comments[20000];
static	FILE	*infiles[NUM_FILES];			/* input files */
static	int	closed[NUM_FILES];
static	FILE	*outfile;

/***************************************
  getnextline: reads in next non-comment
  line
***************************************/

char	*getnextline ( int file, char *text )
{
	char *rc;

	if (closed[file])
		return NULL;

	do
	{
		rc = fgets (text, 960, infiles[file]);
		if (rc && text[0] == '!' && text[1] == '!' && !strstr(comments, text))
		{
				strcat ( comments, text );
		}
	} while (text[0] == '!' && text[1] == '!' && rc != NULL );

	if (!rc)
	{
		fclose(infiles[file]);
		closed[file] = TRUE;
	}

	return rc;
}

/***************************************
  getnext
***************************************/

int	getnext ( void )
{
	char	*rc;
	int	i,j;
	int	done=TRUE;
	char	*ptr;
	int	interps;
	int 	printed;

	printed = FALSE;

	for (i=0; i<numfiles; i++)
	{
		rc = getnextline( i, bufline );

		if (rc)
		{
			/* We have a non-comment line in "bufline" */

			strcpy ( tokens[i], bufline );

			/* Strip newlines */

			ptr = strchr(tokens[i], '\n');

			if (ptr)
				*ptr = '\0';
		}
	}

	printf("%s", comments);
	comments[0] = '\0';

	memset(buffer, 0, 960);

	/* Now we have to check to be sure there are no
	   tandem interpretations or local comments
		in any of the spines and if there are, 
		we have to space them out
	*/

	interps = TRUE;

	while ( interps )
	{
		for (i=0; i<numfiles; i++)
		{
			if (tokens[i][0] == '*' && !closed[i])
			{
				/* Found interpretations */

				if (tokens[i][0] == '*' && tokens[i][1] == '*')
				{
					/* Found exclusive interpretation */

					for (j=0; j<numfiles; j++)
					{
						if (!closed[j])
						{
							if (printed)
								printf("\t");

							if (j!=i)
							{
								if ( tokens[j][0] == '*' && tokens[j][1] == '*' )
								{
									printf("%s", tokens[j]);
									printed = TRUE;

									rc = getnextline( j, tokens[j] );
									ptr = strchr(tokens[j], '\n');
									if (ptr) 
										*ptr = '\0';

									if (!rc)
									{
										tokens[j][0] = '\0';
									}
								}
								else
								{
									ptr = tokens[j];
									while (*ptr != '\0')
									{
										if (*ptr == '\t')
										{
											printf("*\t");
										}
										ptr++;
									}
									printf("*");
									printed = TRUE;
								}
							}
							else
							{
								printf("%s", tokens[j]);
								printed = TRUE;
								rc = getnextline ( j, tokens[j] );
								if (!rc)
								{
									tokens[j][0] = '\0';
								}
								else
								{
									ptr = strchr(tokens[j], '\n');
									if (ptr) 
										*ptr = '\0';
									i--;
								}
							}
						}
					} /* for */
					printf("\n");
					printed = FALSE;
				}
				else if ( strstr (tokens[i], "*v") )
				{
					/* Join spines */
					for (j=0; j<numfiles; j++)
					{
						if (!closed[j])
						{
							if (printed)
								printf("\t");

							if (j!=i)
							{
								if (strchr(tokens[j], '*'))
								{
									if ( strstr( tokens[j], "*v" ) ||
										 (!strstr( tokens[j], "*-" ) &&
										  !strstr( tokens[j], "*x" ) &&
										  !strstr( tokens[j], "*+" ) &&
										  !strstr( tokens[j], "*^" )))
									{
										ptr = tokens[j];
										while (*ptr != '\0')
										{
											if (*ptr == '\t')
											{
												printf("*\t");
											}
											ptr++;
										}
										printf("*");
										printed = TRUE;
									}
									else
									{
										printf("%s",tokens[j]);
										printed = TRUE;
										rc = getnextline ( j, tokens[j] );
										ptr = strchr( tokens[j], '\n');

										if (ptr)
											*ptr = '\0';

										if (!rc)
										{
											tokens[j][0] = '\0';
										}
									}
								}
								else
								{
									ptr = tokens[j];
									while (*ptr != '\0')
									{
										if (*ptr == '\t')
										{
											printf("*\t");
										}
										ptr++;
									}
									printf ("*");
									printed = TRUE;
								}
							}
							else
							{
								printf("%s", tokens[i]);
								printed = TRUE;
								rc = getnextline ( i, tokens[i] );

								if (!rc)
								{
									tokens[i][0] = '\0';
								}
								else
								{
									ptr = strchr( tokens[i], '\n');
									if (ptr)
										*ptr = '\0';
									i--;
								}
							}
						} 
					} /* for */
					printf("\n");
					printed = FALSE;
				}
				else if ( strstr (tokens[i], "*x") )
				{
					/* Exchange spines */
					for (j=0; j<numfiles; j++)
					{
						if (!closed[j])
						{
							if (printed)
								printf("\t");

							if (j!=i)
							{
								if (strchr(tokens[j], '*'))
								{
									if ( strstr( tokens[j], "*x" ) ||
										  (!strstr( tokens[j], "*-" ) &&
										  !strstr( tokens[j], "*v" ) &&
										  !strstr( tokens[j], "*+" ) &&
										  !strstr( tokens[j], "*^" )))
									{
										ptr = tokens[j];
										while (*ptr != '\0')
										{
											if (*ptr == '\t')
											{
												printf("*\t");
											}
											ptr++;
										}
										printf("*");
										printed = TRUE;
									}
									else
									{
										printf("%s",tokens[j]);
										printed = TRUE;
										rc = getnextline ( j, tokens[j] );

										if (!rc)
										{
											tokens[j][0] = '\0';
										}
										ptr = strchr( tokens[j], '\n');
										if (ptr)
											*ptr = '\0';
									}
								}
								else
								{
									ptr = tokens[j];
									while (*ptr != '\0')
									{
										if (*ptr == '\t')
										{
											printf("*\t");
										}
										ptr++;
									}
									printf("*");
									printed = TRUE;
								}
							}
							else
							{
								printf("%s", tokens[j]);
								printed = TRUE;
								rc = getnextline ( j, tokens[j] );

								if (!rc)
								{
									tokens[j][0] = '\0';
								}
								else
								{
									ptr = strchr( tokens[j], '\n');
									if (ptr)
										*ptr = '\0';
									i--;
								}
							}
						}
					} /* for */
					printf("\n");
					printed = FALSE;
				}
				else if ( strstr(tokens[i],"*^") ||
							 strstr(tokens[i],"*+") ||
							 strstr(tokens[i],"*-"))
				{
					/* other spine path indicator */
					for (j=0; j<numfiles; j++)
					{
						if (!closed[j])
						{
							if (printed)
								printf("\t");

							if (j!=i)
							{
								if (strchr(tokens[j], '*'))
								{
									if ( !strstr( tokens[j], "*-" ) &&
										  !strstr( tokens[j], "*+" ) &&
										  !strstr( tokens[j], "*^" ))
									{
										ptr = tokens[j];
										while (*ptr != '\0')
										{
											if (*ptr == '\t')
											{
												printf("*\t");
											}
											ptr++;
										}
										printf("*");
										printed = TRUE;
									}
									else
									{
										printf("%s",tokens[j]);
										printed = TRUE;
										rc = getnextline ( j, tokens[j] );

										if (!rc)
										{
											tokens[j][0] = '\0';
										}
										ptr = strchr( tokens[j], '\n');
										if (ptr)
											*ptr = '\0';
									}
								}
								else
								{
									ptr = tokens[j];
									while (*ptr != '\0')
									{
										if (*ptr == '\t')
										{
											printf("*\t");
										}
										ptr++;
									}
									printf("*");
									printed = TRUE;
								}
							}
							else
							{
								printf("%s", tokens[j]);
								printed = TRUE;
								rc = getnextline ( j, tokens[j] );

								if (!rc)
								{
									tokens[j][0] = '\0';
								}
								else
								{
									ptr = strchr( tokens[j], '\n');
									if (ptr)
										*ptr = '\0';
									i--;
								}
							}
						}
					} /* for */
					printf("\n");
					printed = FALSE;
				}
				else   /* Interpretation is only tandem */
				{
					for (j=0; j<numfiles; j++)
					{
						if (!closed[j])
						{
							if (printed)
								printf("\t");

							if (j!=i)
							{
								if (tokens[j][0] == '*' && 
									 !strstr(tokens[j],"**") &&
									 !strstr(tokens[j],"*-") &&
									 !strstr(tokens[j],"*+") &&
									 !strstr(tokens[j],"*v") &&
									 !strstr(tokens[j],"*^") &&
									 !strstr(tokens[j],"*x"))
								{
									printf("%s", tokens[j]);
									printed = TRUE;
									rc = getnextline( j, tokens[j] );
									ptr = strchr(tokens[j], '\n');

									if (ptr)
										*ptr = '\0';

									if (!rc)
									{
										tokens[j][0] = '\0';
									}
								}
								else
								{
									ptr = tokens[j];
									while (*ptr != '\0')
									{
										if (*ptr == '\t')
										{
											printf("*\t");
										}
										ptr++;
									}
									printf("*");
									printed = TRUE;
								}
							}
							else
							{
								printf("%s", tokens[i]);
								printed = TRUE;
								rc = getnextline ( i, tokens[i] );

								if (!rc)
								{
									tokens[i][0] = '\0';
								}
								else
								{
									ptr = strchr( tokens[i], '\n');
									if (ptr)
										*ptr = '\0';
									i--;
								}
							}
						}
					} /* for */
					printf("\n");
					printed = FALSE;
				}  /* Line contains tandem interpretation */
			}  /* does line contain an interpretation? */
			else if (tokens[i][0] == '!' && !closed[i])
			{
				/* Local comment has been found */

				for (j=0; j<numfiles; j++)
				{
					if (!closed[j])
					{
						if (printed)
							printf("\t");

						if (j!=i)
						{
							if (tokens[j][0] == '!')
							{
								printf("%s", tokens[j]);
								printed = TRUE;
								rc = getnextline( j, tokens[j] );
								ptr = strchr(tokens[j], '\n');

								if (ptr)
									*ptr = '\0';

								if (!rc)
								{
									tokens[j][0] = '\0';
								}
							}
							else
							{
								ptr = tokens[j];
								while (*ptr != '\0')
								{
									if (*ptr == '\t')
									{
										printf("!\t");
									}
									ptr++;
								}
								printf("!");
								printed = TRUE;
							}
						}
						else
						{
							printf("%s", tokens[i]);
							printed = TRUE;
							rc = getnextline ( i, tokens[i] );

							if (!rc)
							{
								tokens[i][0] = '\0';
							}
							else
							{
								ptr = strchr( tokens[i], '\n');
								if (ptr)
									*ptr = '\0';
								i--;
							}
						}
					}
				} /* for */
				printf("\n");
				printed = FALSE;
			}  /* Line contains local comment */
		}

		interps = FALSE;
		for (i=0;i<numfiles;i++)
		{
			if (tokens[i][0]=='*')
				interps = TRUE;
		}
	}


	printed = FALSE;
	for (i=0; i<numfiles; i++)
	{
		if (printed && !closed[i])
			strcat (buffer, "\t");

		if (!closed[i])
		{
			strcat (buffer, tokens[i]);
			printed = TRUE;
		}
	}
	strcat(buffer, "\n");

	printf("%s", comments);
	comments[0] = '\0';

	done = TRUE;
	for (i=0; i<numfiles && done; i++)
		if (!closed[i])
			done = FALSE;

	return !done;
}

/**************************************
  usage : prints a usage message
**************************************/

void	usage (void)
{
	fprintf(stderr,"usage: assemble [inputfiles...]\n");
}

/**************************************
  help : prints out the help screen
**************************************/

void	help ( void )
{
	puts("");
	puts("ASSEMBLE  : Amalgamate two or more Humdrum files.");
	puts("");
	puts("     This command allows two or more structurally similar Humdrum files to");
	puts("     be aligned together -- such as where a full score is assembled from");
	puts("     files containing individual parts.");
	puts("");
	puts("     The ASSEMBLE command vertically combines two or more files in a manner");
	puts("     similar to the UNIX \"paste\" command.");
	puts("");
	puts("Syntax:");
	puts("");
	puts("     assemble file1 [file2] [file3 ...]");
	puts("");
	puts("Options:");
	puts("");
	puts("     None.");
	puts("");
	puts("See Also:");
	puts("");
	puts("     TIMEBASE");
	puts("");
	puts("  Refer to reference manual for further details.");
	puts("");
}

/*************************************
  read_data : the main loop for the
  conversion utility
**************************************/

void	read_data ( void )
{
	while (getnext())
	{
		printf("%s", buffer);
	}
}

/***************************************
  Program mainline.
***************************************/

int	main 	(int argc, char *argv[])
{
	int	i;
	int 	filenames;

	outfile = stdout;

	/********************************************
	  Command line argument processing.
	  Currently available options:
	  -h : help
	********************************************/

	if (argc < 2)
	{
		usage();
		exit(1);
	}
	else
	{
		filenames = FALSE;
		for (i=1; i< argc; i++)
		{
			if (argv[i][0] == '-' && !filenames)
			{
				switch (argv[i][1])
				{
					case 'h':
						help();
						exit(1);
						break;
					case '-':
						closed[i-1] = TRUE;
						filenames = TRUE;
						break;
					default:
						fprintf(stderr,"assemble: unknown option: -%c\n", argv[i][1]);
						exit(1);
						break;
				}
			}
			else 
			{
				infiles[i-1] = fopen(argv[i], "r");
				if (infiles[i-1] == NULL)
				{
					fprintf(stderr, "assemble: file not found: %s\n", argv[i]);
					exit(1);
				}
				else
				{
					closed[i-1] = FALSE;
				}
			}
		}
	}
	numfiles = argc-1;
	read_data ();

	for (i=0; i<numfiles; i++)
	{
		if (!closed[i])
			fclose(infiles[i]);
	}
}
