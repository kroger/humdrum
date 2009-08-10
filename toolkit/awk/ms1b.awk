###############################################################################
#                                  MS1B.AWK
#
# Programmed by: David Huron        Date: January, 1997
# Copyright (c) 1997 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
#
#
# This program translates **kern input into MUP data.  It assumes as input,
# **kern data that has been amalgamated so that each non-null data record
# contains a single measure of input.  This can be done via the "context"
# command.
#
BEGIN	{
	FS=" "	# Space is the field separator.
	TRUE = 1; FALSE = 0
	NEWBAR = TRUE
	PROCESS = FIRST_NOTE_ENCOUNTERED = FALSE
	N_TUPLET = FALSE
	HEADER_PRINTED = FALSE
	PHRASE_PRINTED = FALSE
	LYRICS_PRINTED = FALSE
	REPEATSTART = FALSE
	KEY="0&"
	VOICE = 1		# Default "voice" in case no tandem
				# interpretations appear in the input.
	bar_number = 0
	count_duration = 1	# Assume quarter-note "denominator" if no meter.
	}
{
###############################################################################
#                          Ignore Null Data Records.
#

# Ignore null data tokens.
if ($0 ~ /^\.(	\.)*/) next

###############################################################################
#                          Process  COMMENTS
#

if ($0 ~/^!/)
	{
	# TITLE
	if ($0 ~/^!!!OTL:/)
		{
		gsub("^!!!OTL: *","",$0)
		title = $0
		next
		}
	# COMPOSER
	if ($0 ~/^!!!COM:/)
		{
		gsub("^!!!COM: *","",$0)
		if ($0 ~ /, /)
			{
			split($0,array,",")
			composer = array[2] " " array[1]
			}
		else composer = $0
		next
		}
	# MOVEMENT NUMBER
	if ($0 ~/^!!!OMV:/)
		{
		movement_number = $0
		gsub("^!!!OMV: *","",movement_number)
		gsub("[Mm]ov[a-z]*[.]*","Mov.",movement_number) # Standardize to "Mov."
		gsub("\.$","",movement_number)
		next
		}
	# MOVEMENT DESIGNATOR
	if ($0 ~/^!!!OMD:/)
		{
		gsub("^!!!OMD: *","",$0)
		movement_designation = $0
		next
		}
	# COPYRIGHT
	if ($0 ~/^!!!YEC:/)
		{
		gsub("^!!!YEC: *","",$0)
		copyright = $0
		next
		}
	# PAGE FOOTER LABEL
	if ($0 ~/^!!!APL:/)
		{
		gsub("^!!!APL: *","",$0)
		footer2 = $0
		next
		}
	# COMMENTS
	if ($0 ~ /^!/)
		{
		gsub("^\!*","//",$0)
		print $0
		next
		}
	}

###############################################################################
#                          Process  EXCLUSIVE INTERPRETATIONS
#
# Begin translation only if **kern input is encountered.
if ($0 ~/^\*\*kern/)
	{
	PROCESS=TRUE
	# The default number of staffs in the system equals the number of
	# **kern spines.
	staffs = gsub("\*\*kern","",$0)
	next
	}

###############################################################################
#                          SPINE-PATH INTERPRETATIONS
#

# For now, we process only spine-path terminators.
if ($0 == "*-")
	{
	PROCESS=FALSE
	next
	}

###############################################################################
#                          CONTINUE PROCESSING
#
# Continue processing only if **kern input is active.

if (!PROCESS)		{next}

###############################################################################
#                          Process  TANDEM INTERPRETATIONS
#

if ($0 == "*")	{next}	# (Eliminate null interpretations.)
if ($0 ~ /^\*[^\*]/)
	{
	# INSTRUMENT
	if ($0 ~ /^\*I[^A-Z]/)
		{
		if ($0 ~/^\*I"/)	# Literal instrument name takes precedent.
			{
			instrument = $0
			gsub("\*I\"","",instrument)
			}
		else			# Instrument code is second best.
			{
			if (instrument == "") instrument = find_instrument_name()
			}
		}
	# TRANSPOSITION
	if ($0 ~ /^\*IT/)
		{
		if ($0 == "*ITd-1c-2")	instrument = instrument " in B-flat"
		if ($0 == "*ITd-2c-3")	instrument = instrument " in A"
		}
	# CLEFS
	if ($0 ~ /^\*clef/)
		{
		if ($0 ~ /^\*clefG2$/)	CLEF = "clef = treble"
		if ($0 ~ /^\*clefGv2$/)	CLEF = "clef = treble8"
		if ($0 ~ /^\*clefC1$/)	CLEF = "clef = soprano"
		if ($0 ~ /^\*clefC2$/)	CLEF = "clef = mezzosoprano"
		if ($0 ~ /^\*clefC3$/)	CLEF = "clef = alto"
		if ($0 ~ /^\*clefC4$/)	CLEF = "clef = tenor"
		if ($0 ~ /^\*clefC5$/)	CLEF = "clef = baritone"
		if ($0 ~ /^\*clefF4$/)	CLEF = "clef = bass"
	
		if (HEADER_PRINTED)
			{
			print ""
			print "staff " STAFF
			print "        " CLEF
			print ""
			print "music"
			print ""
			LAST_PRINTED = "not a barline"
			}
		next
		}
	
	# PART PREFIX:  A "part" consists of a STAFF number followed by a VOICE number.
	PART = STAFF " " VOICE ": "
	
	# METER SIGNATURES
	if ($0 ~/^\*M[0-9]+\/[0-9]+$/ || $0 ~ /^\*M[X?]$/)
		{
		gsub("^\*M","",$0)
		METER = $0
		# (For irregular meters (*MX) set the meter to
		# an arbrarily long meter; but don't print it.)
		if (METER == "X") METER="15/2n"
		# (For explicitly "unknown" meters (*M?) set the meter
		# to the 4/4 default; but don't print it.)
		if (METER == "?") METER="4/4n"

		# Determine the default beaming style (if there are no
		# beams present).
		BEAMSTYLE = determine_beamstyle(METER)

		# If the header has already been printed, then the input
		# represents a *change* of meter -- so print it.
		if (HEADER_PRINTED)
			{
			print ""
			print "score"
			print "        time=" METER
			print "        beamstyle=" BEAMSTYLE
			print ""
			print "music"
			print ""
			LAST_PRINTED = "not a barline"
			}
		# Keep track of the "denominator" or "count" (MUP term).
		split(METER,array,"/")
		measure_duration = array[1] * (1/array[2])
		count=array[2]
		count_duration = find_duration(count)
		next
		}
	# KEY SIGNATURES
	if ($0 ~/^\*k\[/)
		{
		# "zero" any current accidentals from possible prior signature.
		for (i in accidental) accidental[i] = ""
		sharps = flats = 0
		gsub("^\\*k\\[","",$0);  gsub("\\]","",$0)
		sharps = gsub("#","",$0)
		flats  = gsub("-","",$0)
		if (sharps == 0 && flats == 0)	next
		if (sharps > flats)
			{
			KEY = sharps "#"
			if ($0 ~ /a/) accidental["a"]="sharp"
			if ($0 ~ /b/) accidental["b"]="sharp"
			if ($0 ~ /c/) accidental["c"]="sharp"
			if ($0 ~ /d/) accidental["d"]="sharp"
			if ($0 ~ /e/) accidental["e"]="sharp"
			if ($0 ~ /f/) accidental["f"]="sharp"
			if ($0 ~ /g/) accidental["g"]="sharp"
			}
		if (sharps <= flats)
			{
			KEY = flats "&"
			if ($0 ~ /a/) accidental["a"]="flat"
			if ($0 ~ /b/) accidental["b"]="flat"
			if ($0 ~ /c/) accidental["c"]="flat"
			if ($0 ~ /d/) accidental["d"]="flat"
			if ($0 ~ /e/) accidental["e"]="flat"
			if ($0 ~ /f/) accidental["f"]="flat"
			if ($0 ~ /g/) accidental["g"]="flat"
			}
		next
		}
	# KEYS
	if ($0 ~/^\*[a-gA-G][-#]*:/)
		{
		if ($0 ~ /[A-G]/)	MODE = "major"
		if ($0 ~ /[a-g]/)	MODE = "minor"
	
		if (HEADER_PRINTED)
			{
			print "        key=" KEY " " MODE
			LAST_PRINTED = "not a barline"
			}
		next
		}

	# SECTION LABELS
	# Process section labels but not expansion lists.
	if ($0 ~/^\*\>/ && $0 !~ /^\*\>.*\[.*\]/)
		{
		# The following two lines are a work-around to avoid bad behavior by some
		# awk interpreters:
		token = "XXX" $0
		gsub("^XXX\\*>","",token)
		#if (FIRST_NOTE_ENCOUNTERED) print "newcentury bold (15) above 1: 1.2 \"" token "\";"
		if (FIRST_NOTE_ENCOUNTERED) print "newcentury bold (15) above 1: " beat_position "  \"" token "\";"
		else SECTION = "newcentury bold (14) above 1: 1.0 \"" token "\";"
		}

	# Ignore all other tandem interpretations.
	next
	}

###############################################################################
#                          PROCESS DATA RECORDS
#
# Note that only data records (notes, rests and barlines) pass beyond this point.

# Reset cummulative duration and beat_position to zero for each measure.

cummulative_duration = 0
beat_position = 0

# Suppress possible leading barlines (that occur prior to any notes.
if ($0 ~/^=1-$/)	next	# (The obvious case.)
if ($0 ~/^=\|\|*:1$/)	# Watch for initial "repeat" barlines.
	{
	REPEATSTART = TRUE
	next
	}
if ($0 ~/^=([0-9]+[a-z]*)*$/)	next	# (The general case of an isolated barline.)

###############################################################################
#                          Print the MUP file header.
#
if (!FIRST_NOTE_ENCOUNTERED)
	{
	# If the first note had not yet been encountered, then output the
	# MUP "header" containing any stored title, composer, and copyright information.

	if (HEADER_PRINTED == FALSE)
		{
		generate_MUP_header()
		HEADER_PRINTED = TRUE
		}
	}

###############################################################################
#                          Process  NOTES and RESTS
#
record = $0
if (record ~ /<SEP>/) gsub("<SEP>[^ ]*","",record)	# Eliminate underlay material if present.

# First, determine whether the summed duration for the measure
# adds up to the meter signature.  If not, pad the measure with MUP spacer tokens.

padding = pad_measure(record,measure_duration)

# (If the last token is a barline, don't process it until later.)
if ($NF ~ /=/) last_token = NF -1
else last_token= NF
for (i=1; i<=last_token; i++)
	{
	if ($i ~ /<SEP>/)	# Keep tabs on possible underlay material.
		{
		OPTIONS = OPTIONS "U"	# ("U" is an internal option)
		split($i,aray,"<SEP>")
		token = aray[1]
		underlay_token = aray[2]
		if (underlay_token == "") underlay_token = "."
		}
	else token = $i

	# Set-up a few variables.
	# Set a global state.
	FIRST_NOTE_ENCOUNTERED = TRUE

	# States pertaining to the current sonority/note.
	EMPTY_PREFIX = TRUE
	EMPTY_SUFFIX = TRUE
	prefix = suffix = ""

	# If the -e option is selected, store the input as underlay.
	if (OPTIONS ~ /e/) underlay = underlay " " token
	if (OPTIONS ~ /U/) underlay = underlay " " underlay_token

	# (For now, treat slurs simply as phrases; i.e., translate ( to { etc.)
	gsub("\\(","{",token)
	gsub("\\)","}",token)

	# Process phrases.
	if (OPTIONS !~ /f/)
		{
		# Check for start and end phrases.  (N.B. This needs to be done before
		# changing kern flats (-) to MUP flats (&).  Otherwise "&}" may occur.)
		if (token ~ /[{}]/) process_phrase(token)
		gsub("[{}]","",token)
		}

	# Set aside any multiple-stops, and add later.  (N.B.  The "ms1a.awk"
	# preprocessor delimits multiple stops by the "+++" designation.)
	stops = ""
	if (token ~/+++/)
		{
		num_stops = split(token,multistops,"+++")
		for (j=2; j<=num_stops; j++)
			{
			# Process accidentals.
			multistops[j] = process_accidentals(multistops[j])

			stops = stops " " determine_pitch(multistops[j])
			}
		gsub("+++.*","",token)	# Make "token" only the first note.
		}

	token = process_accidentals(token)

	gsub("[/\\\\()Kk]","",token)	# Eliminate untranslated signifiers.
	gsub("[Pp]","",token)	# Appoggiatures not translated at this time.
	if (gsub("[ijlNVZ%+<>]","",token) > 0)	# Translate user-defined chars.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"X\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"X\""}
		#if (EMPTY_PREFIX) { prefix = "[with \"\\f(TX)X\\f(PV)\""; EMPTY_PREFIX = FALSE}
		#else {prefix = prefix ", \"\\f(TX)X\\f(PV)\""}
		}
	if (gsub("{","",token) > 0)	# If -f option has left phrase markings in the
		{			# token, then translate them.  First open phrases.
		if (EMPTY_PREFIX) { prefix = "[with \"{\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"{\""}
		}
	if (gsub("}","",token) > 0)	# If -f option has left phrase markings in the
		{			# token, then translate them.  Now close phrases.
		if (EMPTY_PREFIX) { prefix = "[with \"}\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"}\""}
		}
	if (gsub(";","",token) > 0)	# Translate fermatas.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\(ferm)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\(ferm)\""}
		}
	if (gsub("`","",token) > 0)	# Translate attacca/martellato marks (wedge)
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\(wedge)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\(wedge)\""}
		}
	if (gsub("~","",token) > 0)	# Translate tenutos.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\(leg)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\(leg)\""}
		}
	if (gsub("\\^","",token) > 0)	# Translate accent marks.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\(acc_gt)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\(acc_gt)\""}
		}
	if (gsub(",","",token) > 0)	# Translate breath marks.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\f(TX)    ,\\f(PV)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\f(TX)    ,\\f(PV)\""}
		}
	if (gsub("?","",token) > 0)	# Translate editorial footnotes.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"[NB.]\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"[NB.]\""}
		}
	if (gsub("I","",token) > 0)	# Translate generic articulations.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"[Art.]\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"[Art.]\""}
		}
	if (gsub("[Mm]","",token) > 0)	# Translate mordents.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\(mor)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\(mor)\""}
		}
	if (gsub("O","",token) > 0)	# Translate generic ornaments.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"[Orn.]\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"[Orn.]\""}
		}
	if (gsub("S","",token) > 0)	# Translate turns.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\(turn)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\(turn)\""}
		}
	if (gsub("[Tt]","",token) > 0)	# Translate trills.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\(tr)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\(tr)\""}
		}
	if (gsub("U","",token) > 0)	# Translate mute indications.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\f(TX)con sordino\\f(PV)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\f(TX)con sordino\\f(PV)\""}
		}
	if (gsub("[Ww]","",token) > 0)	# Translate inverted mordents.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\(invmor)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\(invmor)\""}
		}
	if (gsub("[Xxy]","",token) > 0)	# Translate editorial markers.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"[Ed.]\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"[Ed.]\""}
		}
	# Now that "x" has been translated ...
	# ... translate double-sharps to the MUP signifiers.
	gsub("##","x",token)	# Translate double sharps.
	if (gsub("Y","",token) > 0)	# Translate editorial "sic" markings.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"[sic]\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"[sic]\""}
		}
	if (gsub("o","",token) > 0)	# Translate harmonics.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\f(TX)o\\f(PV)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\f(TX)o\\f(PV)\""}
		}
	if (gsub("s","",token) > 0)	# Translate spiccatos.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\f(TX)spiccato\\f(PV)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\f(TX)spiccato\\f(PV)\""}
		}
	if (gsub("u","",token) > 0)	# Translate down-bows.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\(dnbow)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\(dnbow)\""}
		}
	if (gsub("v","",token) > 0)	# Translate up-bows.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\(upbow)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\(upbow)\""}
		}
	if (gsub("z","",token) > 0)	# Translate sforzandos.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\f(TX)sfz\\f(PV)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\f(TX)sfz\\f(PV)\""}
		}
	if (gsub("\\$","",token) > 0)	# Translate inverted turns.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\(invturn)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\(invturn)\""}
		}
	if (gsub("\"","",token) > 0)	# Translate pizzicato marks.
		{
		if (EMPTY_PREFIX) { prefix = "[with \"\\f(TX)pizz.\\f(PV)\""; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", \"\\f(TX)pizz.\\f(PV)\""}
		}
	staccato = gsub("'","",token)	# (This is to avoid quotation problems.)
	if (staccato > 0)	# Translate staccato marks.
		{
		if (EMPTY_PREFIX) { prefix = "[with ."; EMPTY_PREFIX = FALSE}
		else {prefix = prefix ", ."}
		}
	if (gsub("[[]","",token) > 0)	# Translate first tied notes.
		{
		if (EMPTY_SUFFIX) { suffix = "~"; EMPTY_SUFFIX = FALSE}
		else {suffix = suffix "~"}
		}
	if (gsub("[_]","",token) > 0)	# Translate continuing tied notes.
		{
		gsub("[x#n&]","",token)	# Eliminate any accidentals.
		if (EMPTY_SUFFIX) { suffix = "~"; EMPTY_SUFFIX = FALSE}
		else {suffix = suffix "~"}
		}
	if (gsub("[]]","",token) > 0)	# Translate final tied notes.
		{
		gsub("[x#n&]","",token)	# Eliminate any accidentals.
		}

	# Keep a running tab of the current beat position.
	note_duration = find_duration(token)
	cummulative_duration += note_duration
	beat_position = cummulative_duration / count_duration

	# Determine the MUP duration coding.
	coded_duration = code_duration(token)
	# Eliminate duration information for token itself.
	gsub("[0-9.]","",token)

	# Determine the pitch.
	token = determine_pitch(token)

	# Determine any beaming.
	beam = ""
	if (token ~ /[LJ]/)
		{
		if (OPTIONS !~ /f/)
			{
			# Avoid a few possible errors: Don't restart already active beams.
			if (token ~ /L/ && previous_beam != " bm") beam = " bm"
			# And don't terminate already terminated beams.
			if (token ~ /J/ && previous_beam != " ebm") beam = " ebm"
			previous_beam = beam
			}
		gsub("[LJ]*","",token)
		}

	if (prefix != "") prefix = prefix "] "
	token = begin_tuplet prefix coded_duration token stops suffix beam

	if (current_line == "")
		{
		current_line = PART token ";"
		NEWBAR = FALSE
		}
	else current_line = current_line " " token ";"
	}
