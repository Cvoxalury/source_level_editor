//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "stdafx.h"
#include "hammer.h"
#include "GameConfig.h"
#include "OptionProperties.h"
#include "OPTTextures.h"
#include "Options.h"
#ifdef SLE //// SLE NEW - Favourites for material browser
#include "texturebrowser.h"
#include "StrDlg.h"
#endif
#include "tier1/strtools.h"
#include <shlobj.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

/////////////////////////////////////////////////////////////////////////////
// COPTTextures property page
#ifdef SLE //// SLE NEW - game config selection for Materials tab, as Materials are game-specific
void UpdateConfigList(CComboBox &combo);
void SelectActiveConfig(CComboBox &combo);
#endif
IMPLEMENT_DYNCREATE(COPTTextures, CPropertyPage)

COPTTextures::COPTTextures() : CPropertyPage(COPTTextures::IDD)
{
	//{{AFX_DATA_INIT(COPTTextures)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pMaterialConfig = NULL;
	m_bDeleted = FALSE;
#ifdef SLE //// SLE NEW - game config selection for Materials tab, as Materials are game-specific
	m_pConfig = NULL;
#endif
}

COPTTextures::~COPTTextures()
{
	// detach the material exclusion list box
	m_MaterialExcludeList.Detach();
}

#ifdef SLE //// SLE NEW - customisable material cache size for the mat browser
void PASCAL DDV_MaterialCacheSizeRange(CDataExchange *pDX, int value)
{
	if (value < 32)
	{
		AfxMessageBox("The material cache size needs to be at least 32 MB.", MB_ICONEXCLAMATION | MB_OK);
		pDX->Fail();
	}

	if (value > 2048)
	{
		AfxMessageBox("The material cache size shouldn't exceed 2048 MB.", MB_ICONEXCLAMATION | MB_OK);
		pDX->Fail();
	}
}
#endif

void COPTTextures::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COPTTextures)
	DDX_Control(pDX, IDC_TEXTUREFILES, m_TextureFiles);
#ifndef SLE //// SLE REMOVE, deprecatedm Quake 2
	DDX_Control(pDX, IDC_BRIGHTNESS, m_cBrightness);
#endif
#ifdef SLE //// SLE NEW - game config selection for Materials tab, as Materials are game-specific
	DDX_Control(pDX, IDC_CONFIGS, m_cConfigs);
	//// SLE NEW - customisable material cache size for the mat browser
	DDX_Text(pDX, IDC_MATERIALCACHESIZE, Options.view3d.iMaterialCacheSize);
	DDV_MaterialCacheSizeRange(pDX, Options.view3d.iMaterialCacheSize);
#endif
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COPTTextures, CPropertyPage)
	//{{AFX_MSG_MAP(COPTTextures)
	ON_BN_CLICKED(IDC_EXTRACT, OnExtract)
	ON_BN_CLICKED(IDC_ADDTEXFILE, OnAddtexfile)
	ON_BN_CLICKED(IDC_REMOVETEXFILE, OnRemovetexfile)
	ON_WM_HSCROLL()
#ifndef SLE //// SLE REMOVE - deprecated, Quake 2
	ON_BN_CLICKED(IDC_ADDTEXFILE2, OnAddtexfile2)
#endif
	ON_BN_CLICKED( ID_MATERIALEXCLUDE_ADD, OnMaterialExcludeAdd )
	ON_BN_CLICKED( ID_MATERIALEXCLUDE_REM, OnMaterialExcludeRemove )
	ON_LBN_SELCHANGE(ID_MATERIALEXCLUDE_LIST, OnMaterialExcludeListSel)
#ifdef SLE //// SLE NEW - Favourites for material browser
	ON_BN_CLICKED(ID_MATERIALFAVOURITE_ADD, OnFavouriteAdd)
	ON_BN_CLICKED(ID_MATERIALFAVOURITE_REM, OnFavouriteRemove)
	ON_LBN_SELCHANGE(ID_MATERIALFAVOURITES_LIST, OnFavouriteListSel)
	//// SLE NEW - game config selection for Materials tab, as Materials are game-specific
	ON_CBN_SELCHANGE(IDC_CONFIGS, OnSelchangeConfigs)
