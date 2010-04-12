#####################################################################
##					KEY.AWK
# Programmed by: David Huron        Date: September, 1990
# Copyright (c) 1993 David Huron
#
# Modifications:
#    Date:     Programmer:        	Description:
# July 7/91	David Huron		Changed from **hohum to **kern input.
# June 16/94	Tim Racinsky		Modified parse_command to work with getopts
#
# This program estimates the key of a given musical passage.
# The input may be either **kern or **semits representations conforming
# to the humdrum protocols.  The program adapts to input having varying
# numbers spines each with a different interpretation.
#
# KEY is unable to distinguish the following enharmonic spellings
# for tonic pitches: C-flat, B-sharp, E-sharp, F-flat.  Also,
# is it unable to distinguish enharmonic spellings involving
# double- or triple- sharps or flats.  KEY is able to distinguish
# more common enharmonic spellings (such as E-flat versus D-sharp).
#
BEGIN {
	USAGE="\nUSAGE: key -h                (Help Screen)\n       key [-af] "\
		 "[file ...]"
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#
	# Set stderr to point to appropriat place for error output
	#
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	# Set the other global variables
	#
	null_interp = "^\\*(\t\\*)*$"
	spine_path_record = "^\\*(\\+|v|\\^|x|-)?(\t\\*(\\+|v|\\^|x|-)?)*$"
	exclusive_record = "^\\*(\\*([^\t])+)?(\t\\*(\\*([^\t])+)?)*$"
	spine_terminate = "^\\*-(\t\\*-)*$"
	input_interps = "^\\*\\*(kern|semits)$"
	kern_pitch = "a+|b+|c+|d+|e+|f+|g+|A+|B+|C+|D+|E+|F+|G+"
	kern_duration = "(0|[1-9][0-9]*)"
	floating_pt_num = "[+-]?(([0-9]+(\\.[0-9]*)?)|((0*)?\\.[0-9]*))"
	no_interpretation_yet = TRUE
	current_no_of_spines = 0
	options = ""
	#
	# Make an array of kern pitches and their corresponding semits value
	#
	kern_array["a"] = 9  ; kern_array["b"] = 11 ; kern_array["c"] = 0;
	kern_array["d"] = 2  ; kern_array["e"] = 4  ; kern_array["f"] = 5;
	kern_array["g"] = 7  ; kern_array["A"] = -3 ; kern_array["B"] = -1;
	kern_array["C"] = -12; kern_array["D"] = -10; kern_array["E"] = -8;
	kern_array["F"] = -7 ; kern_array["G"] = -5 ;
	#
	# Parse the command line options
	#
	parse_command()
}
{
if ($0 ~ /^!/) next
else if ($0 ~ exclusive_record && $0 !~ null_interp) process_exclusive()
else if ($0 ~ spine_path_record && $0 !~ null_interp) process_spine_path()
else if ($0 ~ /^\*/) next
else process_data()
}
END	{
	for (i in class) counter += 1
	if (counter == 0)
		{
		print "key: ERROR: No valid kern or semits values found." > stderr
		exit
		}
	#
	# If the "f" option is flagged, then print the
	# frequency counts for all pitch classes.
	#
	if (options ~ /f/)
		{
		for (i = 0; i <= 11; i += 1)
			print "PC[" i "]: " class[i]
		}
	#
	# The tonality coefficients as determined by Krumhansl & Kessler (1982)
	# are defined for both the major and minor keys.
	#
	major[0] = 6.35; major[1] = 2.23; major[2] = 3.48; major[3] = 2.33
	major[4] = 4.38; major[5] = 4.09; major[6] = 2.52; major[7] = 5.19
	major[8] = 2.39; major[9] = 3.66; major[10]= 2.29; major[11]= 2.88
	#
	minor[0] = 6.33; minor[1] = 2.68; minor[2] = 3.52; minor[3] = 5.38
	minor[4] = 2.60; minor[5] = 3.53; minor[6] = 2.54; minor[7] = 4.75
	minor[8] = 3.98; minor[9] = 2.69; minor[10]= 3.34; minor[11]= 3.17
	#
	#
	# Determine the mean frequency for the pitch distribution.
	#
	for (i = 0; i <= 11; i += 1)
		sum += class[i]
	mean = sum / 12
	#
	# Determine the correlations between the frequency counts and
	# the tonality coefficients.   Do this for each of 12 keys.
	#
	major_mean = 3.4825
	minor_mean = 3.709167
	#
	# Try all keys.
	#
	for (i = 0; i <= 11; i += 1)
		{
		maj_numerator = min_numerator = 0
		maj_denominator = min_denominator = maj_temp = min_temp = temp = 0
		#
		# Examine all pitches for each key.
		#
		for (j = 0; j <= 11; j += 1)
			{
			subscript = (i+j) % 12
			#
			# For the major keys.
			#
			maj_numerator += (major[j]-major_mean) * (class[subscript]-mean)
			maj_temp += (major[j] - major_mean)^2
			#
			temp += (class[subscript]-mean)^2
			#
			# For the minor keys.
			#
			min_numerator += (minor[j]-minor_mean) * (class[subscript]-mean)
			min_temp += (minor[j] - minor_mean)^2
			}
		maj_denominator = sqrt(maj_temp * temp)
		min_denominator = sqrt(min_temp * temp)
		#
		if (maj_denominator == 0 || min_denominator == 0)
			{
			print "All keys equally likely."
			exit
			}
		r_major[i] = maj_numerator / maj_denominator
		r_minor[i] = min_numerator / min_denominator
		#
		# If the "a" option is flagged, then print the
		# frequency counts for all pitch classes.
		#
		if (options ~ /a/)
			{
			print "Tonic[" i "]	major " r_major[i] "	minor " r_minor[i]
			}
		}
	#
	# Now determine which correlation is the greatest.
	#
	# Start off with the assumption that C major is the best key.
	#
	best_key = 0; mode = "unknown"
	#
	# Compare all the remaining key correlations.
	#
	for (i = 0; i <= 11; i += 1)
		{
		if (r_major[i] > best_key)
			{
			#second_best_key = best_key
			best_key = r_major[i]; mode = "major"; pitch_class = i
			}
		if (r_minor[i] > best_key)
			{
			#second_best_key = best_key
			best_key = r_minor[i]; mode = "minor"; pitch_class = i
			}
		}
	#
	# A confidence measure can be determined by taking the difference
	# between the correlation for the "best key" and subtracting the
	# correlation for the "second best key".  The maximum confidence
	# score is 100; the minimum is zero.
	#
	# First, having found the "best key", find the "second best key."
	#
	second_best_key = 0
	#
	for (i = 0; i <= 11; i += 1)
		{
		if (r_major[i] != best_key && r_major[i] > second_best_key)
			{
			second_best_key = r_major[i]
			}
		if (r_minor[i] != best_key && r_minor[i] > second_best_key)
			{
			second_best_key = r_minor[i]
			}
		}
	#
	# The value 3 below is a scaling factor.
	#
	confidence = (best_key - second_best_key) * 100 * 3
	#
	# Avoid exceeding 100%.
	#
	if (confidence > 100) confidence = 100
	#
	# Determine the name of the tonic.
	#
	if (pitch_class == 0) tonic = "C"
	if (pitch_class == 1)
		{
		if (Csharp > Dflat) tonic = "C#"
		else  tonic = "D-flat"
		}
	if (pitch_class == 2) tonic = "D"
	if (pitch_class == 3)
		{
		if (Dsharp > Eflat) tonic = "D#"
		else  tonic = "E-flat"
		}
	if (pitch_class == 4) tonic = "E"
	if (pitch_class == 5) tonic = "F"
	if (pitch_class == 6)
		{
		if (Fsharp > Gflat) tonic = "F#"
		else  tonic = "G-flat"
		}
	if (pitch_class == 7) tonic = "G"
	if (pitch_class == 8)
		{
		if (Gsharp > Aflat) tonic = "G#"
		else  tonic = "A-flat"
		}
	if (pitch_class == 9) tonic = "A"
	if (pitch_class == 10)
		{
		if (Asharp > Bflat) tonic = "A#"
		else  tonic = "B-flat"
		}
	if (pitch_class == 11) tonic = "B"
	#
	# Print the result.
	#
	printf "Estimated key: %s %s	(r=%6.4f)	confidence:	"\
		"%2.1f%%\n", tonic, mode, best_key, confidence
}

