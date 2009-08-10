###########################################################################
#                          VOX
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer: 	     Description:
# May 9/94	Tim Racinsky		Fixed double stop problem when counting
#							voices -- This should not have been a problem
#							but was changed anyway.
#
#
# 	This program calculates the number of voices in a humdrum file
# which are active at successive moments in time.  For each valid
# data record, vox outputs a single integer indicating
# how many voices are concurrently sounding.  The program is able
# to correctly handle double-stops, and triple-stops etc.  The program
# deals with any 'valid' type of pitch unit.
#
#	The following gives a brief summary of the available options:
#
# Main Functions used:
#	Initialize_last_value()	Store_indicators()	Process_indicators()
#	Ins_array_pos()		Del_array_pos()	Num_of_Exchanges()
#	Exchange_spines()		Join_paths()		Store_new_interps()
#	Process_data()
#
# Helper Functions used:
#	Print_interps()
#
#	VARIABLES:
#
#  -first_interpretation_yet:  flag to indicate if first exlusive interpretation
#   record has been encountered yet.
#  -spine_path_record:  flag to indicate if current interpretation record is
#   a spine-path record or not.
#  -current_no_of_spines:  holds current number of active spines.
#  -pitch_units: a variable to help match pitch unit interpretations
#  -last_record: a variable to hold the last record processed.
#
#	ARRAYS:
#
#  -last_value:  each element holds the number of currently sounding voices
#   of the previous token for each corresponding spine.
#  -current_interp:  holds exclusive interpretations for each spine.
#  -path_indicator:  holds spine-path indicators for each spine.
#
BEGIN {
	#
	# Delete the DUMMY variable sent from the shell script
	#
	ARGV[1] = ""
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	pitch_units = "\\*\\*kern|\\*\\*semits|\\*\\*pitch|\\*\\*Tonh|\\*\\*solfg"\
			    "|\\*\\*cents|\\*\\*freq|\\*\\*barks|\\*\\*cbr|\\*\\*cocho"\
			    "|\\*\\*deg|\\*\\*degree|\\*\\*pc|\\*\\*solfa|\\*\\*specC"
	last_record = ""
	}
{
#
# If the last record was an interpretation record, print it now since it
# wasn't printed previously so that the end of file can be printed properly
#
if (last_record ~ /^\*/) print "*"
#
# If this is the first record, reset variables
#
if (FNR == 1)
	{
	first_interpretation_yet = spine_path_record = FALSE
	current_no_of_spines = 0
	last_record = ""
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
	# If this is the first interpretation record print the new interpretation
	# title and set all of the necessary variables.
	#
	if (!first_interpretation_yet)
		{
		print "**vox#"	
		current_no_of_spines = NF
		first_interpretation_yet = TRUE
		store_new_interps()
		initialize_last_value()
		#
		# Skip to the start of the loop so that last_record is not updated
		#
		last_record = ""
		next
		}
	#
	# If this is not the first interpretation record
	#
	else
		{ 
		#
		# Check if it is a spine path record or not
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
		# If it is a spine path record, store and process the indicators and
		# reset the flag if necessary.
		#
		if (spine_path_record)
			{
			store_indicators()
			process_indicators()
			#
			# If all paths terminate, reset variables
			#
			if (current_no_of_spines == 0)
				{
				first_interpretation_yet = FALSE
				initialize_last_value()
				}
			}
		#
		# Otherwise store the new interps.
		#
		else store_new_interps()
		}
	}
#
# For local comments, always print nothing
#
else if ($0 ~ /^![^!]/) print "!"
#
# For any other records, print the appropriate fields
#
else process_data()
last_record = $0
}
#
# Print the spine-path terminator at the end of the file.
#
END	{
	if (last_record ~ /^!!/) exit
	else if (last_record == "") exit
	else print "*-"
	}

######################################################################
#			FUNCTIONS USED IN VOX.AWK
######################################################################

#################################################################
##			Function Initialize_last_value
#	This function simply resets the array last_value
#
function initialize_last_value(  j)
	{
	for (j = 1; j <= current_no_of_spines; j++) last_value[i] = 0
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
##				Function Process_indicators
#	This function takes the spine-path indicators that were stored
# in the array 'path_indicator' in the function 'store_indicators'
# and manipulates the arrays 'path_indicator', 'current_interp', and
# 'last_value' according to the contents of the array 'path_indicator'.
#
function process_indicators(    i,exchanges,join)
	{
	#
	#	Loop through the entire array of 'path_indicator'
	#	NOTE: This array may be altered during the process
	#
	i = 1
	while (i <= current_no_of_spines)
		{
		#
		#	If a spine-split indicator occurred
		#
		if (path_indicator[i] == "^")
			{
			#
			#	Insert a new array position in 'path_indicator'
			# 	and 'current_interp'
			#
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = current_interp[i]
			last_value[i+1] = last_value[i]
			}
		#
		#	If a spine-termination indicator occurred
		#
		if (path_indicator[i] == "-")
			{
			#
			#	Delete that spine from the arrays
			#
			del_array_pos(i)
			i--
			}
		#
		#	If a spine-exchange indicator occurred
		#
		if (path_indicator[i] == "x")
			{
			#
			#	Determine the number of exchange indicators that
			#	occur and store them in 'spines_to_exchange'
			#
			spines_to_exchange = ""
			exchanges = num_of_exchanges()
			exchange_spines()
			#	
			}
		#
		#	If a join-path indicator occurred
		#
		if (path_indicator[i] == "v")
			{
			#
			# Join the paths and delete the joined spine
			#
			join_paths(i,i+1)
			del_array_pos(i+1)
			#
			#	If the next indicator is not a join-path indicator
			#	then return to original conditions
			#
			if (path_indicator[i+1] != "v")
				path_indicator[i] = "*"
			#
			#	Otherwise, look at current and next paths to join
			#
			else i--
			}
		#
		#	If a new-path indicator occurred
		#
		if (path_indicator[i] == "+")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = ""
			last_value[i+1] = 0
			}
		i++
		} #end while loop
	} #end function process_indicators

