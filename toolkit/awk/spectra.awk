###########################################################################
#                                 SPECTRA
# Programmed by: Keith Mashinter & David Huron        Date: March, 1994
# Copyright (c) 1994 Keither Mashinter & David Huron
#
# Modifications:
#   Date:      Programmer: 	     Description:
#
#
#        This program generates a **spect (acoustic spectral data) output
# from a **semits score input.  It recognizes instrument tandem
# interpretations (e.g. *Iclarinet)
# and fetches a corresponding spectral data file (e.g. clarinet.spe).
#	The following gives a brief summary of the available options:
#
# Main Functions used:
#
# Helper Functions used:
#       function loadtimbre() - gets the spectral data for some instrument
#
#	VARIABLES:
#
#	ARRAYS:
#
#
BEGIN {
	# Pipe all errors to this variable so that error messages are always
	# printed to the terminal.  (Use print "message" > stderr)
	#
	stderr = "/dev/stderr"
	#
	YES = 1; NO = 0
	PROCESS = NO
	#
	GAIN = 57	# Set 0 dB relative amplitude to 57 dB SPL.
	THRESHOLD = 0	# Set hearing threshold to 0 dB SPL.
	#
	FS = "\t"
	OFS = "\t"

	# Determine the path for the Humdrum spectral files.
	#echo "$HUMDRUM > $TMPDIR/humdir.tmp"
	
	"echo $HUMDRUM" | getline humdrum_dir
}
{
# Process Exclusive and Tandem Interpretations
outputline = ""
if ($0 ~ /^\*/)
	{
	# First, process exclusive interpretations.
	if ($0 ~ /^\*\*/)
		{
		for (i=1; i<=NF; i++)
			{
			if ($i == "**semits")
				{
				process[i] = YES
				PROCESS = YES
				$i = "**spect"
				}
			}
		print
		next
		}
	# Second, process tandem interpretations (including
	# spine path indicators).

	# (NOTE: If we encounter more than one instrument interpretations
	# on the current line, we will need to keep track of all
	# fields for the current line, so as not to interfere with the
	# getline command in the loadtimbre function.

	for (i=1; i<=NF; i++) input_field[i] = $i
	n_infields = NF
	for (i=1; i<=n_infields; i++)
		{
		if (input_field[i] ~ /^\*[+vx^]$/)
			{
			print "spect: ERROR : spect command unable "\
			       "to handle spine-path indicators."
			exit
			}
		# Check for instrument interpretations.
		if (process[i] && input_field[i] ~ /^\*I[^C]/)
			{
			newinstrument = input_field[i]
			sub("\*I","",newinstrument)
			for (j in instruments)
				{
				if (newinstrument == instruments[j])
					{
					instruments[i] = newinstrument
					newinstrument = ""
					}
				}
			if (newinstrument != "")
				{
				instruments[i] = newinstrument
				loadtimbre(newinstrument)
				}
			}
		}
	}
# Process data records.
else	{
	if (!PROCESS) {print; next}

	# PROCESS BARLINES
	if ($1 ~ /^=/) {print; next}

	for (i=1; i<=NF; i++)
		{
		# Process only spines containing instrumental notes (in semits).
		if (instruments[i] == "" )
			{
			outputline = outputline $i OFS
			continue
			}
		num_tokens = split($i,tokens," ")
		for (j=1; j<=num_tokens; ++j)
		  {
		  token = tokens[j]
		  if (token !~  "[-]*[0-9]+") {continue}
		  num_fields=split(spectra[instruments[i],token],fields,"\t")
		  if (num_fields == 0)
			  {
			  print "spect: ERROR: Pitch " token \
			  " out of range for instrument \"" instruments[i] \
			  "\" in line " NR "." > stderr
			  exit
			  #instruments[i] = ""
			  #outputline = outputline $i OFS
			  #continue
			  }
		  for (k=4; k<=num_fields; k++)
			  {
			  frequency =  fields[3] * (k-3)
			  amplitude = fields[k] + GAIN
			  if (amplitude > THRESHOLD)
			    outputline = outputline frequency ";" amplitude " "
			  }
		  }
		sub(" $","",outputline)
		outputline = outputline OFS
		}
	sub(OFS "$","",outputline)
	print outputline
	}
}
function loadtimbre(instrumt_name)
	{
	# Read in the template file into the array "spectra".
	filename = humdrum_dir "/spectra.dat/" instrumt_name ".spe"
	while ((filestatus = getline < filename) > 0)
		{
		sub(/[\t\.]+$/,"",$0)  # Eliminate null tokens at end of line.
		# Only store data records; cue on **pitch data.
		if ($1 ~ "^[A-G].?[0-9]") # (Only process records beginning
					  # with pitches.)
			{
			spectra[instrumt_name,$2] = $0
			}
		}
	if (filestatus == -1)
		{
		print "spect: ERROR: No spectral data for instrument \"" \
		   instrumt_name "\" -- referenced in line " NR "." > stderr
		}
	}
