
// editgcodeView.cpp : CeditgcodeView 클래스의 구현
//

#include "stdafx.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "editgcode.h"
#endif

#include "openglGui.h"
#include "opengl.h"
#include "sceneView.h"
#include "profile.h"
#include "meshLoader.h"
#include "stlio.h"
#include "gcodeInterpreter.h"
#include "mesh.h"
#include "objectScene.h"

#include "editgcodeDoc.h"
#include "editgcodeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CeditgcodeView

IMPLEMENT_DYNCREATE(CeditgcodeView, CView)

BEGIN_MESSAGE_MAP(CeditgcodeView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CeditgcodeView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEHWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CeditgcodeView 생성/소멸

CeditgcodeView::CeditgcodeView()
{
	_scene = NULL;
#if ENABLE_GCODE
	_gcode = NULL;
	glReleaseList = new GLVBOList();
#endif

	_objectShader = NULL;
	_objectLoadShader = NULL;
	_objectOverhangShader = NULL;

#if GLGUI_ANIMATION
	_animView = NULL;
	_animZoom = NULL;
#endif

	tempMatrix = NULL;

	openFileButton = NULL;
	printButton = NULL;

	// TODO: 여기에 생성 코드를 추가합니다.
	m_hRC = NULL;	//렌더링 컨텍스트
	m_hDC = NULL;		//GDI장치 컨텍스트

	this->_yaw = 30;
	this->_pitch = 60;
	this->_zoom = 300;
	this->_scene = new Scene();
#if ENABLE_GCODE
	this->_gcode = NULL;
	this->_gcodeVBOs = NULL; // [];
	this->_gcodeFilename[0] = 0;
	this->_gcodeLoadThread = NULL;
	this->_gcodeLoadThread_isAlive = 0;
#endif

	this->_objectShader = NULL;
	this->_objectLoadShader = NULL;
	this->_focusObj = NULL;
	this->_selectedObj = NULL;
	// this->_objColors = NULL; // [NULL,NULL,NULL,NULL];
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
#if USE_SIMPLEMODE
	this->_isSimpleMode = True;
#endif
	this->_viewport[0] = -99999999; // = NULL;
	this->_viewport[1] = -99999999; // = NULL;
	//this->_modelMatrix = NULL;
	//this->_projMatrix = NULL;
	this->tempMatrix = NULL;
}

BOOL CeditgcodeView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CeditgcodeView 인쇄
void CeditgcodeView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CeditgcodeView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CeditgcodeView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CeditgcodeView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}

void CeditgcodeView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CeditgcodeView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CeditgcodeView 진단

#ifdef _DEBUG
void CeditgcodeView::AssertValid() const
{
	CView::AssertValid();
}

void CeditgcodeView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CeditgcodeDoc* CeditgcodeView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CeditgcodeDoc)));
	return (CeditgcodeDoc*)m_pDocument;
}
#endif //_DEBUG


CeditgcodeView::~CeditgcodeView()
{
	if(_scene != NULL)
		delete _scene;
#if ENABLE_GCODE
	if(_gcode != NULL)
		delete _gcode;
	if(	glReleaseList != NULL)
		delete glReleaseList;
	if(_gcodeVBOs != NULL)
		delete _gcodeVBOs ;
//	if(_gcodeLoadThread != NULL)
//		delete _gcodeLoadThread ;
#endif

	if(_objectShader != NULL)
		delete _objectShader ;
	if(_objectLoadShader != NULL)
		delete _objectLoadShader ;
	if(_objectOverhangShader != NULL)
		delete _objectOverhangShader;

#if GLGUI_ANIMATION
	animation* _animView;
	animation* _animZoom;
#endif
	float _platformMesh;
#if USE_SIMPLEMODE
	int   _isSimpleMode;
#endif
	float _usbPrintMonitor;
	float _printerConnectionManager;

	GLdouble _viewport[4];
	GLdouble _modelMatrix[16];
	GLdouble _projMatrix[16];
	if(tempMatrix != NULL)
		delete tempMatrix;

	fpxyz _machineSize;

	if(openFileButton != NULL)
		delete openFileButton ;
	if(NULL != printButton)
		delete printButton;
}

// CeditgcodeView 그리기

void CeditgcodeView::OnDraw(CDC* /*pDC*/)
{
	CeditgcodeDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
	wglMakeCurrent(m_hDC, m_hRC);

//	GLRenderScene();
	OnPaint(0);

	SwapBuffers(m_hDC);

	wglMakeCurrent(m_hDC, NULL);
}

void CeditgcodeView::OnRepaint()
{
	OnDraw(NULL);
}

// CeditgcodeView 인쇄

// CeditgcodeView 진단

// CeditgcodeView 메시지 처리기

void CeditgcodeView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	CView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	VERIFY(wglMakeCurrent(m_hDC, m_hRC));
	GLResize(cx,cy);
	VERIFY(wglMakeCurrent(NULL, NULL));
}

void CeditgcodeView::GLResize(int cx, int cy)
{
	GLfloat fAspect;

	if(cy == 0)
		cy = 1;

	glViewport(0, 0, cx, cy);				//함수를 사용하여, 모니터 전체 화면을 뷰포트 영역으로 잡아 준다.

	fAspect = (GLfloat)cx / (GLfloat)cy;
	
	glMatrixMode(GL_PROJECTION);			// 원근 투영 임을 선언
	glLoadIdentity();

	gluPerspective(60.0f, fAspect, 1.0f, 10000.0f);		//관측 공간을 설정

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


int CeditgcodeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.
	openFileButton      = NULL; // new glButton(this, 4, _T("Load"), (0,0), showLoadModel);
	printButton         = NULL; // new glButton(this, 6, _T("Print"), (1,0), OnPrintButton);
	//printButton->setDisabled(True);

	int group = 0; // group should be a list of controls
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
*/
	static int imgids[] = {7,19,11,15,23, -1};

//	this->layerSelect = openglGui.glSlider(self, 10000, 0, 1, (-1,-2), lambda : this->QueueRefresh())
/*
	this->youMagineButton = openglGui.glButton(self, 26, _("Share on YouMagine"), (2,0), lambda button: youmagineGui.youmagineManager(this->GetTopLevelParent(), this->_scene))
	this->youMagineButton.setDisabled(True)

	this->notification = openglGui.glNotification(self, (0, 0))
*/

	/*
	this->_sceneUpdateTimer = wx.Timer(self)
	this->Bind(wx.EVT_TIMER, this->_onRunSlicer, this->_sceneUpdateTimer)
	this->Bind(wx.EVT_MOUSEWHEEL, this->OnMouseWheel)
	this->Bind(wx.EVT_LEAVE_WINDOW, this->OnMouseLeave)

	this->OnViewChange()
	this->OnToolSelect(0)
	this->updateToolButtons()
*/
	this->updateProfileToControls();

	int nPixelFormat;
	m_hDC = ::GetDC(m_hWnd);

	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(	PIXELFORMATDESCRIPTOR ),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0,0,0,0,0,0,
		0,0,
		0,0,0,0,0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0,0,0
	};
	nPixelFormat = ChoosePixelFormat(m_hDC, &pfd);
	VERIFY(SetPixelFormat(m_hDC, nPixelFormat, &pfd));
	m_hRC = wglCreateContext(m_hDC);
	VERIFY(wglMakeCurrent(m_hDC, m_hRC));
	wglMakeCurrent(NULL,NULL);

	testShader();

	return 0;
}

void CeditgcodeView::GLRenderScene(void)
{
	glLoadIdentity();	//행렬을 초기화
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//현재 버퍼를 비워줌
	glColor3f(1.0f,1.0f,1.0f);	// 버텍스의 색깔을 흰색으로 셋팅
	glEnable(GL_DEPTH_TEST);	// 깊이 테스트 On

	gluLookAt(0.0f, 0.0f, 1000.0f, 0.0f, 10.f, 0.0f, 0.0f, 1.0f, 0.0f);
	//카메라의 위치는 (0,0,1000), 카메라가 바라보는곳 (0, 10, 0), 카메라 up vector (0, 1, 0);
	
	glBegin(GL_QUADS);
		glVertex3f(-50, -50, 0);
		glVertex3f(50, -50, 0);
		glVertex3f(50, 50, 0);
		glVertex3f(-50, 50, 0);
	glEnd();

	glFlush();
}



BOOL CeditgcodeView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
//  OpenGL 사용할 때 껌벅거리는 것을 막는다
//	return CView::OnEraseBkgnd(pDC);
	return TRUE;
}

void CeditgcodeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	this->OnMouseDown(point.x, point.y, nFlags);

	CView::OnLButtonDown(nFlags, point);
}

void loadGCodeCaller(LPVOID lpVoid)
{
	CeditgcodeView* view = (CeditgcodeView*)lpVoid;
	view->_loadGCode();
}


