#########################################################################
# 					TIMEBASE.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# June 7/94	Tim Racinsky	Modified parse_command to work with getopts
# July 12/94	Tim Racinsky	Modified to allow for dotted timebase
#
#
# This program reformats a humdrum score input so that it exhibits a
# regular time base  -- in other words, each successive line of code
# represents the passage of an equivalent duration of time.
#
#	OPTIONS ARE:
#	t timebase sets timebase for current file (timebase is of form N)
#	x the encoded duration attributes are stripped from the output;
#	  otherwise duration attributes are echoed in the output.
#
# Main Functions used:
#	Parse_command()		Get_durations() 		Find_duration()
#	Find_shortest() 		Subtract_time()		Print_null()
#	Store_indicators()		Process_indicators()	Ins_array_pos()
#	Del_array_pos()		Num_of_exchanges()		Exchange_spines()
#	Store_new_interps()
#
# Helper Functions used:
#	Print_indics()			Print_interps() 		Print_durations()
#
#	VARIABLES:
#
#  -spine_path_record:  flag to indicate if current interpretation record
#   is also a spine-path record.
#  -FIRST_DATA_RECORD:  flag to indicate if this is the first data record of
#   the file or first one after all spines have been terminated so that the
#   timebase interpretation record may be printed.
#  -no_interpretation_yet:  flag to indicate if first interpretation record
#   has been found yet.
#  -current_no_of_spines:  holds current number of active spines.
#  -temp_base:  temporarily holds the timebase specified by the user.
#  -time_base:  holds the floating pt. timebase the user wishes to use.
#  -counter:  holds how much time remains until the next line must be 
#   printed according to the timebase.
#  -shortest_duration:  holds the note with the least amount of time
#   remaining at the current input record.
#  -barline: flag to indicate if current record is a barline or not.
#  -kern_recip: flag to indicate if current input contains any kern or
#   recip spines.
#  -timebase_reg: regular expression to match a valid timebase value.
#  -NULL: flag to indicate if current record is all null tokens.
#  -GRACE: flag to indicate if current record contains grace note.
#
#	ARRAYS:
#
#  -running_durations:  holds the amount of time that remains for each
#   note (or rest) in the current input for each spine.
#  -path_indicator and current_interp:  the standard arrays to assist in
#   processing spine-path indicators.
#
BEGIN {
	USAGE="\nUSAGE: timebase -h                 (Help Screen)\n       timeb"\
		 "ase [-x] -t N [file ...]"
	TRUE = 1; FALSE = 0
	FS = OFS = "\t"
	timebase_reg = "^[0-9]+[.]*$"
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	# Parse the command line options
	#
	parse_command()
	#
	# Determine the floating point value of the timebase
	#
	match(temp_base,/[0-9]+/)
	duration = substr(temp_base,RSTART,RLENGTH)
	dots = gsub("[.]",".",temp_base)
	#
	# Determine initial timebase
	#
	if (duration != "0") time_base = 1 / (duration + 0)
	else time_base = 2
	#
	# Determine the extra 'dotted' time
	#
	half = time_base / 2
	while (dots > 0)
		{
		time_base += half
		half /= 2
		dots -= 1
		}
	}
{
#
# If this is the first record for the current file, reset all the variables
#
if (FNR == 1)
	{
	spine_path_record = FALSE
	FIRST_DATA_RECORD = no_interpretation_yet = TRUE
	current_no_of_spines = shortest_duration = 0
	counter = time_base
	for (i in running_durations) delete running_durations[i]
	kern_recip = FALSE
	}
#
# If this is a comment, print it
#
if ($0 ~ /^!/)
	{
	print $0
	next
	}
#
# If it is an interpretation record, print and process it
#
if ($0 ~ /^\*/)
	{
	print $0
	#
	# If it is the first interpretation record, initialize the variables
	#
	if (no_interpretation_yet)
		{
		current_no_of_spines = NF
		no_interpretation_yet = FALSE
		for (i = 1; i <= current_no_of_spines; i++) running_duration[i] = 0
		store_new_interps()
		}
	#
	# If it is not the first interpretation record
	#
	else
		{
		spine_path_record = FALSE
		for (i=1; i<=current_no_of_spines ; i++)
			{			
			if ($i ~ /^\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x$/)
				{ spine_path_record = TRUE ; break }
			}
		#
		# If it is a spine-path record, store and process the spine-path
		# indicators and reset variables if necessary
		#
		if (spine_path_record)
			{
			store_indicators()
			process_indicators()
			if (current_no_of_spines == 0)
				{
				no_interpretation_yet = TRUE
				FIRST_DATA_RECORD = TRUE
				}
			}
		#
		# Otherwise, simply store the new interpretations
		#
		else store_new_interps()
		}
	next
	}
#
# If the current record does not contain any kern or recip spines, then
# just print the record and move to the next record.
#
if (!kern_recip)
	{
	print $0
	next
	}
#
# Otherwise, the record is a data record and should contain some kern notes
# If it is the first data record, print out timebase indicators
#
if (FIRST_DATA_RECORD == TRUE)
	{
	for (i = 1; i <= current_no_of_spines; i++)
		{
		if (current_interp[i] == "**kern" || current_interp[i] == "**recip")
			output_line = output_line "*tb" temp_base "\t"
		else output_line = output_line "*\t" 
		}
	print substr(output_line,1,length(output_line)-1)
	output_line = ""
	FIRST_DATA_RECORD = FALSE
	}
#
# If the current record contains a barline in a kern or recip spine
#
barline = FALSE
for (i = 1; i <= current_no_of_spines; i++)
	{
	if ((current_interp[i] == "**kern" || current_interp[i] == "**recip") \
	    && ($i ~ /^=/))
		{
		#
		# Print it out
		#
		print $0
		#
		# Reset the counter and running durations
		#
		counter = time_base
		for (i = 1; i <= current_no_of_spines; i++) running_duration[i] = 0
		barline = TRUE
		break
		}
	}
if (barline) next
#
# If all kern and recip spines contain only null tokens, skip the record
#
NULL = TRUE
for (i = 1; i <= current_no_of_spines; i++)
	{
	if ((current_interp[i] == "**kern" || current_interp[i] == "**recip") \
	    && ($i !~ /^[.]$/))
		{
		NULL = FALSE
		break
		}
	}
if (NULL) next
#
# If any kern or recip spine contains a grace note, skip the record
#
GRACE = FALSE
for (i = 1; i <= current_no_of_spines; i++)
	{
	if ((current_interp[i] == "**kern" || current_interp[i] == "**recip") \
	    && ($i ~ /q/))
		{
		GRACE = TRUE
		break
		}
	}
if (GRACE) next
#
# Get the next set of durations and find the shortest duration
#
get_durations()
shortest_duration = find_shortest()
#
# If the user did not specify -x option, leave the record as it was.
#
if (options !~ /x/) record = $0
#
# Otherwise, strip away the duration information.
#
else
	{
	for (i = 1; i <= current_no_of_spines; i++)
		{
		if (current_interp[i] == "**kern" || current_interp[i] == "**recip")
			{
			gsub(/[0-9]+/,"",$i)
			gsub(/[.]+/,"",$i)
			}
		if ($i == "") $i = "."
		if (i == 1) record = $i
		else record = record "\t" $i
		}
	}
#
# If the counter has been reset to the current timebase, print the record
#
if (counter == time_base)
	{
	print record
	subtract_time(shortest_duration)
	counter = counter - shortest_duration
	#
	# If current durations last longer than timebase, null records must be
	# inserted to 'pad' the output
	#
	while (counter < -0.0001)
		{
		print_null()
		counter = counter + time_base
		}
	if (counter > -0.0001 && counter < 0.0001) counter = time_base
	}
#
# If the shortest duration lasts longer than the time remaining on the counter,
# then print the record
#
else if (shortest_duration - counter > 0.0001)
	{
	print record
	subtract_time(shortest_duration)
	counter = (counter - shortest_duration) + time_base
	#
	# If current durations last longer than timebase, null records must be
	# inserted to 'pad' the output
	#
	while (counter < -0.0001)
		{
		print_null()
		counter = counter + time_base
		}
	if (counter > -0.0001 && counter < 0.0001) counter = time_base
	}
#
# Otherwise, do not print the current record
#
else
	{
	subtract_time(shortest_duration)
	counter = counter - shortest_duration
	if (counter > -0.0001 && counter < 0.0001) counter = time_base
	}
}

