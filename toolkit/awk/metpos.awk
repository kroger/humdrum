#####################################################################
##                                METPOS.AWK
# Programmed by: David Huron        Date: 1989
# Copyright (c) 1993 David Huron
#
# Modifications:
#    Date:     Programmer:        	Description:
# June 16/94	Tim Racinsky	Modified parse_command to work with getopts
# June 12/94	Tim Racinsky	Changed options and made some general refinements
# 1994 Nov. 27	David Huron	Eliminated bug in process_tandem() function
#
# Functions:
#	parse_command()		process_interpretations()	process_tandem()
#	calculate_metpos()		simple_2()				simple_3()
#	compound_2()			compound_3()				compound_4()
#	process_barlines()		process_data()				print_anacrusis()
#
# Variables:
#
#  -stderr: Determines where error messages will be sent to 
#  -timebase_value:  Regular expression for valid timebase value
#  -timebase_reg:  Regular expression for valid timebase interpretation
#  -meter_value:  Regular expression for valid meter signature value
#  -meter_sig_reg:  Regular expression for valid meter signature interpretation
#  -null_interp:  Regular expression for null interpretation record
#  -spine_terminate:  Regular expression for spine-path terminating record
#  -spine_path_record:  Regular expression for spine-path record
#  -exclusive_record:  Regular expression for exclusive interpretation record
#  -totally_exclusive:  Regular expression for all spines with exclu. interp.
#  -first_bar_yet:  Flag to indicate if first barline has been seen yet
#  -first_interpretation_yet:  Flag to indicate if first interpretation has
#						 been seen yet
#  -tvalue:  Holds timebase value specified on the command line
#  -mvalue:  Holds meter signature value specified on the command line
#  -timebase:  Holds timebase value to be used in processing records
#  -meter:  Holds meter signature value to be used in processing records
#  -number_of_metric_positions:  Holds number of metric positions based on the
#						   given timebase and meter signature
#  -data_count: Holds number of data records in anacrusis material
#  -anacrusis_length: Holds number of total records in anacrusis material
#
# Arrays:
#
#  -metric_position:  Holds values for each metric position based on current
#                     timebase and meter signature
#  -simple_triple:  Holds values for each metric position for simple triple time
#  -compound_duple:  Holds values for each metric position for compound duple
#				 time
#  -compound_triple:  Holds values for each metric position for compound triple
#				  time
#  -compound_quad:  Holds values for each metric position for compound
#				quadruple time
#  -anacrusis:  Holds all records from anacrusis material
#
BEGIN {
	USAGE="USAGE: metpos -h                (Help Screen)\n       metpos [-m"\
		 " n/p] [-t n] [file ...]"
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	# Set global variables
	#
	timebase_value = "^[0-9]+$"
	timebase_reg = "^\\*tb[0-9]+$"
	meter_value = "^[1-9]+/[1-9]+[.]*$"
	meter_sig_reg = "^\\*M(([1-9]+((\\+)([1-9]+))*(\\/)([0-9]+)(\\.)*)|(\\?)|(X))$"
	null_interp = "^\\*(\t\\*)*$"
	spine_terminate = "^\\*-(\t\\*-)*$"
	spine_path_record = "^\\*(\\+|v|\\^|x|-)?(\t\\*(\\+|v|\\^|x|-)?)*$"
	exclusive_record = "^\\*(\\*([^\t])+)?(\t\\*(\\*([^\t])+)?)*$"
	totally_exclusive = "^\\*\\*([^\t])+(\t\\*\\*([^\t])+)*$"
	first_bar_yet = FALSE
	data_count = 0
	anacrusis_length = 0
	first_interpretation_yet = FALSE
	tvalue = mvalue = ""
	#
	# Parse the command line and set meter and timebase accordingly
	#
	parse_command()
	timebase = tvalue
	meter = mvalue
	#
	# If both the meter and timebase were specified on the command line, then
	# determine the metric position array now
	#
	if (timebase != "" && meter != "") calculate_metpos()
	}
{
#
# Process the input records according to their type; if the first bar has
# not yet been encountered, then store the data in anacrusis[]
#
if ($0 ~ /^!!/)
	{
	if (!first_bar_yet) anacrusis[++anacrusis_length] = $0
	else print $0
	}
else if ($0 ~ /^!/)
	{
	if (!first_bar_yet) anacrusis[++anacrusis_length] = $0 "\t!"
	else print $0 "\t!"
	}
else if ($0 ~ /^\*/) process_interpretations()
else if ($1 ~ /\=/) process_barlines()
else process_data()
}
END	{
	if (anacrusis_length > 0) print_anacrusis()
	}

