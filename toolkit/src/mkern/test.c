#include <stdio.h>
#include <string.h>

int main (int argc, char **argv)
{
	char string[100];
	char *cur;
	int	len;

	printf("I am now printing into the string...");

	cur = string;
	len = sprintf(cur, "%s\n", argv[1]);

	cur += len;

	len += sprintf(cur, "%s\n", argv[1]);

	printf ("Printed %d bytes for %s", len, string);
}
