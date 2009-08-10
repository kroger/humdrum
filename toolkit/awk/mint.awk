######################################################################
##					MINT.AWK
# Programmed by: Tim Racinsky        Date: May, 1994
# Copyright (c) 1994 David Huron
#
# Modifications:
#   Date:      Programmer:       	Description:
# June 7/94	Tim Racinsky	Modified to work with getopts
# April 27/95	David Huron	Added the -e option
# Jan. 2/97	David Huron	Fixed bug in pitch-processing functions
#
#
# Main Functions used:
#	parse_command()		process_interpretation()		store_new_interps()
#	store_indicators()		process_indicators()		ins_array_pos()
#	del_array_pos()		exchange_spines()			join_paths()
#	process_kern()			process_pitch()			process_Tonh()
#	process_solfg()		double_mint()				do_mint()
#	get_diatonic()			chromatic()
#
#	VARIABLES:
#
#  -stderr:  holds destination of error messages
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
#  -break_reg:  Holds break regular expression for -b option
#  -skip_reg:  Holds skip regular expression for -s option
#  -cur_pos:  Holds position of current spine being processed
#
#	ARRAYS:
#
#  -current_interps:  holds exclusive interpretation for each active spine
#  -path_indicator:  holds spine-path indicators for each active spine
#  -previous_value:  holds last token processed for each processed spine
#  -kern_array:  holds semit values for corresponding kern pitches
#  -pitch_array:  holds semit values for corresponding pitch pitches
#  -Tonh_array:  holds semit values for corresponding Tonh pitches
#  -solfg_array:  holds semit values for corresponding solfg pitches
#  -kern_comp:  holds order of pitches in an octave for kern (and pitch) pitches
#  -Tonh_comp:  holds order of pitches in an octave for Tonh pitches
#  -solfg_comp:  holds order of pitches in an octave for solfg pitches
#  -perfect:  holds number of semitones for "perfect" intervals
#  -major:  holds number of semitones for "major" intervals
#
#
BEGIN {
	USAGE="\nUSAGE: mint -h                (Help Screen)\n       mint "\
		 "[-acde] [-b regexp] [-s regexp] [file ...]\n"
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
	null_interp = "^\\*(\t\\*)*$"
	deviation = "[+-][1-9][0-9]*"
	input_interps = "^(\\*\\*kern|\\*\\*pitch|\\*\\*Tonh|\\*\\*solfg)$"
	kern_pitch = "a+|b+|c+|d+|e+|f+|g+|A+|B+|C+|D+|E+|F+|G+"
	pitch_pitch = "[A-G]"
	Tonh_pitch = "Es|As|[A-HS]"
	solfg_pitch = "do|re|mi|fa|sol|la|si"
	octave_class = "[^+-][0-9]"
	options = ""
	break_reg = ""
	skip_reg = ""
	no_interpretation_yet = TRUE
	current_no_of_spines = cur_pos = 0
	#
	kern_array["a"] = 9  ; kern_array["b"] = 11 ; kern_array["c"] = 0;
	kern_array["d"] = 2  ; kern_array["e"] = 4  ; kern_array["f"] = 5;
	kern_array["g"] = 7  ; kern_array["A"] = -3 ; kern_array["B"] = -1;
	kern_array["C"] = -12; kern_array["D"] = -10; kern_array["E"] = -8;
	kern_array["F"] = -7 ; kern_array["G"] = -5 ;
	#
	kern_comp["c"] = kern_comp["C"] = 0 ; kern_comp["d"] = kern_comp["D"] = 1
	kern_comp["e"] = kern_comp["E"] = 2 ; kern_comp["f"] = kern_comp["F"] = 3
	kern_comp["g"] = kern_comp["G"] = 4 ; kern_comp["a"] = kern_comp["A"] = 5
	kern_comp["b"] = kern_comp["B"] = 6
	#
	pitch_array["A"] = 9; pitch_array["B"] = 11; pitch_array["C"] = 0;
	pitch_array["D"] = 2; pitch_array["E"] = 4 ; pitch_array["F"] = 5;
	pitch_array["G"] = 7;
	#
	Tonh_array["As"] = 8; Tonh_array["A"] = 9; Tonh_array["B"] = 10;
	Tonh_array["H"] = 11; Tonh_array["C"] = 0; Tonh_array["D"] = 2;
	Tonh_array["Es"] = 3; Tonh_array["S"] = 3; Tonh_array["E"] = 4;
	Tonh_array["F"] = 5 ; Tonh_array["G"] = 7;
	#
	Tonh_comp["C"] = 0 ; Tonh_comp["D"] = 1
	Tonh_comp["E"] = Tonh_comp["Es"] = Tonh_comp["S"] = 2 ; Tonh_comp["F"] = 3
	Tonh_comp["G"] = 4 ; Tonh_comp["A"] = Tonh_comp["As"] = 5
	Tonh_comp["B"] = Tonh_comp["H"] = 6
	#
	solfg_array["la"] = 9; solfg_array["si"] = 11; solfg_array["do"] = 0;
	solfg_array["re"] = 2; solfg_array["mi"] = 4 ; solfg_array["fa"] = 5;
	solfg_array["sol"] = 7;
	#
	solfg_comp["do"] = 0; solfg_comp["re"] = 1; solfg_comp["mi"] = 2
	solfg_comp["fa"] = 3; solfg_comp["sol"] = 4; solfg_comp["la"] = 5
	solfg_comp["si"] = 6
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
	for (i in previous_value) delete previous_value[i]
	}
