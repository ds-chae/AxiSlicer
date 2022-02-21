
// gcodegenDlg.h : 헤더 파일
//

#pragma once

class dec3 {
public:
	int value;
	char szbuf[16];
	TCHAR tszbuf[16];

	dec3(LPCTSTR tstr)
	{
		fromStr(tstr);
	}

	dec3(int _v)
	{
		value = _v;
	}

	dec3()
	{
		value = 0;
	}

	char* tostr()
	{
		char temp[32];
		int  sign = 1;
		int  tv = value;
		if(tv < 0) {
			tv = -tv;
			sign = -1;
		}
		int len = 0;
		while(tv > 0) {
			if(len == 3)
				temp[len++] = '.';
			temp[len++] = '0' + (tv % 10);
			tv /= 10;
		}
		while(len < 3)
			temp[len++] = '0'; // make 3
		if(len == 3) {
			temp[len++] = '.'; // make 4
			temp[len++] = '0'; // make 5
		}
		if(sign == -1)
			temp[len++] = '-';

		int slen = 0;
		int not_zero = 0;
		while(len > 0 && slen < sizeof(szbuf)-1) {
			szbuf[slen++] = temp[len-1];
			len--;
			ASSERT(slen < sizeof(szbuf)-1);
		}
		szbuf[slen] = 0;

		return szbuf;
	}

	TCHAR* totstr()
	{
		TCHAR* tp = tszbuf;
		char* cp = tostr();
		while(*cp) {
			*tp++ = *cp++;
		}
		*tp = 0;
		return tszbuf;
	}

	void fromStr(LPCTSTR tstr)
	{
		value = 0;
		int sign = 1;
		int decount = -1;
		while(*tstr) {
			if(*tstr == '-')
				sign = -1;
			else if(*tstr == '.')
				decount = 0;
			else if(*tstr >= '0' && *tstr <= '9') {
				value = value * 10 + (*tstr - '0');
				if(decount != -1) decount++;
			}
			if(decount >= 3)
				break;
			tstr++;
		}
		if(decount == -1)
			decount = 0;
		while(decount < 3) {
			value = value * 10;
			decount++;
		}
		if(sign == -1)
			value *= -1;

		tostr();
	}

	int operator ==(dec3& d2)
	{
		return value == d2.value;
	}

	int operator !=(dec3& d2)
	{
		return value != d2.value;
	}


};

// CgcodegenDlg 대화 상자
class CgcodegenDlg : public CDialogEx
{
// 생성입니다.
public:
	CgcodegenDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_GCODEGEN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

public:
	dec3  fx1, fx2, fsx;

	dec3 fy1, fy2, fsy;

	dec3 fz1, fz2, fsz;

	int frx, frz;

	dec3 fsh;
	int  xcut;

	dec3 last_x, last_y;
	int   last_feed;
	dec3 last_z;

#define	_spsize	(4096*1204-1)
	char* _sztext;
	int  _splen;
	void spadd(char* str);

	dec3 GetDlgItemDec3(int nID);
	void putxygcode(char* code, dec3& x, dec3& y, int feed);
	void SurfaceCut();
	void CutXFirst();
	void CutYFirst();

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedCutLineno();
};
