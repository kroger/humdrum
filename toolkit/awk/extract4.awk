#########################################################################
##                              EXTRACT4.AWK
# Programmed by: Tim Racinsky       Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#    Date:    Programmer:        Description:
#
#
#	This program implements the 'spine-path' mode of the extract program. It
# is called from extract.ksh and takes as its input a single path, indicated
# by a number.  The program simply follows that path all the way through the
# input file, making sure to follow all spine-path indicators.
#
# Main Functions used:
#	Initialize_field_array()	Print_fields()		Store_indicators()
#	Process_indicators()	Ins_array_pos() 	Del_array_pos()
#	Determine_exchanges() 	Exchange_spines()	Check_terminate() 
#
# Helper Functions used:
#	Print_field_array()		Print_path_indics()
#
#	VARIABLES:
#
#  -current_no_of_spines:  holds current number of active spines.
#  -first_interpretation_yet:  flag to indicate if first interpretation
#   record has been processed yet.
#  -spine_path_record:  flag to indicate if current interpretation record
#   is also a spine-path record.
#  -specified_field:  holds the spine number the user wishes to extract.
#
#	ARRAYS:
#
#  -field_array:  each element represents a currently active spine and contains
#   either a 'C' or an 'N' which represent 'cut' and 'no-cut' respectively.
#  -path_indicator:  holds spine-path indicators for each active spine of 
#   the current spine-path record.
#
BEGIN {
	#
	# Delete the DUMMY variable sent from the shell script
	#
	ARGV[1] = ""
	NUMBER = "^[1-9][0-9]*$"
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	# Check that the passed argument is a valid, single number
	#
	specified_field = 0
	check_argument(ARGV[2])
	}
{
#
# Reset variables for the first record of every file
#
if (FNR == 1)
	{
	current_no_of_spines = 0
	first_interpretation_yet = FALSE
	for (i in field_array) delete field_array[i]
	}
#
# Print all global comments
#
if ($1 ~ /^!!/) print $0
#
# For each interpretation record
#
if ($1 ~ /^\*/)
	{
	#
	# If this is the first interpretation record (exclusive)
	#
	if (!first_interpretation_yet)
		{
		current_no_of_spines = NF
		#
		# If the specified field is outside of valid spine numbers, error
		#
		if (specified_field > current_no_of_spines || specified_field <= 0) 	
			{
			print "extract: ERROR: Spine specified is outside of range." \
				 > stderr
			exit
			}
		#
		# Otherwise, initialize the array field_array
		#
		initialize_field_array(specified_field,current_no_of_spines)
		first_interpretation_yet = TRUE
		print_fields()
		}
	else
		{
		#
		# Otherwise check if it is an spine_path record
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
		# If it is, store and process the indicators
		#
		if (spine_path_record)
			{
			print_fields()
			store_indicators()
			process_indicators()
			if (current_no_of_spines == 0)
				{
				first_interpretation_yet = FALSE
				for (i in field_array) delete field_array[i]
				}
			}
		#
		# Otherwise, just print the fields
		#
		else print_fields()	
		}
	}
#
# For all other records, print the specified fields
#
else if ($1 !~ /^!!/) print_fields()
}

######################################################################
#				FUNCTIONS USED IN EXTRACT4
######################################################################

#####################################################################
##				Function Check_argument
#	This function simply checks that the given input is a single number
#
function check_argument(input)
	{
	if (input !~ NUMBER)
		{
		print "extract: ERROR: Invalid number given: " input > stderr
		exit
		}
	else 
		{
		specified_field = input
		ARGV[2] = ""
		}
	}

###################################################################
##			Function Initialize_field_array
#	This function initializes the array field_array so that the
# appropriate fields can be cut.
#
function initialize_field_array(cut_field,limit,     j)
	{
	#
	# Loop through the currently active spines
	#
	for (j = 1; j <= limit+0; j++)
		{
		if (j != cut_field) field_array[j] = "N"
		#
		# If the current field is the specified field, mark it as such
		#
		else field_array[j] = "C"
		}
	}

