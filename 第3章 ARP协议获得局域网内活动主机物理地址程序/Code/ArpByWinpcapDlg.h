// ArpByWinpcapDlg.h : header file
//

#if !defined(AFX_ARPBYWINPCAPDLG_H__221CC83F_8F4B_4DE4_9DDA_D1BB4ECE6698__INCLUDED_)
#define AFX_ARPBYWINPCAPDLG_H__221CC83F_8F4B_4DE4_9DDA_D1BB4ECE6698__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CArpByWinpcapDlg dialog

class CArpByWinpcapDlg : public CDialog
{
// Construction
public:
	CArpByWinpcapDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CArpByWinpcapDlg)
	enum { IDD = IDD_ARPBYWINPCAP_DIALOG };
	CListBox	m_Mac_list;
	CListBox	m_Dev;
	int		m_Dev_No;
	CString	m_sending;
	CString	m_count;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CArpByWinpcapDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CArpByWinpcapDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnGetDev();
	afx_msg void OnGetMac();
	afx_msg void OnSelChangeDevList();
	afx_msg void OnStopCap();
	afx_msg LRESULT OnPacket(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ARPBYWINPCAPDLG_H__221CC83F_8F4B_4DE4_9DDA_D1BB4ECE6698__INCLUDED_)
