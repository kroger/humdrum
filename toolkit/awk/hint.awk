######################################################################
##                               HINT.AWK
# Programmed by: Tim Racinsky        Date: May, 1994
# Copyright (c) 1994 David Huron
#
# Modifications:
#   Date:      Programmer:             Description:
# June 7/94    Tim Racinsky      Modified parse_command to work with getopts
# July 14/95   David Huron       Added -v option
#
# This program calculates harmonic intervals between concurrent pitches
#
# Main Functions used:
#	parse_command()		process_local()		process_interpretation()
#	store_new_interps()		store_indicators()		process_indicators()
#	ins_array_pos()		del_array_pos()		exchange_spines()
#	process_data()			process_kern()			process_pitch()
#	process_Tonh()			process_solfg()		sort_array()
#	do_no_option()			do_loption()			do_aoption()
#	do_hint()				get_diatonic()			chromatic()
#
#	VARIABLES:
#
#  -stderr:  holds destination of error messages
#  -current_no_of_spines:  holds current number of active spines
#  -no_interpretation_yet:  flag to indicate if first exclusive has been seen
#  -spine_path_record:  Regular expression to match spine-path record
#  -null_interp:  Regular expression to match null interpretation record
#  -deviation:  Regular expression to match equally tempered deviations
#  -input_interps:  Regular expression to match proper input interpretations
#  -kern_pitch:  Regular expression to match correct kern pitch
#  -pitch_pitch:  Regular expression to match correct **pitch pitch
#  -Tonh_pitch:  Regular expression to match correct Tonh pitch
#  -solfg_pitch:  Regular expression to match correct solfg pitch
#  -octave_class:  Regular expression to match an octave class designation
#  -options:  Holds options specified by the user
#  -skip_reg:  Holds skip regular expression for -s option
#  -rests:  Flag to indicate if only rests have been processed
#  -token_count:  Counts number of tokens to be processed for this record
#
#	ARRAYS:
#
#  -current_interps:  holds exclusive interpretation for each active spine
#  -path_indicator:  holds spine-path indicators for each active spine
#  -kern_array:  holds semit values for corresponding kern pitches
#  -kern_to_pitch:  holds **pitch representation for kern pitches
#  -pitch_array:  holds semit values for corresponding pitch pitches
#  -pitch_comp:  holds order of pitches in an octave for **pitch pitches
#  -Tonh_array:  holds semit values for corresponding Tonh pitches
#  -Tonh_to_pitch:  holds **pitch representation for Tonh pitches
#  -solfg_array:  holds semit values for corresponding solfg pitches
#  -solfg_to_pitch:  holds **pitch representation for solfg pitches
#  -perfect:  holds number of semitones for "perfect" intervals
#  -major:  holds number of semitones for "major" intervals
#  -tokens:  holds the notes to be processed for current record
#  -sorted_array:  holds sorted notes for current record
#
#
BEGIN {
	USAGE="\nUSAGE: hint -h                (Help Screen)\n       hint "\
		 "[-a|-l|-v] [-cdu] [-s regexp] [file ...]\n"
	#
	# Set global variables and expressions
	#
	FS = OFS = "\t"
	TRUE = 1 ; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	spine_path_record = "^\\*(\\+|v|\\^|x|-)?(\t\\*(\\+|v|\\^|x|-)?)*$"
	exclusive_record = "^\\*(\\*([^\t])+)?(\t\\*(\\*([^\t])+)?)*$"
	null_interp = "^\\*(\t\\*)*$"
	spine_terminate = "^\\*-(\t\\*-)*$"
	deviation = "[+-][1-9][0-9]*"
	input_interps = "^(\\*\\*kern|\\*\\*pitch|\\*\\*Tonh|\\*\\*solfg)$"
	kern_pitch = "a+|b+|c+|d+|e+|f+|g+|A+|B+|C+|D+|E+|F+|G+"
	pitch_pitch = "[ABCDEFG]"
	Tonh_pitch = "Es|As|[ABCDEFGHS]"
	solfg_pitch = "do|re|mi|fa|sol|la|si"
	octave_class = "[^+-][0-9]"
	options = ""
	break_reg = ""
	skip_reg = ""
	no_interpretation_yet = TRUE
	current_no_of_spines = cur_pos = 0
	#
	kern_array["a"] = 9  ; kern_array["b"] = 11 ; kern_array["c"] = 0
	kern_array["d"] = 2  ; kern_array["e"] = 4  ; kern_array["f"] = 5
	kern_array["g"] = 7  ; kern_array["A"] = -3 ; kern_array["B"] = -1
	kern_array["C"] = -12; kern_array["D"] = -10; kern_array["E"] = -8
	kern_array["F"] = -7 ; kern_array["G"] = -5
	#
	kern_to_pitch["a"] = kern_to_pitch["A"] = "A"
	kern_to_pitch["b"] = kern_to_pitch["B"] = "B"
	kern_to_pitch["c"] = kern_to_pitch["C"] = "C"
	kern_to_pitch["d"] = kern_to_pitch["D"] = "D"
	kern_to_pitch["e"] = kern_to_pitch["E"] = "E"
	kern_to_pitch["f"] = kern_to_pitch["F"] = "F"
	kern_to_pitch["g"] = kern_to_pitch["G"] = "G"
	#
	pitch_array["A"] = 9; pitch_array["B"] = 11; pitch_array["C"] = 0
	pitch_array["D"] = 2; pitch_array["E"] = 4 ; pitch_array["F"] = 5
	pitch_array["G"] = 7;
	#
	pitch_comp["C"] = 0 ; pitch_comp["D"] = 1 ; pitch_comp["E"] = 2
	pitch_comp["F"] = 3 ; pitch_comp["G"] = 4 ; pitch_comp["A"] = 5
	pitch_comp["B"] = 6
	#
	Tonh_array["As"] = 8; Tonh_array["A"] = 9; Tonh_array["B"] = 10
	Tonh_array["H"] = 11; Tonh_array["C"] = 0; Tonh_array["D"] = 2
	Tonh_array["Es"] = 3; Tonh_array["S"] = 3; Tonh_array["E"] = 4
	Tonh_array["F"] = 5 ; Tonh_array["G"] = 7
	#
	Tonh_to_pitch["As"] = "A"; Tonh_to_pitch["A"] = "A"
	Tonh_to_pitch["B"] = "B" ; Tonh_to_pitch["H"] = "B"
	Tonh_to_pitch["C"] = "C"; Tonh_to_pitch["D"] = "D"
	Tonh_to_pitch["Es"] = "E"; Tonh_to_pitch["S"] = "E"
	Tonh_to_pitch["E"] = "E"; Tonh_to_pitch["F"] = "F"
	Tonh_to_pitch["G"] = "G"
	#
	solfg_array["la"] = 9; solfg_array["si"] = 11; solfg_array["do"] = 0;
	solfg_array["re"] = 2; solfg_array["mi"] = 4 ; solfg_array["fa"] = 5;
	solfg_array["sol"] = 7;
	#
	solfg_to_pitch["do"] = "C"; solfg_to_pitch["re"] = "D"
	solfg_to_pitch["mi"] = "E"; solfg_to_pitch["fa"] = "F"
	solfg_to_pitch["sol"] = "G"; solfg_to_pitch["la"] = "A"
	solfg_to_pitch["si"] = "B";
	#
	perfect[1] = 0; perfect[4] = 5; perfect[5] = 7
	#
	major[2] = 2; major[3] = 4; major[6] = 9; major[7] = 11
	#
	# Determine user's specified options
	#
	parse_command()
	}
{
#
# For each new file, set the variables
#
if (FNR == 1)
	{
	current_no_of_spines = 0
	no_interpretation_yet = TRUE
	}
#
# Print all comments.
#
if ($0 ~ /^!!/) print $0
#
# Print local comment if all same (otherwise print null local comment)
#
else if ($0 ~ /^!/) process_local()
#
# Process interpretations.
#
else if ($0 ~ /^\*/) process_interpretation()
#
# Process current data record
#
else process_data()
}	

