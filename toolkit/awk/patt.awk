#####################################################################
##                          PATT.AWK
# Programmed by:  David Huron     Date: Dec., 1989
#
# Modifications:
#   Date:      Programmer:    Description:
# 1993 Aug.5   David Huron    Added spine-path indicators
# 1993 Aug.19  David Huron    Added **patt output
# May 17/94	Tim Racinsky	Solidified options -- c, e, s, t, f
# June 7/94	Tim Racinsky	Modified to work with getopts
# June 20/94	Tim Racinsky	Added -m option (exhaustive search)
#
# The purpose of this program is to identify all occurrences of
# a user-defined pattern in the input stream.
#
# Main Functions used:
#	parse_command()		store_indicators()			store_new_interps()
#	process_indicators()	ins_array_pos()			del_array_pos()
#	exchange_spines()		reset_new()				reset_space()
#	process_comments()		process_interpretations()	process_pattern()
#	process_tag()			process_echo()				process_no_option()
#
#	VARIABLES:
#
#  -stderr:  variable holding destination of error messages
#  -first_file:  flag to indicate if this is the first file to be processed
#  -no_interpretation_yet:  flag to indicate if first interp. record processed
#  -current_no_of_spines:  holds current number of active spines
#  -null_interp:  Regular expression to match null interpretations
#  -spine_path_record:  Regular expression to match spine-path records
#  -spine_terminate:  Regular expression to match all spines terminating
#  -options:  Holds options specified by the user
#  -skip_reg:  Holds skip regular expression for -s option (if specified)
#  -output_tag:  Holds output tag for -t option (if specified)
#  -pattern_length:  Holds length of pattern to match
#  -token_length:  Holds current number of records read into token array
#  -buffer_length:  Holds current number of records read into buffer array
#  -pattern_start:  Pointer into buffer array showing where next possible 
#                   pattern may start
#  -interp_head:  Pointer showing where to insert next exclusive interpretation
#                 record into past_interp array (see past_interp definition)
#  -interp_tail:  Same as above except showing where to delete next
#  -buffer_start:  Pointer into buffer array after last printed entry
#
#	ARRAYS:
#
#  -path_indicator:  Holds spine-path indicator symbols for current spine-path
#                    record
#  -current_interp:  Holds exclusive interpretations for currently active spines
#  -past_interp:  Holds exclusive interpretations that were active at the time
#                 for the last set of records that may be part of a pattern.
#                 There are pattern_length + 1 elements and it is implemented as
#                 a limited queue -- thus, the header and tail pointers move
#                 mod (pattern_length + 1).  This array is necessary so that if
#                 a pattern is found, and the -e options was specified, the
#                 exclusive interpretations that were active when the pattern
#                 started can be printed.
#  -pattern:  Holds the pattern to match (read in from template file)
#  -token:  Holds the latest set of records to test against pattern array
#  -buffer:  Holds all records read in so that they may be output again.
#
BEGIN {
	USAGE="\nUSAGE: patt -h                   (Help Screen)\n       patt [-c"\
		 "em] [-t output_tag] [-s skip_reg] -f template [file ...]"
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
	null_interp = "^\\*(\t\\*)*$"
	spine_path_record = "^\\*(\\+|v|\\^|x|-)?(\t\\*(\\+|v|\\^|x|-)?)*$"
	spine_terminate = "^\\*-(\t\\*-)*$"
	#
	# Parse command line options
	#
	no_interpretation_yet = TRUE
	current_no_of_spines = 0
	token_length = buffer_length = pattern_start = 0
	interp_head = interp_tail = 0
	buffer_start = 1
	first_file = TRUE
	options = ""
	skip_reg = ""
	output_tag = ""
	pattern_length = 0
	parse_command()
}
{
#
# If this is the first record of a file, reset variables and arrays
#
if (FNR == 1) reset_new()
#
# If the buffer has more than 5000 entries, clean it up
#
if (buffer_length > 5000) reset_space()
#
# Process the records according to their record type
#
if ($0 ~ /^!/ && options !~ /c/) process_comments()
else if ($0 ~ /^\*/) process_interpretations()
else process_pattern()
}
END {
	#
	# If -m option specified, there may be some more patterns to find in
	# the last few lines of data
	#
	if (options ~ /m/)
		{
		while (buffer_start <= buffer_length)
			{
			if (options ~ /t/) process_tag()
			else if (options ~ /e/) process_echo()
			else process_no_option()
			}
		}
	#
	# Be sure to flush the buffer if -t option specified
	#
	else if (options ~ /t/)
		{
		for (i = buffer_start; i <= buffer_length; i += 1) print buffer[i]
		}
	}

