#####################################################################
##                          CORREL.AWK
# Originally programmed by David Huron, 1992.
# Re-programmed by:  Tim Racinsky     Date: June, 1994
#
# Modifications:
#   Date:      Programmer:    Description:
# 1995 May 29  David Huron    Fixed divide by zero when input values are sparse.
#
# The purpose of this program is to calculate the numerical 
# correlation between two spines.
#
# Main Functions used:
#	parse_command()			read_template()		process_comments()
#	process_interpretations()	process_dual()			process_single()
#	compute_correl()			determine_numbers()
#
#	VARIABLES:
#
#  -stderr:  variable holding destination of error messages
#  -precision:  holds precision of output specified by user (default 3)
#  -options:  holds options specified by the user
#  -template_max:  holds maximum number of data records in template array
#  -dual:  flag to indicate if dual or single input mode
#  -no_interpretation_yet:  flag to indicate if exclusive interp has been found
#  -buffer_head:  pointer to latest entry to buffer
#  -buffer_tail:  pointer to oldest, non-printed entry in buffer
#  -data_count:  holds number of data records still to be processed 
#			  currently stored in buffer
#  -error:  flag to indicate if error has occurred
#  -sin_count:  counts number of data records for single input mode
#  -precision_reg:  regular expression for precision value
#  -number_reg:  regular expression for valid number to be processed
#  -spine_terminate:  regular expression for spine terminating record
#
#	ARRAYS:
#
#  -template:  Holds data records from template file (-f option)
#  -buffer:  Holds data records from input file (-f option)
#  -sin_value1:  Holds spine one data records for single input mode
#  -sin_value2:   Holds spine two data records for single input mode
#
BEGIN {
	USAGE="\nUSAGE: correl -h                   (Help Screen)\n       correl "\
		 "[-f templatefile ] [-m] [-p n] [-s regexp] [file]"
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	# Set regular expressions to be used in program
	#
	spine_terminate = "^\\*-(\t\\*-)*$"
	precision_reg = "^[1-9][0-9]*$"
	number_reg = "[+-]?(([0-9]*\\.[0-9]+)|([0-9]+))"
	#
	# Parse command line options
	#
	no_interpretation_yet = TRUE
	buffer_head = data_count = 0
	buffer_tail = 1
	sin1_count = sin2_count = 0
	options = ""
	precision = 3
	skip_reg = ""
	template_max = 0
	dual = FALSE
	error = FALSE
	parse_command()
}
{
#
# Dual input mode requires a single spine in the input file
#
if (dual && NF != 1)
	{
	print "correl: ERROR: Input file must contain exactly one spine." > stderr
	error = TRUE
	exit
	}
#
# Single input mode requires two spines in the input file
#
if (!dual && NF != 2 && $0 !~ /^!!/)
	{
	print "correl: ERROR: Input file must contain exactly two spines." > stderr
	error = TRUE
	exit
	}
#
# Process records according to their types
#
if ($0 ~ /!/) process_comments()
else if ($0 ~ /^\*/) process_interpretations()
else if (options ~ /s/ && $0 ~ skip_reg)
	{
	if (dual) buffer[++buffer_head] = $0
	}
else if (dual) process_dual()
else process_single()
}
END	{
	#
	# If there are records still to be output in dual mode, print them
	#
	if (!error && dual)
		{
		for (i = buffer_tail; i < buffer_head; i += 1)
			{
			if (buffer[i] ~ /^(\*|!)/) print buffer[i]
			else print "."
			}
		#
		# Print a spine-path terminator at the end of the input regardless
		# of what is there already
		#
		if (buffer_head != 0)
			{
			if (buffer[buffer_head] == "*") print "*-"
			else if (buffer[buffer_head] ~ /^!/) print buffer[buffer_head]
			else
				{
				print "."
				print "*-"
				}
			}
		}
	#
	# If single mode, calculate correlation from stored data
	#
	else if (!error && !dual)
		{
		if (sin1_count == 0) exit
		for (i = 1; i <= sin1_count; i += 1)
			{
			sum1 += sin_value1[i]
			sum2 += sin_value2[i]
			}
		mean1 = sum1 / sin1_count
		mean2 = sum2 / sin1_count
		numerator = denominator = temp1 = temp2 = 0
		for (i = 1; i <= sin1_count; i += 1)
			{
			numerator += (sin_value1[i] - mean1) * (sin_value2[i] - mean2)
			temp1 += (sin_value1[i] - mean1)^2
			temp2 += (sin_value2[i] - mean2)^2
			}
		denominator = sqrt(temp1 * temp2)
		if (denominator == 0) print "0"
		else printf("%." precision "f\n",numerator / denominator)
		}
	}

############################################################################
#				FUNCTIONS USED IN CORREL.AWK
############################################################################

