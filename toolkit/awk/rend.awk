#########################################################################
##						REND.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# July 20/94	Tim Racinsky	Modified to use with getopts function in shell
#
#	This program is called from rend.ksh and allows the user to 
# split any spine containing a specific exclusive interpertation into any
# number of resulting spines, depending on a regular expression.
#
# Main Functions used:
#	Parse_command()		Print_interps()		Print_locals()
#	Print_data_records()	Store_indicators()		Print_indicators()
#	Ins_array_pos()		Num_of_exchanges()		Exchange_spines()
#	Del_array_pos()		Store_new_interps()
#
#	VARIABLES:
#
#  -first_interpretation_yet:  indicates if the first exclusive interpretation
#   has been found yet.
#  -current_no_of_spines:  holds the current number of active spines.
#  -max_splits:  holds the number of new spines the target spine is being
#   separated in to.
#
#	ARRAYS:
#
#  -split_interp:  holds the interpretation names of each of the resulting
#   spines from the spine to be separated.
#  -reg_exp:  holds the corresponding regular expressions for each
#   interpretation in split_interp.
#  -current_interp:  holds the current exclusive interpretation for each 
#   currently active spine.
#  -path_indicator:  holds the spine-path indicators from a spine-path
#   record for each currently active spine.
#
BEGIN {
	USAGE="\nUSAGE: rend -h                 (Help Screen)\n       rend -f "\
		 "reassign -i 'target_interp' [-s] [file ...]"
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	options = ""
	max_splits = 0
	#
	# Parse the list of interpretations and regular expressions provided from
	# the user and return the number of new interpretations.
	#
	parse_command()
	}
{
#
# If this is the first record of a file, reset the variables and arrays
#
if (FNR == 1)
	{
	first_interpretation_yet = FALSE
	current_no_of_spines = 0
	for (i in current_interp) delete current_interp[i]
	}
#
# Always print the global comments
#
if ($0 ~ /^!!/) print $0
#
# If the current record is an interpretation record
#
else if ($0 ~ /^\*/)
	{
	#
	# If this is the first interpretation record (exclusive only) store the
	# new interpretations and print the appropriate interpretations.
	#
	if (!first_interpretation_yet)
		{
		current_no_of_spines = NF
		first_interpretation_yet = TRUE
		store_new_interps()
		print_interps()
		}
	else
		{
		#
		# Otherwise check if it is a spine path record or not
		#
		spine_path_record = FALSE
		for (i = 1; i <= current_no_of_spines ; i++)
			{			
			if ($i ~ /^\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x$/) 
				{
				spine_path_record = TRUE 
				break
				}
			}
		#
		# If it is a spine path record, store and print the appropriate
		# spine-path indicators.
		#
		if (spine_path_record)
			{
			store_indicators()
			print_indicators()
			if (current_no_of_spines == 0) first_interpretation_yet = FALSE
			}
		#
		# Otherwise store the new interpretations and print the appropriate
		# interpretations for each spine.
		#
		else 
			{
			store_new_interps()
			print_interps()
			}
		}
	}
#
# If it is a local comment record, print the correct number of local comments
#
else if ($0 ~ /^![^!]/) print_locals()
#
# Lastly, print the correct data tokens for each spine
#
else print_data_records()
}

######################################################################
#				FUNCTIONS USED IN REND
######################################################################

