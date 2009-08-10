#####################################################################
##					STROPHE.AWK
# Programmed by: Tim Racinsky         Date: July, 1994
# Copyright (c) 1994 David Huron
#
# Modifications:
#    Date:   Programmer:         	Description:
#
# This program processes strophe defined sections of a file
#
# Main Functions used
#	parse_command()		process_comments()			process_exclusive()
#	process_spine_path()	process_tandem()			process_other()
#	process_strophe_record()	process_other_option()		process_xoption()
#	process_strophe_end()	process_strophe_terminate()	store_indicators()
#	process_indicators()	ins_array_pos()			del_array_pos()
#	exchange_spines()		join_paths()				reset()
#
#	VARIABLES:
#
#  -stderr:  variable holding destination of error messages
#  -null_interp:  regular expression finding null interpretation records
#  -exclusive_record:  regular expression for exclusive records
#  -spine_path_record:  regular expression for spine-path records
#  -strophe_record:  regular expression for records containing *strophe
#  -strophe_end:  regular expression for records containing *S/fin
#  -strophe_terminate:  regular expression for records containing *S-
#  -strophe_interp:  regular expression for *S/
#  -no_interpretation_yet:  flag to indicate if first interpretation found yet
#  -in_strophe:  flag to indicate if currently in strophic section
#  -split_strophe:  flag to indicate if currently splitting strophe spines
#  -join_strophe:  flag to indicate if currently joining strophe spines
#  -next_main:  holds next main number for strophe label to find
#  -next_part:  holds next sub part of number for strophe label to find
#  -s_counter:  holds number of labels specified with -s option
#  -s_count:  holds number of labels used so far with -s option
#  -options:  holds options specified by the user
#  -extract:  holds label to extract with -x option
#
#	ARRAYS:
#
#  -strophe_list:  holds labels to extract, in order, with -s option
#  -strophe_spines:  Determines which of the currently active spines are 
# 				 considered 'strophe' spines
#  -spines_to_cut:  Determines which of the currently active spines are to
#				be extracted as strophic material
#
BEGIN {
	USAGE="\nUSAGE: strophe -h               (Help Screen)\n       strophe "\
		 "[-s strophe_list | -x strophe_label] [file ...]"
	TRUE = 1; FALSE = 0
	FS = OFS = "\t"
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	# Set global regular expressions
	#
	null_interp = "^\\*(\t\\*)*$"
	exclusive_record = "^\\*(\\*([^\t])+)?(\t\\*(\\*([^\t])+)?)*$"
	spine_path_record = "^\\*(\\+|v|\\^|x|-)?(\t\\*(\\+|v|\\^|x|-)?)*$"
	strophe_record = "(^|\t)\\*strophe(\t|$)"
	strophe_end = "(^|\t)\\*S\\/fin(\t|$)"
	strophe_terminate = "(^|\t)\\*S-(\t|$)"
	strophe_interp = "^\\*S\\/"
	#
	# Set other global variables
	#
	no_interpretation_yet = TRUE
	in_strophe = FALSE
	split_strophe = FALSE
	join_strophe = FALSE
	next_main = "1"
	next_part = 0
	s_counter = 1
	s_count = 0
	options = ""
	extract = ""
	#
	# Determine the options specified by the user
	#
	parse_command()
	if (options ~ /s/) next_main = strophe_list[1]
	}
{
#
# Process each record according to its type
#
if ($0 ~ /^!/) process_comments()
else if ($0 ~ exclusive_record && $0 !~ null_interp) process_exclusive()
else if ($0 ~ spine_path_record && $0 !~ null_interp) process_spine_path()
else if ($0 ~ /^\*/) process_tandem()
else process_other()
}

############################################################################
#				FUNCTIONS USED IN STROPHE.AWK
############################################################################

############################################################################
##				Function parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command(    i)
	{
	#
	# Otherwise, process any necessary variables
	#
	if (ARGV[2] != "null")
		{
		#
		# Read in the strophe labels to process as specified by the
		# -s option on the command line
		#
		s_count = split(ARGV[2],strophe_list,",")
		for (i in strophe_list)
			{
			if (strophe_list[i] !~ /^[1-9][0-9]*$/)
				{
				print "strophe: ERROR: Invalid strophe label specified: "\
					 strophe_list[i] > stderr
				exit
				}
			}
		options = "s"
		}
	#
	# Store the label specified by -x option
	#
	else if (ARGV[3] != "null")
		{
		extract = ARGV[3]
		options = "x"
		}
	ARGV[1] = ARGV[2] = ARGV[3] = ""
	}