#
# Print all comments.
#
if ($0 ~ /^!/) print $0
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
##			FUNCTIONS USED IN MINT.AWK
##################################################################

##################################################################
##			Function Parse_command
#	This function checks that the input passed from mint.ksh
# contains a list of valid options and assigns it to the variable
# options if that is the case.
#
function parse_command()
	{
	if (ARGV[2] != "null") options = ARGV[2]
	if (ARGV[3] != "null")
		{
		break_reg = ARGV[3]
		options = options "b"
		}
	if (ARGV[4] != "null")
		{
		skip_reg = ARGV[4]
		options = options "s"
		}
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ""
	}

##############################################################################
##				Function process_interpretation
#	This function processes interpretation records according to their type
#
function process_interpretation()
	{
	#
	# If this is the first interpretation, set up the arrays and variables
	#
	if (no_interpretation_yet)
		{
		current_no_of_spines = NF
		no_interpretation_yet = FALSE
		store_new_interps()
		}
	#
	# Otherwise check to see if it is a spine_path record
	#
	else
		{
		#
		# If it's a spine-path record, store and process the indicators
		#
		if ($0 ~ spine_path_record && $0 !~ null_interp)
			{
			print $0
			store_indicators()
			process_indicators()
			if (current_no_of_spines == 0) no_interpretation_yet = TRUE
			}
		#
		# Otherwise, store the exclusive interpretations
		#
		else store_new_interps()
		}
	}

################################################################
##				Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
#
function store_new_interps(      j,interp_line)
	{
	interp_line = ""
	#
	# Store the exclusive interpretations and reset array previous_value
	#
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		if ($j ~ /^\*\*/)
			{
			current_interp[j] = $j
			previous_value[j] = ""
			}
		#
		# Output **mint for the processed spines
		#
		if ($j ~ input_interps) interp_line = interp_line "**mint\t"
		else interp_line = interp_line $j "\t"
		}
	print substr(interp_line,1,length(interp_line)-1)
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
# and manipulates the arrays 'path_indicator', 'current_interp',
# and 'previous_value'.
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
			previous_value[i+1] = previous_value[i]
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
			join_paths(i,i+1)
			del_array_pos(i+1)
			if (path_indicator[i+1] != "v") path_indicator[i] = "*"
			else i -= 1
			}
		if (path_indicator[i] == "+")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = ""
			previous_value[i+1] = ""
			}
		i += 1
		}
	}