######################################################################
##				Function Parse_command
#	This function stores the target interpretation in a variable and
# the resulting interpretations and regular expressions in two separate
# arrays.  Note that ARGV[2] always contains the target interpretation
# and ARGV[3] contains the number of PAIRS of resulting interpretations where
# each pair represents an exclusive interpretation and a regular expression.
#
function parse_command()
	{
	if (ARGV[2] != "null") options = ARGV[2]
	#
	# Store the target interpretation
	#
	if (ARGV[3] !~ /^\*\*/)
		{
		print "rend: ERROR: Invalid target interpretation specified: "\
			 ARGV[3] > stderr
		print USAGE > stderr
		exit
		}
	target_interp = ARGV[3]
	#
	# Read in the "re-assignment" file and parse into two
	# arrays: split_interp and reg_exp.
	#
	while (getline < ARGV[4] > 0)
		{
		#
		# Both elements must be present or an error is issued
		#
		if ($1 !~ /^\*\*/)
			{
			print "rend: ERROR: Invalid assignment interpretation specified"\
				 " in\n             reassignment file: " $1 > stderr
			exit
			}
		else split_interp[++max_splits] = $1
		if ($2 == "")
			{
			print "rend: ERROR: No regular expression specified for interp"\
				 "retation " $1 "\n             in reassignment "\
				 "file." > stderr
			exit
			}
		else reg_exp[max_splits] = $2
		}
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ""
	}

##################################################################
##			Function Print_Interps
#	This function prints the correct number of interpretations for a
# record containing exclusive or tandem interpretations.
#
function print_interps(    current_record,i,j)
	{
	current_record = ""
	#
	# Loop through each active spine
	#
	for (i = 1; i <= current_no_of_spines; i++)
		{
		#
		# If the current spine contains the target interpretation
		#
		if (current_interp[i] == target_interp)
			{
			#
			# Loop through the number of resulting interpretations
			#
			for (j = 1; j <= max_splits; j++)
				{
				#
				# If the current interpretation is the target interpretation
				# replace it with the resulting interpretations.
				#
				if ($i == target_interp)
					current_record = current_record split_interp[j] "\t"
				#
				# Otherwise, simply repeat the given interpretation
				#
				else
					current_record = current_record $i "\t"
				}
			}
		#
		# Otherwise, simply print the given interpretation
		#
		else current_record = current_record $i "\t"
		}
	#
	# Print the record withouth the final tab character
	#
	print substr(current_record,1,length(current_record)-1)
	}

##################################################################
##			Function Print_locals
#	This function prints the correct number of local comments.
#
function print_locals(     current_record,i,j)
	{
	current_record = ""
	#
	# Loop through each active spine.
	#
	for (i = 1; i <= current_no_of_spines; i++)
		{
		#
		# If the current spine contains the target interpretation, print out
		# the extra local comments for each resulting interpretation
		#
		if (current_interp[i] == target_interp)
			{
			for (j = 1; j <= max_splits; j++)
				current_record = current_record $i "\t"
			}
		#
		# Otherwise, simply print the given local comment
		#
		else current_record = current_record $i "\t"
		}
	#
	# Print the record without the extra tab character.
	#
	print substr(current_record,1,length(current_record)-1)
	}

