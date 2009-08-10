#####################################################################
##                          NUM.AWK
# Programmed by:  Tim Racinsky     Date: June, 1994
# Copyright (c) 1994 David Huron
#
# Modifications:
#   Date:      Programmer:    Description:
# August 5/94	Tim Racinsky	Removed getopts from shell script so parse.
#						command needed modifications.
# August 8/94	Tim Racinsky	Added -p and -P options
#
#
# Main Functions used:
#	parse_command()		process_data()			process_interpretations()
#	process_comments()		check_and_print()		print_numbers()
#	print_no_numbers()
#
#	VARIABLES:
#
#  -stderr:  Holds destination of error messages
#  -exclusive_record:  Regular expression for exclusive interpretation records
#  -totally_exclusive:  Regular expression for all spines with excl. interps.
#  -null_interp:  Regular expression for null interpretation record
#  -spine_terminate:  Regular expression for all spines terminating
#  -number_reg:  Regular expression for valid number
#  -suspended:  Flag to indicate if currently suspending printing of numbers
#  -first_interpretation_yet:  Flag to indicate if first excl. interp. found
#  -attach:  Holds value for -a option
#  -number:  Holds value for -n option
#  -resume:  Holds value for -r option
#  -Resume:  Holds value for -R option
#  -suspend:  Holds value for -s option
#  -Suspend:  Holds value for -S option
#  -exclude:  Holds value for -x option
#  -reset:  Holds value for -z option
#  -Reset:  Holds value for -Z option
#  -increment:  Holds value for -i option
#  -offset:  Holds value for -o option
#  -Offset:  Holds value for -O option
#  -position:  Holds value for -p option
#  -POSITION:  Holds value for -P option
#  -options:  Holds options specified by user
#  -data_count:  Holds number of current data record
#
#	ARRAYS:
#
#
BEGIN {
	USAGE="\nUSAGE: num -h                   (Help Screen)\n       num [-efT]"\
		 " [a **interp] [-i n] [-n regexp] [-o n] [-O n]\n           [-p re"\
		 "gexp] [-P regexp] [-r regexp] [-R regexp] [-s regexp]\n           "\
		 "[-S regexp] [-x regexp] [-z regexp] [-Z regexp] [file ...]"
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	# These regular expressions are used within this program
	#
	exclusive_record = "^\\*(\\*([^\t])+)?(\t\\*(\\*([^\t])+)?)*$"
	totally_exclusive = "^\\*\\*([^\t])+(\t\\*\\*([^\t])+)*$"
	null_interp = "^\\*(\t\\*)*$"
	spine_terminate = "^\\*-(\t\\*-)*$"
	number_reg = "^[0-9]+(\\.[0-9]*)?$"
	#
	# These global variables are set according to the options specified
	#
	suspended = FALSE
	first_interpretation_yet = FALSE
	attach = number = resume = Resume = suspend = Suspend = exclude = ""
	reset = Reset = options = ""
	increment = offset = Offset = 1
	#
	# Parse the command line and begin counting records
	#
	parse_command()
	data_count = offset
	}
{
#
# Process the input record according to its 'type'
#
if ($0 ~ /^!/) process_comments()
else if ($0 ~ /^\*/) process_interpretations()
else process_data()
}

############################################################################
#				FUNCTIONS USED IN NUM.AWK
############################################################################