# Process the BARLINE
	
# Terminate any active N-tuplets.
if (N_TUPLET == TRUE)
	{
	N_TUPLET = FALSE
	current_line = current_line end_tuplet
	}
NEWBAR = TRUE
print current_line padding

# With the -e option, echo the spine data as "lyrics" underlay:
if (OPTIONS ~/e/ || OPTIONS ~/U/)
	{
	# Replace minus sign and underscore character in the underlay
	# to avoid syllable parsing by Mup.
	gsub("-$","=",underlay)
	gsub("_","*",underlay)
	gsub("+++","+",underlay)	# Re-open multiple-stops.
	gsub("\\\\","&&",underlay)	# Escape backslash chars.
	# Reduce the current line to duration information only:
	gsub(".*: ","",current_line)
	gsub("\\[with [^ ]*\\] ","",current_line)
	duration_line = current_line padding
	n=split(duration_line,array,";")
	gsub("^ *","",array[1])	# Eliminate leading blanks.
	gsub(" .*","",array[1])	# Eliminate subsequent multiple-stops.
	gsub("[a-grs].*",";",array[1])	# Eliminate all but leading duration code.
	duration_line = array[1]
	for (i=2; i<=n; i++)
		{
		#if (array[i] ~ /with/) {i++; continue}	# Skip "with" "string".
		gsub("^ *","",array[i])	# Eliminate leading blanks.
		gsub(" .*","",array[i])	# Eliminate subsequent multiple-stops.
		gsub("[a-grs].*",";",array[i])	# Eliminate subsequent multiple-stops.
		#gsub("[^0-9.;]","",array[i])
		duration_line = duration_line " " array[i]
		}
	print "lyrics " STAFF ": " duration_line " \" " underlay underlay_pad "\";"
	LYRICS_PRINTED = TRUE
	}
