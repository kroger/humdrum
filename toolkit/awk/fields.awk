#####################################################################
##					FIELDS.AWK
# Programmed by: Tim Racinsky         Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#    Date:   Programmer:         	Description:
# May 9/94	Tim Racinsky		Changed -u option to -r
# June 7/94	Tim Racinsky		Modified parse_command for getopts command
#
#
#	This program simply allows the user to obtain a listing of record
# numbers and the corresponding number of fields for that record for
# types of records that the user specifies on the command line.  Each
# line of input is also appended as a comment to each line of output. This
# program is called using 'fields.ksh'.
#	This program will usually be used in conjuntion with the extract2.awk.
#
#	The following gives a brief summary of the available options:
#
#	-g	signify global comments
#	-l	signify local comments
#	-s	sifnify spine-path interpretations only
#	-i	signify all interpretations
#	-r [regexpr] allow user to specify marker
#
#	For global comments, the number of fields output for the current
# line is equal to the number of fields for the most recent non-global
# comment record (held in variable current_no_of_spines).
#	For spine-path records, the current record as well as the next
# record are output so that the user knows the changes in the number of spines. 
#	This program assumes a valid humdrum file as input.
#
# Main Functions used
#	Parse_command()	Store_indiators()		Process_indicators()
#	Determine_print()
#
#	VARIABLES:
#
#  -spine_path_record:  flag to indicate if current interpretation record
#   is also a spine-path record.
#  -USER:  flag to determine if user has specified '-r' option.
#  -REGEXP:  holds regular expression if user specified '-r' option.
#  -options:  holds list of options that user has specified.
#  -print_next:  flag to determine if current record or next non-global
#   comment record should be printed (used for spine-path records).
#  -current_no_of_spines:
#
# (Note that it is possible for one line to be printed more than once!)?????
#
BEGIN {
	USAGE="\nUSAGE: fields -h               (Help Screen)\n       fields "\
		 "[-gils] [-r regexpr] [file ...]"
	TRUE = 1; FALSE = 0
	FS = OFS = "\t"
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	USER = FALSE
	REGEXP = ""
	options = ""
	#
	# Determine the options specified by the user
	#
	parse_command()
	}
{
#
# If this is the first record of the current file, print the line
# and reset the variables.
#
if (FNR == 1)
	{
	#
	# The output always includes the first line.
	#
	if ($0 ~ /^!!/)
		{
		print FNR "\t1-1\t# Line 1 must appear in the file."
		current_no_of_spines = 1
		}
	else
		{
		print FNR "\t1-" NF "\t# Line 1 must appear in the file."
		current_no_of_spines = NF
		}
	print_next = FALSE
	spine_path_record = FALSE
	next
	}
#
# If currently looking at a record and it follows a spine-path
# record (and spine-path records are being extracted)
#
if (print_next) determine_print()
#
# This checks for global comments.
#
if ($0 ~ /^!!/)
	{
	if (options ~ /g/)
		{
		print FNR "\t1-" current_no_of_spines "\t# " substr($0,1,10) " ...."
		next
		}
	}
#
# This checks for local comments.
#
else if ($0 ~ /^!/ && options ~ /l/)
	{
	print FNR "\t1-" current_no_of_spines "\t# " substr($0,1,10) " ...."
	next
	}
#
# This checks for interpretation records.
#
if ($0 ~ /^\*/)
	{
	current_no_of_spines = NF
	spine_path_record = FALSE
	for (i = 1; i <= current_no_of_spines; i++)
		{
		if ($i ~ /^\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x$/)
			{
			spine_path_record = TRUE
			break
			}
		}
	#
	# If it is a spine_path_record and the user wants to print it, then
	# print the number of fields of the NEXT line (if it is not a global
	# comment) as well as the current one.
	#
	if (spine_path_record)
		{
		if (options ~ /[is]/)
			{
			print FNR "\t1-" current_no_of_spines "\t# " substr($0,1,10) \
				 " ...."
			print_next = TRUE
			}
		store_indicators()
		process_indicators()
		if (current_no_of_spines == 0)
			{
			current_no_of_spines = 1
			print_next = FALSE
			}
		if (options ~ /[is]/) next
		}
	#
	# Otherwise just print out the record if it was specified.
	#
	else
		{
		if (options ~ /i/)
			{
			print FNR "\t1-" current_no_of_spines "\t# " substr($0,1,10) \
				 " ...."
			next
			}
		}
	}
#
# If the user has specified a 'custom' regular expression, check for it
# and print the line if it exists.
#
if (USER && $0 ~ REGEXP)
	print FNR "\t1-" current_no_of_spines "\t# " substr($0,1,10) " ...."
}

