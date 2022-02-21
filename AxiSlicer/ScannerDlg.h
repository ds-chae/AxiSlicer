#pragma once
#include "afxwin.h"
#include "ComPortCombo.h"

// CScannerDlg ��ȭ �����Դϴ�.

class CScannerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CScannerDlg)

public:
	CScannerDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CScannerDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_SCANNER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

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
