######################################################################
##					SEMITS.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       	Description:
# May 10/94	Tim Racinsky		Corrected Relative Tuning problem for multi-
#							stringed courses in **fret spines
# June 9/94	Tim Racinsky		Modified parse_command to work with getopts
# June 23/94	Tim Racinsky		Modified solfg representation to include "-"
#
#
#	This program is used to convert pitch units encoded  in various
# representations to semits.
#
# Main Functions used:
#	parse_command()		store_new_interps()		get_absolute()
#	get_semits()			get_relative()			get_frets()
#	store_indicators()		process_indicators()	ins_array_pos()
#	del_array_pos()		exchange_spines()		copy_fret()
#	process_data()			convert_to_freq()		process_kern()
#	process_pitch()		process_Tonh()			process_solfg()
#	process_semits()		process_cents()		process_cbr()
#	process_cocho()		process_MIDI()			process_fret()
#	process_specC()		process_freq()
#
#	VARIABLES:
#
#  -spine_path_record:  flag used to indicate if the current record
#   is a spine-path record or not.
#  -no_interpretation_yet:  flag used to indicate if an interpretation
#   record has been processed yet.
#  -current_no_of_spines:  holds the current number of active spines.
#  -options:  holds which options the user has specified.
#  -stderr: allows print to be re-directed to standard error.
#  -middle_C_freq:  holds the frequency of middle c.
#  -floating_pt_num:  defines a valid floating point number.
#  -uftp: defines an unsigned floating point number.
#  -deviation:  defines a valid equally-tempered deviation.
#  -indicators: defines spine path indicators. 
#  -input_interps:  defines valid input interpretations to process.
#  -kern_pitch:  defines a valid kern pitch letter
#  -pitch_pitch:  defines a valid pitch pitch letter
#  -Tonh_pitch:  defines a valid Tonh pitch letter
#  -solfg_pitch:  defines a valid solfg pitch letter
#  -octave_class:  defines a valid octave class specfication
#  -fret_index:  holds index of fret spine to be defined next
#  -abs_tuning:  defines absolute tuning interpretation
#  -rel_tuning:  defines relative tuning interpretation
#  -fret_tuning:  defines fret tuning interpretation
#  -precision: defines precision of output
#
#	ARRAYS:
#
#  -path_indicator and current_interp:  the standard arrays for
#   tracking interpretations.
#  -fret_array:  for each fret spine it contains the index of the arrays that
#   the given spine corresponds to.
#  -kern_array:  contains kern pitches and corresponding semit values
#  -pitch_array:  contains pitch pitches and corresponding semit values
#  -Tonh_array:  contains Tonh pitches and corresponding semit values
#  -solfg_array:  contains solfg pitches and corresponding semit values
#
BEGIN {
	USAGE="\nUSAGE: semits -h                (Help Screen)\n       semits "\
		 "[-p n] [-tx] [file ...]\n"
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
	middle_C_freq = 261.625549
	floating_pt_num = "[+-]?(([0-9]+(\\.[0-9]*)?)|((0*)?\\.[0-9]*))"
	uftp = "(([0-9]+[.]?[0-9]*)|([0-9]*[.]?[0-9]+))"
	deviation = "[+-][1-9][0-9]*"
	tie_reg = "\\]|\\[|_"
	cont_tie_reg = "\\]|_"
	indicators = "^(\\*\\+|\\*\\-|\\*\\^|\\*v|\\*x)$"
	input_interps = "^(\\*\\*specC|\\*\\*kern|\\*\\*pitch|\\*\\*Tonh"\
				 "|\\*\\*solfg|\\*\\*semits|\\*\\*cents|\\*\\*MIDI"\
				 "|\\*\\*fret|\\*\\*freq)$"
	kern_pitch = "a+|b+|c+|d+|e+|f+|g+|A+|B+|C+|D+|E+|F+|G+"
	pitch_pitch = "[A-G]"
	Tonh_pitch = "Es|As|[A-HS]"
	solfg_pitch = "do|re|mi|fa|sol|la|si"
	octave_class = "[0-9]"
	fret_index = 1
	abs_tuning = "^\\*AT:[A-G][#b]?[0-9]+(" deviation ")?$"
	rel_tuning = "^\\*RT:" uftp "(," uftp ")*(:" uftp "(," uftp ")*)*$"
	fret_tuning = "^\\*FT:" uftp "(," uftp ")*$"
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
	# Determine user's specified options
	#
	precision = 0
	options = ""
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
			print $0
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
##			FUNCTIONS USED IN SEMITS.AWK
##################################################################

##################################################################
##			Function Parse_command
#	This function checks that the input passed from semits.ksh
# contains a list of valid options and assigns it to the variable
# options if that is the case.
#
function parse_command()
	{
	#
	# Set the options variable if not null
	#
	if (ARGV[2] != "null") options = ARGV[2]
	if (ARGV[3] != "null")
		{
		if (ARGV[3] !~ /^[0-9]*$/)
			{
			print "semits: ERROR: Invalid precision specified: " \
				 ARGV[3] > stderr
			print USAGE > stderr
			exit
			}
		else precision = ARGV[3] + 0
		}
	ARGV[1] = ARGV[2] = ARGV[3] = ""
	}

################################################################
##				Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
#
function store_new_interps(      j,interp_line)
	{
	interp_line = ""
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		#
		# Store exclusive interpretations.
		#
		if ($j ~ /^\*\*/) current_interp[j] = $j
		#
		# Check for any information pertaining to a spine with the
		# fret interpretation
		#
		if ($j ~ /^\*\*fret/)
			{
			absolute[fret_index] = -20
			fret_array[j] = fret_index
			fret_index += 1
			}
		else if ($j ~ /^\*AT:/) get_absolute($j,j)
		else if ($j ~ /^\*RT:/) get_relative($j,j)
		else if ($j ~ /^\*FT:/) get_frets($j,j)
		#
		# If the interpretation is a 'convertible' interpretation, then
		# the output line must show semits.
		#
		if ($j ~ input_interps) interp_line = interp_line "**semits\t"
		else interp_line = interp_line $j "\t"
		}
	#
	# Print the new interpretation line
	#
	print substr(interp_line,1,length(interp_line)-1)
	}

