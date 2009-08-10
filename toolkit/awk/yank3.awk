####################################################################
##					YANK3.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# June 7/94	Tim Racinsky	Modified parse_command to work with getopts
#
#
#	This program implements the 'n' option or, the 'numbered marker' option,
# of the yank program. It is called from the program yank.ksh.
# It allows the user to specify a 'marker' and a range of numbers which
# represent numbered occurrences of that marker in the input.
# As with all yank programs, comments and interpretations 
# within the yanked material are output as well as all interpretations 
# before the yanked material.
# It should be noted that the '$' as a part of a range has no meaning
# within this version of yank.
#
# Main Functions used:
#	Parse_command()			Determine_range()
#	Separate_number_range()		Start_printing()
#	Stop_printing() 			Determine_number_of_fields()
#	Store_indicators()			Process_indicators()
#
# Helper Functions used:
#	Print_records_to_cut()
#
#	VARIABLES:
#
#  -RANGE:  holds the range of data records to yank between each successive
#   pair of regular expression records that is specified by the user.
#  -REGEXP:  holds the regular expression to search for.
#  -COMMENTS:  flag showing if user specified '-c' option or not.
#  -PRINTING:  flag indicating whether or not records are currently being
#   printed or not.
#  -STORING:  flag indicating if records should be stored or not.
#  -FIRST_MATCH_FOUND:  flag indicating when first record with regular
#   expression that is supposed to be printed has been found.
#  -ZERO_RANGE:  flag indicating whether first match of regular expression
#   has been found or not (note difference between first match that is 
#   supposed to be printed and first match of any record).
#  -number_of_fields:  holds current number of active spines.
#  -current_array_index: holds index of last element in store_array.
#  -number:  holds the 'number' of the current regular expression.
#  -current_file:  holds the name of the current file being processed. 
#  -spine_path_record: flag indicating if current interpretation record
#   is also a spine-path record.
#
#	ARRAYS:
#
#  -store_array:  holds all records since the most recent record has been
#   printed (and could possibly be the last one).  It is used when the
#   flag STORING is true.
#  -records_to_cut:  holds the numbers of the measures that the user wishes
#   to yank.
#
BEGIN {
	OPTIONS = "c"
	OPTIONS_REGEXP = "[" OPTIONS "]"
	OPTIONS_NEG_REGEXP = "[^" OPTIONS "]"
	USAGE = "\nUSAGE: yank -h|-?                    (Help Screen)\n       "\
		   "yank -l range [-c] [file ...]\n       yank -m regexp range [-c]"\
		   " [file ...]\n       yank -n regexp range [-c] [file ...]\n     "\
		   "  yank -o regexp [-e regexp] range [-c] [file ...]"
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	NUMBER = "^(0|[1-9][0-9]*)$"
	SEARCH_NUMBER = "^[1-9][0-9]*"
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	RANGE = ""
	REGEXP = ""
	COMMENTS = FALSE
	parse_command()
	determine_range(RANGE)
	}
