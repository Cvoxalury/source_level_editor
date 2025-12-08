//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MDICLIENTWND_H
#define MDICLIENTWND_H
#ifdef _WIN32
#pragma once
#endif

class CMDIClientWnd : public CWnd
{
public:

	CMDIClientWnd();
	virtual ~CMDIClientWnd();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMDIClientWnd)
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(CMDIClientWnd)
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);

#ifdef SLE_DARK_THEME
	//// SLE dark theme test
	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
	{
		HBRUSH hBrush = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
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
#endif
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif // MDICLIENTWND_H