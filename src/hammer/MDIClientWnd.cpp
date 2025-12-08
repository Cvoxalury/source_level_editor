//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "stdafx.h"
#include "hammer.h"
#include "MDIClientWnd.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

BEGIN_MESSAGE_MAP(CMDIClientWnd, CWnd)
	//{{AFX_MSG_MAP(CMDIClientWnd)
	ON_WM_ERASEBKGND()
#ifdef SLE_DARK_THEME
	ON_WM_CTLCOLOR()
#endif
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CMDIClientWnd::CMDIClientWnd()
{
}

//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
CMDIClientWnd::~CMDIClientWnd()
{
}

//-----------------------------------------------------------------------------
// Purpose: Makes our background color mesh with the splash screen for maximum effect.
//-----------------------------------------------------------------------------
BOOL CMDIClientWnd::OnEraseBkgnd(CDC *pDC)
{
	//// SLE TODO: hook up to clientscheme
	// Set brush to desired background color
#ifdef SLE_DARK_THEME //// SLE NEW - dark theme test
	CBrush backBrush(SLE_DARK_THEME_CLR_BACK); // This color blends with the splash image!
#else
	CBrush backBrush(RGB(141, 136, 130)); // This color blends with the splash image!
#endif
	// Save old brush
	CBrush *pOldBrush = pDC->SelectObject(&backBrush);

	CRect rect;
	pDC->GetClipBox(&rect);     // Erase the area needed

	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);

	pDC->SelectObject(pOldBrush);
	return TRUE;
}
