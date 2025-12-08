//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <stdafx.h>
#include "GlobalFunctions.h"
#include "MainFrm.h"
#include "RenderSettingsDlg.h"
#include "MapDoc.h"
#include "hammer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgoff.h>


BEGIN_MESSAGE_MAP(CRenderSettingsDlg, CHammerBar)
	//{{AFX_MSG_MAP( CRenderSettingsDlg )
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CRenderSettingsDlg::CRenderSettingsDlg()
{
}

static const int SLIDER_NUMTICS = 100;

bool CRenderSettingsDlg::Create(CWnd *pParentWnd)
{
	//
	// create a modeless dialog toolbar
	//
	if (!(CHammerBar::Create(pParentWnd, IDD, CBRS_RIGHT, IDCB_RENDERSETTINGS, "View Settings Bar")))
	{
		return false;
	}

	// to remain consistant with the other toolbars in the editor
	SetWindowText(_T("View Settings"));

	// set dialog bar style
	SetBarStyle(GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);

	// enable docking
	EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_RIGHT);

	//
	// initialize the dialog items
	//
	m_DistanceSlider.SubclassDlgItem(IDC_BACKPLANE, this);
	m_DistanceSlider.SetRange(0, SLIDER_NUMTICS);
	m_DistanceSlider.SetTicFreq(SLIDER_NUMTICS / 4);
	m_DistanceSlider.SetPos(0);
	m_DistanceSlider.EnableWindow(true);

	// show the dialog
	ShowWindow(SW_SHOW);

	// created successfully
	return true;
}

void CRenderSettingsDlg::UpdateControl(CCmdUI *pCmdUI)
{
	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	if (!pDoc )
	{
		pCmdUI->Enable(false);
		return;
	}
	else
	{
		pCmdUI->Enable(true);
	}
}

void CRenderSettingsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	CHammerBar::OnHScroll(nSBCode, nPos, pScrollBar);
}
