##############################################################################
##                           POST.AWK
#
# Programmed by:  David Huron     Date: 1995 June
# Copyright (c) 1995 David Huron
#
# Modifications:
#   Date:      Programmer:    Description:
#
# The POST program is a  post-processor for ESAC database conversion.
#
# This program carries out post-processing in the conversion of ESAC-data
# files to Humdrum **kern data.  This program should be invoked following
# use of the "ekern" program.
#
# This program has two functions (1) it changes occurrences of double
# breves ("00") to two tied notes; (2) it re-positions changes of
# meter so the meter signature appears in the correct place.
#
BEGIN	{
	TRUE = 1; FALSE = 0
	process = initial_process = subsequent_process = FALSE
	}
{
# If the end of spine is reached, reset the initial conditions.
if ($0 == "*-")
	{
	# Flush any stored input.
	if (process) for (i=1; i<lines; i++) print record[i]
	print $0
	process = initial_process = subsequent_process = FALSE
	next
	}
# If no processing is required, simply echo input.
if (!process && $0 !~/^\*MZ/)
	{
	if ($0 ~ /00/ && $0 !~ /^[!*]/)	# Re-write double-breves as tied notes.
		{
		gsub("00","0",$0)
		first = second = $0
		gsub("00","[0",first)
		gsub("[^{(0ABCDEFGabcdefg#-]","",first)
		gsub("[^})0ABCDEFGabcdefg#-]","",second)
		gsub("0","[&",first)
		gsub("[ABCDEFGabcdefg#-]+","&]",second)
		print first
		print second
		}
	else print $0
	next
	}
else
	{
	if ($0 ~/^\*MZ/)	# Process *MZ records.
		{
		process = initial_process = TRUE
		next
		}
	else
		{		# Process other records.
		if (initial_process)
			{
			current = $0
			lines = 1
			record[lines++] = $0
			while (current !~ /^!\*M/)
				{
				getline
				current = $0
				# Re-write double-breves as tied notes.
				if ($0 ~ /00/ && $0 !~ /^[!*]/)
					{
					gsub("00","0",$0)
					first = second = $0
					gsub("00","[0",first)
					gsub("[^{(0ABCDEFGabcdefg#-]","",first)
					gsub("[^})0ABCDEFGabcdefg#-]","",second)
					gsub("0","[&",first)
					gsub("[ABCDEFGabcdefg#-]+","&]",second)
					record[lines++] = first
					record[lines++] = second
					}
				else record[lines++] = $0
				}
			gsub("^!.M","*M",$0)
			print $0
			for (i=1; i<lines-1; i++) print record[i]
			lines = 1
			initial_process = FALSE
			subsequent_process = TRUE
			next
			}
		if (subsequent_process)
			{
			if ($0 ~ /^=/)
				{
				if (lines > 1) print record[1]
				if (new_meter) {print meter; new_meter = FALSE}
				for (i=2; i<lines; i++) print record[i]
				lines = 1
				record[lines++] = $0
				}
			else
				{
				if ($0 ~/^!\*M/)
					{
					new_meter = TRUE
					gsub("^!.M","*M",$0)
					meter = $0
					}
				else
					{
					# Re-write double-breves as tied notes.
					if ($0 ~ /00/ && $0 !~ /^[!*]/)
						{
						gsub("00","0",$0)
						first = second = $0
						gsub("00","[0",first)
						gsub("[^{(0ABCDEFGabcdefg#-]","",first)
						gsub("[^})0ABCDEFGabcdefg#-]","",second)
						gsub("0","[&",first)
						gsub("[ABCDEFGabcdefg#-]+","&]",second)
						record[lines++] = first
						record[lines++] = second
						}
					else record[lines++] = $0
					}
				}
			}
		}
	}
}
