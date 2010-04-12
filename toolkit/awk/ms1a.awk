##################################################################
#                         MS1A.AWK
#
# Programmed by: David Huron        Date: January, 1997
# Copyright (c) 1997 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
#
#
# This program does some pre-processing prior to the ms1b.awk
# program.
#
{
if ($0 ~ /^[\*!=]/)	# Echo comments, barlines & interpretations.
	{
	print $0
	next
	}
record = $0
# Eliminate the duration information from all but the first note
# in a multiple-stop.  Also, ensure that all beaming, slur, tie,
# phrase, articulation and other material is present in the first
# note of the multiple-stop.
if (record ~ / /)
	{
	# Split the multiple stops.
	num = split(record,token," ")

	# Determine if there is any non-duration/non-pitch information
	# in subsequent tokens of multiple-stops that is not present in
	# the first token.  If so, transfer this material.
	transfer = ""
	for (i=1; i<=num; i++)
		{
		gsub("[0-9.ABCDEFGabcdefg#n-]","",token[i])
		if (i > 1 && length(token[i]) == 0) continue
		if (token[i] == token[1]) continue
		else transfer = transfer token[i]
		}
	# Eliminate pitch and duration information from all tokens.
	# Again split the multiple stops.
	num = split(record,token," ")
	record = token[1] transfer	# Add transfered signifiers.
	for (i=2; i<=num; i++)
		{
		# Eliminate everything but the pitch and
		# accidental information.
		gsub("[^ABCDEFGabcdefg#n-]","",token[i])
		record = record "+++" token[i]
		}
	}
print record
}
