#####################################################################
##					THRU.AWK
# Programmed by: Tim Racinsky         Date: May, 1994
# Copyright (c) 1994 David Huron
#
# Modifications:
#    Date:   Programmer:         	Description:
# June 7/94	Tim Racinsky	Modified parse_command to work with getopts
# Dec 12/96	David Huron	Fixed bug with wrong spine terminators after comment.
#
# This program prints out a humdrum file according to section labels and
# an expansion-list interpretation.
#
# Main Functions used
#	parse_command()		process_comments()		process_interpretations()
#	process_data()			process_exclusive()		process_tandem()
#	process_spine_path()	process_section_label()	process_expansion_list()
#	process_thru_record()	print_and_reset()
#
#	VARIABLES:
#
#  -null_interp:  Regular expression for null interpretation record.
#  -exclusive_record:  Regular expression for exclusive interpretation record.
#  -totally_exclusive:  Regular expression for exclusive interpretation record
#   that has an exclusive interpretation in every spine.
#  -tandem_interp:  Regular expression for tandem interpretation name.
#  -tandem_record:  Regular expression for tandem interpretation record.
#  -spine_terminate:  Regular expression for spine-terminating record.
#  -section_label_name:  Regular expression for section label name.
#  -expansion_list_name:  Regular expression for expansion-list name.
#  -thru_record:  Regular expression for *thru interpretation.
#  -options:  holds options specified by the user.
#  -version:  holds version to 'thru' if specified by user.
#  -expansion_list_found:  flag to indicate if the expansion-list has
#					  been found yet.
#  -storing:  flag to indicate if currently storing records in a file.
#  -section_count:  increments each time a new section is stored in a file.
#  -output_file:  holds name of output file (for re-direction).
#  -error:  flag to indicate if an error has occurred.
#  -no_of_sections:  holds number of sections specified in expansion-list.
#  -num_of_spines:  holds number of active spines in the printed output.
#  -first_file:  flag to indicate if first file has been processed yet.
#  -last_file:  holds name of file last processed (or currently if FNR != 1)
#
#	ARRAYS:
#
#  -files:  holds the file name for each corresponding section label.
#  -section_progression:  list of sections to be output (in order).
#
BEGIN {
	USAGE="\nUSAGE: thru -h               (Help Screen)\n       thru "\
		 "[-v version] [file ...]"
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
	totally_exclusive = "^\\*\\*([^\t])+(\t\\*\\*([^\t])+)*$"
	tandem_interp = "([vx+^-][^\t]+)|([^*vx+\t^-][^\t]*)"
	tandem_record = "^\\*(" tandem_interp ")?(\t\\*(" tandem_interp ")?)*$"
	spine_terminate = "^\\*-(\t\\*-)*$"
	s_label = "[[:alnum:] _+.#@~-]"
	section_label_name = "(^|\t)\\*>" s_label "+($|\t)"
	expansion_list_name="(^|\t)\\*>" s_label "*\\[" s_label "+(," s_label "+)*\\](\t|$)"
	thru_record = "(^|\t)\\*thru(\t|$)"
	#
	# Set other global variables
	#
	error = FALSE
	expansion_list_found = FALSE
	storing = FALSE
	first_file = TRUE
	options = ""
	version = ""
	last_file = ""
	section_count = 1
	no_of_sections = 0
	#
	# Determine the options specified by the user
	#
	parse_command()
	}
{
#
# If this is the first record finish the last file and re-set variables
#
if (FNR == 1)
	{
	if (first_file)
		{
		first_file = FALSE
		last_file = FILENAME
		}
	else print_and_reset()
	}
#
# Process comments, interpretations, and data depending on the first character
#
if ($0 ~ /^!/) process_comments()
else if ($0 ~ /^\*/) process_interpretations()
else process_data()
}
#
# Print out the sections now
#
END	{
#
# If an error has occurred, simply exit
#
if (error)
	exit
else
	{
	#
	# Loop through the sections listed in the expansion-list
	#
	for (i = 1; i <= no_of_sections; i += 1)
		{
		#
		# If the section was not in the input print an error, otherwise, print
		# the section stored in the file
		#
		if (!(section_progression[i] in files))
			{
			print "thru: ERROR: Section " section_progression[i] " not "\
				 "found in input for file " FILENAME > stderr
			error = TRUE
			exit
			}
		else
			{
			while (getline < files[section_progression[i]] > 0)
				{
				print $0
				# Keep track of the number of active spines
				# so that a proper spine-path terminator
				# can be printed.
				if ($0 !~ /^[\*!]/) num_of_spines = NF
				}
			close(files[section_progression[i]])
			}
		}
	#
	# Print out a terminating record
	#
	if (no_of_sections > 0)
		{
		output_line = "*-"
		for (i = 2; i <= num_of_spines; i += 1) output_line = output_line "\t*-"
		print output_line
		}
	#
	# Close and remove any files which may still exist
	#
	for (i in files)
		{
		close(files[i])
		system("rm " files[i])
		}
	}
}