#endif
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COPTTextures message handlers

BOOL COPTTextures::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// load texture file list with options
	int i;
	for(i = 0; i < Options.textures.nTextureFiles; i++)
	{
		m_TextureFiles.AddString(Options.textures.TextureFiles[i]);
	}
#ifndef SLE //// SLE REMOVE - deprecated, Quake 2
	// set brightness control & values
	m_cBrightness.SetRange(1, 50); // 10 is default
	m_cBrightness.SetPos(int(Options.textures.fBrightness * 10));
#endif
	// attach the material exclusion list box
	m_MaterialExcludeList.Attach( GetDlgItem( ID_MATERIALEXCLUDE_LIST )->m_hWnd );

#ifdef SLE //// SLE NEW - Favourites for material browser
	m_MaterialFavouritesList.Attach(GetDlgItem(ID_MATERIALFAVOURITES_LIST)->m_hWnd);

	//// SLE NEW - game config selection for Materials tab, as Materials are game-specific
	UpdateConfigList();
#endif
	return TRUE;
}

void COPTTextures::OnExtract() 
{
	// redo listbox content
	m_TextureFiles.ResetContent();
	for(int i = 0; i < Options.textures.nTextureFiles; i++)
	{
		m_TextureFiles.AddString(Options.textures.TextureFiles[i]);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
//-----------------------------------------------------------------------------
void COPTTextures::OnAddtexfile(void)
{
	static char szInitialDir[MAX_PATH] = "\0";

	CFileDialog dlg(TRUE, "wad", NULL, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST, "Texture files (*.wad;*.pak)|*.wad; *.pak||");

	if (szInitialDir[0] == '\0')
	{
		Q_snprintf( szInitialDir, sizeof( szInitialDir ), "%s\\wads\\", g_pGameConfig->m_szModDir );
	}

	dlg.m_ofn.lpstrInitialDir = szInitialDir;

	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	//
	// Get all the filenames from the open file dialog.
	//
	POSITION pos = dlg.GetStartPosition();
	CString str;
	while (pos != NULL)
	{
		str = dlg.GetNextPathName(pos);
		str.MakeLower();
		m_TextureFiles.AddString(str);
		SetModified();
	}

	//
	// Use this directory as the default directory for the next time.
	//
	int nBackslash = str.ReverseFind('\\');
	if (nBackslash != -1)
	{
		lstrcpyn(szInitialDir, str, nBackslash + 1);
	}
}

void COPTTextures::OnRemovetexfile() 
{
	int i = m_TextureFiles.GetCount();

	for (i--; i >= 0; i--)
	{
		if (m_TextureFiles.GetSel(i))
			m_TextureFiles.DeleteString(i);
	}

	m_bDeleted = TRUE;
}

void COPTTextures::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
#ifndef SLE //// SLE REMOVE - deprecated, Quake 2
	if(pScrollBar == (CScrollBar*) &m_cBrightness)
		SetModified();
#endif
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL COPTTextures::OnApply() 
{
#ifndef SLE //// SLE REMOVE - deprecated, Quake 2
	Options.textures.fBrightness = (float)m_cBrightness.GetPos() / 10.0f;
#endif
	int iSize = m_TextureFiles.GetCount();
	CString str;
	Options.textures.nTextureFiles = iSize;
	Options.textures.TextureFiles.RemoveAll();
	for(int i = 0; i < iSize; i++)
	{
		m_TextureFiles.GetText(i, str);
		Options.textures.TextureFiles.Add(str);
	}
	
	if(m_bDeleted)
	{
		// inform them that deleted files will only be reapplied after
		// they reload the editor
		MessageBox("You have removed some texture files from the list. "
			"These texture files will continue to be used during this "
			"session, but will not be loaded the next time you run "
			"Hammer.", "A Quick Note");
	}

	Options.PerformChanges(COptions::secTextures);

	return CPropertyPage::OnApply();
}

void GetDirectory(char *pDest, const char *pLongName)
{
	strcpy(pDest, pLongName);
	int i = strlen(pDest);
	while (pLongName[i] != '\\' && pLongName[i] != '/' && i > 0)
		i--;

	if (i <= 0)
		i = 0;
	
	pDest[i] = 0;

	return;
}
#ifndef SLE //// SLE REMOVE - deprecated, Quake 2
void COPTTextures::OnAddtexfile2() 
{
	BROWSEINFO bi;
	char szDisplayName[MAX_PATH];

	bi.hwndOwner = m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = szDisplayName;
	bi.lpszTitle = "Select your Quake II directory.";
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	bi.lParam = 0;
	
	LPITEMIDLIST pidlNew = SHBrowseForFolder(&bi);

	if(pidlNew)
	{
		// get path from the id list
		char szPathName[MAX_PATH];
		SHGetPathFromIDList(pidlNew, szPathName);
				
		if (AfxMessageBox("Add all subdirectories as separate Texture Groups?", MB_YESNO) == IDYES)
		//if (!strcmpi("\\textures", &szPathName[strlen(szPathName) - strlen("\\textures")]))
		{
			char szNewPath[MAX_PATH];
			strcpy(szNewPath, szPathName);
			strcat(szNewPath, "\\*.*");
			WIN32_FIND_DATA FindData;
			HANDLE hFile = FindFirstFile(szNewPath, &FindData);

			if (hFile != INVALID_HANDLE_VALUE) do
			{
				if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						&&(FindData.cFileName[0] != '.'))
				{
					sprintf(szNewPath, "%s\\%s", szPathName, FindData.cFileName);
					strlwr(szNewPath);
					if (m_TextureFiles.FindStringExact(-1, szNewPath) == CB_ERR)
						m_TextureFiles.AddString(szNewPath);
				}
			} while (FindNextFile(hFile, &FindData));

		}
		else
		{
			strlwr(szPathName);
			if (m_TextureFiles.FindStringExact(-1, szPathName) == CB_ERR)
				m_TextureFiles.AddString(strlwr(szPathName));
		}
		SetModified();

		// free the previous return value from SHBrowseForFolder
		CoTaskMemFree(pidlNew);
	}
}
#endif
static char s_szStartFolder[MAX_PATH];
static int CALLBACK BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
	switch ( uMsg )
	{
		case BFFM_INITIALIZED:
		{
			if ( lpData )
			{
				SendMessage( hwnd, BFFM_SETSELECTION, TRUE, ( LPARAM ) s_szStartFolder );
			}
			break;
		}

		default:
		{
		   break;
		}
	}
         
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszTitle - 
//			*pszDirectory - 
// Output : Returns TRUE on success, FALSE on failure.
//-----------------------------------------------------------------------------
BOOL COPTTextures::BrowseForFolder( char *pszTitle, char *pszDirectory )
{
	USES_CONVERSION;

	static bool s_bFirst = true;
	if ( s_bFirst )
	{
		APP()->GetDirectory( DIR_MATERIALS, s_szStartFolder );
		s_bFirst = false;
	}

	LPITEMIDLIST pidlStartFolder = NULL;

	IShellFolder *pshDesktop = NULL;
	SHGetDesktopFolder( &pshDesktop );
	if ( pshDesktop )
	{
		ULONG ulEaten;
		ULONG ulAttributes;
		pshDesktop->ParseDisplayName( NULL, NULL, A2OLE( s_szStartFolder ), &ulEaten, &pidlStartFolder, &ulAttributes );
	}	
	
	char szTmp[MAX_PATH];

	BROWSEINFO bi;
	memset( &bi, 0, sizeof( bi ) );
	bi.hwndOwner = m_hWnd;
	bi.pszDisplayName = szTmp;
	bi.lpszTitle = pszTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS /*| BIF_NEWDIALOGSTYLE*/;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = TRUE;

	LPITEMIDLIST idl = SHBrowseForFolder( &bi );

	if ( idl == NULL )
	{
		return FALSE;
	}

	SHGetPathFromIDList( idl, pszDirectory );

	// Start in this folder next time.	
	Q_strncpy( s_szStartFolder, pszDirectory, sizeof( s_szStartFolder ) ); 

	CoTaskMemFree( pidlStartFolder );
	CoTaskMemFree( idl );

	return TRUE;
}

//-----------------------------------------------------------------------------
// Purpose: intercept this call and update the "material" configuration pointer
//          if it is out of sync with the game configuration "parent" (on the
//          "Options Configs" page)
//  Output: returns TRUE on success, FALSE on failure
//-----------------------------------------------------------------------------
BOOL COPTTextures::OnSetActive( void )
{
	//
	// get the current game configuration from the "Options Configs" page
	//
	COptionProperties *pOptProps = ( COptionProperties* )GetParent();
	if( !pOptProps )
		return FALSE;

#ifdef SLE	//// SLE NEW - game config selection for Materials tab, as Materials are game-specific
	CGameConfig *pConfig = m_pConfig;
#else
	CGameConfig *pConfig = pOptProps->Configs.GetCurrentConfig();
#endif
	if( !pConfig )
		return FALSE;

	// compare for a change
	if( m_pMaterialConfig != pConfig )
	{
		// update the material config
		m_pMaterialConfig = pConfig;

		// update the all config specific material data on this page
		MaterialExcludeUpdate();
#ifdef SLE //// SLE NEW - Favourites for material browser
		MaterialFavouritesUpdate();
#endif
		// update the last material config
		m_pMaterialConfig = pConfig;
	}

	return CPropertyPage::OnSetActive();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void COPTTextures::MaterialExcludeUpdate( void )
{
	// remove all of the data in the current "material exclude" list box
	m_MaterialExcludeList.ResetContent();

	//
	// add the data from the current material config
	//
	for( int i = 0; i < m_pMaterialConfig->m_MaterialExcludeCount; i++ )
	{
		int result = m_MaterialExcludeList.AddString( m_pMaterialConfig->m_MaterialExclusions[i].szDirectory );
		m_MaterialExcludeList.SetItemData( result, 1 );
		if( ( result == LB_ERR ) || ( result == LB_ERRSPACE ) )
			return;
	}
	if (pGD != NULL)	
	{
		for( int i = 0; i < pGD->m_FGDMaterialExclusions.Count(); i++ )
		{
			char szFolder[MAX_PATH];
			strcpy( szFolder, pGD->m_FGDMaterialExclusions[i].szDirectory );
#ifdef SLE //// clarity
			strcat(szFolder, " (FGD)");
#else
			strcat( szFolder, " (default)" );
#endif
			int result = m_MaterialExcludeList.AddString( szFolder );
			m_MaterialExcludeList.SetItemData( result, 0 );
			if( ( result == LB_ERR ) || ( result == LB_ERRSPACE ) )
				return;
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void StripOffMaterialDirectory( const char *pszDirectoryName, char *pszName )
{
	// clear name
	pszName[0] = '\0';

	// create a lower case version of the string
	char *pLowerCase = _strlwr( _strdup( pszDirectoryName ) );
	char *pAtMat = strstr( pLowerCase, "materials" );
	if( !pAtMat )
		return;

	// move the pointer ahead 10 spaces = "materials\"
	pAtMat += 10;

	// copy the rest to the name string
	strcpy( pszName, pAtMat );

	// free duplicated string's memory
	free( pLowerCase );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void COPTTextures::OnMaterialExcludeAdd( void )
{
	//
	// get the directory path to exclude
	//
#ifdef SLE //// SLE CHANGE - replace the inconvenient tree-based dialogue with just a text line!
	char szSubDirName[MAX_PATH];

	szSubDirName[0] = 0;

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
			strcpy(szSubDirName, dlg.m_string);
		}
	}
	else
		return;
#else
	char szTmp[MAX_PATH];
	if( !BrowseForFolder( "Select Game Executable Directory", szTmp ) )
		return;
	// strip off the material directory
	char szSubDirName[MAX_PATH];
	StripOffMaterialDirectory( szTmp, &szSubDirName[0] );
#endif
	if (szSubDirName[0] == '\0')
	{
		AfxMessageBox("You must enter a folder name longer than 0 characters.");
		return;
	}

	//
	// add directory to list box
	//
	int result = m_MaterialExcludeList.AddString( szSubDirName );
	m_MaterialExcludeList.SetItemData( result, 1 );
	if( ( result == LB_ERR ) || ( result == LB_ERRSPACE ) )
		return;
	
	//
	// add name of directory to the global exclusion list
	//
	int ndx = m_pMaterialConfig->m_MaterialExcludeCount;
#ifndef SLE //// SLE REMOVE - no limit on material exclude directories
	if( ndx >= MAX_DIRECTORY_SIZE )
		return;
#endif
	m_pMaterialConfig->m_MaterialExcludeCount++;

	int index = m_pMaterialConfig->m_MaterialExclusions.AddToTail();
	Q_strncpy( m_pMaterialConfig->m_MaterialExclusions[index].szDirectory, szSubDirName, sizeof ( m_pMaterialConfig->m_MaterialExclusions[index].szDirectory ) );

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void COPTTextures::OnMaterialExcludeRemove( void )
{
	//
	// get the directory to remove
	//
	int ndxSel = m_MaterialExcludeList.GetCurSel();
	if( ndxSel == LB_ERR )
		return;

	char szTmp[MAX_PATH];
	m_MaterialExcludeList.GetText( ndxSel, &szTmp[0] );

	//
	// remove directory from the list box
	//
	int result = m_MaterialExcludeList.DeleteString( ndxSel );
	if( result == LB_ERR )
		return;

	//
	// remove the name of the directory from the global exclusion list
	//
	for( int i = 0; i < m_pMaterialConfig->m_MaterialExcludeCount; i++ )
	{
		if( !strcmp( szTmp, m_pMaterialConfig->m_MaterialExclusions[i].szDirectory ) )
		{
			// remove the directory
			if( i != ( m_pMaterialConfig->m_MaterialExcludeCount - 1 ) )
			{
				m_pMaterialConfig->m_MaterialExclusions.Remove( i );
			}

			// decrement count
			m_pMaterialConfig->m_MaterialExcludeCount--;
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void COPTTextures::OnMaterialExcludeListSel( void )
{
	int ndxSel = m_MaterialExcludeList.GetCurSel();
	if( ndxSel == LB_ERR )
		return;

	char szTmp[MAX_PATH];
	m_MaterialExcludeList.GetText( ndxSel, &szTmp[0] );

	// Item data of 0 = FGD exclusion, 1 = user-created exclusion
	DWORD dwData = m_MaterialExcludeList.GetItemData( ndxSel );
	GetDlgItem( ID_MATERIALEXCLUDE_REM )->EnableWindow( dwData ? TRUE : FALSE );
}

#ifdef SLE //// SLE NEW - Favourites for material browser
void COPTTextures::MaterialFavouritesUpdate(void)
{
	// remove all of the data in the current "material favourites" list box
	m_MaterialFavouritesList.ResetContent();

	//
	// add the data from the current material config
	//
	for ( int i = 0; i < m_pMaterialConfig->m_MaterialFavouritesCount; i++ )
	{
		int result = m_MaterialFavouritesList.AddString(m_pMaterialConfig->m_MaterialFavourites[ i ].szName);
		m_MaterialFavouritesList.SetItemData(result, 1);
		if ( ( result == LB_ERR ) || ( result == LB_ERRSPACE ) )
			return;
	}
	if (0) //( pGD != NULL )
	{
		for ( int i = 0; i < pGD->m_FGDMaterialExclusions.Count(); i++ )
		{
			char szFolder[ MAX_PATH ];
			strcpy(szFolder, pGD->m_FGDMaterialExclusions[ i ].szDirectory);
			strcat(szFolder, " (FGD)");
			int result = m_MaterialFavouritesList.AddString(szFolder);
			m_MaterialFavouritesList.SetItemData(result, 0);
			if ( ( result == LB_ERR ) || ( result == LB_ERRSPACE ) )
				return;
		}
	}
}

void COPTTextures::OnFavouriteAdd(void)
{
	CTextureBrowser *pBrowser = new CTextureBrowser(this);
	if ( pBrowser != NULL )
	{
		//
		// Use the currently selected texture format for browsing.
		//
		TEXTUREFORMAT eTextureFormat = g_pGameConfig->GetTextureFormat();
		pBrowser->SetTextureFormat(eTextureFormat);

	//	pBrowser->SetInitialTexture(strTex);

		if ( pBrowser->DoModal() == IDOK )
		{
			//
			// add name to list box
			//
			int result = m_MaterialFavouritesList.AddString(pBrowser->m_cTextureWindow.szCurTexture);
			m_MaterialFavouritesList.SetItemData(result, 1);
			if ( ( result == LB_ERR ) || ( result == LB_ERRSPACE ) )
				return;

			//
			// add name of the texture to the global favourites list
			//
			int ndx = m_pMaterialConfig->m_MaterialFavouritesCount;

			m_pMaterialConfig->m_MaterialFavouritesCount++;

			int index = m_pMaterialConfig->m_MaterialFavourites.AddToTail();
			Q_strncpy(m_pMaterialConfig->m_MaterialFavourites[ index ].szName, pBrowser->m_cTextureWindow.szCurTexture, sizeof (m_pMaterialConfig->m_MaterialFavourites[ index ].szName));
		}

		delete pBrowser;
	}
	else
	{
		return;
	}
}

void COPTTextures::OnFavouriteRemove(void)
{
	//
	// get the name to remove
	//
	int ndxSel = m_MaterialFavouritesList.GetCurSel();
	if ( ndxSel == LB_ERR )
		return;

	char szTmp[ MAX_PATH ];
	m_MaterialFavouritesList.GetText(ndxSel, &szTmp[ 0 ]);

	//
	// remove name from the list box
	//
	int result = m_MaterialFavouritesList.DeleteString(ndxSel);
	if ( result == LB_ERR )
		return;

	//
	// remove the name of the favourite from the global favourites list
	//
	for ( int i = 0; i < m_pMaterialConfig->m_MaterialFavouritesCount; i++ )
	{
		if ( !strcmp(szTmp, m_pMaterialConfig->m_MaterialFavourites[ i ].szName) )
		{
			// remove the directory
			if ( i != ( m_pMaterialConfig->m_MaterialFavouritesCount - 1 ) )
			{
				m_pMaterialConfig->m_MaterialFavourites.Remove(i);
			}

			// decrement count
			m_pMaterialConfig->m_MaterialFavouritesCount--;
		}
	}
}

void COPTTextures::OnFavouriteListSel(void)
{
	int ndxSel = m_MaterialFavouritesList.GetCurSel();
	if ( ndxSel == LB_ERR )
		return;

	char szTmp[ MAX_PATH ];
	m_MaterialFavouritesList.GetText(ndxSel, &szTmp[ 0 ]);

	// Item data of 0 = FGD favourite, 1 = user-created favourite
	DWORD dwData = m_MaterialFavouritesList.GetItemData(ndxSel);
	GetDlgItem(ID_MATERIALFAVOURITE_REM)->EnableWindow(dwData ? TRUE : FALSE);
}

//// SLE NEW - game config selection for Materials tab, as Materials are game-specific
void COPTTextures::OnSelchangeConfigs()
{
//	SaveInfo(m_pConfig); //textures are saved immediately, no need

	m_pConfig = NULL;

	int iCurSel = m_cConfigs.GetCurSel();

	if (iCurSel == CB_ERR)
		return;

	// get pointer to the configuration
	m_pConfig = Options.configs.FindConfig(m_cConfigs.GetItemData(iCurSel));

	// update dialog data
	// load texture file list with options
	int i;
	for (i = 0; i < Options.textures.nTextureFiles; i++)
	{
		m_TextureFiles.AddString(Options.textures.TextureFiles[i]);
	}

	OnSetActive();
}

void COPTTextures::UpdateConfigList()
{
	m_pConfig = NULL;

	::UpdateConfigList(m_cConfigs);
	::SelectActiveConfig(m_cConfigs);

	OnSelchangeConfigs();
	SetModified();
}
#endif