#########################################################################
##					Function Get_absolute
#	This function obains the absolute tuning for a fret spine.
#
function get_absolute(interp,position,    temp_index)
	{
	#
	# The interpretation must be valid
	#
	if (interp !~ abs_tuning)
		{
		print "semits: ERROR: Invalid absolute tuning interpretation in "\
			 "line " NR "." > stderr
		exit
		}
	#
	# Only proceses if current spine is for fret
	#
	if (current_interp[position] == "**fret")
		{
		temp_index = fret_array[position]
		split(interp,at_array,":")
		absolute[temp_index] = get_semits(at_array[2])
		}
	}

#########################################################################
##					Function Get_semits
#	This function simply translates pitch into semits without any checking
#
function get_semits(note,   semits)
	{
	semits = 0
	#
	# Get pitch
	#
	match(note,pitch_pitch)
	pitch = substr(note,RSTART,1)
	semits = pitch_array[pitch]
	#
	# Get accidentals
	#
	if (match(note,/b+/)) semits -= RLENGTH
	else if (match(note,/#+/)) semits += RLENGTH
	#
	# Get deviation
	#
	if (match(note,deviation)) semits += (substr(note,RSTART,RLENGTH) * 0.01)
	#
	# Get octave class
	#
	match(note,octave_class)
	semits += (12 * (substr(note,RSTART,1) + 0)) - 48
	return semits
	}

#########################################################################
##					Function Get_relative
#	This function gets the relative tuning for a fret spine.
#
function get_relative(interp,position,   courses,i,temp_index,num1,string_num)
	{
	#
	# The interpretation must be a valid relative tuning interpretation
	#
	if (interp !~ rel_tuning)
		{
		print "semits: ERROR: Invalid relative tuning interpretation in "\
			 "line " NR "." > stderr
		exit
		}
	#
	# The current spine must have a fret exclusive interpretation
	#
	if (current_interp[position] == "**fret")
		{
		temp_index = fret_array[position]
		num1 = split(interp,courses,":")
		#
		# Store the value(s) for each course in relative
		#
		for (i = 2; i <= num1; i += 1)
			{
			relative[temp_index,i-1] = courses[i]
			}
		}
	}

#########################################################################
##					Function Get_frets
#	This function gets the fret tuning for a fret spine
#
function get_frets(interp,position,   temp_index,temp_array,num1,frets,i)
	{
	#
	# The interpretation must be a valid fret tuning interpretation
	#
	if (interp !~ fret_tuning)
		{
		print "semits: ERROR: Invalid fret tuning interpretation in "\
			 "line " NR "." > stderr
		exit
		}
	#
	# Only process if the current spine is a fret spine
	#
	if (current_interp[position] == "**fret")
		{
		temp_index = fret_array[position]
		split(interp,temp_array,":")
		num1 = split(temp_array[2],frets,",")
		for (i = 1; i <= num1; i += 1) fret_val[temp_index,i] = frets[i]
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
# and manipulates the arrays 'path_indicator', 'current_interp', and
# 'fret_array' according to the contents of the array 'path_indicator'.
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
			if (current_interp[i] == "**fret") copy_fret(i,i+1)
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
#	This function inserts new positions in the arrays 'path_indicator' and
# 'current_interp' and copies elements so that everything is preserved
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j -= 1)
		{
		path_indicator[j] = path_indicator[j-1]
		current_interp[j] = current_interp[j-1]
		fret_array[j] = fret_array[j-1]
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
		fret_array[j] = fret_array[j+1]
		}
	delete path_indicator[j]
	delete current_interp[j]
	delete fret_array[j]
	current_no_of_spines -= 1
	} 

##########################################################################
##				Function Copy_fret
#	A fret spine that is split must create a copy of its values in the 
# tuning arrays so that the new spine can manipulate them freely.
#
function copy_fret(current,new,    current_index,temp_array,i)
	{
	current_index = fret_array[current]
	#
	# Copy the absolute tuning values
	#
	absolute[fret_index] = absolute[current_index]
	#
	# Copy the relative tuning values
	#
	for (i in relative)
		{
		split(i,temp_array,SUBSEP)
		if (temp_array[1] == current_index)
			{
			relative[fret_index,temp_array[2]] \
							= relative[current_index,temp_array[2]]
			}
		}
	#
	# Copy the fret tuning values
	#
	for (i in fret_val)
		{
		split(i,temp_array,SUBSEP)
		if (temp_array[1] == current_index)
			fret_val[fret_index,temp_array[2]] \
							= fret_val[current_index,temp_array[2]]
		}
	#
	# Store the fret index in fret_array and update the fret index
	#
	fret_array[new] = fret_index
	fret_index += 1
	} 

###################################################################
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in current_interp and fret_array.
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
	temp = fret_array[arraya[1]]
	fret_array[arraya[1]] = fret_array[arraya[2]]
	fret_array[arraya[2]] = temp
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
				# interpretation in order to obtain semits output.
				#
				if (current_interp[j] == "**kern")
					current_token = process_kern($j)
				else if (current_interp[j] == "**pitch")
					current_token = process_pitch($j)
				else if (current_interp[j] == "**Tonh")
					current_token = process_Tonh($j)
				else if (current_interp[j] == "**solfg")
					current_token = process_solfg($j)
				else if (current_interp[j] == "**cents")
					current_token = process_cents($j)
				else if (current_interp[j] == "**freq")
					current_token = process_freq($j)
				else if (current_interp[j] == "**specC")
					current_token = process_specC($j)
				else	if (current_interp[j] == "**fret")
					current_token = process_fret($j,j)
				else if (current_interp[j] == "**MIDI")
					current_token = process_MIDI($j)
				else if (current_interp[j] == "**semits")
					current_token = process_semits($j)
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
#	This function takes kern input and translates to semits
#
function process_kern(data_token,  arrayc,j,semits,return_token,found\
												,pitch,split_num)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayc," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Process ties if -t option specified
		#
		if (options ~ /t/ && arrayc[j] ~ cont_tie_reg)
			{ if (split_num == 1) return_token = "." }
		#
		# Simply output any rests
		#
		else if (arrayc[j] ~ /r/) return_token = return_token arrayc[j] " "
		#
		# Otherwise, process the note
		#
		else
			{
			if (options ~ /x/ && arrayc[j] ~ tie_reg)
				sub(tie_reg,"",arrayc[j])
			found = TRUE
			semits = 0
			#
			# First find the letter name of the note; store and remove it
			#
			if (match(arrayc[j],kern_pitch))
				{
				pitch = substr(arrayc[j],RSTART,1)
				if (pitch ~ /[a-g]/)
					semits = kern_array[pitch]+(12*(RLENGTH-1))
				else semits = kern_array[pitch]-(12*(RLENGTH-1))
				sub(kern_pitch,SUBSEP,arrayc[j])
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
				if (match(arrayc[j],/-+/))
					{ semits -=  RLENGTH ; sub(/-+/,"",arrayc[j]) }
				if (match(arrayc[j],/#+/))
					{ semits += RLENGTH ; sub(/#+/,"",arrayc[j]) }
				gsub("n","",arrayc[j])
				semits = sprintf("%." precision "f",semits)
				sub(SUBSEP,semits,arrayc[j])
				return_token = return_token arrayc[j] " "
				}
			#
			# If no letter name, return proper token depending on -x option
			#
			else return_token = return_token arrayc[j] " "
			}
		}
	#
	# Return at least a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#########################################################################
##				Function Process_pitch
#	This function accepts pitch input and translates to semits 
#
function process_pitch(data_token,  return_token,semits,arrayd,j,found\
												,pitch,split_num)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayd," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If a rest occurs, simply echo it
		#
		if (arrayd[j] ~ /r/) return_token = return_token arrayd[j] " "
		#
		# Otherwise, process the pitch note
		#
		else
			{
			semits = 0
			found = TRUE
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arrayd[j],pitch_pitch))
				{
				pitch = substr(arrayd[j],RSTART,1)
				semits = pitch_array[pitch]
				sub(pitch_pitch,SUBSEP,arrayd[j])
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
				if (match(arrayd[j],/b+/))
					{ semits -= RLENGTH ; sub(/b+/,"",arrayd[j]) }
				else
					{
					if (match(arrayd[j],/x+/))
						{ semits += 2 * RLENGTH ; sub(/x+/,"",arrayd[j]) }
					if (match(arrayd[j],/#+/))
						{ semits += RLENGTH ; sub(/#+/,"",arrayd[j]) }
					}
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayd[j],deviation))
					{
					semits += (substr(arrayd[j],RSTART,RLENGTH) * 0.01)
					sub(deviation,"",arrayd[j])
					}
				#
				# Determine which octave the note is in
				#
				if (match(arrayd[j],octave_class)) 
					{
					semits += (12 * (substr(arrayd[j],RSTART,1) + 0)) - 48
					sub(octave_class,"",arrayd[j])
					}
				semits = sprintf("%." precision "f",semits)
				sub(SUBSEP,semits,arrayd[j])
				return_token = return_token arrayd[j] " "
				}
			#
			# If no letter name, return appropriate token depending on
			# -x option
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
	
###########################################################################
##					Function Process_Tonh
#	This function takes Tonh input and translates to semits
#
function process_Tonh(data_token,  found,semits,arrayi,j,return_token\
												,pitch,split_num)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayi," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Echo any rests that occur in the data token
		#
		if (arrayi[j] ~ /r/) return_token = return_token arrayi[j] " "
		#
		# Otherwise, process the Tonh note
		#
		else
			{
			semits = 0
			found = TRUE
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arrayi[j],Tonh_pitch))
				{
				pitch = substr(arrayi[j],RSTART,RLENGTH)
				semits = Tonh_array[pitch]
				sub(Tonh_pitch,SUBSEP,arrayi[j])
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
				if (match(arrayi[j],/(es)+/))
					{
					semits -= RLENGTH/2
					sub(/(es)+/,"",arrayi[j])
					}
				else if (match(arrayi[j],/(is)+/))
					{
					semits += RLENGTH/2
					sub(/(is)+/,"",arrayi[j])
					}
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayi[j],deviation))
					{
					semits += (substr(arrayi[j],RSTART,RLENGTH) * 0.01)
					sub(deviation,"",arrayi[j])
					}
				#
				# Determine which octave the note is in
				#
				if (match(arrayi[j],octave_class)) 
					{
					semits += (12 * (substr(arrayi[j],RSTART,1) + 0)) - 48
					sub(octave_class,"",arrayi[j])
					}
				semits = sprintf("%." precision "f",semits)
				sub(SUBSEP,semits,arrayi[j])
				return_token = return_token arrayi[j] " "
				}
			#
			# If no letter name, the return token depends on the -x option
			#
			else return_token = return_token arrayi[j] " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

###########################################################################
##					Function Process_solfg
#	This function takes solfg input and translates to semits
#
function process_solfg(data_token, found,semits,arrayj,j,return_token\
												,pitch,split_num)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayj," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Echo any rests in the input.  Be sure to check for 're'
		#
		if (arrayj[j] ~ /r/ && arrayj[j] !~ /re/)
			return_token = return_token arrayj[j] " "
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
			if (match(arrayj[j],solfg_pitch))
				{
				pitch = substr(arrayj[j],RSTART,RLENGTH)
				semits = solfg_array[pitch]
				sub(solfg_pitch,SUBSEP,arrayj[j])
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
				if (match(arrayj[j],/~b+/))
					{ semits -= (RLENGTH - 1) ; sub(/~b+/,"",arrayj[j]) }
				else if (match(arrayj[j],/~d+/))
					{ semits += (RLENGTH - 1) ; sub(/~d+/,"",arrayj[j]) }
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayj[j],deviation))
					{
					semits += (substr(arrayj[j],RSTART,RLENGTH) * 0.01)
					sub(deviation,"",arrayj[j])
					}
				#
				# Determine which octave the note is in
				#
				if (match(arrayj[j],octave_class)) 
					{
					semits += (12 * (substr(arrayj[j],RSTART,1) + 0)) - 48
					sub(octave_class,"",arrayj[j])
					}
				semits = sprintf("%." precision "f",semits)
				sub(SUBSEP,semits,arrayj[j])
				return_token = return_token arrayj[j] " "
				}
			#
			# If no letter name was found, return token depends on -x option
			#
			else return_token = return_token arrayj[j] " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#############################################################################
##				Function Process_specC
#	This function takes specC input and translates to semits
#
function process_specC(data_token, j,return_token,split_num,arraym,specC,semits)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arraym," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Echo any rests that occur in the input
		#
		if (arraym[j] ~ /r/) return_token = return_token arraym[j] " "
		#
		# Otherwise, process the specC note token
		#
		else
			{
			specC = 0
			#
			# If there is a valid specC expression, store and process it
			#
			if (match(arraym[j],floating_pt_num))
				{
				specC = substr(arraym[j],RSTART,RLENGTH) + 0
				sub(floating_pt_num,SUBSEP,arraym[j])
				#
				# Check for validity of log function
				#
				if (specC > 0)
					semits = log(specC/middle_C_freq)/log(1.059463094)
				else
					{
					print "semits: ERROR: Invalid negative specC value in"\
						 " line " NR "." > stderr
					exit
					}
				semits = sprintf("%." precision "f",semits)
				sub(SUBSEP,semits,arraym[j])
				return_token = return_token arraym[j] " "
				}
			#
			# If no specC value, the return token depends on -x option
			#
			else return_token = return_token arraym[j] " "
			}
		}	
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#############################################################################
##				Function Process_freq
#	This function takes frequency input and translates to semits
#
function process_freq(data_token,   j,return_token,split_num,arrayn,freq,semits)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayn," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Echo any rests that occur in the input
		#
		if (arrayn[j] ~ /r/) return_token = return_token arrayn[j] " "
		#
		# Otherwise, process the freq note token
		#
		else
			{
			freq = 0
			#
			# If there is a valid freq expression, store and process it
			#
			if (match(arrayn[j],floating_pt_num))
				{
				freq = substr(arrayn[j],RSTART,RLENGTH) + 0
				sub(floating_pt_num,SUBSEP,arrayn[j])
				#
				# Check for validity of log function
				#
				if (freq > 0)
					semits = log(freq/middle_C_freq)/log(1.059463094)
				else
					{
					print "semits: ERROR: Invalid negative freq value in"\
						 " line " NR "." > stderr
					exit
					}
				semits = sprintf("%." precision "f",semits)
				sub(SUBSEP,semits,arrayn[j])
				return_token = return_token arrayn[j] " "
				}
			#
			# If no freq value, the return token depends on -x option
			#
			else return_token = return_token arrayn[j] " "
			}
		}	
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#####################################################################
##				Function Process_semits
#	This function takes semits input and translates to semits
#
function process_semits(data_token,  j,num,arrayb,semits,return_token,split_num)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayb," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Echo any rests that occur in the input
		#
		if (arrayb[j] ~ /r/) return_token = return_token arrayb[j] " "
		#
		# Otherwise, process the semits note token
		#
		else
			{
			semits = 0
			#
			# If there is a valid semits expression, store and process it
			#
			if (match(arrayb[j],floating_pt_num))
				{
				semits = substr(arrayb[j],RSTART,RLENGTH) + 0
				semits = sprintf("%." precision "f",semits)
				sub(floating_pt_num,semits,arrayb[j])
				return_token = return_token arrayb[j] " "
				}
			#
			# If no semit value, the return token depends on -x option
			#
			else return_token = return_token arrayb[j] " "
			}
		}	
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

