##########################################################################
# 						CENSUS.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# June 7/94	Tim Racinsky	Modified parse_command to work with getopts
# June 22/94	Tim Racinsky	Changed name from snoop to census
#
#
# 	This program determines various gross properties concerning humdrum files.
#
#	OPTIONS:
#	 -k : prints out information concerning kern spines
#
#	Main Functions Used:
#	Parse_command()		Store_new_interps()		Store_indicators()
#	Process_indicators()	Ins_array_pos()		Del_array_pos()
#	Exchange_spines()		Process_data()			Check_pitch()
#	Check_duration()
#
#	VARIABLES:
#
#  -options:  holds options specified by the user.
#  -no_interpretation_yet:  flag to indicate when the first exclusive
#   interpretation occurs.
#  -error:  flag to indicate if error occurs so that END block can be exited.
#  -current_no_of_spines:  holds current number of active spines.
#  -comments:  holds number of comments in input.
#  -interpretations:  holds number of interpretations in input.
#  -data_records:  holds number of data records in input.
#  -data_tokens:  holds number of data tokens in input.
#  -null_tokens:  holds number of null tokens in input.
#  -multiple_stops:  holds number of multiple-stopped data tokens in input.
#  -notes:  holds number of kern notes in the input.
#  -untied_notes:  same as above but tied notes count as one.
#  -rests:  holds number of kern rests in the input.
#  -max_voices:  holds maximum number of concurrently sounding kern
#   voices in the input.
#  -single_bars:  holds number of kern single barlines in the input.
#  -double_bars:  holds number of kern double barlines in the input.
#  -shortest:  holds fractional duration of current shortest kern note in input.
#  -shortest_name:  holds kern name of above.
#  -longest: holds fractional duration of current longest kern note in input.
#  -longest_name:  holds kern name of above.
#  -highest:  holds semits value of current highest kern note in input.
#  -highest_name:  holds kern name of above.
#  -lowest:  holds semits value of current lowest kern note in input.
#  -lowest_name:  holds kern name of above.
#
#	ARRAYS:
#
#  -previous_count: holds number of voices sounding in previous token for
#   each kern spine.
#  -current_interp:  holds current exclusive interpretation of each spine. 
#  -path_indicator:  holds current path indicator of each spine.
# 
BEGIN {
	USAGE="\nUSAGE: census -h                 (Help Screen)\n       census ["\
		 "-k] [file ...]"
	FS = OFS = "\t"
	TRUE = 1 ; FALSE = 0
	cont_tie_reg = "\\]|_"
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	options = ""
	no_interpretation_yet = TRUE
	error = FALSE
	current_no_of_spines = 0
	#
	# Humdrum variables
	#
	comments = 0
	interpretations = 0
	data_records = 0
	data_tokens = 0
	null_tokens = 0
	multiple_stops = 0
	#
	# kern variables
	#
	notes = 0
	untied_notes = 0
	rests = 0
	max_voices = 0
	single_bars = 0
	double_bars = 0
	shortest = ""
	shortest_name = ""
	longest = ""
	longest_name = ""
	highest = ""
	highest_name = ""
	lowest = ""
	lowest_name = ""
	#
	# Deterimine the options
	#
	parse_command()
	}
{
#
# If the record is a comment, increment the count
#
if ($0 ~ /^!/) comments += 1
#
# If the record is an interpretation, increment the count and process
#
else if ($0 ~ /^\*/)
	{
	interpretations += 1
	#
	# Process the first interpretation record.
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
		for (i = 1; i <= current_no_of_spines ; i++)
			{			
			if ($i ~ /^\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x$/) {
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
			if (current_no_of_spines == 0) no_interpretation_yet = TRUE
			}
		#
		# Otherwise, process the interpretations
		#
		else store_new_interps()
		}
	}
#
# Otherwise, process any data records
#
else process_data()
}
END {
	#
	# If there was an error in entring options, exit the program
	#
	if (error) exit
	print "HUMDRUM DATA\n"
	print "Number of data tokens:     " data_tokens
	print "Number of null tokens:     " null_tokens
	print "Number of multiple-stops:  " multiple_stops
	print "Number of data records:    " data_records
	print "Number of comments:        " comments
	print "Number of interpretations: " interpretations
	print "Number of records:         " NR
	#
	# If the user specified the -k option, print the kern data
	#
	if (options ~ /k/)
		{
		print "\nKERN DATA\n"
		print "Number of note-heads:      " notes
		print "Number of notes:           " untied_notes
		print "Longest note:              " longest_name
		print "Shortest note:             " shortest_name
		print "Highest note:              " highest_name
		print "Lowest note:               " lowest_name
		print "Number of rests:           " rests
		print "Maximum number of voices:  " max_voices
		print "Number of single barlines: " single_bars
		print "Number of double barlines: " double_bars
		}
}

###########################################################################
#				FUNCTIONS USED IN CENSUS.AWK
###########################################################################