if (!LYRICS_PRINTED) print "//lyrics"	# Print dummy lyrics comment for "combined".
LYRICS_PRINTED = FALSE

if (!PHRASE_PRINTED) print "//phrase"	# Print dummy phrase comment for "combined".
PHRASE_PRINTED = FALSE

current_line = underlay = underlay_pad = ""

# Reset cummulative duration to zero for each barline.
cummulative_duration = 0

# Reset bar-related accidentals to none.
for (x in bar_accident) delete bar_accident[x]

# If there are active phrases, up-date the measure count.
if (OPEN_PHRASE != "") CROSSED_BARLINES++

# Suppress outputting a barline until after the
# first note or rest has appeared.
if (FIRST_NOTE_ENCOUNTERED == FALSE) next

if ($NF ~ /^==/)
	{
	if ($NF ~ /^==\|\|/) print "dblbar"
	else if ($NF ~ /^==:/) print "repeatend"
	else print "endbar"
	}
else
	{
	# Process invisible barlines.
	if ($NF ~ /-/)
		{
		print "XXXinvisbar"	# (The "XXX" is used by ms1c.awk.)
		}
	# Process repeat barlines.
	else if ($NF ~ /:/)
		{
		if ($NF ~ /:\|\|*:/) print ++bar_number "XXXrepeatboth"
		else if ($NF ~ /:\|/) print ++bar_number "XXXrepeatend"
		else if ($NF ~ /\|:/) print ++bar_number "XXXrepeatstart"
		}
	# Process unnumbered barlines.
	else if ($NF !~ /=[0-9]/)
		{
		print "XXXbar"
		}
	# Numbered barlines.
	else
		{
		gsub("[^0-9a-z]","",$NF)
		if (OPTIONS ~ /m/)
			{
			if ($NF != "") print ++bar_number "XXXbar"
			else print bar_number++ "XXXbar"
			}
		else
			{
			if ($NF != "") print ++bar_number "XXXbar reh " "\"" $NF "\""
			else print bar_number++ "XXXbar"
			}
		}
	}