#################################################################
##			Function Print_data_records
#	This function prints the appropriate data tokens for the given
# record.
#
function print_data_records(    current_record,i,j,k,arraya,count\
							,double_stop,NULL,temp_token)
	{
	current_record = ""
	double_stop[0]
	arraya[0]
	#
	# Loop through each currently active spine
	#
	for (i = 1; i <= current_no_of_spines; i++)
		{
		#
		# If the current spine contains the target interpretation
		#
		if (current_interp[i] == target_interp)
			{
			#
			# Reset the array double_stop, place each double stop in the
			# current token into a separate array element in the
			# array double_stop.
			#
			for (k in double_stop) delete double_stop[k]
			count = split($i,double_stop," ")
			#
			# Loop through each possible resulting interpretation
			#
			for (j = 1; j <= max_splits; j++)
				{
				for (k in arraya) delete arraya[k]
				#
				# Loop through each double stop
				#
				for (k = 1; k <= count; k++)
					{
					#
					# If the current data token contains characters
					# matching the current regular expression, place those
					# characters in the array arraya.
					#
					if (match(double_stop[k],reg_exp[j]) > 0)
						{
						#
						# If the 's' option was specified, only output
						# the first occurence of the regular expression.
						#
						if (options ~ /s/)
							arraya[k]\
			 					= substr(double_stop[k],RSTART,RLENGTH)
						#
						# Otherwise, output all occurrences.
						#
						else
							{
							temp_token = double_stop[k]
							while(match(temp_token,reg_exp[j]) > 0)
								{
								arraya[k] = arraya[k] \
								   substr(temp_token,RSTART,RLENGTH)
								sub(reg_exp[j],"",temp_token)
								}
							}
						}
					#
					# Otherwise place a null token in the array arraya
					#
					else arraya[k] = "."
					}
				#
				# Determine if only null tokens exist in arraya
				#
				NULL = TRUE
				for (k in arraya)
					if (arraya[k] != ".") { NULL = FALSE ; break }
				#
				# If there are only null tokens, print a null token
				#
				if (NULL) current_record = current_record ".\t"
				#
				# Otherwise, print all non-null tokens
				#
				else
					{
					for (k = 1; k <= count; k++)
						{
						if (arraya[k] != ".")
							current_record = current_record \
									arraya[k] " "
						}
					current_record = \
					 substr(current_record,1,length(current_record)-1) "\t"
					}
				}
			}
		#
		# Otherwise simply print the given data token
		#
		else current_record = current_record $i "\t"
		}
	#
	# Print the record without the final tab character.
	#
	print substr(current_record,1,length(current_record)-1)
	#
	# If the new record is a comment record, print a warning
	# to standard error
	#
	if (current_record ~ /^!/)
		{
		print "rend: WARNING: Data record is now a "\
			 "comment record at \n               line " FNR \
			 ", file " FILENAME "." > stderr
		}
	#
	# If the new record is an interpretation record, print a warning
	# to standard error
	#
	else if (current_record ~ /^\*/)
		{
		print "rend: WARNING: Data record is now an "\
			 "interpretation record at \n               line " \
			 FNR ", file " FILENAME "." > stderr
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
		#	If the current field is a null interpretation
		#
		if ($i == "*") path_indicator[i] = "*"
		#
		#	If the current field is a spine-path indicator, store it
		#
		else path_indicator[i] = substr($i,2,1)
		}
	}