############################################################################
#				FUNCTIONS USED IN PATT.AWK
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
	# Store the skip option and value if specified
	#
	if (ARGV[3] != "null")
		{
		skip_reg = ARGV[3]
		options = options "s"
		}
	#
	# Store the tag option and value if specified
	#
	if (ARGV[4] != "null")
		{
		output_tag = ARGV[4]
		options = options "t"
		}
	if (options ~ /t/ && options ~ /e/)
		{
		print "patt: ERROR: Cannot specify both -t and -e." > stderr
		print USAGE > stderr
		exit
		}
	#
	# Read in the "template" file.
	#
	while (getline < ARGV[5] > 0) pattern[++pattern_length] = $0
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ARGV[5] = ""
	}

############################################################################
##				Function reset_new
#	This function resets the variables when processing a new file
#
function reset_new(   i)
	{
	#
	# If this isn't the first file, flush the buffer
	#
	if (!first_file)
		{
		if (options ~ /t/)
			{
			for (i = buffer_start; i <= buffer_length; i += 1)
				print buffer[i]
			}
		}
	else first_file = FALSE
	#
	# Reset all variables and delete arrays
	#
	token_length = buffer_length = pattern_start = 0
	interp_head = interp_tail = 0
	buffer_start = 1
	for (i in buffer) delete buffer[i]
	for (i in token) delete token[i]
	no_interpretation_yet = TRUE
	current_no_of_spines = 0
	if (FILENAME == "-") file_name = "STNDIN"
	else file_name = FILENAME
	}

############################################################################
##				Function reset_space
#	This function resets the variables when the buffer array becomes too large
#
function reset_space(    i,counter,max_length)
	{
	counter = 0
	max_length = buffer_length
	#
	# Move the unprinted buffer information back to the start of the array
	#
	for (i = buffer_start; i <= buffer_length; i += 1)
		{
		buffer[++counter] = buffer[i]
		}
	#
	# Reset the variables and delete unused array elements
	#
	pattern_start = 1 + pattern_start - buffer_start
	buffer_start = 1
	buffer_length = counter
	for (i = counter + 1; i <= max_length; i += 1) delete buffer[i]
	}

###########################################################################
##				Function process_comments
#	This function processes comments when the -c option is not specified. If
# the -t option was specified and this is a local comment, add a null local
# comment to the end of the record.  Always store it in the buffer.
#
function process_comments()
	{
	if (options ~ /t/ && $0 ~ /^![^!]/) $0 = $0 "\t!"
	buffer[++buffer_length] = $0
	}

############################################################################
##				Function process_interpretations
#	This function processes interpretation records
#
function process_interpretations()
	{
	#
	# If this is the first exclusive interpretation, store the interps., set
	# the variables and add the **patt spine if -t selected
	#
	if (no_interpretation_yet)
		{
		current_no_of_spines = NF
		no_interpretation_yet = FALSE
		store_new_interps()
		if (options ~ /t/) $0 = $0 "\t**patt"
		}
	else
		{
		#
		# If its a spine-path record, process the spine-path indicators
		#
		if ($0 ~ spine_path_record && $0 !~ null_interp)
			{
			store_indicators()
			process_indicators()
			if (current_no_of_spines == 0) no_interpretation_yet = TRUE
			}
		else store_new_interps()
		#
		# If the -t option specified, add the extra null interpretation or a
		# spine-path terminator if all other spines terminated
		#
		if (options ~ /t/)
			{
			if ($0 ~ spine_terminate) $0 = $0 "\t*-"
			else $0 = $0 "\t*"
			}
		}
	#
	# Store the record in the buffer
	#
	buffer[++buffer_length] = $0
	}