############################################################
##			Function Ins_array_pos
#	This function inserts new positions in the arrays 'path_indicator',
# 'current_interp',and 'previous_value' and copies elements so that 
# everything is preserved
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j -= 1)
		{
		path_indicator[j] = path_indicator[j-1]
		current_interp[j] = current_interp[j-1]
		previous_value[j] = previous_value[j-1]
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
		previous_value[j] = previous_value[j+1]
		}
	delete path_indicator[j]
	delete current_interp[j]
	delete previous_value[j]
	current_no_of_spines -= 1
	} 

###################################################################
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in current_interp and previous_value.
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
	temp = previous_value[arraya[1]]
	previous_value[arraya[1]] = previous_value[arraya[2]]
	previous_value[arraya[2]] = temp
	}

############################################################
##				Function join_paths
#	This function appends tokens of the joined spines to form a single
# token as a multiple stopped token.
#
function join_paths(i,j)
	{
	if (previous_value[i] == "")
		previous_value[i] = previous_value[j]
	else if (previous_value[j] != "")
		previous_value[i] = previous_value[i] " " previous_value[j]
	}

####################################################################
##				Function Process_data
#	This function takes the given input record and processes the data
# tokens in each spine according to their current exclusive interpretation
# and the value of the data tokens.
#
function process_data(     output_record,current_token)
	{
	output_record = ""
	#
	# Loop through each of the currently active spines and process the data
	# tokens that are in each spine.
	#
	for (cur_pos = 1; cur_pos <= current_no_of_spines; cur_pos += 1)
		{
		#
		# If the current interpretation is a valid input interpretation
		#
		if (current_interp[cur_pos] ~ input_interps)
			{
			current_token = ""
			#
			# Skip null tokens, barlines
			#
			if ($cur_pos ~ /^[.]$/ || $cur_pos ~ /^=/)
				current_token = $cur_pos
			#
			# Skip the matching token if -s is specified
			#
			else if (options ~ /s/ && $cur_pos ~ skip_reg)
				{
				if (options ~ /e/) current_token = $cur_pos
				else current_token = "."
				}
			#
			# Break on this token if -b specified
			#
			else if (options ~ /b/ && $cur_pos ~ break_reg)
				{
				current_token = $cur_pos
				previous_value[cur_pos] = ""
				}
			#
			# Process the pitch using the correct function
			#
			else
				{
				if (current_interp[cur_pos] == "**kern")
					current_token = process_kern($cur_pos)
				else if (current_interp[cur_pos] == "**pitch")
					current_token = process_pitch($cur_pos)
				else if (current_interp[cur_pos] == "**Tonh")
					current_token = process_Tonh($cur_pos)
				else if (current_interp[cur_pos] == "**solfg")
					current_token = process_solfg($cur_pos)
				}
			#
			# Construct a string for the current line
			#
			output_record = output_record current_token "\t"
			}
		#
		# All other spines simply have their data tokens echoed
		#
		else output_record = output_record $cur_pos "\t"
		}
	#
	# After all spines have been processed, print the resulting record
	#
	print substr(output_record,1,length(output_record)-1)
	}

