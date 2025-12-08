//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// EditGameConfigs.cpp : implementation file
//

#include "stdafx.h"
#include "hammer.h"
#include "EditGameConfigs.h"
#include "StrDlg.h"
#include "MapDoc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

/////////////////////////////////////////////////////////////////////////////
// CEditGameConfigs dialog

CEditGameConfigs::CEditGameConfigs(BOOL bSelectOnly, 
								   CWnd* pParent /*=NULL*/)
	: CDialog(CEditGameConfigs::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditGameConfigs)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bSelectOnly = bSelectOnly;
}

void CEditGameConfigs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditGameConfigs)
	DDX_Control(pDX, IDC_CONFIGS, m_cConfigs);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEditGameConfigs, CDialog)
	//{{AFX_MSG_MAP(CEditGameConfigs)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_LBN_SELCHANGE(IDC_CONFIGS, OnSelchangeConfigs)
	ON_LBN_DBLCLK(IDC_CONFIGS, OnDblclkConfigs)
#ifdef SLE
	ON_BN_CLICKED(IDC_OPEN_SOURCE, OnOpenSource) //// SLE NEW - button to open game configs folder
	ON_BN_CLICKED(IDC_RENAME, OnRename) //// SLE NEW - allow renaming of existing configs
#endif
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditGameConfigs message handlers

void CEditGameConfigs::OnAdd() 
{
	char szName[128];
	szName[0] = 0;

#ifdef SLE //// SLE NEW - don't allow empty names (and it was allowed!)
	CStrDlg dlg(0, "", "Enter the game's name:", "Add a game");
	if (dlg.DoModal() == IDOK)
	{
		if (strlen(dlg.m_string) == 0)
		{
			AfxMessageBox("The game's name needs to be longer than 0 characters.");
			return;
		}
		else
		{
			strcpy(szName, dlg.m_string);
		}
	}
	else
		return;
#else
	CStrDlg dlg(0, szName, "Enter the game's name:", "Add a game");
	if (dlg.DoModal() != IDOK)
		return;
#endif

	// add a new game config
	CGameConfig *pConfig = Options.configs.AddConfig();
	strcpy(pConfig->szName, dlg.m_string);

	FillConfigList(pConfig->dwID);
}

void CEditGameConfigs::OnCopy() 
{
	int iCurSel = m_cConfigs.GetCurSel();
	if(iCurSel == CB_ERR)
		return;

	CGameConfig *pConfig = Options.configs.FindConfig(
		m_cConfigs.GetItemData(iCurSel));

	CGameConfig *pNewConfig = Options.configs.AddConfig();
	pNewConfig->CopyFrom(pConfig);
#ifdef SLE //// SLE CHANGE - automatically rename copied configs
	strcat(pNewConfig->szName, " (Copy)");
#endif
	FillConfigList(pNewConfig->dwID);
}

void CEditGameConfigs::OnRemove() 
{
	int iCurSel = m_cConfigs.GetCurSel();
	if(iCurSel == CB_ERR)
		return;

	int iArrayIndex;
	CGameConfig *pConfig = Options.configs.FindConfig(
		m_cConfigs.GetItemData(iCurSel), &iArrayIndex);

	// check to see if any docs use this game - if so, can't
	//  delete it.
	for ( int i=0; i<CMapDoc::GetDocumentCount(); i++ )
	{
		CMapDoc *pDoc = CMapDoc::GetDocument(i);
		if(pDoc->GetGame() == pConfig)
		{
			AfxMessageBox("You can't delete this game configuration now\n"
				"because some loaded documents are using it.\n"
				"If you want to delete it, you must close those\n"
				"documents first.");
			return;
		}
	}

	bool bResetDefaults = false;

	// Check to see if this is the last configuation and prompt for the user to make a decision
	if ( Options.configs.nConfigs <= 1 )
	{
		if ( AfxMessageBox( "At least one configuration must be present!\n"
							"Would you like to reset to the default configurations?", MB_YESNO ) == IDNO )
		{
			return;
		}

		bResetDefaults = true;
	}

#ifdef SLE //// SLE CHANGE - prompt the user for confirmation of removal
	CString str;
	str.Format("Remove the config %s?", pConfig->szName);
	if ( MessageBox(str, "Confirm removal", MB_YESNO | MB_ICONEXCLAMATION) == IDNO )
		return;
#endif

	// Remove selection
	m_cConfigs.DeleteString( iCurSel );

	// FIXME: This will apply the change even if you cancel the dialog.  This needs to store a copy
	//		  of the data which then reconciles the two versions on OK or Apply! -- jdw

	Options.configs.Configs.RemoveAt(iArrayIndex);
	Options.configs.nConfigs--;

	// Reset to defaults
	if ( bResetDefaults )
	{
		Options.configs.ResetGameConfigs( false );
		FillConfigList();
	}

	// Put the selection back to the top
	m_cConfigs.SetCurSel( 0 );
}

