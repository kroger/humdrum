#####################################################################
##					SYNCO.AWK
# Programmed by: David Huron         Date: ?
# Copyright (c) 1994 David Huron
#
# Modifications:
#    Date:   Programmer:         	Description:
# Aug 2/94	Tim Racinsky	Modified parse_command to work with getopts
#
# Main Functions used:
#	Parse_command()		Store_indicators()		Store_new_interps()
#	Process_indicators()	Ins_array_pos()		Del_array_pos()
#	Exchange_spines()		Initialize()
#
#
#	VARIABLES:
#
#	options			holds options passed to this program
#	output_timebase	flag to indicate timebase interpretation should be
#					printed
#	note_onset		indicates whether the current sonority has a note onset
#	first_onset		indicates whether an onset has happened yet
#					(syncopated moments can happen only after an onset)
#	no_interpretation_yet	indicates whether first exclusive interpretat
#					has been encountered
#	current_no_of_spines	current number of spines in the input
#	metpos_spine		the input field number for **metpos information
#	onset_metpos_value	holds the current metric position value from **metpos
#					input
#	timebase			holds the current time-base value (*tb...)
#	processing		true when the input stream is suitable for calculating
#					and printing an output
#	ARRAYS:
#
#	target_interp[]	exclusive interpretations to be processed
#	process_spine[]	which data columns should be processed
#	path_indicator[]	contains successive spine-path record tokens
#	arraya[]			temporary array
#	spine_path_record	true when the current record is a spine-path record
#
BEGIN {
	#
	# Flags, variables, and constants:
	#
	FS = OFS = "\t"		# Set field separator to tab.
	YES = TRUE = 1;   NO = FALSE = 0
	#
	# Pipe all errors to this variable so that error messages are printed
	# to the terminal always
	#
	"echo $CON" | getline stderr
	close("echo $CON")
	options = ""
	cont_tie_reg = "\\]|_"
	#
	initialize()
	#
	target_interp[1]  = "**pitch"
	target_interp[2]  = "**barks"
	target_interp[3]  = "**cbr"
	target_interp[4]  = "**cents"
	target_interp[5]  = "**cocho"
	target_interp[6]  = "**deg"
	target_interp[7]  = "**degree"
	target_interp[8]  = "**freq"
	target_interp[9]  = "**fret"
	target_interp[10] = "**kern"
	target_interp[11] = "**mels"
	target_interp[12] = "**pc"
	target_interp[13] = "**Tonh"
	target_interp[14] = "**pu"
	target_interp[15] = "**semits"
	target_interp[16] = "**solfa"
	target_interp[17] = "**solfg"
	target_interp[18] = "**specC"
	#
	# Determine the options specified by the user, passed from the shell script
	#
	parse_command()
	}
############################################################################
#				              MAIN
############################################################################

{
if (FNR == 1)
	{
	#
	# In the case of multiple input files, the following variables are reset.
	#
	initialize()
	#
	}
if ($0 ~ /^!!/)
	{
	# Echo global comments
	#
	print $0
	next
	#
	}
else if ($0 ~ /^!([^!]|$)/)
	{
	# Echo local comments as a null local comment.
	#
	if (options ~ /e/) print $0 "\t!"
	else print "!"
	next
	#
	}
else if ($0 ~ /^\*/)
	{
	#
	# Process the first interpretation record.  (By definition, the
	# first interpretation record must be an exclusive interpretation.)
	#
	if (no_interpretation_yet)
		{
		current_no_of_spines = NF
		no_interpretation_yet = FALSE
		store_new_interps()
		}
	#
	# Otherwise, check to see if it is a spine-path record
	#
	else
		{
		spine_path_record = FALSE
		for (i=1; i<=current_no_of_spines ; i++)
			{			
			if ($i ~ /^\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x$/)
				{
				spine_path_record = TRUE 
				break
				}
			}
		#
		# If it is a spine-path record, store and process the indicators
		#
		if (spine_path_record)
			{
			store_indicators()
			process_indicators()
			#
			# Reset any necessary variables if all spines terminate
			#
			if (current_no_of_spines == 0)
				{
				no_interpretation_yet = TRUE
				}
			}
		#
		# Otherwise, process the interpretations (both tandem & exclusive)
		#
		else
			{
			store_new_interps()
			}		
		}
	}
else
	{
	########################################################################
	#
	#	Process data records.
	#
	########################################################################
	#
	# Don't process data unless you have to.
	#
	if (!processing) next
	#
	# Determine whether the current sonority has a note onset.
	#
	note_onset = FALSE
	for (i=1; i<=current_no_of_spines; i++)
		{
		if (!process_spine[i]) continue
		# Echo barlines.
		if ($i ~ /^\=/)
			{
			if (options ~ /e/) print $0 "\t" $i
			else print $i
			next
			}
		#
		if ($i != "." && $i !~ /r/ && $i !~ cont_tie_reg)
			{
			note_onset = TRUE
			first_onset = TRUE
			# Record the associated metpos value.
			onset_metpos_value = $metpos_spine
			}
		}
	# Syncopations arise when no note-onset happens at a moment whose metric
	# position is more important than that of the most recent note onset.
	#
	if (!note_onset && first_onset && $metpos_spine < onset_metpos_value)
		{
		# Avoid taking the logarithm of zero.
		#
		if (onset_metpos_value != 0 && $metpos_spine != 0)
			{
			if (options ~ /e/) printf("%s\t%4.2f\n",$0,\
					log(onset_metpos_value) - log($metpos_spine))
			else printf("%4.2f\n",\
					log(onset_metpos_value) - log($metpos_spine))
			}
		else
			{
			print "synco: ERROR: Illegal metric position value `0' found" \
							" near line: " NR > stderr
			exit
			}
		#
		# Syncopated moments can happen only after an onset;
		# subsequent syncopated moments will require another onset.
		# (i.e. two syncopated moments can't occur without some note
		#  onset intervening).
		first_onset = FALSE
		}
	else
		{
		if (options ~ /e/) print $0 "\t0"
		else print "0"
		}
	}
}

