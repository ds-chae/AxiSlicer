// CommPortDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "AxiSlicer.h"
#include "CommPortDlg.h"
#include "afxdialogex.h"
#include "EnumSerial.h"

// CCommPortDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CCommPortDlg, CDialogEx)

CCommPortDlg::CCommPortDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCommPortDlg::IDD, pParent)
{

}

CCommPortDlg::~CCommPortDlg()
{
}

void CCommPortDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMM_PORT, m_CommPort);
	DDX_Control(pDX, IDC_COMM_SPEED, m_CommSpeed);
}


BEGIN_MESSAGE_MAP(CCommPortDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CONNECT, &CCommPortDlg::OnBnClickedConnect)
END_MESSAGE_MAP()


// CConnectPortDlg message handlers

int ConnectCommPort(LPCTSTR port, int ispeed, TCHAR* msg);

#define	APP_NAME "SOFTWARE\\SEETECH\\ST150BOOTUTIL"

int ConnectTcpPort(char* ip, int port, TCHAR* msg);

#if SUPPORT_TCPIP
TCHAR tszip[32] = L"192.168.1.75";
int TcpIpPort = 20000;
#endif

void RegSetOptions()
{
	DWORD dwDisp = 0;
	HKEY hkey = NULL;
	LONG err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT(APP_NAME), 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisp);
	if(err != ERROR_SUCCESS)
		return;

#if SUPPORT_TCPIP
	RegSetValueEx(hkey, TEXT("IPADDR"), 0, REG_BINARY, (LPBYTE)tszip, sizeof(tszip));
	RegSetValueEx(hkey, TEXT("IPPORT"), 0, REG_BINARY, (LPBYTE)&TcpIpPort, sizeof(TcpIpPort));
#endif
	RegCloseKey(hkey);
}

void RegGetOptions()
{
	HKEY hkey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT(APP_NAME), 0, KEY_ALL_ACCESS, &hkey) != ERROR_SUCCESS)
		return;

	DWORD dwType, dwSize;

#if SUPPORT_TCPIP
	dwSize = sizeof(tszip);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("IPADDR"), NULL, &dwType, (LPBYTE)tszip, &dwSize);

	dwSize = sizeof(TcpIpPort);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("IPPORT"), NULL, &dwType, (LPBYTE)&TcpIpPort, &dwSize);
#endif
	RegCloseKey(hkey);
}

#if SUPPORT_TCPIP
void CConnectPortDlg::OnBnClickedButtonConnectIp()
{
	GetDlgItemText(IDC_EDIT_IPADDR, tszip, 32);
	char szip[32];
	WideCharToMultiByte(CP_ACP, 0, tszip, lstrlen(tszip)+1, szip, 32, NULL, NULL);


	TcpIpPort = GetDlgItemInt(IDC_EDIT_TCP_PORT);

	RegSetOptions();

	if(ConnectTcpPort(szip, TcpIpPort, msg))
		OnOK();
	else
		MessageBox(msg);
}
#endif

void CCommPortDlg::OnBnClickedConnect()
{
	if(m_CommPort.GetCount() < 1)
		return;

	CString port;
	m_CommPort.GetFileName(port);

	CString speed;
	m_CommSpeed.GetWindowText(speed);
	int ispeed = StrToInt(speed);

	TCHAR msg[256];
	if(ConnectCommPort(port, ispeed, msg)) {
		int portNum = m_CommPort.GetPortNum();
		AfxGetApp()->WriteProfileInt(_T("Config"), _T("ComPort"), portNum);
		OnOK();
	} else {
		MessageBox(msg);
	}
}


BOOL CCommPortDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

// By default, the first combo box item is "<None>".
	m_CommPort.SetNoneItem(0);
    // The default strings may be also change using SetNoneStr().
//    m_listPorts.SetNoneStr(_T("No port"));
    // By default, all COM ports are listed.
//    m_listPorts.SetOnlyPhysical(1);
    // By default, only present COM ports are listed.
//    m_listPorts.SetOnlyPresent(0);
// Pre-select the configured port
	int portNum = AfxGetApp()->GetProfileInt(_T("Config"), _T("ComPort"), -1);
	m_CommPort.InitList(portNum);
/*
	CArray<SSerInfo,SSerInfo&> asi;
	EnumSerialPorts(asi, TRUE);

	for(int i = 0; i < asi.GetCount(); i++) {
		CString port = asi[i].strPortName;
		m_CommPort.AddString(port);
	}
*/
	m_CommSpeed.AddString(TEXT("115200"));
	m_CommSpeed.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}
