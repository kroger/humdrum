######################################################################
##					TRANS1.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# June 7/94	Tim Racinsky	Modified parse_command to work with getopts
# April 8/95	David Huron	Fixed so **kern signifiers output in order of input
# Jan. 5/97	David Huron	Added -k option; fixed ph_key_sig regular exp.
#
#
# This program is used to change modes of kern, pitch, Tonh, and solfg encodings
#
# Main Functions used:
#	parse_command()		store_new_interps()		store_indicators()
#	process_indicators()	ins_array_pos()		del_array_pos()
#	exchange_spines()		process_data()			check_pc_sig()
#	to_upper()			to_lower()			find_position()
#	transpose()			process_pitch()		back_to_pitch()
#	process_Tonh()			back_to_Tonh()			process_solfg()
#	back_to_solfg()		process_kern()			back_to_kern()
#
#	VARIABLES:
#
#  -spine_path_record:  flag used to indicate if the current record
#   is a spine-path record or not.
#  -no_interpretation_yet:  flag used to indicate if an interpretation
#   record has been processed yet.
#  -current_no_of_spines:  holds the current number of active spines.
#  -options:  holds which options the user has specified.
#  -stderr: allows print to be re-directed to standard error.
#  -floating_pt_num:  defines a valid floating point number.
#  -deviation:  defines a valid equally-tempered deviation.
#  -indicators: defines spine path indicators. 
#  -input_interps:  defines valid input interpretations to process.
#  -diatonic_reg: defines valid value of diatonic interval
#  -key_reg: defines valid key interpretation
#  -pc_key_sig:  defines valid pitch class key signature interpretation
#  -ph_key_sig: defines valid pitch height key signature interpretation
#  -key_sig: holds command-line keysig specified for -k option
#  -repetition: defines invalid pitch class key signature interpretation
#  -pc_note:  defines pitch class note
#  -pitch_reg: defines pitch
#  -null_interps: defines line of only null interpretations
#  -pc_index:  holds value of latest pitch class key signature stored
#  -diatonic:  holds value of diatonic change
#  -octave_class: regular expression to find octave class
#
#	ARRAYS:
#
#  -path_indicator and current_interp:  the standard arrays for
#   tracking interpretations.
#  -current_note: a global array used to store the current note. It holds
#   the ocatave class, letter name, accidentals, and equally tempered deviation.
#  -key: a 2-dimensional array holding pitches for each pitch in the given key
#  -pc: same as key except for pitch class key signature interpretations
#  -current_key: holds the current key of each of the currently active spines.
#
BEGIN {
	#
	# Set global variables and expressions
	#
	FS = OFS = "\t"
	TRUE = 1 ; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	diatonic_reg = "^[+-]?[1-9][0-9]*$"
	deviation = "[+-][1-9][0-9]*"
	octave_class = "[0-9]"
	indicators = "^(\\*\\+|\\*\\-|\\*\\^|\\*v|\\*x)$"
	input_interps = "^(\\*\\*kern|\\*\\*pitch|\\*\\*Tonh|\\*\\*solfg)$"
	key_reg = "^\\*(([ABCDEFGabcdefg](#?|-?))|(\\?)|(X)|(Cx)|(cx)|(Dx)):$"
	pc_key_sig = "^\\*k\\[([abcdefg]((n)|(#)|(x+(#?))|(-)+))*\\]$"
	ph_key_sig = "^\\*K\\[([ABCDEFG]((n)|(#)|(x+(#?))|(-)+)*[1-9])*\\]$"
	repetition = "(a.*a)|(b.*b)|(c.*c)|(d.*d)|(e.*e)|(f.*f)|(g.*g)"
	pc_note = "[abcdefg]((n)|(#)|(x+#?)|(-+))"
	pitch_reg = "[abcdefg]"
	kern_pitch_token = "[abcdefgABCDEFG]+[-#n]*"
	null_interps = "^\\*(	\\*)*$"
	pc_index = 1
	diatonic = 0
	#
	# Determine user's specified options
	#
	parse_command()
	#
	# Initialize the key array
	#
	key["*C:",0] = "C"; key["*C:",1] = "D"; key["*C:",2] = "E";
	key["*C:",3] = "F"; key["*C:",4] = "G"; key["*C:",5] = "A";
	key["*C:",6] = "B";
	key["*c:",0] = "C"; key["*c:",1] = "D"; key["*c:",2] = "E/-";
	key["*c:",3] = "F"; key["*c:",4] = "G"; key["*c:",5] = "A/-";
	key["*c:",6] = "B";
	key["*C#:",0] = "C/#"; key["*C#:",1] = "D/#"; key["*C#:",2] = "E/#";
	key["*C#:",3] = "F/#"; key["*C#:",4] = "G/#"; key["*C#:",5] = "A/#";
	key["*C#:",6] = "B/#";
	key["*c#:",0] = "C/#"; key["*c#:",1] = "D/#"; key["*c#:",2] = "E";
	key["*c#:",3] = "F/#"; key["*c#:",4] = "G/#"; key["*c#:",5] = "A";
	key["*c#:",6] = "B/#";
	key["*C-:",0] = "C/-"; key["*C-:",1] = "D/-"; key["*C-:",2] = "E/-";
	key["*C-:",3] = "F/-"; key["*C-:",4] = "G/-"; key["*C-:",5] = "A/-";
	key["*C-:",6] = "B/-";
	key["*c-:",0] = "C/-"; key["*c-:",1] = "D/-"; key["*c-:",2] = "E/--";
	key["*c-:",3] = "F/-"; key["*c-:",4] = "G/-"; key["*c-:",5] = "A/--";
	key["*C-:",6] = "B/-";
	key["*D:",0] = "C/#"; key["*D:",1] = "D"; key["*D:",2] = "E";
	key["*D:",3] = "F/#"; key["*D:",4] = "G"; key["*D:",5] = "A";
	key["*D:",6] = "B";
	key["*d:",0] = "C/#"; key["*d:",1] = "D"; key["*d:",2] = "E";
	key["*d:",3] = "F"  ; key["*d:",4] = "G"; key["*d:",5] = "A";
	key["*d:",6] = "B/-";
	key["*D#:",0] = "C/##"; key["*D#:",1] = "D/#"; key["*D#:",2] = "E/#";
	key["*D#:",3] = "F/##"; key["*D#:",4] = "G/#"; key["*D#:",5] = "A/#";
	key["*D#:",6] = "B/#" ;
	key["*d#:",0] = "C/##"; key["*d#:",1] = "D/#"; key["*d#:",2] = "E/#";
	key["*d#:",3] = "F/#" ; key["*d#:",4] = "G/#"; key["*d#:",5] = "A/#";
	key["*d#:",6] = "B";
	key["*D-:",0] = "C"  ; key["*D-:",1] = "D/-"; key["*D-:",2] = "E/-";
	key["*D-:",3] = "F"  ; key["*D-:",4] = "G/-"; key["*D-:",5] = "A/-";
	key["*D-:",6] = "B/-";
	key["*d-:",0] = "C"   ; key["*d-:",1] = "D/-"; key["*d-:",2] = "E/-";
	key["*d-:",3] = "F/-" ; key["*d-:",4] = "G/-"; key["*d-:",5] = "A/-";
	key["*d-:",6] = "B/--";
	key["*E:",0] = "C/#"; key["*E:",1] = "D/#"; key["*E:",2] = "E";
	key["*E:",3] = "F/#"; key["*E:",4] = "G/#"; key["*E:",5] = "A";
	key["*E:",6] = "B";
	key["*e:",0] = "C"  ; key["*e:",1] = "D/#"; key["*e:",2] = "E";
	key["*e:",3] = "F/#"; key["*e:",4] = "G"  ; key["*e:",5] = "A";
	key["*e:",6] = "B"  ;
	key["*E#:",0] = "C/##"; key["*E#:",1] = "D/##"; key["*E#:",2] = "E/#";
	key["*E#:",3] = "F/##"; key["*E#:",4] = "G/##"; key["*E#:",5] = "A/#";
	key["*E#:",6] = "B/#" ;
	key["*e#:",0] = "C/#" ; key["*e#:",1] = "D/##"; key["*e#:",2] = "E/#";
	key["*e#:",3] = "F/##"; key["*e#:",4] = "G/#" ; key["*e#:",5] = "A/#";
	key["*e#:",6] = "B/#" ;
	key["*E-:",0] = "C"  ; key["*E-:",1] = "D"; key["*E-:",2] = "E/-";
	key["*E-:",3] = "F"  ; key["*E-:",4] = "G"; key["*E-:",5] = "A/-";
	key["*E-:",6] = "B/-";
	key["*e-:",0] = "C/-"; key["*e-:",1] = "D"  ; key["*e-:",2] = "E/-";
	key["*e-:",3] = "F"  ; key["*e-:",4] = "G/-"; key["*e-:",5] = "A/-";
	key["*e-:",6] = "B/-";
	key["*F:",0] = "C"  ; key["*F:",1] = "D"; key["*F:",2] = "E";
	key["*F:",3] = "F"  ; key["*F:",4] = "G"; key["*F:",5] = "A";
	key["*F:",6] = "B/-";
	key["*f:",0] = "C"  ; key["*f:",1] = "D/-"; key["*f:",2] = "E";
	key["*f:",3] = "F"  ; key["*f:",4] = "G"  ; key["*f:",5] = "A/-";
	key["*f:",6] = "B/-";
	key["*F#:",0] = "C/#"; key["*F#:",1] = "D/#"; key["*F#:",2] = "E/#";
	key["*F#:",3] = "F/#"; key["*F#:",4] = "G/#"; key["*F#:",5] = "A/#";
	key["*F#:",6] = "B";
	key["*f#:",0] = "C/#"; key["*f#:",1] = "D"  ; key["*f#:",2] = "E/#";
	key["*f#:",3] = "F/#"; key["*f#:",4] = "G/#"; key["*f#:",5] = "A";
	key["*f#:",6] = "B"  ;
	key["*F-:",0] = "C/-" ; key["*F-:",1] = "D/-"; key["*F-:",2] = "E/-";
	key["*F-:",3] = "F/-" ; key["*F-:",4] = "G/-"; key["*F-:",5] = "A/-";
	key["*F-:",6] = "B/--";
	key["*f-:",0] = "C/-"; key["*f-:",1] = "D/--"; key["*f-:",2] = "E/-";
	key["*f-:",3] = "F/-"; key["*f-:",4] = "G/-" ; key["*f-:",5] = "A/--";
	key["*f-:",6] = "B/--";
	key["*G:",0] = "C"  ; key["*G:",1] = "D"; key["*G:",2] = "E";
	key["*G:",3] = "F/#"; key["*G:",4] = "G"; key["*G:",5] = "A";
	key["*G:",6] = "B"  ;
	key["*g:",0] = "C"  ; key["*g:",1] = "D"; key["*g:",2] = "E/-";
	key["*g:",3] = "F/#"; key["*g:",4] = "G"; key["*g:",5] = "A";
	key["*g:",6] = "B/-";
	key["*G#:",0] = "C/#" ; key["*G#:",1] = "D/#"; key["*G#:",2] = "E/#";
	key["*G#:",3] = "F/##"; key["*G#:",4] = "G/#"; key["*G#:",5] = "A/#";
	key["*G#:",6] = "B/#" ;
	key["*g#:",0] = "C/#" ; key["*g#:",1] = "D/#"; key["*g#:",2] = "E";
	key["*g#:",3] = "F/##"; key["*g#:",4] = "G/#"; key["*g#:",5] = "A/#";
	key["*g#:",6] = "B"   ;
	key["*G-:",0] = "C/-"; key["*G-:",1] = "D/-"; key["*G-:",2] = "E/-";
	key["*G-:",3] = "F"  ; key["*G-:",4] = "G/-"; key["*G-:",5] = "A/-";
	key["*G-:",6] = "B/-";
	key["*g-:",0] = "C/-" ; key["*g-:",1] = "D/-"; key["*g-:",2] = "E/--";
	key["*g-:",3] = "F"   ; key["*g-:",4] = "G/-"; key["*g-:",5] = "A/-";
	key["*g-:",6] = "B/--";
	key["*A:",0] = "C/#"; key["*A:",1] = "D"  ; key["*A:",2] = "E";
	key["*A:",3] = "F/#"; key["*A:",4] = "G/#"; key["*A:",5] = "A";
	key["*A:",6] = "B"  ;
	key["*a:",0] = "C"; key["*a:",1] = "D"  ; key["*a:",2] = "E";
	key["*a:",3] = "F"; key["*a:",4] = "G/#"; key["*a:",5] = "A";
	key["*a:",6] = "B";
	key["*A#:",0] = "C/##"; key["*A#:",1] = "D/#" ; key["*A#:",2] = "E/#";
	key["*A#:",3] = "F/##"; key["*A#:",4] = "G/##"; key["*A#:",5] = "A/#";
	key["*A#:",6] = "B/#" ;
	key["*a#:",0] = "C/#"; key["*a#:",1] = "D/#" ; key["*a#:",2] = "E/#";
	key["*a#:",3] = "F/#"; key["*a#:",4] = "G/##"; key["*a#:",5] = "A/#";
	key["*a#:",6] = "B/#";
	key["*A-:",0] = "C"  ; key["*A-:",1] = "D/-"; key["*A-:",2] = "E/-";
	key["*A-:",3] = "F"  ; key["*A-:",4] = "G"  ; key["*A-:",5] = "A/-";
	key["*A-:",6] = "B/-";
	key["*a-:",0] = "C/-"; key["*a-:",1] = "D/-"; key["*a-:",2] = "E/-";
	key["*a-:",3] = "F/-"; key["*a-:",4] = "G"  ; key["*a-:",5] = "A/-";
	key["*a-:",6] = "B/-";
	key["*B:",0] = "C/#"; key["*B:",1] = "D/#"; key["*B:",2] = "E";
	key["*B:",3] = "F/#"; key["*B:",4] = "G/#"; key["*B:",5] = "A/#";
	key["*B:",6] = "B"  ;
	key["*b:",0] = "C/#"; key["*b:",1] = "D"; key["*b:",2] = "E";
	key["*b:",3] = "F/#"; key["*b:",4] = "G"; key["*b:",5] = "A/#";
	key["*b:",6] = "B"  ;
	key["*B#:",0] = "C/##"; key["*B#:",1] = "D/##"; key["*B#:",2] = "E/#";
	key["*B#:",3] = "F/##"; key["*B#:",4] = "G/##"; key["*B#:",5] = "A/##";
	key["*B#:",6] = "B/#" ;
	key["*b#:",0] = "C/##"; key["*b#:",1] = "D/#"; key["*b#:",2] = "E/#";
	key["*b#:",3] = "F/##"; key["*b#:",4] = "G/#"; key["*b#:",5] = "A/##";
	key["*b#:",6] = "B/#" ;
	key["*B-:",0] = "C"  ; key["*B-:",1] = "D"; key["*B-:",2] = "E/-";
	key["*B-:",3] = "F"  ; key["*B-:",4] = "G"; key["*B-:",5] = "A";
	key["*B-:",6] = "B/-";
	key["*b-:",0] = "C"  ; key["*b-:",1] = "D/-"; key["*b-:",2] = "E/-";
	key["*b-:",3] = "F"  ; key["*b-:",4] = "G/-"; key["*b-:",5] = "A";
	key["*b-:",6] = "B/-";
	}
{
#
# For each new file, set the variables
#
if (FNR == 1)
	{
	spine_path_record = FALSE
	no_interpretation_yet = TRUE
	current_no_of_spines = 0
	}
#
# Print all comments.
#
if ($0 ~ /^!/) print $0
#
# Process interpretations.
#
else if ($0 ~ /^\*/)
	{
	# Change key signatures if -k option specified.
	if (keysig == "NONE") print $0
	else
		{
		if ($1 ~ pc_key_sig || $1 ~ ph_key_sig) interp_out_line = keysig
		else interp_out_line = $1
		for (i=2; i<=NF; i++)
			{
			if ($i ~ pc_key_sig || $i ~ ph_key_sig) interp_out_line = interp_out_line "	" keysig
			else interp_out_line = interp_out_line "	" $i
			}
		print interp_out_line
		}
	#
	# If this is the first interpretation, set up the arrays and variables
	#
	if (no_interpretation_yet)
		{
		current_no_of_spines = NF
		no_interpretation_yet = FALSE
		store_new_interps()
		}
	#
	# Otherwise check to see if it is a spine_path record
	#
	else
		{
		spine_path_record = FALSE
		for (i = 1; i <= current_no_of_spines; i += 1)
			{			
			if ($i ~ indicators) {
				spine_path_record = TRUE 
				break
				}
			}
		#
		# If it is a spine-path record, print and process the indicators
		#
		if (spine_path_record)
			{
			store_indicators()
			process_indicators()
			if (current_no_of_spines == 0) no_interpretation_yet = TRUE
			}
		#
		# Otherwise, just store the interpretations
		#
		else store_new_interps()
		}
	}
#
# Process current data record
#
else process_data()
}	

