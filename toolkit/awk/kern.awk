######################################################################
##					KERN.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       	Description:
# May 10,94	Tim Racinsky		Corrected Relative Tuning error for multi-
#							stringed courses in **fret spines
# June 9/94	Tim Racinsky	Modified parse_command to work with getopts
# June 23/94	Tim Racinsky	Modified solfg representation to include "-"
# May 26/97	David Huron	Fixed bug in process_fret function.
#
#
#	This program is used to convert pitch units to kern notes 
#
# Main Functions used:
#	Store_new_interps()		Check_pc_key()
#	Find_semits1()			Check_ph_key()			Find_semits2()
#	Get_absolute()			Get_semits()			Get_relative()
#	Get_frets()			Store_indicators()		Process_indicators()
#	Ins_array_pos()		Del_array_pos()		Copy_fret()
#	Exchange_spines()		Process_data()			Convert_to_kern()
#	To_lower()			To_upper()			Semits_to_kern()
#	Process_pitch()		Process_Tonh()			Process_solfg()
#	Process_semits()		Process_cents()		Process_specC()
#	Process_freq()			Process_MIDI()			Process_fret()
#	Process_degree()
#
#	VARIABLES:
#
#  -spine_path_record:  flag used to indicate if the current record
#   is a spine-path record or not.
#  -no_interpretation_yet:  flag used to indicate if an interpretation
#   record has been processed yet.
#  -current_no_of_spines:  holds the current number of active spines.
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
#  -pitch_pitch:  defines valid **pitch pitch
#  -solfg_pitch:  defines valid solfg pitch
#  -abs_tuning:  defines absolute tuning interpretation
#  -rel_tuning:  defines relative tuning interpretation
#  -fret_tuning:  defines fret tuning interpretation
#  -ph_index, pc_index, fret_index:  indexes for what type of key or fret
#   is being used for the current array
#
#	ARRAYS:
#
#  -path_indicator and current_interp:  the standard arrays for
#   tracking interpretations.
#  -current_note: a global array used to store the current note. It holds
#   the ocatave class, letter name, accidentals, and equally tempered deviation.
#  -key: a 2-dimensional array holding pitches of corresponding semits values
#   for each common key.
#  -deg_key: a 2-dimensional array holding pitches of corresponding degree
#   values for each common key.
#  -current_key: holds the current key of each of the currently active spines.
#  -pitch_array:  contains pitch pitches and corresponding semit values
#  -solfg_array:  contains solfg pitches and corresponding semit values
#
BEGIN {
	ARGV[1] = ""
	USAGE="\nUSAGE: kern -h                (Help Screen)\n       kern [-x] ["\
		 "file ...]\n"
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
	uftp = "(([0-9]+[.]?[0-9]*)|([0-9]*[.]?[0-9]+))"
	deviation = "[+-][1-9][0-9]*"
	indicators = "^(\\*\\+|\\*\\-|\\*\\^|\\*v|\\*x)$"
	input_interps = "^(\\*\\*specC|\\*\\*pitch|\\*\\*Tonh|\\*\\*solfg"\
				 "|\\*\\*semits|\\*\\*cents|\\*\\*MIDI|\\*\\*fret"\
				 "|\\*\\*freq|\\*\\*degree)$"
	key_reg = "^\\*(([ABCDEFGabcdefg](#?|-?))|(\\?)|(X)|(Cx)|(cx)|(Dx)):$"
	pc_key_sig = "^\\*k\\[([abcdefg]((n)|(#)|(x+(#?))|(-)+))*\\]$"
	repetition = "(a.*a)|(b.*b)|(c.*c)|(d.*d)|(e.*e)|(f.*f)|(g.*g)"
	ph_key_sig = "^\\*K\\[([ABCDEFG][1-9]((n)|(#)|(x+(#?))|(-)+))*\\]$"
	pc_note = "[abcdefg]((n)|(#)|(x+#?)|(-+))"
	pitch_reg = "[abcdefg]"
	ph_note = "[ABCDEFG][1-9]((n)|(#)|(x+#?)|(-+))"
	pitch_class = "[ABCDEFG][1-9]"
	octave_class = "[0-9]"
	pitch_pitch = "[ABCDEFG]"
	solfg_pitch = "do|re|mi|fa|sol|la|si"
	abs_tuning = "^\\*AT:[ABCDEFG][#b]?[0-9]+(" deviation ")?$"
	rel_tuning = "^\\*RT:" uftp "(," uftp ")*(:" uftp "(," uftp ")*)*$"
	fret_tuning = "^\\*FT:" uftp "(," uftp ")*$"
	ph_index = pc_index = fret_index = 1
	#
	solfg_array["do"] = "C"; solfg_array["re"] = "D" ; solfg_array["mi"] = "E";
	solfg_array["fa"] = "F"; solfg_array["sol"] = "G"; solfg_array["la"] = "A";
	solfg_array["si"] = "B";
	#
	pitch_array["A"] = 9; pitch_array["B"] = 11; pitch_array["C"] = 0;
	pitch_array["D"] = 2; pitch_array["E"] = 4 ; pitch_array["F"] = 5;
	pitch_array["G"] = 7;
	#
	# Initialize the key array
	#
	key["*A:",0] = "C/n"; key["*A:",1] = "C/#"; key["*A:",2] = "D"
	key["*A:",3] = "D/#"; key["*A:",4] = "E"  ; key["*A:",5] = "F/n"
	key["*A:",6] = "F/#"; key["*A:",7] = "G/n"; key["*A:",8] = "G/#"
	key["*A:",9] = "A"  ; key["*A:",10]= "A/#"; key["*A:",11]= "B"

	key["*a:",0] = "C"  ; key["*a:",1] = "C/#"; key["*a:",2] = "D"
	key["*a:",3] = "D/#"; key["*a:",4] = "E"  ; key["*a:",5] = "F"
	key["*a:",6] = "F/#"; key["*a:",7] = "G/n"; key["*a:",8] = "G/#"
	key["*a:",9] = "A"  ; key["*a:",10]= "B/-"; key["*a:",11]= "B"

	key["*A-:",0] = "C"  ; key["*A-:",1] = "D/-"; key["*A-:",2] = "D/n"
	key["*A-:",3] = "E/-"; key["*A-:",4] = "E/n"; key["*A-:",5] = "F"
	key["*A-:",6] = "G/-"; key["*A-:",7] = "G"  ; key["*A-:",8] = "A/-"
	key["*A-:",9] = "A/n"; key["*A-:",10]= "B/-"; key["*A-:",11]= "C/-"

	key["*a-:",0] = "C/n" ; key["*a-:",1] = "D/-"; key["*a-:",2] = "D/n"
	key["*a-:",3] = "E/-" ; key["*a-:",4] = "F/-"; key["*a-:",5] = "F/n"
	key["*a-:",6] = "G/-" ; key["*a-:",7] = "G/n"; key["*a-:",8] = "A/-"
	key["*a-:",9] = "B/--"; key["*a-:",10]= "B/-"; key["*a-:",11]= "C/-"
	
	key["*A#:",0] = "B/#"; key["*A#:",1] = "C/#"; key["*A#:",2] = "C/##"
	key["*A#:",3] = "D/#"; key["*A#:",4] = "D/##"; key["*A#:",5] = "E/#"
	key["*A#:",6] = "F/#"; key["*A#:",7] = "F/##"; key["*A#:",8] = "G/#"
	key["*A#:",9] = "G/##"; key["*A#:",10]= "A/#"; key["*A#:",11]= "B"
	
	key["*a#:",0] = "B/#"; key["*a#:",1] = "C/#"; key["*a#:",2] = "C/##";
	key["*a#:",3] = "D/#"; key["*a#:",4] = "D/##"; key["*a#:",5] = "E/#";
	key["*a#:",6] = "F/#"; key["*a#:",7] = "F/##"; key["*a#:",8] = "G/#";
	key["*a#:",9] = "G/##"; key["*a#:",10]= "A/#"; key["*a#:",11]= "B";
	
	key["*B:",0] = "C/n"; key["*B:",1] = "C/#"; key["*B:",2] = "D/n";
	key["*B:",3] = "D/#"; key["*B:",4] = "E"  ; key["*B:",5] = "E/#";
	key["*B:",6] = "F/#"; key["*B:",7] = "G/n"; key["*B:",8] = "G/#";
	key["*B:",9] = "A/n"; key["*B:",10]= "A/#"; key["*B:",11]= "B";
	
	key["*b:",0] = "C/n"; key["*b:",1] = "C/#"; key["*b:",2] = "D";
	key["*b:",3] = "D/#"; key["*b:",4] = "E"  ; key["*b:",5] = "E/#";
	key["*b:",6] = "F/#"; key["*b:",7] = "G"  ; key["*b:",8] = "G/#";
	key["*b:",9] = "A/n"; key["*b:",10]= "A/#"; key["*b:",11]= "B";
	
	key["*B-:",0] = "C"  ; key["*B-:",1] = "D/-"; key["*B-:",2] = "D";
	key["*B-:",3] = "E/-"; key["*B-:",4] = "E/n"; key["*B-:",5] = "F";
	key["*B-:",6] = "G/-"; key["*B-:",7] = "G"  ; key["*B-:",8] = "A/-";
	key["*B-:",9] = "A"  ; key["*B-:",10]= "B/-"; key["*B-:",11]= "C/-";
	
	key["*b-:",0] = "C"  ; key["*b-:",1] = "D/-"; key["*b-:",2] = "D/n";
	key["*b-:",3] = "E/-"; key["*b-:",4] = "E/n"; key["*b-:",5] = "F";
	key["*b-:",6] = "G/-"; key["*b-:",7] = "G/n"; key["*b-:",8] = "A/-";
	key["*b-:",9] = "A/n"; key["*b-:",10]= "B/-"; key["*b-:",11]= "C/-";
	
	key["*B#:",0] = "B/#"; key["*B#:",1] = "C/#"; key["*B#:",2] = "C/##";
	key["*B#:",3] = "D/#"; key["*B#:",4] = "D/##"; key["*B#:",5] = "E/#";
	key["*B#:",6] = "E/##"; key["*B#:",7] = "F/##"; key["*B#:",8] = "G/#";
	key["*B#:",9] = "G/##"; key["*B#:",10]= "A/#"; key["*B#:",11]= "A/##";
	
	key["*b#:",0] = "B/#"; key["*b#:",1] = "C/#"; key["*b#:",2] = "C/##";
	key["*b#:",3] = "D/#"; key["*b#:",4] = "D/##"; key["*b#:",5] = "E/#";
	key["*b#:",6] = "E/##"; key["*b#:",7] = "F/##"; key["*b#:",8] = "G/#";
	key["*b#:",9] = "G/##"; key["*b#:",10]= "A/#"; key["*b#:",11]= "A/##";
	
	key["*C:",0] = "C"  ; key["*C:",1] = "D/-"; key["*C:",2] = "D";
	key["*C:",3] = "E/-"; key["*C:",4] = "E"  ; key["*C:",5] = "F";
	key["*C:",6] = "F/#"; key["*C:",7] = "G"  ; key["*C:",8] = "A/-";
	key["*C:",9] = "A"  ; key["*C:",10]= "B/-"; key["*C:",11]= "B";
	
	key["*c:",0] = "C"  ; key["*c:",1] = "D/-"; key["*c:",2] = "D";
	key["*c:",3] = "E/-"; key["*c:",4] = "E"  ; key["*c:",5] = "F";
	key["*c:",6] = "F/#"; key["*c:",7] = "G"  ; key["*c:",8] = "A/-";
	key["*c:",9] = "A/n"; key["*c:",10]= "B/-"; key["*c:",11]= "B/n";
	
	key["*C-:",0] = "C/n" ; key["*C-:",1] = "D/-" ; key["*C-:",2] = "E/--";
	key["*C-:",3] = "E/-" ; key["*C-:",4] = "F/-" ; key["*C-:",5] = "F/n";
	key["*C-:",6] = "G/-" ; key["*C-:",7] = "A/--"; key["*C-:",8] = "A/-";
	key["*C-:",9] = "B/--"; key["*C-:",10]= "B/-" ; key["*C-:",11]= "C/-";
	
	key["*c-:",0] = "D/--"; key["*c-:",1] = "D/-" ; key["*c-:",2] = "E/--";
	key["*c-:",3] = "E/-" ; key["*c-:",4] = "F/-" ; key["*c-:",5] = "F/n";
	key["*c-:",6] = "G/-" ; key["*c-:",7] = "A/--"; key["*c-:",8] = "A/-";
	key["*c-:",9] = "B/--"; key["*c-:",10]= "B/-" ; key["*c-:",11]= "C/-";
	
	key["*C#:",0] = "B/#"; key["*C#:",1] = "C/#"; key["*C#:",2] = "D/n";
	key["*C#:",3] = "D/#"; key["*C#:",4] = "E/n"; key["*C#:",5] = "E/#";
	key["*C#:",6] = "F/#"; key["*C#:",7] = "F/##"; key["*C#:",8] = "G/#";
	key["*C#:",9] = "A/n"; key["*C#:",10]= "A/#"; key["*C#:",11]= "B/n";
	
	key["*c#:",0] = "B/#"; key["*c#:",1] = "C/#"; key["*c#:",2] = "D/n";
	key["*c#:",3] = "D/#"; key["*c#:",4] = "E/n"; key["*c#:",5] = "E/#";
	key["*c#:",6] = "F/#"; key["*c#:",7] = "F/##"; key["*c#:",8] = "G/#";
	key["*c#:",9] = "A/n"; key["*c#:",10]= "A/#"; key["*c#:",11]= "B/n";
	
	key["*D:",0] = "C/n"; key["*D:",1] = "C/#"; key["*D:",2] = "D";
	key["*D:",3] = "E/-"; key["*D:",4] = "E"  ; key["*D:",5] = "F/n";
	key["*D:",6] = "F/#"; key["*D:",7] = "G"  ; key["*D:",8] = "G/#";
	key["*D:",9] = "A"  ; key["*D:",10]= "B/-"; key["*D:",11]= "B";
	
	key["*d:",0] = "C/n"; key["*d:",1] = "C/#"; key["*d:",2] = "D";
	key["*d:",3] = "E/-"; key["*d:",4] = "E"  ; key["*d:",5] = "F";
	key["*d:",6] = "F/#"; key["*d:",7] = "G"  ; key["*d:",8] = "G/#";
	key["*d:",9] = "A"  ; key["*d:",10]= "B/-"; key["*d:",11]= "B/n";
	
	key["*D-:",0] = "C/" ; key["*D-:",1] = "D/-"; key["*D-:",2] = "D/n";
	key["*D-:",3] = "E/-"; key["*D-:",4] = "F/-"; key["*D-:",5] = "F/n";
	key["*D-:",6] = "G/-"; key["*D-:",7] = "G/n"; key["*D-:",8] = "A/-";
	key["*D-:",9] = "A/n"; key["*D-:",10]= "B/-"; key["*D-:",11]= "C/-";
	
	key["*d-:",0] = "C/n" ; key["*d-:",1] = "D/-"; key["*d-:",2] = "E/--";
	key["*d-:",3] = "E/-" ; key["*d-:",4] = "F/-"; key["*d-:",5] = "F/n";
	key["*d-:",6] = "G/-" ; key["*d-:",7] = "G/n"; key["*d-:",8] = "A/-";
	key["*d-:",9] = "B/--"; key["*d-:",10]= "B/-"; key["*d-:",11]= "C/-";
	
	key["*D#:",0] = "B/#"; key["*D#:",1] = "C/#"; key["*D#:",2] = "C/##";
	key["*D#:",3] = "D/#"; key["*D#:",4] = "E/n"; key["*D#:",5] = "E/#";
	key["*D#:",6] = "F/#"; key["*D#:",7] = "F/##"; key["*D#:",8] = "G/#";
	key["*D#:",9] = "G/##"; key["*D#:",10]= "A/#"; key["*D#:",11]= "B/n";
	
	key["*d#:",0] = "B/#"; key["*d#:",1] = "C/#"; key["*d#:",2] = "C/##";
	key["*d#:",3] = "D/#"; key["*d#:",4] = "E/n"; key["*d#:",5] = "E/#";
	key["*d#:",6] = "F/#"; key["*d#:",7] = "F/##"; key["*d#:",8] = "G/#";
	key["*d#:",9] = "G/##"; key["*d#:",10]= "A/#"; key["*d#:",11]= "B/n";
	
	key["*E:",0] = "C/n"; key["*E:",1] = "C/#"; key["*E:",2] = "D/n";
	key["*E:",3] = "D/#"; key["*E:",4] = "E"  ; key["*E:",5] = "F/n";
	key["*E:",6] = "F/#"; key["*E:",7] = "G/n"; key["*E:",8] = "G/#";
	key["*E:",9] = "A"  ; key["*E:",10]= "A/#"; key["*E:",11]= "B";
	
	key["*e:",0] = "C"  ; key["*e:",1] = "C/#"; key["*e:",2] = "D/n";
	key["*e:",3] = "D/#"; key["*e:",4] = "E"  ; key["*e:",5] = "F/n";
	key["*e:",6] = "F/#"; key["*e:",7] = "G"  ; key["*e:",8] = "G/#";
	key["*e:",9] = "A"  ; key["*e:",10]= "A/#"; key["*e:",11]= "B";
	
	key["*E-:",0] = "C"  ; key["*E-:",1] = "D/-"; key["*E-:",2] = "D";
	key["*E-:",3] = "E/-"; key["*E-:",4] = "E/n"; key["*E-:",5] = "F";
	key["*E-:",6] = "G/-"; key["*E-:",7] = "G"  ; key["*E-:",8] = "A/-";
	key["*E-:",9] = "A/n"; key["*E-:",10]= "B/-"; key["*E-:",11]= "C/-";
	
	key["*e-:",0] = "C/n"; key["*e-:",1] = "D/-"; key["*e-:",2] = "D/n";
	key["*e-:",3] = "E/-"; key["*e-:",4] = "F/-"; key["*e-:",5] = "F";
	key["*e-:",6] = "G/-"; key["*e-:",7] = "G/n"; key["*e-:",8] = "A/-";
	key["*e-:",9] = "A/n"; key["*e-:",10]= "B/-"; key["*e-:",11]= "C/-";
	
	key["*E#:",0] = "B/#" ; key["*E#:",1] = "C/#" ; key["*E#:",2] = "C/##";
	key["*E#:",3] = "D/#" ; key["*E#:",4] = "D/##"; key["*E#:",5] = "E/#";
	key["*E#:",6] = "F/#" ; key["*E#:",7] = "F/##"; key["*E#:",8] = "G/#";
	key["*E#:",9] = "G/##"; key["*E#:",10]= "A/#" ; key["*E#:",11]= "A/##";
	
	key["*e#:",0] = "B/#" ; key["*e#:",1] = "C/#" ; key["*e#:",2] = "C/##";
	key["*e#:",3] = "D/#" ; key["*e#:",4] = "D/##"; key["*e#:",5] = "E/#";
	key["*e#:",6] = "F/#" ; key["*e#:",7] = "F/##"; key["*e#:",8] = "G/#";
	key["*e#:",9] = "G/##"; key["*e#:",10]= "A/#" ; key["*e#:",11]= "A/##";
	
	key["*F:",0] = "C"  ; key["*F:",1] = "D/-"; key["*F:",2] = "D";
	key["*F:",3] = "E/-"; key["*F:",4] = "E"  ; key["*F:",5] = "F";
	key["*F:",6] = "G/-"; key["*F:",7] = "G"  ; key["*F:",8] = "A/-";
	key["*F:",9] = "A"  ; key["*F:",10]= "B/-"; key["*F:",11]= "B/n";
	
	key["*f:",0] = "C"  ; key["*f:",1] = "D/-"; key["*f:",2] = "D/n";
	key["*f:",3] = "E/-"; key["*f:",4] = "E/n"; key["*f:",5] = "F";
	key["*f:",6] = "G/-"; key["*f:",7] = "G"  ; key["*f:",8] = "A/-";
	key["*f:",9] = "A/n"; key["*f:",10]= "B/-"; key["*f:",11]= "B/n";
	
	key["*F-:",0] = "D/--"; key["*F-:",1] = "D/-" ; key["*F-:",2] = "E/--";
	key["*F-:",3] = "E/-" ; key["*F-:",4] = "F/-" ; key["*F-:",5] = "F/n";
	key["*F-:",6] = "G/-" ; key["*F-:",7] = "A/--"; key["*F-:",8] = "A/-";
	key["*F-:",9] = "B/--"; key["*F-:",10]= "B/-" ; key["*F-:",11]= "C/-";
	
	key["*f-:",0] = "D/--"; key["*f-:",1] = "D/-" ; key["*f-:",2] = "E/--";
	key["*f-:",3] = "E/-" ; key["*f-:",4] = "F/-" ; key["*f-:",5] = "G/--";
	key["*f-:",6] = "G/-" ; key["*f-:",7] = "A/--"; key["*f-:",8] = "A/-";
	key["*f-:",9] = "B/--"; key["*f-:",10]= "B/-" ; key["*f-:",11]= "C/-";
	
	key["*F#:",0] = "B/#"; key["*F#:",1] = "C/#"; key["*F#:",2] = "D/n";
	key["*F#:",3] = "D/#"; key["*F#:",4] = "E/n"; key["*F#:",5] = "E/#";
	key["*F#:",6] = "F/#"; key["*F#:",7] = "G/n"; key["*F#:",8] = "G/#";
	key["*F#:",9] = "A/n"; key["*F#:",10]= "A/#"; key["*F#:",11]= "B";
	
	key["*f#:",0] = "B/#"; key["*f#:",1] = "C/#"; key["*f#:",2] = "D";
	key["*f#:",3] = "D/#"; key["*f#:",4] = "E"  ; key["*f#:",5] = "E/#";
	key["*f#:",6] = "F/#"; key["*f#:",7] = "G/n"; key["*f#:",8] = "G/#";
	key["*f#:",9] = "A"  ; key["*f#:",10]= "A/#"; key["*f#:",11]= "B";
	
	key["*G:",0] = "C"  ; key["*G:",1] = "C/#"; key["*G:",2] = "D";
	key["*G:",3] = "E/-"; key["*G:",4] = "E"  ; key["*G:",5] = "F/n";
	key["*G:",6] = "F/#"; key["*G:",7] = "G"  ; key["*G:",8] = "A/-";
	key["*G:",9] = "A"  ; key["*G:",10]= "B/-"; key["*G:",11]= "B";
	
	key["*g:",0] = "C"  ; key["*g:",1] = "C/#"; key["*g:",2] = "D";
	key["*g:",3] = "E/-"; key["*g:",4] = "E/n"; key["*g:",5] = "F";
	key["*g:",6] = "F/#"; key["*g:",7] = "G"  ; key["*g:",8] = "A/-";
	key["*g:",9] = "A"  ; key["*g:",10]= "B/-"; key["*g:",11]= "B/n";
	
	key["*G-:",0] = "C/n" ; key["*G-:",1] = "D/-" ; key["*G-:",2] = "E/--";
	key["*G-:",3] = "E/-" ; key["*G-:",4] = "F/-" ; key["*G-:",5] = "F/n";
	key["*G-:",6] = "G/-" ; key["*G-:",7] = "A/--"; key["*G-:",8] = "A/-";
	key["*G-:",9] = "B/--"; key["*G-:",10]= "B/-" ; key["*G-:",11]= "C/-";
	
	key["*g-:",0] = "C/n" ; key["*g-:",1] = "D/-" ; key["*g-:",2] = "E/--";
	key["*g-:",3] = "E/-" ; key["*g-:",4] = "F/-" ; key["*g-:",5] = "F/n";
	key["*g-:",6] = "G/-" ; key["*g-:",7] = "A/--"; key["*g-:",8] = "A/-";
	key["*g-:",9] = "B/--"; key["*g-:",10]= "B/-" ; key["*g-:",11]= "C/-";
	
	key["*G#:",0] = "B/#"; key["*G#:",1] = "C/#"; key["*G#:",2] = "C/##";
	key["*G#:",3] = "D/#"; key["*G#:",4] = "E/n"; key["*G#:",5] = "E/#";
	key["*G#:",6] = "F/#"; key["*G#:",7] = "F/##"; key["*G#:",8] = "G/#";
	key["*G#:",9] = "A/n"; key["*G#:",10]= "A/#"; key["*G#:",11]= "B/n";
	
	key["*g#:",0] = "B/#"; key["*g#:",1] = "C/#"; key["*g#:",2] = "C/##";
	key["*g#:",3] = "D/#"; key["*g#:",4] = "E"  ; key["*g#:",5] = "E/#";
	key["*g#:",6] = "F/#"; key["*g#:",7] = "F/##"; key["*g#:",8] = "G/#";
	key["*g#:",9] = "A/n"; key["*g#:",10]= "A/#"; key["*g#:",11]= "B";
	
	key["*Cx:",0] = "B/#"; key["*Cx:",1] = "B/##"; key["*Cx:",2] = "C/##";
	key["*Cx:",3] = "D/#"; key["*Cx:",4] = "D/##"; key["*Cx:",5] = "E/#";
	key["*Cx:",6] = "E/##"; key["*Cx:",7] = "F/##"; key["*Cx:",8] = "F/###";
	key["*Cx:",9] = "G/##"; key["*Cx:",10]= "A/#"; key["*Cx:",11]= "A/##";
	
	key["*cx:",0] = "B/#"; key["*cx:",1] = "B/##"; key["*cx:",2] = "C/##";
	key["*cx:",3] = "D/#"; key["*cx:",4] = "D/##"; key["*cx:",5] = "E/#";
	key["*cx:",6] = "E/##"; key["*cx:",7] = "F/##"; key["*cx:",8] = "F/###";
	key["*cx:",9] = "G/##"; key["*cx:",10]= "A/#"; key["*cx:",11]= "A/##";
	
	key["*Dx:",0] = "B/#" ; key["*Dx:",1] = "B/##" ; key["*Dx:",2] = "C/##";
	key["*Dx:",3] = "C/###"; key["*Dx:",4] = "D/##" ; key["*Dx:",5] = "E/#";
	key["*Dx:",6] = "E/##" ; key["*Dx:",7] = "F/##" ; key["*Dx:",8] = "F/###";
	key["*Dx:",9] = "G/##" ; key["*Dx:",10]= "G/###"; key["*Dx:",11]= "A/##";
	#
	# Initialize the deg_key array
	#
	deg_key["*C:",1] = "C"; deg_key["*C:",2] = "D"; deg_key["*C:",3] = "E";
	deg_key["*C:",4] = "F"; deg_key["*C:",5] = "G"; deg_key["*C:",6] = "A";
	deg_key["*C:",7] = "B";

	deg_key["*c:",1] = "C"; deg_key["*c:",2] = "D"; deg_key["*c:",3] = "E/-";
	deg_key["*c:",4] = "F"; deg_key["*c:",5] = "G"; deg_key["*c:",6] = "A/-";
	deg_key["*c:",7] = "B";
	
	deg_key["*C#:",1] = "C/#"; deg_key["*C#:",2] = "D/#";
	deg_key["*C#:",3] = "E/#"; deg_key["*C#:",4] = "F/#";
	deg_key["*C#:",5] = "G/#"; deg_key["*C#:",6] = "A/#";
	deg_key["*C#:",7] = "B/#";
	
	deg_key["*c#:",1] = "C/#"; deg_key["*c#:",2] = "D/#";
	deg_key["*c#:",3] = "E"; deg_key["*c#:",4] = "F/#";
	deg_key["*c#:",5] = "G/#"; deg_key["*c#:",6] = "A";
	deg_key["*c#:",7] = "B/#";
	
	deg_key["*Cx:",1] = "C/##"; deg_key["*Cx:",2] = "D/##";
	deg_key["*Cx:",3] = "E/##"; deg_key["*Cx:",4] = "F/##";
	deg_key["*Cx:",5] = "G/##"; deg_key["*Cx:",6] = "A/##";
	deg_key["*Cx:",7] = "B/##";
	
	deg_key["*cx:",1] = "C/##"; deg_key["*cx:",2] = "D/##";
	deg_key["*cx:",3] = "E/#" ; deg_key["*cx:",4] = "F/##";
	deg_key["*cx:",5] = "G/##"; deg_key["*cx:",6] = "A/#" ;
	deg_key["*cx:",7] = "B/##";
	
	deg_key["*C-:",1] = "C/-"; deg_key["*C-:",2] = "D/-";
	deg_key["*C-:",3] = "E/-"; deg_key["*C-:",4] = "F/-";
	deg_key["*C-:",5] = "G/-"; deg_key["*C-:",6] = "A/-";
	deg_key["*C-:",7] = "B/-";
	
	deg_key["*c-:",1] = "C/-" ; deg_key["*c-:",2] = "D/-" ;
	deg_key["*c-:",3] = "E/--"; deg_key["*c-:",4] = "F/-" ;
	deg_key["*c-:",5] = "G/-" ; deg_key["*c-:",6] = "A/--";
	deg_key["*c-:",7] = "B/-" ;
	
	deg_key["*D:",7] = "C/#"; deg_key["*D:",1] = "D"; deg_key["*D:",2] = "E";
	deg_key["*D:",3] = "F/#"; deg_key["*D:",4] = "G"; deg_key["*D:",5] = "A";
	deg_key["*D:",6] = "B";
	
	deg_key["*d:",7] = "C/#"; deg_key["*d:",1] = "D"; deg_key["*d:",2] = "E";
	deg_key["*d:",3] = "F"  ; deg_key["*d:",4] = "G"; deg_key["*d:",5] = "A";
	deg_key["*d:",6] = "B/-";
	
	deg_key["*D#:",7] = "C/##"; deg_key["*D#:",1] = "D/#" ;
	deg_key["*D#:",2] = "E/#" ; deg_key["*D#:",3] = "F/##";
	deg_key["*D#:",4] = "G/#" ; deg_key["*D#:",5] = "A/#" ;
	deg_key["*D#:",6] = "B/#";
	
	deg_key["*Dx:",7] = "C/###"; deg_key["*Dx:",1] = "D/##";
	deg_key["*Dx:",2] = "E/##" ; deg_key["*Dx:",3] = "F/###";
	deg_key["*Dx:",4] = "G/##" ; deg_key["*Dx:",5] = "A/##";
	deg_key["*Dx:",6] = "B/##" ;
	
	deg_key["*d#:",7] = "C/##"; deg_key["*d#:",1] = "D/#";
	deg_key["*d#:",2] = "E/#" ; deg_key["*d#:",3] = "F/#";
	deg_key["*d#:",4] = "G/#" ; deg_key["*d#:",5] = "A/#" ;
	deg_key["*d#:",6] = "B";
	
	deg_key["*D-:",7] = "C"  ; deg_key["*D-:",1] = "D/-";
	deg_key["*D-:",2] = "E/-"; deg_key["*D-:",3] = "F"  ;
	deg_key["*D-:",4] = "G/-"; deg_key["*D-:",5] = "A/-";
	deg_key["*D-:",6] = "B/-";
	
	deg_key["*d-:",7] = "C"  ; deg_key["*d-:",1] = "D/-";
	deg_key["*d-:",2] = "E/-"; deg_key["*d-:",3] = "F/-";
	deg_key["*d-:",4] = "G/-"; deg_key["*d-:",5] = "A/-";
	deg_key["*d-:",6] = "B/--";
	
	deg_key["*E:",6] = "C/#"; deg_key["*E:",7] = "D/#"; deg_key["*E:",1] = "E";
	deg_key["*E:",2] = "F/#"; deg_key["*E:",3] = "G/#"; deg_key["*E:",4] = "A";
	deg_key["*E:",5] = "B";
	
	deg_key["*e:",6] = "C"; deg_key["*e:",7] = "D/#"; deg_key["*e:",1] = "E";
	deg_key["*e:",2] = "F/#"; deg_key["*e:",3] = "G"; deg_key["*e:",4] = "A";
	deg_key["*e:",5] = "B";
	
	deg_key["*E#:",6] = "C/##"; deg_key["*E#:",7] = "D/##";
	deg_key["*E#:",1] = "E/#"; deg_key["*E#:",2] = "F/##";
	deg_key["*E#:",3] = "G/##"; deg_key["*E#:",4] = "A/#";
	deg_key["*E#:",5] = "B/#";
	
	deg_key["*e#:",6] = "C/#"; deg_key["*e#:",7] = "D/##";
	deg_key["*e#:",1] = "E/#"; deg_key["*e#:",2] = "F/##";
	deg_key["*e#:",3] = "G/#"; deg_key["*e#:",4] = "A/#";
	deg_key["*e#:",5] = "B/#";
	
	deg_key["*E-:",6] = "C"; deg_key["*E-:",7] = "D";deg_key["*E-:",1] = "E/-";
	deg_key["*E-:",2] = "F"; deg_key["*E-:",3] = "G";deg_key["*E-:",4] = "A/-";
	deg_key["*E-:",5] = "B/-";
	
	deg_key["*e-:",6] = "C/-"; deg_key["*e-:",7] = "D";
	deg_key["*e-:",1] = "E/-"; deg_key["*e-:",2] = "F";
	deg_key["*e-:",3] = "G/-"; deg_key["*e-:",4] = "A/-";
	deg_key["*e-:",5] = "B/-";
	
	deg_key["*F:",5] = "C"; deg_key["*F:",6] = "D"; deg_key["*F:",7] = "E";
	deg_key["*F:",1] = "F"; deg_key["*F:",2] = "G"; deg_key["*F:",3] = "A";
	deg_key["*F:",4] = "B/-";
	
	deg_key["*f:",5] = "C"; deg_key["*f:",6] = "D/-"; deg_key["*f:",7] = "E";
	deg_key["*f:",1] = "F"; deg_key["*f:",2] = "G"; deg_key["*f:",3] = "A/-";
	deg_key["*f:",4] = "B/-";
	
	deg_key["*F#:",5] = "C/#"; deg_key["*F#:",6] = "D/#";
	deg_key["*F#:",7] = "E/#"; deg_key["*F#:",1] = "F/#";
	deg_key["*F#:",2] = "G/#"; deg_key["*F#:",3] = "A/#";
	deg_key["*F#:",4] = "B";
	
	deg_key["*f#:",5] = "C/#"; deg_key["*f#:",6] = "D";
	deg_key["*f#:",7] = "E/#"; deg_key["*f#:",1] = "F/#";
	deg_key["*f#:",2] = "G/#"; deg_key["*f#:",3] = "A";deg_key["*f#:",4] = "B";
	
	deg_key["*F-:",5] = "C/-"; deg_key["*F-:",6] = "D/-";
	deg_key["*F-:",7] = "E/-"; deg_key["*F-:",1] = "F/-";
	deg_key["*F-:",2] = "G/-"; deg_key["*F-:",3] = "A/-";
	deg_key["*F-:",4] = "B/--";
	
	deg_key["*f-:",5] = "C/-"; deg_key["*f-:",6] = "D/--";
	deg_key["*f-:",7] = "E/-"; deg_key["*f-:",1] = "F/-";
	deg_key["*f-:",2] = "G/-"; deg_key["*f-:",3] = "A/--";
	deg_key["*f-:",4] = "B/--";
	
	deg_key["*G:",4] = "C"; deg_key["*G:",5] = "D"; deg_key["*G:",6] = "E";
	deg_key["*G:",7] = "F/#"; deg_key["*G:",1] = "G"; deg_key["*G:",2] = "A";
	deg_key["*G:",3] = "B";
	
	deg_key["*g:",4] = "C"; deg_key["*g:",5] = "D"; deg_key["*g:",6] = "E/-";
	deg_key["*g:",7] = "F/#"; deg_key["*g:",1] = "G"; deg_key["*g:",2] = "A";
	deg_key["*g:",3] = "B/-";
	
	deg_key["*G#:",4] = "C/#"; deg_key["*G#:",5] = "D/#";
	deg_key["*G#:",6] = "E/#"; deg_key["*G#:",7] = "F/##";
	deg_key["*G#:",1] = "G/#"; deg_key["*G#:",2] = "A/#";
	deg_key["*G#:",3] = "B/#";
	
	deg_key["*g#:",4] = "C/#"; deg_key["*g#:",5] = "D/#";
	deg_key["*g#:",6] = "E"; deg_key["*g#:",7] = "F/##";
	deg_key["*g#:",1] = "G/#"; deg_key["*g#:",2] = "A/#";
	deg_key["*g#:",3] = "B";
	
	deg_key["*G-:",4] = "C/-"; deg_key["*G-:",5] = "D/-";
	deg_key["*G-:",6] = "E/-"; deg_key["*G-:",7] = "F";
	deg_key["*G-:",1] = "G/-"; deg_key["*G-:",2] = "A/-";
	deg_key["*G-:",3] = "B/-";
	
	deg_key["*g-:",4] = "C/-"; deg_key["*g-:",5] = "D/-";
	deg_key["*g-:",6] = "E/--"; deg_key["*g-:",7] = "F";
	deg_key["*g-:",1] = "G/-"; deg_key["*g-:",2] = "A/-";
	deg_key["*g-:",3] = "B/--";
	
	deg_key["*A:",3] = "C/#"; deg_key["*A:",4] = "D"; deg_key["*A:",5] = "E";
	deg_key["*A:",6] = "F/#"; deg_key["*A:",7] = "G/#"; deg_key["*A:",1] = "A";
	deg_key["*A:",2] = "B";
	
	deg_key["*a:",3] = "C"; deg_key["*a:",4] = "D"; deg_key["*a:",5] = "E";
	deg_key["*a:",6] = "F"; deg_key["*a:",7] = "G/#"; deg_key["*a:",1] = "A";
	deg_key["*a:",2] = "B";
	
	deg_key["*A#:",3] = "C/##"; deg_key["*A#:",4] = "D/#";
	deg_key["*A#:",5] = "E/#"; deg_key["*A#:",6] = "F/##";
	deg_key["*A#:",7] = "G/##"; deg_key["*A#:",1] = "A/#";
	deg_key["*A#:",2] = "B/#";
	
	deg_key["*a#:",3] = "C/#"; deg_key["*a#:",4] = "D/#";
	deg_key["*a#:",5] = "E/#"; deg_key["*a#:",6] = "F/#";
	deg_key["*a#:",7] = "G/##"; deg_key["*a#:",1] = "A/#";
	deg_key["*a#:",2] = "B/#";
	
	deg_key["*A-:",3] = "C"; deg_key["*A-:",4] = "D/-";
	deg_key["*A-:",5] = "E/-"; deg_key["*A-:",6] = "F";
	deg_key["*A-:",7] = "G"; deg_key["*A-:",1] = "A/-";
	deg_key["*A-:",2] = "B/-";
	
	deg_key["*a-:",3] = "C/-"; deg_key["*a-:",4] = "D/-";
	deg_key["*a-:",5] = "E/-"; deg_key["*a-:",6] = "F/-";
	deg_key["*a-:",7] = "G"; deg_key["*a-:",1] = "A/-";
	deg_key["*a-:",2] = "B/-";
	
	deg_key["*B:",2] = "C/#"; deg_key["*B:",3] = "D/#"; deg_key["*B:",4] = "E";
	deg_key["*B:",5] = "F/#";deg_key["*B:",6] = "G/#";deg_key["*B:",7] = "A/#";
	deg_key["*B:",1] = "B";
	
	deg_key["*b:",2] = "C/#"; deg_key["*b:",3] = "D"; deg_key["*b:",4] = "E";
	deg_key["*b:",5] = "F/#"; deg_key["*b:",6] = "G"; deg_key["*b:",7] = "A/#";
	deg_key["*b:",1] = "B";
	
	deg_key["*B#:",2] = "C/##"; deg_key["*B#:",3] = "D/##";
	deg_key["*B#:",4] = "E/#"; deg_key["*B#:",5] = "F/##";
	deg_key["*B#:",6] = "G/##"; deg_key["*B#:",7] = "A/##";
	deg_key["*B#:",1] = "B/#";
	
	deg_key["*b#:",2] = "C/##"; deg_key["*b#:",3] = "D/#";
	deg_key["*b#:",4] = "E/#"; deg_key["*b#:",5] = "F/##";
	deg_key["*b#:",6] = "G/#"; deg_key["*b#:",7] = "A/##";
	deg_key["*b#:",1] = "B/#";
	
	deg_key["*B-:",2] = "C"; deg_key["*B-:",3] = "D";deg_key["*B-:",4] = "E/-";
	deg_key["*B-:",5] = "F"; deg_key["*B-:",6] = "G"; deg_key["*B-:",7] = "A";
	deg_key["*B-:",1] = "B/-";
	
	deg_key["*b-:",2] = "C"; deg_key["*b-:",3] = "D/-";
	deg_key["*b-:",4] = "E/-"; deg_key["*b-:",5] = "F";
	deg_key["*b-:",6] = "G/-"; deg_key["*b-:",7] = "A";
	deg_key["*b-:",1] = "B/-";
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
##			FUNCTIONS USED IN KERN.AWK
##################################################################

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
		else if ($j ~ key_reg) current_key[j] = $j
		else if ($j ~ /^\*[kK]\[/)
			{
			if ($j ~ pc_key_sig) check_pc_key($j,j)
			else if ($j ~ ph_key_sig) check_ph_key($j,j)
			else
				{
				print "kern: ERROR: Invalid key signature interpretation "\
					 "in spine " j ", line " FNR "." > stderr
				exit
				}
			}
		#
		# Check for any information pertaining to a spine with the
		# fret interpretation
		#
		if ($j ~ /^\*\*fret/)
			{
			absolute[fret_index] = -20
			fret_array[j] = fret_index
			fret_index += 1
			}
		else if ($j ~ /^\*AT:/) get_absolute($j,j)
		else if ($j ~ /^\*RT:/) get_relative($j,j)
		else if ($j ~ /^\*FT:/) get_frets($j,j)
		#
		# If the interpretation is a 'convertible' interpretation, then
		# the output line must show kern.
		#
		if ($j ~ input_interps) interp_line = interp_line "**kern\t"
		else interp_line = interp_line $j "\t"
		}
	#
	# Print the new interpretation line
	#
	print substr(interp_line,1,length(interp_line)-1)
	}