##########################################################################
#				Function Parse_command
#
#	This function parses the options sent by the user
#
function parse_command()
	{
	#
	# Set the 'options' variable if not null
	#
	if (ARGV[2] != "null") options = ARGV[2]
	ARGV[1] = ARGV[2] = ""
	}

################################################################
##			Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine and the array previous_count
# is initialized to show the maximum number of voices.
#
function store_new_interps(      j)
	{
	#
	# Loop through each valid spine and store only exclusive interpretations
	# and initialize the previous_count array
	#	
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if ($j ~ /^\*\*/)
			{
			current_interp[j] = $j
			previous_count[j] = 0
			}
		}
	}

##############################################################
##			Function Store_indicators
#	This function allows the spine-path indicators for the 
# current record to be stored in the array 'path_indicator' so
# that they may be used in process_indicators().
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
# and manipulates the arrays 'path_indicator' and 'current_interp'
# according to the contents of the array 'path_indicator'.
#
function process_indicators(    i)
	{
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
			# and 'current_interp' and 'previous_count'
			#
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = current_interp[i]
			previous_count[i+1] = 0
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
			i--
			}
		#
		# If a spine-exchange indicator occurred
		#
		if (path_indicator[i] == "x")
			{
			#
			# Exchange the corresponding array elements
			#
			exchange_spines()
			}
		#
		# If a join-path indicator occurred
		#
		if (path_indicator[i] == "v")
			{
			#
			# Update the previous count appropriately and delete the
			# element from the arrays
			#
			previous_count[i] += previous_count[i+1]
			del_array_pos(i+1)
			#
			# If the next indicator is not a join-path indicator
			# then return to original conditions
			#
			if (path_indicator[i+1] != "v") path_indicator[i] = "*"
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
			# Insert a new position in the arrays.
			#
			ins_array_pos(i+1)
			current_interp[i+1] = ""
			previous_count[i+1] = 0
			}
		i++
		}
	}

############################################################
##			Function Ins_array_pos
#	This function inserts a new position in the arrays 'path_indicator',
# 'current_interp', and 'previous_count' and copies elements so that
# order is preserved
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j--)
		{
		path_indicator[j] = path_indicator[j-1]
		current_interp[j] = current_interp[j-1]	
		previous_count[j] = previous_count[j-1]
		}
	path_indicator[current_element-1] = "*"
	path_indicator[current_element] = "*"
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
		previous_count[j] = previous_count[j+1]
		}
	delete path_indicator[j]
	delete current_interp[j]
	delete previous_count[j]
	current_no_of_spines--
	} 

###################################################################
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in 'current_interp' and 'previous_count'.
#
function exchange_spines(    j,count,temp)
	{
	count = 1
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if (path_indicator[j] == "x")
			{
			exchange[count] = j
			path_indicator[j] = "*"
			count += 1
			}
		}
	temp = current_interp[exchange[1]]
	current_interp[exchange[1]] = current_interp[exchange[2]]
	current_interp[exchange[2]] = temp
	temp = previous_count[exchange[1]]
	previous_count[exchange[1]] = previous_count[exchange[2]]
	previous_count[exchange[2]] = temp
	}

###########################################################################
##					Function Process_data
#
#	This function processes any data records by collecting the appropriate
# information.
function process_data(      i,j,multiple_count,voice_count,single,double\
											,current_voice_count)
	{
	#
	# Increment the counting varibles and initialize the variables for
	# the function.
	#
	data_records += 1
	data_tokens += NF
	single FALSE
	double = FALSE
	voice_count = 0
	#
	# Loop through each token in the current record
	#
	for (j = 1; j <= current_no_of_spines; j++)
		{
		#
		# If the current token is a null token, increment the counter
		#
		if ($j ~ /^[.]$/) null_tokens += 1
		#
		# Split the token into multiple stops and store the number of
		# multiple stops
		#
		multiple_count = split($j,arraya," ")
		#
		# Increment the multiple stop counter if necessary
		#
		if (multiple_count > 1) multiple_stops += 1
		#
		# If the user selected the -k option and the current spine is
		# a kern spine, collect data on it.
		#
		# Craig changed && on following line to ||
		# probably causes secondary bugs, but needed for 
		# piping from PERL programs for some reason...
		if (options ~ /k/ || current_interp[j] == "**kern")
			{
			current_voice_count = 0
			#
			# If the current token is a null token, increment the voice
			# count by the value of the previous token.
			#
			if ($j ~ /^[.]$/)
				{
				voice_count += previous_count[j]
				}
			#
			# If the token is a double barline, increment the double barline
			# count if this is the first barline in this record
			#
			else if ($j ~ /^==/)
				{
				if (!double)
					{
					double_bars += 1
					double = TRUE
					}
				}
			#
			# If the token is a single barline, increment the single barline
			# count if this is the first barline in this record
			#
			else if ($j ~ /^=/)
				{
				if (!single)
					{
					single_bars += 1
					single = TRUE
					}
				}
			#
			# Otherwise, process the kern note token
			#
			else
				{
				#
				# Loop through each part of a multiple stop
				#
				for (i in arraya)
					{
					#
					# If there is a rest, increment the counter
					#
					if (arraya[i] ~ /r/) rests += 1
					#
					# Otherwise, determine its characteristics
					#
					else
						{
						#
						# Increment voice count for this token, voice
						# count for this record, and note count.
						current_voice_count += 1
						notes += 1
						voice_count += 1
						#
						# Increment untied note count if this token is not
						# the beginning or end of a tie.
						#
						if (arraya[i] !~ cont_tie_reg) untied_notes += 1
						#
						# Check the pitch and duration of the note
						#
						check_pitch(arraya[i])
						check_duration(arraya[i])
						}
					}
				#
				# Update the previous count array for this spine
				#
				previous_count[j] = current_voice_count
				}
			}
		}
	#
	# Update the maximum voice count if necessary
	#
	if (voice_count > max_voices) max_voices = voice_count
	}

