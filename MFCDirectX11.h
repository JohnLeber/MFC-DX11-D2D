
// MFCDirectX11.h : main header file for the MFCDirectX11 application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CMFCDirectX11App:
// See MFCDirectX11.cpp for the implementation of this class
//

class CMFCDirectX11App : public CWinApp
{
public:
	CMFCDirectX11App() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle(LONG lCount);
};

extern CMFCDirectX11App theApp;
