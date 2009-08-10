######################################################################
##					CBR.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# June 7/94	Tim Racinsky	Modified parse_command to work with getopts
#
#
#	This program is used to convert pitch units encoded  in various
# representations to cbr.
#
# Main Functions used:
#	parse_command()		store_new_interps()		store_indicators()
#	process_indicators()	ins_array_pos()		del_array_pos()
#	exchange_spines()		process_data()			convert_to_cbr()
#	process_cbr()			process_cocho()		process_freq()
#
#	VARIABLES:
#
#  -spine_path_record:  flag used to indicate if the current record
#   is a spine-path record or not.
#  -no_interpretation_yet:  flag used to indicate if an interpretation
#   record has been processed yet.
#  -current_no_of_spines:  holds the current number of active spines.
#  -stderr: allows print to be re-directed to standard error.
#  -floating_pt_num:  defines a valid floating point number.
#  -indicators: defines spine path indicators. 
#  -input_interps:  defines valid input interpretations to process.
#  -precision: holds precision of output tokens
#
#	ARRAYS:
#
#  -path_indicator and current_interp:  the standard arrays for
#   tracking interpretations.
#
BEGIN {
	USAGE="\nUSAGE: cbr -h                (Help Screen)\n       cbr [-p n] "\
		 "[-x] [file ...]\n"
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
	input_interps = "^(\\*\\*cbr|\\*\\*cocho|\\*\\*freq)$"
	#
	# Determine user's specified options
	#
	precision = 2
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
##			FUNCTIONS USED IN CBR.AWK
##################################################################

##################################################################
##			Function Parse_command
#	This function checks that the input passed from cbr.ksh
# contains a list of valid options and assigns it to the variable
# options if that is the case.
#
function parse_command()
	{
	if (ARGV[2] != "null")
		{
		if (ARGV[2] !~ /^[0-9]*$/)
			{
			print "cbr: ERROR: Invalid precision specified: " \
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
		# the output line must show cbr.
		#
		if ($j ~ input_interps) interp_line = interp_line "**cbr\t"
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
				# interpretation in order to obtain cbr output.
				#
				if (current_interp[j] == "**cocho")
					current_token = process_cocho($j)
				else if (current_interp[j] == "**freq")
					current_token = process_freq($j)
				else if (current_interp[j] == "**cbr")
					current_token = process_cbr($j)
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

########################################################################
##				Function Convert_to_cbr
#	This function accepts a frequency value from all the functions and
# converts it to cbr.
#
function convert_to_cbr(freq)
	{
	return \
		(sprintf("%." precision "f",21.4*(log(4.37*(freq/1000) + 1)/log(10))))
	}

#############################################################################
##				Function Process_freq
#	This function takes frequency input and translates to cbr
#
function process_freq(data_token,   j,return_token,split_num,arrayn,freq)
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
				# The given frequency must be greater than -228
				#
				if (freq < -228)
					{
					print "cbr: ERROR: Invalid negative freq value in "\
						 "line " NR "." > stderr
					exit
					}
				freq = convert_to_cbr(freq)
				sub(SUBSEP,freq,arrayn[j])
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

####################################################################
##				Function Process_cbr
#	This function takes cbr input and translates to cbr
#
function process_cbr(data_token,    cbr,freq,return_token,arrayf,j,split_num)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayf," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Echo any rests that occur in the input
		#
		if (arrayf[j] ~ /r/) return_token = return_token arrayf[j] " "
		#
		# Otherwise, process the cbr note token
		#
		else
			{
			cbr = 0
			#
			# If a valid cbr number exists, store and process it
			#
			if (match(arrayf[j],floating_pt_num))
				{
				cbr = substr(arrayf[j],RSTART,RLENGTH) + 0
				sub(floating_pt_num,SUBSEP,arrayf[j])
				cbr = sprintf("%." precision "f",cbr)
				sub(SUBSEP,cbr,arrayf[j])
				return_token = return_token arrayf[j] " "
				}
			#
			# If there is no cbr value present, output depends on -x option
			#
			else return_token = return_token arrayf[j] " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

######################################################################
##				Function Process_cocho
#	This function takes cocho input and translates to cbr
#
function process_cocho(data_token,   cocho,freq,return_token,arrayg,j,split_num)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayg," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Echo any rests that occur in the input
		#
		if (arrayg[j] ~ /r/) return_token = return_token arrayg[j] " "
		#
		# Otherwise, process the cochlear coordinate
		#
		else
			{
			cocho = 0
			#
			# If a valid cochlear coordinate exists, store an process it by
			# first converting it to frequency and then to freq
			#
			if (match(arrayg[j],floating_pt_num))
				{
				cocho = substr(arrayg[j],RSTART,RLENGTH) + 0
				sub(floating_pt_num,SUBSEP,arrayg[j])
				#
				# The given value must be within range
				#
				if (cocho < -3600 || cocho > 3600)
					{
					print "cbr: ERROR: Cocho value out of range in "\
						 "line " NR "." > stderr
					exit
					}
				freq = 165 * (10 ^ (0.06 * cocho) - 1)
				freq = convert_to_cbr(freq)
				sub(SUBSEP,freq,arrayg[j])
				return_token = return_token arrayg[j] " "
				}
			#
			# If no cochlear coordinate present, output depends on -x option
			#
			else return_token = return_token arrayg[j] " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}
