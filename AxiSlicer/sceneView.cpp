#include "stdafx.h"
#include "mesh.h"
#include "sceneView.h"
#include "profile.h"
#include "meshLoader.h"
#include "opengl.h"
#include "openglGui.h"

/*
from __future__ import absolute_import
__copyright__ = "Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License"

import wx
import numpy
import time
import os
import traceback
import threading
import math
import platform

import OpenGL
OpenGL.ERROR_CHECKING = False
from OpenGL.GLU import *
from OpenGL.GL import *

from Cura.gui import printWindow
from Cura.gui import printWindow2
from Cura.util import profile
from Cura.util import meshLoader
from Cura.util import objectScene
from Cura.util import resources
from Cura.util import sliceEngine
from Cura.util import machineCom
from Cura.util import removableStorage
from Cura.util import gcodeInterpreter
from Cura.util import explorer
from Cura.util.printerConnection import printerConnectionManager
from Cura.gui.util import previewTools
from Cura.gui.util import opengl
from Cura.gui.util import openglGui
from Cura.gui.tools import youmagineGui
from Cura.gui.tools import imageToMesh
*/
SceneView::SceneView()
{
	this->_yaw = 30;
	this->_pitch = 60;
	this->_zoom = 300;
	this->_scene = new Scene();
	this->_gcode = NULL;
	this->_gcodeVBOs = NULL; // [];
	this->_gcodeFilename = NULL;
	this->_gcodeLoadThread = NULL;
	this->_objectShader = NULL;
	this->_objectLoadShader = NULL;
	this->_focusObj = NULL;
	this->_selectedObj = NULL;
	this->_objColors = NULL; // [NULL,NULL,NULL,NULL];
	this->_mouseX = -1;
	this->_mouseY = -1;
	this->_mouseState = NULL;
	this->_viewTarget = fpxyz(0,0,0);
#if GLGUI_ANIMATION
	this->_animView = NULL;
	this->_animZoom = NULL;
#endif
	this->_platformMesh = NULL;// {};
	this->_platformTexture = NULL;
	this->_isSimpleMode = True;

	this->_viewport = NULL;
	this->_modelMatrix = NULL;
	this->_projMatrix = NULL;
	this->tempMatrix = NULL;

//	group = []
/*
	this->scaleForm = openglGui.glFrame(self, (2, -2))
	openglGui.glGuiLayoutGrid(this->scaleForm)
	openglGui.glLabel(this->scaleForm, _("Scale X"), (0,0))
	this->scaleXctrl = openglGui.glNumberCtrl(this->scaleForm, '1.0', (1,0), lambda value: this->OnScaleEntry(value, 0))
	openglGui.glLabel(this->scaleForm, _("Scale Y"), (0,1))
	this->scaleYctrl = openglGui.glNumberCtrl(this->scaleForm, '1.0', (1,1), lambda value: this->OnScaleEntry(value, 1))
	openglGui.glLabel(this->scaleForm, _("Scale Z"), (0,2))
	this->scaleZctrl = openglGui.glNumberCtrl(this->scaleForm, '1.0', (1,2), lambda value: this->OnScaleEntry(value, 2))
	openglGui.glLabel(this->scaleForm, _("Size X (mm)"), (0,4))
	this->scaleXmmctrl = openglGui.glNumberCtrl(this->scaleForm, '0.0', (1,4), lambda value: this->OnScaleEntryMM(value, 0))
	openglGui.glLabel(this->scaleForm, _("Size Y (mm)"), (0,5))
	this->scaleYmmctrl = openglGui.glNumberCtrl(this->scaleForm, '0.0', (1,5), lambda value: this->OnScaleEntryMM(value, 1))
	openglGui.glLabel(this->scaleForm, _("Size Z (mm)"), (0,6))
	this->scaleZmmctrl = openglGui.glNumberCtrl(this->scaleForm, '0.0', (1,6), lambda value: this->OnScaleEntryMM(value, 2))
	openglGui.glLabel(this->scaleForm, _("Uniform scale"), (0,8))
	this->scaleUniform = openglGui.glCheckbox(this->scaleForm, True, (1,8), NULL)

	this->viewSelection = openglGui.glComboButton(self, _("View mode"), [7,19,11,15,23], [_("Normal"), _("Overhang"), _("Transparent"), _("X-Ray"), _("Layers")], (-1,0), this->OnViewChange)
	this->layerSelect = openglGui.glSlider(self, 10000, 0, 1, (-1,-2), lambda : this->QueueRefresh())

	this->youMagineButton = openglGui.glButton(self, 26, _("Share on YouMagine"), (2,0), lambda button: youmagineGui.youmagineManager(this->GetTopLevelParent(), this->_scene))
	this->youMagineButton.setDisabled(True)

	this->notification = openglGui.glNotification(self, (0, 0))

	this->_slicer = sliceEngine.Slicer(this->_updateSliceProgress)
	this->_sceneUpdateTimer = wx.Timer(self)
	this->Bind(wx.EVT_TIMER, this->_onRunSlicer, this->_sceneUpdateTimer)
	this->Bind(wx.EVT_MOUSEWHEEL, this->OnMouseWheel)
	this->Bind(wx.EVT_LEAVE_WINDOW, this->OnMouseLeave)

	this->OnViewChange()
	this->OnToolSelect(0)
	this->updateToolButtons()
	this->updateProfileToControls()
*/
}

void SceneView::loadGCodeFile(char* filename)
{
	this->OnDeleteAll(NULL);
/*
	if this->_gcode != NULL){
		delete _gcoe;
		this->_gcode = NULL;
		for layerVBOlist in this->_gcodeVBOs:
			for vbo in layerVBOlist:
				this->glReleaseList.append(vbo)
		this->_gcodeVBOs = [];
	}
	this->_gcode = gcodeInterpreter.gcode();
	this->_gcodeFilename = filename;
	this->printButton.setBottomText("");
	this->viewSelection.setValue(4);
	this->printButton.setDisabled(False);
	this->youMagineButton.setDisabled(True);
	this->OnViewChange();
*/
}

void SceneView::loadSceneFiles(strList* filenames)
{
//		this->youMagineButton.setDisabled(False)
//		#if this->viewSelection.getValue() == 4:
//		#	this->viewSelection.setValue(0)
//		#	this->OnViewChange()
		this->loadScene(filenames);
}

TCHAR* getFileExt(TCHAR* filename)
{
	TCHAR* ep = filename;
	while(*ep) ep++;
	while(ep != filename) {
		ep--;
		if(*ep == '.')
			break;
	}
	if(*ep == '.') {
		return ep;
	}

	// go to the end of name
	while(*ep) ep++;
	return ep;
}

int isDirectory(TCHAR* filename)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(filename, &wfd);
	if(hFind == INVALID_HANDLE_VALUE)
		return 0;
	int ret = 0;
	if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		ret = 1;

	return ret;
}

int isFile(LPCTSTR filename)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(filename, &wfd);
	if(hFind == INVALID_HANDLE_VALUE)
		return 0;
//	int ret = 0;
//	if(wfd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL)
//		ret = 1;
	FindClose(hFind);

	return 1;
}

int tstrcmpnocase(char* s1, char* s2);

int isMeshLoaderSupportedExtensions(TCHAR* ext)
{
	if(tstrcmpnocase(ext, L".stl") == 0)
		return 1;
	return 0;
}

int isImageToMeshSupportedExtensions(TCHAR* ext)
{
	return 0;
}

