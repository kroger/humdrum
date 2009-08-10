#!/bin/bash

SRCBASE=..

rm *.cpp 

# classes borrowed from sigControl:
for i in $SRCBASE/sigControl/* 
do
   ln -s $i
done


# classes borrowed from sigBase:
#ln -s $SRCBASE/sigBase/Array.cpp          # template class so put in include/
#ln -s $SRCBASE/sigBase/CircularBuffer.cpp # template class so put in include/
ln -s $SRCBASE/sigBase/FileIO.cpp
ln -s $SRCBASE/sigBase/Options.cpp
ln -s $SRCBASE/sigBase/Options_private.cpp

# classes borrowed from sigInfo:
ln -s $SRCBASE/sigInfo/MidiFile.cpp

# display any bad links:
file * | grep "broken symbolic link to"


