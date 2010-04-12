######################################################################
##					TRANS2.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# June 7/94	Tim Racinsky	Modified parse_command to work with getopts
# April 8/95	David Huron	Fixed so **kern signifiers output in order of input
# Jan. 5/97	David Huron	Added -k option; fixed ph_key_sig regular exp.
#
#
#	This program is used to transpose kern, pitch, Tonh, and solfg encodings
# according to a diatonic interval and chromatic interval provided by the user
#
# Main Functions used:
#	parse_command()		store_new_interps()		store_indicators()
#	process_indicators()	ins_array_pos()		del_array_pos()
#	exchange_spines()		to_lower()			to_upper()
#	process_data()			process_kern()			transpose_to_kern()
#	process_pitch()		transpose_to_pitch()	process_Tonh()
#	transpose_to_Tonh()		process_solfg()		transpose_to_solfg()
#
#	VARIABLES:
#
#  -spine_path_record:  flag used to indicate if the current record
#   is a spine-path record or not.
#  -no_interpretation_yet:  flag used to indicate if an interpretation
#   record has been processed yet.
#  -current_no_of_spines:  holds the current number of active spines.
#  -stderr: allows print to be re-directed to standard error.
#  -deviation:  defines a valid equally-tempered deviation.
#  -indicators: defines spine path indicators. 
#  -input_interps:  defines valid input interpretations to process.
#  -pc_key_sig:  defines valid pitch class key signature interpretation
#  -ph_key_sig: defines valid pitch height key signature interpretation
#  -key_sig: holds command-line keysig specified for -k option
#  -kern_pitch: defines valid kern pitch
#  -solfg_pitch: defines valid solfg pitch
#  -octave_class:  defines valid octave class for pitch, Tonh, solfg
#  -null_interps:  defines lines consisting of only null tokens
#  -diatonic:  holds value of diatonic change provided by user
#  -chromatic:  holds value of chromatic change provided by user
#
#	ARRAYS:
#
#  -path_indicator and current_interp:  the standard arrays for
#   tracking interpretations.
#  -current_note: a global array used to store the current note. It holds
#   the letter name, semits value, equally tempered deviation, and octave class.
#  -kern_array:  holds kern pitches and corresponding semit values
#  -pitch_array:  holds pitch pitches and corresponding semit values
#  -Tonh_array:  holds Tonh pitches and corresponding semit values
#  -solfg_array:  holds solfg pitches and corresponding semit values
#  -pitches:  holds relative position of kern and pitch pitches
#  -Tonh_pitches: holds relative position of Tonh pitches
#  -solfg_pitches: holds relative position of solfg pitches
#
BEGIN {
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
	diatonic_reg = "^[+-]?[0-9]*$"
	deviation = "[+-][1-9][0-9]*"
	indicators = "^(\\*\\+|\\*\\-|\\*\\^|\\*v|\\*x)$"
	input_interps = "^(\\*\\*kern|\\*\\*pitch|\\*\\*Tonh|\\*\\*solfg)$"
	kern_pitch = "a+|b+|c+|d+|e+|f+|g+|A+|B+|C+|D+|E+|F+|G+"
	kern_pitch_token = "[abcdefgABCDEFG]+[-#n]*"
	pc_key_sig = "^\\*k\\[([abcdefg]((n)|(#)|(x+(#?))|(-)+))*\\]$"
	ph_key_sig = "^\\*K\\[([ABCDEFG]((n)|(#)|(x+(#?))|(-)+)*[1-9])*\\]$"
	pitch_pitch = "[ABCDEFG]"
	Tonh_pitch = "Es|As|[ABCDEFGHS]"
	solfg_pitch = "do|re|mi|fa|sol|la|si"
	octave_class = "[0-9]"
	null_interps = "^\\*(	\\*)*$"
	#
	diatonic = 0
	chromatic = 0
	#
	kern_array["a"] = 9  ; kern_array["b"] = 11 ; kern_array["c"] = 0;
	kern_array["d"] = 2  ; kern_array["e"] = 4  ; kern_array["f"] = 5;
	kern_array["g"] = 7  ; kern_array["A"] = -3 ; kern_array["B"] = -1;
	kern_array["C"] = -12; kern_array["D"] = -10; kern_array["E"] = -8;
	kern_array["F"] = -7 ; kern_array["G"] = -5 ;
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
	solfg_array["la"] = 9; solfg_array["si"] = 11; solfg_array["do"] = 0;
	solfg_array["re"] = 2; solfg_array["mi"] = 4 ; solfg_array["fa"] = 5;
	solfg_array["sol"] = 7;
	#
	pitches[0] = "C"; pitches[1] = "D"; pitches[2] = "E"; pitches[3] = "F";
	pitches[4] = "G"; pitches[5] = "A"; pitches[6] = "B";
	#
	Tonh_pitches[0] = "C"; Tonh_pitches[1] = "D"; Tonh_pitches[2] = "E|Es|S"
	Tonh_pitches[3] = "F"; Tonh_pitches[4] = "G"; Tonh_pitches[5] = "A|As"
	Tonh_pitches[6] = "B|H";
	#
	solfg_pitches[0] = "do"; solfg_pitches[1] = "re" ; solfg_pitches[2] = "mi";
	solfg_pitches[3] = "fa"; solfg_pitches[4] = "sol"; solfg_pitches[5] = "la";
	solfg_pitches[6] = "si";
	#
	# Determine user's specified diatonic and chromatic shifts
	#
	parse_command()
	}
{
#
# For each new file, set the variables
#
if (FNR == 1)
	{
	spine_path_record = FALSE
	no_interpretation_yet = TRUE
	current_no_of_spines = 0
	}
#
# Print all comments.
#
if ($0 ~ /^!/) print $0
#
# Process interpretations.
#
else if ($0 ~ /^\*/)
	{
	# Change key signatures if -k option specified.
	if (keysig == "NONE") print $0
	else
		{
		if ($1 ~ pc_key_sig || $1 ~ ph_key_sig) interp_out_line = keysig
		else interp_out_line = $1
		for (i=2; i<=NF; i++)
			{
			if ($i ~ pc_key_sig || $i ~ ph_key_sig) interp_out_line = interp_out_line "	" keysig
			else interp_out_line = interp_out_line "	" $i
			}
		print interp_out_line
		}
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
		spine_path_record = FALSE
		for (i = 1; i <= current_no_of_spines; i += 1)
			{			
			if ($i ~ indicators) {
				spine_path_record = TRUE 
				break
				}
			}
		#
		# If it is a spine-path record, print and process the indicators
		#
		if (spine_path_record)
			{
			store_indicators()
			process_indicators()
			if (current_no_of_spines == 0) no_interpretation_yet = TRUE
			}
		#
		# Otherwise, just store the interpretations
		#
		else store_new_interps()
		}
	}
