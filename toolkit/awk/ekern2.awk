#####################################################################
##                          EKERN.AWK
#
# Programmed by:  David Huron     Date: 1994 August and 1995 June
# Copyright (c) 1995 David Huron
#
# Modifications:
#   Date:      Programmer:    Description:
#
# The purpose of this program is to translate music encoded in the
# Essen Associative Code (ESAC) to the **kern representation.
#
# The ESAC MEL code characters include the following:
#
# 0   - rest
# 1   - tonic (principal octave)
# 2   - supertonic (principal octave)
# etc.
# 7   - leading tone (principal octave)
# -1  - tonic (lower octave)
# +1  - tonic (higher octave)
#  #  - raised tone
#  b  - lowered tone
#  _  - duration doubler
#  .  - augmentation dot
#  ^  - tie between preceding note and current duration
#  ~  - meaning unknown; not present in the databases; possibly early
#       version of tie (^) since ^ is not listed in ESAC documentation
#  x  - meaning unknown (appears to have duration like a pitch or rest)
# ()  - triplets indicator [not yet implemented in "ekern"]
# <sp>- two blank spaces designate barlines
# //  - end of melody
#
#  Pitches may range between --1 and ++7.
#  Accidentals are places following the pitch symbol.
#  Barlines are signified by two blanks.
#
# - The  characters used  in ESAC are: 0 1 2 3 4 5 6 7 ~ x + - _ . b
#     #. Then  there are special signs like ( ) / and the BLANK. Any
#     other character would be marked as "unidentified".
#
#     Illegitimate combination include:
#       +-5__ , 7bb, -3b_.__  or 1_..
#     [Evidently doubly-dotted notes are forbidden, as are double-flats.]
#
BEGIN	{
	YES = TRUE = 1
	NO = FALSE = 0
	single_record = TRUE
	lines = 1
	diatonic["C"] = 0; diatonic_number[0] = "C"
	diatonic["D"] = 1; diatonic_number[1] = "D"
	diatonic["E"] = 2; diatonic_number[2] = "E"
	diatonic["F"] = 3; diatonic_number[3] = "F"
	diatonic["G"] = 4; diatonic_number[4] = "G"
	diatonic["A"] = 5; diatonic_number[5] = "A"
	diatonic["B"] = 6; diatonic_number[6] = "B"
	key["C"] = "";
	sharp_key["G"] = "F";
	sharp_key["D"] = "FC";
	sharp_key["A"] = "FCG";
	sharp_key["E"] = "FCGD";
	sharp_key["B"] = "FCGDA";
	sharp_key["F#"] = "FCGDAE";
	sharp_key["C#"] = "FCGDAEB";
	flat_key["G"] = "";
	flat_key["D"] = "";
	flat_key["A"] = "";
	flat_key["E"] = "";
	flat_key["B"] = "";
	flat_key["F#"] = "";

	flat_key["F"] = "B";
	flat_key["Bb"] = "BE";
	flat_key["Eb"] = "BEA";
	flat_key["Ab"] = "BEAD";
	flat_key["Db"] = "BEADG";
	flat_key["Gb"] = "BEADGC";
	flat_key["Cb"] = "BEADGCF";
	flat_key["Cb"] = "BEADGCF";

	key_signature["C-"] = "b-e-a-d-g-c-f-";
	key_signature["C"] = ""; key_signature["C#"] = "f#c#g#d#a#e#b#";
	key_signature["c"] = "c-e-a-"; key_signature["c#"] = "f#c#g#d#";
	key_signature["D"] = "f#c#"; key_signature["D-"] = "b-e-a-d-g-";
	key_signature["d-"] = "b--e-a-d-g-c-f-"; key_signature["d"] = "b-";
	key_signature["E"] = "f#c#g#d#"; key_signature["E-"] = "b-e-a-";
	key_signature["e-"] = "b-e-a-d-g-c-"; key_signature["e"] = "f#";
	key_signature["F"] = "b-"; key_signature["F#"] = "f#c#g#d#a#e#";
	key_signature["f"] = "b-e-a-d-"; key_signature["f#"] = "f#c#g#";
	key_signature["G-"] = "b-e-a-d-g-c-"; key_signature["G"] = "f#";
	key_signature["g-"] = "b--e--a-d-g-c-f-"; key_signature["g"] = "b-e-";
	key_signature["G#"] = "f##c#g#d#a#e#b#"; key_signature["A-"] = "b-e-a-d-";
	key_signature["g#"] = "f#c#g#d#a#"; key_signature["a-"] = "b-e-a-d-g-c-f-";
	key_signature["A"] = "f#c#g#"; key_signature["B-"] = "b-e-";
	key_signature["a"] = ""; key_signature["b-"] = "b-e-a-d-g-";
	key_signature["B"] = "f#c#g#d#a#"; key_signature["b"] = "f#c#";
	}
{
# ACC  - possible melodic incipit or extract?   *
# BEM  - further source-related information?
# CAD  - possible melodic incipit or extract?   *
# CMT  - comment
# CNR  - first line / title?   *
# CUT  - title of piece
# ETH  - ethnicity of piece
# F    - database name
# FA   - [unknown]
# KEY  - work # within collection, shortest note, tonic pitch, meter signature
# FCT  - genre or function
# FKT  - genre or function ?
# MEL  - pitch and duration
# MOD  - [unknown]
# REG  - geographical region
# REM  - comment
# SRC  - published source
# ST   - [unknown]
# STN  - [unknown]
# TRD  - [unknown]
# TTR  - [unknown]
# TXT  - text / lyrics   *
# ZZ   - unknown
#
# Catch any continuations of multi-record data types.
if (!single_record)
	{
	single_record = gsub("\]$","",$0)
	print "!!" $0
	next
	}
if ($0 ~ /^$/)  # Blank lines separate individual pieces.
	{
	mel_processing = FALSE
	for (i=1; i<lines; i++) {print end_comments[i]; delete end_comments[i]}
	lines = 1
	# Print some trailing comments:
	print "!!!AMT: " meter_classification
	print "!!!AIN: vox"
	#print "!!!ENC: Helmut Schaffrath"
	print "!!!EED: Helmut Schaffrath"
	print "!!!EEV: 1.0"
	print "*-"   # Output spine-path terminator.
	meter_classification = ""
	next         # Eliminate empty lines.
	}
if ($0 ~/^[[:upper:]]+$/)  # Single upper-case words appear to indicate the
	{            # type of database.
	end_comments[lines++] = "!!!ONB: ESAC (Essen Associative Code) Database: " $0
	next
	}
if ($0 ~/^BEM/)  # The "BEM" keyword pertains to source-related information.
	{
	gsub("^BEM\\[","!! ",$0)
	single_record = gsub("\]$","",$0)
	print $0
	next
	}
if ($0 ~/^CMT/ || $0 ~/^REM/)  # The "CMT" and "REM" keywords denotes comments.
	{
	gsub("^CMT\\[","!! ",$0)
	gsub("^REM\\[","!! ",$0)
	single_record = gsub("\]$","",$0)
	print $0
	next
	}
if ($0 ~/^CUT/)  # The "CUT" keyword denotes the title of the piece.
	{
	output_line = $0
	gsub("^CUT\\[","!!!OTL: ",output_line)
	while (output_line !~ /\]$/)
		{
		getline
		gsub("^ +","",$0)  # Eliminate leading blanks.
		output_line = output_line " " $0
		}
	gsub("\]$","",output_line)  # Eliminate trailing bracket.
	print output_line
	next
	}