####################################################################
##				Function Process_kern
#	This function takes kern input and translates to semits and passes the
# result to double_mint.
#	In this function, the variable 'current_value' will hold a token consisting
# of possible multiple stops where each multiple stop is in the format
# 'note_token/pitch&octave/semits_value'.  This is passed to double_mint
#
function process_kern(token    ,arraya,split_num,j,semits,note\
			   ,current_value,found,pitch,do_save,return_token)
	{
	#
	# Process multiple stops
	#
	note = ""
	current_value = ""
	return_token = ""
	#
	# If there is no previous value to compare against, we will simply save
	# this token and print it without processing an interval
	#
	if (previous_value[cur_pos] == "") do_save = TRUE
	else do_save = FALSE
	split_num = split(token,arraya," ")
	#
	# Loop through the multiple stop
	#
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If there is a rest, note it in current_value
		#
		if (arraya[j] ~ /r/)
			{
			current_value = current_value "r/r/r "
			if (do_save) return_token = return_token "r "
			}
		else
			{
			found = TRUE
			semits = 0
			#
			# First find the letter name of the note
			#
			if (match(arraya[j],kern_pitch))
				{
				pitch = substr(arraya[j],RSTART,1)
				if (pitch ~ /[a-g]/)
					semits = kern_array[pitch]+(12*(RLENGTH-1))
				else
					semits = kern_array[pitch]-(12*(RLENGTH-1))
				pitch = substr(arraya[j],RSTART,RLENGTH)
				note = pitch
				}
			else found = FALSE
			#
			# If a letter name was found, continue to process
			#
			if (found)
				{
				if (match(arraya[j],/-+/))
					{
					semits -=  RLENGTH
					note = note substr(arraya[j],RSTART,RLENGTH)
					}
				else if (match(arraya[j],/#+/))
					{
					semits += RLENGTH
					note = note substr(arraya[j],RSTART,RLENGTH)
					}
				if (do_save) return_token = return_token "[" note "] "
				current_value = \
						current_value note "/" pitch "/" semits " "
				}
			}
		}
	#
	# If there is something to process, process it
	#
	if (current_value != "")
		{
		if (do_save) previous_value[cur_pos] = current_value
		else return_token = double_mint(current_value)
		}
	#
	# Return at least a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#########################################################################
##				Function Process_pitch
#	This function takes **pitch input and translates to semits and passes the
# result to double_mint.
#	In this function, the variable 'current_value' will hold a token consisting
# of possible multiple stops where each multiple stop is in the format
# 'note_token/pitch&octave/semits_value'.  This is passed to double_mint
#
function process_pitch(token    ,arrayb,split_num,j,semits,note\
			   ,current_value,found,pitch,do_save,return_token)
	{
	#
	# Process multiple stops
	#
	note = ""
	current_value = ""
	return_token = ""
	if (previous_value[cur_pos] == "") do_save = TRUE
	else do_save = FALSE
	split_num = split(token,arrayb," ")
	#
	# Loop through the multiple stop
	#
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If there is a rest, note it in current_value
		#
		if (arrayb[j] ~ /r/)
			{
			current_value = current_value "r/r/r "
			if (do_save) return_token = return_token "r "
			}
		else
			{
			semits = 0
			found = TRUE
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arrayb[j],pitch_pitch))
				{
				pitch = substr(arrayb[j],RSTART,1)
				semits = pitch_array[pitch]
				note = pitch
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
				if (match(arrayb[j],/b+/))
					{
					semits -= RLENGTH
					note = note substr(arrayb[j],RSTART,RLENGTH)
					}
				else
					{
					if (match(arrayb[j],/x+/))
						{
						semits += 2 * RLENGTH
						note = note substr(arrayb[j],RSTART,RLENGTH)
						}
					if (match(arrayb[j],/#+/))
						{
						semits += RLENGTH
						note = note substr(arrayb[j],RSTART,RLENGTH)
						}
					}
				#
				# Determine which octave the note is in
				#
				if (match(arrayb[j],octave_class)) 
					{
					semits += (12*(substr(arrayb[j],RSTART+1,1) + 0)) - 48
					note = note substr(arrayb[j],RSTART+1,RLENGTH-1)
					pitch = pitch "^" substr(arrayb[j],RSTART+1,RLENGTH-1)
					}
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayb[j],deviation))
					{
					semits += (substr(arrayb[j],RSTART,RLENGTH) * 0.01)
					note = note substr(arrayb[j],RSTART,RLENGTH)
					}
				if (do_save) return_token = return_token "[" note "] "
				current_value = \
						current_value note "/" pitch "/" semits " "
				}
			}
		}
	#
	# If there is something to process, then process it
	#
	if (current_value != "")
		{
		if (do_save) previous_value[cur_pos] = current_value
		else return_token = double_mint(current_value)
		}
	#
	# Return at least a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}
	
