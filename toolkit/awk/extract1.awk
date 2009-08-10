#########################################################################
##                              EXTRACT1.AWK
# Programmed by: Tim Racinsky       Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#    Date:    Programmer:        Description:
#
#
#  	This program implements the 'field' mode of the extract program.  It
# is called from extract.ksh and accepts as input a range of values and
# a list of filenames.
#
# Main Functions used:
#	Determine_range() 		Parse_dollar_range()	Separate_number_range()
#	Print_fields() 		Store_indicators()		Process_indicators()
#	Ins_array_pos()		Del_array_pos()
#
# Helper Functions:
#	Print_fields_to_cut()		Print_field_array()
#	
#	VARIABLES:
#
#  -first_interpretation_yet:  flag that indicates if first interpretation
#   record has been parsed yet.
#  -spine_path_record:  flag to indicate if current interpretation record
#   is a spine-path record as well.
#  -current_no_of_spines:  holds current number of active spines.
#  -range_value: holds the range of numbers entered on command line.
#
#	ARRAYS:
#
#  -left_fields_to_cut:  holds which fields are to be cut with respect to
#   the left-most spine in the input.
#  -right_fields_to_cut:  holds which fields are to be cut with respect
#   to the right most spine in the input.
#  -field_array:  each element represents a currently active spine and is
#   either a 'C' or an 'N' indicating 'cut' or 'no-cut' respectively.
#  -path_indicator:  holds spine-path indicators to be processed.
#
BEGIN {
	#
	# Delete the DUMMY variable sent from the shell script
	#
	ARGV[1] = ""
	FS = OFS = "\t"
	NUMBER = "^[1-9][0-9]*$"
	TRUE = 1; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	# Determine which fields are to be cut w.r.t. both sides of the input
	#
	range_value = ARGV[2]
	ARGV[2] = ""
	}
{
#
# Reset variables and arrays at the start of every new file
#
if (FNR == 1)
	{
	first_interpretation_yet = FALSE
	current_no_of_spines = 0
	}
#
# If a global comment is encountered, print it
#
if ($1 ~ /^!!/) print $0
#
# If an interpretation record is encountered, process it
#
if ($1 ~ /^\*/)
	{
	#
	# If the first exclusive interpretation has not yet been seen, this
	# must be it
	#
	if (!first_interpretation_yet)
		{
		#
		# Set all necessary variables and delete all elements from the 
		# existing arrays.  Then the range is determined and the array
		# field array is created.
		#
		first_interpretation_yet = TRUE
		current_no_of_spines = NF
		for (i in left_fields_to_cut) delete left_fields_to_cut[i]
		for (i in right_fields_to_cut) delete right_fields_to_cut[i]
		for (i in field_array) delete field_array[i]
		determine_range(range_value)
		print_fields()
		}
	else
		{
		#
		# If it's not the first record, determine if it's a spine-path record
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
		# If it is a spine-path record, process the spine path indicators
		#
		if (spine_path_record)
			{
			print_fields()
			store_indicators()
			process_indicators()
			#
			# If all of the spines were terminated, reset values
			#
			if (current_no_of_spines == 0)
				first_interpretation_yet = FALSE
			}
		#
		# Otherwise, just print the appropriate fields
		#
		else print_fields()	
		}
	}
# For any other type of record, just print the fields
else if ($1 !~ /^!!/) print_fields()
}

######################################################################
#			FUNCTIONS USED IN EXTRACT1.AWK
######################################################################

