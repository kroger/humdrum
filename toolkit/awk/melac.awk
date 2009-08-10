#####################################################################
##                               MELAC.AWK
# Programmed by: David Huron        Date:  1993 March 8
# Copyright (c) 1993 David Huron
#
# Modifications:
#    Date:     Programmer:        	Description:
# July 12/94	Tim Racinsky	Added parse_command function and some 
#						error messages
#
# This program accepts as input humdrum **semits data and outputs
# values representing the degree of melodic accent associated with
# each note.  Tonal accent values vary between 0 (no salience or
# prominence) and 1 (maximum salience).
#
# The program implements the model of melodic accent described in
# Joseph Thomassen, "Melodic accent: Experiments and a tentative model,"
# Journal of the Acoustical Society of America, Vol. 71, No. 6 (1982) pp.1598-
# 1605; see also, Erratum, Journal of the Acoustical Society of America,
# Vol. 73, No. 1 (1983) p.373.
#
# Thomassen's model assigns melodic accents according to the possible
# melodic contours arising in 3-pitch windows.
#
BEGIN	{
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0

	# Set standard error to the appropriate 'file'

	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")

	# Define the three types of melodic motion distinguished in
	# Thomassen's model.

	unison = 0
	ascending = +1
	descending = -1

	# Define a special marker using an unlikely input data token.

	note_marker = "9s$v;m.2tw("

	# Other variables

	input_buf_length = note_buf_length = 0
	not_processing = TRUE

	# Parse the command line option

	parse_command()
	}
{
# MAIN		***********************************************************

# If the number of input fields is not 1, then print an error

if ($0 !~ /^!!/ && NF != 1)
	{
	print "melac: ERROR: Input must contain precisely one spine." > stderr
	exit
	}

# Exclusive Interpretations must begin new input - echo any non-semits input
#
if ($0 ~ /^\*\*/)
	{
	if ($0 == "**semits")
		{
		flush_input_buf()
		add_input_buf("**melac")
		not_processing = FALSE
		}
	else
		{
		flush_input_buf()
		print $0
		not_processing = TRUE
		}
	next
	}
else if (not_processing) print $0
else
	{
	# If the current token contains a double barline, rest, or spine-path
	# terminator, stop processing the input

	if ($0 ~ /^((\=\=)|(r))/ || $0 == "*-")
		{
		flush_input_buf()
		print $0
		}
	# If the current token is a comment, barline, tandem interpretation, or
	# null data token, simply add the token to the buffer

	else if ($0 ~ /^[*!=]/ || $0 == ".") add_input_buf($0)

	# Otherwise, process the note token

	else
		{
		add_note_buf($0)
		add_input_buf(note_marker)
		salience()
		}
	}
}
# Always flush the input buffer at the end of the program

END	{
	flush_input_buf()
	}

###########################################################################
# 				FUNCTIONS	USED IN MELAC.AWK
###########################################################################

###########################################################################
#                        Function parse_command
function parse_command()
	{
	ARGV[1] = ""
	}

###########################################################################
#                        Function motion
# Determine the type of melodic motion.
#
function motion(note_difference)
	{
	if (note_difference > 0) return ascending
	if (note_difference < 0) return descending
	if (note_difference == 0) return unison
	}

###########################################################################
#                       Function add_input_buf
function add_input_buf(token)
	{
	input_buf[++input_buf_length] = token
	}

###########################################################################
#                       Function flush_input_buf
function flush_input_buf()
	{
	for (i = 1; i <= input_buf_length; i += 1)
		{
		if (input_buf[i] != note_marker) print input_buf[i]
		else
			{
			#
			# Order is important here
			#
			print melodic_accent[1]
			subtract_melodic_accent()
			subtract_note_buf()
			}
		}
	input_buf_length = 0
	}

###########################################################################
#                         Function add_note_buf
function add_note_buf(note)
	{
	note_buf[++note_buf_length] = note
	}

###########################################################################
#                         Function subtract_note_buf
function subtract_note_buf( j)
	{
	for (j = 1; j < note_buf_length; j += 1)
		note_buf[j] = note_buf[j + 1]
	note_buf_length--
	}

###########################################################################
#                         Function subtract_melodic_accent
function subtract_melodic_accent( j)
	{
	for (j = 1; j < note_buf_length; j += 1)
		melodic_accent[j] = melodic_accent[j + 1]
	}

