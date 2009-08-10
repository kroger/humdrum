###########################################################################
#                          DUR
# Programmed by: Tim Racinsky         Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:        Description:
# June 7/94	Tim Racinsky	Modified for use with getopts
#
#
# 	This program calculates the duration in seconds of notes found in
# **kern and **recip spines in a valid humdrum file.
#
# Main Functions used:
#	Parse_command()	Store_indicators()		Process_indicators()
#	Ins_array_pos()	Del_array_pos()		Num_of_exchanges()
#	Exchange_spines()	Store_new_interps()		Process_data()
#	Parse_tempo()		Find_average()			Find_words()
#	To_lower()		Store_exclusives()		Store_tandems()
#
# Helper Functions used:
#	Print_interps()
#
#	VARIABLES:
#
#  -dur_number:  a regular expression to identify duration information in
#   a **kern or **recip data token.
#  -question_mark:  a regular expression to identify a question mark.
#  -number:  a regular expression to identify a valid tempo number.
#  -range:  a regular expression to identify a valid tempo range.
#  -words:  a regular expression to identify a 'word' tempo (e.g. Largo)
#  -tempo_interp:  a regular expression to identify any valid tempo tandem
#   interpretation marking.
#  -options:  holds the options specified by the user.
#  -tempo:  hold the current tempo as seconds per quarter-duration.
#  -beats_per_min:  holds the current tempo as quarter-durations per minute.
#  -first_interpretation_yet:  flag to indicate if the first exclusive 
#   interpretation has been found yet.
#  -spine_path_record:  flag to indicate if current interpretation record is
#   also a spine-path record.
#  -current_no_of_spines:  holds current number of active spine.
#
#	ARRAYS:
#
#  -Metronome:  each subscript is a valid tempo 'word' (e.g. Presto) and each
#   element contains the corresponding tempo that will be used.
#  -curent_interp:  each element holds the exclusive interpretation for the
#   corresponding spine in the input stream.
#  -path_indicator:  each element holds the spine-path indicator of the 
#   corresponding spine in the input stream for the current spine-path record.
#
BEGIN {
	USAGE="\nUSAGE: dur -h                 (Help Screen)\n       dur [-x] "\
		 "[-t num | -T num | -m num | -M num] [file ...]"
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#
	# Create the necessary regular expression variables
	#
	dur_number = "(0)|([1-9][0-9]*)"
	question_mark = "[?]"
	tnumber = "^(([+]?[1-9][0-9]*(\\.[0-9]*)?)|([+]?(0)?\\.(0)*[1-9][0-9]*))$"
	number = "(([1-9][0-9]*(\\.[0-9]*)?)|((0)?\\.(0)*[1-9][0-9]*))"
	range = number "[-]" number
	words = "\\[.+\\]"
	tempo_interp = "^\\*MM((" question_mark ")|(" number ")|(" range ")"\
				"|(" words "))$"
	#
	# Create the array 'Metronome' so that the program can determine the proper
	# tempo to use when these words are encountered in the input.
	#
	Metronome["grave"] = 40  		;	Metronome["largo"] = 50
	Metronome["lento"] = 56  		;	Metronome["larghetto"] = 63
	Metronome["adagio"] = 71  		;	Metronome["andante"] = 92
	Metronome["andantino"] = 95  		;	Metronome["moderato"] = 113
	Metronome["allegretto"] = 127  	;	Metronome["allegro"] = 144
	Metronome["tempo commodo"] = 150	;  	Metronome["tempo ordinario"] = 154
	Metronome["con moto"] = 166  		;	Metronome["vivace"] = 172
	Metronome["presto"] = 184  		;	Metronome["prestissimo"] = 200
	#
	# Set the options and get the tempo from the command line
	#
	options = ""
	tempo = 0
	parse_command()
	beats_per_min = (1 / tempo) * 60
	}
{
#
# If this is the first record, reset variables
#
if (FNR == 1)
	{
	first_interpretation_yet = spine_path_record = FALSE
	current_no_of_spines = 0
	}
#
# Print all comments.
#
if ($0 ~ /^[!]/) print $0
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
		current_no_of_spines = NF
		first_interpretation_yet = TRUE
		store_new_interps()
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
			print $0
			store_indicators()
			process_indicators()
			#
			# If all paths terminate, reset variables
			#
			if (current_no_of_spines == 0) first_interpretation_yet = FALSE
			}
		#
		# Otherwise store the new interps.
		#
		else store_new_interps()
		}
	}
#
# For any other records, print the appropriate fields
#
else process_data()
}

######################################################################
#			FUNCTIONS USED IN DUR.AWK
######################################################################

