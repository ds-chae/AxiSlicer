
// gcodegenDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "gcodegen.h"
#include "gcodegenDlg.h"
#include "afxdialogex.h"

#include "resource.h"

#define	_CRT_SECURE_NO_WARNINGS	1

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CgcodegenDlg 대화 상자




CgcodegenDlg::CgcodegenDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CgcodegenDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CgcodegenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CgcodegenDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CgcodegenDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_SAVE, &CgcodegenDlg::OnBnClickedSave)
	ON_BN_CLICKED(IDC_CUT_LINENO, &CgcodegenDlg::OnBnClickedCutLineno)
END_MESSAGE_MAP()


// CgcodegenDlg 메시지 처리기

BOOL CgcodegenDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	fx1.fromStr(L"100.0");
	fx2.fromStr(L"110");
	fsx.fromStr(L"3");

	fy1.fromStr(L"20.0");
	fy2.fromStr(L"21.0");
	fsy.fromStr(L"2.1");

	fz1.fromStr(L"0.0");
	fz2.fromStr(L"1.1");
	fsz.fromStr(L".4");

	frx = 1000;
	frz = 700;
	fsh.fromStr(L"50");
	xcut = 0;

	HANDLE h = CreateFile(L"C:\\gcodegen.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h != INVALID_HANDLE_VALUE) {
		DWORD dwio = 0;
		ReadFile(h, &fx1, sizeof(fx1), &dwio, NULL);
		ReadFile(h, &fx2, sizeof(fx2), &dwio, NULL);
		ReadFile(h, &fsx, sizeof(fsx), &dwio, NULL);

		ReadFile(h, &fy1, sizeof(fy1), &dwio, NULL);
		ReadFile(h, &fy2, sizeof(fy2), &dwio, NULL);
		ReadFile(h, &fsy, sizeof(fsy), &dwio, NULL);

		ReadFile(h, &fz1, sizeof(fz1), &dwio, NULL);
		ReadFile(h, &fz2, sizeof(fz2), &dwio, NULL);
		ReadFile(h, &fsz, sizeof(fsz), &dwio, NULL);

		ReadFile(h, &frx, sizeof(frx), &dwio, NULL);
		ReadFile(h, &frz, sizeof(frz), &dwio, NULL);
		ReadFile(h, &fsh, sizeof(fsh), &dwio, NULL);

		ReadFile(h, &xcut, sizeof(xcut), &dwio, NULL);

		CloseHandle(h);
	}

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	SetDlgItemText(IDC_X1, fx1.totstr());
	SetDlgItemText(IDC_X2, fx2.totstr());
	SetDlgItemText(IDC_XSTEP, fsx.totstr());

	SetDlgItemText(IDC_Y1, fy1.totstr());
	SetDlgItemText(IDC_Y2, fy2.totstr());
	SetDlgItemText(IDC_YSTEP, fsy.totstr());

	SetDlgItemText(IDC_Z1, fz1.totstr());
	SetDlgItemText(IDC_Z2, fz2.totstr());
	SetDlgItemText(IDC_ZSTEP, fsz.totstr());

	SetDlgItemInt(IDC_XYFEED, frx);
	SetDlgItemInt(IDC_ZFEED, frz);
	SetDlgItemText(IDC_SAFEZ, fsh.totstr());

	if(xcut) {
		GetDlgItem(IDC_XCUT)->SendMessage(BM_SETCHECK, BST_CHECKED);
		GetDlgItem(IDC_YCUT)->SendMessage(BM_SETCHECK, BST_UNCHECKED);
	} else {
		GetDlgItem(IDC_XCUT)->SendMessage(BM_SETCHECK, BST_UNCHECKED);
		GetDlgItem(IDC_YCUT)->SendMessage(BM_SETCHECK, BST_CHECKED);
	}

	_sztext = (char*)malloc(_spsize+1);
	_sztext[0] = 0;
	_splen = 0;

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CgcodegenDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CgcodegenDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CgcodegenDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


dec3 CgcodegenDlg::GetDlgItemDec3(int nID)
{
	dec3 temp3;

	TCHAR tszBuf[16];
	GetDlgItemText(nID, tszBuf, 16);
	temp3.fromStr(tszBuf);
	
	return temp3;
/*
	float divider = 1.0;
	int sign = 1;
	int idx = 0;
	float fv = 0;
	for(idx = 0; tszBuf[idx] != 0; idx++) {
		TCHAR tc = tszBuf[idx];
		if(tc == '-') sign = -1;
		else if(tc == '+') sign = 1;
		else if(tc == '.') divider = 0.1;
		else if(tc >= '0' && tc <= '9') {
			float f = (float)(tc - '0');
			if(divider == 1.0)
				fv = fv * 10 + f;
			else {
				fv = fv + (f * divider);
				divider *= 0.1;
			}
		} else {
			break;
		}
	}

	return fv * sign;
*/
}

