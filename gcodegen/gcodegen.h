
// gcodegen.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CgcodegenApp:
// �� Ŭ������ ������ ���ؼ��� gcodegen.cpp�� �����Ͻʽÿ�.
//

class CgcodegenApp : public CWinApp
{
public:
	CgcodegenApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CgcodegenApp theApp;