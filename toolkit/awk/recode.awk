#####################################################################
##					RECODE.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       	Description:
# May 9/94	Tim Racinsky		Fixed 'Eating Backslash' problem (recoded
#							tokens needed an extra '\' added)
# June 7/94	Tim Racinsky		Modified parse_command to work with getopts
#
#
#	This program is called from recode.ksh and is used to 're-code' any
# tokens that contain numeric values for a given exclusive interpretation
# based on a file provided by the user.
#
#	The following gives a brief summary of the available options:
#
#	-s reg_exp: allows user to skip over lines with the given regular expres.
#	-x: suppresses output of non-processed data signifiers
#
# Main Functions used:
#	Parse_command()		Store_indicators()		Store_new_interps()
#	Process_indicators()	Ins_array_pos()		Del_array_pos()
#	Exchange_spines()		Process_data()			Recode()
#
#	VARIABLES:
#
#  -stderr : allows errors to be printed to standard error.
#  -number : regular expression for a valid number.
#  -rel_reg_exp : regular expression for a valid relation
#  -options : holds options specified by user.
#  -reg_exp :  holds regular expression specified by user (if any).
#  -target_interp : holds name of exclusive target interpretation
#  -default_output : holds value of default action.
#  -num_of_conditions : holds number of different conditions to test.
#  -current_no_of_spines : holds current number of active spines.
#  -no_interpretation_yet : flag to indicate if first interp. found yet.
#
#	ARRAYS:
#
#  -current_interp and path_indicator are used to store exclusive
#   interpretations and spine path indicators respectively.
#
BEGIN {
	USAGE="\nUSAGE: recode -h                 (Help Screen)\n       recode "\
		 "[-x] [-s reg_exp] -i **interpretation -f reassign [file ...]"
	#
	# Set any necessary flags, variables, and constants here
	#
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	number = "[+-]?(([0-9]+(\\.[0-9]*)?)|((0*)?\\.[0-9]+))"
	rel_reg_exp = "^((==)|(>=)|(<=)|(>)|(<)|(!=))"
	#
	options = ""
	reg_exp = ""
	target_interp = ""
	default_output = ""
	num_of_conditions = 0
	#
	# Determine the options specified by the user, passed from the shell script
	#
	parse_command()
	}
{
#
# Reset variables for each new file
#
if (FNR == 1)
	{
	current_no_of_spines = 0
	no_interpretation_yet = TRUE
	}
#
# Print all comments
#
if ($0 ~ /^!/) print $0
#
# If the current record is an interpretation record
#
else if ($0 ~ /^\*/)
	{
	#
	# Always print it
	#
	print $0
	#
	# Process the first interpretation record.
	#
	if (no_interpretation_yet)
		{
		current_no_of_spines = NF
		no_interpretation_yet = FALSE
		store_new_interps()
		}
	#
	# Otherwise, check to see if it is a spine-path record
	#
	else
		{
		spine_path_record = FALSE
		for (i = 1; i <= current_no_of_spines ; i++)
			{			
			if ($i ~ /^\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x$/) {
				spine_path_record = TRUE 
				break
				}
			}
		#
		# If it is a spine-path record, store and process the indicators
		#
		if (spine_path_record)
			{
			store_indicators()
			process_indicators()
			#
			# Reset any necessary variables if all spines terminate
			#
			if (current_no_of_spines == 0)
				{
				no_interpretation_yet = TRUE
				}
			}
		#
		# Otherwise, process the interpretations
		#
		else store_new_interps()
		}
	}
#
# Otherwise, process the data record
#
else process_data()
}

############################################################################
#				FUNCTIONS USED IN RECODE.AWK
############################################################################

############################################################################
##				Function Parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	#
	# Process any necessary variables
	#
	if (ARGV[2] != "null") options = ARGV[2]
	if (ARGV[3] != "null") reg_exp = ARGV[3]
	if (ARGV[4] !~ /^\*\*/)
		{
		print "recode: ERROR: Target interpretation must be"\
			 " exclusive" > stderr
		exit
		}
	target_interp = ARGV[4]
	#
	# Read in the "re-assignment" file and parse into three
	# arrays: relation, condition, and action.
	# (Awk automatically generates an error
	# message if the input file is not found.)
	#
	while (getline < ARGV[5] > 0)
		{
		#
		# Parse each input line into three elements:
		# 	relation[]   - i.e. <  <=  >  >=  !=  ==
		#	condition[]  - numeric test, e.g. "3" in "==3"
		#	action[]     - corresponding output when test is true
		#
		# All 3 elements must be present or an error is issued
		#
		if ($1 ~ /^(else)$/) default_output = $2
		else
			{
			if (match($1,rel_reg_exp))	
				{
				relation[++num_of_conditions] = \
									substr($1,RSTART,RLENGTH)
				if (match($1,number))
					{
					condition[num_of_conditions] = \
									substr($1,RSTART,RLENGTH)+0
					if ($2 != "")
						{
						action[num_of_conditions] = $2
						}
					else
						{
						print "recode: ERROR: No action specified " \
							 "on line " num_of_conditions " in " \
							 "reassign file" > stderr
						exit
						}
					}
				else
					{
					print "recode: ERROR: No condition specified " \
						 "on line " num_of_conditions " in " \
						 "reassign file" > stderr
					exit
					}
				}
			else
				{
				print "recode: ERROR: No relation specified " \
					 "on line " num_of_conditions+1 " in " \
					 "reassign file" > stderr
				exit
				}
			}
		}
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ARGV[5] = ""
	}

