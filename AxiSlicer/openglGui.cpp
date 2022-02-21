#include "stdafx.h"
#include "openglGui.h"
#include "opengl.h"

//OpenGL.ERROR_CHECKING = False

animation::animation(CView* gui, float start, float end, float runTime)
{
	_memnew(this, "animation", __FILE__, __LINE__);
	this->_start = start;
	this->_end = end;
	this->_startTime = (float)time(NULL);
	this->_runTime = runTime;
//		gui._animationList.append(self);
}

int animation::isDone()
{
	time_t t;
	time(&t);
	return t > (this->_startTime + this->_runTime);
}

float animation::getPosition()
{
	if (this->isDone())
		return this->_end;
	float f = ((float)time(NULL) - this->_startTime) / this->_runTime;
	float ts = f*f;
	float tc = f*f*f;
//		#f = 6*tc*ts + -15*ts*ts + 10*tc
	f = tc + -3*ts + 3*f;
	return this->_start + (this->_end - this->_start) * f;
}

glGuiControl::glGuiControl(CWnd* parent, fpxy& pos)
{
	_memnew(this, "glGuiControl", "", __LINE__);
	_parent = parent;
//	_base = parent->_base;
	_pos = pos;
	_size = CRect(0,0, 1, 1);
//	_parent.add(self)
}

glGuiControl::~glGuiControl()
{
	_memdel(this);
}

void glGuiControl::setSize(int x, int y, int w, int h)
{
	this->_size = CRect(x, y, w, h);
}

CRect glGuiControl::getSize()
{
	return this->_size;
}

CSize glGuiControl::getMinSize()
{
	return CSize(1, 1);
}

void glGuiControl::updateLayout()
{
//		pass
}

void glGuiControl::focusNext()
{
/*	for n in xrange(this->_parent._glGuiControlList.index(self) + 1, len(this->_parent._glGuiControlList))){
		if this->_parent._glGuiControlList[n].setFocus()){
			return;
		}
	}
	for n in xrange(0, this->_parent._glGuiControlList.index(self)){
		if this->_parent._glGuiControlList[n].setFocus()){
			return;
		}
	}*/
}


void glGuiControl::focusPrevious()
{/*
	for n in xrange(this->_parent._glGuiControlList.index(self) -1, -1, -1)){
		if this->_parent._glGuiControlList[n].setFocus()){
			return;
		}
	}
	for n in xrange(len(this->_parent._glGuiControlList) - 1, this->_parent._glGuiControlList.index(self), -1)){
		if this->_parent._glGuiControlList[n].setFocus()){
			return;
		}
	}*/
}

int glGuiControl::setFocus()
{
	return False;
}

int glGuiControl::hasFocus()
{
	return this->_focus == this;
}

void glGuiControl::OnMouseUp(int x, int y)
{
//		pass
}

