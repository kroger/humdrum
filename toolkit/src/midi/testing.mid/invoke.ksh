# Tests of command-line invocation of "midi"
#
echo "Using normal command-line syntax ... (1)"
midi test07 | perform
echo "Using file redirection (<) ... (2)"
midi < test07 | perform
echo "Using midi as a filter ...(3)"
cat test07 | midi | perform
#
echo "With two input files ... (1)"
midi test07 test07 | perform
echo "With two input files ... (2)"
midi < test07 test07 | perform
echo "With two input files ...(3)"
cat test07 test07 | midi | perform
#
echo "With the -c option (1) ..."
midi -c test07 | perform
echo "With the -c option (2) ..."
midi -c < test07 | perform
echo "With the -c option (3) ..."
cat test07 | midi -c | perform
#
echo "With the -C option (1) ..."
midi -C test07 | perform
echo "With the -C option (2) ..."
midi -C < test07 | perform
echo "With the -C option (3) ..."
cat test07 | midi -C | perform
#
echo "With the -d option (1) ..."
midi -d test07 | perform
echo "With the -d option (2) ..."
midi -d < test07 | perform
echo "With the -d option (3) ..."
cat test07 | midi -d | perform
#
echo "With the -q 80 option (1) ..."
midi -q 80 test07 | perform
echo "With the -q 80 option (2) ..."
midi -q 80 < test07 | perform
echo "With the -q 80 option (3) ..."
cat test07 | midi -q 80 | perform
#
echo "With the -r 4. option (1) ..."
midi -r 4. test07 | perform
echo "With the -r 4. option (2) ..."
midi -r 4. < test07 | perform
echo "With the -r 4. option (3) ..."
cat test07 | midi -r 4. | perform
#
echo "With the -C and -c options (1) ..."
midi -C -c test07 | perform
echo "With the -C and -c options (2) ..."
midi -C -c < test07 | perform
echo "With the -C and -c options (3) ..."
cat test07 | midi -c -C | perform
#
echo "With the -C and -d options (1) ..."
midi -C -d test07 | perform
echo "With the -C and -d options (2) ..."
midi -C -d < test07 | perform
echo "With the -C and -d options (3) ..."
cat test07 | midi -d -C | perform
#
echo "With the -c and -d options (1) ..."
midi -c -d test07 | perform
echo "With the -c and -d options (2) ..."
midi -c -d < test07 | perform
echo "With the -c and -d options (3) ..."
cat test07 | midi -d -c | perform
#
echo "With the -c and -q 80 options (1) ..."
midi -c -q 80 test07 | perform
echo "With the -c and -q 80 options (2) ..."
midi -c -q 80 < test07 | perform
echo "With the -c and -q 80 options (3) ..."
cat test07 | midi -q 80 -c | perform
#
echo "With the -c and -r 4. options (1) ..."
midi -c -r 4. test07 | perform
echo "With the -c and -r 4. options (2) ..."
midi -c -r 4. < test07 | perform
echo "With the -c and -r 4. options (3) ..."
cat test07 | midi -r 4. -c | perform
#
echo "With the -q 80 and -r 4. options (1) ..."
midi -q 80 -r 4. test07 | perform
echo "With the -q 80 and -r 4. options (2) ..."
midi -q 80 -r 4. < test07 | perform
echo "With the -q 80 and -r 4. options (3) ..."
cat test07 | midi -r 4. -q 80 | perform
#
echo "With the -c, -q 80 and -r 4. options (1) ..."
midi -c -q 80 -r 4. test07 | perform
echo "With the -c, -q 80 and -r 4. options (2) ..."
midi -q 80 -c -r 4. < test07 | perform
echo "With the -c, -q 80 and -r 4. options (3) ..."
cat test07 | midi -r 4. -q 80 -c | perform