##################################################################
##			FUNCTIONS USED IN TRANS1.AWK
##################################################################

##################################################################
##			Function Parse_command
#
function parse_command()
	{
	#
	# The given diatonic interval must be an integer
	#
	if (ARGV[2] !~ diatonic_reg)
		{
		print "trans: ERROR: Invalid value specified for diatonic interval "\
			 "on command line." > stderr
		exit
		}
	else diatonic = ARGV[2] + 0
	if (ARGV[3] !~ pc_key_sig && ARGV[3] !~ ph_key_sig && ARGV[3] !="NONE")
		{
		print "trans: ERROR: Invalid key signature specified with -k option."\
			 > stderr
		exit
		}
	keysig = ARGV[3]
	ARGV[1] = ARGV[2] = ARGV[3] = ""
	}

################################################################
##				Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
#
function store_new_interps(      j,interp_line,null_line)
	{
	interp_line = ""
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		#
		# Store exclusive interpretations and key interpretations
		#
		if ($j ~ /^\*\*/) current_interp[j] = $j
		else if ($j ~ key_reg) current_key[j] = $j
		else if ($j ~ /^\*[kK]\[/)
			{
			#
			# The given key signature interpretation must be valid
			# If it is a pitch class signature, store it in the array pc
			#
			if ($j ~ pc_key_sig) check_pc_key($j,j)
			else if ($j ~ ph_key_sig) ;
			else
				{
				print "trans: ERROR: Invalid key signature interpretation "\
					 "in spine " j ", line " FNR "." > stderr
				exit
				}
			}
		#
		# If the current line has target interpretations, then the
		# next output line must show the transposition interpretation.
		#
		if ($j ~ input_interps) interp_line = interp_line "*Trd" diatonic "\t"
		else interp_line = interp_line "*\t"
		}
	#
	# Print the next interpretation line if it is NOT all null interpretations
	#
	null_line = substr(interp_line,1,length(interp_line)-1)
	if (null_line !~ null_interps) print null_line
	}

