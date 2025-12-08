//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The UI for simple map compiles.
//
//=============================================================================//

#include "stdafx.h"
#include "hammer.h"
#include "RunMap.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

static LPCTSTR pszSection = "Run Map";

CRunMap::CRunMap(CWnd* pParent /*=NULL*/)
	: CDialog(CRunMap::IDD, pParent)
{
	m_bSwitchMode = FALSE;

	//{{AFX_DATA_INIT(CRunMap)
	m_iVis = -1;
	m_bDoNotRunGame = FALSE;
	m_strQuakeParms = _T("");
	m_iLight = -1;
	m_iQBSP = -1;
	m_bHDRLight = FALSE;
#ifdef SLE  //// SLE NEW - extra normal compile options
	m_bBSPAllow = false;
	m_bBSPNoCubemap = false;
	m_bVRadLDR = true;
	m_bVRadTS = false;
	m_bVRadSPL = false;
	m_bVRadSPP = false;
	m_bVRadNOSR = false;
	m_bUseProcessWnd = true;
	m_strVisThreads = _T("");
	m_strVRadThreads = _T("");
#endif
	//}}AFX_DATA_INIT

	// read from ini
	CWinApp *App = AfxGetApp();
	m_iQBSP = App->GetProfileInt(pszSection, "QBSP", 1);

	m_iVis = App->GetProfileInt(pszSection, "Vis", 1);
	m_iLight = App->GetProfileInt(pszSection, "Light", 1);
	m_bHDRLight = App->GetProfileInt(pszSection, "HDRLight", 0);
	m_bDoNotRunGame = App->GetProfileInt(pszSection, "No Game", 0);
	m_strQuakeParms = App->GetProfileString(pszSection, "Game Parms", "");
#ifdef SLE  //// SLE NEW - extra normal compile options
	m_bBSPAllow = App->GetProfileInt(pszSection, "BSPAllowDynamicStatic", 0);
	m_bBSPNoCubemap = App->GetProfileInt(pszSection, "BSPNoCubemap", 0);
	m_bVRadLDR = App->GetProfileInt(pszSection, "VRadLDR", 0);
	m_bVRadTS = App->GetProfileInt(pszSection, "VRadTextureShadows", 0);
	m_bVRadSPL = App->GetProfileInt(pszSection, "VRadStaticPropLighting", 0);
	m_bVRadSPP = App->GetProfileInt(pszSection, "VRadStaticPropPolys", 0);
	m_bVRadNOSR = App->GetProfileInt(pszSection, "VRadNoSkyboxRecurse", 0);
	m_bUseProcessWnd = App->GetProfileInt(pszSection, "UseProcessWindow", 1);
	m_strVisThreads = App->GetProfileString(pszSection, "VisThreads", "");
	m_strVRadThreads = App->GetProfileString(pszSection, "VRadThreads", "");
#endif
}

void CRunMap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRunMap)
	DDX_Check(pDX, IDC_NOQUAKE, m_bDoNotRunGame);
	DDX_Text(pDX, IDC_LAUNCHGAMEPARMS, m_strQuakeParms);
	DDX_Radio(pDX, IDC_BSP0, m_iQBSP);
	DDX_Radio(pDX, IDC_VIS0, m_iVis);
	DDX_Radio(pDX, IDC_RAD0, m_iLight);
	DDX_Check(pDX, IDC_RAD_HDR, m_bHDRLight);
#ifdef SLE  //// SLE NEW - extra normal compile options
	DDX_Check(pDX, IDC_BSP_ALLOW, m_bBSPAllow);
	DDX_Check(pDX, IDC_BSP_NOCUBEMAP, m_bBSPNoCubemap);
	DDX_Check(pDX, IDC_RAD_LDR, m_bVRadLDR);
	DDX_Check(pDX, IDC_RAD_SPL, m_bVRadSPL);
	DDX_Check(pDX, IDC_RAD_SPP, m_bVRadSPP);
	DDX_Check(pDX, IDC_RAD_NOSR, m_bVRadNOSR);
	DDX_Check(pDX, IDC_RAD_TS, m_bVRadTS);
	DDX_Text(pDX,  IDC_VIS_THREADS, m_strVisThreads);
	DDX_Text(pDX,  IDC_RAD_THREADS, m_strVRadThreads);
	DDX_Check(pDX, IDC_USEPROCESSWND, m_bUseProcessWnd);
#endif
	//}}AFX_DATA_MAP
}

void CRunMap::SaveToIni(void)
{
	CWinApp *App = AfxGetApp();
	App->WriteProfileInt(pszSection, "QBSP", m_iQBSP);
	App->WriteProfileInt(pszSection, "Vis", m_iVis);
	App->WriteProfileInt(pszSection, "Light", m_iLight);
	App->WriteProfileInt(pszSection, "HDRLight", m_bHDRLight);
	App->WriteProfileInt(pszSection, "No Game", m_bDoNotRunGame);
	App->WriteProfileString(pszSection, "Game Parms", m_strQuakeParms);
#ifdef SLE  //// SLE NEW - extra normal compile options
	App->WriteProfileInt(pszSection, "BSPAllowDynamicStatic", m_bBSPAllow);
	App->WriteProfileInt(pszSection, "BSPNoCubemap", m_bBSPNoCubemap);
	App->WriteProfileInt(pszSection, "VRadLDR", m_bVRadLDR);
	App->WriteProfileInt(pszSection, "VRadTextureShadows", m_bVRadTS);
	App->WriteProfileInt(pszSection, "VRadStaticPropLighting", m_bVRadSPL);
	App->WriteProfileInt(pszSection, "VRadStaticPropPolys", m_bVRadSPP);
	App->WriteProfileInt(pszSection, "VRadNoSkyboxRecurse", m_bVRadNOSR);
	App->WriteProfileInt(pszSection, "UseProcessWindow", m_bUseProcessWnd);
	App->WriteProfileString(pszSection, "VisThreads", m_strVisThreads);
	App->WriteProfileString(pszSection, "VRadThreads", m_strVRadThreads);
#endif
}

BEGIN_MESSAGE_MAP(CRunMap, CDialog)
	//{{AFX_MSG_MAP(CRunMap)
	ON_BN_CLICKED(IDC_EXPERT, OnExpert)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRunMap message handlers

void CRunMap::OnExpert() 
{
	m_bSwitchMode = TRUE;
	EndDialog(IDOK);
}

BOOL CRunMap::OnInitDialog() 
{
	CDialog::OnInitDialog();
	return TRUE;
}