##################################################################
##				Function Print_indicators
#	This function scans the array of path indicators and prints
# the appropriate results.
#
function print_indicators(    i,j,split_spine,terminate,counter)
	{
	#
	# Store the field number of all spines that have a spine-path terminator
	# and are targeted to be split so that these fields can be modified later.
	#
	for (i = 1; i <= current_no_of_spines; i++)
		{
		if ((path_indicator[i] == "-" || path_indicator[i] == "*") \
		         && current_interp[i] == target_interp)
			terminate[++counter] = i
		}
	counter = 0
	i = 1
	#
	# Loop through each spine-path indicator
	#
	while (i <= current_no_of_spines)
		{
		split_spine = ""
		#
		# If a '+', '^', 'v', or 'x' path indicator has been used, in a spine
		# that is targeted to be split, print an error since these may not
		# be used with this program.
		#
		if (path_indicator[i] ~ /^\+$|^\^$|^v$|^x$/ \
		    && current_interp[i] == target_interp) 
			{
			print "rend: ERROR: Only spine path terminators are allowed"\
				 " in spines \n             targeted to be split." \
				 > stderr
			exit
			}
		#
		# If the current indicator is a split-path indicator
		#
		if (path_indicator[i] == "^")
			{
			#
			# Create a new position in the arrays and copy the
			# interpretations of the original spine to the new spine
			#
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = current_interp[i]
			}
		#
		#	If a spine-termination indicator occurred
		#
		if (path_indicator[i] == "-")
			{
			#
			# If the current spine contains the target interpretation
			#
			if (current_interp[i] == target_interp)
				{
				#
				# Print out the correct number of terminating indicators
				#
				for (j = 1; j <= max_splits; j++)
					split_spine = split_spine "*-\t"
				$terminate[++counter] \
					= substr(split_spine,1,length(split_spine)-1)
				}
			#
			# Delete the spine from the appropriate arrays
			#
			del_array_pos(i)
			i--
			}
		#
		#	If a null indicator occurred
		#
		if (path_indicator[i] == "*")
			{
			#
			# If the current spine contains the target interpretation
			#
			if (current_interp[i] == target_interp)
				{
				#
				# Print out the correct number of terminating indicators
				#
				for (j = 1; j <= max_splits; j++)
					split_spine = split_spine "*\t"
				$terminate[++counter] \
					= substr(split_spine,1,length(split_spine)-1)
				}
			}
		#
		# If the current indicator is an exchange-path indicator
		#
		if (path_indicator[i] == "x")
			{
			#
			# Determine the spines to exchange and exchange them (assumes
			# they are valid spines)
			#
			spines_to_exchange = ""
			exchanges = num_of_exchanges(i)
			exchange_spines()
			}
		#
		# If the current indicator is a join-path indicator
		#
		if (path_indicator[i] == "v")
			{
			#
			# If the spine to join is a spine targeted to be split, print
			# out an error.
			#
			if (path_indicator[i+1] == "v" \
			    && current_interp[i+1] == target_interp)
				{
				print "rend: ERROR: Only spine path terminators are al"\
					 "lowed in spines targeted to be split." > stderr
				exit
				}
			else
				{
				#
				# Join the two spines by deleting the second spine since
				# they must have the same exclusive interpretation.
				#
				del_array_pos(i+1)
				#
				# If there is another join-path indicator, be prepared to 
				# join it to the current spine next time through the loop
				#
				if (path_indicator[i+1] != "v") path_indicator[i] = "*"
				else i--
				}
			}
		#
		# If the current indicator is a new-path indicator
		#
		if (path_indicator[i] == "+")
			{
			#
			# Create the new corresponding array position.  The next record
			# should contain the new interpretation
			#
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = ""
			}
		i++
		}
	#
	# Print the given record
	#
	print $0
	}

############################################################
##			Function Ins_array_pos
#	This function inserts new positions in the arrays 'path_indicator'
# and 'current_interp' and copies elements so that everything is preserved
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j--)
		{
		path_indicator[j] = path_indicator[j-1]
		current_interp[j] = current_interp[j-1]	
		}
	path_indicator[current_element-1] = "*"
	current_no_of_spines++
	}

##############################################################
##			Function Del_array_pos
#	This function deletes the appropriate spine from the arrays
# path_indicator and current_interp and shifts all other material.
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

###############################################################
##				Function Num_of_exchanges
#	This function determines how  many exchange-path indicators
# appear in the current spine-path record and stores them in
# the variable 'spines_to_exchange'.
#
function num_of_exchanges(i,    j,exchange_count)
	{
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if (path_indicator[j] == "x")
			{
			#
			# If one of the spines to be exchanged is a targeted spine,
			# then print an error and exit
			#
			if (current_interp[i] == target_interp)
				{
				print "rend: ERROR: Only spine path terminators are al"\
					 "lowed in spines targeted to be split." > stderr
				exit
				}
			else
				{
 				exchange_count++
				spines_to_exchange = spines_to_exchange j "\t"
				path_indicator[j] = "*"
				}
			}
		}
	return exchange_count
	}	

###################################################################
##			Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in 'current_interp'
#
function exchange_spines(    arrayb,temp)
	{
	split(spines_to_exchange,arrayb)
	temp = current_interp[arrayb[1]]
	current_interp[arrayb[1]] = current_interp[arrayb[2]]
	current_interp[arrayb[2]] = temp
	}

################################################################
##				Function Store_new_interps
#	This function stores any new exlusive interpretations.
#
function store_new_interps(      j)
	{
	#
	#	Loop through each valid spine
	#	
	for (j = 1; j <= current_no_of_spines; j++)
		{
		#
		#	If the current field is an exclusive interpretation,
		#	then it replaces the old string of interpretations
		#
		if ($j ~ /^\*\*/) current_interp[j] = $j
		}
	}
