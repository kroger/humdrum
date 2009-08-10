#
# Perform these commands:
#

# Test -D option
echo "Checking result01"
rid -D test01 > output
diff output result01

# Test -G option
echo "Checking result02"
rid -G test01 > output
diff output result02

# Test -I option
echo "Checking result03"
rid -I test01 > output
diff output result03

# Test -L option
echo "Checking result04"
rid -L test01 > output
diff output result04

# Test -d option
echo "Checking result05"
rid -d test01 > output
diff output result05

# Test -g option
echo "Checking result06"
rid -g test01 > output
diff output result06

# Test -i option
echo "Checking result07"
rid -i test01 > output
diff output result07

# Test -l option
echo "Checking result08"
rid -l test01 > output
diff output result08

# Test all options together
echo "Checking result09"
rid -dgilDGIL test01 > output
diff output result09

# Test on a totally empty file
echo "Checking result10"
rid -dgil test02 > output
diff output result10

# Test on standard input
echo "Checking result11"
cat test01 test03 | rid -GL - > output
diff output result11

# Test 2 files specified on command line
echo "Checking result12"
rid -GL test01 test03 > output
diff output result12

# A test of --
echo "Checking result13"
rid -GL -- -test13 > output
diff output result13

# test14 contains only global comments
echo "Checking result14"
rid -G test14 > output
diff output result14

# A test of -U and -u options
echo "Checking result15"
rid -U test15 > output
diff output result15

# Another test of -U and -u options
echo "Checking result16"
rid -u test16 > output
diff output result16

# A test of -T option
echo "Checking result17"
rid -T test17 > output
diff output result17

# A test of -U with -T option
echo "Checking result18"
rid -U -T test18 > output
diff output result18
