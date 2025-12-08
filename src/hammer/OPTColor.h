//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#ifndef OPTCOLOR_H
#define OPTCOLOR_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxcolorbutton.h"

class CColorButton : public CMFCButton
{
public:
	using CMFCButton::CMFCButton;

	void Setup(UINT nID, CWnd* parent, COLORREF* clr);
	void UpdateColor(COLORREF color);
	void UpdateButton(COLORREF color);

private:
	COLORREF* m_color = NULL;
};

// COPTColor dialog

class COPTColor : public CPropertyPage
{
// Construction
public:
	COPTColor();   // standard constructor

// Dialog Data
	//{{AFX_DATA(COPTColor)
	enum { IDD = IDD_OPTIONS_COLOR };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COPTColor)
public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COPTColor)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomClick();
	afx_msg BOOL OnCheckBoxClicked( UINT nID );
	afx_msg void OnAxisColor();
	afx_msg void OnGridColor();
	afx_msg void OnDotGridColor();
	afx_msg void On10GridColor();
	afx_msg void On1024GridColor();
	afx_msg void On2DBackgroundColor();
	afx_msg void OnBrushColor();
	afx_msg void OnEntityColor();
	afx_msg void On2DSelectionColor();
	afx_msg void OnVertexColor();
	afx_msg void OnToolHandleColor();
	afx_msg void OnToolBlockColor();
	afx_msg void OnToolSelectionColor();
	afx_msg void OnToolMorhColor();
	afx_msg void OnToolPathColor();
	afx_msg void OnToolDragColor();
	afx_msg void OnModelCollisionWireframeColor();
	afx_msg void OnModelCollisionWireframeNSColor();
	afx_msg void On3DBackgroundColor();
	afx_msg void On3DSelectionColor();
	afx_msg void OnMapBoundsColor();
	afx_msg void OnModelSelectionColor();
	afx_msg void On3DEdgesColor();
	afx_msg void OnInstancesColor();
	afx_msg void OnInstancesSelColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CButton m_Enable;
	CButton m_ScaleAxis;
	CButton m_ScaleGrid;
	CButton m_ScaleDotGrid;
	CButton m_Scale10Grid;
	CButton m_Scale1024Grid;
	CColorButton m_Colors[25];

private:
	COLORREF SetupColorPickerButton(COLORREF clrIn);
};
#endif // OPTCOLOR_H
