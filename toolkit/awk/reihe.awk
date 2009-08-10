#####################################################################
##                          REIHE.AWK
# Programmed by:  David Huron     Date: June, 1994
#
# Modifications:
#   Date:      Programmer:    Description:
# June 20/94	Tim Racinsky	Added parse_command function
# July 12/94	Tim Racinsky	Fixed error with -r option when no integer is
#						given (used to change A's, B's, etc. to integers)
#
# The purpose of this program is to output a specified set-complex
# variant for a given prime-form tone row.
#
# Main Functions used:
#	parse_command()		transpose()			invert()
#	retrograde()			shift()
#
#	VARIABLES:
#
#  -stderr:  variable holding destination of error messages
#  -first_file:  flag to indicate if this is the first file to be processed
#  -no_interpretation_yet:  flag to indicate if first interp. record processed
#  -options:  Holds options specified by the user
#  -setform: Holds the option indicating the type of row variant
#            operation: [Pp]-prime, [Ii]-inversion, [Rr]-retrograde,
#            [X]-retrograde-inversion, [Ss]-cyclic rotation (shift)
#  -integer: Hold specific integer value of setform, e.g. P1, R8, RI7, etc.
#  -data_count:  Holds number of data records stored
#  -base:  Holds value for modulo arithmetic
#  -interpretation: Holds type of interpretation processed
#  -error: Flag to indicate if error has occurred in the main loop
#
#	ARRAYS:
#
#  -current_interp:  Holds exclusive interpretations for currently active spines
#  -pattern:  Holds the pattern to match (read in from template file)
#  -token:  Holds the latest set of records to test against pattern array
#
BEGIN {
	FS = OFS = "\t"
	TRUE = 1 ; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	pc_number = "[0-9]+|[AB]|[TE]"
	shift_number = "^[+-]?[0-9]+$"
	trans_number = "^(([0-9]+)|([ABTE]))$"
	no_interpretation_yet = TRUE
	error = FALSE
	data_count = 0
	integer = 0
	base = 12
	setform = ""
	options = ""
	parse_command()
}
{
if ($0 !~ /^!!/ && NF != 1)
	{
	print "reihe: ERROR: Only one spine of input is permissible." > stderr
	error = TRUE
	exit
	}
#
# Skip those lines which should not be considered
#
if ($0 == "*-") exit
else if ($0 ~ /^!/) next
else if ($0 ~ /^\*[^*]/) next
else if ($0 ~ /[=]/) next
else if ($0 ~ /r/ && integer != "null" && setform !~ /[Ss]/) next
else if ($0 ~ /^\.$/) next
else if ($0 ~ /^\*\*/)
	{
	#
	# Only one exclusive interpretation is allowed per file
	#
	if (no_interpretation_yet)
		{
		if (integer != "null" && setform !~ /[Ss]/ && $0 != "**pc")
			{
			print "reihe: ERROR: Input must be of type **pc." > stderr
			error = TRUE
			exit
			}
		else interpretation = $0
		no_interpretation_yet = FALSE
		}
	else
		{
		print "reihe: ERROR: Cannot specify new exclusive interpretation"\
			 "." > stderr
		error = TRUE
		exit
		}
	}
else
	{
	#
	# Store integer values if processing pc input
	#
	if (integer != "null" && setform !~ /[Ss]/)
		{
		if (match($0,pc_number) > 0)
			{
			token = substr($0,RSTART,RLENGTH)
			prime[++data_count] = token
			}
		}
	#
	# Otherwise, store all data values
	#
	else prime[++data_count] = $0
	}
}
END	{
	#
	# Quit the program if an error occurred or there were no data values
	#
	if (error || data_count == 0) exit
	#
	# Change any A's or B's to 10's and 11's if processing pc input
	#
	if (integer != "null" && setform !~ /[Ss]/)
		{
		for (i = 1; i <= data_count; i += 1)
			{
			if (prime[i] == "A" || prime[i] == "T") prime[i] = 10
			else if (prime[i] == "B" || prime[i] == "E") prime[i] = 11
			}
		#
		# Change all input values to modulo the base
		#
		if (integer != "null" && setform !~ /[Ss]/)
			{
			for (i = 1; i <= data_count; i += 1)
				prime[i] = ((prime[i] % base) + base) % base
			}
		}
	#
	# Process primes.
	#
	if (setform ~ /[pP]/) transpose(prime,final)
	#
	# Process retrogrades.
	#
	else if (setform ~ /[rR]/)
		{
		#
		# If processing **pc input, perform the transpose as well
		#
		if (integer != "null")
			{
			transpose(prime,mid)
			retrograde(mid,final)
			}
		else retrograde(prime,final)
		}
	#
	# Process inversions.
	#
	else if (setform ~ /[Ii]/)
		{
		invert(prime,mid)
		transpose(mid,final)
		}
	#
	# Process retrograde-inversions.
	#
	else if (setform ~ /X/)
		{
		invert(prime,mid1)
		transpose(mid1,mid2)
		retrograde(mid2,final)
		}
	#
	# Process cyclic rotations.
	#
	else if (setform ~ /[Ss]/) shift(prime,final)
	#
	# Change any 10's and 11's to A's and B's if -a option selected
	#
	if (options ~ /a/)
		{
		for (i = 1; i <= data_count; i += 1)
			{
			if (final[i] == 10) final[i] = "A"
			else if (final[i] == 11) final[i] = "B"
			}
		}
	#
	# Output the final array
	#
	print interpretation
	for (i = 1; i <= data_count; i += 1) print final[i]
	print "*-"
	}

