####################################################################
##					YANK2.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       	Description:
# May 9/94	Tim Racinsky		Removed 'Not found' error statement so that
#							'silence is golden' rule was maintained
# June 7/94	Tim Racinsky		Modified parse_command for getopts
#
#
#	This program implements the 'm' option or, the 'marker' option,
# of the yank program. It is called from the program yank.ksh.
# It allows the user to specify a 'marker' and a range of numbers which
# represent line numbers appearing after each occurrence of 'marker' int
# the input.  As with all yank programs, comments and interpretations 
# within the yanked material are output as well as all interpretations 
# before the yanked material.
#
# Main Functions used:
#	Parse_command()			Determine_range()
#	Parse_dollar_range()		Separate_number_range()
#	Terminate()				Process_match()
#	Determine_number_of_fields()	Store_indicators()
#	Process_indicators()
#
# Helper Functions used:
#	Print_records_to_cut()
#
# The program find_reg.awk is also used to assist in finding all occurrences
# of the specified regular expression before any records are processed.
#
#	VARIABLES:
#
#  -ANACRUSIS:  flags whether or not we have come upon the first record that
#   matches the regular expression (TRUE means we have not).
#  -FINAL:  does the same job for the material after the last record that
#   matches the regular expression.
#  -FIRST_RECORD_PRINTED_YET:  differs from ANACRUSIS in that it marks the
#   the first record that is printed (as opposed to the first matched record).
#  -last_number:  keeps track of the last record to print out in the FINAL
#   segment.
#  -data_count:  keeps track of how many data records have been parsed for
#   the current segment.
#  -occurrence:  keeps track of how many records have been found that contained
#   the regular expression.
#  -number_of_fields: keeps track of how many fields are currently active so
#   that the proper number of spine-path terminators may be printed at the
#   conclusion of the yanked material.
#  -current_file:  holds the name of the current input file
#
#	ARRAYS:
#
#  -number_of_lines:  each successive element holds the number of data
#   records that occur between successive records that match the reg. exp.
#  -records_to_cut:  subscripts represent the records that
#   are to be cut from the current segment.  This array has to be 
#   recalculated for each different segment.
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
	current_file = ""
	parse_command()
	}
{
#
# If the variable current_file has been set to the current filename, then
# skip through the rest of the records of that file.
#
if (current_file == FILENAME) next
#
# If this is the first record of the current file
#
if (FNR == 1)
	{
	#
	# Reset variables for each file encountered
	#
	FIRST_RECORD_PRINTED_YET = FALSE
	ANACRUSIS = TRUE
	FINAL = FALSE
	spine_path_record = FALSE
	occurrence = 0
	last_number = 0
	number_of_fields = 0
	data_count = 0
	next_line = 0
	for (j in number_of_lines) delete number_of_lines[j]
	#
	# Use the program 'find_reg.awk' in order to get a list of the number
	# of data records between each set of regular expression records
	#
	("echo $TMPDIR/temp-$$-" ++i) | getline tmpname
   	system("$AWK_VER -f ${HUMDRUM}/bin/find_reg.awk '" REGEXP "' " FILENAME " > " tmpname)
	#
	# Store the values in the array number_of_lines
	#
	while (("cat " tmpname) | getline no_of_lines > 0)
		number_of_lines[++next_line] = no_of_lines
	system("rm " tmpname)
	#
	# If there is more than one entry, then at least one record matches REGEXP
	#
	if ("2" in number_of_lines)
		determine_range(RANGE,number_of_lines[1]+0)
	#
	# Otherwise, we are already into the 'final' segment
	#
	else
		{
		# This print statement was removed so that the general rule of 
		# 'Silence is Golden' was adhered to
		#
		#print "yank: ERROR: No record in " FILENAME " matches the"\
		#	 " given regular expression." > stderr
		exit
		}
	}
#
# If the current record matches the regular expression
#
if ($0 ~ REGEXP)
	{
	#
	# Check variables and process the record appropriately
	#
	process_match()
	#
	# If this is the last record to print, then end this file
	#
	if (0 == last_number && FINAL) terminate()
	#
	# Otherwise process the next record
	#
	else next
	}
#
# If the current record is a comment
#
else if ($0 ~ /^!/)
	{
	#
	# If there were no records printed yet in the file
	#
	if (!FIRST_RECORD_PRINTED_YET)
		{
		#
		# Print the comments only if -c option specified
		#
		if (COMMENTS)
			{
			print $0
			if ($0 !~ /^!!/) number_of_fields = NF
			}
		}
	#
	# If there has been a data record printed, print every comment
	#
	else
		{
		print $0
		if ($0 !~ /^!!/) number_of_fields = NF
		}
	}
#	
# Always print the interpretations
#
else if ($0 ~ /^\*/)
		{
		print $0
		determine_number_of_fields()
		}
#
# If it is a data record
#
else
	{
	#
	# Increase the data count and see if the record should be cut
	#
	data_count++
	if (data_count in records_to_cut)
		{
		print $0
		if (!FIRST_RECORD_PRINTED_YET)
			FIRST_RECORD_PRINTED_YET = TRUE
		}
	#
	# If currently in the 'final' segment, check to see if we
	# should move on to the next file
	#
	if (FINAL)
		{
		if (data_count == last_number || last_number == 0) terminate()
		}
	}
}