LAST_PRINTED = "barline"
next
}
END	{
	if (current_line != "") {print current_line; LAST_PRINTED = "not a barline"}
	if (LAST_PRINTED == "not a barline") print "bar"
	}

###############################################################################
#                          FUNCTIONS
#

###############################################################################
#                          GENERATE_MUP_HEADER
#
function generate_MUP_header()
	{
	print "header"
	if (OPTIONS !~/p/) SIZE=12	# Default point size.
	# In light of the available information, choose an appropriate left title.
	left_title = movement_number
	if (movement_designation != "") left_title = movement_designation
	print "        size=" SIZE
	print "        title bold (" SIZE+6 ") \"" title "\""
	print "        title bold (" SIZE+2 ") \"" left_title "\"        \"" composer "\""
	print ""
	LAST_PRINTED = "not a barline"

	# Be certain to retain any copyright notices.
	if (copyright != "")
		{
		gsub("[Cc]opyright","",copyright)	# Use the (C) character.
		print "footer"
		print ""
		print "        title bold (12) \"\\(copyright) " copyright "\""
		print "footer2"
		print "        title bold (12) \"" footer2 "\""
		print ""
		}

	print "score"
	print "        lyricssize=" SIZE
	print "        staffs=" staffs
	if (METER == "")	# Set a default meter if necessary.
		{
		print "        time=4/4n"	# Suppress printing this default.
		METER = "4/4"
		measure_duration = 1
		}
	else print "        time=" METER

	print "        beamstyle=" BEAMSTYLE

	if (CLEF == "") CLEF = "clef = treble"	# Default clef if none specified.
	print ""
	print "staff " STAFF
	print "        " CLEF
	print "        key=" KEY
	print "        label=\"" instrument "\""
	#if (KEY != "do not print" && KEY != "") print "        key=" KEY " " MODE
	print ""
	print "music"
	print ""

	if (REPEATSTART)
		{
		print PART "ms;"
		print "repeatstart"
		print ""
		}

	if (SECTION != "")
		{
		print SECTION
		#print ""
		}

	}
###############################################################################
#                          DETERMINE_BEAMSTYLE
#
# This function determines the default beaming style based on the meter
# signature.