############################################################################
##					function Parse_Command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	#
	# Set the 'options' variable
	#
	if (ARGV[2] != "null") options = ARGV[2]
	ARGV[1] = ARGV[2] = ""
	}

###########################################################################
##				Function process_exclusive
#
function process_exclusive()
	{
	#
	# If this is the first interpretation, set up the arrays and variables
	#
	if (no_interpretation_yet)
		{
		current_no_of_spines = NF
		no_interpretation_yet = FALSE
		store_new_interps()
		}
	else store_new_interps()
	}

###########################################################################
##				Function process_spine_path
#
function process_spine_path()
	{
	store_indicators()
	process_indicators()
	if (current_no_of_spines == 0) no_interpretation_yet = TRUE
	}

################################################################
##				Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
#
function store_new_interps(  j)
	{
	#
	# Store the exclusive interpretations
	#
	for (j = 1; j <= current_no_of_spines; j += 1)
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
	for (i = 1; i <= current_no_of_spines; i += 1)
		{
		if ($i == "*") path_indicator[i] = "*"
		else path_indicator[i] = substr($i,2,1)
		}
	}

##################################################################
##			Function Process_indicators
#	This function takes the spine-path indicators that were stored
# in the array 'path_indicator' in the function 'store_indicators'
# and manipulates the arrays 'path_indicator', 'current_interp',
# and 'previous_value'.
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
			current_interp[i+1] = current_interp[i]
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
			del_array_pos(i+1)
			if (path_indicator[i+1] != "v") path_indicator[i] = "*"
			else i -= 1
			}
		if (path_indicator[i] == "+")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = ""
			}
		i += 1
		}
	}

