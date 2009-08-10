mkern -s /huron/bach/bg/keybd/0862/stage2/01/01 > 01.krn
mkern -s /huron/bach/bg/keybd/0862/stage2/01/02 > 02.krn
mkern -s /huron/bach/bg/keybd/0862/stage2/01/03 > 03.krn
timebase -t 16 01.krn > 01.tb
timebase -t 16 02.krn > 02.tb
timebase -t 16 03.krn > 03.tb
assemble 03.tb 02.tb 01.tb | rid -d > wtc1p17.krn
