##########################################################################
#					PROOF1.AWK
#
# Programmed by: Kyle Dawkins        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
#  Apr 24/95   David Huron       Revised barline handling.
#
#
#	PROOF1.AWK performs syntax checking on Kern files.
#   See below for a description of its function.
#
#	The following gives a brief summary of the available options:
#
#				List any options here.
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
# * check for barline at same place and with same measure number
#   in each spine
# * check for measure numbers that may be out of sequence

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

	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	TOL = 0.0001

	"echo $CON" | getline stderr
	close("echo $CON")

	previous_bar_no = 0

	############################
	# Some regular expressions:
	############################

	question_mark = "[?]"
	number = "(([1-9][0-9]*(\\.[0-9]*)?)|((0)?\\.(0)*[1-9][0-9]*))"
	range = number "[-]" number
	words = "\\[.+\\]"
	tempo_interp = "^\\*MM((" question_mark ")|(" number ")|(" range ")"\
				"|(" words "))$"
	meter_sig = "^\\*M(([1-9]+((\\+)([1-9]+))*(\\/)([0-9]+)(\\.)*)|(\\?)|(X))$"
	KEY = "^\\*(([ABCDEFGabcdefg](#?|-?))|(\\?)|(X)|(Cx)|(cx)|(Dx)):$"
#	PC_KEY_SIGNATURE = "^\\*k\\[([abcdefg]((n)|(#)|(x+(#?))|(-)+))*\\]$"
	PC_KEY_SIGNATURE = "^\\*k\\[([abcdefg]((n)|(#)+|(-)+))*\\]$"
	PH_KEY_SIGNATURE = "^\\*K\\[([ABCDEFG][1-9]((n)|(#)|(x+(#?))|(-)+))*\\]$"
