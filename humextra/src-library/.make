#!/bin/bash

SRCBASE=../

rm *

# classes borrowed from sigInfo:
for i in $SRCBASE/sigInfo/* 
do
   ln -s $i
done


# classes borrowed from sigBase:
ln -s $SRCBASE/sigBase/PosixRegularExpression.cpp
ln -s $SRCBASE/sigBase/PerlRegularExpression.cpp
ln -s $SRCBASE/sigBase/FileIO.cpp
#ln -s $SRCBASE/sigBase/RationalNumber.cpp
ln -s $SRCBASE/sigBase/Options.cpp
ln -s $SRCBASE/sigBase/Options_private.cpp
ln -s $SRCBASE/sigBase/PixelColor.cpp
ln -s $SRCBASE/sigBase/EnvelopeString.cpp
ln -s $SRCBASE/sigBase/PlotData.cpp
ln -s $SRCBASE/sigBase/PlotFigure.cpp
ln -s $SRCBASE/sigBase/Array-typed.cpp
ln -s $SRCBASE/sigBase/SigString.cpp
ln -s $SRCBASE/sigBase/CheckSum.cpp


# classes borrowed from sigControl:
ln -s $SRCBASE/sigControl/MidiMessage.cpp
#ln -s $SRCBASE/sigControl/MidiOutput.cpp
#ln -s $SRCBASE/sigControl/MidiOutPort_linux.cpp
#ln -s $SRCBASE/sigControl/MidiOutPort_visual.cpp
#ln -s $SRCBASE/sigControl/MidiOutPort_unsupported.cpp
#ln -s $SRCBASE/sigControl/MidiInput.cpp
#ln -s $SRCBASE/sigControl/MidiInPort_linux.cpp
#ln -s $SRCBASE/sigControl/MidiInPort_visual.cpp
#ln -s $SRCBASE/sigControl/MidiInPort_unsupported.cpp
#ln -s $SRCBASE/sigControl/Sequencer_oss.cpp
#ln -s $SRCBASE/sigControl/Sequencer_alsa.cpp
#ln -s $SRCBASE/sigControl/Sequencer_alsa05.cpp
#ln -s $SRCBASE/sigControl/Sequencer_alsa09.cpp
#ln -s $SRCBASE/sigControl/SigTimer.cpp


# display any bad links:
file * | grep "broken symbolic link to"


