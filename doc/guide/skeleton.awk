#####################################################################
##					PROGRAM.AWK
#
#	This file contains a skeleton of a Humdrum awk program that may
# be of use to you if you wish to create new tools to work along with
# the existing Humdrum tools.  This is NOT a complete program but should
# be used as a guide only.  It would be used in conjunction with
# skeleton.ksh.
#
# Programmed by:                  Date:
#
# Modifications:
#   Date:      Programmer:       Description:
#
#	Place a brief description of the program here.
#
# Main Functions used:
#
#	Parse_command()		Store_indicators()		Store_new_interps()
#	Process_indicators()	Ins_array_pos()		Del_array_pos()
#	Exchange_spines()
#
#				List all other functions used here.
#
#	VARIABLES:
#
#  -current_no_of_spines:  Tracks current number of active spines (necessary
#					  for processing spine-path indicators)
#  -first_interpretation_yet:  Flag to indicate if first interpretation record
#						 (by definition, should be exclusive) has been
#						 encountered yet.
#
#			List and describe all global variables used here.
#
#	ARRAYS:
#
#  -current_interp:  Holds the current exclusive interpretation for each spine
#  -path_indicator:  Holds spine-path indicators for current spine-path record
#
#			List and describe any other global arrays used here.
#
BEGIN {
	#
	# Set appropriate USAGE
	#
	USAGE=""
	#
	# Set any necessary flags, variables, and constants here
	#
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	current_no_of_spines = 0
	first_interpretation_yet = FALSE
	#
	# Set standard error to the appropriate 'file'
	#
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	# Here are some useful regular expressions for any Humdrum file
	#
	#
	# Matches all null interpretation records
	#
	null_interp = "^\\*(\t\\*)*$"
	#
	# Matches a spine-path record but also matches a null interpretation record
	# so it should be used in conjunction with null_interp
	#
	spine_path_record = "^\\*(\\+|v|\\^|x|-)?(\t\\*(\\+|v|\\^|x|-)?)*$"
	#
	# Matches an exclusive interpretation record but it also matches a null
	# interpretation record so it should be used in conjunction with null_interp
	#
	exclusive_record = "^\\*(\\*([^\t])+)?(\t\\*(\\*([^\t])+)?)*$"
	#
	# Matches a totally exclusive interpretation record (i.e. each spine has an
	# exclusive interpretation
	#
	totally_exclusive = "^\\*\\*([^\t])+(\t\\*\\*([^\t])+)*$"
	#
	# Matches a valid tandem interpretation NAME
	#
	tandem_interp = "([vx+^-][^\t]+)|([^*vx+\t^-][^\t]*)"
	#
	# Matches a tandem interpretation record (need tandem_interp to be defined) but
	# also matches a null interpretation record so it should be used in conjunction
	# with null_interp
	#
	tandem_record = "^\\*(" tandem_interp ")?(\t\\*(" tandem_interp ")?)*$"
	#
	# Matches a record consisting of all spine-path terminators
	#
	spine_terminate = "^\\*-(\t\\*-)*$"
	#
	# Determine the options specified by the user, passed from the shell script
	#
	parse_command()
	}
{
if (FNR == 1)
	{
	# 
	# For programs expecting multiple input files, reset any
	# necessary variables here.
	#
	}
if ($0 ~ /^!!/)
	{
	process_global()		# Process global comments
	}
else if ($0 ~ /^!/)
	{
	process_local()		# Process local comments
	}
else if ($0 ~ exclusive_record && $0 !~ null_interp)
	{
	process_exclusive()		# Process exclusive interpretation records
	}
else if ($0 ~ tandem_record && $0 !~ null_interp)
	{
	process_tandem()		# Process tandem interpretation records
	}
else if ($0 ~ spine_path_record && $0 !~ null_interp)
	{
	process_spine_path()	# Process spine-path records
	}
else
	{
	process_data()			# Process data records
	}
}
END	{
	#
	# Some programs may need some 'clean-up' actions after all the records
	# have been processed (careful with mulitiple files, though)
	#
	}

############################################################################
#				FUNCTIONS USED IN PROGRAM.AWK
############################################################################

############################################################################
##				Function parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	#
	# Set the options that were sent from the shell script
	#
	if (ARGV[2] != "null") options = ARGV[2]
	#
	# Avoid processing passed options as input files.
	#
	ARGV[1] =  ARGV[2] = ""
	}

################################################################
##			Function store_new_interps
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
		# Append tandem interpretations to current_interp[]
		#
		if ($j ~ /^\*\*/) current_interp[j] = $j
		else current_interp[j] = current_interp[j] "\t" $j
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
	current_no_of_spines++
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
	current_no_of_spines--
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

###########################################################################
##				Function process_global
#
function process_global()
	{
	#
	# Perform the appropriate operations on global comments here
	#
	}

###########################################################################
##				Function process_local
#
function process_local()
	{
	#
	# Perform the appropriate operations on local comments here
	#
	}

###########################################################################
##				Function process_exclusive
#
function process_exclusive()
	{
	#
	# If this is the first interpretation record, set the variables
	#
	if (!first_interpretation_yet)
		{
		first_interpretationt_yet = TRUE
		current_no_of_spines = NF
		}
	#
	# Store the new interpretations
	#
	store_new_interps()
	}

###########################################################################
##				Function process_tandem
#
function process_tandem()
	{
	#
	# Perform the appropriate operations on tandem interpretations here
	#
	}

###########################################################################
##				Function process_spine_path
#
function process_spine_path()
	{
	store_indicators()
	process_indicators()
	if (current_no_of_spines == 0) first_interpretation_yet = FALSE
	}

###########################################################################
##				Function process_data
#
function process_data()
	{
	#
	# Perform the appropriate operations on data records here
	#
	}
