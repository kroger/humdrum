######################################################################
##                              DISS.AWK
# Programmed by: Keith Mashinter     Date: April, 1994
# Copyright (c) 1994, 1995  Keith Mashinter
#
# Modifications:
#   Date:      Programmer:       Description:
#
#
# This program measures the degree of sensory dissonance for successive
# acoustic moments.  It outputs a single **diss spine containing
# numerical values -- where higher values indicate greater amounts of
# sensory dissonance.  The input must consist of one or more **spect spines.
# Each data record in the **spect input represents a concurrent set of
# discrete frequencies ("spectrum").  Spectral data consist of sets of
# paired frequency/amplitude values for each pure tone component present.
# **spect data is in the form  <freq>;<dB SPL> <freq>;<dB SPL> etc.
#
# The program implements Kameoka and Kuriyagawa's method for measuring
# sensory dissonance.
#
#
# FUNCTIONS:
#
#     log10()  - calculates base-10 logarithms
#
#
# VARIABLES:
#
#   f1, f2      - frequencies of pure tone components for which dissonance
#                 is calculated   (NOTE: f1 must be < f2)
#   p1, p2      - sound pressure levels (in microbars) of pure tone components
#                 for which dissonance is calculated
#   fb          - distance (in hertz) above the lower frequency component to the
#                 freq having the greatest dissonance
#   p0          - sound pressure level for 57 dB SPL (2* 10^-1.15 microbars)
#   k0, C0      - scale conversion constants from relative dissonance scale
#                 (RD) to absolute dissonance scale (AD)
#   B           - a constant consistent with the dissonance sensation, used
#                 with the power law in psychological significance
#   NOTE: Kameoka & Kuriyagawa used k0=1.0 & C0=65 for their calculations.
#
# outline        - output line that is assembled by the program
#
# ARRAYS:
#
#   spectspine[]  - index to which input spines contain **spect data
#   subfield[]    - contains data for all multiple stops in the current token
#   component[]   - points to the two parts (freq & SPL) for a multiple stop
#   freq[]        - frequency of current spectral component
#   loud[]        - loudness level of current spectral component
#

