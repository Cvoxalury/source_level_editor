//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ====
//
// Purpose: //// SLE NEW: Colors config tab (ported from sdk-2013-hammer)
//
//=============================================================================

#include "stdafx.h"
#include "hammer.h"
#include "gameconfig.h"
#include "optcolor.h"
#include "options.h"
#include "OP_Entity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

COPTColor::COPTColor()
	: CPropertyPage(COPTColor::IDD)
{
	//{{AFX_DATA_INIT(COPTColor)
	//}}AFX_DATA_INIT
}

void CColorButton::Setup(UINT nID, CWnd* parent, COLORREF* clr)
{
	SubclassDlgItem(nID, parent);
	m_color = clr;
	SetFaceColor(*clr);
}

void CColorButton::UpdateColor(COLORREF color)
{
//	*m_color = color;
//	SetFaceColor(color);
}

void CColorButton::UpdateButton(COLORREF color)
{
	//	*m_color = color;
	SetFaceColor(color);
}

BEGIN_MESSAGE_MAP(COPTColor, CPropertyPage)
	//{{AFX_MSG_MAP(COPTColor)
	ON_BN_CLICKED(IDC_COLOR_ENABLE, OnCustomClick)
	ON_COMMAND_EX(IDC_SCALE_AXIS, OnCheckBoxClicked)
	ON_COMMAND_EX(IDC_SCALE_GRID, OnCheckBoxClicked)
	ON_COMMAND_EX(IDC_SCALE_DOT_GRID, OnCheckBoxClicked)
	ON_COMMAND_EX(IDC_SCALE_10_GRID, OnCheckBoxClicked)
	ON_COMMAND_EX(IDC_SCALE_1024_GRID, OnCheckBoxClicked)
	ON_COMMAND(IDC_COLOR_AXIS, OnAxisColor)
	ON_COMMAND(IDC_COLOR_GRID, OnGridColor)
	ON_COMMAND(IDC_COLOR_DOT_GRID, OnDotGridColor)
	ON_COMMAND(IDC_COLOR_10_GRID, On10GridColor)
	ON_COMMAND(IDC_COLOR_1024_GRID, On1024GridColor)
	ON_COMMAND(IDC_COLOR_2DBACKGROUND, On2DBackgroundColor)
	ON_COMMAND(IDC_COLOR_BRUSH, OnBrushColor)
	ON_COMMAND(IDC_COLOR_ENTITY, OnEntityColor)
	ON_COMMAND(IDC_COLOR_2DSELECTION, On2DSelectionColor)
	ON_COMMAND(IDC_COLOR_VERTEX, OnVertexColor)
	ON_COMMAND(IDC_COLOR_TOOL_HANDLE, OnToolHandleColor)
	ON_COMMAND(IDC_COLOR_TOOL_BLOCK, OnToolBlockColor)
	ON_COMMAND(IDC_COLOR_TOOL_SELECTION, OnToolSelectionColor)
	ON_COMMAND(IDC_COLOR_TOOL_MORPH, OnToolMorhColor)
	ON_COMMAND(IDC_COLOR_TOOL_PATH, OnToolPathColor)
	ON_COMMAND(IDC_COLOR_TOOL_DRAG, OnToolDragColor)
	ON_COMMAND(IDC_COLOR_WIREFRAME, OnModelCollisionWireframeColor)
	ON_COMMAND(IDC_COLOR_WIREFRAME_NS, OnModelCollisionWireframeNSColor)
	ON_COMMAND(IDC_COLOR_3DBACKGROUND, On3DBackgroundColor)
	ON_COMMAND(IDC_COLOR_3DSELECTION, On3DSelectionColor)
	ON_COMMAND(IDC_COLOR_MAPBOUNDS, OnMapBoundsColor)
	ON_COMMAND(IDC_COLOR_MODELSELECTION, OnModelSelectionColor)
	ON_COMMAND(IDC_COLOR_3DEDGES, On3DEdgesColor)
	ON_COMMAND(IDC_COLOR_INSTANCES, OnInstancesColor)
	ON_COMMAND(IDC_COLOR_INSTANCES_SEL, OnInstancesSelColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void COPTColor::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COPTColor)
	DDX_Control(pDX, IDC_COLOR_ENABLE, m_Enable);
	DDX_Control(pDX, IDC_SCALE_AXIS, m_ScaleAxis);
	DDX_Control(pDX, IDC_SCALE_GRID, m_ScaleGrid);
	DDX_Control(pDX, IDC_SCALE_DOT_GRID, m_ScaleDotGrid);
	DDX_Control(pDX, IDC_SCALE_10_GRID, m_Scale10Grid);
	DDX_Control(pDX, IDC_SCALE_1024_GRID, m_Scale1024Grid);
	//}}AFX_DATA_MAP
	
	m_Colors[0].Setup( IDC_COLOR_AXIS, this, &Options.colors.clrAxis );
	m_Colors[1].Setup( IDC_COLOR_GRID, this, &Options.colors.clrGrid );
	m_Colors[2].Setup( IDC_COLOR_DOT_GRID, this, &Options.colors.clrGridDot );
	m_Colors[3].Setup( IDC_COLOR_10_GRID, this, &Options.colors.clrGrid10 );
	m_Colors[4].Setup( IDC_COLOR_1024_GRID, this, &Options.colors.clrGrid1024 );
	m_Colors[5].Setup( IDC_COLOR_2DBACKGROUND, this, &Options.colors.clr2DBackground );
	m_Colors[6].Setup( IDC_COLOR_BRUSH, this, &Options.colors.clrBrush );
	m_Colors[7].Setup( IDC_COLOR_ENTITY, this, &Options.colors.clrEntity );
	m_Colors[8].Setup( IDC_COLOR_2DSELECTION, this, &Options.colors.clr2DSelection );
	m_Colors[9].Setup( IDC_COLOR_VERTEX, this, &Options.colors.clrVertex );
	m_Colors[10].Setup( IDC_COLOR_TOOL_HANDLE, this, &Options.colors.clrToolHandle );
	m_Colors[11].Setup( IDC_COLOR_TOOL_BLOCK, this, &Options.colors.clrToolBlock );
	m_Colors[12].Setup( IDC_COLOR_TOOL_SELECTION, this, &Options.colors.clrToolSelection );
	m_Colors[13].Setup( IDC_COLOR_TOOL_MORPH, this, &Options.colors.clrToolMorph );
	m_Colors[14].Setup( IDC_COLOR_TOOL_PATH, this, &Options.colors.clrToolPath );
	m_Colors[15].Setup( IDC_COLOR_TOOL_DRAG, this, &Options.colors.clrToolDrag );
	m_Colors[16].Setup( IDC_COLOR_WIREFRAME, this, &Options.colors.clrModelCollisionWireframe );
	m_Colors[17].Setup( IDC_COLOR_WIREFRAME_NS, this, &Options.colors.clrModelCollisionWireframeDisabled );
	m_Colors[18].Setup( IDC_COLOR_3DBACKGROUND, this, &Options.colors.clr3DBackground );
	m_Colors[19].Setup( IDC_COLOR_3DSELECTION, this, &Options.colors.clr3DSelection );
	m_Colors[20].Setup( IDC_COLOR_MAPBOUNDS, this, &Options.colors.clrMapBounds);
	m_Colors[21].Setup( IDC_COLOR_MODELSELECTION, this, &Options.colors.clrModelSelection);
	m_Colors[22].Setup( IDC_COLOR_3DEDGES, this, &Options.colors.clr3DEdges);
	m_Colors[23].Setup( IDC_COLOR_INSTANCES, this, &Options.colors.clrInstances);
	m_Colors[24].Setup( IDC_COLOR_INSTANCES_SEL, this, &Options.colors.clrInstancesSel);

	m_Enable.SetCheck( Options.colors.bUseCustom ? BST_CHECKED : BST_UNCHECKED );
	m_ScaleAxis.SetCheck( Options.colors.bScaleAxisColor ? BST_CHECKED : BST_UNCHECKED );
	m_ScaleGrid.SetCheck( Options.colors.bScaleGridColor ? BST_CHECKED : BST_UNCHECKED );
	m_ScaleDotGrid.SetCheck( Options.colors.bScaleGridDotColor ? BST_CHECKED : BST_UNCHECKED );
	m_Scale10Grid.SetCheck( Options.colors.bScaleGrid10Color ? BST_CHECKED : BST_UNCHECKED );
	m_Scale1024Grid.SetCheck( Options.colors.bScaleGrid1024Color ? BST_CHECKED : BST_UNCHECKED );

