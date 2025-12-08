//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef MAPCHECKDLG_H
#define MAPCHECKDLG_H
#ifdef _WIN32
#pragma once
#endif

#include "mapdoc.h"
#ifdef SLE //// SLE CHANGE - make Map Problems resizable
#include "AnchorMgr.h"
#endif
struct MapError;
class CMapClass;

class CMapCheckDlg : public CDialog
{
public:
	static void CheckForProblems(CWnd *pwndParent);

private:
	CMapCheckDlg(CWnd *pParent = NULL);
	enum { IDD = IDD_MAPCHECK };
	bool DoCheck();
	
protected:
	//{{AFX_DATA(CMapCheckDlg)
	CButton	m_cFixAll;
	CButton	m_Fix;
	CButton	m_Go;
	CEdit	m_Description;
	CListBox	m_Errors;
	BOOL	m_bCheckVisible;
#ifdef SLE //// SLE CHANGE - make Map Problems resizable
	virtual BOOL OnInitDialog();
#endif
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapCheckDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CMapCheckDlg)
	afx_msg void OnGo();
	afx_msg void OnSelchangeErrors();
	afx_msg void OnDblClkErrors();
	afx_msg void OnPaint();
	afx_msg void OnFix();
	afx_msg void OnFixall();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnCheckVisibleOnly();
#ifdef SLE //// SLE CHANGE - make Map Problems resizable
	afx_msg void OnSize(UINT nType, int cx, int cy);
#endif
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
#ifdef SLE //// SLE CHANGE - make Map Problems resizable
	void UpdateAnchors(CWnd *pPage);
	CAnchorMgr m_AnchorMgr;
#endif

private:
	void Fix(MapError *pError, UpdateBox &ub);
	void KillErrorList();
	void GotoSelectedErrors();
};

#endif // MAPCHECKDLG_H