##############################################################################
##					Function process_comments
#	This function processes all global and local comments
#
function process_comments(   output)
	{
	output = ""
	#
	# Always print global comments
	#
	if ($0 ~ /^!!/) print $0
	else
		{
		#
		# Local comments are not allowed when splitting or joining strophe
		# spines (when setting up those spines)
		#
		if (split_strophe || join_strophe)
			{
			print "strophe: ERROR: Invalid record while splitting or"\
				 " joining\n                strophe spines at "\
				 "line " NR > stderr
			exit
			}
		#
		# If in a strophic passage, output the correct spines
		#
		else if (in_strophe)
			{
			for (i = 1; i <= NF; i += 1)
				{
				if (!strophe_spines[i] || spines_to_cut[i])
					output = output $i "\t"
				}
			print substr(output,1,length(output)-1)
			}
		#
		# Otherwise, simply print the record
		#
		else print $0
		}
	}

##############################################################################
##					Function process_exclusive
#	This function processes all exclusive interpretation records
#
function process_exclusive()
	{
	#
	# If this is the first interpretation record, store the variables
	#
	if (no_interpretation_yet)
		{
		no_interpretationt_yet = FALSE
		current_no_of_spines = NF
		}
	#
	# Process the record the same as any data record
	#
	process_other()
	}

