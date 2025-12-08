//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#if !defined(AFX_SCALEVERTICESDLG_H__1E50C989_FEEB_11D0_AFA8_0060979D2F4E__INCLUDED_)
#define AFX_SCALEVERTICESDLG_H__1E50C989_FEEB_11D0_AFA8_0060979D2F4E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ScaleVerticesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScaleVerticesDlg dialog

class CScaleVerticesDlg : public CDialog
{
// Construction
public:
	CScaleVerticesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CScaleVerticesDlg)
	enum { IDD = IDD_SCALEVERTICES };
#ifdef SLE //// SLE NEW - better vertex tool scaling
	CSpinButtonCtrl	m_cTransformXSpin;
	CSpinButtonCtrl	m_cTransformYSpin;
	CSpinButtonCtrl	m_cTransformZSpin;
	CEdit m_cTransformX;
	CEdit m_cTransformY;
	CEdit m_cTransformZ;

	void SaveValues();
#else
	CSpinButtonCtrl	m_cScaleSpin;
	CEdit	m_cScale;
#endif
	//}}AFX_DATA

#ifdef SLE //// SLE NEW - better vertex tool scaling
	int m_iMode;
	int m_idummy;
	float m_fTransformX;
	float m_fTransformY;
	float m_fTransformZ;
#else
	float m_fScale;
#endif
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScaleVerticesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CScaleVerticesDlg)
#ifdef SLE //// SLE NEW - better vertex tool scaling
	afx_msg void OnChangeTransformX();
	afx_msg void OnChangeTransformY();
	afx_msg void OnChangeTransformZ();
	afx_msg void OnDeltaposTransformXSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposTransformYSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposTransformZSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMode(void);
	afx_msg void OnCancel();
#else
	afx_msg void OnChangeScale();
	afx_msg void OnDeltaposScalespin(NMHDR* pNMHDR, LRESULT* pResult);
#endif
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCALEVERTICESDLG_H__1E50C989_FEEB_11D0_AFA8_0060979D2F4E__INCLUDED_)