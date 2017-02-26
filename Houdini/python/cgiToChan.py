#!/usr/bin/env python
# .cgi to chan (http://www.supertechno.com/product/technodolly.html)
# Usage: cgiToChan.py inputFile (.cgi) [outPutFile (.chan)]
# docs at www.lo-motion.de/Code/cgiToChan_doc.pdf
#
#yzx rt
#startLetter, frameNumber,
#X, Y, Z,
#pan, tilt, roll,
#zoom, focus, iris
#R1.00,4.2376,-1.0234,1.1942,-65.029,3.959,0.000,0.000,0.001,0.000

#Technodolly-x corresponds to Maya-x, 
#Technodolly-y corresponds to negative Maya-z,
#Technodolly-z corresponds to Maya-y. Consequently, pan is
#a rotation around Maya-y, tilt is a rotation around Maya-z and roll is a
#rotation around Maya-x. The order of rotations is yzx.

import os
import math

from optparse import OptionParser

DEBUG = 0
parser = OptionParser()

def getStartframe(frame):
    return frame[1:]

def changeRotate(frame):
    ''' modified from maya plugin '''
    camXPos = frame[1]
    camYPos = frame[3]
    camZPos =- float(frame[2])
   
    camZRot = frame[6]
    camYRot =- (float(frame[4]) + 90.0) 
    camXRot = frame[5]
    
    res = [str(camXPos), str(camYPos), str(camZPos), str(camXRot), 
        str(camYRot), str(camZRot),\
        str(frame[7]), str(frame[8]), str(frame[9])]
    
    return res



def loadFile(sf=0, ef=0, inputF="", outputF=""):
    ''' '''
    myFile = inputF 
    sentences = []
    i = 0
    outFile = outputF 
    if outputF == "":
         fPath = os.path.split(inputF)
         fName = ("%s.chan") % fPath[1].split(".")[0]
         outFile = os.path.join(fPath[0], fName)

    for line in open(myFile, 'r'):
        res = line.split(",")
        if DEBUG and i < 1:
            print "ch1:", res, len(res)
        stFrame = getStartframe(res[0])
        res = changeRotate(res)
        if DEBUG and i < 1:
            print "ch2:", res, len(res)
        channels = ("%s %s")% (stFrame, (" ").join(res))

        i += 1
        sentences.append(channels)

    file_out = open(outFile, 'w+') 
    if len(sentences) > 0:
        for sentence in sentences:
            file_out.write(sentence)
    file_out.close()
    
    return outFile


if __name__ == '__main__':
    (options, args) = parser.parse_args()
    outFile = ""
    if DEBUG:
        print len(args)
        
    if len(args) < 1:
        print "Usage: cgiToChan.py inputFile (.cgi) [outPutFile (.chan)].\n\
            Must at least provide an input file name."
    elif len(args) == 1:
        outFile = loadFile(inputF=args[0])
    else:
        outFile = loadFile(inputF= rgs[0], outputF=args[1])
        
    if not len(args) < 1:
        print "Done converting to:", outFile