############################################################################
##				Function parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	#
	# Set the meter value if specified on the command line and is valid
	#
	if (ARGV[2] != "null")
		{
		if (ARGV[2] !~ meter_value)
			{
			print "metpos: ERROR: Invalid meter specified: " ARGV[2] > stderr
			exit
			}
		else mvalue = ARGV[2]
		}
	#
	# Set the timebase value if specified on the command line and is valid
	#
	if (ARGV[3] != "null")
		{
		if (ARGV[3] !~ timebase_value)
			{
			print "metpos: ERROR: Invalid timebase specified: " \
				 ARGV[3] > stderr
			exit
			}
		else tvalue = ARGV[3]
		}
	ARGV[1] = ARGV[2] = ARGV[3] = ""
	}

###########################################################################
##				Function process_interpretations
#
function process_interpretations()
	{
	#
	# If all spines contain an exlusive interpretation then output an
	# exclusive interpretation for the metpos spine and reset all of the
	# necessary variables
	#
	if ($0 ~ totally_exclusive)
		{
		if (!first_bar_yet) anacrusis[++anacrusis_length] = $0 "\t**metpos"
		else print $0 "\t**metpos"
		first_interpretation_yet = TRUE
		first_bar_yet = FALSE
		print_anacrusis()
		meter = mvalue
		timebase = tvalue
		}
	#
	# If only some spines have exclusive interpretation records
	#
	else if ($0 ~ exclusive_record && $0 !~ null_interp)
		{
		#
		# If this is the first interpretation record, reset values
		#
		if (!first_interpretation_yet)
			{
			if (!first_bar_yet)
				anacrusis[++anacrusis_length] = $0 "\t**metpos"
			else
				print $0 "\t**metpos"
			first_interpretation_yet = TRUE
			meter = mvalue
			timebase = tvalue
			}
		else if (!first_bar_yet) anacrusis[++anacrusis_length] = $0 "\t*"
		else print $0 "\t*"
		}
	#
	# If all spines terminate, print a spine-path terminator for the metpos
	# spine and reset the value first_interpretation_yet
	#
	else if ($0 ~ spine_terminate)
		{
		if (!first_bar_yet) anacrusis[++anacrusis_length] = $0 "\t*-"
		else print $0 "\t*-"
		first_interpretation_yet = FALSE
		first_bar_yet = FALSE
		print_anacrusis()
		}
	#
	# If the current record contains a possible timebase or
	# meter interpretation
	#
	####else if ($1 ~ timebase_reg || $1 ~ meter_sig_reg)
	else if ($0 ~ /tb[0-9]+/ || $0 ~ /M[0-9]+/)
		{
		###if (!first_bar_yet) anacrusis[++anacrusis_length] = $0 "\t" $1
		###else print $0 "\t" $1
		process_tandem()
		}
	#
	# Otherwise, simply print the record with a null interpreatation for the
	# metpos spine
	#
	else if (!first_bar_yet) anacrusis[++anacrusis_length] = $0 "\t*"
	else print $0 "\t*"
	}