#########################################################################
##				Function Check_pc_key
#	This function checks and stores the pitch class key signature interp
#
function check_pc_key(sig,spine,   note,pitch,semits,i,pc_array)
	{
	#
	# Check that no pitches are repeated
	#
	if (sig ~ repetition)
		{
		print "kern: ERROR: Invalid key signature interpretation "\
			 "in spine " spine ", line " FNR "." > stderr
		exit
		}
	else
		{
		#
		# Store the values in an array
		#
		while (match(sig,pc_note))
			{
			note = substr(sig,RSTART,RLENGTH)
			sub(pc_note,"",sig)
			sub("x","##",note)
			semits = find_semits1(note)
			pitch = to_upper(substr(note,1,1))
			sub(pitch_reg,"",note)
			#
			# No semits value may be repeated
			#
			if (pc[pc_index,semits] != "")
				{
				print "kern: ERROR: Invalid key signature interpretation "\
					 "in spine " spine ", line " FNR "." > stderr
				exit
				}
			if (note != "") pc[pc_index,semits] = pitch "/" note
			else pc[pc_index,semits] = pitch
			}
		#
		# Any unspecified semit values are taken from C major
		#
		for (i = 0; i <= 11; i += 1)
			{
			if (pc[pc_index,i] == "") pc[pc_index,i] = key["*C:",i]
			}
		current_key[spine] = "pc" pc_index
		pc_index += 1
		}
	}

