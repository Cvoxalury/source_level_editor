//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef TEXTUREBAR_H
#define TEXTUREBAR_H
#ifdef _WIN32
#pragma once
#endif

#include "TextureBox.h"
#include "IEditorTexture.h"
#include "wndTex.h"
#include "ControlBarIDs.h"
#include "HammerBar.h"

class IEditorTexture;

class CTextureBar : public CHammerBar
{
	public:
		CTextureBar() : CHammerBar() {}
		BOOL Create(CWnd *pParentWnd, int IDD = IDD_TEXTUREBAR, int iBarID = IDCB_TEXTUREBAR);

		void NotifyGraphicsChanged(void);
		void NotifyNewMaterial( IEditorTexture *pTexture );
		void SelectTexture(LPCSTR pszTextureName);

	protected:
		void UpdateTexture(void);

		IEditorTexture *m_pCurTex;
		CTextureBox m_TextureList;
		CComboBox m_TextureGroupList;
		wndTex m_TexturePic;
		
		afx_msg void UpdateControl(CCmdUI *);
		afx_msg void OnBrowse(void);
		afx_msg void OnChangeTextureGroup(void);
		afx_msg void OnReplace(void);
#ifdef SLE //// SLE NEW - mark button on texture bar
		afx_msg void OnMark(void);
#endif
		afx_msg void OnUpdateTexname(void);
		afx_msg void OnWindowPosChanged(WINDOWPOS *pPos);
		virtual afx_msg void OnSelChangeTexture(void);

#ifdef SLE_DARK_THEME
		//// SLE dark theme test
		afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
		{
			HBRUSH hBrush = CDialogBar::OnCtlColor(pDC, pWnd, nCtlColor);
			static CBrush statBrush(SLE_DARK_THEME_CLR_BACK); // this is the brush that will be returned to avoid static text's unfilled portion's color "leaking".
			static CBrush editBrush(SLE_DARK_THEME_CLR_EDIT_BACK); // this is the brush that will be returned to avoid edit text's unfilled portion's color "leaking".
			pDC->SetBkColor(SLE_DARK_THEME_CLR_BACK);
			switch (nCtlColor)
			{
			case CTLCOLOR_STATIC: // text of labels
				pDC->SetTextColor(SLE_DARK_THEME_CLR_TEXT);
				hBrush = statBrush;
				return hBrush;
			case CTLCOLOR_EDIT: // text of entered values
				pDC->SetTextColor(SLE_DARK_THEME_CLR_EDIT_TEXT);
				pDC->SetBkColor(SLE_DARK_THEME_CLR_EDIT_BACK);
				hBrush = editBrush;
				return hBrush;
			case CTLCOLOR_BTN: // text of buttons
				pDC->SetTextColor(SLE_DARK_THEME_CLR_TEXT);
				pDC->SetBkColor(SLE_DARK_THEME_CLR_BTN);
				return hBrush;
			case CTLCOLOR_LISTBOX: // text of listboxes
				pDC->SetTextColor(SLE_DARK_THEME_CLR_TEXT);
				pDC->SetBkColor(SLE_DARK_THEME_CLR_EDIT_BACK);
				hBrush = editBrush;
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
		DECLARE_MESSAGE_MAP()
};

#endif // TEXTUREBAR_H