############################################################################
##				Function Parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command(    good_options,wrong_options)
	{
	good_options = "[aefhinpPoOrRsSTxzZ]"
	wrong_options = "[^aefhinpPoOrRsSTxzZ]"
	if (ARGV[2] != "null")
		{
		options = ARGV[2]
		gsub(/-/,"",options)
		if (options ~ wrong_options)
			{
			gsub(good_options,"",options)
			print "num: ERROR: Invalid option(s) specified: " options \
				 > stderr
			print USAGE > stderr
			exit
			}
		}
	if (options ~ /a/)
		{
		if (ARGV[3] !~ /^\*\*[^\t]/)
			{
			print "num: ERROR: Invalid interpretation specified with "\
				 "-a option: " ARGV[3] > stderr
			print USAGE > stderr
			exit
			}
		else if (options ~ /[efp]/)
			{
			print "num: ERROR: Cannot specify -e, -f, or -p options with -a"\
				 " option." > stderr
			print USAGE > stderr
			exit
			}
		else attach = ARGV[3]
		}
	if (options ~ /i/)
		{
		if (ARGV[4] !~ number_reg)
			{
			print "num: ERROR: Invalid number specified with -i "\
				 "option: " ARGV[4] "."> stderr
			print USAGE > stderr
			exit
			}
		else increment = ARGV[4]
		}
	if (options ~ /n/) number = ARGV[5]
	if (options ~ /o/)
		{
		if (ARGV[6] !~ number_reg)
			{
			print "num: ERROR: Invalid number specified with -o "\
				 "option: " ARGV[6] "."> stderr
			print USAGE > stderr
			exit
			}
		else offset = ARGV[6]
		}
	if (options ~ /O/)
		{
		if (ARGV[7] !~ number_reg)
			{
			print "num: ERROR: Invalid number specified with -O "\
				 "option: " ARGV[7] "."> stderr
			print USAGE > stderr
			exit
			}
		else Offset = ARGV[7]
		}
	if (options ~ /[pP]/)
		{
		if (options ~ /[efa]/)
			{
			print "num: ERROR: Cannot specify -e, -f, or -a options with -p"\
				 " or -P options." > stderr
			print USAGE > stderr
			exit
			}
		else if (options ~ /p/) position = ARGV[15]
		else if (options ~ /P/) POSITION = ARGV[16]
		}
	if (options ~ /r/) resume = ARGV[8]
	if (options ~ /R/) Resume = ARGV[9]
	if (options ~ /s/) suspend = ARGV[10]
	if (options ~ /S/) Suspend = ARGV[11]
	if (options ~ /x/) exclude = ARGV[12]
	if (options ~ /z/) reset = ARGV[13]
	if (options ~ /Z/) Reset = ARGV[14]
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ARGV[5] = ARGV[6] = ARGV[7] = ""
	ARGV[8] = ARGV[9] = ARGV[10] = ARGV[11] = ARGV[12] = ARGV[13] = ""
	ARGV[14] = ARGV[15] = ARGV[16] = ""
	}

###############################################################################
##					Function process_comments
#	This function processes comment records according to the options
#
function process_comments()
	{
	if ($0 ~ /^![^!]/ && options ~ /a/) $0 = $0 "\t!"
	print $0
	}

#############################################################################
##					Function process_interpretations
#	This function processes interpretation records according to the options
#
function process_interpretations()
	{
	if ($0 ~ exclusive_record && $0 !~ null_interp)
		{
		#
		# If this is the first interpretation record, set the values and add
		# an extra spine if -a option specified
		#
		if (!first_interpretation_yet)
			{
			first_interpretation_yet = TRUE
			data_count = offset
			if (options ~ /a/) $0 = $0 "\t" attach
			}
		else
			{
			if ($0 ~ totally_exclusive && options ~ /T/) data_count = Offset
			if (options ~ /a/) $0 = $0 "\t*"
			}
		}
	#
	# If all spines terminate, reset variables and add a spine-terminator
	# indicator if required by -a option
	#
	else if ($0 ~ spine_terminate)
		{
		first_interpretation_yet = FALSE
		suspended = FALSE
		if (options ~ /a/) $0 = $0 "\t*-"
		}
	else if (options ~ /a/) $0 = $0 "\t*"
	print $0
	}

