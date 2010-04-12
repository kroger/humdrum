#####################################################################
##					FILL.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#    Date:     Programmer:        	Description:
# May 12/94	Tim Racinsky		Added -c option
# June 7/94	Tim Racinsky		Modified parse_command to work with getopts
#
#
# 	This program replaces null tokens (".") in a humdrum file with 
# the voice's previous non-null token value.  Output is unpredicatable
# if null tokens occur at the beginning of humdrum spines.
#
# Main Functions used:
#	Parse_command()		Store_indicators()		Process_indicators()
#	Ins_array_pos()		Del_array_pos()		Num_of_exchanges()
#	Exchange_spines()		Join_paths()
#
# Helper Functions used:
#	Print_indics()
#
#	VARIABLES:
#
#  -no_interpretation_yet:  flag to indicate if first exclusive interpretation
#   has been found yet.
#  -current_no_of_spines:  holds current number of currently active spines.
#  -spine_path_record:  flag to indicate if current interpretation record
#   is a spine-path path record or not.
#  -regexp: holds regular expression specified by user.
#  -options:  hold options specified by the user.
#  -current_line:  holds current line to be printed.
#
#	ARRAYS:
#
#  -last_value:  each array element holds the value of the last non-null token
#   for each corresponding spine.
#  -new_value:  holds value of current tokens to print out for each spine.
#  -path_indicator:  holds any spine-path indicators for each spine.
#
BEGIN {
	USAGE="\nUSAGE: fill -h                (Help Screen)\n       fill [-p] "\
		 "[-s reg_exp] [-c chars] [file ...]"
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	options = ""
	regexp = ""
	chars = ""
	parse_command()
	}
{
#
# If this is the first line of the current file, reset the the variables
#
if (FNR == 1)
	{
	no_interpretation_yet = TRUE
	current_no_of_spines = 0
	spine_path_record = FALSE
	for (i in last_value) delete last_value[i]
	}
#
# Print all comments and barlines.
#
if ($0 ~ /^!/) print $0
#
# For each line of input that is an interpretation record
#
else if ($0 ~ /^\*/)
	{
	print $0
	#
	# If this is the first interpretation record
	#
	if (no_interpretation_yet)
		{
		#
		# Set the number of currently active spines and reset the variable,
		# and store the new interpretations
		#
		current_no_of_spines = NF
		for (i = 1; i <= current_no_of_spines; i++) last_value[i] = ""
		no_interpretation_yet = FALSE
		}
	#
	# If it is not the first interpretation record
	#
	else
		{
		#
		# Check to see if it is a spine-path record
		#
		spine_path_record = FALSE
		for (i = 1; i <= current_no_of_spines ; i++)
			{			
			if ($i ~ /^\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x$/) {
				spine_path_record = TRUE 
				break
				}
			}
		#
		# If it is a spine-path record
		#
		if (spine_path_record)
			{
			#
			# Store and process the spine-path indicators
			#
			store_indicators()
			process_indicators()
			#
			# If all of the spines have been terminated, set the flag
			# to show this.
			#
			if (current_no_of_spines == 0) no_interpretation_yet = TRUE
			}
		}
	}
#
# Print all records containing user's regular expression
#
else if (options ~ /s/ && $0 ~ regexp)
	{
	print $0
	}
#
# For all other records
#
else
	{
	#
	# Loop through each currently active spine.
	#
	for (i = 1; i <= current_no_of_spines; i++)
		{
		#
		# If the current spine contains a null token
		#
		if ($i == ".")
			{
			#
			# If the last value for this spine has not been set yet, 
			# output a null token anyway.
			#
			if (last_value[i] == "") new_value[i] = "."
			else
				{
				new_value[i] = last_value[i]
				#
				# If the -c option was specified, remove all characters
				# that the user did NOT specify from the token
				#
				if (options ~ /c/)
					{
					gsub(chars,"",new_value[i])
					if (new_value[i] == "") new_value[i] = "."
					}
				#
				# If the -p option was specified, place parentheses around
				# the value if it is not a null token
				#
				if (options ~ /p/ && new_value[i] != ".")
					new_value[i] = "(" new_value[i] ")"
				}
			}
		#
		# If it is not a null token, reset all the values
		#
		else new_value[i] = last_value[i] = $i
		}
	#
	# Print out the current line.
	#
	current_line = ""
	for (i = 1; i <= current_no_of_spines; i++)
		current_line = current_line new_value[i] "\t"
	print substr(current_line,1,length(current_line)-1)
	}
}

