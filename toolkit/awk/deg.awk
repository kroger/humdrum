######################################################################
##					DEG.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       	Description:
# May 10/94	Tim Racinsky		Added Error statement that requires a key to
#							be defined for a processed spine.
# June 9/94	Tim Racinsky		Modified parse_command to work with getopts
# June 23/94	Tim Racinsky		Modified solfg representation to include "-"
#
#
#	This program is used to convert pitch units to deg notes 
#
# Main Functions used:
#	Parse_command()		Store_new_interps()		Store_indicators()
#	Process_indicators()	Ins_array_pos()		Del_array_pos()
#	Exchange_spines()		Process_data()			Convert_to_deg()
#	To_upper()			Process_pitch()		Process_Tonh()
#	Process_solfg()		Process_kern()
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
#  -deviation:  defines equally tempered deviation
#  -indicators:  defines spine path indicators
#  -input_interps:  defines valid input interpretations
#  -key_reg:  defines valid key interpretation
#  -octave_class:  defines octave class
#  -kern_pitch:  defines valid kern pitch
#  -pitch_pitch:  defines valid **pitch pitch
#  -Tonh_pitch_reg:  defines valid Tonh pitch
#  -solfg_pitch_reg:  defines valid solfg pitch
#
#	ARRAYS:
#
#  -path_indicator and current_interp:  the standard arrays for
#   tracking interpretations.
#  -current_note: a global array used to store the current note. It holds
#   the letter name and semit value of the current note.
#  -key: a 2-dimensional array holding degrees of corresponding pitch
#   values for each common key.
#  -current_key: holds the current key of each of the currently active spines.
#  -kern_array:  contains kern pitches and corresponding semit values
#  -pitch_array:  contains pitch pitches and corresponding semit values
#  -Tonh_array:  contains Tonh pitches and corresponding semit values
#  -solfg_array:  contains solfg pitches and corresponding semit values
#  -Tonh_pitch:  converts Tonh to pitch pitches
#  -solfg_pitch_reg:  converts solfg to pitch pitches
#  -last_semits: holds semit value of note occuring in previous record
#
BEGIN {
	USAGE="\nUSAGE: deg -h                (Help Screen)\n       deg "\
		 "[-tx] [file ...]\n"
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
	deviation = "[+-][1-9][0-9]*"
	tie_reg = "\\]|\\[|_"
	cont_tie_reg = "\\]|_"
	indicators = "^(\\*\\+|\\*\\-|\\*\\^|\\*v|\\*x)$"
	input_interps = "^(\\*\\*pitch|\\*\\*Tonh|\\*\\*solfg|\\*\\*kern)$"
	# $ removed from following regex by Craig Sapp 20130407 so that
	# modal key signatures can be handled properly.
	#key_reg = "^\\*(([A-Ga-g](#?|-?))|(\\?)|(X)|(Cx)|(cx)|(Dx)):$"
	key_reg = "^\\*(([A-Ga-g](#?|-?))|(\\?)|(X)|(Cx)|(cx)|(Dx)):"
	pc_key_sig = "^\\*k\\[([a-g]((n)|(#)|(x+(#?))|(-)+))*\\]$"
	ph_key_sig = "^\\*K\\[([A-G][1-9]((n)|(#)|(x+(#?))|(-)+))*\\]$"
	kern_pitch = "a+|b+|c+|d+|e+|f+|g+|A+|B+|C+|D+|E+|F+|G+"
	pitch_pitch = "[A-G]"
	Tonh_pitch_reg = "Es|As|[A-GHS]"
	solfg_pitch_reg = "do|re|mi|fa|sol|la|si"
	octave_class = "[0-9]"
	#
	kern_array["a"] = 9  ; kern_array["b"] = 11 ; kern_array["c"] = 0;
	kern_array["d"] = 2  ; kern_array["e"] = 4  ; kern_array["f"] = 5;
	kern_array["g"] = 7  ; kern_array["A"] = -3 ; kern_array["B"] = -1;
	kern_array["C"] = -12; kern_array["D"] = -10; kern_array["E"] = -8;
	kern_array["F"] = -7 ; kern_array["G"] = -5 ;
	#
	pitch_array["A"] = 9; pitch_array["B"] = 11; pitch_array["C"] = 0;
	pitch_array["D"] = 2; pitch_array["E"] = 4 ; pitch_array["F"] = 5;
	pitch_array["G"] = 7;
	#
	Tonh_array["As"] = 8; Tonh_array["A"] = 9; Tonh_array["B"] = 10;
	Tonh_array["H"] = 11; Tonh_array["C"] = 0; Tonh_array["D"] = 2;
	Tonh_array["Es"] = 3; Tonh_array["S"] = 3; Tonh_array["E"] = 4;
	Tonh_array["F"] = 5 ; Tonh_array["G"] = 7;
	#
	Tonh_pitch["As"] = "A"; Tonh_pitch["A"] = "A"; Tonh_pitch["B"] = "B";
	Tonh_pitch["H"] = "B"; Tonh_pitch["C"] = "C"; Tonh_pitch["D"] = "D";
	Tonh_pitch["Es"] = "E"; Tonh_pitch["S"] = "E"; Tonh_pitch["E"] = "E";
	Tonh_pitch["F"] = "F"; Tonh_pitch["G"] = "G";
	#
	solfg_array["la"] = 9; solfg_array["si"] = 11; solfg_array["do"] = 0;
	solfg_array["re"] = 2; solfg_array["mi"] = 4 ; solfg_array["fa"] = 5;
	solfg_array["sol"] = 7;
	#
	solfg_pitch["do"] = "C"; solfg_pitch["re"] = "D"; solfg_pitch["mi"] = "E";
	solfg_pitch["fa"] = "F"; solfg_pitch["sol"] = "G"; solfg_pitch["la"] = "A";
	solfg_pitch["si"] = "B";
	#
	# Initialize the key array
	#
	key["*C:","C"] = "1/0"; key["*C:","D"] = "2/2"; key["*C:","E"] = "3/4";
	key["*C:","F"] = "4/5"; key["*C:","G"] = "5/7"; key["*C:","A"] = "6/9";
	key["*C:","B"] = "7/11";

	key["*c:","C"] = "1/0"; key["*c:","D"] = "2/2"; key["*c:","E"] = "3/3";
	key["*c:","F"] = "4/5"; key["*c:","G"] = "5/7"; key["*c:","A"] = "6/8";
	key["*c:","B"] = "7/11";

	key["*C#:","C"] = "1/1"; key["*C#:","D"] = "2/3"; key["*C#:","E"] = "3/5";
	key["*C#:","F"] = "4/6"; key["*C#:","G"] = "5/8"; key["*C#:","A"] = "6/10";
	key["*C#:","B"] = "7/0";

	key["*c#:","C"] = "1/1"; key["*c#:","D"] = "2/3"; key["*c#:","E"] = "3/4";
	key["*c#:","F"] = "4/6"; key["*c#:","G"] = "5/8"; key["*c#:","A"] = "6/9";
	key["*c#:","B"] = "7/0";

	key["*Cx:","C"] = "1/2"; key["*Cx:","D"] = "2/4"; key["*Cx:","E"] = "3/6";
	key["*Cx:","F"] = "4/7"; key["*Cx:","G"] = "5/9"; key["*Cx:","A"] = "6/11";
	key["*Cx:","B"] = "7/1";

	key["*cx:","C"] = "1/2"; key["*cx:","D"] = "2/4"; key["*cx:","E"] = "3/5";
	key["*cx:","F"] = "4/7"; key["*cx:","G"] = "5/9"; key["*cx:","A"] = "6/10";
	key["*cx:","B"] = "7/1";
	
	key["*C-:","C"] = "1/11"; key["*C-:","D"] = "2/1"; key["*C-:","E"] = "3/3";
	key["*C-:","F"] = "4/4"; key["*C-:","G"] = "5/6"; key["*C-:","A"] = "6/8";
	key["*C-:","B"] = "7/10";

	key["*c-:","C"] = "1/11"; key["*c-:","D"] = "2/1"; key["*c-:","E"] = "3/2";
	key["*c-:","F"] = "4/4"; key["*c-:","G"] = "5/6"; key["*c-:","A"] = "6/7";
	key["*c-:","B"] = "7/10";

	key["*D:","C"] = "7/1"; key["*D:","D"] = "1/2"; key["*D:","E"] = "2/4";
	key["*D:","F"] = "3/6"; key["*D:","G"] = "4/7"; key["*D:","A"] = "5/9";
	key["*D:","B"] = "6/11";

	key["*d:","C"] = "7/1"; key["*d:","D"] = "1/2"; key["*d:","E"] = "2/4";
	key["*d:","F"] = "3/5"; key["*d:","G"] = "4/7"; key["*d:","A"] = "5/9";
	key["*d:","B"] = "6/10";

	key["*D#:","C"] = "7/2"; key["*D#:","D"] = "1/3"; key["*D#:","E"] = "2/5";
	key["*D#:","F"] = "3/7"; key["*D#:","G"] = "4/8"; key["*D#:","A"] = "5/10";
	key["*D#:","B"] = "6/0";

	key["*Dx:","C"] = "7/3"; key["*Dx:","D"] = "1/4"; key["*Dx:","E"] = "2/6";
	key["*Dx:","F"] = "3/8"; key["*Dx:","G"] = "4/9"; key["*Dx:","A"] = "5/11";
	key["*Dx:","B"] = "6/1";

	key["*d#:","C"] = "7/2"; key["*d#:","D"] = "1/3"; key["*d#:","E"] = "2/5";
	key["*d#:","F"] = "3/6"; key["*d#:","G"] = "4/8";
	key["*d#:","A"] = "5/10";
	key["*d#:","B"] = "6/11";

	key["*D-:","C"] = "7/0"; key["*D-:","D"] = "1/1"; key["*D-:","E"] = "2/3";
	key["*D-:","F"] = "3/5"; key["*D-:","G"] = "4/6"; key["*D-:","A"] = "5/8";
	key["*D-:","B"] = "6/10";

	key["*d-:","C"] = "7/0"; key["*d-:","D"] = "1/1"; key["*d-:","E"] = "2/3";
	key["*d-:","F"] = "3/4"; key["*d-:","G"] = "4/6"; key["*d-:","A"] = "5/8";
	key["*d-:","B"] = "6/9";

	key["*E:","C"] = "6/1"; key["*E:","D"] = "7/3"; key["*E:","E"] = "1/4";
	key["*E:","F"] = "2/6"; key["*E:","G"] = "3/8"; key["*E:","A"] = "4/9";
	key["*E:","B"] = "5/11";

	key["*e:","C"] = "6/0"; key["*e:","D"] = "7/3"; key["*e:","E"] = "1/4";
	key["*e:","F"] = "2/6"; key["*e:","G"] = "3/7"; key["*e:","A"] = "4/9";
	key["*e:","B"] = "5/11";

	key["*E#:","C"] = "6/2"; key["*E#:","D"] = "7/4"; key["*E#:","E"] = "1/5";
	key["*E#:","F"] = "2/7"; key["*E#:","G"] = "3/9"; key["*E#:","A"] = "4/10";
	key["*E#:","B"] = "5/0";

	key["*e#:","C"] = "6/1"; key["*e#:","D"] = "7/4"; key["*e#:","E"] = "1/5";
	key["*e#:","F"] = "2/7"; key["*e#:","G"] = "3/8"; key["*e#:","A"] = "4/10";
	key["*e#:","B"] = "5/0";

	key["*E-:","C"] = "6/0"; key["*E-:","D"] = "7/2"; key["*E-:","E"] = "1/3";
	key["*E-:","F"] = "2/5"; key["*E-:","G"] = "3/7"; key["*E-:","A"] = "4/8";
	key["*E-:","B"] = "5/10";

	key["*e-:","C"] = "6/1"; key["*e-:","D"] = "7/2"; key["*e-:","E"] = "1/3";
	key["*e-:","F"] = "2/5"; key["*e-:","G"] = "3/6"; key["*e-:","A"] = "4/8";
	key["*e-:","B"] = "5/10";

	key["*F:","C"] = "5/0"; key["*F:","D"] = "6/2"; key["*F:","E"] = "7/4";
	key["*F:","F"] = "1/5"; key["*F:","G"] = "2/7"; key["*F:","A"] = "3/9";
	key["*F:","B"] = "4/10";

	key["*f:","C"] = "5/0"; key["*f:","D"] = "6/1"; key["*f:","E"] = "7/4";
	key["*f:","F"] = "1/5"; key["*f:","G"] = "2/7"; key["*f:","A"] = "3/8";
	key["*f:","B"] = "4/10";

	key["*F#:","C"] = "5/1"; key["*F#:","D"] = "6/3"; key["*F#:","E"] = "7/5";
	key["*F#:","F"] = "1/6"; key["*F#:","G"] = "2/8"; key["*F#:","A"] = "3/10";
	key["*F#:","B"] = "4/11";

	key["*f#:","C"] = "5/1"; key["*f#:","D"] = "6/2"; key["*f#:","E"] = "7/5";
	key["*f#:","F"] = "1/6"; key["*f#:","G"] = "2/8"; key["*f#:","A"] = "3/9";
	key["*f#:","B"] = "4/11";

	key["*F-:","C"] = "5/11"; key["*F-:","D"] = "6/1"; key["*F-:","E"] = "7/3";
	key["*F-:","F"] = "1/4"; key["*F-:","G"] = "2/6"; key["*F-:","A"] = "3/8";
	key["*F-:","B"] = "4/9";

	key["*f-:","C"] = "5/11"; key["*f-:","D"] = "6/0"; key["*f-:","E"] = "7/3";
	key["*f-:","F"] = "1/4"; key["*f-:","G"] = "2/6"; key["*f-:","A"] = "3/7";
	key["*f-:","B"] = "4/9"; 

	key["*G:","C"] = "4/0"; key["*G:","D"] = "5/2"; key["*G:","E"] = "6/4";
	key["*G:","F"] = "7/6"; key["*G:","G"] = "1/7"; key["*G:","A"] = "2/9";
	key["*G:","B"] = "3/11";

	key["*g:","C"] = "4/0"; key["*g:","D"] = "5/2"; key["*g:","E"] = "6/3";
	key["*g:","F"] = "7/6"; key["*g:","G"] = "1/7"; key["*g:","A"] = "2/9";
	key["*g:","B"] = "3/10";

	key["*G#:","C"] = "4/1"; key["*G#:","D"] = "5/3"; key["*G#:","E"] = "6/5";
	key["*G#:","F"] = "7/7"; key["*G#:","G"] = "1/8"; key["*G#:","A"] = "2/10";
	key["*G#:","B"] = "3/0";

	key["*g#:","C"] = "4/1"; key["*g#:","D"] = "5/3"; key["*g#:","E"] = "6/4";
	key["*g#:","F"] = "7/7"; key["*g#:","G"] = "1/8"; key["*g#:","A"] = "2/10";
	key["*g#:","B"] = "3/11";

	key["*G-:","C"] = "4/11"; key["*G-:","D"] = "5/1"; key["*G-:","E"] = "6/3";
	key["*G-:","F"] = "7/5"; key["*G-:","G"] = "1/6"; key["*G-:","A"] = "2/8";
	key["*G-:","B"] = "3/10";

	key["*g-:","C"] = "4/11"; key["*g-:","D"] = "5/1"; key["*g-:","E"] = "6/2";
	key["*g-:","F"] = "7/5"; key["*g-:","G"] = "1/6"; key["*g-:","A"] = "2/8";
	key["*g-:","B"] = "3/9";

	key["*A:","C"] = "3/1"; key["*A:","D"] = "4/2"; key["*A:","E"] = "5/4";
	key["*A:","F"] = "6/6"; key["*A:","G"] = "7/8"; key["*A:","A"] = "1/9";
	key["*A:","B"] = "2/11";

	key["*a:","C"] = "3/0"; key["*a:","D"] = "4/2"; key["*a:","E"] = "5/4";
	key["*a:","F"] = "6/5"; key["*a:","G"] = "7/8"; key["*a:","A"] = "1/9";
	key["*a:","B"] = "2/11";

	key["*A#:","C"] = "3/2"; key["*A#:","D"] = "4/3"; key["*A#:","E"] = "5/5";
	key["*A#:","F"] = "6/7"; key["*A#:","G"] = "7/9"; key["*A#:","A"] = "1/10";
	key["*A#:","B"] = "2/0";

	key["*a#:","C"] = "3/1"; key["*a#:","D"] = "4/3"; key["*a#:","E"] = "5/5";
	key["*a#:","F"] = "6/6"; key["*a#:","G"] = "7/9"; key["*a#:","A"] = "1/10";
	key["*a#:","B"] = "2/0";

	key["*A-:","C"] = "3/0"; key["*A-:","D"] = "4/1"; key["*A-:","E"] = "5/3";
	key["*A-:","F"] = "6/5"; key["*A-:","G"] = "7/7"; key["*A-:","A"] = "1/8";
	key["*A-:","B"] = "2/10";

	key["*a-:","C"] = "3/11"; key["*a-:","D"] = "4/1"; key["*a-:","E"] = "5/3";
	key["*a-:","F"] = "6/4"; key["*a-:","G"] = "7/7"; key["*a-:","A"] = "1/8";
	key["*a-:","B"] = "2/10";

	key["*B:","C"] = "2/1"; key["*B:","D"] = "3/3"; key["*B:","E"] = "4/4";
	key["*B:","F"] = "5/6"; key["*B:","G"] = "6/8"; key["*B:","A"] = "7/10";
	key["*B:","B"] = "1/11";

	key["*b:","C"] = "2/1"; key["*b:","D"] = "3/2"; key["*b:","E"] = "4/4";
	key["*b:","F"] = "5/6"; key["*b:","G"] = "6/7"; key["*b:","A"] = "7/10";
	key["*b:","B"] = "1/11";

	key["*B#:","C"] = "2/2"; key["*B#:","D"] = "3/4"; key["*B#:","E"] = "4/5";
	key["*B#:","F"] = "5/7"; key["*B#:","G"] = "6/9"; key["*B#:","A"] = "7/11";
	key["*B#:","B"] = "1/0";

	key["*b#:","C"] = "2/2"; key["*b#:","D"] = "3/3"; key["*b#:","E"] = "4/5";
	key["*b#:","F"] = "5/7"; key["*b#:","G"] = "6/8"; key["*b#:","A"] = "7/11";
	key["*b#:","B"] = "1/0";

	key["*B-:","C"] = "2/0"; key["*B-:","D"] = "3/2"; key["*B-:","E"] = "4/3";
	key["*B-:","F"] = "5/5"; key["*B-:","G"] = "6/7"; key["*B-:","A"] = "7/9";
	key["*B-:","B"] = "1/10";

	key["*b-:","C"] = "2/0"; key["*b-:","D"] = "3/1"; key["*b-:","E"] = "4/3";
	key["*b-:","F"] = "5/5"; key["*b-:","G"] = "6/6"; key["*b-:","A"] = "7/9";
	key["*b-:","B"] = "1/10";
	#
	# Determine user's specified options
	#
	options = ""
	parse_command()
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
			print $0
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
##			FUNCTIONS USED IN DEG.AWK
##################################################################

##################################################################
##			Function Parse_command
#	This function checks that the input passed from deg.ksh
# contains a list of valid options and assigns it to the variable
# options if that is the case.
#
function parse_command()
	{
	if (ARGV[2] != "null") options = ARGV[2]
	ARGV[1] = ARGV[2] = ""
	}

################################################################
##				Function Store_new_interps
#	This function stores the new interpretations found in an
# interpretation record for each spine.
#
function store_new_interps(      j,interp_line)
	{
	interp_line = ""
	for (j = 1; j <= current_no_of_spines; j += 1)
		{
		#
		# Store exclusive interpretations and key interpretations
		#
		if ($j ~ /^\*\*/)
			{
			current_interp[j] = $j
			current_key[j] = ""
			last_semits[j] = "x"
			}
		else if ($j ~ key_reg) {
			current_key[j] = $j
			sub(/:.../, ":", current_key[j])
			}
		else if ($j ~ /^\*[kK]\[/)
			{
			if ($j !~ pc_key_sig && $j !~ ph_key_sig)
				{
				print "deg: ERROR: Invalid key signature interpretati"\
					 "on in spine " j ", line " FNR "." > stderr
				exit
				}
			}
		#
		# If the interpretation is a 'convertible' interpretation, then
		# the output line must show deg.
		#
		if ($j ~ input_interps) interp_line = interp_line "**deg\t"
		else interp_line = interp_line $j "\t"
		}
	#
	# Print the new interpretation line
	#
	print substr(interp_line,1,length(interp_line)-1)
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
			last_semits[i+1] = last_semits[i]
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
			last_semits[i+1] = "x"
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
		last_semits[j] = last_semits[j-1]
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
		last_semits[j] = last_semits[j+1]
		}
	delete path_indicator[j]
	delete current_interp[j]
	delete current_key[j]
	delete last_semits[j]
	current_no_of_spines -= 1
	} 

###################################################################
##				Function Exchange_spines
#	This function exhanges two spines by exchanging the corresponding
# elements in current_interp.
#
function exchange_spines(    j,count,ex_array,temp)
	{
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
	temp = current_interp[ex_array[1]]
	current_interp[ex_array[1]] = current_interp[ex_array[2]]
	current_interp[ex_array[2]] = temp
	temp = current_key[ex_array[1]]
	current_key[ex_array[1]] = current_key[ex_array[2]]
	current_key[ex_array[2]] = temp
	temp = last_semits[ex_array[1]]
	last_semits[ex_array[1]] = last_semits[ex_array[2]]
	last_semits[ex_array[2]] = temp
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
			# For all other tokens
			#
			else
				{
				#
				# Each spine to be processed must also have a key defined
				# for that spine (with tandem interpretation)
				#
				if (current_key[j] == "")
					{
					print "deg: ERROR: Cannot process pitches without " \
					"key information." > stderr
					exit
					}
				#
				# Call the appropriate function based on the exclusive 
				# interpretation in order to obtain deg output.
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
##				Function Convert_to_deg
#	This function converts the note in current_note to a deg value and
# returns it to the calling function.
#
function convert_to_deg(spine,  note,deg,semits,sign,relative,temp_semits)
	{
	#
	# Depending on the -a option, determine if the current note is higher
	# or lower than the previous note
	#
	if (options ~ /a/) relative = ""
	else
		{
		if (last_semits[spine] == "x" || current_note[2] == last_semits[spine])
			relative = ""
		else if (current_note[2] > last_semits[spine])
			relative = "^"
		else if (current_note[2] < last_semits[spine])
			relative = "v"
		}
	#
	# Determine the pitch spelling from the current key
	#
	if (current_key[spine] ~ key_reg)
		split(key[current_key[spine],current_note[1]],note,"/")
	else
		split(key["*C:",current_note[1]],note,"/")
	deg = note[1]
	semits = note[2]
	#
	# Determine if the current note is rasied or lowered in the current key
	#
	temp_semits = (((current_note[2] + 0) % 12) + 12) % 12
	if (semits > temp_semits) sign = "-"
	else if (semits < temp_semits) sign = "+"
	last_semits[spine] = current_note[2]
	return (relative deg sign)
	}

########################################################################
##				Function to_upper
#	This function converts a character to upper case.
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

#########################################################################
##				Function Process_pitch
#	This function accepts pitch input and translates to deg
#
function process_pitch(data_token,position,  new,return_token,arraya,j,found\
													,split_num)
	{
	#
	# Process multiple stops
	#
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
			current_note[2] = 0
			found = TRUE
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arraya[j],pitch_pitch))
				{
				current_note[1] = substr(arraya[j],RSTART,1)
				current_note[2] = pitch_array[current_note[1]]
				sub(pitch_pitch,SUBSEP,arraya[j])
				}
			else found = FALSE
			#
			# If there was a letter name, continue to process
			#
			if (found)
				{
				#
				# Determine if there are any flats or sharps
				#
				if (match(arraya[j],/b+/))
					{ current_note[2] -= RLENGTH ; sub(/b+/,"",arraya[j]) }
				else
					{
					if (match(arraya[j],/x+/))
						{
						current_note[2] += 2 * RLENGTH
						sub(/x+/,"",arraya[j])
						}
					if (match(arraya[j],/#+/))
						{
						current_note[2] += RLENGTH
						sub(/#+/,"",arraya[j])
						}
					}
				#
				# Determine any equally-tempered deviations
				#
				if (match(arraya[j],deviation))
					{
					#
					# The current program does not account for equally
					# tempered deviations
					#current_note[2] \
					#		+= (substr(arraya[j],RSTART,RLENGTH) * 0.01)
					sub(deviation,"",arraya[j])
					}
				#
				# Determine which octave the note is in
				#
				if (match(arraya[j],octave_class)) 
					{
					current_note[2] \
						+= (12 * (substr(arraya[j],RSTART,1) + 0)) - 48
					sub(octave_class,"",arraya[j])
					}
				new = convert_to_deg(position)
				sub(SUBSEP,new,arraya[j])
				return_token = return_token arraya[j] " "
				}
			#
			# If no letter name, return appropriate token depending on
			# -x option
			#
			else return_token = return_token arraya[j] " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}
	
###########################################################################
##					Function Process_Tonh
#	This function takes Tonh input and translates to solfa
#
function process_Tonh(data_token,position,  found,arrayb,j,return_token\
												,split_num,new)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayb," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Echo any rests that occur in the data token
		#
		if (arrayb[j] ~ /r/) return_token = return_token arrayb[j] " "
		#
		# Otherwise, process the Tonh note
		#
		else
			{
			current_note[2] = 0
			found = TRUE
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arrayb[j],Tonh_pitch_reg))
				{
				current_note[1] = substr(arrayb[j],RSTART,RLENGTH)
				current_note[2] = Tonh_array[current_note[1]]
				current_note[1] = Tonh_pitch[current_note[1]]
				sub(Tonh_pitch_reg,SUBSEP,arrayb[j])
				}
			else found = FALSE
			#
			# If a letter name was found, continue to process
			#
			if (found)
				{
				#
				# Determine the number of flats and sharps
				#
				if (match(arrayb[j],/(es)+/))
					{
					current_note[2] -= RLENGTH/2
					sub(/(es)+/,"",arrayb[j])
					}
				else if (match(arrayb[j],/(is)+/))
					{
					current_note[2] += RLENGTH/2
					sub(/(is)+/,"",arrayb[j])
					}
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayb[j],deviation))
					{
					#
					# The current program does not account for equally
					# tempered deviations
					#current_note[2] \
					#	+= (substr(arrayb[j],RSTART,RLENGTH) * 0.01)
					sub(deviation,"",arrayb[j])
					}
				#
				# Determine which octave the note is in
				#
				if (match(arrayb[j],octave_class)) 
					{
					current_note[2] \
						+= (12 * (substr(arrayb[j],RSTART,1) + 0)) - 48
					sub(octave_class,"",arrayb[j])
					}
				new = convert_to_deg(position)
				sub(SUBSEP,new,arrayb[j])
				return_token = return_token arrayb[j] " "
				}
			#
			# If no letter name, the return token depends on the -x option
			#
			else return_token = return_token arrayb[j] " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

###########################################################################
##					Function Process_solfg
#	This function takes solfg input and translates to solfa
#
function process_solfg(data_token,position,   found,arrayc,j,return_token,new\
													,split_num)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayc," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Echo any rests in the input.  Be sure to check for 're'
		#
		if (arrayc[j] ~ /r/ && arrayc[j] !~ /re/)
			return_token = return_token arrayc[j] " "
		#
		# Otherwise, process the solfg note
		#
		else
			{
			current_note[2] = 0
			found = TRUE
			#
			# Determine the letter name of the note and store and remove it
			#
			if (match(arrayc[j],solfg_pitch_reg))
				{
				current_note[1] = substr(arrayc[j],RSTART,RLENGTH)
				current_note[2] = solfg_array[current_note[1]]
				current_note[1] = solfg_pitch[current_note[1]]
				sub(solfg_pitch_reg,SUBSEP,arrayc[j])
				}
			else found = FALSE
			#
			# If a letter name was found, continue to process
			#
			if (found)
				{
				#
				# Determine if there are any flats or sharps
				#
				if (match(arrayc[j],/~b+/))
					{
					current_note[2] -= (RLENGTH - 1)
					sub(/~b+/,"",arrayc[j])
					}
				else if (match(arrayc[j],/~d+/))
					{
					current_note[2] += (RLENGTH - 1)
					sub(/~d+/,"",arrayc[j])
					}
				#
				# Determine any equally-tempered deviations
				#
				if (match(arrayc[j],deviation))
					{
					#
					# The current program does not account for equally
					# tempered deviations
					#current_note[2] \
					#		+= (substr(arrayc[j],RSTART,RLENGTH) * 0.01)
					sub(deviation,"",arrayc[j])
					}
				#
				# Determine which octave the note is in
				#
				if (match(arrayc[j],octave_class)) 
					{
					current_note[2] \
						+= (12 * (substr(arrayc[j],RSTART,1) + 0)) - 48
					sub(octave_class,"",arrayc[j])
					}
				new = convert_to_deg(position)
				sub(SUBSEP,new,arrayc[j])
				return_token = return_token arrayc[j] " "
				}
			#
			# If no letter name was found, return token depends on -x option
			#
			else return_token = return_token arrayc[j] " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

####################################################################
##				Function Process_kern
#	This function takes kern input and translates to solfa
#
function process_kern(data_token,position,  arrayd,j,return_token,found,new\
													,split_num)
	{
	# Remove all duration information.
	#
	gsub("[0-9.]","",data_token)
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayd," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Process ties if -t option specified
		#
		if (options ~ /t/ && arrayd[j] ~ cont_tie_reg)
			{ if (split_num == 1) return_token = "." }
		#
		# Simply output any rests
		#
		else if (arrayd[j] ~ /r/) return_token = return_token arrayd[j] " "
		#
		# Otherwise, process the note
		#
		else
			{
			if (options !~ /p/ && arrayd[j] ~ tie_reg)
				sub(tie_reg,"",arrayd[j])
			found = TRUE
			current_note[2] = 0
			#
			# First find the letter name of the note; store and remove it
			#
			if (match(arrayd[j],kern_pitch))
				{
				current_note[1] = substr(arrayd[j],RSTART,1)
				if (current_note[1] ~ /[abcdefg]/)
					{
					current_note[2] \
						= kern_array[current_note[1]]+(12*(RLENGTH-1))
					current_note[1] = to_upper(current_note[1])
					}
				else current_note[2] \
						= kern_array[current_note[1]]-(12*(RLENGTH-1))
				sub(kern_pitch,SUBSEP,arrayd[j])
				}
			else found = FALSE
			#
			# If a letter name was found, continue to process
			#
			if (found)
				{
				#
				# Next determine the value of any flats or sharps
				#
				if (match(arrayd[j],/-+/))
					{ current_note[2] -= RLENGTH ; sub(/-+/,"",arrayd[j]) }
				if (match(arrayd[j],/#+/))
					{ current_note[2] += RLENGTH ; sub(/#+/,"",arrayd[j]) }
				gsub("n","",arrayd[j])
				new = convert_to_deg(position)
				sub(SUBSEP,new,arrayd[j])
				return_token = return_token arrayd[j] " "
				}
			#
			# If no letter name, return proper token depending on -x option
			#
			else return_token = return_token arrayd[j] " "
			}
		}
	#
	# Return at least a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}