############################################################################
#				FUNCTIONS USED IN THRU.AWK
############################################################################

############################################################################
##				Function parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	#
	# Otherwise, process any necessary variables
	#
	if (ARGV[2] != "null")
		{
		options = "v"
		version = ARGV[2]
		}
	ARGV[1] = ARGV[2] = ""
	}

##############################################################################
##					Function process_comments
#	This function processes all global and local comments
#
function process_comments()
	{
	if (!expansion_list_found) print $0
	if (storing) print $0 >> output_file
	}

##############################################################################
##					Function process_interpretations
#	This function processes all interpretation records
#
function process_interpretations()
	{
	if ($0 ~ exclusive_record && $0 !~ null_interp) process_exclusive()
	else if ($0 ~ tandem_interp && $0 !~ null_interp) process_tandem()
	else process_spine_path()
	}

##############################################################################
##					Function process_exclusive
#	This function processes all exclusive interpretation records
#
function process_exclusive(    i,thru_line)
	{
	thru_line = ""
	#
	# Create a record showing the *thru interpretation
	#
	for (i = 1; i <= NF; i += 1)
		{
		if ($i ~ /^\*\*/) thru_line = thru_line "*thru\t"
		else thru_line = thru_line "*\t"
		}
	thru_line = substr(thru_line,1,length(thru_line)-1)
	#
	# If the expansion-list has not been found, print the records
	#
	if (!expansion_list_found)
		{
		print $0
		print thru_line
		}
	#
	# If currently storing records and all spines have exclusive
	# interpretations, stop storing.  Otherwise, print the records
	#
	if (storing)
		{
		if ($0 ~ totally_exclusive)
			{
			close(output_file)
			storing = FALSE
			}
		else
			{
			print $0 >> output_file
			print thru_line >> output_file
			}
		}
	}

##############################################################################
##					Function process_tandem
#	This function processes all tandem interpretation records
#
function process_tandem()
	{
	if ($0 ~ section_label_name) process_section_label()
	else if ($0 ~ expansion_list_name) process_expansion_list()
	else if ($0 ~ thru_record) process_thru_record()
	else
		{
		if (!expansion_list_found) print $0
		if (storing) print $0 >> output_file
		}
	}

#############################################################################
##					Function process_section_label
#	This function processes records that contain section labels
#
function process_section_label(   good_record,label)
	{
	if (!expansion_list_found) print $0
	good_record = TRUE
	#
	# Check that all spines have the same section label
	#
	for (i = 2; i <= NF; i += 1)
		{
		if ($i != $1) { good_record = FALSE; break }
		}
	#
	# If all spines have the same label
	#
	if (good_record)
		{
		#
		# Stop storing if currently storing a section
		#
		if (storing)
			{
			storing = FALSE
			close(output_file)
			}
		label = substr($1,3)
		#
		# If this label has not yet been seen yet, begin storing it in a file
		#
		if (!(label in files))
			{
			#"echo $TMPDIR/thru" section_count | getline output_file
			#close("echo $TMPDIR/thru" section_count)
			cmd = "echo $TMPDIR/thru-$$-" section_count
			cmd | getline output_file
			close(cmd)

			files[label] = output_file
			section_count += 1
			storing = TRUE
			print $0 > output_file
			}
		}
	#
	# Print an error if it is an invalid section label record
	#
	else
		{
		print "thru: ERROR: Invalid section label record at line " \
			 FNR " of file " FILENAME > stderr
		error = TRUE
		exit
		}
	}