##############################################################################
##					Function process_tandem
#	This function processes all tandem interpretation records
#
function process_tandem()
	{
	#
	# If this is a record that contains a *strophe interpretation
	#
	if ($0 ~ strophe_record)
		{
		#
		# If already in strophe, print an error
		#
		if (in_strophe)
			{
			print "strophe: ERROR: Nested strophic passages found at "\
				 "line " NR > stderr
			exit
			}
		#
		# Otherwise, process that record
		#
		else process_strophe_record()
		}
	#
	# If this is a record that contains a *S/fin interpretation
	#
	else if ($0 ~ strophe_end)
		{
		#
		# If not in a strophic passage, print an error
		#
		if (!in_strophe)
			{
			print "strophe: ERROR: Must initiate strophic passage with "\
				 "*strophe\n                interpretation at "\
				 "line " NR > stderr
			exit
			}
		#
		# If currently splitting strophe spines, print an error
		#
		else if (split_strophe)
			{
			print "strophe: ERROR: Strophe end indicator occurs before "\
				 "strophe label at line " NR > stderr
			exit
			}
		#
		# If currently joining strophe spines, print an error
		#
		else if (join_strophe)
			{
			print "strophe: ERROR: Repeated strophe end indicator at "\
				 "line " NR > stderr
			exit
			}
		#
		# Otherwise, process the record
		#
		else process_strophe_end()
		}
	#
	# If the current record contains a *S- interpretation
	#
	else if ($0 ~ strophe_terminate)
		{
		#
		# If not in the midst of joining spines, print an error
		#
		if (!join_strophe)
			{
			print "strophe: ERROR: Strophic passage terminator does not "\
				 "occur at correct point in\n                strophic "\
				 "passage at line " NR > stderr
			exit
			}
		#
		# Otherwise, process the record
		#
		else process_strophe_terminate()
		}
	#
	# If it is a strophe label
	#
	else if ($0 ~ /(^|\t)\*S\//)
		{
		#
		# If not currently splitting strophe spines, print an error
		#
		if (!split_strophe)
			{
			print "strophe: ERROR: Strophe labels must occur after strophic"\
				 " passage\n                initiator at line " NR > stderr
			exit
			}
		#
		# Otherwise, process according to the options
		#
		else if (options ~ /x/) process_xoption()
		else process_other_option()
		}
	#
	# Otherwise, process the tandem interpretations like a data record
	#
	else process_other()
	}

#############################################################################
##					Function process_strophe_record
#
function process_strophe_record(   i,found,good_record)
	{
	good_record = TRUE
	found = FALSE
	#
	# Loop through each of the currently active spines
	#
	for (i = 1; i <= NF; i += 1)
		{
		#
		# If a strophe interpretation is found, note it
		#
		if ($i == "*strophe")
			{
			#
			# If one was already found, this is incorrect since only one
			# such interpretation is allowed at any one time
			#
			if (found)
				{
				good_record = FALSE
				break
				}
			else
				{
				$i = "*"
				found = TRUE
				strophe_spines[i] = TRUE
				}
			}
		else strophe_spines[i] = FALSE
		}
	if (!good_record)
		{
		print "strophe: ERROR: Invalid strophic passage initiator found at " \
			 "line " NR > stderr
		exit
		}
	#
	# Set the flags to show where we are in processing the strophic material
	#
	in_strophe = TRUE
	split_strophe = TRUE
	}

#############################################################################
##					Function process_other_option
#
function process_other_option(   i,main,part,position)
	{
	#
	# Loop through each of the currently active spines
	#
	for (i = 1; i <= NF; i += 1)
		{
		#
		# If this spines is a strophic spines
		#
		if (strophe_spines[i])
			{
			#
			# It must contain a strophe label
			#
			if ($i !~ strophe_interp)
				{
				print "strophe: ERROR: Each 'strophe' spine must contain a"\
					 " strophe number at line " NR > stderr
				exit
				}
			#
			# Extract the label into the corresponding array position in
			# the arrays main and part
			#
			else
				{
				match($i,"[1-9][0-9]*")
				main[i] = substr($i,RSTART,RLENGTH) + 0
				if (match($i,"\\.[0-9]*[1-9]") > 0)
					part[i] = substr($i,RSTART + 1,RLENGTH - 1) + 0
				else
					part[i] = 0
				$i = "*"
				}
			}
		#
		# If it's not a strophe spine, then there should not be a label
		#
		else if ($i != "*")
			{
			print "strophe: ERROR: Non-strophe spines must contain a null "\
				 "interpretation at line " NR > stderr
			exit
			}
		else main[i] = 0
		}
	#
	# Search the labels for the strophe spine to extract
	#
	position = 0
	for (i = 1; i <= NF; i += 1)
		{
		if (main[i] == next_main && part[i] == next_part)
			{
			position = i
			break
			}
		}
	#
	# If the label was not found
	#
	if (position == 0)
		{
		#
		# If we were searching for the first label (eg. 2), then print an 
		# error since it must be present
		#
		if (next_part == 0)
			{
			print "strophe: ERROR: Next strophe label in list not found at "\
				 "line " NR > stderr
			exit
			}
		#
		# If the -s option was specified, get the next label from the array
		# strophe_list if another one exists
		#
		else if (options ~ /s/)
			{
			s_counter += 1
			if (s_counter > s_count)
				{
				print "strophe: ERROR: No more strophe labels to "\
					 "output at line " NR > stderr
				exit
				}
			next_main = strophe_list[s_counter] + 0
			next_part = 0
			}
		#
		# Otherwise, increment the main number by 1
		#
		else
			{
			next_main += 1
			next_part = 0
			}
		#
		# Again, search for the label in the strophe spines
		#
		for (i = 1; i <= NF; i += 1)
			{
			if (main[i] == next_main && part[i] == next_part)
				{
				position = i
				break
				}
			}
		}
	#
	# If one was still not found, print an error
	#
	if (position == 0)
		{
		print "strophe: ERROR: Next strophe label in list not found at " \
			 "line " NR > stderr
		exit
		}
	#
	# Loop through the spines, designating which spines are to be cut
	#
	for (i = 1; i <= NF; i += 1)
		{
		if (i != position) spines_to_cut[i] = FALSE
		else spines_to_cut[i] = TRUE
		}
	#
	# Update the flag to indicate were are inside the strophic passage and
	# increment next_part for the next strophic section to be processed
	#
	split_strophe = FALSE
	next_part += 1
	}

#############################################################################
##					Function process_xoption
#
function process_xoption(   i,label)
	{
	#
	# Loop throught the currently active spines
	#
	for (i = 1; i <= NF; i += 1)
		{
		#
		# If the current spine is a strophe spines
		#
		if (strophe_spines[i])
			{
			#
			# It must contain a strophe label
			#
			if ($i !~ strophe_interp)
				{
				print "strophe: ERROR: Each 'strophe' spine must contain a"\
					 " strophe number at line " NR > stderr
				exit
				}
			#
			# Extract the strophe label and store in the corresponding array
			# position in the array label
			#
			else
				{
				label[i] = substr($i,4,length($i)-3)
				$i = "*"
				}
			}
		#
		# Non-strophe spines should contain null interpretations
		#
		else if ($i != "*")
			{
			print "strophe: ERROR: Non-strophe spines must contain a null "\
				 "interpretation at line " NR > stderr
			exit
			}
		else label[i] = ""
		}
	#
	# Search the list of labels for the required label
	#
	position = 0
	for (i = 1; i <= NF; i += 1)
		{
		if (strophe_spines[i] && label[i] == extract)
			{
			position = i
			break
			}
		}
	#
	# If it was not found then print an error
	#
	if (position == 0)
		{
		print "strophe: ERROR: Strophe label not found at " \
			 "line " NR > stderr
		exit
		}
	#
	# Designate which spines are to be extracted
	#
	for (i = 1; i <= NF; i += 1)
		{
		if (i != position) spines_to_cut[i] = FALSE
		else spines_to_cut[i] = TRUE
		}
	#
	# Modify the flag to indicate we are in a strophic passage
	#
	split_strophe = FALSE
	}

#############################################################################
##					Function process_strophe_end
#
function process_strophe_end(   i)
	{
	#
	# Loop through each of the currently active spines
	#
	for (i = 1; i <= NF; i += 1)
		{
		#
		# Each strophe spine must contain a *S/fin
		#
		if (strophe_spines[i])
			{
			if ($i != "*S/fin")
				{
				print "strophe: ERROR: Spine " i " requires the interpret"\
					 "ation *S/fin at line " NR > stderr
				exit
				}
			else $i = "*"
			}
		else if ($i != "*")
			{
			print "strophe: ERROR: Non-strophe spines must contain a null "\
				 "interpretation at line " NR > stderr
			exit
			}
		}
	#
	# Change flag to indicate we need to join the spines now
	#
	join_strophe = TRUE
	}

############################################################################
##					Function process_strophe_terminate
#
function process_strophe_terminate(    i,count,terminated)
	{
	count = 0
	#
	# Loop through each of the currently active spines
	#
	for (i = 1; i <= NF; i += 1)
		{
		#
		# If this is a strophe spine
		#
		if (strophe_spines[i])
			{
			terminated = i
			#
			# Count the spines, they should all be joined to one
			#
			if (count > 0)
				{
				print "strophe: ERROR: All strophe spines have not yet bee"\
					 "n joined together at line " NR > stderr
				exit
				}
			else if ($i != "*S-")
				{
				print "strophe: ERROR: Strophe spine does not have a stro"\
					 "phe terminator at line " NR > stderr
				exit
				}
			else
				{
				$i = "*"
				count += 1
				}
			}
		#
		# Non-strophe spines should have a null interpretation
		#
		else if ($i != "*")
			{
			print "strophe: ERROR: Non-strophe spines must contain a null "\
				 "interpretation at line " NR > stderr
			exit
			}
		}
	#
	# Reset the variables to show the strophic passage is over
	#
	strophe_spines[terminated] = FALSE
	spines_to_cut[terminated] = FALSE
	in_strophe = FALSE
	join_strophe = FALSE
	}

##############################################################################
##					Function process_spine_path
#
function process_spine_path(    i,output)
	{
	output = ""
	#
	# If not in a strophic passage, simply print and process the record
	#
	if (!in_strophe)
		{
		print $0
		store_indicators()
		process_indicators()
		if (current_no_of_spines == 0)
			{
			no_interpretation_yet = TRUE
			reset()
			}
		}
	#
	# If currently splitting strophe spines, ensure that the record is
	# of the correct format
	#
	else if (split_strophe)
		{
		for (i = 1; i <= NF; i += 1)
			{
			if ((strophe_spines[i] && $i != "*" && $i != "*^") \
								|| (!strophe_spines[i] && $i != "*"))
				{
				print "strophe: ERROR: Invalid spine-path indicator used "\
					 "when splitting\n                strophe spine at"\
					 " line " NR > stderr
				exit
				}
			}
		store_indicators()
		process_indicators()
		if (current_no_of_spines == 0)
			{
			no_interpretation_yet = TRUE
			reset()
			}
		}
	#
	# If currently joining strophe spines, ensure that the record is
	# of the correct format
	#
	else if (join_strophe)
		{
		for (i = 1; i <= NF; i += 1)
			{
			if ((strophe_spines[i] && $i != "*" && $i != "*v") \
								|| (!strophe_spines[i] && $i != "*"))
				{
				print "strophe: ERROR: Invalid spine-path indicator used "\
					 "when joining\n                strophe spines at "\
					 "line " NR > stderr
				exit
				}
			}
		store_indicators()
		process_indicators()
		if (current_no_of_spines == 0)
			{
			no_interpretation_yet = TRUE
			reset()
			}
		}
	#
	# Otherwise, output the correct spines and process the spines
	#
	else
		{
		for (i = 1; i <= NF; i += 1)
			{
			if (!strophe_spines[i] || spines_to_cut[i])
				output = output $i "\t"
			}
		store_indicators()
		process_indicators()
		if (current_no_of_spines == 0)
			{
			no_interpretation_yet = TRUE
			reset()
			}
		print substr(output,1,length(output)-1)
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
	for (i = 1; i <= NF; i += 1)
		{
		if ($i == "*") path_indicator[i] = "*"
		else path_indicator[i] = substr($i,2,1)
		}
	}

##################################################################
##			Function Process_indicators
#	This function processes the record according to the spine path indicators
# in it and modifies the arrays strophe_spines, path_indicator and spines_to_cut
# accordingly
#
function process_indicators(    i)
	{
	i = 1
	while (i <= current_no_of_spines)
		{
		if (path_indicator[i] == "^")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			strophe_spines[i+1] = strophe_spines[i]
			spines_to_cut[i+1] = spines_to_cut[i]
			}
		if (path_indicator[i] == "-")
			{
			del_array_pos(i)
			i -= 1
			}
		if (path_indicator[i] == "x")
			{
			exchange_spines()
			}
		if (path_indicator[i] == "v")
			{
			join_paths(i,i+1)
			del_array_pos(i+1)
			if (path_indicator[i+1] != "v") path_indicator[i] = "*"
			else i -= 1
			}
		if (path_indicator[i] == "+")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			strophe_spines[i+1] = strophe_spines[i]
			spines_to_cut[i+1] = spines_to_cut[i]
			}
		i += 1
		}
	}

