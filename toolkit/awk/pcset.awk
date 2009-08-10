#####################################################################
##					   PCSET.AWK
# Programmed by: David Huron      Date: 1993 September 8
#
# Modifications:
#   Date:      Programmer:       Description:
# June 23/94	Tim Racinsky	Modified for getopts, added options n and v, 
#						rewrote much of main program.
# July 14/94	Tim Racinsky	Fixed bug regarding 'Z' values for **pcset
#
# Main Functions used:
#	Parse_command()		Store_indicators()		Store_new_interps()
#	Process_indicators()	Ins_array_pos()		Del_array_pos()
#	Exchange_spines()
#
#	VARIABLES:
#
#	ARRAYS:
#
BEGIN {
	USAGE="\nUSAGE: pcset -h                (Help Screen)\n       pcset "\
		 "[-c] [-n|-p|-v] [file ...]\n"
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
	#
	# Other global variables
	#
	no_interpretation_yet = TRUE
	previous_output = ""
	null_interp = "^\\*(\t\\*)*$"
	spine_path_record = "^\\*(\\+|v|\\^|x|-)?(\t\\*(\\+|v|\\^|x|-)?)*$"
	exclusive_record = "^\\*(\\*([^\t])+)?(\t\\*(\\*([^\t])+)?)*$"
	spine_terminate = "^\\*-(\t\\*-)*$"
	input_interps = "^(\\*\\*((pc)|(semits)|(pcset)|(nf)|(pf)|(iv)))$"
	pc_pitch = "(1[01])|[0-9]|([AB])|([TE])"
	semits_pitch = "[+-]?[0-9]+"
	pcset_pitch = "[1-9]+-[Z]?[0-9]+"
	nf_pitch = "\\([0-9ABTE]+\\)"
	iv_pitch = "<[0-9AB]+>"
	#
	# Determine the options specified by the user, passed from the shell script
	#
	options = ""
	parse_command()
	#
	# Define the pitch-class set arrays
	#
	define_pc_array()
	}
{
if ($0 ~ /^!/) process_comments()
else if ($0 ~ exclusive_record && $0 !~ null_interp) process_exclusive()
else if ($0 ~ spine_path_record && $0 !~ null_interp) process_spine_path()
else if ($0 ~ /^\*/) process_tandem()
else process_data()
}

############################################################################
#				FUNCTIONS USED IN PCSET.AWK
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
	if (ARGV[2] != "null")
		{
		if ((ARGV[2] ~ /n/ && ARGV[2] ~ /[pv]/) ||\
			(ARGV[2] ~ /p/ && ARGV[2] ~ /[nv]/))
			{
			print "pcset: ERROR: Can only specify one of -n, -p, and "\
				 "-v options." > stderr
			print USAGE > stderr
			exit
			}
		options = ARGV[2]
		}
	ARGV[1] = ARGV[2] = ""
	}