#############################################################################
##					Function process_expansion_list
#	This function processes records that contain expansion-lists
#
function process_expansion_list(   label,good_record,list)
	{
	good_record = TRUE
	label = ""
	#
	# Check that all spines have the same expansion-list interpretation
	#
	for (i = 2; i <= NF; i += 1)
		{
		if ($i != $1) { good_record = FALSE; break }
		}
	#
	# If all spines have the same expansion-list interpretation
	#
	if (good_record)
		{
		#
		# Check if there is a 'version' with the expansion-list
		#
		if (match($1,"^\\*>" s_label "+\\[") > 0)
			label = substr($1,RSTART+2,RLENGTH-3)
		#
		# If this is the correct version or new version was specified
		#
		if ((options ~ /v/ && label == version) \
								|| (options !~ /v/ && label == ""))
			{
			#
			# If this is not the first expansion-list for this version, error
			#
			if (expansion_list_found)
				{
				print "thru: ERROR: Repeated expansion-list at line " \
					 FNR " of file " FILENAME > stderr
				error = TRUE
				exit
				}
			#
			# Otherwise, store the sections to be output in the array
			# section_progression
			#
			else
				{
				expansion_list_found = TRUE
				match($1,"\\[" s_label "+(," s_label "+)*\\]")
				list = substr($1,RSTART+1,RLENGTH-2)
				no_of_sections = split(list,section_progression,",")
				}
			}
		}
	#
	# If not a valid record, print an error
	#
	else
		{
		print "thru: ERROR: Invalid expansion-list record at line " \
			 FNR " of file " FILENAME > stderr
		error = TRUE
		exit
		}
	}

############################################################################
##					Function process_thru_record
#	This record processes those records that already contain a thru
# tandem interpretation.
#
function process_thru_record(    i)
	{
	#
	# Delete any instances of *thru found in the input
	#
	for (i = 1; i <= NF; i += 1)
		{
		if ($i == "*thru") $i = "*"
		}
	if ($0 !~ null_interp)
		{
		if (!expansion_list_found) print $0
		if (storing) print $0 >> output_file
		}
	}

##############################################################################
##					Function process_spine_path
#	This function processes all spine_path interpretation records (and null
# interpretation records).
#
function process_spine_path()
	{
	if (!expansion_list_found) print $0
	#
	# If currently storing records and this is a spine-terminating record
	# (i.e. all spines terminate), then stop storing
	#
	if (storing)
		{
		if ($0 ~ spine_terminate)
			{
			storing = FALSE
			close(output_file)
			}
		else print $0 >> output_file
		}
	}

#############################################################################
##					Function process_data
#	This function processes data records
#
function process_data()
	{
	if (!expansion_list_found) print $0
	if (storing) print $0 >> output_file
	}

############################################################################
##				Function print_and_reset
#	This function prints the sections from the previous file and resets
# the variables for the new file.  It is similar to the END section of this
# program except the arrays need to be reset as well.
#
function print_and_reset(    i,temp_line)
	{
	temp_line = $0
	for (i = 1; i <= no_of_sections; i += 1)
		{
		if (!(section_progression[i] in files))
			{
			print "thru: ERROR: Section " section_progression[i] " not "\
				 "found in input for file " last_file > stderr
			error = TRUE
			exit
			}
		else
			{
			while (getline < files[section_progression[i]] > 0) print $0
			close(files[section_progression[i]])
			}
		}
	if (no_of_sections > 0)
		{
		output_line = "*-"
		for (i = 2; i <= NF; i += 1) output_line = output_line "\t*-"
		print output_line
		}
	for (i in files)
		{
		close(files[i])
		system("rm " files[i])
		}
	expansion_list_found = FALSE
	storing = FALSE
	last_file = FILENAME
	section_count = 1
	no_of_sections = 0
	for (i in files) delete files[i]
	for (i in section_progression) delete section_progression[i]
	$0 = temp_line
	}