#
# If looking at the first record of the current file, reset the variables
#
{
if (FNR == 1)
	{
	#
	# If the current file name has changed, then a terminating record
	# should be printed out.
	#
	if (current_file == "") current_file = FILENAME
	if (current_file != FILENAME)
		{
		terminate = ""
		for (j = 1; j <= number_of_fields; j++) terminate = terminate "*-\t"
		if (terminate != "") print substr(terminate,1,length(terminate)-1)
		current_file = FILENAME
		}
	PRINTING = STORING = FIRST_MATCH_FOUND = FALSE
	ZERO_RANGE = TRUE
	number_of_fields = 0
	current_array_index = 0
	number = ""
	for (j in store_array) delete store_array[j]
	}
#
# If the current line contains REGEXP
#
if ($0 ~ REGEXP)
	{
	#
	# If this is the first match, set the variable ZERO_RANGE
	#
	if (ZERO_RANGE) ZERO_RANGE = FALSE
	#
	# If records were being printed, stop printing them
	#
	if (PRINTING) PRINTING = FALSE
	#
	# Remove the part of the current line up to and including the 
	# matching expression
	#
	temp_string = substr($0,match($0,REGEXP)+RLENGTH)
	#
	# If the matched expression has a number with it
	#
	if (temp_string ~ SEARCH_NUMBER)
		{
		#
		# Assume the next set of digits make up the required number and
		# store those digits in the variable 'number'.
		#
		while (substr(temp_string,1,1) ~ /[0-9]/)
			{
			number = number substr(temp_string,1,1)
			temp_string = substr(temp_string,2)
			}
		number = number + 0
		#
		# If this is one of the numbers in the range, start printing records
		#
		if (number in records_to_cut) start_printing()
		else stop_printing()
		number = ""	
		}
	#
	# If there was no number with the REGEXP stop printing for now
	#
	else stop_printing()
	}
#
# Otherwise, if the current record is an interpretation
#
else if ($0 ~ /^\*/)
	{
	#
	# If the current records are being stored, store this as well
	#
	if (STORING) store_array[++current_array_index] = $0
	#
	# Otherwise, print this record
	#
	else
		{
		print $0
		determine_number_of_fields()
		}
	}
#
# If the current record is a comment
#
else if ($0 ~ /^!/)
	{
	#
	# If the first match has not yet been found,  either the -c had to be
	# chosen or the user wants to print material before first match.
	#
	if (!FIRST_MATCH_FOUND)
		{
		if ((COMMENTS) || ((ZERO_RANGE) && (0 in records_to_cut)))
			{
			print $0
			if ($0 !~ /^!!/) number_of_fields = NF
			}
		}
	#
	# Otherwise, print if Printing or store in array
	#
	else
		{
		if (PRINTING)
			{ 
			print $0
			if ($0 !~ /^!!/) number_of_fields = NF
			}
		else if (STORING) store_array[++current_array_index] = $0
		}
	}
#
# If it is a data record
#
else
	{
	#
	# If the first match has been found then print or store the record
	#
	if (FIRST_MATCH_FOUND)
		{
		if (PRINTING) { print $0 ; number_of_fields = NF }
		}
	#
	# Otherwise print the record if 'anacrusis' is desired
	#
	else if ((ZERO_RANGE) && (0 in records_to_cut))
		{
		print $0
		number_of_fields = NF
		}
	}
}
END	{
	terminate = ""
	for (j = 1; j <= number_of_fields; j++) terminate = terminate "*-\t"
	if (terminate != "") print substr(terminate,1,length(terminate)-1)
	}
	
###########################################################################
#			 	FUNCTIONS IN YANK3.AWK
###########################################################################

######################################################################
##				Function Parse_Command
#	This function determines what options the user wants
#
function parse_command()
	{
	if (ARGV[2] ~ /c/) COMMENTS = TRUE
	REGEXP = ARGV[3]
	RANGE = ARGV[4]
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ""
	if (RANGE ~ /\$/) 
		{
		print "yank: ERROR: Cannot specify '$' ranges for -n "\
			 "option." > stderr
		exit
		}
	}

#################################################################
##			Function Determine_range
#	This function determines the range of numbered markers that the
# user wishes to yank
#
function determine_range(range_string,    arraya,j)
	{
	#
	# Delete the old values
	#
	for (j in records_to_cut) delete records_to_cut[j]
	#
	# Place each separate range in an array element and parse
	#
	split(range_string,arraya,",")
	for (j in arraya)
		{
		#
		# If the current element is a range, parse it
		#
		if (arraya[j] ~ /[-]/) separate_number_range(arraya[j])
		#
		# Otherwise it must be a valid number
		#
		else if (arraya[j] !~ NUMBER)
			{
			print "yank: ERROR: Invalid number given: " arraya[j] \
				 > stderr
			exit
			}
		else records_to_cut[arraya[j]] = ""
		}
	}

##################################################################
##		 	Function Separate_number_range
#	This function splits apart a number range
#
function separate_number_range(current_string,    j,arrayb)
	{
	#
	# Place each number of the range in a separate array element
	#
	split(current_string,arrayb,"-")
	#
	# There cannot be more than two arguments in a range !
	#
	if ("3" in arrayb)
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}
	#
	# Both arguments must be valid numbers
	#
	else if (arrayb[1] !~ NUMBER || arrayb[2] !~ NUMBER)
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}	
	#
	# The first argument must be smaller than the second argument
	#
	else if (arrayb[1] > arrayb[2])
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}	
	else
		{
		for (j = arrayb[1]+0; j <= arrayb[2]+0; j++)
			records_to_cut[j] = ""
		}
	}

