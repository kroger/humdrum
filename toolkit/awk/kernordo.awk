# KERNORDO.AWK
#
# This program reorders the signifiers within **kern inputs so that
# they are in canonical order.
#
BEGIN	{
	FS = "\t"
	#
	#          Canonical ordering:
	type[01] = open_phrase_elision  = "\&{";        replace[01] = "&{"
	type[02] = open_phrase_mark     = "{";          replace[02] = "{"
	type[03] = open_slur_elision    = "\&\\(";      replace[03] = "&("
	type[04] = open_slur            = "\\(";        replace[04] = "("
	type[05] = open_tie             = "\\[";        replace[05] = "["
	type[06] = duration             = "[0-9]+";     replace[06] = ""
	type[07] = augmentation_dot     = "\\.+";       replace[07] = ""
	type[08] = pitch_or_rest        = "[abcdefgABCDEFGr]+"; replace[08] = ""
	type[09] = sharp                = "#+";         replace[09] = ""
	type[10] = flat                 = "-+";         replace[10] = ""
	type[11] = natural              = "n";          replace[11] = "n"
	type[12] = pause                = ";";          replace[12] = ";"
	type[13] = begin_glissando      = "H";          replace[13] = "H"
	type[14] = end_glissando        = "h";          replace[14] = "h"
	type[15] = generic_ornament     = "O";          replace[15] = "O"
	type[16] = whole_mordent        = "M";          replace[16] = "M"
	type[17] = half_mordent         = "m";          replace[17] = "m"
	type[18] = turn                 = "S";          replace[18] = "S"
	type[19] = inverted_turn        = "\\$";        replace[19] = "$"
	type[20] = whole_trill          = "T";          replace[20] = "T"
	type[21] = half_trill           = "t";          replace[21] = "t"
	type[22] = whole_inv_mordent    = "W";          replace[22] = "W"
	type[23] = half_inv_mordent     = "w";          replace[23] = "w"
	type[24] = end_turn             = "R";          replace[24] = "R"
	type[25] = harmonic             = "j";          replace[25] = "j"
	type[26] = appoggiatura1        = "p";          replace[26] = "p"
	type[27] = appoggiatura2        = "P";          replace[27] = "P"
	type[28] = acciaccatura         = "q";          replace[28] = "q"
	type[29] = groupetto            = "Q";          replace[29] = "Q"
	type[30] = generic_articulation = ":";          replace[30] = "I"
	type[31] = spiccato             = "s";          replace[31] = "s"
	type[32] = sforzando            = "z";          replace[32] = "z"
	type[33] = staccato             = "'";          replace[33] = "'"
	type[34] = pizzicato            = "\"";         replace[34] = "\""
	type[35] = attacca              = "`";          replace[35] = "`"
	type[36] = tenuto               = "~";          replace[36] = "~"
	type[37] = accent               = "\\^";        replace[37] = "^"
	type[38] = arpeggiation         = ":";          replace[38] = ":"
	type[39] = up_bow               = "u";          replace[39] = "u"
	type[40] = down_bow             = "v";          replace[40] = "v"
	type[41] = up_stem              = "\\\\";       replace[41] = "\\"
	type[42] = down_stem            = "/";          replace[42] = "/"
	type[43] = open_beam            = "L+";         replace[43] = ""
	type[44] = closed_beam          = "J+";         replace[44] = ""
	type[45] = left_partial_beam    = "k+";         replace[45] = ""
	type[46] = right_partial_beam   = "K+";         replace[46] = ""
	type[47] = user_defined1        = "i+";         replace[47] = ""
	type[48] = user_defined2        = "l+";         replace[48] = ""
	type[49] = user_defined3        = "N+";         replace[49] = ""
	type[50] = user_defined4        = "U+";         replace[50] = ""
	type[51] = user_defined5        = "V+";         replace[51] = ""
	type[52] = user_defined6        = "Z+";         replace[52] = ""
	type[53] = user_defined7        = "@+";         replace[53] = ""
	type[54] = user_defined8        = "%+";         replace[54] = ""
	type[55] = user_defined9        = "\++";        replace[55] = ""
	type[56] = user_defined11       = "|+";         replace[56] = ""
	type[57] = user_defined12       = "<+";         replace[57] = ""
	type[58] = user_defined13       = ">+";         replace[58] = ""
	type[59] = close_tie            = "]";          replace[59] = "]"
	type[60] = continuing_tie       = "_";          replace[60] = "_"
	type[61] = close_slur_elision   = "\&\\)";      replace[61] = "&)"
	type[62] = close_slur           = "\\)";        replace[62] = ")"
	type[63] = close_phrase_elision = "\&}";        replace[63] = "&}"
	type[64] = close_phrase         = "}";          replace[64] = "}"
	type[65] = breath_mark          = ",";          replace[65] = ","
	type[66] = editorial_mark1      = "X+";         replace[66] = ""
	type[67] = editorial_mark2      = "x+";         replace[67] = ""
	type[68] = editorial_mark3      = "y";          replace[68] = "y"
	type[69] = editorial_mark4      = "Y";          replace[69] = "Y"
	type[70] = editorial_mark5      = "?+";         replace[70] = ""
	}
{
# Echo comments and interpretations.
if ($0 ~ /^[!*]/)
	{
	print $0
	next
	}
# Process first spine.

if ($1 ~ / /)
	{
	num_of_sub_tokens = split($1,subtoken," ")
	multi_stop = process_token(subtoken[1])
	for (x=2; x<=num_of_sub_tokens; x++)
		{
		multi_stop = multi_stop " " process_token(subtoken[x])
		}
	line = multi_stop
	}
else line = process_token($1)

# Process any subsequent spines.

for (i=2; i<=NF; i++)
	{
	if ($i ~ / /)
		{
		num_of_sub_tokens = split($i,subtoken," ")
		multi_stop = process_token(subtoken[1]);
		for (x=2; x<=num_of_sub_tokens; x++)
			{
			multi_stop = multi_stop " " process_token(subtoken[x])
			}
		line = line "	" multi_stop
		}
	else
		{
		line = line "	" process_token($i)
		}
	}
print line
}
function process_token(token)
	{
	# Determine if the token is a (1) null token, (2) barline,
	# (3) rest, or (4) note.
	new = ""
	if (token == ".")	# Process null token.
		{
		return "."
		}
	if (token ~ /^=/)	# Process barlines.
		{
		return token
		}
	if (token ~ /r/)	# Process rests.
		{
		}
	else			# Process notes.
		{
		for (j=1; j<=70; j++)
			{
			#print "type test is: " j
			if (token ~ type[j])
				{
				if (replace[j] != "")
					{
					new = new replace[j]
					}
				else
					{
					temp = token
					sub(type[j],"!!!&===",temp)
					#print "::: " temp
					sub(/.*!!!/,"",temp)
					sub(/===.*/,"",temp)
					#print "::: " temp
					#pos=index(token,type[j])
					#length(
					#string=substr(token,pos,1)
					#sub(type[j],"",token)
					new = new temp
					}
				}
			}
		return new
		}
	}