##################################################################
##			FUNCTIONS USED IN HINT.AWK
##################################################################

##################################################################
##			Function Parse_command
#	This function checks that the input passed from hint.ksh
# contains a list of valid options and assigns it to the variable
# options if that is the case.
#
function parse_command()
	{
	#
	# Ensure that no more than one of -a, -l, and -v options is specified
	#
	if (ARGV[2] ~ /a/ && ARGV[2] ~ /l/)
		{
		print "hint: ERROR: Cannot specify both -a and -l options." > stderr
		print USAGE > stderr
		exit
		}
	if (ARGV[2] ~ /a/ && ARGV[2] ~ /v/)
		{
		print "hint: ERROR: Cannot specify both -a and -v options." > stderr
		print USAGE > stderr
		exit
		}
	if (ARGV[2] ~ /l/ && ARGV[2] ~ /v/)
		{
		print "hint: ERROR: Cannot specify both -l and -v options." > stderr
		print USAGE > stderr
		exit
		}
	#
	# Otherwise, set the options variable if not null
	#
	else
		{
		if (ARGV[2] != "null") options = ARGV[2]
		if (ARGV[3] != "null")
			{
			skip_reg = ARGV[3]
			options = options "s"
			}
		ARGV[1] = ARGV[2] = ARGV[3] = ""
		}
	}