void CgcodegenDlg::putxygcode(char* code, dec3& x, dec3& y, int feed)
{
	char sztemp[64];

	if(last_x == x && last_y == y) {
		last_feed = 0;
		return;
	}
	spadd(code);

	if(last_x != x && last_y != y) {
		sprintf_s(sztemp, 63, "X%sY%s", x.tostr(), y.tostr());
	} else {
		if(last_x != x)
			sprintf_s(sztemp, 63, "X%s", x.tostr());
		else
			sprintf_s(sztemp, 63, "Y%s", y.tostr());
	}
	spadd(sztemp);

	if(feed == 0) {
		last_feed = 0;
	} else {
		if( feed != last_feed) {
			sprintf_s(sztemp, 63, "F%d", feed);
			spadd(sztemp);
			last_feed = feed;
		}
	} 

	sprintf_s(sztemp, 63, "\r\n");
	spadd(sztemp);

	last_x = x;
	last_y = y;
}

void CgcodegenDlg::OnBnClickedOk()
{
	fx1 = GetDlgItemDec3(IDC_X1);
	fx2 = GetDlgItemDec3(IDC_X2);
	fsx = GetDlgItemDec3(IDC_XSTEP);

	fy1 = GetDlgItemDec3(IDC_Y1);
	fy2 = GetDlgItemDec3(IDC_Y2);
	fsy = GetDlgItemDec3(IDC_YSTEP);

	fz1 = GetDlgItemDec3(IDC_Z1);
	fz2 = GetDlgItemDec3(IDC_Z2);
	fsz = GetDlgItemDec3(IDC_ZSTEP);

	frx = GetDlgItemInt(IDC_XYFEED);
	frz = GetDlgItemInt(IDC_ZFEED);
	fsh = GetDlgItemDec3(IDC_SAFEZ);

	xcut = GetDlgItem(IDC_XCUT)->SendMessage(BM_GETCHECK) == BST_CHECKED;

	if(fx1.value > fx2.value) {
		MessageBox(L"Start X shoule be less than or equal to End X.");
		return;
	}
	if(fy1.value > fy2.value) {
		MessageBox(L"Start Y shoule be less than or equal to End Y.");
		return;
	}
	if(fz1.value < fz2.value) {
		MessageBox(L"Start Z shoule be greater than or equal to End Z.");
		return;
	}
	if((fsh.value - fz1.value) < 5000) {
		MessageBox(L"Safe Z shoule be greater than start Z about 5mm.");
		return;
	}

	if(fsx.value <= 0 || fsy.value <= 0 || fsz.value <= 0) {
		MessageBox(L"Step values shoule be greater than zero.");
		return;
	}

	HANDLE h = CreateFile(L"C:\\gcodegen.txt", GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h != INVALID_HANDLE_VALUE) {
		DWORD dwio;

		WriteFile(h, &fx1, sizeof(fx1), &dwio, NULL);
		WriteFile(h, &fx2, sizeof(fx2), &dwio, NULL);
		WriteFile(h, &fsx, sizeof(fsx), &dwio, NULL);

		WriteFile(h, &fy1, sizeof(fy1), &dwio, NULL);
		WriteFile(h, &fy2, sizeof(fy2), &dwio, NULL);
		WriteFile(h, &fsy, sizeof(fsy), &dwio, NULL);

		WriteFile(h, &fz1, sizeof(fz1), &dwio, NULL);
		WriteFile(h, &fz2, sizeof(fz2), &dwio, NULL);
		WriteFile(h, &fsz, sizeof(fsz), &dwio, NULL);

		WriteFile(h, &frx, sizeof(frx), &dwio, NULL);
		WriteFile(h, &frz, sizeof(frz), &dwio, NULL);
		WriteFile(h, &fsh, sizeof(fsh), &dwio, NULL);

		WriteFile(h, &xcut, sizeof(xcut), &dwio, NULL);

		CloseHandle(h);
	}

	int bufsize = 2048*1024;

	_sztext[0] = 0;
	_splen = 0;

	char sztemp[64];
	spadd("M3\r\nM8\r\n");

	// goto starting position
	last_z = 10000;
	last_x = -10000, last_y = -10000;
	last_feed = 0;
	sprintf_s(sztemp, 63, "G0Z%s\r\n",  fsh.tostr()); last_z = fsh;
	spadd(sztemp);
	putxygcode("G0", fx1, fy1, 0);

	if(fx1 == fx2) {
		// cutting x direction
		CutYFirst();
	} else if(fy1 == fy2) {
		CutXFirst();
	} else {
		SurfaceCut();
	}

	sprintf_s(sztemp, 63, "G0Z%s\r\n",  fsh.tostr());
	spadd(sztemp);
	sprintf_s(sztemp, 63, "M30\r\n");
	spadd(sztemp);

	int buflen = strlen(_sztext);
	CString tsztext(_sztext);
	SetDlgItemText(IDC_GCODE, tsztext);

	MessageBox(L"Generation finished.");
}

