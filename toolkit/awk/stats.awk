#########################################################################
##                                 STATS
#
# Calculate the total, mean, and standard deviation for a single
# column of numerical input.
#
{
if ($1 ~/[^-0-9.]/)
	{
	print "stats: ERROR: Non-numeric input in line " NR "."
	exit
	}
array[NR] = $1
}
END	{
	for (i=1; i<=NR; i++) total += array[i]
	if (NR != 0) mean = total / NR
	else
		{
		print "stats: ERROR: No input."
		exit
		}
	for (i=1; i<=NR; i++)
		{
		sum_of_squares += (abs(mean - array[i])) ^ 2
		}
	print "n:	" NR
	print "total:	" total
	print "mean:	" mean
	print "S.D.:	"  sqrt(sum_of_squares)
	}
function abs(value)
	{
	if (value >= 0) return value
	else return 0 - value
	}