######################################################################
##			Function Determine_range
# This function takes as input the list of spines-to-cut that were
# passed from the spine.ksh shell.  It makes use of the functions
# parse_dollar_range(), and separate_number_range().
#
function determine_range(range_string,    arraya,j)
	{
	# 
	# Place each separate 'range' into an array element and then process
	#
	split(range_string,arraya,",")
	for (j in arraya)
		{
		#
		# If there is a dollar in the range, then it must be referencing
		# spines with respect to the right hand side
		#
		if (arraya[j] ~ /\$/) parse_dollar_range(arraya[j])
		# 
		# If there is a "-" character, then it must be a range of numbers
		#
		else if (arraya[j] ~ /[-]/) separate_number_range(arraya[j])
		#
		# Otherwise, a single element must be a valid number
		#
		else if (arraya[j] !~ NUMBER)
			{
			print "extract: ERROR: Invalid number given: " arraya[j] \
				 > stderr
			exit
			}
		#
		# If it is a valid number, then it is referenced w.r.t
		# the left hand side
		#
		else if (arraya[j] <= current_no_of_spines)
			field_array[arraya[j]] = "C"
		}
	for (j = 1; j <= current_no_of_spines; j++)
		if (field_array[j] != "C") field_array[j] = "N"
	}

##################################################################
##			Function Parse_dollar_range
#	This function accepts a range of numbers from determine_range() that
# are referencing spines w.r.t. to the right hand side of the input.
# It fills in the appropriate values in the array field_array.
#
function parse_dollar_range(current_string,       arrayb,j,num1,num2)
	{
	#
	# Split the passed parameter according to the "-" character so that each
	# element either holds a number or a dollar sign
	#
	split(current_string,arrayb,"-")
	#
	# If there are more than 4 elements in the array, the range is wrong
	#
	if ("5" in arrayb)
		{
		print "extract: ERROR: Invalid range given: " current_string > \
			 stderr
		exit
		}
	#
	# If there are four elements in the array
	#
	if ("4" in arrayb)
		{
		#
		# The first and third elements must be dollars and the second and 
		# fourth must be numbers with the second being larger than the fourth
		#
		if ((arrayb[1] != "$" || arrayb[3] != "$") \
			|| (arrayb[2] !~ NUMBER || arrayb[4] !~ NUMBER) \
			|| (arrayb[2] < arrayb[4]))
			{
				print "extract: ERROR: Invalid range given: " \
					  current_string > stderr
				exit
			}
		else
			{ 
			#
			# Simply create the appropriate array subscripts
			#
			if (current_no_of_spines-arrayb[2]+0 < 1)
				num2 = current_no_of_spines-1
			else
				num2 = arrayb[2]+0
			for (j = arrayb[4]+0; j <= num2; j++)
				field_array[current_no_of_spines-j] = "C"
			}
		}
	#
	# Otherwise, if there are only three elements in the array
	#
	else if ("3" in arrayb)
		{
		#
		# The first and third elements must be dollars and the second
		# element must be a number
		#
		if (arrayb[1] != "$" || arrayb[3] != "$" || arrayb[2] !~ NUMBER)
			{
			print "extract: ERROR: Invalid range given: " \
                         current_string > stderr
			exit
			}
		else
			{
			#
			# Again, simply create the appropriate array subscripts
			#
			if (current_no_of_spines-arrayb[2]+0 < 1)
				num2 = current_no_of_spines-1
			else
				num2 = arrayb[2]+0
			for (j = 0; j <= num2; j++)
				field_array[current_no_of_spines-j] = "C"
			}
		}
	#
	# Otherwise, if there are only two elements
	#
	else if ("2" in arrayb)
		{
		#
		# The first must be a dollar and the second a number
		#
		if (arrayb[1] != "$" || arrayb[2] !~ NUMBER)
			{
			print "extract: ERROR: Invalid range given: " \
				  current_string > stderr
			exit
			}
		else
			{
			if (current_no_of_spines-arrayb[2]+0 < 1)
				field_array[1] = "C"
			else
				field_array[current_no_of_spines-arrayb[2]] = "C"
			}
		}
	#
	# If the single element is not just a dollar sign
	#
	else if (arrayb[1] != "$")
		{
		print "extract: ERROR: Invalid range given: " \
			  current_string > stderr
		exit
		}
	else field_array[current_no_of_spines] = "C"
	}

