
// AxiSlicerDoc.cpp : CAxiSlicerDoc Ŭ������ ����
//

#include "stdafx.h"
// SHARED_HANDLERS�� �̸� ����, ����� �׸� �� �˻� ���� ó���⸦ �����ϴ� ATL ������Ʈ���� ������ �� ������
// �ش� ������Ʈ�� ���� �ڵ带 �����ϵ��� �� �ݴϴ�.
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


// CAxiSlicerDoc ����/�Ҹ�

CAxiSlicerDoc::CAxiSlicerDoc()
{
	// TODO: ���⿡ ��ȸ�� ���� �ڵ带 �߰��մϴ�.

}

CAxiSlicerDoc::~CAxiSlicerDoc()
{
}

BOOL CAxiSlicerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: ���⿡ ���ʱ�ȭ �ڵ带 �߰��մϴ�.
	// SDI ������ �� ������ �ٽ� ����մϴ�.
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
		// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	}
	else
	{
		// TODO: ���⿡ �ε� �ڵ带 �߰��մϴ�.
	}
}

#ifdef SHARED_HANDLERS

// ����� �׸��� �����մϴ�.
void CAxiSlicerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// ������ �����͸� �׸����� �� �ڵ带 �����Ͻʽÿ�.
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

// �˻� ó���⸦ �����մϴ�.
void CAxiSlicerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// ������ �����Ϳ��� �˻� �������� �����մϴ�.
	// ������ �κ��� ";"�� ���еǾ�� �մϴ�.

	// ��: strSearchContent = _T("point;rectangle;circle;ole object;");
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

// CAxiSlicerDoc ����

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


// CAxiSlicerDoc ���


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