############################################################################
##				Function Parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	#
	# Otherwise, process any necessary variables
	#
	if (ARGV[2] != "null") options = ARGV[2]
	#
	# Obtain the precision value if specified
	#
	if (ARGV[3] != "null")
		{
		if (ARGV[3] !~ precision_reg)
			{
			print "correl: ERROR: Invalid precision specified on command "\
				 "line: " ARGV[3] > stderr
			print USAGE > stderr
			error = TRUE
			exit
			}
		else precision = ARGV[3]
		}
	#
	# Store the skip option and value if specified
	#
	if (ARGV[4] != "null")
		{
		skip_reg = ARGV[4]
		options = options "s"
		}
	#
	# Read in template file if specified
	#
	if (ARGV[5] != "null")
		{
		read_template(ARGV[5])
		dual = TRUE
		}
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ARGV[5] = ""
	}

############################################################################
##					Function read_template
#	This function reads in the template file if specified on the command
# line with -f option and stores the data records in the array template
#
function read_template(template_file     ,number_found)
	{
	number_found = FALSE
	#
	# Loop through the template file
	#
	while (getline < template_file > 0)
		{
		#
		# The template file must have only one spine of input
		#
		if (NF != 1)
			{
			print "correl: ERROR: Must specify precisely one spine in the "\
				 "template file." > stderr
		 	error = TRUE
			exit
			}
		#
		# Skip any interpretations, commens or user defined skips
		#
		else if (($0 ~ /^(\*|!)/) || (options ~ /s/ && $0 ~ skip_reg))
			continue
		else
			{
			template[++template_max] = $0
			if (!number_found && $0 ~ number_reg) number_found = TRUE
			}
		}
	#
	# The template array must have at least one data record containing a number
	#
	if (!number_found)
		{
		print "correl: ERROR: Must specify at least one valid data record "\
			 "in template file." > stderr
		exit
		}
	close(template_file)
	}

###########################################################################
##				Function process_comments
#	This function stores comment records if dual input mode is specified
#
function process_comments()
	{
	if (dual) buffer[++buffer_head] = $0
	}

############################################################################
##				Function process_interpretations
#	This function processes interpretation records if dual input mode
#
function process_interpretations()
	{
	if (dual)
		{
		#
		# Store the exclusive interpretation to show the type of output
		#
		if (no_interpretation_yet)
			{
			no_interpretation_yet = FALSE
			buffer[++buffer_head] = "**correl"
			}
		else
			{
			#
			# The only types of interpretations that should be stored
			# are spine terminators
			#
			#if ($0 ~ spine_terminate)
			#	{
			#	buffer[++buffer_head] = "*-"
			#	no_interpretation_yet = TRUE
			#	}
			#else buffer[++buffer_head] = "*"
			buffer[++buffer_head] = "*"
			}
		}
	}

#########################################################################
##				Function process_dual
#	This function processes data records if dual input mode is specified
#
function process_dual(   i)
	{
	#
	# Add the data record to the buffer array
	#
	buffer[++buffer_head] = $0
	data_count += 1
	#
	# If there are enough data records accumulated in the buffer array, then
	# compute the correlation
	#
	if (data_count == template_max)
		{
		i = buffer_tail
		#
		# Loop through and print off any records that will not be computed
		# in the correlation
		#
		while (buffer[i] ~ /^(\*|!)/ || \
								(options ~ /s/ && buffer[i] ~ skip_reg))
			{
			if (buffer[i] ~ /^(\*|!)/) print buffer[i]
			else print "."
			buffer_tail += 1
			i += 1
			}
		#
		# Compute the correlation at the current record and modify the
		# variables accordingly
		#
		compute_correl()
		buffer_tail += 1
		data_count -= 1
		}
	}

###########################################################################
##				Function compute_correl
#	This function performs the actual correlation computation
#
function compute_correl(    i,temp_count,no_of_values,sum1,sum2,mean1,mean2,\
				  temp_buf,numerator,denominator,temp1,temp2,value1,value2)
	{
	#
	# Place into a temporary array the records in buffer that should be
	# processed (i.e. only data records)
	#
	temp_count = 0
	for (i = buffer_tail; i <= buffer_head; i += 1)
		{
		if ((buffer[i] ~ /^(\*|!)/)||(options ~ /s/ && buffer[i] ~ skip_reg))
			continue
		else
			temp_buf[++temp_count] = buffer[i]
		}
	#
	# Place the numbers to be processed into the arrays value1 and value2
	#
	no_of_values = determine_numbers(temp_buf,value1,value2)
	#
	# If the number of values is zero or one, then the correlation
	# is undefined.
	if (no_of_values <= 1)
		{
		if (denominator == 0) print "?"
		return
		}
	#
	# Compute the correlation of the values (similar to END function)
	#
	for (i = 1; i <= no_of_values; i += 1)
		{
		sum1 += value1[i]
		sum2 += value2[i]
		}
	mean1 = sum1 / no_of_values
	mean2 = sum2 / no_of_values
	numerator = denominator = temp1 = temp2 = 0
	for (i = 1; i <= no_of_values; i += 1)
		{
		numerator += (value1[i] - mean1) * (value2[i] - mean2)
		temp1 += (value1[i] - mean1)^2
		temp2 += (value2[i] - mean2)^2
		}
	denominator = sqrt(temp1 * temp2)
	if (denominator == 0) print "?"
	else printf("%." precision "f\n",numerator / denominator)
	}