#########################################################################
##				Function Check_pc_key
#	This function checks that the pitch class key signature interpretation
# is a valid one and stores its value in pc
#
function check_pc_key(sig,spine,   note,pitch,position,i,pc_array)
	{
	#
	# Check that no pitches are repeated
	#
	if (sig ~ repetition)
		{
		print "trans: ERROR: Invalid key signature interpretation "\
			 "in spine " spine ", line " FNR "." > stderr
		exit
		}
	else
		{
		#
		# Loop through all of the pitches in the interpretation
		# and store the values in the array pc
		#
		while (match(sig,pc_note))
			{
			note = substr(sig,RSTART,RLENGTH)
			sub(pc_note,"",sig)
			sub("x","##",note)
			position = find_position(note)
			pitch = to_upper(substr(note,1,1))
			sub(pitch_reg,"",note)
			if (note != "") pc[pc_index,position] = pitch "/" note
			else pc[pc_index,position] = pitch
			}
		#
		# For any pitches that were not specified, use the pitch values
		# of C major to fill in the gaps
		#
		for (i = 0; i <= 6; i += 1)
			{
			if (pc[pc_index,i] == "") pc[pc_index,i] = key["*C:",i]
			}
		#
		# Store the 'index' of the array in current_key and update the index
		#
		current_key[spine] = "pc" pc_index
		pc_index += 1
		}
	}