######################################################################
##				Function Parse_command
#	This function processes the command line data passed from the
# shell script 'dur.ksh'.  The following variable may be sent in this
# order:  a '-x' option if specified, a (-t or -T or -M or -m) option
# if specified, a number to accompany the previous option and a list
# of filenames.  None of the options are mandatory.
#
function parse_command()
	{
	#
	# Delete the DUMMY variable sent from the shell script
	#
	ARGV[1] = ""
	if (ARGV[2] != "null") options = "x"
	if (ARGV[3] != "null")
		{
		#
		# A valid tempo number must accompany the tempo option
		#
		if (ARGV[4] !~ tnumber)
			{
			print "dur: ERROR: Invalid tempo given on command " \
				 "line: " ARGV[4] > stderr
			exit
			}
		#
		# Depending on whether or not [Tt] or [Mm] were specified,
		# determine the correct tempo
		#
		else if (ARGV[3] ~ /[tT]/) tempo = ARGV[4]+0
		else tempo = 60 / ARGV[4]
		options = options ARGV[3]
		}
	#
	# Otherwise use default tempo
	#
	else tempo = 1
	ARGV[2] = ARGV[3] = ARGV[4] = ""
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
			# Simply delete the second spine.
			#
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
			}
		i++
		} #end while loop
	} #end function process_indicators

############################################################
##			Function Ins_array_pos
#	This function inserts new positions in the arrays 'path_indicator' and
# 'current_interp' and copies elements so that everything is preserved.
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
# elements in 'current_interp'.
#
function exchange_spines(    arraya,temp)
	{
	split(spines_to_exchange,arraya)
	temp = current_interp[arraya[1]]
	current_interp[arraya[1]] = current_interp[arraya[2]]
	current_interp[arraya[2]] = temp
	}

################################################################
##				Function Store_new_interps
#	This function calls the appropriate function depending on whether
# or not the given interpretation record contains tamdem or exclusive
# interpretations.
#
function store_new_interps()
	{
	#
	# If the current line contains an exclusive interpretation.
	#
	if ($0 ~ /(^|\t)\*\*/) store_exclusives()
	#
	# If the current line has only tandem and/or null interpretations
	#
	else store_tandems()
	}

########################################################################
##				Function Store_exclusives()
#	This function stores the exclusive interpretations and prints them and
# also prints a line of tandem interpretations showing the current tempo.
#
function store_exclusives(    j,next_line)
	{
	next_line = ""
	#
	#	Loop through every currently active spine.
	#
	for (j = 1; j <= current_no_of_spines; j++)
		{
		#
		# If the current token indicates **kern or **recip, store the
		# interpretation and replace the token with a **dur interpretation and
		# store the current tempo to be printed on the next line.
		#
		if ($j ~ /^\*\*(kern|recip)$/)
			{
			#
			#
			current_interp[j] = $j
			$j = "**dur"
			next_line = next_line "*MM" beats_per_min "\t"
			}
		#
		# Otherwise, simply store the interpretation and store a null
		# interpretation to printed on the next line.
		#
		else if ($j ~ /^\*\*/)
			{
			current_interp[j] = $j
			next_line = next_line "*\t"
			}
		#
		# If no interpretation specified, store a null interpretation to
		# be printed on the next line.
		#
		else next_line = next_line "*\t"
		}
	#
	# Print the current input line.
	#
	print $0
	#
	# If the next line has a tempo interpretation stored in it, print it.
	#
	if (next_line ~ /\*MM/)
		print substr(next_line,1,length(next_line)-1)
	}

#################################################################
##				Function Store_tandems
#	This function checks the current record of tandem interpretations
# for changes and inconsistencies in tempo interpretations.
#
function store_tandems(    j,current_beats,temp_beats)
	{
	current_beats = ""
	#
	# Loop through each currently active spine.
	#
	for (j = 1; j <= current_no_of_spines; j++)
		{
		#
		# If the current input spine contains a tempo interpretation and
		# is a **kern or **recip spine.
		#
		if (($j ~ /\*MM/) && (current_interp[j] ~ /\*\*(kern|recip)/))
			{
			#
			# Parse the interpretation to determine the tempo it signifies
			#
			temp_beats = parse_tempo($j)
			#
			# If the variable current_beats has not been set, set it
			#
			if (current_beats == "") current_beats = temp_beats
			#
			# Otherwise, if it has been set and temp_beats is not the same,
			# then different tempi have been specified so print an error.
			#
			else if (temp_beats != current_beats)
				{
				print "dur: ERROR: A different tempo was specified "\
					 "for concurrent spines in line " NR "." > stderr
				exit
				}
			#
			# If the new tempo is different from the existing tempo and the
			# over-ride options are in effect, print a null interpretation
			#
			if (current_beats != beats_per_min \
			    && (options ~ /M/ || options ~ /T/))
				$j = "*"
			#
			# Otherwise, print the new tempo interpretation.
			#
			else if (current_beats == "?" && options !~ /M/ \
				    && options !~ /T/)
				$j = "*MM" beats_per_min
			}
		}
	#
	# Print the current (new) interpretation record
	#
	print $0
	#
	# If the tempo was changed, update the tempo variables
	#
	if (current_beats != "" && current_beats != beats_per_min \
	    && current_beats != "?" && options !~ /M/ && options !~ /T/)
	    	{
		beats_per_min = current_beats
		tempo = 60 / beats_per_min
		}
	}