##############################################################################
##					Function process_data
#	This function processes the data tokens according to the specified options
#
function process_data()
	{
	#
	# Check if the counter should be reset for this record
	#
	if (options ~ /z/ && $0 ~ reset) data_count = Offset
	#
	# If printing of numbers has been suspended
	#
	if (suspended)
		{
		#
		# If we are to resume printing numbers now, do so
		#
		if (options ~ /r/ && $0 ~ resume)
			{
			suspended = FALSE
			check_and_print()
			}
		#
		# If we are to resume printing numbers on the next record, do so
		#
		else if (options ~ /R/ && $0 ~ Resume)
			{
			suspended = FALSE
			print_no_numbers()
			}
		#
		# Otherwise, carry on, printing no numbers
		#
		else print_no_numbers()
		#
		# If printing of numbers should be suspended with next record, do so
		#
		if (options ~ /S/ && $0 ~ Suspend) suspended = TRUE
		}
	#
	# If printing has not been suspended
	#
	else
		{
		#
		# If printing of numbers should be suspended now, do so
		#
		if (options ~ /s/ && $0 ~ suspend)
			{
			suspended = TRUE
			print_no_numbers()
			}
		#
		# If printing of numbers should be suspended with next record, do so
		#
		else if (options ~ /S/ && $0 ~ Suspend)
			{
			suspended = TRUE
			check_and_print()
			}
		#
		# Otherwise, print numbers for this record
		#
		else check_and_print()
		#
		# If we are to resume printing numbers on the next record, do so
		#
		if (options ~ /R/ && $0 ~ Resume) suspended = FALSE
		}
	#
	# Update the data counter depending on the -n and -x options
	#
	if (options ~ /n/)
		{
		if (options ~ /x/)
			{
			if ($0 ~ number && $0 !~ exclude) data_count += increment
			}
		else if ($0 ~ number) data_count += increment
		}
	else if (options ~ /x/)
		{
		if ($0 !~ exclude) data_count += increment
		}
	else data_count += increment
	#
	# If the counter is to be reset for the next record, do so now
	#
	if (options ~ /Z/ && $0 ~ Reset) data_count = Offset
	}

##############################################################################
##					Function check_and_print
#	This function determines if numbers should be added to the record and in
# what format according to the options.  It then prints the record.
#
function check_and_print()
	{
	#
	# If -n option specified
	#
	if (options ~ /n/)
		{
		#
		# If -x option was also specified
		#
		if (options ~ /x/)
			{
			#
			# Print numbers only if both conditions are satisfied
			#
			if ($0 ~ number && $0 !~ exclude) print_numbers()
			else print_no_numbers()
			}
		#
		# If no -x option, print the record if it satisfies the condition
		#
		else if ($0 ~ number) print_numbers()
		#
		# Otherwise, print no numbers
		#
		else print_no_numbers()
		}
	#
	# If only the -x option was specified
	#
	else if (options ~ /x/)
		{
		#
		# Print the numbers if the condition is satisfied
		#
		if ($0 !~ exclude) print_numbers()
		else print_no_numbers()
		}
	#
	# Otherwise, print the numbers
	#
	else print_numbers()
	}

##############################################################################
##					Function print_numbers
#	This function does the actual printing of records with number counts.
#
function print_numbers(    i,output,token)
	{
	output = ""
	#
	# Print the numbers on the record according to the -a, -e, and -f options
	#
	if (options ~ /a/) output = $0 "\t" data_count
	else if (options ~ /f/)
		{
		if (options ~ /e/)
			{
			output = $1 data_count
			for (i = 2; i <= NF; i += 1) output = output "\t" $i data_count
			}
		else
			{
			output = data_count $1
			for (i = 2; i <= NF; i += 1) output = output "\t" data_count $i
			}
		}
	else if (options ~ /e/)
		{
		output = $1 data_count
		for (i = 2; i <= NF; i += 1) output = output "\t" $i
		}
	else if (options ~ /p/)
		{
		for (i = 1; i <= NF; i += 1)
			{
			if (match($i,position) > 0)
				{
				token = substr($i,1,RSTART-1) substr($i,RSTART,RLENGTH) \
					   data_count substr($i,RSTART+RLENGTH,\
					   length($i)-(RSTART+RLENGTH-1))
				}
			else token = $i
			output = output token "\t"
			}
		output = substr(output,1,length(output)-1)
		}
	else if (options ~ /P/)
		{
		for (i = 1; i <= NF; i += 1)
			{
			if (match($i,POSITION) > 0)
				{
				token = substr($i,1,RSTART-1) data_count \
					   substr($i,RSTART,RLENGTH) substr($i,RSTART+RLENGTH,\
					   length($i)-(RSTART+RLENGTH-1))
				}
			else token = $i
			output = output token "\t"
			}
		output = substr(output,1,length(output)-1)
		}
	else
		{
		output = data_count $1
		for (i = 2; i <= NF; i += 1) output = output "\t" $i
		}
	print output
	}

##############################################################################
##					Function print_no_numbers
#	This function does the actual printing of records without number counts.
#
function print_no_numbers(    i)
	{
	if (options ~ /a/) print $0 "\t."
	else print $0
	}