##############################################################################
##				Function process_local
#	This function processes local comment records
#
function process_local(    i,found,comment)
	{
	found = FALSE
	comment = "!"
	#
	# Print the local comment if local comment is the same for all spines to be
	# processed -- otherwise print a null local comment
	#
	for (i = 1; i <= current_no_of_spines; i += 1)
		{
		if (current_interp[i] ~ input_interps)
			{
			if (!found) { found = TRUE ; comment = $i }
			else if ($i != comment) { comment = "!" ; break }
			}
		}
	print comment
	}

##############################################################################
##				Function process_interpretation
#	This function processes interpretation records according to their type
#
function process_interpretation(    i,found,tandem)
	{
	#
	# If this is an exclusive interpretation record
	#
	if ($0 ~ exclusive_record && $0 !~ null_interp)
		{
		#
		# If this is the first interpretation, set up the arrays and variables
		#
		if (no_interpretation_yet)
			{
			print "**hint"
			if (options ~ /a/) print "*all"
			current_no_of_spines = NF
			no_interpretation_yet = FALSE
			store_new_interps()
			}
		#
		# Otherwise, simply print a "*" and store any interpretations
		#
		else
			{
			print "*"
			store_new_interps()
			}
		}
	#
	# If it's a spine-path record, store and process the indicators
	#
	else if ($0 ~ spine_path_record && $0 !~ null_interp)
		{
		if ($0 ~ spine_terminate) print "*-"
		else print "*"
		store_indicators()
		process_indicators()
		if (current_no_of_spines == 0) no_interpretation_yet = TRUE
		}
	#
	# Otherwise, process the tandem interpretations like local comments
	#
	else
		{
		found = FALSE
		tandem = "*"
		for (i = 1; i <= current_no_of_spines; i += 1)
			{
			if (current_interp[i] ~ input_interps)
				{
				if (!found) { found = TRUE ; tandem = $i }
				else if ($i != tandem) { tandem = "*" ; break }
				}
			}
		print tandem
		}
	}

################################################################
##				Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
#
function store_new_interps(      j)
	{
	#
	# Store the exclusive interpretations
	#
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		if ($j ~ /^\*\*/) current_interp[j] = $j
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
	for (i = 1; i <= current_no_of_spines; i += 1)
		{
		if ($i == "*") path_indicator[i] = "*"
		else path_indicator[i] = substr($i,2,1)
		}
	}

