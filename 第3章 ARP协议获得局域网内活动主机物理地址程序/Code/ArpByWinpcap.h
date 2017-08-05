// ArpByWinpcap.h : main header file for the ARPBYWINPCAP application
//

#if !defined(AFX_ARPBYWINPCAP_H__5D2F6FC4_7CCA_40B4_B91C_D81286CBE7FB__INCLUDED_)
#define AFX_ARPBYWINPCAP_H__5D2F6FC4_7CCA_40B4_B91C_D81286CBE7FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CArpByWinpcapApp:
// See ArpByWinpcap.cpp for the implementation of this class
//

class CArpByWinpcapApp : public CWinApp
{
public:
	CArpByWinpcapApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CArpByWinpcapApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CArpByWinpcapApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ARPBYWINPCAP_H__5D2F6FC4_7CCA_40B4_B91C_D81286CBE7FB__INCLUDED_)
