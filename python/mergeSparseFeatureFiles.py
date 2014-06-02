#!/usr/bin/python

from scipy.sparse import hstack
import svmlight_loader as io
import sys

if len(sys.argv) < 4:
	print '''Merge two files in libsvm / svmlight format
into a single file.

Parameters: inFile1 inFile2 outFile

implemented June 2014 by Pascal Welke'''

fileOne = sys.argv[1]
fileTwo = sys.argv[2]
fileThree = sys.argv[3]

xOne, yOne = io.load_svmlight_file(fileOne)
xTwo, yTwo = io.load_svmlight_file(fileTwo)

X = hstack((xOne, xTwo))

io.dump_svmlight_file(X, yOne, fileThree, False)