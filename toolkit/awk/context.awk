#####################################################################
##                          CONTEXT.AWK
# Programmed by:  Tim Racinsky     Date: May 25/94
# Copyright (c) 1994 David Huron
#
# Modifications:
#   Date:      Programmer:    Description:
# 1997 Mar 18  David Huron    Fixed bug with -o option
#
#	This program congeals data records to form a contextual frame
#
# Main Functions used:
#	parse_command()		process_terminate()		process_echo()
#	process_data()			flush_buffer()			process_noption()
#	process_boption()		process_eoption()		print_buffer()
#	reset_new()			reset_space()
#
#	VARIABLES:
#
#  -stderr:  Destination of error messages
#  -echo:  Regular expression to match comments, tand. interps., and null tokens
#  -buffer_head:  holds position of last entry in array buffer
#  -buffer_tail:  holds postion of oldest non-printed entry in buffer
#  -data_count:  holds number of 'counted' data records not printed in buffer
#  -storing:  flag to indicate if currently storing records in buffer
#  -printed_pre_nulls:  flag to indicate if pre-nulls have been printed for -p
#  -first_file:  flag to indicate if this is the first file processed
#  -options:  holds options specified by user
#  -begin_reg:  holds regular expression for -b option
#  -end_reg:  holds regular expression for -e option
#  -delimiter:  holds string value for -d option
#  -del_length:  holds length of string for -d option
#  -ignore_reg:  holds regular expression for -i option
#  -omit_reg:  holds regular expression for -o option
#  -number:   holds value for -n option
#  -placement:   holds value for -p option
#
#	ARRAYS:
#
#  -buffer:  Holds data records temporarily so they can be processed
#            several times
#
BEGIN {
	USAGE="\nUSAGE: context -h                   (Help Screen)\n       contex"\
		 "t [-b regexp] [-d string] [-e regexp] [-i regexp] [-n n]\n       "\
		 "        [-o regexp] [-p n] [file ...]"
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
	# Set other global variables
	#
	echo = "^(([!*])|(\\.$))"
	buffer_head = 0
	buffer_tail = 1
	data_count = 0
	storing = FALSE
	printed_pre_nulls = FALSE
	first_file = TRUE
	#
	# Parse command line options
	#
	options = ""
	begin_reg = ""
	end_reg = ""
	delimiter = " "
	del_length = 1
	ignore_reg = ""
	omit_reg = ""
	number = 0
	placement = 0
	parse_command()
	}
{
#
# If this is the first file, set the flag, otherwise, reset the environment
#
if (FNR == 1)
	{
	if (first_file) first_file = FALSE
	else reset_new()
	}
#
# If the buffer array has become too large, delete unused entries
#
if (buffer_head > 5000)
	reset_space()
#
# If there is more than one spine present, print an error
#
if ($0 !~ /^!!/ && NF > 1)
	{
	print "context: ERROR: Only one spine is allowed in input (line " FNR ")"\
		 "." > stderr
	exit
	}
#
# If an exclusive interpretation or spine-path terminator occur, process
#
else if ($0 ~ /^\*\*/ || $0 ~ /^\*-$/) process_terminate()
#
# If a comment, tandem interpretation, or null data token occur, process
#
else if ($0 ~ echo) process_echo()
#
# Otherwise, process the given data record according to the options
#
else process_data()
}

############################################################################
#				FUNCTIONS USED IN CONTEXT.AWK
############################################################################

############################################################################
##				Function Parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	#
	# Delete the DUMMY variable sent from the shell script
	#
	ARGV[1] = ""
	if (ARGV[2] != "null")
		{ begin_reg = ARGV[2] ; options = options "b" }
	if (ARGV[3] != "null")
		{ delimiter = ARGV[3] ; del_length = length(delimiter) }
	if (ARGV[4] != "null")
		{ end_reg = ARGV[4] ; options = options "e" }
	if (ARGV[5] != "null")
		{ ignore_reg = ARGV[5] ; options = options "i" }
	if (ARGV[6] != "null")
		{ number = ARGV[6] ; options = options "n" }
	if (ARGV[7] != "null")
		{ omit_reg = ARGV[7] ; options = options "o" }
	if (ARGV[8] != "null")
		{ placement = ARGV[8] ; options = options "p" }
	if (options ~ /[be]/ && options ~ /[npi]/)
		{
		print "context: ERROR: Cannot specify -b or -e options with "\
			 "-n, -p, or -i options." > stderr
		print USAGE > stderr
		exit
		}
	if (options ~ /n/)
		{
		if (number < 2)
			{
			print "context: ERROR: Number for -n option must be greater "\
				 "than 1." > stderr
			exit
			}
		if (options ~ /p/)
			{
			if (placement < 1)
				{
				print "context: ERROR: Number for -p option must be "\
					 "positive." > stderr
				exit
				}
			else if (placement > number - 1)
				{
				print "context: ERROR: -p number must be at most one less "\
					 "than -n number." > stderr
				exit
				}
			}
		}
	ARGV[2] = ARGV[3] = ARGV[4] = ARGV[5] = ARGV[6] = ARGV[7] = ARGV[8] = ""
	}

