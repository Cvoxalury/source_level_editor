//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "stdafx.h"
#include <shlobj.h>
#include "GameConfig.h"
#include "EditGameConfigs.h"
#include "hammer.h"
#include "OPTConfigs.h"
#include "ConfigManager.h"
#include "process.h"
#include "Options.h"
#include "TextureBrowser.h"

#include "tier2/vconfig.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

IMPLEMENT_DYNCREATE(COPTConfigs, CPropertyPage)

//-----------------------------------------------------------------------------
// Purpose: Returns the string value of a registry key
// Input  : *pName - name of the subKey to read
//			*pReturn - string buffer to receive read string
//			size - size of specified buffer
//-----------------------------------------------------------------------------
bool GetPersistentEnvironmentVariable( const char *pName, char *pReturn, int size )
{
	// Open the key
	HKEY hregkey; 
	// Changed to HKEY_CURRENT_USER from HKEY_LOCAL_MACHINE
	if ( RegOpenKeyEx( HKEY_CURRENT_USER, VPROJECT_REG_KEY, 0, KEY_QUERY_VALUE, &hregkey ) != ERROR_SUCCESS )
		return false;
	
	// Get the value
	DWORD dwSize = size;
	if ( RegQueryValueEx( hregkey, pName, NULL, NULL,(LPBYTE) pReturn, &dwSize ) != ERROR_SUCCESS )
		return false;
	
	// Close the key
	RegCloseKey( hregkey );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Set the registry entry to a string value, under the given subKey
// Input  : *pName - name of the subKey to set
//			*pValue - string value
//-----------------------------------------------------------------------------
void SetPersistentEnvironmentVariable( const char *pName, const char *pValue )
{
	HKEY hregkey; 
	DWORD dwReturnValue = 0;

	// Changed from HKEY_LOCAL_MACHINE to HKEY_CURRENT_USER
	if ( RegOpenKeyEx( HKEY_CURRENT_USER, VPROJECT_REG_KEY, 0, KEY_ALL_ACCESS, &hregkey ) != ERROR_SUCCESS )
		return;
	
	// Set the value to the string passed in
	RegSetValueEx( hregkey, pName, 0, REG_SZ, (const unsigned char *)pValue, (int) strlen(pValue) );
	
	// Propagate changes so that environment variables takes immediate effect!
	SendMessageTimeout( HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", SMTO_ABORTIFHUNG, 5000, &dwReturnValue );

	// Close the key
	RegCloseKey( hregkey );
}

//-----------------------------------------------------------------------------
// Purpose: Converts a Steam path prefix to %STEAM% and back.
//-----------------------------------------------------------------------------
void EditorUtil_ConvertPath(CString &str, bool bExpand)
{
	CString strSteamDir;
	g_pGameConfig->GetSteamDir(strSteamDir);

	CString strSteamUserDir;
	g_pGameConfig->GetSteamUserDir(strSteamUserDir);

	if (!strSteamDir.GetLength() && !strSteamUserDir.GetLength())
	{
		// Can't do the token replacement.
		return;
	}

	const char *szSteamDirToken = "$SteamDir";
	const char *szSteamUserDirToken = "$SteamUserDir";

	char szPathOut[MAX_PATH];

	if (bExpand)
	{
		// Replace the tokens with the full strings
		if (Q_StrSubst(str, szSteamUserDirToken, strSteamUserDir, szPathOut, sizeof(szPathOut)))
		{
			str = szPathOut;
		}

		if (Q_StrSubst(str, szSteamDirToken, strSteamDir, szPathOut, sizeof(szPathOut)))
		{
			str = szPathOut;
		}
	}
	else
	{
		// Replace the full strings with the tokens
		// Go from longest paths to shortest paths to insure the most brief expression.
		if (Q_StrSubst(str, strSteamUserDir, szSteamUserDirToken, szPathOut, sizeof(szPathOut)))
		{
			str = szPathOut;
		}

		if (Q_StrSubst(str, strSteamDir, szSteamDirToken, szPathOut, sizeof(szPathOut)))
		{
			str = szPathOut;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Exchanges path data between a CString and an edit control, converting
//			the Steam path to %STEAM% and back.
//-----------------------------------------------------------------------------
void EditorUtil_TransferPath(CDialog *pDlg, int nIDC, char *szDest, bool bExpand)
{
	CWnd *pwnd = pDlg->GetDlgItem(nIDC);
	if (!pwnd)
		return;

	CString str;

	if (bExpand)
	{
		pwnd->GetWindowText(str);
		EditorUtil_ConvertPath(str, true);
		strcpy(szDest, str);
	}
	else
	{
		str = szDest;
		EditorUtil_ConvertPath(str, false);
		pwnd->SetWindowText(str);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
COPTConfigs::COPTConfigs(void) : CPropertyPage(COPTConfigs::IDD)
{
	//{{AFX_DATA_INIT(COPTConfigs)
	//}}AFX_DATA_INIT

	m_pLastSelConfig = NULL;
	m_pInitialSelectedConfig = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
COPTConfigs::~COPTConfigs(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pDX - 
//-----------------------------------------------------------------------------
void COPTConfigs::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COPTConfigs)
	DDX_Control(pDX, IDC_MAPDIR, m_cMapDir);
	DDX_Control(pDX, IDC_GAMEEXEDIR, m_cGameExeDir);
	DDX_Control(pDX, IDC_MODDIR, m_cModDir);
	DDX_Control(pDX, IDC_MAPFORMAT, m_cMapFormat);
	DDX_Control(pDX, IDC_CORDON_TEXTURE, m_cCordonTexture);
	DDX_Control(pDX, IDC_TEXTUREFORMAT, m_cTextureFormat);
	DDX_Control(pDX, IDC_DEFAULTPOINT, m_cDefaultPoint);
	DDX_Control(pDX, IDC_DEFAULTENTITY, m_cDefaultSolid);
	DDX_Control(pDX, IDC_DATAFILES, m_cGDFiles);
	DDX_Control(pDX, IDC_CONFIGURATIONS, m_cConfigs);
	DDX_Control(pDX, IDC_DEFAULT_TEXTURE_SCALE, m_cDefaultTextureScale);
#ifdef SLE //// SLE NEW - quick access class buttons for setting current entity's class
	DDX_Control(pDX, IDC_ENTITY_PRESET1_CLASS, m_cQuickAccessClass1);
	DDX_Control(pDX, IDC_ENTITY_PRESET2_CLASS, m_cQuickAccessClass2);
	DDX_Control(pDX, IDC_ENTITY_PRESET3_CLASS, m_cQuickAccessClass3);
	DDX_Control(pDX, IDC_ENTITY_PRESET4_CLASS, m_cQuickAccessClass4);
#endif
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COPTConfigs, CPropertyPage)
	//{{AFX_MSG_MAP(COPTConfigs)
	ON_BN_CLICKED(IDC_EDITCONFIGS, OnEditconfigs)
	ON_BN_CLICKED(IDC_GDFILE_ADD, OnGdfileAdd)
	ON_BN_CLICKED(IDC_GDFILE_EDIT, OnGdfileEdit)
	ON_BN_CLICKED(IDC_GDFILE_REMOVE, OnGdfileRemove)
	ON_CBN_SELCHANGE(IDC_CONFIGURATIONS, OnSelchangeConfigurations)
	ON_BN_CLICKED(IDC_BROWSEMAPDIR, OnBrowsemapdir)
	ON_BN_CLICKED(IDC_BROWSEGAMEEXEDIR, OnBrowseGameExeDir)
	ON_BN_CLICKED(IDC_BROWSEMODDIR, OnBrowseModDir)
	ON_BN_CLICKED(IDC_BROWSE_CORDON_TEXTURE, OnBrowseCordonTexture)
	ON_MESSAGE( WM_SETTINGCHANGE, OnSettingChange )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void COPTConfigs::OnEditconfigs(void)
{
	SaveInfo(m_pLastSelConfig);

	CEditGameConfigs dlg;
	dlg.DoModal();
	UpdateConfigList();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COPTConfigs::OnGdfileAdd(void)
{
	if (m_pLastSelConfig == NULL)
		return;

	char szAppDir[MAX_PATH];
	APP()->GetDirectory(DIR_PROGRAM, szAppDir);

	// browse for .FGD files
	CFileDialog dlg(TRUE, ".fgd", NULL, OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST, "Game Data Files (*.fgd)|*.fgd||");
	dlg.m_ofn.lpstrInitialDir = szAppDir;
	if (dlg.DoModal() != IDOK)
		return;

	CString str = dlg.GetPathName();
	EditorUtil_ConvertPath(str, false);
	m_cGDFiles.AddString(str);

	SaveInfo(m_pLastSelConfig);
	m_pLastSelConfig->LoadGDFiles();
	UpdateEntityLists();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COPTConfigs::OnGdfileEdit(void)
{
	if(m_pLastSelConfig == NULL)
		return;

	// edit the selected FGD file
	int iCurSel = m_cGDFiles.GetCurSel();
	if(iCurSel == LB_ERR)
		return;

	CString str;
	m_cGDFiles.GetText(iCurSel, str);
	EditorUtil_ConvertPath(str, true);

#ifdef SLE //// SLE NEW: Changed method so that opening FGD for edit calls the default text editor (e. g. Notepad++), instead of hardcoded Notepad.exe
	ShellExecute(NULL, "open", str, NULL, NULL, SW_SHOWNORMAL);
#else
	// call editor (notepad!)
	//	char szBuf[MAX_PATH];
	//	GetWindowsDirectory(szBuf, MAX_PATH);
	//	strcat(szBuf, "\\notepad.exe");
	//	_spawnl(_P_WAIT, szBuf, szBuf, str, NULL);
#endif
	m_pLastSelConfig->LoadGDFiles();
	UpdateEntityLists();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COPTConfigs::OnGdfileRemove(void)
{
	if(m_pLastSelConfig == NULL)
		return;

	int iCurSel = m_cGDFiles.GetCurSel();
	if(iCurSel == LB_ERR)
		return;

	m_cGDFiles.DeleteString(iCurSel);

	SaveInfo(m_pLastSelConfig);
	m_pLastSelConfig->LoadGDFiles();
	UpdateEntityLists();
}
#ifdef SLE //// SLE NEW: quick fgd reload button
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void COPTConfigs::OnGdfileReload(void)
{
	if (m_pLastSelConfig == NULL)
	{
		return;
	}

	m_pLastSelConfig->LoadGDFiles();
	UpdateEntityLists();
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pConfig - 
//-----------------------------------------------------------------------------
void COPTConfigs::SaveInfo(CGameConfig *pConfig)
{
	if (pConfig == NULL)
	{
		return;
	}

	// game data files
	pConfig->nGDFiles = 0;
	pConfig->GDFiles.RemoveAll();
	int iSize = m_cGDFiles.GetCount();
	for(int i = 0; i < iSize; i++)
	{
		CString str;
		m_cGDFiles.GetText(i, str);

		EditorUtil_ConvertPath(str, true);

		pConfig->GDFiles.Add(str);
		++pConfig->nGDFiles;
	}

	//
	// Map file type.
	//
	int nIndex = m_cMapFormat.GetCurSel();
	if (nIndex != CB_ERR)
	{
		pConfig->mapformat = (MAPFORMAT)m_cMapFormat.GetItemData(nIndex);
	}
	else
	{
		pConfig->mapformat = mfHalfLife2;
	}

	//
	// WAD file type.
	//
	nIndex = m_cTextureFormat.GetCurSel();
	if (nIndex != CB_ERR)
	{
		pConfig->SetTextureFormat((TEXTUREFORMAT)m_cTextureFormat.GetItemData(nIndex));
	}
	else
	{
		pConfig->SetTextureFormat(tfVMT);
	}

	m_cDefaultSolid.GetWindowText(pConfig->szDefaultSolid, sizeof pConfig->szDefaultSolid);
	m_cDefaultPoint.GetWindowText(pConfig->szDefaultPoint, sizeof pConfig->szDefaultPoint);
#ifdef SLE //// SLE NEW - quick access class buttons for setting current entity's class
	m_cQuickAccessClass1.GetWindowText(pConfig->szQuickAccessClass1, sizeof pConfig->szQuickAccessClass1);
	m_cQuickAccessClass2.GetWindowText(pConfig->szQuickAccessClass2, sizeof pConfig->szQuickAccessClass2);
	m_cQuickAccessClass3.GetWindowText(pConfig->szQuickAccessClass3, sizeof pConfig->szQuickAccessClass3);
	m_cQuickAccessClass4.GetWindowText(pConfig->szQuickAccessClass4, sizeof pConfig->szQuickAccessClass4);
#endif
	EditorUtil_TransferPath(this, IDC_GAMEEXEDIR, pConfig->m_szGameExeDir, true);
	EditorUtil_TransferPath(this, IDC_MODDIR, pConfig->m_szModDir, true);
	EditorUtil_TransferPath(this, IDC_MAPDIR, pConfig->szMapDir, true);

	char szCordonTexture[MAX_PATH];
	m_cCordonTexture.GetWindowText(szCordonTexture, sizeof(szCordonTexture));
	pConfig->SetCordonTexture(szCordonTexture);

	//
	// Default texture scale.
	//
	char szText[100];
	m_cDefaultTextureScale.GetWindowText(szText, sizeof(szText));
	float fScale = (float)atof(szText);
	if (fScale == 0)
	{
		fScale = 1;
	}
	pConfig->SetDefaultTextureScale(fScale);

	//
	// Default lightmap scale.
	//
	int nLightmapScale = GetDlgItemInt(IDC_DEFAULT_LIGHTMAP_SCALE, NULL, FALSE);
	pConfig->SetDefaultLightmapScale(nLightmapScale);
#ifdef SLE
	m_pLastSelConfig->LoadGDFiles();
	UpdateEntityLists();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COPTConfigs::OnSelchangeConfigurations(void) 
{
	// save info from controls into last selected config
	SaveInfo(m_pLastSelConfig);

	m_pLastSelConfig = NULL;

	// load info from newly selected config into controls
	int iCurSel = m_cConfigs.GetCurSel();
	CGameConfig *pConfig = Options.configs.FindConfig(m_cConfigs.GetItemData(iCurSel));

	BOOL bKillFields = FALSE;
	if (pConfig == NULL)
	{
		bKillFields = TRUE;
	}

	m_cGDFiles.EnableWindow(!bKillFields);
	m_cDefaultPoint.EnableWindow(!bKillFields);
	m_cDefaultSolid.EnableWindow(!bKillFields);
#ifdef SLE //// SLE NEW - quick access class buttons for setting current entity's class
	m_cQuickAccessClass1.EnableWindow(!bKillFields);
	m_cQuickAccessClass2.EnableWindow(!bKillFields);
	m_cQuickAccessClass3.EnableWindow(!bKillFields);
	m_cQuickAccessClass4.EnableWindow(!bKillFields);
#endif
	m_cTextureFormat.EnableWindow(!bKillFields);
	m_cMapFormat.EnableWindow(!bKillFields);
	m_cGameExeDir.EnableWindow(!bKillFields);
	m_cModDir.EnableWindow(!bKillFields);
	m_cMapDir.EnableWindow(!bKillFields);
	m_cCordonTexture.EnableWindow(!bKillFields);

	if (pConfig == NULL)
	{
		return;
	}

	m_pLastSelConfig = pConfig;

	// load game data files
	m_cGDFiles.ResetContent();
	for (int i = 0; i < pConfig->nGDFiles; i++)
	{
		CString str(pConfig->GDFiles[i]);
		EditorUtil_ConvertPath(str, false);
		m_cGDFiles.AddString(str);
	}

	//
	// Select the correct map format.
	//
	m_cMapFormat.SetCurSel(0);
	int nItems = m_cMapFormat.GetCount();
	for (int i = 0; i < nItems; i++)
	{
		if (pConfig->mapformat == (MAPFORMAT)m_cMapFormat.GetItemData(i))
		{
			m_cMapFormat.SetCurSel(i);
			break;
		}
	}

	//
	// Select the correct texture format.
	//
	m_cTextureFormat.SetCurSel(0);
	nItems = m_cTextureFormat.GetCount();
	for (int i = 0; i < nItems; i++)
	{
		if (pConfig->GetTextureFormat() == (TEXTUREFORMAT)m_cTextureFormat.GetItemData(i))
		{
			m_cTextureFormat.SetCurSel(i);
			break;
		}
	}

	EditorUtil_TransferPath(this, IDC_GAMEEXEDIR, pConfig->m_szGameExeDir, false);
	EditorUtil_TransferPath(this, IDC_MODDIR, pConfig->m_szModDir, false);
	EditorUtil_TransferPath(this, IDC_MAPDIR, pConfig->szMapDir, false);

	m_cCordonTexture.SetWindowText(pConfig->GetCordonTexture());
	
	char szText[100];
	sprintf(szText, "%g", pConfig->GetDefaultTextureScale());
	m_cDefaultTextureScale.SetWindowText(szText);

	SetDlgItemInt(IDC_DEFAULT_LIGHTMAP_SCALE, pConfig->GetDefaultLightmapScale(), FALSE);
	
	// load entity type comboboxes and set strings
	UpdateEntityLists();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COPTConfigs::UpdateEntityLists(void)
{
	if(m_pLastSelConfig == NULL)
		return;

	m_cDefaultPoint.ResetContent();
	m_cDefaultSolid.ResetContent();
#ifdef SLE //// SLE NEW - quick access class buttons for setting current entity's class
	m_cQuickAccessClass1.ResetContent();
	m_cQuickAccessClass2.ResetContent();
	m_cQuickAccessClass3.ResetContent();
	m_cQuickAccessClass4.ResetContent();
#endif
	CGameConfig *pConfig = m_pLastSelConfig;

	int nCount = pConfig->GD.GetClassCount();
	for (int i = 0; i < nCount; i++)
	{
		GDclass *pClass = pConfig->GD.GetClass(i);
		if (pClass->IsBaseClass())
		{
			continue;
		}
		else if (pClass->IsSolidClass())
		{
			m_cDefaultSolid.AddString(pClass->GetName());
#ifdef SLE //// SLE NEW - quick access class buttons for setting current entity's class
			m_cQuickAccessClass1.AddString(pClass->GetName());
			m_cQuickAccessClass2.AddString(pClass->GetName());
			m_cQuickAccessClass3.AddString(pClass->GetName());
			m_cQuickAccessClass4.AddString(pClass->GetName());
#endif
		}
		else
		{
			m_cDefaultPoint.AddString(pClass->GetName());
#ifdef SLE //// SLE NEW - quick access class buttons for setting current entity's class
			m_cQuickAccessClass1.AddString(pClass->GetName());
			m_cQuickAccessClass2.AddString(pClass->GetName());
			m_cQuickAccessClass3.AddString(pClass->GetName());
			m_cQuickAccessClass4.AddString(pClass->GetName());
#endif
		}
	}

	if (pConfig->szDefaultSolid[0] != '\0')
	{
		m_cDefaultSolid.SetWindowText(pConfig->szDefaultSolid);
	}
	else
	{
		m_cDefaultSolid.SetCurSel(0);
	}

		
	if (pConfig->szDefaultPoint[0] != '\0')
	{
		m_cDefaultPoint.SetWindowText(pConfig->szDefaultPoint);
	}
	else
	{
		m_cDefaultPoint.SetCurSel(0);
	}
#ifdef SLE //// SLE NEW - quick access class buttons for setting current entity's class
	if (pConfig->szQuickAccessClass1[0] != '\0')
	{
		m_cQuickAccessClass1.SetWindowText(pConfig->szQuickAccessClass1);
	}
	else
	{
		m_cQuickAccessClass1.SetCurSel(0);
	}

	if (pConfig->szQuickAccessClass2[0] != '\0')
	{
		m_cQuickAccessClass2.SetWindowText(pConfig->szQuickAccessClass2);
	}
	else
	{
		m_cQuickAccessClass2.SetCurSel(0);
	}

	if (pConfig->szQuickAccessClass3[0] != '\0')
	{
		m_cQuickAccessClass3.SetWindowText(pConfig->szQuickAccessClass3);
	}
	else
	{
		m_cQuickAccessClass3.SetCurSel(0);
	}

	if (pConfig->szQuickAccessClass4[0] != '\0')
	{
		m_cQuickAccessClass4.SetWindowText(pConfig->szQuickAccessClass4);
	}
	else
	{
		m_cQuickAccessClass4.SetCurSel(0);
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Fills the combo box with the list of configs. Tries to select the
//			config that was selected before coming in or, failing that, the
//			currently active config.
//-----------------------------------------------------------------------------
void UpdateConfigList(CComboBox &combo)
{
	int iCurSel = combo.GetCurSel();
	DWORD dwSelID = 0xffffffff;
	if (iCurSel != CB_ERR)
	{
		dwSelID = combo.GetItemData(iCurSel);
	}

	combo.ResetContent();

	// add configs to the combo box
	for (int i = 0; i < Options.configs.nConfigs; i++)
	{
		CGameConfig *pConfig = Options.configs.Configs[i];

		int iIndex = combo.AddString(pConfig->szName);
		combo.SetItemData(iIndex, pConfig->dwID);
	}

	if (dwSelID == 0xffffffff)
	{
		dwSelID = g_pGameConfig->dwID;
	}

	// Find the item with the given ID.
	int nSelIndex = -1;
	for (int i = 0; i < Options.configs.nConfigs; i++)
	{
		DWORD dwData = combo.GetItemData(i);
		if (dwData == dwSelID)
		{
			nSelIndex = i;
		}
	}

	combo.SetCurSel(nSelIndex == -1 ? 0 : nSelIndex);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SelectActiveConfig(CComboBox &combo)
{
	char szGameDir[MAX_PATH];
	if (GetPersistentEnvironmentVariable(GAMEDIR_TOKEN, szGameDir, sizeof(szGameDir)))
	{
		CGameConfig *pConfig = Options.configs.FindConfigForGame(szGameDir);
		if (pConfig)
		{
			// Find the item with the given ID.
			int nSelIndex = -1;
			int nCount = combo.GetCount();
			for (int i = 0; i < nCount; i++)
			{
				DWORD dwData = combo.GetItemData(i);
				if (pConfig->dwID == dwData)
				{
					nSelIndex = i;
					break;
				}
			}

			if (nSelIndex != -1)
			{
				combo.SetCurSel(nSelIndex);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COPTConfigs::UpdateConfigList()
{
	m_pLastSelConfig = NULL;

	::UpdateConfigList(m_cConfigs);
	::SelectActiveConfig(m_cConfigs);

	OnSelchangeConfigurations();
	SetModified();
}

//-----------------------------------------------------------------------------
// Purpose: Populates the combo boxes with the map formats and WAD formats, and
//			the list of game configurations.
//-----------------------------------------------------------------------------
BOOL COPTConfigs::OnInitDialog(void) 
{
	CPropertyPage::OnInitDialog();

	int nIndex;

	//
	// Add map formats.
	//
	nIndex = m_cMapFormat.AddString("Half-Life 2");
	m_cMapFormat.SetItemData(nIndex, mfHalfLife2);

	nIndex = m_cMapFormat.AddString("Half-Life");
	m_cMapFormat.SetItemData(nIndex, mfHalfLife);
#ifdef SLE //// SLE NEW - support for modern delimeters (TF2)
	nIndex = m_cMapFormat.AddString("Team Fortress 2");
	m_cMapFormat.SetItemData(nIndex, mfTF2);
#endif
	//
	// Add texture formats.
	//
	nIndex = m_cTextureFormat.AddString("VMT (Source)");
	m_cTextureFormat.SetItemData(nIndex, tfVMT);

#ifndef SLE //// SLE TODO: add WAD support later
	nIndex = m_cTextureFormat.AddString("WAD3 (GoldSource)");
	m_cTextureFormat.SetItemData(nIndex, tfWAD3);
#endif
	UpdateConfigList();

	int nCurSel = m_cConfigs.GetCurSel();
	m_pInitialSelectedConfig = Options.configs.FindConfig(m_cConfigs.GetItemData(nCurSel));

	m_strInitialGameDir.Empty();
	if (m_pInitialSelectedConfig)
	{
		m_strInitialGameDir = m_pInitialSelectedConfig->m_szModDir;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the given config is different from the one set when
//			OnInitDialog was called.
//-----------------------------------------------------------------------------
bool COPTConfigs::ConfigChanged(CGameConfig *pConfig)
{
	if (m_pInitialSelectedConfig != pConfig)
		return true;

	if (pConfig && m_pInitialSelectedConfig)
	{
		if (m_pInitialSelectedConfig->dwID != pConfig->dwID)
			return true;

		if (m_strInitialGameDir.CompareNoCase(pConfig->m_szModDir))
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns TRUE on success, FALSE on failure.
//-----------------------------------------------------------------------------
BOOL COPTConfigs::OnApply(void)
{
	SaveInfo(m_pLastSelConfig);

	int nCurSel = m_cConfigs.GetCurSel();
	CGameConfig *pConfig = Options.configs.FindConfig(m_cConfigs.GetItemData(nCurSel));

	if ( pConfig != NULL && ConfigChanged( pConfig ) )
	{
		SetPersistentEnvironmentVariable("vproject", pConfig->m_szModDir);
		AfxMessageBox("Your changes to the active configuration will not take effect until the next time you run the editor.");
	}

	Options.PerformChanges(COptions::secConfigs);
#ifdef SLE
	m_pLastSelConfig->LoadGDFiles();
	UpdateEntityLists();
#endif
	return CPropertyPage::OnApply();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszTitle - 
//			*pszDirectory - 
// Output : Returns TRUE on success, FALSE on failure.
//-----------------------------------------------------------------------------
BOOL COPTConfigs::BrowseForFolder(char *pszTitle, char *pszDirectory)
{
	char szTmp[MAX_PATH];

	BROWSEINFO bi;
	memset(&bi, 0, sizeof bi);
	bi.hwndOwner = m_hWnd;
	bi.pszDisplayName = szTmp;
	bi.lpszTitle = pszTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;

	LPITEMIDLIST idl = SHBrowseForFolder(&bi);

	if(idl == NULL)
		return FALSE;

	SHGetPathFromIDList(idl, pszDirectory);
	CoTaskMemFree(idl);

	return TRUE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COPTConfigs::OnBrowseCordonTexture(void)
{
	CTextureBrowser *pBrowser = new CTextureBrowser(this);
	if (pBrowser != NULL)
	{
		//
		// Use the currently selected texture format for browsing.
		//
		TEXTUREFORMAT eTextureFormat = tfVMT;
		int nIndex = m_cTextureFormat.GetCurSel();
		if (nIndex != LB_ERR)
		{
			eTextureFormat = (TEXTUREFORMAT)m_cTextureFormat.GetItemData(nIndex);
		}
		pBrowser->SetTextureFormat(eTextureFormat);

		//
		// Select the current cordon texture in the texture browser.
		//
		CString strTex;
		m_cCordonTexture.GetWindowText(strTex);
		pBrowser->SetInitialTexture(strTex);

		if (pBrowser->DoModal() == IDOK)
		{
			m_cCordonTexture.SetWindowText(pBrowser->m_cTextureWindow.szCurTexture);
		}

		delete pBrowser;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COPTConfigs::OnBrowseGameExeDir(void)
{
#ifdef SLE //// SLE CHANGE - replace the inconvenient tree-based dialogue with full directory browser
	char szInitialDir[MAX_PATH];
	APP()->GetDirectory(DIR_PROGRAM, szInitialDir);
		
	CFolderPickerDialog dlg(szInitialDir, OFN_FILEMUSTEXIST | OFN_ENABLESIZING, this,
		sizeof(OPENFILENAME));

	dlg.m_ofn.lpstrTitle = "Select Game Executable Directory";

	if (dlg.DoModal() == IDCANCEL)
	{
		return;
	}

	CString str(dlg.GetPathName());
#else
	char szTmp[MAX_PATH];
	if (!BrowseForFolder("Select Game Executable Directory", szTmp))
	{
		return;
	}
	CString str(szTmp);
#endif
	EditorUtil_ConvertPath(str, false);
	m_cGameExeDir.SetWindowText(str);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COPTConfigs::OnBrowseModDir(void)
{
#ifdef SLE //// SLE CHANGE - replace the inconvenient tree-based dialogue with full directory browser
	char szInitialDir[MAX_PATH];
	APP()->GetDirectory(DIR_PROGRAM, szInitialDir);

	CFolderPickerDialog dlg(szInitialDir, OFN_FILEMUSTEXIST | OFN_ENABLESIZING, this,
		sizeof(OPENFILENAME));

	dlg.m_ofn.lpstrTitle = "Select Mod Directory";

	if (dlg.DoModal() == IDCANCEL)
	{
		return;
	}

	CString str(dlg.GetPathName());
#else
	char szTmp[MAX_PATH];
	if (!BrowseForFolder("Select Mod Directory", szTmp))
	{
		return;
	}

	CString str(szTmp);
#endif
	EditorUtil_ConvertPath(str, false);
	m_cModDir.SetWindowText(str);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COPTConfigs::OnBrowsemapdir(void)
{
#ifdef SLE //// SLE CHANGE - replace the inconvenient tree-based dialogue with full directory browser
	char szInitialDir[MAX_PATH];
	APP()->GetDirectory(DIR_PROGRAM, szInitialDir);

	CFolderPickerDialog dlg(szInitialDir, OFN_FILEMUSTEXIST | OFN_ENABLESIZING, this,
		sizeof(OPENFILENAME));

	dlg.m_ofn.lpstrTitle = "Select Map Directory";

	if (dlg.DoModal() == IDCANCEL)
	{
		return;
	}

	CString str(dlg.GetPathName());
#else
	char szTmp[MAX_PATH];
	if (!BrowseForFolder("Select Map Directory", szTmp))
	{
		return;
	}

	CString str(szTmp);
#endif
	EditorUtil_ConvertPath(str, false);
	m_cMapDir.SetWindowText(str);
}

//-----------------------------------------------------------------------------
// Purpose: Handles the notification from the VCONFIG that the active config has changed.
//-----------------------------------------------------------------------------
LRESULT COPTConfigs::OnSettingChange(WPARAM wParam, LPARAM lParam)
{
    const char *changedSection = (const char *)lParam;
    if ( Q_stricmp( changedSection, "Environment" ) == 0 )
	{
		UpdateConfigList();		
		//SelectActiveConfig();
	}

	return 0;
}