void CEditGameConfigs::FillConfigList(DWORD dwSelectID)
{
	// get current selection so we can keep it
	DWORD dwCurID = dwSelectID;
	int iNewIndex = -1;
	
	if(m_cConfigs.GetCurSel() != LB_ERR && dwCurID == 0xFFFFFFFF)
	{
		dwCurID = m_cConfigs.GetItemData(m_cConfigs.GetCurSel());
	}

	m_cConfigs.ResetContent();

	for(int i = 0; i < Options.configs.nConfigs; i++)
	{
		CGameConfig *pConfig = Options.configs.Configs[i];
#ifdef SLE //// SLE NEW - display mod folder in the config list when selecting them // undone, names get long very fast
	//	char configName[128];
	//	strcpy(configName, pConfig->szName);
	//	strcat(configName, " (");
	//	strcat(configName, pConfig->m_szModDir);
	//	strcat(configName, " )");
#endif
		int iIndex = m_cConfigs.AddString(pConfig->szName /*configName*/);
		m_cConfigs.SetItemData(iIndex, pConfig->dwID);

		if (dwCurID == pConfig->dwID)
		{
			iNewIndex = iIndex;
		}
	}

	if (iNewIndex == -1)
	{
		iNewIndex = 0;
	}
	m_cConfigs.SetCurSel(iNewIndex);

	OnSelchangeConfigs();

	if (m_bSelectOnly && Options.configs.nConfigs == 1)
		OnOK();
}

void CEditGameConfigs::OnSelchangeConfigs() 
{
	int iCurSel = m_cConfigs.GetCurSel();
	if(iCurSel == LB_ERR)
		return;

	m_pSelectedGame = Options.configs.FindConfig(
		m_cConfigs.GetItemData(iCurSel));
}
#ifdef SLE 
//// SLE NEW - button to open game configs folder
void CEditGameConfigs::OnOpenSource()
{
	static char dir[ MAX_PATH ] = "";	
	APP()->GetDirectory(DIR_PROGRAM, dir);
	strcat(dir, "level_editor\\cfg\\");

	CString file = "/select, ";
	file += dir;

	ShellExecute(NULL, "open", "explorer", dir, NULL, SW_SHOWNORMAL);
}
//// SLE NEW - allow renaming of existing configs
void CEditGameConfigs::OnRename()
{	
	int iCurSel = m_cConfigs.GetCurSel();
	if(iCurSel == CB_ERR)
		return;

	CGameConfig *pConfig = Options.configs.FindConfig(
		m_cConfigs.GetItemData(iCurSel));

	char szName[128];
	szName[0] = 0;
	CStrDlg dlg(0, szName, "Enter the new name:", "Rename a configuration");
	if(dlg.DoModal() != IDOK)
		return;

	strcpy(pConfig->szName, dlg.m_string);

	FillConfigList(pConfig->dwID);
}
#endif
BOOL CEditGameConfigs::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if(m_bSelectOnly)
	{
		SetWindowText("Select a game configuration to use");

		GetDlgItem(IDOK)->SetWindowText("OK");
		GetDlgItem(IDC_REMOVE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ADD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COPY)->ShowWindow(SW_HIDE);
#ifdef SLE
		GetDlgItem(IDC_RENAME)->ShowWindow(SW_HIDE);
#endif
	}

	FillConfigList();

	return TRUE;
}

void CEditGameConfigs::OnDblclkConfigs() 
{
	OnOK();	
}
