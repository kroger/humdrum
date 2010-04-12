#########################################################################
##						YDELTA.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# June 7/94	Tim Racinsky	Modified parse_command to work with getopts
#
#
#	This program is called from ydelta.ksh and its purpose is to determine
# the numerical differences between data tokens within the same record for
# a specific interpretation that is specified by the user.
#
# Main Functions used:
#	Parse_command()		Store_indicators()		Process_indicators()
#	Ins_array_pos()		Del_array_pos()		Num_of_exchanges()
#	Exchange_spines()		Join_paths()			Store_new_interps()
#	Process_data()			Purge_array()			Sort_the_array()
#	Find_min()
#
# Helper Functions used:
#	Print_interps()		Print_sort_array()
#
#	VARIABLES:
#
#  -NUMBER:  A regular expression that will identify a 'number' within 
#   a data token.
#  -latest_line: A string that holds the most recent line to be printed.  It
#   allows the program to print out the final '*-' without knowing how 
#   many lines are in the current input file.
#  -first_interpretation_yet:  A flag to indicate if the first exclusive 
#   interpretation has been encountered yet.
#  -spine_path_record:  A flag to indicate if the current interpretation record
#   is a spine-path record or not.
#  -current_no_of_spines:  Holds how many spines are currently active.
#  -interpretation:  holds the target interpretation for the program to key
#   on.  It is specified by the user.
#  -options:  holds the 's' option if specified.
#  -reg_exp:  holds the regular expression for the 's' option if specified.
#  -first_token:  holds the token in the first spine of a record that matches
#   the regular expression for the 's' option if specified.
#
#	ARRAYS:
#
#  -current_interp:  each element corresponds to a currently active spine and
#   either contains the target interpretation or nothing.
#  -path_indicator:  each element corresponds to a currently active spine and
#   contains the current spine-path indicator for that spine.
#  -sort_array:  for each data record, this array holds each separate token.
#   The array is then purged of non-numeric data and then sorted in 
#   ascending order.
#
BEGIN {
	USAGE="\nUSAGE: ydelta -h                   (Help Screen)\n       ydel"\
		 "ta [-s reg_exp] -i interpretation [file ...]"
	NUMBER = "[+-]?([0-9]+([.][0-9]+)?|[.][0-9]+)"
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	options = interpretation = reg_exp = ""
	latest_line = ""
	parse_command()
	}
{
#
# If this is the first record, reset variables and arrays.
#
if (FNR == 1)
	{
	if (latest_line != "") print "*-"
	first_interpretation_yet = spine_path_record = FALSE
	current_no_of_spines = 0
	}
else print latest_line
#
# If the current record matches the regular expression and the 's' option
# has been specified.
#
if (options ~ /s/ && $0 ~ reg_exp)
	{
	first_token = $1
	for (j = 2; j <= current_no_of_spines; j++)
		{
		#
		# If one of the spines contains a token not equal to the token in
		# the first spine, print the entire line with spaces instead of tabs
		#
		if ($j != first_token)
			{
			gsub(/\t/," ")
			latest_line = $0
			break
			}
		}
	#
	# Otherwise, print just the first token
	#
	if (j > current_no_of_spines) latest_line = first_token
	}
#
# Always print the global comments
#
else if ($0 ~ /^!!/) latest_line = $0
#
# If the current record is an interpretation record
#
else if ($0 ~ /^\*/)
	{
	#
	# If this is the first interpretation record print the new interpretation
	# title and set all of the necessary variables.
	#
	if (!first_interpretation_yet)
		{
		if (interpretation ~ /^\*\*/)
			latest_line = "**Y" substr(interpretation,3)
		else
			latest_line = "**Y" substr(interpretation,2)
		current_no_of_spines = NF
		first_interpretation_yet = TRUE
		store_new_interps()
		}
	#
	# If this is not the first interpretation record
	#
	else
		{
		latest_line = "*"
		#
		# Check if it is a spine path record or not
		#
		spine_path_record = FALSE
		for (i=1; i<=current_no_of_spines ; i++)
			{			
			if ($i ~ /^\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x$/) 
				{
				spine_path_record = TRUE 
				break
				}
			}
		#
		# If it is a spine path record, store and process the indicators and
		# reset the flag if necessary.
		#
		if (spine_path_record)
			{
			store_indicators()
			process_indicators()
			if (current_no_of_spines == 0) first_interpretation_yet = FALSE
			}
		#
		# Otherwise store the new interps.
		#
		else store_new_interps()
		}
	}
#
# For local comments, always print nothing
#
else if ($0 ~ /^![^!]/) latest_line = "!"
#
# For any other records, print the appropriate fields
#
else process_data()
}
END	{
	if (latest_line == "") ;
	else if (latest_line ~ /^!!/) print latest_line
	#
	# Print the spine-path terminator at the end of the file.
	#
	else if (latest_line != "*")
		{
		print latest_line
		print "*-"
		}
	else print "*-"
	}