################################################################
##			Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
#
function store_new_interps(      j)
	{
	#
	# Loop through each valid spine storing exclusive interps in current_interp
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
	path_indicator[current_element-1] = "*"	# (Avoid possibility of
	path_indicator[current_element] = "*"	#  reprocessing the old spine.)
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
##				Function process_comments
#
function process_comments()
	{
	if ($0 ~ /^!!/) print $0
	else print "!"
	}

###########################################################################
##				Function process_exclusive
#
function process_exclusive()
	{
	if (no_interpretation_yet)
		{
		current_no_of_spines = NF
		no_interpretation_yet = FALSE
		store_new_interps()
		if (options ~ /n/) print "**nf"
		else if (options ~ /v/) print "**iv"
		else if (options ~ /p/) print "**pf"
		else print "**pcset"
		}
	else
		{
		store_new_interps()
		print "*"
		}
	}

###########################################################################
##				Function process_tandem
#
function process_tandem()
	{
	print "*"
	}

###########################################################################
##				Function process_spine_path
#
function process_spine_path()
	{
	store_indicators()
	process_indicators()
	#
	# Reset any necessary variables if all spines terminate
	#
	if (current_no_of_spines == 0)
		{
		no_interpretation_yet = TRUE
		print "*-"
		}
	#
	# Otherwise echo a null interpretation as a
	# place-holder.
	#
	else
		{
		print "*"
		}
	}

###########################################################################
##				Function process_data
#
function process_data(   i,counter,pitch_count,interm,prime,output\
													,temp_array)
	{
	pitch_count = 0
	output = ""
	for (i in pitches) delete pitches[i]
	#
	# Put all pitches in the current data record into the array pitches[].
	#
	for (i = 1; i <= current_no_of_spines; i += 1)
		{
		if (current_interp[i] == "**pc")
			pitch_count = add_pc($i,pitches,pitch_count)
		else if (current_interp[i] == "**semits")
			pitch_count = add_semits($i,pitches,pitch_count)
		else if (current_interp[i] == "**pcset")
			pitch_count = add_pcset($i,pitches,pitch_count)
		else if (current_interp[i] == "**nf" || current_interp[i] == "**pf")
			pitch_count = add_nf($i,pitches,pitch_count)
		else if (current_interp[i] == "**iv")
			pitch_count = add_iv($i,pitches,pitch_count)
		}
	if (pitch_count > 0)
		{
		counter = 0
		for (i in pitches) interm[counter++] = i
		sort(interm,pitch_count)
		if (options ~ /c/)
			{
			pitch_count = complement(interm,pitch_count)
			if (pitch_count == 0)
				{
				print "."
				return
				}
			}
		if (options ~ /n/)
			{
			normform(interm,pitch_count,prime)
			for (i = 0; i <= pitch_count; i += 1)
				{
				if (prime[i] == "10") prime[i] = "A"
				else if (prime[i] == "11") prime[i] = "B"
				}
			}
		else primeform(interm,pitch_count,prime)
		output = "("
		for (i = 0; i < pitch_count; i += 1)
			output = output prime[i]
		output = output ")"
		if (options ~ /[pn]/) print output
		else if (options ~ /v/)
			{
			output = pf[output]
			split(output,temp_array,"XXX")
			print temp_array[2]
			}
		else
			{
			output = pf[output]
			split(output,temp_array,"XXX")
			print temp_array[1]
			}
		}
	else
		{
		print "."
		}
	}

###################################################################
##				Function primeform
#
function primeform(pitches,pitch_count,prime     ,i,norm1,norm2,int1,int2)
	{
	normform(pitches,pitch_count,norm1)
	transpose(norm1,pitch_count)
	invert(pitches,pitch_count)
	sort(pitches,pitch_count)
	normform(pitches,pitch_count,norm2)
	transpose(norm2,pitch_count)
	for (i = 0; i < pitch_count; i += 1)
		{
		int1 = norm1[(i + 1) % 12] - norm1[i]
		int2 = norm2[(i + 1) % 12] - norm2[i]
		if (int1 != int2) break
		}
	if (int1 < int2)
		{
		for (i = 0; i <= pitch_count; i += 1) prime[i] = norm1[i]
		}
	else for (i = 0; i <= pitch_count; i += 1) prime[i] = norm2[i]
	}

###################################################################
##				Function normform
#
function normform(sorted,pitch_count,norm  ,first,i,j,next_int,intervals\
					      				,temp_int,min,int_count,comp)
	{
	for (i = 0; i < pitch_count; i += 1) intervals[i] = ""
	int_count = pitch_count
	comp = pitch_count - 1
	while (int_count > 1 && comp > 0)
		{
		first = TRUE
		for (i in intervals)
			{
			if (first)
				{
				first = FALSE
				min = (((sorted[(i + comp) % pitch_count] - sorted[i])\
					    % 12) + 12) % 12
				temp_int[i] == ""
				}
			else
				{
				next_int = (((sorted[(i + comp) % pitch_count] - sorted[i])\
						    % 12) + 12) % 12
				if (next_int + 0 < min + 0)
					{
					min = next_int
					for (j in temp_int) delete temp_int[j]
					temp_int[i] = ""
					int_count = 1
					}
				else if (next_int == min)
					{
					temp_int[i] = ""
					int_count += 1
					}
				}
			}
		comp -= 1
		for (i in intervals) delete intervals[i]
		for (i in temp_int) intervals[i] = ""
		for (i in temp_int) delete temp_int[i]
		}
	if (int_count > 1)
		{
		first = TRUE
		for (i in intervals)
			{
			if (first)
				{
				min = i
				first = FALSE
				}
			else if (i < min) min = i
			}
		}
	else for (i in intervals) min = i
	for (i = 0; i < pitch_count; i += 1)
		norm[i] = sorted[(min + i) % pitch_count]
	}

###################################################################
##				Function transpose
#
function transpose(array,pitch_count    ,i,offset)
	{
	offset = array[0]
	for (i = 0; i < pitch_count; i += 1)
		array[i] = (((array[i] - offset) % 12) + 12) % 12
	}

###################################################################
##				Function invert
#
function invert(sorted,pitch_count    ,i)
	{
	for (i = 0; i < pitch_count; i += 1) sorted[i] = 12 - sorted[i]
	}

###################################################################
##				Function complement
#
function complement(sorted,pitch_count    ,i,counter,comp_temp)
	{
	counter = 0
	for (j = 0; j < sorted[0] + 0; j += 1) comp_temp[counter++] = j
	for (i = 0; i < pitch_count - 1; i += 1)
		{
		for (j = sorted[i] + 1; j < sorted[i + 1] + 0; j += 1)
			comp_temp[counter++] = j
		}
	for (j = sorted[pitch_count - 1] + 1; j < 12; j += 1)
		comp_temp[counter++] = j
	for (i in sorted) delete sorted[i]
	for (i = 0; i < counter; i += 1) sorted[i] = comp_temp[i]
	return counter
	}

###########################################################################
##				Function add_pc()
#
function add_pc(token,pitches,pitch_count   ,i,pc_array,note)
	{
	split(token,pc_array," ")
	for (i in pc_array)
		{
		if (pc_array[i] !~ /[r=]/ && match(pc_array[i],pc_pitch) > 0)
			{
			note = substr(pc_array[i],RSTART,RLENGTH)
			if (note == "A" || note == "T") note = 10
			if (note == "B" || note == "E") note = 11
			if (!(note in pitches))
				{
				pitches[note] = ""
				pitch_count += 1
				}
			}
		}
	return pitch_count
	}

###########################################################################
##				Function add_semits()
#
function add_semits(token,pitches,pitch_count    ,i,semits_array,note)
	{
	split(token,semits_array," ")
	for (i in semits_array)
		{
		if (semits_array[i] !~ /[r=]/ \
							&& match(semits_array[i],semits_pitch) > 0)
			{
			note = substr(semits_array[i],RSTART,RLENGTH) + 0
			note = ((note % 12) + 12) % 12
			if (!(note in pitches))
				{
				pitches[note] = ""
				pitch_count += 1
				}
			}
		}
	return pitch_count
	}

###########################################################################
##				Function add_pcset()
#
function add_pcset(token,pitches,pitch_count     ,i,pcset_array,note)
	{
	split(token,pcset_array," ")
	for (i in pcset_array)
		{
		if (pcset_array[i] !~ /[=r]/ && match(pcset_array[i],pcset_pitch)>0)
			{
			note = substr(pcset_array[i],RSTART,RLENGTH)
			if (note in pcset)
				pitch_count = add_nf(pcset[note],pitches,pitch_count)
			}
		}
	return pitch_count
	}

###########################################################################
##				Function add_nf()
#
function add_nf(token,pitches,pitch_count     ,i,j,nf_array,note,pitch)
	{
	split(token,nf_array," ")
	for (i in nf_array)
		{
		if (nf_array[i] !~ /[=r]/ && match(nf_array[i],nf_pitch) > 0)
			{
			note = substr(nf_array[i],RSTART,RLENGTH)
			for (j = 1; j <= RLENGTH; j += 1)
				{
				pitch = substr(note,j,1)
				if (pitch == "A" || pitch == "T") pitch = 10
				if (pitch == "B" || pitch == "E") pitch = 11
				if (!(pitch in pitches) && pitch !~ /[()]/)
					{
					pitches[pitch] = ""
					pitch_count += 1
					}
				}
			}
		}
	return pitch_count
	}

###########################################################################
##				Function add_iv()
#
function add_iv(token,pitches,pitch_count    ,i,iv_array,note)
	{
	split(token,iv_array," ")
	for (i in iv_array)
		{
		if (iv_array[i] !~ /[=r]/ && match(iv_array[i],iv_pitch) > 0)
			{
			note = substr(iv_array[i],RSTART,RLENGTH)
			if (note in vector)
				pitch_count = add_nf(vector[note],pitches,pitch_count)
			}
		}
	return pitch_count
	}

###########################################################################
##				Function sort
#
function sort(arraya,pitch_count   ,i,j,first,min,min_index,sorted)
	{
	for (i = 0; i < pitch_count; i += 1)
		{
		first = TRUE
		for (j in arraya)
			{
			if (first)
				{
				min = arraya[j]
				min_index = j
				first = FALSE
				}
			else if (arraya[j] + 0 < min + 0)
				{
				min = arraya[j]
				min_index = j
				}
			}
		sorted[i] = min
		delete arraya[min_index]
		}
	for (i in sorted) arraya[i] = sorted[i]
	}

###################################################################
##				Function Define_pc_array
#	This function is called once by the BEGIN block, to define the
# pitch-class set information, including the Fortean set type, and
# the interval vector for each pitch-class set.
#
function define_pc_array()
	{
	pcset["1-1"] = vector["<000000>"] = "(0)"
	pcset["2-1"] = vector["<100000>"] = "(01)"
	pcset["2-2"] = vector["<010000>"] = "(02)"
	pcset["2-3"] = vector["<001000>"] = "(03)"
	pcset["2-4"] = vector["<000100>"] = "(04)"
	pcset["2-5"] = vector["<000010>"] = "(05)"
	pcset["2-6"] = vector["<000001>"] = "(06)"
	pcset["3-1"] = vector["<210000>"] = "(012)"
	pcset["3-2"] = vector["<111000>"] = "(013)"
	pcset["3-3"] = vector["<101100>"] = "(014)"
	pcset["3-4"] = vector["<100110>"] = "(015)"
	pcset["3-5"] = vector["<100011>"] = "(016)"
	pcset["3-6"] = vector["<020100>"] = "(024)"
	pcset["3-7"] = vector["<011010>"] = "(025)"
	pcset["3-8"] = vector["<010101>"] = "(026)"
	pcset["3-9"] = vector["<010020>"] = "(027)"
	pcset["3-10"] = vector["<002001>"] = "(036)"
	pcset["3-11"] = vector["<001110>"] = "(037)"
	pcset["3-12"] = vector["<000300>"] = "(048)"
	pcset["4-1"] = vector["<321000>"] = "(0123)"
	pcset["4-2"] = vector["<221100>"] = "(0124)"
	pcset["4-3"] = vector["<212100>"] = "(0134)"
	pcset["4-4"] = vector["<211110>"] = "(0125)"
	pcset["4-5"] = vector["<210111>"] = "(0126)"
	pcset["4-6"] = vector["<210021>"] = "(0127)"
	pcset["4-7"] = vector["<201210>"] = "(0145)"
	pcset["4-8"] = vector["<200121>"] = "(0156)"
	pcset["4-9"] = vector["<200022>"] = "(0167)"
	pcset["4-10"] = vector["<122010>"] = "(0235)"
	pcset["4-11"] = vector["<121110>"] = "(0135)"
	pcset["4-12"] = vector["<112101>"] = "(0236)"
	pcset["4-13"] = vector["<112011>"] = "(0136)"
	pcset["4-14"] = vector["<111120>"] = "(0237)"
	pcset["4-Z15"] = vector["<111111>"] = "(0146)"
	pcset["4-16"] = vector["<110121>"] = "(0157)"
	pcset["4-17"] = vector["<102210>"] = "(0347)"
	pcset["4-18"] = vector["<102111>"] = "(0147)"
	pcset["4-19"] = vector["<101310>"] = "(0148)"
	pcset["4-20"] = vector["<101220>"] = "(0158)"
	pcset["4-21"] = vector["<030201>"] = "(0246)"
	pcset["4-22"] = vector["<021120>"] = "(0247)"
	pcset["4-23"] = vector["<021030>"] = "(0257)"
	pcset["4-24"] = vector["<020301>"] = "(0248)"
	pcset["4-25"] = vector["<020202>"] = "(0268)"
	pcset["4-26"] = vector["<012120>"] = "(0358)"
	pcset["4-27"] = vector["<012111>"] = "(0258)"
	pcset["4-28"] = vector["<004002>"] = "(0369)"
	pcset["4-Z29"] = vector["<111111>"] = "(0137)"
	pcset["5-1"] = vector["<432100>"] = "(01234)"
	pcset["5-2"] = vector["<332110>"] = "(01235)"
	pcset["5-3"] = vector["<322210>"] = "(01245)"
	pcset["5-4"] = vector["<322111>"] = "(01236)"
	pcset["5-5"] = vector["<321121>"] = "(01237)"
	pcset["5-6"] = vector["<311221>"] = "(01256)"
	pcset["5-7"] = vector["<310132>"] = "(01267)"
	pcset["5-8"] = vector["<232201>"] = "(02346)"
	pcset["5-9"] = vector["<231211>"] = "(01246)"
	pcset["5-10"] = vector["<223111>"] = "(01346)"
	pcset["5-11"] = vector["<222220>"] = "(02347)"
	pcset["5-Z12"] = vector["<222121>"] = "(01356)"
	pcset["5-13"] = vector["<221311>"] = "(01248)"
	pcset["5-14"] = vector["<221131>"] = "(01257)"
	pcset["5-15"] = vector["<220222>"] = "(01268)"
	pcset["5-16"] = vector["<213211>"] = "(01347)"
	pcset["5-Z17"] = vector["<212320>"] = "(01348)"
	pcset["5-Z18"] = vector["<212221>"] = "(01457)"
	pcset["5-19"] = vector["<212122>"] = "(01367)"
	pcset["5-20"] = vector["<211231>"] = "(01568)"
	pcset["5-21"] = vector["<202420>"] = "(01458)"
	pcset["5-22"] = vector["<202321>"] = "(01478)"
	pcset["5-23"] = vector["<132130>"] = "(02357)"
	pcset["5-24"] = vector["<131221>"] = "(01357)"
	pcset["5-25"] = vector["<123121>"] = "(02358)"
	pcset["5-26"] = vector["<122311>"] = "(02458)"
	pcset["5-27"] = vector["<122230>"] = "(01358)"
	pcset["5-28"] = vector["<122212>"] = "(02368)"
	pcset["5-29"] = vector["<122131>"] = "(01368)"
	pcset["5-30"] = vector["<121321>"] = "(01468)"
	pcset["5-31"] = vector["<114112>"] = "(01369)"
	pcset["5-32"] = vector["<113221>"] = "(01469)"
	pcset["5-33"] = vector["<040402>"] = "(02468)"
	pcset["5-34"] = vector["<032221>"] = "(02469)"
	pcset["5-35"] = vector["<032140>"] = "(02479)"
	pcset["5-Z36"] = vector["<222121>"] = "(01247)"
	pcset["5-Z37"] = vector["<212320>"] = "(03458)"
	pcset["5-Z38"] = vector["<212221>"] = "(01258)"
	pcset["6-1"] = vector["<543210>"] = "(012345)"
	pcset["6-2"] = vector["<443211>"] = "(012346)"
	pcset["6-Z3"] = vector["<433221>"] = "(012356)"
	pcset["6-Z4"] = vector["<432321>"] = "(012456)"
	pcset["6-5"] = vector["<422232>"] = "(012367)"
	pcset["6-Z6"] = vector["<421242>"] = "(012567)"
	pcset["6-7"] = vector["<420243>"] = "(012678)"
	pcset["6-8"] = vector["<343230>"] = "(023457)"
	pcset["6-9"] = vector["<342231>"] = "(012357)"
	pcset["6-Z10"] = vector["<333321>"] = "(013457)"
	pcset["6-Z11"] = vector["<333231>"] = "(012457)"
	pcset["6-Z12"] = vector["<332232>"] = "(012467)"
	pcset["6-Z13"] = vector["<324222>"] = "(013467)"
	pcset["6-14"] = vector["<323430>"] = "(013458)"
	pcset["6-15"] = vector["<323421>"] = "(012458)"
	pcset["6-16"] = vector["<322431>"] = "(014568)"
	pcset["6-Z17"] = vector["<322332>"] = "(012478)"
	pcset["6-18"] = vector["<322242>"] = "(012578)"
	pcset["6-Z19"] = vector["<313431>"] = "(013478)"
	pcset["6-20"] = vector["<303630>"] = "(014589)"
	pcset["6-21"] = vector["<242412>"] = "(023468)"
	pcset["6-22"] = vector["<241422>"] = "(012468)"
	pcset["6-Z23"] = vector["<234222>"] = "(023568)"
	pcset["6-Z24"] = vector["<233331>"] = "(013468)"
	pcset["6-Z25"] = vector["<233241>"] = "(013568)"
	pcset["6-Z26"] = vector["<232341>"] = "(013578)"
	pcset["6-27"] = vector["<225222>"] = "(013469)"
	pcset["6-Z28"] = vector["<224322>"] = "(013569)"
	pcset["6-Z29"] = vector["<224232>"] = "(023679)" # Forte & Straus differ.
	pcset["6-30"] = vector["<224223>"] = "(013679)"
	pcset["6-31"] = vector["<223431>"] = "(014579)"
	pcset["6-32"] = vector["<143250>"] = "(024579)"
	pcset["6-33"] = vector["<143241>"] = "(023579)"
	pcset["6-34"] = vector["<142422>"] = "(013579)"
	pcset["6-35"] = vector["<060603>"] = "(02468A)"
	pcset["6-Z36"] = vector["<433221>"] = "(012347)"
	pcset["6-Z37"] = vector["<432321>"] = "(012348)"
	pcset["6-Z38"] = vector["<421242>"] = "(012378)"
	pcset["6-Z39"] = vector["<333321>"] = "(023458)"
	pcset["6-Z40"] = vector["<333231>"] = "(012358)"
	pcset["6-Z41"] = vector["<332232>"] = "(012368)"
	pcset["6-Z42"] = vector["<324222>"] = "(012369)"
	pcset["6-Z43"] = vector["<322332>"] = "(012568)"
	pcset["6-Z44"] = vector["<313431>"] = "(012569)"
	pcset["6-Z45"] = vector["<234222>"] = "(023469)"
	pcset["6-Z46"] = vector["<233331>"] = "(012469)"
	pcset["6-Z47"] = vector["<233241>"] = "(012479)"
	pcset["6-Z48"] = vector["<232341>"] = "(012579)"
	pcset["6-Z49"] = vector["<224322>"] = "(013479)"
	pcset["6-Z50"] = vector["<224232>"] = "(014679)"
	pcset["7-1"] = vector["<654321>"] = "(0123456)"
	pcset["7-2"] = vector["<554331>"] = "(0123457)"
	pcset["7-3"] = vector["<544431>"] = "(0123458)"
	pcset["7-4"] = vector["<544332>"] = "(0123467)"
	pcset["7-5"] = vector["<543342>"] = "(0123567)"
	pcset["7-6"] = vector["<533442>"] = "(0123478)"
	pcset["7-7"] = vector["<532353>"] = "(0123678)"
	pcset["7-8"] = vector["<454422>"] = "(0234568)"
	pcset["7-9"] = vector["<453432>"] = "(0123468)"
	pcset["7-10"] = vector["<445332>"] = "(0123469)"
	pcset["7-11"] = vector["<444441>"] = "(0134568)"
	pcset["7-Z12"] = vector["<444342>"] = "(0123479)"
	pcset["7-13"] = vector["<443532>"] = "(0124568)"
	pcset["7-14"] = vector["<443352>"] = "(0123578)"
	pcset["7-15"] = vector["<442443>"] = "(0124678)"
	pcset["7-16"] = vector["<435432>"] = "(0123569)"
	pcset["7-Z17"] = vector["<434541>"] = "(0124569)"
	pcset["7-Z18"] = vector["<434442>"] = "(0123589)"
	pcset["7-19"] = vector["<434343>"] = "(0123679)"
	pcset["7-20"] = vector["<433452>"] = "(0135679)"
	pcset["7-21"] = vector["<424641>"] = "(0124589)"
	pcset["7-22"] = vector["<424542>"] = "(0125689)"
	pcset["7-23"] = vector["<354351>"] = "(0234579)"
	pcset["7-24"] = vector["<353442>"] = "(0123579)"
	pcset["7-25"] = vector["<345342>"] = "(0234679)"
	pcset["7-26"] = vector["<344532>"] = "(0134579)"
	pcset["7-27"] = vector["<344451>"] = "(0124579)"
	pcset["7-28"] = vector["<344433>"] = "(0135679)"
	pcset["7-29"] = vector["<344352>"] = "(0124679)"
	pcset["7-30"] = vector["<343542>"] = "(0124689)"
	pcset["7-31"] = vector["<336333>"] = "(0134679)"
	pcset["7-32"] = vector["<335442>"] = "(0134689)"
	pcset["7-33"] = vector["<262623>"] = "(012468A)"
	pcset["7-34"] = vector["<254442>"] = "(013468A)"
	pcset["7-35"] = vector["<254361>"] = "(013568A)"
	pcset["7-Z36"] = vector["<444342>"] = "(0123568)"
	pcset["7-Z37"] = vector["<434541>"] = "(0134578)"
	pcset["7-Z38"] = vector["<434442>"] = "(0124578)"
	pcset["8-1"] = vector["<765442>"] = "(01234567)"
	pcset["8-2"] = vector["<665542>"] = "(01234568)"
	pcset["8-3"] = vector["<656542>"] = "(01234569)"
	pcset["8-4"] = vector["<655552>"] = "(01234578)"
	pcset["8-5"] = vector["<654553>"] = "(01234678)"
	pcset["8-6"] = vector["<654463>"] = "(01235678)"
	pcset["8-7"] = vector["<645652>"] = "(01234589)"
	pcset["8-8"] = vector["<644563>"] = "(01234789)"
	pcset["8-9"] = vector["<644464>"] = "(01236789)"
	pcset["8-10"] = vector["<566452>"] = "(02345679)"
	pcset["8-11"] = vector["<565552>"] = "(01234579)"
	pcset["8-12"] = vector["<556543>"] = "(01345679)"
	pcset["8-13"] = vector["<556453>"] = "(01234679)"
	pcset["8-14"] = vector["<555562>"] = "(01245679)"
	pcset["8-Z15"] = vector["<555553>"] = "(01234689)"
	pcset["8-16"] = vector["<554563>"] = "(01235789)"
	pcset["8-17"] = vector["<546652>"] = "(01345689)"
	pcset["8-18"] = vector["<546553>"] = "(01235689)"
	pcset["8-19"] = vector["<545752>"] = "(01245689)"
	pcset["8-20"] = vector["<545662>"] = "(01245789)"
	pcset["8-21"] = vector["<474643>"] = "(0123468A)"
	pcset["8-22"] = vector["<465562>"] = "(0123568A)"
	pcset["8-23"] = vector["<465472>"] = "(0123578A)"
	pcset["8-24"] = vector["<464743>"] = "(0124568A)"
	pcset["8-25"] = vector["<464644>"] = "(0124678A)"
	pcset["8-26"] = vector["<456562>"] = "(0124579A)"
	pcset["8-27"] = vector["<456553>"] = "(0124578A)"
	pcset["8-28"] = vector["<448444>"] = "(0134679A)"
	pcset["8-Z29"] = vector["<555553>"] = "(01235679)"
	pcset["9-1"] = vector["<876663>"] = "(012345678)"
	pcset["9-2"] = vector["<777663>"] = "(012345679)"
	pcset["9-3"] = vector["<767763>"] = "(012345689)"
	pcset["9-4"] = vector["<766773>"] = "(012345789)"
	pcset["9-5"] = vector["<766674>"] = "(012346789)"
	pcset["9-6"] = vector["<686763>"] = "(01234568A)"
	pcset["9-7"] = vector["<677673>"] = "(01234578A)"
	pcset["9-8"] = vector["<676764>"] = "(01234678A)"
	pcset["9-9"] = vector["<676683>"] = "(01235678A)"
	pcset["9-10"] = vector["<668664>"] = "(01234679A)"
	pcset["9-11"] = vector["<667773>"] = "(01235679A)"
	pcset["9-12"] = vector["<666963>"] = "(01245689A)"
	pcset["10-1"] = vector["<988888>"] = "(0123456789)"
	pcset["10-2"] = vector["<898888>"] = "(012345678A)"
	pcset["10-3"] = vector["<889888>"] = "(012345679A)"
	pcset["10-4"] = vector["<888988>"] = "(012345689A)"
	pcset["10-5"] = vector["<888898>"] = "(012345789A)"
	pcset["10-6"] = vector["<888889>"] = "(012346789A)"
	pcset["11-1"] = vector["<AAAAAA>"] = "(0123456789A)"
	pcset["12-1"] = vector["<BBBBBB>"] = "(0123456789AB)"

	pf["(0)"] = "1-1XXX<000000>"
	pf["(01)"] = "2-1XXX<100000>"
	pf["(02)"] = "2-2XXX<010000>"
	pf["(03)"] = "2-3XXX<001000>"
	pf["(04)"] = "2-4XXX<000100>"
	pf["(05)"] = "2-5XXX<000010>"
	pf["(06)"] = "2-6XXX<000001>"
	pf["(012)"] = "3-1XXX<210000>"
	pf["(013)"] = "3-2XXX<111000>"
	pf["(014)"] = "3-3XXX<101100>"
	pf["(015)"] = "3-4XXX<100110>"
	pf["(016)"] = "3-5XXX<100011>"
	pf["(024)"] = "3-6XXX<020100>"
	pf["(025)"] = "3-7XXX<011010>"
	pf["(026)"] = "3-8XXX<010101>"
	pf["(027)"] = "3-9XXX<010020>"
	pf["(036)"] = "3-10XXX<002001>"
	pf["(037)"] = "3-11XXX<001110>"
	pf["(048)"] = "3-12XXX<000300>"
	pf["(0123)"] = "4-1XXX<321000>"
	pf["(0124)"] = "4-2XXX<221100>"
	pf["(0134)"] = "4-3XXX<212100>"
	pf["(0125)"] = "4-4XXX<211110>"
	pf["(0126)"] = "4-5XXX<210111>"
	pf["(0127)"] = "4-6XXX<210021>"
	pf["(0145)"] = "4-7XXX<201210>"
	pf["(0156)"] = "4-8XXX<200121>"
	pf["(0167)"] = "4-9XXX<200022>"
	pf["(0235)"] = "4-10XXX<122010>"
	pf["(0135)"] = "4-11XXX<121110>"
	pf["(0236)"] = "4-12XXX<112101>"
	pf["(0136)"] = "4-13XXX<112011>"
	pf["(0237)"] = "4-14XXX<111120>"
	pf["(0146)"] = "4-Z15XXX<111111>"
	pf["(0157)"] = "4-16XXX<110121>"
	pf["(0347)"] = "4-17XXX<102210>"
	pf["(0147)"] = "4-18XXX<102111>"
	pf["(0148)"] = "4-19XXX<101310>"
	pf["(0158)"] = "4-20XXX<101220>"
	pf["(0246)"] = "4-21XXX<030201>"
	pf["(0247)"] = "4-22XXX<021120>"
	pf["(0257)"] = "4-23XXX<021030>"
	pf["(0248)"] = "4-24XXX<020301>"
	pf["(0268)"] = "4-25XXX<020202>"
	pf["(0358)"] = "4-26XXX<012120>"
	pf["(0258)"] = "4-27XXX<012111>"
	pf["(0369)"] = "4-28XXX<004002>"
	pf["(0137)"] = "4-Z29XXX<111111>"
	pf["(01234)"] = "5-1XXX<432100>"
	pf["(01235)"] = "5-2XXX<332110>"
	pf["(01245)"] = "5-3XXX<322210>"
	pf["(01236)"] = "5-4XXX<322111>"
	pf["(01237)"] = "5-5XXX<321121>"
	pf["(01256)"] = "5-6XXX<311221>"
	pf["(01267)"] = "5-7XXX<310132>"
	pf["(02346)"] = "5-8XXX<232201>"
	pf["(01246)"] = "5-9XXX<231211>"
	pf["(01346)"] = "5-10XXX<223111>"
	pf["(02347)"] = "5-11XXX<222220>"
	pf["(01356)"] = "5-Z12XXX<222121>"
	pf["(01248)"] = "5-13XXX<221311>"
	pf["(01257)"] = "5-14XXX<221131>"
	pf["(01268)"] = "5-15XXX<220222>"
	pf["(01347)"] = "5-16XXX<213211>"
	pf["(01348)"] = "5-Z17XXX<212320>"
	pf["(01457)"] = "5-Z18XXX<212221>"
	pf["(01367)"] = "5-19XXX<212122>"
	pf["(01568)"] = "5-20XXX<211231>"
	pf["(01458)"] = "5-21XXX<202420>"
	pf["(01478)"] = "5-22XXX<202321>"
	pf["(02357)"] = "5-23XXX<132130>"
	pf["(01357)"] = "5-24XXX<131221>"
	pf["(02358)"] = "5-25XXX<123121>"
	pf["(02458)"] = "5-26XXX<122311>"
	pf["(01358)"] = "5-27XXX<122230>"
	pf["(02368)"] = "5-28XXX<122212>"
	pf["(01368)"] = "5-29XXX<122131>"
	pf["(01468)"] = "5-30XXX<121321>"
	pf["(01369)"] = "5-31XXX<114112>"
	pf["(01469)"] = "5-32XXX<113221>"
	pf["(02468)"] = "5-33XXX<040402>"
	pf["(02469)"] = "5-34XXX<032221>"
	pf["(02479)"] = "5-35XXX<032140>"
	pf["(01247)"] = "5-Z36XXX<222121>"
	pf["(03458)"] = "5-Z37XXX<212320>"
	pf["(01258)"] = "5-Z38XXX<212221>"
	pf["(012345)"] = "6-1XXX<543210>"
	pf["(012346)"] = "6-2XXX<443211>"
	pf["(012356)"] = "6-Z3XXX<433221>"
	pf["(012456)"] = "6-Z4XXX<432321>"
	pf["(012367)"] = "6-5XXX<422232>"
	pf["(012567)"] = "6-Z6XXX<421242>"
	pf["(012678)"] = "6-7XXX<420243>"
	pf["(023457)"] = "6-8XXX<343230>"
	pf["(012357)"] = "6-9XXX<342231>"
	pf["(013457)"] = "6-Z10XXX<333321>"
	pf["(012457)"] = "6-Z11XXX<333231>"
	pf["(012467)"] = "6-Z12XXX<332232>"
	pf["(013467)"] = "6-Z13XXX<324222>"
	pf["(013458)"] = "6-14XXX<323430>"
	pf["(012458)"] = "6-15XXX<323421>"
	pf["(014568)"] = "6-16XXX<322431>"
	pf["(012478)"] = "6-Z17XXX<322332>"
	pf["(012578)"] = "6-18XXX<322242>"
	pf["(013478)"] = "6-Z19XXX<313431>"
	pf["(014589)"] = "6-20XXX<303630>"
	pf["(023468)"] = "6-21XXX<242412>"
	pf["(012468)"] = "6-22XXX<241422>"
	pf["(023568)"] = "6-Z23XXX<234222>"
	pf["(013468)"] = "6-Z24XXX<233331>"
	pf["(013568)"] = "6-Z25XXX<233241>"
	pf["(013578)"] = "6-Z26XXX<232341>"
	pf["(013469)"] = "6-27XXX<225222>"
	pf["(013569)"] = "6-Z28XXX<224322>"
	pf["(023679)"] = "6-Z29XXX<224232>"
	pf["(013679)"] = "6-30XXX<224223>"
	pf["(014579)"] = "6-31XXX<223431>"
	pf["(024579)"] = "6-32XXX<143250>"
	pf["(023579)"] = "6-33XXX<143241>"
	pf["(013579)"] = "6-34XXX<142422>"
	pf["(02468A)"] = "6-35XXX<060603>"
	pf["(012347)"] = "6-Z36XXX<433221>"
	pf["(012348)"] = "6-Z37XXX<432321>"
	pf["(012378)"] = "6-Z38XXX<421242>"
	pf["(023458)"] = "6-Z39XXX<333321>"
	pf["(012358)"] = "6-Z40XXX<333231>"
	pf["(012368)"] = "6-Z41XXX<332232>"
	pf["(012369)"] = "6-Z42XXX<324222>"
	pf["(012568)"] = "6-Z43XXX<322332>"
	pf["(012569)"] = "6-Z44XXX<313431>"
	pf["(023469)"] = "6-Z45XXX<234222>"
	pf["(012469)"] = "6-Z46XXX<233331>"
	pf["(012479)"] = "6-Z47XXX<233241>"
	pf["(012579)"] = "6-Z48XXX<232341>"
	pf["(013479)"] = "6-Z49XXX<224322>"
	pf["(014679)"] = "6-Z50XXX<224232>"
	pf["(0123456)"] = "7-1XXX<654321>"
	pf["(0123457)"] = "7-2XXX<554331>"
	pf["(0123458)"] = "7-3XXX<544431>"
	pf["(0123467)"] = "7-4XXX<544332>"
	pf["(0123567)"] = "7-5XXX<543342>"
	pf["(0123478)"] = "7-6XXX<533442>"
	pf["(0123678)"] = "7-7XXX<532353>"
	pf["(0234568)"] = "7-8XXX<454422>"
	pf["(0123468)"] = "7-9XXX<453432>"
	pf["(0123469)"] = "7-10XXX<445332>"
	pf["(0134568)"] = "7-11XXX<444441>"
	pf["(0123479)"] = "7-Z12XXX<444342>"
	pf["(0124568)"] = "7-13XXX<443532>"
	pf["(0123578)"] = "7-14XXX<443352>"
	pf["(0124678)"] = "7-15XXX<442443>"
	pf["(0123569)"] = "7-16XXX<435432>"
	pf["(0124569)"] = "7-Z17XXX<434541>"
	pf["(0123589)"] = "7-Z18XXX<434442>"
	pf["(0123679)"] = "7-19XXX<434343>"
	pf["(0135679)"] = "7-20XXX<433452>"
	pf["(0124589)"] = "7-21XXX<424641>"
	pf["(0125689)"] = "7-22XXX<424542>"
	pf["(0234579)"] = "7-23XXX<354351>"
	pf["(0123579)"] = "7-24XXX<353442>"
	pf["(0234679)"] = "7-25XXX<345342>"
	pf["(0134579)"] = "7-26XXX<344532>"
	pf["(0124579)"] = "7-27XXX<344451>"
	pf["(0135679)"] = "7-28XXX<344433>"
	pf["(0124679)"] = "7-29XXX<344352>"
	pf["(0124689)"] = "7-30XXX<343542>"
	pf["(0134679)"] = "7-31XXX<336333>"
	pf["(0134689)"] = "7-32XXX<335442>"
	pf["(012468A)"] = "7-33XXX<262623>"
	pf["(013468A)"] = "7-34XXX<254442>"
	pf["(013568A)"] = "7-35XXX<254361>"
	pf["(0123568)"] = "7-Z36XXX<444342>"
	pf["(0134578)"] = "7-Z37XXX<434541>"
	pf["(0124578)"] = "7-Z38XXX<434442>"
	pf["(01234567)"] = "8-1XXX<765442>"
	pf["(01234568)"] = "8-2XXX<665542>"
	pf["(01234569)"] = "8-3XXX<656542>"
	pf["(01234578)"] = "8-4XXX<655552>"
	pf["(01234678)"] = "8-5XXX<654553>"
	pf["(01235678)"] = "8-6XXX<654463>"
	pf["(01234589)"] = "8-7XXX<645652>"
	pf["(01234789)"] = "8-8XXX<644563>"
	pf["(01236789)"] = "8-9XXX<644464>"
	pf["(02345679)"] = "8-10XXX<566452>"
	pf["(01234579)"] = "8-11XXX<565552>"
	pf["(01345679)"] = "8-12XXX<556543>"
	pf["(01234679)"] = "8-13XXX<556453>"
	pf["(01245679)"] = "8-14XXX<555562>"
	pf["(01234689)"] = "8-Z15XXX<555553>"
	pf["(01235789)"] = "8-16XXX<554563>"
	pf["(01345689)"] = "8-17XXX<546652>"
	pf["(01235689)"] = "8-18XXX<546553>"
	pf["(01245689)"] = "8-19XXX<545752>"
	pf["(01245789)"] = "8-20XXX<545662>"
	pf["(0123468A)"] = "8-21XXX<474643>"
	pf["(0123568A)"] = "8-22XXX<465562>"
	pf["(0123578A)"] = "8-23XXX<465472>"
	pf["(0124568A)"] = "8-24XXX<464743>"
	pf["(0124678A)"] = "8-25XXX<464644>"
	pf["(0124579A)"] = "8-26XXX<456562>"
	pf["(0124578A)"] = "8-27XXX<456553>"
	pf["(0134679A)"] = "8-28XXX<448444>"
	pf["(01235679)"] = "8-Z29XXX<555553>"
	pf["(012345678)"] = "9-1XXX<876663>"
	pf["(012345679)"] = "9-2XXX<777663>"
	pf["(012345689)"] = "9-3XXX<767763>"
	pf["(012345789)"] = "9-4XXX<766773>"
	pf["(012346789)"] = "9-5XXX<766674>"
	pf["(01234568A)"] = "9-6XXX<686763>"
	pf["(01234578A)"] = "9-7XXX<677673>"
	pf["(01234678A)"] = "9-8XXX<676764>"
	pf["(01235678A)"] = "9-9XXX<676683>"
	pf["(01234679A)"] = "9-10XXX<668664>"
	pf["(01235679A)"] = "9-11XXX<667773>"
	pf["(01245689A)"] = "9-12XXX<666963>"
	pf["(0123456789)"] = "10-1XXX<988888>"
	pf["(012345678A)"] = "10-2XXX<898888>"
	pf["(012345679A)"] = "10-3XXX<889888>"
	pf["(012345689A)"] = "10-4XXX<888988>"
	pf["(012345789A)"] = "10-5XXX<888898>"
	pf["(012346789A)"] = "10-6XXX<888889>"
	pf["(0123456789A)"] = "11-1XXX<AAAAAA>"
	pf["(0123456789AB)"] = "12-1XXX<BBBBBB>"
	}