####################################################################
##				Function Check_pitch
#	This function takes kern input and translates to semits and
# determines if the current note is higher or lower than any previous note.
#
function check_pitch(token,    process,semits,letter_name)
	{
	semits = 0
	process = TRUE
	#
	# First find the letter name of the note and determine it value in semits.
	#
	if (match(token,/a+/)) semits = 9 + (12 * (RLENGTH - 1))
	else if (match(token,/b+/)) semits = 11 + (12 * (RLENGTH - 1))
	else if (match(token,/c+/)) semits = 0 + (12 * (RLENGTH - 1))
	else if (match(token,/d+/)) semits = 2 + (12 * (RLENGTH - 1))
	else if (match(token,/e+/)) semits = 4 + (12 * (RLENGTH - 1))
	else if (match(token,/f+/)) semits = 5 + (12 * (RLENGTH - 1))
	else if (match(token,/g+/)) semits = 7 + (12 * (RLENGTH - 1))
	else if (match(token,/A+/)) semits = -3 - (12 * (RLENGTH - 1))
	else if (match(token,/B+/)) semits = -1 - (12 * (RLENGTH - 1))
	else if (match(token,/C+/)) semits = -12 - (12 * (RLENGTH - 1))
	else if (match(token,/D+/)) semits = -10 - (12 * (RLENGTH - 1))
	else if (match(token,/E+/)) semits = -8 - (12 * (RLENGTH - 1))
	else if (match(token,/F+/)) semits = -7 - (12 * (RLENGTH - 1))
	else if (match(token,/G+/)) semits = -5 - (12 * (RLENGTH - 1))
	else process = FALSE
	#
	# If a note was actually found, continue
	#
	if (process)
		{
		#
		# Store the found note name
		#
		letter_name = letter_name substr(token,RSTART,RLENGTH)
		#
		# If there are flats, update the semits value and letter_name value
		#
		if (match(token,/-+/))
			{
			semits = semits - RLENGTH
			letter_name = letter_name substr(token,RSTART,RLENGTH)
			}
		#
		# If there are sharps, update the semits value and letter_name value
		#
		else if (match(token,/#+/))
			{
			semits = semits + RLENGTH
			letter_name = letter_name substr(token,RSTART,RLENGTH)
			}
		#
		# If this is the first note, update both variables
		#
		if (highest == "")
			{
			highest = lowest = semits+0
			highest_name = lowest_name = letter_name
			}
		#
		# Otherwise, check to see if this note is higher or lower than
		# any other note.
		#
		else
			{
			if (semits > highest)
				{
				highest = semits+0
				highest_name = letter_name
				}
			if (semits < lowest)
				{
				lowest = semits+0
				lowest_name = letter_name
				}
			}
		}
	}

###########################################################################
## 				Function Check_duration
#
# 	This function checks the duration of the given note to see if it is
# the longest or shortest note.
#
function check_duration(token,       dots,proportion,duration)
	{
	dots = 0
	#
	# If there is actually a duration value in the current note, process it
	#
	if (match(token,/(0)|([1-9]+[0-9]*)/))
		{
		#
		# Store the kern duration name
		#
		duration = substr(token,RSTART,RLENGTH)
		#
		# Determine the numerical value of the note
		#
		if (duration != "0") proportion = 1 / duration+0
		else proportion = 2
		#
		# Check for and store any augmentation dots.
		#
		if (match(token,/\./))
			{
			duration = duration substr(token,RSTART,RLENGTH)
			dots = gsub(/\./,".",token)
			}
		half = proportion / 2
		#
		# Compute the value of adding the augmentation dots.
		#
		while (dots > 0)
			{
			proportion = proportion + half
			half = half / 2
			dots--
			}
		#
		# If this is the first duration, store it
		#
		if (shortest == "")
			{
			shortest = longest = proportion+0
			shortest_name = longest_name = duration
			}
		#
		# Otherwise, if this duration is shorter or longer than any 
		# previous duration, store it.
		#
		else
			{
			if (proportion < shortest)
				{
				shortest = proportion+0
				shortest_name = duration
				}
			if (proportion > longest)
				{
				longest = proportion+0
				longest_name = duration
				}
			}
		}
	}
