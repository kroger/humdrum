##########################################################################
##                           URRHYTHM.AWK
# Programmed by: David Huron        Date: ?
# Copyright (c) 1993 David Huron
#
# Modifications:
#    Date:     Programmer:        	Description:
# June 17/94	Tim Racinsky	Modified parse_command to work with getopts
# July 29/94	Tim Racinsky	Made general changes to program (e.g. modular).
#						but did not change main workings of program
#
# This program outputs a humdrum spine (**URrhythm) containing data
# representing rhythmic `prototypes' evident in a musical passage.
# URRHYTHM accepts two input spines: **kern (or **recip) and **metpos.
# The program implements a variation of Johnson-Laird's theory of rhythmic
# prototypes.
#
BEGIN {
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	# Some global variables
	#
	note_onset = FALSE
	first_onset = FALSE
	kern_spine = 0
	metpos_spine = 0
	metric_value = ""
	meter_sig = "^\\*M(([1-9]+((\\+)([1-9]+))*(\\/)([0-9]+)(\\.)*)|(\\?)|(X))$"
	null_interp = "^\\*(\t\\*)*$"
	spine_path_record = "^\\*(\\+|v|\\^|x|-)?(\t\\*(\\+|v|\\^|x|-)?)*$"
	exclusive_record = "^\\*(\\*([^\t])+)?(\t\\*(\\*([^\t])+)?)*$"
	spine_terminate = "^\\*-(\t\\*-)*$"
	cont_tie_reg = "\\]|_"
	#
	# Define the lowest metric-position (highest metpos value) for meters
	# having various numerators (e.g. 2/2, 3/2, etc.)
	beats[2] = 2
	beats[3] = 2
	beats[4] = 3
	beats[6] = 2
	beats[9] = 2
	beats[12] = 3
	ARGV[1] = ""
	}
{
if ($0 ~ /^!!/) print $0
else if (NF != 2)
	{
	print "urrhythm: ERROR: Input must contain precisely two spines." > stderr
	exit
	}
else if ($0 ~ /^!/) print $0 "\t!"
else if ($0 ~ exclusive_record && $0 !~ null_interp) process_exclusive()
else if ($0 ~ spine_terminate) reset_spines()
else if ($0 ~ spine_path_record && $0 !~ null_interp)
	{
	print "urrhythm: ERROR: Spine-path terminators are the only spine-path "\
		 "indicators\n                 allowed in the input." > stderr
	exit
	}
else if ($0 ~ /^\*/) process_tandem()
else process_data()
}

###########################################################################
##				Function reset_spines
#
function reset_spines()
	{
	note_onset = FALSE
	first_onset = FALSE
	kern_spine = 0
	metpos_spine = 0
	print $0 "\t*-"
	}

###########################################################################
##				Function process_exclusive
#
function process_exclusive(    i)
	{
	for (i = 1; i <= NF; i += 1)
		{
		if ($i ~ /^\*\*(kern|recip)$/) kern_spine = i
		else if ($i ~ /^\*\*metpos$/) metpos_spine = i
		}
	if (kern_spine == 0)
		{
		print "urrhythm: ERROR: No kern spine in input at line "\
			  FNR "." > stderr
		exit
		}
	else if (metpos_spine == 0)
		{
		print "urrhythm: ERROR: No metpos spine in input at line "\
			  FNR "." > stderr
		exit
		}
	else if (kern_spine == metpos_spine)
		{
		print "urrhythm: ERROR: Both spines are now of the same type at "\
			 "line " FNR "." > stderr
		exit
		}
	print $0 "\t**URrhythm"
	}

###########################################################################
##				Function process_tandem
#
function process_tandem(   meter,numerator)
	{
	if (kern_spine == 0)
		{
		print "urrhythm: ERROR: No kern spine has been started." > stderr
		exit
		}
	else
		{
		print $0 "\t" $kern_spine
		#
		# Establish the meter signature.  Find out the number of beats per
		# measure.
		#
		if ($kern_spine ~ meter_sig)
			{
			meter = $kern_spine
			gsub("*M","",meter)
			numerator = meter		  # Find the metric numerator ...
			gsub("\/.*","",numerator)  # ... by eliminating the denominator.
			#
			# If the numerator is other than 2, 3, 4, 6, 9, or 12,
			# issue an error and exit.
			#
			if (numerator !=2 && numerator !=3 && numerator !=4 && \
				numerator !=6 && numerator !=9 && numerator !=12)
				{
				print "urrhythm: ERROR: cannot handle meter: " meter " in line "\
					NR "." > stderr
				exit
				}
			metric_value = beats[numerator]
			}
		}
	}

###########################################################################
##				Function process_data
#
function process_data( )
	{
	if ($kern_spine ~ /^=/) print $0 "\t" $kern_spine
	else
		{
		#
		# Determine whether the current sonority has a note onset.
		#
		if ($kern_spine != "." && $kern_spine !~ /r/ \
									&& $kern_spine !~ cont_tie_reg)
			{
			note_onset = TRUE
			first_onset = TRUE
			#
			# Record the associated metpos value.
			#
			onset_metpos_value = $metpos_spine
			}
		else note_onset = FALSE
		#
		# Syncopations arise when no note-onset happens at a moment whose
		# metric position is more important than that of the most recent
		# note onset.
		#
		if (!note_onset && first_onset && $metpos_spine < onset_metpos_value)
			{
			if ($metpos_spine > metric_value) print $0 "\t."
			else print $0 "\tS"
			#
			# Syncopated moments can happen only after an onset;
			# subsequent syncopated moments will require another onset.
			# (i.e. two syncopated moments can't occur without some note
			#  onset intervening).
			#
			first_onset = FALSE
			}
		else
			{
			# Determine whether the input is a Note (N) or Other (O)
			if ($metpos_spine > metric_value) print $0 "\t."
			else
				{
				if (note_onset) print $0 "\tN"
				else print $0 "\tO"
				}
			}
		}
	}
