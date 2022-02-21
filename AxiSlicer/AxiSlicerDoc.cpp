
// AxiSlicerDoc.cpp : CAxiSlicerDoc 클래스의 구현
//

#include "stdafx.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "AxiSlicer.h"
#endif

#include "mesh.h"
#include "objectScene.h"
#include "openglGui.h"
#include "opengl.h"
#include "sliceEngine.h"
#include "sceneView.h"
#include "profile.h"
#include "meshLoader.h"
#include "stlio.h"
#include "gcodeInterpreter.h"

#include "AxiSlicerDoc.h"
#include "AxiSlicerView.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAxiSlicerDoc

IMPLEMENT_DYNCREATE(CAxiSlicerDoc, CDocument)

BEGIN_MESSAGE_MAP(CAxiSlicerDoc, CDocument)
END_MESSAGE_MAP()


// CAxiSlicerDoc 생성/소멸

CAxiSlicerDoc::CAxiSlicerDoc()
{
	// TODO: 여기에 일회성 생성 코드를 추가합니다.

}

CAxiSlicerDoc::~CAxiSlicerDoc()
{
}

BOOL CAxiSlicerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 여기에 재초기화 코드를 추가합니다.
	// SDI 문서는 이 문서를 다시 사용합니다.
	POSITION pos = GetFirstViewPosition();
	CAxiSlicerView* pView = (CAxiSlicerView*)GetNextView(pos);
	pView->viewMode = _view_scanner;

	return TRUE;
}




// CAxiSlicerDoc serialization

void CAxiSlicerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 여기에 저장 코드를 추가합니다.
	}
	else
	{
		// TODO: 여기에 로딩 코드를 추가합니다.
	}
}

#ifdef SHARED_HANDLERS

// 축소판 그림을 지원합니다.
void CAxiSlicerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 문서의 데이터를 그리려면 이 코드를 수정하십시오.
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 검색 처리기를 지원합니다.
void CAxiSlicerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 문서의 데이터에서 검색 콘텐츠를 설정합니다.
	// 콘텐츠 부분은 ";"로 구분되어야 합니다.

	// 예: strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CAxiSlicerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CAxiSlicerDoc 진단

#ifdef _DEBUG
void CAxiSlicerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAxiSlicerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CAxiSlicerDoc 명령


BOOL CAxiSlicerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	strList *flist = new strList(__FILE__, __LINE__);
	flist->add(lpszPathName);

	POSITION pos = GetFirstViewPosition();
	while (pos != NULL) {
		CAxiSlicerView* pView = (CAxiSlicerView*)GetNextView(pos);
		pView->loadFiles(flist);
	}   

	delete flist;

	return TRUE;
}