#############################################################
##			Function Print_fields
#	This function prints the marked fields according to field_array
#
function print_fields(     j,current_line)
	{
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if (field_array[j] == "C") current_line = current_line $j "\t"
		}
	if (current_line != "")
		print substr(current_line,1,length(current_line)-1)
	#
	# If the input record was a local comment record and the extracted record
	# is a global comment record, print a warning to standard error
	#
	if ($0 ~ /^![^!]/ && current_line ~ /^!!/)
		{
		print "extract: WARNING: Local comment record extracted as global "\
			 "comment at \n                  line " FNR "." > stderr
		}
	#
	# If the input record was a data record
	#
	else if ($0 !~ /^[!\*]/)
		{
		#
		# If the extracted record is a comment record, print a warning
		# to standard error
		#
		if (current_line ~ /^!/)
			{
			print "extract: WARNING: Data record has been extracted as "\
				 "comment record at \n                  line " FNR \
				 "." > stderr
			}
		#
		# If the extracted record is an interpretation record, print a warning
		# to standard error
		#
		else if (current_line ~ /^\*/)
			{
			print "extract: WARNING: Data record has been extracted as "\
				 "interpretation record at \n                  line " \
				 FNR "." > stderr
			}
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
	for (i=1; i<=current_no_of_spines; i++)
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
##			Function Process_indicators
#	This function takes the spine-path indicators that were stored
# in the array 'path_indicator' in the function 'store_indicators'
# and manipulates the arrays 'path_indicator' and 'current_interp'
# according to the contents of the array 'path_indicator'.
#
function process_indicators(    i)
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
			#
			# Also 'split' the specified field is necessary
			#
			if (field_array[i] == "C") field_array[i+1] = "C"
			else field_array[i+1] = "N"
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
			#
			# If no specified fields remain to be cut, then exit
			#
			# if (check_terminate()) exit
			}
		#
		if (path_indicator[i] == "x")
			{
			spines_to_exchange = ""
			if (determine_exchanges()) exchange_spines(spines_to_exchange)
			}
		#
		#	If a join-path indicator occurred
		#
		if (path_indicator[i] == "v")
			{
			#
			# If joining a specified field, result is also to be cut
			#
			if (path_indicator[i+1] == "v")
				{
				if (field_array[i+1] == "C") field_array[i] = "C"
				del_array_pos(i+1)
				i--
				}
			else
				{
				path_indicator[i] = "*"
				}
			}
		#
		#	If a new-path indicator occurred
		#
		if (path_indicator[i] == "+")
			{
			#
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			field_array[i+1] = "N"
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
		field_array[j] = field_array[j-1]	
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
		field_array[j] = field_array[j+1]
		}
	delete path_indicator[j]
	delete field_array[j]
	current_no_of_spines--
	}

###############################################################
##			Function Determine_exchanges
#	This function determines if an exchange needs to take place.
# Note that only if one of the paths to be exchanged is marked to
# be cut do we need to 'exchange' them.
#
function determine_exchanges(    j,cut)
	{
	cut = FALSE
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if (path_indicator[j] == "x")
			{
			if (field_array[j] == "C") cut = TRUE
			spines_to_exchange = spines_to_exchange j "\t"
			path_indicator[j] = "*"
			}
		}
	return cut
	}	

###################################################################
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in 'field_array'.
#
function exchange_spines(spines_to_exchange,    arraya,temp)
	{
	split(spines_to_exchange,arraya)
	temp = field_array[arraya[1]]
	field_array[arraya[1]] = field_array[arraya[2]]
	field_array[arraya[2]] = temp
	}

#################################################################
##				Function Check_terminate
#	This function determines if all specified fields have been
# terminated.
#
function check_terminate(    j,terminate)
	{
	terminate = TRUE
	for (j = 1; j <= current_no_of_spines; j++)
		{
		#
		# If there is a marked field that does not terminate, then
		# do not terminate the program
		#
		if (field_array[j] == "C" && path_indicator[j] != "-")
			{
			terminate = FALSE
			break
			}
		}
	return terminate
	}

###################################################################
#			Helper Function Print_field_array
function print_field_array(    j)
	{
	print "Array Field_Array"
	for (j in field_array) print j, field_array[j]
	}

######################################################################
##			Helper Function Print_path_indics
function print_path_indics(   j)
	{
	print "Array Path Indicator"
	for (j in path_indicator) print j, path_indicator[j]
	}				