###########################################################################
##				Function process_pattern
#	This function processes the records which may make up a pattern
#
function process_pattern(   i,interp_line)
	{
	interp_line = ""
	#
	# If -t option, append a null token or null local comment
	#
	if (options ~ /t/)
		{
		if ($0 ~ /^![^!]/) $0 = $0 "\t!"
		else if ($0 !~ /^!!/) $0 = $0 "\t."
		}
	#
	# If -e option, store the exclusive interpretations for the current record
	# in past_interp.  If the array is full (number of elements = 
	# pattern_length, then delete the oldest element (at interp_tail)
	#
	else if (options ~ /e/)
		{
		for (i = 1; i <= current_no_of_spines; i += 1)
			{
			if (i == current_no_of_spines)
				interp_line = interp_line current_interp[i]
			else
				interp_line = interp_line current_interp[i] "\t"
			}
		if (((interp_head + 1) % (pattern_length + 1)) == interp_tail)
			interp_tail = (interp_tail + 1) % (pattern_length + 1)
		past_interp[interp_head] = interp_line
		interp_head = (interp_head + 1) % (pattern_length + 1)
		}
	#
	# Store the record and process if this record should not be skipped
	#
	buffer[++buffer_length] = $0
	if (options !~ /s/ || (options ~ /s/ && $0 !~ skip_reg))
		{
		#
		# Add the record to token array and set pattern_start pointer if
		# this is the first record of the first possible pattern
		#
		token[++token_length] = $0
		if (token_length == 1) pattern_start = buffer_length
		#
		# If there are enough records in token array, check for match
		#
		if (token_length == pattern_length)
			{
			if (options ~ /t/) process_tag()
			else if (options ~ /e/) process_echo()
			else process_no_option()
			}
		}
	}

#########################################################################
##				Function process_tag
#	This function will process the patterns as specified by -t option
#
function process_tag(    i,j,one_match,match_found)
	{
	#
	# Loop through the arrays to look for a pattern
	#
	match_found = TRUE
	#
	# If -m option was specified, then match as many patterns on each line
	# as is possible
	#
	if (options ~ /m/)
		{
		one_match = FALSE
		j = 1
		i = 1
		while (match_found && i <= pattern_length)
			{
			if (token[j] !~ pattern[i] && !one_match) match_found = FALSE
			else if (token[j] !~ pattern[i] && one_match)
				{
				j += 1
				one_match = FALSE
				}
			else
				{
				i += 1
				one_match = TRUE
				}
			}
		}
	else
		{
		for (i = 1; i <= pattern_length; i += 1)
			{
			if (token[i] !~ pattern[i]) { match_found = FALSE ; break }
			}
		}
	#
	# If they match, append the output tag to the end of the line
	#
	if (match_found)
		{
		if (buffer[pattern_start] ~ /^!!/)
			buffer[pattern_start] = buffer[pattern_start] " " output_tag
		else if (buffer[pattern_start] ~ /^!/)
			buffer[pattern_start] = buffer[pattern_start] output_tag
		else
			buffer[pattern_start] = substr(buffer[pattern_start],1,\
							length(buffer[pattern_start])-1) output_tag
		}
	#
	# Regardless, print any records up to the start of the current pattern
	# start record
	#
	for (i = buffer_start; i <= pattern_start; i += 1) print buffer[i]
	#
	# Determine where the next possible pattern could start and then reset
	# the token array
	#
	i = pattern_start + 1
	while ((buffer[i] ~ /^[*!]/) || (options ~ /s/ && buffer[i] ~ skip_reg))
		{
		if (options ~ /c/ && buffer[i] ~ /^!/) break
		print buffer[i++]
		}
	buffer_start = pattern_start = i
	for (i = 1; i <= token_length; i += 1) token[i] = token[i+1]
	token_length -= 1
	}

