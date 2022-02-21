#ifndef _OPENGLGUI_H_
#define _OPENGLGUI_H_

#include "mesh.h"

/*
from __future__ import absolute_import
from __future__ import division
__copyright__ = "Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License"

import wx
import traceback
import sys
import os
import time

from wx import glcanvas
import OpenGL
OpenGL.ERROR_CHECKING = False
from OpenGL.GL import *

from Cura.util import version
from Cura.gui.util import opengl
*/
class animation
{
public:
	float _start;
	float _end;
	float _startTime;
	float _runTime;

	animation( CView* gui, float start, float end, float runTime);
	~animation()
	{
		_memdel(this);
	}
	int isDone();
	float getPosition();
	/*:
		if self.isDone():
			return self._end
		f = (time.time() - self._startTime) / self._runTime
		ts = f*f
		tc = f*f*f
		#f = 6*tc*ts + -15*ts*ts + 10*tc
		f = tc + -3*ts + 3*f
		return self._start + (self._end - self._start) * f
*/
};

class glGuiControl : public CButton
{
public:
	glGuiControl(CWnd* parent, fpxy& pos);
	~glGuiControl();

	CWnd* _parent;
	float _base;
	fpxy _pos;
	CRect _size;
	CWnd* _focus;


	void setSize(int x, int y, int w, int h);
	CRect getSize();
	CSize getMinSize();
	void updateLayout();
	void focusNext();
	void focusPrevious();
	int  setFocus();
	int  hasFocus();
	void OnMouseUp(int x, int y);
	void OnKeyChar(int key);
};
/*
class glGuiContainer(glGuiControl):
	def __init__(self, parent, pos):
		self._glGuiControlList = []
		glGuiLayoutButtons(self)
		super(glGuiContainer, self).__init__(parent, pos)

	def add(self, ctrl):
		self._glGuiControlList.append(ctrl)
		self.updateLayout()

	def OnMouseDown(self, x, y, button):
		for ctrl in self._glGuiControlList:
			if ctrl.OnMouseDown(x, y, button):
				return True
		return False

	def OnMouseUp(self, x, y):
		for ctrl in self._glGuiControlList:
			if ctrl.OnMouseUp(x, y):
				return True
		return False

	def OnMouseMotion(self, x, y):
		handled = False
		for ctrl in self._glGuiControlList:
			if ctrl.OnMouseMotion(x, y):
				handled = True
		return handled

	def draw(self):
		for ctrl in self._glGuiControlList:
			ctrl.draw()

	def updateLayout(self):
		self._layout.update()
		for ctrl in self._glGuiControlList:
			ctrl.updateLayout()

class glGuiPanel(glcanvas.GLCanvas):
	def __init__(self, parent):
		attribList = (glcanvas.WX_GL_RGBA, glcanvas.WX_GL_DOUBLEBUFFER, glcanvas.WX_GL_DEPTH_SIZE, 24, glcanvas.WX_GL_STENCIL_SIZE, 8, 0)
		glcanvas.GLCanvas.__init__(self, parent, style=wx.WANTS_CHARS, attribList = attribList)
		self._base = self
		self._focus = None
		self._container = None
		self._container = glGuiContainer(self, (0,0))
		self._shownError = False

		self._context = glcanvas.GLContext(self)
		self._glButtonsTexture = None
		self._glRobotTexture = None
		self._buttonSize = 64

		self._animationList = []
		self.glReleaseList = []
		self._refreshQueued = False
		self._idleCalled = False

		wx.EVT_PAINT(self, self._OnGuiPaint)
		wx.EVT_SIZE(self, self._OnSize)
		wx.EVT_ERASE_BACKGROUND(self, self._OnEraseBackground)
		wx.EVT_LEFT_DOWN(self, self._OnGuiMouseDown)
		wx.EVT_LEFT_DCLICK(self, self._OnGuiMouseDown)
		wx.EVT_LEFT_UP(self, self._OnGuiMouseUp)
		wx.EVT_RIGHT_DOWN(self, self._OnGuiMouseDown)
		wx.EVT_RIGHT_DCLICK(self, self._OnGuiMouseDown)
		wx.EVT_RIGHT_UP(self, self._OnGuiMouseUp)
		wx.EVT_MIDDLE_DOWN(self, self._OnGuiMouseDown)
		wx.EVT_MIDDLE_DCLICK(self, self._OnGuiMouseDown)
		wx.EVT_MIDDLE_UP(self, self._OnGuiMouseUp)
		wx.EVT_MOTION(self, self._OnGuiMouseMotion)
		wx.EVT_CHAR(self, self._OnGuiKeyChar)
		wx.EVT_KILL_FOCUS(self, self.OnFocusLost)
		wx.EVT_IDLE(self, self._OnIdle)

	def _OnIdle(self, e):
		self._idleCalled = True
		if len(self._animationList) > 0 or self._refreshQueued:
			self._refreshQueued = False
			for anim in self._animationList:
				if anim.isDone():
					self._animationList.remove(anim)
			self.Refresh()

	def _OnGuiKeyChar(self, e):
		if self._focus is not None:
			self._focus.OnKeyChar(e.GetKeyCode())
			self.Refresh()
		else:
			self.OnKeyChar(e.GetKeyCode())

	def OnFocusLost(self, e):
		self._focus = None
		self.Refresh()

	def _OnGuiMouseDown(self,e):
		self.SetFocus()
		if self._container.OnMouseDown(e.GetX(), e.GetY(), e.GetButton()):
			self.Refresh()
			return
		self.OnMouseDown(e)

	def _OnGuiMouseUp(self, e):
		if self._container.OnMouseUp(e.GetX(), e.GetY()):
			self.Refresh()
			return
		self.OnMouseUp(e)

	def _OnGuiMouseMotion(self,e):
		self.Refresh()
		if not self._container.OnMouseMotion(e.GetX(), e.GetY()):
			self.OnMouseMotion(e)

	def _OnGuiPaint(self, e):
		self._idleCalled = False
		h = self.GetSize().GetHeight()
		w = self.GetSize().GetWidth()
		oldButtonSize = self._buttonSize
		if h / 3 < w / 4:
			w = h * 4 / 3
		if w < 64 * 8:
			self._buttonSize = 32
		elif w < 64 * 10:
			self._buttonSize = 48
		elif w < 64 * 15:
			self._buttonSize = 64
		elif w < 64 * 20:
			self._buttonSize = 80
		else:
			self._buttonSize = 96
		if self._buttonSize != oldButtonSize:
			self._container.updateLayout()

		dc = wx.PaintDC(self)
		try:
			self.SetCurrent(self._context)
			for obj in self.glReleaseList:
				obj.release()
			del self.glReleaseList[:]
			renderStartTime = time.time()
			self.OnPaint(e)
			self._drawGui()
			glFlush()
			if version.isDevVersion():
				renderTime = time.time() - renderStartTime
				if renderTime == 0:
					renderTime = 0.001
				glLoadIdentity()
				glTranslate(10, self.GetSize().GetHeight() - 30, -1)
				glColor4f(0.2,0.2,0.2,0.5)
				opengl.glDrawStringLeft("fps:%d" % (1 / renderTime))
			self.SwapBuffers()
		except:
			errStr = _("An error has occurred during the 3D view drawing.")
			tb = traceback.extract_tb(sys.exc_info()[2])
			errStr += "\n%s: '%s'" % (str(sys.exc_info()[0].__name__), str(sys.exc_info()[1]))
			for n in xrange(len(tb)-1, -1, -1):
				locationInfo = tb[n]
				errStr += "\n @ %s:%s:%d" % (os.path.basename(locationInfo[0]), locationInfo[2], locationInfo[1])
			if not self._shownError:
				wx.CallAfter(wx.MessageBox, errStr, _("3D window error"), wx.OK | wx.ICON_EXCLAMATION)
				self._shownError = True

	def _drawGui(self):
		if self._glButtonsTexture is None:
			self._glButtonsTexture = opengl.loadGLTexture('glButtons.png')
			self._glRobotTexture = opengl.loadGLTexture('UltimakerRobot.png')

		glDisable(GL_DEPTH_TEST)
		glEnable(GL_BLEND)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
		glDisable(GL_LIGHTING)
		glColor4ub(255,255,255,255)

		glMatrixMode(GL_PROJECTION)
		glLoadIdentity()
		size = self.GetSize()
		glOrtho(0, size.GetWidth()-1, size.GetHeight()-1, 0, -1000.0, 1000.0)
		glMatrixMode(GL_MODELVIEW)
		glLoadIdentity()

		self._container.draw()

		# glBindTexture(GL_TEXTURE_2D, self._glRobotTexture)
		# glEnable(GL_TEXTURE_2D)
		# glPushMatrix()
		# glColor4f(1,1,1,1)
		# glTranslate(size.GetWidth(),size.GetHeight(),0)
		# s = self._buttonSize * 1
		# glScale(s,s,s)
		# glTranslate(-1.2,-0.2,0)
		# glBegin(GL_QUADS)
		# glTexCoord2f(1, 0)
		# glVertex2f(0,-1)
		# glTexCoord2f(0, 0)
		# glVertex2f(-1,-1)
		# glTexCoord2f(0, 1)
		# glVertex2f(-1, 0)
		# glTexCoord2f(1, 1)
		# glVertex2f(0, 0)
		# glEnd()
		# glDisable(GL_TEXTURE_2D)
		glPopMatrix()

	def _OnEraseBackground(self,event):
		#Workaround for windows background redraw flicker.
		pass

	def _OnSize(self,e):
		self._container.setSize(0, 0, self.GetSize().GetWidth(), self.GetSize().GetHeight())
		self._container.updateLayout()
		self.Refresh()

	def OnMouseDown(self,e):
		pass
	def OnMouseUp(self,e):
		pass
	def OnMouseMotion(self, e):
		pass
	def OnKeyChar(self, keyCode):
		pass
	def OnPaint(self, e):
		pass
	def OnKeyChar(self, keycode):
		pass

	def QueueRefresh(self):
		wx.CallAfter(self._queueRefresh)

	def _queueRefresh(self):
		if self._idleCalled:
			wx.CallAfter(self.Refresh)
		else:
			self._refreshQueued = True

	def add(self, ctrl):
		if self._container is not None:
			self._container.add(ctrl)

class glGuiLayoutButtons(object):
	def __init__(self, parent):
		self._parent = parent
		self._parent._layout = self

	def update(self):
		bs = self._parent._base._buttonSize
		x0, y0, w, h = self._parent.getSize()
		gridSize = bs * 1.0
		for ctrl in self._parent._glGuiControlList:
			pos = ctrl._pos
			if pos[0] < 0:
				x = w + pos[0] * gridSize - bs * 0.2
			else:
				x = pos[0] * gridSize + bs * 0.2
			if pos[1] < 0:
				y = h + pos[1] * gridSize * 1.2 - bs * 0.0
			else:
				y = pos[1] * gridSize * 1.2 + bs * 0.2
			ctrl.setSize(x, y, gridSize, gridSize)

	def getLayoutSize(self):
		_, _, w, h = self._parent.getSize()
		return w, h

class glGuiLayoutGrid(object):
	def __init__(self, parent):
		self._parent = parent
		self._parent._layout = self
		self._size = 0,0
		self._alignBottom = True

	def update(self):
		borderSize = self._parent._base._buttonSize * 0.2
		x0, y0, w, h = self._parent.getSize()
		x0 += borderSize
		y0 += borderSize
		widths = {}
		heights = {}
		for ctrl in self._parent._glGuiControlList:
			x, y = ctrl._pos
			w, h = ctrl.getMinSize()
			if not x in widths:
				widths[x] = w
			else:
				widths[x] = max(widths[x], w)
			if not y in heights:
				heights[y] = h
			else:
				heights[y] = max(heights[y], h)
		self._size = sum(widths.values()) + borderSize * 2, sum(heights.values()) + borderSize * 2
		if self._alignBottom:
			y0 -= self._size[1] - self._parent.getSize()[3]
			self._parent.setSize(x0 - borderSize, y0 - borderSize, self._size[0], self._size[1])
		for ctrl in self._parent._glGuiControlList:
			x, y = ctrl._pos
			x1 = x0
			y1 = y0
			for n in xrange(0, x):
				if not n in widths:
					widths[n] = 3
				x1 += widths[n]
			for n in xrange(0, y):
				if not n in heights:
					heights[n] = 3
				y1 += heights[n]
			ctrl.setSize(x1, y1, widths[x], heights[y])

	def getLayoutSize(self):
		return self._size
*/
class glButton : public glGuiControl
{
public:
	glButton(CWnd* parent, int imageID, TCHAR* tooltip, fpxy pos, int buttonid, fpxy size = fpxy(0,0));
	~glButton()
	{
		_memdel(this);
	}

