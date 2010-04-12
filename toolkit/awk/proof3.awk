##########################################################################
#                                 PROOF3.AWK
#
# Programmed by: Kyle Dawkins        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# 1996 July 12 David Huron    Added checking of beaming.
#                             Fixed bug in slur checking.
#                             Added phrase elision checking.
# 1996 July 14 David Huron    Extensive revisions.
#
#	PROOF3.AWK performs token-based checking on kern files
#	to ensure that there are no illegal note/duration/ornament/
#	phrase/slur/tie/beam designations.
#
#	The following gives a brief summary of the available options:
#
# Main Functions used:
#	Parse_command()		Store_indicators()		Store_new_interps()
#	Process_indicators()	Ins_array_pos()		Del_array_pos()
#	Exchange_spines()	check_pitch()			check_duration()
#	check_ornaments()	check_markings()		find_pitch()
#
#	VARIABLES:
#
#		bar: contains the current bar number
#		ornaments, accidentals, ties, slurs, beams phrases: simplified
#				regular expressions for searching
#				tokens.
#
#	ARRAYS:
#
#		kern_list : contains TRUE for every **kern spine
#		toks : each spine token is broken up into separate tokens
#			   (double stops) for spearate processing.
#
# * check for unmatched braces marking phrases,
# * check for unmatched parentheses marking slurs,
# * check for unmatched L-J pairs marking beams,
# * check to ensure that tied notes share the identical pitch.
# * check for more than one of: trill, mordent, tie (either start, stop,
#    or continue), pauses, phrases, slurs, etc. within note tokens
# * check to ensure that notes do not have both sharps and flat accidentals
#    and naturals.
# * incoherent pitch tokens (i.e. assigning two pitches to the
#    same note).
# * incoherent duration tokens (i.e. assigning two durations
#	      to the same note).

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

	bar = 0
	notnote = "[^ABCDEFGabcdefg]+"
	ornaments = "[mMwWtTOS]"
	accidentals = "[#n-]"
	ties = "[][_]"
	slurs = "[()]"
	beams = "[LJ]"
	phrases = "[{}]"

	###########################################################################
	# Determine the options specified by the user, passed from the shell script
	###########################################################################

	parse_command()
}
{
	#############################################
	# For multiple files, every time we begin
	# reading a new one, we reset some variables.
	#############################################

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

					######################################
					# Check for any unmatched ties, slurs,
					# or beams in terminated spines.
					######################################

					if ($i ~ /^\*-$/)
					{
						if ($0 ~/^\*-(	\*-)*$/) # End?
						{
							for (k in beam)
							{
								if (beam[k] != 0)
									print "proof: Error: Incomplete beam in spine " k "."
							}
							for (k in slur)
							{
								if (slur[k] != 0)
									print "proof: Error: Incomplete slur in spine " k "."
							}
							for (k in phrasing)
							{
								if (phrasing[k] != 0)
									print "proof: Error: Incomplete phrase in spine " k "."
							}
							for (k in elided_phrase)
							{
								if (elided_phrase[k] != 0)
									print "proof: Error: Incomplete elided phrase in spine " k "."
							}
						}
						else
						{
							if (beam[i] != 0)
								print "proof: Error: Incomplete beam in spine " i "."
							if (slur[i] != 0)
								print "proof: Error: Incomplete slur in spine " i "."
							if (phrasing[i] != 0)
								print "proof: Error: Incomplete phrase in spine " i "."
							if (elided_phrase[i] != 0)
								print "proof: Error: Incomplete elided phrase in spine " i "."
						}
					}
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
			}		
		}
	}
	else
	{
		#####################
		# Processing tokens
		#####################

		if ($0 !~ /(^\=)|(\t=)/)
		{
			for (i=1; i<=current_no_of_spines; i++)
			{
				if (kern_list[i] == TRUE)
				{
					split ($i, toks, " ")
		
					for (j in toks)
					{
						if ( toks[j] !~ /^\./ )
						{
							check_pitch( toks[j], i )
 							check_duration( toks[j], i )
 							check_accidentals( toks[j], i )
 							check_ornaments( toks[j], i ) 
 							check_markings( toks[j], i )
						}
						delete toks[j]
					}
				}
			}
		}
	}
}
END	{
	for (j in tied)
	{
		if (tied[j] != 0)
			print "proof: Error: Unmatched ties in spine " j "."
	}
	for (j in slur)
	{
		if (slur[j] != 0)
			print "proof: Error: Unmatched slurs in spine " j "."
	}
}

