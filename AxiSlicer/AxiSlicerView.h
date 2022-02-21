
// AxiSlicerView.h : CAxiSlicerView 클래스의 인터페이스
//

#include "opencv2/opencv.hpp"
//#include <cv.h>
#include <opencv/highgui.h>
#include "CvvImage.h"

#pragma once

#define	ID_ONTOOL_ROTATE	1001
#define	ID_ONTOOL_SCALE		1002
#define	ID_ONTOOL_MIRROR	1003

#define	ID_ONROTATE_RESET	1011
#define	ID_ONLAY_FLAT		1012

#define	ID_ONSCALE_RESET	1013
#define	ID_ONSCALE_MAX		1014

#define	ID_ONMIRROR_0		1021
#define	ID_ONMIRROR_1		1022
#define	ID_ONMIRROR_2		1023

#define	_view_0		0
#define	_view_gcode	1
#define	_view_overhang	2
#define	_view_transparent	3
#define	_view_xray	4
#define	_view_normal	5
#define	_view_scanner	6

#define	LINEBUFSIZE	256

class CAxiSlicerView : public CView
{
protected: // serialization에서만 만들어집니다.
	CAxiSlicerView();
	DECLARE_DYNCREATE(CAxiSlicerView)

// 특성입니다.
public:
	CAxiSlicerDoc* GetDocument() const;

// 작업입니다.
public:
	HGLRC m_hRC;	//렌더링 컨텍스트
	HDC m_hDC;		//GDI장치 컨텍스트

	void GLResize(int cx, int cy);
	void GLRenderScene(void);

	float _yaw;// = 30
	float _pitch ;
#if GLGUI_ANIMATION
	fpxyz _zoom ;
#else
	float _zoom ;
#endif
	Scene* _scene ;
#if ENABLE_GCODE
	gcode* _gcode ;
	GLVBOListList* _gcodeVBOs ;
	TCHAR _gcodeFilename[MAX_PATH];
	float _gcodeLoadThread ;
	int _gcodeLoadThread_isAlive;
	GLVBOList* glReleaseList;
#endif

	GLFakeShader* _objectShader ;
	GLFakeShader* _objectLoadShader ;
	GLFakeShader* _objectOverhangShader;

	printableObject* _focusObj ;
	printableObject* _selectedObj ;
	float _objColors[4][4];

	int _mouseX ;
	int _mouseY ;
	int   _mouseState ;

	fpxyz _viewTarget;
	fpxyz _mouse3Dpos;
#if GLGUI_ANIMATION
	animation* _animView;
	animation* _animZoom;
#endif
	float _platformMesh;
	float _platformTexture;
#if USE_SIMPLEMODE
	int   _isSimpleMode;
#endif
	float _usbPrintMonitor;
	float _printerConnectionManager;

	int viewMode;
	GLdouble _viewport[4];
	GLdouble _modelMatrix[16];
	GLdouble _projMatrix[16];
	vertexmat* tempMatrix;

	float scaleForm;

	Slicer *_slicer;

	fpxyz _machineSize;

	glButton* openFileButton;
	glButton* printButton;

	glButton* rotateToolButton;
	glButton* scaleToolButton;
	glButton* mirrorToolButton;

	glButton* resetRotationButton;
	glButton* layFlatButton;

	glButton* resetScaleButton;
	glButton* scaleMaxButton;

	glButton* mirrorXButton;
	glButton* mirrorYButton;
	glButton* mirrorZButton;

	glComboButton *viewSelection;

	CSliderCtrl	layerSelect;

	fpxyz _mouseClick3DPos;// = this->_mouse3Dpos;
	printableObject* _mouseClickFocus; // = this->_focusObj;

	int linelen;
	char linebuf[LINEBUFSIZE];

//----------- openCV variance begin ---------------------------
	IplImage* frame_copy;

	IplImage		*image_0;			// 관련 변수들 
	IplImage		*image_1;			// 관련 변수들 

	IplImage		*raw_image;			// 관련 변수들 
	CvCapture		*capture;		//
	CvvImage		cImage;			//

	CDC				scannerDC;
	CBitmap			scannerBitmap;

	void StartScanner();
	void OnStop();
	void InitialiseCamera();
	void DoTest();
//----------- openCV variance end -----------------------------

	void testShader();
	void QueueRefresh();
	void onDrawGCode();

	CSize GetSize();
	void loadGCodeFile(TCHAR* filename);
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

	void OnMouseDown(int x, int y, UINT flags);
	void OnMouseUp(int x, int y, int flags);
	void OnMouseMotion(int x, int y, int nflags);

	void OnMouseWheel(int e);
	void OnMouseLeave(int e);
	void getMouseRay(int x, int y, fpxyz& p0, fpxyz& p1);
	void _init3DView();
	void OnPaint(int e);
	void OnRepaint();

	void _renderObject(printableObject* obj, int brightness = False, int addSink = True);
	void _drawMachine();
	GLVBOList* _generateGCodeVBOs(PathList* pl);
	GLVBOList* _generateGCodeVBOs2(PathList* pl);
	fpxyz getObjectCenterPos();
	void getObjectBoundaryCircle();
	void getObjectSize();
	void getObjectMatrix();

// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 구현입니다.
public:
	virtual ~CAxiSlicerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	virtual void OnInitialUpdate();
};

#ifndef _DEBUG  // AxiSlicerView.cpp의 디버그 버전
inline CAxiSlicerDoc* CAxiSlicerView::GetDocument() const
   { return reinterpret_cast<CAxiSlicerDoc*>(m_pDocument); }
#endif