##############################################################################
##					Function reset_new
#	This function resets the environment for each new file after the first one.
#
function reset_new(   i)
	{
	buffer_head = 0
	buffer_tail = 1
	data_count = 0
	storing = FALSE
	printed_pre_nulls = FALSE
	for (i in buffer) delete buffer[i]
	}

##############################################################################
##					Function reset_space
#	This function resets the environment when too many lines have been stored
# in the buffer array
#
function reset_space(   i,counter,old_head)
	{
	counter = 0
	old_head = buffer_head
	#
	# Move buffer entries back to the beginning of the array and delete
	# the old entries from the array
	#
	if (buffer_tail <= buffer_head)
		{
		for (i = buffer_tail; i <= buffer_head; i += 1)
			buffer[++counter] = buffer[i]
		buffer_tail = 1
		buffer_head = counter
		for (i = counter + 1; i <= buffer_head; i += 1) delete buffer[i]
		}
	#
	# If buffer_tail is greater than buffer_head, then no entries need to
	# be saved
	#
	else
		{
		buffer_tail = 1
		buffer_head = 0
		for (i = 1; i <= buffer_head; i += 1) delete buffer[i]
		}
	}

##############################################################################
##					Function process_terminate
#	This function processes those records containing a spine-path terminator
# or an exclusive interpretation.
#
function process_terminate(    i,null_tokens)
	{
	#
	# If currently storing records
	#
	if (storing)
		{
		#
		# If -n option was specified
		#
		if (options ~ /n/)
			{
			#
			# If not enough data tokens exist, then print out null tokens
			#
			if (data_count < number)
				{
				for (i = buffer_tail; i <= buffer_head; i += 1)
					{
					if (buffer[i] ~ echo) print buffer[i]
					else print "."
					}
				}
			#
			# Otherwise, print according to the entries in the array and
			# print extra null tokens according to the -p option if specified
			#
			else
				{
				null_tokens = number-1-placement
				for (i = buffer_tail + 1; i <= buffer_head; i += 1)
					{
					if (buffer[i] ~ echo)
						print buffer[i]
					else if (options ~ /o/ && buffer[i] ~ omit_reg)
						print "."
					else if (options ~ /i/ && buffer[i] ~ ignore_reg)
						print "."
					else if (null_tokens > 0)
						{
						print "."
						null_tokens -= 1
						}
					}
				}
			#
			# Reset the variables for the -n option
			#
			data_count = 0
			buffer_tail = buffer_head + 1
			storing = FALSE
			printed_pre_nulls = FALSE
			}
		#
		# Otherwise, for the other options, simply flush the buffer contents
		#
		else if (options ~ /[be]/) flush_buffer()
		}
	#
	# If not storing in buffer array, simply print the record
	#
	print $0
	}

###############################################################################
##					Function flush_buffer
#	This function ouputs the current contents of buffer for the -b and -e
# options.
#
function flush_buffer(   i,output_token,printed)
	{
	printed = FALSE
	output_token = ""
	#
	# Move through the appropriate section of the buffer array and construct
	# the output token of amalgamated data tokens
	#
	for (i = buffer_tail; i <= buffer_head; i += 1)
		{
		if (buffer[i] ~ echo) continue
		else if (options ~ /o/ && buffer[i] ~ omit_reg) continue
		else output_token = output_token buffer[i] delimiter
		}
	output_token = substr(output_token,1,length(output_token) - del_length)
	#
	# Again, loop through the array and print the appropriate tokens at
	# each position
	#
	for (i = buffer_tail; i <= buffer_head; i += 1)
		{
		if (buffer[i] ~ echo) print buffer[i]
		else if (options ~ /o/ && buffer[i] ~ omit_reg) print "."
		else if (printed) print "."
		else { print output_token ; printed = TRUE }
		}
	buffer_tail = buffer_head + 1
	}

##############################################################################
##					Function process_echo
#	This function processes those records containing tokens that should be
# output where they were found in the input.
#
function process_echo()
	{
	if (storing) buffer[++buffer_head] = $0
	else print $0
	}

#############################################################################
##					Function process_data
#	This function processes the data tokens according to the specified options
#
function process_data()
	{
	if (options ~ /n/) process_noption()
	else if (options ~ /b/) process_boption()
	else process_eoption()
	}

