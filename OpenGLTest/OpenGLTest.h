
// OpenGLTest.h : OpenGLTest ���� ���α׷��� ���� �� ��� ����
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"       // �� ��ȣ�Դϴ�.


// COpenGLTestApp:
// �� Ŭ������ ������ ���ؼ��� OpenGLTest.cpp�� �����Ͻʽÿ�.
//

class COpenGLTestApp : public CWinApp
{
public:
	COpenGLTestApp();


// �������Դϴ�.
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// �����Դϴ�.
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern COpenGLTestApp theApp;