//	for ( auto& c : m_Colors )
//		c.EnableWindow( Options.colors.bUseCustom );
	m_ScaleAxis.EnableWindow( Options.colors.bUseCustom );
	m_ScaleGrid.EnableWindow( Options.colors.bUseCustom );
	m_ScaleDotGrid.EnableWindow( Options.colors.bUseCustom );
	m_Scale10Grid.EnableWindow( Options.colors.bUseCustom );
	m_Scale1024Grid.EnableWindow( Options.colors.bUseCustom );
}

BOOL COPTColor::OnInitDialog()
{
	CPropertyPage::OnInitDialog(); 

	SetModified(TRUE);

	return TRUE;
}

BOOL COPTColor::OnApply()
{
	Options.PerformChanges(COptions::secColors);
	return CPropertyPage::OnApply();
}

void COPTColor::OnCustomClick()
{
	Options.colors.bUseCustom = m_Enable.GetCheck() == BST_CHECKED;
	for ( auto& c : m_Colors )
		c.EnableWindow( Options.colors.bUseCustom );
	m_ScaleAxis.EnableWindow( Options.colors.bUseCustom );
	m_ScaleGrid.EnableWindow( Options.colors.bUseCustom );
	m_ScaleDotGrid.EnableWindow( Options.colors.bUseCustom );
	m_Scale10Grid.EnableWindow( Options.colors.bUseCustom );
	m_Scale1024Grid.EnableWindow( Options.colors.bUseCustom );

	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

BOOL COPTColor::OnCheckBoxClicked( UINT nID )
{
	switch ( nID )
	{
	case IDC_SCALE_AXIS:
		Options.colors.bScaleAxisColor = m_ScaleAxis.GetCheck() == BST_CHECKED;
		break;
	case IDC_SCALE_GRID:
		Options.colors.bScaleGridColor = m_ScaleGrid.GetCheck() == BST_CHECKED;
		break;
	case IDC_SCALE_DOT_GRID:
		Options.colors.bScaleGridDotColor = m_ScaleDotGrid.GetCheck() == BST_CHECKED;
		break;
	case IDC_SCALE_10_GRID:
		Options.colors.bScaleGrid10Color = m_Scale10Grid.GetCheck() == BST_CHECKED;
		break;
	case IDC_SCALE_1024_GRID:
		Options.colors.bScaleGrid1024Color = m_Scale1024Grid.GetCheck() == BST_CHECKED;
		break;
	NO_DEFAULT
	}

	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);

	return TRUE;
}

