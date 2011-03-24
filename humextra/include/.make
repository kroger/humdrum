#!/bin/bash

SRCBASE=../

rm *

# classes borrowed from sigInfo:
for i in $SRCBASE/sigInfo/* 
do
   ln -s $i
done


# classes borrowed from sigBase:
ln -s $SRCBASE/sigBase/Array.cpp
ln -s $SRCBASE/sigBase/RationalNumber.h
ln -s $SRCBASE/sigBase/Array.h
ln -s $SRCBASE/sigBase/SigString.h
ln -s $SRCBASE/sigBase/PosixRegularExpression.h
ln -s $SRCBASE/sigBase/PerlRegularExpression.h
ln -s $SRCBASE/sigBase/SigCollection.cpp
ln -s $SRCBASE/sigBase/SigCollection.h
ln -s $SRCBASE/sigBase/CircularBuffer.cpp
ln -s $SRCBASE/sigBase/CircularBuffer.h
ln -s $SRCBASE/sigBase/EnvelopeString.h
ln -s $SRCBASE/sigBase/FileIO.h
ln -s $SRCBASE/sigBase/Options.h
ln -s $SRCBASE/sigBase/Options_private.h
ln -s $SRCBASE/sigBase/PixelColor.h
ln -s $SRCBASE/sigBase/PlotData.h
ln -s $SRCBASE/sigBase/PlotFigure.h
ln -s $SRCBASE/sigBase/CheckSum.h


# classes borrowed from sigControl:
ln -s $SRCBASE/sigControl/gminstruments.h
ln -s $SRCBASE/sigControl/MidiMessage.h
#ln -s $SRCBASE/sigControl/MidiOutput.h
#ln -s $SRCBASE/sigControl/MidiOutPort.h
#ln -s $SRCBASE/sigControl/MidiOutPort_linux.h
#ln -s $SRCBASE/sigControl/MidiOutPort_oss.h
#ln -s $SRCBASE/sigControl/MidiOutPort_alsa.h
#ln -s $SRCBASE/sigControl/MidiOutPort_visual.h
#ln -s $SRCBASE/sigControl/MidiOutPort_unsupported.h
#ln -s $SRCBASE/sigControl/MidiInput.h
#ln -s $SRCBASE/sigControl/MidiInPort.h
#ln -s $SRCBASE/sigControl/MidiInPort_linux.h
#ln -s $SRCBASE/sigControl/MidiInPort_oss.h
#ln -s $SRCBASE/sigControl/MidiInPort_alsa.h
#ln -s $SRCBASE/sigControl/MidiInPort_visual.h
#ln -s $SRCBASE/sigControl/MidiInPort_unsupported.h
#ln -s $SRCBASE/sigControl/SigTimer.h
#ln -s $SRCBASE/sigControl/LineDisplay.h
#ln -s $SRCBASE/sigControl/Sequencer_oss.h
#ln -s $SRCBASE/sigControl/Sequencer_alsa.h

# misc files from sig:
ln -s $SRCBASE/sig/sigTypes.h
ln -s $SRCBASE/sig/sigConfiguration.h
ln -s $SRCBASE/sig/Matrix.cpp
ln -s $SRCBASE/sig/Matrix.h

# display any bad links:
file * | grep "broken symbolic link to"