######################################################################
##			FUNCTIONS USED IN TIMEBASE.AWK
######################################################################

#####################################################################
##				Function Parse_command
#	This function parses the options passed from timebase.ksh.
# If there are any options (other than -t), they will be in ARGV[2].
#
function parse_command()
	{
	#
	# Set the 'options' variable
	#
	if (ARGV[2] != "null") options = ARGV[2]
	if (ARGV[3] !~ timebase_reg)
		{
		print "timebase: ERROR: Invalid timebase value "\
			 "specified: " ARGV[3] > stderr
		exit
		}
	else temp_base = ARGV[3]
	if (ARGV[4] != "null") pre_marker = ARGV[4]
	if (ARGV[5] != "null") post_marker = ARGV[5]
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ARGV[5] = ""
	}

############################################################################
##				Function Get_durations
#	This function determines the new durations for the current input line.
# Note that only those spines whose durations are 0 (or close to it because
# of roundoff) are replaced with new durations.  It is assumed the user has
# encoded correctly and thus the remaining spines should have null tokens
#
function get_durations(   j)
	{
	for (j = 1; j <= current_no_of_spines; j++)
		{
		#
		# If the current spine has note values, process
		#
		if (current_interp[j] == "**kern" || current_interp[j] == "**recip")
			{
			#
			# If the current spine's duration is very close to 0, get
			# the duration for that spine
			#
			if (running_duration[j] <= 0.0001 \
					&& running_duration[j] >= -0.0001)
				running_duration[j] = find_duration($j)
			}
		}
	}