/*
void SceneView::loadFiles(strList* filenames)
{
//		# only one GCODE file can be active
//	# so if single gcode file, process this
//	# otherwise ignore all gcode files

	char* gcodeFilename = NULL;
	if (filenames->size == 1) {
		char* filename = filenames->ptrs[0];
		char* ext = getFileExt(filename);
		if (strcmp(ext, ".g") == 0 || strcmp(ext, ".gcode") == 0)
			gcodeFilename = filename;
			//mainWindow.addToModelMRU(filename)
	if (gcodeFilename != NULL){
		this->loadGCodeFile(gcodeFilename);
	}else{
//		# process directories and special file types
//		# and keep scene files for later processing
		strList scene_filenames;
		//# use file list as queue
		//# pop first entry for processing and append new files at end
		while (filenames->size > 0) {
			char* filename = filenames->pop(0);
			if(isDirectory(filename)) {
			} else {
				char* ext = getFileExt(filename);
				if(strcmp(ext, ".ini") == 0) {
					profile.loadProfile(filename);
//					mainWindow.addToProfileMRU(filename)
				} else {
					if(isMeshLoaderSupportedExtensions(ext) || isImageToMeshSupportedExtensions(ext))
						scene_filenames.add(filename);
					//mainWindow.addToModelMRU(filename)
				}
			}
		}
//		mainWindow.updateProfileToAllControls();
		//# now process all the scene files
		if (scene_filenames.size > 0){
			this->loadSceneFiles(&scene_filenames);
			this->_selectObject(NULL);
			this->sceneUpdated();
			float newZoom = float_max(this->_machineSize.);
			this->_animView = openglGui.animation(self, this->_viewTarget.copy(), numpy.array([0,0,0], numpy.float32), 0.5)
			this->_animZoom = openglGui.animation(self, this->_zoom, newZoom, 0.5);
		}
	}
}

void SceneView::showLoadModel(self, button = 1)
{
		if (button == 1){
			dlg=wx.FileDialog(self, _("Open 3D model"), os.path.split(profile.getPreference('lastFile'))[0], style=wx.FD_OPEN|wx.FD_FILE_MUST_EXIST|wx.FD_MULTIPLE)
			dlg.SetWildcard(meshLoader.loadWildcardFilter() + imageToMesh.wildcardList() + "|GCode file (*.gcode)|*.g;*.gcode;*.G;*.GCODE")
			if dlg.ShowModal() != wx.ID_OK){
				dlg.Destroy()
				return;
			}
			filenames = dlg.GetPaths()
			dlg.Destroy()
			if len(filenames) < 1:
				return False
			profile.putPreference('lastFile', filenames[0])
			this->loadFiles(filenames);
		}
}

void SceneView::showSaveModel(self)
{
		if len(this->_scene.objects()) < 1)
			return;
		dlg=wx.FileDialog(self, _("Save 3D model"), os.path.split(profile.getPreference('lastFile'))[0], style=wx.FD_SAVE|wx.FD_OVERWRITE_PROMPT)
		dlg.SetWildcard(meshLoader.saveWildcardFilter())
		if dlg.ShowModal() != wx.ID_OK){
			dlg.Destroy()
			return;
		}
		filename = dlg.GetPath()
		dlg.Destroy()
		meshLoader.saveMeshes(filename, this->_scene.objects())
}

void SceneView::OnPrintButton(self, button)
{
		if button == 1){
			connectionGroup = this->_printerConnectionManager.getAvailableGroup();
			if machineCom.machineIsConnected(){
				this->showPrintWindow()
			}elif len(removableStorage.getPossibleSDcardDrives()) > 0 and (connectionGroup is NULL or connectionGroup.getPriority() < 0){
				drives = removableStorage.getPossibleSDcardDrives()
				if len(drives) > 1){
					dlg = wx.SingleChoiceDialog(self, "Select SD drive", "Multiple removable drives have been found,\nplease select your SD card drive", map(lambda n: n[0], drives))
					if dlg.ShowModal() != wx.ID_OK){
						dlg.Destroy()
						return;
					}
					drive = drives[dlg.GetSelection()]
					dlg.Destroy()
				} else {
					drive = drives[0];
				}
				filename = this->_scene._objectList[0].getName() + '.gcode'
				threading.Thread(target=this->_copyFile,args=(this->_gcodeFilename, drive[1] + filename, drive[1])).start()
			} elif connectionGroup is not NULL){
				connections = connectionGroup.getAvailableConnections()
				if len(connections) < 2){
					connection = connections[0]
				}else{
					dlg = wx.SingleChoiceDialog(self, "Select the %s connection to use" % (connectionGroup.getName()), "Multiple %s connections found" % (connectionGroup.getName()), map(lambda n: n.getName(), connections))
					if dlg.ShowModal() != wx.ID_OK){
						dlg.Destroy()
						return;
					}
					connection = connections[dlg.GetSelection()]
					dlg.Destroy();
				}
				this->_openPrintWindowForConnection(connection)
			} else {
				this->showSaveGCode();
				}
		if button == 3){
			menu = wx.Menu()
			this->Bind(wx.EVT_MENU, lambda e: this->showPrintWindow(), menu.Append(-1, _("Print with USB")))
			connections = this->_printerConnectionManager.getAvailableConnections()
			menu.connectionMap = {}
			for connection in connections){
				i = menu.Append(-1, _("Print with %s") % (connection.getName()))
				menu.connectionMap[i.GetId()] = connection
				this->Bind(wx.EVT_MENU, lambda e: this->_openPrintWindowForConnection(e.GetEventObject().connectionMap[e.GetId()]), i);
			}
			this->Bind(wx.EVT_MENU, lambda e: this->showSaveGCode(), menu.Append(-1, _("Save GCode...")))
			this->Bind(wx.EVT_MENU, lambda e: this->_showSliceLog(), menu.Append(-1, _("Slice engine log...")))
			this->PopupMenu(menu)
			menu.Destroy();
		}
}

void SceneView::_openPrintWindowForConnection(self, connection)
{
		print '_openPrintWindowForConnection', connection.getName()
		if connection.window is NULL or not connection.window:
			connection.window = printWindow2.printWindow(connection)
		connection.window.Show()
		connection.window.Raise()
		if not connection.loadFile(this->_gcodeFilename){
			if connection.isPrinting():
				this->notification.message("Cannot start print, because other print still running.")
			else:
				this->notification.message("Failed to start print...");
		}
}

void SceneView::showPrintWindow(self)
{
		if this->_gcodeFilename is NULL:
			return
		if profile.getMachineSetting('gcode_flavor') == 'UltiGCode':
			wx.MessageBox(_("USB printing on the Ultimaker2 is not supported."), _("USB Printing Error"), wx.OK | wx.ICON_WARNING)
			return
		this->_usbPrintMonitor.loadFile(this->_gcodeFilename, this->_slicer.getID())
		if this->_gcodeFilename == this->_slicer.getGCodeFilename():
			this->_slicer.submitSliceInfoOnline()
}

void SceneView::showSaveGCode(self)
{
		if len(this->_scene._objectList) < 1:
			return
		dlg=wx.FileDialog(self, _("Save toolpath"), os.path.dirname(profile.getPreference('lastFile')), style=wx.FD_SAVE)
		filename = this->_scene._objectList[0].getName() + '.gcode'
		dlg.SetFilename(filename)
		dlg.SetWildcard('Toolpath (*.gcode)|*.gcode;*.g')
		if dlg.ShowModal() != wx.ID_OK){
			dlg.Destroy()
			return;
		}
		filename = dlg.GetPath()
		dlg.Destroy()

		threading.Thread(target=this->_copyFile,args=(this->_gcodeFilename, filename)).start()
}

void SceneView::_copyFile(self, fileA, fileB, allowEject = False)
{
		try:
			size = float(os.stat(fileA).st_size)
			with open(fileA, 'rb') as fsrc:
				with open(fileB, 'wb') as fdst:
					while 1:
						buf = fsrc.read(16*1024)
						if not buf:
							break
						fdst.write(buf)
						this->printButton.setProgressBar(float(fsrc.tell()) / size)
						this->_queueRefresh()
		except:
			import sys
			print sys.exc_info()
			this->notification.message("Failed to save")
		else:
			if allowEject:
				this->notification.message("Saved as %s" % (fileB), lambda : this->_doEjectSD(allowEject), 31, 'Eject')
			elif explorer.hasExplorer():
				this->notification.message("Saved as %s" % (fileB), lambda : explorer.openExplorer(fileB), 4, 'Open folder')
			else:
				this->notification.message("Saved as %s" % (fileB))
		this->printButton.setProgressBar(NULL)
		if fileA == this->_slicer.getGCodeFilename():
			this->_slicer.submitSliceInfoOnline()
}

void SceneView::_doEjectSD(self, drive)
{
		if removableStorage.ejectDrive(drive):
			this->notification.message('You can now eject the card.')
		else:
			this->notification.message('Safe remove failed...')
}

void SceneView::_showSliceLog(self)
{
		dlg = wx.TextEntryDialog(self, _("The slicing engine reported the following"), _("Engine log..."), '\n'.join(this->_slicer.getSliceLog()), wx.TE_MULTILINE | wx.OK | wx.CENTRE)
		dlg.ShowModal();
		dlg.Destroy();
}
*/
void SceneView::OnToolSelect(int button)
{
/*
	if (this->rotateToolButton->getSelected()){
		this->tool = previewTools.toolRotate(self);
	elif this->scaleToolButton->getSelected():
		this->tool = previewTools.toolScale(self);
	elif this->mirrorToolButton->getSelected():
		this->tool = previewTools.toolNULL(self);
	else:
		this->tool = previewTools.toolNone(self);
	this->resetRotationButton->setHidden(not this->rotateToolButton.getSelected());
	this->layFlatButton->setHidden(not this->rotateToolButton.getSelected());
	this->resetScaleButton->setHidden(not this->scaleToolButton.getSelected());
	this->scaleMaxButton->setHidden(not this->scaleToolButton.getSelected());
	this->scaleForm->setHidden(not this->scaleToolButton.getSelected());
	this->mirrorXButton->setHidden(not this->mirrorToolButton.getSelected());
	this->mirrorYButton->setHidden(not this->mirrorToolButton.getSelected());
	this->mirrorZButton->setHidden(not this->mirrorToolButton.getSelected());
*/
}

void SceneView::sceneUpdated()
{
	Invalidate();
}

