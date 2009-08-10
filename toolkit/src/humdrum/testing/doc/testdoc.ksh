# Test first example of humdrum(4)
echo "Testing first example of humdrum(4)"
humdrum -v doc1 > output
diff output result1