############################################################
##			Function Ins_array_pos
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines + 1; j > current_element; j -= 1)
		{
		path_indicator[j] = path_indicator[j-1]
		strophe_spines[j] = strophe_spines[j-1]
		spines_to_cut[j] = spines_to_cut[j-1]
		}
	path_indicator[current_element-1] = "*"
	current_no_of_spines += 1
	}

##############################################################
##			Function Del_array_pos
#	Performs the opposite of function 'ins_array_pos'.
#
function del_array_pos(current_element,     j)
	{
	for (j = current_element; j < current_no_of_spines; j += 1)
		{
		path_indicator[j] = path_indicator[j+1]
		strophe_spines[j] = strophe_spines[j+1]
		spines_to_cut[j] = spines_to_cut[j+1]
		}
	delete path_indicator[j]
	delete strophe_spines[j]
	delete spines_to_cut[j]
	current_no_of_spines -= 1
	} 

###################################################################
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in strophe_spines and spines_to_cut.
#
function exchange_spines(    j,count,arraya,temp)
	{
	count = 1
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		if (path_indicator[j] == "x")
			{
			arraya[count] = j
			count += 1
			path_indicator[j] = "*"
			}
		}
	temp = strophe_spines[arraya[1]]
	strophe_spines[arraya[1]] = strophe_spines[arraya[2]]
	strophe_spines[arraya[2]] = temp
	temp = spines_to_cut[arraya[1]]
	spines_to_cut[arraya[1]] = spines_to_cut[arraya[2]]
	spines_to_cut[arraya[2]] = temp
	}