###########################################################################
##				Function process_tandem
#
function process_tandem(    i,j)
	{
	for (j=1; j<=NF; j++)
		{
		#
		# Identify any timebase interpretations.
		#
		if ($j ~ timebase_reg)
    			{
    			timebase = $j
    			sub(/\*tb/,"",timebase)
			#
			# Ensure that other timebase values in any
			# adjacent spines are identical.
			#
			for (i = j+1; i <= NF; i++)
				{
				if ($i ~ timebase_reg && $i != $j)
					{
					print "metpos: ERROR: Different "\
						"timebase values in concurrent"\
					 	" spines at line: " NR > stderr
					exit
					}
				}
			if (!first_bar_yet) anacrusis[++anacrusis_length] = $0 "\t" $j
			else print $0 "\t" $j
			}
		#
		# Identify any meter signature interpretations.
		#
		else if ($j ~ meter_sig_reg)
			{
			meter = $j
			sub(/\*M/,"",meter)
			#
			# Ensure that other meter signatures in any
			# adjacent spines are identical.
			#
			for (i = j+1; i <= NF; i++)
				{
				if ($i ~ meter_sig_reg && $i != $j)
					{
					print "metpos: ERROR: Different meter "\
						"signatures in concurrent "\
					 	"spines at line: " NR > stderr
					exit
					}
				}
			if (!first_bar_yet) anacrusis[++anacrusis_length] = $0 "\t" $j
			else print $0 "\t" $j
			}
		}
	#
	# If both the meter signature and timebase values are defined,
	# then calculate the metric position values.
	#
	if (meter != "" && timebase != "") calculate_metpos()
	}

