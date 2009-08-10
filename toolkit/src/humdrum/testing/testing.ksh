#
# Perform these commands:
#

# Tests error1
echo "Checking result01"
humdrum test01 > output
diff output result01

# Tests error2
echo "Checking result02"
humdrum test02 > output
diff output result02

# Tests error3
echo "Checking result03"
humdrum test03 > output
diff output result03

# Tests error4
echo "Checking result04"
humdrum test04 > output
diff output result04

# Tests error5
echo "Checking result05"
humdrum test05 > output
diff output result05

# Tests error6
echo "Checking result06"
humdrum test06 > output
diff output result06

# Tests error7
echo "Checking result07"
humdrum test07 > output
diff output result07

# Tests error8
echo "Checking result08"
humdrum test08 > output
diff output result08

# Tests error9
echo "Checking result09"
humdrum test09 > output
diff output result09

# Tests error10
echo "Checking result10"
humdrum test10 > output
diff output result10

# Tests error11
echo "Checking result11"
humdrum test11 > output
diff output result11

# Tests error12
echo "Checking result12"
humdrum test12 > output
diff output result12

# Tests error13
echo "Checking result13"
humdrum test13 > output
diff output result13

# Tests error14
echo "Checking result14"
humdrum test14 > output
diff output result14

# Tests error15
echo "Checking result15"
humdrum test15 > output
diff output result15

# Tests error16
echo "Checking result16"
humdrum test16 > output
diff output result16

# Tests error17
echo "Checking result17"
humdrum test17 > output
diff output result17

# Tests error18
echo "Checking result18"
humdrum test18 > output
diff output result18

# Tests error19
echo "Checking result19"
humdrum test19 > output
diff output result19

# Tests error20
echo "Checking result20"
humdrum test20 > output
diff output result20

# Tests error21
echo "Checking result21"
humdrum test21 > output
diff output result21

# Tests error22
echo "Checking result22"
humdrum test22 > output
diff output result22

# Tests warning1
echo "Checking result23"
humdrum test23 > output
diff output result23

# Tests warning2
echo "Checking result24"
humdrum test24 > output
diff output result24

# Tests warning3
echo "Checking result25"
humdrum test25 > output
diff output result25

# Tests warning4
echo "Checking result26"
humdrum test26 > output
diff output result26

# Tests warning5
echo "Checking result27"
humdrum test27 > output
diff output result27

# Test humdrum on an empty file
echo "Checking result30"
humdrum test30 > output
diff output result30

# Test the -v option
echo "Checking result31"
humdrum -v test31 > output
diff output result31

# Test --
echo "Checking result32"
humdrum -- -test32 > output
diff output result32

# test33 has only global comments
echo "Checking result33"
humdrum test33 > output
diff output result33

# A test of multiple files on command line
echo "Checking result35"
humdrum test34 test35 > output
diff output result35

# Test error23
echo "Checking result36"
humdrum test36 > output
diff output result36

# Test error24
echo "Checking result37"
humdrum test37 > output
diff output result37

# Test error25
echo "Checking result38"
humdrum test38 > output
diff output result38

# Test error26
echo "Checking result39"
humdrum test39 > output
diff output result39