####################################################################
##				Function Process_cents
#	This function takes cents input and translates to semits
#
function process_cents(data_token, arraye,j,return_token,cents,split_num,semits)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arraye," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Echo any rests that occur in the input
		#
		if (arraye[j] ~ /r/) return_token = return_token arraye[j] " "
		#
		# Otherwise process the cents note token
		#
		else
			{
			cents = 0
			#
			# If there is a valid cents expression, store and process it
			#
			if (match(arraye[j],floating_pt_num))
				{
				cents = substr(arraye[j],RSTART,RLENGTH) + 0
				sub(floating_pt_num,SUBSEP,arraye[j])
				cents *= 0.01
				semits = sprintf("%." precision "f",cents)
				sub(SUBSEP,semits,arraye[j])
				return_token = return_token arraye[j] " "
				}
			#
			# If there is no cents token, output depends on -x option
			#
			else return_token = return_token arraye[j] " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

###########################################################################
##					Function Process_MIDI
#	This function takes MIDI input and translates to semits
#
function process_MIDI(data_token,    note,arrayk,j,return_token,split_num\
											,midi_array)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayk," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Process the MIDI data
		#
		split(arrayk[j],midi_array,"/")
		match(midi_array[2],/[+-]?[1-9][0-9]*/)
		note = substr(midi_array[2],RSTART,RLENGTH) + 0
		#
		# Only for positive midi notes is data output
		#
		if (note > 0)
			{
			note -= 60
			return_token = return_token sprintf("%." precision "f",note) " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}
	
###########################################################################
##					Function Process_fret
#	This function takes fret input and translates to semits
#
function process_fret(data_token,position,   strings,fret_num,semits,arrayl,i,j\
						    ,old_semits,return_token,temp_index,split_num)
	{
	return_token = ""
	old_semits = 0
	#
	# Echo any rests that occur in the input
	#
	if (data_token ~ /r/) return_token = data_token " "
	else
		{
		temp_index = fret_array[position]
		split_num = split(data_token,arrayl," ")
		#
		# Loop through each course for the spine
		#
		for (j = 1; j <= split_num; j += 1)
			{
			#
			# Only process the strings that were struck
			#
			if (arrayl[j] ~ /[^x:-]/)
				{
				#
				# There must be a value in array relative for each string
				#
				if (!((temp_index,j) in relative))
					{
					print "semits: ERROR: No pitch specified for string "\
						 "number " j " at line " NR "." > stderr
					exit
					}
				#
				# Loop through the separate strings in each course
				#
				for (i = 1; i <= split(relative[temp_index,j],strings,",");\
													        i += 1)
					{
					#
					# See if a fret is being pressed
					#
					if (match(arrayl[j],/[1-9][0-9]*/))
						{
						fret_num = substr(arrayl[j],RSTART,RLENGTH) + 0
						#
						# If a fret value is specified in fret_val, use it
						#
						if ((temp_index,fret_num) in fret_val)
							semits = absolute[temp_index] \
									+ strings[i] \
									+ fret_val[temp_index,fret_num]
						#
						# Otherwise, use a default
						#
						else semits = absolute[temp_index] \
									+ strings[i] + fret_num
						}
					#
					# Otherwise, just use the value of the string
					#
					else semits = absolute[temp_index] + strings[i]
					#
					# Construct returning token
					#
					if (semits != old_semits || return_token == "")
						return_token = return_token \
							sprintf("%." precision "f",semits) " "
					old_semits = semits
					}
				}
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}
