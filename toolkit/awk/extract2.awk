#########################################################################
##                              EXTRACT2.AWK
# Programmed by: Tim Racinsky       Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#    Date:    Programmer:        Description:
#
#
# 	This program implements the 'field trace' mode of the extract program and
# is called from the shell script 'extract.ksh'. It is designed to be used
# in conjunction with the fields program.
#
# Main Functions used:
#	Get_range()	Determine_fields_to_cut()	Separate_number_range()
#	Print_fields()
#
# Helper Functions used:
#	Print_fields_to_cut()	
#
#	VARIABLES:
#
#  -current_array_pos:   holds which element of first_array is currently
#   being processed by the program.
#  -first_line:  indicates the first line number of the next set of records
#   that will have particluar fields extracted.  These records will have
#   different fields extracted than previous and following sets of records.
#  -second_line:  indicates the first line number of the following set of
#   of records.
#
#	ARRAYS:
#
#  -first_array:  holds the contents of the 'tracefile' provided by
#   the use.  Each element contains two items separated by a tab.  The first
#   item is a line number, and the second item is a range of fields to cut
#   starting from that line number.
#  -fields_to_cut:  each element represents a currently active spine and the
#   contents of each element is either a 'C' or an 'N' which represent 'cut'
#   and 'no-cut' respectively.  This array must be computed for each element
#   in first_array.
#
BEGIN {
	#
	# Delete the DUMMY variable sent from the shell script
	#
	ARGV[1] = ""
	#
	# Read the tracefile into the array first_array
	#
	while (getline < ARGV[2] == 1) first_array[++i] = $1 "\t" $2
	ARGV[2] = ""
	FS = OFS = "\t"	
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	NUMBER = "^[0-9]+$"
	current_array_pos = 1
	}
{
#
# Determine the first range of fields to cut
#
if (NR == 1) get_range()
#
# If the current line starts a new series of fields to cut, determine exactly
# which fields are to be cut and start cutting.
#
if (FNR == second_line) 
	{
	get_range()
	print_fields()
	}
#
# Otherwise, just cut the correct fields.
#
else print_fields()
}

###############################################################################
#					FUNCTIONS USED IN EXTRACT2.AWK
###############################################################################

###############################################################################
##					Function Get_range
#	This function is used to determine the next set of fields to cut (from 
# the current line number until the next line number in the array first_array).
#
function get_range(     arraya,arrayb)
	{
	#
	# Place the current line number and range of fields into an array
	#
	split(first_array[current_array_pos],arraya)
	#
	# Do the same for the next line
	#
	split(first_array[current_array_pos+1],arrayb)
	#
	# Store the current line number in first_line
	#
	first_line = arraya[1]
	#
	# If this is the first element looked at and it is not a 1, then print
	# an error since the first line must always be the first element
	#
	if (current_array_pos == 1 && first_line != 1)
		{
		print "extract: ERROR: First line in Trace file must reference "\
			 "line number 1." > stderr
		exit
		}
	#
	# If the next line does not exist (i.e. user wants to go to the end
	# of the file) then place a null value in second_line
	#
	if (first_array[current_array_pos+1] == "") second_line = ""
	#
	# Otherwise, store the next line number the user wants to make changes
	# at in second_line
	#
	else second_line = arrayb[1]
	#
	# Determine which fields to cut from the range given
	#
	determine_fields_to_cut(arraya[2])
	current_array_pos++
	}
	
##################################################################
##		 	Function Determine_fields_to_cut
#  This function is called from get_range() and it parses the range
# of numbers given to represent the fields to cut.
#
function determine_fields_to_cut(current_string,    j,arrayc)
	{
	#
	# Separate the list of numbers and ranges by the comma and place
	# into individual array elements.
	#
	split(current_string,arrayc,",")
	#
	# Remove the old elements (if any) in fields_to_cut
	#
	for (j in fields_to_cut) delete fields_to_cut[j]
	#
	# For each range of numbers specified
	#
	for (j in arrayc)
		{
		#
		# If a "-" character is present, assume it is a range and parse
		#
		if (arrayc[j] ~ /[-]/) separate_number_range(arrayc[j])
			#
			# Otherwise, a single element must be a valid number
			#
		else if (arrayc[j] !~ NUMBER)
			{
			print "extract: ERROR: Invalid number given: " \
				 arrayc[j] > stderr
			exit
			}
		else fields_to_cut[arrayc[j]] = ""
		}
	}