######################################################################
##				Function Start_printing
#	When a matching record (measure) is found that is to be printed,
# then the following function is carried out
#
function start_printing(     i)
	{
	#
	# If this was the first match, set the variable
	#
	if (!FIRST_MATCH_FOUND) FIRST_MATCH_FOUND = TRUE
	#
	# If records were being stored, print them out
	#
	if (STORING)
		{
		STORING = FALSE
		for (i = 1 ;; i++)
			{
			if (store_array[i] == "") break
			else print store_array[i]
			}
		for (i in store_array) delete store_array[i]
		current_array_index = 0
		}
	#
	# Set printing variable and print the current (matching record)
	#
	PRINTING = TRUE
	print $0
	if ($0 !~ /^!!/ && $0 !~ /^\*/) number_of_fields = NF
	else if ($0 ~ /^\*/) determine_number_of_fields()
	}

#######################################################################
##				Function Stop_printing
#	This function is called after a record matching REGEXP is found
# but is not a record(measure) to be yanked.
#
function stop_printing()
	{
	#
	# If this is not the first match and is a comment or interp. store it
	#
	if (FIRST_MATCH_FOUND) 
		{
		STORING = TRUE
		if ($0 ~ /^\*/ || $0 ~ /^!/)
			store_array[++current_array_index] = $0
		}
	# 
	# If the first match (to be yanked) as still not yet been found
	#
	else
		{
		#
		# Print all interpretations
		#
		if ($0 ~ /^\*/)
			{
			print $0
			determine_number_of_fields()
			}
		#
		# Print comments only if Comments option specified
		#
		else if ($0 ~ /^!/)
			{
			if (COMMENTS)
				{ 
				print $0
				if ($0 !~ /^!!/) number_of_fields = NF
				}
			}
		}
	}

########################################################################
##			Function Determine_number_of_fields
#	This function is used to determine if the current spine-path record
# consists of all spine-path terminators.  If it does, then the variables
# must be reset.  Otherwise, the number of resulting spines is calculated.
#
function determine_number_of_fields(   j,spine_path_record)
	{
	number_of_fields = NF
	spine_path_record = FALSE
	for (i = 1; i <= NF ; i++)
		{			
		if ($i ~ /^\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x$/)
			{
			spine_path_record = TRUE 
			break
			}
		}
	#
	# If it is a spine-path record, determine how many spines 
	# are added (or deleted) from the current number of fields
	#
	if (spine_path_record)
		{
		store_indicators()
		process_indicators()
		}
	if (number_of_fields == 0)
		{
		#
		# If records were being stored, delete those records
		#
		if (STORING)
			{
			for (i in store_array) delete store_array[i]
			current_array_index = 0
			}
		#
		# Reset all variables
		#
		FIRST_MATCH_FOUND = PRINTING = STORING = FALSE
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
	for (i = 1; i <= NF; i++)
		{
		if ($i == "*") path_indicator[i] = "*"
		else path_indicator[i] = substr($i,2,1)
		}
	}

##################################################################
##				Function Process_indicators
#	This function takes the spine-path indicators that were stored
# in the array 'path_indicator' in the function 'store_indicators'
# and manipulates the variable number_of_fields to reflect the number
# of fields that results from the spine-path indicators.
#
function process_indicators(    i)
	{
	for (i = 1; i <= NF; i++)
		{
		#
		# A field has been added or split, increase number of fields
		#
		if (path_indicator[i] == "^" || path_indicator[i] == "+")
			number_of_fields++
		#
		# A field has been terminated, decrease number of fields
		#
		if (path_indicator[i] == "-")
			number_of_fields--
		#
		# For join-path indicators, decrease number of fields only if
		# next indicator is also for join-path
		#
		if (path_indicator[i] == "v")
			{
			if (path_indicator[i+1] == "v")
				number_of_fields--
			}
		}
	}

########################################################################
##			function print_records_to_cut
#
function print_records_to_cut(   j)
	{
	print "Array Fields to cut"
	for (j in records_to_cut) print j
	print "End array fields to cut"
	}