############################################################################
#				FUNCTIONS USED IN SYNCO.AWK
############################################################################

############################################################################
##				Function Parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	#
	# Process any necessary variables
	#
	if (ARGV[2] != "null") options = ARGV[2]
	ARGV[1] = ARGV[2] = ""
	}

################################################################
##			Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
#
function store_new_interps(      j,i)
	{
	#
	# Process exclusive interpretations
	#
	if ($0 ~ /^\*\*/ || $0 ~ /\t\*\*/)
		{
		# Process data only if a **metpos spine and at least
		# one **[target_interp] spine are found in the input.
		#
		processing = FALSE
		#
		# Identify whether an active **metpos spine has been changed
		# to some other interpretation.
		#
		#if (metpos_spine == j && $j !~ /^\*\*metpos$/) metpos_spine = 0
		#
		# Identify any **metpos spine.
		#
		for (j = 1; j <= current_no_of_spines; j++)
			{
			if ($j ~ /^\*\*metpos$/) metpos_spine = j
			}
		#
		# Identify all input spines that contain target interpretations.
		#
		for (j = 1; j <= current_no_of_spines; j++)
			{
			if ($j ~ /^\*\*/) process_spine[j] = FALSE
			#
			for (i in target_interp)
				{
				if ($j == target_interp[i]) process_spine[j] = YES
				if (process_spine[j] && metpos_spine !=0) processing = YES
				}
			}
		if (processing)
			{
			if (options ~ /e/) print $0 "\t**synco"
			else print "**synco"
			}
		}
	#
	# Process tandem interpretations.
	#
	# If a timebase tandem interpretation is present, check to ensure
	# that the timebase is not excessively short.  It is possible to
	# have timebase values that exceed the temporal resolving power of
	# listeners.  For example, if an onset appears a thirty-second
	# duration prior to an expected down-beat, the listener is apt to
	# hear the displaced onset as occuring ON the beat rather than being
	# a very short syncopation.  "Excessively short" is defined as a
	# timebase resolution shorter than a 16th note.
	#
	else
		{
		if (!processing) return
		output_timebase = FALSE
		for (j = 1; j <= current_no_of_spines; j++)
			{
			if ($j ~ /^\*tb[1-9]?[0-9]*$/)
				{
				timebase = $j
				output_timebase = YES
				gsub("*tb","",timebase)
				if (timebase+0 > 16) print "synco: WARNING: Time-base very"\
				" short; listener's temporal resolution exceeded?" > stderr
				}
			}
		if (output_timebase)
			{
			if (options ~ /e/) print $0 "\t*tb" timebase
			else print "*tb" timebase
			}
		else # Echo all other tandem interpretations.
			{
			if (options ~ /e/) print $0 "\t*"
			else print "*"
			}
		}
	}

##############################################################
##			Function Store_indicators
#	This function allows the spine-path indicators for the 
# current record to be stored in the array 'path_indicator' so
# that they may be used later.
#
function store_indicators(   i)
	{
	for (i = 1; i <= current_no_of_spines; i++)
		{
		#
		# If the current field is a null interpretation
		#
		if ($i == "*") path_indicator[i] = "*"
		#
		# If the current field is a spine-path indicator, store it
		#
		else path_indicator[i] = substr($i,2,1)
		}
	} 

