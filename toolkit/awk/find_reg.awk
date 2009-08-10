##############################################################
##				Find_reg.awk
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
#
#
#	This program is used by yank2.awk.  Its purpose is to locate
# all of the occurrences of the regular expression that the user has
# specified in the input file and to print out the number of data
# records that occur between each occurrence of the regular expression
#
BEGIN {
	#
	# Store the regular expression
	#
	FS = OFS = "\t"
	regular = ARGV[1] 
	ARGV[1] = ""
	num_rec = 0
	}
{
#
# If the current line contains the regular expression, output the number
# of data records that have been counted since the last occurrence of 'regular'
#
if ($0 ~ regular) 
	{
	print num_rec
	num_rec = 0
	}
#
# If the current record does not match, and it is not an interpretation or
# comment record (i.e. a data record) then increment num_rec
#
else	if($0 !~ /^\*/ && $0 !~ /^!/) num_rec++
}
END {
	#
	# Print the number of records occurring after the last matching record
	#
	print num_rec
	}
