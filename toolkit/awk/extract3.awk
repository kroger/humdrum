#########################################################################
##                              EXTRACT3.AWK
# Programmed by: Tim Racinsky       Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#    Date:    Programmer:        Description:
# June 3 /95  David Huron      Ensure that single occurrences of exchange-path
#                              or join-path interps do not occur in the output.
#
#	This program is called from extract.ksh and implements the 'interpretation'
# mode of the extract program.  It receives as input a list of interpretations
# that will indicate which of the fields are to be cut from the given input.
#
# Main Functions used:
#	Parse_list() 			Determine_fields()		Print_fields()
#	Print_justified()		Print_non_justified()	Store_indicators()
#	Process_indicators() 	Ins_array_pos()		Del_array_pos()
#	Num_of_exchanges()		Exchange_spines()		Join_check()
#	Join_paths() 			Store_new_interps()
#
# Helper Functions used:
#	Print_interps() 		Print_cut_interps()		Print_fields_to_cut()
#	Print_new_fields()
#
#	VARIABLES:
#
#  -first_interpretation_yet:  flag indicates if the first interpretation
#   record has been found yet.
#  -current_no_of_spines:  holds current number of active spines
#  -spine_path_record:  flag indicates if current interpretation record is also
#   a spine-path record.
#
#	ARRAYS:
#
#  -cut_interps:  each subscript of this array is an interpretation that the
#   user has specified to cut from the input file.
#  -fields_to_cut:  each element represents a currently active spine and
#   contains either a 'C' or an 'N' representing 'cut' and
#   'no-cut' respectively.
#  -path_indicator:  containes spine-path indicators for each spine in current
#   spine-path record.
#  -current_interp:  contains interpretations for each spine.
#  -new_fields: if a new spine is to be extracted, it is indicated in this array
#
BEGIN {
	#
	# Delete the DUMMY variable sent from the shell script
	#
	ARGV[1] = ""
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
	# Parse the list of interpretations provided from the user
	#
	parse_list(ARGV[2])
	ARGV[2] = ""
	}
{
#
# If this is the beginning of a file, reset variables and arrays
#
if (FNR == 1)
	{
	first_interpretation_yet = FALSE
	current_no_of_spines = 0
	for (i in fields_to_cut) delete fields_to_cut[i]
	for (i in current_interp) delete current_interp[i]
	for (i in new_fields) delete new_fields[i]
	}
#
# Just print all global comments
#
if ($0 ~ /^!!/) print $0
#
# If the current record is an interpretation record
#
if ($0 ~ /^\*/)
	{
	#
	# If this is the first interpretation record (exclusive only) store the
	# new interpretations and determine which fields are to be cut and
	# then print those fields
	#
	if (!first_interpretation_yet)
		{
		current_no_of_spines = NF
		first_interpretation_yet = TRUE
		store_new_interps()
		determine_fields()
		print_fields()
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
		# If it is a spine path record, print the fields to be cut, and then
		# store and process the indicators
		#
		if (spine_path_record)
			{
			print_fields()
			store_indicators()
			process_indicators()
			#
			# If all paths terminate, reset the flags.
			#
			if (current_no_of_spines == 0)
				{
				first_interpretation_yet = FALSE
				for (i in new_field) delete new_fields[i]
				}
			}
		#
		# Otherwise store the new interps and determine which fields
		# are to be cut and then extract those fields
		#
		else 
			{
			store_new_interps()
			determine_fields()
			print_fields()
			}
		}
	}
#
# For any other records, print the appropriate fields
#
else if ($0 !~ /^!!/) print_fields()
}

######################################################################
#				FUNCTIONS USED IN EXTRACT3
######################################################################