#
# Process current data record
#
else process_data()
}	

##################################################################
##			FUNCTIONS USED IN TRANS2.AWK
##################################################################

##################################################################
##			Function Parse_command
#
function parse_command()
	{
	#
	# Make sure valid diatonic and chromatic values are entered
	#
	if (ARGV[2] !~ diatonic_reg)
		{
		print "trans: ERROR: Invalid value specified for diatonic interval "\
			 "on command line." > stderr
		exit
		}
	else diatonic = ARGV[2] + 0
	if (ARGV[3] !~ diatonic_reg)
		{
		print "trans: ERROR: Invalid value specified for chromatic interval "\
			 "on command line." > stderr
		exit
		}
	else chromatic = ARGV[3] + 0
	if (ARGV[4] !~ pc_key_sig && ARGV[4] !~ ph_key_sig && ARGV[4] !="NONE")
		{
		print "trans: ERROR: Invalid key signature specified with -k option."\
			 > stderr
		exit
		}
	keysig = ARGV[4]
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ""
	}

################################################################
##				Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
#
function store_new_interps(      j,interp_line,null_line)
	{
	interp_line = ""
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		#
		# Store exclusive interpretations
		#
		if ($j ~ /^\*\*/) current_interp[j] = $j
		#
		# If the interpretation is a 'convertible' interpretation, then
		# the output line must show the transposition interpretation.
		#
		if ($j ~ input_interps)
			interp_line = interp_line "*Trd" diatonic "c" chromatic "\t"
		else interp_line = interp_line "*\t"
		}
	#
	# Print the new interpretation line if it is not all null interpretations
	#
	null_line = substr(interp_line,1,length(interp_line)-1)
	if (null_line !~ null_interps) print null_line
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
# and manipulates the arrays 'path_indicator' and 'current_interp'
# according to the contents of the array 'path_indicator'.
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
function exchange_spines(    j,count,ex_array,temp)
	{
	count = 1
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		if (path_indicator[j] == "x")
			{
			ex_array[count] = j
			count += 1
			path_indicator[j] = "*"
			}
		}
	temp = current_interp[ex_array[1]]
	current_interp[ex_array[1]] = current_interp[ex_array[2]]
	current_interp[ex_array[2]] = temp
	}

