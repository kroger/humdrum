#####################################################################
##					CLEAVE.AWK
# Programmed by: Tim Racinsky         Date: May, 1994
# Copyright (c) 1994 David Huron
#
# Modifications:
#    Date:   Programmer:         	Description:
#
#	This program allows the user to specify specific exclusive interpretations
# to process and these spines are joined together to form one output spine.
# This program is called by 'cleave.ksh'.
#
#	The following gives a brief summary of the available options:
#
#	-r		suppress output of repeated signifiers (in different tokens)
#	-d delim	specify delimiter to place between tokens
#	-i	specify input interpretations
#	-o	specify output interpretations
#
# Main Functions used
#	parse_command()			set_input_array()		process_locals()
#	process_interpretations()	process_exclusive()		process_tandem()	
#	process_spine_path()		process_data()			determine_order()
#	store_new_interps()			store_indicators()		process_indicators()
#	ins_array_pos()			del_array_pos()		exchange_spines()
#
#	VARIABLES:
#
#  -stderr:  Determines where error messages are sent
#  -options:  Holds which options were selected by user
#  -delimiter:  Holds possible delimiting characters specified by user
#  -delim_reg:  Holds the delimiter in a form that can be used
#			 in a regular expression
#  -output_interp:  Holds output interpretation name
#  -null_interp:  Regular expression for null interpretations
#  -exclusive_interp:  Regular expression for exclusive interp names
#  -exclusive_record:  Regular expression for exclusive interp. records
#  -tandem_interp:  Regular expression for tandem interpretation name
#  -tandem_record:  Regular expression for tandem interp. record
#  -no_interpretation_yet:  Flag to indicate if first exclusive has been seen
#  -current_no_of_spines:  Holds current number of active spines
#  -number_of_interps:  Holds number of input interpretations SPECIFIED
#  -max_present: Holds number of input interpretations currently ACTIVE in input
#
#	ARRAYS:
#
#  -path_indicator:  Holds spine-path indicators from current spine-path record
#  -current_interp:  Holds exclusive interpretations for each active spine
#  -input_interps:  Holds input interpretations in order specified on the
#                   command line
#  -order_array:  Each array element (indexed from 1 to max_present) holds the
#                 position of the next spine to be processed
#
BEGIN {
	USAGE="\nUSAGE: cleave -h               (Help Screen)\n       cleave "\
		 "[-r] [-d delim] -i '**in_interp1,**in_interp2,...' \n       "\
		 "       -o '**out_interp' [file ...]"
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
	# Set regular expressions for program
	#
	null_interp = "^\\*(\t\\*)*$"
	exclusive_interp = "^\\*\\*[^\t]+$"
	exclusive_record = "^\\*(\\*([^\t])+)?(\t\\*(\\*([^\t])+)?)*$"
	tandem_interp = "([vx+^-][^\t]+)|([^*vx+\t^-][^\t]*)"
	tandem_record = "^\\*(" tandem_interp ")?(\t\\*(" tandem_interp ")?)*$"
	#
	# Set global variables
	#
	no_interpretation_yet = TRUE
	current_no_of_spines = number_of_interps = max_present = 0
	options = delimiter = delim_reg = output_interp = ""
	#
	# Determine the options specified by the user
	#
	parse_command()
	}
{
#
# Process each type of record according to the first symbol(s) in each record
#
if ($0 ~ /^!!/) print $0
else if ($0 ~ /^!/) process_locals()
else if ($0 ~ /^\*/) process_interpretations()
else process_data()
}

############################################################################
#				FUNCTIONS USED IN CLEAVE.AWK
############################################################################

############################################################################
##				Function parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	ARGV[1] = ""
	if (ARGV[2] != "null") options = ARGV[2]
	if (ARGV[3] != "null")
		{
		delimiter = ARGV[3]
		delim_reg = delimiter
		if (delim_reg ~ /\\/)
			{
			gsub(/\\/,"",delim_reg)
			delim_reg = "\\\\" delim_reg
			}
		if (delim_reg ~ /\]/)
			{
			gsub(/]/,"",delim_reg)
			#delim_reg = "\]" delim_reg
			delim_reg = "]" delim_reg
			}
		if (delim_reg ~ /\^/)
			{
			gsub(/\^/,"",delim_reg)
			#delim_reg = delim_reg "\^"
			delim_reg = delim_reg "^"
			}
		}
	set_input_array(ARGV[4])
	#
	# Ensure the output interpretation is correct
	#
	sub(/^[ ]+/,"",ARGV[5])
	if (ARGV[5] !~ /^\*\*/)
		{
		print "cleave: ERROR: Invalid output interpretation: " ARGV[5] >stderr
		print USAGE > stderr
		exit
		}
	else output_interp = ARGV[5]
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ARGV[5] = ""
	}

######################################################################
##				Function set_input_array
#	This function checks to see if the input interpretations are correct
# and if so, sets up the input interpretation array so that they can be
# processed correctly.
#
function set_input_array(command_line,     arraya,j)
	{
	number_of_interps = split(command_line,arraya,",")
	#
	# Loop through each intepretation given on the command line and store
	# them in the array input_interps if they are valid
	#
	for (j = 1; j <= number_of_interps; j += 1)
		{
		sub(/^[ ]+/,"",arraya[j])
		if (arraya[j] !~ /^\*\*/)
			{
			print "cleave: ERROR: Invalid input interpretation: " \
												arraya[j] > stderr
			print USAGE > stderr
			exit
			}
		input_interps[j] = arraya[j]
		}
	}

