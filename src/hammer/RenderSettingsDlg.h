//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef RENDERSETTINGSDLG_H
#define RENDERSETTINGSDLG_H
#ifdef _WIN32
#pragma once
#endif


#include "HammerBar.h"
#include "MapClass.h"


class CRenderSettingsDlg : public CHammerBar
{
public:
	CRenderSettingsDlg();

	bool Create(CWnd *pParentWnd);

protected:

	//{{AFX_DATA(CRenderSettingsDlg)
	enum { IDD = IDD_RENDERSETTINGS };
	CSliderCtrl		m_DistanceSlider;        // time in animation
											 //}}AFX_DATA

											 //{{AFX_MSG( CRenderSettingsDlg )
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void UpdateControl(CCmdUI *pCmdUI);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif // RENDERSETTINGSDLG_H
