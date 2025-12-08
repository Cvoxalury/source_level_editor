#if !defined(AFX_MYBUTTON_H__46A1ECCC_0FAD_485A_B6B8_C21B6538148E__INCLUDED_)
#define AFX_MYBUTTON_H__46A1ECCC_0FAD_485A_B6B8_C21B6538148E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "afxwin.h"

class CButtonContext : public CButton
{
public:
	CButtonContext();

public:
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyButton)
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CButtonContext();

	// Generated message map functions
protected:
	//{{AFX_MSG(CButtonContext)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_MYBUTTON_H__46A1ECCC_0FAD_485A_B6B8_C21B6538148E__INCLUDED_)
