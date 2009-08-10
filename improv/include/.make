#!/bin/sh

SRCBASE=../sig

rm version
rm *.h *.cpp

ln -s $SRCBASE/version
ln -s $SRCBASE/sigConfiguration.h
ln -s $SRCBASE/sigControl.h
ln -s $SRCBASE/sigCVI.h

# classes borrowed from sigControl
ln -s $SRCBASE/Event.h
ln -s $SRCBASE/OneStageEvent.h
ln -s $SRCBASE/MidiEvent.h
ln -s $SRCBASE/TwoStageEvent.h
ln -s $SRCBASE/NoteEvent.h
ln -s $SRCBASE/MultiStageEvent.h
ln -s $SRCBASE/FunctionEvent.h
ln -s $SRCBASE/EventBuffer.h
ln -s $SRCBASE/MidiFileWrite.h
ln -s $SRCBASE/MidiIO.h
ln -s $SRCBASE/MidiInPort.h
ln -s $SRCBASE/MidiInPort_unsupported.h
ln -s $SRCBASE/MidiInPort_linux.h
ln -s $SRCBASE/MidiInPort_alsa.h
ln -s $SRCBASE/MidiInPort_alsa05.h
ln -s $SRCBASE/MidiInPort_oss.h
ln -s $SRCBASE/MidiInPort_osx.h
ln -s $SRCBASE/MidiInPort_visual.h
ln -s $SRCBASE/MidiInput.h
ln -s $SRCBASE/MidiMessage.h
ln -s $SRCBASE/MidiOutPort.h
ln -s $SRCBASE/MidiOutPort_unsupported.h
ln -s $SRCBASE/MidiOutPort_linux.h
ln -s $SRCBASE/MidiOutPort_alsa.h
ln -s $SRCBASE/MidiOutPort_oss.h
ln -s $SRCBASE/MidiOutPort_osx.h
ln -s $SRCBASE/MidiOutPort_visual.h
ln -s $SRCBASE/MidiOutput.h
ln -s $SRCBASE/MidiPerform.h
ln -s $SRCBASE/MidiPort.h
ln -s $SRCBASE/Performance.h
ln -s $SRCBASE/PerformData.h
ln -s $SRCBASE/PerformDataRecord.h
ln -s $SRCBASE/RadioBaton.h
ln -s $SRCBASE/AdamsStick.h
ln -s $SRCBASE/batonprotocol.h
ln -s $SRCBASE/Synthesizer.h
ln -s $SRCBASE/SigTimer.h
ln -s $SRCBASE/Voice.h
ln -s $SRCBASE/KeyboardInput.h
ln -s $SRCBASE/KeyboardInput_visual.h
ln -s $SRCBASE/KeyboardInput_unix.h
ln -s $SRCBASE/KeyboardInput_osx.h
ln -s $SRCBASE/Idler.h
ln -s $SRCBASE/LineDisplay.h
ln -s $SRCBASE/Sequencer_oss.h
ln -s $SRCBASE/Sequencer_alsa.h
ln -s $SRCBASE/Sequencer_alsa05.h
ln -s $SRCBASE/gminstruments.h
ln -s $SRCBASE/midichannels.h
ln -s $SRCBASE/mididefines.h
ln -s $SRCBASE/notenames.h
ln -s $SRCBASE/midiiolib.h
ln -s $SRCBASE/Tablet.h
ln -s $SRCBASE/RadioBatonTablet.h
ln -s $SRCBASE/Tablet_visual.h

# classes borrowed from sigInfo
ln -s $SRCBASE/MidiFile.h

# Windows 95 specific classes:
ln -s $SRCBASE/Nidaq.h


# improv environments:
ln -s $SRCBASE/improv.h
ln -s $SRCBASE/synthImprov.h
ln -s $SRCBASE/outputImprov.h
ln -s $SRCBASE/hciImprov.h
ln -s $SRCBASE/hciImprovGUI.h
ln -s $SRCBASE/stickImprov.h
ln -s $SRCBASE/batonImprov.h
ln -s $SRCBASE/batonImprovGUI.h
ln -s $SRCBASE/tabletImprov.h
ln -s $SRCBASE/batonCompImprov.h
ln -s $SRCBASE/batonSynthImprov.h


# classes borrowed from sigBase
ln -s $SRCBASE/CircularBuffer.h
ln -s $SRCBASE/CircularBuffer.cpp
ln -s $SRCBASE/FileIO.h
ln -s $SRCBASE/Options.h
ln -s $SRCBASE/Options_private.h
ln -s $SRCBASE/Array.h
ln -s $SRCBASE/Array.cpp
ln -s $SRCBASE/SigCollection.h
ln -s $SRCBASE/SigCollection.cpp



# display the broken links:

file * | grep "broken symbolic link to"