void glGuiControl::OnKeyChar(int key)
{
}
/*
class glGuiContainer(glGuiControl):
	def __init__(self, parent, pos):
		this->_glGuiControlList = []
		glGuiLayoutButtons(self)
		super(glGuiContainer, self).__init__(parent, pos)

	def add(self, ctrl):
		this->_glGuiControlList.append(ctrl)
		this->updateLayout()

	def OnMouseDown(self, x, y, button):
		for ctrl in this->_glGuiControlList:
			if ctrl.OnMouseDown(x, y, button):
				return True
		return False

	def OnMouseUp(self, x, y):
		for ctrl in this->_glGuiControlList:
			if ctrl.OnMouseUp(x, y):
				return True
		return False

	def OnMouseMotion(self, x, y):
		handled = False
		for ctrl in this->_glGuiControlList:
			if ctrl.OnMouseMotion(x, y):
				handled = True
		return handled

	def draw(self):
		for ctrl in this->_glGuiControlList:
			ctrl.draw()

	def updateLayout(self):
		this->_layout.update()
		for ctrl in this->_glGuiControlList:
			ctrl.updateLayout()

class glGuiPanel(glcanvas.GLCanvas):
	def __init__(self, parent):
		attribList = (glcanvas.WX_GL_RGBA, glcanvas.WX_GL_DOUBLEBUFFER, glcanvas.WX_GL_DEPTH_SIZE, 24, glcanvas.WX_GL_STENCIL_SIZE, 8, 0)
		glcanvas.GLCanvas.__init__(self, parent, style=wx.WANTS_CHARS, attribList = attribList)
		this->_base = self
		this->_focus = None
		this->_container = None
		this->_container = glGuiContainer(self, (0,0))
		this->_shownError = False

		this->_context = glcanvas.GLContext(self)
		this->_glButtonsTexture = None
		this->_glRobotTexture = None
		this->_buttonSize = 64

		this->_animationList = []
		this->glReleaseList = []
		this->_refreshQueued = False
		this->_idleCalled = False

		wx.EVT_PAINT(self, this->_OnGuiPaint)
		wx.EVT_SIZE(self, this->_OnSize)
		wx.EVT_ERASE_BACKGROUND(self, this->_OnEraseBackground)
		wx.EVT_LEFT_DOWN(self, this->_OnGuiMouseDown)
		wx.EVT_LEFT_DCLICK(self, this->_OnGuiMouseDown)
		wx.EVT_LEFT_UP(self, this->_OnGuiMouseUp)
		wx.EVT_RIGHT_DOWN(self, this->_OnGuiMouseDown)
		wx.EVT_RIGHT_DCLICK(self, this->_OnGuiMouseDown)
		wx.EVT_RIGHT_UP(self, this->_OnGuiMouseUp)
		wx.EVT_MIDDLE_DOWN(self, this->_OnGuiMouseDown)
		wx.EVT_MIDDLE_DCLICK(self, this->_OnGuiMouseDown)
		wx.EVT_MIDDLE_UP(self, this->_OnGuiMouseUp)
		wx.EVT_MOTION(self, this->_OnGuiMouseMotion)
		wx.EVT_CHAR(self, this->_OnGuiKeyChar)
		wx.EVT_KILL_FOCUS(self, this->OnFocusLost)
		wx.EVT_IDLE(self, this->_OnIdle)

	def _OnIdle(self, e):
		this->_idleCalled = True
		if len(this->_animationList) > 0 or this->_refreshQueued:
			this->_refreshQueued = False
			for anim in this->_animationList:
				if anim.isDone():
					this->_animationList.remove(anim)
			this->Refresh()

	def _OnGuiKeyChar(self, e):
		if this->_focus is not None:
			this->_focus.OnKeyChar(e.GetKeyCode())
			this->Refresh()
		else:
			this->OnKeyChar(e.GetKeyCode())

	def OnFocusLost(self, e):
		this->_focus = None
		this->Refresh()

	def _OnGuiMouseDown(self,e):
		this->SetFocus()
		if this->_container.OnMouseDown(e.GetX(), e.GetY(), e.GetButton()):
			this->Refresh()
			return
		this->OnMouseDown(e)

	def _OnGuiMouseUp(self, e):
		if this->_container.OnMouseUp(e.GetX(), e.GetY()):
			this->Refresh()
			return
		this->OnMouseUp(e)

	def _OnGuiMouseMotion(self,e):
		this->Refresh()
		if not this->_container.OnMouseMotion(e.GetX(), e.GetY()):
			this->OnMouseMotion(e)

	def _OnGuiPaint(self, e):
		this->_idleCalled = False
		h = this->GetSize().GetHeight()
		w = this->GetSize().GetWidth()
		oldButtonSize = this->_buttonSize
		if h / 3 < w / 4:
			w = h * 4 / 3
		if w < 64 * 8:
			this->_buttonSize = 32
		elif w < 64 * 10:
			this->_buttonSize = 48
		elif w < 64 * 15:
			this->_buttonSize = 64
		elif w < 64 * 20:
			this->_buttonSize = 80
		else:
			this->_buttonSize = 96
		if this->_buttonSize != oldButtonSize:
			this->_container.updateLayout()

		dc = wx.PaintDC(self)
		try:
			this->SetCurrent(this->_context)
			for obj in this->glReleaseList:
				obj.release()
			del this->glReleaseList[:]
			renderStartTime = time.time()
			this->OnPaint(e)
			this->_drawGui()
			glFlush()
			if version.isDevVersion():
				renderTime = time.time() - renderStartTime
				if renderTime == 0:
					renderTime = 0.001
				glLoadIdentity()
				glTranslate(10, this->GetSize().GetHeight() - 30, -1)
				glColor4f(0.2,0.2,0.2,0.5)
				opengl.glDrawStringLeft("fps:%d" % (1 / renderTime))
			this->SwapBuffers()
		except:
			errStr = _("An error has occurred during the 3D view drawing.")
			tb = traceback.extract_tb(sys.exc_info()[2])
			errStr += "\n%s: '%s'" % (str(sys.exc_info()[0].__name__), str(sys.exc_info()[1]))
			for n in xrange(len(tb)-1, -1, -1):
				locationInfo = tb[n]
				errStr += "\n @ %s:%s:%d" % (os.path.basename(locationInfo[0]), locationInfo[2], locationInfo[1])
			if not this->_shownError:
				wx.CallAfter(wx.MessageBox, errStr, _("3D window error"), wx.OK | wx.ICON_EXCLAMATION)
				this->_shownError = True

	def _drawGui(self):
		if this->_glButtonsTexture is None:
			this->_glButtonsTexture = opengl.loadGLTexture('glButtons.png')
			this->_glRobotTexture = opengl.loadGLTexture('UltimakerRobot.png')

		glDisable(GL_DEPTH_TEST)
		glEnable(GL_BLEND)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
		glDisable(GL_LIGHTING)
		glColor4ub(255,255,255,255)

		glMatrixMode(GL_PROJECTION)
		glLoadIdentity()
		size = this->GetSize()
		glOrtho(0, size.GetWidth()-1, size.GetHeight()-1, 0, -1000.0, 1000.0)
		glMatrixMode(GL_MODELVIEW)
		glLoadIdentity()

		this->_container.draw()

		# glBindTexture(GL_TEXTURE_2D, this->_glRobotTexture)
		# glEnable(GL_TEXTURE_2D)
		# glPushMatrix()
		# glColor4f(1,1,1,1)
		# glTranslate(size.GetWidth(),size.GetHeight(),0)
		# s = this->_buttonSize * 1
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
		this->_container.setSize(0, 0, this->GetSize().GetWidth(), this->GetSize().GetHeight())
		this->_container.updateLayout()
		this->Refresh()

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
		wx.CallAfter(this->_queueRefresh)

	def _queueRefresh(self):
		if this->_idleCalled:
			wx.CallAfter(this->Refresh)
		else:
			this->_refreshQueued = True

	def add(self, ctrl):
		if this->_container is not None:
			this->_container.add(ctrl)

class glGuiLayoutButtons(object):
	def __init__(self, parent):
		this->_parent = parent
		this->_parent._layout = self

	def update(self):
		bs = this->_parent._base._buttonSize
		x0, y0, w, h = this->_parent.getSize()
		gridSize = bs * 1.0
		for ctrl in this->_parent._glGuiControlList:
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
		_, _, w, h = this->_parent.getSize()
		return w, h

class glGuiLayoutGrid(object):
	def __init__(self, parent):
		this->_parent = parent
		this->_parent._layout = self
		this->_size = 0,0
		this->_alignBottom = True

	def update(self):
		borderSize = this->_parent._base._buttonSize * 0.2
		x0, y0, w, h = this->_parent.getSize()
		x0 += borderSize
		y0 += borderSize
		widths = {}
		heights = {}
		for ctrl in this->_parent._glGuiControlList:
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
		this->_size = sum(widths.values()) + borderSize * 2, sum(heights.values()) + borderSize * 2
		if this->_alignBottom:
			y0 -= this->_size[1] - this->_parent.getSize()[3]
			this->_parent.setSize(x0 - borderSize, y0 - borderSize, this->_size[0], this->_size[1])
		for ctrl in this->_parent._glGuiControlList:
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
		return this->_size
*/
glButton::glButton(CWnd* parent, int imageID, TCHAR* tooltip, fpxy pos, int buttonid, fpxy size) : glGuiControl(parent, pos)
{
	_memnew(this, "glButton", __FILE__, __LINE__);

	this->_buttonSize = size;
	this->_hidden = False;
	this->_tooltip = tooltip;
	this->_parent = parent;
	this->_imageID = imageID;
	this->_buttonid = buttonid;
	this->_selected = False;
	this->_focus = False;
	this->_disabled = False;
	this->_showExpandArrow = False;
	this->_progressBar = NULL;
	this->_altTooltip = "";
}