########################################################################
##				Function to_lower
#	This function converts a character to lower case.
#
function to_lower(letter)
	{
	if (letter == "A") return "a"
	else if (letter == "B") return "b"
	else if (letter == "C") return "c"
	else if (letter == "D") return "d"
	else if (letter == "E") return "e"
	else if (letter == "F") return "f"
	else if (letter == "G") return "g"
	}

########################################################################
##				Function to_upper
#	This function converts a character to upper case.
#
function to_upper(letter)
	{
	if (letter == "a") return "A"
	else if (letter == "b") return "B"
	else if (letter == "c") return "C"
	else if (letter == "d") return "D"
	else if (letter == "e") return "E"
	else if (letter == "f") return "F"
	else if (letter == "g") return "G"
	}

####################################################################
##				Function Process_data
#	This function takes the given input record and processes the data
# tokens in each spine according to their current exclusive interpretation
# and the value of the data tokens.
#
function process_data(     j,current_string,current_token)
	{
	current_string = ""
	#
	# Loop through each of the currently active spines and process the data
	# tokens that are in each spine.
	#
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		#
		# If the current interpretation is a valid input interpretation
		#
		if (current_interp[j] ~ input_interps)
			{
			current_token = ""
			#
			# If the token contains a null token or barline, print it
			#
			if ($j ~ /^[.]$/ || $j ~ /^=/) current_token = $j
			#
			# For all other tokens
			#
			else
				{
				#
				# Call the appropriate function based on the exclusive 
				# interpretation in order to obtain the transposed output.
				#
				if (current_interp[j] == "**pitch")
					current_token = process_pitch($j)
				else if (current_interp[j] == "**Tonh")
					current_token = process_Tonh($j)
				else if (current_interp[j] == "**solfg")
					current_token = process_solfg($j)
				else if (current_interp[j] == "**kern")
					current_token = process_kern($j)
				}
			#
			# Construct a string for the current line
			#
			current_string = current_string current_token "\t"
			}
		#
		# All other spines simply have their data tokens echoed
		#
		else current_string = current_string $j "\t"
		}
	#
	# After all spines have been processed, print the resulting record
	#
	print substr(current_string,1,length(current_string)-1)
	}

