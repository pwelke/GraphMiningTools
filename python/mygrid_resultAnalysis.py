'''returns some information about a mygrid run.

Usage: arrayfile topk type

where 
	arrayfile is the output of mygrid that contains 
		the resulting auc values
	topk is the number of highest auc values you 
		want to see
	type is either 'lin' or 'rbf' depending on your
		input file'''


import os
import pickle

import numpy as np
from matplotlib.pyplot import *

import mygrid_rbf
import mygrid_lin

def plotGridPerformance(array):
	imshow(array)
	show()


def loadArrayFile(file):
	f = open(file, 'rb')
	array = pickle.load(f)
	f.close()
	return array

	

def findTop(array, k):
	'''return twodimensional indices and value of top k 
	elements in a two-d array'''
	nparray = np.array(array)
	nparrayflat = nparray.ravel()
	topKIndices = nparrayflat.argsort()[-k:]
	twodimind = np.unravel_index(topKIndices, nparray.shape)
	twodimind = np.array(twodimind).T
	return np.hstack([twodimind, nparrayflat[topKIndices].reshape((k,1))])

def getParametersofTopK_rbf(topK):
	c = mygrid_rbf.range_f(mygrid_rbf.c_begin, mygrid_rbf.c_end, mygrid_rbf.c_step)
	g = mygrid_rbf.range_f(mygrid_rbf.g_begin, mygrid_rbf.g_end, mygrid_rbf.g_step)
	for i in range(topK.shape[0]):
		topK[i,0] = 2 ** c[int(topK[i,0])]
		topK[i,1] = 2 ** g[int(topK[i,1])]
	return topK

def getParametersofTopK_lin(topK):
	c = mygrid_lin.range_f(mygrid_lin.c_begin, mygrid_lin.c_end, mygrid_lin.c_step)
	w = mygrid_lin.range_f(mygrid_lin.w_begin, mygrid_lin.w_end, mygrid_lin.w_step)
	for i in range(topK.shape[0]):
		topK[i,0] = 2 ** c[int(topK[i,0])]
		topK[i,1] = 2 ** w[int(topK[i,1])]
	return topK

def main(infile, k, typ):
	array = loadArrayFile(infile)
	topK = findTop(array, int(k))
	if typ == 'rbf':
		print 'Header: C\tgamma\tAUC'
		topK = getParametersofTopK_rbf(topK)
	if typ == 'lin':
		print 'Header: C\tw\tAUC'
		topK = getParametersofTopK_lin(topK)

	print topK

	plotGridPerformance(array)

if __name__ == '__main__':
	print 'in: ' + sys.argv[1]
	print 'k: ' + sys.argv[2]
	print 'type: ' + sys.argv[3]
	main(sys.argv[1], sys.argv[2], sys.argv[3])