##########################################################################
##				Function process_locals
#	This function processes local comments
#
function process_locals(   dont_print,local_comment,i,j)
	{
	dont_print = FALSE
	local_comment = ""
	#
	# Loop through the spines to be processed and if they have the same
	# local comment, print it.  Otherwise print a null local comment
	#
	for (i = 1; i <= max_present; i += 1)
		{
		if (local_comment == "") local_comment = $order_array[i]
		else if ($order_array[i] != local_comment)
			{
			dont_print = TRUE
			break
			}
		}
	if (dont_print || local_comment == "") print "!"
	else print local_comment
	}

#######################################################################
##				Function process_interpretations
#	This function will process the interpretation records
#
function process_interpretations()
	{
	#
	# Process interpretation records according to their 'type'
	#
	if ($0 ~ exclusive_record && $0 !~ null_interp) process_exclusive()
	else if ($0 ~ tandem_record && $0 !~ null_interp) process_tandem()
	else process_spine_path()
	}

############################################################################
##				Function process_exclusive
#	This function processes exclusive interpretations
#
function process_exclusive()
	{
	#
	# If this is the first interpretation record (of the file or after all
	# spines have terminated), then print the output interpretation and
	# set the variables.  Otherwise, just print a null interpretation
	#
	if (no_interpretation_yet)
		{
		print output_interp
		current_no_of_spines = NF
		no_interpretation_yet = FALSE
		}
	else print "*"
	#
	# Store the new interpretations and determine the order in which the
	# spines in the next set of records should be processed.
	#
	store_new_interps()
	determine_order()
	}

############################################################################
##				Function process_tandem
#	This function processes tandem interpretation records
#
function process_tandem(    i,j,tandem_result,dont_print)
	{
	dont_print = FALSE
	tandem_result = ""
	#
	# Loop through the spines to be processed and if they all have the same
	# tandem interpretation, then print it.  Otherwise print a null interp.
	#
	for (i = 1; i <= max_present; i += 1)
		{
		if (tandem_result == "") tandem_result = $order_array[i]
		else if ($order_array[i] != tandem_result)
			{
			dont_print = TRUE
			break
			}
		}
	if (dont_print || tandem_result == "") print "*"
	else print tandem_result
	}

############################################################################
##				Function process_spine_path
#	This function processes spine-path interpretation records
#
function process_spine_path()
	{
	#
	# Store and process the spine-path indicators
	#
	store_indicators()
	process_indicators()
	#
	# If all of the spines have terminated then output a terminator, otherwise
	# print a null interpretation
	#
	if (current_no_of_spines == 0)
		{
		no_interpretation_yet = TRUE
		print "*-"
		}
	else print "*"
	#
	# Determine the order in which the spines should now be processed
	#
	determine_order()
	}

#########################################################################
##				Function process_data
#	This function processes the data records
#
function process_data(     i,j,output_line,temp)
	{
	output_line = ""
	#
	# Loop through each of the spines to be processed
	#
	for (i = 1; i <= max_present; i += 1)
		{
		#
		# If repeated signifiers should be omitted, omit them from subsequent
		# tokens and then assemble the output string
		#
		if (options ~ /r/ && output_line != "")
			{
			temp = output_line
			if (temp ~ /\\/) { gsub(/\\/,"",temp) ; temp = "\\\\" temp }
			if (temp ~ /\]/) { gsub(/]/,"",temp) ; temp = "]" temp }
			if (temp ~ /\^/) { gsub(/\^/,"",temp) ; temp = temp "^" }
			if (temp ~ /-/) { gsub(/-/,"",temp) ; temp = temp "-" }
			if (delim_reg != "") gsub("[" delim_reg "]","",temp)
			gsub("[" temp "]","",$order_array[i])
			}
		if ($order_array[i] != "" && $order_array[i] != ".")
			output_line = output_line $order_array[i] delimiter
		}
	#
	# Print either a null token or the output token
	#
	if (output_line == "") print "."
	else if (delimiter == "") print output_line
	else print substr(output_line,1,length(output_line)-1)
	}

##########################################################################
##				Function determine_order
#	This function determines the order in which the current input spines
# should be processed.  The order is stored in array 'order_array'.
#
function determine_order(    i,j)
	{
	max_present = 0
	#
	# Loop through the command line order of interpretations
	#
	for (i = 1; i <= number_of_interps; i += 1)
		{
		#
		# Loop through each of the currently active spines
		#
		for (j = 1; j <= current_no_of_spines; j += 1)
			{
			if (input_interps[i] == current_interp[j])
			order_array[++max_present] = j
			}
		}
	}

################################################################
##			Function store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
#
function store_new_interps(     i)
	{
	#
	# Loop through each valid spine and store any exclusive interpretations
	#	
	for (i = 1; i <= current_no_of_spines; i += 1)
		{
		if ($i ~ /^\*\*/) current_interp[i] = $i
		}
	}

##############################################################
##			Function store_indicators
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
##				Function process_indicators
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
##			Function ins_array_pos
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
	current_no_of_spines += 1
	}

##############################################################
##			Function del_array_pos
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
	current_no_of_spines -= 1
	} 

###################################################################
##				Function exchange_spines
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