############################################################
##			Function Ins_array_pos
#	This function inserts new positions in the arrays 'path_indicator',
# 'current_interp', and 'last_value' and copies elements so that
# everything is preserved
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j--)
		{
		path_indicator[j] = path_indicator[j-1]
		current_interp[j] = current_interp[j-1]	
		last_value[j] = last_value[j-1]
		}
	path_indicator[current_element-1] = "*"
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
		last_value[j] = last_value[j+1]
		}
	delete path_indicator[j]
	delete current_interp[j]
	delete last_value[j]
	current_no_of_spines--
	} 

###############################################################
##			Function Num_of_exchanges
#	This function determines how  many exchange-path indicators
# appear in the current spine-path record and stores them in
# the variable 'spines_to_exchange'.
#
function num_of_exchanges(    j,exchange_count)
	{
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if (path_indicator[j] == "x")
			{
 			exchange_count++
			spines_to_exchange = spines_to_exchange j "\t"
			path_indicator[j] = "*"
			}
		}
	return exchange_count
	}	

###################################################################
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in 'current_interp' and 'last_value'.
#
function exchange_spines(    arraya,temp)
	{
	split(spines_to_exchange,arraya)
	temp = current_interp[arraya[1]]
	current_interp[arraya[1]] = current_interp[arraya[2]]
	current_interp[arraya[2]] = temp
	temp = last_value[arraya[1]]
	last_value[arraya[1]] = last_value[arraya[2]]
	last_value[arraya[2]] = temp
	}

############################################################
##				Function Join_paths
#	This function uses the last value of the first spine of the two
# spines to be joined unless it is null.
#
function join_paths(first_spine,second_spine)
	{
	last_value[first_spine] += last_value[second_spine]
	}

################################################################
##				Function Store_new_interps
#	This function stores only exclusive interpretations for each
# spine and resets the corresponding array element in last_value
# if necessary.
#
function store_new_interps(    j)
	{
	#
	#	Loop through every currently active spine.
	#	
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if ($j ~ /^\*\*/)
			{
			current_interp[j] = $j
			last_value[j] = 0
			}
		}
	}

#############################################################
##				Function Process_data
#	This function determines the number of the concurrently sounding voices
# for the current data record based on its contents and the contents 
# of the array last_value.
#
function process_data(     i,j,current_number,double_array,double_count,barline\
				  ,first_token,different)
	{
	#
	# If the current record is a barline print 1 if they are equal or the 
	# entire line if they differ.
	#
	barline = FALSE
	different = FALSE
	for (i = 1; i <= NF; i++)
		{
		if (current_interp[i] ~ pitch_units)
			{
			if ($i ~ /^=/)
				{
				if (!barline)
					{
					barline = TRUE
					first_token = $i
					}
				else if ($i != first_token)
					{
					gsub(/\t/," ")
					print $0
					different = TRUE
					break
					}
				}
			}
		}
	#
	# Go to the next record if a barline was found
	#
	if (barline)
		{
		if (!different) print first_token
		return
		}
	current_number = 0
	#
	# Loop through each currently active spine
	#
	for (i = 1; i <= current_no_of_spines; i++)
		{
		#
		# If the current spine contains a valid pitch unit representation
		# as an exclusive interpretation
		#
		if (current_interp[i] ~ pitch_units)
			{
			#
			# If the current token is a null token
			#
			if ($i == ".")
				{
				#
				# If the last value was greater than 0, add it
				#
				if (last_value[i] > 0) current_number += last_value[i]
				}
			#
			# For all other notes, store in last_value and add to the
			# number of currently sounding voices the number of double
			# stops that appear in the current token.
			#
			else
				{
				double_count = 0
				#
				# You should be able to loop through the array with a simple
				# 'for j in array' but this does not work here for some
				# strange reason!
				#
				for (j = 1; j <= split($i,double_array," "); j += 1)
					{
					if (double_array[j] !~ /r/ && double_array[j] != ".")
						double_count += 1
					}
				last_value[i] = double_count
				current_number += double_count
				}
			}
		}
	#
	# Print the number of concurrently sounding voices for the current record
	#
	print current_number
	}	

#########################################################################
##			Helper Function Print_interps
#
function print_interps(        j)
	{
	print "Array current_interp"
	for(j in current_interp)
		print j, current_interp[j] 
	print "End Array current_interp"
	}
