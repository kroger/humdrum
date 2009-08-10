##################################################################
#                         MS.AWK
#
# Programmed by: David Huron        Date: January, 1997
# Copyright (c) 1997 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
#
# This program does some pre-processing prior to part extraction
# in the MS program.
#
# Currently, this pre-processor carries out only a single task.
# It monitors changes of clef, and ensures that when a change occurs,
# the current respective clefs are echoed in each of the other
# spines.
#
{
# Print anything that isn't an interpretation.
if ($0 !~ /^\*/) print $0
else
	{
	line = $0
	# Keep track of clefs in each spine.  If one spine changes
	# clef, then echo the current clef in each of the other
	# spines.
	if ($0 ~ /^\*clef[CFG][0-9]/ || $0 ~ /	\*clef[CFG][0-9]/)
		{
		if ($1 ~ /^\*clef/)
			{
			line = $1
			clef[1] = $1
			}
		else
			{
			if (clef[1] != "") line = clef[1]
			else line = "*"
			}
		for (i=2; i<=NF; i++)
			{
			if ($i ~ /^\*clef/)
				{
				clef[i] = $i
				line =  line "	" $i
				}
			else if ($i == "*")
				{
				line = line "	" clef[i]
				}
			}
		}
	print line
	next
	}
}
