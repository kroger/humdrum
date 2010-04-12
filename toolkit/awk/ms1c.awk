#####################################################################
#                            MS1C.AWK
#
# Programmed by: David Huron        Date: January, 1997
# Copyright (c) 1997 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
#
#
# The purpose of this program is to reorder MUP phrase records
# so they are placed prior to the appropriate MUP barline.
# For example, the input record beginning "23XXXphrase" should
# be placed before the barline "23XXXbar".
#
# In addition, this program identifies measures that consist
# solely of rests, and replaces them with the Mup "mr;".
#
{
# Replace bars containing rests only with whole-measure rests ("mr;").
if ($0 ~ /^[0-9][0-9]* [0-9][0-9]*:/ && $0 !~ /[abcdefgs]/) gsub(":.*",": mr;",$0)
# Store all of the input records.
input[NR] = $0
}
END	{
	# Store all of the phrase-mark data in the array "phrase".
	for (i=1; i<=NR; i++)
		{
		if (input[i] ~ /XXXphrase/)
			{
			# For each phrase-mark, store the associated
			# bar number in the "phrase_list".
			split(input[i],array,"XXX")
			phrase_list = phrase_list array[1]
			#print "phrase-list: " phrase_list
			# Store the phrase data.
			if (phrase[array[1]] == "") phrase[array[1]] = array[2]
			else phrase[array[1]] = phrase[array[1]] "/" array[2]
			#print "phrase[" array[1] "]: " phrase[array[1]]
			#print "phrase[1]: " phrase[1]
			}
		}
	# Finally, print the input in correct order.

	for (i=1; i<=NR; i++)
		{
		#if (input[i] ~/XXXbar/)	# ORIGINAL
		if (input[i] ~/XXXbar/ || input[i] ~ /XXXinvisbar/ || input[i] ~ /XXXrepeat/)	# NEW
			{
			#print "PHRASE[" array[1] "]: " phrase["array[1]"]
			# Determine the bar number.
			split(input[i],array,"XXX")
			current_bar = array[1]
			#print "CURRENT BAR IS: " current_bar

			# Print any phrases beginning in the
			# current bar.
			if (phrase_list ~ current_bar)
				{
				#print "CURRENT BAR IS MATCHED IN PHRASE LIST"
				#print "phrase[1] is: " phrase["1"]
				#print "phrase[current_bar] is: " phrase[current_bar]
				output_line = phrase[current_bar]
				if (output_line ~ /\//)
					{
					num = split(output_line,multiphrase,"/")
					for (j=1; j<=num; j++)
						{
						print multiphrase[j]
						}
					}
				else if (output_line != "") print output_line
				}
			# Then print the barline itself.
			gsub("^[0-9]*XXX","",input[i])
			print input[i]
			}
		# Avoid printing the original phrase records.
		else if (input[i] !~ /XXXphrase/) print input[i]
		}
	}