################################################################
##			Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
#
function store_new_interps(      j)
	{
	#
	# Loop through each valid spine and store exclusive interpretations
	#	
	for (j = 1; j <= current_no_of_spines; j++)
		{
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
			current_interp[i+1] = current_interp[i]
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
			del_array_pos(i+1)
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
	path_indicator[current_element-1] = "*"
	path_indicator[current_element] = "*"
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
function exchange_spines(    j,arraya,count,temp)
	{
	count = 1
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		if (path_indicator[j] == "x")
			{
			arraya[count] = j
			path_indicator[j] = ""
			count += 1
			}
		}
	temp = current_interp[arraya[1]]
	current_interp[arraya[1]] = current_interp[arraya[2]]
	current_interp[arraya[2]] = temp
	}

#######################################################################
## 				Function Process_data
#	This function processes all tokens that belong to the target
# interpretation and makes use of the function recode to obtain the 
# appropriate resulting token.
#	
function process_data(     i,j,arrayb,split_num,current_token,current_line,
												return_token)
	{
	current_line = ""
	#
	# Loop through each spine
	#
	for (i = 1; i <= current_no_of_spines; i += 1)
		{
		current_token = ""
		#
		# If the current spine contains the target interpretation
		#
		if (current_interp[i] == target_interp)
			{
			split_num = split($i,arrayb," ")
			#
			# Loop through each part of a multiple stop
			#
			for (j = 1; j <= split_num; j += 1)
				{
				#
				# If the token contains the user defined regular expression
				# skip it
				#
				if (options ~ /s/ && arrayb[j] ~ reg_exp)
					current_token = current_token arrayb[j] " "
				#
				# Otherwise, process it
				#
				else
					{
					return_token = recode(arrayb[j])
					#
					# Make sure multiple stops don't have null tokens
					#
					if (return_token != "." || split_num == 1)
						current_token = current_token return_token " "
					}
				}
			#
			# Make sure at least a null token is output
			#
			if (current_token == "") current_line = current_line ".\t"
			else current_line = current_line \
					substr(current_token,1,length(current_token)-1) "\t"
			}
		#
		# If the current spine does not contain the target interpretation,
		# simply print it out
		#
		else
			{
			current_line = current_line $i "\t"
			}
		}
	#
	# Print the resulting data record
	#
	print substr(current_line,1,length(current_line)-1)
	}

##########################################################################
##				Function Recode
#	Called from process_data(), this function determines which condition
# the current token satisfies (from top of reassign file to bottom) and
# returns the appropriate token.
#
function recode(token,    i,return_token,value,matched_token)
	{
	return_token = ""
	value = ""
	#
	# If there is a number in the current token
	#
	if (match(token,number))
		{
		#
		# Get the number and loop through all of the conditions. As soon as
		# a condition is satisfied, the loop is broken
		#
		matched_token = substr(token,RSTART,RLENGTH)
		value = matched_token + 0
		sub(number,SUBSEP,token)
		for (i = 1; i <= num_of_conditions; i++)
			{
			if (relation[i] == "==")
				{
				if (value == condition[i])
					{
					return_token = action[i]
					break
					}
				}
			else if (relation[i] == "!=")
				{
				if (value != condition[i])
					{
					return_token = action[i]
					break
					}
				}
			else if (relation[i] == "<=")
				{
				if (value <= condition[i])
					{
					return_token = action[i]
					break
					}
				}
			else if (relation[i] == ">=")
				{
				if (value >= condition[i])
					{
					return_token = action[i]
					break
					}
				}
			else if (relation[i] == "<")
				{
				if (value < condition[i])
					{
					return_token = action[i]
					break
					}
				}
			else if (relation[i] == ">")
				{
				if (value > condition[i])
					{
					return_token = action[i]
					break
					}
				}
			}
		#
		# If there was no matching condition
		#
		if (return_token == "")
			{
			#
			# If there is a default output, return that
			#
			if (default_output != "")
				{
				if (options ~ /x/) return default_output
				else
					{
					sub(SUBSEP,default_output,token)
					return token
					}
				}
			#
			# Else, simply return the token
			#
			else
				{
				if (options ~ /x/) return matched_token
				else
					{
					sub(SUBSEP,matched_token,token)
					return token
					}
				}
			}
		#
		# Otherwise, return the resulting token
		#
		else
			{
			if (options ~ /x/) return return_token
			else
				{
				# This first gsub command is supposed to replace any
				# backslashes with 2 backslashes so that the sub command
				# that follows returns the correct number of backslashes
				# (It doesn't look like it should work but it does!)
				gsub(/\\/,"\\\\\\\\",return_token)
				sub(SUBSEP,return_token,token)
				return token
				}
			}
		}
	#
	# If there was no number in the token, return the entire token
	#
	else
		{
		if (options ~ /x/) return "."
		else return token
		}
	}