######################################################################
##				Function Parse_list
#	This function takes as its argument the list of interpretations
# that the user has entered.  It first checks that each has valid 
# interpretation syntax and if so, creates an array subscript in 
# cut_interps for each of the interpretations
#
function parse_list(list,     j,arraya)
	{
	#
	# The interpretations should be separated by commas
	#
	gsub(/XTIMX/," ",list)
	split(list, arraya, ",")
	#
	# For each interpretation in the list
	#
	for (j in arraya)
		{
		#
		# If the current interpretation is not specified as exclusive
		# or tandem or if it contains a space character, print an error
		#
		if (arraya[j] !~ /^(\*|\*\*)/)
			{
			print "extract: ERROR: Invalid interpretation specified in "\
				 "option list (" arraya[j] ")." > stderr
			exit
			}
		#
		# Otherwise create the appropriate array subscript
		#
		else cut_interps[arraya[j]] = ""
		}
	}

#####################################################################
##				Function Determine_fields
#	This function determines which fields (spines) are to be extracted.
# If a new spine is to be extracted, the array new_fields is updated to
# reflect this.  The array fields_to_cut indicates which fields will be
# extracted.
#
function determine_fields(    i,j,k,arrayb,match_found)
	{
	for (i = 1; i <= current_no_of_spines; i++) new_fields[i] = FALSE
	#
	# Loop through each currently active spine
	#
	for (i = 1; i <= current_no_of_spines; i++)
		{
		match_found = FALSE
		#
		# Split apart the list of interpretations for the current spine.
		#
		split(current_interp[i],arrayb)
		#
		# Loop through each subsript (interpretation) in cut_interps
		#
		for (j in cut_interps)
			{
			#
			# Loop through each interpretation for the current spine
			#
			for (k in arrayb)
				{
				#
				# If the current interpretation for the current spine
				# matches the current subsrcript in cut_interps
				#
				if (arrayb[k] == j) 
					{
					#
					# If this spine has not been extracted before
					# indicate this and then break out of the loops
					#
					if (fields_to_cut[i] != "C")
						new_fields[i] = TRUE
					fields_to_cut[i] = "C"
					match_found = TRUE
					break
					}
				} #End for loop (k)
			#
			# If a match was found then go the the next subsript
			#
			if (match_found) break
			} #End for loop (j)
		#
		# If a match was not found, place an "N" for 'no_cut'
		#
		if (!match_found) fields_to_cut[i] = "N"
		} #End for loop (i)
	}

#############################################################
##				Function Print_fields
#	This function  determines how to print the given input line
# depending on it's type.
#
function print_fields(     j,current_line,exclusive,new_ones)
	{
	#
	# If the current record is an interpretation record and not a 
	# spine path record
	#
	if ($0 ~ /^\*/ && !spine_path_record)
		{
		exclusive = FALSE
		new_ones = FALSE
		#
		# Check to see if an exclusive interpretation will be extracted
		#
		for (j = 1; j <= current_no_of_spines; j++)
			{
			if ($j ~ /^\*\*/ && fields_to_cut[j] == "C")
				{
				exclusive = TRUE
				break
				}
			}
		#
		# If an exclusive interpretation is to be cut, simply print
		# the line
		#
		if (exclusive)
			print_non_justified()
		else
			{
			#
			# Determine if the current line has any new spines to extract
			#
			for (j = 1; j <= current_no_of_spines; j++)
				{
				if (new_fields[j] == TRUE)
					{
					new_ones = TRUE
					break
					}
				}
			#
			# If there are new spines, printing is more difficult
			#
			if (new_ones) print_justified()
			#
			# Otherwise, simply print the line
			#
			else print_non_justified()
			}
		}
	#
	# For all other records, simply print the line
	#
	else
		{
		print_non_justified()
		}
	}	