void CeditgcodeView::InvalidateAllGCodeVBOs()
{
	if(this->_gcodeVBOs != NULL) {
		for(int i = 0; i < this->_gcodeVBOs->len; i++) {
			GLVBOList* layerVBOlist = this->_gcodeVBOs->data[i];
			for(int j = 0; j < layerVBOlist->len; j++){
				GLVBO* vbo = layerVBOlist->data[j];
				this->glReleaseList->append(vbo);
			}
		}
		delete this->_gcodeVBOs;
	}
	this->_gcodeVBOs = new GLVBOListList;
}

void CeditgcodeView::ReloadGCodeFiles()
{
#if ENABLE_GCODE
	if (this->_gcode != NULL){
		delete _gcode;
		this->_gcode = NULL;
	}
	InvalidateAllGCodeVBOs();

	this->_gcode = new gcode(this);
	this->_gcode->ReloadFiles();
	Invalidate();
#endif
}

void CeditgcodeView::loadGCodeFile(TCHAR* filename)
{
	this->OnDeleteAll(NULL);
#if ENABLE_GCODE
	if (this->_gcode != NULL){
		delete _gcode;
		this->_gcode = NULL;
	}
	InvalidateAllGCodeVBOs();

	this->_gcode = new gcode(this);
	lstrcpy(this->_gcodeFilename, filename);
/*
	this->printButton.setBottomText("");
*/

/*
	this->printButton.setDisabled(False);
	this->youMagineButton.setDisabled(True);
*/
	if (this->_gcode != NULL && this->_gcode->layerList == NULL){
		this->_gcodeLoadThread_isAlive = 1;
		this->_gcodeLoadThread = _beginthread(loadGCodeCaller, 0, this); // this->_gcodeLoadThread = threading.Thread(target=this->_loadGCode);
		//this->_gcodeLoadThread.daemon = True;
		//this->_gcodeLoadThread.start(); // in Windows, automatic start
	}

	this->OnViewChange();
#endif
}

void CeditgcodeView::loadSceneFiles(strList* filenames)
{
//		this->youMagineButton.setDisabled(False)
//		#if this->viewSelection.getValue() == 4:
//		#	this->viewSelection.setValue(0)
//		#	this->OnViewChange()
		this->loadScene(filenames);
}

int tstrcmpnocase(TCHAR* s1, TCHAR* s2);

int isGCodeExt(TCHAR* ext)
{
	if(tstrcmpnocase(ext, L".g") == 0)
		return 1;
	if(tstrcmpnocase(ext, L".gcode") == 0)
		return 1;
	if(tstrcmpnocase(ext, L".tap") == 0)
		return 1;

	return 0;
}

void CeditgcodeView::loadFiles(strList* filenames)
{
//		# only one GCODE file can be active
//	# so if single gcode file, process this
//	# otherwise ignore all gcode files

	TCHAR* gcodeFilename = NULL;
	if (filenames->size == 1) {
		TCHAR* filename = filenames->ptrs[0];
		TCHAR* ext = getFileExt(filename);
		if (isGCodeExt(ext))
			gcodeFilename = filename;
			//mainWindow.addToModelMRU(filename)
	}
	if (gcodeFilename != NULL){
		this->loadGCodeFile(gcodeFilename);
	}else{
//		# process directories and special file types
//		# and keep scene files for later processing
		strList* scene_filenames = new strList(__FILE__, __LINE__);
		//# use file list as queue
		//# pop first entry for processing and append new files at end
		while (filenames->size > 0) {
			TCHAR* filename = filenames->pop(0);
			if(isDirectory(filename)) {
			} else {
				TCHAR* ext = getFileExt(filename);
				if(tstrcmpnocase(ext, L".ini") == 0) {
					profile.loadProfile(filename);
//					mainWindow.addToProfileMRU(filename)
				} else {
					if(isMeshLoaderSupportedExtensions(ext) || isImageToMeshSupportedExtensions(ext))
						scene_filenames->add(filename);
					//mainWindow.addToModelMRU(filename)
				}
			}
			free(filename); // you must free popped item on your hand.
		}
//		mainWindow.updateProfileToAllControls();
		//# now process all the scene files
		if (scene_filenames->size > 0){
			this->loadSceneFiles(scene_filenames);
			this->_selectObject(NULL);
			this->sceneUpdated();
			float newZoom = float_max(this->_machineSize.data, 0, 3);
#if GLGUI_ANIMATION
			this->_animView = openglGui.animation(self, this->_viewTarget.copy(), numpy.array([0,0,0], numpy.float32), 0.5)
			this->_animZoom = openglGui.animation(self, this->_zoom, newZoom, 0.5);
#endif
		}
		delete scene_filenames;
	}
}
/*
void CeditgcodeView::showLoadModel(self, button = 1)
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

void CeditgcodeView::showSaveModel(self)
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

void CeditgcodeView::OnPrintButton(self, button)
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
			} elif connectionGroup != NULL){
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

void CeditgcodeView::_openPrintWindowForConnection(self, connection)
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

void CeditgcodeView::showPrintWindow(self)
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

void CeditgcodeView::showSaveGCode(self)
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

void CeditgcodeView::_copyFile(self, fileA, fileB, allowEject = False)
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

void CeditgcodeView::_doEjectSD(self, drive)
{
		if removableStorage.ejectDrive(drive):
			this->notification.message('You can now eject the card.')
		else:
			this->notification.message('Safe remove failed...')
}

void CeditgcodeView::_showSliceLog(self)
{
		dlg = wx.TextEntryDialog(self, _("The slicing engine reported the following"), _("Engine log..."), '\n'.join(this->_slicer.getSliceLog()), wx.TE_MULTILINE | wx.OK | wx.CENTRE)
		dlg.ShowModal();
		dlg.Destroy();
}
*/
void CeditgcodeView::OnToolSelect(int button)
{
#if ENABLE_TOOL_BUTTON
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
#endif
}

void CeditgcodeView::sceneUpdated()
{
	Invalidate();
}

void CeditgcodeView::updateToolButtons()
{
	int hidden = 0;

	if (this->_selectedObj == NULL)
		hidden = True;
	else
		hidden = False;
}

void CeditgcodeView::OnViewChange()
{
	if (this->_gcode != NULL && this->_gcode->layerList != NULL)
		this->layerSelect.SetRange(1, this->_gcode->layerList->len - 1);
	this->_selectObject(NULL);
#if ENABLE_GUI
	this->layerSelect.ShowWindow(this->viewMode == _view_gcode);
#endif
	this->QueueRefresh();
}
/*
void CeditgcodeView::OnRotateReset(self, button)
{
		if this->_selectedObj is NULL:
			return
		this->_selectedObj.resetRotation()
		this->_scene.pushFree(this->_selectedObj)
		this->_selectObject(this->_selectedObj)
		this->sceneUpdated()
}

void CeditgcodeView::OnLayFlat(self, button)
{
		if this->_selectedObj is NULL:
			return
		this->_selectedObj.layFlat()
		this->_scene.pushFree(this->_selectedObj)
		this->_selectObject(this->_selectedObj)
		this->sceneUpdated()
}

void CeditgcodeView::OnScaleReset(self, button)
{
		if this->_selectedObj is NULL:
			return
		this->_selectedObj.resetScale()
		this->_selectObject(this->_selectedObj)
		this->updateProfileToControls()
		this->sceneUpdated()
}

void CeditgcodeView::OnScaleMax(self, button)
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

void CeditgcodeView::OnMirror(int axis)
{
		if this->_selectedObj is NULL:
			return
		this->_selectedObj.mirror(axis)
		this->sceneUpdated()
}

void CeditgcodeView::OnScaleEntry(self, value, axis):
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

void CeditgcodeView::OnScaleEntryMM(self, value, axis):
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
void CeditgcodeView::OnDeleteAll(int e)
{
//	while len(this->_scene.objects()) > 0:
//		this->_deleteObject(this->_scene.objects()[0])
//	this->_animView = openglGui.animation(self, this->_viewTarget.copy(), numpy.array([0,0,0], numpy.float32), 0.5)
}
/*
void CeditgcodeView::OnMultiply(self, e):
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

void CeditgcodeView::OnSplitObject(self, e):
		if this->_focusObj is NULL:
			return
		this->_scene.remove(this->_focusObj)
		for obj in this->_focusObj.split(this->_splitCallback):
			if numpy.max(obj.getSize()) > 2.0:
				this->_scene.add(obj)
		this->_scene.centerAll()
		this->_selectObject(NULL)
		this->sceneUpdated()

void CeditgcodeView::OnCenter(self, e):
		if this->_focusObj is NULL:
			return
		this->_focusObj.setPosition(numpy.array([0.0, 0.0]))
		this->_scene.pushFree(this->_selectedObj)
		newViewPos = numpy.array([this->_focusObj.getPosition()[0], this->_focusObj.getPosition()[1], this->_focusObj.getSize()[2] / 2])
		this->_animView = openglGui.animation(self, this->_viewTarget.copy(), newViewPos, 0.5)
		this->sceneUpdated()

void CeditgcodeView::_splitCallback(self, progress):
		print progress

void CeditgcodeView::OnMergeObjects(self, e):
		if this->_selectedObj is NULL or this->_focusObj is NULL or this->_selectedObj == this->_focusObj:
			if len(this->_scene.objects()) == 2:
				this->_scene.merge(this->_scene.objects()[0], this->_scene.objects()[1])
				this->sceneUpdated()
			return
		this->_scene.merge(this->_selectedObj, this->_focusObj)
		this->sceneUpdated()

void CeditgcodeView::sceneUpdated(self):
		this->_sceneUpdateTimer.Start(500, True)
		this->_slicer.abortSlicer()
		this->_scene.updateSizeOffsets()
		this->QueueRefresh()
*/
void CeditgcodeView::_onRunSlicer(int e)
{
}