##################################################################
##				Function Parse_tempo
#	This function takes a tempo interpretation and first checks that
# it is valid and then determines the tempo it specifies.
#
function parse_tempo(current_tempo,   token)
	{
	#
	# If the intepretation is not a valid tempo interpretation
	#
	if (current_tempo !~ tempo_interp)
		{
		print "dur: ERROR: Invalid tempo interpretation "\
			 "specified at line " NR "." > stderr
		exit
		}
	token = substr(current_tempo,4)
	if (token ~ question_mark) return "?"
	else if (token ~ words) return parse_words(token) + 0
	else if (token ~ range) return find_average(token) + 0
	else if (token ~ number) return token + 0
	}

####################################################################
##				Function Find_average
#	This function determines the average of a 'range' tempo marking
#
function find_average(range,   arrayb)
	{
	split(range,arrayb,"-")
	return (((arrayb[1] + 0) + (arrayb[2] + 0)) / 2)
	}

###################################################################
##				Function Parse_words
#	This function determines which tempo is desired from a tempo 'word'
#
function parse_words(word,  j,lower_word)
	{
	#
	# Convert the word to lower-case letters
	#
	lower_word = to_lower(word)
	for (j in Metronome)
		{
		#
		# If an array subscript is in the word, return the corresponding tempo
		#
		if (index(lower_word,j) > 0) return Metronome[j]
		}
	return beats_per_min
	}

###############################################################
##				Function To_lower
#	A 'naive' way to convert each alphabetic character to lower-case
#
function to_lower(word)
	{
	gsub(/A/,"a",word) ; gsub(/B/,"b",word) ; gsub(/C/,"c",word)
	gsub(/D/,"d",word) ; gsub(/E/,"e",word) ; gsub(/F/,"f",word)
	gsub(/G/,"g",word) ; gsub(/H/,"h",word) ; gsub(/I/,"i",word)
	gsub(/J/,"j",word) ; gsub(/K/,"k",word) ; gsub(/L/,"l",word)
	gsub(/M/,"m",word) ; gsub(/N/,"n",word) ; gsub(/O/,"o",word)
	gsub(/P/,"p",word) ; gsub(/Q/,"q",word) ; gsub(/R/,"r",word)
	gsub(/S/,"s",word) ; gsub(/T/,"t",word) ; gsub(/U/,"u",word)
	gsub(/V/,"v",word) ; gsub(/W/,"w",word) ; gsub(/X/,"x",word)
	gsub(/Y/,"y",word) ; gsub(/Z/,"z",word)
	return word
	}

#############################################################
##				Function Process_data
#	This function takes a data record and converts each duration found
# in a **recip and **kern spine.
#
function process_data(     i,current_line,token,dots,duration,dot_duration)
	{
	current_line = ""
	#
	# Loop through each currently active spine
	#
	for (i = 1; i <= current_no_of_spines; i++)
		{
		#
		# If the current spine contains a **kern or **recip interpretation
		# and the current token is not a null and not a barline
		#
		if (current_interp[i] ~ /\*\*(kern|recip)/ && $i !~ /^[.]$/ \
		    && $i !~ /^[=]/)
			{
			#
			# Place all double stops into separate array elements.
			#
			doubles = split($i,double_array," ")
			#
			# Loop through the array elements (double_stops)
			#
			for (j = 1; j <= doubles; j++)
				{
				#
				# If the current token has no duration information, error
				#
				if (match(double_array[j],dur_number) == 0)
					{
					print "dur: ERROR: Missing duration in line " NR "." \
						 > stderr
					exit
					}
				#
				# Otherwise, remove that information and place a SUBSEP in
				# its place so that it may be replaced
				#
				else
					{
					token = substr(double_array[j],RSTART,RLENGTH) + 0
					sub(dur_number,SUBSEP,double_array[j])
					}
				#
				# Store any augmentation dots.
				#
				dots = gsub(/[.]/,"",double_array[j])
				#
				# If not a breve note, determine the duration
				#
				if (token != 0) duration = (1 / token) * 4 * tempo
				#
				# If it is a breve note
				#
				else duration = 8 * tempo
				#
				# Determine the new duration from augmentation dots.
				#
				dot_duration = duration/2.
				while (dots > 0)
					{
					duration = duration + dot_duration
					dot_duration = dot_duration/2.
					dots--
					}
				#
				# Use 4 decimal places for every duration
				#
				duration = sprintf("%6.4f",duration)
				#
				# If the x option was specified, simply display the duration
				#
				if (options ~ /x/) current_line = current_line duration " "
				#
				# Otherwise, put the new duration back in its original pos.
				#
				else
					{
					sub(SUBSEP,duration,double_array[j])
					current_line = current_line double_array[j] " "
					}
				}
			#
			# Replace the last blank with a tab for between spines.
			#
			sub(/ $/,"\t",current_line)
			}
		#
		# If not a kern or recip spine, simply store the given token
		#
		else current_line = current_line $i "\t"
		}
	#
	# Print the data record withouth the last tab
	#
	print substr(current_line,1,length(current_line)-1)
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