function determine_beamstyle(METER)
	{
	split(METER,array,"/")
	if (array[1] == 1) beat = array[2]
	if (array[1] == 2) beat = array[2] "," array[2]
	if (array[1] == 3) beat = array[2] "," array[2] "," array[2]
	if (array[1] == 4) beat = array[2] "," array[2] "," array[2] "," array[2]
	if (array[1] == 5) beat = array[2] "," array[2] "," array[2] "," array[2] "," array[2]
	if (array[1] == 6 || array[1] == 9 || array[1] == 12 || array[1] == 15)
		{
		dots = array[2]
		gsub("\.","",dots)
		double = array[2] / 2
		compound = double "." dots

		if (array[1] == 6) beat = compound "," compound
		if (array[1] == 9) beat = compound "," compound "," compound
		if (array[1] == 12) beat = compound "," compound "," compound "," compound
		if (array[1] == 15) beat = compound "," compound "," compound "," compound "," compound

		if (array[1] == 7) beat = compound "," array[2] "," array[2]
		if (array[1] == 8) beat = double "," double "," double "," double
		if (array[1] == 10) beat = double "," double "," double "," double "," double
		if (array[1] == 11) beat = compound "," double "," double "," double "," double
		if (array[1] == 13) beat = compound "," compound "," double "," double "," double
		}
	return beat
	}
###############################################################################
#                          PAD_MEASURE
#
# This function ensures that the accumulated duration for a measure
# adds up to the meter signature (as required by MUP).  If not,
# "silent notes" are added.
#
function pad_measure(record,measure_duration,        temp)
	{
	temp = record		# Process only a copy of the input record.
	gsub("=.*","",temp)	# Eliminate any trailing barline signifier.
	if (temp !~ / $/) temp = temp " "
	# Eliminate non-duration information.
	gsub("[^0-9. ]","",temp)
	split(temp,array," ")

	total_duration = 0
	for (i in array) total_duration += find_duration(array[i])

	if (total_duration > measure_duration)
		{
		print "ms1b.awk: ERROR: Accumulated duration in measure, line " NR ", exceeds prevailing meter signature."
		exit
		}
	if (total_duration < measure_duration)
		{
		difference =  measure_duration - total_duration
		duration = 1/difference
		print "//difference: " difference
		if (duration == 1 || duration == 2 || duration == 4 || duration == 8)
			{
			underlay_pad = " ."
			return " " duration "s;"
			}
		if (difference == 0.0625)	{underlay_pad = " ."; return " 16s;"}
		if (difference == 0.1875)	{underlay_pad = " ."; return " 8.s;"}
		if (difference == 0.3125)	{underlay_pad = " . ."; return " 16s; 4s;"}
		if (difference == 0.375)	{underlay_pad = " . ."; return " 8s; 4s;"}
		if (difference == 0.5625)	{underlay_pad = " . . ."; return " 16s; 8s; 4.s;"}
		if (difference == 0.625)	{underlay_pad = " . ."; return " 8s; 2s;"}
		if (difference == 0.6875)	{underlay_pad = " . ."; return " 8.s; 2s;"}
		if (difference == 0.71875)	{underlay_pad = " . ."; return " 8..s; 2s;"}
		if (difference == 0.75)		{underlay_pad = " ."; return " 2.s;"}
		if (difference == 0.78125)	{underlay_pad = " . ."; return " 32s; 2.s;"}
		if (difference == 0.8125)	{underlay_pad = " . ."; return " 16s; 2.s;"}
		if (difference == 0.90625)	{underlay_pad = " . ."; return " 32s; 2..s;"}
		if (difference == 0.875)	{underlay_pad = " ."; return " 2..s;"}
		if (difference == 0.9375)	{underlay_pad = " ."; return " 2...s;"}
		if (difference == 0.96875)	{underlay_pad = " ."; return " 2....s;"}
		if (difference == 1.25)		{underlay_pad = " . ."; return " 4s; 1s;"}
		if (difference == 1.5)		{underlay_pad = " ."; return " 1.s;"}
		if (difference == 1.75)		{underlay_pad = " . . ."; return " 4s; 2s; 1s;"}
		if (difference == 2.0)		{underlay_pad = " . ."; return " 1s; 1s;"}
		if (difference == 2.5)		{underlay_pad = " . . ."; return " 1s; 1s; 2s;"}
		if (difference == 2.75)		{underlay_pad = " . . ."; return " 1s; 1s; 2.s;"}
		if (difference == 3.0)		{underlay_pad = " . . ."; return " 1s; 1s; 1s;"}
		if (difference == 3.25)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 4s;"}
		if (difference == 3.5)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 2s;"}
		if (difference == 3.75)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 2.s;"}
		if (difference == 4.0)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 1s;"}
		if (difference == 4.25)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 1s; 4s;"}
		if (difference == 4.5)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 1s; 2s;"}
		if (difference == 4.75)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 1s; 2.s;"}
		if (difference == 5.0)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 1s; 1s;"}
		if (difference == 5.25)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 1s; 1s; 4s;"}
		if (difference == 5.5)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 1s; 1s; 2s;"}
		if (difference == 5.75)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 1s; 1s; 2.s;"}
		if (difference == 6.0)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 1s; 1s; 1s;"}
		if (difference == 6.25)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 1s; 1s; 1s; 4s;"}
		if (difference == 6.5)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 1s; 1s; 1s; 2s;"}
		if (difference == 6.75)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 1s; 1s; 1s; 2.s;"}
		if (difference == 7.0)		{underlay_pad = " . . ."; return " 1s; 1s; 1s; 1s; 1s; 1s; 1s;"}
		}
	}

###############################################################################
#                          DETERMINE_PITCH
#
# This function determines the MUP pitch value from the **kern pitch.
#
function determine_pitch(token)
	{
	if (token ~ /[a-g]/) octave = 4
	if (token ~ /[A-G]/) octave = 3
	if (token ~ /[a-g][a-g]/)
		{
		while (token ~ /[a-g][a-g]/)
			{
			sub("aa","a",token)
			sub("bb","b",token)
			sub("cc","c",token)
			sub("dd","d",token)
			sub("ee","e",token)
			sub("ff","f",token)
			sub("gg","g",token)
			octave++
			}
		}
	if (token ~ /[A-G][A-G]/)
		{
		octave = 3
		while (token ~ /[A-G][A-G]/)
			{
			sub("AA","A",token)
			sub("BB","B",token)
			sub("CC","C",token)
			sub("DD","D",token)
			sub("EE","E",token)
			sub("FF","F",token)
			sub("GG","G",token)
			octave--
			}
		}
	sub("A","a",token)	# Translate to lower-case.
	sub("B","b",token)
	sub("C","c",token)
	sub("D","d",token)
	sub("E","e",token)
	sub("F","f",token)
	sub("G","g",token)
	
	if (token ~/r/) octave = ""
	token = token octave
	return token
	}
