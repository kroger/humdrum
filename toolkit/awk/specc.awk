######################################################################
##					SPECC.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# June 9/94	Tim Racinsky	Modified parse_command to work with getopts
#
#
#	This program is used to convert pitch units encoded  in various
# representations to spectral centroid.
#
# Main Functions used:
#	parse_command()		store_new_interps()			store_indicators()
#	process_indicators()	ins_array_pos()			del_array_pos()
#	exchange_spines()		process_specC()			process_freq()
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
#  -floating_pt_num:  defines a valid floating point number.
#  -indicators: defines spine path indicators. 
#  -input_interps:  defines valid input interpretations to process.
#  -precision: specifies precision of output token
#
#	ARRAYS:
#
#  -path_indicator and current_interp:  the standard arrays for
#   tracking interpretations.
#
BEGIN {
	USAGE="\nUSAGE: specc -h                (Help Screen)\n       specc "\
		 "[-p n] [-x] [file ...]\n"
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
	floating_pt_num = "[+-]?(([0-9]+(\\.[0-9]*)?)|((0*)?\\.[0-9]*))"
	indicators = "^(\\*\\+|\\*\\-|\\*\\^|\\*v|\\*x)$"
	input_interps = "^(\\*\\*specC|\\*\\*freq)$"
	#
	# Determine user's specified options
	#
	precision = 2
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
##			FUNCTIONS USED IN SPECC.AWK
##################################################################

##################################################################
##			Function Parse_command
#	This function checks that the input passed from specc.ksh
# contains a list of valid options and assigns it to the variable
# options if that is the case.
#
function parse_command()
	{
	#
	# Set the options variable if not null
	#
	if (ARGV[2] != "null")
		{
		if (ARGV[2] !~ /^[0-9]*$/)
			{
			print "specc: ERROR: Invalid precision specified: " \
				 ARGV[2] > stderr
			print USAGE > stderr
			exit
			}
		else precision = ARGV[2] + 0
		}
	ARGV[1] = ARGV[2] = ""
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
		# If the interpretation is a 'convertible' interpretation, then
		# the output line must show specC.
		#
		if ($j ~ input_interps) interp_line = interp_line "**specC\t"
		else interp_line = interp_line $j "\t"
		}
	#
	# Print the new interpretation line
	#
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
# and manipulates the arrays 'path_indicator', and 'current_interp'
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
# 'current_interp' and copies elements so that everything is preserved
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
				# interpretation in order to obtain specc output.
				#
				if (current_interp[j] == "**freq")
					current_token = process_freq($j)
				else if (current_interp[j] == "**specC")
					current_token = process_specC($j)
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

#############################################################################
##				Function Process_specC
#	This function takes specC input and translates to specC
#
function process_specC(data_token,   j,return_token,split_num,arrayb,specC)
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
		# Otherwise, process the specC note token
		#
		else
			{
			specC = 0
			#
			# If there is a valid specC expression, store and process it
			#
			if (match(arrayb[j],floating_pt_num))
				{
				specC = substr(arrayb[j],RSTART,RLENGTH) + 0
				sub(floating_pt_num,SUBSEP,arrayb[j])
				specC = sprintf("%." precision "f",specC)
				sub(SUBSEP,specC,arrayb[j])
				return_token = return_token arrayb[j] " "
				}
			#
			# If no specC value, the return token depends on -x option
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

#############################################################################
##				Function Process_freq
#	This function takes frequency input and translates to specC
#
function process_freq(data_token,   j,return_token,split_num,arrayc,freq)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayc," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Echo any rests that occur in the input
		#
		if (arrayc[j] ~ /r/) return_token = return_token arrayc[j] " "
		#
		# Otherwise, process the freq note token
		#
		else
			{
			freq = 0
			#
			# If there is a valid freq expression, store and process it
			#
			if (match(arrayc[j],floating_pt_num))
				{
				freq = substr(arrayc[j],RSTART,RLENGTH) + 0
				sub(floating_pt_num,SUBSEP,arrayc[j])
				freq = sprintf("%." precision "f",freq)
				sub(SUBSEP,freq,arrayc[j])
				return_token = return_token arrayc[j] " "
				}
			#
			# If no freq value, the return token depends on -x option
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
