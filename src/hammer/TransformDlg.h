//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#if !defined(AFX_TRANSFORMDLG_H__B21B769F_F316_11D0_AFA6_0060979D2F4E__INCLUDED_)
#define AFX_TRANSFORMDLG_H__B21B769F_F316_11D0_AFA6_0060979D2F4E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TransformDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTransformDlg dialog

class CTransformDlg : public CDialog
{
// Construction
public:
	CTransformDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTransformDlg)
	enum { IDD = IDD_TRANSFORM };
	int		m_iMode;
#ifdef SLE //// SLE NEW - more transform dialogue controls
	BOOL	m_bUseLocalTransform; //// SLE NEW - scale/rotate/move in local (entity's own) 
	BOOL	m_bScaleModelEntities; //// SLE NEW - relay transform scale to modelscale keyvalues
	BOOL	m_bLockTextureScale;
	CButton m_cLockTexScale;

	float	m_modelScale_float;
#endif
	float	m_X;
	float	m_Y;
	float	m_Z;
	//}}AFX_DATA
	
	void SaveValues();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTransformDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
#ifdef SLE
	virtual BOOL OnInitDialog(void);
#endif
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTransformDlg)
#ifdef SLE //// SLE NEW - overhauled transform dialogue
	afx_msg void OnButtonReset();//// SLE NEW - reset values to 0 for transform dialogue
	afx_msg void OnButtonCalculateSkyScale(); //// SLE NEW - calculate scale from sky camera's value
	afx_msg void OnButtonLockTexScale(); //// SLE NEW - toggle texture scale lock from the dlg

#ifdef SLE_DARK_THEME
	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
	{
		HBRUSH hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
		pDC->SetBkColor(SLE_DARK_THEME_CLR_BACK); 
		static CBrush statBrush(SLE_DARK_THEME_CLR_BACK); // this is the brush that will be returned to avoid static text's unfilled portion's color "leaking".
		static CBrush editBrush(SLE_DARK_THEME_CLR_EDIT_BACK); // this is the brush that will be returned to avoid edit text's unfilled portion's color "leaking".

		switch (nCtlColor)
		{
		case CTLCOLOR_STATIC: // text of labels
			pDC->SetTextColor(SLE_DARK_THEME_CLR_TEXT);
			hBrush = statBrush;
			return hBrush;
		case CTLCOLOR_EDIT: // text of entered values
			pDC->SetTextColor(SLE_DARK_THEME_CLR_EDIT_TEXT);
			pDC->SetBkColor(SLE_DARK_THEME_CLR_EDIT_BACK);
			hBrush = editBrush;
			return hBrush;
		case CTLCOLOR_BTN: // text of buttons
			pDC->SetTextColor(SLE_DARK_THEME_CLR_TEXT);
			pDC->SetBkColor(SLE_DARK_THEME_CLR_BTN);
			return hBrush;
		case CTLCOLOR_LISTBOX: // text of buttons
			pDC->SetTextColor(SLE_DARK_THEME_CLR_TEXT);
			pDC->SetBkColor(SLE_DARK_THEME_CLR_EDIT_BACK);
			hBrush = editBrush;
			return hBrush;
		default:
			return hBrush;
		}
	}

	afx_msg BOOL OnEraseBkgnd(CDC* pDC)
	{
		CBrush backBrush(SLE_DARK_THEME_CLR_BACK); // This color blends with the splash image!
		// Save old brush
		CBrush *pOldBrush = pDC->SelectObject(&backBrush);

		CRect rect;
		pDC->GetClipBox(&rect);     // Erase the area needed

		pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);

		pDC->SelectObject(pOldBrush);
		return TRUE;
	}
#endif
#endif //// SLE
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSFORMDLG_H__B21B769F_F316_11D0_AFA6_0060979D2F4E__INCLUDED_)