#############################################################
##			Function Print_non_justified
#	This function simply prints the fields specified by the
# array fields_to_cut
#
function print_non_justified(     j,current_line)
	{
	for (j = 1; j <= current_no_of_spines; j++)
		{
		if (fields_to_cut[j] == "C") current_line = current_line $j "\t"
		}
	if (current_line != "")
		{
		# Ensure that single occurrences of exchange-path or
		# join-path interpretations do not occur in the output.
		#
		if ($0 ~ /^\*/ && spine_path_record)
			{
			if (current_line ~/\*x/)
				{
				if (current_line !~/\*x.*\*x/)
				gsub("x","",current_line)
				}
			if (current_line ~/\*v/)
				{
				if (current_line !~/\*v.*\*v/)
				gsub("v","",current_line)
				}
			}
		print substr(current_line,1,length(current_line)-1)
		}
	#
	# If the input record was a local comment record and the 
	# extracted record is a global comment record, print a 
	# warning to standard error
	#
	if ($0 ~ /^![^!]/ && current_line ~ /^!!/)
		{
		print "extract: WARNING: Local comment record extracted as"\
			 " global comment at \n                  line " FNR \
			 "." > stderr
		}
	#
	# If the input record was a data record
	#
	else if ($0 !~ /^[!\*]/)
		{
		#
		# If the extracted record is a comment record, print a 
		# warning to standard error
		#
		if (current_line ~ /^!/)
			{
			print "extract: WARNING: Data record has been extrac"\
				 "ted as comment record at \n                  "\
				 "line " FNR "." > stderr
			}
		#
		# If the extracted record is an interpretation record, 
		# print a warning to standard error
		#
		else if (current_line ~ /^\*/)
			{
			print "extract: WARNING: Data record has been extract"\
				 "ed as interpretation record at \n             "\
				 "     line " FNR "." > stderr
			}
		}
	}

#############################################################
##			Function Print_justified
#	This function is called when there are new spines to extract and
# they are started because of a tandem interpretation.  Special care
# must be taken to try to produce 'readable' output.  However, the
# output may still be in a non-humdrum format.
#
function print_justified(     j,interp_number,current_line,arrayc,null)
	{
	interp_number = 1
	null = FALSE
	#
	# Loop until there is no more information that needs to be printed
	#
	while (TRUE)
		{
		current_line = ""
		#
		# For the first line, print out the stored exclusive interpretations
		# for the new spines that are being extracted
		#
		if (interp_number == 1)
			{
			for (j = 1; j <= current_no_of_spines; j++)
				{
				if (fields_to_cut[j] == "C")
					{
					if (new_fields[j] == TRUE)
						{
						split(current_interp[j],arrayc)
						current_line = current_line arrayc[1] "\t"
						}
					else
						{
						current_line = current_line "*\t"
						}
					}
				}
			if (current_line != "")
				{
				print substr(current_line,1,length(current_line)-1)
				}
			}
		#
		# For the second line, print out the tandem interpretations of
		# any of the previous fields to be cut along with any other
		# tandem interpretations of the new spines.
		#
		else if (interp_number == 2)
			{
			for (j = 1; j <= current_no_of_spines; j++)
				{
				if (fields_to_cut[j] == "C")
					{
					if (new_fields[j] == TRUE)
						{
						split(current_interp[j],arrayc)
						if ("2" in arrayc)
							{
							current_line = current_line \
										arrayc[2] "\t"
							}
						else
							{
							current_line = current_line "*\t"
							}
						}
					else
						{
						current_line = current_line $j "\t"
						}
					}
				}
			if (current_line != "")
				{
				print substr(current_line,1,length(current_line)-1)
				}
			}
		#
		# For the rest of the times through the loop, print out any 
		# remaining tandem interpretations in the new spines to extract
		#
		else
			{
			null = TRUE
			for (j = 1; j <= current_no_of_spines; j++)
				{
				if (fields_to_cut[j] == "C")
					{
					if (new_fields[j] == TRUE)
						{
						split(current_interp[j],arrayc)
						if (arrayc[interp_number] != "")
							{
							current_line = current_line \
										arrayc[interp_number] "\t"
							null = FALSE
							}
						else
							{
							current_line = current_line "*\t"
							}
						}
					else
						{
						current_line = current_line "*\t"
						}
					}
				}
			if (current_line != "" && !null)
				{
				print substr(current_line,1,length(current_line)-1)
				}
			}
		#
		# When there are no more tandem interpretations to print out, exit
		#
		if (null) break
		interp_number += 1
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
	for (i=1; i<=current_no_of_spines; i++)
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
# and manipulates the arrays 'path_indicator' and 'current_interp'
# according to the contents of the array 'path_indicator'.
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
			fields_to_cut[i+1] = fields_to_cut[i]
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
			fields_to_cut[i+1] = "N"
			}
		i++
		} #end while loop
	} #end function process_indicators

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
		fields_to_cut[j] = fields_to_cut[j-1]
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
		fields_to_cut[j] = fields_to_cut[j+1]
		}
	delete path_indicator[j]
	delete current_interp[j]
	delete fields_to_cut[j]
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
# elements in 'current_interp'
#
function exchange_spines(    arrayd,temp)
	{
	split(spines_to_exchange,arrayd)
	temp = current_interp[arrayd[1]]
	current_interp[arrayd[1]] = current_interp[arrayd[2]]
	current_interp[arrayd[2]] = temp
	temp = fields_to_cut[arrayd[1]]
	fields_to_cut[arrayd[1]] = fields_to_cut[arrayd[2]]
	fields_to_cut[arrayd[2]] = temp
	}