##################################################################
##			Function Process_indicators
#	This function takes the spine-path indicators that were stored
# in the array 'path_indicator' in the function 'store_indicators'
# and manipulates the arrays 'path_indicator' and 'current_interp'.
#
function process_indicators(    i)
	{
	i = 1
	while (i <= current_no_of_spines)
		{
		if (path_indicator[i] == "^")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = current_interp[i]
			}
		if (path_indicator[i] == "-")
			{
			del_array_pos(i)
			i -= 1
			}
		if (path_indicator[i] == "x")
			{
			exchange_spines()
			}
		if (path_indicator[i] == "v")
			{
			del_array_pos(i+1)
			if (path_indicator[i+1] != "v") path_indicator[i] = "*"
			else i -= 1
			}
		if (path_indicator[i] == "+")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = ""
			}
		i += 1
		}
	}

############################################################
##			Function Ins_array_pos
#	This function inserts new positions in the arrays 'path_indicator'
# and 'current_interp' and copies elements so that everything is preserved
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j -= 1)
		{
		path_indicator[j] = path_indicator[j-1]
		current_interp[j] = current_interp[j-1]
		}
	path_indicator[current_element-1] = "*"
	current_no_of_spines += 1
	}

##############################################################
##			Function Del_array_pos
#	Performs the opposite of function 'ins_array_pos'.
#
function del_array_pos(current_element,     j)
	{
	for (j = current_element; j < current_no_of_spines; j += 1)
		{
		path_indicator[j] = path_indicator[j+1]
		current_interp[j] = current_interp[j+1]
		}
	delete path_indicator[j]
	delete current_interp[j]
	current_no_of_spines -= 1
	} 

###################################################################
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in current_interp.
#
function exchange_spines(    j,count,arraya,temp)
	{
	count = 1
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		if (path_indicator[j] == "x")
			{
			arraya[count] = j
			count += 1
			path_indicator[j] = "*"
			}
		}
	temp = current_interp[arraya[1]]
	current_interp[arraya[1]] = current_interp[arraya[2]]
	current_interp[arraya[2]] = temp
	}

####################################################################
##				Function Process_data
#	This function processes the data records according to the specified options
#
function process_data(   i,found,barline,processed)
	{
	rests = TRUE
	processed = FALSE
	token_count = 0
	#
	# Skip this record if options are -s and this record matches
	#
	if (options ~ /s/ && $0 ~ skip_reg)
		{
		print "."
		return
		}
	#
	# Process barlines like local comments and tandem interpretations
	#
	if ($0 ~ /\=/)
		{
		found = FALSE
		for (i = 1; i <= current_no_of_spines; i += 1)
			{
			if (current_interp[i] ~ input_interps && $i ~ /\=/)
				{
				if (!found) { found = TRUE ; barline = $i }
				else if ($i != barline) { barline = "=" ; break }
				}
			}
		if (found) { print barline ; return }
		}
	#
	# Loop through each of the currently active spines and process the
	# data tokens that are in each spine.
	#
	for (i = 1; i <= current_no_of_spines; i += 1)
		{
		#
		# If the current interpretation is a valid input interpretation
		# and is not a null token, then process the data token
		#
		if (current_interp[i] ~ input_interps)
			{
			#
			# Process the spines according to their interpretation
			#
			processed = TRUE
			if ($i ~ /^\.$/) rests = FALSE
			else
				{
				if (current_interp[i] == "**kern") process_kern($i)
				else if (current_interp[i] == "**pitch") process_pitch($i)
				else if (current_interp[i] == "**Tonh") process_Tonh($i)
				else if (current_interp[i] == "**solfg") process_solfg($i)
				}
			}
		}
	#
	# If no tokens were found to be processed
	#
	if (token_count == 0)
		{
		#
		# If all tokens were rests and there was at least one spine processed
		# then print a rest, otherwise print a null data token
		#
		if (rests && processed) print "r"
		else print "."
		}
	#
	# If only one token is present, print a hyphen
	#
	else if (token_count == 1) print "-"
	#
	# Otherwise, sort the array of tokens and processed according to
	# the specified options
	#
	else
		{
		if (options ~ /v/) do_voption()
		else
			{
			sort_array()
			if (options ~ /l/) do_loption()
			else if (options ~ /a/) do_aoption()
			else do_no_option()
			}
		}
	}