###########################################################################
##					Function Process_Tonh
#	This function takes Tonh input and translates to semits and passes the
# result to double_mint.
#	In this function, the variable 'current_value' will hold a token consisting
# of possible multiple stops where each multiple stop is in the format
# 'note_token/pitch&octave/semits_value'.  This is passed to double_mint
#
function process_Tonh(token    ,arrayc,split_num,j,semits,note\
			   ,current_value,found,pitch,do_save,return_token)
	{
	#
	# Process multiple stops
	#
	note = ""
	current_value = ""
	return_token = ""
	if (previous_value[cur_pos] == "") do_save = TRUE
	else do_save = FALSE
	split_num = split(token,arrayc," ")
	#
	# Loop through each of the multiple stops
	#
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If there is a rest, note it in current_value
		#
		if (arrayc[j] ~ /r/)
			{
			current_value = current_value "r/r/r "
			if (do_save) return_token = return_token "r "
			}
		else
			{
			semits = 0
			found = TRUE
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arrayc[j],Tonh_pitch))
				{
				pitch = substr(arrayc[j],RSTART,RLENGTH)
				semits = Tonh_array[pitch]
				note = pitch
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
				if (match(arrayc[j],/(es)+/))
					{
					semits -= RLENGTH/2
					note = note substr(arrayc[j],RSTART,RLENGTH)
					}
				else if (match(arrayc[j],/(is)+/))
					{
					semits += RLENGTH/2
					note = note substr(arrayc[j],RSTART,RLENGTH)
					}
				#
				# Determine which octave the note is in
				#
				if (match(arrayc[j],octave_class)) 
					{
					semits += (12*(substr(arrayc[j],RSTART+1,1) + 0)) - 48
					note = note substr(arrayc[j],RSTART+1,RLENGTH-1)
					pitch = pitch "^" substr(arrayc[j],RSTART+1,RLENGTH-1)
					}
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayc[j],deviation))
					{
					semits += (substr(arrayc[j],RSTART,RLENGTH) * 0.01)
					note = note substr(arrayc[j],RSTART,RLENGTH)
					}
				if (do_save) return_token = return_token "[" note "] "
				current_value = \
						current_value note "/" pitch "/" semits " "
				}
			}
		}
	#
	# If there is something to process, then process it
	#
	if (current_value != "")
		{
		if (do_save) previous_value[cur_pos] = current_value
		else return_token = double_mint(current_value)
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

###########################################################################
##					Function Process_solfg
#	This function takes solfg input and translates to semits and passes the
# result to double_mint.
#	In this function, the variable 'current_value' will hold a token consisting
# of possible multiple stops where each multiple stop is in the format
# 'note_token/pitch&octave/semits_value'.  This is passed to double_mint
#
function process_solfg(token    ,arrayd,split_num,j,semits,note\
			   ,current_value,found,pitch,do_save,return_token)
	{
	#
	# Process multiple stops
	#
	note = ""
	current_value = ""
	return_token = ""
	if (previous_value[cur_pos] == "") do_save = TRUE
	else do_save = FALSE
	split_num = split(token,arrayd," ")
	#
	# Loop through each of the multiple stops
	#
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If there is a rest, note it in current_value
		#
		if (arrayd[j] ~ /r/ && arrayd[j] !~ /re/)
			{
			current_value = current_value "r/r/r "
			if (do_save) return_token = return_token "r "
			}
		#
		# Otherwise, process the solfg note
		#
		else
			{
			semits = 0
			found = TRUE
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arrayd[j],solfg_pitch))
				{
				pitch = substr(arrayd[j],RSTART,RLENGTH)
				semits = solfg_array[pitch]
				note = pitch
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
				if (match(arrayd[j],/_b+/))
					{
					semits -= (RLENGTH - 1)
					note = note substr(arrayd[j],RSTART,RLENGTH)
					}
				else if (match(arrayd[j],/_d+/))
					{
					semits += (RLENGTH - 1)
					note = note substr(arrayd[j],RSTART,RLENGTH)
					}
				#
				# Determine which octave the note is in
				#
				if (match(arrayd[j],octave_class)) 
					{
					semits += (12*(substr(arrayd[j],RSTART+1,1) + 0)) - 48
					note = note substr(arrayd[j],RSTART+1,RLENGTH-1)
					pitch = pitch "^" substr(arrayd[j],RSTART+1,RLENGTH-1)
					}
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayd[j],deviation))
					{
					semits += (substr(arrayd[j],RSTART,RLENGTH) * 0.01)
					note = note substr(arrayd[j],RSTART,RLENGTH)
					}
				if (do_save) return_token = return_token "[" note "] "
				current_value = \
						current_value note "/" pitch "/" semits " "
				}
			}
		}
	#
	# If there is something to process, then process it
	#
	if (current_value != "")
		{
		if (do_save) previous_value[cur_pos] = current_value
		else return_token = double_mint(current_value)
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

############################################################################
##					Function double_mint
#	This function determines the melodic interval for the given token
# with respect to the previous token.  Rest tokens must be processed correctly
# as well.  Each time a rest is compared with a previous value, that value must
# appear again in 'previous_value[]' for the next token.
#
function double_mint(current_value   ,previous,previous_max,current\
					,current_max,outside,inside,current_line,next_token)
	{
	temp_previous = ""
	current_line = ""
	current_max = split(current_value,current," ")
	previous_max = split(previous_value[cur_pos],previous," ")
	#
	# If there are an equal number of 'stops' in the tokens, then match
	# them 1-1.
	#
	if (current_max == previous_max)
		{
		for (outside = 1; outside <= current_max; outside++)
			{
			if (current[outside] ~ /r$/)
				temp_previous = temp_previous previous[outside] " "
			else
				temp_previous = temp_previous current[outside] " "
			next_token = do_mint(current,outside,previous,outside)
			current_line = current_line next_token " "
			}
		}
	#
	# If at least one token has only one number, then process that number
	# with the rest from the other token
	#
	else if (current_max == 1 || previous_max == 1)
		{
		for (outside = 1; outside <= current_max; outside++)
			{
			if (current[outside] !~ /r$/)
				temp_previous = temp_previous current[outside] " "
			for (inside = 1; inside <= previous_max; inside++)
				{
				if (current[outside] ~ /r$/)
					temp_previous = temp_previous previous[inside] " "
				next_token = \
						 do_mint(current,outside,previous,inside)
				current_line = current_line next_token " "
				}
			}
		}
	#
	# Otherwise, there are 3 cases to deal with.  All of them start by 
	# processing the first two numbers in each token and finishing with
	# the last two numbers in each token. Middle values are output 
	# within parantheses.
	#
	else
		{
		if (current[1] ~ /r$/)
			temp_previous = temp_previous previous[1] " "
		else
			temp_previous = temp_previous current[1] " "
		current_line = current_line do_mint(current,1,previous,1) " "
		#
		# If the previous token had only 2 elements (Note that the current
		# token must have greater than 2 elements).
		#
		if (previous_max == 2)
			{
			for (outside = 2; outside < current_max; outside++)
				{
				if (current[outside] !~ /r$/)
					temp_previous = temp_previous current[outside] " "
				for (inside = 1; inside <= previous_max; inside++)
					{
					if (current[outside] ~ /r$/)
						temp_previous = temp_previous previous[inside] " "
					next_token = \
						 do_mint(current,outside,previous,inside)
					current_line = current_line "(" next_token ") "
					}
				}
			}
		#
		# If the current token has only 2 elements (Likewise, the previous
		# token has greater than 2 elements).
		#
		else if (current_max == 2)
			{
			for (outside = 2; outside < previous_max; outside++)
				{
				for (inside = 1; inside <= current_max; inside++)
					{
					if (current[inside] ~ /r$/)
						temp_previous=temp_previous previous[outside] " "
					next_token = \
					      do_mint(current,inside,previous,outside)
					current_line = current_line "(" next_token ") "
					}
				}
			}
		#
		# Otherwise, both tokens have greater than 2 elements
		#
		else
			{
			for (outside = 2; outside < current_max; outside++)
				{
				if (current[outside] !~ /r$/)
					temp_previous = temp_previous current[outside] " "
				for (inside = 2; inside < previous_max; inside++)
					{
					if (current[outside] ~ /r$/)
						temp_previous = temp_previous previous[inside] " "
					next_token = \
						 do_mint(current,outside,previous,inside)
					current_line = current_line "(" next_token ") "
					}
				}
			}
		if (current[current_max] ~ /r$/)
			temp_previous = temp_previous previous[previous_max] " "
		else
			temp_previous = temp_previous current[current_max] " "
		current_line = current_line \
			  do_mint(current,current_max,previous,previous_max) " "
		}
	current_line = substr(current_line,1,length(current_line)-1) "\t"
	previous_value[cur_pos] = substr(temp_previous,1,length(temp_previous)-1)
	return current_line
	}

##########################################################################
##			 	Function do_mint
#	This function is called from double_mint() and is used to do the
# actual melodic interval calculations.  The elements in the arrays 'this'
# and 'last' are in the form 'note/pitch/semits'.  The variable 
# dia_info is in the form 'diatonic_interval/semits_difference/direction'.
#
function do_mint(cur,cur_ndx,pre,pre_ndx    ,this,last,dia_info,mint_array)
	{
	split(cur[cur_ndx],this,"/")
	split(pre[pre_ndx],last,"/")
	if (this[3] ~ /r/) return "r"
	if (last[1] == "") return "[" this[1] "]"
	if (last[3] ~ /r/) return "[" this[1] "]"
	#
	# Get the diatonic interval according to the input interpretation
	#
	if (current_interp[cur_pos] == "**kern")
		dia_info = get_diatonic(last[2],this[2],last[3],this[3],kern_comp)
	else if (current_interp[cur_pos] == "**pitch")
		dia_info = get_diatonic(last[2],this[2],last[3],this[3],kern_comp)
	else if (current_interp[cur_pos] == "**Tonh")
		dia_info = get_diatonic(last[2],this[2],last[3],this[3],Tonh_comp)
	else if (current_interp[cur_pos] == "**solfg")
		dia_info = get_diatonic(last[2],this[2],last[3],this[3],solfg_comp)
	split(dia_info,mint_array,"/")
	#
	# If -d is specified, don't determine the interval quality
	#
	if (options ~ /d/)
		{
		if (mint_array[3] ~ /[+-]/) return mint_array[3] mint_array[1]
		else return mint_array[1]
		}
	#
	# Otherwise, determine the interval quality
	#
	else return chromatic(mint_array)
	}

##########################################################################
##				Function get_diatonic
#	This function determines the diatonic interval of two pitches
#
function get_diatonic(last_pitch,this_pitch,last_semits,this_semits,\
					   comp_array  ,sign,old_pitch,new_pitch,diff_semits,\
					   diatonic,pold,pnew,last_oc,this_oc)
	{
	sign = ""
	diatonic = 1
	split(last_pitch,old_pitch,"^")
	split(this_pitch,new_pitch,"^")
	#
	# Determine the ocatve class and pitch letter name depending on whether
	# this is a kern spine or one of the other types
	#
	if (current_interp[cur_pos] == "**kern")
		{
		pold = substr(last_pitch,1,1)
		pnew = substr(this_pitch,1,1)
		if (last_pitch ~ /[A-G]/) last_oc = length(last_pitch) * -1
		else last_oc = length(last_pitch) - 1
		if (this_pitch ~ /[A-G]/) this_oc = length(this_pitch) * -1
		else this_oc = length(this_pitch) - 1
		}
	else
		{
		pold = old_pitch[1]
		pnew = new_pitch[1]
		last_oc = old_pitch[2] - 4
		this_oc = new_pitch[2] - 4
		}
	#
	# If the previous note was in a lower octave, raise it
	#
	if (last_oc < this_oc)
		{
		sign = "+"
		while (last_oc < this_oc)
			{
			last_semits += 12
			diatonic += 7
			last_oc += 1
			}
		#
		# Determine the diatonic interval and semits difference depending
		# on the relative positions of the notes
		#
		if (comp_array[pold] < comp_array[pnew])
			{
			diatonic += (comp_array[pnew] - comp_array[pold])
			diff_semits = this_semits - last_semits
			}
		else if (comp_array[pnew] < comp_array[pold])
			{
			diatonic -= (comp_array[pold] - comp_array[pnew])
			diff_semits = this_semits - (last_semits - 12)
			}
		else
			{
			diatonic -= (comp_array[pold] - comp_array[pnew])
			diff_semits = this_semits - last_semits
			}
		}
	#
	# If the current note is in a lower octave, raise it
	#
	else if (this_oc < last_oc)
		{
		sign = "-"
		while (this_oc < last_oc)
			{
			this_semits += 12
			diatonic += 7
			this_oc += 1
			}
		#
		# Determine the diatonic interval and semits difference depending
		# on the relative positions of the notes
		#
		if (comp_array[pnew] < comp_array[pold])
			{
			diatonic += (comp_array[pold] - comp_array[pnew])
			diff_semits = last_semits - this_semits
			}
		else if (comp_array[pnew] > comp_array[pold])
			{
			diatonic -= (comp_array[pnew] - comp_array[pold])
			diff_semits = last_semits - (this_semits - 12)
			}
		else
			{
			diatonic -= (comp_array[pnew] - comp_array[pold])
			diff_semits = this_semits - last_semits
			}
		}
	#
	# If the notes are in the same octave, then compare them
	#
	else if (comp_array[pold] < comp_array[pnew])
		{
		sign = "+"
		diatonic += (comp_array[pnew] - comp_array[pold])
		diff_semits = this_semits - last_semits
		}
	else if (comp_array[pnew] < comp_array[pold])
		{
		sign = "-"
		diatonic += (comp_array[pold] - comp_array[pnew])
		diff_semits = last_semits - this_semits
		}
	else
		{
		sign = ""
		diff_semits = this_semits - last_semits
		}
	#
	# If -a was specified, then remove the sign
	#
	if (options ~ /a/) sign = ""
	#
	# If -c was specified, reduce the interval to a non-compound form
	#
	if (options ~ /c/) while (diatonic > 7) diatonic -= 7
	#
	# Round the semits difference to the neares whole semitone
	#
	if (diff_semits > 0) diff_semits = int(diff_semits + .5)
	else diff_semits = int(diff_semits - .5)
	#
	# Return the resulting token
	#
	return diatonic "/" diff_semits "/" sign
	}

##############################################################################
##					Function chromatic
#	This function determines the actual melodic interval based on the diatonic
# interval and the semits of the preceding and current notes
#
function chromatic(mint_array    ,temp_diatonic,chromat)
	{
	chromat = ""
	temp_diatonic = mint_array[1]
	while (temp_diatonic > 7) temp_diatonic -= 7
	#
	# If this is a 'perfect' interval, determine the correct interval quality
	#
	if (temp_diatonic in perfect)
		{
		if (perfect[temp_diatonic] == mint_array[2])
			chromat = "P"	
		else if (perfect[temp_diatonic] > mint_array[2])
			{
			while (mint_array[2] < perfect[temp_diatonic])
				{
				chromat = chromat "d"
				mint_array[2] += 1
				}
			}
		else
			{
			while (mint_array[2] > perfect[temp_diatonic])
				{
				chromat = chromat "A"
				mint_array[2] -= 1
				}
			}
		}
	#
	# If this is a 'major' interval, determine the correct interval quality
	#
	else
		{
		if (major[temp_diatonic] == mint_array[2])
			chromat = "M"
		else if (major[temp_diatonic] == mint_array[2] + 1)
			chromat = "m"
		else if (major[temp_diatonic] > mint_array[2] + 1)
			{
			while (mint_array[2] + 1 < major[temp_diatonic])
				{
				chromat = chromat "d"
				mint_array[2] += 1
				}
			}
		else
			{
			while (mint_array[2] > major[temp_diatonic])
				{
				chromat = chromat "A"
				mint_array[2] -= 1
				}
			}
		}
	return mint_array[3] chromat mint_array[1]
	}
