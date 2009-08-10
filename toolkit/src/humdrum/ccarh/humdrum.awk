##########################################################################
#					HUMDRUM.AWK
#	This program allows the user to check whether a specified file or list
# of files conform(s) to proper humdrum syntax.  The -v option allows
# the user to see a summary of the file's contents.
#
# Main Functions used:
#	Parse_command()		Chk_first_interp()		Store_indicators()
#	Process_indicators()	Ins_array_pos()		Del_array_pos()
#	Num_of_exchanges()		Exchange_spines()		Join_check()
#	Store_new_interps()		Print_error()
#
# Helper Functions used:
#	Print_interps()	Print_indics()
#
#	VARIABLES:
#
#  -new_path:  flag to indicate if a new path (*+) has been issued.
#  -no_interpretation_yet:  flag to indicate when first interpretation
#   has been issued.
#  -current_no_of_spines:  holds current number of active spines.
#  -VERBOSE:  flag to indicate if '-v' options has been specified.
#  -global_comments:  holds the number of global comments found.
#  -local_comments:  holds the number of local comments found.
#  -maximum_no_of_spines:  holds the maximum number of concurrent spines
#  -minimum_no_of_spines:  holds the minimum number of concurrent spines
#   (without including 0 when all spines have been terminated)
#  -number_of_data_records: holds the number of data records parsed.
#  -added_spines:  holds the number of spines added with *+.
#  -terminated_spines:  holds the number of terminated spines with *-.
#  -split_spines:  holds the number of splits spines with *^.
#  -joined_spines:  holds the number joined spines with *v.
#  -exchanged_spines:  holds the number of exchanged spines with *x.
#
#	ARRAYS:
#
#  -UPPER:  contains upper case letters of alphabet.
#  -LOWER:  contains lower case letters of alphabet.
#  -NUM:  contains all numbers (0-9).
#  -ASCII:  contains all other ascii characters (except special characters).
#  -ERROR:  contains all the errors that are ever issued.
#  -WARNING: contains any warnings that are issued.
#  -signifier:  contains all special characters that need to be escaped.
#  -path_indicator:  holds spine-path indicator for each spine of current
#   spine-path record.
#  -current_interp:  holds exclusive and tandem interpretations for each spine.
#  -ex_interp_inventory:  holds all exclusive interpretations processed.
#  -tand_interp_inventory:  holds all tandem interpretations processed.
#
BEGIN {
	OPTIONS = "v"
	OPTIONS_REGEXP = "[" OPTIONS "]"
	OPTIONS_NEG_REGEXP = "[^" OPTIONS "]"
	USAGE = "usage: humdrum [option=" OPTIONS "] file ..."
	FS = "\t"
	TRUE = 1; FALSE = 0
	new_path = FALSE
	no_interpretation_yet = TRUE
	current_no_of_spines = 0
	VERBOSE = FALSE
	#
	# Define the error array
	#
	ERROR[1] = "humdrum: ERROR 1: Record containing add-spine indicator has "\
			 "not been \n                  followed by exclusive interpret"\
			 "ation for that spine\n                  in line "
	ERROR[2] = "humdrum: ERROR 2: Illegal empty record in line "
	ERROR[3] = "humdrum: ERROR 3: Leading tab in line "
	ERROR[4] = "humdrum: ERROR 4: Trailing tab in line "
	ERROR[5] = "humdrum: ERROR 5: Consecutive tabs in line "
	ERROR[6] = "humdrum: ERROR 6: Missing initial asterisk in interpretation"\
			 "\n                  keyword in line "
	ERROR[7] = "humdrum: ERROR 7: Null exclusive interpretation found in "\
			 "line\n                  "
	ERROR[8] = "humdrum: ERROR 8: Incorrect number of spines in interpretati"\
			 "on\n                  record in line "
	ERROR[9] = "humdrum: ERROR 9: Local comment precedes first exclusive"\
			  " interpretation\n                  record in line "
	ERROR[10] = "humdrum: ERROR 10: Number of sub-comments in local comment "\
			  "does not match the\n                   number of currently "\
			  "active spines in line "
	ERROR[11] = "humdrum: ERROR 11: Missing initial exclamation mark in loca"\
			  "l comment\n                   in line "
	ERROR[12] = "humdrum: ERROR 12: Data record appears before first exclusi"\
			  "ve interpretation\n                   record in line "
	ERROR[13] = "humdrum: ERROR 13: Number of tokens in data record does not"\
			  " match the\n                   number of currently active sp"\
			  "ines in line "
	ERROR[14] = "humdrum: ERROR 14: All spines have not been properly termina"\
			  "ted\n                   at line "
	ERROR[15] = "humdrum: ERROR 15: First exclusive interpretation record con"\
			  "tains a null\n                   interpretation in line "
	ERROR[16] = "humdrum: ERROR 16: First exclusive interpretation record con"\
			  "tains a spine-path\n                   indicator in line "
	ERROR[17] = "humdrum: ERROR 17: First exclusive interpretation record con"\
			  "tains a \n                   non-exclusive interpretation"\
			  " in line "
	ERROR[18] = "humdrum: ERROR 18: Spine-path indicators mixed with\n       "\
			  "            keyword interpretations in line "
	ERROR[19] = "humdrum: ERROR 19: Improper number of exchange-path indicat"\
			  "ors\n                   in line "
	ERROR[20] = "humdrum: ERROR 20: Single join-path indicator found at end"\
			  " of\n                   interpretation record in line "
	ERROR[21] = "humdrum: ERROR 21: Join-path indicator is not adjacent to an"\
			  "other\n                   join-path indicator in line "
	ERROR[22] = "humdrum: ERROR 22: Exclusive interpretations do not match fo"\
			  "r designated\n                   join spines in line "
	WARNING[1] = "humdrum: WARNING 1: Local comment may be mistaken for "\
			   "global comment \n                    in line "
	WARNING[2] = "humdrum: WARNING 2: Data token may be mistaken for global "\
			   "comment \n                    in line "
	WARNING[3] = "humdrum: WARNING 3: Data token may be mistaken for local "\
			   "comment \n                    in line "
	WARNING[4] = "humdrum: WARNING 4: Data token may be mistaken for exclusi"\
			   "ve interpretation \n                    in line "
	WARNING[5] = "humdrum: WARNING 5: Data token may be mistaken for tandem"\
			   " interpretation \n                    in line "
	#
	# Define various Ascii characters for the signifier inventory.
	#
	UPPER[1] = "A";  UPPER[2] = "B";  UPPER[3] = "C";  UPPER[4] = "D"
	UPPER[5] = "E";  UPPER[6] = "F";  UPPER[7] = "G";  UPPER[8]  = "H"
	UPPER[9] = "I";  UPPER[10] = "J"; UPPER[11] = "K"; UPPER[12] = "L"
	UPPER[13] = "M"; UPPER[14] = "N"; UPPER[15] = "O"; UPPER[16] = "P"
	UPPER[17] = "Q"; UPPER[18] = "R"; UPPER[19] = "S"; UPPER[20] = "T"
	UPPER[21] = "U"; UPPER[22] = "V"; UPPER[23] = "W"; UPPER[24] = "X"
	UPPER[25] = "Y"; UPPER[26] = "Z"
	LOWER[28] = "a"; LOWER[29] = "b"
	LOWER[30] = "c"; LOWER[31] = "d"; LOWER[32] = "e"; LOWER[33] = "f"
	LOWER[34] = "g"; LOWER[35] = "h"; LOWER[36] = "i"; LOWER[37] = "j"
	LOWER[38] = "k"; LOWER[39] = "l"; LOWER[40] = "m"; LOWER[41] = "n"
	LOWER[42] = "o"; LOWER[43] = "p"; LOWER[44] = "q"; LOWER[45] = "r"
	LOWER[46] = "s"; LOWER[47] = "t"; LOWER[48] = "u"; LOWER[49] = "v"
	LOWER[50] = "w"; LOWER[51] = "x"; LOWER[52] = "y"; LOWER[53] = "z"
	NUM[55] = "0"; NUM[56] = "1"; NUM[57] = "2"; NUM[58] = "3"
	NUM[59] = "4"; NUM[60] = "5"; NUM[61] = "6"; NUM[62] = "7"
	NUM[63] = "8"; NUM[64] = "9"
	ASCII[66] = "`"; ASCII[67] = "'"
	ASCII[68] = "\""; ASCII[69] = ","; ASCII[70] = ";"; ASCII[71] = ":"
	ASCII[72] = "?"; ASCII[73] = "!"; ASCII[74] = "@"; ASCII[75] = "#"
	ASCII[76] = "%"; ASCII[77] = "&"; ASCII[78] = "*"; ASCII[79] = "~"
	ASCII[80] = "_"; ASCII[81] = "-"; ASCII[82] = "+"; ASCII[83] = "="
	ASCII[84] = "<"; ASCII[85] = ">"; ASCII[86] = "{"; ASCII[87] = "}"
	}
{
if (NR == 1) parse_command()
#
# If an add-path indicator occurred, it must be followed immediately
# by an interpretation record containing an exclusive interpretation
# for the newly created path.
#
if (new_path)
	{
	for (i = 1; i <= current_no_of_spines; i++)
		{
		if (path_indicator[i] == "NEW")
			{
			if ($i !~ /^\*\*[^\t]/)
				{
				print ERROR[1] FNR ", file " FILENAME "."
				current_interp[i] = "**\t"
				}
			path_indicator[i]  = "*"
			}
		}
	new_path = FALSE
	}
#
# No blank lines are allowed in a Humdrum file
#
if ($0 == "") 
	{
	print ERROR[2] FNR ", file " FILENAME "."
	next
	}
#
# No leading tabs are allowed (Also remove the tabs to limit errors)
#
if ($0 ~ /^\t/)
	{
	print ERROR[3] FNR ", file " FILENAME "."
	sub(/(\t)+/,"")
	}
#
# If not looking at a global comment, consecutive, and 
# trailing tabs are not allowed. (Try to remove them as well)
#
if ($1 !~ /^\!\!/)
	{
	if ($0 ~ /\t$/)
		{
		print ERROR[4] FNR ", file " FILENAME "."
		sub(/(\t)+$/,"")
		}
	if ($0 ~ /\t\t/)
		{
		print ERROR[5] FNR ", file " FILENAME "."
		gsub(/\t(\t)+/,"\t")
		}
	}
#
# If the current record is a comment or an interpretation record
#
if ($1 ~/^\*/ || $1 ~ /^\!/)
	{
	if ($1 ~ /^\*/)
		{
		#
		# Interpretation records must have interpretations in each
		# field and cannot contain null exclusive interpretations
		#
		for (i = 1; i <= NF; i++)
			{
			if ($i !~/^\*/) print ERROR[6] FNR ", file " FILENAME "."
			if ($i ~ /^\*\*$/) print ERROR[7] FNR ", file " FILENAME "."
			}
		#
		# If this is the first interpretation record
		#
		if (no_interpretation_yet)
			{
			#
			# Check that it conforms to specifications and set
			# the specified variables
			#
			chk_first_interp()
			current_no_of_spines = NF
			if (NF > maximum_no_of_spines) maximum_no_of_spines = NF
			if (NF < minimum_no_of_spines || minimum_no_of_spines == 0) 
				minimum_no_of_spines = NF
			no_interpretation_yet = FALSE
			#
			# Save the new interpretations
			#
			store_new_interps()
			}
		#
		# If this is not the first interpretation record
		#
		else
			{
			#
			# Determine if the correct number of spines exists
			#
			if (NF != current_no_of_spines)
				print ERROR[8] FNR ", file " FILENAME "."
			#
			# Determine if this record is a spine-path record
			#
			spine_path_record = FALSE
			for (i = 1; i <= current_no_of_spines; i++)
				{			
				if ($i ~ /^\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x$/) {
					spine_path_record = TRUE 
					break
					}
				}
			#
			# If it is a spine-path record
			#
			if (spine_path_record)
				{
				#
				# Store and process the given spine-path indicators
				#
				store_indicators()
				process_indicators()
				if (current_no_of_spines > maximum_no_of_spines)
					maximum_no_of_spines = current_no_of_spines
				else if (NF < minimum_no_of_spines)
					minimum_no_of_spines = NF
				#
				# If all spines have been terminated, reset
				# the global flag that signals this
				#
				if (current_no_of_spines == 0) no_interpretation_yet = TRUE
				}
			#
			# If it is not a spine-path record
			#
			else
				{
				#
				# Store the new interpretations
				#
				store_new_interps()
				}		
			}
		}
	#
	# If it is a global comment
	#
	if ($1 ~/\!\!/)
		{
		#
		# Count the number of global comments
		#
		global_comments++
		}
	#
	# If it is a local comment
	#
	else if ($1 ~/\!/)
		{
		#
		# Count the number of local comments
		#
		local_comments++
		#
		# Local comments may not appear before the first
		# exclusive interpretation record
		#
		if (no_interpretation_yet) print ERROR[9] FNR ", file " FILENAME "."
		#
		# There must be one and only one local comment for each spine
		#
		else if (NF != current_no_of_spines)
			print ERROR[10] FNR ", file " FILENAME "."
		#
		# Determine if each field starts with an exclamation mark and
		# contains only local comments
		#
		for (i = 1; i <= current_no_of_spines; i++)
			{
			if ($i !~/^\!/) print ERROR[11] FNR ", file " FILENAME "."
			if (gsub("!","!",$i) > 1)
				print WARNING[1] FNR ", file " FILENAME "."
			}
		}
	}
#
# Else, if the current record is a data record
#
else
	{
	#
	# Count the number of data records
	#
	number_of_data_records++
	#
	# No data records may appear before the first exclusive 
	# interpretation record (Set 'current_no_of_spines' to avoid more errors)
	#
	if (no_interpretation_yet)
		{
		print ERROR[12] FNR ", file " FILENAME "."
		current_no_of_spines = NF
		}
	#
	# Number of data tokens must equal the number of currently active spines
	#
	if (NF != current_no_of_spines) print ERROR[13] FNR ", file " FILENAME "."
	#
	# Check to see if any spine contains a 'local comment', 'global comment',
	# of 'interpretation token'.  If so, print a warning.
	#
	for (j = 2; j <= current_no_of_spines; j += 1)
		{
		if ($j ~ /^!!/) print WARNING[2] FNR ", file " FILENAME "."
		else if ($j ~ /^!/) print WARNING[3] FNR ", file " FILENAME "."
		else if ($j ~ /^\*\*/) print WARNING[4] FNR ", file " FILENAME "."
		else if ($j ~ /^\*/) print WARNING[5] FNR ", file " FILENAME "."
		}
	#
	# If the user specified the verbose option, count all signifiers,
	# and count the number of null tokens.
	#
	if (VERBOSE)
		{
		# SIGNIFIER INVENTORY
		#
		# Maintain an inventory of all signifiers used in the data records.
		#
		# Upper- and lower-case alphabetic, numeric, and special characters
		# are treated separately in order to enhance speed.
		#
		# Numeric characters (0-9).
		for (j in NUM)
			{
			if (index($0,NUM[j])) {signifier[j]=NUM[j]; delete NUM[j]}
			}
		#
		# Upper-case alphabetic characters (A-Z).
		for (j in UPPER)
			{
			if (index($0,UPPER[j])) {signifier[j]=UPPER[j]; delete UPPER[j]}
			}
		#
		# Lower-case alphabetic characters (a-z).
		for (j in LOWER)
			{
			if (index($0,LOWER[j])) {signifier[j]=LOWER[j]; delete LOWER[j]}
			}
		#
		# Other special ASCII characters (e.g. !#%*&:;, etc.)
		for (j in ASCII)
			{
			if (index($0,ASCII[j])) {signifier[j]=ASCII[j]; delete ASCII[j]}
			}
		#
		# Finally, deal with some problem characters (meta characters)
		if ($0 ~ /\(/)	signifier[88] = "("
		if ($0 ~ /\)/)	signifier[89] = ")"
		if ($0 ~ /\[/)	signifier[90] = "["
		if ($0 ~ /\]/)	signifier[91] = "]"
		if ($0 ~ /\|/) signifier[92] = "|"
		if ($0 ~ /\//)	signifier[93] = "/"
		if ($0 ~ /\\/) signifier[94] = "\\"
		if ($0 ~ /\^/)	signifier[95] = "^"
		if ($0 ~ /\$/)	signifier[96] = "$"
		if ($0 ~ /\./)	signifier[97] = "."
		#
		# Also count the number of null tokens
		for (i=1; i<=NF; i++) if ($i == ".") number_of_null_tokens++
		}
	}
}
END	{
	#
	# If the spines were not properly terminated, print an error
	#
	if (current_no_of_spines != 0) print ERROR[14] FNR ", file " FILENAME "."
	#
	# If the verbose option is specified then print
	# the following humdrum file summary -- else exit:
	#
	if (!VERBOSE) exit
	#
	print "Inventory of Interpretations:                              " \
		 "HUMDRUM FILE SUMMARY"
	print ""
	print "\tInterpretations\tOccurrences"
	#
	for (i in ex_interp_inventory)
		{
		printf("\t    %-15s%3s\n",i,ex_interp_inventory[i])
		}
	for (i in tand_interp_inventory)
		{
		printf("\t    %-15s%3s\n",i,tand_interp_inventory[i])
		}
	printf("\nInventory of data token signifiers:")
	#
	inventory = ""
	signifier[27] = "\n\t\t\t\t\t"
	signifier[54] = "\n\t\t\t\t\t"
	signifier[65] = "\n\t\t\t\t\t"
	for (i = 1; i <= 97; i++)
		{
		if (length(signifier[i]) == 0) signifier[i] = " "
		inventory = inventory signifier[i]
		}
	gsub(/( )+/,"",inventory)
	print "\t" inventory "\n"
	#
	print "Number of global comments:       " global_comments+0   \
	"	Number of local comments:        " local_comments+0
	print "Maximum # of concurrent spines:  " maximum_no_of_spines+0   \
	"	Minimum # of concurrent spines:  " minimum_no_of_spines+0
	print "Number of data records:          " number_of_data_records+0   \
	"	Number of null tokens:           " number_of_null_tokens+0
	#
	# Print information on spine path indicators.
	#
	print "\nChanges in the number of spines:"
	print "                     New spines: " added_spines+0
	print "              Terminated spines: " terminated_spines+0
	print "                   Split spines: " split_spines+0
	print "                  Joined spines: " joined_spines+0
	print "               Exchanged spines: " exchanged_spines+0
	}

##################################################################
#				FUNCTIONS FOR HUMDRUM.AWK
##################################################################

#################################################################
##				Function Parse_command
#	This function parses the commands passed from humdrum.ksh
# and sets the verbose variable if necessary.
#
function parse_command()
	{
	if (option ~ OPTIONS_NEG_REGEXP)
		{
		gsub(OPTIONS_REGEXP,"",option)
		print "ERROR: Unknown option(s): " option
		print USAGE
		exit
		}
	else if (option ~ OPTIONS_REGEXP)
		{
		#
		# Otherwise, set the 'options' variable
		#
		VERBOSE = TRUE
		}
	}

##################################################################
##			Function Chk_first_interp
#	This function checks whether or not the first interpretation
# record meets the specified guidelines for that record
#
function chk_first_interp(   i)
	{
	for (i = 1; i <= NF; i++)
		{
		#
		# This record cannot contain a null interpretation 
		#
		if ($i == "*") print ERROR[15] FNR ", file " FILENAME "."
		#
		# This record cannot contain a spine-path indicator
		#
		if ($i ~ /^(\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x)$/)
			print ERROR[16] FNR ", file " FILENAME "."
		#
		# This record cannot contain something other than an
		# exclusive interpretation
		#	
		if ($i !~ /^\*\*/) print ERROR[17] FNR ", file " FILENAME "."
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
		# If the current field contains something other than a 
		# spine-path indicator or null interpretation, print error
		# (Do not repeat 'incorrect number of spines' error)
		#
		if ($i !~ /^(\*[\+\^vx-]?)$/ && $i != "")
			{
			print ERROR[18] FNR ", file " FILENAME "."
			$i = "*"
			}
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
#	The local variable 'join' indicates if a join-path is currently
# being performed.  The variable 'exchanges' holds the number of
# exchange-path indicators that were found.
#
function process_indicators(    i,exchanges,join)
	{
	join = FALSE
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
			split_spines++
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
			terminated_spines++
			}
		#
		# If a spine-exchange indicator occurred
		#
		if (path_indicator[i] == "x")
			{
			spines_to_exchange = ""
			#
			# Determine the number of exchange indicators that
			# occur and store them in 'spines_to_exchange'
			#
			exchanges = num_of_exchanges()
			#
			# If there were exactly two exchange indicators
			#
			if (exchanges == 2)
				{
				#
				# Exchange the paths
				#
				exchange_spines()
				exchanged_spines+=2
				}
			#
			# If there were not 2 or 0 exchange indicators, print error
			#
			else if (exchanges != 0)
				print ERROR[19] FNR ", file " FILENAME "."
			}
		#
		# If a join-path indicator occurred
		#
		if (path_indicator[i] == "v")
			{
			#
			# If this is the first join-indicator in a group of joins
			#
			if (!join)
				{
				join = TRUE
				joined_spines++
				#
				# If this is the last indicator in the record, then
				# print an error
				#
				if (i+1 > current_no_of_spines)
					{
					print ERROR[20] FNR ", file " FILENAME "."
					path_indicator[i] = "*"
					join = FALSE ; i++ ; continue
					}
				#
				# If the next indicator is not also a join-path 
				# indicator, print an error
				#	
				if (path_indicator[i+1] != "v")
					{
					print ERROR[21] FNR ", file " FILENAME "."
					path_indicator[i] = "*"
					join = FALSE ; i++ ; continue
					}
				}
			#
			# If the two spines have interpretations allowing
			# them to join, then join them.
			#
			if (join_check(i,i+1))
				{
				del_array_pos(i+1)
				joined_spines++
				#
				# If the next indicator is not a join-path indicator
				# then return to original conditions
				#
				if (path_indicator[i+1] != "v")
					{
					path_indicator[i] = "*"
					join = FALSE
					}
				#
				# Otherwise, look at current and next paths to join
				#
				else i--
				}
			#
			# If the spines cannot be joined
			#
			else
				{
				path_indicator[i] = "*"
				path_indicator[i+1] = "*"
				join = FALSE
				}
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
			path_indicator[i+1] = "NEW"
			current_interp[i+1] = ""
			new_path = TRUE
			added_spines++
			}
		i++
		} #end while loop
	} #end function process_indicators

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
# elements in 'current_interp'
#
function exchange_spines(    temp_array,temp)
	{
	split(spines_to_exchange,temp_array)
	temp = current_interp[temp_array[1]]
	current_interp[temp_array[1]] = current_interp[temp_array[2]]
	current_interp[temp_array[2]] = temp
	}

#######################################################################
##				Function Join_check
#	This function determines whether or not two spines may be joined
# without violating exclusive interpretations.
#
function join_check(first_spine,second_spine,    j)
	{
	#
	# If the exclusive interpretations are not identical, they
	# cannot be joined together
	#
	if (current_interp[first_spine] != current_interp[second_spine])
		{
		print ERROR[22] FNR ", file " FILENAME "."
		return FALSE
		}
	else return TRUE 
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
		#
		# If the current field is an exclusive interpretation,
		# then it replaces the old exclusive interpretation
		#
		if ($j ~ /^\*\*/) current_interp[j] = $j
		}
	#
	# Update the inventory of interpretations
	#
	if ($j ~ /^\*\*/) ex_interp_inventory[$j]++
	else tand_interp_inventory[$j]++
	}

#####################################################################
#			Helper Function Print_interps
function print_interps(        j)
	{
	print "Array Current_interp"
	for(j in current_interp)
		print j, current_interp[j] 
	print "End of array Current_interp"
	}

#####################################################################
#			Helper Function Print_indics
function print_indics(        j)
	{
	print "Array Path_indicator"
	for(j in path_indicator)
		print j, path_indicator[j] 
	print "End of array Path_indicator"
	}
