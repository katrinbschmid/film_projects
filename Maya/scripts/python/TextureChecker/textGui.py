#!/usr/bin/env python

import os
import subprocess

import maya.cmds 
import maya.OpenMayaUI 
import sip
from PyQt4 import QtGui, QtCore, uic

import textureCheck

DEBUG = 0
OP_CURRENT_PATH = "op_currProjectPath"


#Get the absolute path to ui file
uiFile = os.path.join(maya.cmds.internalVar(upd = True), 'ui', 'textui.ui')
if DEBUG:
    print 'Loading ui file:', os.path.normpath(uiFile)
#Load the ui file, and create my class
form_class, base_class = uic.loadUiType(uiFile)


def getDefaultDir():
    ''' openPipeline '''
    if maya.cmds.optionVar(exists=OP_CURRENT_PATH) \
      and maya.cmds.optionVar(q=OP_CURRENT_PATH) != 0:
        return maya.cmds.optionVar(q=OP_CURRENT_PATH)
    else:
        return ""

def getMayaWindow():
    ''' Get the maya main window as a QMainWindow instance'''
    ptr = maya.OpenMayaUI.MQtUtil.mainWindow()
    return sip.wrapinstance(long(ptr), QtCore.QObject)


class TestThread(QtCore.QThread):
    def run(self):
        self.emit(QtCore.SIGNAL("sphere"))

class TestThread0(QtCore.QThread):
    def run(self):
        self.emit(QtCore.SIGNAL("btn_signal0"))

class TestThread1(TestThread):
    def run(self):
        self.emit(QtCore.SIGNAL("btn_signal1"))

class TestThread2(TestThread):
    def run(self):
        self.emit(QtCore.SIGNAL("btn_signal2"))

class TestThread3(TestThread):
    def run(self):
        self.emit(QtCore.SIGNAL("btn_signal3"))

class TestThread4(TestThread):
    def run(self):
        self.emit(QtCore.SIGNAL("btn_signal4"))


