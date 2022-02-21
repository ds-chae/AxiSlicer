
// OpenGLTestView.h : COpenGLTestView 클래스의 인터페이스
//

#pragma once


class COpenGLTestView : public CView
{
protected: // serialization에서만 만들어집니다.
	COpenGLTestView();
	DECLARE_DYNCREATE(COpenGLTestView)

// 특성입니다.
public:
	COpenGLTestDoc* GetDocument() const;

// 작업입니다.
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
	virtual ~COpenGLTestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // OpenGLTestView.cpp의 디버그 버전
inline COpenGLTestDoc* COpenGLTestView::GetDocument() const
   { return reinterpret_cast<COpenGLTestDoc*>(m_pDocument); }
#endif

