# Test first example from rid(4)
echo "Testing first example from rid(4)"
rid -dlu doc1 > output
diff output result1

# Test second example from rid(4)
echo "Testing second example from rid(4)"
rid -DGLiT doc1 > output
diff output result2