###############################################################################
#                          PROCESS_ACCIDENTALS
#
# This function eliminates unnecessary accidentals.

function process_accidentals(value)
	{
	#print "IN FUNCTION, value is: " value
	if (value ~ /[-#n]/)
		{
		# Eliminate accidentals that are already in the key signature.
		if (value ~ /[Aa]/ && accidental["a"]=="sharp") sub("#","",value)
		if (value ~ /[Bb]/ && accidental["b"]=="sharp") sub("#","",value)
		if (value ~ /[Cc]/ && accidental["c"]=="sharp") sub("#","",value)
		if (value ~ /[Dd]/ && accidental["d"]=="sharp") sub("#","",value)
		if (value ~ /[Ee]/ && accidental["e"]=="sharp") sub("#","",value)
		if (value ~ /[Ff]/ && accidental["f"]=="sharp") sub("#","",value)
		if (value ~ /[Gg]/ && accidental["g"]=="sharp") sub("#","",value)

		if (value ~ /[Aa]/ && accidental["a"]=="flat") sub("-","",value)
		if (value ~ /[Bb]/ && accidental["b"]=="flat") sub("-","",value)
		if (value ~ /[Cc]/ && accidental["c"]=="flat") sub("-","",value)
		if (value ~ /[Dd]/ && accidental["d"]=="flat") sub("-","",value)
		if (value ~ /[Ee]/ && accidental["e"]=="flat") sub("-","",value)
		if (value ~ /[Ff]/ && accidental["f"]=="flat") sub("-","",value)
		if (value ~ /[Gg]/ && accidental["g"]=="flat") sub("-","",value)

		# Eliminate accidentals that occurred previously in the measure.
		# Determine the type of modification for the current pitch.
		subscript = value
		gsub("[^A-Ga-g#n-]","",subscript)
		if (value ~ /#/) modification = gsub("#","",subscript) " #"
		if (value ~ /-/) modification = gsub("-","",subscript) " &"
		if (value ~ /n/) modification = gsub("n","",subscript) " n"

		# Check whether this pitch is already so modified.
		if (bar_accident[subscript] == modification)
			{
			# If so, eliminate the unnecessary accidentals.
			gsub("[-#n]","",value)
			return value
			}
		# Otherwise record the modification. 
		else bar_accident[subscript] = modification

		# NORMALLY, THIS WOULD BE A GOOD PLACE TO TRANSLATE ACCIDENTALS
		# FROM **kern TO MUP.  HOWEVER, CHANGING ## to "x" CAUSES
		# PROBLEMS WITH EDITORIAL ("[Ed.]") OUTPUTS.  SO PROCESSING
		# THE CHANGE OF ACCIDENTALS IS DELAYED UNTIL LATER.
		# Translate any remaining accidentals to the MUP signifiers.
		gsub("-","\\&",value)	# Translate flats.
		#gsub("##","x",value)	# Translate double sharps.

		}
	# Also watch out for missing naturals.
	else
		{
		subscript = value
		gsub("[^A-Ga-g]","",subscript)
		if (bar_accident[subscript] ~ /[#&]/)
			{
			value = value "n"
			# Don't forget to add any new natural to the bar_accidental array.
			bar_accident[subscript] = "1 n"
			}
		}
	return value
	}
###############################################################################
#                          PROCESS_PHRASE
#
# This function determines the placement of phrase beginnings and endings.
#
function process_phrase(token)
	{
	if (token ~ /&{/ || token ~ /&}/)
		{
		print "ERROR: token is: " token
		print "ms1b.awk: ERROR: Cannot currently process elided phrase in line " NR "."
		exit
		}
	if (token ~ /}/ && token ~/{/)
		{
		print "ms1b.awk: ERROR: Cannot currently process phrases beginning and "\
			"ending on the same note."
		exit
		}
	if (token ~ /{/)
		{
		OPEN_PHRASE = beat_position + 1
		OPEN_PHRASE_BAR = bar_number + 1
		}
	if (token ~ /}/)
		{
		CLOSE_PHRASE = beat_position + 1
		if (CROSSED_BARLINES != 0) CLOSE_PHRASE = CROSSED_BARLINES "m+" CLOSE_PHRASE
		print OPEN_PHRASE_BAR "XXXphrase " STAFF ": " OPEN_PHRASE " til " CLOSE_PHRASE ";"
		PHRASE_PRINTED = TRUE
		OPEN_PHRASE = CLOSE_PHRASE = ""
		CROSSED_BARLINES = 0
		}
	}
###############################################################################
#                          CODE_DURATION
#
# This function translates **kern reciprocal durations to MUP-type duration
# signifiers.  N.B. MUP uses reciprocal durations except for N-tuplets.
#
function code_duration(temp)
	{
	gsub("[^0-9.]","",temp)

	# Process regular durations independently of n-tuplets.
	if (temp ~ /^0\.*$/ || temp ~ /^1\.*$/ || temp ~ /^2\.*$/ || temp ~ /^4\.*$/ \
		|| temp ~ /^8\.*$/ || temp ~ /^16\.*$/ || temp ~ /^32\.*$/ \
		|| temp ~ /^64\.*$/ || temp ~ /^128\.*$/ || temp ~ /^256\.*$/)
		{
		if (N_TUPLET == TRUE)
			{
			N_TUPLET = FALSE
			current_line = current_line end_tuplet
			}
		return temp
		}
	else	# Process n-tuplets.
		{
		if (temp ~ /^24\.*$/)
			{
			if (N_TUPLET == FALSE)
				{
				N_TUPLET = TRUE
				begin_tuplet = " { "
				end_tuplet =  " } 3; "
				}
			else
				{
				begin_tuplet = ""
				}
			duration = 16
			return duration
			}
		if (temp ~ /^12\.*$/)
			{
			if (N_TUPLET == FALSE)
				{
				N_TUPLET = TRUE
				begin_tuplet = " { "
				end_tuplet =  " } 3; "
				}
			else
				{
				begin_tuplet = ""
				}
			duration = 8
			return duration
			}
		if (temp ~ /^6\.*$/)
			{
			if (N_TUPLET == FALSE)
				{
				N_TUPLET = TRUE
				begin_tuplet = " { "
				end_tuplet =  " } 3; "
				}
			else
				{
				begin_tuplet = ""
				}
			duration = 4
			return duration
			}
		if (temp ~ /^3\.*$/)
			{
			if (N_TUPLET == FALSE)
				{
				N_TUPLET = TRUE
				begin_tuplet = " { "
				end_tuplet =  " } 3; "
				}
			else
				{
				begin_tuplet = ""
				}
			duration = 2
			return duration
			}
		}
	}
###############################################################################
#                          FIND_DURATION
#
# This function determines the duration of an input in absolute units.
# (Don't confuse this with the "code_duration" function, which translates
# **kern durations to MUP duration signifiers.
#
function find_duration(temp)
	{
	num_of_dots = gsub("[.]","",temp)

	if (temp == 0) basic_duration = 2
	else basic_duration = 1 / (temp * 1.0)
	duration = basic_duration

	# Now consider augmentation dots.
	dotted_duration = 0.5 * basic_duration
	for (j=1; j<=num_of_dots; j++)
		{
		duration += dotted_duration
		dotted_duration = dotted_duration * 0.5
		}
	return duration
	}
###############################################################################
#                          FIND_INSTRUMENT_NAME
#
# This function eliminates unnecessary accidentals.

function find_instrument_name()
	{
	# The most frequent instruments first.
	if ($0 == "*Ivioln")	return "Violin"
	if ($0 == "*Ipiano")	return "Pianoforte"
	if ($0 == "*Iviola")	return "Viola"
	if ($0 == "*Icello")	return "Violoncello"
	if ($0 == "*Iflt")	return "Flauto"
	if ($0 == "*Ioboe")	return "Oboe"
	if ($0 == "*Icor")	return "Cor"
	if ($0 == "*Ifagot")	return "Fagotto"
	if ($0 == "*Icemba")	return "Harpsichord"
	if ($0 == "*Iorgan")	return "Organ"
	if ($0 == "*Itromp")	return "Trumpet"
	if ($0 == "*Iclars")	return "Clarinet"
	if ($0 == "*Iliuto")	return "Lute"
	if ($0 == "*Icbass")	return "Contrabass"
	if ($0 == "*Isoprn")	return "Soprano"
	if ($0 == "*Itenor")	return "Tenor"
	if ($0 == "*Ibass")	return "Bass"
	if ($0 == "*Ialto")	return "Alto"
	if ($0 == "*Ivox")	return "Voice"
	if ($0 == "*Itimpa")	return "Timpani"
	if ($0 == "*Ifag_c")	return "Contrafagotto"

	# The remaining instruments in alphabetical order.
	if ($0 == "*Iaccor")	return "Accordion"
	if ($0 == "*Iarchl")	return "Archlute"
	if ($0 == "*Iarmon")	return "Harmonica"
	if ($0 == "*Iarpa")	return "Harp"
	if ($0 == "*IbagpI")	return "Bagpipe"
	if ($0 == "*IbagpS")	return "Bagpipe"
	if ($0 == "*Ibanjo")	return "Banjo"
	if ($0 == "*Ibarit")	return "Baritone"
	if ($0 == "*Ibdrum")	return "B. Drum"
	if ($0 == "*Ibguit")	return "Elec. B. Guitar"
	if ($0 == "*Ibiwa")	return "Biwa"
	if ($0 == "*Ibscan")	return "Basso Cantante"
	if ($0 == "*Ibspro")	return "Basso Profondo"
	if ($0 == "*Icalam")	return "Chalumeau"
	if ($0 == "*Icalpe")	return "Calliope"
	if ($0 == "*Icalto")	return "Contralto"
	if ($0 == "*Icampn")	return "Bell"
	if ($0 == "*Icangl")	return "Cor Anglais"
	if ($0 == "*Icaril")	return "Carillon"
	if ($0 == "*Icastr")	return "Castrato"
	if ($0 == "*Icasts")	return "Castanets"
	if ($0 == "*Icetra")	return "Cittern"
	if ($0 == "*Ichime")	return "Chimes"
	if ($0 == "*Ichlma")	return "Alto Shawm"
	if ($0 == "*Ichlms")	return "Soprano Shawm"
	if ($0 == "*Ichlmt")	return "Tenor Shawm"
	if ($0 == "*Iclara")	return "Alto Clarinet"
	if ($0 == "*Iclarb")	return "Bass Clarinet"
	if ($0 == "*Iclarp")	return "Piccolo Clarinet"
	if ($0 == "*Iclavi")	return "Clavichord"
	if ($0 == "*Iclest")	return "Celesta"
	if ($0 == "*Icolsp")	return "Coloratura Soprano"
	if ($0 == "*Icornm")	return "Cornemuse"
	if ($0 == "*Icorno")	return "Cornett"
	if ($0 == "*Icornt")	return "Cornet"
	if ($0 == "*Icrshc")	return "Crash Cymb."
	if ($0 == "*Ictenor")	return "Counter-Tenor"
	if ($0 == "*Ictina")	return "Concertina"
	if ($0 == "*Idrmsp")	return "Dramatic Soprano"
	if ($0 == "*Idulc")	return "Dulcimer"
	if ($0 == "*Ieguit")	return "Elec. Guitar"
	if ($0 == "*Ifalse")	return "Falsetto"
	if ($0 == "*Ifeme")	return "Female Voice"
	if ($0 == "*Ifife")	return "Fife"
	if ($0 == "*Ifingc")	return "Finger Cymb."
	if ($0 == "*Iflt_a")	return "Alto Flute"
	if ($0 == "*Iflt_b")	return "Bass Flute"
	if ($0 == "*Ifltda")	return "Alto Recorder"
	if ($0 == "*Ifltdb")	return "Bass Recorder"
	if ($0 == "*Ifltdn")	return "Sopranino"
	if ($0 == "*Ifltds")	return "Soprano Recorder"
	if ($0 == "*Ifltdt")	return "Tenor Recorder"
	if ($0 == "*Iflugh")	return "Flugelhorn"
	if ($0 == "*Iforte")	return "Fortepiano"
	if ($0 == "*Iglock")	return "Glockenspiel"
	if ($0 == "*Igong")	return "Gong"
	if ($0 == "*Iguitr")	return "Guitar"
	if ($0 == "*Ihammd")	return "Hammond Organ"
	if ($0 == "*Iheltn")	return "Heldentenor"
	if ($0 == "*Ihichi")	return "Hichiriki"
	if ($0 == "*Ihurdy")	return "Hurdy-gurdy"
	if ($0 == "*Ikit")	return "Kit"
	if ($0 == "*Ikokyu")	return "Kokyu"
	if ($0 == "*Ikomun")	return "Komun'go"
	if ($0 == "*Ikoto")	return "Koto"
	if ($0 == "*Ikruma")	return "Alto Crumhorn"
	if ($0 == "*Ikrumb")	return "Bass Crumhorn"
	if ($0 == "*Ikrums")	return "Soprano Crumhorn"
	if ($0 == "*Ikrumt")	return "Tenor Crumhorn"
	if ($0 == "*Ilyrsp")	return "Lyric Soprano"
	if ($0 == "*Ilyrtn")	return "Lyric Tenor"
	if ($0 == "*Imale")	return "Male Voice"
	if ($0 == "*Imando")	return "Mandolin"
	if ($0 == "*Imarac")	return "Maracas"
	if ($0 == "*Imarim")	return "Marimba"
	if ($0 == "*Imezzo")	return "Mezzo"
	if ($0 == "*Infant")	return "Child's Voice"
	if ($0 == "*Inokan")	return "Nokan"
	if ($0 == "*IoboeD")	return "Oboe d'amore"
	if ($0 == "*Iocari")	return "Ocarina"
	if ($0 == "*Ipanpi")	return "Panpipe"
	if ($0 == "*Ipiatt")	return "Piatti"
	if ($0 == "*Ipicco")	return "Piccolo"
	if ($0 == "*Ipipa")	return "Chinese Lute"
	if ($0 == "*Iporta")	return "Portative Organ"
	if ($0 == "*Ipsalt")	return "Psaltery"
	if ($0 == "*Iqin")	return "Qin"
	if ($0 == "*Iquitr")	return "Gittern"
	if ($0 == "*Irackt")	return "Racket"
	if ($0 == "*Irebec")	return "Rebec"
	if ($0 == "*Irecit")	return "Recitativo"
	if ($0 == "*Ireedo")	return "Reed Organ"
	if ($0 == "*Irhode")	return "Fender-Rhodes Piano"
	if ($0 == "*Iridec")	return "Ride Cymbal"
	if ($0 == "*Isarod")	return "Sarod"
	if ($0 == "*Isarus")	return "Sarrusophone"
	if ($0 == "*IsaxA")	return "Alto Saxophone"
	if ($0 == "*IsaxB")	return "Bass Saxophone"
	if ($0 == "*IsaxC")	return "Contrabass Saxophone"
	if ($0 == "*IsaxN")	return "Sopranino Saxophone"
	if ($0 == "*IsaxR")	return "Baritone Saxophone"
	if ($0 == "*IsaxS")	return "Soprano Saxophone"
	if ($0 == "*IsaxT")	return "Tenor Saxophone"
	if ($0 == "*Isdrum")	return "Snare drum"
	if ($0 == "*Ishaku")	return "Shakuhachi"
	if ($0 == "*Ishami")	return "Shamisen"
	if ($0 == "*Isheng")	return "Sheng"
	if ($0 == "*Isho")	return "Sho"
	if ($0 == "*Isitar")	return "Sitar"
	if ($0 == "*Ispshc")	return "Splash Cymbal"
	if ($0 == "*Isteel")	return "Steel-drum"
	if ($0 == "*IsxhA")	return "Alto Saxhorn"
	if ($0 == "*IsxhB")	return "Bass Saxhorn"
	if ($0 == "*IsxhC")	return "Contrabass Saxhorn"
	if ($0 == "*IsxhR")	return "Baritone Saxhorn"
	if ($0 == "*IsxhS")	return "Soprano Saxhorn"
	if ($0 == "*IsxhT")	return "Tenor Saxhorn"
	if ($0 == "*Isynth")	return "Synthesizer"
	if ($0 == "*Itabla")	return "Tabla"
	if ($0 == "*Itambn")	return "Tambourine"
	if ($0 == "*Itambu")	return "Tambura"
	if ($0 == "*Itanbr")	return "Tanbur"
	if ($0 == "*Itiorb")	return "Theorbo"
	if ($0 == "*Itom")	return "Tom-Tom"
	if ($0 == "*Itrngl")	return "Triangle"
	if ($0 == "*Itromb")	return "Bass Trombone"
	if ($0 == "*Itromt")	return "Tenor Trombone"
	if ($0 == "*Ituba")	return "Tuba"
	if ($0 == "*Iud")	return "Ud"
	if ($0 == "*Iukule")	return "Ukulele"
	if ($0 == "*Ivibra")	return "Vibraphone"
	if ($0 == "*Ivina")	return "Vina"
	if ($0 == "*Iviolb")	return "Bass Viola da Gamba"
	if ($0 == "*Iviold")	return "Viola d'amore"
	if ($0 == "*Iviols")	return "Treble viola da gamba"
	if ($0 == "*Iviolt")	return "Tenor viola da gamba"
	if ($0 == "*Ixylo")	return "Xylophone"
	if ($0 == "*Izithr")	return "Zither"
	if ($0 == "*Izurna")	return "Zurna"
	
	# Anything else.
	temp_inst = $0
	gsub("\*I","",temp_inst)
	return temp_inst

	# For the future:
	if ($0 == "*ICbras")	return "Brass instrument"
	if ($0 == "*ICidio")	return "Percussion"
	if ($0 == "*ICklav")	return "Keyboard instrument"
	if ($0 == "*ICstr")	return "String instrument"
	if ($0 == "*ICvox")	return "Voice"
	if ($0 == "*ICww")	return "Woodwind instrument"
	}
