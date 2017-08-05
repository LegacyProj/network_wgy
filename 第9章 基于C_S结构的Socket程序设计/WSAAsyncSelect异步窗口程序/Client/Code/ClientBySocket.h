// ClientBySocket.h : main header file for the CLIENTBYSOCKET application
//

#if !defined(AFX_CLIENTBYSOCKET_H__E21B98D5_6E7A_4471_8318_4582DA295B31__INCLUDED_)
#define AFX_CLIENTBYSOCKET_H__E21B98D5_6E7A_4471_8318_4582DA295B31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CClientBySocketApp:
// See ClientBySocket.cpp for the implementation of this class
//

class CClientBySocketApp : public CWinApp
{
public:
	CClientBySocketApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClientBySocketApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CClientBySocketApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENTBYSOCKET_H__E21B98D5_6E7A_4471_8318_4582DA295B31__INCLUDED_)
