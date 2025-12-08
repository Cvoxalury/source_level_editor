//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TEXTUREBROWSER_H
#define TEXTUREBROWSER_H
#pragma once


#include "resource.h"
#include "AutoSelCombo.h"
#include "texturewindow.h"
#ifdef SLE
#define SLE_TEXTUREBROWSER_FAVOURITES //// SLE NEW - a tab where you can pin your often used textures in the Texture Browser
#endif
#ifdef SLE_TEXTUREBROWSER_FAVOURITES
//// undone, a regular mini-browser is used
/*
class CTextureFavourites : public CWnd
{
public:
	CTextureFavourites() {}
	virtual ~CTextureFavourites() {}

	void Create(CWnd *pParentWnd, RECT& rect);

};
*/
#endif
#ifdef SLE //// SLE NEW - a tab that lists the contents of a texture's vmt
/*
class CTextureVMTView : public CWnd
{
public:
	CTextureVMTView();
	virtual ~CTextureVMTView();

	void Create(CWnd *pParentWnd, RECT& rect);

};
*/
#endif
class CTextureBrowser : public CDialog
{
public:
	CTextureBrowser(CWnd *pParent);

	//{{AFX_DATA(CTextureBrowser)
	enum { IDD = IDD_TEXTURES };
	//}}AFX_DATA

	void SetInitialTexture(LPCTSTR);
	inline CString GetCurTexture(void);
	void WriteSettings();
	void SetUsed(BOOL);
	void SaveAndExit();
	void SetFilter(const char *pszFilter);
	void SetTextureFormat(TEXTUREFORMAT eTextureFormat);

	CTextureWindow m_cTextureWindow; // dvs: make protected
#ifdef SLE_TEXTUREBROWSER_FAVOURITES
	CTextureWindow m_cFavouritesWindow;
	void AddFavourites(void);
#endif
protected:
	//{{AFX_MSG(CTextureBrowser)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelendokTexturesize();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnShowOnlyUsedTextures();
	afx_msg void OnReplace();
	afx_msg void OnMark();

	afx_msg void OnFilterOpaque();
	afx_msg void OnFilterTranslucent();
	afx_msg void OnFilterSelfIllum();
	afx_msg void OnFilterSpecular();
	afx_msg void OnShowErrors();

	afx_msg void OnOpenSource();
	afx_msg void OnReload();
	afx_msg void OnChangeFilterOrKeywords(void);
#ifndef SLE_NO_TEXTURE_KEYWORDS
	afx_msg void OnUpdateFiltersNOW();
	afx_msg void OnUpdateKeywordsNOW(void);
#endif
	afx_msg LRESULT OnTextureWindowDblClk(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTexturewindowSelchange(WPARAM, LPARAM);
#ifdef SLE_TEXTUREBROWSER_FAVOURITES
	afx_msg void OnAddCurrentMaterialToFavs();
	afx_msg void OnRemoveCurrentMaterialFromFavs();
	afx_msg void OnOpenMaterialConfig();
#endif
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	afx_msg void OnCancel();

	static CStringArray m_FilterHistory;
	static int m_nFilterHistory;
#ifndef SLE_NO_TEXTURE_KEYWORDS
	static char m_szLastKeywords[MAX_PATH];	// The text in the keywords combo when the user last exited the browser.
#endif
	CComboBox m_cSizeList;
	CStatic m_cCurName;
	CStatic m_cCurDescription;
	CButton m_cUsed;
	char szInitialTexture[128];
	char m_szNameFilter[128];				// Overrides the name filter history for a single browser session.
	DWORD m_uLastFilterChange;
	BOOL m_bFilterChanged;
	BOOL m_bUsed;

	CAutoSelComboBox m_cFilter;
#ifndef SLE_NO_TEXTURE_KEYWORDS
	CAutoSelComboBox m_cKeywords;
#endif
	CButton m_FilterOpaque;
	CButton m_FilterTranslucent;
	CButton m_FilterSelfIllum;
	CButton m_FilterSpecular;
	CButton m_ShowErrors;

	TextureWindowTexList m_TextureSubList;	// Holds a specific sublist of textures to browse.
#ifdef SLE_TEXTUREBROWSER_FAVOURITES
	TextureWindowTexList m_FavouritesSubList; // isolated list for favourites
#endif
};

//-----------------------------------------------------------------------------
// Purpose: Returns the name of the currently selected texture.
//-----------------------------------------------------------------------------
CString CTextureBrowser::GetCurTexture(void)
{
	return(CString(m_cTextureWindow.szCurTexture));
}

#endif // TEXTUREBROWSER_H