######################################################################
#				FUNCTIONS USED IN YDELTA.AWK
######################################################################

#######################################################################
##				Function Parse_command()
#
function parse_command()
	{
	if (ARGV[2] != "null")
		{
		options = "s"
		reg_exp = ARGV[2]
		}
	if (ARGV[3] !~ /^\*\*/)
		{
		print "ydelta: ERROR: Invalid interpretation specified: "\
			  ARGV[3] "." > stderr
		exit
		}
	interpretation = ARGV[3]
	ARGV[1] = ARGV[2] = ARGV[3] = ""
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
		#
		#	If the current field is a null interpretation
		#
		if ($i == "*") path_indicator[i] = "*"
		#
		#	If the current field is a spine-path indicator, store it
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
function process_indicators(    i,exchanges,join)
	{
	#
	#	Loop through the entire array of 'path_indicator'
	#	NOTE: This array may be altered during the process
	#
	i = 1
	while (i <= current_no_of_spines)
		{
		#
		#	If a spine-split indicator occurred
		#
		if (path_indicator[i] == "^")
			{
			#
			#	Insert a new array position in 'path_indicator'
			# 	and 'current_interp'
			#
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = current_interp[i]
			}
		#
		#	If a spine-termination indicator occurred
		#
		if (path_indicator[i] == "-")
			{
			#
			#	Delete that spine from the arrays
			#
			del_array_pos(i)
			i--
			}
		#
		#	If a spine-exchange indicator occurred
		#
		if (path_indicator[i] == "x")
			{
			#
			#	Determine the number of exchange indicators that
			#	occur and store them in 'spines_to_exchange'
			#
			spines_to_exchange = ""
			exchanges = num_of_exchanges()
			exchange_spines()
			#	
			}
		#
		#	If a join-path indicator occurred
		#
		if (path_indicator[i] == "v")
			{
			#
			# Join the paths and delete the joined spine
			#
			join_paths(i,i+1)
			del_array_pos(i+1)
			#
			#	If the next indicator is not a join-path indicator
			#	then return to original conditions
			#
			if (path_indicator[i+1] != "v")
				path_indicator[i] = "*"
			#
			#	Otherwise, look at current and next paths to join
			#
			else i--
			}
		#
		#	If a new-path indicator occurred
		#
		if (path_indicator[i] == "+")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = ""
			}
		i++
		} #end while loop
	} #end function process_indicators

############################################################
##			Function Ins_array_pos
#	This function inserts new positions in the arrays 'path_indicator'
# and 'current_interp' and copies elements so that everything is preserved
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j--)
		{
		path_indicator[j] = path_indicator[j-1]
		current_interp[j] = current_interp[j-1]	
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
		current_interp[j] = current_interp[j+1]
		}
	delete path_indicator[j]
	delete current_interp[j]
	current_no_of_spines--
	} 

###############################################################
##			Function Num_of_exchanges
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
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in 'current_interp'
#
function exchange_spines(    arraya,temp)
	{
	split(spines_to_exchange,arraya)
	temp = current_interp[arraya[1]]
	current_interp[arraya[1]] = current_interp[arraya[2]]
	current_interp[arraya[2]] = temp
	}

############################################################
##				Function Join_paths
#	This function joins two paths.  If either of the original spines
# was specified to be marked, then the resulting spine should be marked too.
#
function join_paths(first_spine,second_spine)
	{
	if (current_interp[first_spine] == interpretation \
	    || current_interp[second_spine] == interpretation)
		current_interp[first_spine] = interpretation
	}

################################################################
##				Function Store_new_interps
#	This function stores only the interpretation specified by the user
# in the array current_interp for each currently active spine.
#
function store_new_interps(    j)
	{
	#
	#	Loop through every currently active spine.
	#	
	for (j = 1; j <= current_no_of_spines; j++)
		{
		#
		#	If the current field is the target interpretation, then store
		# it in the corresponding array position.
		#
		if ($j == interpretation) current_interp[j] = $j
		#
		# Otherwise, if the current field is an exclusive interpretation
		# and the current spine was marked, unmark the current spine.
		#
		else if ($j ~ /^\*\*/ && current_interp[j] == interpretation)
				current_interp[j] = ""
		}
	}