##########################################################################
##				Function Find_duration
#	This function is called from get_durations() and it is passed the token
# of the current spine in order to determine the duration of the note.
# If a double stop is used, process only the first token in the double stop.
#
function find_duration(token,    duration,arraya)
	{
	#
	# Split the current token to separate double stops
	#
	split(token,arraya," ")
	#
	# If there is no duration information, print an error
	#
	if (match(arraya[1],/[0-9]+/) == 0)
		{
		print "timebase: ERROR: No duration specified in line " NR " for "\
			 "token '" arraya[1] "'." > stderr
		exit
		}
	#
	# Otherwise, get the duration
	#
	else
		{
		#
		# Store the number value in duration and determine number of 'dots'
		#
		duration = substr(arraya[1],RSTART,RLENGTH)
		dots = gsub(/[\.]/,"",arraya[1])
		#
		# Determine duration for all notes except breve
		#
		if ((duration != "0")&&(duration != "00")) {proportion = 1 / duration+0}
		#
		# Assign breve duration
		#
		if (duration == "0") { proportion = 2}
		if (duration == "00") {proportion = 4}
		half = proportion / 2
		#
		# Change duration according to number of dots
		#
		while (dots > 0)
			{
			proportion = proportion + half
			half = half / 2
			dots--
			}
		return (proportion)
		}
	}

##########################################################################
##				Function Find_shortest
#	This function determine which spine contains the note with the least
# amount of time left on it.
#
function find_shortest(  j,current_shortest)
	{
	#
	# Assume a maximum of 100 and loop through each valid spine to
	# find a smaller value
	#
	current_shortest = 100
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if (current_interp[j] == "**kern" || current_interp[j] == "**recip")
			{
			if (running_duration[j] < current_shortest)
				current_shortest = running_duration[j]
			}
		}
	return current_shortest
	}

###############################################################################
##				Function Subtract_time
#	This function subtracts the appropriate amount of time from each
# of the running durations for each spine.
#
function subtract_time(time,   j,null_line)
	{
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if (current_interp[j] == "**kern" || current_interp[j] == "**recip")
			running_duration[j] = running_duration[j] - time
		}
	}

###############################################################################
##				Function Print_null
#	This function prints a null data record in order to pad the output
#
function print_null(   j,null_line)
	{
	null_line = ""
	for (j = 1; j <= current_no_of_spines; j++) null_line = null_line ".\t"
	print pre_marker substr(null_line,1,length(null_line)-1) post_marker
	}