############################################################################
#				FUNCTIONS USED IN FILL.AWK
############################################################################

############################################################################
##				Function Parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	#
	# Set the 'options' variable and store the regular
	# expression and character options if specified
	#
	if (ARGV[2] != "null") options = ARGV[2]
	if (ARGV[3] != "null")
		{
		regexp = ARGV[3]
		options = options "s"
		}
	if (ARGV[4] != "null")
		{
		chars = ARGV[4]
		if (chars ~ /\\/)
			{ gsub(/\\/,"",chars) ; chars = "\\\\" chars }
		if (chars ~ /\]/)
			{ gsub(/]/,"",chars) ; chars = "]" chars }
		if (chars ~ /\^/)
			{ gsub(/\^/,"",chars) ; chars = chars "^" }
		if (chars ~ /-/)
			{ gsub(/-/,"",chars) ; chars = chars "-" }
		chars = "[^" chars "]"
		options = options "c"
		}
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ""
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
# and manipulates the arrays 'path_indicator' and 'last_value'
# according to the contents of the array 'path_indicator'.
#	The local variable 'join' indicates if a join-path is currently
# being performed.  The variable 'exchanges' holds the number of
# exchange-path indicators that were found.
#
function process_indicators(    i,exchanges)
	{
	i = 1
	#
	# Loop through each active spine, processing each indicator
	#
	while (i <= current_no_of_spines)
		{
		#
		# If the current indicator is a split-path indicator
		#
		if (path_indicator[i] == "^")
			{
			#
			# Create a new position in the arrays and copy the
			# interpretations of the original spine to the new spine
			#
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			last_value[i+1] = last_value[i]
			}
		#
		# If the current indicator is a spine-terminate indicator
		#
		if (path_indicator[i] == "-")
			{
			#
			# Delete the corresponding array positions
			#
			del_array_pos(i)
			i--
			}
		#
		# If the current indicator is an exchange-path indicator
		#
		if (path_indicator[i] == "x")
			{
			#
			# Determine the spines to exchange and exchange them (assumes
			# they are valid spines)
			#
			spines_to_exchange = ""
			exchanges = num_of_exchanges()
			exchange_spines()
			}
		#
		# If the current indicator is a join-path indicator
		#
		if (path_indicator[i] == "v")
			{
			#
			# Join the current spine with the next spine 
			#
			join_paths(i,i+1)
			del_array_pos(i+1)
			#
			# If there is another join-path indicator, be prepared to join
			# it to the current spine the next time through the loop
			#
			if (path_indicator[i+1] != "v")
				{
				path_indicator[i] = "*"
				join = FALSE
				}
			else i--
			}
		#
		# If the current indicator is a new-path indicator
		#
		if (path_indicator[i] == "+")
			{
			#
			# Create the new corresponding array position.  The next record
			# should contain the new interpretation
			#
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			last_value[i+1] = ""
			}
		i++
		}
	}

############################################################
##			Function Ins_array_pos
#	This function inserts new positions in the arrays 'path_indicator'
# and 'last_value' and copies elements so that everything is preserved
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j--)
		{
		path_indicator[j] = path_indicator[j-1]
		last_value[j] = last_value[j-1]	
		}
	path_indicator[current_element-1] = "*"
	current_no_of_spines++
	}

##############################################################
##				Function Del_array_pos
#	Performs the opposite of function 'ins_array_pos'.
#
function del_array_pos(current_element,     j)
	{
	for (j = current_element; j < current_no_of_spines; j++)
		{
		path_indicator[j] = path_indicator[j+1]
		last_value[j] = last_value[j+1]
		}
	delete path_indicator[j]
	delete last_value[j]
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
# elements in 'last_value'
#
function exchange_spines(    arraya,temp)
	{
	split(spines_to_exchange,arraya)
	temp = last_value[arraya[1]]
	last_value[arraya[1]] = last_value[arraya[2]]
	last_value[arraya[2]] = temp
	}

############################################################
##				Function Join_paths
#	This function actually joins two spines by concatentating
# the last values for each spine.
#
function join_paths(first_spine,second_spine)   
	{
	last_value[first_spine] = last_value[first_spine] \
						 " " last_value[second_spine]
	}

#####################################################################
##			Helper Function Print_indics
#
function print_indics(        j)
	{
	print "Array Path_Indicator"
	for(j in path_indicator)
		print j, path_indicator[j]
	print "End Array Path_Indicator"
	}