###########################################################################
#                         Function salience
function salience()
	{
	if (note_buf_length == 1)
		{
		melodic_accent[1] = 1.00	# Salience for a single note.
		}
	if (note_buf_length == 2)
		{
		if (note_buf[1] == note_buf[2])		# Unison condition.
			{
			melodic_accent[1] = 0.50
			melodic_accent[2] = 0.50
			}
		else					# Non-unison condition.
			{
			melodic_accent[1] = 1.00
			melodic_accent[2] = 1.00
			}
		}
	if (note_buf_length == 3)
		{
		motion1 = motion(note_buf[2] - note_buf[1])
		motion2 = motion(note_buf[3] - note_buf[2])

		if (motion1 == unison && motion2 == unison)
			{
			melodic_accent[1] = 1.00
			melodic_accent[2] = 0.00
			melodic_accent[3] = 0.00
			}

		if (motion1 != unison && motion2 == unison)
			{
			melodic_accent[1] = 1.00
			melodic_accent[2] = 1.00
			melodic_accent[3] = 0.00
			}

		if (motion1 == unison && motion2 != unison)
			{
			melodic_accent[1] = 1.00
			melodic_accent[2] = 0.00
			melodic_accent[3] = 1.00
			}

		if (motion1 == ascending && motion2 == descending)
			{
			melodic_accent[1] = 1.00
			melodic_accent[2] = 0.83
			melodic_accent[3] = 0.17
			}

		if (motion1 == descending && motion2 == ascending)
			{
			melodic_accent[1] = 1.00
			melodic_accent[2] = 0.71
			melodic_accent[3] = 0.29
			}

		if (motion1 == ascending && motion2 == ascending)
			{
			melodic_accent[1] = 1.00
			melodic_accent[2] = 0.33
			melodic_accent[3] = 0.67
			}

		if (motion1 == descending && motion2 == descending)
			{
			melodic_accent[1] = 1.00
			melodic_accent[2] = 0.50
			melodic_accent[3] = 0.50
			}
		}
	if (note_buf_length > 3)
		{
		motion1 = motion(note_buf[note_buf_length-1] \
				 - note_buf[note_buf_length-2])
		motion2 = motion(note_buf[note_buf_length]  \
				- note_buf[note_buf_length-1])

		if (motion1 == unison && motion2 == unison)
			{
			melodic_accent[note_buf_length-2] =	\
				melodic_accent[note_buf_length-2] * 1.00
			melodic_accent[note_buf_length-1] =	\
				melodic_accent[note_buf_length-1] * 0.00
			melodic_accent[note_buf_length] = 0.00
			}

		if (motion1 != unison && motion2 == unison)
			{
			melodic_accent[note_buf_length-2] =	\
				melodic_accent[note_buf_length-2] * 1.00
			melodic_accent[note_buf_length-1] =	\
				melodic_accent[note_buf_length-1] * 1.00
			melodic_accent[note_buf_length] = 0.00
			}

		if (motion1 == unison && motion2 != unison)
			{
			melodic_accent[note_buf_length-2] =	\
				melodic_accent[note_buf_length-2] * 1.00
			melodic_accent[note_buf_length-1] =	\
				melodic_accent[note_buf_length-1] * 0.00
			melodic_accent[note_buf_length] = 1.00
			}

		if (motion1 == ascending && motion2 == descending)
			{
			melodic_accent[note_buf_length-2] =	\
				melodic_accent[note_buf_length-2] * 1.00
			melodic_accent[note_buf_length-1] =	\
				melodic_accent[note_buf_length-1] * 0.83
			melodic_accent[note_buf_length] = 0.17
			}

		if (motion1 == descending && motion2 == ascending)
			{
			melodic_accent[note_buf_length-2] =	\
				melodic_accent[note_buf_length-2] * 1.00
			melodic_accent[note_buf_length-1] =	\
				melodic_accent[note_buf_length-1] * 0.71
			melodic_accent[note_buf_length] = 0.29
			}

		if (motion1 == ascending && motion2 == ascending)
			{
			melodic_accent[note_buf_length-2] =	\
				melodic_accent[note_buf_length-2] * 1.00
			melodic_accent[note_buf_length-1] =	\
				melodic_accent[note_buf_length-1] * 0.33
			melodic_accent[note_buf_length] = 0.67
			}

		if (motion1 == descending && motion2 == descending)
			{
			melodic_accent[note_buf_length-2] =	\
				melodic_accent[note_buf_length-2] * 1.00
			melodic_accent[note_buf_length-1] =	\
				melodic_accent[note_buf_length-1] * 0.50
			melodic_accent[note_buf_length] = 0.50
			}
		}
	}