####################################################################
##				Function Process_kern
#	This function processes kern notes by determining the pitch (in 
# **pitch format), the octave class, and the semits value.  These are stored
# as a single element in the array 'tokens'.
#
function process_kern(token  ,j,arraya,split_num,found,semits,pitch,octave)
	{
	split_num = split(token,arraya," ")
	#
	# Loop through the multiple stop
	#
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Process all non-rests
		#
		if (arraya[j] !~ /r/)
			{
			rests = FALSE
			found = TRUE
			semits = 0
			#
			# First find the letter name of the note
			#
			if (match(arraya[j],kern_pitch))
				{
				pitch = substr(arraya[j],RSTART,1)
				if (pitch ~ /[abcdefg]/)
					semits = kern_array[pitch]+(12*(RLENGTH-1))
				else
					semits = kern_array[pitch]-(12*(RLENGTH-1))
				if (pitch ~ /[ABCDEFG]/) octave = RLENGTH * -1
				else octave = RLENGTH - 1
				pitch = kern_to_pitch[pitch]
				}
			else found = FALSE
			#
			# If a letter name was found, continue to process
			#
			if (found)
				{
				if (match(arraya[j],/-+/)) semits -=  RLENGTH
				else if (match(arraya[j],/#+/)) semits += RLENGTH
				tokens[++token_count] = pitch "/" octave "/" semits
				}
			}
		}
	}

#########################################################################
##				Function Process_pitch
#	This function processes pitch notes by determining the pitch (in 
# **pitch format), the octave class, and the semits value.  These are stored
# as a single element in the array 'tokens'.
#
function process_pitch(token  ,j,arrayb,split_num,found,semits,pitch,octave)
	{
	split_num = split(token,arrayb," ")
	#
	# Loop through the multiple stop
	#
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Process all non-rest tokens
		#
		if (arrayb[j] !~ /r/)
			{
			rests = FALSE
			found = TRUE
			semits = 0
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arrayb[j],pitch_pitch))
				{
				pitch = substr(arrayb[j],RSTART,1)
				semits = pitch_array[pitch]
				}
			else found = FALSE
			#
			# If there was a letter name, continue to process
			#
			if (found)
				{
				#
				# Determine if there are any flats or sharps
				#
				if (match(arrayb[j],/b+/)) semits -= RLENGTH
				else
					{
					if (match(arrayb[j],/x+/)) semits += 2 * RLENGTH
					if (match(arrayb[j],/#+/)) semits += RLENGTH
					}
				#
				# Determine which octave the note is in
				#
				if (match(arrayb[j],octave_class)) 
					{
					octave = substr(arrayb[j],RSTART+1,1) - 4
					semits += (12*octave)
					}
				else octave = 0
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayb[j],deviation))
					semits += (substr(arrayb[j],RSTART,RLENGTH) * 0.01)
				#
				# Store values in tokens array
				#
				tokens[++token_count] = pitch "/" octave "/" semits
				}
			}
		}
	}
	
