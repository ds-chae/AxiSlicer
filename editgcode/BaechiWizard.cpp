// BaechiWizard.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "editgcode.h"
#include "BaechiWizard.h"
#include "afxdialogex.h"
#include "MainFrm.h"

#include "openglGui.h"
#include "opengl.h"
#include "sceneView.h"
#include "profile.h"
#include "meshLoader.h"
#include "stlio.h"
#include "gcodeInterpreter.h"
#include "mesh.h"
#include "objectScene.h"
#include "gcodefilelist.h"

#include "editgcodeDoc.h"
#include "editgcodeView.h"

// CBaechiWizard 대화 상자입니다.

IMPLEMENT_DYNAMIC(CBaechiWizard, CDialogEx)

CBaechiWizard::CBaechiWizard(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBaechiWizard::IDD, pParent)
{

}

CBaechiWizard::~CBaechiWizard()
{
}

void CBaechiWizard::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_FileList);
}


BEGIN_MESSAGE_MAP(CBaechiWizard, CDialogEx)
	ON_BN_CLICKED(IDC_ADD, &CBaechiWizard::OnBnClickedAdd)
END_MESSAGE_MAP()


CBaechiWizard *pBaechiWizard = NULL;

// CBaechiWizard 메시지 처리기입니다.


void CBaechiWizard::PostNcDestroy()
{
	CDialogEx::PostNcDestroy();

	if(pBaechiWizard != NULL) {
		delete pBaechiWizard;
		pBaechiWizard = NULL;
	}
}

void CBaechiWizard::OnBnClickedAdd()
{
	LPTSTR szFilter = L"NC Files(*.tap)|*.tap|All Files(*.*) | *.* ||";
	CFileDialog dlg(TRUE, // TRUE--> 열기        FALSE--> 다른 이름으로 저장
		L"tap", // 기본 확장자. 만약 cpp라고 적으면 파일명이 자동으로 test.cpp가 됨
		NULL, // 대화상자가 처음 출력되었을 때 "파일 이름" Edit 컨트롤에 출력될 파일명 지정
		OFN_EXPLORER |  //   윈도우 탐색기 스타일로 출력
		OFN_FILEMUSTEXIST | //    존재하지 않는 파일명을 입력할 수 없도록 함
		OFN_HIDEREADONLY | //      읽기 전용 파일은 출력하지 않음
		OFN_LONGNAMES, // 긴 파일 이름 포맷 지원
		szFilter, this);
	if(IDOK == dlg.DoModal()) {
        CString strPathName = dlg.GetPathName();
		this->m_FileList.AddString(strPathName);
		gcodeFileList.addFilePath((LPCTSTR)strPathName);
		CMainFrame *pFrame = (CMainFrame *)GetParent();
		CeditgcodeView* pView = (CeditgcodeView* )pFrame->GetActiveView();
		pView->ReloadGCodeFiles();
//		gcode *_gcode = new gcode(this);
//		_gcode->add((LPCTSTR)strPathName);
//		delete _gcode;
	}
}
