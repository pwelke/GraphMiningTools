#!/usr/bin/python
import sys
from math import log, exp, pow, sqrt
from scipy import stats

def incrementalAvg(avg, count, new):
    return avg + ((new-avg)/(count))

def geometricMean(data):
    c=len(data)
    v=1.0
    for d in data:
        v=v*d
    print(v)
    return pow(v,1/c)

def geomSV(gm, data):
    c=len(data)
    v=0
    for d in data:
        u=log(d/gm)
        v=v+(u*u)/c
    return exp(sqrt(v))

if __name__ == "__main__":
    size=0
    for inpFile in sys.argv[1:]:
        f= open(inpFile)
        pos=[]
        neg=[]
        for l in f:
            splitted= l.split()
            if splitted[0] == "+":
                pos.append(float(splitted[3]))
            elif splitted[0] == "-":
                neg.append(float(splitted[3]))
        gmP = stats.gmean(pos)
        gmN = stats.gmean(neg)
        gm = stats.gmean(pos+neg)

        gsvP = geomSV(gmP,pos)
        gsvN = geomSV(gmN, neg)
        gsv = geomSV(gm, pos+neg)
        print("\\draw[plus] ("+str(size+.15)+", "+str(gmP*sqrt(gsvP))+") -- ("+str(size+.35)+","+str(gmP*sqrt(gsvP)) +");")
        print("\\draw[plus] ("+str(size+.15)+", "+str(gmP/sqrt(gsvP))+") -- ("+str(size+.35)+","+str(gmP/sqrt(gsvP)) +");")
        print("\\draw[plus] ("+str(size+.25)+","+str(gmP/sqrt(gsvP))+") -- ("+str(size+.25)+","+str(gmP*sqrt(gsvP)) +");")
        print("\\draw[plus, line width=2pt] ("+str(size+.15)+","+str(gmP)+") -- ("+str(size+.35)+","+str(gmP) +");")
        print("\\draw[neg] ("+str(size-.15)+", "+str(gmN*sqrt(gsvN))+") -- ("+str(size-.35)+","+str(gmN*sqrt(gsvN)) +");")
        print("\\draw[neg] ("+str(size-.15)+", "+str(gmN/sqrt(gsvN))+") -- ("+str(size-.35)+","+str(gmN/sqrt(gsvN)) +");")
        print("\\draw[neg] ("+str(size-.25)+","+str(gmN/sqrt(gsvN))+") -- ("+str(size-.25)+","+str(gmN*sqrt(gsvN)) +");")
        print("\\draw[neg, line width=2pt] ("+str(size-.15)+", "+str(gmN)+") -- ("+str(size-.35)+","+str(gmN) +");")
        print("\\draw[neut] ("+str(size+.12)+", "+str(gm*sqrt(gsv))+") -- ("+str(size-.12)+","+str(gm*sqrt(gsv)) +");")
        print("\\draw[neut] ("+str(size+.12)+", "+str(gm/sqrt(gsv))+") -- ("+str(size-.12)+","+str(gm/sqrt(gsv)) +");")
        print("\\draw[neut] ("+str(size)+","+str(gm/sqrt(gsv))+") -- ("+str(size)+","+str(gm*sqrt(gsv)) +");")
        print("\\draw[neut, line width=2pt] ("+str(size+.12)+", "+str(gm)+") -- ("+str(size-.12)+","+str(gm) +");")
        size=size+1
