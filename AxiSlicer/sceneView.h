#include "stdafx.h"
#include "mesh.h"
#include "objectScene.h"
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

class SceneView : public CView
{
public:
	float _yaw;// = 30
	float _pitch ;
#if GLGUI_ANIMATION
	fpxyz _zoom ;
#else
	float _zoom ;
#endif
	Scene* _scene ;
	float _gcode ;
	float _gcodeVBOs ;
	float _gcodeFilename ;
	float _gcodeLoadThread ;
	float _objectShader ;
	float _objectLoadShader ;
	float _focusObj ;
	printableObject* _selectedObj ;
	float _objColors ;
	float _mouseX ;
	float _mouseY ;
	float _mouseState ;
	fpxyz _viewTarget;
#if GLGUI_ANIMATION
	animation* _animView;
	animation* _animZoom;
#endif
	float _platformMesh;
	float _platformTexture;
	int   _isSimpleMode;
	float _usbPrintMonitor;
	float _printerConnectionManager;

	float _viewport;
	float _modelMatrix;
	float _projMatrix;
	vertexmat* tempMatrix;

	float scaleForm;

	float _slicer;

	fpxyz _machineSize;

	glButton* rotateToolButton;
	glButton* scaleToolButton;
	glButton* mirrorToolButton;

	SceneView();

	void loadGCodeFile(char* filename);
	void loadSceneFiles(strList* filenames);
	void loadFiles(strList* filenames);
	void showLoadModel(int button = 1);
	void showSaveModel();
	void OnPrintButton(int button);
	void _openPrintWindowForConnection(int connection);
	void showPrintWindow();
	void showSaveGCode();
	void _copyFile(char* fileA, char* fileB, int allowEject = False);
	void _doEjectSD(int drive);
	void _showSliceLog();
	void OnToolSelect(int button);
	void updateToolButtons();
	void OnViewChange();
	void OnRotateReset(int button);
	void OnLayFlat(int button);
	void OnScaleReset(int button);
	void OnScaleMax(int button);
	void OnMirror(int axis);
	void OnScaleEntry(float value, int axis);
	void OnScaleEntryMM(float value, int axis);
	void OnDeleteAll(int e);
	void OnMultiply(int e);
	void OnSplitObject(int e);
	void OnCenter(int e);
	void _splitCallback(float progress);
	void OnMergeObjects(int e);
	void sceneUpdated();
	void _onRunSlicer(int e);
	void _updateSliceProgress(float progressValue, int ready);
	void _loadGCode();
	void _gcodeLoadCallback(float progress);
	void loadScene(strList* fileList);
	void _deleteObject(int obj);
	void _selectObject(printableObject* obj, int zoom = True);
	void updateProfileToControls();
	void updateModelSettingsToControls();
	void OnKeyChar(int keyCode);
	void ShaderUpdate(float v, float f);
	void OnMouseDown(int e);
	void OnMouseUp(int e);
	void OnMouseMotion(int e);
	void OnMouseWheel(int e);
	void OnMouseLeave(int e);
	void getMouseRay(float x, float y);
	void _init3DView();
	void OnPaint(int e);

	void _renderObject(printableObject* obj, int brightness = False, int addSink = True);
	void _drawMachine();
	void _generateGCodeVBOs(int layer);
	void _generateGCodeVBOs2(int layer);
	void getObjectCenterPos();
	void getObjectBoundaryCircle();
	void getObjectSize();
	void getObjectMatrix();
};

class shaderEditor
{
public:
/*
	shaderEditor(parent, callback, v, f)
	{
		super(shaderEditor, self).__init__(parent, title="Shader editor", style=wx.DEFAULT_DIALOG_STYLE|wx.RESIZE_BORDER)
		self._callback = callback
		s = wx.BoxSizer(wx.VERTICAL)
		self.SetSizer(s)
		self._vertex = wx.TextCtrl(self, -1, v, style=wx.TE_MULTILINE)
		self._fragment = wx.TextCtrl(self, -1, f, style=wx.TE_MULTILINE)
		s.Add(self._vertex, 1, flag=wx.EXPAND)
		s.Add(self._fragment, 1, flag=wx.EXPAND)

		self._vertex.Bind(wx.EVT_TEXT, self.OnText, self._vertex)
		self._fragment.Bind(wx.EVT_TEXT, self.OnText, self._fragment)

		self.SetPosition(self.GetParent().GetPosition())
		self.SetSize((self.GetSize().GetWidth(), self.GetParent().GetSize().GetHeight()))
		self.Show()
	}
*/

	void OnText(int e) {
//		self._callback(self._vertex.GetValue(), self._fragment.GetValue())
	}
};
