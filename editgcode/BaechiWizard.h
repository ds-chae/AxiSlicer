#pragma once
#include "afxwin.h"


// CBaechiWizard ��ȭ �����Դϴ�.

class CBaechiWizard : public CDialogEx
{
	DECLARE_DYNAMIC(CBaechiWizard)

public:
	CBaechiWizard(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CBaechiWizard();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_BAECHI_WIZARD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
	virtual void PostNcDestroy();
public:
	afx_msg void OnBnClickedAdd();
	CListBox m_FileList;
};

extern CBaechiWizard *pBaechiWizard;
