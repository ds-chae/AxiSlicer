// BaechiWizard.cpp : ���� �����Դϴ�.
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

// CBaechiWizard ��ȭ �����Դϴ�.

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

// CBaechiWizard �޽��� ó�����Դϴ�.


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
	CFileDialog dlg(TRUE, // TRUE--> ����        FALSE--> �ٸ� �̸����� ����
		L"tap", // �⺻ Ȯ����. ���� cpp��� ������ ���ϸ��� �ڵ����� test.cpp�� ��
		NULL, // ��ȭ���ڰ� ó�� ��µǾ��� �� "���� �̸�" Edit ��Ʈ�ѿ� ��µ� ���ϸ� ����
		OFN_EXPLORER |  //   ������ Ž���� ��Ÿ�Ϸ� ���
		OFN_FILEMUSTEXIST | //    �������� �ʴ� ���ϸ��� �Է��� �� ������ ��
		OFN_HIDEREADONLY | //      �б� ���� ������ ������� ����
		OFN_LONGNAMES, // �� ���� �̸� ���� ����
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