##################################################################
##				Function Process_indicators
#	This function takes the spine-path indicators that were stored
# in the array 'path_indicator' in the function 'store_indicators'
# and manipulates the arrays 'path_indicator' and 'process_spine'
# according to the contents of the array 'path_indicator'.
#
function process_indicators(    i,k,end)
	{
	# If all spine-path indicators are spine terminators, print
	# the appropriate output record, and get the next input record.
	#
	end = TRUE
	for (i=1; i<=current_no_of_spines; i++) if ($i != "*-") end = FALSE
	if (processing)
		{
		if (end)
			{
			if (options ~/e/) print $0 "\t*-"
			else print "*-"
			initialize()
			return
			}
		# Otherwise print the appropriate spine-path record.
		#
		else
			{
			if (options ~ /e/) print $0 "\t*"
			else print "*"
			}
		}
	#
	i = 1
	#
	# Loop through the entire array of 'path_indicator'
	# NOTE: This array may be altered during the process
	#
	while (i <= current_no_of_spines)
		{
		#
		# If a spine-split indicator occurred
		#
		if (path_indicator[i] == "^")
			{
			#
			# Insert a new array position in 'path_indicator'
			# and 'process_spine'
			#
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			process_spine[i+1] = process_spine[i]	# Copy interpretations
											# to the new spine.
			if (metpos_spine > i) metpos_spine++
			}
		#
		# If a spine-termination indicator occurred
		#
		if (path_indicator[i] == "-")
			{
			#
			# Delete that spine from the arrays and look at the same
			# position the next time through the loop
			#
			del_array_pos(i)
			if (metpos_spine > i) metpos_spine--
			i--
			}
		#
		# If a spine-exchange indicator occurred
		#
		if (path_indicator[i] == "x")
			{
			#
			# Exchange the paths
			#
			exchange_spines()
			}
		#
		# If a join-path indicator occurred
		#
		if (path_indicator[i] == "v")
			{
			del_array_pos(i+1)	# Delete NEXT spine (must be join-path
							# spine according to humdrum).
			if (metpos_spine > i) metpos_spine--
			#
			# If the next indicator is not a join-path indicator
			# then return to original conditions
			#
			if (path_indicator[i+1] != "v")
				{
				path_indicator[i] = "*"
				}
			#
			# Otherwise, look at current and next paths to join
			#
			else i--
			}
		#
		# If a new-path indicator occurred
		#
		if (path_indicator[i] == "+")
			{
			#
			# Insert a new position in the arrays
			#
			ins_array_pos(i+1)
			process_spine[i+1] = ""
			if (metpos_spine > i) metpos_spine++
			}
		i++
		}
	}

############################################################
##			Function Ins_array_pos
#	This function inserts a new position in the arrays 'path_indicator'
# and 'process_spine' and copies elements so that order is preserved
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j--)
		{
		path_indicator[j] = path_indicator[j-1]
		process_spine[j] = process_spine[j-1]
		}
	path_indicator[current_element-1] = "*"	# (Avoid possibility of
	path_indicator[current_element] = "*"	#  reprocessing the old spine.)
	current_no_of_spines++
	}

##############################################################
##			Function Del_array_pos
#	Performs the opposite of function 'ins_array_pos'.
#
function del_array_pos(current_element,     j)
	{
	for (j = current_element; j < current_no_of_spines; j++)
		{
		path_indicator[j] = path_indicator[j+1]
		process_spine[j] = process_spine[j+1]
		}
	delete path_indicator[j]
	delete process_spine[j]
	current_no_of_spines--
	} 

###################################################################
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in 'process_spine'
#
function exchange_spines(    j,count,arraya,temp)
	{
	count = 1
	# Determine which spines are exchanged
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		if (path_indicator[j] == "x")
			{
			arraya[count] = j
			count += 1
			path_indicator[j] = "*"
			}
		}
	# Exchange interpretations between the two spines
	temp = process_spine[arraya[1]]
	process_spine[arraya[1]] = process_spine[arraya[2]]
	process_spine[arraya[2]] = temp
	}
###################################################################
##				Function Initialize
#	This function initializes or resets various values for a new input.
function initialize()
	{
	note_onset = FALSE
	first_onset = FALSE
	no_interpretation_yet = TRUE
	current_no_of_spines = 0
	metpos_spine = 0
	for (i in process_spine) process_spine[i] = FALSE
	}