####################################################################
##				Function Find_position
#	This function determines which array postion the note should be in
#
function find_position(note,  position)
	{
	position = 0
	#
	# Find the letter name of the note
	#
	if (note ~ /c/) position = 0
	else if (note ~ /d/) position = 1
	else if (note ~ /e/) position = 2
	else if (note ~ /f/) position = 3
	else if (note ~ /g/) position = 4
	else if (note ~ /a/) position = 5
	else if (note ~ /b/) position = 6
	return position
	}

########################################################################
##				Function to_lower
#	This function converts a pitch to lower case.
#
function to_lower(letter)
	{
	if (letter == "A") return "a"
	else if (letter == "B") return "b"
	else if (letter == "C") return "c"
	else if (letter == "D") return "d"
	else if (letter == "E") return "e"
	else if (letter == "F") return "f"
	else if (letter == "G") return "g"
	}

########################################################################
##				Function to_upper
#	This function converts a pitch to upper case.
#
function to_upper(letter)
	{
	if (letter == "a") return "A"
	else if (letter == "b") return "B"
	else if (letter == "c") return "C"
	else if (letter == "d") return "D"
	else if (letter == "e") return "E"
	else if (letter == "f") return "F"
	else if (letter == "g") return "G"
	}

##############################################################
##			Function Store_indicators
#	This function allows the spine-path indicators for the 
# current record to be stored in the array 'path_indicator' so
# that they may be used later.
#
function store_indicators(   i)
	{
	for (i = 1; i <= current_no_of_spines; i += 1)
		{
		if ($i == "*") path_indicator[i] = "*"
		else path_indicator[i] = substr($i,2,1)
		}
	}

