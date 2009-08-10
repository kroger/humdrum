#####################################################################
##					XDELTA.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# June 7/94	Tim Racinsky	Modified parse_command to work with getopts
#
#
#	This program receives a valid humdrum file as input and calculates
# the changes in the numbers (the "delta x", so to speak) between records
# for each separate spine.  It is called from xdelta.ksh and is passed
# the corresponding options.
#
#	The following gives a brief summary of the available options:
#
#	-i include non-numeric data in output of processed tokens
#	-a print absolute values for output
#	-b regexp specify which tokens to stop processing on
#	-s regexp specify which tokens to ignore
#
# Main Functions used:
#	Parse_command()		Initialize_arrays()		Reset_arrays()
#	Purge_array()			Process_deltas()		Deltax()
#	Store_indicators()		Process_indicators() 	Ins_array_pos()
#	Del_array_pos()		Num_of_Exchanges()		Exchange_spines()
#	Join_paths()			Store_new_interps()		Find_number()
#
# Helper Functions used:
#	Print_indics()			Print_previous()
#	Print_current()		Print_token()
#
#	VARIABLES:
#
#  -spine_path_record:  flag to indicate if current interpretation record
#   is also a spine-path record.
#  -options:  holds the options the user selected.
#  -no_interpretation_yet:  flag to indicate if first interpretation record
#   has been parsed yet.
#  -current_no_of_spines:  holds current number of active spines.
#  -current_line:  string holding the current line of data tokens to be output.
#  -previous_max:  holds the number of 'stops' in the previous token for
#   the curent spine.
#  -number_in_previous:  flag to determine if any tokens in the previous record
#   for the current spine contained a number.
#  -current_max:  same as previous except for current token.
#  -number_in_current:  same as previous except for current token.
#  -breg_exp:  holds the regular expression that forces the program to break
#   on data tokens matching it.
#  -sreg_exp:  same as above except program 'skips' the data token.
#
#	ARRAYS:
#
#  -previous_tokens:  each element corresponds to the data token(s) for 
#   each spine of the previous record.
#  -previous_array:  when looping through each spine, this array holds the
#   tokens for the current spine that existed in the previous record (i.e. if 
#   there is a double stop then there are 2 array elements).
#  -previous_temp:  holds all non-numeric data from the array previous_array.
#   This allows only numbers to be printed or both numbers and non-number data.
#  -current_array:  same as previous_array except for current token.
#  -current_temp:  sames as preveious_temp except for current token.
#  -path_indicator:  holds spine-path indicators for current spine-path record.
#
BEGIN {
	USAGE="\nUSAGE: xdelta -h                    (Help Screen)\n       xde"\
		 "lta [-ae] [-b reg_exp] [-s reg_exp] [file ...]"
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	NUMBER = "[+-]?([0-9]+([.][0-9]+)?|[.][0-9]+)"
	options = breg_exp = sreg_exp ""
	previous_array[0] ;
	current_array[0] ;
	#
	# Determine the options specified by the user
	#
	parse_command()
	}
{
#
# If this is the first record of the current file, reset variables
#
if (FNR == 1)
	{
	no_interpretation_yet = TRUE
	current_no_of_spines = 0
	spine_path_record = FALSE
	initialize_arrays()
	reset_arrays()
	}
#
# Print each comment line without upsetting data values
#
if ($0 ~ /^!/) print $0
#
# If this is an interpretation record, print and process it
#
else if ($0 ~ /^\*/)
	{
	#
	# If this is the first inter. record, initialize variables
	#
	if (no_interpretation_yet)
		{
		current_no_of_spines = NF
		no_interpretation_yet = FALSE
		store_new_interps()
		initialize_arrays()
		#
		# All exclusive interpretations have an "X" prepended to them
		#
		for (i = 1; i <= current_no_of_spines; i++)
			{ if ($i ~ /^\*\*/) sub(/^\*\*/,"**X",$i) }	
		print $0
		}
	#
	# Otherwise, check to see if it is a spine-path record
	#
	else
		{
		spine_path_record = FALSE
		for (i=1; i<=current_no_of_spines ; i++)
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
			print $0
			store_indicators()
			process_indicators()
			if (current_no_of_spines == 0) no_interpretation_yet = TRUE
			}
		#
		# Otherwise, process the interpretations
		#
		else
			{
			store_new_interps()
			#
			# All exclusive interpretations have an "X" prepended to them
			#
			for (i = 1; i <= current_no_of_spines; i++)
				{
				if ($i ~ /^\*\*/) sub(/^\*\*/,"**X",$i)
				}	
			print $0
			}		
		}
	}