############################################################################
##				Function determine_numbers
#	This function determines the numbers to be compared by looping through
# the arrays as well as any multiple stops
#
function determine_numbers(tempbuf,value1,value2,     i,j,value1_count,\
					value2_count,count1,count2,double1,double2,buf1,buf2,\
			max_count,max_number,number_found1,number_found2,number1,number2)
	{
	#
	# Loop through the given arrays (each should have the same number of
	# elements since they are the same number of data records)
	#
	for (i = 1; i <= template_max; i += 1)
		{
		count1 = count2 = 0
		#
		# Determine the number of 'number signifiers' present in the double 
		# stop of each of the array elements
		#
		double1 = split(template[i],buf1," ")
		for (j = 1; j <= double1; j += 1)
			{
			if (buf1[j] ~ number_reg) count1 += 1
			}
		double2 = split(tempbuf[i],buf2," ")
		for (j = 1; j <= double2; j += 1)
			{
			if (buf2[j] ~ number_reg) count2 += 1
			}
		#
		# If there are not the same number of 'number tokens' in the
		# current elements and -m option was no specified, print an error
		#
		if (count1 != count2 && options !~ /m/)
			{
			print "correl: ERROR: Matched-pairs criterion transgressed"\
				 " near line " FNR "." > stderr
			error = TRUE
			exit
			}
		#
		# Determine max number of multiple stops
		#
		if (double1 > double2) max_count = double1
		else max_count = double2
		#
		# Determine how many 'number tokens' should be processed
		#
		if (count1 > count2) max_number = count2
		else max_number = count1
		number_found1 = number_found2 = 0
		#
		# Loop through the multiple stops and store any 'number' tokens
		# in the arrays value1 and value2
		#
		for (j = 1; j <= max_count; j += 1)
			{
			if ((match(buf1[j],number_reg) > 0) \
									 && (number_found1 < max_number))
				{
				number1 = substr(buf1[j],RSTART,RLENGTH) + 0
				value1[++value1_count] = number1
				number_found1 += 1
				}
			if ((match(buf2[j],number_reg) > 0) \
									 && (number_found2 < max_number))
				{
				number2 = substr(buf2[j],RSTART,RLENGTH) + 0
				value2[++value2_count] = number2
				number_found2 += 1
				}
			}
		}
	# 
	# Return the number of values to be used in the computation
	#
	return value1_count
	}

##########################################################################
##				Function process_single
#	This function processes data records for single input mode.  It
# performs the same task as the function determine_numbers().
#
function process_single(     count1,count2,double1,doubl2,buf1,buf2,max_count,\
					max_number,number_found1,number_found2,number1,number2)
	{
	count1 = count2 = 0
	#
	# Determine the number of 'number signifiers' present in the double 
	# stop of each of the array elements
	#
	double1 = split($1,buf1," ")
	for (j = 1; j <= double1; j += 1)
		{
		if (buf1[j] ~ number_reg) count1 += 1
		}
	double2 = split($2,buf2," ")
	for (j = 1; j <= double2; j += 1)
		{
		if (buf2[j] ~ number_reg) count2 += 1
		}
	#
	# If there are not the same number of 'number tokens' in the
	# current elements and -m option was no specified, print an error
	#
	if (count1 != count2 && options !~ /m/)
		{
		print "correl: ERROR: Matched-pairs criterion transgressed"\
			 " near line " FNR "." > stderr
		error = TRUE
		exit
		}
	#
	# Determine max number of multiple stops
	#
	if (double1 > double2) max_count = double1
	else max_count = double2
	#
	# Determine how many 'number tokens' should be processed
	#
	if (count1 > count2) max_number = count2
	else max_number = count1
	sin_count += max_number
	number_found1 = number_found2 = 0
	#
	# Loop through the multiple stops and store any 'number' tokens
	# in the arrays sin_value1 and sin_value2
	#
	for (j = 1; j <= max_count; j += 1)
		{
		if ((match(buf1[j],number_reg) > 0) \
								 && (number_found1 < max_number))
			{
			number1 = substr(buf1[j],RSTART,RLENGTH) + 0
			sin_value1[++sin1_count] = number1
			number_found1 += 1
			}
		if ((match(buf2[j],number_reg) > 0) \
								 && (number_found2 < max_number))
			{
			number2 = substr(buf2[j],RSTART,RLENGTH) + 0
			sin_value2[++sin2_count] = number2
			number_found2 += 1
			}
		}
	}
