#pragma once
#include "afxwin.h"

#include "ComPortCombo.h"

// CCommPortDlg ��ȭ �����Դϴ�.

class CCommPortDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCommPortDlg)

public:
	CCommPortDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CCommPortDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_CONNECTPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedConnect();
	CComPortCombo m_CommPort;
	CComboBox m_CommSpeed;
	virtual BOOL OnInitDialog();
};

