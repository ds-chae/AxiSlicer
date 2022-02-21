
// OpenGLTestView.h : COpenGLTestView Ŭ������ �������̽�
//

#pragma once


class COpenGLTestView : public CView
{
protected: // serialization������ ��������ϴ�.
	COpenGLTestView();
	DECLARE_DYNCREATE(COpenGLTestView)

// Ư���Դϴ�.
public:
	COpenGLTestDoc* GetDocument() const;

// �۾��Դϴ�.
public:
	UINT_PTR	m_unpTimer;

	float m_fLastX;
	float m_fLastY;
	float m_fRotX;
	float m_fRotY;
	float m_fZoom;
	float m_fPosX;
	float m_fPosY;

private:
	HDC	hdc;
	HGLRC	hrc;
	int	m_nPixelFormat;
	CRect	m_rect;
	CRect	m_oldWindow;
	CRect	m_originalRect;

	void oglInitialize();
	void oglDrawScene(void);

// �������Դϴ�.
public:
	virtual void OnDraw(CDC* pDC);  // �� �並 �׸��� ���� �����ǵǾ����ϴ�.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// �����Դϴ�.
public:
	virtual ~COpenGLTestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // OpenGLTestView.cpp�� ����� ����
inline COpenGLTestDoc* COpenGLTestView::GetDocument() const
   { return reinterpret_cast<COpenGLTestDoc*>(m_pDocument); }
#endif