void CgcodegenDlg::SurfaceCut()
{
	char sztemp[64];

	dec3 z;
	for(z.value = fz1.value; z.value >= fz2.value; ) {
		z.value = z.value - fsz.value;
		if(z.value < fz2.value) z = fz2;

		sprintf_s(sztemp, 63, "(NEW PLANE AT %s)\r\n", z.tostr());
		spadd(sztemp);
		if(last_z != fsh) {
			dec3 tempz = last_z;
			tempz.value += 2000;
			sprintf_s(sztemp, 63, "G0Z%s\r\n", tempz.tostr()); last_feed = 0;
			spadd(sztemp);
		}
		putxygcode("G0", fx1, fy1, 0);
		sprintf_s(sztemp, 63, "G1Z%sF%d\r\n", z.tostr(), frz); last_z = z; last_feed = 0;
		spadd(sztemp);
		if(!xcut) {
			// cut y first
			for(dec3 x = fx1; x.value <= fx2.value; ) {
				putxygcode("G1", x, fy1, frx);
				putxygcode("G1", x, fy2, frx);
				if(x == fx2)
					break;
				x.value += fsx.value; if(x.value > fx2.value) x = fx2;
				putxygcode("G1", x, fy2, frx);
				putxygcode("G1", x, fy1, frx);
				if( x == fx2)
					break;
				x.value += fsx.value; if(x.value > fx2.value) x = fx2;
			}
			if(fy1 != fy2) {
				spadd("(FINISH)\r\n");
				if(last_y == fy1) {
					putxygcode("G1", fx1, fy1, frx);
					putxygcode("G1", fx1, fy2, frx);
					putxygcode("G1", fx2, fy2, frx);
				} else {
					putxygcode("G1", fx1, fy2, frx);
					putxygcode("G1", fx1, fy1, frx);
					putxygcode("G1", fx2, fy1, frx);
				}
			}
		} else {
			// cut x first
			for(dec3 y = fy1; y.value <= fy2.value; ) {
				putxygcode("G1", fx1, y, frx);
				putxygcode("G1", fx2, y, frx);
				if(y.value >= fy2.value)
					break;
				y.value += fsy.value; if(y.value > fy2.value) y = fy2;
				putxygcode("G1", fx2, y, frx);
				putxygcode("G1", fx1, y, frx);
				if( y.value >= fy2.value)
					break;
				y.value += fsy.value; if(y.value > fy2.value) y = fy2;
			}
			if(fx1 != fx2) {
				spadd("(FINISH LINE)\r\n");
				if(last_x == fx1) {
					putxygcode("G1", fx1, fy1, frx);
					putxygcode("G1", fx2, fy1, frx);
					putxygcode("G1", fx2, fy2, frx);
				} else {
					putxygcode("G1", fx2, fy1, frx);
					putxygcode("G1", fx1, fy1, frx);
					putxygcode("G1", fx1, fy2, frx);
				}
			}
		}
		if(z.value <= fz2.value)
			break;
	}
}


void CgcodegenDlg::CutXFirst()
{
	char sztemp[64];

	dec3 z;
	for(z.value = fz1.value; z.value >= fz2.value; ) {
		z.value = z.value - fsz.value;
		if(z.value < fz2.value) z = fz2;

		sprintf_s(sztemp, 63, "(NEW LINE AT %s)\r\n", z.tostr());
		spadd(sztemp);
		sprintf_s(sztemp, 63, "G1Z%sF%d\r\n", z.tostr(), frz); last_z = z; last_feed = 0;
		spadd(sztemp);
		// cut x first
		putxygcode("G1", fx2, fy1, frx);
		if(z.value <= fz2.value)
			break;
		z.value -= fsz.value;
		if(z.value < fz2.value) z.value = fz2.value;

		sprintf_s(sztemp, 63, "(NEW LINE AT %s)\r\n", z.tostr());
		spadd(sztemp);
		sprintf_s(sztemp, 63, "G1Z%sF%d\r\n", z.tostr(), frz); last_z = z; last_feed = 0;
		spadd(sztemp);
		putxygcode("G1", fx1, fy1, frx);
		if(z.value <= fz2.value)
			break;
	}
}