##################################################################
##		 	Function Separate_number_range
#  This function takes what is assumed to be a range of numbers from the
# function determine_range() and it creates the appropriate subscripts
# in field_array from this range.
#
function separate_number_range(current_string,    j,arrayc)
	{
	#
	# Place each number in a separate array element
	#
	split(current_string,arrayc,"-")
	#
	# If there are more than two elements then the range is incorrect
	#
	if ("3" in arrayc)
		{
		print "extract: ERROR: Invalid range given: " current_string > \
			 stderr
		exit
		}
	#
	# If both elements are not numbers, then the range is incorrect
	#
	else if (arrayc[1] !~ NUMBER || arrayc[2] !~ NUMBER)
		{
		print "extract: ERROR: Invalid range given: " current_string > \
			 stderr
		exit
		}	
	#
	# The range must be in the correct order of value
	#
	else if (arrayc[1] > arrayc[2])
		{
		print "extract: ERROR: Invalid range given: " current_string > \
			 stderr
		exit
		}
	else
		{
		#
		# Create the appropriate subscripts
		#
		if (arrayc[1]+0 <= current_no_of_spines)
			{
			if (arrayc[2]+0 > current_no_of_spines)
				num2 = current_no_of_spines
			else
				num2 = arrayc[2]+0
			for (j = arrayc[1]+0; j <= num2; j++)
				field_array[j] = "C" 
			}
		}
	}

#############################################################
##				Function Print_fields
# This function simply prints the appropriate spines and checks
# that the extracted record is of the same type as the
# input record.
#
function print_fields(     j,current_line)
	{
	for (j = 1; j <= current_no_of_spines; j++)
		{
		#
		# If the current spines should be printed according to the
		# array field_array, then print it
		#
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
# and manipulates the arrays 'path_indicator' and 'field_array'
# according to the contents of the array 'path_indicator'.
# being performed.
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
		# For any spine-path indicator(except "-"), make sure it does 
		# not appear in the field to be cut.
		#
		if (path_indicator[i] ~ /^\+$|^\^$|^v$|^x$/)
			{
			if (field_array[i] == "C")
				{
				print "extract: ERROR: Cannot use spine path indicators "\
					 "in \n                fields to be cut. "\
					 "(Line " NR ")." > stderr
				exit
				}
			}	
		#
		#	If a spine-split indicator occurred
		#
		if (path_indicator[i] == "^")
			{
			#
			# Insert a new array position in 'path_indicator' 
			# and 'field_array'
			#
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			field_array[i+1] = "N"
			}
		#
		#	If a spine-termination indicator occurred
		#
		if (path_indicator[i] == "-")
			{
			#
			# Delete that spine from the arrays.
			#
			del_array_pos(i)
			i--
			}
		#
		#	If a spine-exchange indicator occurred
		#	Nothing needs to be done (assuming valid Humdrum file)
		#
		#	If a join-path indicator occurred
		#
		if (path_indicator[i] == "v")
			{
			#
			# Check that the next field should not be cut if it is also
			# to be joined to the current field
			#
			if (path_indicator[i+1] == "v")
				{
				if (field_array[i+1] == "C")
					{
					print "extract: ERROR: Cannot use spine path indicat"\
						 "ors in\n                fields to be cut."\
						 " (Line " NR ")." > stderr
					exit
					}
				else
					{
					del_array_pos(i+1)
					i--
					}
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
# and 'field_array' and copies elements so that everything is preserved
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

####################################################################
#			Helper Function Print_fields_to_cut
function print_fields_to_cut(     j)
	{
	print "Array Left_Fields_To_Cut"
	for (j in left_fields_to_cut) print j
	print "Array Right_Fields_To_Cut"
	for (j in right_fields_to_cut) print j
	}
###################################################################
#			Helper Function Print_field_array
function print_field_array(    j)
	{
	print "Array Field_Array"
	for (j in field_array) print j, field_array[j]
	}				
