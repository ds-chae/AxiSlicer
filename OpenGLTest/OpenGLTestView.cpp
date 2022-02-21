
// OpenGLTestView.cpp : COpenGLTestView Ŭ������ ����
//

#include "stdafx.h"
// SHARED_HANDLERS�� �̸� ����, ����� �׸� �� �˻� ���� ó���⸦ �����ϴ� ATL ������Ʈ���� ������ �� ������
// �ش� ������Ʈ�� ���� �ڵ带 �����ϵ��� �� �ݴϴ�.
#ifndef SHARED_HANDLERS
#include "OpenGLTest.h"
#endif

#include "OpenGLTestDoc.h"
#include "OpenGLTestView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COpenGLTestView

IMPLEMENT_DYNCREATE(COpenGLTestView, CView)

BEGIN_MESSAGE_MAP(COpenGLTestView, CView)
	// ǥ�� �μ� ����Դϴ�.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// COpenGLTestView ����/�Ҹ�

COpenGLTestView::COpenGLTestView()
{
	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	m_fPosX = 0.0f;    // X position of model in camera view
	m_fPosY = 0.0f;    // Y position of model in camera view
	m_fZoom = 10.0f;   // Zoom on model in camera view
	m_fRotX = 0.0f;    // Rotation on model in camera view
	m_fRotY = 0.0f;    // Rotation on model in camera view
}

COpenGLTestView::~COpenGLTestView()
{
}

BOOL COpenGLTestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs�� �����Ͽ� ���⿡��
	//  Window Ŭ���� �Ǵ� ��Ÿ���� �����մϴ�.

	return CView::PreCreateWindow(cs);
}

// COpenGLTestView �׸���

void COpenGLTestView::OnDraw(CDC* /*pDC*/)
{
	COpenGLTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: ���⿡ ���� �����Ϳ� ���� �׸��� �ڵ带 �߰��մϴ�.
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -m_fZoom);
	glTranslatef(m_fPosX, m_fPosY, 0.0f);
	glRotatef(m_fRotX, 1.0f, 0.0f, 0.0f);
	glRotatef(m_fRotY, 0.0f, 1.0f, 0.0f);
}


// COpenGLTestView �μ�

BOOL COpenGLTestView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// �⺻���� �غ�
	return DoPreparePrinting(pInfo);
}

void COpenGLTestView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: �μ��ϱ� ���� �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
}

void COpenGLTestView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: �μ� �� ���� �۾��� �߰��մϴ�.
}


// COpenGLTestView ����

#ifdef _DEBUG
void COpenGLTestView::AssertValid() const
{
	CView::AssertValid();
}

void COpenGLTestView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

COpenGLTestDoc* COpenGLTestView::GetDocument() const // ����׵��� ���� ������ �ζ������� �����˴ϴ�.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(COpenGLTestDoc)));
	return (COpenGLTestDoc*)m_pDocument;
}
#endif //_DEBUG


// COpenGLTestView �޽��� ó����


int COpenGLTestView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  ���⿡ Ư��ȭ�� �ۼ� �ڵ带 �߰��մϴ�.
	oglInitialize();

	m_oldWindow = CRect(lpCreateStruct->x, lpCreateStruct->y, lpCreateStruct->x+lpCreateStruct->cx, lpCreateStruct->y+lpCreateStruct->cy);
	m_originalRect = m_oldWindow;

	SetTimer(1, 1, 0);

	return 0;
}


void COpenGLTestView::OnPaint()
{
	//CPaintDC dc(this); // device context for painting
	ValidateRect(NULL);
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	// �׸��� �޽����� ���ؼ��� CView::OnPaint()��(��) ȣ������ ���ʽÿ�.
}

void COpenGLTestView::oglInitialize()
{
   // Initial Setup:
   //
   static PIXELFORMATDESCRIPTOR pfd =
   {
      sizeof(PIXELFORMATDESCRIPTOR),
      1,
      PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
      PFD_TYPE_RGBA,
      32,    // bit depth
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      16,    // z-buffer depth
      0, 0, 0, 0, 0, 0, 0,
   };
 
   // Get device context only once.
   hdc = GetDC()->m_hDC;
 
   // Pixel format.
   m_nPixelFormat = ChoosePixelFormat(hdc, &pfd);
   SetPixelFormat(hdc, m_nPixelFormat, &pfd);
 
   // Create the OpenGL Rendering Context.
   hrc = wglCreateContext(hdc);
   wglMakeCurrent(hdc, hrc);
 
   // Basic Setup:
   //
   // Set color to use when clearing the background.
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glClearDepth(1.0f);
 
   // Turn on backface culling
   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);
 
   // Turn on depth testing
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
 
   // Send draw request
   OnDraw(NULL);
}