BEGIN {
	# Define various constants.
	k0=1.0	
	C0=65
	B=0.25		# Kameoka & Kuriyagawa's "beta" constant.
	n_e=0.20
	n_h=0.15
	n_l=0.32
	p0=2*10^-1.15	# The standard dissonance values are based on two equal-
 			# -loudness tones having a combined loudness of 60 dB,
			# i.e. 57 dB SPL for each tone.
	TRUE = 1
	FALSE = 0

	FS = "\t"	# set field separators to TAB only for HUMDRUM spines
	OFS = "\t"

	num_spectspines = 0  # Number of input spines containing **spect data.
}
###############################################################################
#
# MAIN
#
{
num_components = 0
outline = ""		# Reset output line to null string.
for ( i=1; i<=NF; i++ )  # Cycle through all input fields looking for
  {                      # **spect spines.
  if ( $i == "**spect" )
  {
   num_spectspines++
   spectspine[i] = TRUE
   if ( num_spectspines == 1 )
   {
    # If this is the first **spect spine found, then it will be replaced by
    # the **diss spine, and all other **spect spine data will be combined
    # into the single **diss spine.
    diss_spine = i
    $i =  "**diss"
   }
   else  # Replace extra **spect spines with dummy tandem interpretations.
   {
    $i = "*"
   }
  }
  else if ( $i ~ "^\*[+x^v]$" )  # check for spine redirections
  {
   print "diss: This tool does not handle spine redirection."
   exit 1
  }
  else if ( spectspine[i] == TRUE && $i ~ "^[0-9.-]+;[0-9.-]+" )
  {
   # Store the data from this **spect record in the freq[] and loud[] arrays.
   count = split($i, subfield, " ")
   for ( j=1; j<=count; j++ )
   {
    if ( split(subfield[j], component, ";") == 2 )
    {
     num_components++
     # Treat negative frequencies as positive frequencies with a shift of phase.
     if (component[1] < 0) freq[num_components] = 0 - component[1]
     else freq[num_components] = component[1]
     loud[num_components] = component[2]
    }
   }  # for ( j
  }  # else if
 }  # for ( i

 if ( num_components > 0 )
 {
  # Cycle through all pairs of pure-tone components.
  DIt = 0	# Reset cumulative dissonance intensity to zero.
  for ( i=1; i<num_components; i++ )
  {
   for ( j=i+1; j<=num_components; j++ )
   {
    # Make sure f2 is greater than f1.
    if (freq[j] >= freq[i])
    {
     f1 = freq[i]
     f2 = freq[j]
     p1 = loud[i]
     p2 = loud[j]
    }
    else
    {
     f1 = freq[j]
     f2 = freq[i]
     p1 = loud[j]
     p2 = loud[i]
    }
 
    # Ignore masked components (mentioned after eqn. (15) of K&K Part II).
    if (abs(p1,p2) > 25) continue
 
    # DETERMINE THE MAXIMALLY DISSONANT FREQUENCY ABOVE F1.
    #
    # Calculate the most dissonant frequency (fb) on the basis of the
    # lowest tone.
    #
    # Critical bandwidths increase in size with increasing loudness,
    # so the following calculation depends on the loudness of f1.
    # Note that equation (6) in K&K only applies for pressure levels
    # of 17 dB SPL or greater.  In order to avoid a maximally dissonant
    # frequency less than or equal to f1, we test for the case of p1 <= 17 dB,
    # and if true, suspend the dissonance calculation for the current pair
    # of components.

    if (p1 <= 17) continue

    fb = 2.27*( ((p1-57)/40)+1 ) * f1^0.477			# Equation (6).

    # DETERMINE THE ABSOLUTE DISSONANCE OF THE F1-F2 DYAD.
    #
    # In determining the dissonance three mutually exclusive conditions
    # should be considered: (1) Dynamic Domain, (2) Static Domain,
    # (3) Supra-Octave.  The Dynamic domain occurs when the frequency
    # difference is in the first half of the "V".  The Static domain occurs
    # when the frequency different is in the second half of the "V".
    # The Supra-octave domain occurs when the frequency difference is
    # greater than an octave.
    #
    # The absolute dissonance of the F1-F2 dyad is stored in the
    # variable D2ei.
    #
    if ( f2 >= 2*f1 || (f2-f1)/f1 <= 0.01 )
    {  # Supra-octave domain -- and near unison domain.
       # (We avoid calculations near zero frequency difference in order
       #  to avoid log(0) when (f2-f1)/f1 is <=0.01)
       #
       # Assign only dissonance arising from ambient noise.
     D2ei = k0*C0						# Equation (9).
    }
    else
    {
     if ( f2-f1 <= fb )
     {  # Dynamic Domain.  Note that the case where (f2-f1)/f1 is close to 0
        # is taken care of above to avoid log(0).           Equation (7).
      D2ei = k0*( 100*( 2+log10((f2-f1)/f1) )/( 2+log10(fb/f1) ) + C0 )
     }
     else if ( f2-f1 > fb )
     {  # Static Domain.
      D2ei = k0*( 90*( log10((f2-f1)/f1) )/( log10(fb/f1) ) + 10 + C0 )
     }
    }
 
    # Compute the Absolute Dissonance Intensity (DI) for the F1-F2 dyad.

    DI2ei = (D2ei/k0)^(1/B)					# Equation (10).

    # Compute dissonance intensity of noise DIn = (Dn0/k0)^(1/B) = C0^(1/B)

    DIn = C0^(1/B)						# Equation (11).

    # Subtract noises from dissonance.

    DI2ei = DI2ei - DIn

    # Real absolute dissonance of dyads.

    D2ei = k0*DI2ei^B						# Equation (12)
 
    # Account for SPL levels.

    # First change the sound pressure levels from dB SPL to microbars, as
    # noted in K&K Part I (after equation (8)).
    p1 = 10^(p1/20)/5000
    p2 = 10^(p2/20)/5000

    if ( p1 == p2 )
    {
     D2i = D2ei * (p1/p0)^n_e					# Equation (13)
    }
    else if ( p1 > p2 )
    {
     D2i = ( D2ei*(p1/p0)^n_e ) * (p2/p1)^n_h			# Equation (14)
    }
    else  # p1 < p2
    {
     D2i = ( D2ei*(p2/p0)^n_e ) * (p1/p2)^n_l			# Equation (15)
    }
 
    # Dissonance Intensity.
    DI2i = (D2i/k0)^(1/B)					# Equation (16).
 
    # Total dissonance intensity is the sum of all DI2i for all combinations
    # of the partials plus the noise:  DIt = sum(DI2i) + DIn
    # where the sum goes from 1 to M=m*(m-1)/2, m is the number of partials
    DIt += DI2i
   }  # for ( j
  }  # for ( i
 
  # Add in the ambient noise.
  DIt += (k0*C0)^(1/B)
 
  # The total absolute dissonance of the complex tone. 
  Dm = k0*DIt^B

  # Replace the first **spect spine data with the **diss data, and put
  # HUMDRUM null tokens (periods) in the leftover **spect spines.
  for ( i=1; i<=NF; i++ )
  {
   if ( spectspine[i] )
   {
    if ( i == diss_spine )  outline = outline Dm OFS
    else  outline = outline "." OFS
   }
   else  outline = outline $i OFS
  }
  # Eliminate any trailing tabs before printing.
  sub("\t$","",outline)
  print outline
 }  # if ( num_components
 else  # No dissonance calculation; just pass the line through.
 {
  print
 }
}

function log10(value)
{
 return log(value)/log(10)
}
function abs(value1,value2)
	{
	if (value1 >= value2) return value1 - value2
	else return value2 - value1
	}