############################################################
##				Function Join_paths
#	This function actually joins two spines by adding interpretations
# from the second spine to those in the first spine and removing all
# redundant interpretations.
#
function join_paths(first_spine,second_spine,     i,j,first_array,second_array)
	{
	split(current_interp[first_spine],first_array)
	split(current_interp[second_spine],second_array)
	for (j in first_array)
		{
		for(i in second_array)
			{
			if (first_array[j] == second_array[i]) 
				{
				delete second_array[i]
				break
				}
			}
		}
	for (i in second_array) 
		{
		current_interp[first_spine] = current_interp[first_spine] \
								second_array[i] "\t"
		}
	if (fields_to_cut[first_spine] == "C" \
	    || fields_to_cut[second_spine] == "C")
		{
		fields_to_cut[first_spine] = "C"
		}
	}

################################################################
##				Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
#
function store_new_interps(      j)
	{
	#
	#	Loop through each valid spine
	#	
	for (j=1; j <= current_no_of_spines; j++)
		{
		#
		#	If the current field is an exclusive interpretation,
		#	then it replaces the old string of interpretations
		#
		if ($j ~ /^\*\*/) current_interp[j] = $j "\t" 
		#
		#	If it is not an exclusive interpretation
		#	If it is a null interpretation, do nothing
		#
		else if ($j ~ /^\*$/) ;
		#
		#	Otherwise, add it if it does not already exist
		#
		else if (index(current_interp[j],"\t" $j "\t") == 0)
			current_interp[j] = current_interp[j] $j "\t"
		}
	}

#########################################################################
##				Helper Function Print_interps
#
function print_interps(        j)
	{
	print "Array current_interp"
	for(j in current_interp)
		print j, current_interp[j] 
	print "End Array current_interp"
	}
####################################################################
##			Helper Function Print_cut_interps
#
function print_cut_interps(   j)
	{
	print "Array cut_interps"
	for (j in cut_interps) print j
	print "End Array cut_interps"
	}
#####################################################################
##			Helper Function Print_fields_to_cut
#
function print_fields_to_cut(   j)
	{
	print "Array fields_to_cut"
	for (j in fields_to_cut) print j, fields_to_cut[j]
	print "End Array fields_to_cut"
	}
#####################################################################
##			Helper Function Print_new_fields
#
function print_new_fields(   j)
	{
	print "Array new_fields"
	for (j in new_fields) print j, new_fields[j]
	print "End Array new_fields"
	}
