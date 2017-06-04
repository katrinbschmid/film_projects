# maya software texture node  requires mayapy
# Usage: createNode cellNoiseTexture
# simple cell noise generator
# Has random, grid and heaxgon mode.
# python and swig-.dll, should run on any Maya version supporting python.
#
# To install put all python files in your plugin directory.
# Load cellNoise.py, create texture in Hypershade.
#
#  * Has random, grid and heaxgon mode (0, 1, 2)
#  * If you have cells with jagged edges increase texture size attribute
#  * Min and max value to set color range

import sys, math

import maya.OpenMaya as OpenMaya
import maya.OpenMayaMPx as OpenMayaMPx

import Voronoi #links to plattform dependent Voronoi.pyc library

kPluginNodeName = "cellNoiseTexture"
kPluginNodeId = OpenMaya.MTypeId(0x8188d)


class cellNoiseTexture(OpenMayaMPx.MPxNode):
    ''' '''
    mDiagram = Voronoi.Voronoi()
    mResultColor = Voronoi.Color()
        
    ## Input attributes
    aUVCoord = OpenMaya.MObject()
    a_w = OpenMaya.MObject()
    a_Nmode = OpenMaya.MObject() 
    a_numc = OpenMaya.MObject()
    aFilterSize = OpenMaya.MObject()
    a_maxV = OpenMaya.MObject()
    a_minV = OpenMaya.MObject()
    ## Output attributes
    aOutColor = OpenMaya.MObject()

    def __init__(self):
        OpenMayaMPx.MPxNode.__init__(self)
        
    def compute(self, plug, data):
        ''' outColor or individual R, G, B channel '''
        if plug != self.aOutColor and self.plug.parent() != aOutColor:
            return OpenMaya.kUnknownParameter

        self.mResultColor.zero()
        self.mResultColor.r = 1
                
        dSize = data.inputValue(self.a_w).asInt()
        cellSize = data.inputValue(self.a_numc).asInt()
        cMode = data.inputValue(self.a_Nmode).asInt()
        dMin = data.inputValue(self.a_maxV).asFloat()
        dMax = data.inputValue(self.a_minV).asFloat()
                
        uv = []
        uv = data.inputValue(self.aUVCoord).asFloat2(); 
        ##   normalize the UV coords
        if len(uv) > 1:
            u = uv[0]
            v = uv[1]
            u -= math.floor(uv[0])
            v -= math.floor(uv[1])
            x = math.floor(u*dSize)
            y = math.floor(v*dSize)
            cellNoiseTexture.mDiagram.doVoronoiXY(self.mResultColor, cMode, cellSize, dSize, dSize, x, y)
        else:
            print "cellNoiseTexture Error: can not read uvs"
                        
        outColorHandle = OpenMaya.MDataHandle()
        outColorHandle = data.outputValue(self.aOutColor)
        resultColor = OpenMaya.MFloatVector(self.remap(dMin, dMax,cellNoiseTexture.mResultColor.r),
                         self.remap(dMin, dMax,cellNoiseTexture.mResultColor.g),
                         self.remap(dMin, dMax,cellNoiseTexture.mResultColor.b))
        outColorHandle = data.outputValue(cellNoiseTexture.aOutColor)
        outColorHandle.setMFloatVector(resultColor)
        outColorHandle.setClean()
            
    def remap(self, minV, maxV, value):
        return (minV +(maxV- minV)*value);

 
