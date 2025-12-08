//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FACEEDIT_MATERIALPAGE_H
#define FACEEDIT_MATERIALPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "resource.h"
#include "TextureBox.h"
#include "IEditorTexture.h"
#include "wndTex.h"
#include "MapFace.h"
#include "SmoothingGroupsDlg.h" // renamed for clarity

class CMapSolid;

// Flags for the Apply function
#define FACE_APPLY_MATERIAL			0x01
#define FACE_APPLY_MAPPING			0x02
#define FACE_APPLY_LIGHTMAP_SCALE	0x04
#define FACE_APPLY_ALIGN_EDGE		0x08	// NOT included in FACE_APPLY_ALL!
#define FACE_APPLY_CONTENTS_DATA	0x10
#ifdef SLE
#define FACE_APPLY_SHEAR			0x20 //// SLE NEW - texture shear
#else
#endif
#define FACE_APPLY_ALL				FACE_APPLY_MATERIAL | FACE_APPLY_MAPPING | FACE_APPLY_LIGHTMAP_SCALE

class CFaceEditMaterialPage : public CPropertyPage
{
	
	DECLARE_DYNAMIC( CFaceEditMaterialPage );

public:
	enum
	{
		MATERIALPAGETOOL_NONE = 0,
		MATERIALPAGETOOL_MATERIAL,
		MATERIALPAGETOOL_SMOOTHING_GROUP
	};

	//=========================================================================
	//
	// Creation/Destruction
	//
	CFaceEditMaterialPage();
	~CFaceEditMaterialPage();

	void Init( void );

	//=========================================================================
	//
	// Update
	//
#ifdef SLE //// SLE CHANGE - add argument for manual vs automatic clicking, because of m_pickByAngle_bool
	void ClickFace( CMapSolid *pSolid, int faceIndex, int cmd, int clickMode = -1, bool manual = false );	// primary interface update call
#else
	void ClickFace( CMapSolid *pSolid, int faceIndex, int cmd, int clickMode = -1 );	// primary interface update call
#endif
	void Apply( CMapFace *pOnlyFace, int flags );

	void NotifyGraphicsChanged( void );
	void UpdateDialogData( CMapFace *pFace = NULL );

	void SetMaterialPageTool( unsigned short iMaterialTool );
	unsigned short GetMaterialPageTool( void )					 { return m_iMaterialTool; }

	// Called when a new material is detected.
	void NotifyNewMaterial( IEditorTexture *pTex );

	//=========================================================================
	//
	// Dialog Data
	//
	//{{AFX_DATA( CFaceEditMaterialPage )
	enum { IDD = IDD_FACEEDIT };
	//}}AFX_DATA

	//=========================================================================
	//
	// Virtual Overrides
	//
	//{{AFX_VIRTUAL( CFaceEditMaterialPage )
	BOOL OnSetActive( void );
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	//}}AFX_VIRTUAL

	//=========================================================================
	//
	// Face Attributes
	//
	struct FaceAttributeInfo_t
	{
		unsigned int uControlID;		// Control ID of corresponding checkbox.
		unsigned int *puAttribute;		// Pointer to bit flags attribute being modified.
		unsigned int uFlag;				// Bit flag(s) to set in the above attribute.
	};

	static unsigned int m_FaceContents;
	static unsigned int m_FaceSurface;

protected:
	CEdit				m_shiftX;
	CEdit				m_shiftY;
	CEdit				m_scaleX;
	CEdit				m_scaleY;
	CEdit				m_rotate;
	CEdit				m_cLightmapScale;
	CButton				m_cHideMask;
	CButton				m_cExpand;
	wndTex				m_texture;

#ifdef SLE //// SLE NEW - texture shear
	CEdit				m_shearX;
	CEdit				m_shearY;
	CEdit				m_shearZ;

	BOOL				m_shearLock_bool;
	//// SLE NEW - grow/shrink face selection
	CEdit				m_cGrowAngle;
	CButton				m_cGrowButton;
	CButton				m_cShrinkButton;
	
	//// SLE NEW - texture shift step control
	CEdit				m_shiftStepX;
	CEdit				m_shiftStepY;
public:
	BOOL				m_pickByAngle_bool;
protected:
#endif
	BOOL				m_bInitialized;
	BOOL				m_bHideMask;
	BOOL				m_bIgnoreResize;
	BOOL				m_bTreatAsOneFace;				// whether to consider all selected faces as one face.

	FaceOrientation_t	m_eOrientation;					// The orientation of the lifted face.

	IEditorTexture		*m_pCurTex;
	wndTex				m_TexturePic;
	CTextureBox			m_TextureList;
	CComboBox			m_TextureGroupList;

	unsigned short		m_iMaterialTool;
	CFaceSmoothingDlg	m_FaceSmoothDlg;

	void SetReadOnly( bool bIsReadOnly );

	//=========================================================================
	//
	// Texture Browser/Update 
	//
	void SelectTexture( LPCSTR pszTextureName );
	void UpdateTexture( void );

	//=========================================================================
	//
	// Texture Alignement
	//
	void AlignToView( CMapFace *pFace );
	void CopyTCoordSystem( const CMapFace *pFrom, CMapFace *pTo );
	void GetAllFaceExtents( Extents_t Extents );

	//=========================================================================
	//
	// Message Map
	//
	//{{AFX_MSG( CFaceEditMaterialPage )
	afx_msg void OnButtonApply( void );
	afx_msg BOOL OnAlign(UINT uCmd);
	afx_msg void OnHideMask();
	afx_msg BOOL OnJustify( UINT uCmd );
	afx_msg void OnMode();
	afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar );
	afx_msg void OnDeltaPosFloatSpin( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnSize( UINT, int, int );
	afx_msg void OnSelChangeTexture( void );
	afx_msg void OnCheckUnCheck( void );
	afx_msg void OnTreatAsOne( void );
	afx_msg void OnReplace( void );
	afx_msg BOOL OnSwitchMode( UINT id );
	afx_msg void OnBrowse( void );
	afx_msg void OnChangeTextureGroup( void );
	afx_msg void OnButtonSmoothingGroups( void );
#ifdef SLE
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnShiftXRandom(void); //// SLE NEW - randomise texture shift buttons
	afx_msg void OnShiftYRandom(void);
	afx_msg BOOL OnFitMode(UINT id); //// SLE NEW - extra fit modes
	afx_msg void OnFitExpand(); //// SLE CHANGE - Fit now works as an expand list for extra fit modes
	afx_msg void OnButtonApplyNodraw(void); //// SLE NEW - button to texture selected faces as nodraw 
	afx_msg void OnButtonMarkAll(void); //// SLE NEW - mark button on texture tool, without needing to open texture browser every time
public:
	afx_msg void OnGrowSelection();
	afx_msg void OnShrinkSelection();
	afx_msg void OnPickByAngle(void);
	afx_msg void OnClickPickByAngle(CMapFace *face, int cmd, int clickMode = -1);
	afx_msg void OnToggleShearLock(void);
protected:
	afx_msg void OnClose();
	virtual BOOL OnKillActive();
#endif
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

#ifdef SLE
public:
	CMapFaceList m_growIgnoreList;
	float m_growAngle_float;
#endif
};

#endif // FACEEDIT_MATERIALPAGE_H