############################################################################
#			FUNCTIONS USED IN REIHE.AWK
############################################################################

###########################################################################
##				Function parse_command
#	This function ensures the correctness of the command line options
#
function parse_command()
	{
	setform = ARGV[2]
	#
	# Check to see if -a and -m options were both specified
	#
	if (ARGV[3] != "null")
		{
		if (ARGV[3] ~ /a/ && ARGV[3] ~ /m/)
			{ 
			print "reihe: ERROR: Cannot specify both -a and -m options"\
				 "." > stderr
			error = TRUE
			exit
			}
		else options = ARGV[3]
		}
	#
	# Make sure the value of 'integer' is correct depending on the options
	#
	if (setform ~ /[Ss]/)
		{
		if (ARGV[4] !~ shift_number)
			{
			print "reihe: ERROR: Invalid shift specified: " ARGV[4] > stderr
			error = TRUE
			exit
			}
		else integer = ARGV[4]
		}
	else if (ARGV[4] != "null" && ARGV[4] !~ trans_number)
		{
		print "reihe: ERROR: Invalid integer specified: " ARGV[4] > stderr
		error = TRUE
		exit
		}
	else
		{
		if (ARGV[4] == "A" || ARGV[4] == "T") integer = 10
		else if (ARGV[4] == "B" || ARGV[4] == "E") integer = 11
		else integer = ARGV[4]
		}
	if (integer == "null")
		{
		if (options ~ /[am]/)
			{ 
			print "reihe: ERROR: Cannot specify -a or -m options with "\
				 "-R option\n              when processing non-pc"\
				 "input." > stderr
			error = TRUE
			exit
			}
		}
	if (ARGV[5] != "null") base = ARGV[5]
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ARGV[5] = ""
	}

###########################################################################
##				Function transpose
#	This function transposes the values in array input according to integer
#
function transpose(input,output   ,offset,i)
	{
	offset = (((input[1] - integer) % base) + base) % base
	for (i = 1; i <= data_count; i += 1)
		output[i] = (((input[i] - offset) % base) + base) % base
	}

###########################################################################
##				Function retrograde
#	This function reverses the order of the array input
#
function retrograde(input,output   ,i)
	{
	for (i = 1; i <= data_count; i += 1) output[data_count+1-i] = input[i]
	}

###########################################################################
##				Function invert
#	This function inverts the elements of the array input
#
function invert(input,output   ,i)
	{
	for (i = 1; i <= data_count; i += 1)
		output[i] = (((input[1] - input[i] + input[1]) % base) + base) % base
	}

###########################################################################
##				Function shift
#	This function shifts the elements in input 'integer' positions
#
function shift(input,output   ,i)
	{
	for (i = 1; i <= data_count; i += 1)
		output[i] \
= input[(((((i - integer - 1) % data_count) + data_count) % data_count) + 1)]
	}