####################################################################
##				Function Process_kern
#	This function takes kern input and transposes it
#
function process_kern(data_token,  arraya,j,return_token,found,split_num,pitch)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arraya," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Simply output any rests
		#
		if (arraya[j] ~ /r/) return_token = return_token arraya[j] " "
		#
		# Otherwise, process the note
		#
		else
			{
			found = TRUE
			current_note[2] = 0
			#
			# Save the initial and final signifiers surrounding
			# the pitch information.
			#
			split(arraya[j],non_pitch_info,kern_pitch_token)
			#
			# First find the letter name of the note; store and remove it
			# Also determine the octave class.
			#
			if (match(arraya[j],kern_pitch))
				{
				pitch = substr(arraya[j],RSTART,1)
				if (pitch ~ /[abcdefg]/)
					{
					current_note[2] = kern_array[pitch]+(12*(RLENGTH-1))
					current_note[4] = 3 + RLENGTH
					pitch = to_upper(pitch)
					}
				else
					{
					current_note[2] = kern_array[pitch]-(12*(RLENGTH-1))
					current_note[4] = 4 - RLENGTH
					}
				current_note[1] = pitch
				sub(kern_pitch,"",arraya[j])
				}
			else found = FALSE
			#
			# If a letter name was found, continue to process
			#
			if (found)
				{
				#
				# Next determine the value of any flats or sharps
				#
				if (match(arraya[j],/-+/))
					{ current_note[2] -= RLENGTH ; sub(/-+/,"",arraya[j]) }
				if (match(arraya[j],/#+/))
					{ current_note[2] += RLENGTH ; sub(/#+/,"",arraya[j]) }
				#
				# Remove any naturals
				#
				gsub("n","",arraya[j])
				#
				# Transpose the note
				#
				return_token = return_token non_pitch_info[1] \
					transpose_to_kern() non_pitch_info[2] " "
				}
			#
			# If no letter name, return proper token depending on -x option
			#
			else return_token = return_token arraya[j] " "
			}
		}
	#
	# Return at least a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

########################################################################
##				Function Transpose_to_kern
#	This function does the actual transpose of the kern note
#
function transpose_to_kern(   i,old_pos,new_pos,new_pitch,new_semits,note\
												,semits_count)
	{
	#
	# Determine the existing 'position' of the note and determine the new pitch
	#
	for (i = 0; i <= 6; i += 1)
		{
		if (pitches[i] == current_note[1])
			{
			old_pos = i
			new_pos = (((old_pos + diatonic) % 7) + 7) % 7
			new_pitch = pitches[new_pos]
			break
			}
		}
	#
	# Determine the new octave class
	#
	new_pos = old_pos + diatonic
	if (diatonic > 0)
		current_note[4] = current_note[4] + int((new_pos) / 7)
	else
		current_note[4] = current_note[4] \
				+ (new_pos >= 0 ? 0 : int((new_pos + 1) / 7) - 1)
	#
	# Build the kern pitch
	#
	if (current_note[4] >= 4)
		{
		new_pitch = to_lower(new_pitch)
		note = new_pitch
		semits_count = kern_array[note]
		while (current_note[4] > 4)
			{
			note = note new_pitch
			semits_count += 12
			current_note[4] -= 1
			}
		}
	else
		{
		note = new_pitch
		semits_count = kern_array[note]
		while (current_note[4] < 3)
			{
			note = note new_pitch
			semits_count -= 12
			current_note[4] += 1
			}
		}
	new_semits = current_note[2] + chromatic
	#
	# Add the necessary accidentals
	#
	if (new_semits > semits_count)
		{
		while (semits_count != new_semits)
			{
			semits_count += 1
			note = note "#"
			}
		}
	else if (new_semits < semits_count)
		{
		while (semits_count != new_semits)
			{
			semits_count -= 1
			note = note "-"
			}
		}
	return note
	}

#########################################################################
##				Function Process_pitch
#	This function accepts pitch input and transposes it
#
function process_pitch(data_token,  return_token,arrayb,j,found,split_num,pitch)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayb," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If a rest occurs, simply echo it
		#
		if (arrayb[j] ~ /r/) return_token = return_token arrayb[j] " "
		#
		# Otherwise, process the pitch note
		#
		else
			{
			current_note[2] = 0
			found = TRUE
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arrayb[j],pitch_pitch))
				{
				pitch = substr(arrayb[j],RSTART,1)
				current_note[1] = pitch
				current_note[2] = pitch_array[pitch]
				sub(pitch_pitch,"",arrayb[j])
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
					{ current_note[2] -= RLENGTH ; sub(/b+/,"",arrayb[j]) }
				else
					{
					if (match(arrayb[j],/x+/))
						{
						current_note[2] += 2 * RLENGTH
						sub(/x+/,"",arrayb[j])
						}
					if (match(arrayb[j],/#+/))
						{
						current_note[2] += RLENGTH
						sub(/#+/,"",arrayb[j])
						}
					}
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayb[j],deviation))
					{
					current_note[3] = substr(arrayb[j],RSTART,RLENGTH)
					sub(deviation,"",arrayb[j])
					}
				else current_note[3] = ""
				#
				# Determine which octave the note is in
				#
				if (match(arrayb[j],octave_class)) 
					{
					current_note[4] = substr(arrayb[j],RSTART,RLENGTH) + 0
					current_note[2] \
						+= (12 * (substr(arrayb[j],RSTART,1) + 0)) - 48
					sub(octave_class,"",arrayb[j])
					}
				else current_note[4] = 4
				return_token = return_token arrayb[j] \
											transpose_to_pitch() " "
				}
			else return_token = return_token arrayb[j] " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

########################################################################
##				Function Transpose_to_pitch
#	This function actually transposes the pitch pitch
#
function transpose_to_pitch(   i,old_pos,new_pos,new_pitch,new_semits,note,class\
													,semits_count)
	{
	#
	# Determine the position of the note and determine the resulting pitch
	#
	for (i = 0; i <= 6; i += 1)
		{
		if (pitches[i] == current_note[1])
			{
			old_pos = i
			new_pos = (((old_pos + diatonic) % 7) + 7) % 7
			new_pitch = pitches[new_pos]
			break
			}
		}
	#
	# Determine the new octave class
	#
	new_pos = old_pos + diatonic
	if (diatonic > 0)
		current_note[4] = current_note[4] + int((new_pos) / 7)
	else
		current_note[4] = current_note[4] \
				+ (new_pos >= 0 ? 0 : int((new_pos + 1) / 7) - 1)
	#
	# Build the pitch pitch
	#
	note = new_pitch
	semits_count = pitch_array[note]
	class = current_note[4]
	if (class >= 4)
		{
		while (class > 4)
			{
			semits_count += 12
			class -= 1
			}
		}
	else
		{
		semits_count -= 12
		while (class < 3)
			{
			semits_count -= 12
			class += 1
			}
		}
	new_semits = current_note[2] + chromatic
	#
	# Add any necessary accidentals to the pitch note
	#
	if (new_semits > semits_count)
		{
		while (semits_count != new_semits)
			{
			semits_count += 1
			note = note "#"
			}
		}
	else if (new_semits < semits_count)
		{
		while (semits_count != new_semits)
			{
			semits_count -= 1
			note = note "b"
			}
		}
	#
	# Add the octave class and deviation
	#
	note = note current_note[4]
	note = note current_note[3]
	return note
	}
	
###########################################################################
##					Function Process_Tonh
#	This function takes Tonh input and transposes it
#
function process_Tonh(data_token,  found,arrayc,j,return_token,split_num,pitch)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayc," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Echo any rests that occur in the data token
		#
		if (arrayc[j] ~ /r/) return_token = return_token arrayc[j] " "
		#
		# Otherwise, process the Tonh note
		#
		else
			{
			current_note[2] = 0
			found = TRUE
			#
			# Determine the Tonh pitch and accidentals
			#
			if (match(arrayc[j],Tonh_pitch))
				{
				pitch = substr(arrayc[j],RSTART,RLENGTH)
				current_note[1] = pitch
				current_note[2] = Tonh_array[pitch]
				sub(Tonh_pitch,"",arrayc[j])
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
					current_note[2] -= RLENGTH/2
					sub(/(es)+/,"",arrayc[j])
					}
				else if (match(arrayc[j],/(is)+/))
					{
					current_note[2] += RLENGTH/2
					sub(/(is)+/,"",arrayc[j])
					}
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayc[j],deviation))
					{
					current_note[3] = substr(arrayc[j],RSTART,RLENGTH)
					sub(deviation,"",arrayc[j])
					}
				#
				# Determine which octave the note is in
				#
				if (match(arrayc[j],octave_class)) 
					{
					current_note[4] = substr(arrayc[j],RSTART,RLENGTH) + 0
					current_note[2] \
						+= (12 * (substr(arrayc[j],RSTART,1) + 0)) - 48
					sub(octave_class,"",arrayc[j])
					}
				else current_note[4] = 4
				return_token = return_token arrayc[j] \
										transpose_to_Tonh() " "
				}
			#
			# If no letter name, the return token depends on the -x option
			#
			else return_token = return_token arrayc[j] " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

########################################################################
##				Function Transpose_to_Tonh
#	This function actually transposes the Tonh note
#
function transpose_to_Tonh(   i,old_pos,new_pos,new_pitch,new_semits,note,class\
													,semits_count)
	{
	#
	# Determine the position of the note and the resulting pitch
	#
	for (i = 0; i <= 6; i += 1)
		{
		if (current_note[1] ~ Tonh_pitches[i])
			{
			old_pos = i
			new_pos = (((old_pos + diatonic) % 7) + 7) % 7
			new_pitch = pitches[new_pos]
			break
			}
		}
	#
	# Determine the new octave class
	#
	new_pos = old_pos + diatonic
	if (diatonic > 0)
		current_note[4] = current_note[4] + int((new_pos) / 7)
	else
		current_note[4] = current_note[4] \
				+ (new_pos >= 0 ? 0 : int((new_pos + 1) / 7) - 1)
	#
	# Build the Tonh pitch
	#
	note = new_pitch
	semits_count = pitch_array[note]
	class = current_note[4]
	if (class >= 4)
		{
		while (class > 4)
			{
			semits_count += 12
			class -= 1
			}
		}
	else
		{
		semits_count -= 12
		while (class < 3)
			{
			semits_count -= 12
			class += 1
			}
		}
	new_semits = current_note[2] + chromatic
	#
	# Add any necessary accidentals and make sure the pitch spelling
	# conforms to the peculiarities of the Tonh system
	#
	if (new_semits > semits_count)
		{
		if (note == "B") note = "H"
		while (semits_count != new_semits)
			{
			semits_count += 1
			note = note "is"
			}
		}
	else if (new_semits < semits_count)
		{
		if (note == "E")
			{
			note = "Es"
			semits_count -= 1
			}
		else if (note == "A")
			{
			note = "As"
			semits_count -= 1
			}
		else if (note == "B")
			{
			if ((semits_count - new_semits) == 1) semits_count -= 1
			else note = "H"
			}
		while (semits_count != new_semits)
			{
			semits_count -= 1
			note = note "es"
			}
		}
	else
		{
		if (note == "B") note = "H"
		}
	#
	# Add the octave class and deviation
	#
	note = note current_note[4]
	note = note current_note[3]
	return note
	}

###########################################################################
##					Function Process_solfg
#	This function takes solfg input and  transposes it
#
function process_solfg(data_token, found,arrayd,j,return_token,split_num,pitch)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayd," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Echo any rests in the input.  Be sure to check for 're'
		#
		if (arrayd[j] ~ /r/ && arrayd[j] !~ /re/)
			return_token = return_token arrayd[j] " "
		#
		# Otherwise, process the solfg note
		#
		else
			{
			current_note[2] = 0
			found = TRUE
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arrayd[j],solfg_pitch))
				{
				pitch = substr(arrayd[j],RSTART,RLENGTH)
				current_note[1] = pitch
				current_note[2] = solfg_array[pitch]
				sub(solfg_pitch,"",arrayd[j])
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
				if (match(arrayd[j],/~b+/))
					{
					current_note[2] -= (RLENGTH - 1)
					sub(/~b+/,"",arrayd[j])
					}
				else if (match(arrayd[j],/~d+/))
					{
					current_note[2] += (RLENGTH - 1)
					sub(/~d+/,"",arrayd[j])
					}
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayd[j],deviation))
					{
					current_note[3] = substr(arrayd[j],RSTART,RLENGTH)
					sub(deviation,"",arrayd[j])
					}
				#
				# Determine which octave the note is in
				#
				if (match(arrayd[j],octave_class)) 
					{
					current_note[4] = substr(arrayd[j],RSTART,RLENGTH) + 0
					current_note[2] \
						+= (12 * (substr(arrayd[j],RSTART,1) + 0)) - 48
					sub(octave_class,"",arrayd[j])
					}
				else current_note[4] = 4
				return_token = return_token arrayd[j] \
										transpose_to_solfg() " "
				}
			#
			# If no letter name was found, return token depends on -x option
			#
			else return_token = return_token arrayd[j] " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