void CeditgcodeView::_updateSliceProgress(float progressValue, int ready)
{
	if (!ready) {
		if (this->printButton->getProgressBar() != NULL && progressValue >= 0.0 && abs(this->printButton->getProgressBar() - progressValue) < 0.01)
			return;
	}
	this->printButton->setDisabled(!ready);
	if (progressValue >= 0.0)
		this->printButton->setProgressBar(progressValue);
	else
		this->printButton->setProgressBar(NULL);
#if ENABLE_GCODE
	if (this->_gcode != NULL) {
		delete this->_gcode;
		this->_gcode = NULL;
		for (int li = 0; li < this->_gcodeVBOs->len; li++) {
			GLVBOList* layerVBOlist = this->_gcodeVBOs->data[li];
			for (int vi = 0; vi < layerVBOlist->len; vi++){
				GLVBO* vbo = layerVBOlist->data[vi];
				this->glReleaseList->append(vbo);
			}
		}
		if(this->_gcodeVBOs != NULL)
			delete this->_gcodeVBOs ;
		this->_gcodeVBOs = new GLVBOListList();
	}
#endif
	this->QueueRefresh();
}


void CeditgcodeView::QueueRefresh()
{
	Invalidate();
}

void CeditgcodeView::_loadGCode()
{
#if ENABLE_GCODE
#if GCODE_LOAD_CALLBACK
	this->_gcode->progressCallback = this->_gcodeLoadCallback;
#endif
	this->_gcode->load(this->_gcodeFilename);
	this->_gcodeLoadThread_isAlive = 0;
	Invalidate();
#endif
}
/*
void CeditgcodeView::_gcodeLoadCallback(self, progress):
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
void CeditgcodeView::loadScene(strList* fileList)
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
				if (this->_objectLoadShader != NULL) {
//					obj->_loadAnim = openglGui.animation( 1, 0, 1.5);
				} else
					obj->_loadAnim = NULL;
				this->_scene->add(obj);
				if (!this->_scene->checkPlatform(obj)) {
					this->_scene->centerAll();
				}
				this->_selectObject(obj);
				if (obj->getScale().data[0] < 1.0) {
				//	this->notification.message("Warning: Object scaled down.");
				}
				objList->list[on] = NULL; // to protect from deletion when 'delete objList', set this value to NULL
			}
			delete objList;
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
			if m.vbo != NULL and m.vbo.decRef():
				this->glReleaseList.append(m.vbo)
		import gc
		gc.collect()
		this->sceneUpdated()
*/
void CeditgcodeView::_selectObject(printableObject* obj, int zoom )
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

//if you do not call updateProfileToControls, updateMachineDimensions is not called either.
// then _machinePolygons is not set.
void CeditgcodeView::updateProfileToControls()
{
#if USE_SIMPLEMODE
	int oldSimpleMode = this->_isSimpleMode;
	this->_isSimpleMode = strcmp(profile.getPreference("startMode"), "Simple") == 0;
	if (this->_isSimpleMode != oldSimpleMode) {
		this->_scene.arrangeAll();
		this->sceneUpdated();
	}
#endif
	this->_scene->updateSizeOffsets(True);
	this->_machineSize = GetMachineDimensions();

	profile.getPreferenceColour("model_colour", _objColors[0]);
	profile.getPreferenceColour("model_colour2", this->_objColors[1]);
	profile.getPreferenceColour("model_colour3", this->_objColors[2]);
	profile.getPreferenceColour("model_colour4", this->_objColors[3]);

	this->_scene->updateMachineDimensions();
	this->updateModelSettingsToControls();
}

void CeditgcodeView::updateModelSettingsToControls()
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
			if this->_selectedObj != NULL:
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
*/
#define	state_dragOrClick	1
#define	state_tool	2
#define	state_drag	3
#define	state_dragObject	4

void CeditgcodeView::OnMouseDown(int x, int y, UINT flags)
{
	this->_mouseX = x;
	this->_mouseY = y;
	this->_mouseClick3DPos = this->_mouse3Dpos;
	this->_mouseClickFocus = this->_focusObj;

/*
	if e.ButtonDClick():
		this->_mouseState = 'doubleClick'
	else:
		this->_mouseState = 'dragOrClick'
		*/
	this->_mouseState = state_dragOrClick;
	fpxyz p0, p1;
	this->getMouseRay(this->_mouseX, this->_mouseY, p0, p1);
	p0 -= this->getObjectCenterPos() - this->_viewTarget;
	p1 -= this->getObjectCenterPos() - this->_viewTarget;
//	if (this->tool.OnDragStart(p0, p1))
//		this->_mouseState = state_tool;
	if (this->_mouseState == state_dragOrClick) {
		if (flags & MK_LBUTTON) { // right mouse button down
			if (this->_focusObj != NULL) {
				this->_selectObject(this->_focusObj, False);
				this->QueueRefresh();
			}
		}
	}
}

void CeditgcodeView::OnMouseUp(int x, int y, int flags)
{
//	if e.LeftIsDown() or e.MiddleIsDown() or e.RightIsDown():
//		return
	if (this->_mouseState == state_dragOrClick) {
		if (flags & MK_LBUTTON) // L button
			this->_selectObject(this->_focusObj);
		if (flags & MK_RBUTTON) { // R button
/*				menu = wx.Menu();
				if (this->_focusObj != NULL) {
					this->Bind(wx.EVT_MENU, lambda e: this->_deleteObject(this->_focusObj), menu.Append(-1, _("Delete object")))
					this->Bind(wx.EVT_MENU, this->OnCenter, menu.Append(-1, _("Center on platform")))
					this->Bind(wx.EVT_MENU, this->OnMultiply, menu.Append(-1, _("Multiply object")))
					this->Bind(wx.EVT_MENU, this->OnSplitObject, menu.Append(-1, _("Split object into parts")))
				}
				if ((this->_selectedObj != this->_focusObj and this->_focusObj != NULL and this->_selectedObj != NULL) or len(this->_scene.objects()) == 2) and int(profile.getMachineSetting('extruder_amount')) > 1:
					this->Bind(wx.EVT_MENU, this->OnMergeObjects, menu.Append(-1, _("Dual extrusion merge")))
				if len(this->_scene.objects()) > 0:
					this->Bind(wx.EVT_MENU, this->OnDeleteAll, menu.Append(-1, _("Delete all objects")))
				if menu.MenuItemCount > 0:
					this->PopupMenu(menu)
				menu.Destroy();*/
		}
	} else if (this->_mouseState == state_dragObject && this->_selectedObj != NULL) {
		this->_scene->pushFree(this->_selectedObj);
		this->sceneUpdated();
	} else if (this->_mouseState == 'tool') {
		if (this->tempMatrix != NULL && this->_selectedObj != NULL) {
			this->_selectedObj->applyMatrix(*this->tempMatrix);
			this->_scene->pushFree(this->_selectedObj);
			this->_selectObject(this->_selectedObj);
		}
		if(this->tempMatrix != NULL) {
			delete this->tempMatrix ;
			this->tempMatrix = NULL;
		}
//		this->tool.OnDragEnd();
		this->sceneUpdated();
	}
	this->_mouseState = NULL;
}

