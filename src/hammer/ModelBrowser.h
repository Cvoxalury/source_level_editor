//========= Copyright Valve Corporation, All rights reserved. ============//
#pragma once

#include "resource.h"
#include "VGuiWnd.h"
#ifdef SLE //// SLE NEW - backports for matsys_controls
#include "matsys_controls\baseassetpicker.h"
#include "utlvector.h"
#endif
// CModelBrowser dialog

namespace vgui
{
	class TextEntry;
	class Splitter;
	class Button;
}

class CModelBrowserPanel;
class CMDLPicker;
#ifdef SLE //// SLE NEW - backports for matsys_controls
#define ID_FIND_ASSET	100
#endif
// Hammer-2013 port
class CModelBrowser : public CDialog
{
	DECLARE_DYNAMIC(CModelBrowser)

public:
	CModelBrowser(CWnd* pParent = NULL);   // standard constructor
	virtual ~CModelBrowser();
#ifdef SLE //// SLE NEW - backports for matsys_controls
	void SetUsedModelList(CUtlVector<AssetUsageInfo_t> &usedModels);
#endif
	void	SetModelName( const char *pModelName );
	void	GetModelName( char *pModelName, int length );
	void	GetSkin( int &nSkin );
	void	SetSkin( int nSkin );

// Dialog Data
	enum { IDD = IDD_MODEL_BROWSER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage( MSG* pMsg ); 


	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
#ifdef SLE
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	/*
	afx_msg BOOL OnEraseBkgnd(CDC* pDC)
	{
		CRect r;
		pDC->GetClipBox(&r);

		pDC->FillSolidRect(r, RGB(100, 100, 100)); //// SLE NEW - set colour background in model viewer
		return TRUE;
	}
	*/
#endif
	virtual BOOL OnInitDialog();

	void UpdateStatusLine();
	void SaveLoadSettings( bool bSave ); 
	void Resize( void );

	CVGuiPanelWnd	m_VGuiWindow;

	CMDLPicker		*m_pPicker;
	vgui::Button	*m_pButtonOK;
	vgui::Button	*m_pButtonCancel;
	vgui::TextEntry	*m_pStatusLine;

	void Show();
	void Hide();
	
};