#########################################################################
##				Function process_echo
#	This function will process the patterns as specified by -e option
#
function process_echo(   i,j,one_match,match_found,terminate,spine_count\
								,data_count,temp_array,end_of_pattern)
	{
	#
	# Loop through the arrays to look for a pattern
	#
	match_found = TRUE
	terminate = ""
	#
	# If -m option was specified, then match as many patterns on each line
	# as is possible
	#
	if (options ~ /m/)
		{
		one_match = FALSE
		j = 1
		i = 1
		while (match_found && i <= pattern_length)
			{
			if (token[j] !~ pattern[i] && !one_match) match_found = FALSE
			else if (token[j] !~ pattern[i] && one_match)
				{
				j += 1
				one_match = FALSE
				}
			else
				{
				i += 1
				one_match = TRUE
				}
			}
		end_of_pattern = j
		}
	else
		{
		for (i = 1; i <= pattern_length; i += 1)
			{
			if (token[i] !~ pattern[i]) { match_found = FALSE ; break }
			}
		end_of_pattern = i - 1
		}
	#
	# If they match, print a comment showing this; print the correct exclusive
	# interpretations for the first record of the pattern; print the records
	# of the pattern (and any other records in between); and print some
	# spine-path terminators.
	#
	if (match_found)
		{
		data_count = 1
		print "!! Pattern found at line " \
		      FNR - (buffer_length-pattern_start) " of file " file_name
		if (past_interp[interp_tail] != "") print past_interp[interp_tail]
		#
		# Print out only the data records that were matched by the pattern
		# (may be less than pattern_length because of -x option)
		#
		for (i = pattern_start; i <= buffer_length; i += 1)
			{
			if (data_count > end_of_pattern) break
			else if (buffer[i] ~ /^\*/) print buffer[i]
			else if (buffer[i] ~ /^!/ && options !~ /c/) print buffer[i]
			else if (options ~ /s/ && buffer[i] ~ skip_reg) print buffer[i]
			else
				{
				print buffer[i]
				data_count += 1
				}
			}
		#
		# Determine how many spines were active during the last data record
		# and print out the appropriate number of spine-path terminators
		#
		i -= 1
		while (buffer[i] ~ /^!!/ && i >= pattern_start) i -= 1
		if (i < pattern_start || buffer[i] ~ spine_terminate) spine_count = 0
		else spine_count = split(buffer[i],temp_array,"\t")
		if (spine_count > 0) terminate = "*-"
		for (i = 2; i <= spine_count; i += 1)
			terminate = terminate "\t*-"
		if (terminate != "") print terminate
		}
	#
	# Determine where the next possible pattern could start and then reset
	# the token array
	#
	i = pattern_start + 1
	while ((buffer[i] ~ /^[*!]/) || (options ~ /s/ && buffer[i] ~ skip_reg))
		{
		if (options ~ /c/ && buffer[i] ~ /^!/) break
		i += 1
		}
	buffer_start = pattern_start = i
	for (i = 1; i <= token_length; i += 1) token[i] = token[i+1]
	token_length -= 1
	}