void CeditgcodeView::OnMouseMotion(int x, int y, int nflags)
{
//	if(this->viewMode == _view_gcode)
//		return;

	fpxyz p0, p1;
	this->getMouseRay(x, y, p0, p1);

	p0 -= this->getObjectCenterPos() - this->_viewTarget;
	p1 -= this->getObjectCenterPos() - this->_viewTarget;

	if (/*e.Dragging() && */ this->_mouseState != NULL) {
		if (this->_mouseState == state_tool) {
//			this->tool.OnDrag(p0, p1);
		} else if (!(nflags & MK_LBUTTON) && (nflags & MK_RBUTTON)) {
			this->_mouseState = state_drag;
			if (False/*GetAyncKeyState() & wx.GetKeyState(wx.WXK_SHIFT)*/) {
/*
				a = math.cos(math.radians(this->_yaw)) / 3.0
				b = math.sin(math.radians(this->_yaw)) / 3.0
				this->_viewTarget[0] += float(e.GetX() - this->_mouseX) * -a
				this->_viewTarget[1] += float(e.GetX() - this->_mouseX) * b
				this->_viewTarget[0] += float(e.GetY() - this->_mouseY) * b
				this->_viewTarget[1] += float(e.GetY() - this->_mouseY) * a
*/
			} else {
				this->_yaw += x - this->_mouseX;
				this->_pitch -= y - this->_mouseY;
			}
			if (this->_pitch > 170) {
				this->_pitch = 170;
			}
			if (this->_pitch < 10) {
				this->_pitch = 10;
			}
		} else if ((nflags & MK_LBUTTON) && (nflags & MK_RBUTTON) || (nflags & MK_MBUTTON)) {
			this->_mouseState = state_drag;
			this->_zoom += y - this->_mouseY;
			if (this->_zoom < 1) {
				this->_zoom = 1;
			}
			if (this->_zoom > float_max(this->_machineSize.data, 0, 3) * 3) {
				this->_zoom = float_max(this->_machineSize.data, 0, 3) * 3;
			}
		} else if ((nflags & MK_LBUTTON) && this->_selectedObj != NULL && this->_selectedObj == this->_mouseClickFocus) {
			this->_mouseState = state_dragObject;
			float z = max(0, this->_mouseClick3DPos.data[2]);
			fpxyz p0, p1;
			this->getMouseRay(this->_mouseX, this->_mouseY, p0, p1);
			fpxyz p2, p3;
			this->getMouseRay(x, y, p2, p3);
			p0.data[2] -= z;
			p1.data[2] -= z;
			p2.data[2] -= z;
			p3.data[2] -= z;
			fpxyz cursorZ0 = p0 - (p1 - p0) * (p0.data[2] / (p1.data[2] - p0.data[2]));
			fpxyz cursorZ1 = p2 - (p3 - p2) * (p2.data[2] / (p3.data[2] - p2.data[2]));
			fpxyz diff = cursorZ1 - cursorZ0;
			fpxy p = this->_selectedObj->getPosition();
			p.x += diff.data[0];
			p.y += diff.data[1];
			this->_selectedObj->setPosition(p);
		}
	}
/*
	if (!e.Dragging() || this->_mouseState != state_tool) {
		this->tool.OnMouseMove(p0, p1);
	}
*/
	this->_mouseX = x;
	this->_mouseY = y;

	Invalidate();
}
/*
	def OnMouseLeave(self, e):
		#this->_mouseX = -1
		pass
*/

void CeditgcodeView::getMouseRay(int x, int y, fpxyz& p0, fpxyz& p1)
{
	if (this->_viewport[0] == -99999999 && this->_viewport[1] == -99999999) {
		p0.data[0] = p0.data[1] = p0.data[2] = 0;
		p1.data[0] = p1.data[1] = 0; p1.data[2] = 1;
		// return numpy.array([0,0,0],numpy.float32), numpy.array([0,0,1],numpy.float32);
	}

	p0 = unproject(x, this->_viewport[1] + this->_viewport[3] - y, 0, this->_modelMatrix, this->_projMatrix, this->_viewport);
	p1 = unproject(x, this->_viewport[1] + this->_viewport[3] - y, 1, this->_modelMatrix, this->_projMatrix, this->_viewport);
	p0 -= this->_viewTarget;
	p1 -= this->_viewTarget;
}