###########################################################################
##				Function calculate_metpos
#
function calculate_metpos(    numerator,denominator)
	{
	#
	# Determine the "numerator" and "denominator" for the active
	# meter signature.
	#
	denominator = numerator = meter
	sub(/.*\//,"",denominator)
	sub(/\/.*/,"",numerator)
	#
	#Classify the type of meter signature in terms of meter and beat type.
	#
	# 1) First deal with unusual meter signatures such as "2/4." (i.e. two
	#    dotted quarters per bar -- i.e. 6/8).
	#
	if (denominator ~ /\./)
		{
		numerator = numerator * 3
		gsub(/\./,"",denominator)	# (Double-dotted denominators
		denominator = denominator * 2	#  aren't correctly handled.)
		}
	#
	# 2) Second, determine the meter type.
	#
	if (numerator == 2 || numerator == 6) meter_type = "duple"
	if (numerator == 3 || numerator == 9) meter_type = "triple"
	if (numerator == 4 || numerator == 12) meter_type = "quadruple"
	if (numerator == 5 || numerator == 7 || numerator == 10 || 
		numerator == 11 || int(numerator) >= 13)
			{
			print "metpos: ERROR:  Cannot handle irregular meter: "\
				meter " in line " NR "." > stderr
			exit
			}
	#
	# 3) Third determine the beat type (i.e. simple or compound).
	#
	if (numerator == 6 || numerator == 9 || numerator == 12 || \
											numerator == 15)
		beat_type = "compound"
	else
		beat_type = "simple"
	#
	# Also, determine the number of positions in the metric hierarchy.
	#
	number_of_metric_positions = numerator * (timebase / denominator)
	#
	# Calculate the appropriate metric hierarchy according to
	# the class of meter signature.
	#
	if (beat_type == "simple")
		{
		if (meter_type == "duple" || meter_type == "quadruple")
			simple_2()
		else if (meter_type == "triple")
			simple_3()
		}
	else if (beat_type == "compound")
		{
		if (meter_type == "duple")
			compound_2()
		else if (meter_type == "triple")
			compound_3()
		else if (meter_type == "quadruple")
			compound_4()
		}
	}

###########################################################################
##				Function simple_2
#
function simple_2(   i,j,k,max_metric_value)
	{
	# Establish the maximum number of distinct values in
	# the metric hierarchy.
	#
	max_metric_value = log(number_of_metric_positions)/log(2) + 1
	#
	# Assign the appropriate values to each position in
	# the metric hierarchy.
	#
	for (i = 1; i <= number_of_metric_positions; i += 1)
		{
		j = number_of_metric_positions + 1 - i
		#
		# Begin by assigning all metric positions to the
		# maximum metric value.
		#
		metric_position[j] = max_metric_value
		#
		# Now use base-two calculations to reassign the
		# metric values for various levels in the hierarchy.
		#
		if ((i % 2) == 0) metric_position[j] = max_metric_value - 1
		for (k = 2; k <= max_metric_value; k += 1)
			{
			if ((i % 2**k) == 0) metric_position[j] = max_metric_value - k
			}
		}
	}

###########################################################################
##				Function simple_3
#
function simple_3(   i,j,divisor)
	{
	for (i = 1; i <= 96; i += 1) simple_triple[i] = 7
	for (i = 1; i <= 96; i += 1) if ((i % 2) == 0) simple_triple[i-1] = 6
	for (i = 1; i <= 96; i += 1) if ((i % 4) == 0) simple_triple[i+1] = 5
	for (i = 1; i <= 96; i += 1) if ((i % 8) == 0) simple_triple[i+1] = 4
	for (i = 1; i <= 96; i += 1) if ((i % 16)== 0) simple_triple[i+1] = 3
	for (i = 1; i <= 96; i += 1) if ((i % 32)== 0) simple_triple[i+1] = 2
	simple_triple[1] = 1
	#
	# Assign the values in the metric hierarchy according
	# to the number of metric positions (as determined by
	# the meter signature and timebase values).
	#
	divisor = 96. / number_of_metric_positions
	j = 1
	for (i = 1; i <= number_of_metric_positions; i += 1)
		{
		metric_position[i] = simple_triple[j]
		j += divisor
		}
	}

###########################################################################
##				Function compound_2
#
function compound_2(   i,j,divisor)
	{
	for (i = 1; i <= 96; i += 1) compound_duple[i] = 7
	for (i = 1; i <= 96; i += 1) if ((i % 2)== 0) compound_duple[i-1] = 6
	for (i = 1; i <= 96; i += 1) if ((i % 4)== 0) compound_duple[i+1] = 5
	for (i = 1; i <= 96; i += 1) if ((i % 8)== 0) compound_duple[i+1] = 4
	for (i = 1; i <= 96; i += 1) if ((i % 16)== 0) compound_duple[i+1] = 3
	for (i = 1; i <= 96; i += 1) if ((i % 48)== 0) compound_duple[i+1] = 2
	compound_duple[1] = 1
	#
	# Assign the values in the metric hierarchy according
	# to the number of metric positions (as determined by
	# the meter signature and timebase values).
	#
	divisor = 96. / number_of_metric_positions
	j = 1
	for (i = 1; i <= number_of_metric_positions; i += 1)
		{
		metric_position[i] = compound_duple[j]
		j += divisor
		}
	}

###########################################################################
##				Function compound_3
#
function compound_3(   i,j,divisor)
	{
	for (i = 1; i <= 108; i += 1) compound_triple[i] = 6
	for (i = 1; i <= 108; i += 1) if ((i % 2) == 0) compound_triple[i-1]=5
	for (i = 1; i <= 108; i += 1) if ((i % 4) == 0) compound_triple[i+1]=4
	for (i = 1; i <= 108; i += 1) if ((i % 12) ==0) compound_triple[i+1]=3
	for (i = 1; i <= 108; i += 1) if ((i % 36) ==0) compound_triple[i+1]=2
	compound_triple[1] = 1
	#
	# Assign the values in the metric hierarchy according
	# to the number of metric positions (as determined by
	# the meter signature and timebase values).
	#
	divisor = 108. / number_of_metric_positions
	j = 1
	for (i = 1; i <= number_of_metric_positions; i += 1)
		{
		metric_position[i] = compound_triple[j]
		j += divisor
		}
	}

###########################################################################
##				Function compound_4
#
function compound_4(   i,j,divisor)
	{
	for (i = 1; i <= 96; i += 1) compound_quad[i] = 7
	for (i = 1; i <= 96; i += 1) if ((i % 2)== 0) compound_quad[i-1] = 6
	for (i = 1; i <= 96; i += 1) if ((i % 4)== 0) compound_quad[i+1] = 5
	for (i = 1; i <= 96; i += 1) if ((i % 8)== 0) compound_quad[i+1] = 4
	for (i = 1; i <= 96; i += 1) if ((i % 24)== 0) compound_quad[i+1] = 3
	for (i = 1; i <= 96; i += 1) if ((i % 48)== 0) compound_quad[i+1] = 2
	compound_quad[1] = 1
	#
	# Assign the values in the metric hierarchy according
	# to the number of metric positions (as determined by
	# the meter signature and timebase values).
	#
	divisor = 96. / number_of_metric_positions
	j = 1
	for (i = 1; i <= number_of_metric_positions; i += 1)
		{
		metric_position[i] = compound_quad[j]
		j += divisor
		}
	}

###########################################################################
#				Function process_barlines
#
function process_barlines(    i,j,anacrusis_length)
	{
	#
	# If this is the first barline, deal with the possible anacrusis
	# condition.
	#
	if (!first_bar_yet)
		{
		#
		# If data records have already been encountered, then print them
		#
		print_anacrusis()
		first_bar_yet = TRUE
		if ($1 ~ /\=\=/) first_bar_yet = FALSE
		}
	#
	# Issue a warning if the previous measure was incomplete.
	#
	else
		{
		if (sonority_number < number_of_metric_positions)
			{
			if ($1 !~ /\=\=/)	# It's okay for the final measure to be
				{			# incomplete.
				print "metpos: WARNING: Unexpected barline encountered in "\
					 "line " NR ".\n                 Too few timebase "\
					 "sonorities in measure." > stderr
				}
			else
				{
				first_bar_yet = FALSE
				print_anacrusis()
				}
			}
		else if ($1 ~ /\=\=/)
			{
			first_bar_yet = FALSE
			print_anacrusis()
			}
		}
	print $0 "\t" $1
	sonority_number = 0		# Reset the sonority number to zero.
	}

###########################################################################
##				Function process_data
#
function process_data()
	{
	#
	# If no meter signature has been defined, data records cannot be
	# properly processed.  Before issuing an error, try the interpretation
	# function once again to see if the definition can be resolved.
	#
	if (meter == "")
		{
		print "metpos: ERROR: Data record encountered prior to "\
			"meter signature at line " NR "." > stderr
		exit
		}
	# If no timebase value has been defined, data records cannot be
	# properly processed.  Before issuing an error, try the interpretation
	# function once again to see if the definition can be resolved.
	#
	if (timebase == "")
		{
		print "metpos: ERROR: Data record encountered prior to "\
			"timebase definition at line " NR "." > stderr
		exit
		}
	#
	# If the first bar has already been found
	#
	if (first_bar_yet)
		{
		sonority_number++
		if (sonority_number > number_of_metric_positions)
			{
			print "metpos: ERROR: Number of timebase sonorities exceeds "\
				 "length of\n               measure in input "\
				 "line: " NR > stderr
			exit
			}
		print $0 "\t" metric_position[sonority_number]
		}
	else
		{
		anacrusis[++anacrusis_length] = $0
		data_count += 1
		}
	}

###########################################################################
##				Function print_anacrusis
#
function print_anacrusis(   i)
	{
	#
	# Print all the records stored in anacrusis
	#
	for (i = 1; i <= anacrusis_length; i += 1)
		{
		#
		# Simply print comments, interpretation, and barlines
		#
		if (anacrusis[i] ~ /^[!*=]/) print anacrusis[i]
		#
		# Print the metpos information for data records
		#
		else
			{
			print anacrusis[i] "\t" \
			      metric_position[number_of_metric_positions - data_count+1]
			data_count -= 1
			}
		}
	#
	# Reset the variables handling anacrusis material
	#
	data_count = 0
	anacrusis_length = 0
	}
