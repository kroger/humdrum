
#########################################################################
##					NUMBER.AWK
# Programmed by: Tim Racinsky        Date: May, 1994
# Copyright (c) 1994 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
#
#	This program is used by yank5.awk to printout the number
# of occurrences of a particular section label.
#
#	VARIABLES:
#
BEGIN { 
	FS = OFS = "\t"
	label = ARGV[1] 
	ARGV[1] = ""
	occurrences = 0
	}
{
if ($0 ~ /^\*/ && $0 ~ label) occurrences += 1
}
END {
	print occurrences
}