###########################################################################
#				 FUNCTIONS IN YANK2.AWK
###########################################################################

######################################################################
##				Function Parse_Command
#	This function simply determines which options the user has specified.
#
function parse_command()
	{
	if (ARGV[2] ~ /c/) COMMENTS = TRUE
	REGEXP = ARGV[3]
	RANGE = ARGV[4]
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ""
	}

#################################################################
##				Function Determine_range
#	This function is used to determine the next range of records that
# are to be output.  This range can change throught the input file since
# there may be a variable number of data records between each occurrence
# of the given regular expression. It is passed the whole string of 
# ranges as well as the number of data records following the current
# record that matches the regular expression.
#
function determine_range(range_string,num_of_lines,    arraya,j)
	{
	#
	# Delete the old array subscripts
	#
	for (j in records_to_cut) delete records_to_cut[j]
	#
	# Place each separate range in an array element and loop through them
	#
	split(range_string,arraya,",")
	for (j in arraya)
		{
		#
		# If the current range contains a dollar sign, parse it
		#
		if (arraya[j] ~ /\$/) 
			{
			#
			# Only parse a 'dollar' range if not working on the 
			# final segment (after last record with regular expression)
			#
			if (!FINAL) parse_dollar_range(arraya[j],num_of_lines)
			else continue
			}
		#
		# Otherwise, if the current range is a range of numbers
		#
		else if (arraya[j] ~ /[-]/)
			{
			#
			# Only parse the number range if not working on segment
			# before first record with regular expression
			#
			if (!ANACRUSIS) separate_number_range(arraya[j],num_of_lines)
			else continue
			}
		#
		# Otherwise it must be a single number
		#
		else if (arraya[j] !~ NUMBER)
			{
			print "yank: ERROR: Invalid number given: " arraya[j] \
				 > stderr
			exit
			}
		#
		# Process the number only if it is within the valid range
		# and we are not currently in the anacrusis
		#
		else if (arraya[j]+0 <= num_of_lines && !ANACRUSIS)
			{
			#
			# Keep track of the last record to output only
			# when in the FINAL segment
			#
			if (FINAL)
				{	
				if (arraya[j]+0 > last_number) last_number = arraya[j]+0
				}
			records_to_cut[arraya[j]] = ""
			}
		}
	}

##################################################################
##			Function Parse_dollar_range
#	This function accepts a range containing dollar signs and the
# number of records it has to work with and it pareses the range and
# creates the appropriate array subscripts in records_to_cut
#
function parse_dollar_range(current_string,num_of_lines,  arrayb,j,num1,num2)
	{
	#
	# Split the passed string according to the "-" and parse the array
	#
	split(current_string,arrayb,"-")
	#
	# There should not be more than 4 elements in the array
	#
	if ("5" in arrayb)
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}
	#
	# If there are four elements in the array
	#
	if ("4" in arrayb)
		{
		#
		# The 1st and 3rd elements must be dollars. The 2nd and 4th must
		# be numbers with the 2nd less than the 4th
		#
		if ((arrayb[1] != "$" || arrayb[3] != "$") \
			|| (arrayb[2] !~ NUMBER || arrayb[4] !~ NUMBER) \
			|| (arrayb[2] < arrayb[4]))
			{
			print "yank: ERROR: Invalid range given: " current_string  \
				 > stderr
			exit
			}
		else
			{
			#
			# Determine the proper range (must be valid or default to 0)
			# and add the appropriate subscripts.
			#
			if (num_of_lines-arrayb[2] < 0) num1 = 0
			else num1 = num_of_lines-arrayb[2]
			if (num_of_lines-arrayb[4] < 0) num2 = 0
			else num2 = num_of_lines-arrayb[4]
			for (j = num1; j <= num2; j++) records_to_cut[j] = ""
			}
		}
	#
	# If there are only 3 elements in the array, there are 2 possible cases
	#
	else if ("3" in arrayb)
		{
		# 
		# First case, 1st and 3rd elements are a "$" and the 2nd must
		# be a number
		#
		if (arrayb[1] == "$")
			{
			if (arrayb[3] != "$" || arrayb[2] !~ NUMBER)
				{
				print "yank: ERROR: Invalid range given: " \
                               current_string > stderr
				exit
				}
			else
				{
				#
				# Determine range and add the subscripts
				#
				if (num_of_lines-arrayb[2] < 0) num1 = 0
				else num1 = num_of_lines-arrayb[2]
				for (j = num1; j <= num_of_lines+0; j++)
					records_to_cut[j] = ""
				}
			}
		#
		# Otherwise they must be inverted
		#
		else
			{
			if ((arrayb[2] != "$") \
				|| (arrayb[1] !~ NUMBER || arrayb[3] !~ NUMBER))
				{
				print "yank: ERROR: Invalid range given: " \
                          current_string > stderr
				exit
				}
			else
				{
				#
				# Only add the subscripts if the first number comes
				# before the second number
				#
				if (arrayb[1]+0 <= num_of_lines-arrayb[3])
					{
					for (j=arrayb[1]+0;j<=num_of_lines-arrayb[3];j++)
						records_to_cut[j] = ""
					}
				}
			}
		}
	#
	# If there are 2 elements, again 2 cases
	#
	else if ("2" in arrayb)
		{
		#
		# If the first element is a dollar, then a single record
		# is being referenced and the 2nd element must be a number.
		#
		if (arrayb[1] == "$")
			{
			if (arrayb[2] !~ NUMBER)
				{
				print "yank: ERROR: Invalid range given: " \
					 current_string > stderr
				exit
				}
			#
			# If it is a positive reference, add the subscript
			#
			else if (num_of_lines - arrayb[2]+0 >= 0)
				records_to_cut[num_of_lines-arrayb[2]+0] = ""
			}
		#
		# Otherwise, the order must be switched
		#
		else
			{
			if (arrayb[1] !~ NUMBER || arrayb[2] != "$")
				{
				print "yank: ERROR: Invalid range given: " \
					  current_string > stderr
				exit
				}
			#
			# The number must be less than the number of lines
			#
			else	if (arrayb[1]+0 <= num_of_lines)
				{
				for (j=arrayb[1]+0; j <= num_of_lines+0; j++)
					records_to_cut[j] = ""
				}
			}
		}
	#
	# Otherwise, must be a single dollar sign
	#
	else if (arrayb[1] != "$") 
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}
	else records_to_cut[num_of_lines] = ""
	}