void CeditgcodeView::_init3DView()
{
//	# set viewing projection
	CSize size = this->GetSize();
	glViewport(0, 0, size.cx, size.cy);
	glLoadIdentity();

	float f4[4] = {0.2, 0.2, 1.0, 0.0};
	glLightfv(GL_LIGHT0, GL_POSITION, f4);

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
	float aspect = float(size.cx) / float(size.cy);
	gluPerspective(45.0, aspect, 1.0, float_max(this->_machineSize.data, 0, 3) * 4);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

#if 0
void CeditgcodeView::OnPaint(int e)
{
}
#else
char* objectVertexSL = "varying float light_amount;\n"
"\n"
"void main(void)\n"
"{\n"
"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
"    gl_FrontColor = gl_Color;\n"
"\n"
"	light_amount = abs(dot(normalize(gl_NormalMatrix * gl_Normal), normalize(gl_LightSource[0].position.xyz)));\n"
"	light_amount += 0.2;\n"
"}\n"
"				";

char* objectFragmentSL = "varying float light_amount;\n"
"\n"
"void main(void)\n"
"{\n"
"	gl_FragColor = vec4(gl_Color.xyz * light_amount, gl_Color[3]);\n"
"}\n";

char* overhangVertexSL = ""
"uniform float cosAngle;\n"
"uniform mat3 rotMatrix;\n"
"varying float light_amount;\n"
"\n"
"void main(void)\n"
"{\n"
"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
"    gl_FrontColor = gl_Color;\n"
"\n"
"	light_amount = abs(dot(normalize(gl_NormalMatrix * gl_Normal), normalize(gl_LightSource[0].position.xyz)));\n"
"	light_amount += 0.2;\n"
"	if (normalize(rotMatrix * gl_Normal).z < -cosAngle)\n"
"	{\n"
"		light_amount = -10.0;\n"
"	}\n"
"}";

char* overhangFragmentSL = ""
"varying float light_amount;\n"
"\n"
"void main(void)\n"
"{\n"
"	if (light_amount == -10.0)\n"
"	{\n"
"		gl_FragColor = vec4(1.0, 0.0, 0.0, gl_Color[3]);\n"
"	}else{\n"
"		gl_FragColor = vec4(gl_Color.xyz * light_amount, gl_Color[3]);\n"
"	}\n"
"}";

char* loadVertexSL = ""
"uniform float intensity;\n"
"uniform float scale;\n"
"varying float light_amount;\n"
"\n"
"void main(void)\n"
"{\n"
"	vec4 tmp = gl_Vertex;\n"
"    tmp.x += sin(tmp.z/5.0+intensity*30.0) * scale * intensity;\n"
"    tmp.y += sin(tmp.z/3.0+intensity*40.0) * scale * intensity;\n"
"    gl_Position = gl_ModelViewProjectionMatrix * tmp;\n"
"    gl_FrontColor = gl_Color;\n"
"\n"
"	light_amount = abs(dot(normalize(gl_NormalMatrix * gl_Normal), normalize(gl_LightSource[0].position.xyz)));\n"
"	light_amount += 0.2;\n"
"}\n";
					
char* loagFragmentSL = "uniform float intensity;\n"
"varying float light_amount;\n"
"\n"
"void main(void)\n"
"{\n"
"	gl_FragColor = vec4(gl_Color.xyz * light_amount, 1.0-intensity);\n"
"}\n";

#define	PI	3.1415926535

double radians(double x)
{
	return x * PI / 180.0;
}

void CeditgcodeView::testShader()
{
	if (this->_objectShader == NULL) {
		if (hasShaderSupport()) {
				this->_objectShader = new GLShader(objectVertexSL, objectFragmentSL);
				this->_objectOverhangShader = new GLShader(overhangVertexSL, overhangFragmentSL);
				this->_objectLoadShader = new GLShader(loadVertexSL, loagFragmentSL);
		}
		if (this->_objectShader == NULL || !this->_objectShader->isValid()){
			this->_objectShader = new GLFakeShader();
			this->_objectOverhangShader = new GLFakeShader();
			this->_objectLoadShader = NULL;
		}
	}
}

void CeditgcodeView::onDrawGCode()
{
	if (this->_gcode != NULL && this->_gcode->layerList != NULL){
		glPushMatrix();
		if( strcmp(profile.getMachineSetting("machine_center_is_zero"), "True") != 0)
			glTranslatef(-this->_machineSize.data[0] / 2, -this->_machineSize.data[1] / 2, 0);
#if ENABLE_GUI
		int drawUpTill = min(this->_gcode->layerList->len, this->layerSelect.GetPos() + 1);
#else
		int drawUpTill = this->_gcode->layerList->len;
#endif
		for (int n = 0; n < drawUpTill; n++) {
			float c = 1.0 - float(drawUpTill - n) / 15;
			c = max(0.3, c);
			if (this->_gcodeVBOs->len < n + 1){
				this->_gcodeVBOs->append(this->_generateGCodeVBOs(this->_gcode->layerList->data[n]));
			}
			// #['WALL-OUTER', 'WALL-INNER', 'FILL', 'SUPPORT', 'SKIRT']
			if (n == drawUpTill - 1) {
				if (this->_gcodeVBOs->data[n]->len < 9) {
					// this->_gcodeVBOs->data[n] += this->_generateGCodeVBOs2(this->_gcode->layerList->data[n]);
					GLVBOList* srcgbl = this->_generateGCodeVBOs2(this->_gcode->layerList->data[n]);
					GLVBOList* dstgbl = this->_gcodeVBOs->data[n];
					for(int x = 0; x < srcgbl->len; x++) {
						dstgbl->append(srcgbl->data[x]);
						srcgbl->data[x] = NULL;
					}
					srcgbl->len = 0;
					delete srcgbl;
				}

				GLVBOList *glvbon = this->_gcodeVBOs->data[n];
				glColor3f(c, 0, 0);
				glvbon->data[8]->render(GL_QUADS);
				glColor3f(c/2, 0, c);
				glvbon->data[9]->render(GL_QUADS);
				glColor3f(0, c, c/2);
				glvbon->data[10]->render(GL_QUADS);
				glColor3f(c, 0, 0);
				glvbon->data[11]->render(GL_QUADS);

				glColor3f(0, c, 0);
				glvbon->data[12]->render(GL_QUADS);
				glColor3f(c/2, c/2, 0.0);
				glvbon->data[13]->render(GL_QUADS);
				glColor3f(0, c, c);
				glvbon->data[14]->render(GL_QUADS);
				glvbon->data[15]->render(GL_QUADS);
				glColor3f(0, 0, c);
				glvbon->data[16]->render(GL_LINES);
			} else {
				GLVBOList *glvbon = this->_gcodeVBOs->data[n];
				glColor3f(c, 0, 0);
				glvbon->data[0]->render(GL_LINES);
				glColor3f(c/2, 0, c);
				glvbon->data[1]->render(GL_LINES);
				glColor3f(0, c, c/2);
				glvbon->data[2]->render(GL_LINES);
				glColor3f(c, 0, 0);
				glvbon->data[3]->render(GL_LINES);

				glColor3f(0, c, 0);
				glvbon->data[4]->render(GL_LINES);
				glColor3f(c/2, c/2, 0.0);
				glvbon->data[5]->render(GL_LINES);
				glColor3f(0, c, c);
				glvbon->data[6]->render(GL_LINES);
				glvbon->data[7]->render(GL_LINES);
			}
		}
		glPopMatrix();
	}
}

void CeditgcodeView::OnPaint(int e)
{
	if (this->_gcodeLoadThread_isAlive)
		return;

/*	connectionGroup = this->_printerConnectionManager.getAvailableGroup()
	if machineCom.machineIsConnected() {
		this->printButton._imageID = 6
		this->printButton._tooltip = _("Print")
	} elif len(removableStorage.getPossibleSDcardDrives()) > 0 and (connectionGroup is NULL or connectionGroup.getPriority() < 0) {
		this->printButton._imageID = 2
		this->printButton._tooltip = _("Toolpath to SD")
	} elif connectionGroup != NULL {
		this->printButton._imageID = connectionGroup.getIconID()
		this->printButton._tooltip = _("Print with %s") % (connectionGroup.getName())
	} else {
		this->printButton._imageID = 3
		this->printButton._tooltip = _("Save toolpath")
	}
	*/
#if GLGUI_ANIMATION
	if (this->_animView != NULL){
		this->_viewTarget = this->_animView.getPosition();
		if (this->_animView.isDone())
			this->_animView = NULL;
	}
	if (this->_animZoom != NULL){
		this->_zoom = this->_animZoom.getPosition();
		if (this->_animZoom.isDone())
			this->_animZoom = NULL;
	}
#endif
#if ENABLE_GCODE
	if ( this->_gcode != NULL){
		if(this->_gcode->layerList != NULL) {
#if	ENABLE_GUI
			PathList *pl = this->_gcode->layerList->data[this->layerSelect.GetPos()];
#else
			PathList *pl = this->_gcode->layerList->data[0];
#endif
			try {
			gcodePath *gp = pl->data[pl->len-1];
			this->_viewTarget.data[2] = gp->points->data[0].data[2];
			}catch(std::exception& e){
				return;
			}
		}
	}
#endif
//dschae	testShader();// prepare shaders

	this->_init3DView();
	glTranslatef(0,0,-this->_zoom);
	glRotatef(-this->_pitch, 1,0,0);
	glRotatef(this->_yaw, 0,0,1);
	glTranslatef(-this->_viewTarget.data[0],-this->_viewTarget.data[1],-this->_viewTarget.data[2]); // move object

	GLint iv[8];//dschae
	glGetIntegerv(GL_VIEWPORT, iv);
	this->_viewport[0] = iv[0];
	this->_viewport[1] = iv[1];
	this->_viewport[2] = iv[2];
	this->_viewport[3] = iv[3];

	glGetDoublev(GL_MODELVIEW_MATRIX, this->_modelMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, this->_projMatrix);

	glClearColor(1,1,1,1);;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if (this->_mouseX > -1){
		try {
			GLuint data[2];
			glFlush();
			glReadPixels(this->_mouseX, this->GetSize().cy - 1 - this->_mouseY, 1, 1, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, data);
			GLuint n = data[0] >> 8;
			if (n < this->_scene->_objectList->length) {
				this->_focusObj = this->_scene->_objectList->data[n];
			} else {
				this->_focusObj = NULL;
			}
			glReadPixels(this->_mouseX, this->GetSize().cy - 1 - this->_mouseY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, data);
			GLuint f = data[0];
	//		#this->GetTopLevelParent().SetTitle(hex(n) + " " + str(f));
			this->_mouse3Dpos = unproject(this->_mouseX, this->_viewport[1] + this->_viewport[3] - this->_mouseY, f, this->_modelMatrix, this->_projMatrix, this->_viewport);
			this->_mouse3Dpos -= this->_viewTarget;
		} catch(std::exception& e) {
			return;
		}
	}

try {
	this->_init3DView();
	glTranslatef(0,0,-this->_zoom);
	glRotatef(-this->_pitch, 1,0,0);
	glRotatef(this->_yaw, 0,0,1);
	glTranslatef(-this->_viewTarget.data[0],-this->_viewTarget.data[1],-this->_viewTarget.data[2]);

	if (1){
		onDrawGCode();
	} else {
		glStencilFunc(GL_ALWAYS, 1, 1);
		glStencilOp(GL_INCR, GL_INCR, GL_INCR);

		this->_objectShader->bind();

		for (int objn = 0; objn < this->_scene->_objectList->length; objn++) {
			printableObject* obj = this->_scene->_objectList->data[objn]; // obj in .objects()){
			if (obj->_loadAnim != NULL){
				if (obj->_loadAnim->isDone())
					obj->_loadAnim = NULL;
				else
					continue;
			}
			float brightness = 1.0;
			if (this->_focusObj == obj)
				brightness = 1.2;
			else if (this->_focusObj != NULL || this->_selectedObj != NULL && obj != this->_selectedObj)
				brightness = 0.8;

			if (this->_selectedObj == obj || this->_selectedObj == NULL){
				//#If we want transparent, then first render a solid black model to remove the printer size lines.
/*				if (this->viewMode == _view_transparent){
					glColor4f(0, 0, 0, 0);
					this->_renderObject(obj);
					glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_ONE);
					glDisable(GL_DEPTH_TEST);
					brightness *= 0.5;
				}
				if (this->viewMode == _view_xray)
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);*/
				glStencilOp(GL_INCR, GL_INCR, GL_INCR);
				glEnable(GL_STENCIL_TEST);
			}
/*
			if (this->viewMode == _view_overhang){
				if (this->_selectedObj == obj && this->tempMatrix != NULL) {
					vertexmat* mat = *obj->getMatrix() * *this->tempMatrix;
					this->_objectOverhangShader->setUniform("rotMatrix", (float*)mat->data);
					delete mat;
				} else {
					this->_objectOverhangShader->setUniform("rotMatrix", (float*)obj->getMatrix()->data);
				}
			}
*/
			if (! this->_scene->checkPlatform(obj)){
				glColor4f(0.5 * brightness, 0.5 * brightness, 0.5 * brightness, 0.8 * brightness);
				this->_renderObject(obj);
			} else {
				this->_renderObject(obj, brightness);
			}
			glDisable(GL_STENCIL_TEST);
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}
/*
		if (this->viewMode == _view_xray){
			glPushMatrix();
			glLoadIdentity();
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glDisable(GL_DEPTH_TEST);
			for (int i = 2; i < 15; i += 2) { // in xrange(2, 15, 2)){
				glStencilFunc(GL_EQUAL, i, 0xFF);
				glColor3f(float(i)/10, float(i)/10, float(i)/5);
				glBegin(GL_QUADS);
				glVertex3f(-1000,-1000,-10);
				glVertex3f( 1000,-1000,-10);
				glVertex3f( 1000, 1000,-10);
				glVertex3f(-1000, 1000,-10);
				glEnd();
			}
			for (int i = 1; i < 15; i += 2) { // i in xrange(1, 15, 2)){
				glStencilFunc(GL_EQUAL, i, 0xFF);
				glColor3f(float(i)/10, 0, 0);
				glBegin(GL_QUADS);
				glVertex3f(-1000,-1000,-10);
				glVertex3f( 1000,-1000,-10);
				glVertex3f( 1000, 1000,-10);
				glVertex3f(-1000, 1000,-10);
				glEnd();
			}
			glPopMatrix();
			glDisable(GL_STENCIL_TEST);
			glEnable(GL_DEPTH_TEST);
		}
*/
		this->_objectShader->unbind();

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		if (this->_objectLoadShader != NULL){
			this->_objectLoadShader->bind();
			glColor4f(0.2, 0.6, 1.0, 1.0);
			for(int obn = 0; obn < this->_scene->_objectList->length; obn++) { // for (obj in this->_scene->objects()){
				printableObject* obj = this->_scene->_objectList->data[obn];
				if (obj->_loadAnim == NULL)
					continue;
				this->_objectLoadShader->setUniform("intensity", obj->_loadAnim->getPosition());
				this->_objectLoadShader->setUniform("scale", obj->getBoundaryCircle() / 10);
				this->_renderObject(obj);
			}
			this->_objectLoadShader->unbind();
			glDisable(GL_BLEND);
		}
	}

	this->_drawMachine();
/*
	if (this->_usbPrintMonitor.getState() == "PRINTING" && this->_usbPrintMonitor.getID() == this->_slicer.getID()){
		float z = this->_usbPrintMonitor.getZ();
		if (strcmp(this->viewMode , "gcode") == 0){
#if ENABLE_GCODE
			int layer = 0;
			float layer_height = profile.getProfileSettingFloat("layer_height");
			float layer1_height = profile.getProfileSettingFloat("bottom_thickness");
			if (layer_height > 0){
				if (layer1_height > 0)
					layer = int((z - layer1_height) / layer_height) + 1;
				else
					layer = int(z / layer_height);
			} else {
				layer = 1;
			}
			this->layerSelect->setValue(layer);
#endif
		} else {
			fpxyz size = this->_machineSize;
			glEnable(GL_BLEND);
			glColor4ub(255,255,0,128);
			glBegin(GL_QUADS);
			glVertex3f(-size.data[0]/2,-size.data[1]/2, z);
			glVertex3f( size.data[0]/2,-size.data[1]/2, z);
			glVertex3f( size.data[0]/2, size.data[1]/2, z);
			glVertex3f(-size.data[0]/2, size.data[1]/2, z);
			glEnd();
		}
	}
*/
	if (1/*this->viewMode == _view_gcode*/) {
#if ENABLE_GCODE
		if (this->_gcodeLoadThread != NULL && this->_gcodeLoadThread_isAlive) {
			glDisable(GL_DEPTH_TEST);
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(0,-4,-10);
			glColor4ub(60,60,60,255);
			glDrawStringCenter(_T("Loading toolpath for visualization..."));
			glPopMatrix();
		}
#endif
	} else {
		//#Draw the object box-shadow, so you can see where it will collide with other objects.
		if (this->_selectedObj != NULL) {
			glEnable(GL_BLEND);
			glEnable(GL_CULL_FACE);
			glColor4f(0,0,0,0.16);
			glDepthMask(False);
			for (int objn = 0; objn < this->_scene->_objectList->length; objn++) {
				printableObject* obj = this->_scene->_objectList->data[objn];
				glPushMatrix();
				glTranslatef(obj->getPosition().x, obj->getPosition().y, 0);
				glBegin(GL_TRIANGLE_FAN);
				for (int pn = obj->_boundaryHull->size; pn > 0; pn--) {
					fpxy p = obj->_boundaryHull->data[pn-1];
					glVertex3f(p.x, p.y, 0);
				}
				glEnd();
				glPopMatrix();
			}
			if (this->_scene->isOneAtATime()){
				glPushMatrix();
				glColor4f(0,0,0,0.06);
				glTranslatef(this->_selectedObj->getPosition().x, this->_selectedObj->getPosition().y, 0);
				glBegin(GL_TRIANGLE_FAN);
				for (int pn = this->_selectedObj->_printAreaHull->size; pn > 0; pn--) {
					fpxy p = this->_selectedObj->_printAreaHull->data[pn-1];
					glVertex3f(p.x, p.y, 0);
				}
				glEnd();
				glBegin(GL_TRIANGLE_FAN);
				for (int pn = this->_selectedObj->_headAreaMinHull->size; pn > 0; pn--) {
					fpxy p = this->_selectedObj->_headAreaMinHull->data[pn-1];
					glVertex3f(p.x, p.y, 0);
				}
				glEnd();
				glPopMatrix();
			}
			glDepthMask(True);
			glDisable(GL_CULL_FACE);
		}
		//#Draw the outline of the selected object, on top of everything else except the GUI.
		if (this->_selectedObj != NULL && this->_selectedObj->_loadAnim == NULL) {
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glEnable(GL_STENCIL_TEST);
			glDisable(GL_BLEND);
			glStencilFunc(GL_EQUAL, 0, 255);

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(2);
			glColor4f(1,1,1,0.5);
			this->_renderObject(this->_selectedObj);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glViewport(0, 0, this->GetSize().cx, this->GetSize().cy);
			glDisable(GL_STENCIL_TEST);
			glDisable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
		}
		if (this->_selectedObj != NULL) {
			glPushMatrix();
			fpxyz pos = this->getObjectCenterPos();
			glTranslatef(pos.data[0], pos.data[1], pos.data[2]);
#if ENABLE_TOOL_BUTTON
			this->tool.OnDraw();
#endif
			glPopMatrix();
		}
	}
/*
	if (this->viewMode == _view_overhang && !hasShaderSupport()){
		glDisable(GL_DEPTH_TEST);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(0,-4,-10);
		glColor4ub(60,60,60,255);
		glDrawStringCenter(_T("Overhang view not working due to lack of OpenGL shaders support."));
		glPopMatrix();
	}
*/
}catch(std::exception& e){
	return;
}
}
#endif

void CeditgcodeView::_renderObject(printableObject* obj, int brightness, int addSink)
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
	float fz = obj->getSize()[2];
	glTranslatef(-offset.data[0], -offset.data[1], -offset.data[2] - fz / 2);

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
				fcolors[fi] = _objColors[n][fi] * brightness;
			glColor4fv(fcolors);
			n += 1;
		}
		m->vbo->render();
	}
	glPopMatrix();
}