############################################################
##				Function join_paths
#
function join_paths(i,j)
	{
	if (in_strophe && !join_strophe)
		{
		if ((spines_to_cut[i] && !spines_to_cut[j]) \
							|| (spines_to_cut[j] && !spines_to_cut[i]))
			{
			print "strophe: ERROR: Cannot join a spine to be cut "\
				 "with one that\n                is not to be cut at "\
				 "line " NR > stderr
			exit
			}
		}
	}

#############################################################################
##					Function process_other
#	This function processes other records
#
function process_other(    i,output)
	{
	output = ""
	#
	# Cannot have any other record while splitting or joining strophe spines
	#
	if (split_strophe || join_strophe)
		{
		print "strophe: ERROR: Invalid record while splitting or "\
			 "joining\n                strophe spines at line " NR > stderr
		exit
		}
	#
	# If in a strophe passage, print the correct spines
	#
	else if (in_strophe)
		{
		for (i = 1; i <= NF; i += 1)
			{
			if (!strophe_spines[i] || spines_to_cut[i])
				output = output $i "\t"
			}
		print substr(output,1,length(output)-1)
		}
	#
	# Otherwise, print the record
	#
	else print $0
	}

###########################################################################
## 					Function reset
#	This function is used to reset all variables after spines terminate
#
function reset(   i)
	{
	for (i in strophe_spines) delete strophe_spines[i]
	for (i in spines_to_cut) delete spines_to_cut[i]
	if (options ~ /s/)
		{
		next_main = strophe_list[1]
		s_counter = 1
		}
	else next_main = "1"
	next_part = 0
	}