############################################################################
#				FUNCTIONS USED IN PROOF3.AWK
############################################################################

############################################################################
##				Function Parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
{
	ARGV[1] = ""

	##########################################################
	# If there is an invalid option specified in the options,
	# remove the 'good' options and print an error with usage
	##########################################################

	if (ARGV[2] != "null" && ARGV[2] ~ OPTIONS_NEG_REGEXP)
	{
		gsub(OPTIONS_REGEXP,"",ARGV[2])
		print "proof: ERROR: Unknown option(s): " ARGV[2] > stderr
		print USAGE > stderr
		exit
	}
	else
	{
		#############################################
		# Otherwise, process any necessary variables
		#############################################

		if ( ARGV[2] != "null" )
			options = ARGV[2]

		ARGV[2] = ""
	}
}

################################################################
##			Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
################################################################

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
		}

		if ($j ~ /^\*\*/)
			current_interp[j] = $j
		else
			current_interp[j] = current_interp[j] "\t" $j
	}
}

###############################################################
##			Function Store_indicators
#	This function allows the spine-path indicators for the 
# current record to be stored in the array 'path_indicator' so
# that they may be used later.
###############################################################

function store_indicators(   i)
{
	for (i = 1; i <= current_no_of_spines; i++)
	{
		################################################
		# If the current field is a null interpretation
		################################################

		if ($i == "*") path_indicator[i] = "*"

		###########################################################
		# If the current field is a spine-path indicator, store it
		###########################################################

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
	
	######################################################
	# Loop through the entire array of 'path_indicator'
	# NOTE: This array may be altered during the process
	######################################################

	while (i <= current_no_of_spines)
	{
	
		######################################
		# If a spine-split indicator occurred
		######################################

		if (path_indicator[i] == "^")
		{

			###################################################
			# Insert a new array position in 'path_indicator'
			# and 'current_interp'
			###################################################

			ins_array_pos(i+1)
			delete tied_note[i+1]
			delete tied[i+1]
			path_indicator[i+1] = "*"
			current_interp[i+1] = current_interp[i]
		}

		#############################################
		# If a spine-termination indicator occurred
		#############################################

		if (path_indicator[i] == "-")
		{

			#########################################################
			# Delete that spine from the arrays and look at the same
			# position the next time through the loop
			#########################################################

			del_array_pos(i)
			i--
		}

		#########################################
		# If a spine-exchange indicator occurred
		#########################################

		if (path_indicator[i] == "x")
		{

			#####################
			# Exchange the paths
			#####################

			exchange_spines()
		}

		####################################
		# If a join-path indicator occurred
		####################################

		if (path_indicator[i] == "v")
		{
			del_array_pos(i+1)

			#####################################################
			# If the next indicator is not a join-path indicator
			# then return to original conditions
			#####################################################

			if (path_indicator[i+1] != "v")
			{
				path_indicator[i] = "*"
			}

			####################################################
			# Otherwise, look at current and next paths to join
			####################################################

			else i--
		}

		###################################
		# If a new-path indicator occurred
		###################################

		if (path_indicator[i] == "+")
		{

			#########################################################
			# Insert a new position in the arrays and set the global
			# flag 'new_path' so that the next record can be checked
			# for the appropriate condition
			#########################################################

			ins_array_pos(i+1)
			current_interp[i+1] = ""
		}

		i++
	}
}

#########################################################################
##			Function Ins_array_pos
#	This function inserts a new position in the arrays 'path_indicator'
# and 'current_interp' and copies elements so that order is preserved
#########################################################################

function ins_array_pos(current_element,     j) 
{
	for (j = current_no_of_spines+1; j > current_element; j--)
	{
		path_indicator[j] = path_indicator[j-1]
		current_interp[j] = current_interp[j-1]	
		kern_list[j] = kern_list[j-1]
		tied[j] = tied[j-1]
		tied_note[j] = tied_note[j-1]
		slur[j] = slur[j-1]
		beam[j] = beam[j-1]
		phrasing[j] = phrasing[j-1]
		elided_phrase[j] = elided_phrase[j-1]
	}
	path_indicator[current_element-1] = "*"
	path_indicator[current_element] = "*"
	current_no_of_spines++
}

##############################################################
##			Function Del_array_pos
#	Performs the opposite of function 'ins_array_pos'.
##############################################################

function del_array_pos(current_element,     j)
{
	for (j = current_element; j < current_no_of_spines; j++)
	{
		path_indicator[j] = path_indicator[j+1]
		current_interp[j] = current_interp[j+1]
		kern_list[j] = kern_list[j+1]
		tied[j] = tied[j+1]
		tied_note[j] = tied_note[j+1]
		slur[j] = slur[j+1]
		beam[j] = beam[j+1]
		phrasing[j] = phrasing[j+1]
		elided_phrase[j] = elided_phrase[j+1]
	}
	delete path_indicator[j]
	delete current_interp[j]
	delete kern_list[j]
	delete tied[j]
	delete tied_note[j]
	delete slur[j]
	delete beam[j]
	delete phrasing[j]
	delete elided_phrase[j]
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
	temp = kern_list[arraya[1]]
	kern_list[arraya[1]] = kern_list[arraya[2]]
	kern_list[arraya[2]] = temp
}

#################################################
# check_pitch : checks to see if the pitch
# is correctly encoded 
#################################################

function	check_pitch( token, num, found )
{
	if ( match ( token, /[ABCDEFGabcdefg]/ ))
	{
		name = substr ( token, RSTART, 1 )
		gsub ( name, "", token )
		if ( token ~ /[ABCDEFGabcdefg]/ )
		{
			print "proof: Error: Incorrect pitch specification in spine "\
				  num ", line " FNR ", \"" $num "\""
		}
	}
	else
	{
		if ( token != "." && token !~ /r/ )
		{
			print "proof: Error: Incorrect pitch specification in spine "\
			  num ", line " FNR ", \"" $num "\""
		}
	}
}

###################################################
# check_duration : checks to see if the duration
# is encoded once only for each stop in each token
###################################################

function	check_duration( token, num )
{
	if ( token ~ /[0-9]+[^ ]+[0-9]+/ )
		print "proof: Error: Incorrect duration specification in spine "\
			   num ", line " FNR ", \"" $num "\""
}

######################################################
# check_accidental : makes sure there is only
# one accidental type per token
######################################################

function	check_accidentals( token, num )
{
	if ( match ( token, accidentals ))
	{
		acc_start = RSTART
		match ( token, /[ABCDEFGabcdefg]+/ )
		note_start = RSTART
		if (note_start > acc_start)
		{
			print "proof: Error: Incorrect accidental specification in spine "\
						 num ", line " FNR ", \"" $num "\""
		}
		name = substr ( token, acc_start, 1 )
		gsub ( name, "", token )
		if ( token ~ accidentals )
		{
			print "proof: Error: Incorrect accidental specification in spine "\
						 num ", line " FNR ", \"" $num "\""
		}
	}
}

#######################################################
# check_ornaments : ensures there is only one ornament
# on any given note
#######################################################

function check_ornaments( token, num, found )
{
	if ( match ( token, ornaments) )
	{
		name = substr ( token, RSTART, 1 )
		sub ( name, "", token )
		if ( token ~ ornaments) 
		{
			print "proof: Error: Incorrect ornament specification in spine " \
						num ", line " FNR ", \"" $num "\""
		}
	}
}

#####################################################
# check_markings : checks ties, slurs and phrases
#####################################################

function check_markings( token, num, name )
{

	####################################
	# First we check for invalid ties
	####################################

	if ( match ( token, ties ) )
	{
		name = substr ( token, RSTART, 1 )

		if ( token ~ /\[/ )
		{
			#####################################
			# If it matches an open tie marker,
			# we add it to the list of open ties
			# for that given spine
			#####################################

			newnote = find_pitch( token )

			if (newnote == "")
			{
				print "proof: Error: Tie must begin on valid kern note, "\
					   "spine " num ", line " FNR	
			}
			else
			{
				tied[num] += 1
				tied_note[num] = tied_note[num] "/" newnote
			}
		}
		else if ( token ~ /\_/ )
		{
			######################################
			# If it matches a continue tie marker
			# we check if it is consistent in
			# pitch with an open tie marker
			######################################

			if (tied[num] <= 0)
			{ 
				print "proof: Error: Incorrect tie encoding in spine "\
					num ", line " FNR ", \"" $num "\""
				delete tied_note[num]
			}
			else if ( !pitch_in( find_pitch(token), tied_note[num] ) )
			{
				print "proof: Error: Tied notes inconsistent in spine " \
							num ", line " FNR ", \"" $num "\""
			}
		}
		else if ( token ~ /\]/ )
		{
			#######################################################
			# If it matches a close tie marker, we check if it
			# is consistent with an open tie marker in the spine
			# and if it is, we remove it from the list
			#######################################################

			if (tied[num] <= 0)
			{
				print "proof: Error: Incorrect tie in spine " num ", line " FNR\
					  " \"" $num "\""
			}
			else if (pitch_in( find_pitch(token), tied_note[num] ) )
			{
				tied[num] -= 1;
				if (tied[num] == 0)
				{
					delete tied_note[num]
				}
				else
				{
					remove(find_pitch(token), num)
				}
			}
			else
			{
				print "proof: Error: Tied notes inconsistent in spine "\
							num ", line " FNR ", \"" $num "\""
			}
		}

		#######################################################
		# Check for double instances of tie elements in tokens
		#######################################################
		
		name = "\\" name
		sub ( name, "", token )
		if ( token ~ ties )
		{
			print "proof: Error: Incorrect tie specification in spine " \
						num ", line " FNR " \"" $num "\""
		}
	}
	else if (tied[num] == TRUE)
	{
		######################################################
		# If a tie is not matched but there are ties open in
		# the current spine, report an error and remove any
		# open ties from the voice.
		######################################################

		print "proof: Error: Incorrect tie specification in spine " num ", line " FNR\
				" \"" $num "\""
		tied[num] = FALSE
		delete tied_note[num]
	}

	####################################
	# Next we check for errors in slurs
	####################################

	if ( match ( token, slurs ) )
	{
		if ( token ~ /\(/)
		{
			slur[num] = slur[num] + 1
		}
		else if ( token ~ /\)/ )
		{
			if (slur[num] <= 0)
			{
				slur[num] = 0
				print "proof: Error: Incorrect slur specification in spine " num ", line " FNR\
						" \"" $num "\""
			}
			else
				slur[num] -= 1
		}

		#######################################################
		# Check for double slur markings within a single token
		#######################################################

		name = substr ( token, RSTART, 1 )
		name = "\\" name
		sub ( name, "", token )
		if ( token ~ slurs )
		{
			print "proof: Error: Incorrect slur specification in spine " \
					num ", line " FNR ", \"" $num "\""
		}
	}

	#####################################
	# Next we check for errors in beaming
	#####################################

	if ( match ( token, beams ) )
	{
		temp = token

		if ( token ~ /L/)
		{
			beam[num] = beam[num] + gsub("L","",temp)
		}
		else if ( token ~ /J/ )
		{
			if (beam[num] <= 0)
			{
				beam[num] = 0
				print "proof: Error: Incorrect beam specification in spine " num ", near line " FNR\
						" \"" $num "\""
			}
			else
				beam[num] = beam[num] - gsub("J","",temp)
		}
		####################################################
		# Check that the number of partial beams makes sense
		# given the duration of the note.
		####################################################

		if ( token ~ /[Kk]/)
		{
			temp = token
			no_left_partial_beams  = gsub("k","",temp)
			no_right_partial_beams = gsub("K","",temp)
			max_no_partial_beams = no_left_partial_beams
			if (max_no_partial_beams < no_right_partial_beams)
				max_no_partial_beams = no_right_partial_beams
			gsub("[^0-9]","",temp)
			error_msg = "proof: Error: Incorrect partial beam specification in spine " num ", line " FNR\
						" \"" $num "\""
			if ( temp >= 128 && temp <= 192 && max_no_partial_beams > 4)
				print error_msg
			if ( temp >= 64  && temp <= 96  && max_no_partial_beams > 3)
				print error_msg
			if ( temp >= 32  && temp <= 48  && max_no_partial_beams > 2)
				print error_msg
			if ( temp >= 16  && temp <= 24  && max_no_partial_beams > 1)
				print error_msg
			if ( temp >= 8   && temp <= 12  && max_no_partial_beams > 0)
				print error_msg
		}
	}

	########################################
	# Finally, we look for invalid phrases
	########################################

	if ( match ( token, phrases ) )
	{
		if ( token ~ /{/)	# Monitor for open phrases.
		{
			if ( token ~ /\&{/)	# Elided phrases.
			{
				if (elided_phrase[num] > 0)
				{
					print "proof: Error: Restarting elided " \
						"phrase in spine " num ", line " \
						FNR ", \"" $num "\""
					elided_phrase[num] = 1
				}
				else
					elided_phrase[num] += 1
			}
			else			# Regular phrases.
			{
				if (phrasing[num] >0)
				{
					print "proof: Error: Restarting active " \
						"phrase in spine " num ", line " \
						FNR ", \"" $num "\""
					phrasing[num] = 1
				}
				else
					phrasing[num] += 1
			}
		}
		else if ( token ~ /}/ )	# Monitor phrase endings.
		{
			if ( token ~ /\&}/)	# Elided phrases.
			{
				if (elided_phrase[num] <=0)
				{
					print "proof: Error: Incorrect phrase " \
						"elision in spine " num \
						 ", line " FNR ", \"" $num "\""
					elided_phrase[num] = 0
				}
				else
					elided_phrase[num] -= 1
			}
			else			# Regular phrases.
			{
				if (phrasing[num] <=0)
				{
					print "proof: Error: Incorrect phrase " \
						"marking in spine " num \
						 ", line " FNR ", \"" $num "\""
					phrasing[num] = 0
				}
				else
					phrasing[num] -= 1
			}
		}

		###################################################################
		# Check for doubled instances of phrase markings in a single token
		###################################################################

		name = substr ( token, RSTART, 1 )
		name = "\\" name
		sub ( name, "", token )
		if ( token ~ phrases )
		{
			print "proof: Error: Incorrect phrase specification in spine " \
						num ", line " FNR ", \"" $num "\""
		}
	}

	##################################################################
	# Check for illegal double instances of markings in a single token
	##################################################################

	# First process those signifiers most commonly present.

	if ( token ~ /[nr]/ )
	{
		num = gsub ( "n", "n", token )
		if (num > 1)
		{
			print "proof: Error: Only one natural (n) permitted in note token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "r", "r", token )
		if (num > 1)
		{
			print "proof: Error: Only one rest (r) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
	}

	# Now process the less common signifiers.

	if ( token ~ /[hHImMoOpPqQRsStTuvwWz;:,'`~"^$]/ )
	{
		num = gsub ( "h", "h", token )
		if (num > 1)
		{
			print "proof: Error: Only one end-glissand mark (h) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "H", "H", token )
		if (num > 1)
		{
			print "proof: Error: Only one begin-glissand mark (H) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "I", "I", token )
		if (num > 1)
		{
			print "proof: Error: Only one generic articulation mark (I) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "m", "m", token )
		if (num > 1)
		{
			print "proof: Error: Only one mordent mark (m) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "M", "M", token )
		if (num > 1)
		{
			print "proof: Error: Only one mordent mark (M) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "o", "o", token )
		if (num > 1)
		{
			print "proof: Error: Only one harmonic mark (o) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "O", "O", token )
		if (num > 1)
		{
			print "proof: Error: Only one generic ornament mark (O) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "p", "p", token )
		if (num > 1)
		{
			print "proof: Error: Only one appoggiatura-consequent mark (p) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "P", "P", token )
		if (num > 1)
		{
			print "proof: Error: Only one appoggiatura mark (P) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "q", "q", token )
		if (num > 1)
		{
			print "proof: Error: Only one grace-note designator (q) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "Q", "Q", token )
		if (num > 1)
		{
			print "proof: Error: Only one groupetto designator (Q) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "R", "R", token )
		if (num > 1)
		{
			print "proof: Error: Only one terminating turn (R) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "s", "s", token )
		if (num > 1)
		{
			print "proof: Error: Only one spiccato mark (s) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "S", "S", token )
		if (num > 1)
		{
			print "proof: Error: Only one turn (S) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "t", "t", token )
		if (num > 1)
		{
			print "proof: Error: Only one trill (t) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "T", "t", token )
		if (num > 1)
		{
			print "proof: Error: Only one trill (T) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "u", "u", token )
		if (num > 1)
		{
			print "proof: Error: Only one down-bow mark (u) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "v", "v", token )
		if (num > 1)
		{
			print "proof: Error: Only one up-bow mark (v) mark permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "w", "w", token )
		if (num > 1)
		{
			print "proof: Error: Only one inverted mordent mark (w) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "W", "W", token )
		if (num > 1)
		{
			print "proof: Error: Only one inverted mordent mark (W) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "z", "z", token )
		if (num > 1)
		{
			print "proof: Error: Only one sforzando mark (z) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( ",", ",", token )
		if (num > 1)
		{
			print "proof: Error: Only one breath mark (,) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "\"", "\"", token )
		if (num > 1)
		{
			print "proof: Error: Only one pizzicato mark (\")permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( ";", ";", token )
		if (num > 1)
		{
			print "proof: Error: Only one pause (;) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( ":", ":", token )
		if (num > 1)
		{
			print "proof: Error: Only one arpeggiation designator (:) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "~", "~", token )
		if (num > 1)
		{
			print "proof: Error: Only one tenuto mark (~) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "`", "`", token )
		if (num > 1)
		{
			print "proof: Error: Only one attacca mark (`) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "\\^", "\\^", token )
		if (num > 1)
		{
			print "proof: Error: Only one accent mark (^) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "'", "'", token )
		if (num > 1)
		{
			print "proof: Error: Only one staccato (') permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
		num = gsub ( "\\$", "\\$", token )
		if (num > 1)
		{
			print "proof: Error: Only one inverted turn ($) permitted in data token."\
				"  Spine " num ", line " FNR ", \"" $num "\""
		}
	}

	##########################################################
	# Check for illegally concurrent signifiers in data tokens
	##########################################################

	if ( token ~ /H/ && token ~ /h/)
	{
		print "proof: Error: Glissando starts and ends on same note in data token."\
			"  Spine " num ", line " FNR ", \"" $num "\""
	}
	if ( token ~ /P/ && token ~ /Q/)
	{
		print "proof: Error: Note cannot be appoggiatura and groupetto concurrently."\
			"  Spine " num ", line " FNR ", \"" $num "\""
	}
	if ( token ~ /P/ && token ~ /q/)
	{
		print "proof: Error: Note cannot be appoggiatura and grace note concurrently."\
			"  Spine " num ", line " FNR ", \"" $num "\""
	}
	if ( token ~ /q/ && token ~ /Q/)
	{
		print "proof: Error: Note cannot be grace note and groupetto concurrently."\
			"  Spine " num ", line " FNR ", \"" $num "\""
	}
	if ( token ~ /R/ && token !~ /[OmMStTwW$]/)
	{
		print "proof: Error: No ornament specified with ending turn signifier."\
			"  Spine " num ", line " FNR ", \"" $num "\""
	}

}

############################################
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

##################################################
# remove : removes an element from a list
# by splitting the list on the "/" character,
# and reassembling it without the deleted element
##################################################

function remove( note, num )
{
	split ( tied_note[num], tied_arr, "/" )

	delete tied_note[num]

	for (k in tied_arr)
	{
		if (tied_arr[k] != note)
			tied_note[num] = tied_note[num] "/" tied_arr[k]
		delete tied_arr[k]
	}
}	

###############################################
# Pitch_in : checks if a pitch is in the tied
# note list
###############################################

function	pitch_in( pitch, t, tied_arr, k )
{
	split (t, tied_arr, "/")

	found = FALSE

	for (k in tied_arr)
	{
		if (find_pitch( tied_arr[k] ) == pitch)
			found = TRUE

		delete tied_arr[k]
	}
	return found
}