void glButton::setSelected(int value)
{
	this->_selected = value;
}

void glButton::setExpandArrow(int value)
{
	this->_showExpandArrow = value;
}

void glButton::setHidden(int value)
{
	this->_hidden = value;
}

void glButton::setDisabled(int value)
{
	this->_disabled = value;
}

void glButton::setProgressBar(float value)
{
	this->_progressBar = value;
}

float glButton::getProgressBar()
{
	return this->_progressBar;
}

void glButton::setBottomText(char* value)
{
	this->_altTooltip = value;
}

int glButton::getSelected()
{
	return this->_selected;
}

fpxy glButton::getMinSize()
{
	if( this->_hidden)
		return 0, 0;
	if( this->_buttonSize.x && _buttonSize.y)
		return _buttonSize;

	return fpxy(0,0);
//	return this->_base._buttonSize, this->_base._buttonSize;
}

fpxy glButton::_getPixelPos()
{
	CRect r = this->getSize();
	return fpxy(r.left + r.Width() / 2, r.top + r.Height() / 2);
}

void glButton::draw()
{
	if (this->_hidden)
		return;

	int cx = (this->_imageID % 4) / 4;
	int cy = int(this->_imageID / 4) / 4;
	float bs = this->getMinSize().x;
	fpxy pos = this->_getPixelPos();

	glBindTexture(GL_TEXTURE_2D, this->_glButtonsTexture);
	float scale = 0.8;
	if( this->_selected)
		scale = 1.0;
	else if( this->_focus)
		scale = 0.9;
	if( this->_disabled)
		glColor4ub(128,128,128,128);
	else
		glColor4ub(255,255,255,255);
	glDrawTexturedQuad(pos.x-bs*scale/2, pos.y-bs*scale/2, bs*scale, bs*scale, 0);
	glDrawTexturedQuad(pos.x-bs*scale/2, pos.y-bs*scale/2, bs*scale, bs*scale, this->_imageID);
	if( this->_showExpandArrow) {
		if( this->_selected)
			glDrawTexturedQuad(pos.x+bs*scale/2-bs*scale/4*1.2, pos.y-bs*scale/2*1.2, bs*scale/4, bs*scale/4, 1);
		else
			glDrawTexturedQuad(pos.x+bs*scale/2-bs*scale/4*1.2, pos.y-bs*scale/2*1.2, bs*scale/4, bs*scale/4, 1, 2);
	}
	glPushMatrix();
	glTranslatef(pos.x, pos.y, 0);
	glDisable(GL_TEXTURE_2D);
	if( this->_focus){
		glTranslatef(0, -0.55*bs*scale, 0);

		glPushMatrix();
		glColor4ub(60,60,60,255);
		glTranslatef(-1, -1, 0);
		glDrawStringCenter(this->_tooltip);
		glTranslatef(0, 2, 0);
		glDrawStringCenter(this->_tooltip);
		glTranslatef(2, 0, 0);
		glDrawStringCenter(this->_tooltip);
		glTranslatef(0, -2, 0);
		glDrawStringCenter(this->_tooltip);
		glPopMatrix();

		glColor4ub(255,255,255,255);
		glDrawStringCenter(this->_tooltip);
	}
	glPopMatrix();
	float progress = this->_progressBar;
	if( progress != NULL){
		glColor4ub(60,60,60,255);
		glDrawQuad(pos.x-bs/2, pos.y+bs/2, bs, bs / 4);
		glColor4ub(255,255,255,255);
		glDrawQuad(pos.x-bs/2+2, pos.y+bs/2+2, (bs - 5) * progress + 1, bs / 4 - 4);
	} else if( strlen(this->_altTooltip) > 0) {
		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);
		glTranslatef(0, 0.6*bs, 0);
		glTranslatef(0, 6, 0);
//		#glTranslatef(0.6*bs*scale, 0, 0);

		strList* lines = new strList(__FILE__, __LINE__);
		lines->split(this->_altTooltip, '\n');
		for(int l = 0; l < lines->size; l++) {
			TCHAR* line = lines->ptrs[l];
			glPushMatrix();
			glColor4ub(60,60,60,255);
			glTranslatef(-1, -1, 0);
			glDrawStringCenter(line);
			glTranslatef(0, 2, 0);
			glDrawStringCenter(line);
			glTranslatef(2, 0, 0);
			glDrawStringCenter(line);
			glTranslatef(0, -2, 0);
			glDrawStringCenter(line);
			glPopMatrix();

			glColor4ub(255,255,255,255);
			glDrawStringCenter(line);
			glTranslatef(0, 18, 0);
		}
		delete lines;

		glPopMatrix();
	}
}