void SceneView::updateToolButtons()
{
	int hidden = 0;

	if (this->_selectedObj == NULL)
		hidden = True;
	else
		hidden = False;

	this->rotateToolButton->setHidden(hidden);
	this->scaleToolButton->setHidden(hidden);
	this->mirrorToolButton->setHidden(hidden);
	if (hidden){
		this->rotateToolButton->setSelected(False);
		this->scaleToolButton->setSelected(False);
		this->mirrorToolButton->setSelected(False);
		this->OnToolSelect(0);
	}
}
/*
void SceneView::OnViewChange(self)
{
		if this->viewSelection.getValue() == 4:
			this->viewMode = 'gcode'
			if this->_gcode is not NULL and this->_gcode.layerList is not NULL:
				this->layerSelect.setRange(1, len(this->_gcode.layerList) - 1)
			this->_selectObject(NULL)
		elif this->viewSelection.getValue() == 1:
			this->viewMode = 'overhang'
		elif this->viewSelection.getValue() == 2:
			this->viewMode = 'transparent'
		elif this->viewSelection.getValue() == 3:
			this->viewMode = 'xray'
		else:
			this->viewMode = 'normal'
		this->layerSelect.setHidden(this->viewMode != 'gcode')
		this->QueueRefresh()
}

void SceneView::OnRotateReset(self, button)
{
		if this->_selectedObj is NULL:
			return
		this->_selectedObj.resetRotation()
		this->_scene.pushFree(this->_selectedObj)
		this->_selectObject(this->_selectedObj)
		this->sceneUpdated()
}

void SceneView::OnLayFlat(self, button)
{
		if this->_selectedObj is NULL:
			return
		this->_selectedObj.layFlat()
		this->_scene.pushFree(this->_selectedObj)
		this->_selectObject(this->_selectedObj)
		this->sceneUpdated()
}

void SceneView::OnScaleReset(self, button)
{
		if this->_selectedObj is NULL:
			return
		this->_selectedObj.resetScale()
		this->_selectObject(this->_selectedObj)
		this->updateProfileToControls()
		this->sceneUpdated()
}

void SceneView::OnScaleMax(self, button)
{
		if this->_selectedObj is NULL:
			return
		machine = profile.getMachineSetting('machine_type')
		this->_selectedObj.setPosition(numpy.array([0.0, 0.0]))
		this->_scene.pushFree(this->_selectedObj)
		#this->sceneUpdated()
		if machine == "ultimaker2"){
			#This is bad and Jaime should feel bad!
			this->_selectedObj.setPosition(numpy.array([0.0,-10.0]))
			this->_selectedObj.scaleUpTo(this->_machineSize - numpy.array(profile.calculateObjectSizeOffsets() + [0.0], numpy.float32) * 2 - numpy.array([1,1,1], numpy.float32))
			this->_selectedObj.setPosition(numpy.array([0.0,0.0]))
			this->_scene.pushFree(this->_selectedObj)
		} else {
			this->_selectedObj.setPosition(numpy.array([0.0, 0.0]))
			this->_scene.pushFree(this->_selectedObj)
			this->_selectedObj.scaleUpTo(this->_machineSize - numpy.array(profile.calculateObjectSizeOffsets() + [0.0], numpy.float32) * 2 - numpy.array([1,1,1], numpy.float32));
		}
		this->_scene.pushFree(this->_selectedObj);
		this->_selectObject(this->_selectedObj);
		this->updateProfileToControls();
		this->sceneUpdated();
}

void SceneView::OnMirror(int axis)
{
		if this->_selectedObj is NULL:
			return
		this->_selectedObj.mirror(axis)
		this->sceneUpdated()
}

void SceneView::OnScaleEntry(self, value, axis):
		if this->_selectedObj is NULL:
			return
		try:
			value = float(value)
		except:
			return
		this->_selectedObj.setScale(value, axis, this->scaleUniform.getValue())
		this->updateProfileToControls()
		this->_scene.pushFree(this->_selectedObj)
		this->_selectObject(this->_selectedObj)
		this->sceneUpdated()

void SceneView::OnScaleEntryMM(self, value, axis):
		if this->_selectedObj is NULL:
			return
		try:
			value = float(value)
		except:
			return
		this->_selectedObj.setSize(value, axis, this->scaleUniform.getValue())
		this->updateProfileToControls()
		this->_scene.pushFree(this->_selectedObj)
		this->_selectObject(this->_selectedObj)
		this->sceneUpdated()
*/
void SceneView::OnDeleteAll(int e)
{
//	while len(this->_scene.objects()) > 0:
//		this->_deleteObject(this->_scene.objects()[0])
//	this->_animView = openglGui.animation(self, this->_viewTarget.copy(), numpy.array([0,0,0], numpy.float32), 0.5)
}
/*
void SceneView::OnMultiply(self, e):
		if this->_focusObj is NULL:
			return
		obj = this->_focusObj
		dlg = wx.NumberEntryDialog(self, _("How many copies do you want?"), _("Number of copies"), _("Multiply"), 1, 1, 100)
		if dlg.ShowModal() != wx.ID_OK:
			dlg.Destroy()
			return
		cnt = dlg.GetValue()
		dlg.Destroy()
		n = 0
		while True:
			n += 1
			newObj = obj.copy()
			this->_scene.add(newObj)
			this->_scene.centerAll()
			if not this->_scene.checkPlatform(newObj):
				break
			if n > cnt:
				break
		if n <= cnt:
			this->notification.message("Could not create more then %d items" % (n - 1))
		this->_scene.remove(newObj)
		this->_scene.centerAll()
		this->sceneUpdated()

void SceneView::OnSplitObject(self, e):
		if this->_focusObj is NULL:
			return
		this->_scene.remove(this->_focusObj)
		for obj in this->_focusObj.split(this->_splitCallback):
			if numpy.max(obj.getSize()) > 2.0:
				this->_scene.add(obj)
		this->_scene.centerAll()
		this->_selectObject(NULL)
		this->sceneUpdated()

void SceneView::OnCenter(self, e):
		if this->_focusObj is NULL:
			return
		this->_focusObj.setPosition(numpy.array([0.0, 0.0]))
		this->_scene.pushFree(this->_selectedObj)
		newViewPos = numpy.array([this->_focusObj.getPosition()[0], this->_focusObj.getPosition()[1], this->_focusObj.getSize()[2] / 2])
		this->_animView = openglGui.animation(self, this->_viewTarget.copy(), newViewPos, 0.5)
		this->sceneUpdated()

void SceneView::_splitCallback(self, progress):
		print progress

void SceneView::OnMergeObjects(self, e):
		if this->_selectedObj is NULL or this->_focusObj is NULL or this->_selectedObj == this->_focusObj:
			if len(this->_scene.objects()) == 2:
				this->_scene.merge(this->_scene.objects()[0], this->_scene.objects()[1])
				this->sceneUpdated()
			return
		this->_scene.merge(this->_selectedObj, this->_focusObj)
		this->sceneUpdated()

void SceneView::sceneUpdated(self):
		this->_sceneUpdateTimer.Start(500, True)
		this->_slicer.abortSlicer()
		this->_scene.updateSizeOffsets()
		this->QueueRefresh()

void SceneView::_onRunSlicer(self, e):
		if this->_isSimpleMode:
			this->GetTopLevelParent().simpleSettingsPanel.setupSlice()
		this->_slicer.runSlicer(this->_scene)
		if this->_isSimpleMode:
			profile.resetTempOverride()

void SceneView::_updateSliceProgress(self, progressValue, ready):
		if not ready:
			if this->printButton.getProgressBar() is not NULL and progressValue >= 0.0 and abs(this->printButton.getProgressBar() - progressValue) < 0.01:
				return
		this->printButton.setDisabled(not ready)
		if progressValue >= 0.0:
			this->printButton.setProgressBar(progressValue)
		else:
			this->printButton.setProgressBar(NULL)
		if this->_gcode is not NULL:
			this->_gcode = NULL
			for layerVBOlist in this->_gcodeVBOs:
				for vbo in layerVBOlist:
					this->glReleaseList.append(vbo)
			this->_gcodeVBOs = []
		if ready:
			this->printButton.setProgressBar(NULL)
			text = '%s' % (this->_slicer.getPrintTime())
			for e in xrange(0, int(profile.getMachineSetting('extruder_amount'))):
				amount = this->_slicer.getFilamentAmount(e)
				if amount is NULL:
					continue
				text += '\n%s' % (amount)
				cost = this->_slicer.getFilamentCost(e)
				if cost is not NULL:
					text += '\n%s' % (cost)
			this->printButton.setBottomText(text)
			this->_gcode = gcodeInterpreter.gcode()
			this->_gcodeFilename = this->_slicer.getGCodeFilename()
		else:
			this->printButton.setBottomText('')
		this->QueueRefresh()

void SceneView::_loadGCode():
		this->_gcode.progressCallback = this->_gcodeLoadCallback
		this->_gcode.load(this->_gcodeFilename)

void SceneView::_gcodeLoadCallback(self, progress):
		if not self or this->_gcode is NULL:
			return True
		if len(this->_gcode.layerList) % 15 == 0:
			time.sleep(0.1)
		if this->_gcode is NULL:
			return True
		this->layerSelect.setRange(1, len(this->_gcode.layerList) - 1)
		if this->viewMode == 'gcode':
			this->_queueRefresh()
		return False
*/
void SceneView::loadScene(strList* fileList)
{
	printableObjectList* objList = NULL;
	for (int fn = 0; fn < fileList->size; fn++) {
		TCHAR* filename = fileList->ptrs[fn];
		TCHAR* ext = getFileExt(filename);
		if(isImageToMeshSupportedExtensions( ext )) {
			//imageToMesh.convertImageDialog(self, filename).Show()
			//	objList = []
		} else {
			objList = loadMeshes(filename);
		}

		if(objList != NULL) {
			for (int on = 0; on < objList->size; on++) {
				printableObject* obj = objList->list[on];
	//			if this->_objectLoadShader is not NULL:
	//				obj._loadAnim = openglGui.animation(self, 1, 0, 1.5)
	//			else:
	//				obj._loadAnim = NULL;
				this->_scene->add(obj);
				if (!this->_scene->checkPlatform(obj)) {
					this->_scene->centerAll();
				}
				this->_selectObject(obj);
				if (obj->getScale().data[0] < 1.0) {
				//	this->notification.message("Warning: Object scaled down.");
				}
			}
		}
	}
	this->sceneUpdated();
}
/*
	def _deleteObject(self, obj):
		if obj == this->_selectedObj:
			this->_selectObject(NULL)
		if obj == this->_focusObj:
			this->_focusObj = NULL
		this->_scene.remove(obj)
		for m in obj._meshList:
			if m.vbo is not NULL and m.vbo.decRef():
				this->glReleaseList.append(m.vbo)
		import gc
		gc.collect()
		this->sceneUpdated()
*/
void SceneView::_selectObject(printableObject* obj, int zoom )
{
	if (obj != this->_selectedObj){
		this->_selectedObj = obj;
		this->updateModelSettingsToControls();
		this->updateToolButtons();
	}
	if (zoom && obj != NULL){
		fpxyz newViewPos = fpxyz(obj->getPosition().x, obj->getPosition().y, obj->getSize()[2] / 2);
#if GLGUI_ANIMATION
		this->_animView = new animation(this, this->_viewTarget, newViewPos, 0.5);
		float newZoom = obj->getBoundaryCircle() * 6;
		if (newZoom > float_max(this->_machineSize.data, 0, 3) * 3)
			newZoom = float_max(this->_machineSize.data, 0, 3) * 3;
		this->_animZoom = new animation(this, this->_zoom, newZoom, 0.5);
#endif
	}
}
/*
	def updateProfileToControls(self):
		oldSimpleMode = this->_isSimpleMode
		this->_isSimpleMode = profile.getPreference('startMode') == 'Simple'
		if this->_isSimpleMode != oldSimpleMode:
			this->_scene.arrangeAll()
			this->sceneUpdated()
		this->_scene.updateSizeOffsets(True)
		this->_machineSize = numpy.array([profile.getMachineSettingFloat('machine_width'), profile.getMachineSettingFloat('machine_depth'), profile.getMachineSettingFloat('machine_height')])
		this->_objColors[0] = profile.getPreferenceColour('model_colour')
		this->_objColors[1] = profile.getPreferenceColour('model_colour2')
		this->_objColors[2] = profile.getPreferenceColour('model_colour3')
		this->_objColors[3] = profile.getPreferenceColour('model_colour4')
		this->_scene.updateMachineDimensions()
		this->updateModelSettingsToControls()
*/
void SceneView::updateModelSettingsToControls()
{
	if (this->_selectedObj != NULL){
		fpxyz scale = this->_selectedObj->getScale();
		fpxyz size = this->_selectedObj->getSize();
/*
		this->scaleXctrl.setValue(round(scale[0], 2));
		this->scaleYctrl.setValue(round(scale[1], 2));
		this->scaleZctrl.setValue(round(scale[2], 2));;
		this->scaleXmmctrl.setValue(round(size[0], 2))
		this->scaleYmmctrl.setValue(round(size[1], 2));
		this->scaleZmmctrl.setValue(round(size[2], 2));
*/
	}
}