#
# Otherwise, it must be a data record
#
else
	{
	#
	# Use 'current_line' for printing the current record
	#
	current_line = ""
	#
	# Process each spine separately
	#
	for (outside = 1; outside <= current_no_of_spines; outside++)
		{
		if (options ~ /s/ && $outside ~ sreg_exp)
			{
			current_line = current_line $outside "\t"
			continue
			}
		else if (options ~ /b/ && $outside ~ breg_exp)
			{
			current_line = current_line $outside "\t"
			previous_tokens[outside] = ""
			continue
			}
		else if ($outside == ".")
			{
			current_line = current_line $outside "\t"
			continue
			}
		#
		# Reset all of the temporary arrays that are created
		#
		reset_arrays()
		#
		# Split the previous data token and the current data token by the
		# space so that multiple 'stops' may be processed separately. Also
		# purge the resulting arrays of non-numeric values and determine
		# the largest subscript in each array
		#
		previous_max = split(previous_tokens[outside],previous_array," ")
		number_in_previous = find_number(previous_array)
		purge_array(previous_array,previous_temp)
		#
		current_max = split($outside,current_array," ")
		number_in_current = find_number(current_array)
		purge_array(current_array,current_temp)
		#
		# If the previous token contains no numbers
		#
		if (!number_in_previous)
			{
			#
			# If the current token contains no numbers, just print the token
			#
			if (!number_in_current)
				current_line = current_line $outside "\t"
			#
			# Otherwise, print each numeric token in square brackets
			#
			else
				{
				for (j = 1; j <= current_max; j++)
					{
					#
					# If the user wants non-numeric data printed
					#
					if (options ~ /e/)
						{
						sub(SUBSEP,current_array[j],current_temp[j])
						if (current_array[j] != "")
							current_line = current_line "[" \
										current_temp[j] "] "
						else current_line = current_line \
										current_temp[j] " "
						}
					#
					# If the user wants only numbers printed
					#
					else
						{
						if (current_array[j] != "")
							current_line = current_line "[" \
										current_array[j] "] "
						else
							current_line = current_line \
										current_temp[j] " "
						}
					}
				current_line = substr(current_line,1\
							,length(current_line)-1) "\t"
				}
			}
		#
		# If the previous token does contain a number
		#
		else
			{
			#
			# If the current token also contains a number, process them
			#
			if (number_in_current)
				{
				process_deltas()
				}
			#
			# Otherwise, if the current token has no number, just print it
			#
			else current_line = current_line $outside "\t"
			}
		#
		# Store the current token as the previous token
		#
		previous_tokens[outside] = $outside
		}
	#
	# Print the resulting line
	#
	print substr(current_line,1,length(current_line)-1)
	}
}

############################################################################
#				FUNCTIONS USED IN XDELTA.AWK
############################################################################

############################################################################
##				Function Parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	if (ARGV[2] != "null") options = ARGV[2]
	if (ARGV[3] != "null")
		{
		options = options "b"
		breg_exp = ARGV[3]
		}
	if (ARGV[4] != "null")
		{
		options = options "s"
		sreg_exp = ARGV[4]
		}
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ""
	}

############################################################################
##				Function Initialize_arrays
#	This function initializes the previous_tokens to show no numbers
#
function initialize_arrays(    j)
	{
	for (j = 1; j <= current_no_of_spines; j++) previous_tokens[j] = ""
	}

