mkern -s data/01 > 01.krn
mkern -s data/02 > 02.krn
mkern -s data/03 > 03.krn
mkern -s data/04 > 04.krn
mkern -s data/05 > 05.krn
timebase -t 16 01.krn > 01.tb
timebase -t 16 02.krn > 02.tb
timebase -t 16 03.krn > 03.tb
timebase -t 16 04.krn > 04.tb
timebase -t 16 05.krn > 05.tb
assemble 05.tb 04.tb 03.tb 02.tb 01.tb | rid -d > wtc1f04.krn