void CgcodegenDlg::CutYFirst()
{
	char sztemp[64];

	dec3 z;
	for(z.value = fz1.value; z.value >= fz2.value; ) {
		z.value = z.value - fsz.value;
		if(z.value < fz2.value) z = fz2;

		sprintf_s(sztemp, 63, "(NEW LINE AT %s)\r\n", z.tostr());
		spadd(sztemp);
		sprintf_s(sztemp, 63, "G1Z%sF%d\r\n", z.tostr(), frz); last_z = z; last_feed = 0;
		spadd(sztemp);
		putxygcode("G1", fx1, fy2, frx);
		if(z.value <= fz2.value)
			break;

		z.value = z.value - fsz.value;
		if(z.value < fz2.value)
			z = fz2;
		sprintf_s(sztemp, 63, "(NEW LINE AT %s)\r\n", z.tostr());
		spadd(sztemp);
		sprintf_s(sztemp, 63, "G1Z%sF%d\r\n", z.tostr(), frz); last_z = z; last_feed = 0;
		spadd(sztemp);
		putxygcode("G1", fx1, fy1, frx);
		if(z.value <= fz2.value)
			break;
	}
}


void CgcodegenDlg::OnBnClickedSave()
{
	if(strlen(_sztext) == 0) {
		MessageBox(L"There is nothing to save.");
		return;
	}
	
	CFileDialog dlg(FALSE, L".TXT");
	if(dlg.DoModal() == IDOK) {
		CString fname = dlg.GetPathName();
		HANDLE h = CreateFile((LPCTSTR)fname, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h == INVALID_HANDLE_VALUE) {
			MessageBox(L"File creation failed");
			return;
		}
		DWORD dwio = 0;
		WriteFile(h, _sztext, strlen(_sztext), &dwio, NULL);
		CloseHandle(h);
		if(dwio == strlen(_sztext)) {
			MessageBox(L"File is saved.");
		}
	}
}

void CgcodegenDlg::spadd(char* str)
{
	int slen = strlen(str);
	if(_splen + slen < _spsize) {
		while(*str) {
			_sztext[_splen++] = *str++;
		}
		_sztext[_splen] = 0;
	} else {
		ASSERT(0);
	}
}


void CgcodegenDlg::OnBnClickedCutLineno()
{
	CFileDialog dlg(TRUE, L".TAP");
	if(dlg.DoModal() == IDOK) {
		CString fname = dlg.GetPathName();
		HANDLE h = CreateFile((LPCTSTR)fname, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h == INVALID_HANDLE_VALUE) {
			MessageBox(L"File creation failed");
			return;
		}
		DWORD dwio = 0;
		int fsize = GetFileSize(h, NULL);
		char* buf = (char*)malloc(fsize+1);
		if(buf == NULL) {
			CloseHandle(h);
			MessageBox(L"Not enough memory");
			return;
		}

		dwio = 0;
		ReadFile(h, buf, fsize, &dwio, NULL);
		CloseHandle(h);
		if(dwio != fsize) {
			MessageBox(L"Read failed");
			return;
		}
		buf[fsize] = 0;

		TCHAR cvtfname[MAX_PATH];
		lstrcpy(cvtfname, (LPCTSTR)fname);
		lstrcat(cvtfname, L".TAP");
		HANDLE oh = CreateFile(cvtfname, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(oh == INVALID_HANDLE_VALUE) {
			TCHAR msgbuf[MAX_PATH+64];
			wsprintf(msgbuf, L"Creating %s failed.", cvtfname);
			MessageBox(msgbuf);
			return;
		}

		char linebuf[256];
		int  linelen = 0;
		int  idx = 0;
		int  state = 0;
		for(int idx = 0; idx < fsize; idx++) {
			char c = buf[idx];
			switch(state){
			case 0 : // wait 'N'
				if(c == 'N')
					state = 1;
				else {
					if(linelen < sizeof(linebuf)-2)
						linebuf[linelen++] = c;
				}
				break;
			case 1 : // skip line number
				if(c >= '0' && c <= '9') {
				} else {
					if(linelen < sizeof(linebuf)-2)
						linebuf[linelen++] = c;
					state = 2;
				}
				break;
			case 2 :
				if(linelen < sizeof(linebuf)-2)
					linebuf[linelen++] = c;
			}
			if(c == '\n') {
				linebuf[linelen] = 0;
				WriteFile(oh, linebuf, linelen, &dwio, NULL);
			}
		}
		CloseHandle(oh);
		TCHAR msgbuf[MAX_PATH+64];
		wsprintf(msgbuf, L"File %s was written.", cvtfname);
	}
}