########################################################################
##				Function Reset_arrays
#	This function resets the temporary arrays that are created for
# each separate spine.
#
function reset_arrays(    j)
	{
	for (j in previous_array) delete previous_array[j]
	for (j in current_array) delete current_array[j]
	for (j in previous_temp) delete previous_temp[j]
	for (j in current_temp) delete current_temp[j]
	}

######################################################################
##				Function Find_number
#	This funtion determines if the array passed to it contains any
# numbers in its elements.
#
function find_number(array,  number_found,j)
	{
	number_found = FALSE
	for (j in array)
		{
		if (array[j] ~ NUMBER)
			{
			number_found = TRUE
			break
			}
		}
	#
	# If no element was found to contain a number, return negative
	# Otherwise, return positive.
	#
	return number_found
	}

######################################################################
##				Function Purge_array
#	This function takes the array 'array' and removes all non-numeric
# material to array_temp.
#
function purge_array(array,array_temp,  j,temp)
	{
	#
	# Loop through the given array
	#
	for (j in array)
		{
		#
		# If there is no number, entire contents are transferred
		#
		if (match(array[j],NUMBER) == 0)
			{
			array_temp[j] = array[j]
			array[j] = ""
			}
		#
		# Otherwise, separate the contents
		#
		else
			{
			temp = array[j]
			array[j] = substr(array[j],RSTART,RLENGTH)
			sub(NUMBER,SUBSEP,temp)
			array_temp[j] = temp
			}
		}
	}

###################################################################
##				Function Process_deltas
#	This function determines the changes in the numbers between two
# data tokens.  Note that this may include multiple 'stops' and they
# are represented here as distinct array elements.  This function also
# continues to create 'current_line', the line that is printed to output.
# It uses the deltax() function to determine the actual token to print out
# and sends as parameters the array subscripts to process.
#
function process_deltas(    outside,inside,next_token)
	{
	#
	# If there are an equal number of 'stops' in the tokens, then match
	# them 1-1.
	#
	if (current_max == previous_max)
		{
		for (outside = 1; outside <= current_max; outside++)
			{
			next_token = deltax(outside,outside)
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
			for (inside = 1; inside <= previous_max; inside++)
				{
				next_token = deltax(outside,inside)
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
		current_line = current_line deltax(1,1) " "
		#
		# If the previous token had only 2 elements (Note that the current
		# token must have greater than 2 elements).
		#
		if (previous_max == 2)
			{
			for (outside = 2; outside < current_max; outside++)
				{
				for (inside = 1; inside <= previous_max; inside++)
					{
					next_token = deltax(outside,inside)
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
			for (outside = 1; outside <= current_max; outside++)
				{
				for (inside = 2; inside < previous_max; inside++)
					{
					next_token = deltax(outside,inside)
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
				for (inside = 2; inside < previous_max; inside++)
					{
					next_token = deltax(outside,inside)
					current_line = current_line "(" next_token ") "
					}
				}
			}
		current_line = current_line deltax(current_max,previous_max) " "
		}
	current_line = substr(current_line,1,length(current_line)-1) "\t"
	}

##########################################################################
##			 	Function Deltax
#	This function is called from Process_deltas() and is used to do the
# actual subtraction of the values.
#
function deltax(current,previous,   result,temp)
	{
	#
	# If current_array at the current position has no numeric data,
	# then simply return the non-numeric data
	#
	if (current_array[current] == "") return current_temp[current]
	#
	# Else, if previous_array at current position has no numeric data
	# 
	else if (previous_array[previous] == "")
		{
		#
		# If non-numeric data is to be output, return the original token
		# in square brackets.
		#
		if (options ~ /e/)
			{
			sub(SUBSEP,current_array[current],current_temp[current])
			return "[" current_temp[current] "]"
			}
		#
		# Otherwise, just print the numbers in square brackets.
		#
		else return "[" current_array[current] "]"
		}
	#
	# If both contain numeric data
	#
	else
		{
		result = current_array[current] - previous_array[previous] + 0
		if (options ~ /a/ && result < 0) result *= -1
		if (options ~ /e/)
			{
			temp = current_temp[current]
			sub(SUBSEP,result,temp)
			return temp
			}
		else return (result)
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
		if ($i == "*") path_indicator[i] = "*"
		else path_indicator[i] = substr($i,2,1)
		}
	}

##################################################################
##			Function Process_indicators
#	This function takes the spine-path indicators that were stored
# in the array 'path_indicator' in the function 'store_indicators'
# and manipulates the arrays 'path_indicator' and 
# according to the contents of the array 'path_indicator'.
#
function process_indicators(    i)
	{
	i = 1
	while (i <= current_no_of_spines)
		{
		#
		# For split-paths, both of the resulting paths use the value of
		# the path before the split
		#
		if (path_indicator[i] == "^")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			previous_tokens[i+1] = previous_tokens[i]
			}
		#
		# For spine termination, terminate everything for that path
		#
		if (path_indicator[i] == "-")
			{
			del_array_pos(i)
			i--
			}
		#
		# For exchange paths, exchange the previous values of the paths
		#
		if (path_indicator[i] == "x")
			{
			spines_to_exchange = ""
			num_of_exchanges()
			exchange_spines()
			}
		#
		# For join paths, join the values of the original paths to
		# form a multiple 'stop' token.
		#
		if (path_indicator[i] == "v")
			{
			join_paths(i,i+1)
			del_array_pos(i+1)
			if (path_indicator[i+1] != "v")
				{
				path_indicator[i] = "*"
				join = FALSE
				}
			else i--
			}
		#
		# For a new path, start the path as fresh
		#
		if (path_indicator[i] == "+")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			previous_tokens[i+1] = ""
			}
		i++
		} #end while loop
	} #end function process_indicators

############################################################
##			Function Ins_array_pos
#	This function inserts new positions in the arrays 'path_indicator'
# and and 'previous_tokens' copies elements so that everything is preserved
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j--)
		{
		path_indicator[j] = path_indicator[j-1]
		previous_tokens[j] = previous_tokens[j-1]
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
		previous_tokens[j] = previous_tokens[j+1]
		}
	delete path_indicator[j]
	delete previous_tokens[j]
	current_no_of_spines--
	} 

###############################################################
##			Function Num_of_exchanges
#	This function determines how  many exchange-path indicators
# appear in the current spine-path record and stores them in
# the variable 'spines_to_exchange'.
#
function num_of_exchanges(    j)
	{
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if (path_indicator[j] == "x")
			{
			spines_to_exchange = spines_to_exchange j "\t"
			path_indicator[j] = "*"
			}
		}
	return
	}	