####################################################################
##				Function Find_semits1
#	This function determines the semits value of a given note
#
function find_semits1(note,  semits)
	{
	semits = 0
	#
	# First find the letter name of the note
	#
	if (note ~ /a/) semits = 9
	else if (note ~ /b/) semits = 11
	else if (note ~ /c/) semits = 0
	else if (note ~ /d/) semits = 2
	else if (note ~ /e/) semits = 4
	else if (note ~ /f/) semits = 5
	else if (note ~ /g/) semits = 7
	#
	# Next determine the value of any flats or sharps
	#
	if (match(note,/-+/)) semits -=  RLENGTH
	else if (match(note,/#+/)) semits += RLENGTH
	#
	semits = ((semits % 12) + 12) % 12
	return semits
	}

#########################################################################
##				Function Check_ph_key
#	This function checks the validity of a pitch height key signature interp
#
function check_ph_key(sig,spine,   i,array_index,pitch_list)
	{
	pitch_list = ""
	#
	# Store each pitch in an array
	#
	while (match(sig,ph_note))
		{
		note = substr(sig,RSTART,RLENGTH)
		sub(ph_note,"",sig)
		sub("x","##",note)
		semits = find_semits2(note)
		match(note,pitch_class)
		pit_clas = substr(note,RSTART,RLENGTH)
		sub(pitch_class,"",note)
		#
		# Make sure no pitches are repeated
		#
		if (match(pitch_list,pit_clas))
			{
			print "kern: ERROR: Invalid key signature interpretation "\
				 "in spine " spine ", line " FNR "." > stderr
			exit
			}
		else pitch_list = pitch_list pit_clas
		pitch = substr(pit_clas,1,1)
		if (note != "") ph[ph_index,semits] = pitch "/" note
		else ph[ph_index,semits] = pitch
		}
	current_key[spine] = "ph" ph_index
	ph_index += 1
	}

####################################################################
##				Function Find_semits2
#	This function determines the semits value of a given note
#
function find_semits2(note,  class,semits)
	{
	semits = 0
	#
	# First find the letter name of the note
	#
	if (note ~ /A/) semits = 9
	else if (note ~ /B/) semits = 11
	else if (note ~ /C/) semits = 0
	else if (note ~ /D/) semits = 2
	else if (note ~ /E/) semits = 4
	else if (note ~ /F/) semits = 5
	else if (note ~ /G/) semits = 7
	match(note,octave_class)
	class = substr(note,RSTART,RLENGTH) + 0
	if (class < 4)
		{
		while (class < 4)
			{
			semits -= 12
			class += 1
			}
		}
	else if (class > 4)
		{
		while (class > 4)
			{
			semits += 12
			class -= 1
			}
		}
	#
	# Next determine the value of any flats or sharps
	#
	if (match(note,/-+/)) semits -=  RLENGTH
	else if (match(note,/#+/)) semits += RLENGTH
	#
	return semits
	}

#########################################################################
##					Function Get_absolute
#	This function obains the absolute tuning for a fret spine.
#
function get_absolute(interp,position,    temp_index)
	{
	#
	# The interpretation must be valid
	#
	if (interp !~ abs_tuning)
		{
		print "kern: ERROR: Invalid absolute tuning interpretation in "\
			 "line " NR "." > stderr
		exit
		}
	#
	# Only proceses if current spine is for fret
	#
	if (current_interp[position] == "**fret")
		{
		temp_index = fret_array[position]
		split(interp,at_array,":")
		absolute[temp_index] = get_semits(at_array[2])
		}
	}

#########################################################################
##					Function Get_semits
#	This function simply translates pitch into semits without any checking
#
function get_semits(note,   semits)
	{
	semits = 0
	#
	# Get pitch
	#
	match(note,pitch_pitch)
	pitch = substr(note,RSTART,1)
	semits = pitch_array[pitch]
	#
	# Get accidentals
	#
	if (match(note,/b+/)) semits -= RLENGTH
	else if (match(note,/#+/)) semits += RLENGTH
	#
	# Get deviation
	#
	if (match(note,deviation)) semits += (substr(note,RSTART,RLENGTH) * 0.01)
	#
	# Get octave class
	#
	match(note,octave_class)
	semits += (12 * (substr(note,RSTART,1) + 0)) - 48
	return semits
	}

#########################################################################
##					Function Get_relative
#	This function gets the relative tuning for a fret spine.
#
function get_relative(interp,position, courses,i,j,temp_index,num1)
	{
	#
	# The interpretation must be a valid relative tuning interpretation
	#
	if (interp !~ rel_tuning)
		{
		print "kern: ERROR: Invalid relative tuning interpretation in "\
			 "line " NR "." > stderr
		exit
		}
	#
	# The current spine must have a fret exclusive interpretation
	#
	if (current_interp[position] == "**fret")
		{
		temp_index = fret_array[position]
		num1 = split(interp,courses,":")
		#
		# Store the value for each string in relative
		#
		for (i = 2; i <= num1; i += 1)
			{
			relative[temp_index,i-1] = courses[i]
			}
		}
	}

#########################################################################
##					Function Get_frets
#	This function gets the fret tuning for a fret spine
#
function get_frets(interp,position,   temp_index,temp_array,num1,frets,i)
	{
	#
	# The interpretation must be a valid fret tuning interpretation
	#
	if (interp !~ fret_tuning)
		{
		print "kern: ERROR: Invalid fret tuning interpretation in "\
			 "line " NR "." > stderr
		exit
		}
	#
	# Only process if the current spine is a fret spine
	#
	if (current_interp[position] == "**fret")
		{
		temp_index = fret_array[position]
		split(interp,temp_array,":")
		num1 = split(temp_array[2],frets,",")
		for (i = 1; i <= num1; i += 1) fret_val[temp_index,i] = frets[i]
		}
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
			if (current_interp[i] == "**fret") copy_fret(i,i+1)
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
		fret_array[j] = fret_array[j-1]
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
		fret_array[j] = fret_array[j+1]
		}
	delete path_indicator[j]
	delete current_interp[j]
	delete current_key[j]
	delete fret_array[j]
	current_no_of_spines -= 1
	} 

##########################################################################
##				Function Copy_fret
#	A fret spine that is split must create a copy of its values in the 
# tuning arrays so that the new spine can manipulate them freely.
#
function copy_fret(current,new,    current_index,temp_array,i)
	{
	current_index = fret_array[current]
	#
	# Copy the absolute tuning values
	#
	absolute[fret_index] = absolute[current_index]
	#
	# Copy the relative tuning values
	#
	for (i in relative)
		{
		split(i,temp_array,SUBSEP)
		if (temp_array[1] == current_index)
			{
			relative[fret_index,temp_array[2]] \
							= relative[current_index,temp_array[2]]
			}
		}
	#
	# Copy the fret tuning values
	#
	for (i in fret_val)
		{
		split(i,temp_array,SUBSEP)
		if (temp_array[1] == current_index)
			fret_val[fret_index,temp_array[2]] \
							= fret_val[current_index,temp_array[2]]
		}
	#
	# Store the fret index in fret_array and update the fret index
	#
	fret_array[new] = fret_index
	fret_index += 1
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
	temp = fret_array[ex_array[1]]
	fret_array[ex_array[1]] = fret_array[ex_array[2]]
	fret_array[ex_array[2]] = temp
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
				# Call the appropriate function based on the exclusive 
				# interpretation in order to obtain kern output.
				#
				if (current_interp[j] == "**pitch")
					current_token = process_pitch($j)
				else if (current_interp[j] == "**Tonh")
					current_token = process_Tonh($j)
				else if (current_interp[j] == "**solfg")
					current_token = process_solfg($j)
				else if (current_interp[j] == "**semits")
					current_token = process_semits($j,j)
				else if (current_interp[j] == "**cents")
					current_token = process_cents($j,j)
				else if (current_interp[j] == "**freq")
					current_token = process_freq($j,j)
				else if (current_interp[j] == "**specC")
					current_token = process_specC($j,j)
				else	if (current_interp[j] == "**fret")
					current_token = process_fret($j,j)
				else if (current_interp[j] == "**MIDI")
					current_token = process_MIDI($j,j)
				else if (current_interp[j] == "**degree")
					current_token = process_degree($j,j)
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
##				Function Convert_to_kern
#	This function converts the note in current_note to a kern value and
# returns it to the calling function.
#
function convert_to_kern(    i,max_val,return_token)
	{
	if (current_note[1] >= 4)
		{
		current_note[2] = to_lower(current_note[2])
		max_val = current_note[1] - 3
		}
	else max_val = 4 - current_note[1]
	for (i = 1; i <= max_val; i += 1)
		return_token = return_token current_note[2]
	return_token = return_token current_note[3]
	return return_token
	}

########################################################################
##				Function to_lower
#	This function converts a character to lower case.
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

#############################################################################
##				Function Semits_to_kern
#	This function converts semits to kern notes
#
function semits_to_kern(semits,spine,  i,note,class,temp,deviation,pitch,arrayl)
	{
	if (semits+0 <= -0.5)
		{
		note = int(semits - .5)
		class = 3
		temp = note
		while (temp + 12 < 0) { temp += 12 ; class -= 1 }
		}
	else
		{
		note = int(semits + .5)
		class = 4
		temp = note
		while (temp - 12 >= 0) { temp -= 12 ; class += 1 }
		}
	deviation = int((semits - note) * 100)
	if (deviation == 0) deviation = ""
	else if (deviation > 0) deviation = "+" deviation
	for (i = 1; i <= 4; i += 1) arrayl[i] = ""
	if ((current_key[spine] ~ /ph/) \
					&& ((substr(current_key[spine],3)+0,note) in ph))
		{
		split(ph[substr(current_key[spine],3)+0,note],arrayl,"/")
		current_note[1] = class
		current_note[2] = arrayl[1]
		current_note[3] = arrayl[2]
		current_note[4] = deviation
		}
	else
		{
		pitch = ((note % 12) + 12) % 12
		if (current_key[spine] ~ key_reg)
			split(key[current_key[spine],pitch],arrayl,"/")
		else if (current_key[spine] ~ /pc/)
			split(pc[substr(current_key[spine],3)+0,pitch],arrayl,"/")
		else if ((current_key[spine] == "") || (current_key[spine] ~ /ph/))
			split(key["*C:",pitch],arrayl,"/")
		current_note[1] = class
		current_note[2] = arrayl[1]
		current_note[3] = arrayl[2]
		current_note[4] = deviation
		}
	return convert_to_kern()
	}

#########################################################################
##				Function Process_pitch
#	This function accepts pitch input and translates to kern 
#
function process_pitch(data_token,  return_token,arraya,j,split_num,not_found,
									accidentals,original,new)
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
				new = convert_to_kern()
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
#	This function takes Tonh input and translates to kern
#
function process_Tonh(data_token,   return_token,arrayb,j,split_num,not_found,
										accidentals,original,new)
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
				new = convert_to_kern()
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
#	This function takes solfg input and translates to kern
#
function process_solfg(data_token,  return_token,arrayc,j,split_num,not_found,\
									letter,accidentals,original,new)
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
				current_note[2] = \
						solfg_array[letter]
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
				new = convert_to_kern()
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

#####################################################################
##				Function Process_semits
#	This function takes semits input and translates to kern
#
function process_semits(data_token,spine,  arrayg,split_num,j,return_token,\
													original)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayg," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If a rest occurs, simply echo it
		#
		if (arrayg[j] ~ /r/) return_token = return_token arrayg[j] " "
		#
		# Otherwise, process the semits note
		#
		else
			{
			original = arrayg[j]
			if (match(arrayg[j],floating_pt_num))
				{
				note = substr(arrayg[j],RSTART,RLENGTH)+0
				sub(floating_pt_num,SUBSEP,arrayg[j])
				note = semits_to_kern(note,spine)
				sub(SUBSEP,note,arrayg[j])
				return_token = return_token arrayg[j] " "
				}
			else return_token = return_token original " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

####################################################################
##				Function Process_cents
#	This function takes cents input and translates to kern
#
function process_cents(data_token,spine,  arrayh,split_num,j,return_token,\
													original)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayh," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If a rest occurs, simply echo it
		#
		if (arrayh[j] ~ /r/) return_token = return_token arrayh[j] " "
		#
		# Otherwise, process the cents note
		#
		else
			{
			original = arrayh[j]
			if (match(arrayh[j],floating_pt_num))
				{
				note = substr(arrayh[j],RSTART,RLENGTH) * .01
				sub(floating_pt_num,SUBSEP,arrayh[j])
				note = semits_to_kern(note,spine)
				sub(SUBSEP,note,arrayh[j])
				return_token = return_token arrayh[j] " "
				}
			else return_token = return_token original " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#############################################################################
##				Function Process_specC
#	This function takes specC input and translates to kern
#
function process_specC(data_token,spine,  arraye,split_num,j,return_token,\
													original)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arraye," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If a rest occurs, simply echo it
		#
		if (arraye[j] ~ /r/) return_token = return_token arraye[j] " "
		#
		# Otherwise, process the specC note
		#
		else
			{
			original = arraye[j]
			if (match(arraye[j],floating_pt_num))
				{
				note = substr(arraye[j],RSTART,RLENGTH)+0
				sub(floating_pt_num,SUBSEP,arraye[j])
				#
				# Check for validity of log function
				#
				if (note > 0)
					note = log(note/middle_C_freq)/log(1.059463094)
				else
					{
					print "kern: ERROR: Invalid frequency value in "\
						 "line " NR "." > stderr
					exit
					}
				note = semits_to_kern(note,spine)
				sub(SUBSEP,note,arraye[j])
				return_token = return_token arraye[j] " "
				}
			else return_token = return_token original " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

#############################################################################
##				Function Process_freq
#	This function takes frequency input and translates to kern
#
function process_freq(data_token,spine,  arrayf,split_num,j,return_token,\
													original)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayf," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# If a rest occurs, simply echo it
		#
		if (arrayf[j] ~ /r/) return_token = return_token arrayf[j] " "
		#
		# Otherwise, process the freq note
		#
		else
			{
			original = arrayf[j]
			if (match(arrayf[j],floating_pt_num))
				{
				note = substr(arrayf[j],RSTART,RLENGTH)+0
				sub(floating_pt_num,SUBSEP,arrayf[j])
				#
				# Check for validity of log function
				#
				if (note > 0)
					note = log(note/middle_C_freq)/log(1.059463094)
				else
					{
					print "kern: ERROR: Invalid frequency value in "\
						 "line " NR "." > stderr
					exit
					}
				note = semits_to_kern(note,spine)
				sub(SUBSEP,note,arrayf[j])
				return_token = return_token arrayf[j] " "
				}
			else return_token = return_token original " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}

###########################################################################
##					Function Process_MIDI
#	This function takes MIDI input and translates to kern
#
function process_MIDI(data_token,spine,   note,arrayi,j,return_token,split_num\
													,midi_array)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayi," ")
	for (j = 1; j <= split_num; j += 1)
		{
		#
		# Process the MIDI data
		#
		split(arrayi[j],midi_array,"/")
		match(midi_array[2],/[+-]?[1-9][0-9]*/)
		note = substr(midi_array[2],RSTART,RLENGTH) + 0
		#
		# Only for positive midi notes is data output
		#
		if (note > 0)
			{
			note -= 60
			return_token = return_token semits_to_kern(note,spine) " "
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}
	
###########################################################################
##					Function Process_fret
#	This function takes fret input and translates to kern
#
function process_fret(data_token,position,   strings,fret_num,semits,arrayl,i,j\
						    ,old_semits,return_token,temp_index,split_num)
	{
	return_token = ""
	old_semits = 0
	#
	# Echo any rests that occur in the input
	#
	if (data_token ~ /r/) return_token = data_token " "
	else
		{
		temp_index = fret_array[position]
		split_num = split(data_token,arrayl," ")
		#
		# Loop through each course for the spine
		#
		for (j = 1; j <= split_num; j += 1)
			{
			#
			# Only process the strings that were struck
			#
			if (arrayl[j] !~ /x/ && arrayl[j] !~ /:/ && arrayl[j] !~ /-/)
				{
				#
				# There must be a value in array relative for each string
				#
				if (!((temp_index,j) in relative))
					{
					print "kern: ERROR: No pitch specified for string "\
						 "number " j " at line " NR "." > stderr
					exit
					}
				#
				# Loop through the separate strings in each course
				#
				for (i = 1; i <= split(relative[temp_index,j],strings,",");\
													        i += 1)
					{
					#
					# See if a fret is being pressed
					#
					if (match(arrayl[j],/[1-9][0-9]*/))
						{
						fret_num = substr(arrayl[j],RSTART,RLENGTH) + 0
						#
						# If a fret value is specified in fret_val, use it
						#
						if ((temp_index,fret_num) in fret_val)
							semits = absolute[temp_index] \
									+ strings[i] \
									+ fret_val[temp_index,fret_num]
						#
						# Otherwise, use a default
						#
						else semits = absolute[temp_index] \
									+ strings[i] + fret_num
						}
					#
					# Otherwise, just use the value of the string
					#
					else semits = absolute[temp_index] + strings[i]
					#
					# Construct returning token
					#
					if (semits != old_semits || return_token == "")
						return_token = return_token \
									semits_to_kern(semits,position) " "
					old_semits = semits
					}
				}
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}
	
############################################################################
##					Function Process_degree
#
function process_degree(data_token,spine,   arrayk,degree_array,degree,change\
									,note,accidental)
	{
	#
	# Process multiple stops
	#
	return_token = ""
	split_num = split(data_token,arrayk," ")
	for (j = 1; j <= split_num; j += 1)
		{
		change = ""
		#
		# If a rest occurs, simply echo it
		#
		if (arrayk[j] ~ /r/) return_token = return_token arrayk[j] " "
		#
		# Otherwise, process the degree note
		#
		else
			{
			original = arrayk[j]
			split(arrayk[j],degree_array,"/")
			#
			# Determine which degree position is specified
			#
			if (match(degree_array[1],/[1-7]/))
				{
				degree = substr(degree_array[1],RSTART,RLENGTH)
				#
				# Determine any pitch changes
				#
				if (degree_array[1] ~ /-/) change = "-"
				else if (degree_array[1] ~ /\+/) change = "+"
				#
				# Determin specified octave
				#
				if (match(degree_array[2],octave_class))
					current_note[1] \
								= substr(degree_array[2],RSTART,RLENGTH)
				else current_note[1] = 4
				#
				# Determine the pitch spelling from the current key
				#
				if (current_key[spine] ~ key_reg)
					split(deg_key[current_key[spine],degree],note,"/")
				else
					split(deg_key["*C:",degree],note,"/")
				#
				# Store the spelling and change accidentals if necessary
				#
				current_note[2] = note[1]
				current_note[3] = note[2]
				if (change == "-")
					{
					if (current_note[3] ~ /#/)
						sub(/#/,"",current_note[3])
					else if (current_note[3] ~ /-/)
						current_note[3] = current_note[3] "-"
					else
						current_note[3] = "-"
					}
				else if (change == "+")
					{
					if (current_note[3] ~ /-/)
						sub(/-/,"",current_note[3])
					else if (current_note[3] ~ /#/)
						current_note[3] = current_note[3] "#"
					else
						current_note[3] = "#"
					}
				current_note[4] = ""
				#
				# Convert and return the note
				#
				return_token = return_token convert_to_kern() " "
				}
			}
		}
	#
	# At least return a null token
	#
	if (return_token == "" || return_token == ".") return "."
	else return substr(return_token,1,length(return_token)-1)
	}