def nodeInitializer():
    ''' '''
    nAttr = OpenMaya.MFnNumericAttribute()
    ## Implicit shading network attributes
    child1 = OpenMaya.MObject()
    child1 = nAttr.create("uCoord", "u", OpenMaya.MFnNumericData.kFloat)
    child2 = OpenMaya.MObject()
    child2 = nAttr.create("vCoord", "v", OpenMaya.MFnNumericData.kFloat)
    cellNoiseTexture.aUVCoord = nAttr.create("uvCoord", "uv", child1, child2)
    nAttr.setKeyable(True)
    nAttr.setStorable(True)
    nAttr.setReadable(True)
    nAttr.setWritable(True)
    nAttr.setHidden(True)
    cellNoiseTexture.addAttribute(cellNoiseTexture.aUVCoord)
        
    child1 = nAttr.create("uvFilterSizeX", "fsx", OpenMaya.MFnNumericData.kFloat)
    child2 = nAttr.create("uvFilterSizeY", "fsy", OpenMaya.MFnNumericData.kFloat)
    cellNoiseTexture.aFilterSize = nAttr.create("uvFilterSize", "fs", child1, child2)
    nAttr.setKeyable(True)
    nAttr.setStorable(True)
    nAttr.setReadable(True)
    nAttr.setWritable(True)
    nAttr.setHidden(True)
    cellNoiseTexture.addAttribute(cellNoiseTexture.aFilterSize)

    cellNoiseTexture.a_numc = nAttr.create("cellCount", "cc", OpenMaya.MFnNumericData.kInt, 64)
    nAttr.setKeyable(True)
    nAttr.setStorable(True)
    nAttr.setReadable(True)
    nAttr.setWritable(True)
    cellNoiseTexture.addAttribute(cellNoiseTexture.a_numc)
        
    cellNoiseTexture.a_w = nAttr.create("textureSize", "tsz", OpenMaya.MFnNumericData.kInt, 512);    
    nAttr.setKeyable(True)
    nAttr.setStorable(True)
    nAttr.setReadable(True)
    nAttr.setWritable(True)
    cellNoiseTexture.addAttribute(cellNoiseTexture.a_w)
        
    cellNoiseTexture.a_Nmode = nAttr.create("mode", "cm", OpenMaya.MFnNumericData.kInt, 0);
    nAttr.setKeyable(True)
    nAttr.setStorable(True)
    nAttr.setReadable(True)
    nAttr.setWritable(True)
    cellNoiseTexture.addAttribute(cellNoiseTexture.a_Nmode)

    cellNoiseTexture.a_minV = nAttr.create("minValue", "minv", OpenMaya.MFnNumericData.kFloat, 0.);
    nAttr.setKeyable(True)
    nAttr.setStorable(True)
    nAttr.setReadable(True)
    nAttr.setWritable(True)
    cellNoiseTexture.addAttribute(cellNoiseTexture.a_minV)

    cellNoiseTexture.a_maxV = nAttr.create("maxValue", "maxv", OpenMaya.MFnNumericData.kFloat, 1.);
    nAttr.setKeyable(True)
    nAttr.setStorable(True)
    nAttr.setReadable(True)
    nAttr.setWritable(True)
    cellNoiseTexture.addAttribute(cellNoiseTexture.a_maxV)

    ## Output attributes
    cellNoiseTexture.aOutColor = nAttr.createColor("outColor", "oc")
    nAttr.setStorable(False)
    nAttr.setReadable(True)
    nAttr.setKeyable(False)
    nAttr.setWritable(False)
    cellNoiseTexture.addAttribute(cellNoiseTexture.aOutColor)

    cellNoiseTexture.attributeAffects(cellNoiseTexture.a_minV, cellNoiseTexture.aOutColor) 
    cellNoiseTexture.attributeAffects(cellNoiseTexture.a_maxV, cellNoiseTexture.aOutColor)     
    cellNoiseTexture.attributeAffects(cellNoiseTexture.a_w, cellNoiseTexture.aOutColor) 
    cellNoiseTexture.attributeAffects(cellNoiseTexture.a_Nmode, cellNoiseTexture.aOutColor) 
    cellNoiseTexture.attributeAffects(cellNoiseTexture.a_numc, cellNoiseTexture.aOutColor) 
    cellNoiseTexture.attributeAffects(cellNoiseTexture.aUVCoord,cellNoiseTexture.aOutColor) 


def nodeCreator():
    ''' '''
    return OpenMayaMPx.asMPxPtr(cellNoiseTexture())

# initialize the script plug-in
def initializePlugin(mobject):
    ''' '''
    uc = "texture/2d"
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    try:
        mplugin.registerNode(kPluginNodeName, kPluginNodeId, nodeCreator, nodeInitializer, OpenMayaMPx.MPxNode.kDependNode, uc)
    except:
        sys.stderr.write("Failed to register node: %s" % kPluginNodeName)
        raise

# uninitialize the script plug-in
def uninitializePlugin(mobject):
    ''' '''
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    try:
        mplugin.deregisterNode(kPluginNodeId)
    except:
        sys.stderr.write("Failed to deregister node: %s" % kPluginNodeName)
    raise