/*
	def OnKeyChar(self, keyCode):
		if keyCode == wx.WXK_DELETE or keyCode == wx.WXK_NUMPAD_DELETE or (keyCode == wx.WXK_BACK and platform.system() == "Darwin"):
			if this->_selectedObj is not NULL:
				this->_deleteObject(this->_selectedObj)
				this->QueueRefresh()
		if this->viewMode == 'gcode' and (wx.GetKeyState(wx.WXK_SHIFT) or wx.GetKeyState(wx.WXK_CONTROL)):
			if keyCode == wx.WXK_UP:
				this->layerSelect.setValue(this->layerSelect.getValue() + 1)
				this->QueueRefresh()
			elif keyCode == wx.WXK_DOWN:
				this->layerSelect.setValue(this->layerSelect.getValue() - 1)
				this->QueueRefresh()
			elif keyCode == wx.WXK_PAGEUP:
				this->layerSelect.setValue(this->layerSelect.getValue() + 10)
				this->QueueRefresh()
			elif keyCode == wx.WXK_PAGEDOWN:
				this->layerSelect.setValue(this->layerSelect.getValue() - 10)
				this->QueueRefresh()
		else:
			if keyCode == wx.WXK_UP:
				if wx.GetKeyState(wx.WXK_SHIFT):
					this->_zoom /= 1.2
					if this->_zoom < 1:
						this->_zoom = 1
				else:
					this->_pitch -= 15
				this->QueueRefresh()
			elif keyCode == wx.WXK_DOWN:
				if wx.GetKeyState(wx.WXK_SHIFT):
					this->_zoom *= 1.2
					if this->_zoom > numpy.max(this->_machineSize) * 3:
						this->_zoom = numpy.max(this->_machineSize) * 3
				else:
					this->_pitch += 15
				this->QueueRefresh()
			elif keyCode == wx.WXK_LEFT:
				this->_yaw -= 15
				this->QueueRefresh()
			elif keyCode == wx.WXK_RIGHT:
				this->_yaw += 15
				this->QueueRefresh()
			elif keyCode == wx.WXK_NUMPAD_ADD or keyCode == wx.WXK_ADD or keyCode == ord('+') or keyCode == ord('='):
				this->_zoom /= 1.2
				if this->_zoom < 1:
					this->_zoom = 1
				this->QueueRefresh()
			elif keyCode == wx.WXK_NUMPAD_SUBTRACT or keyCode == wx.WXK_SUBTRACT or keyCode == ord('-'):
				this->_zoom *= 1.2
				if this->_zoom > numpy.max(this->_machineSize) * 3:
					this->_zoom = numpy.max(this->_machineSize) * 3
				this->QueueRefresh()
			elif keyCode == wx.WXK_HOME:
				this->_yaw = 30
				this->_pitch = 60
				this->QueueRefresh()
			elif keyCode == wx.WXK_PAGEUP:
				this->_yaw = 0
				this->_pitch = 0
				this->QueueRefresh()
			elif keyCode == wx.WXK_PAGEDOWN:
				this->_yaw = 0
				this->_pitch = 90
				this->QueueRefresh()
			elif keyCode == wx.WXK_END:
				this->_yaw = 90
				this->_pitch = 90
				this->QueueRefresh()

		if keyCode == wx.WXK_F3 and wx.GetKeyState(wx.WXK_SHIFT):
			shaderEditor(self, this->ShaderUpdate, this->_objectLoadShader.getVertexShader(), this->_objectLoadShader.getFragmentShader())
		if keyCode == wx.WXK_F4 and wx.GetKeyState(wx.WXK_SHIFT):
			from collections import defaultdict
			from gc import get_objects
			this->_beforeLeakTest = defaultdict(int)
			for i in get_objects():
				this->_beforeLeakTest[type(i)] += 1
		if keyCode == wx.WXK_F5 and wx.GetKeyState(wx.WXK_SHIFT):
			from collections import defaultdict
			from gc import get_objects
			this->_afterLeakTest = defaultdict(int)
			for i in get_objects():
				this->_afterLeakTest[type(i)] += 1
			for k in this->_afterLeakTest:
				if this->_afterLeakTest[k]-this->_beforeLeakTest[k]:
					print k, this->_afterLeakTest[k], this->_beforeLeakTest[k], this->_afterLeakTest[k] - this->_beforeLeakTest[k]

	def ShaderUpdate(self, v, f):
		s = opengl.GLShader(v, f)
		if s.isValid():
			this->_objectLoadShader.release()
			this->_objectLoadShader = s
			for obj in this->_scene.objects():
				obj._loadAnim = openglGui.animation(self, 1, 0, 1.5)
			this->QueueRefresh()

	def OnMouseDown(self,e):
		this->_mouseX = e.GetX()
		this->_mouseY = e.GetY()
		this->_mouseClick3DPos = this->_mouse3Dpos
		this->_mouseClickFocus = this->_focusObj
		if e.ButtonDClick():
			this->_mouseState = 'doubleClick'
		else:
			this->_mouseState = 'dragOrClick'
		p0, p1 = this->getMouseRay(this->_mouseX, this->_mouseY)
		p0 -= this->getObjectCenterPos() - this->_viewTarget
		p1 -= this->getObjectCenterPos() - this->_viewTarget
		if this->tool.OnDragStart(p0, p1):
			this->_mouseState = 'tool'
		if this->_mouseState == 'dragOrClick':
			if e.GetButton() == 1:
				if this->_focusObj is not NULL:
					this->_selectObject(this->_focusObj, False)
					this->QueueRefresh()

	def OnMouseUp(self, e):
		if e.LeftIsDown() or e.MiddleIsDown() or e.RightIsDown():
			return
		if this->_mouseState == 'dragOrClick':
			if e.GetButton() == 1:
				this->_selectObject(this->_focusObj)
			if e.GetButton() == 3:
					menu = wx.Menu()
					if this->_focusObj is not NULL:
						this->Bind(wx.EVT_MENU, lambda e: this->_deleteObject(this->_focusObj), menu.Append(-1, _("Delete object")))
						this->Bind(wx.EVT_MENU, this->OnCenter, menu.Append(-1, _("Center on platform")))
						this->Bind(wx.EVT_MENU, this->OnMultiply, menu.Append(-1, _("Multiply object")))
						this->Bind(wx.EVT_MENU, this->OnSplitObject, menu.Append(-1, _("Split object into parts")))
					if ((this->_selectedObj != this->_focusObj and this->_focusObj is not NULL and this->_selectedObj is not NULL) or len(this->_scene.objects()) == 2) and int(profile.getMachineSetting('extruder_amount')) > 1:
						this->Bind(wx.EVT_MENU, this->OnMergeObjects, menu.Append(-1, _("Dual extrusion merge")))
					if len(this->_scene.objects()) > 0:
						this->Bind(wx.EVT_MENU, this->OnDeleteAll, menu.Append(-1, _("Delete all objects")))
					if menu.MenuItemCount > 0:
						this->PopupMenu(menu)
					menu.Destroy()
		elif this->_mouseState == 'dragObject' and this->_selectedObj is not NULL:
			this->_scene.pushFree(this->_selectedObj)
			this->sceneUpdated()
		elif this->_mouseState == 'tool':
			if this->tempMatrix is not NULL and this->_selectedObj is not NULL:
				this->_selectedObj.applyMatrix(this->tempMatrix)
				this->_scene.pushFree(this->_selectedObj)
				this->_selectObject(this->_selectedObj)
			this->tempMatrix = NULL
			this->tool.OnDragEnd()
			this->sceneUpdated()
		this->_mouseState = NULL

	def OnMouseMotion(self,e):
		p0, p1 = this->getMouseRay(e.GetX(), e.GetY())
		p0 -= this->getObjectCenterPos() - this->_viewTarget
		p1 -= this->getObjectCenterPos() - this->_viewTarget

		if e.Dragging() and this->_mouseState is not NULL:
			if this->_mouseState == 'tool':
				this->tool.OnDrag(p0, p1)
			elif not e.LeftIsDown() and e.RightIsDown():
				this->_mouseState = 'drag'
				if wx.GetKeyState(wx.WXK_SHIFT):
					a = math.cos(math.radians(this->_yaw)) / 3.0
					b = math.sin(math.radians(this->_yaw)) / 3.0
					this->_viewTarget[0] += float(e.GetX() - this->_mouseX) * -a
					this->_viewTarget[1] += float(e.GetX() - this->_mouseX) * b
					this->_viewTarget[0] += float(e.GetY() - this->_mouseY) * b
					this->_viewTarget[1] += float(e.GetY() - this->_mouseY) * a
				else:
					this->_yaw += e.GetX() - this->_mouseX
					this->_pitch -= e.GetY() - this->_mouseY
				if this->_pitch > 170:
					this->_pitch = 170
				if this->_pitch < 10:
					this->_pitch = 10
			elif (e.LeftIsDown() and e.RightIsDown()) or e.MiddleIsDown():
				this->_mouseState = 'drag'
				this->_zoom += e.GetY() - this->_mouseY
				if this->_zoom < 1:
					this->_zoom = 1
				if this->_zoom > numpy.max(this->_machineSize) * 3:
					this->_zoom = numpy.max(this->_machineSize) * 3
			elif e.LeftIsDown() and this->_selectedObj is not NULL and this->_selectedObj == this->_mouseClickFocus:
				this->_mouseState = 'dragObject'
				z = max(0, this->_mouseClick3DPos[2])
				p0, p1 = this->getMouseRay(this->_mouseX, this->_mouseY)
				p2, p3 = this->getMouseRay(e.GetX(), e.GetY())
				p0[2] -= z
				p1[2] -= z
				p2[2] -= z
				p3[2] -= z
				cursorZ0 = p0 - (p1 - p0) * (p0[2] / (p1[2] - p0[2]))
				cursorZ1 = p2 - (p3 - p2) * (p2[2] / (p3[2] - p2[2]))
				diff = cursorZ1 - cursorZ0
				this->_selectedObj.setPosition(this->_selectedObj.getPosition() + diff[0:2])
		if not e.Dragging() or this->_mouseState != 'tool':
			this->tool.OnMouseMove(p0, p1)

		this->_mouseX = e.GetX()
		this->_mouseY = e.GetY()

	def OnMouseWheel(self, e):
		delta = float(e.GetWheelRotation()) / float(e.GetWheelDelta())
		delta = max(min(delta,4),-4)
		this->_zoom *= 1.0 - delta / 10.0
		if this->_zoom < 1.0:
			this->_zoom = 1.0
		if this->_zoom > numpy.max(this->_machineSize) * 3:
			this->_zoom = numpy.max(this->_machineSize) * 3
		this->Refresh()

	def OnMouseLeave(self, e):
		#this->_mouseX = -1
		pass
*/
/*
def getMouseRay(self, x, y):
		if this->_viewport is NULL:
			return numpy.array([0,0,0],numpy.float32), numpy.array([0,0,1],numpy.float32)
		p0 = opengl.unproject(x, this->_viewport[1] + this->_viewport[3] - y, 0, this->_modelMatrix, this->_projMatrix, this->_viewport)
		p1 = opengl.unproject(x, this->_viewport[1] + this->_viewport[3] - y, 1, this->_modelMatrix, this->_projMatrix, this->_viewport)
		p0 -= this->_viewTarget
		p1 -= this->_viewTarget
		return p0, p1

void SceneView::_init3DView()
{
	# set viewing projection
	size = this->GetSize();
	glViewport(0, 0, size.GetWidth(), size.GetHeight());
	glLoadIdentity();

	glLightfv(GL_LIGHT0, GL_POSITION, [0.2, 0.2, 1.0, 0.0]);

	glDisable(GL_RESCALE_NORMAL);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.8, 0.8, 0.8, 1.0);
	glClearStencil(0);
	glClearDepth(1.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	aspect = float(size.GetWidth()) / float(size.GetHeight());
	gluPerspective(45.0, aspect, 1.0, numpy.max(this->_machineSize) * 4);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void SceneView::OnPaint(int e)
{
	connectionGroup = this->_printerConnectionManager.getAvailableGroup()
	if machineCom.machineIsConnected() {
		this->printButton._imageID = 6
		this->printButton._tooltip = _("Print")
	} elif len(removableStorage.getPossibleSDcardDrives()) > 0 and (connectionGroup is NULL or connectionGroup.getPriority() < 0) {
		this->printButton._imageID = 2
		this->printButton._tooltip = _("Toolpath to SD")
	} elif connectionGroup is not NULL {
		this->printButton._imageID = connectionGroup.getIconID()
		this->printButton._tooltip = _("Print with %s") % (connectionGroup.getName())
	} else {
		this->printButton._imageID = 3
		this->printButton._tooltip = _("Save toolpath")
	}

	if this->_animView is not NULL){
		this->_viewTarget = this->_animView.getPosition()
		if this->_animView.isDone():
			this->_animView = NULL
	}
	if this->_animZoom is not NULL){
		this->_zoom = this->_animZoom.getPosition()
		if this->_animZoom.isDone():
			this->_animZoom = NULL
	}
	if this->viewMode == 'gcode' and this->_gcode is not NULL){
		try:
			this->_viewTarget[2] = this->_gcode.layerList[this->layerSelect.getValue()][-1]['points'][0][2]
		except:
			pass
	}
	if this->_objectShader is NULL:
		if opengl.hasShaderSupport():
				this->_objectShader = opengl.GLShader("""
varying float light_amount;

void main(void)
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;

	light_amount = abs(dot(normalize(gl_NormalMatrix * gl_Normal), normalize(gl_LightSource[0].position.xyz)));
	light_amount += 0.2;
}
				""","""
varying float light_amount;

void main(void)
{
	gl_FragColor = vec4(gl_Color.xyz * light_amount, gl_Color[3]);
}
				""");

				this->_objectOverhangShader = opengl.GLShader("""
uniform float cosAngle;
uniform mat3 rotMatrix;
varying float light_amount;

void main(void)
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;

	light_amount = abs(dot(normalize(gl_NormalMatrix * gl_Normal), normalize(gl_LightSource[0].position.xyz)));
	light_amount += 0.2;
	if (normalize(rotMatrix * gl_Normal).z < -cosAngle)
	{
		light_amount = -10.0;
	}
}
				""","""
varying float light_amount;

void main(void)
{
	if (light_amount == -10.0)
	{
		gl_FragColor = vec4(1.0, 0.0, 0.0, gl_Color[3]);
	}else{
		gl_FragColor = vec4(gl_Color.xyz * light_amount, gl_Color[3]);
	}
}
				""");
				this->_objectLoadShader = opengl.GLShader("""
uniform float intensity;
uniform float scale;
varying float light_amount;

void main(void)
{
	vec4 tmp = gl_Vertex;
    tmp.x += sin(tmp.z/5.0+intensity*30.0) * scale * intensity;
    tmp.y += sin(tmp.z/3.0+intensity*40.0) * scale * intensity;
    gl_Position = gl_ModelViewProjectionMatrix * tmp;
    gl_FrontColor = gl_Color;

	light_amount = abs(dot(normalize(gl_NormalMatrix * gl_Normal), normalize(gl_LightSource[0].position.xyz)));
	light_amount += 0.2;
}
			""","""
uniform float intensity;
varying float light_amount;

void main(void)
{
	gl_FragColor = vec4(gl_Color.xyz * light_amount, 1.0-intensity);
}
				""");
			if this->_objectShader is NULL or not this->_objectShader.isValid()){
				this->_objectShader = opengl.GLFakeShader()
				this->_objectOverhangShader = opengl.GLFakeShader()
				this->_objectLoadShader = NULL;
			}

		this->_init3DView();
		glTranslate(0,0,-this->_zoom);
		glRotate(-this->_pitch, 1,0,0);
		glRotate(this->_yaw, 0,0,1);
		glTranslate(-this->_viewTarget[0],-this->_viewTarget[1],-this->_viewTarget[2]);

		this->_viewport = glGetIntegerv(GL_VIEWPORT);
		this->_modelMatrix = glGetDoublev(GL_MODELVIEW_MATRIX);
		this->_projMatrix = glGetDoublev(GL_PROJECTION_MATRIX);

		glClearColor(1,1,1,1);;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		if this->viewMode != 'gcode'){
			for n in xrange(0, len(this->_scene.objects()))){
				obj = this->_scene.objects()[n]
				glColor4ub((n >> 16) & 0xFF, (n >> 8) & 0xFF, (n >> 0) & 0xFF, 0xFF)
				this->_renderObject(obj);
			}
		}

		if this->_mouseX > -1){
			glFlush()
			n = glReadPixels(this->_mouseX, this->GetSize().GetHeight() - 1 - this->_mouseY, 1, 1, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8)[0][0] >> 8
			if n < len(this->_scene.objects()):
				this->_focusObj = this->_scene.objects()[n]
			else:
				this->_focusObj = NULL;
			f = glReadPixels(this->_mouseX, this->GetSize().GetHeight() - 1 - this->_mouseY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT)[0][0]
			#this->GetTopLevelParent().SetTitle(hex(n) + " " + str(f))
			this->_mouse3Dpos = opengl.unproject(this->_mouseX, this->_viewport[1] + this->_viewport[3] - this->_mouseY, f, this->_modelMatrix, this->_projMatrix, this->_viewport)
			this->_mouse3Dpos -= this->_viewTarget;
		}

		this->_init3DView();
		glTranslate(0,0,-this->_zoom);
		glRotate(-this->_pitch, 1,0,0);
		glRotate(this->_yaw, 0,0,1);
		glTranslate(-this->_viewTarget[0],-this->_viewTarget[1],-this->_viewTarget[2]);

		if this->viewMode == 'gcode'){
			if this->_gcode is not NULL and this->_gcode.layerList is NULL){
				this->_gcodeLoadThread = threading.Thread(target=this->_loadGCode)
				this->_gcodeLoadThread.daemon = True
				this->_gcodeLoadThread.start();
			}
			if this->_gcode is not NULL and this->_gcode.layerList is not NULL){
				glPushMatrix()
				if profile.getMachineSetting('machine_center_is_zero') != 'True':
					glTranslate(-this->_machineSize[0] / 2, -this->_machineSize[1] / 2, 0)
				t = time.time()
				drawUpTill = min(len(this->_gcode.layerList), this->layerSelect.getValue() + 1)
				for n in xrange(0, drawUpTill)){
					c = 1.0 - float(drawUpTill - n) / 15
					c = max(0.3, c)
					if len(this->_gcodeVBOs) < n + 1:
						this->_gcodeVBOs.append(this->_generateGCodeVBOs(this->_gcode.layerList[n]))
						if time.time() - t > 0.5:
							this->QueueRefresh()
							break
					#['WALL-OUTER', 'WALL-INNER', 'FILL', 'SUPPORT', 'SKIRT']
					if n == drawUpTill - 1){
						if len(this->_gcodeVBOs[n]) < 9:
							this->_gcodeVBOs[n] += this->_generateGCodeVBOs2(this->_gcode.layerList[n])
						glColor3f(c, 0, 0)
						this->_gcodeVBOs[n][8].render(GL_QUADS)
						glColor3f(c/2, 0, c)
						this->_gcodeVBOs[n][9].render(GL_QUADS)
						glColor3f(0, c, c/2)
						this->_gcodeVBOs[n][10].render(GL_QUADS)
						glColor3f(c, 0, 0)
						this->_gcodeVBOs[n][11].render(GL_QUADS)

						glColor3f(0, c, 0)
						this->_gcodeVBOs[n][12].render(GL_QUADS)
						glColor3f(c/2, c/2, 0.0)
						this->_gcodeVBOs[n][13].render(GL_QUADS)
						glColor3f(0, c, c)
						this->_gcodeVBOs[n][14].render(GL_QUADS)
						this->_gcodeVBOs[n][15].render(GL_QUADS)
						glColor3f(0, 0, c)
						this->_gcodeVBOs[n][16].render(GL_LINES)
					} else {
						glColor3f(c, 0, 0)
						this->_gcodeVBOs[n][0].render(GL_LINES)
						glColor3f(c/2, 0, c)
						this->_gcodeVBOs[n][1].render(GL_LINES)
						glColor3f(0, c, c/2)
						this->_gcodeVBOs[n][2].render(GL_LINES)
						glColor3f(c, 0, 0)
						this->_gcodeVBOs[n][3].render(GL_LINES)

						glColor3f(0, c, 0)
						this->_gcodeVBOs[n][4].render(GL_LINES)
						glColor3f(c/2, c/2, 0.0)
						this->_gcodeVBOs[n][5].render(GL_LINES)
						glColor3f(0, c, c)
						this->_gcodeVBOs[n][6].render(GL_LINES)
						this->_gcodeVBOs[n][7].render(GL_LINES);
					}
				}
				glPopMatrix();
			}
		} else {
			glStencilFunc(GL_ALWAYS, 1, 1);
			glStencilOp(GL_INCR, GL_INCR, GL_INCR);

			if this->viewMode == 'overhang'){
				this->_objectOverhangShader.bind()
				this->_objectOverhangShader.setUniform('cosAngle', math.cos(math.radians(90 - 60)));
			} else {
				this->_objectShader.bind();
			}
			for obj in this->_scene.objects()){
				if obj._loadAnim is not NULL){
					if obj._loadAnim.isDone():
						obj._loadAnim = NULL
					else:
						continue;
				}
				brightness = 1.0
				if this->_focusObj == obj:
					brightness = 1.2
				elif this->_focusObj is not NULL or this->_selectedObj is not NULL and obj != this->_selectedObj:
					brightness = 0.8

				if this->_selectedObj == obj or this->_selectedObj is NULL){
					#If we want transparent, then first render a solid black model to remove the printer size lines.
					if this->viewMode == 'transparent'){
						glColor4f(0, 0, 0, 0)
						this->_renderObject(obj)
						glEnable(GL_BLEND)
						glBlendFunc(GL_ONE, GL_ONE)
						glDisable(GL_DEPTH_TEST)
						brightness *= 0.5;
					}
					if this->viewMode == 'xray':
						glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE)
					glStencilOp(GL_INCR, GL_INCR, GL_INCR)
					glEnable(GL_STENCIL_TEST);
				}
				if this->viewMode == 'overhang'){
					if this->_selectedObj == obj and this->tempMatrix is not NULL:
						this->_objectOverhangShader.setUniform('rotMatrix', obj.getMatrix() * this->tempMatrix)
					else:
						this->_objectOverhangShader.setUniform('rotMatrix', obj.getMatrix())
				}
				if not this->_scene.checkPlatform(obj)){
					glColor4f(0.5 * brightness, 0.5 * brightness, 0.5 * brightness, 0.8 * brightness)
					this->_renderObject(obj);
				} else {
					this->_renderObject(obj, brightness);
				}
				glDisable(GL_STENCIL_TEST)
				glDisable(GL_BLEND)
				glEnable(GL_DEPTH_TEST)
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			}
			if this->viewMode == 'xray'){
				glPushMatrix()
				glLoadIdentity()
				glEnable(GL_STENCIL_TEST)
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP)
				glDisable(GL_DEPTH_TEST)
				for i in xrange(2, 15, 2)){
					glStencilFunc(GL_EQUAL, i, 0xFF)
					glColor(float(i)/10, float(i)/10, float(i)/5)
					glBegin(GL_QUADS)
					glVertex3f(-1000,-1000,-10)
					glVertex3f( 1000,-1000,-10)
					glVertex3f( 1000, 1000,-10)
					glVertex3f(-1000, 1000,-10)
					glEnd();
				}
				for i in xrange(1, 15, 2)){
					glStencilFunc(GL_EQUAL, i, 0xFF)
					glColor(float(i)/10, 0, 0)
					glBegin(GL_QUADS)
					glVertex3f(-1000,-1000,-10)
					glVertex3f( 1000,-1000,-10)
					glVertex3f( 1000, 1000,-10)
					glVertex3f(-1000, 1000,-10)
					glEnd();
				}
				glPopMatrix();
				glDisable(GL_STENCIL_TEST);
				glEnable(GL_DEPTH_TEST);
			}
			this->_objectShader.unbind();

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			if this->_objectLoadShader is not NULL){
				this->_objectLoadShader.bind()
				glColor4f(0.2, 0.6, 1.0, 1.0)
				for obj in this->_scene.objects()){
					if obj._loadAnim is NULL:
						continue
					this->_objectLoadShader.setUniform('intensity', obj._loadAnim.getPosition())
					this->_objectLoadShader.setUniform('scale', obj.getBoundaryCircle() / 10)
					this->_renderObject(obj);
				}
				this->_objectLoadShader.unbind();
				glDisable(GL_BLEND);
			}

		this->_drawMachine()

		if this->_usbPrintMonitor.getState() == 'PRINTING' and this->_usbPrintMonitor.getID() == this->_slicer.getID():
			z = this->_usbPrintMonitor.getZ()
			if this->viewMode == 'gcode':
				layer_height = profile.getProfileSettingFloat('layer_height')
				layer1_height = profile.getProfileSettingFloat('bottom_thickness')
				if layer_height > 0:
					if layer1_height > 0:
						layer = int((z - layer1_height) / layer_height) + 1
					else:
						layer = int(z / layer_height)
				else:
					layer = 1
				this->layerSelect.setValue(layer)
			else:
				size = this->_machineSize
				glEnable(GL_BLEND)
				glColor4ub(255,255,0,128)
				glBegin(GL_QUADS)
				glVertex3f(-size[0]/2,-size[1]/2, z)
				glVertex3f( size[0]/2,-size[1]/2, z)
				glVertex3f( size[0]/2, size[1]/2, z)
				glVertex3f(-size[0]/2, size[1]/2, z)
				glEnd()

		if this->viewMode == 'gcode':
			if this->_gcodeLoadThread is not NULL and this->_gcodeLoadThread.isAlive():
				glDisable(GL_DEPTH_TEST)
				glPushMatrix()
				glLoadIdentity()
				glTranslate(0,-4,-10)
				glColor4ub(60,60,60,255)
				opengl.glDrawStringCenter(_("Loading toolpath for visualization..."))
				glPopMatrix()
		else:
			#Draw the object box-shadow, so you can see where it will collide with other objects.
			if this->_selectedObj is not NULL:
				glEnable(GL_BLEND)
				glEnable(GL_CULL_FACE)
				glColor4f(0,0,0,0.16)
				glDepthMask(False)
				for obj in this->_scene.objects():
					glPushMatrix()
					glTranslatef(obj.getPosition()[0], obj.getPosition()[1], 0)
					glBegin(GL_TRIANGLE_FAN)
					for p in obj._boundaryHull[::-1]:
						glVertex3f(p[0], p[1], 0)
					glEnd()
					glPopMatrix()
				if this->_scene.isOneAtATime():
					glPushMatrix()
					glColor4f(0,0,0,0.06)
					glTranslatef(this->_selectedObj.getPosition()[0], this->_selectedObj.getPosition()[1], 0)
					glBegin(GL_TRIANGLE_FAN)
					for p in this->_selectedObj._printAreaHull[::-1]:
						glVertex3f(p[0], p[1], 0)
					glEnd()
					glBegin(GL_TRIANGLE_FAN)
					for p in this->_selectedObj._headAreaMinHull[::-1]:
						glVertex3f(p[0], p[1], 0)
					glEnd()
					glPopMatrix()
				glDepthMask(True)
				glDisable(GL_CULL_FACE)

			#Draw the outline of the selected object, on top of everything else except the GUI.
			if this->_selectedObj is not NULL and this->_selectedObj._loadAnim is NULL:
				glDisable(GL_DEPTH_TEST)
				glEnable(GL_CULL_FACE)
				glEnable(GL_STENCIL_TEST)
				glDisable(GL_BLEND)
				glStencilFunc(GL_EQUAL, 0, 255)

				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)
				glLineWidth(2)
				glColor4f(1,1,1,0.5)
				this->_renderObject(this->_selectedObj)
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)

				glViewport(0, 0, this->GetSize().GetWidth(), this->GetSize().GetHeight())
				glDisable(GL_STENCIL_TEST)
				glDisable(GL_CULL_FACE)
				glEnable(GL_DEPTH_TEST)

			if this->_selectedObj is not NULL:
				glPushMatrix()
				pos = this->getObjectCenterPos()
				glTranslate(pos[0], pos[1], pos[2])
				this->tool.OnDraw()
				glPopMatrix()
		if this->viewMode == 'overhang' and not opengl.hasShaderSupport():
			glDisable(GL_DEPTH_TEST)
			glPushMatrix()
			glLoadIdentity()
			glTranslate(0,-4,-10)
			glColor4ub(60,60,60,255)
			opengl.glDrawStringCenter(_("Overhang view not working due to lack of OpenGL shaders support."))
			glPopMatrix()
*/
void SceneView::_renderObject(printableObject* obj, int brightness, int addSink)
{
	glPushMatrix();
	if (addSink)
		glTranslatef(obj->getPosition().x, obj->getPosition().y, obj->getSize()[2] / 2 - profile.getProfileSettingFloat("object_sink"));
	else
		glTranslatef(obj->getPosition().x, obj->getPosition().y, obj->getSize()[2] / 2);

	if (this->tempMatrix != NULL && obj == this->_selectedObj) {
		float matrixf[16];
		convert3x3MatrixTo4x4(this->tempMatrix, matrixf);
		glMultMatrixf(matrixf);
	}

	fpxyz offset = obj->getDrawOffset();
	glTranslatef(-offset.data[0], -offset.data[1], -offset.data[2] - obj->getSize()[2] / 2);

	float matrixf[16];
	//tempMatrix = convert3x3MatrixTo4x4(obj->getMatrix(), matrixf);
	convert3x3MatrixTo4x4(obj->getMatrix(), matrixf);
	glMultMatrixf(matrixf);

	int n = 0;
	for (int oidx = 0; oidx < obj->_meshList->len; oidx++){
		mesh* m = obj->_meshList->data[oidx];
		if (m->vbo == NULL)
			m->vbo = new GLVBO(m->vertexes, m->normal, FALSE, __FILE__, __LINE__);
		if( brightness > 0){
// map(lambda... fucking python
//			glColor4fv(map(lambda n: n * brightness, this->_objColors[n]));
// equivalent is
			float fcolors[4];
			for(int fi = 0; fi < 4; fi++) // assume _objColors has 4 entries...
				fcolors[fi] = n * brightness;
			glColor4fv(fcolors);
			n += 1;
		}
		m->vbo->render();
	}
	glPopMatrix();
}
/*
	def _drawMachine(self):
		glEnable(GL_CULL_FACE)
		glEnable(GL_BLEND)

		size = [profile.getMachineSettingFloat('machine_width'), profile.getMachineSettingFloat('machine_depth'), profile.getMachineSettingFloat('machine_height')]

		machine = profile.getMachineSetting('machine_type')
		if machine.startswith('ultimaker'):
			if machine not in this->_platformMesh:
				meshes = meshLoader.loadMeshes(resources.getPathForMesh(machine + '_platform.stl'))
				if len(meshes) > 0:
					this->_platformMesh[machine] = meshes[0]
				else:
					this->_platformMesh[machine] = NULL
				if machine == 'ultimaker2':
					this->_platformMesh[machine]._drawOffset = numpy.array([0,-37,145], numpy.float32)
				else:
					this->_platformMesh[machine]._drawOffset = numpy.array([0,0,2.5], numpy.float32)
			glColor4f(1,1,1,0.5)
			this->_objectShader.bind()
			this->_renderObject(this->_platformMesh[machine], False, False)
			this->_objectShader.unbind()

			#For the Ultimaker 2 render the texture on the back plate to show the Ultimaker2 text.
			if machine == 'ultimaker2':
				if not hasattr(this->_platformMesh[machine], 'texture'):
					this->_platformMesh[machine].texture = opengl.loadGLTexture('Ultimaker2backplate.png')
				glBindTexture(GL_TEXTURE_2D, this->_platformMesh[machine].texture)
				glEnable(GL_TEXTURE_2D)
				glPushMatrix()
				glColor4f(1,1,1,1)

				glTranslate(0,150,-5)
				h = 50
				d = 8
				w = 100
				glEnable(GL_BLEND)
				glBlendFunc(GL_DST_COLOR, GL_ZERO)
				glBegin(GL_QUADS)
				glTexCoord2f(1, 0)
				glVertex3f( w, 0, h)
				glTexCoord2f(0, 0)
				glVertex3f(-w, 0, h)
				glTexCoord2f(0, 1)
				glVertex3f(-w, 0, 0)
				glTexCoord2f(1, 1)
				glVertex3f( w, 0, 0)

				glTexCoord2f(1, 0)
				glVertex3f(-w, d, h)
				glTexCoord2f(0, 0)
				glVertex3f( w, d, h)
				glTexCoord2f(0, 1)
				glVertex3f( w, d, 0)
				glTexCoord2f(1, 1)
				glVertex3f(-w, d, 0)
				glEnd()
				glDisable(GL_TEXTURE_2D)
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
				glPopMatrix()
		else:
			glColor4f(0,0,0,1)
			glLineWidth(3)
			glBegin(GL_LINES)
			glVertex3f(-size[0] / 2, -size[1] / 2, 0)
			glVertex3f(-size[0] / 2, -size[1] / 2, 10)
			glVertex3f(-size[0] / 2, -size[1] / 2, 0)
			glVertex3f(-size[0] / 2+10, -size[1] / 2, 0)
			glVertex3f(-size[0] / 2, -size[1] / 2, 0)
			glVertex3f(-size[0] / 2, -size[1] / 2+10, 0)
			glEnd()

		glDepthMask(False)

		polys = profile.getMachineSizePolygons()
		height = profile.getMachineSettingFloat('machine_height')
		glBegin(GL_QUADS)
		for n in xrange(0, len(polys[0])):
			if n % 2 == 0:
				glColor4ub(5, 171, 231, 96)
			else:
				glColor4ub(5, 171, 231, 64)
			glVertex3f(polys[0][n][0], polys[0][n][1], height)
			glVertex3f(polys[0][n][0], polys[0][n][1], 0)
			glVertex3f(polys[0][n-1][0], polys[0][n-1][1], 0)
			glVertex3f(polys[0][n-1][0], polys[0][n-1][1], height)
		glEnd()
		glColor4ub(5, 171, 231, 128)
		glBegin(GL_TRIANGLE_FAN)
		for p in polys[0][::-1]:
			glVertex3f(p[0], p[1], height)
		glEnd()

		#Draw checkerboard
		if this->_platformTexture is NULL:
			this->_platformTexture = opengl.loadGLTexture('checkerboard.png')
			glBindTexture(GL_TEXTURE_2D, this->_platformTexture)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
		glColor4f(1,1,1,0.5)
		glBindTexture(GL_TEXTURE_2D, this->_platformTexture)
		glEnable(GL_TEXTURE_2D)
		glBegin(GL_TRIANGLE_FAN)
		for p in polys[0]:
			glTexCoord2f(p[0]/20, p[1]/20)
			glVertex3f(p[0], p[1], 0)
		glEnd()
		glDisable(GL_TEXTURE_2D)
		glColor4ub(127, 127, 127, 200)
		for poly in polys[1:]:
			glBegin(GL_TRIANGLE_FAN)
			for p in poly:
				glTexCoord2f(p[0]/20, p[1]/20)
				glVertex3f(p[0], p[1], 0)
			glEnd()

		glDepthMask(True)
		glDisable(GL_BLEND)
		glDisable(GL_CULL_FACE)

	def _generateGCodeVBOs(self, layer):
		ret = []
		for extrudeType in ['WALL-OUTER:0', 'WALL-OUTER:1', 'WALL-OUTER:2', 'WALL-OUTER:3', 'WALL-INNER', 'FILL', 'SUPPORT', 'SKIRT']:
			if ':' in extrudeType:
				extruder = int(extrudeType[extrudeType.find(':')+1:])
				extrudeType = extrudeType[0:extrudeType.find(':')]
			else:
				extruder = NULL
			pointList = numpy.zeros((0,3), numpy.float32)
			for path in layer:
				if path['type'] == 'extrude' and path['pathType'] == extrudeType and (extruder is NULL or path['extruder'] == extruder):
					a = path['points']
					a = numpy.concatenate((a[:-1], a[1:]), 1)
					a = a.reshape((len(a) * 2, 3))
					pointList = numpy.concatenate((pointList, a))
			ret.append(opengl.GLVBO(pointList))
		return ret

	def _generateGCodeVBOs2(self, layer):
		filamentRadius = profile.getProfileSettingFloat('filament_diameter') / 2
		filamentArea = math.pi * filamentRadius * filamentRadius
		useFilamentArea = profile.getMachineSetting('gcode_flavor') == 'UltiGCode'

		ret = []
		for extrudeType in ['WALL-OUTER:0', 'WALL-OUTER:1', 'WALL-OUTER:2', 'WALL-OUTER:3', 'WALL-INNER', 'FILL', 'SUPPORT', 'SKIRT']:
			if ':' in extrudeType:
				extruder = int(extrudeType[extrudeType.find(':')+1:])
				extrudeType = extrudeType[0:extrudeType.find(':')]
			else:
				extruder = NULL
			pointList = numpy.zeros((0,3), numpy.float32)
			for path in layer:
				if path['type'] == 'extrude' and path['pathType'] == extrudeType and (extruder is NULL or path['extruder'] == extruder):
					a = path['points']
					if extrudeType == 'FILL':
						a[:,2] += 0.01

					normal = a[1:] - a[:-1]
					lens = numpy.sqrt(normal[:,0]**2 + normal[:,1]**2)
					normal[:,0], normal[:,1] = -normal[:,1] / lens, normal[:,0] / lens
					normal[:,2] /= lens

					ePerDist = path['extrusion'][1:] / lens
					if useFilamentArea:
						lineWidth = ePerDist / path['layerThickness'] / 2.0
					else:
						lineWidth = ePerDist * (filamentArea / path['layerThickness'] / 2)

					normal[:,0] *= lineWidth
					normal[:,1] *= lineWidth

					b = numpy.zeros((len(a)-1, 0), numpy.float32)
					b = numpy.concatenate((b, a[1:] + normal), 1)
					b = numpy.concatenate((b, a[1:] - normal), 1)
					b = numpy.concatenate((b, a[:-1] - normal), 1)
					b = numpy.concatenate((b, a[:-1] + normal), 1)
					b = b.reshape((len(b) * 4, 3))

					if len(a) > 2:
						normal2 = normal[:-1] + normal[1:]
						lens2 = numpy.sqrt(normal2[:,0]**2 + normal2[:,1]**2)
						normal2[:,0] /= lens2
						normal2[:,1] /= lens2
						normal2[:,0] *= lineWidth[:-1]
						normal2[:,1] *= lineWidth[:-1]

						c = numpy.zeros((len(a)-2, 0), numpy.float32)
						c = numpy.concatenate((c, a[1:-1]), 1)
						c = numpy.concatenate((c, a[1:-1]+normal[1:]), 1)
						c = numpy.concatenate((c, a[1:-1]+normal2), 1)
						c = numpy.concatenate((c, a[1:-1]+normal[:-1]), 1)

						c = numpy.concatenate((c, a[1:-1]), 1)
						c = numpy.concatenate((c, a[1:-1]-normal[1:]), 1)
						c = numpy.concatenate((c, a[1:-1]-normal2), 1)
						c = numpy.concatenate((c, a[1:-1]-normal[:-1]), 1)

						c = c.reshape((len(c) * 8, 3))

						pointList = numpy.concatenate((pointList, b, c))
					else:
						pointList = numpy.concatenate((pointList, b))
			ret.append(opengl.GLVBO(pointList))

		pointList = numpy.zeros((0,3), numpy.float32)
		for path in layer:
			if path['type'] == 'move':
				a = path['points'] + numpy.array([0,0,0.01], numpy.float32)
				a = numpy.concatenate((a[:-1], a[1:]), 1)
				a = a.reshape((len(a) * 2, 3))
				pointList = numpy.concatenate((pointList, a))
			if path['type'] == 'retract':
				a = path['points'] + numpy.array([0,0,0.01], numpy.float32)
				a = numpy.concatenate((a[:-1], a[1:] + numpy.array([0,0,1], numpy.float32)), 1)
				a = a.reshape((len(a) * 2, 3))
				pointList = numpy.concatenate((pointList, a))
		ret.append(opengl.GLVBO(pointList))

		return ret
/*
	def getObjectCenterPos(self):
		if this->_selectedObj is NULL:
			return [0.0, 0.0, 0.0]
		pos = this->_selectedObj.getPosition()
		size = this->_selectedObj.getSize()
		return [pos[0], pos[1], size[2]/2 - profile.getProfileSettingFloat('object_sink')]

	def getObjectBoundaryCircle(self):
		if this->_selectedObj is NULL:
			return 0.0
		return this->_selectedObj.getBoundaryCircle()

	def getObjectSize(self):
		if this->_selectedObj is NULL:
			return [0.0, 0.0, 0.0]
		return this->_selectedObj.getSize()

	def getObjectMatrix(self):
		if this->_selectedObj is NULL:
			return numpy.matrix([[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]])
		return this->_selectedObj.getMatrix()

class shaderEditor(wx.Dialog):
	def __init__(self, parent, callback, v, f):
		super(shaderEditor, self).__init__(parent, title="Shader editor", style=wx.DEFAULT_DIALOG_STYLE|wx.RESIZE_BORDER)
		this->_callback = callback
		s = wx.BoxSizer(wx.VERTICAL)
		this->SetSizer(s)
		this->_vertex = wx.TextCtrl(self, -1, v, style=wx.TE_MULTILINE)
		this->_fragment = wx.TextCtrl(self, -1, f, style=wx.TE_MULTILINE)
		s.Add(this->_vertex, 1, flag=wx.EXPAND)
		s.Add(this->_fragment, 1, flag=wx.EXPAND)

		this->_vertex.Bind(wx.EVT_TEXT, this->OnText, this->_vertex)
		this->_fragment.Bind(wx.EVT_TEXT, this->OnText, this->_fragment)

		this->SetPosition(this->GetParent().GetPosition())
		this->SetSize((this->GetSize().GetWidth(), this->GetParent().GetSize().GetHeight()))
		this->Show()

	def OnText(self, e):
		this->_callback(this->_vertex.GetValue(), this->_fragment.GetValue())
*/