############################################################################
##					Function process_noption
#	This function processes data records for the -n option
#
function process_noption(   i)
	{
	#
	# If not currently storing, begin storing
	#
	if (!storing) storing = TRUE
	#
	# If this record should be ignored or omitted, store it in the buffer
	#
	if ((options ~ /i/ && $0 ~ ignore_reg) || (options ~ /o/ && $0 ~ omit_reg))
		{
		if (options ~ /o/) buffer[++buffer_head] = "."	# NEW
		else buffer[++buffer_head] = $0
		}
	else
		{
		#
		# If the data count is not yet full, add this record to the buffer
		# and increment the count
		#
		if (data_count < number)
			{
			buffer[++buffer_head] = $0
			data_count += 1
			#
			# If the data_count is now full, process the data tokens
			#
			if (data_count == number)
				{
				#
				# If the pre-nulls (according to -p option) have not been
				# printed, print them now
				#
				if (!printed_pre_nulls)
					{
					for (i = 1; i <= placement; i += 1) print "."
					printed_pre_nulls = TRUE
					}
				#
				# Print the contents of the buffer and continue to print
				# until the buffer tail is pointing to a data record that is
				# not to be ignored or omitted
				#
				print_buffer()
				if (options ~ /i/ || options ~ /o/)
					{
					while ((options ~ /o/ \
							&& buffer[buffer_tail] ~ omit_reg) \
							|| (options ~ /i/ \
							&& buffer[buffer_tail] ~ ignore_reg))
						{
						buffer_tail += 1
						print_buffer()
						}
					}
				}
			}
		#
		# If the data count is already full, then we just finished printing a
		# set of records and we can do so again by storing the new token and
		# incrementing buffer_tail as well.  Print the buffer again as above
		#
		else if (data_count == number)
			{
			buffer_tail += 1
			buffer[++buffer_head] = $0
			print_buffer()
			if (options ~ /i/ || options ~ /o/)
				{
				while ((options ~ /o/ && buffer[buffer_tail] ~ omit_reg) \
				     || (options ~ /i/ && buffer[buffer_tail] ~ ignore_reg))
					{
					buffer_tail += 1
					print_buffer()
					}
				}
			}
		}
	}

############################################################################
##					Function print_buffer
#	This function prints the buffer for the -n option
#
function print_buffer(   i,output_token)
	{
	output_token = ""
	#
	# Print out any 'non-data' tokens
	#
	i = buffer_tail
	while (buffer[i] ~ echo) { print buffer[i] ; i += 1 }
	buffer_tail = i
	#
	# Loop through the buffer and construct the output token
	#
	for (i = buffer_tail; i <= buffer_head; i += 1)
		{
		if (!(options ~ /o/ && buffer[i] ~ omit_reg) && (buffer[i] !~ echo))
			output_token = output_token buffer[i] delimiter
		}
	#
	# Print the output token
	#
	output_token = substr(output_token,1,length(output_token) - del_length)
	print output_token
	}

############################################################################
##					Function process_boption
#	This function processes data records for the -b option
#
function process_boption()
	{
	#
	# -e option only takes precedence if this record contains a token that is
	# matched by the end regular expression
	#
	if (options ~ /e/ && $0 ~ end_reg) process_eoption()
	#
	# If storing and a 'begin' is found, print buffer and start again
	#
	else if (storing && $0 ~ begin_reg)
		{
		flush_buffer()
		buffer[++buffer_head] = $0
		}
	#
	# If simply storing, then store this record
	#
	else if (storing) buffer[++buffer_head] = $0
	#
	# If this is the first 'begin', then store it and set the flag
	#
	else if ($0 ~ begin_reg)
		{
		buffer[++buffer_head] = $0
		storing = TRUE
		}
	#
	# Otherwise, if this token should be omitted, print null, otherwise print
	#
	else
		{
		if (options ~ /o/ && $0 ~ omit_reg) print "."
		else print $0
		}
	}

############################################################################
##					Function process_eoption
#	This function processes data records for the -e option
#
function process_eoption()
	{
	#
	# If currently storing and this is an end token, store it in the buffer,
	# print the buffer, and set the flag depending on -b option
	#
	if (storing && $0 ~ end_reg)
		{
		buffer[++buffer_head] = $0
		flush_buffer()
		if (options ~ /b/) storing = FALSE
		}
	#
	# If simply storing, then store this record
	#
	else if (storing) buffer[++buffer_head] = $0
	#
	# If this record has an 'end' token, then print it and begin storing if
	# the -b option was not specified
	#
	else if ($0 ~ end_reg)
		{
		print $0
		if (options !~ /b/) storing = TRUE
		}
	#
	# Otherwise, simply store the record and set the storing flag
	#
	else
		{
		storing = TRUE
		buffer[++buffer_head] = $0
		}
	}
