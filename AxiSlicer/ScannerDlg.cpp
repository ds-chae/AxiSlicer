// ScannerDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "AxiSlicer.h"
#include "ScannerDlg.h"
#include "afxdialogex.h"
#include "CommAPI.h"
#include "resource.h"

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

// CScannerDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CScannerDlg, CDialogEx)

CScannerDlg* pScanner = NULL;

CScannerDlg::CScannerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CScannerDlg::IDD, pParent)
{

}

CScannerDlg::~CScannerDlg()
{
}

void CScannerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMM_PORT, m_CommPort);
}


BEGIN_MESSAGE_MAP(CScannerDlg, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &CScannerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_SET_COMM, &CScannerDlg::OnBnClickedSetComm)
	ON_BN_CLICKED(IDC_START, &CScannerDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_STOP, &CScannerDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_TEST, &CScannerDlg::OnBnClickedTest)
END_MESSAGE_MAP()


// CScannerDlg �޽��� ó�����Դϴ�.


BOOL CScannerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	int portNum = AfxGetApp()->GetProfileInt(_T("Config"), _T("ComPort"), -1);
	m_CommPort.InitList(portNum);
//	OnBnClickedSetComm(); // Connect to robot
	SetDlgItemInt(IDC_STEPS, 40); // 16400/40 = 410 points.

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

void CScannerDlg::OnBnClickedSetComm()
{
	TCHAR msg[256];
	CString port;

	m_CommPort.GetFileName(port);
	if(!ConnectCommPort(port, 19200, msg)) {
		MessageBox(msg);
	} else {
		int portnum = m_CommPort.GetCurSel();
		AfxGetApp()->WriteProfileInt(_T("Config"), _T("ComPort"), portnum);
	}
}

void CScannerDlg::OnBnClickedCancel()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CDialogEx::OnCancel();
}

void CScannerDlg::OnBnClickedStart()
{
	CAxiSlicerView* pView = (CAxiSlicerView*)GetParent();
	pView->StartScanner();
}


extern int scan_started;

void CScannerDlg::OnBnClickedStop()
{
	if(scan_started != 0)
		scan_started = 3;
}


void CScannerDlg::OnBnClickedTest()
{
	if(scan_started == 0)
		scan_started = 4;
}
