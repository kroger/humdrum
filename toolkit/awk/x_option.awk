######################################################################
##					X_OPTION.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# June 23/94	Tim Racinsky	Changed solfg representation to include "-"
#
#
#	This function is used is a preprocessor when the -x option is invoked
# for the pitch conversion programs.  It merely strips away all data that
# is not part of the note for the given representation.
#
# Main Functions used:
#	parse_command()		store_new_interps()		store_indicators()
#	process_indicators()	ins_array_pos()		del_array_pos()
#	exchange_spines()		process_data()			process_kern()
#	process_pitch()		process_Tonh()			process_solfg()
#	process_numeric()
#	
#
#	VARIABLES:
#
#  -spine_path_record:  flag used to indicate if the current record
#   is a spine-path record or not.
#  -no_interpretation_yet:  flag used to indicate if an interpretation
#   record has been processed yet.
#  -current_no_of_spines:  holds the current number of active spines.
#  -options:  holds which options the user has specified.
#  -floating_pt_num:  defines a valid floating point number.
#  -deviation:  defines a valid equally-tempered deviation.
#  -indicators: defines spine path indicators. 
#  -input_interps:  defines valid input interpretations to process.
#  -octave_class: defines a valid octave class specification
#
#	ARRAYS:
#
#  -path_indicator and current_interp:  the standard arrays for
#   tracking interpretations.
#
BEGIN {
	#
	# Delete the DUMMY variable sent from the shell script
	#
	ARGV[1] = ""
	#
	# Set global variables and expressions
	#
	FS = OFS = "\t"
	TRUE = 1 ; FALSE = 0
	floating_pt_num = "[+-]?(([0-9]+(\\.[0-9]*)?)|((0*)?\\.[0-9]*))"
	deviation = "[+-][1-9][0-9]*"
	tie_reg = "\\]|\\[|_"
	indicators = "^(\\*\\+|\\*\\-|\\*\\^|\\*v|\\*x)$"
	input_interps = "^(\\*\\*specC|\\*\\*kern|\\*\\*pitch|\\*\\*Tonh"\
				 "|\\*\\*solfg|\\*\\*semits|\\*\\*cents|\\*\\*cbr"\
				 "|\\*\\*barks|\\*\\*cocho|\\*\\*freq|\\*\\*pc)$"
	kern_pitch = "a+|b+|c+|d+|e+|f+|g+|A+|B+|C+|D+|E+|F+|G+"
	pitch_pitch = "[ABCDEFG]"
	Tonh_pitch = "Es|As|[ABCDEFGHS]"
	solfg_pitch = "do|re|mi|fa|sol|la|si"
	octave_class = "[0-9]"
	pc_number = "(1[01])|[0-9]|([AB])|([TE])"
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
##			FUNCTIONS USED IN X_OPTION.AWK
##################################################################

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
		# Store only exclusive interpretations.
		#
		if ($j ~ /^\*\*/) current_interp[j] = $j
		}
	print $0
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
#	This function inserts new positions in the arrays 'path_indicator' and
# 'current_interp' and copies elements so that 
# everything is preserved
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
				# interpretation in order to obtain the 'stripped' output.
				#
				if (current_interp[j] == "**kern")
					current_token = process_kern($j)
				else if (current_interp[j] == "**pitch")
					current_token = process_pitch($j)
				else if (current_interp[j] == "**Tonh")
					current_token = process_Tonh($j)
				else if (current_interp[j] == "**solfg")
					current_token = process_solfg($j)
				else if (current_interp[j] == "**MIDI")
					current_token = process_MIDI($j)
				else if (current_interp[j] == "**pc")
					current_token = process_pc($j)
				else current_token = process_numeric($j)
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
#	This function takes kern input and strips away none-note data
#
function process_kern(data_token,  arrayc,j,return_token,note,split_num)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayc," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Simply output any rests
		#
		if (arrayc[j] ~ /r/) return_token = return_token "r "
		#
		# Otherwise, process the note
		#
		else
			{
			if (match(arrayc[j],kern_pitch))
				{
				note = substr(arrayc[j],RSTART,RLENGTH)
				sub(kern_pitch,"",arrayc[j])
				if (match(arrayc[j],/(-+)|(#+)/))
					note = note substr(arrayc[j],RSTART,RLENGTH)
				if (match(arrayc[j],tie_reg))
					note = note substr(arrayc[j],RSTART,RLENGTH)
				return_token = return_token note " "
				}
			}
		}
	#
	# Return at least a null token
	#
	if (return_token == "") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#########################################################################
##				Function Process_pitch
#	This function accepts pitch input and strips away non-note data
#
function process_pitch(data_token,  return_token,note,arrayd,j,split_num)
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
		if (arrayd[j] ~ /r/) return_token = return_token "r "
		#
		# Otherwise, process the pitch note
		#
		else
			{
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arrayd[j],pitch_pitch))
				{
				note = substr(arrayd[j],RSTART,RLENGTH)
				sub(pitch_pitch,"",arrayd[j])
				if (match(arrayd[j],/(b+)|([x#]+)/))
					{
					note = note substr(arrayd[j],RSTART,RLENGTH)
					sub(/(b+)|([x#]+)/,"",arrayd[j])
					}
				if (match(arrayd[j],deviation))
					{
					note = note substr(arrayd[j],RSTART,RLENGTH)
					sub(deviation,"",arrayd[j])
					}
				if (match(arrayd[j],octave_class)) 
					note = substr(arrayd[j],RSTART,RLENGTH) note
				return_token = return_token note " "
				}
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "") return "."
	else return substr(return_token,1,length(return_token)-1)
	}
	
###########################################################################
##					Function Process_Tonh
#	This function takes Tonh input and strips away none-note data
#
function process_Tonh(data_token,    note,arrayi,j,return_token,split_num)
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
		if (arrayi[j] ~ /r/) return_token = return_token "r "
		#
		# Otherwise, process the Tonh note
		#
		else
			{
			if (match(arrayi[j],Tonh_pitch))
				{
				note = substr(arrayi[j],RSTART,RLENGTH)
				sub(Tonh_pitch,"",arrayi[j])
				if (match(arrayi[j],/(es)+|(is)+/))
					{
					note = note substr(arrayi[j],RSTART,RLENGTH)
					sub(/(es)+|(is)+/,"",arrayi[j])
					}
				if (match(arrayi[j],deviation))
					{
					note = note substr(arrayi[j],RSTART,RLENGTH)
					sub(deviation,"",arrayi[j])
					}
				if (match(arrayi[j],octave_class)) 
					note = substr(arrayi[j],RSTART,RLENGTH) note
				return_token = return_token note " "
				}
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

###########################################################################
##					Function Process_solfg
#	This function takes solfg input and strips away non-note data
#
function process_solfg(data_token,    note,arrayj,j,return_token,split_num)
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
			return_token = return_token "r "
		#
		# Otherwise, process the solfg note
		#
		else
			{
			if (match(arrayj[j],solfg_pitch))
				{
				note = substr(arrayj[j],RSTART,RLENGTH)
				sub(solfg_pitch,"",arrayj[j])
				if (match(arrayj[j],/(~b+)|(~d+)/))
					{
					note = note substr(arrayj[j],RSTART,RLENGTH)
					sub(/(~b+)|(~d+)/,"",arrayj[j])
					}
				if (match(arrayj[j],deviation))
					{
					note = note substr(arrayj[j],RSTART,RLENGTH)
					sub(deviation,"",arrayj[j])
					}
				if (match(arrayj[j],octave_class)) 
					note = substr(arrayj[j],RSTART,RLENGTH) note
				return_token = return_token note " "
				}
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#############################################################################
##				Function Process_pc
#	This function takes pc input and strips away non-pc data
#
function process_pc(data_token,   j,return_token,split_num,arrayn,note)
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
		if (arrayn[j] ~ /r/) return_token = return_token "r "
		#
		# Otherwise, process the note token
		#
		else
			{
			#
			# If there is a valid numeric expression, store and process it
			#
			if (match(arrayn[j],pc_number))
				{
				note = substr(arrayn[j],RSTART,RLENGTH)
				return_token = return_token note " "
				}
			}
		}	
	#
	# At least return a null token
	#
	if (return_token == "") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#############################################################################
##				Function Process_numeric
#	This function takes any other numeric input and strips away non-note data
#
function process_numeric(data_token,   j,return_token,split_num,arraym,note)
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
		if (arraym[j] ~ /r/) return_token = return_token "r "
		#
		# Otherwise, process the note token
		#
		else
			{
			#
			# If there is a valid numeric expression, store and process it
			#
			if (match(arraym[j],floating_pt_num))
				{
				note = substr(arraym[j],RSTART,RLENGTH)
				return_token = return_token note " "
				}
			}
		}	
	#
	# At least return a null token
	#
	if (return_token == "") return "."
	else return substr(return_token,1,length(return_token)-1)
	}