########################################################################
##				Function Transpose_to_solfg
#	This function does the actual transposition of the solfg pitch
#
function transpose_to_solfg(  i,old_pos,new_pos,new_pitch,new_semits,note,class\
													,semits_count)
	{
	#
	# Determine the existing 'position' of the note and determine the new pitch
	#
	for (i = 0; i <= 6; i += 1)
		{
		if (solfg_pitches[i] == current_note[1])
			{
			old_pos = i
			new_pos = (((old_pos + diatonic) % 7) + 7) % 7
			new_pitch = solfg_pitches[new_pos]
			break
			}
		}
	#
	# Determine the new octave class
	#
	new_pos = old_pos + diatonic
	if (diatonic > 0)
		current_note[4] = current_note[4] + int((new_pos) / 7)
	else
		current_note[4] = current_note[4] \
				+ (new_pos >= 0 ? 0 : int((new_pos + 1) / 7) - 1)
	#
	# Build the solfg pitch
	#
	note = new_pitch
	semits_count = solfg_array[note]
	class = current_note[4]
	if (class >= 4)
		{
		while (class > 4)
			{
			semits_count += 12
			class -= 1
			}
		}
	else
		{
		semits_count -= 12
		while (class < 3)
			{
			semits_count -= 12
			class += 1
			}
		}
	new_semits = current_note[2] + chromatic
	#
	# Add any necessary accidentals
	#
	if (new_semits > semits_count)
		{
		note = note "~d"
		semits_count += 1
		while (semits_count != new_semits)
			{
			semits_count += 1
			note = note "d"
			}
		}
	else if (new_semits < semits_count)
		{
		note = note "~b"
		semits_count -= 1
		while (semits_count != new_semits)
			{
			semits_count -= 1
			note = note "b"
			}
		}
	#
	# Add the octave class and deviation
	#
	note = note current_note[4]
	note = note current_note[3]
	return note
	}
