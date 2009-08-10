######################################################################
##					INFOT.AWK
# Programmed by: Tim Racinsky        Date: July, 1994
# Copyright (c) 1994 David Huron
#
# Modifications:
#   Date:      Programmer:       	Description:
#
# Main Functions used:
#	parse_command()
#
#	VARIABLES:
#
#  -stderr:  holds destination of error messages
#  -data_found:  flag to indicate if any data records were found
#  -options:  holds options specified by the user
#  -command:  holds command (b,n,p, or s) specified by user
#  -skip_reg:  holds regular expression to skip data records for -x option
#  -file1:  holds name of first temporary file
#  -file2:  holds name of second temporary file
#
#	ARRAYS:
#
#  -data_array:  holds count of each data token to be processed.  The data
#			  tokens themselves are the indices.
#  -prob_array:  similar to data_array except holds probability of data token
#			  occurring in the input.
#
BEGIN {
	FS = OFS = "\t"
	TRUE = 1 ; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	# Other global variables
	#
	data_found = FALSE
	options = command = skip_reg = ""
	#
	# Determine user's specified options
	#
	parse_command()
	#
	# Open a temporary file
	#
	"echo $TMPDIR/infotok" | getline file1
	close("echo $TMPDIR/infotok")
	}
{
#
# All non-global comment records must have one spine only.
#
if ($0 !~ /^!!/ && NF > 1)
	{
	print "infot: ERROR: Only one spine of input is permitted." > stderr
	data_found = FALSE
	exit
	}
#
# If this is a data record and the user does not want to skip it, place
# it in the temporary file that was created in the BEGIN block
#
else if (($0 !~ /^[!*]/) && (options !~ /x/ || $0 !~ skip_reg))
	{
	data_found = TRUE
	print $0 > file1
	}
}	
END	{
	#
	# If there were no data records found, exit the program
	#
	if (!data_found) exit
	close(file1)
	#
	# Create a second temporary file and then sort and unique the original
	# temporary file, storing the contents in the second temporary file.
	#
	"echo $TMPDIR/infosort" | getline file2
	close("echo $TMPDIR/infosort")
	system("sort " file1 " | uniq -c > " file2)
	#
	# Reset the field separator to a space and then read the temporary
	# file into the array data_array
	#
	FS = " "
	data_count = 0
	while ((getline < file2) > 0)
		{
		data_count += 1
		token = ""
		#
		# Be sure to include multiple stops
		#
		for (i = 2; i <= NF; i += 1) token = token $i " "
		token = substr(token,1,length(token)-1)
		data_array[token] = $1
		}
	#
	# Close the files and compute the information calculations
	#
	close(file1)
	close(file2)
	total = 0
	average_information = 0
	for (i in data_array) total += data_array[i]
	for (i in data_array) prob_array[i] = data_array[i]/total
	for (i in prob_array)
		average_information += prob_array[i]*log(1/prob_array[i])/log(2)
	Hmax = log(data_count)/log(2)
	#
	# If the n option was selected, ouput the counts of each token
	#
	if (command == "n")
		{
		for (i in data_array)
			{
			if (options ~ /H/)
				{
				printf("s/^%s$/%d/",i,data_array[i])
				printf(";s/^%s\t/%d\t/",i,data_array[i])
				printf(";s/\t%s\t/\t%d\t/",i,data_array[i])
				printf(";s/\t%s$/\t%d/\n",i,data_array[i])
				}
			else
				print i "\t" data_array[i]
			}
		}
	#
	# If the -p option was selected, output the probability of each token
	#
	else if (command == "p")
		{
		for (i in prob_array)
			{
			if (options ~ /H/)
				{
				printf("s/^%s$/%.3f/",i,prob_array[i])
				printf(";s/^%s\t/%.3f\t/",i,prob_array[i])
				printf(";s/\t%s\t/\t%.3f\t/",i,prob_array[i])
				printf(";s/\t%s$/\t%.3f/\n",i,prob_array[i])
				}
			else printf("%s\t%.3f\n",i,prob_array[i])
			}
		}
	#
	# If the -b option was selected, output the information in bits of tokens
	#
	else if (command == "b")
		{
		for (i in prob_array)
			{
			bits = log(1/prob_array[i])/log(2)
			if (options ~ /H/)
				{
				printf("s/^%s$/%.3f/",i,bits)
				printf(";s/^%s\t/%.3f\t/",i,bits)
				printf(";s/\t%s\t/\t%.3f\t/",i,bits)
				printf(";s/\t%s$/\t%.3f/\n",i,bits)
				}
			else
				printf("%s\t%.3f\n",i,bits)
			}
		}
	#
	# If the -s option was selected, output a summary screen.
	#
	else if (command == "s")
		{
		if (Hmax == 0) redundancy = 100
		else redundancy = (Hmax - average_information)/ Hmax * 100.0
		print "Total number of input states in message:	" data_count
		print "Total information of message  (in bits):	" \
		    average_information * total
		print "Total possible  information for message:	" \
		    Hmax * total
		print "Info  per state  for equi-prob  distrib:	" Hmax
		print "Average information conveyed  per state:	" \
		    average_information 
		print "Percent redundancy  evident  in message:	" \
		    redundancy
		}
	system("rm " file1)
	system("rm " file2)
	}

##################################################################
##			FUNCTIONS USED IN INFOT.AWK
##################################################################

##################################################################
##			Function Parse_command
#
function parse_command()
	{
	command = ARGV[2]
	if (ARGV[3] != "null") options = ARGV[3]
	if (ARGV[4] != "null")
		{
		skip_reg = ARGV[4]
		options = options "x"
		}
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ""
	}