##################################################################
##		 	Function Separate_number_range
#	This function separates a range of numbers passed to it using the
# variable num_of_lines and creates the appropriate array subscripts
#
function separate_number_range(current_string,num_of_lines,    j,arrayc,num2)
	{
	#
	# Place each number in a separate element and then parse
	#
	split(current_string,arrayc,"-")
	#
	# If there are 3 elements, an error has occurred
	#
	if ("3" in arrayc)
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}
	#
	# Both elements must be numbers with the 1st not greater than the 2nd
	#
	else if ((arrayc[1] !~ NUMBER || arrayc[2] !~ NUMBER) \
			|| arrayc[1]+0 > arrayc[2]+0)
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}
	else
		{
		#
		# Find the appropriate range
		#
		if (arrayc[2] > num_of_lines) num2 = num_of_lines
		else num2 = arrayc[2]+0
		if (arrayc[1]+0 <= num_of_lines)
			{
			for (j = arrayc[1]+0; j <= num2; j++)
				{
				#
				# If working on the FINAL segment, keep track of 
				# the last record to print out
				#
				if (FINAL)
					{
					if (j > last_number) last_number = j
					}
				records_to_cut[j] = ""
				}
			}
		}
	}

######################################################################
##				Function Process_match
#	This function is called each time a match is found
#
function process_match()
	{
	#
	# The 'anacrusis' segment is finished
	#
	if (ANACRUSIS) ANACRUSIS = FALSE
	#
	# Update the occurrence count
	#
	occurrence++
	#
	# If this is the last record matching the reg.exp. then set the flag
	# to show we are on the last occurrence.
	#
	if (number_of_lines[occurrence+2] == "") FINAL = TRUE
	#
	# Determine the next range
	#
	determine_range(RANGE,number_of_lines[occurrence+1]+0)
	data_count = 0
	#
	# If the user wishes to print the pattern-matching record then print it
	#
	if (data_count in records_to_cut)
		{
		print $0
		if ($0 ~ /^\*/) determine_number_of_fields()
		if ($0 !~ /^\*/ && $0 !~ /^!!/) number_of_fields = NF
		if (!FIRST_RECORD_PRINTED_YET) FIRST_RECORD_PRINTED_YET = TRUE
		}
	}

##################################################################
##			Function Determine_number_of_fields
#	This function is called when an interpretation record is 
# encountered.  It first determines if it is a spine-path record or not.
# If it is, it determines how many spines result from the record.
#
function determine_number_of_fields(  spine_path_record)
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
##			Function Process_indicators
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

#####################################################################
##				Function Terminate
#	This function is called when it is time to terminate processing
# this file and move on to the next file (or quit the program)
#
function terminate(    lastline)
	{
	#
	# Create and print the terminating record
	#
	for (j = 1; j <= number_of_fields; j++) lastline = lastline "*-\t"
	if (lastline != "") print substr(lastline,1,length(lastline)-1)
	current_file = FILENAME
	}

########################################################################
##			Helper Function print_records_to_cut
#
function print_records_to_cut(   j)
	{
	print "Array Records to cut"
	for (j in records_to_cut) print j
	print "End array records to cut"
	}