#############################################################
##				Function Process_data
#	This function takes the current data record and determines the
# numerical differences between the data tokens in the marked spines.
#
function process_data(     j,i,current_line,arrayb,sort_index)
	{
	#
	# Reset the current_line, and the sort_array
	#
	current_line = ""
	sort_index = 0
	sort_array[0]
	arrayb[0]
	for (j in sort_array) delete sort_array[j]
	#
	# Loop through each currently active spine.
	#
	for (j = 1; j <= current_no_of_spines; j++)
		{
		#
		# If the current spine is marked, then store the contents of
		# its data token in the array sort_array.
		#
		if (current_interp[j] == interpretation)
			{
			#
			# Split apart any double stops
			#
			for (i in arrayb) delete arrayb[i]
			split($j,arrayb," ")
			#
			# Place each double stop in a separate array position
			#
			for (i in arrayb) sort_array[++sort_index] = arrayb[i]
			}
		}
	#
	# If at least one token was stored in the array
	#
	if (sort_index > 0)
		{
		#
		# Purge all non-numeric data from the array
		#
		purge_array(sort_array)
		#
		# Sort the array into ascending order
		#
		sort_the_array(sort_index,sort_array)
		#
		# Loop through each array position and subtract the first element
		# from every other element.
		#
		for (j = 2; j <= sort_index; j++)
			{
			if (sort_array[j] != ".")
				sort_array[j] = sort_array[j] - sort_array[1]
			else break
			}
		#
		# If the first array position is a null token, then the entire output
		# token should also be a null token
		#
		if (sort_array[1] == ".") current_line = "."
		#
		# Otherwise, each element of the array should be output with the first
		# element appearing in square ([]) brackets.
		#
		else
			{
			current_line = "[" sort_array[1] "]"
			for (j = 2; j <= sort_index; j++)
				{
				if (sort_array[j] == ".") break
				current_line = current_line " " sort_array[j]
				}
			}
		}
	#
	# If no tokens were stored in the array, print a null token
	#
	else current_line = "."
	latest_line = current_line
	}	

######################################################################
##					Function Purge_array
#	This function removes all non-numeric data from the array sort_array
#
function purge_array(sort_array,   j)
	{
	for (j in sort_array)
		{
		#
		# If there is no number, entire contents are transferred
		#
		if (match(sort_array[j],NUMBER) == 0) sort_array[j] = "."
		#
		# Otherwise, separate the contents
		#
		else sort_array[j] = substr(sort_array[j],RSTART,RLENGTH)
		}
	}

#######################################################################
##				Function Sort_the_array
#	This function sorts the contents of the array sort_array in ascending
# order.
#
function sort_the_array(sort_index,sort_array,   j,temp_index,arrayc,min_index)
	{
	min_index = 0
	temp_index = 0
	#
	# Loop for as many elements are in sort_array
	#
	for (j = 1; j <= sort_index; j++)
		{
		#
		# Find the index of the smallest element in sort_array and store it
		# in the next position in arrayc and then delete the element from
		# sort_array.
		#
		min_index = find_min(sort_array)
		arrayc[++temp_index] = sort_array[min_index]
		delete sort_array[min_index]
		}
	#
	# Move the sorted contents of arrayc back into sort_array
	#
	for (j in arrayc) sort_array[j] = arrayc[j]
	}

########################################################################
##				Function Find_min
#	This function returns the index of the smallest numeric element in
# sort array.
#
function find_min(sort_array,    j,min,count)
	{
	count = 1
	#
	# Loop through each element in sort_array
	#
	for (j in sort_array)
		{
		#
		# If this is the first time through the loop, set the min variable
		# to the index of the current element.
		#
		if (count == 1)
			{ min = j ; count++ }
		#
		# Otherwise, if the current element is smaller than the element
		# at min or the element at min is a null token, set min to the index
		# of the current element.
		#
		else if ((sort_array[j] + 0 < sort_array[min] + 0 \
		         || sort_array[min] == ".") && (sort_array[j] != "."))
			min = j
		}
	return min
	}

#########################################################################
##			Helper Function Print_interps
#
function print_interps(        j)
	{
	print "Array current_interp"
	for(j in current_interp)
		print j, current_interp[j] 
	print "End Array current_interp"
	}

####################################################################
##			Helper Function Print_sort_array
#
function print_sort_array(   j)
	{
	print "Array sort_array"
	for (j in sort_array) print j, sort_array[j]
	print "End Array sort_array"
	}