#########################################################################
##				Function process_no_option
#	This function will process the patterns as specified when neither the
# -t nor the -e options were specified.
#
function process_no_option(   i,j,one_match,match_found)
	{
	#
	# Loop through the arrays to look for a pattern
	#
	match_found = TRUE
	#
	# If -m option was specified, then match as many patterns on each line
	# as is possible
	#
	if (options ~ /m/)
		{
		one_match = FALSE
		j = 1
		i = 1
		while (match_found && i <= pattern_length)
			{
			if (token[j] !~ pattern[i] && !one_match) match_found = FALSE
			else if (token[j] !~ pattern[i] && one_match)
				{
				j += 1
				one_match = FALSE
				}
			else
				{
				i += 1
				one_match = TRUE
				}
			}
		}
	else
		{
		for (i = 1; i <= pattern_length; i += 1)
			{
			if (token[i] !~ pattern[i]) { match_found = FALSE ; break }
			}
		}
	#
	# If a match was found, print a comment showing this
	#
	if (match_found)
		print "!! Pattern found at line " \
		      FNR - (buffer_length-pattern_start) " of file " file_name
	#
	# Determine where the next possible pattern could start and then reset
	# the token array
	#
	i = pattern_start + 1
	while ((buffer[i] ~ /^[*!]/) || (options ~ /s/ && buffer[i] ~ skip_reg))
		{
		if (options ~ /c/ && buffer[i] ~ /^!/) break
		i += 1
		}
	buffer_start = pattern_start = i
	for (i = 1; i <= token_length; i += 1) token[i] = token[i+1]
	token_length -= 1
	}

################################################################
##			Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
#
function store_new_interps(      j)
	{
	#
	# Loop through each valid spine
	#	
	for (j = 1; j <= current_no_of_spines; j++)
		{
		#
		# Store exclusive interpretations in current_interp[]
		#
		if ($j ~ /^\*\*/) current_interp[j] = $j
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
		# If the current field is a null interpretation
		#
		if ($i == "*") path_indicator[i] = "*"
		#
		# If the current field is a spine-path indicator, store it
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
function process_indicators(    i)
	{
	i = 1
	#
	# Loop through the entire array of 'path_indicator'
	# NOTE: This array may be altered during the process
	#
	while (i <= current_no_of_spines)
		{
		#
		# If a spine-split indicator occurred
		#
		if (path_indicator[i] == "^")
			{
			#
			# Insert a new array position in 'path_indicator'
			# and 'current_interp'
			#
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = current_interp[i]	# Copy interpretations
											# to the new spine.
			}
		#
		# If a spine-termination indicator occurred
		#
		if (path_indicator[i] == "-")
			{
			#
			# Delete that spine from the arrays and look at the same
			# position the next time through the loop
			#
			del_array_pos(i)
			i--
			}
		#
		# If a spine-exchange indicator occurred
		#
		if (path_indicator[i] == "x")
			{
			#
			# Exchange the paths
			#
			exchange_spines()
			}
		#
		# If a join-path indicator occurred
		#
		if (path_indicator[i] == "v")
			{
			del_array_pos(i+1)	# Delete NEXT spine (must be join-path
							# spine according to humdrum).
			#
			# If the next indicator is not a join-path indicator
			# then return to original conditions
			#
			if (path_indicator[i+1] != "v")
				{
				path_indicator[i] = "*"
				}
			#
			# Otherwise, look at current and next paths to join
			#
			else i--
			}
		#
		# If a new-path indicator occurred
		#
		if (path_indicator[i] == "+")
			{
			#
			# Insert a new position in the arrays
			#
			ins_array_pos(i+1)
			current_interp[i+1] = ""
			}
		i++
		}
	}

############################################################
##			Function Ins_array_pos
#	This function inserts a new position in the arrays 'path_indicator'
# and 'current_interp' and copies elements so that order is preserved
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j--)
		{
		path_indicator[j] = path_indicator[j-1]
		current_interp[j] = current_interp[j-1]	
		}
	path_indicator[current_element-1] = "*"	# (Avoid possibility of
	path_indicator[current_element] = "*"	#  reprocessing the old spine.)
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

###################################################################
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in 'current_interp'
#
function exchange_spines(    j,count,arraya,temp)
	{
	count = 1
	# Determine which spines are exchanged
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		if (path_indicator[j] == "x")
			{
			arraya[count] = j
			count += 1
			path_indicator[j] = "*"
			}
		}
	# Exchange interpretations between the two spines
	temp = current_interp[arraya[1]]
	current_interp[arraya[1]] = current_interp[arraya[2]]
	current_interp[arraya[2]] = temp
	}
