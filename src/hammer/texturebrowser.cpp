//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "stdafx.h"
#include <oleauto.h>
#include <oaidl.h>
#if _MSC_VER < 1300
#include <afxpriv.h>
#endif
#include "CustomMessages.h"
#include "GlobalFunctions.h"
#include "History.h"
#include "FaceEditSheet.h"
#include "IEditorTexture.h"
#include "MainFrm.h"
#include "MapDoc.h"
#include "MapWorld.h"
#include "ReplaceTexDlg.h"
#include "TextureBrowser.h"
#include "TextureSystem.h"
#include "hammer.h"
#include "Selection.h"
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#ifdef SLE_TEXTUREBROWSER_FAVOURITES
/*
void CTextureFavourites::Create(CWnd *pParentWnd, RECT& rect)
{
	static CString FavWndClassName;

	if (FavWndClassName.IsEmpty())
	{
		// create class
		FavWndClassName = AfxRegisterWndClass(CS_DBLCLKS | CS_HREDRAW |
			CS_VREDRAW, LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)GetStockObject(BLACK_BRUSH),
			AfxGetApp()->LoadIcon(IDI_TEXTUREWINDOW));
	}

	CWnd::Create(FavWndClassName, "TextureBrowserFavourites",
		SS_SUNKEN | WS_TABSTOP | WS_CHILD | WS_VSCROLL | WS_HSCROLL,
		rect, pParentWnd, IDC_TEXTUREWINDOW);

//	UpdateScrollSizes();

	// create font
//	if (!TexFont.m_hObject)
//	TexFont.CreatePointFont(iTexNameFontHeight * 10, "Arial");

	CDC *pDC = GetDC();
//	pDC->SelectObject(&TexFont);
//	pDC->GetCharWidth('A', 'A', &iTexNameCharWidth);

	ReleaseDC(pDC);
}
*/
#endif

static LPCTSTR pszIniSection = "Texture Browser";

CStringArray CTextureBrowser::m_FilterHistory;
int CTextureBrowser::m_nFilterHistory;
#ifndef SLE_NO_TEXTURE_KEYWORDS
char CTextureBrowser::m_szLastKeywords[MAX_PATH];
#endif

BEGIN_MESSAGE_MAP(CTextureBrowser, CDialog)
	//{{AFX_MSG_MAP(CTextureBrowser)
	ON_WM_SIZE()
	ON_CBN_SELENDOK(IDC_TEXTURESIZE, OnSelendokTexturesize)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_CBN_EDITCHANGE(IDC_FILTER, OnChangeFilterOrKeywords)
	ON_CBN_SELENDOK(IDC_FILTER, OnUpdateFiltersNOW)
#ifndef SLE_NO_TEXTURE_KEYWORDS
	ON_CBN_EDITCHANGE(IDC_KEYWORDS, OnChangeFilterOrKeywords)
	ON_CBN_SELENDOK(IDC_KEYWORDS, OnUpdateKeywordsNOW)
#endif

	ON_BN_CLICKED(IDC_FILTER_OPAQUE, OnFilterOpaque)
	ON_BN_CLICKED(IDC_FILTER_TRANSLUCENT, OnFilterTranslucent)
	ON_BN_CLICKED(IDC_FILTER_SELFILLUM, OnFilterSelfIllum)
	ON_BN_CLICKED(IDC_FILTER_SPECULAR, OnFilterSpecular)
	ON_BN_CLICKED(IDC_SHOW_ERROR, OnShowErrors)

	ON_BN_CLICKED(IDC_SHOW_ONLY_USED_TEXTURES, OnShowOnlyUsedTextures)
	ON_BN_CLICKED(IDC_MARK, OnMark)
	ON_BN_CLICKED(IDC_REPLACE, OnReplace)
	ON_BN_CLICKED(IDC_TEXTURES_OPEN_SOURCE, OnOpenSource)
	ON_BN_CLICKED(IDC_TEXTURES_RELOAD, OnReload)
	ON_MESSAGE(TWN_SELCHANGED, OnTexturewindowSelchange)
	ON_MESSAGE(TWN_LBUTTONDBLCLK, OnTextureWindowDblClk)
