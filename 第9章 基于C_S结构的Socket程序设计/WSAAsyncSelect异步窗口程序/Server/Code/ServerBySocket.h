// ServerBySocket.h : main header file for the SERVERBYSOCKET application
//

#if !defined(AFX_SERVERBYSOCKET_H__AD85103D_BD0B_46DC_812D_DE5605F3A7F6__INCLUDED_)
#define AFX_SERVERBYSOCKET_H__AD85103D_BD0B_46DC_812D_DE5605F3A7F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CServerBySocketApp:
// See ServerBySocket.cpp for the implementation of this class
//

class CServerBySocketApp : public CWinApp
{
public:
	CServerBySocketApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerBySocketApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CServerBySocketApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERBYSOCKET_H__AD85103D_BD0B_46DC_812D_DE5605F3A7F6__INCLUDED_)