###########################################################################
##					Function Process_Tonh
#	This function processes Tonh notes by determining the pitch (in 
# **pitch format), the octave class, and the semits value.  These are stored
# as a single element in the array 'tokens'.
#
function process_Tonh(token  ,j,arrayc,split_num,found,semits,pitch,octave)
	{
	split_num = split(token,arrayc," ")
	#
	# Loop through each of the multiple stops
	#
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Process all non-rest tokens
		#
		if (arrayc[j] !~ /r/)
			{
			rests = FALSE
			found = TRUE
			semits = 0
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arrayc[j],Tonh_pitch))
				{
				pitch = substr(arrayc[j],RSTART,RLENGTH)
				semits = Tonh_array[pitch]
				pitch = Tonh_to_pitch[pitch]
				}
			else found = FALSE
			#
			# If a letter name was found, continue to process
			#
			if (found)
				{
				#
				# Determine the number of flats and sharps
				#
				if (match(arrayc[j],/(es)+/)) semits -= RLENGTH/2
				else if (match(arrayc[j],/(is)+/)) semits += RLENGTH/2
				#
				# Determine which octave the note is in
				#
				if (match(arrayc[j],octave_class)) 
					{
					octave = substr(arrayc[j],RSTART+1,1) - 4
					semits += (12*octave)
					}
				else octave = 0
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayc[j],deviation))
					semits += (substr(arrayc[j],RSTART,RLENGTH) * 0.01)
				#
				# Store values in tokens array
				#
				tokens[++token_count] = pitch "/" octave "/" semits
				}
			}
		}
	}

###########################################################################
##					Function Process_solfg
#	This function processes solfg notes by determining the pitch (in 
# **pitch format), the octave class, and the semits value.  These are stored
# as a single element in the array 'tokens'.
#
function process_solfg(token  ,j,arrayd,split_num,found,semits,pitch,octave)
	{
	split_num = split(token,arrayd," ")
	#
	# Loop through each of the multiple stops
	#
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Process all non-rest tokens
		#
		if (arrayd[j] ~ /re/ || arrayd[j] !~ /r/)
			{
			rests = FALSE
			found = TRUE
			semits = 0
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arrayd[j],solfg_pitch))
				{
				pitch = substr(arrayd[j],RSTART,RLENGTH)
				semits = solfg_array[pitch]
				pitch = solfg_to_pitch[pitch]
				}
			else found = FALSE
			#
			# If a letter name was found, continue to process
			#
			if (found)
				{
				#
				# Determine if there are any flats or sharps
				#
				if (match(arrayd[j],/_b+/)) semits -= (RLENGTH - 1)
				else if (match(arrayd[j],/_d+/)) semits += (RLENGTH - 1)
				#
				# Determine which octave the note is in
				#
				if (match(arrayd[j],octave_class)) 
					{
					octave = substr(arrayd[j],RSTART+1,1) - 4
					semits += (12*octave)
					}
				else octave = 0
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayd[j],deviation))
					semits += (substr(arrayd[j],RSTART,RLENGTH) * 0.01)
				#
				# Store values in tokens array
				#
				tokens[++token_count] = pitch "/" octave "/" semits
				}
			}
		}
	}

#############################################################################
##				Function sort_array
#	This function sorts the tokens array and places the result in the
# array sorted_array
#
function sort_array(    i,split_array,p_array,o_array,s_array,p_min,o_min,\
											    	    s_min,min_index)
	{
	#
	# Split apart the information into 3 different arrays
	#
	for (i = 1; i <= token_count; i += 1)
		{
		split(tokens[i],split_array,"/")
		p_array[i] = split_array[1]
		o_array[i] = split_array[2]
		s_array[i] = split_array[3]
		}
	#
	# Loop through the original array
	#
	for (i = 1; i <= token_count; i += 1)
		{
		o_min = ""
		#
		# Find the smallest element in the arrays where "smallest" is defined
		# as the 1) lowest octave, 2) lowest pitch, 3) lowest semits value
		#
		for (j in o_array)
			{
			if (o_min == "" || o_array[j] < o_min)
				{
				o_min = o_array[j]
				p_min = pitch_comp[p_array[j]]
				s_min = s_array[j]
				min_index = j
				}
			else if (o_array[j] == o_min)
				{
				if (pitch_comp[p_array[j]] < p_min || \
				    (pitch_comp[p_array[j]] == p_min && s_array[j] < s_min))
					{
					o_min = o_array[j]
					p_min = pitch_comp[p_array[j]]
					s_min = s_array[j]
					min_index = j
					}
				}
			}
		#
		# Store the next smallest value in sorted_array and delete from
		# the other arrays
		#
		sorted_array[i] = tokens[min_index]
		delete o_array[min_index]
		}
	}

