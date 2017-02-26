#!/usr/bin/env python

import os
import shutil

import maya.cmds 


DEBUG = 0
EMPTY_PATH = "emptyPath"

class Texture():
    ''' '''
    def __init__(self):
        ''' '''
        self.fileNode = ""
        self.type = "file"
        self.name = ""
        self.path = ""
        self.newPath = ""

        self.selected = 0
        self.found = 0
        self.moved = 0


def op_getFiles(selected=0):
    ''' ouput files path to process in array, display result in layout ''' 
    if (DEBUG):
        print ("\n op_getFiles():")       
    allFileNodes = [] 
    fileNodes = []

    #all
    if not selected:
        allFileNodes = maya.cmds.ls(type="file")
    allFileNodes.extend(maya.cmds.ls(type="psdFileTex"))
    allFileNodes.extend(maya.cmds.ls(type="mentalrayTexture"))
    IBLs = []
    if maya.cmds.pluginInfo("Mayatomr", q=1, loaded=1):
        IBLs  = maya.cmds.ls(type="mentalrayIblShape")
        allFileNodes.extend(IBLs)   
    else:
        fileNodes = maya.cmds.ls(sl = 1)
        for fileNode in fileNodes:
            if (maya.cmds.nodeType(fileNodes) in ["file", "mentalrayIblShape", \
                        "mib_texture_lookup", "mentalrayTexture","psdFileTex"]):
                allFileNodes.append(fileNode) 
                  
    maya.cmds.select(cl=1)
    filePathGroups = op_groupByPath(allFileNodes) 

    return filePathGroups

# operation types:
#0: move
#1: copy
#2: set path
#3: set path and copy
#4: set path and move      
def op_processFiles(textureDict=None, desPathName="", mvCopy=1,\
                setPath=1, selectedPaths=None):
    """copy, move or set paths of files"""
    success = []
    fail = []
    textureDict = textureDict or {}
    selectedPaths = selectedPaths or []
    if textureDict == {} or not len(desPathName) > 2:
        print "Failed, path does not exists or no texture"
        return success, fail

    if not os.path.isdir(desPathName):
        os.makedirs(desPathName)

    for k,v in textureDict.items():
        for texObj in v:
            result = 0
            if DEBUG >10:
                print texObj , "________desPathName:", mvCopy, setPath
            src = os.path.join(texObj.path, texObj.name)
            dst = desPathName
            if texObj.found and mvCopy != 0:#and (operationType ! = 2 ):
                if mvCopy == 1:# mv   
                    try:
                        shutil.copy(src, dst)
                        result = 1
                    except:
                        result = 0
                elif mvCopy == 2: #copy 
                    try:
                        result = shutil.move(src, dst)
                        v.moved = 1
                        result = 1
                    except:
                        result = 0

            if setPath:
                result = 0
                destPath = os.path.join(desPathName, texObj.name)
                if texObj.type == "mib_texture_lookup":
                    maya.cmds.setAttr(texObj.fileNode + ".fileTextureName", \
                                        destPath, type = "string")
                    result = 1
                elif texObj.type == "mentalrayIblShape":
                    maya.cmds.setAttr(texObj.fileNode + ".tx",\
                                        destPath, type = "string")
                    result = 1                           
                else:
                    maya.cmds.setAttr(texObj.fileNode + ".fileTextureName", \
                                        destPath, type = "string")
                    result = 1

             #log
            if result:
                success.append(texObj) 
            else:
                fail.append(texObj) 
   
    return success, fail

def alreadyInList(name, texObjList=None):
    ''' '''
    texObjList = texObjList or []
    for texObj in texObjList:
            if name == texObj.name:
                return 1
    return 0

# group textures by paths
def op_groupByPath(fileNodes=None):
    ''' '''
    fileNodes = fileNodes or []
    pathGroups = {}
    if DEBUG:
       print "fileNode:", fileNodes
    textureDict = {}
    textureDict[EMPTY_PATH] = []

    for fileNode in fileNodes:
        t = Texture()
        t.fileNode = fileNode
        filePath = ""
        # query
        if maya.cmds.nodeType(fileNode) == "mentalrayIblShape":
            t.type = "mentalrayIblShape"
            filePath = maya.cmds.getAttr(fileNode + ".texture")

        elif maya.cmds.nodeType(fileNode) == "mib_texture_lookup":
            texNode = maya.cmds.connectionInfo(fileNode +".tex", sourceFromDestination = 1)
        buffer = texNode.split(".")
        if maya.cmds.nodeType(buffer[0]) != "mentalrayTexture":
            filePath = maya.cmds.getAttr(buffer[0] + ".fileTextureName")
        else:
            filePath = maya.cmds.getAttr(fileNode + ".fileTextureName")


    if filePath and len(filePath) > 1:
        if DEBUG:
            print "filePath:",  filePath
        filePath = os.path.normpath(filePath)
        t.path = os.path.dirname(filePath)

        # relative paths?
        if os.path.isfile(filePath):
            t.found = 1
        t.name = os.path.basename(filePath)
        if not textureDict.has_key(t.path):
            textureDict[t.path] = []
        if not alreadyInList(t.name, texObjList = textureDict[t.path]): # no duplicates
               textureDict[t.path].append(t)
        else:
            if not alreadyInList(t.name, texObjList=textureDict[EMPTY_PATH]): # no duplicates
                textureDict[EMPTY_PATH].append(t)

    if DEBUG:
        print "textureDict:", textureDict
    # group 0 none, 1 empty paths
    return textureDict