###################################################################
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in 'previous_tokens'.
#
function exchange_spines(    arraya,temp)
	{
	split(spines_to_exchange,arraya)
	temp = previous_tokens[arraya[1]]
	previous_tokens[arraya[1]] = previous_tokens[arraya[2]]
	previous_tokens[arraya[2]] = temp
	}

############################################################
##				Function Join_paths
#	This function appends tokens of the joined spines to form a single
# token as a multiple stopped token.
#
function join_paths(first_spine,second_spine,     i,j)
	{
	previous_tokens[first_spine] = previous_tokens[first_spine] \
				" " previous_tokens[second_spine]
	}

################################################################
##				Function Store_new_interps
#	This function resets a spine if an exclusive interpretation is found.
#
function store_new_interps(      j)
	{
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if ($j ~ /^\*\*/) previous_tokens[j] = ""
		}
	}

#####################################################################
##			Helper Function Print_indics
#
function print_indics(        j)
	{
	print "Array Path_Indicator"
	for (j in path_indicator) print j, path_indicator[j]
	print "End Array Path_Indicator"
	}

######################################################################
##			Helper Function Print_previous
#
function print_previous(   j)
	{
	print "Array Previous_array"
	for (j in previous_array) print j, previous_array[j]
	print "End Array Previous_array"
	}

######################################################################
##			Helper Function Print_current
#
function print_current(   j)
	{
	print "Array Current_array"
	for (j in current_array) print j, current_array[j]
	print "End Array Current_array"
	}

######################################################################
##			Helper Function Print_token
#
function print_token(element)
	{
	print "Previous_tokens element"
	print element, previous_tokens[element]
	print "End Previous_tokens element"
	}