#ifdef SLE_TEXTUREBROWSER_FAVOURITES
	ON_BN_CLICKED(IDC_TOFAVS, OnAddCurrentMaterialToFavs)
	ON_BN_CLICKED(IDC_FROMFAVS, OnRemoveCurrentMaterialFromFavs)
	ON_BN_CLICKED(IDC_FAVSLIST, OnOpenMaterialConfig)
#endif
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pParent - 
//-----------------------------------------------------------------------------
CTextureBrowser::CTextureBrowser(CWnd* pParent)
	: CDialog(IDD, pParent)
{
	m_szNameFilter[0] = '\0';
	szInitialTexture[0] = '\0';
	m_bFilterChanged = FALSE;
	m_uLastFilterChange = 0xffffffff;
	m_bUsed = FALSE;
#ifndef SLE_NO_TEXTURE_KEYWORDS
	m_szLastKeywords[0] = '\0';
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Creates the dialog with the textures thumbnails, filters, the rest
//-----------------------------------------------------------------------------
BOOL CTextureBrowser::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Iterate all the active textures for debugging.
	//int nCount = g_Textures.GetActiveTextureCount();
	//for (int nTexture = 0; nTexture < nCount; nTexture++)
	//{
	//	IEditorTexture *pTexture = g_Textures.GetActiveTexture(nTexture);
	//	const char *pszName = pTexture->GetName();
	//	DBG("%d: %s\n", nTexture, pszName);
	//}

	m_cSizeList.SubclassDlgItem(IDC_TEXTURESIZE, this);
	m_cFilter.SubclassDlgItem(IDC_FILTER, this);
#ifndef SLE_NO_TEXTURE_KEYWORDS
	m_cKeywords.SubclassDlgItem(IDC_KEYWORDS, this);
#endif
	m_cCurName.SubclassDlgItem(IDC_CURNAME, this);
	m_cCurDescription.SubclassDlgItem(IDC_CURDESCRIPTION, this);
	m_cUsed.SubclassDlgItem(IDC_SHOW_ONLY_USED_TEXTURES, this);

	m_FilterOpaque.SubclassDlgItem(IDC_FILTER_OPAQUE, this);
	m_FilterTranslucent.SubclassDlgItem(IDC_FILTER_TRANSLUCENT, this);
	m_FilterSelfIllum.SubclassDlgItem(IDC_FILTER_SELFILLUM, this);
	m_FilterSpecular.SubclassDlgItem(IDC_FILTER_SPECULAR, this);
	m_ShowErrors.SubclassDlgItem(IDC_SHOW_ERROR, this);
	m_FilterOpaque.SetCheck(true);
	m_FilterTranslucent.SetCheck( true );
	m_FilterSelfIllum.SetCheck( true );
	m_FilterSpecular.SetCheck( true );
	m_ShowErrors.SetCheck( true );
	//
	// Create CTextureWindow that takes up area of dummy control.
	//
	{
		RECT r;
		GetDlgItem(IDC_BROWSERDUMMY)->GetClientRect(&r);
		m_cTextureWindow.Create(this, r);
				
#ifdef SLE_TEXTUREBROWSER_FAVOURITES
		m_cTextureWindow.SetFavouritesOnly(false);

		if ( g_pGameConfig->m_MaterialFavouritesCount > 0 )
		{
			m_cFavouritesWindow.Create(this, r);
			m_cFavouritesWindow.SetFavouritesOnly(true);
			AddFavourites();
		}
#endif
	}

	// Show everything initially
	m_cTextureWindow.SetTypeFilter(~0, true);

	//
	// Add latest history to the filter combo.
	//
	for (int i = 0; i < m_nFilterHistory; i++)
	{
		m_cFilter.AddString(m_FilterHistory[i]);
	}

	//
	// Set the name filter unless one was explicitly specified.
	//
	if (m_szNameFilter[0] == '\0')
	{
		//
		// No name filter specified. Use whatever is on top of the history.
		//
		if (m_cFilter.GetCount() > 0)
		{
			m_cFilter.GetLBText(0, m_szNameFilter);
			m_cFilter.SetCurSel(0);
		}
	}

	m_cFilter.SetWindowText(m_szNameFilter);
	m_cTextureWindow.SetNameFilter(m_szNameFilter);

	//
	// Done with the name filter; clear it for next time.
	//
	m_szNameFilter[0] = '\0';

	// Add the global list of keywords to the keywords combo.
	for (int i = 0; i< g_Textures.GetNumKeywords(); i++)
	{
		m_cKeywords.AddString(g_Textures.GetKeyword(i));
	}

	//
	// Set the keyword filter.
	//
#ifndef SLE_NO_TEXTURE_KEYWORDS
	m_cKeywords.SetWindowText(m_szLastKeywords);
	m_cTextureWindow.SetKeywords(m_szLastKeywords);
#endif
	m_cUsed.SetCheck(m_bUsed);

	// Refresh the list of used textures if enabled.
	if (m_bUsed)
	{
		SetUsed(TRUE);
	}

	CWinApp *pApp = AfxGetApp();
	CString str = pApp->GetProfileString(pszIniSection, "Position");
	if (!str.IsEmpty())
	{
		RECT r;
		sscanf(str, "%d %d %d %d", &r.left, &r.top, &r.right, &r.bottom);

		if (r.left < 0)
		{
			ShowWindow(SW_SHOWMAXIMIZED);
		}
		else
		{
			MoveWindow(r.left, r.top, r.right - r.left, r.bottom - r.top, FALSE);
		}
	}

	int iSize = pApp->GetProfileInt(pszIniSection, "ShowSize", 0);
	m_cSizeList.SetCurSel(iSize);
	OnSelendokTexturesize();

	if (szInitialTexture[0])
	{
		m_cTextureWindow.SelectTexture(szInitialTexture);
	}

	m_cTextureWindow.ShowWindow(SW_SHOW);

	SetTimer(1, 500, NULL);

	m_cFilter.SetFocus();

	return(FALSE);
}