COLORREF COPTColor::SetupColorPickerButton(COLORREF clrIn)
{
	BYTE r = 255, g = 255, b = 255;
	DWORD brightness = 0xffffffff;
	
	CColorDialog dlg(clrIn, CC_FULLOPEN);
	if (dlg.DoModal() != IDOK)
		return clrIn;

	r = GetRValue(dlg.m_cc.rgbResult);
	g = GetGValue(dlg.m_cc.rgbResult);
	b = GetBValue(dlg.m_cc.rgbResult);

	return RGB(r, g, b);
}

void COPTColor::OnAxisColor(void)
{
	Options.colors.clrAxis = SetupColorPickerButton(Options.colors.clrAxis);
	m_Colors[0].UpdateButton(Options.colors.clrAxis);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnGridColor(void)
{
	Options.colors.clrGrid = SetupColorPickerButton(Options.colors.clrGrid);
	m_Colors[1].UpdateButton(Options.colors.clrGrid);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnDotGridColor(void)
{
	Options.colors.clrGridDot = SetupColorPickerButton(Options.colors.clrGridDot);
	m_Colors[2].UpdateButton(Options.colors.clrGridDot);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::On10GridColor(void)
{
	Options.colors.clrGrid10 = SetupColorPickerButton(Options.colors.clrGrid10);
	m_Colors[3].UpdateButton(Options.colors.clrGrid10);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::On1024GridColor(void)
{
	Options.colors.clrGrid1024 = SetupColorPickerButton(Options.colors.clrGrid1024);
	m_Colors[4].UpdateButton(Options.colors.clrGrid1024);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::On2DBackgroundColor(void)
{
	Options.colors.clr2DBackground = SetupColorPickerButton(Options.colors.clr2DBackground);
	m_Colors[5].UpdateButton(Options.colors.clr2DBackground);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnBrushColor()
{
	Options.colors.clrBrush = SetupColorPickerButton(Options.colors.clrBrush);
	m_Colors[6].UpdateButton(Options.colors.clrBrush);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnEntityColor()
{
	Options.colors.clrEntity = SetupColorPickerButton(Options.colors.clrEntity);
	m_Colors[7].UpdateButton(Options.colors.clrEntity);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::On2DSelectionColor()
{
	Options.colors.clr2DSelection = SetupColorPickerButton(Options.colors.clr2DSelection);
	m_Colors[8].UpdateButton(Options.colors.clr2DSelection);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnVertexColor()
{
	Options.colors.clrVertex = SetupColorPickerButton(Options.colors.clrVertex);
	m_Colors[9].UpdateButton(Options.colors.clrVertex);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnToolHandleColor()
{
	Options.colors.clrToolHandle = SetupColorPickerButton(Options.colors.clrToolHandle);
	m_Colors[10].UpdateButton(Options.colors.clrToolHandle);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnToolBlockColor()
{
	Options.colors.clrToolBlock = SetupColorPickerButton(Options.colors.clrToolBlock);
	m_Colors[11].UpdateButton(Options.colors.clrToolBlock);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnToolSelectionColor()
{
	Options.colors.clrToolSelection = SetupColorPickerButton(Options.colors.clrToolSelection);
	m_Colors[12].UpdateButton(Options.colors.clrToolSelection);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnToolMorhColor()
{
	Options.colors.clrToolMorph = SetupColorPickerButton(Options.colors.clrToolMorph);
	m_Colors[13].UpdateButton(Options.colors.clrToolMorph);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnToolPathColor()
{
	Options.colors.clrToolPath = SetupColorPickerButton(Options.colors.clrToolPath);
	m_Colors[14].UpdateButton(Options.colors.clrToolPath);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnToolDragColor()
{
	Options.colors.clrToolDrag = SetupColorPickerButton(Options.colors.clrToolDrag);
	m_Colors[15].UpdateButton(Options.colors.clrToolDrag);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnModelCollisionWireframeColor()
{
	Options.colors.clrModelCollisionWireframe = SetupColorPickerButton(Options.colors.clrModelCollisionWireframe);
	m_Colors[16].UpdateButton(Options.colors.clrModelCollisionWireframe);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnModelCollisionWireframeNSColor()
{
	Options.colors.clrModelCollisionWireframeDisabled = SetupColorPickerButton(Options.colors.clrModelCollisionWireframeDisabled);
	m_Colors[17].UpdateButton(Options.colors.clrModelCollisionWireframeDisabled);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::On3DBackgroundColor()
{
	Options.colors.clr3DBackground = SetupColorPickerButton(Options.colors.clr3DBackground);
	m_Colors[18].UpdateButton(Options.colors.clr3DBackground);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::On3DSelectionColor()
{
	Options.colors.clr3DSelection = SetupColorPickerButton(Options.colors.clr3DSelection);
	m_Colors[19].UpdateButton(Options.colors.clr3DSelection);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}
void COPTColor::OnMapBoundsColor()
{
	Options.colors.clrMapBounds = SetupColorPickerButton(Options.colors.clrMapBounds);
	m_Colors[20].UpdateButton(Options.colors.clrMapBounds);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnModelSelectionColor()
{
	Options.colors.clrModelSelection = SetupColorPickerButton(Options.colors.clrModelSelection);
	m_Colors[21].UpdateButton(Options.colors.clrModelSelection);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::On3DEdgesColor()
{
	Options.colors.clr3DEdges = SetupColorPickerButton(Options.colors.clr3DEdges);
	m_Colors[22].UpdateButton(Options.colors.clr3DEdges);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnInstancesColor()
{
	Options.colors.clrInstances = SetupColorPickerButton(Options.colors.clrInstances);
	m_Colors[23].UpdateButton(Options.colors.clrInstances);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}

void COPTColor::OnInstancesSelColor()
{
	Options.colors.clrInstancesSel = SetupColorPickerButton(Options.colors.clrInstancesSel);
	m_Colors[24].UpdateButton(Options.colors.clrInstancesSel);
	Options.PerformChanges(COptions::secColors);
	SetModified(TRUE);
}