############################################################
##			Function Ins_array_pos
#	This function inserts new positions in the arrays 'path_indicator',
# and 'current_interp' and copies elements so that everything is preserved
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j -= 1)
		{
		path_indicator[j] = path_indicator[j-1]
		current_interp[j] = current_interp[j-1]
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
		current_interp[j] = current_interp[j+1]
		}
	delete path_indicator[j]
	delete current_interp[j]
	current_no_of_spines -= 1
	} 

###################################################################
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in current_interp.
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
	temp = current_interp[arraya[1]]
	current_interp[arraya[1]] = current_interp[arraya[2]]
	current_interp[arraya[2]] = temp
	}

###########################################################################
##				Function process_data
#
function process_data(    i,j,ms_count,ms_array)
	{
	for (i = 1; i <= current_no_of_spines; i += 1)
		{
		if (current_interp[i] ~ input_interps)
			{
			if ($i ~ /^[.]$/) continue
			else
				{
				ms_count = split($i,ms_array," ")
				for (j = 1; j <= ms_count; j += 1)
					{
					if (ms_array[j] ~ /[=r]/)
						continue
					else if (current_interp[i] ~ /^\*\*kern$/)
						process_kern(ms_array[j])
					else
						process_semits(ms_array[j])
					}
				}
			}
		}
	}

###########################################################################
##				Function process_kern
#
function process_kern(token,  pitch,semits,kern_dur,duration,dots,half,\
													pc,sharps,flats)
	{
	semits = 0
	#
	#
	#
	if (match(token,kern_pitch) == 0) return
	else
		{
		pitch = substr(token,RSTART,1)
		if (pitch ~ /[abcdefg]/) semits = kern_array[pitch]+(12*(RLENGTH-1))
		else semits = kern_array[pitch]-(12*(RLENGTH-1))
		#
		# Next determine the value of any flats or sharps
		#
		if (match(token,/-+/))
			{
			semits -=  RLENGTH
			flats = RLENGTH
			}
		else if (match(token,/#+/))
			{
			semits += RLENGTH
			sharps = RLENGTH
			}
		#
		# Keep track of the durations.
		#
		if (match(token,kern_duration) == 0) duration = 0.25
		else
			{
			kern_dur = substr(token,RSTART,RLENGTH) + 0
			dots = gsub(/[.]/,"",token)
			if (kern_dur != 0) duration = 1 / kern_dur
			else	 duration = 2
			half = duration / 2
			while (dots > 0)
				{
				duration = duration + half
				half = half / 2
				dots -= 1
				}
			}
		#
		# Update the class array
		#
		pc = int((((semits % 12) + 12) % 12) + .5)
		class[pc] += duration
		if (sharps > 0 || flats > 0)
			{
			if (pitch ~ /[Cc]/)
				{
				if (sharps == 1) Csharp += 1
				else if (flats == 1) Cflat += 1
				}
			else if (pitch ~ /[Dd]/)
				{
				if (sharps == 1) Dsharp += 1
				if (flats == 1) Dflat += 1
				}
			else if (pitch ~ /[Ee]/)
				{
				if (sharps == 1) Esharp += 1
				if (flats == 1) Eflat += 1
				}
			else if (pitch ~ /[Ff]/)
				{
				if (sharps == 1) Fsharp += 1
				if (flats == 1) Fflat += 1
				}
			else if(pitch ~ /[Gg]/)
				{
				if (sharps == 1) Gsharp += 1
				if (flats == 1) Gflat += 1
				}
			else if(pitch ~ /[Aa]/)
				{
				if (sharps == 1) Asharp += 1
				if (flats == 1) Aflat += 1
				}
			else if(pitch ~ /[Bb]/)
				{
				if (sharps == 1) Bsharp += 1
				if (flats == 1) Bflat += 1
				}
			}
		}
	}

###########################################################################
##				Function process_semits
#
function process_semits(token,   semits,pc)
	{
	semits = 0
	#
	# If there is a valid semits expression, store and process it
	#
	if (match(token,floating_pt_num))
		{
		semits = substr(token,RSTART,RLENGTH) + 0
		pc = int((((semits % 12) + 12) % 12) + .5)
		#
		# Default to a Quarter duration
		#
		class[pc] += 0.25
		}
	}
