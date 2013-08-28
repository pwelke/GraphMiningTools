import os

def getMap(mapFile):
	map = dict()
	f = open(mapFile, 'r')
	for line in f:
		map[int(line)] = 1
	f.close()
	return map

def getSet(mapFile):	
	map = set()
	f = open(mapFile, 'r')
	for line in f:
		map.add(int(line))
	f.close()
	return map

def filterBySet(file, set, outfile='results/counts.filtered'):
	f = open(file, 'r')
	g = open(outfile, 'w')
	for line in f:
		pair = [int(x) for x in line.split()]
		if pair[1] in set:
			g.write(line)
	f.close()
	g.close()

def filterByMap(file, map, outfile='results/counts.filtered'):
	f = open(file, 'r')
	g = open(outfile, 'w')
	for line in f:
		pair = [int(x) for x in line.split()]
		if pair[1] in map:
			g.write(line)
	f.close()
	g.close()