if ($0 ~/^ETH/)  # The "ETH" keyword denotes the "ethnicity" of the piece.
	{
	gsub("^ETH\\[","!! Ethnic Group: ",$0)
	# Change a few spellings:
	gsub("Araber","Arab",$0)
	gsub("Indianer","American Indian",$0)
	single_record = gsub("\]$","",$0)
	print $0
	next
	}
if ($0 ~/^F[^[:upper:]]/)  # The "F" keyword denotes the database name to which
	{         # the piece belongs.
	gsub("^F\\[","",$0)
	single_record = gsub("\]$","",$0)
	end_comments[lines++] = "!!!ONB: ESAC (Essen Associative Code) Database: " $0
	next
	}
if ($0 ~/^FA[^[:upper:]]/)  # The "FA" keyword denotes unknown information.
	{
	gsub("^FA\\[","!! ",$0)
	single_record = gsub("\]$","",$0)
	print $0
	next
	}
if ($0 ~/^F[CK]T/)  # The "FCT" or "FKT" keyword denotes the social "function"
	{           # or genre of the piece.
	gsub("^FCT\\[","",$0)
	gsub("^FKT\\[","",$0)
	single_record = gsub("\]$","",$0)
	print "!!!AGN: " $0
	next
	}
if ($0 ~/^KEY/)  # The "KEY" record identifies the work number within
                 # the collection ($1), the shortest note ($2), the
                 # tonic pitch ($3), and the meter signature ($4).
	{
	# First output some additional comments.
	work_number = $1
	gsub("^KEY\\[","",work_number)
	print "!!!SCT: " work_number
	print "!!!YEM: Copyright 1995, estate of Helmut Schaffrath."
	print "**kern"
	print "*ICvox"
	print "*Ivox"
	single_record = gsub("\]$","",$0)
	time_units = $2
	gsub("^0","",time_units)  # Remove any leading zeros.
	# Assign the tonic (remember: "H" = B natural; "B" = B-flat)
	tonic = $3
	if (tonic == "B") tonic = "Bb"
	if (tonic == "H") tonic = "B"

	# Determine and print the meter signature.  There are three
	# possibilities: (1) a free-meter (designated FREI), (2) mixed
	# meter signatures (more than one meter), and (3) a single
	# meter signature.
	
	if ($4 == "FREI" || (NF >4 && $5 !~/%/))  # (Watch out for "free" and multiple meters.)
		{
		if ($4 == "FREI")
			{
			print "!! Irregular meter."
			print "*MX"
			meter_classification = "irregular"
			free_meter = TRUE
			}
		else	# Process mixed meters.
			{
			# Determine the bar lengths for each of the meters.
			meters = $4
			meter_signature[1] = $4
			bar_duration[1] = calculate_bar_duration($4)
			no_of_meters = 1
			for (x=5; x<=NF; x++)
				{
				if ($x ~/%/)
					{
					gsub("%","",$x)
					print "!! " $x " metric grouping"
					continue
					}
				no_of_meters++
				meters = meters "; " $x
				meter_signature[no_of_meters] = $x
				bar_duration[no_of_meters] = calculate_bar_duration($x)
				}
			print "!! Mixed meters:  " meters
			print "*MZ"
			meter_classification = "irregular"
			free_meter = TRUE
			}
		}
	else
		{
		free_meter = FALSE
		if ($5 ~/%/)
			{
			gsub("%","",$5)
			print "!! " $5 " metric grouping"
			}
		print "*M" $4
		if ($4 ~ /^2\//) meter_classification = "simple duple"
		if ($4 ~ /^3\//) meter_classification = "simple triple"
		if ($4 ~ /^4\//) meter_classification = "simple quadruple"
		if ($4 ~ /^5\//) meter_classification = "simple quintuple"
		if ($4 ~ /^6\//) meter_classification = "compound duple"
		if ($4 ~ /^9\//) meter_classification = "compound triple"
		if ($4 ~ /^12\//) meter_classification = "compound quadruple"
		if (meter_classification == "") meter_classification = "irregular"

		# Calculate the duration for a complete measure.
		no_of_meters = 1
		bar_duration[1] = calculate_bar_duration($4)
		}
	next
	}
if ($0 ~/^MOD/)  # The "MOD" keyword denotes unknown information.
	{
	gsub("^MOD\\[","!! ",$0)
	single_record = gsub("\]$","",$0)
	print $0
	next
	}
if ($0 ~/^REG/)  # The "REG" keyword denotes the "region" of the piece.
	{
	gsub("^REG\\[","!!!ARE: ",$0)
	# Change a few spellings:
	gsub("Amerika","America",$0)
	gsub("Kanada","Canada",$0)
	gsub("Mexiko","Mexico",$0)
	gsub("Spanien","Espana",$0)
	gsub("Mittelamerika","Central America",$0)
	gsub("Italien","Italia",$0)
	gsub("Irland","Ireland",$0)
	gsub("Schweden","Sverige",$0)
	gsub("Syrien","Syria",$0)
	gsub("Turkei","Turkey",$0)
	gsub("Frankreich","France",$0)
	gsub("Baltikum","Balkan",$0)
	gsub("Jugoslawien","Jugoslavija",$0)
	gsub("Russland","Rossiya",$0)
	gsub("(DDR)","",$0)
	gsub("ARE: Osteuropa","ARE: Europa, Osteuropa",$0)
	gsub("ARE: Mitteleuropa","ARE: Europa, Mitteleuropa",$0)
	gsub("ARE: Suedosteuropa","ARE: Europa, Suedosteuropa",$0)
	gsub("ARE: China","ARE: Asia, China",$0)
	gsub("Asien","Asia",$0)
	gsub(" +,",",",$0)
	single_record = gsub("\]$","",$0)
	print $0
	next
	}
if ($0 ~/^SRC/)  # The "SRC" keyword denotes source-related information
	{
	gsub("^SRC\\[","!!!YOR: ",$0)
	single_record = gsub("\]$","",$0)
	print $0
	next
	}
if ($0 ~/^ST/)  # The "ST" and "STN" keyword denote unknown information.
	{
	gsub("^ST\\[","!! ",$0)
	gsub("^STN\\[","!! ",$0)
	single_record = gsub("\]$","",$0)
	print $0
	next
	}
if ($0 ~/^TXT/)  # The "TXT" keyword denotes text/lyrics information.
	{
	current_line = $0
	while (current_line !~/\]/)
		{
		getline
		current_line = $0
		}
	next
	}
if ($0 ~/^TRD/)  # The "TRD" keyword denotes unknown information.
	{
	gsub("^TRD\\[","!! ",$0)
	single_record = gsub("\]$","",$0)
	print $0
	next
	}
if ($0 ~/^TTR/)  # The "TTR" keyword denotes unknown information.
	{
	gsub("^TTR\\[","!! ",$0)
	single_record = gsub("\]$","",$0)
	print $0
	next
	}
if ($0 ~/^ZZ[^[:upper:]]/)  # The "ZZ" keyword denotes unknown information.
	{
	gsub("^ZZ\\[","!!ZZ ",$0)
	single_record = gsub("\]$","",$0)
	print $0
	next
	}
if ($0 ~/^MEL/ || mel_processing) # The "MEL" keyword denotes the pitch and
	{                      # rhythm data for the piece.

	if ($0 ~/^MEL/)
		{
		#measure_number = 1
		if (mel_processing) print "	ERROR: Second \"MEL\" statement"\
					" encountered in input line " NR "."
		mel_processing = TRUE
		# Some set-up tasks.
		total_duration = 0

		# Check whether the melody begins with an anarcrusis
		# of whether it begins with a barline (leading spaces).
		if ($0 ~/^MEL. /) measure_number = 1
		else measure_number = 0 # (Anacrusis condition)
		gsub("^MEL\\[","",$0)   # Eliminate the "MEL" keyword
		gsub("^ +","",$0)       # Eliminate leading blanks.

		# Determine the key.
		kern_tonic = tonic
		# Change ESAC flats to **kern flats.
		gsub("b","-",kern_tonic)
		# (Also remember the diatonic name of the tonic -- without
		#  the accidental:)
		tonic_name = tonic
		gsub("[#b]","",tonic_name)

		# Determine whether the key should be major or minor.
		# Search for lowered mediant or submediant tones as a sign
		# of a minor key.
		if ($0 ~/3b/ || $0 ~/6b/ && $0 !~ /3[^b]/)
			{
			kern_tonic = tolower(kern_tonic)
			print "*k[" key_signature[kern_tonic] "]"
			print "*" kern_tonic ":"
			}
		else	{
			print "*k[" key_signature[kern_tonic] "]"
			print "*" kern_tonic ":"
			}
		}

	if ($0 ~/\/\//)        # Keep watch for the double barline.
		{
		gsub(" \/\/.*","",$0)
		double_bar = YES
		}
	gsub("^ +","",$0)      # Eliminate leading blanks (non MEL
				       # records only), and identify.

	# Before splitting up measures, check for ties that cross barlines.
	if ($0 ~ /  ^/)
		{
		gsub("  ^","[  ^",$0)
		}

	# Most measures are separated by two spaces.
	# Insert barlines in the current record.
	gsub("  "," = ",$0)

	# Now split the current data record into space-delineated tokens.
	no_of_tokens = split($0,array," ")

	# Process each space-delineated token separately.
	for (i=1; i<=no_of_tokens; i++)
		{
		# Re-write data token so each note/rest/barline is
		# separated by a space.
		gsub("[-+(0-7^]"," &",array[i])
		gsub("- ","-",array[i])
		gsub("+ ","+",array[i])
		gsub("\\( ","(",array[i])
		no_of_notes = split(array[i],notes," ") # Split apart notes.

		# Process for ties.
		for (j=1; j<=no_of_notes; j++)
			{
			if (notes[j] ~ /\^/)
				{
				if (notes[j-1] != "=") notes[j-1] = notes[j-1] "["
				else notes[j-2] = notes[j-2] "["
				}
			}

		# Now process each individual note/rest/barline token.
		for (j=1; j<=no_of_notes; j++)
			{
			xtemp = notes[j]

			# Process BARLINES.

			# Barlines require three different treatments.
			# (1) The explicit presence of a barline.
			if (notes[j] == "=")
				{
				output_meter_sig()
				print "=" measure_number++
				total_duration = 0
				continue
				}
			# (2) The beginning of a record might require
			# a preceding barline.
			if (j == 1)
				{
				barline_print = NO
				if (measure_number ==1) barline_print = YES

				output_meter_sig()

				if (barline_print)
					{
					print "=" measure_number++
					total_duration = 0
					}
				}
			# (3) When there is an anacrusis, avoiding printing.
			if (measure_number == 0)
				{
				measure_number++
				total_duration = 0
				}

			# Eliminate some input characters that
			# should not be present.
			gsub("\\]","",notes[j])

			#
			# Process special case of tied notes of unit duration.
			if (notes[j] == "^")
				{
				special_tied_note()
				continue
				}

			# Process NOTES and RESTS.

			# Determine the duration of each note/rest.
			# Durations are indicated by the number of underscore
			# characters and any augmentation dots (periods).
			num = gsub("_","",notes[j])
			duration = time_units / 2^num
			# Revise notes/rests that are longer than a whole note.
			if (duration < 1)
				{
				if (duration == 0.5) duration = "0"
				if (duration == 0.25) duration = "00"
				}
			dots = ""
			number_of_dots = gsub("\\.","",notes[j])
			for (k=1; k<=number_of_dots; k++) dots = dots "."
			# Also watch out for triplets.
			begin_triplet = gsub("\\(","",notes[j])
			end_triplet = gsub("\\)","",notes[j])
			if (begin_triplet) triplet = TRUE
			if (triplet)
				{
				duration += duration * 0.5
				if (end_triplet) triplet = FALSE
				}

			# Maintain a running count of the elapsed duration
			# for the current bar.
			if (duration ~ /00/) current_duration = 4
			else if (duration == "0") current_duration = 2
			else current_duration = 1 / duration
			dot_value = 0.5 * current_duration
			note_duration = current_duration
			for (k=1; k<=number_of_dots; k++)
				{
				note_duration = note_duration + dot_value
				dot_value = 0.5 * dot_value
				}
			total_duration += note_duration

			# Determine the pitch for notes.
			# First look for tied notes.
			# ... notes starting a tie ...
			begin_tie = end_tie = ""
			if (notes[j] ~/\[/)
				{
				begin_tie = "["
				gsub("\\[","",notes[j])
				}
			# ... notes ending a tie ...
			if (notes[j] ~/\^/)
				{
				pitch = previous_pitch
				end_tie = "]"
				}
			else
				{
				# Determine pitch (or rest).
				pitch = calculate_pitch(notes[j])
				previous_pitch = pitch
				}

			# Each input line represents a single phrase.
			begin_phrase = end_phrase = ""
			if (i==1 && j==1) begin_phrase = "{"
			if (i==no_of_tokens && j==no_of_notes) end_phrase="}"
			if (pitch ~/r/)  # Avoid printing rests with ties.
				{
				print begin_phrase duration dots pitch end_phrase # " " total_duration " " xtemp
				}
			else print begin_phrase begin_tie duration dots pitch end_tie end_phrase # " " total_duration " " xtemp
			}
		}
	if (double_bar) {print "=="; double_bar = NO}
	}
}
function calculate_pitch(note)
	{
	# Identify possible rests.
	if (note == 0) return "r"

	# Now process pitches.
	# (1) Save any accidental information in the pitch token.
	accidental = note
	gsub("[-+0-9]","",accidental)

	# (2) Both ESAC and **kern use # for the sharp sign.  But
	#     translate ESAC flats ("b") to **kern flats ("-").
	if (accidental == "b") accidental = "-"

	# (3) Now eliminate the accidental information from the token.
	gsub("[b#]","",note)

	# (4) Identify whether the pitch is in the principal octave,
	#     or the higher or lower octave.
	octave = "principal"
	count = gsub("+","",note)
	if (count == 1) octave = "higher"
	if (count == 2) octave = "highest"
	count = gsub("-","",note)
	if (count == 1) octave = "lower"
	if (count == 2) octave = "lowest"
	#if (gsub("+","",note) > 0) octave = "higher"
	#if (gsub("-","",note) > 0) octave = "lower"

	# (5) Identify the pitch letter name.
	tonic_number = diatonic[tonic_name]
	pitch_name = diatonic_number[((tonic_number+note-1)%7)]

	# (6) Modify pitches according to default key signature.
	sharp = flat = ""
	if (sharp_key[tonic] ~ pitch_name) accidental = accidental "#"
	if (flat_key[tonic] ~ pitch_name) accidental = accidental "-"
	if (accidental ~ /-/ && accidental ~ /#/) accidental = "n"

	# (7) Modify case of pitch-name according to the octave.
	if (octave == "principal")
		{
		if (diatonic[pitch_name] < diatonic[tonic_name])
			{
			#pitch_name = toupper(pitch_name)
			pitch_name = tolower(pitch_name) "" tolower(pitch_name)
			}
		else pitch_name = tolower(pitch_name)
		}
	if (octave == "lower")
		{
		if (diatonic[pitch_name] < diatonic[tonic_name])
			{
			pitch_name = tolower(pitch_name)
			}
		}
	if (octave == "lowest")
		{
		if (diatonic[pitch_name] < diatonic[tonic_name])
			{
			pitch_name = toupper(pitch_name)
			}
		else pitch_name = toupper(pitch_name) "" toupper(pitch_name)
		}
	if (octave == "higher")
		{
		if (diatonic[pitch_name] < diatonic[tonic_name])
			{
			pitch_name = tolower(pitch_name) "" tolower(pitch_name) "" tolower(pitch_name)
			}
		else pitch_name = tolower(pitch_name) "" tolower(pitch_name)
		}
	if (octave == "highest")
		{
		if (diatonic[pitch_name] < diatonic[tonic_name])
			{
			pitch_name = tolower(pitch_name) "" tolower(pitch_name) "" tolower(pitch_name) "" tolower(pitch_name)
			}
		else pitch_name = tolower(pitch_name) "" tolower(pitch_name) "" tolower(pitch_name)
		}
	return pitch_name "" accidental
	}
function special_tied_note()
	{
	# This function prints the last note of a tie which has a duration
	# equivalent to the minimum time-unit.

	if (previous_pitch ~/r/)  # Avoid printing rests with ties.
		{
		print time_units previous_pitch end_phrase # " " total_duration " " xtemp
		}
	else print time_units previous_pitch "]" end_phrase # " " total_duration " " xtemp
	total_duration += 1/time_units
	return
	}
function calculate_bar_duration(token)
	{
	if (split(token,temp,"/") != 2)
		{
		print "ekern: ERROR: Incorrect meter specification "\
		      "in line " NR "."
		}
	dots = ""
	number_of_dots = gsub("\\.","",temp[2])
	temp[2] = 1 / temp[2]
	dot_value = 0.5 * temp[2]
	denumerator = temp[2]
	for (k=1; k<=number_of_dots; k++)
		{
		denumerator = denumerator + dot_value
		dot_value = 0.5 * dot_value
		}
	return temp[1] * denumerator
	}
function output_meter_sig()
	{
	for (k=1; k<=no_of_meters; k++)
		{
		if (total_duration == bar_duration[k])
			{
			barline_print = YES
			if (no_of_meters > 1 && previous_meter != k)
			#if (no_of_meters > 1)
				{
				print "!*M" meter_signature[k]
				previous_meter = k
				}
			}
		}
	}