//-----------------------------------------------------------------------------
// Purpose: Handles resize messages. Moves the child windows to the proper positions.
// Input  : nType - 
//			cx - 
//			cy - 
//-----------------------------------------------------------------------------
void CTextureBrowser::OnSize(UINT nType, int cx, int cy) 
{
	if (nType == SIZE_MINIMIZED || !IsWindow(m_cTextureWindow.m_hWnd))
	{
		CDialog::OnSize(nType, cx, cy);
		return;
	}
	
	// reposition controls
	CRect clientrect;
	GetClientRect(&clientrect);

	CRect CtrlRect;
	GetDlgItem(IDC_CONTROLHEIGHT)->GetWindowRect(&CtrlRect);

	int iControlHeight = (CtrlRect.bottom - CtrlRect.top);
	int iThisCtrlHeight;

	//
	// Resize the texture window.
	//
	CtrlRect = clientrect;
	CtrlRect.bottom -= iControlHeight;
#ifdef SLE_TEXTUREBROWSER_FAVOURITES
	int iFavBrowserLeft;
	if ( g_pGameConfig->m_MaterialFavouritesCount > 0 )
	{
		CtrlRect.right -= CtrlRect.Width() / 4; //// SLE NEW - make space for the favourites window
		iFavBrowserLeft = CtrlRect.right;
	}
#endif
	m_cTextureWindow.MoveWindow(CtrlRect);

#ifdef SLE_TEXTUREBROWSER_FAVOURITES
	if ( g_pGameConfig->m_MaterialFavouritesCount > 0)
	{
		CRect FavRect;
		GetDlgItem(IDC_CONTROLHEIGHT)->GetWindowRect(&FavRect);
		FavRect = clientrect;
		FavRect.bottom -= iControlHeight;
		FavRect.left += FavRect.Width() * 0.75;

		m_cFavouritesWindow.MoveWindow(FavRect);
	}
#endif

	clientrect.top = (clientrect.bottom - iControlHeight) + 4;

	//
	// Move the top row of controls to the correct vertical position,
	// leaving their horizontal position as it was set up in the dialog.
	//
	int iIDList[] = 
	{
		IDC_TEXTURESIZE,
		IDC_SIZEPROMPT,
		IDC_FILTERPROMPT,
		IDC_FILTER,
		IDC_CURNAME,
#ifndef SLE_NO_TEXTURE_KEYWORDS 
		IDC_KEYWORDS_TEXT, //// SLE CHANGE - keep keywords at top too
		IDC_KEYWORDS,
#endif
		IDC_FILTER_OPAQUE,
		IDC_FILTER_TRANSLUCENT,
		IDC_TEXTURES_OPEN_SOURCE,
#ifdef SLE //// SLE NEW - for Portal
		IDC_FILTER_PORTALABLE,
#endif
#ifdef SLE_TEXTUREBROWSER_FAVOURITES
		IDC_TOFAVS,
		IDC_FAVSLIST,
#endif
		-1
	};

	for (int i = 0; iIDList[i] != -1; i++)
	{
		CWnd *pWnd = GetDlgItem(iIDList[i]);
		Assert(pWnd != NULL);

		if (pWnd != NULL)
		{
			pWnd->GetWindowRect(&CtrlRect);
			ScreenToClient(CtrlRect);
			iThisCtrlHeight = CtrlRect.bottom - CtrlRect.top;
			CtrlRect.top = clientrect.top;
			CtrlRect.bottom = CtrlRect.top + iThisCtrlHeight;
#if 0 //def SLE_TEXTUREBROWSER_FAVOURITES // todo, later
			// move the buttons to align with the left edge of the fav. browser
			if ( g_pGameConfig->m_MaterialFavouritesCount > 0 && (i > 9) )
			{
				CtrlRect.left = (i == 10) ? iFavBrowserLeft : iFavBrowserLeft + CtrlRect.Width() + 8; // 10 is +Favs, 11 is Favs List
				CtrlRect.right = CtrlRect.left + CtrlRect.Width();
			}
#endif
			pWnd->MoveWindow(CtrlRect);
		}
	}

	//
	// Move the middle row of controls to the correct vertical position,
	// leaving their horizontal position as it was set up in the dialog.
	//
	int iIDList2[] = 
	{
#ifndef SLE_NO_TEXTURE_KEYWORDS
#if 0 // these shouldn't be in this list or they get moved off-screen
		IDC_KEYWORDS_TEXT,
		IDC_KEYWORDS,
#endif
#endif
		IDC_SHOW_ONLY_USED_TEXTURES,
		IDC_MARK,
		IDC_REPLACE,
		IDC_CURDESCRIPTION,

		IDC_FILTER_SELFILLUM,
		IDC_FILTER_SPECULAR,
		IDC_SHOW_ERROR,

		IDC_TEXTURES_RELOAD,
		IDC_TEXTURES_OPEN_SOURCE,
#ifdef SLE_TEXTUREBROWSER_FAVOURITES
		IDC_FROMFAVS,
#endif
		-1
	};

	for (int i = 0; iIDList2[i] != -1; i++)
	{
		CWnd *pWnd = GetDlgItem(iIDList2[i]);
		Assert(pWnd != NULL);

		if (pWnd != NULL)
		{
			pWnd->GetWindowRect(&CtrlRect);
			ScreenToClient(CtrlRect);
			iThisCtrlHeight = CtrlRect.bottom - CtrlRect.top;
			CtrlRect.top = clientrect.top + iControlHeight / 2 + 2;
			CtrlRect.bottom = CtrlRect.top + iThisCtrlHeight;
			pWnd->MoveWindow(CtrlRect);
		}
	}

	CDialog::OnSize(nType, cx, cy);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bUsed - 
//-----------------------------------------------------------------------------
void CTextureBrowser::SetUsed(BOOL bUsed)
{
	m_bUsed = bUsed;

	if (m_bUsed)
	{
		CUsedTextureList Used;
		GetActiveWorld()->GetUsedTextures(Used);

		m_TextureSubList.RemoveAll();
		for (int i = 0; i < Used.Count(); i++)
		{
			TextureWindowTex_t Tex;
			Tex.pTex = Used.Element(i).pTex;
			Tex.nUsageCount = Used.Element(i).nUsageCount;

			m_TextureSubList.AddToTail(Tex);
		}

		m_cTextureWindow.SetSpecificList(&m_TextureSubList);
	}
	else
	{
		m_cTextureWindow.SetSpecificList(NULL);
	}
}
#ifdef SLE_TEXTUREBROWSER_FAVOURITES
void CTextureBrowser::AddFavourites(void)
{
	if ( g_pGameConfig->m_MaterialFavouritesCount < 1 ) return;

	if ( m_cFavouritesWindow )
	{
		m_cFavouritesWindow.ShowWindow(SW_SHOW);
		
		m_FavouritesSubList.RemoveAll();
		for ( int i = 0; i < g_pGameConfig->m_MaterialFavouritesCount; i++ )
		{
			TextureWindowTex_t Tex;
			Tex.pTex = g_Textures.FindActiveTexture(g_pGameConfig->m_MaterialFavourites[ i ].szName);
			if ( !Tex.pTex ) continue;

			Tex.nUsageCount = 0;
			m_FavouritesSubList.AddToTail(Tex);
		}

		m_cFavouritesWindow.SetSpecificList(&m_FavouritesSubList);
	} 
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTextureBrowser::OnClose(void)
{
	WriteSettings();
	SaveAndExit();
	CDialog::OnCancel();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTextureBrowser::OnCancel()
{
	WriteSettings();
	SaveAndExit();
	CDialog::OnCancel();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTextureBrowser::OnShowOnlyUsedTextures()
{
	if(!GetActiveWorld())
		return;

	SetUsed(m_cUsed.GetCheck());
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pszTexture - 
//-----------------------------------------------------------------------------
void CTextureBrowser::SetInitialTexture(LPCTSTR pszTexture)
{
	strcpy(szInitialTexture, pszTexture);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTextureBrowser::OnSelendokTexturesize() 
{
	// change size of textures the texture window displays
	int iCurSel = m_cSizeList.GetCurSel();

	switch(iCurSel)
	{
	case 0:
		m_cTextureWindow.SetDisplaySize(128);
		break;
	case 1:
		m_cTextureWindow.SetDisplaySize(256);
		break;
	case 2:
		m_cTextureWindow.SetDisplaySize(512);
		break;
	}
#ifdef SLE_TEXTUREBROWSER_FAVOURITES
	if ( m_cFavouritesWindow )
	{
		m_cFavouritesWindow.SetDisplaySize(m_cTextureWindow.GetDisplaySize() / 2);
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Called when either the filter combo or the keywords combo text changes.
//-----------------------------------------------------------------------------
void CTextureBrowser::OnChangeFilterOrKeywords() 
{
	//
	// Start a timer to repaint the texture window using the new filters.
	//
	m_uLastFilterChange = time(NULL);
	m_bFilterChanged = TRUE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#ifndef SLE_NO_TEXTURE_KEYWORDS
void CTextureBrowser::OnUpdateFiltersNOW() 
{
	m_uLastFilterChange = time(NULL);
	m_bFilterChanged = FALSE;

	CString str;
	int iSel = m_cFilter.GetCurSel();
	m_cFilter.GetLBText(iSel, str);
	m_cTextureWindow.SetNameFilter(str);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTextureBrowser::OnUpdateKeywordsNOW() 
{
	m_uLastFilterChange = time(NULL);
	m_bFilterChanged = FALSE;

	CString str;
	int iSel = m_cKeywords.GetCurSel();
	m_cKeywords.GetLBText(iSel, str);
	m_cTextureWindow.SetKeywords(str);
}
#endif
//-----------------------------------------------------------------------------
// Purpose: Timer used to control updates when the filter terms change.
// Input  : nIDEvent - 
//-----------------------------------------------------------------------------
void CTextureBrowser::OnTimer(UINT nIDEvent) 
{
	if (!m_bFilterChanged)
	{
		return;
	}

	if ((time(NULL) - m_uLastFilterChange) > 0)
	{
		KillTimer(nIDEvent);
		m_bFilterChanged = FALSE;

		m_cTextureWindow.EnableUpdate(false);

		CString str;
		m_cFilter.GetWindowText(str);
		m_cTextureWindow.SetNameFilter(str);

		m_cTextureWindow.EnableUpdate(true);
#ifndef SLE_NO_TEXTURE_KEYWORDS
		m_cKeywords.GetWindowText(str);
		m_cTextureWindow.SetKeywords(str);
#endif
		SetTimer(nIDEvent, 500, NULL);
	}

#ifdef SLE_TEXTUREBROWSER_FAVOURITES
#endif

	CDialog::OnTimer(nIDEvent);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : wParam - 
//			lParam - 
// Output : LRESULT
//-----------------------------------------------------------------------------
LRESULT CTextureBrowser::OnTextureWindowDblClk(WPARAM wParam, LPARAM lParam)
{
	WriteSettings();
	SaveAndExit();
	return(0);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : wParam - 
//			lParam - 
// Output : LRESULT
//-----------------------------------------------------------------------------
LRESULT CTextureBrowser::OnTexturewindowSelchange(WPARAM wParam, LPARAM lParam)
{
	IEditorTexture *pTex = g_Textures.FindActiveTexture(m_cTextureWindow.szCurTexture);
#ifdef SLE_TEXTUREBROWSER_FAVOURITES
	if ( m_cFavouritesWindow.m_iLastClickTimeStamp > m_cTextureWindow.m_iLastClickTimeStamp )
	{
		pTex = g_Textures.FindActiveTexture(m_cFavouritesWindow.szCurTexture);
	}
#endif
	CString str;
	char szName[MAX_PATH];

	if (pTex != NULL)
	{
		// create description of texture
#ifdef SLE //// SLE NEW - show reflectivity and shader next to size
		if (pTex->GetMaterial())
		{
			Vector reflectivity = vec3_origin;
			pTex->GetMaterial()->GetReflectivity(reflectivity);
			str.Format("Size: %dx%d | Shader: %s | Reflectivity (RGB): %.3f %.3f %.3f",
				pTex->GetWidth(), pTex->GetHeight(),
				pTex->GetMaterial()->GetShaderName(),
				(float)reflectivity[0], (float)reflectivity[1], (float)reflectivity[2]);
			pTex->GetShortName(szName);
		}
		else
		{
			str.Format("Size: %dx%d",
				pTex->GetWidth(), pTex->GetHeight());
		}
#else
		str.Format("%dx%d", pTex->GetWidth(), pTex->GetHeight());
#endif
	}
	else
	{
		szName[0] = '\0';
	}
	
#ifdef SLE //// SLE CHANGE - only do this for VMTs, not WADs
	if ( g_pGameConfig->GetTextureFormat() != tfVMT )
	{
		//// SLE TODO - there is a function for getting short name for WADs, but the string is corrupted. Fix later.
		return( 0 );
	}
#endif
	m_cCurName.SetWindowText(szName);
	m_cCurDescription.SetWindowText(str);

	return(0);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTextureBrowser::WriteSettings()
{
	// write position information
	CWinApp *pApp = AfxGetApp();
	CString str;
	CRect r;
	GetWindowRect(r);
	str.Format("%d %d %d %d", r.left, r.top, r.right, r.bottom);
	pApp->WriteProfileString(pszIniSection, "Position", str);
	pApp->WriteProfileInt(pszIniSection, "ShowSize", m_cSizeList.GetCurSel());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTextureBrowser::SaveAndExit()
{
	// save current filter string
	CString str;
	m_cFilter.GetWindowText(str);

	int i;
	for(i = 0; i < m_nFilterHistory; i++)
	{
		if(!m_FilterHistory[i].CompareNoCase(str))
			break;
	}

	if(i != m_nFilterHistory)	// delete first
	{
		m_FilterHistory.RemoveAt(i);
		--m_nFilterHistory;
	}
	
	m_FilterHistory.InsertAt(0, str);
	++m_nFilterHistory;
#ifndef SLE_NO_TEXTURE_KEYWORDS
	m_cKeywords.GetWindowText(m_szLastKeywords, sizeof(m_szLastKeywords));
#endif
	EndDialog(IDOK);
}

//-----------------------------------------------------------------------------
// Purpose: Sets a name filter that will override whatever is in the history
//			for this browser session.
//-----------------------------------------------------------------------------
void CTextureBrowser::SetFilter(const char *pszFilter)
{
	if (pszFilter)
	{
		strcpy(m_szNameFilter, pszFilter);
	}
	else
	{
		m_szNameFilter[0] = '\0';
	}
}

//-----------------------------------------------------------------------------
// Filter buttons
//-----------------------------------------------------------------------------
void CTextureBrowser::OnFilterOpaque(void)
{
	bool checked = m_FilterOpaque.GetCheck( ) != 0;
	m_cTextureWindow.SetTypeFilter( CTextureWindow::TYPEFILTER_OPAQUE, checked ); 
}

void CTextureBrowser::OnFilterTranslucent(void)
{
	bool checked = m_FilterTranslucent.GetCheck( ) != 0;
	m_cTextureWindow.SetTypeFilter( CTextureWindow::TYPEFILTER_TRANSLUCENT, checked ); 
}

void CTextureBrowser::OnFilterSelfIllum(void)
{
	bool checked = m_FilterSelfIllum.GetCheck( ) != 0;
	m_cTextureWindow.SetTypeFilter( CTextureWindow::TYPEFILTER_SELFILLUM, checked ); 
}

void CTextureBrowser::OnFilterSpecular(void)
{
	bool checked = m_FilterSpecular.GetCheck( ) != 0;
	m_cTextureWindow.SetTypeFilter( CTextureWindow::TYPEFILTER_SPECULAR, checked ); 
}

void CTextureBrowser::OnShowErrors(void)
{
	bool checked = m_ShowErrors.GetCheck( ) != 0;
	m_cTextureWindow.ShowErrors( checked ); 
}

//-----------------------------------------------------------------------------
// Opens the source file: 
//-----------------------------------------------------------------------------
void CTextureBrowser::OnOpenSource()
{
	if ( m_cTextureWindow.szCurTexture[0] )
	{
		g_Textures.OpenSource( m_cTextureWindow.szCurTexture );
	}
}

void CTextureBrowser::OnReload()
{
	if ( m_cTextureWindow.szCurTexture[0] )
	{
		g_Textures.ReloadTextures( m_cTextureWindow.szCurTexture );
		m_cTextureWindow.Invalidate();

		if (GetMainWnd())
		{
			GetMainWnd()->m_TextureBar.NotifyGraphicsChanged();
			GetMainWnd()->m_pFaceEditSheet->NotifyGraphicsChanged();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTextureBrowser::OnMark(void)
{
	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	if (pDoc != NULL)
	{
		pDoc->ReplaceTextures(m_cTextureWindow.szCurTexture, "", TRUE, 0x100, FALSE, FALSE);
		EndDialog(IDOK);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Invokes the replace texture dialog.
//-----------------------------------------------------------------------------
void CTextureBrowser::OnReplace(void)
{
	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	if(!pDoc)
		return;

	CReplaceTexDlg dlg(pDoc->GetSelection()->GetCount());

	dlg.m_strFind = m_cTextureWindow.szCurTexture;

	if(dlg.DoModal() != IDOK)
		return;
	
	// mark undo position
	GetHistory()->MarkUndoPosition(pDoc->GetSelection()->GetList(), "Replace Textures");

	if(dlg.m_bMarkOnly)
	{
		pDoc->SelectObject(NULL, scClear);	// clear selection first
	}

	dlg.DoReplaceTextures();

	//EndDialog(IDOK);

	if (m_bUsed)
	{
		SetUsed(TRUE);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the texture format for browsing. Only textures of the given
//			format will be visible in the browse window. By default, this
//			format will be the same as the current active texture format.
// Input  : eTextureFormat - Texture format to use for browsing.
//-----------------------------------------------------------------------------
void CTextureBrowser::SetTextureFormat(TEXTUREFORMAT eTextureFormat)
{
	m_cTextureWindow.SetTextureFormat(eTextureFormat);
}

#ifdef SLE_TEXTUREBROWSER_FAVOURITES
void CTextureBrowser::OnAddCurrentMaterialToFavs()
{
	if ( m_cTextureWindow.szCurTexture[ 0 ] )
	{
		// check if it isn't already in the list
		for ( int i = 0; i < g_pGameConfig->m_MaterialFavouritesCount; i++ )
		{
			if ( !Q_strcmp(g_pGameConfig->m_MaterialFavourites[ i ].szName, m_cTextureWindow.szCurTexture) )
			{
				AfxMessageBox("This material is already included in the list of Favourites!\nMake sure to use the Favourites browser (right) for adding texturs to Favourites!", MB_OK);
				return;
			}
		}
		//
		// add name of the texture to the global favourites list
		//
		int ndx = g_pGameConfig->m_MaterialFavouritesCount;

		g_pGameConfig->m_MaterialFavouritesCount++;

		int index = g_pGameConfig->m_MaterialFavourites.AddToTail();
		Q_strncpy(g_pGameConfig->m_MaterialFavourites[ index ].szName, m_cTextureWindow.szCurTexture, sizeof (g_pGameConfig->m_MaterialFavourites[ index ].szName));

		if ( m_cFavouritesWindow )
		{
			g_Textures.ReloadTextures(m_cFavouritesWindow.szCurTexture);
			m_cFavouritesWindow.Invalidate();

			if ( GetMainWnd() )
			{
				GetMainWnd()->m_TextureBar.NotifyGraphicsChanged();
				GetMainWnd()->m_pFaceEditSheet->NotifyGraphicsChanged();
			}
		}
	}
}

void CTextureBrowser::OnRemoveCurrentMaterialFromFavs()
{
	if ( m_cFavouritesWindow && m_cFavouritesWindow.szCurTexture[ 0 ] )
	{
		// find its position in the list
		for ( int i = 0; i < g_pGameConfig->m_MaterialFavouritesCount; i++ )
		{
			if ( !Q_strcmp(g_pGameConfig->m_MaterialFavourites[ i ].szName, m_cFavouritesWindow.szCurTexture) )
			{
				// remove
				g_pGameConfig->m_MaterialFavourites.Remove(i);
				g_pGameConfig->m_MaterialFavouritesCount--;
			}
			else
			{
				AfxMessageBox("The selected texture is not found in the Favourites list.\nMake sure to use the Favourites browser (right) to select the texture to be removed!", MB_OK);
				return;
			}
		}

		g_Textures.ReloadTextures(m_cFavouritesWindow.szCurTexture);
		m_cFavouritesWindow.Invalidate();

		if ( GetMainWnd() )
		{
			GetMainWnd()->m_TextureBar.NotifyGraphicsChanged();
			GetMainWnd()->m_pFaceEditSheet->NotifyGraphicsChanged();
		}
	}
}

void CTextureBrowser::OnOpenMaterialConfig()
{
	GetMainWnd()->ConfigureMaterials();
}
#endif
