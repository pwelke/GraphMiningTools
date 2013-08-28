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

def putStuff(filteredfile, outfile):
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
		g.write(' '.join([str(x) + ':1' for x in line]))
		g.write('\n')
	g.close()