##################################################################
##			Function Process_indicators
#	This function takes the spine-path indicators that were stored
# in the array 'path_indicator' in the function 'store_indicators'
# and manipulates the arrays 'path_indicator' and 'current_interp'
# according to the contents of the array 'path_indicator'.
#
function process_indicators(    i)
	{
	i = 1
	while (i <= current_no_of_spines)
		{
		if (path_indicator[i] == "^")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = current_interp[i]
			current_key[i+1] = current_key[i]
			}
		if (path_indicator[i] == "-")
			{
			del_array_pos(i)
			i -= 1
			}
		if (path_indicator[i] == "x")
			{
			exchange_spines()
			}
		if (path_indicator[i] == "v")
			{
			del_array_pos(i+1)
			if (path_indicator[i+1] != "v") path_indicator[i] = "*"
			else i -= 1
			}
		if (path_indicator[i] == "+")
			{
			ins_array_pos(i+1)
			path_indicator[i+1] = "*"
			current_interp[i+1] = ""
			current_key[i+1] = ""
			}
		i += 1
		}
	}

############################################################
##			Function Ins_array_pos
#	This function inserts new positions in the arrays 'path_indicator',
# 'current_interp', and 'current_key' and copies elements so that 
# everything is preserved
#
function ins_array_pos(current_element,     j) 
	{
	for (j = current_no_of_spines+1; j > current_element; j -= 1)
		{
		path_indicator[j] = path_indicator[j-1]
		current_interp[j] = current_interp[j-1]
		current_key[j] = current_key[j-1]
		}
	path_indicator[current_element-1] = "*"
	current_no_of_spines += 1
	}

##############################################################
##			Function Del_array_pos
#	Performs the opposite of function 'ins_array_pos'.
#
function del_array_pos(current_element,     j)
	{
	for (j = current_element; j < current_no_of_spines; j += 1)
		{
		path_indicator[j] = path_indicator[j+1]
		current_interp[j] = current_interp[j+1]
		current_key[j] = current_key[j+1]
		}
	delete path_indicator[j]
	delete current_interp[j]
	delete current_key[j]
	current_no_of_spines -= 1
	} 

###################################################################
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in current_interp and current_key.
#
function exchange_spines(    j,count,ex_array,temp)
	{
	#
	# Determine which spines are to be exchanged
	#
	count = 1
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		if (path_indicator[j] == "x")
			{
			ex_array[count] = j
			count += 1
			path_indicator[j] = "*"
			}
		}
	#
	# Exchange the spines
	#
	temp = current_interp[ex_array[1]]
	current_interp[ex_array[1]] = current_interp[ex_array[2]]
	current_interp[ex_array[2]] = temp
	temp = current_key[ex_array[1]]
	current_key[ex_array[1]] = current_key[ex_array[2]]
	current_key[ex_array[2]] = temp
	}

####################################################################
##				Function Process_data
#	This function takes the given input record and processes the data
# tokens in each spine according to their current exclusive interpretation
# and the value of the data tokens.
#
function process_data(     j,current_string,current_token)
	{
	current_string = ""
	#
	# Loop through each of the currently active spines and process the data
	# tokens that are in each spine.
	#
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		#
		# If the current interpretation is a valid input interpretation
		#
		if (current_interp[j] ~ input_interps)
			{
			current_token = ""
			#
			# If the token contains a null token or barline, print it
			#
			if ($j ~ /^[.]$/ || $j ~ /^=/) current_token = $j
			#
			# For all other tokens, transpose the notes
			#
			else
				{
				#
				# Call the appropriate function based on the exclusive 
				# interpretation in order to obtain the transposed output.
				#
				if (current_interp[j] == "**pitch")
					current_token = process_pitch($j,j)
				else if (current_interp[j] == "**Tonh")
					current_token = process_Tonh($j,j)
				else if (current_interp[j] == "**solfg")
					current_token = process_solfg($j,j)
				else if (current_interp[j] == "**kern")
					current_token = process_kern($j,j)
				}
			#
			# Construct a string for the current line
			#
			current_string = current_string current_token "\t"
			}
		#
		# All other spines simply have their data tokens echoed
		#
		else current_string = current_string $j "\t"
		}
	#
	# After all spines have been processed, print the resulting record
	#
	print substr(current_string,1,length(current_string)-1)
	}