##############################################################
##			Function Store_indicators
#	This function allows the spine-path indicators for the 
# current record to be stored in the array 'path_indicator' so
# that they may be processed by process_indicators().
#
function store_indicators(   i)
	{
	for (i = 1; i <= current_no_of_spines; i++)
		{
		if ($i == "*") path_indicator[i] = "*"
		else path_indicator[i] = substr($i,2,1)
		}
	}

##################################################################
##			Function Process_indicators
#	This function takes the spine-path indicators that were stored
# in the array 'path_indicator' in the function 'store_indicators'
# and manipulates the arrays 'path_indicator' and 'current_interp'
# according to the contents of the array 'path_indicator'.
# The variable 'exchanges' holds the number of exchange-path indicators
# that were found.
#
function process_indicators(    i,exchanges)
	{
	i = 1
	while (i <= current_no_of_spines)
		{
		#
		# If a split path occurs
		#
		if (path_indicator[i] == "^")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = current_interp[i]
			running_duration[i+1] = running_duration[i]
			}
		#
		# If a spine path termination occurs
		#
		if (path_indicator[i] == "-")
			{
			del_array_pos(i)
			i--
			}
		#
		# If a spine exchange occurs
		#
		if (path_indicator[i] == "x")
			{
			spines_to_exchange = ""
			exchanges = num_of_exchanges()
			exchange_spines()
			}
		#
		# If a join path occurs
		#
		if (path_indicator[i] == "v")
			{
			del_array_pos(i+1)
			if (path_indicator[i+1] != "v")
				{
				path_indicator[i] = "*"
				join = FALSE
				}
			else i--
			}
		#
		# If a new path is added
		#
		if (path_indicator[i] == "+")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = ""
			}
		i++
		} #end while loop
	} #end function process_indicators

############################################################
##			Function Ins_array_pos
#	This function inserts new positions in the arrays 'path_indicator',
# 'current_interp', and running_duration and copies elements so that
# everything is preserved.
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j--)
		{
		path_indicator[j] = path_indicator[j-1]
		current_interp[j] = current_interp[j-1]	
		running_duration[j] = running_duration[j-1]
		}
	path_indicator[current_element-1] = "*"
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
		current_interp[j] = current_interp[j+1]
		running_duration[j] = running_duration[j+1]
		}
	delete path_indicator[j]
	delete current_interp[j]
	delete running_duration[j]
	current_no_of_spines--
	} 

###############################################################
##				Function Num_of_exchanges
#	This function determines how  many exchange-path indicators
# appear in the current spine-path record and stores them in
# the variable 'spines_to_exchange'.
#
function num_of_exchanges(    j,exchange_count)
	{
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if (path_indicator[j] == "x")
			{
 			exchange_count++
			spines_to_exchange = spines_to_exchange j "\t"
			path_indicator[j] = "*"
			}
		}
	return exchange_count
	}	

###################################################################
##			Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in 'current_interp' and 'running_duration'.
#
function exchange_spines(    arrayb,temp)
	{
	split(spines_to_exchange,arrayb)
	temp = current_interp[arrayb[1]]
	current_interp[arrayb[1]] = current_interp[arrayb[2]]
	current_interp[arrayb[2]] = temp
	temp = running_duration[arrayb[1]]
	running_duration[arrayb[1]] = running_duration[arrayb[2]]
	running_duration[arrayb[2]] = temp
	}

################################################################
##			Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine and determines if any of the
# input spines contain a kern or recip interpretation.
#
function store_new_interps(      j)
	{
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if ($j ~ /^\*\*/) current_interp[j] = $j
		}
	kern_recip = FALSE
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if (current_interp[j] == "**kern" || current_interp[j] == "**recip")
			{
			kern_recip = TRUE
			break
			}
		}
	if (!kern_recip)
		{
		spine_path_record = FALSE
		FIRST_DATA_RECORD = no_interpretation_yet = TRUE
		current_no_of_spines = shortest_duration = 0
		counter = time_base
		for (i in running_durations) delete running_durations[i]
		}
	}
