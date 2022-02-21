#pragma once
#include "afxwin.h"


// CBaechiWizard 대화 상자입니다.

class CBaechiWizard : public CDialogEx
{
	DECLARE_DYNAMIC(CBaechiWizard)

public:
	CBaechiWizard(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CBaechiWizard();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_BAECHI_WIZARD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	virtual void PostNcDestroy();
public:
	afx_msg void OnBnClickedAdd();
	CListBox m_FileList;
};

extern CBaechiWizard *pBaechiWizard;