############################################################################
#				FUNCTIONS USED IN FIELDS.AWK
############################################################################

############################################################################
##					Function Parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	if (ARGV[2] != "null") options = ARGV[2]
	if (ARGV[3] != "null")
		{
		USER = TRUE
		REGEXP = ARGV[3]
		}
	ARGV[1] = ARGV[2] = ARGV[3] = ""
	}

##############################################################
##			Function Store_indicators
#	This function allows the spine-path indicators for the 
# current record to be stored in the array 'path_indicator' so
# that they may be used by process_indicators.
#
function store_indicators(   i)
	{
	for (i = 1; i <= current_no_of_spines; i++)
		{
		if ($i == "*") path_indicator[i] = "*"
		else path_indicator[i] = substr($i,2,1)
		}
	}

##################################################################
##			Function Process_indicators
#	This function simply alters the variable current_no_of_spines
# according to the spine path indicators which are present
#
function process_indicators(    i,temp_spines)
	{
	temp_spines = current_no_of_spines
	i = 1
	while (i <= temp_spines)
		{
		if (path_indicator[i] == "x") ;
		else if (path_indicator[i] == "^") current_no_of_spines += 1
		else if (path_indicator[i] == "+") current_no_of_spines += 1
		else if (path_indicator[i] == "-") current_no_of_spines -= 1
		else if (path_indicator[i] == "v")
			{
			i += 1
			while (path_indicator[i] == "v")
				{
				current_no_of_spines -= 1
				i += 1
				}
			i -= 1
			}
		i += 1
		}
	}

############################################################################
##				Function Determine_print()
#	This function determines whether or not a record following a spine path
# record should be printed or not depending on the current options
#
function determine_print(      i,spine_path_record)
	{
	#
	# Determine if it is another spine path record
	#
	spine_path_record = FALSE
	for (i = 1; i <= NF; i++)
		{
		if ($i ~ /^\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x$/)
			{
			spine_path_record = TRUE
			break
			}
		}
	#
	# If it is global comment and global comments are not being printed
	#
	if ($0 ~ /^!!/)
		{
		if(options !~ /g/)
			{
			print FNR "\t1-" current_no_of_spines "\t# "\
				 "Line after path indicator record"
			}
		}
	#
	# If it is a local comment and local comments are not being printed
	#
	else if ($0 ~ /^![^!]/)
		{
		if (options !~ /l/)
			{
			print FNR "\t1-" current_no_of_spines "\t# "\
				 "Line after path indicator record"
			}
		}
	#
	# If it is an interpretation record
	#
	else if ($0 ~ /^\*/)
		{
		#
		# If it is not a spine path record and interpretations are not
		# being printed
		#
		if (!spine_path_record)
			{
			if (options !~ /i/)
				{
				print FNR "\t1-" current_no_of_spines "\t# "\
					 "Line after path indicator record"
				}
			}
		}
	#
	# If the user is matching regular expressions and this line doesn't
	# match it, print it
	#
	else if (USER)
		{
		if ($0 !~ REGEXP)
			{
			print FNR "\t1-" current_no_of_spines "\t# "\
				 "Line after path indicator record"
			}
		}
	else
		{
		print FNR "\t1-" current_no_of_spines "\t# Line after path "\
			 "indicator record"
		}
	print_next = FALSE
	}