##################################################################
##		 	Function Separate_number_range
#  This function takes what is assumed to be a range of numbers from the
# function determine_fields_to_cut() and it creates the appropriate subscripts
# in fields_to_cut from this range.
#
function separate_number_range(current_string,    j,arrayd,num2)
	{
	#
	# Place each number in a separate array element
	#
	split(current_string,arrayd,"-")
	#
	# If there are more than two elements then the range is incorrect
	#
	if ("3" in arrayd)
		{
		print "extract: ERROR: Invalid range given: " current_string > \
			 stderr
		exit
		}
	#
	# If both elements are not numbers, then the range is incorrect
	#
	else if (arrayd[1] !~ NUMBER || arrayd[2] !~ NUMBER)
		{
		print "extract: ERROR: Invalid range given: " current_string > \
			 stderr
		exit
		}	
	#
	# If the numbers are not in the correct 'order'
	#
	else if (arrayd[1] > arrayd[2])
		{
		print "extract: ERROR: Invalid range given: " current_string > \
			 stderr
		exit
		}	
	#
	# If the number is outside the current range of spines, give error
	#
	else if ($0 !~ /^!!/ && arrayd[2] > NF)
		{
		print "extract: ERROR: Invalid range given: " current_string > \
			 stderr
		exit
		}	
	else
		#
		# Create the appropriate subscripts
		#
		{
		for (j = arrayd[1]+0; j <= arrayd[2]+0; j++)
			fields_to_cut[j] = ""
		}
	}

###################################################################
##				Function Print_fields
#	This function actually prints the appropriate fields according
# to the array fields_to_cut.
#
function print_fields(     j,result_string)
	{
	#
	# If 0 is a valid subscript in fields_to_cut, print nothing
	#
	if (0 in fields_to_cut) return
	else 
		{
		#
		# Print all global comments
		#
		if ($1 ~ /^!!/ ) print $0
		else
			#
			# Otherwise, construct the output string
			#
			{
			for (j = 1; j <= NF ; j++)
				{
				if (j in fields_to_cut) 
					result_string = result_string $j "\t"
				}
			if (result_string != "")
				print substr(result_string,1,length(result_string)-1)
			#
			# If the input record was a local comment record and the 
			# extracted record is a global comment record, print a 
			# warning to standard error
			#
			if ($0 ~ /^![^!]/ && result_string ~ /^!!/)
				{
				print "extract: WARNING: Local comment record extracted as"\
					 " global comment at \n                  line " FNR \
					 "." > stderr
				}
			#
			# If the input record was a data record
			#
			else if ($0 !~ /^[!\*]/)
				{
				#
				# If the extracted record is a comment record, print a 
				# warning to standard error
				#
				if (result_string ~ /^!/)
					{
					print "extract: WARNING: Data record has been extrac"\
						 "ted as comment record at \n                  "\
						 "line " FNR "." > stderr
					}
				#
				# If the extracted record is an interpretation record, 
				# print a warning to standard error
				#
				else if (result_string ~ /^\*/)
					{
					print "extract: WARNING: Data record has been extract"\
						 "ed as interpretation record at \n             "\
						 "     line " FNR "." > stderr
					}
				}
			}
		}
	}

######################################################################
##			Helper Function Print_fields_to_cut
#
function print_fields_to_cut(     j)
	{
	print "Array fields_to_cut"
	for (j in fields_to_cut) print j, fields_to_cut[j]
	print "End Array fields_to_cut"
	}