void CeditgcodeView::_drawMachine()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	fpxyz size = this->_machineSize;

	char* machine = profile.getMachineSetting("machine_type");

	if (False/*machine.startswith('ultimaker')*/){
/*
		if (machine not in this->_platformMesh){
			meshes = meshLoader.loadMeshes(resources.getPathForMesh(machine + '_platform.stl'))
			if (len(meshes) > 0){
				this->_platformMesh[machine] = meshes[0];
			} else {
				this->_platformMesh[machine] = NULL;
			}
			if (machine == 'ultimaker2'){
				this->_platformMesh[machine]._drawOffset = numpy.array([0,-37,145], numpy.float32);
			} else {
				this->_platformMesh[machine]._drawOffset = numpy.array([0,0,2.5], numpy.float32);
			}
		}
		glColor4f(1,1,1,0.5);
		this->_objectShader->bind();
		this->_renderObject(this->_platformMesh[machine], False, False);
		this->_objectShader.unbind();
*/
		//#For the Ultimaker 2 render the texture on the back plate to show the Ultimaker2 text.
		if (False/*machine == 'ultimaker2'*/){
/*
			if not hasattr(this->_platformMesh[machine], 'texture')){
				this->_platformMesh[machine].texture = opengl.loadGLTexture('Ultimaker2backplate.png');
			}
			glBindTexture(GL_TEXTURE_2D, this->_platformMesh[machine].texture);
			glEnable(GL_TEXTURE_2D);
			glPushMatrix();
			glColor4f(1,1,1,1);
*/
			glTranslatef(0,150,-5);
			float h = 50;
			float d = 8;
			float w = 100;
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_COLOR, GL_ZERO);
			glBegin(GL_QUADS);
			glTexCoord2f(1, 0);
			glVertex3f( w, 0, h);
			glTexCoord2f(0, 0);
			glVertex3f(-w, 0, h);
			glTexCoord2f(0, 1);
			glVertex3f(-w, 0, 0);
			glTexCoord2f(1, 1);
			glVertex3f( w, 0, 0);

			glTexCoord2f(1, 0);
			glVertex3f(-w, d, h);
			glTexCoord2f(0, 0);
			glVertex3f( w, d, h);
			glTexCoord2f(0, 1);
			glVertex3f( w, d, 0);
			glTexCoord2f(1, 1);
			glVertex3f(-w, d, 0);
			glEnd();
			glDisable(GL_TEXTURE_2D);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glPopMatrix();
		}
	} else {
		// Draw X,Y,Z line axis
		glColor4f(0,0,0,1);
		glLineWidth(3);
		glBegin(GL_LINES);
		glVertex3f(-size.data[0] / 2, -size.data[1] / 2, 0);
		glVertex3f(-size.data[0] / 2, -size.data[1] / 2, 10);
		glVertex3f(-size.data[0] / 2, -size.data[1] / 2, 0);
		glVertex3f(-size.data[0] / 2+10, -size.data[1] / 2, 0);
		glVertex3f(-size.data[0] / 2, -size.data[1] / 2, 0);
		glVertex3f(-size.data[0] / 2, -size.data[1] / 2+10, 0);
		glEnd();
	}
	glDepthMask(False);

	fpxyvectvect* polys = profile.getMachineSizePolygons();
	float height = this->_machineSize.data[2]; // profile.getMachineSettingFloat("machine_height");

	glBegin(GL_QUADS);
	fpxyvect* poly0 = polys->data[0];
	// GL_QUADS will show faces given by CCW direction.
	// 0,1 will not be drawn, as CCW (2,3) only be shown by OpenGL
	for (int n = 0; n < poly0->size; n++){
		if (n % 2 == 0){
			glColor4ub(5, 171, 231, 96);
		} else {
			glColor4ub(5, 171, 231, 64);
		}
		
		int n1 = n -1;
		if(n1 < 0) n1 = poly0->size - 1;
		glVertex3f(poly0->data[n].x, poly0->data[n].y, height);
		glVertex3f(poly0->data[n].x, poly0->data[n].y, 0);
		glVertex3f(poly0->data[n1].x, poly0->data[n1].y, 0);
		glVertex3f(poly0->data[n1].x, poly0->data[n1].y, height);
/*		glVertex3f(polys[0][n][0], polys[0][n][1], height);
		glVertex3f(polys[0][n][0], polys[0][n][1], 0);
		glVertex3f(polys[0][n-1][0], polys[0][n-1][1], 0);
		glVertex3f(polys[0][n-1][0], polys[0][n-1][1], height);*/
	}
	glEnd();

	glColor4ub(5, 171, 231, 128);
	glBegin(GL_TRIANGLE_FAN);
	for (int pn = 0; pn < poly0->size; pn++) { // for p in polys[0][::-1]){
		fpxy p = poly0->data[poly0->size-1 - pn];
		glVertex3f(p.x, p.y, height);
	}
	glEnd();

	//	#Draw checkerboard
	if (this->_platformTexture == 0){
		this->_platformTexture = loadGLTexture("checkerboard.bmp");
		glBindTexture(GL_TEXTURE_2D, this->_platformTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	glColor4f(1,1,1,0.5);
	glBindTexture(GL_TEXTURE_2D, this->_platformTexture);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLE_FAN);
	for (int pn = 0; pn < poly0->size; pn++) { // p in polys[0]){
		fpxy p = poly0->data[pn];
		glTexCoord2f(p.x/20, p.y/20);
		glVertex3f(p.x, p.y, 0);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glColor4ub(127, 127, 127, 200);
	for (int polyn = 1; polyn < polys->len; polyn++) { // poly in polys[1:]){
		fpxyvect* poly = polys->data[polyn];
		glBegin(GL_TRIANGLE_FAN);
		for(int pn = 0; pn < poly->size; pn++) { // for (p in poly) {
			fpxy p = poly->data[pn];
			glTexCoord2f(p.x/20, p.y/20);
			glVertex3f(p.x, p.y, 0);
		}
		glEnd();
	}
	delete polys;

	glDepthMask(True);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}

#if ENABLE_GCODE
GLVBOList* CeditgcodeView::_generateGCodeVBOs(PathList* layer)
{
	GLVBOList* ret = new GLVBOList();
//	for extrudeType in ['WALL-OUTER:0', 'WALL-OUTER:1', 'WALL-OUTER:2', 'WALL-OUTER:3', 'WALL-INNER', 'FILL', 'SUPPORT', 'SKIRT']:
	char *extstr[] = { "WALL-OUTER:0", "WALL-OUTER:1", "WALL-OUTER:2", "WALL-OUTER:3", "WALL-INNER", "FILL", "SUPPORT", "SKIRT", NULL};
	vint extruder;
	char* extrudeType = NULL;
	int sized = 0;

	for(int ei = 0; extstr[ei]; ei++) {
		char* str = extstr[ei];
		char* cp = strchr( str, ':');
		if(cp) {
			extruder.ivalue = atoi(cp+1);
			extruder.valid = 1;
		} else {
			extruder.valid = 0;
		}
		extrudeType = getPathType(str);
		vertexmat* pointList = new vertexmat(__FILE__, __LINE__); // numpy.zeros((0,3), numpy.float32)
		for (int pi = 0; pi < layer->len; pi++) { // path in 
			gcodePath* path = layer->data[pi];
			if (path->type == _extrude && path->pathType == extrudeType && (!extruder.valid || path->extruder == extruder.ivalue)) {
				/*
				a = path->points;
				a = numpy.concatenate((a[:-1], a[1:]), 1);
				a = a.reshape((len(a) * 2, 3));
				pointList = numpy.concatenate((pointList, a));
				*/
				for(int i = 0; i < path->points->size-1; i++) {
					pointList->append(path->points->data[i].data);
					pointList->append(path->points->data[i+1].data);
				}
			}
		}
		GLVBO* vbo = new GLVBO(pointList, NULL, TRUE, __FILE__, __LINE__);
		if(vbo->_size > 0) {
			ret->append(vbo);
			sized = 1;
		} else {
			ret->append(vbo);
		}
	}
	if(sized)
		return ret;
	else
		return ret;
}
#endif

void dumyfunc(int x)
{
}

#if ENABLE_GCODE
GLVBOList* CeditgcodeView::_generateGCodeVBOs2(PathList* layer)
{
	float filamentRadius = profile.getProfileSettingFloat("filament_diameter") / 2;
	float filamentArea = MATH_PI * filamentRadius * filamentRadius;
	int useFilamentArea = strcmp(profile.getMachineSetting("gcode_flavor"), "UltiGCode") == 0;

	GLVBOList* ret = new GLVBOList();
//	for extrudeType in ['WALL-OUTER:0', 'WALL-OUTER:1', 'WALL-OUTER:2', 'WALL-OUTER:3', 'WALL-INNER', 'FILL', 'SUPPORT', 'SKIRT']:
	char* extarray[] = {"WALL-OUTER:0", "WALL-OUTER:1", "WALL-OUTER:2", "WALL-OUTER:3", "WALL-INNER", "FILL", "SUPPORT", "SKIRT", NULL};
	vint extruder;
	char* extrudeType = NULL;
	for(int ei = 0; extarray[ei] != NULL; ei++) {
		char* str = extarray[ei];
		char* cp = strchr(str, ':');
		if (cp) {
			extruder.ivalue = atoi(cp+1);
			extruder.valid = 1;
		} else {
			extruder.valid = 0;
		}
		extrudeType = getPathType(str);
		vertexmat* pointList = new vertexmat(__FILE__, __LINE__); // numpy.zeros((0,3), numpy.float32)
		for (int pi = 0; pi < layer->len; pi++) { // path in
			gcodePath* path = layer->data[pi];
			if (path->type == _extrude && path->pathType == extrudeType && (!extruder.valid || path->extruder == extruder.ivalue)){
				//dschae-------
				if(path->extrusion->len != path->points->size) {
					continue;
				}
				//--------dschae

				vertexmat* a = path->points->copy(__FILE__, __LINE__);
				if (extrudeType == _FILL) {
					// a[:,2] += 0.01;
					for(int i = 0; i < a->size; i++)
						a->data[i].data[2] += 0.01;
				}

				// normal = a[1:] - a[:-1];
				vertexmat* normal = new vertexmat(a->size-1, __FILE__, __LINE__);
				for(int i = 0; i < a->size-1; i++)
					normal->data[i] = a->data[i+1] - a->data[i];

				// lens = numpy.sqrt(normal[:,0]**2 + normal[:,1]**2);
				// normal[:,0], normal[:,1] = -normal[:,1] / lens, normal[:,0] / lens;
				// normal[:,2] /= lens;
				float * lens = (float*)malloc(normal->size * sizeof(float));
				for(int i = 0; i < normal->size; i++) {
					lens[i] = sqrt(normal->data[i].data[0]*normal->data[i].data[0] + normal->data[i].data[1]*normal->data[i].data[1]);
					float temp = normal->data[i].data[0];
					normal->data[i].data[0] = -normal->data[i].data[1] / lens[i];
					normal->data[i].data[1] = temp / lens[i];
					normal->data[i].data[2] /= lens[i];
				}

				//ePerDist = path->extrusion[1:] / lens;
				int eperdistsize = path->extrusion->len-1;
				float* ePerDist = (float*)malloc(eperdistsize * sizeof(float));
				for(int i = 0; i < path->extrusion->len-1; i++) {
					ePerDist[i] = path->extrusion->data[i+1] / lens[i];
				}
				//if (useFilamentArea)
				//	lineWidth = ePerDist / path->layerThickness / 2.0;
				//else:
				//	lineWidth = ePerDist * (filamentArea / path->layerThickness / 2);
				float* lineWidth = (float*)malloc(eperdistsize * sizeof(float));
				for(int i = 0; i < eperdistsize; i++) {
					if (useFilamentArea)
						lineWidth[i] = ePerDist[i] / path->layerThickness / 2.0;
					else
						lineWidth[i] = ePerDist[i] * (filamentArea / path->layerThickness / 2);
				}
				//
				//normal[:,0] *= lineWidth;
				//normal[:,1] *= lineWidth;
				for(int i = 0; i < normal->size; i++) {
					normal->data[i].data[0] *= lineWidth[i];
					normal->data[i].data[1] *= lineWidth[i];
				}
				free(lens); lens = (float*)1;
				free(ePerDist); ePerDist = (float*)3;

				//b = numpy.zeros((len(a)-1, 0), numpy.float32);
				//b = numpy.concatenate((b, a[1:] + normal), 1);
				//b = numpy.concatenate((b, a[1:] - normal), 1);
				//b = numpy.concatenate((b, a[:-1] - normal), 1);
				//b = numpy.concatenate((b, a[:-1] + normal), 1);
				//b = b.reshape((len(b) * 4, 3));
				vertexmat* b = new vertexmat(__FILE__, __LINE__);
				for(int i = 0; i < normal->size; i++) {
					fpxyz f = a->data[i+1] + normal->data[i];//b = numpy.concatenate((b, a[1:] + normal), 1);
					b->append(f.data);
				}
				for(int i = 0; i < normal->size; i++) {
					fpxyz f = a->data[i+1] - normal->data[i];//b = numpy.concatenate((b, a[1:] - normal), 1);
					b->append(f.data);
				}
				for(int i = 0; i < normal->size; i++) {
					fpxyz f = a->data[i] - normal->data[i];//b = numpy.concatenate((b, a[:-1] - normal), 1);
					b->append(f.data);
				}
				for(int i = 0; i < normal->size; i++) {
					fpxyz f = a->data[i] + normal->data[i];//b = numpy.concatenate((b, a[:-1] + normal), 1);
					b->append(f.data);
				}
				//b = b.reshape((len(b) * 4, 3));

				if (a->size > 2) {
					// normal2 = normal[:-1] + normal[1:];
					vertexmat* normal2 = new vertexmat(normal->size-1, __FILE__, __LINE__);
					for(int i = 0; i < normal->size-1; i++)
						normal2->data[i] = normal->data[i] + normal->data[i+1];
					// lens2 = numpy.sqrt(normal2[:,0]**2 + normal2[:,1]**2);
					float *lens2 = (float*)malloc(normal2->size * sizeof(float));
					for(int i = 0; i < normal2->size; i++) {
						lens2[i] = sqrt(normal2->data[i].data[0] * normal2->data[i].data[0] + normal2->data[i].data[1] * normal2->data[i].data[1]);
						normal2->data[i].data[0] /= lens2[i]; // normal2[:,0] /= lens2;
						normal2->data[i].data[1] /= lens2[i]; // normal2[:,1] /= lens2;
						normal2->data[i].data[0] *= lineWidth[i]; // normal2[:,0] *= lineWidth[:-1];
						normal2->data[i].data[1] *= lineWidth[i]; // normal2[:,1] *= lineWidth[:-1];
					}
					free(lens2); lens2 = (float*)5;
					//c = numpy.zeros((len(a)-2, 0), numpy.float32);
					vertexmat* c = new vertexmat(__FILE__, __LINE__);
					//c = numpy.concatenate((c, a[1:-1]), 1);
					for(int i = 0; i < a->size-2; i++)
						c->append(a->data[i+1].data);
					//c = numpy.concatenate((c, a[1:-1]+normal[1:]), 1);
					for(int i = 0; i < a->size-2; i++) {
						fpxyz f = a->data[i+1] + normal->data[i+1];
						c->append(f.data);
					}
					//c = numpy.concatenate((c, a[1:-1]+normal2), 1);
					for(int i = 0; i < a->size-2; i++) {
						fpxyz f = a->data[i+1] + normal2->data[i];
						c->append(f.data);
					}
					//c = numpy.concatenate((c, a[1:-1]+normal[:-1]), 1);
					for(int i = 0; i < a->size-2; i++) {
						fpxyz f = a->data[i+1] + normal->data[i];
						c->append(f.data);
					}

					//c = numpy.concatenate((c, a[1:-1]), 1);
					for(int i = 0; i < a->size-2; i++)
						c->append(a->data[i+1].data);
					//c = numpy.concatenate((c, a[1:-1]-normal[1:]), 1);
					for(int i = 0; i < a->size-2; i++) {
						fpxyz f = a->data[i+1] - normal->data[i];
						c->append(f.data);
					}
					//c = numpy.concatenate((c, a[1:-1]-normal2), 1);
					for(int i = 0; i < a->size-2; i++) {
						fpxyz f = a->data[i+1] - normal2->data[i];
						c->append(f.data);
					}
					//c = numpy.concatenate((c, a[1:-1]-normal[:-1]), 1);
					for(int i = 0; i < a->size-2; i++) {
						fpxyz f = a->data[i+1] - normal->data[i];
						c->append(f.data);
					}
					delete normal2;
					delete normal;
					//c = c.reshape((len(c) * 8, 3));

					//pointList = numpy.concatenate((pointList, b, c));
					pointList->append(b);
					pointList->append(c);
					delete c;
				} else {
					pointList->append(b);
				}
				delete b;
				delete a;
				free(lineWidth); lineWidth = (float*)2;
			}
		}

		ret->append(new GLVBO(pointList, NULL, TRUE, __FILE__, __LINE__));
	}

	vertexmat* pointList = new vertexmat(__FILE__, __LINE__); // numpy.zeros((0,3), numpy.float32);
	for (int pi = 0; pi < layer->len; pi++) { // path in layer) {
		gcodePath *path = layer->data[pi];
		if (path->type == _move) {
			// a = path['points'] + numpy.array([0,0,0.01], numpy.float32);
			vertexmat *a = path->points->copy(__FILE__, __LINE__);
			for(int i = 0; i < a->size; i++) a->data[i].data[2] += 0.01;
			// a = numpy.concatenate((a[:-1], a[1:]), 1);
			vertexmat *b = new vertexmat(__FILE__, __LINE__);
			for(int i = 0; i < a->size-1; i++)
				b->append(a->data[i].data);
			for(int i = 1; i < a->size; i++)
				b->append(a->data[i].data);
			// a = a.reshape((len(a) * 2, 3));
			delete a;
			pointList->append(b); // = numpy.concatenate((pointList, a));
			delete b;
		}
		if (path->type == _retract) {
			//a = path['points'] + numpy.array([0,0,0.01], numpy.float32);
			vertexmat* a = path->points->copy(__FILE__, __LINE__);
			for(int i = 0; i < a->size; i++)
				a->data[i].data[2] += 0.01;
			//a = numpy.concatenate((a[:-1], a[1:] + numpy.array([0,0,1], numpy.float32)), 1);
			vertexmat *b = new vertexmat(__FILE__, __LINE__);
			for(int i = 0; i < a->size-1; i++)
				b->append(a->data[i].data);
			for(int i = 1; i < a->size; i++) {
				fpxyz f = a->data[i];
				f.data[2] += 1;
				b->append(f.data);
			}
			delete a;
			// a = a.reshape((len(a) * 2, 3));
			pointList->append(b); //  = numpy.concatenate((pointList, a));
			delete b;
		}
	}
	ret->append(new GLVBO(pointList, NULL, TRUE, __FILE__, __LINE__));

	return ret;
}
#endif

fpxyz CeditgcodeView::getObjectCenterPos()
{
	if (this->_selectedObj == NULL) {
		fpxyz fp(0,0,0);
		return fp;
	}

	fpxy pos = this->_selectedObj->getPosition();
	fpxyz size = this->_selectedObj->getSize();

//	return [pos[0], pos[1], size[2]/2 - profile.getProfileSettingFloat('object_sink')]
	fpxyz fp;
	fp.data[0] = pos.x;
	fp.data[1] = pos.y;
	fp.data[2] = size.data[2]/2 - profile.getProfileSettingFloat("object_sink");

	return fp;
}
/*
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

CSize CeditgcodeView::GetSize()
{
	CSize size;
	RECT rect;
	GetClientRect(&rect);
	size.cx = rect.right - rect.left;
	size.cy = rect.bottom - rect.top;

	return size;
}


BOOL CeditgcodeView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	float delta = (float)zDelta / (float)1;
	delta = max(min(delta,4),-4);
	this->_zoom *= 1.0 - delta / 10.0;
	if (this->_zoom < 1.0)
		this->_zoom = 1.0;
	if (this->_zoom > float_max(this->_machineSize.data, 0, 3) * 3)
		this->_zoom = float_max(this->_machineSize.data, 0, 3) * 3;
	Invalidate(); // this->Refresh();
	
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CeditgcodeView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	this->OnMouseMotion(point.x, point.y, nFlags);

	CView::OnMouseMove(nFlags, point);
}

