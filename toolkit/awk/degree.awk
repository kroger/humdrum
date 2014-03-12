######################################################################
##					DEGREE.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       	Description:
# May 10/94	Tim Racinsky		Added Error statement that requires each
#							spine have a key defined for it
# June 9/94	Tim Racinsky	Modified to work with getopts
# June 23/94	Tim Racinsky	Modified solfg representation to include "-"
#
#
#	This program is used to convert pitch units to degree notes 
#
# Main Functions used:
#	Parse_command()		Store_new_interps()		Store_indicators()
#	Process_indicators()	Ins_array_pos()		Del_array_pos()
#	Exchange_spines()		Process_data()			Convert_to_degree()
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
#  -middle_C_freq:  holds the frequency of middle c.
#  -uftp:  defines unsigned floating pt number
#  -deviation:  defines equally tempered deviation
#  -indicators:  defines spine path indicators
#  -input_interps:  defines valid input interpretations
#  -key_reg:  defines valid key interpretation
#  -pc_key_sig:  defines pitch class key signature interpretation
#  -repetition:  defines invalid repetition in above interpretation
#  -ph_key_sig:  defines pitch height key signature interpretation
#  -pc_note:  holds pitch class note
#  -pitch_reg:  defines pitch regular expression
#  -ph_note:  holds pitch height note
#  -pitch_class:  defines pitch class
#  -octave_class:  defines octave class
#  -pitch_pitch:  defines valid pitch pitch
#  -solfg_pitch:  defines valid solfg pitch
#
#	ARRAYS:
#
#  -path_indicator and current_interp:  the standard arrays for
#   tracking interpretations.
#  -current_note: a global array used to store the current note. It holds
#   the ocatave class, letter name, accidentals, and equally tempered deviation.
#  -key: a 2-dimensional array holding degree of corresponding pitch
#   values for each common key.
#  -current_key: holds the current key of each of the currently active spines.
#  -pitch_array:  contains pitch pitches and corresponding semit values
#  -solfg_array:  contains solfg pitches and corresponding semit values
#
BEGIN {
	USAGE="\nUSAGE: degree -h                (Help Screen)\n       degree "\
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
	middle_C_freq = 261.625549
	floating_pt_num = "[+-]?(([0-9]+(\\.[0-9]*)?)|((0*)?\\.[0-9]*))"
	deviation = "[+-][1-9][0-9]*"
	tie_reg = "\\]|\\[|_"
	cont_tie_reg = "\\]|_"
	indicators = "^(\\*\\+|\\*\\-|\\*\\^|\\*v|\\*x)$"
	input_interps = "^(\\*\\*pitch|\\*\\*Tonh|\\*\\*solfg|\\*\\*kern)$"
	# $ removed from following regex by Craig Sapp 20140311 so that
	# modal key designations (such as *G:mix) can be handled gracefully.
	#key_reg = "^\\*(([A-Ga-g](#?|-?))|(\\?)|(X)|(Cx)|(cx)|(Dx)):$"
	key_reg = "^\\*(([A-Ga-g](#?|-?))|(\\?)|(X)|(Cx)|(cx)|(Dx)):"
	pc_key_sig = "^\\*k\\[([a-g]((n)|(#)|(x+(#?))|(-)+))*\\]$"
	ph_key_sig = "^\\*K\\[([A-G][1-9]((n)|(#)|(x+(#?))|(-)+))*\\]$"
	kern_pitch = "a+|b+|c+|d+|e+|f+|g+|A+|B+|C+|D+|E+|F+|G+"
	pitch_pitch = "[A-G]"
	solfg_pitch = "do|re|mi|fa|sol|la|si"
	octave_class = "[0-9]"
	#
	solfg_array["do"] = "C"; solfg_array["re"] = "D" ; solfg_array["mi"] = "E";
	solfg_array["fa"] = "F"; solfg_array["sol"] = "G"; solfg_array["la"] = "A";
	solfg_array["si"] = "B";
	#
	# Initialize the key array
	#
	key["*C:","C"] = "1"; key["*C:","D"] = "2"; key["*C:","E"] = "3";
	key["*C:","F"] = "4"; key["*C:","G"] = "5"; key["*C:","A"] = "6";
	key["*C:","B"] = "7";
	
	key["*c:","C"] = "1"; key["*c:","D"] = "2"; key["*c:","E"] = "3/-";
	key["*c:","F"] = "4"; key["*c:","G"] = "5"; key["*c:","A"] = "6/-";
	key["*c:","B"] = "7";
	
	key["*C#:","C"] = "1/#"; key["*C#:","D"] = "2/#"; key["*C#:","E"] = "3/#";
	key["*C#:","F"] = "4/#"; key["*C#:","F"] = "5/#"; key["*C#:","A"] = "6/#";
	key["*C#:","B"] = "7/#";
	
	key["*c#:","C"] = "1/#"; key["*c#:","D"] = "2/#"; key["*c#:","E"] = "3";
	key["*c#:","F"] = "4/#"; key["*c#:","G"] = "5/#"; key["*c#:","A"] = "6";
	key["*c#:","B"] = "7/#";
	
	key["*Cx:","C"] = "1/##";key["*Cx:","D"] = "2/##";key["*Cx:","E"] = "3/##";
	key["*Cx:","F"] = "4/##";key["*Cx:","G"] = "5/##";key["*Cx:","A"] = "6/##";
	key["*Cx:","B"] = "7/##";
	
	key["*cx:","C"] = "1/##"; key["*cx:","D"] = "2/##";key["*cx:","E"] = "3/#";
	key["*cx:","F"] = "4/##"; key["*cx:","G"] = "5/##";key["*cx:","A"] = "6/#";
	key["*cx:","B"] = "7/##";
	
	key["*C-:","C"] = "1/-"; key["*C-:","D"] = "2/-"; key["*C-:","E"] = "3/-";
	key["*C-:","F"] = "4/-"; key["*C-:","G"] = "5/-"; key["*C-:","A"] = "6/-";
	key["*C-:","B"] = "7/-";
	
	key["*c-:","C"] = "1/-"; key["*c-:","D"] = "2/-"; key["*c-:","E"] = "3/--";
	key["*c-:","F"] = "4/-"; key["*c-:","G"] = "5/-"; key["*c-:","A"] = "6/--";
	key["*C-:","B"] = "7/-";
	
	key["*D:","C"] = "7/#"; key["*D:","D"] = "1"; key["*D:","E"] = "2";
	key["*D:","F"] = "3/#"; key["*D:","G"] = "4"; key["*D:","A"] = "5";
	key["*D:","B"] = "6";
	
	key["*d:","C"] = "7/#"; key["*d:","D"] = "1"; key["*d:","E"] = "2";
	key["*d:","F"] = "3"; key["*d:","G"] = "4"; key["*d:","A"] = "5";
	key["*d:","B"] = "6/-";
	
	key["*D#:","C"] = "7/##"; key["*D#:","D"] = "1/#"; key["*D#:","E"] = "2/#";
	key["*D#:","F"] = "3/##"; key["*D#:","G"] = "4/#"; key["*D#:","A"] = "5/#";
	key["*D#:","B"] = "6/#";
	
	key["*Dx:","C"] = "7/###"; key["*Dx:","D"] = "1/##";
	key["*Dx:","E"] = "2/##"; key["*Dx:","F"] = "3/###";
	key["*Dx:","G"] = "4/##"; key["*Dx:","A"] = "5/##";
	key["*Dx:","B"] = "6/##";

	key["*d#:","C"] = "7/##"; key["*d#:","D"] = "1/#"; key["*d#:","E"] = "2/#";
	key["*d#:","F"] = "3/#"; key["*d#:","G"] = "4/#"; key["*d#:","A"] = "5/#";
	key["*d#:","B"] = "6";
	
	key["*D-:","C"] = "7"; key["*D-:","D"] = "1/-"; key["*D-:","E"] = "2/-";
	key["*D-:","F"] = "3"; key["*D-:","G"] = "4/-"; key["*D-:","A"] = "5/-";
	key["*D-:","B"] = "6/-";
	
	key["*d-:","C"] = "7"; key["*d-:","D"] = "1/-"; key["*d-:","E"] = "2/-";
	key["*d-:","F"] = "3/-"; key["*d-:","G"] = "4/-"; key["*d-:","A"] = "5/-";
	key["*d-:","B"] = "6/--";
	
	key["*E:","C"] = "6/#"; key["*E:","D"] = "7/#"; key["*E:","E"] = "1";
	key["*E:","F"] = "2/#"; key["*E:","G"] = "3/#"; key["*E:","A"] = "4";
	key["*E:","B"] = "5";
	
	key["*e:","C"] = "6"; key["*e:","D"] = "7/#"; key["*e:","E"] = "1";
	key["*e:","F"] = "2/#"; key["*e:","G"] = "3"; key["*e:","A"] = "4";
	key["*e:","B"] = "5";
	
	key["*E#:","C"] = "6/##"; key["*E#:","D"] = "7/##";key["*E#:","E"] = "1/#";
	key["*E#:","F"] = "2/##"; key["*E#:","G"] = "3/##";key["*E#:","A"] = "4/#";
	key["*E#:","B"] = "5/#";
	
	key["*e#:","C"] = "6/#"; key["*e#:","D"] = "7/##"; key["*e#:","E"] = "1/#";
	key["*e#:","F"] = "2/##"; key["*e#:","G"] = "3/#"; key["*e#:","A"] = "4/#";
	key["*e#:","B"] = "5/#";
	
	key["*E-:","C"] = "6"; key["*E-:","D"] = "7"; key["*E-:","E"] = "1/-";
	key["*E-:","F"] = "2"; key["*E-:","G"] = "3"; key["*E-:","A"] = "4/-";
	key["*E-:","B"] = "5/-";
	
	key["*e-:","C"] = "6/-"; key["*e-:","D"] = "7"; key["*e-:","E"] = "1/-";
	key["*e-:","F"] = "2"; key["*e-:","G"] = "3/-"; key["*e-:","A"] = "4/-";
	key["*e-:","B"] = "5/-";
	
	key["*F:","C"] = "5"; key["*F:","D"] = "6"; key["*F:","E"] = "7";
	key["*F:","F"] = "1"; key["*F:","G"] = "2"; key["*F:","A"] = "3";
	key["*F:","B"] = "4/-";
	
	key["*f:","C"] = "5"; key["*f:","D"] = "6/-"; key["*f:","E"] = "7";
	key["*f:","F"] = "1"; key["*f:","G"] = "2"; key["*f:","A"] = "3/-";
	key["*f:","B"] = "4/-";
	
	key["*F#:","C"] = "5/#"; key["*F#:","D"] = "6/#"; key["*F#:","E"] = "7/#";
	key["*F#:","F"] = "1/#"; key["*F#:","G"] = "2/#"; key["*F#:","A"] = "3/#";
	key["*F#:","B"] = "4";
	
	key["*f#:","C"] = "5/#"; key["*f#:","D"] = "6"; key["*f#:","E"] = "7/#";
	key["*f#:","F"] = "1/#"; key["*f#:","G"] = "2/#"; key["*f#:","A"] = "3";
	key["*f#:","B"] = "4";
	
	key["*F-:","C"] = "5/-"; key["*F-:","D"] = "6/-"; key["*F-:","E"] = "7/-";
	key["*F-:","F"] = "1/-"; key["*F-:","G"] = "2/-"; key["*F-:","A"] = "3/-";
	key["*F-:","B"] = "4/--";
	
	key["*f-:","C"] = "5/-"; key["*f-:","D"] = "6/--"; key["*f-:","E"] = "7/-";
	key["*f-:","F"] = "1/-"; key["*f-:","G"] = "2/-"; key["*f-:","A"] = "3/--";
	key["*f-:","B"] = "4/--";
	
	key["*G:","C"] = "4"; key["*G:","D"] = "5"; key["*G:","E"] = "6";
	key["*G:","F"] = "7/#"; key["*G:","G"] = "1"; key["*G:","A"] = "2";
	key["*G:","B"] = "3";
	
	key["*g:","C"] = "4"; key["*g:","D"] = "5"; key["*g:","E"] = "6/-";
	key["*g:","F"] = "7/#"; key["*g:","G"] = "1"; key["*g:","A"] = "2";
	key["*g:","B"] = "3/-";
	
	key["*G#:","C"] = "4/#"; key["*G#:","D"] = "5/#"; key["*G#:","E"] = "6/#";
	key["*G#:","F"] = "7/##"; key["*G#:","G"] = "1/#"; key["*G#:","A"] = "2/#";
	key["*G#:","B"] = "3/#";
	
	key["*g#:","C"] = "4/#"; key["*g#:","D"] = "5/#"; key["*g#:","E"] = "6";
	key["*g#:","F"] = "7/##"; key["*g#:","G"] = "1/#"; key["*g#:","A"] = "2/#";
	key["*g#:","B"] = "3";
	
	key["*G-:","C"] = "4/-"; key["*G-:","D"] = "5/-"; key["*G-:","E"] = "6/-";
	key["*G-:","F"] = "7"; key["*G-:","G"] = "1/-"; key["*G-:","A"] = "2/-";
	key["*G-:","B"] = "3/-";
	
	key["*g-:","C"] = "4/-"; key["*g-:","D"] = "5/-"; key["*g-:","E"] = "6/--";
	key["*g-:","F"] = "7"; key["*g-:","G"] = "1/-"; key["*g-:","A"] = "2/-";
	key["*g-:","B"] = "3/--";
	
	key["*A:","C"] = "3/#"; key["*A:","D"] = "4"; key["*A:","E"] = "5";
	key["*A:","F"] = "6/#"; key["*A:","G"] = "7/#"; key["*A:","A"] = "1";
	key["*A:","B"] = "2";
	
	key["*a:","C"] = "3"; key["*a:","D"] = "4"; key["*a:","E"] = "5";
	key["*a:","F"] = "6"; key["*a:","G"] = "7/#"; key["*a:","A"] = "1";
	key["*a:","B"] = "2";
	
	key["*A#:","C"] = "3/##"; key["*A#:","D"] = "4/#"; key["*A#:","E"] = "5/#";
	key["*A#:","F"] = "6/##"; key["*A#:","G"] = "7/##";key["*A#:","A"] = "1/#";
	key["*A#:","B"] = "2/#";
	
	key["*a#:","C"] = "3/#"; key["*a#:","D"] = "4/#"; key["*a#:","E"] = "5/#";
	key["*a#:","F"] = "6/#"; key["*a#:","G"] = "7/##"; key["*a#:","A"] = "1/#";
	key["*a#:","B"] = "2/#";
	
	key["*A-:","C"] = "3"; key["*A-:","D"] = "4/-"; key["*A-:","E"] = "5/-";
	key["*A-:","F"] = "6"; key["*A-:","G"] = "7"; key["*A-:","A"] = "1/-";
	key["*A-:","B"] = "2/-";
	
	key["*a-:","C"] = "3/-"; key["*a-:","D"] = "4/-"; key["*a-:","E"] = "5/-";
	key["*a-:","F"] = "6/-"; key["*a-:","G"] = "7"; key["*a-:","A"] = "1/-";
	key["*a-:","B"] = "2/-";
	
	key["*B:","C"] = "2/#"; key["*B:","D"] = "3/#"; key["*B:","E"] = "4";
	key["*B:","F"] = "5/#"; key["*B:","G"] = "6/#"; key["*B:","A"] = "7/#";
	key["*B:","B"] = "1";
	
	key["*b:","C"] = "2/#"; key["*b:","D"] = "3"; key["*b:","E"] = "4";
	key["*b:","F"] = "5/#"; key["*b:","G"] = "6"; key["*b:","A"] = "7/#";
	key["*b:","B"] = "1";
	
	key["*B#:","C"] = "2/##"; key["*B#:","D"] = "3/##";key["*B#:","E"] = "4/#";
	key["*B#:","F"] = "5/##";key["*B#:","G"] = "6/##";key["*B#:","A"] = "7/##";
	key["*B#:","B"] = "1/#";

	key["*b#:","C"] = "2/##"; key["*b#:","D"] = "3/#"; key["*b#:","E"] = "4/#";
	key["*b#:","F"] = "5/##"; key["*b#:","G"] = "6/#";key["*b#:","A"] = "7/##";
	key["*b#:","B"] = "1/#";
	
	key["*B-:","C"] = "2"; key["*B-:","D"] = "3"; key["*B-:","E"] = "4/-";
	key["*B-:","F"] = "5"; key["*B-:","G"] = "6"; key["*B-:","A"] = "7";
	key["*B-:","B"] = "1/-";
	
	key["*b-:","C"] = "2"; key["*b-:","D"] = "3/-"; key["*b-:","E"] = "4/-";
	key["*b-:","F"] = "5"; key["*b-:","G"] = "6/-"; key["*b-:","A"] = "7";
	key["*b-:","B"] = "1/-";
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
##			FUNCTIONS USED IN DEGREE.AWK
##################################################################

##################################################################
##			Function Parse_command
#	This function checks that the input passed from degree.ksh
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
			}
		else if ($j ~ key_reg) {
			current_key[j] = $j
			sub(/:.../, ":", current_key[j])
			}
		else if ($j ~ /^\*[kK]\[/)
			{
			if ($j !~ pc_key_sig && $j !~ ph_key_sig)
				{
				print "degree: ERROR: Invalid key signature interpretati"\
					 "on in spine " j ", line " FNR "." > stderr
				exit
				}
			}
		#
		# If the interpretation is a 'convertible' interpretation, then
		# the output line must show degree.
		#
		if ($j ~ input_interps) interp_line = interp_line "**degree\t"
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
					print "degree: ERROR: Cannot process pitches " \
					"without key information." > stderr
					exit
					}
				#
				# Call the appropriate function based on the exclusive 
				# interpretation in order to obtain degree output.
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
##				Function Convert_to_degree
#	This function converts the note in current_note to a degree value and
# returns it to the calling function.
#
function convert_to_degree(spine,   note,degree,accid,change,sign)
	{
	#
	# Determine the pitch spelling from the current key
	#
	if (current_key[spine] ~ key_reg)
		split(key[current_key[spine],current_note[2]],note,"/")
	else split(key["*C:",current_note[2]],note,"/")
	degree = note[1]
	accid = note[2]
	if (current_note[3] ~ /#/) change = length(current_note[3])
	else change = length(current_note[3]) * -1
	if (accid ~ /#/) accid = length(accid)
	else accid = length(accid) * -1
	if (change > accid) sign = "+"
	else if (change < accid) sign = "-"
	else sign = ""
	return (degree sign "/" current_note[1])
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
#	This function accepts pitch input and translates to degree 
#
function process_pitch(data_token,position,  return_token,arraya,j,split_num\
								,not_found,accidentals,original,new)
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
			original = arraya[j]
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
			if (match(arraya[j],pitch_pitch))
				{
				current_note[2] = substr(arraya[j],RSTART,RLENGTH)
				sub(pitch_pitch,SUBSEP,arraya[j])
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
				new = convert_to_degree(position)
				sub(SUBSEP,new,arraya[j])
				return_token = return_token arraya[j] " "
				}
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
#	This function takes Tonh input and translates to degree
#
function process_Tonh(data_token,position,   return_token,arrayb,j,split_num,\
								not_found,accidentals,original,new)
	{
	#
	# Process multiple stops
	#
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
			original = arrayb[j]
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
				sub(/As/,SUBSEP,arrayb[j])
				}
			else if (match(arrayb[j],/Es/))
				{
				current_note[2] = "E"
				current_note[3] = current_note[3] "-"
				sub(/Es/,SUBSEP,arrayb[j])
				}
			else if (match(arrayb[j],/[ACDEFG]/))
				{
				current_note[2] = substr(arrayb[j],RSTART,RLENGTH)
				sub(/[ACDEFG]/,SUBSEP,arrayb[j])
				}
			else if (match(arrayb[j],/H/))
				{
				if (current_note[3] == "-") not_found = TRUE
				else
					{
					current_note[2] = "B"
					sub(/H/,SUBSEP,arrayb[j])
					}
				}
			else if (match(arrayb[j],/S/))
				{
				current_note[2] = "E"
				current_note[3] = current_note[3] "-"
				sub(/S/,SUBSEP,arrayb[j])
				}
			else if (match(arrayb[j],/B/))
				{
				current_note[2] = "B"
				current_note[3] = current_note[3] "-"
				sub(/B/,SUBSEP,arrayb[j])
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
				new = convert_to_degree(position)
				sub(SUBSEP,new,arrayb[j])
				return_token = return_token arrayb[j] " "
				}
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
#	This function takes solfg input and translates to degree
#
function process_solfg(data_token,position,  return_token,arrayc,j,split_num,\
							not_found,letter,accidentals,original,new)
	{
	#
	# Process multiple stops
	#
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
			original = arrayc[j]
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
			# Determine the letter name
			#
			if (match(arrayc[j],solfg_pitch))
				{
				letter = substr(arrayc[j],RSTART,RLENGTH)
				current_note[2] = solfg_array[letter]
				sub(letter,SUBSEP,arrayc[j])
				}
			else not_found = TRUE
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
				new = convert_to_degree(position)
				sub(SUBSEP,new,arrayc[j])
				return_token = return_token arrayc[j] " "
				}
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#########################################################################
##				Function Process_kern
#	This function accepts kern input and translates to degree
#
function process_kern(data_token,position,  return_token,arrayd,j,split_num,\
								not_found,accidentals,original,new)
	{
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
			original = arrayd[j]
			if (options ~ /x/ && arrayd[j] ~ tie_reg)
				sub(tie_reg,"",arrayd[j])
			not_found = FALSE
			current_note[4] = ""
			if (match(arrayd[j],/#+/))
				{
				accidentals = substr(arrayd[j],RSTART,RLENGTH)
				current_note[3] = accidentals
				sub(/#+/,"",arrayd[j])
				}
			else if (match(arrayd[j],/-+/))
				{
				accidentals = substr(arrayd[j],RSTART,RLENGTH)
				current_note[3] = accidentals
				sub(/-+/,"",arrayd[j])
				}
			else current_note[3] = ""
			gsub("n","",arrayd[j])
			if (match(arrayd[j],kern_pitch))
				{
				current_note[2] = substr(arrayd[j],RSTART,1)
				if (current_note[2] ~ /[a-g]/)
					{
					current_note[2] = to_upper(current_note[2])
					current_note[1] = 3 + RLENGTH
					}
				else current_note[1] = 4 - RLENGTH
				sub(kern_pitch,SUBSEP,arrayd[j])
				}
			else not_found = TRUE
			if (not_found) return_token = return_token original " "
			else
				{
				new = convert_to_degree(position)
				sub(SUBSEP,new,arrayd[j])
				return_token = return_token arrayd[j] " "
				}
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}