########################################################################
##				Function transpose
#	This function takes the note stored in current_note and transposes
# it according to the value of diatonic
#
function transpose(spine,   i,return_token,old_position,new_position,num,\
												arrayl,temp_index)
	{
	#
	# If the current spine has a key interpretation
	#
	if (current_key[spine] ~ key_reg)
		{
		for (i = 0; i <= 6; i += 1)
			{
			#
			# Determine which position in the array the pitch is
			# and transpose the pitch accordingly
			#
			if (key[current_key[spine],i] ~ current_note[2])
				{
				old_position = i
				new_position = (((old_position + diatonic) % 7) + 7) % 7
				num = split(key[current_key[spine],new_position],arrayl,"/")
				if (num == 1) arrayl[2] = ""
				break
				}
			}
		}
	#
	# Perform the same function if a pitch class key signature is specified
	#
	else if (current_key[spine] ~ /pc/)
		{
		temp_index = substr(current_key[spine],3)
		for (i = 0; i <= 6; i += 1)
			{
			if (pc[temp_index,i] ~ current_note[2])
				{
				old_position = i
				new_position = (((old_position + diatonic) % 7) + 7) % 7
				num = split(pc[temp_index,new_position],arrayl,"/")
				if (num == 1) arrayl[2] = ""
				break
				}
			}
		}
	#
	# If there is no key signature specified, use the values of C major
	#
	if (current_key[spine] == "")
		{
		for (i = 0; i <= 6; i += 1)
			{
			if (key["*C:",i] ~ current_note[2])
				{
				old_position = i
				new_position = (((old_position + diatonic) % 7) + 7) % 7
				num = split(key["*C:",new_position],arrayl,"/")
				if (num == 1) arrayl[2] = ""
				break
				}
			}
		}
	#
	# Determine the new pitch class
	#
	new_position = old_position + diatonic
	if (diatonic > 0)
		current_note[1] = current_note[1] + int((new_position) / 7)
	else
		current_note[1] = current_note[1] \
				+ (new_position >= 0 ? 0 : int((new_position + 1) / 7) - 1)
	current_note[2] = arrayl[1]
	current_note[3] = arrayl[2]
	}