###############################################################################
##					Function do_loption
#	This function creates the output token by finding the intervals with
# respect to the lowest pitch.
#
function do_loption(    i,output_token,return_token)
	{
	output_token = ""
	for (i = 2; i <= token_count; i += 1)
		{
		return_token = do_hint(sorted_array[1],sorted_array[i])
		if (return_token != "") output_token = output_token return_token " "
		}
	if (output_token == "") print "-"
	else print substr(output_token,1,length(output_token)-1)
	}

###############################################################################
##					Function do_aoption
#	This function creates the output token by finding all permutations 
# of possible intervals.
#
function do_aoption(    i,j,output_token,return_token)
	{
	output_token = ""
	for (i = 1; i <= token_count - 1; i += 1)
		{
		for (j = i + 1; j <= token_count; j += 1)
			{
			return_token = do_hint(sorted_array[i],sorted_array[j])
			if (return_token != "")
				output_token = output_token return_token " "
			}
		}
	if (output_token == "") print "-"
	else print substr(output_token,1,length(output_token)-1)
	}

###############################################################################
##					Function do_voption
#	This function creates the output token according to the -v option.
#
function do_voption(    i,output_token,return_token,lowest_pit,pit_name_i,pit_name_i1,oct_num_i,oct_num_i1,sem_num_i,sem_num_i1,diaton_num_i,diaton_num_i1)
	{
	output_token = ""
	for (i = 1; i <= token_count - 1; i += 1)
		{
		#
		# Split apart the pitch-name, octave, and semitone information
		# for the two successive tokens

		split(tokens[i],split_array,"/")
		pit_name_i = split_array[1]
		oct_num_i  = split_array[2]
		sem_num_i  = split_array[3]

		split(tokens[i+1],split_array,"/")
		pit_name_i1 = split_array[1]
		oct_num_i1  = split_array[2]
		sem_num_i1  = split_array[3]

		diaton_num_i  = pitch_array[pit_name_i]
		diaton_num_i1 = pitch_array[pit_name_i1]

		# Find the lower of the two pitches where "lower" is defined as
		# the 1) lowest octave, 2) lowest pitch, 3) lowest semits value

		if (oct_num_i == oct_num_i1)
			{
			if (diaton_num_i == diaton_num_i1)
				{
				if (sem_num_i < sem_num_i1) lowest_pit=i
				else if (sem_num_i > sem_num_i1) lowest_pit=i+1
				else lowest_pit="unison"
				}
			else if (diaton_num_i < diaton_num_i1) lowest_pit=i
			else lowest_pit=i+1
			}
		else if (oct_num_i < oct_num_i1) lowest_pit=i
		else lowest_pit=i+1

		if (lowest_pit=="unison") return_token = do_hint(tokens[i],tokens[i+1])
		else if (lowest_pit==i) return_token = "+" do_hint(tokens[i],tokens[i+1])
		else return_token = "-" do_hint(tokens[i+1],tokens[i])
		if (return_token != "") output_token = output_token return_token " "
		}
	if (output_token == "") print "-"
	else print substr(output_token,1,length(output_token)-1)
	}

###############################################################################
##					Function do_no_option
#	This function creates the output token by finding the intervals of
# successive pitches.
#
function do_no_option(    i,output_token,return_token)
	{
	output_token = ""
	for (i = 1; i <= token_count - 1; i += 1)
		{
		return_token = do_hint(sorted_array[i],sorted_array[i+1])
		if (return_token != "") output_token = output_token return_token " "
		}
	if (output_token == "") print "-"
	else print substr(output_token,1,length(output_token)-1)
	}

##########################################################################
##			 	Function do_hint
#	This function is called from the 'option' functions and is used to find
# the interval of two pitches.
#
function do_hint(first,second      ,dia_info,hint_array)
	{
	#
	# Get the diatonic interval according to the input interpretation
	#
	get_diatonic(first,second,hint_array)
	#
	# Suppress printing of unisons if present and -u option specified
	#
	if (options ~ /u/ && hint_array[1] == 1) return ""
	#
	# If -d is specified, don't determine the interval quality
	#
	else if (options ~ /d/) return hint_array[1]
	#
	# Otherwise, determine the interval quality
	#
	else return chromatic(hint_array)
	}

