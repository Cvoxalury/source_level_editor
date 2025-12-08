//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef OBJECTBAR_H
#define OBJECTBAR_H
#ifdef _WIN32
#pragma once
#endif

#include "AutoSelCombo.h"
#include "HammerBar.h"
#include "FilteredComboBox.h"

class CMapView;
class BoundBox;
class CMapClass;
class Vector;
class CPrefab;

#define MAX_PREV_SEL 12

class CObjectBar : public CHammerBar, public CFilteredComboBox::ICallbacks
{
public:	
	CObjectBar();
	BOOL Create(CWnd *pParentWnd);
	
	static LPCTSTR GetDefaultEntityClass(void);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	
	void UpdateListForTool(int iTool);
	void SetupForBlockTool();
	void DoHideControls();
	CMapClass *CreateInBox(BoundBox *pBox, CMapView *pView = NULL);
	BOOL GetPrefabBounds(BoundBox *pBox);
	
	// If this is on, then it'll randomize the yaw when entities are placed.
	bool UseRandomYawOnEntityPlacement();
#ifdef SLE //// SLE NEW - align placed entity to be perpendicular to surface
	bool UseAlignOnEntityPlacement();
#endif
	void DoDataExchange(CDataExchange *pDX);
#ifdef SLE //// SLE NEW - more util function for TooBlock
	bool IsBlockToolCreatingPrimitive(void);
	bool IsBlockToolCreatingExtendedPrimitive(void);
	bool IsBlockToolCreatingPrefab(void);
	int GetBlockToolActiveCategory(void);
	int GetBlockToolActivePrimitive(void);
	int GetBlockToolFaceCount(void);
	int GetBlockToolStartAngle(void);
	void ResetEntityTool(void);
#endif
	bool IsEntityToolCreatingPrefab( void );
	bool IsEntityToolCreatingEntity( void );
	CMapClass *BuildPrefabObjectAtPoint( Vector const &HitPos );
	
// CFilteredComboBox::ICallbacks implementation.

	virtual void OnTextChanged( const char *pText );
	
private:	
	enum 
	{
		listPrimitives,
#ifdef SLE //// SLE NEW - more stock stolids
		listExtendedPrimitives,
#endif
		listPrefabs,
		listEntities
	} ListType;

	//{{AFX_DATA(CMapViewBar)
	enum { IDD = IDD_OBJECTBAR };
	//}}AFX_DATA
	
	CFilteredComboBox	m_CreateList;				// this should really be m_ItemList
	CComboBox			m_CategoryList;
	CEdit				m_Faces;
	CSpinButtonCtrl		m_FacesSpin; 
#ifdef SLE //// SLE NEW - set start angle for block tool
	CEdit				m_StartAngle;
	CSpinButtonCtrl		m_StartAngleSpin;
#endif
	CPrefab*			FindPrefabByName( const char *pName );

	void LoadBlockCategories( void );
#ifdef SLE //// SLE NEW - more stock solids
	void LoadExtBlockCategories(void);
	void LoadExtBlockItems( void );
#endif
	void LoadEntityCategories( void );
	void LoadPrefabCategories( void );	

	void LoadBlockItems( void );
	void LoadEntityItems( void );
	void LoadPrefabItems( void );

	int UpdatePreviousSelection( int iTool );

	int GetPrevSelIndex(DWORD dwGameID, int *piNewIndex = NULL);
	BOOL EnableFaceControl(CWnd *pWnd, BOOL bModifyWnd);
#ifdef SLE //// SLE NEW - set start angle for block tool
	BOOL EnableAngleControl(CWnd *pWnd, BOOL bModifyWnd);
#endif
	int iEntitySel;
	int iBlockSel;
	
	// previous selections:
	DWORD m_dwPrevGameID;
	struct tagprevsel
	{
		DWORD dwGameID;
		struct tagblock
		{
			CString strItem;
			CString strCategory;
		} block;
		struct tagentity
		{
			CString strItem;
			CString strCategory;
		} entity;
	} 
	m_PrevSel[MAX_PREV_SEL];
	int m_iLastTool;
	
protected:	
	afx_msg void UpdateControl(CCmdUI*);
	afx_msg void UpdateFaceControl(CCmdUI*);
	afx_msg void OnCategorylistSelchange();
	afx_msg void OnChangeCategory();
#ifdef SLE
	//// SLE NEW - set start angle for block tool
	afx_msg void UpdateAngleControl(CCmdUI*);
	afx_msg void OnChangeStartAngle();
	afx_msg void OnChangeStartAngleSpin();

	afx_msg void OnChangeFaceCount();
	afx_msg void OnChangeFaceSpin();
#endif
	DECLARE_MESSAGE_MAP()
};

#endif // OBJECTBAR_H