#	KEY = "^\\*(([ABCDEFGabcdefg](((x)*(#)?)|(-+)))|(\\?)|(X)):$"
#	PC_KEY_SIGNATURE = "^\\*k\\[([abcdefg]((n)|(#)|(x+(#?))|(-)+))*\\]$"
#	PH_KEY_SIGNATURE = "^\\*K\\[([ABCDEFG][1-9]((n)|(#)|(x+(#?))|(-)+))*\\]$"
	REPETITION = "(a.*a)|(b.*b)|(c.*c)|(d.*d)|(e.*e)|(f.*f)|(g.*g)"
	SINGLEBAR = "^=([0-9]+[[:lower:]]?)?;?(:?[|!'`\"]+:?)?-?@?$"
	DOUBLEBAR = "^==;?(:?[|!'`\"]+)?-?@?$"

	###########################################################################
	# Determine the options specified by the user, passed from the shell script
	###########################################################################

	parse_command()
}
{
	if (FNR == 1)
	{
		no_interpretation_yet = TRUE
		first_data_record = TRUE
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

				store_new_interps()

				for (k=1; k<=current_no_of_spines; k++)
				{
					if (kern_list[k] == TRUE)
					{
						if ($k ~ meter_sig) 
							meter_found[k] = TRUE
				
						if ($k ~ tempo_interp) 
							tempo_found[k] = TRUE
	
						if ($k ~ KEY)
							key_found[k] = TRUE
	
						if ($k ~ PH_KEY_SIGNATURE)
						{
							token = $k
							gsub(/*K\[/,"",token); gsub(/\]/,"",token)
							split(token,array,"[n#x-]")
							for (i in array) if (array[i] == "") delete array[i]
							for (i in array)
							{
								for (j in array)
								{
									if (array[i] == array[j] && i != j)
									{
										print "proof: Error: Incoherent key "\
											"signature, line " FNR ":  " \
											array[i] " assigned more than one "\
											"accidental to the same pitch."
									}
								}
							}
							key_sig_found[k] = TRUE
						}
						else
						{
							if ($k ~ PC_KEY_SIGNATURE && $k !~ REPETITION)
							{
								key_sig_found[k] = TRUE
							}
						}
					}
				}		
			}
		}
	}
	else
	{
		#####################
		# Processing tokens
		#####################

		if (options != "w" && first_data_record)
		{
			for (k=1; k<=current_no_of_spines; k++)
			{
				if (kern_list[k] == TRUE)
				{
					if (!meter_found[k])
						print "proof: Warning: No meter declaration for "\
							"spine " k
					if (!tempo_found[k])
						print "proof: Warning: No tempo declaration for "\
							"spine " k
					if (!key_found[k] && !key_sig_found[k])
						print "proof: Warning: No key or key signature "\
								"declaration for spine " k
				}
			}
			first_data_record = FALSE
		}

		if ($0 ~ /^=/)
		{
	
			######################################
			# Checks for incorrect barline syntax.
			######################################

			for (i=1; i<=current_no_of_spines && !done; i++)
			{
				barline_type = ""
				if (kern_list[i] == TRUE)
				{

					if ($i ~ /==/)
						{
						if (barline_type == "single")
							{
							print "proof: Error: Mixed single and double barlines "\
								"in line " FNR ": " $0
							}
						barline_type = "double"
						if ($i !~ DOUBLEBAR)
							{
							print "proof: Error: Incorrect barline in line " FNR ": " $0 
							}

						}
					else
						{
						if (barline_type == "double")
							{
							print "proof: Error: Mixed single and double barlines "\
								"in line " FNR ": " $0
							}
						barline_type = "single"
						if ($i !~ SINGLEBAR)
							{
							print "proof: Error: Incorrect barline in line " FNR ": " $0
							}
						}
				}
			}

			if ($0 ~ /@/) next	# Avoid further processing of 
						# non-metric barlines.
	
			######################################################
			# Checks for inconsistencies across all spines in
			# a barline record.
			######################################################

			last_examined = 0
			done = FALSE

			for (i=1; i<=current_no_of_spines && !done; i++)
			{
				if (kern_list[i] == TRUE)
				{
					if (last_examined != 0)
					{
						if ($i != $last_examined)
						{
							print \
								"proof: Error: Inconsistent barline indication"\
						  		" across spines at line " FNR
							done = TRUE
						}
					}
					last_examined = i
				}
			}
	
			if ($0 ~ /==/) next

			gsub (/[^0-9[:lower:]\t]/, "" )
	
			#####################################################
			# Checks for barline numbers out of sequence.
			#####################################################
	
			current_bar_no = ""
			for (i=1; i<=current_no_of_spines && !done; i++)
			{
				if (kern_list[i] == TRUE)
				{
					if (current_bar_no == "")
					{
						current_bar_no = $i
						# There are six conditions to consider:  e.g.
						# previous-bar-number   current-bar-number
						#  A:     1                    2
						#  B:     1                    2a
						#  C:     1                    1a
						#  D:     1a                   1b
						#  E:     1z                   2
						#  F:     1z                   2a

						if (current_bar_no !~/[[:lower:]]/)	# Process purely numeric current bars.
						{
							# CASE A:
							if (previous_bar_no !~ /[[:lower:]]/)
							{
								if (current_bar_no != previous_bar_no+1 && options !~ /w/)
									print "proof: Warning: Measure " current_bar_no ", may be out"\
									 " of place, near line " FNR "."
		
							}
							else
							{
								# CASE E:
								previous_number_temp = previous_bar_no
								gsub("[[:lower:]]","",previous_number_temp)
								if (current_bar_no == previous_number_temp+1) {}
								else
								{
									print "proof: Warning: Measure " current_bar_no ", may be out"\
									 " of place, near line " FNR "."
								}
							}
						}
						else	# Process mixed alphabetic/numeric bar numbers.
						{

							if (previous_bar_no !~ /[[:lower:]]/)
							{
								current_number_temp = current_bar_no
								gsub("[[:lower:]]","",current_number_temp)

								previous_number_temp = previous_bar_no
								gsub("[[:lower:]]","",previous_number_temp)

								previous_alpha_temp = previous_bar_no
								gsub("[0-9]","",previous_alpha_temp)

								# CASE B:
								if (current_number_temp == previous_number_temp+1) {}
								else
								{
									# CASE C:
									if (current_number_temp == previous_number_temp && current_alpha_temp > previous_alpha_temp) {}
									else
										{
										print "proof: Warning: Measure " current_bar_no ", may be out"\
										 " of place, near line " FNR "."
										}
								}
							}
						}
					}
				}
			}
			previous_bar_no = current_bar_no
		}
	}
}

END	{
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
			options = ARGV[2]
		
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
			kern_list[j] = TRUE

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
			# Insert a new position in the arrays and set the global
			# flag 'new_path' so that the next record can be checked
			# for the appropriate condition
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
	delete kern_list[j]
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
