###################################################################
#                          MSJOIN
#
# Programmed by: David Huron        Date: January, 1997
# Copyright (c) 1997 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
#
#
# This script joins two spines together (a **kern spine and
# some other spine) in preparation for the "ms1" command.
# The second spine will become underlay in the mup output.
#
BEGIN	{
	TRUE = 1; FALSE = 0; PROCESS = FALSE
	FS="	"
	}
{
if ($0 ~/^!!/)	# Echo global comments.
	{
	print $0
	next
	}
if ($0 ~/^!/)	# Echo single local comments.
	{
	print $1
	next
	}
if (NF == 1)
	{
	print $0
	next
	}
if (NF > 2)
	{
	print "msjoin: ERROR: Input to msjoin cannot have more than two spines." 
	exit
	}
if ($0 ~ /^\*/)	# Process interpretations.
	{
	if ($0 ~ /^\*\*/)
		{
		# Establish which representations are present
		# and in what order.
		if ($1 == "**kern") ORDER = "1-2"
		if ($2 == "**kern") ORDER = "2-1"
		PROCESS = TRUE
		print "**kern"
		next
		}
	else	{
		if (ORDER == "1-2") print $1
		else if (ORDER == "2-1") print $2
		else print $1	# Print a default of some sort.
		next
		}
	}
if (PROCESS)
	{
	if ($1 == "." && $2 == ".") next
	if ($0 ~/^=/)
		{
		if (ORDER == "1-2") print $1
		else if (ORDER == "2-1") print $2
		next
		}
	# Process any underlay spine.
	# NOTE: MUP always wants to interpret hyphens and spaces as
	# syllable delimiters, so we have to work around this.
	# Eliminate leading hyphens, change other hyphens to periods, and
	# fill spaces with a period.
	if (ORDER == "1-2") {gsub("^-","",$2); gsub(" ",".",$2); gsub("-",".",$2)}
	else if (ORDER == "2-1") {gsub("^-","",$1); gsub(" ",".",$1); gsub("-",".",$1)}

	# NOTE:  Due to a design problem in MUP, we have to
	# throw away underlay data that isn't associated with a note.
	if (ORDER == "1-2" && $1 != ".") print $1 "<SEP>" $2
	else if (ORDER == "2-1" && $2 != ".") print $2 "<SEP>" $1
	}
}
