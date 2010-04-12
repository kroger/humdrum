###########################################################################
#                        SCRAMBLE.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# June 7/94	Tim Racinsky	Modified to work with getopts
# July 20/94	Tim Racinsky	Added -r, -t, and -m options
# March 23/95	David Huron	Fixed seeding of random numbers.
#
# 	The purpose of this program is to randomize (or scramble) the order of
# data records within a humdrum file.  Only data records are scrambled.
# Interpretation records and comments are output in precisely the same
# location (line no.) as in the input file. The program can also scramble tokens
# within each data record
#
#	Main Functions Used:
#	Parse_command()
#
#	Variables:
#
#   -regexp: holds regular expression specified by user if any.
#   -options: holds options specified by the user.
#   -stderr: for printing errors to standard error.
#   -output_file: holds name of file used to sort the data records.
#   -nondata_length: holds number of records in array nondata.
#   -error: flag to indicate if invalid options were specified
#
#	Arrays:
#
#   -nondata: holds records that will not be sorted in array positions
#    corresponding to their order in the input file.
#
BEGIN {
	USAGE="\nUSAGE: scramble -h                (Help Screen)\n       scramb"\
		 "le -r [-s reg_exp] file\n       scramble -t [-m] [-s reg_exp] file"
	#
	# Set input field-separator to tab only.
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
	regexp = ""
	options = ""
	error = FALSE
	parse_command()
	#
	# Seed the random number generator.
	#
	srand()
	#
	# Create a temporary file for sorting records with -r option
	#
	if (options ~ /r/)
		{
		"echo $TMPDIR/scr001" | getline output_file
		close("echo $TMPDIR/scr001")
		printf("") > output_file
		}
	}
{
if (options ~ /r/)
	{
	#
	# Store comments and interpretations and specified records in 
	# the array "nondata".
	#
	if (($0 ~ /^[!\*]/) || (options ~ /s/ && $0 ~ regexp))
		{
		nondata[NR] = $0
		nondata_length++
		}
	#
	# Store data records in a file (prepend a random number).
	#
	else print rand() "	" $0 >> output_file
	}
else
	{
	#
	# Simply print comments, interpretations, and specified records
	#
	if (($0 ~ /^[!\*]/) || (options ~ /s/ && $0 ~ regexp)) print $0
	else if (options ~ /m/) scramble_all()
	else scramble_tokens()
	}
}
END	{
	if (error) exit
	else if (options ~ /r/)
		{
		close(output_file)
		#
		# Sort the data records according to the prepended random number.
		#
		system("sort -o " output_file " -n " output_file)
		#
		# Read in the sorted file and output data; interleave comments and
		# interpretations from the "nondata" array.
		#
		while (getline < output_file > 0)
			{
			i++
			while (nondata[i] != "")
				{
				print nondata[i]
				i++
				}
			output_line = $2
			for (j = 3; j <= NF; j++)
				output_line = output_line "\t" $j
			print output_line
			}
		#
		# Don't forget trailing nondata records.
		#
		i++
		while (nondata[i] != "")
			{
			print nondata[i]
			i++
			}
		#
		# Delete the temporary file
		#
		system("rm " output_file)
		}
	}

##############################################################################
##				FUNCTIONS IN SCRAMBLE.AWK
##############################################################################

############################################################################
##				Function Parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	options = ARGV[2]
	if (ARGV[3] != "null")
		{
		regexp = ARGV[3]
		options = options "s"
		}
	if (ARGV[4] != "null") options = options "m"
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ""
	}

###########################################################################
##				Function scramble_tokens
#
function scramble_tokens(  i,j,tok_count,sub_count,rec_array,tok_array,string)
	{
	tok_count = split($0,rec_array)
	if (tok_count > 1)
		scramble_array(tok_count,rec_array)
	for (i = 1; i <= tok_count; i += 1)
		{
		for (j in tok_array) delete tok_array[j]
		sub_count = split(rec_array[i],tok_array," ")
		if (sub_count > 1)
			{
			scramble_array(sub_count,tok_array)
			string = tok_array[1]
			for (j = 2; j <= sub_count; j += 1)
				string = string " " tok_array[j]
			rec_array[i] = string
			}
		}
	string = rec_array[1]
	for (i = 2; i <= tok_count; i += 1)
		string = string "\t" rec_array[i]
	print string
	}

###########################################################################
##				Function scramble_array
#
function scramble_array(max_tokens,gen_array,  i,j,rand_array,found,count_down,\
											    next_pos,pos,counter)
	{
	for (i = 1; i <= max_tokens; i += 1) found[i] = FALSE
	count_down = max_tokens
	next_pos = 0
	while (count_down > 0)
		{
		pos = int(rand() * (count_down)) + 1
		counter = 0
		for (i = 1; i <= max_tokens; i += 1)
			{
			if (found[i]) continue
			else counter += 1
			if (counter == pos)
				{
				rand_array[++next_pos] = gen_array[i]
				found[i] = TRUE
				break
				}
			}
		count_down -= 1
		}
	for (i = 1; i <= max_tokens; i += 1) gen_array[i] = rand_array[i]
	}

###########################################################################
##				Function scramble_all
#
function scramble_all(    i,j,tok_count,sub_count,rec_array,tok_array,string)
	{
	tok_count = 0
	for (i = 1; i <= NF; i += 1)
		{
		sub_count = split($i,tok_array," ")
		for (j = 1; j <= sub_count; j += 1)
			{
			rec_array[++tok_count] = tok_array[j]
			}
		}
	spine_scramble(rec_array,tok_count,NF,spine_array)
	string = spine_array[1]
	for (i = 2; i <= NF; i += 1)
		string = string "\t" spine_array[i]
	print string
	}

###########################################################################
##				Function spine_scramble
#
function spine_scramble(rec_array,max_tokens,max_spines,rand_array,  i,j,found,\
					count_down,next_pos,pos,counter)
	{
	for (i = 1; i <= max_tokens; i += 1) found[i] = FALSE
	count_down = max_tokens
	next_pos = 0
	while (count_down > (max_tokens - max_spines))
		{
		pos = int(rand() * (count_down)) + 1
		counter = 0
		for (i = 1; i <= max_tokens; i += 1)
			{
			if (found[i]) continue
			else counter += 1
			if (counter == pos)
				{
				rand_array[++next_pos] = rec_array[i]
				found[i] = TRUE
				break
				}
			}
		count_down -= 1
		}
	for (i in rec_array)
		{
		if (!found[i])
			{
			pos = int(rand() * (max_spines)) + 1
			rand_array[pos] = rand_array[pos] " " rec_array[i]
			found[i] = TRUE
			}
		}
	}