int glButton::_checkHit(int x, int y)
{
	if( this->_hidden || this->_disabled)
		return False;
	float bs = this->getMinSize().x;
	fpxy pos = this->_getPixelPos();
//	return -bs * 0.5 <= x - pos.x <= bs * 0.5 && -bs * 0.5 <= y - pos.y <= bs * 0.5;
	return (-bs * 0.5 <= x - pos.x) && (x - pos.x <= bs * 0.5) && (-bs * 0.5 <= y - pos.y) && (y - pos.y <= bs * 0.5);
}

int glButton::OnMouseMotion(int x, int y)
{
	if( this->_checkHit(x, y)){
		this->_focus = True;
		return True;
	}
	this->_focus = False;
	return False;
}

int glButton::OnMouseDown(int x, int y, int button)
{
	if( this->_checkHit(x, y)){
		this->_parent->PostMessageW(WM_COMMAND, _buttonid, 0);
		return True;
	}
	return False;
}		

glRadioButton::glRadioButton(CWnd* parent, int imageID, TCHAR* tooltip, fpxy pos, int group, int buttonid) :
	glButton(parent, imageID, tooltip, pos, buttonid)
{
//	this->_group = group;
//	this->_group.append(self);
}

void glRadioButton::setSelected(int value)
{
	this->_selected = value;
}

