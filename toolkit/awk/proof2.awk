##########################################################################
#                                  PROOF2.AWK
#
# Programmed by: Kyle Dawkins        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#    Date:      Programmer:           Description:
# 1995 March 1  David Huron     Fixed bug in handling breve durations.
# 1996 July 12  David Huron     False accidental warnings fixed.
#
#
#	PROOF2.AWK performs duration-based checking on a kern file.
#
#
# Main Functions used:
#	Parse_command()		Store_indicators()		Store_new_interps()
#	Process_indicators()	Ins_array_pos()		Del_array_pos()
#	Exchange_spines()
#
#				List all other functions used here.
#
# Helper Functions used:
#		List any functions used here that are only for debugging
#
#	VARIABLES:
#
#			List and describe all global variables used here.
#
#	ARRAYS:
#
#			List and describe all global arrays used here.
#
# * check to ensure that accumulated durations within each measure
#     are equivalent for each voice.
# * check that multiple-stops have the same durations.
# * check for bars which change meters without an explicit change of meter
# * check for repeated material in consecutive bars
# * material after a double barline.
# * check to ensure that the file ends with a double bar line. 
#

BEGIN {

	###############################################################
	# Set OPTIONS to the set of allowable options and fill in the 
	# Appropriate USAGE as well
	###############################################################

	OPTIONS = "w"
	OPTIONS_REGEXP = "[" OPTIONS "]"
	OPTIONS_NEG_REGEXP = "[^" OPTIONS "]"
	USAGE=""

	#########################################################
	# Set any necessary flags, variables, and constants here
	#########################################################

	meter_sig = "^\\*M(([1-9]+((\\+)([1-9]+))*(\\/)([0-9]+)(\\.)*)|(\\?)|(X))$"
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	TOL = 0.0001

	"echo $CON" | getline stderr
	close("echo $CON")

	firstbar = 0
	secondbar = 0
	oldbar = ""
	newbar = ""
	kern_spines = 0

	###########################################################################
	# Determine the options specified by the user, passed from the shell script
	###########################################################################

	parse_command()
}
{
	if (FNR == 1)
	{
		no_interpretation_yet = TRUE
	}
	if ($0 ~ /^!!/)
	{
	}
	else if ($0 ~ /^!([^!]|$)/)
	{
	}
	else if ($0 ~ /^\*/)
	{
		#########################################
		# Process first interpretation record
		#########################################

		if (no_interpretation_yet)
		{
			current_no_of_spines = NF
			no_interpretation_yet = FALSE
			store_new_interps()
		}
		else
		{
			########################################################
			# Otherwise, check to see if it is a spine-path record
			########################################################

			spine_path_record = FALSE
			for (i=1; i<=current_no_of_spines ; i++)
			{			
				if ($i ~ /^\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x$/) 
				{
					spine_path_record = TRUE 
					break
				}
			}

			##################################################################
			# If it is a spine-path record, store and process the indicators
			##################################################################

			if (spine_path_record)
			{
				store_indicators()
				process_indicators()
	
				#########################################################
				# Reset any necessary variables if all spines terminate
				#########################################################

				if (current_no_of_spines == 0)
				{
					no_interpretation_yet = TRUE
				}
			}
			else
			{
				#################################################
				# Process various tandem interpretations
				#################################################

				for (j=1; j<= current_no_of_spines; j++)
				{
					if ($j ~ meter_sig)
					{
						meter_list[j] = find_meter( $j )
					}
				}
					
				store_new_interps()
			}		
		}
	}
	else
	{
		if (double_bar == TRUE)
		{
			if (options !~ /w/)
				print "proof: Warning: Material follows after double barline at line " FNR
			double_bar = FALSE
		}

		################################################
		# First we check if there is repeated material
		# between consecutive bars.
		################################################
	
		if ($0 ~ /(^=)|(\t=)/)
		{
			if ( newbar == oldbar && secondbar>0 )
			{
				if (options !~ /w/)
					print "proof: Warning: Measure " secondbar\
						  " identical to measure " firstbar\
					" at line " FNR
			}
	
			firstbar = secondbar
			match ( $0, /\=[0-9]+/)
			secondbar = substr ( $0, RSTART+1, RLENGTH-1 )
			oldbar = newbar
			newbar = ""
	
			###############################################
			# Check the accumulated duration for each
			# spine
			###############################################
	
			sc = 0

			for (i in sp)
				delete sp[i]


			for (j=1; j<=current_no_of_spines; j++)
			{
				if (kern_list[j] == TRUE)
				{
					if (dur_tally[j] - meter_list[j] > TOL || \
						dur_tally[j] - meter_list[j] < -TOL )
					{
						sc = sc+1
						sp[j] = TRUE
					}
					#printf("%f ", meter_list[j])
					#printf("%f ", dur_tally[j]);
				}
			}
			#printf(" spine count %d %d", sc, current_no_of_spines)
			#printf("\n")
	
			if ($0 ~/@/)	# Avoid further processing of
			{		# non-metric barlines.
				if ($0 ~ /==/) found_double_bar = TRUE
				next
			}
			if ( sc == current_no_of_spines )
			{
				if (firstbar != 0)
				{
					if (options !~ /w/)
						print "proof: Warning: Possible change "\
							"of meter in measure " firstbar ", line " FNR
	
					for (j=1; j<=current_no_of_spines; j++)
					{
						if (kern_list[j] == TRUE)
						{
							meter_list[j] = dur_tally[j]
						}
						delete sp[j]
					}
				}
			}
			else
			{
				for (j in sp)
				{
					print "proof: Error: Timing error in measure " firstbar \
						  ", spine " j ", at line " FNR
					delete sp[j]
				}
			}

			for (j=1; j<=current_no_of_spines; j++)
					dur_tally[j] = 0

			########################################
			# If it's a double barline, set a flag
			########################################

			if ($0 ~ /\=\=.*/)
			{
				double_bar = TRUE
				found_double_bar = TRUE
				firstbar = second_bar
				secondbar = "DOUBLE"

				for (j=1; j<=current_no_of_spines; j++)
				{
					if (kern_list[j] == TRUE)
					{
						if ($j !~ /^\=\=.*/)
						{
							double_bar = FALSE
							found_double_bar = FALSE
						}
					}
				}
			}

			#####################################
			# Remove any recorded accidentals
			#####################################

			for (j in note_acc)
				delete note_acc[j]
		}
		else
		{
			newbar = newbar $0

			#######################################
			# Accumulate durations for each spine
			#######################################

			for (j=1; j<=current_no_of_spines; j++)
			{
				if (kern_list[j] == TRUE)
				{
					if ($j !~ /^\.$/)
					{
						d = find_duration( $j )
						dur_tally[j] = dur_tally[j] + d
					}
	
					###########################################
					# And make sure that double stops have
					# the same duration
					###########################################
	
					split ($j, stops, " ")
	
					old = 0
					temp = 0
	
					for (k in stops)
					{
						old = temp
						temp = find_duration( stops[k] )
	
						if ( old != 0 && (old-temp > TOL || old-temp < -TOL))
						{
							print "proof: Error: Double stops at line " FNR\
							  	" in spine " j " do not have same duration."
							break
						}	
	
						#######################################
						# Perform housekeeping on accidentals
						#######################################
	
						if ( stops[k] ~ /n/ )
							delete note_acc [ find_pitch( stops[k] ) ]

						if ( stops[k] ~ /[#\-]/ )
							note_acc [ find_pitch( stops[k] ) ] = TRUE
						else
						{
							if ( note_acc [ find_pitch( stops[k] ) ] == TRUE \
							 	&& stops[k] !~ /n/ && options !~ /w/ )
							{
								print "proof: Warning: Accidental may be"\
									" missing in m." secondbar\
									", spine " j ", at line " FNR ": " stops[k]
							}
						}	
					}
				}
			}
		}	
	}
}

END	{
	if (found_double_bar != TRUE && options !~ /w/ && kern_spines > 0)
	{
		print "proof: Warning: No double barline in input."
	}	
}

############################################################################
#				FUNCTIONS USED IN PROGRAM.AWK
############################################################################

############################################################################
##				Function Parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
{
	ARGV[1] = ""

	#
	# If there is an invalid option specified in the options,
	# remove the 'good' options and print an error with usage
	#
	if (ARGV[2] != "null" && ARGV[2] ~ OPTIONS_NEG_REGEXP)
	{
		gsub(OPTIONS_REGEXP,"",ARGV[2])
		print "proof: ERROR: Unknown option(s): " ARGV[2] > stderr
		print USAGE > stderr
		exit
	}
	else
	{
		if (ARGV[2]!="null")
			options=ARGV[2]

		ARGV[2] = ""
	}
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
		if ($j ~ /^\*\*kern$/)
		{
			kern_list[j] = TRUE
			kern_spines += 1
		}

		if ($j ~ /^\*\*/)
			current_interp[j] = $j
		else
			current_interp[j] = current_interp[j] "\t" $j
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
			meter_list[i+1] = meter_list[i]
			kern_list[i+1] = kern_list[i]
			dur_tally[i+1] = dur_tally[i]
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
			#
			# Insert a new position in the arrays 
			#
			ins_array_pos(i+1)
			current_interp[i+1] = ""
			dur_tally[i+1] = dur_tally[i]
			meter_list[i+1] = meter_list[i]
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
		dur_tally[j] = dur_tally[j-1]
		kern_list[j] = kern_list[j-1]
		meter_list[j] = meter_list[j-1]
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
		dur_tally[j] = dur_tally[j+1]
		kern_list[j] = kern_list[j+1]
		meter_list[j] = meter_list[j+1]
	}
	delete path_indicator[j]
	delete current_interp[j]
	delete dur_tally[j]
	delete kern_list[j]
	delete meter_list[j]
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
	temp = dur_tally[arraya[1]]
	dur_tally[arraya[1]] = dur_tally[arraya[2]]
	dur_tally[arraya[2]] = temp
	temp = meter_list[arraya[1]]
	meter_list[arraya[1]] = meter_list[arraya[2]]
	meter_list[arraya[2]] = temp
	temp = kern_list[arraya[1]]
	kern_list[arraya[1]] = kern_list[arraya[2]]
	kern_list[arraya[2]] = temp
}

##########################################
# find_duration : returns the duration
# in a kern token
##########################################

function find_duration( token )
{
	rc = match ( token, /[0-9]+/ )
	duration = substr ( token, RSTART, RLENGTH )

	if (duration != "")
	{
		if (duration == 0) duration = 2	# Handle breves separately.
		else
		{
			duration = 1.0/duration

			rc = match ( token, /\.+/ )

			if ( rc != 0 )
			{
				dotted = duration / 2.0

				for (i=1; i<= RLENGTH; i++)
				{
					duration = duration + dotted
					dotted = dotted / 2.0
				}
			}
		}
	}
	return duration+0.0
}

############################################
# find_meter : returns the value, in whole
# notes, (as a floating point value), of
# the meter signature
############################################

function	find_meter( token )
{
	rc = match ( token, /[0-9]+/ )

	if (rc != 0)
	{
		numer = substr ( token, RSTART, RLENGTH )
		sub ( /[0-9]+/, "", token )

		rc = match ( token, /[0-9]+\.*/ )

		if (rc != 0)
		{
			denom = substr ( token, RSTART, RLENGTH )

			d = find_duration( denom )

			meter = (numer + 0) * d
		}
		else
		{
			meter = 0
		}
	}
	else
	{
		meter = 0
	}

	return meter
}

##########################################
# find_pitch : returns a string containing
# solely the pitch information
############################################

function	find_pitch( token, name )
{
	if (match ( token, /[ABCDEFGabcdefg]+/ ))
		name = substr ( token, RSTART, RLENGTH )
	else
		name = ""

	return name
}
