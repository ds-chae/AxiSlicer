#pragma once
#include "afxwin.h"
#include "ComPortCombo.h"

// CScannerDlg 대화 상자입니다.

class CScannerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CScannerDlg)

public:
	CScannerDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CScannerDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_SCANNER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CComPortCombo m_CommPort;
//	CComboBox m_CommPort;
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedSetComm();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedTest();
};

extern CScannerDlg* pScanner;
