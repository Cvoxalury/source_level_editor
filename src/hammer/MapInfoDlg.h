//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef MAPINFODLG_H
#define MAPINFODLG_H
#pragma once

class CMapInfoDlg : public CDialog
{
	// Construction
	public:
		CMapInfoDlg(CMapWorld *, CWnd* pParent = NULL);   // standard constructor

		void CountEntity(CMapEntity *pEntity);
		void CountFace(CMapFace *pFace);
		void CountSolid(CMapSolid *pSolid);
		void CountTexture(IEditorTexture *pTex);

	// Dialog Data
		//{{AFX_DATA(CMapInfoDlg)
		enum { IDD = IDD_MAPINFO };
		CStatic	m_Faces;
		CStatic	m_Solids;
		CStatic	m_SolidEntities;
		CStatic	m_PointEntities;
		CStatic	m_TextureMemory;
		CStatic	m_UniqueTextures;
		CListBox m_WadsUsed;
#ifdef SLE //// SLE NEW - improved Map Information dialogue
		CButton	m_cMapInfoOnlyVisible;
		CStatic	m_Disps;
		CStatic m_StaticProps;
		CStatic m_Overlays;
#endif
		//}}AFX_DATA

		CMapWorld *pWorld;

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CMapInfoDlg)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

		UINT m_uSolidCount;
		UINT m_uPointEntityCount;
		UINT m_uSolidEntityCount;
		UINT m_uFaceCount;
		UINT m_uUniqueTextures;
		UINT m_uTextureMemory;
#ifdef SLE //// SLE NEW - improved Map Information dialogue
		UINT m_uDispCount;
		UINT m_uStaticPropsCount;
		UINT m_uOverlaysCount;
#endif
		IEditorTexture *m_pTextures[1024];

	// Implementation
	protected:
		// Generated message map functions
		//{{AFX_MSG(CMapInfoDlg)
		virtual BOOL OnInitDialog();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()

#ifdef SLE //// SLE NEW - improved Map Information dialogue
		afx_msg void OnOnlyVisible();
		void UpdateDialogue();
		void OnOpenSource();
public:
		BOOL m_bMapInfoOnlyVisible;
#endif
};

#endif // MAPINFODLG_H