#pragma once
#include "afxwin.h"

#include "ComPortCombo.h"

// CCommPortDlg 대화 상자입니다.

class CCommPortDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCommPortDlg)

public:
	CCommPortDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CCommPortDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_CONNECTPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedConnect();
	CComPortCombo m_CommPort;
	CComboBox m_CommSpeed;
	virtual BOOL OnInitDialog();
};