#########################################################################
##				Function Process_pitch
#	This function accepts pitch input and transposes it
#
function process_pitch(data_token,spine,  return_token,arraya,j,split_num,\
									not_found,accidentals,original)
	{
	#
	# Process multiple stops
	#
	original = data_token
	return_token = ""
	split_num = split(data_token,arraya," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If a rest occurs, simply echo it
		#
		if (arraya[j] ~ /r/) return_token = return_token arraya[j] " "
		#
		# Otherwise, process the pitch note
		#
		else
			{
			not_found = FALSE
			if (match(arraya[j],deviation))
				{
				current_note[4] = substr(arraya[j],RSTART,RLENGTH)
				sub(deviation,"",arraya[j])
				}
			else current_note[4] = ""
			if (match(arraya[j],/[x#]+/))
				{
				accidentals = substr(arraya[j],RSTART,RLENGTH)
				gsub(/[x]/,"##",accidentals)
				current_note[3] = accidentals
				sub(/[x#]+/,"",arraya[j])
				}
			else if (match(arraya[j],/b+/))
				{
				accidentals = substr(arraya[j],RSTART,RLENGTH)
				gsub(/[b]/,"-",accidentals)
				current_note[3] = accidentals
				sub(/b+/,"",arraya[j])
				}
			else current_note[3] = ""
			if (match(arraya[j],/[ABCDEFG]/))
				{
				current_note[2] = substr(arraya[j],RSTART,RLENGTH)
				sub(/[ABCDEFG]/,"",arraya[j])
				}
			else not_found = TRUE
			if (match(arraya[j],octave_class))
				{
				current_note[1] = substr(arraya[j],RSTART,RLENGTH)
				sub(octave_class,"",arraya[j])
				}
			else current_note[1] = 4
			if (not_found) return_token = return_token original " "
			else
				{
				transpose(spine)
				return_token = return_token arraya[j] back_to_pitch() " "
				}
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#############################################################################
##					Function Back_to_pitch
#	This function converts the note in current_note back to pitch format
#
function back_to_pitch(  )
	{
	if (gsub("-","b",current_note[3]) == 0) gsub("##","x",current_note[3])
	return (current_note[2] current_note[3] current_note[1] current_note[4])
	}

###########################################################################
##					Function Process_Tonh
#	This function takes Tonh input and transposes it
#
function process_Tonh(data_token,spine,   return_token,arrayb,j,split_num,\
								      not_found,accidentals,original)
	{
	#
	# Process multiple stops
	#
	original = data_token
	return_token = ""
	split_num = split(data_token,arrayb," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If a rest occurs, simply echo it
		#
		if (arrayb[j] ~ /r/) return_token = return_token arrayb[j] " "
		#
		# Otherwise, process the Tonh note
		#
		else
			{
			not_found = FALSE
			#
			# Determine deviation
			#
			if (match(arrayb[j],deviation))
				{
				current_note[4] = substr(arrayb[j],RSTART,RLENGTH)
				sub(deviation,"",arrayb[j])
				}
			else current_note[4] = ""
			#
			# Deterimine any flats
			#
			if (match(arrayb[j],/(es)+/))
				{
				accidentals = substr(arrayb[j],RSTART,RLENGTH)
				gsub(/es/,"-",accidentals)
				current_note[3] = accidentals
				sub(/(es)+/,"",arrayb[j])
				}
			#
			# Determine any sharps
			#
			else if (match(arrayb[j],/(is)+/))
				{
				accidentals = substr(arrayb[j],RSTART,RLENGTH)
				gsub(/is/,"#",accidentals)
				current_note[3] = accidentals
				sub(/(is)+/,"",arrayb[j])
				}
			else current_note[3] = ""
			#
			# Determine the letter name
			#
			if (match(arrayb[j],/As/))
				{
				current_note[2] = "A"
				current_note[3] = current_note[3] "-"
				sub(/As/,"",arrayb[j])
				}
			else if (match(arrayb[j],/Es/))
				{
				current_note[2] = "E"
				current_note[3] = current_note[3] "-"
				sub(/Es/,"",arrayb[j])
				}
			else if (match(arrayb[j],/[ACDEFG]/))
				{
				current_note[2] = substr(arrayb[j],RSTART,RLENGTH)
				sub(/[ACDEFG]/,"",arrayb[j])
				}
			else if (match(arrayb[j],/H/))
				{
				if (current_note[3] == "-") not_found = TRUE
				else
					{
					current_note[2] = "B"
					sub(/H/,"",arrayb[j])
					}
				}
			else if (match(arrayb[j],/S/))
				{
				current_note[2] = "E"
				current_note[3] = current_note[3] "-"
				sub(/S/,"",arrayb[j])
				}
			else if (match(arrayb[j],/B/))
				{
				current_note[2] = "B"
				current_note[3] = current_note[3] "-"
				sub(/B/,"",arrayb[j])
				}
			else not_found = TRUE
			#
			# Determine the octave class
			#
			if (match(arrayb[j],octave_class))
				{
				current_note[1] = substr(arrayb[j],RSTART,RLENGTH)
				sub(octave_class,"",arrayb[j])
				}
			else current_note[1] = 4
			if (not_found) return_token = return_token original " "
			else
				{
				transpose(spine)
				return_token = return_token arrayb[j] back_to_Tonh() " "
				}
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#############################################################################
##					Function Back_to_Tonh
#	This function takes the note in current_note and converts it to Tonh format
#
function back_to_Tonh(  )
	{
	if (current_note[2] == "B" && current_note[3] == "-") current_note[3] = ""
	else if (current_note[2] == "B") current_note[2] = "H"
	else if (current_note[2] == "E" && current_note[3] ~ /-/)
		{
		current_note[2] = "Es"
		sub("-","",current_note[3])
		}
	else if (current_note[2] == "A" && current_note[3] ~ /-/)
		{
		current_note[2] = "As"
		sub("-","",current_note[3])
		}
	if (gsub("-","es",current_note[3]) == 0) gsub("#","is",current_note[3])
	return (current_note[2] current_note[3] current_note[1] current_note[4])
	}

###########################################################################
##					Function Process_solfg
#	This function takes solfg input and transposes it
#
function process_solfg(data_token,spine,  return_token,arrayc,j,split_num,\
									  not_found,accidentals,original)
	{
	#
	# Process multiple stops
	#
	original = data_token
	return_token = ""
	split_num = split(data_token,arrayc," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If a rest occurs, simply echo it
		#
		if (arrayc[j] ~ /r/ && arrayc[j] !~ /re/)
			return_token = return_token arrayc[j] " "
		#
		# Otherwise, process the solfg note
		#
		else
			{
			not_found = FALSE
			#
			# Determine deviation
			#
			if (match(arrayc[j],deviation))
				{
				current_note[4] = substr(arrayc[j],RSTART,RLENGTH)
				sub(deviation,"",arrayc[j])
				}
			else current_note[4] = ""
			#
			# Deterimine any flats
			#
			if (match(arrayc[j],/~b+/))
				{
				accidentals = substr(arrayc[j],RSTART+1,RLENGTH-1)
				gsub(/b/,"-",accidentals)
				current_note[3] = accidentals
				sub(/~b+/,"",arrayc[j])
				}
			#
			# Determine any sharps
			#
			else if (match(arrayc[j],/~d+/))
				{
				accidentals = substr(arrayc[j],RSTART+1,RLENGTH-1)
				gsub(/d/,"#",accidentals)
				current_note[3] = accidentals
				sub(/~d+/,"",arrayc[j])
				}
			else current_note[3] = ""
			#
			# Determine the letter name
			#
			if (match(arrayc[j],/do/))
				{ current_note[2] = "C" ; sub(/do/,"",arrayc[j]) }
			else if (match(arrayc[j],/re/))
				{ current_note[2] = "D" ; sub(/re/,"",arrayc[j]) }
			else if (match(arrayc[j],/mi/))
				{ current_note[2] = "E" ; sub(/mi/,"",arrayc[j]) }
			else if (match(arrayc[j],/fa/))
				{ current_note[2] = "F" ; sub(/fa/,"",arrayc[j]) }
			else if (match(arrayc[j],/sol/))
				{ current_note[2] = "G" ; sub(/sol/,"",arrayc[j]) }
			else if (match(arrayc[j],/la/))
				{ current_note[2] = "A" ; sub(/la/,"",arrayc[j]) }
			else if (match(arrayc[j],/si/))
				{ current_note[2] = "B" ; sub(/si/,"",arrayc[j]) }
			else not_found = TRUE
			#
			# Determine the octave class
			#
			if (match(arrayc[j],octave_class))
				{
				current_note[1] = substr(arrayc[j],RSTART,RLENGTH)
				sub(octave_class,"",arrayc[j])
				}
			else current_note[1] = 4
			if (not_found) return_token = return_token original " "
			else
				{
				transpose(spine)
				return_token = return_token arrayc[j] back_to_solfg() " "
				}
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#############################################################################
##					Function Back_to_solfg
#	This function converts the note in current_note back to solfg format
#
function back_to_solfg(  )
	{
	if (current_note[2] == "A") current_note[2] = "la"
	else if (current_note[2] == "B") current_note[2] = "si"
	else if (current_note[2] == "C") current_note[2] = "do"
	else if (current_note[2] == "D") current_note[2] = "re"
	else if (current_note[2] == "E") current_note[2] = "mi"
	else if (current_note[2] == "F") current_note[2] = "fa"
	else if (current_note[2] == "G") current_note[2] = "sol"
	if (current_note[3] ~ /-/)
		{
		sub("-","~b",current_note[3])
		gsub("-","b",current_note[3])
		}
	else
		{
		sub("#","~d",current_note[3])
		gsub("#","d",current_note[3])
		}
	return (current_note[2] current_note[3] current_note[1] current_note[4])
	}

###########################################################################
##					Function Process_kern
#	This function takes the current data token and transposes it.
#
function process_kern(data_token,spine,  return_token,arrayd,j,split_num,\
									  not_found,accidentals,original)
	{
	#
	# Process multiple stops
	#
	original = data_token
	return_token = ""
	split_num = split(data_token,arrayd," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If a rest occurs, simply echo it
		#
		if (arrayd[j] ~ /r/ && arrayd[j] !~ /re/)
			return_token = return_token arrayd[j] " "
		#
		# Otherwise, process the kern note
		#
		else
			{
			not_found = FALSE
			current_note[4] = ""
			#
			# Save the initial and final signifiers surrounding
			# the pitch information.
			#
			split(arrayd[j],non_pitch_info,kern_pitch_token)
			#
			# Deterimine any flats
			#
			if (match(arrayd[j],/-+/))
				{
				current_note[3] = substr(arrayd[j],RSTART+1,RLENGTH-1)
				sub(/-+/,"",arrayd[j])
				}
			#
			# Determine any sharps
			#
			else if (match(arrayd[j],/#+/))
				{
				current_note[3] = substr(arrayd[j],RSTART+1,RLENGTH-1)
				sub(/#+/,"",arrayd[j])
				}
			else current_note[3] = ""
			#
			# Determine the letter name
			#
			if (arrayd[j] ~ /[aA]/) current_note[2] = "A"
			else if (arrayd[j] ~ /[bB]/) current_note[2] = "B"
			else if (arrayd[j] ~ /[cC]/) current_note[2] = "C"
			else if (arrayd[j] ~ /[dD]/) current_note[2] = "D"
			else if (arrayd[j] ~ /[eE]/) current_note[2] = "E"
			else if (arrayd[j] ~ /[fF]/) current_note[2] = "F"
			else if (arrayd[j] ~ /[gG]/) current_note[2] = "G"
			else not_found = TRUE
			#
			# Determine the ocatave class
			#
			if (arrayd[j] ~ /[ABCDEFG]/)
				current_note[1] = 4 - gsub(/[ABCDEFG]/,"",arrayd[j])
			else
				current_note[1] = 3 + gsub(/[abcdefg]/,"",arrayd[j])
			#
			# Remove any naturals
			#
			gsub("n","",arrayd[j])
			#
			# Return the appropriate token
			#
			if (not_found) return_token = return_token original " "
			else
				{
				transpose(spine)
				#return_token = return_token arrayd[j] back_to_kern() " "
				return_token = return_token non_pitch_info[1] back_to_kern() non_pitch_info[2] " "
				}
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#############################################################################
##					Function Back_to_kern
#	This function converts the note in current_note into kern format
#
function back_to_kern(  i,pitch)
	{
	if (current_note[1] >= 4)
		{
		current_note[2] = to_lower(current_note[2])
		pitch = current_note[2]
		for (i = 4; i < current_note[1]; i += 1)
			current_note[2] = current_note[2] pitch
		}
	else
		{
		pitch = current_note[2]
		for (i = 3; i > current_note[1]; i -= 1)
			current_note[2] = current_note[2] pitch
		}
	return (current_note[2] current_note[3])
	}
