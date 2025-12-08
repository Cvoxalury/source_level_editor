//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef OBJECTPROPERTIES_H
#define OBJECTPROPERTIES_H
#pragma once

#include "MapClass.h"
#include "EditGameClass.h"
#include "ObjectPage.h"
#include "OP_Groups.h"
#include "AnchorMgr.h"

class COP_Entity;
class COP_Flags;
class COP_Output;
class COP_Input;
class COP_Model;

enum LayoutType_t;

//
// Wrap map classes in CObject so we can use runtime identification
// to pass edit data to the object pages.
//
class editCMapClass : public CObject, public CMapClass
{
	DECLARE_DYNAMIC(editCMapClass)

public:
	// kludge:
	MAPCLASSTYPE GetType() { return NULL; }
	BOOL IsMapClass(MAPCLASSTYPE Type) { return FALSE; }
};

class editCEditGameClass : public CObject, public CEditGameClass
{
	DECLARE_DYNAMIC(editCEditGameClass)
};

class CObjectProperties : public CPropertySheet
{
	DECLARE_DYNAMIC(CObjectProperties)

public:
	CObjectProperties(void);
	virtual ~CObjectProperties(void);

	CObjectProperties(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CObjectProperties(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

	inline void UpdateGrouplist(void);

	PVOID GetEditObject(CRuntimeClass *pType);
	PVOID GetEditObjectFromMapObject(CMapClass *pobj, CRuntimeClass *pType);
	void CopyDataToEditObjects(CMapClass *pobj);
	
	void SetPageToOutput(CEntityConnection *pConnection);
	void SetPageToInput(CEntityConnection *pConnection);

	BOOL SetupPages(void);
	void CreatePages(void);
	void SaveData(void);
		
	void LoadDataForPages(int iPage = -1);
#ifdef SLE
	void LoadData(CMapObjectList *pObjects, bool bSave); //// SLE NEW: separate pointer for directly loading object properties of an item.
	void ReloadData(); //// moved from protected to public
#endif
	void SetObjectList(const CMapObjectList *pObjectList);
	void MarkDataDirty();
	
	void SetOutputButtonState(int nState);
	void SetInputButtonState(int nState);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectProperties)
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

protected:
	void AddObjectExpandGroups(CMapClass *pObject);
#ifndef SLE
	void ReloadData();
#endif
	LayoutType_t GetLayout(void);
	void GetTabsForLayout(LayoutType_t eLayoutType, bool &bEntity, bool &bGroups, bool &bFlags, bool &bModel);

	void CreateButtons(void);
#ifdef SLE
public:
#endif
	void UpdateOutputButton(void);

protected:
	void UpdateInputButton(void);

	void SaveActivePage(void);
	void RestoreActivePage(void);
	
	void UpdateAnchors( CWnd *pPage );

	CAnchorMgr m_AnchorMgr;

	//
	// Pages.
	//
	COP_Entity *m_pEntity;
	COP_Groups *m_pGroups;
	COP_Flags *m_pFlags;
	COP_Output *m_pOutput;
	COP_Input *m_pInput;
	COP_Model *m_pModel;

	//
	// instance button
	//
	CButton *m_pInstanceButton;

	//
	// Input/output connection buttons
	//
	CButton *m_pInputButton;
	CButton *m_pOutputButton;

	HICON m_hIconOutputGood;
	HICON m_hIconOutputBad;
	HICON m_hIconInputGood;
	HICON m_hIconInputBad;

	CPropertyPage *m_pDummy;		// 
	bool m_bDummy;					// 

	CMapObjectList			m_DstObjects;	// list of objects we work on
	const CMapObjectList	*m_pOrgObjects;	// list of input object we will work on
	CObjectPage				**m_ppPages;		// Pointers to the pages that are currently in use
	int						m_nPages;					// Number of pages currently in use
	CObjectPage				*m_pLastActivePage;	// Used for restoring the last active page when changing selection.

	bool m_bDataDirty;	// true if selected object somehow changed etc
	bool m_bCanEdit;	// true if the object can be edited
	
	//{{AFX_MSG(CObjectProperties)
	afx_msg void OnOK(void);
	afx_msg void OnApply(void);
	afx_msg void OnCancel(void);
	afx_msg void OnInputs(void);
	afx_msg void OnOutputs(void);
	afx_msg void OnEditInstance(void);
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

#ifdef SLE_DARK_THEME
	//// SLE dark theme test
	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
	{
		HBRUSH hBrush = CPropertySheet::OnCtlColor(pDC, pWnd, nCtlColor);
		pDC->SetBkColor(SLE_DARK_THEME_CLR_BACK);
		switch (nCtlColor)
		{
		case CTLCOLOR_STATIC: // text of labels
			pDC->SetTextColor(SLE_DARK_THEME_CLR_TEXT);
			return hBrush;
		case CTLCOLOR_EDIT: // text of entered values
			pDC->SetTextColor(SLE_DARK_THEME_CLR_EDIT_TEXT);
			pDC->SetBkColor(SLE_DARK_THEME_CLR_EDIT_BACK);
			return hBrush;
		case CTLCOLOR_BTN: // text of buttons
			pDC->SetTextColor(SLE_DARK_THEME_CLR_TEXT);
			pDC->SetBkColor(SLE_DARK_THEME_CLR_BTN);
			return hBrush;
		case CTLCOLOR_LISTBOX: // text of buttons
			pDC->SetTextColor(SLE_DARK_THEME_CLR_TEXT);
			pDC->SetBkColor(SLE_DARK_THEME_CLR_EDIT_BACK);
			return hBrush;
		default:
			return hBrush;
		}
	}

	afx_msg BOOL OnEraseBkgnd(CDC* pDC)
	{
		CBrush backBrush(SLE_DARK_THEME_CLR_BACK); // This color blends with the splash image!
										   // Save old brush
		CBrush *pOldBrush = pDC->SelectObject(&backBrush);

		CRect rect;
		pDC->GetClipBox(&rect);     // Erase the area needed

		pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);

		pDC->SelectObject(pOldBrush);
		return TRUE;
	}
#endif
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

friend CObjectPage;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
inline void CObjectProperties::UpdateGrouplist(void)
{
	if (m_pGroups != NULL)
	{
		m_pGroups->UpdateGroupList();
	}
}

#endif // OBJECTPROPERTIES_H