	fpxy _buttonSize;
	int _hidden;
	TCHAR* _tooltip;
	CWnd* _parent;
	int _imageID;
	int _buttonid;
	int _selected;
	float _focus;
	int _disabled;
	int _showExpandArrow;
	float _progressBar;
	char* _altTooltip;
	GLuint _glButtonsTexture;

	void setSelected(int value);
	void setExpandArrow(int value);
	void setHidden(int value);
	void setDisabled(int value);
	void setProgressBar(float value);
	float getProgressBar();
	void setBottomText(char* value);
	int getSelected();
	fpxy getMinSize();
	fpxy _getPixelPos();
	void draw();
	int _checkHit(int x, int y);
	int OnMouseMotion(int x, int y);
	int OnMouseDown(int x, int y, int button);
};

class glRadioButton : public glButton
{
public:
	glRadioButton(CWnd* parent, int imageID, TCHAR* tooltip, fpxy pos, int group, int buttonid);
	void setSelected(int value);

	int _selected;
	void _onRadioSelect(int button);
};

class glComboButton : public glButton
{
public:
	int *_imageIDs;
	TCHAR** _tooltips;
	void* _comboCallback;
	int _selection;

	glComboButton(CWnd* parent, TCHAR* tooltip, int* imageIDs, TCHAR** tooltips, fpxy& pos, void* callback);
	void draw();
	int getValue();
	void setValue(int value);
};
/*
	def _onComboOpenSelect(self, button):
		if self.hasFocus():
			self._base._focus = None
		else:
			self._base._focus = self
*/
/*
	def OnMouseDown(self, x, y, button):
		if self._hidden or self._disabled:
			return False
		if self.hasFocus():
			bs = self._base._buttonSize / 2
			pos = self._getPixelPos()
			if 0 <= x - pos[0] <= bs and 0 <= y - pos[1] - bs <= bs * len(self._imageIDs):
				self._selection = int((y - pos[1] - bs) / bs)
				self._imageID = self._imageIDs[self._selection]
				self._base._focus = None
				self._comboCallback()
				return True
		return super(glComboButton, self).OnMouseDown(x, y, button)

class glFrame(glGuiContainer):
	def __init__(self, parent, pos):
		super(glFrame, self).__init__(parent, pos)
		self._selected = False
		self._focus = False
		self._hidden = False

	def setSelected(self, value):
		self._selected = value

	def setHidden(self, value):
		self._hidden = value
		for child in self._glGuiControlList:
			if self._base._focus == child:
				self._base._focus = None

	def getSelected(self):
		return self._selected

	def getMinSize(self):
		return self._base._buttonSize, self._base._buttonSize

	def _getPixelPos(self):
		x0, y0, w, h = self.getSize()
		return x0, y0

	def draw(self):
		if self._hidden:
			return

		bs = self._parent._buttonSize
		pos = self._getPixelPos()

		size = self._layout.getLayoutSize()
		glColor4ub(255,255,255,255)
		opengl.glDrawStretchedQuad(pos[0], pos[1], size[0], size[1], bs*0.75, 0)
		#Draw the controls on the frame
		super(glFrame, self).draw()

	def _checkHit(self, x, y):
		if self._hidden:
			return False
		pos = self._getPixelPos()
		w, h = self._layout.getLayoutSize()
		return 0 <= x - pos[0] <= w and 0 <= y - pos[1] <= h

	def OnMouseMotion(self, x, y):
		super(glFrame, self).OnMouseMotion(x, y)
		if self._checkHit(x, y):
			self._focus = True
			return True
		self._focus = False
		return False

	def OnMouseDown(self, x, y, button):
		if self._checkHit(x, y):
			super(glFrame, self).OnMouseDown(x, y, button)
			return True
		return False

class glNotification(glFrame):
	def __init__(self, parent, pos):
		self._anim = None
		super(glNotification, self).__init__(parent, pos)
		glGuiLayoutGrid(self)._alignBottom = False
		self._label = glLabel(self, "Notification", (0, 0))
		self._buttonExtra = glButton(self, 31, "???", (1, 0), self.onExtraButton, 25)
		self._button = glButton(self, 30, "", (2, 0), self.onClose, 25)
		self._padding = glLabel(self, "", (0, 1))
		self.setHidden(True)

	def setSize(self, x, y, w, h):
		w, h = self._layout.getLayoutSize()
		baseSize = self._base.GetSizeTuple()
		if self._anim is not None:
			super(glNotification, self).setSize(baseSize[0] / 2 - w / 2, baseSize[1] - self._anim.getPosition() - self._base._buttonSize * 0.2, 1, 1)
		else:
			super(glNotification, self).setSize(baseSize[0] / 2 - w / 2, baseSize[1] - self._base._buttonSize * 0.2, 1, 1)

	def draw(self):
		self.setSize(0,0,0,0)
		self.updateLayout()
		super(glNotification, self).draw()

	def message(self, text, extraButtonCallback = None, extraButtonIcon = None, extraButtonTooltip = None):
		self._anim = animation(self._base, -20, 25, 1)
		self.setHidden(False)
		self._label.setLabel(text)
		self._buttonExtra.setHidden(extraButtonCallback is None)
		self._buttonExtra._imageID = extraButtonIcon
		self._buttonExtra._tooltip = extraButtonTooltip
		self._extraButtonCallback = extraButtonCallback
		self._base._queueRefresh()
		self.updateLayout()

	def onExtraButton(self, button):
		self.onClose(button)
		self._extraButtonCallback()

	def onClose(self, button):
		if self._anim is not None:
			self._anim = animation(self._base, self._anim.getPosition(), -20, 1)
		else:
			self._anim = animation(self._base, 25, -20, 1)

class glLabel(glGuiControl):
	def __init__(self, parent, label, pos):
		self._label = label
		super(glLabel, self).__init__(parent, pos)

	def setLabel(self, label):
		self._label = label

	def getMinSize(self):
		w, h = opengl.glGetStringSize(self._label)
		return w + 10, h + 4

	def _getPixelPos(self):
		x0, y0, w, h = self.getSize()
		return x0, y0

	def draw(self):
		x, y, w, h = self.getSize()

		glPushMatrix()
		glTranslatef(x, y, 0)

#		glColor4ub(255,255,255,128)
#		glBegin(GL_QUADS)
#		glTexCoord2f(1, 0)
#		glVertex2f( w, 0)
#		glTexCoord2f(0, 0)
#		glVertex2f( 0, 0)
#		glTexCoord2f(0, 1)
#		glVertex2f( 0, h)
#		glTexCoord2f(1, 1)
#		glVertex2f( w, h)
#		glEnd()

		glTranslate(5, h - 5, 0)
		glColor4ub(255,255,255,255)
		opengl.glDrawStringLeft(self._label)
		glPopMatrix()

	def _checkHit(self, x, y):
		return False

	def OnMouseMotion(self, x, y):
		return False

	def OnMouseDown(self, x, y, button):
		return False

class glNumberCtrl(glGuiControl):
	def __init__(self, parent, value, pos, callback):
		self._callback = callback
		self._value = str(value)
		self._selectPos = 0
		self._maxLen = 6
		self._inCallback = False
		super(glNumberCtrl, self).__init__(parent, pos)

	def setValue(self, value):
		if self._inCallback:
			return
		self._value = str(value)

	def getMinSize(self):
		w, h = opengl.glGetStringSize("VALUES")
		return w + 10, h + 4

	def _getPixelPos(self):
		x0, y0, w, h = self.getSize()
		return x0, y0

	def draw(self):
		x, y, w, h = self.getSize()

		glPushMatrix()
		glTranslatef(x, y, 0)

		if self.hasFocus():
			glColor4ub(255,255,255,255)
		else:
			glColor4ub(255,255,255,192)
		glBegin(GL_QUADS)
		glTexCoord2f(1, 0)
		glVertex2f( w, 0)
		glTexCoord2f(0, 0)
		glVertex2f( 0, 0)
		glTexCoord2f(0, 1)
		glVertex2f( 0, h-1)
		glTexCoord2f(1, 1)
		glVertex2f( w, h-1)
		glEnd()

		glTranslate(5, h - 5, 0)
		glColor4ub(0,0,0,255)
		opengl.glDrawStringLeft(self._value)
		if self.hasFocus():
			glTranslate(opengl.glGetStringSize(self._value[0:self._selectPos])[0] - 2, -1, 0)
			opengl.glDrawStringLeft('|')
		glPopMatrix()

	def _checkHit(self, x, y):
		x1, y1, w, h = self.getSize()
		return 0 <= x - x1 <= w and 0 <= y - y1 <= h

	def OnMouseMotion(self, x, y):
		return False

	def OnMouseDown(self, x, y, button):
		if self._checkHit(x, y):
			self.setFocus()
			return True
		return False

	def OnKeyChar(self, c):
		self._inCallback = True
		if c == wx.WXK_LEFT:
			self._selectPos -= 1
			self._selectPos = max(0, self._selectPos)
		if c == wx.WXK_RIGHT:
			self._selectPos += 1
			self._selectPos = min(self._selectPos, len(self._value))
		if c == wx.WXK_UP:
			try:
				value = float(self._value)
			except:
				pass
			else:
				value += 0.1
				self._value = str(value)
				self._callback(self._value)
		if c == wx.WXK_DOWN:
			try:
				value = float(self._value)
			except:
				pass
			else:
				value -= 0.1
				if value > 0:
					self._value = str(value)
					self._callback(self._value)
		if c == wx.WXK_BACK and self._selectPos > 0:
			self._value = self._value[0:self._selectPos - 1] + self._value[self._selectPos:]
			self._selectPos -= 1
			self._callback(self._value)
		if c == wx.WXK_DELETE:
			self._value = self._value[0:self._selectPos] + self._value[self._selectPos + 1:]
			self._callback(self._value)
		if c == wx.WXK_TAB or c == wx.WXK_NUMPAD_ENTER or c == wx.WXK_RETURN:
			if wx.GetKeyState(wx.WXK_SHIFT):
				self.focusPrevious()
			else:
				self.focusNext()
		if (ord('0') <= c <= ord('9') or c == ord('.')) and len(self._value) < self._maxLen:
			self._value = self._value[0:self._selectPos] + chr(c) + self._value[self._selectPos:]
			self._selectPos += 1
			self._callback(self._value)
		self._inCallback = False

	def setFocus(self):
		self._base._focus = self
		self._selectPos = len(self._value)
		return True

class glCheckbox(glGuiControl):
	def __init__(self, parent, value, pos, callback):
		self._callback = callback
		self._value = value
		self._selectPos = 0
		self._maxLen = 6
		self._inCallback = False
		super(glCheckbox, self).__init__(parent, pos)

	def setValue(self, value):
		if self._inCallback:
			return
		self._value = str(value)

	def getValue(self):
		return self._value

	def getMinSize(self):
		return 20, 20

	def _getPixelPos(self):
		x0, y0, w, h = self.getSize()
		return x0, y0

	def draw(self):
		x, y, w, h = self.getSize()

		glPushMatrix()
		glTranslatef(x, y, 0)

		glColor3ub(255,255,255)
		if self._value:
			opengl.glDrawTexturedQuad(w/2-h/2,0, h, h, 28)
		else:
			opengl.glDrawTexturedQuad(w/2-h/2,0, h, h, 29)

		glPopMatrix()

	def _checkHit(self, x, y):
		x1, y1, w, h = self.getSize()
		return 0 <= x - x1 <= w and 0 <= y - y1 <= h

	def OnMouseMotion(self, x, y):
		return False

	def OnMouseDown(self, x, y, button):
		if self._checkHit(x, y):
			self._value = not self._value
			return True
		return False

class glSlider(glGuiControl):
	def __init__(self, parent, value, minValue, maxValue, pos, callback):
		super(glSlider, self).__init__(parent, pos)
		self._callback = callback
		self._focus = False
		self._hidden = False
		self._value = value
		self._minValue = minValue
		self._maxValue = maxValue

	def setValue(self, value):
		self._value = value

	def getValue(self):
		if self._value < self._minValue:
			return self._minValue
		if self._value > self._maxValue:
			return self._maxValue
		return self._value

	def setRange(self, minValue, maxValue):
		if maxValue < minValue:
			maxValue = minValue
		self._minValue = minValue
		self._maxValue = maxValue

	def getMinValue(self):
		return self._minValue

	def getMaxValue(self):
		return self._maxValue

	def setHidden(self, value):
		self._hidden = value

	def getMinSize(self):
		return self._base._buttonSize * 0.2, self._base._buttonSize * 4

	def _getPixelPos(self):
		x0, y0, w, h = self.getSize()
		minSize = self.getMinSize()
		return x0 + w / 2 - minSize[0] / 2, y0 + h / 2 - minSize[1] / 2

	def draw(self):
		if self._hidden:
			return

		w, h = self.getMinSize()
		pos = self._getPixelPos()

		glPushMatrix()
		glTranslatef(pos[0], pos[1], 0)
		glDisable(GL_TEXTURE_2D)
		if self.hasFocus():
			glColor4ub(60,60,60,255)
		else:
			glColor4ub(60,60,60,192)
		glBegin(GL_QUADS)
		glVertex2f( w/2,-h/2)
		glVertex2f(-w/2,-h/2)
		glVertex2f(-w/2, h/2)
		glVertex2f( w/2, h/2)
		glEnd()
		scrollLength = h - w
		if self._maxValue-self._minValue != 0:
			valueNormalized = ((self.getValue()-self._minValue)/(self._maxValue-self._minValue))
		else:
			valueNormalized = 0
		glTranslate(0.0,scrollLength/2,0)
		if True:  # self._focus:
			glColor4ub(0,0,0,255)
			glPushMatrix()
			glTranslate(-w/2,opengl.glGetStringSize(str(self._minValue))[1]/2,0)
			opengl.glDrawStringRight(str(self._minValue))
			glTranslate(0,-scrollLength,0)
			opengl.glDrawStringRight(str(self._maxValue))
			glTranslate(w,scrollLength-scrollLength*valueNormalized,0)
			opengl.glDrawStringLeft(str(self.getValue()))
			glPopMatrix()
		glColor4ub(255,255,255,240)
		glTranslate(0.0,-scrollLength*valueNormalized,0)
		glBegin(GL_QUADS)
		glVertex2f( w/2,-w/2)
		glVertex2f(-w/2,-w/2)
		glVertex2f(-w/2, w/2)
		glVertex2f( w/2, w/2)
		glEnd()
		glPopMatrix()

	def _checkHit(self, x, y):
		if self._hidden:
			return False
		pos = self._getPixelPos()
		w, h = self.getMinSize()
		return -w/2 <= x - pos[0] <= w/2 and -h/2 <= y - pos[1] <= h/2

	def setFocus(self):
		self._base._focus = self
		return True

	def OnMouseMotion(self, x, y):
		if self.hasFocus():
			w, h = self.getMinSize()
			scrollLength = h - w
			pos = self._getPixelPos()
			self.setValue(int(self._minValue + (self._maxValue - self._minValue) * -(y - pos[1] - scrollLength/2) / scrollLength))
			self._callback()
			return True
		if self._checkHit(x, y):
			self._focus = True
			return True
		self._focus = False
		return False

	def OnMouseDown(self, x, y, button):
		if self._checkHit(x, y):
			self.setFocus()
			self.OnMouseMotion(x, y)
			return True
		return False

	def OnMouseUp(self, x, y):
		if self.hasFocus():
			self._base._focus = None
			return True
		return False
*/

#endif // _OPENGLGUI_H_