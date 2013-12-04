#!/usr/bin/env python

import os

def getMap(mapFile):
	''' Given a filename of a file containing exactly one number per line, 
	return a map that has these integers as keys and 1 as value for each.'''
	map = dict()
	f = open(mapFile, 'r')
	for line in f:
		map[int(line)] = 1
	f.close()
	return map

def getSet(mapFile):	
	''' Given a filename of a file containing exactly one number per line, 
	return a set that contains these integers.'''
	map = set()
	f = open(mapFile, 'r')
	for line in f:
		map.add(int(line))
	f.close()
	return map

def filterBySet(file, set, outfile):
	''' Given a file that contains a pair of numbers per line and a set of integers,
	write a file that contains only those lines, where the second number is contained
	in the set.'''
	f = open(file, 'r')
	g = open(outfile, 'w')
	for line in f:
		pair = [int(x) for x in line.split()]
		if pair[1] in set:
			g.write(line)
	f.close()
	g.close()

def filterByMap(file, map, outfile='results/counts.filtered'):
	''' Given a file that contains a pair of numbers per line and a map,
	write a file that contains only those lines, where the second number is contained
	in the key set of the map.'''
	f = open(file, 'r')
	g = open(outfile, 'w')
	for line in f:
		pair = [int(x) for x in line.split()]
		if pair[1] in map:
			g.write(line)
	f.close()
	g.close()

def createSVMFile(filteredfile, outfile):
	''' Given filteredfile containing a pair of numbers x y per line, create a new file containing 
	a sorted list of y's in each line given by x'''
	map = dict()
	
	f = open(filteredfile, 'r')
	for line in f:
		pair = [int(x) for x in line.split()]
		if pair[0] not in map:
			map[pair[0]] = list()
		map[pair[0]].append(pair[1])
	f.close()

	g = open(outfile, 'w')
	keys = map.keys()
	keys.sort()
	for key in keys:
		line = map[key]
		line.sort()
		g.write(str(key) + ' ')
		g.write(' '.join([str(x) + ':1' for x in line]))
		g.write('\n')
	g.close()

def createLabeledSVMFile(filteredfile, outfile, labels):
	''' Given filteredfile containing a pair of numbers x y per line, create a new file containing 
	a sorted list of y's in each line given by x. First column contains a label given by label[x]'''
	map = dict()
	
	f = open(filteredfile, 'r')
	for line in f:
		pair = [int(x) for x in line.split()]
		if pair[0] not in map:
			map[pair[0]] = list()
		map[pair[0]].append(pair[1])
	f.close()

	g = open(outfile, 'w')
	keys = map.keys()
	keys.sort()
	for key in keys:
		line = map[key]
		line.sort()
		if labels[key] == 0:
			continue
		g.write(str(labels[key]) + ' ')
		g.write(' '.join([str(x) + ':1' for x in line]))
		g.write('\n')
	g.close()

def getLabels(infile):
	''' Reads a file in the format of AIDS99.txt and returns a map
	graph number -> activity'''
	labels = dict()
	f = open(infile, 'r')
	for line in f:
		if line.startswith('$'):
			break
		if line.startswith('#'):
			data = line.split()
			labels[int(data[1])] = int(data[2])
	f.close()
	return labels

def changeLabels(labels, switch):
	''' In labels, 

	0 = inactive
	1 = moderately active
	2 = active

	want to change that to 

	1 = positive class
	-1 = negative class
	0 = filter out, when selecting.

	thus, switch may be one of the following:

	AvsI = 0->-1 2->1 1->0
	AvsMI = 0->-1 1->-1 2->1
	AMvsI = 0->-1 1->1 2->1'''
	map = dict()
	if switch == 'AvsI':
		map[0] = -1
		map[1] = 0
		map[2] = 1

	if switch == 'AvsMI':
		map[0] = -1
		map[1] = -1
		map[2] = 1

	if switch == 'AMvsI':
		map[0] = -1
		map[1] = 1
		map[2] = 1

	for key in labels.keys():
		labels[key] = map[labels[key]]


def run(prefixOfOutputFiles, inputFile, labelFlag):
	
	labels = getLabels(inputFile)
	changeLabels(labels, labelFlag)

	interestingFeatures = getMap(prefixOfOutputFiles + '.features')
	filterByMap(prefixOfOutputFiles + '.counts', interestingFeatures, prefixOfOutputFiles + '.countsFrequent')

	createLabeledSVMFile(prefixOfOutputFiles + '.countsFrequent', prefixOfOutputFiles + '.svmFile' + labelFlag, labels)