class Window(base_class, form_class):
    ''' '''        
    def __init__(self, parent = getMayaWindow()):
        self.textureDict = {}

        #init ui using the MayaWindow as parent
        super(base_class, self).__init__(parent)
        # creates all the widgets from the .ui file
        self.setupUi(self)
        self.setObjectName('myWindow')
        self.setWindowTitle("Texture checker")
        
        #Connect buttons
        self.check_btn.clicked.connect(self.listFiles)
        self.set_btn.clicked.connect(self.fileDialog)

        #call backs
        cb1 = lambda : self.processFiles(2, 1)# mvCopy, setPath
        cb2 = lambda : self.processFiles(1, 1)
        cb3 = lambda : self.processFiles(1, 1)
        cb4 = lambda : self.processFiles(1, 1)
        cb5 = lambda : self.processFiles(0, 1)

        self.t1 = TestThread()
        self.connect(self.csff_btn, QtCore.SIGNAL("clicked()"), self.t1.start)
        self.connect(self.t1, QtCore.SIGNAL("btn_signal0"), cb1)

        self.t2 = TestThread1()
        self.connect(self.msff_btn, QtCore.SIGNAL("clicked()"), self.t2.start)
        self.connect(self.t2, QtCore.SIGNAL("btn_signal1"), cb2)
       
        self.t3 = TestThread2()
        self.connect(self.mff_btn, QtCore.SIGNAL("clicked()"), self.t3.start)
        self.connect(self.t3, QtCore.SIGNAL("btn_signal2"), cb3)

        self.t4 = TestThread3()
        self.connect(self.cff_btn, QtCore.SIGNAL("clicked()"), self.t4.start)
        self.connect(self.t4, QtCore.SIGNAL("btn_signal3"), cb4)

        self.t5 = TestThread4()
        self.connect(self.spf_btn, QtCore.SIGNAL("clicked()"), self.t5.start)
        self.connect(self.t5, QtCore.SIGNAL("btn_signal4"), cb5)

        self.clear_btn.clicked.connect(self.clearCheckbox)
        self.close_btn.clicked.connect(self.closeButtonClicked)

        self.checkBox_set.clicked.connect(self.chBoxCallback)
        self.checkBox_sub.clicked.connect(self.chBoxCallbackSubDir)
        self.connect(self.actionAbout, QtCore.SIGNAL("triggered()"), self.showHelp)

        # set default dir
        defDir = getDefaultDir()
        
    if defDir and len(defDir) > 1:
        msg = self.checkBox_set.text()
        msg += " %s" % defDir
        self.checkBox_set.setText(msg)
    else:
        msg = self.checkBox_set.setCheckState(0)

    def showHelp(self):
        ''' '''
        import webbrowser
        webbrowser.open('http://lo-motion.de/scripts.html#op_checkTextures')
        
    if DEBUG:
        print "show help http://lo-motion.de/scripts.html#op_checkTextures"

    def chBoxCallbackSubDir(self):
        ''' '''
        val = self.checkBox_sub.checkState()
        if val == 0:
            self.lineEdit_sub.setEnabled(0)
        else:
            self.lineEdit_sub.setEnabled(1)    

    def chBoxCallback(self):
        ''' '''
        val = self.checkBox_set.checkState()
        if val == 0:
            self.set_btn.setEnabled(1)
            self.tpath.setEnabled(1)
        else:
            self.set_btn.setEnabled(0)
            self.tpath.setEnabled(0)

    def getChecked(self):
        ''' '''
        listWidget = self.chLayout.itemAt(0).widget() 
        items = []
        for index in xrange(listWidget.count()):
            check_box = listWidget.itemWidget(listWidget.item(index))
            state = check_box.checkState()
            if state:
                items.append(str(check_box.text()))
            if DEBUG:
                print index, "state:", state, check_box.text(), dir(check_box)
    
        selDict = {}
        for item in items:
           tmp = item.split()
           splitT = "".join(tmp[3:])
           if DEBUG:
              print "splitT",splitT
           if self.textureDict.has_key(splitT):
                selDict[splitT] = self.textureDict[splitT]
        if DEBUG:
            print selDict,"items", items                
        return selDict

    #operation types:
    #0: move
    #1: copy
    #2: set path
    #3: set path and copy
    #4: set path and move
    def processFiles(self, mvCopy, setPath):
        ''' '''
        if DEBUG:
            print "process files", mvCopy, setPath
     
        dirT = getDefaultDir()
        val = self.checkBox_set.checkState()
        if val == 0:
            dirT = str(self.tpath.text())
        selDict = self.getChecked()
        # add sub dir
        if self.checkBox_sub.checkState():
            subFolder = self.lineEdit_sub.text()
            if subFolder and len(subFolder) > 0:
               if dirT.endswith(":"): #windows
                   dirT += "/"
                   dirT = os.path.join(str(dirT), str(subFolder))
    
            result = textureCheck.op_processFiles(textureDict=selDict,\
            desPathName=dirT, mvCopy=mvCopy, setPath=setPath, selectedPaths=[])
            #update label
        msg = "Result: %s successful, %s failed" % (len(result[0]), len(result[1]))
        if DEBUG:
            print mvCopy, setPath, "msg:", msg
        self.Result_txt.setText(msg)
    
        return result

    def listFiles(self):
        ''' update field '''
        self.Result_txt.setText("Result:")
        self.clearCheckbox()
        val = self.radioButton_sel.isChecked()
        self.textureDict = textureCheck.op_getFiles(selected=val)
        checkbox = myWindow._addCheckboxes(self.textureDict)
        
        return checkbox


    def fileDialog(self):
        ''' '''
        filename = QtGui.QFileDialog.getExistingDirectory(\
            caption='Choose output directory',
            directory=self.tpath.text(),
            options=QtGui.QFileDialog.ShowDirsOnly)
        if not filename:
            return
        self.tpath.setText(str(filename))
        
        return filename


    def closeButtonClicked(self):
        ''' '''
        global myWindow
        myWindow = None
        self.close()

    def clearCheckbox(self):
        ''' '''
        listWidget = self.chLayout.itemAt(0).widget()
        if listWidget:
            listWidget.clear()
        self.textureDict = {}

    def _addCheckboxes(self, names=None):
        ''' '''
        names =names or {}
        frames = []
        checkbox = []
        listWidget = self.chLayout.itemAt(0).widget()
        self.button = {}

        i = 0
        for k, v in names.items():
            item = QtGui.QListWidgetItem(listWidget)
            ch = QtGui.QCheckBox()
            ch.setObjectName(k)
            msg = "%s textures in: %s" % (len(v), k)
            ch.setText(msg)

            #callback
            name = "ch_%s" % i
            i += 1
            self.button[name] = k # create button widget
               #cb = lambda who = name: self._chBoxlistCallback(who)
            #self.connect(ch, QtCore.SIGNAL("stateChanged()"), cb)  
            #self.connect(ch, QtCore.SIGNAL("clicked()"), cb)  
        if DEBUG > 10:
            print type(listWidget)
        listWidget.setItemWidget(item, ch)

        checkbox.append(ch)

    return checkbox
  
    def _chBoxlistCallback(self, who):
        ''' '''
        t = "self.%s" % who
        listWidget = self.chLayout.itemAt(0).widget() 
        items = []
        for index in xrange(listWidget.count()):
            it = listWidget.itemAt(index, 0)
            items.append(it) 
    
        print "chBoxlistCallback", unicode(who)#, eval(t)

def main():
    ''' '''
    global myWindow
    if myWindow:
        try:
            myWindow.closeButtonClicked()
        except Exception as _e:
            pass
    myWindow = Window()
    myWindow.show()
main()