void glRadioButton::_onRadioSelect(int button)
{
/*
	//this->_base._focus = None
	for ctrl in this->_group) {
		if( ctrl != self)
			ctrl.setSelected(False);
	}
	if( (this->getSelected())
		this->setSelected(False);
	else
		this->setSelected(True);
	this->_radioCallback(button);
*/
}


glComboButton::glComboButton(CWnd* parent, TCHAR* tooltip, int* imageIDs, TCHAR** tooltips, fpxy& pos, void* callback) :
	glButton(parent, imageIDs[0], tooltip, pos, 0)
{
		this->_imageIDs = imageIDs;
		this->_tooltips = tooltips;
		this->_comboCallback = callback;
		this->_selection = 0;
}
/*
	def _onComboOpenSelect(self, button):
		if( this->hasFocus():
			this->_base._focus = None
		else:
			this->_base._focus = self
*/
void glComboButton::draw()
{
	if (this->_hidden)
		return;

	this->_selected = this->hasFocus();
	glButton::draw();

//	fpxy bs = this->_base._buttonSize / 2;
	float bs;
	fpxy pos = this->_getPixelPos();

	if (!this->_selected)
		return;

	glPushMatrix();
	glTranslatef(pos.x+bs*0.5, pos.y + bs*0.5, 0);
	glBindTexture(GL_TEXTURE_2D, this->_glButtonsTexture);
	for (int n = 0; this->_imageIDs[n] != -1; n++) {
		glTranslatef(0, bs, 0);
		glColor4ub(255,255,255,255);
		glDrawTexturedQuad(-0.5*bs,-0.5*bs,bs,bs, 0);
		glDrawTexturedQuad(-0.5*bs,-0.5*bs,bs,bs, this->_imageIDs[n]);
		glDisable(GL_TEXTURE_2D);

		glPushMatrix();
		glTranslatef(-0.55*bs, 0.1*bs, 0);

		glPushMatrix();
		glColor4ub(60,60,60,255);
		glTranslatef(-1, -1, 0);
		glDrawStringRight(this->_tooltips[n]);
		glTranslatef(0, 2, 0);
		glDrawStringRight(this->_tooltips[n]);
		glTranslatef(2, 0, 0);
		glDrawStringRight(this->_tooltips[n]);
		glTranslatef(0, -2, 0);
		glDrawStringRight(this->_tooltips[n]);
		glPopMatrix();

		glColor4ub(255,255,255,255);
		glDrawStringRight(this->_tooltips[n]);
		glPopMatrix();
	}
	glPopMatrix();
}

int glComboButton::getValue()
{
	return this->_selection;
}

