/**************************************************
	"lc"
**************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/*******
  comp
*******/

int comp (const void *a, const void *b)
{
	return (strcmp ((char *)a, (char *)b));
}

/*************************************************
	scandirect: given a string, which is a directory
	pathname, this function lists the contents
	of that directory
*************************************************/

void	scandirect ( char *dirname )
{
	DIR *dir;
	char	**names;
	long	numfiles;
	int	longest;
	struct dirent *ent;
	int i;
	int	chrs;
	struct stat buf;
	char pathname[1024];

	if ( (dir=opendir(dirname)) == NULL )
		return;

	i = 0;
	chrs = 0;
	longest = 0;
	numfiles = 0;

	while ( (ent = readdir (dir)) != NULL )
	{
		numfiles++;
	}

	names = (char **) malloc ( numfiles * sizeof (char *) );

	rewinddir(dir);

	while ( (ent = readdir (dir)) != NULL )
	{
		names[i] = (char *) malloc ( strlen(ent->d_name) );

		strcpy( names[i], ent->d_name );

		printf("%s", names[i]);
		chrs += strlen (names[i]);

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
	printf ("\n");

	if (closedir(dir) != 0)
	{
		perror("mkern: Error closing directory.\n");
		exit (1);
	}
}

/* main */

void	main (int argc, char *argv[])
{
	if (argc<2)
		scandirect(".");
	else
		scandirect(argv[1]);
}