##########################################################################
##				Function get_diatonic
#	This function determines the diatonic interval of two pitches
#
function get_diatonic(first,second,hint_array ,note1,note2,diff_semits,diatonic)
	{
	diatonic = 1
	split(first,note1,"/")
	split(second,note2,"/")
	#
	# If the first note is in a lower octave, raise it
	#
	if (note1[2] < note2[2])
		{
		while (note1[2] < note2[2])
			{
			note1[3] += 12
			diatonic += 7
			note1[2] += 1
			}
		#
		# Determine the diatonic interval and semits difference depending
		# on the relative positions of the notes
		#
		if (pitch_comp[note1[1]] < pitch_comp[note2[1]])
			{
			diatonic += (pitch_comp[note2[1]] - pitch_comp[note1[1]])
			diff_semits = note2[3] - note1[3]
			}
		else if (pitch_comp[note2[1]] < pitch_comp[note1[1]])
			{
			diatonic -= (pitch_comp[note1[1]] - pitch_comp[note2[1]])
			diff_semits = note2[3] - (note1[3] - 12)
			}
		else
			{
			diatonic -= (pitch_comp[note1[1]] - pitch_comp[note2[1]])
			diff_semits = note2[3] - note1[3]
			}
		}
	#
	# If the notes are in the same octave, then compare them
	#
	else if (pitch_comp[note1[1]] < pitch_comp[note2[1]])
		{
		diatonic += (pitch_comp[note2[1]] - pitch_comp[note1[1]])
		diff_semits = note2[3] - note1[3]
		}
	else
		{
		diff_semits = note2[3] - note1[3]
		}
	#
	# If -c was specified, reduce the interval to a non-compound form
	#
	if (options ~ /c/) while (diatonic > 7) diatonic -= 7
	#
	# Round the semits difference to the nearest whole semitone
	#
	if (diff_semits > 0) diff_semits = int(diff_semits + .5)
	else diff_semits = int(diff_semits - .5)
	#
	# Return the resulting token
	#
	hint_array[1] = diatonic
	hint_array[2] = diff_semits
	}

##############################################################################
##					Function chromatic
#	This function determines the quality of the interval based on the diatonic
# interval and the semits of the two tokens
#
function chromatic(hint_array    ,temp_diatonic,chromat)
	{
	chromat = ""
	temp_diatonic = hint_array[1]
	while (temp_diatonic > 7) temp_diatonic -= 7
	#
	# If this is a 'perfect' interval, determine the correct interval quality
	#
	if (temp_diatonic in perfect)
		{
		if (perfect[temp_diatonic] == hint_array[2])
			chromat = "P"	
		else if (perfect[temp_diatonic] > hint_array[2])
			{
			while (hint_array[2] < perfect[temp_diatonic])
				{
				chromat = chromat "d"
				hint_array[2] += 1
				}
			}
		else
			{
			while (hint_array[2] > perfect[temp_diatonic])
				{
				chromat = chromat "A"
				hint_array[2] -= 1
				}
			}
		}
	#
	# If this is a 'major' interval, determine the correct interval quality
	#
	else
		{
		if (major[temp_diatonic] == hint_array[2])
			chromat = "M"
		else if (major[temp_diatonic] == hint_array[2] + 1)
			chromat = "m"
		else if (major[temp_diatonic] > hint_array[2] + 1)
			{
			while (hint_array[2] + 1 < major[temp_diatonic])
				{
				chromat = chromat "d"
				hint_array[2] += 1
				}
			}
		else
			{
			while (hint_array[2] > major[temp_diatonic])
				{
				chromat = chromat "A"
				hint_array[2] -= 1
				}
			}
		}
	return hint_array[3] chromat hint_array[1]
	}