void glComboButton::setValue(int value)
{
	this->_selection = value;
/*
	this->_imageID = this->_imageIDs[this->_selection]
	this->_comboCallback();
*/
}
/*
	def OnMouseDown(self, x, y, button):
		if( this->_hidden or this->_disabled:
			return False
		if( this->hasFocus():
			bs = this->_base._buttonSize / 2
			pos = this->_getPixelPos()
			if( 0 <= x - pos[0] <= bs and 0 <= y - pos[1] - bs <= bs * len(this->_imageIDs):
				this->_selection = int((y - pos[1] - bs) / bs)
				this->_imageID = this->_imageIDs[this->_selection]
				this->_base._focus = None
				this->_comboCallback()
				return True
		return super(glComboButton, self).OnMouseDown(x, y, button)

class glFrame(glGuiContainer):
	def __init__(self, parent, pos):
		super(glFrame, self).__init__(parent, pos)
		this->_selected = False
		this->_focus = False
		this->_hidden = False

	def setSelected(self, value):
		this->_selected = value

	def setHidden(self, value):
		this->_hidden = value
		for child in this->_glGuiControlList:
			if( this->_base._focus == child:
				this->_base._focus = None

	def getSelected(self):
		return this->_selected

	def getMinSize(self):
		return this->_base._buttonSize, this->_base._buttonSize

	def _getPixelPos(self):
		x0, y0, w, h = this->getSize()
		return x0, y0

	def draw(self):
		if( this->_hidden:
			return

		bs = this->_parent._buttonSize
		pos = this->_getPixelPos()

		size = this->_layout.getLayoutSize()
		glColor4ub(255,255,255,255)
		opengl.glDrawStretchedQuad(pos[0], pos[1], size[0], size[1], bs*0.75, 0)
		#Draw the controls on the frame
		super(glFrame, self).draw()

	def _checkHit(self, x, y):
		if( this->_hidden:
			return False
		pos = this->_getPixelPos()
		w, h = this->_layout.getLayoutSize()
		return 0 <= x - pos[0] <= w and 0 <= y - pos[1] <= h

	def OnMouseMotion(self, x, y):
		super(glFrame, self).OnMouseMotion(x, y)
		if( this->_checkHit(x, y):
			this->_focus = True
			return True
		this->_focus = False
		return False

	def OnMouseDown(self, x, y, button):
		if( this->_checkHit(x, y):
			super(glFrame, self).OnMouseDown(x, y, button)
			return True
		return False

class glNotification(glFrame):
	def __init__(self, parent, pos):
		this->_anim = None
		super(glNotification, self).__init__(parent, pos)
		glGuiLayoutGrid(self)._alignBottom = False
		this->_label = glLabel(self, "Notification", (0, 0))
		this->_buttonExtra = glButton(self, 31, "???", (1, 0), this->onExtraButton, 25)
		this->_button = glButton(self, 30, "", (2, 0), this->onClose, 25)
		this->_padding = glLabel(self, "", (0, 1))
		this->setHidden(True)

	def setSize(self, x, y, w, h):
		w, h = this->_layout.getLayoutSize()
		baseSize = this->_base.GetSizeTuple()
		if( this->_anim is not None:
			super(glNotification, self).setSize(baseSize[0] / 2 - w / 2, baseSize[1] - this->_anim.getPosition() - this->_base._buttonSize * 0.2, 1, 1)
		else:
			super(glNotification, self).setSize(baseSize[0] / 2 - w / 2, baseSize[1] - this->_base._buttonSize * 0.2, 1, 1)

	def draw(self):
		this->setSize(0,0,0,0)
		this->updateLayout()
		super(glNotification, self).draw()

	def message(self, text, extraButtonCallback = None, extraButtonIcon = None, extraButtonTooltip = None):
		this->_anim = animation(this->_base, -20, 25, 1)
		this->setHidden(False)
		this->_label.setLabel(text)
		this->_buttonExtra.setHidden(extraButtonCallback is None)
		this->_buttonExtra._imageID = extraButtonIcon
		this->_buttonExtra._tooltip = extraButtonTooltip
		this->_extraButtonCallback = extraButtonCallback
		this->_base._queueRefresh()
		this->updateLayout()

	def onExtraButton(self, button):
		this->onClose(button)
		this->_extraButtonCallback()

	def onClose(self, button):
		if( this->_anim is not None:
			this->_anim = animation(this->_base, this->_anim.getPosition(), -20, 1)
		else:
			this->_anim = animation(this->_base, 25, -20, 1)

class glLabel(glGuiControl):
	def __init__(self, parent, label, pos):
		this->_label = label
		super(glLabel, self).__init__(parent, pos)

	def setLabel(self, label):
		this->_label = label

	def getMinSize(self):
		w, h = opengl.glGetStringSize(this->_label)
		return w + 10, h + 4

	def _getPixelPos(self):
		x0, y0, w, h = this->getSize()
		return x0, y0

	def draw(self):
		x, y, w, h = this->getSize()

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
		opengl.glDrawStringLeft(this->_label)
		glPopMatrix()

	def _checkHit(self, x, y):
		return False

	def OnMouseMotion(self, x, y):
		return False

	def OnMouseDown(self, x, y, button):
		return False

class glNumberCtrl(glGuiControl):
	def __init__(self, parent, value, pos, callback):
		this->_callback = callback
		this->_value = str(value)
		this->_selectPos = 0
		this->_maxLen = 6
		this->_inCallback = False
		super(glNumberCtrl, self).__init__(parent, pos)

	def setValue(self, value):
		if( this->_inCallback:
			return
		this->_value = str(value)

	def getMinSize(self):
		w, h = opengl.glGetStringSize("VALUES")
		return w + 10, h + 4

	def _getPixelPos(self):
		x0, y0, w, h = this->getSize()
		return x0, y0

	def draw(self):
		x, y, w, h = this->getSize()

		glPushMatrix()
		glTranslatef(x, y, 0)

		if( this->hasFocus():
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
		opengl.glDrawStringLeft(this->_value)
		if( this->hasFocus():
			glTranslate(opengl.glGetStringSize(this->_value[0:this->_selectPos])[0] - 2, -1, 0)
			opengl.glDrawStringLeft('|')
		glPopMatrix()

	def _checkHit(self, x, y):
		x1, y1, w, h = this->getSize()
		return 0 <= x - x1 <= w and 0 <= y - y1 <= h

	def OnMouseMotion(self, x, y):
		return False

	def OnMouseDown(self, x, y, button):
		if( this->_checkHit(x, y):
			this->setFocus()
			return True
		return False

	def OnKeyChar(self, c):
		this->_inCallback = True
		if( c == wx.WXK_LEFT:
			this->_selectPos -= 1
			this->_selectPos = max(0, this->_selectPos)
		if( c == wx.WXK_RIGHT:
			this->_selectPos += 1
			this->_selectPos = min(this->_selectPos, len(this->_value))
		if( c == wx.WXK_UP:
			try:
				value = float(this->_value)
			except:
				pass
			else:
				value += 0.1
				this->_value = str(value)
				this->_callback(this->_value)
		if( c == wx.WXK_DOWN:
			try:
				value = float(this->_value)
			except:
				pass
			else:
				value -= 0.1
				if( value > 0:
					this->_value = str(value)
					this->_callback(this->_value)
		if( c == wx.WXK_BACK and this->_selectPos > 0:
			this->_value = this->_value[0:this->_selectPos - 1] + this->_value[this->_selectPos:]
			this->_selectPos -= 1
			this->_callback(this->_value)
		if( c == wx.WXK_DELETE:
			this->_value = this->_value[0:this->_selectPos] + this->_value[this->_selectPos + 1:]
			this->_callback(this->_value)
		if( c == wx.WXK_TAB or c == wx.WXK_NUMPAD_ENTER or c == wx.WXK_RETURN:
			if( wx.GetKeyState(wx.WXK_SHIFT):
				this->focusPrevious()
			else:
				this->focusNext()
		if( (ord('0') <= c <= ord('9') or c == ord('.')) and len(this->_value) < this->_maxLen:
			this->_value = this->_value[0:this->_selectPos] + chr(c) + this->_value[this->_selectPos:]
			this->_selectPos += 1
			this->_callback(this->_value)
		this->_inCallback = False

	def setFocus(self):
		this->_base._focus = self
		this->_selectPos = len(this->_value)
		return True

class glCheckbox(glGuiControl):
	def __init__(self, parent, value, pos, callback):
		this->_callback = callback
		this->_value = value
		this->_selectPos = 0
		this->_maxLen = 6
		this->_inCallback = False
		super(glCheckbox, self).__init__(parent, pos)

	def setValue(self, value):
		if( this->_inCallback:
			return
		this->_value = str(value)

	def getValue(self):
		return this->_value

	def getMinSize(self):
		return 20, 20

	def _getPixelPos(self):
		x0, y0, w, h = this->getSize()
		return x0, y0

	def draw(self):
		x, y, w, h = this->getSize()

		glPushMatrix()
		glTranslatef(x, y, 0)

		glColor3ub(255,255,255)
		if( this->_value:
			opengl.glDrawTexturedQuad(w/2-h/2,0, h, h, 28)
		else:
			opengl.glDrawTexturedQuad(w/2-h/2,0, h, h, 29)

		glPopMatrix()

	def _checkHit(self, x, y):
		x1, y1, w, h = this->getSize()
		return 0 <= x - x1 <= w and 0 <= y - y1 <= h

	def OnMouseMotion(self, x, y):
		return False

	def OnMouseDown(self, x, y, button):
		if( this->_checkHit(x, y):
			this->_value = not this->_value
			return True
		return False

class glSlider(glGuiControl):
	def __init__(self, parent, value, minValue, maxValue, pos, callback):
		super(glSlider, self).__init__(parent, pos)
		this->_callback = callback
		this->_focus = False
		this->_hidden = False
		this->_value = value
		this->_minValue = minValue
		this->_maxValue = maxValue

	def setValue(self, value):
		this->_value = value

	def getValue(self):
		if( this->_value < this->_minValue:
			return this->_minValue
		if( this->_value > this->_maxValue:
			return this->_maxValue
		return this->_value

	def setRange(self, minValue, maxValue):
		if( maxValue < minValue:
			maxValue = minValue
		this->_minValue = minValue
		this->_maxValue = maxValue

	def getMinValue(self):
		return this->_minValue

	def getMaxValue(self):
		return this->_maxValue

	def setHidden(self, value):
		this->_hidden = value

	def getMinSize(self):
		return this->_base._buttonSize * 0.2, this->_base._buttonSize * 4

	def _getPixelPos(self):
		x0, y0, w, h = this->getSize()
		minSize = this->getMinSize()
		return x0 + w / 2 - minSize[0] / 2, y0 + h / 2 - minSize[1] / 2

	def draw(self):
		if( this->_hidden:
			return

		w, h = this->getMinSize()
		pos = this->_getPixelPos()

		glPushMatrix()
		glTranslatef(pos[0], pos[1], 0)
		glDisable(GL_TEXTURE_2D)
		if( this->hasFocus():
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
		if( this->_maxValue-this->_minValue != 0:
			valueNormalized = ((this->getValue()-this->_minValue)/(this->_maxValue-this->_minValue))
		else:
			valueNormalized = 0
		glTranslate(0.0,scrollLength/2,0)
		if( True:  # this->_focus:
			glColor4ub(0,0,0,255)
			glPushMatrix()
			glTranslate(-w/2,opengl.glGetStringSize(str(this->_minValue))[1]/2,0)
			opengl.glDrawStringRight(str(this->_minValue))
			glTranslate(0,-scrollLength,0)
			opengl.glDrawStringRight(str(this->_maxValue))
			glTranslate(w,scrollLength-scrollLength*valueNormalized,0)
			opengl.glDrawStringLeft(str(this->getValue()))
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
		if( this->_hidden:
			return False
		pos = this->_getPixelPos()
		w, h = this->getMinSize()
		return -w/2 <= x - pos[0] <= w/2 and -h/2 <= y - pos[1] <= h/2

	def setFocus(self):
		this->_base._focus = self
		return True

	def OnMouseMotion(self, x, y):
		if( this->hasFocus():
			w, h = this->getMinSize()
			scrollLength = h - w
			pos = this->_getPixelPos()
			this->setValue(int(this->_minValue + (this->_maxValue - this->_minValue) * -(y - pos[1] - scrollLength/2) / scrollLength))
			this->_callback()
			return True
		if( this->_checkHit(x, y):
			this->_focus = True
			return True
		this->_focus = False
		return False

	def OnMouseDown(self, x, y, button):
		if( this->_checkHit(x, y):
			this->setFocus()
			this->OnMouseMotion(x, y)
			return True
		return False

	def OnMouseUp(self, x, y):
		if( this->hasFocus():
			this->_base._focus = None
			return True
		return False
*/