void COpenGLTestView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	switch(nIDEvent) {
	case 1 :
		{
			// Clear color and depth buffer bits
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
			// Draw OpenGL scene
			oglDrawScene();
 
			// Swap buffers
			SwapBuffers(hdc);
 
			break;
		}
	default :
		break;
	}

	CView::OnTimer(nIDEvent);
}


void COpenGLTestView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	
	if (0 >= cx || 0 >= cy || nType == SIZE_MINIMIZED) return;
 
	// Map the OpenGL coordinates.
	glViewport(0, 0, cx, cy);
 
	// Projection view
	glMatrixMode(GL_PROJECTION);
 
	glLoadIdentity();
 
	// Set our current view perspective
	gluPerspective(35.0f, (float)cx / (float)cy, 0.01f, 2000.0f);
 
	// Model view
	glMatrixMode(GL_MODELVIEW);
}

void COpenGLTestView::oglDrawScene(void)
{
   // Wireframe Mode
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
 
   glBegin(GL_QUADS);
      // Top Side
      glVertex3f( 1.0f, 1.0f,  1.0f);
      glVertex3f( 1.0f, 1.0f, -1.0f);
      glVertex3f(-1.0f, 1.0f, -1.0f);
      glVertex3f(-1.0f, 1.0f,  1.0f);
 
      // Bottom Side
      glVertex3f(-1.0f, -1.0f, -1.0f);
      glVertex3f( 1.0f, -1.0f, -1.0f);
      glVertex3f( 1.0f, -1.0f,  1.0f);
      glVertex3f(-1.0f, -1.0f,  1.0f);
 
      // Front Side
      glVertex3f( 1.0f,  1.0f, 1.0f);
      glVertex3f(-1.0f,  1.0f, 1.0f);
      glVertex3f(-1.0f, -1.0f, 1.0f);
      glVertex3f( 1.0f, -1.0f, 1.0f);
 
      // Back Side
      glVertex3f(-1.0f, -1.0f, -1.0f);
      glVertex3f(-1.0f,  1.0f, -1.0f);
      glVertex3f( 1.0f,  1.0f, -1.0f);
      glVertex3f( 1.0f, -1.0f, -1.0f);
 
      // Left Side
      glVertex3f(-1.0f, -1.0f, -1.0f);
      glVertex3f(-1.0f, -1.0f,  1.0f);
      glVertex3f(-1.0f,  1.0f,  1.0f);
      glVertex3f(-1.0f,  1.0f, -1.0f);
 
      // Right Side
      glVertex3f( 1.0f,  1.0f,  1.0f);
      glVertex3f( 1.0f, -1.0f,  1.0f);
      glVertex3f( 1.0f, -1.0f, -1.0f);
      glVertex3f( 1.0f,  1.0f, -1.0f);
   glEnd();
}


void COpenGLTestView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	int diffX = (int)(point.x - m_fLastX);
	int diffY = (int)(point.y - m_fLastY);
	m_fLastX  = (float)point.x;
	m_fLastY  = (float)point.y;
 
	// Left mouse button
	if (nFlags & MK_LBUTTON)
	{
		m_fRotX += (float)0.5f * diffY;
 
		if ((m_fRotX > 360.0f) || (m_fRotX < -360.0f))
		{
			m_fRotX = 0.0f;
		}
 
		m_fRotY += (float)0.5f * diffX;
 
		if ((m_fRotY > 360.0f) || (m_fRotY < -360.0f))
		{
			m_fRotY = 0.0f;
		}
	}
 
	// Right mouse button
	else if (nFlags & MK_RBUTTON)
	{
		m_fZoom -= (float)0.1f * diffY;
	}
 
	// Middle mouse button
	else if (nFlags & MK_MBUTTON)
	{
		m_fPosX += (float)0.05f * diffX;
		m_fPosY -= (float)0.05f * diffY;
	}
 
	OnDraw(NULL);

	CView::OnMouseMove(nFlags, point);
}
