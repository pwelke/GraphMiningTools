#!/usr/bin/python

'''Usage: name INFILE OUTFILE PERCENTILE
Where 
	INFILE is a file in libSVM format
	OUTFILE will be a file in libSVM format containing the chosen percentile of top features
	PERCENTILE is an integer specifying the amount of features you want to keep.'''

import numpy as np
import pylab as pl
import svmlight_loader as io
import sys

from sklearn import datasets, svm
from sklearn.feature_selection import SelectPercentile, chi2

#################################################################
X, y = io.load_svmlight_file(sys.argv[1])

#################################################################
# Univariate feature selection with F-test for feature scoring
# We use the default selection function: the 10% most significant features
selector = SelectPercentile(chi2, percentile=int(sys.argv[3]))
selector.fit(X, y)

#################################################################
# store output in file
Xsmall = selector.transform(X)
io.dump_svmlight_file(Xsmall, y, sys.argv[2], False)
