from collections import Counter

def makeKeyValuePairsSingleton(aidsfile, outfile):

	dataset = open(aidsfile, 'r')
	output = open(outfile, 'w')

	graphID = '0'
	for line in dataset:
		if line[0] == '#':
			graphID = line.split()[1]
			line = dataset.next()
			features = set()
			for value in line.split():
				features.add(value)
			for value in features:
				output.write(graphID + ' ' + value + '\n')
	dataset.close()
	output.close()

def makeKeyValuePairsMulti(aidsfile, outfile):

	dataset = open(aidsfile, 'r')
	output = open(outfile, 'w')

	graphID = '0'
	for line in dataset:
		if line[0] == '#':
			graphID = line.split()[1]
			line = dataset.next()
			for value in line.split():
				output.write(graphID + ' ' + value + '\n')
	dataset.close()
	output.close()


def createSVMFile(filteredfile, outfile):
	''' Given filteredfile containing a pair of numbers x y per line, create a new file containing 
	a sorted list of y's in each line given by x'''
	map = dict()
	
	f = open(filteredfile, 'r')
	for line in f:
		pair = line.split()
		if pair[0] not in map:
			map[pair[0]] = Counter()
		map[pair[0]][pair[1]] += 1
	f.close()

	g = open(outfile, 'w')
	graphIDs = map.keys()
	graphIDs.sort()
	for id in graphIDs:
		counts = map[id].keys()
		counts.sort()
		g.write(id + ' ')
		g.write(' '.join([x + ':' + str(map[id][x]) for x in counts]))
		g.write('\n')
	g.close()


# def makeWeightedFile(aidsfile):
# 	dataset = open(aidsfile, 'r')

# 	graphID = 0
# 	for line in dataset:
# 		if line[0] == #:
# 			graphID = line.split()[1]
# 			line = dataset.readline()
# 			features = Counter()
# 			for value in line.split():
# 				features[value] += 1
