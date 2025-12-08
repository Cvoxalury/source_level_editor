//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FILTERCONTROL_H
#define FILTERCONTROL_H
#pragma once


#include "resource.h"
#include "GroupList.h"
#include "HammerBar.h"
#include "IHammer.h"
#ifdef HAMMER2013_PORT_CORDONS
#include "cordonlist.h"
#endif
#ifdef HAMMER2013_PORT_CORDONS
enum FilterDialogMode_t
{
	FILTER_DIALOG_NONE = -1,
	FILTER_DIALOG_USER_VISGROUPS,
	FILTER_DIALOG_AUTO_VISGROUPS,
	FILTER_DIALOG_CORDONS,
};
#endif
class CFilterControl : public CHammerBar
{
public:
#ifdef HAMMER2013_PORT_CORDONS
	CFilterControl() : CHammerBar() { m_bInitialized = FALSE; }
#else
	CFilterControl() : CHammerBar() { bInitialized = FALSE; }
#endif
	BOOL Create(CWnd *pParentWnd);
#ifdef HAMMER2013_PORT_CORDONS
	void UpdateList();
	void UpdateCordonList(Cordon_t *pSelectCordon = NULL, BoundBox *pSelectBox = NULL);
	void SelectCordon(Cordon_t *pSelectCordon, BoundBox *pSelectBox);
	void UpdateGroupList();
	void UpdateGroupListChecks();
	void UpdateCordonListChecks();
#else
	void UpdateGroupList(void);
	void UpdateGroupListChecks(void);
#endif
#ifdef SLE //// SLE NEW - used for 'add to selected visgroup' feature
	CVisGroup *GetCurrentlySelectedVisgroup(void) { return (m_cGroupBox && m_cGroupBox.GetCount() > 0) ?  m_cGroupBox.GetSelectedVisGroup() : NULL; }
#endif
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual CSize CalcDynamicLayout(int nLength, DWORD dwMode);

private:
	//{{AFX_DATA(CFilterControl)
	enum { IDD = IDD_MAPVIEWBAR };
	//}}AFX_DATA

	CBitmapButton m_cMoveUpButton;
	CBitmapButton m_cMoveDownButton;
	CGroupList m_cGroupBox;
#ifdef HAMMER2013_PORT_CORDONS
	CCordonList m_cCordonBox;
#endif
	CTabCtrl m_cTabControl;
#ifdef HAMMER2013_PORT_CORDONS
	bool m_bInitialized;
#else
	BOOL bInitialized;
	BOOL m_bShowingAuto;
#endif
protected:
#ifndef HAMMER2013_PORT_CORDONS
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);
#endif
	virtual BOOL OnInitDialog(void);
	void OnSelChangeTab(NMHDR *header, LRESULT *result); 
#ifdef HAMMER2013_PORT_CORDONS
	void ChangeMode(FilterDialogMode_t oldMode, FilterDialogMode_t newMode);

	void OnCordonListDragDrop(CordonListItem_t *drag, CordonListItem_t *drop);
	void OnVisGroupListDragDrop(CVisGroup *pDragGroup, CVisGroup *pDropGroup);

	void DeleteCordonListItem(CordonListItem_t *pDelete, bool bConfirm);
#endif
	//{{AFX_MSG(CFilterControl)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEditGroups();
#ifdef HAMMER2013_PORT_CORDONS
	afx_msg void OnNew();
	afx_msg void OnDelete();
#endif
	afx_msg void OnMarkMembers();
	afx_msg BOOL OnMoveUpDown(UINT uCmd);
	afx_msg void UpdateControl(CCmdUI *);
	afx_msg void UpdateControlGroups(CCmdUI *pCmdUI);
	afx_msg void OnActivate(UINT nState, CWnd*, BOOL);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnWindowPosChanged(WINDOWPOS *pPos);
	afx_msg void OnEndlabeleditGrouplist(NMHDR*, LRESULT*);
	afx_msg void OnBegindragGrouplist(NMHDR*, LRESULT*);
	afx_msg void OnMousemoveGrouplist(NMHDR*, LRESULT*);
	afx_msg void OnEnddragGrouplist(NMHDR*, LRESULT*);
	afx_msg LRESULT OnListToggleState(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnListLeftDragDrop(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnListRightDragDrop(WPARAM wParam, LPARAM lParam); 
#ifdef HAMMER2013_PORT_CORDONS
	afx_msg LRESULT OnListSelChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnListKeyDown(WPARAM wParam, LPARAM lParam);
#ifdef SLE //// SLE NEW - double click to mark the group
	afx_msg LRESULT OnListDblLClick(WPARAM wParam, LPARAM lParam);
#endif
#endif
	afx_msg void OnShowAllGroups(void);
	//}}AFX_MSG

	CImageList *m_pDragImageList;
#ifdef HAMMER2013_PORT_CORDONS
	FilterDialogMode_t m_mode;		// Whether we're showing user visgroups, auto visgroups, or cordons
#endif
	DECLARE_MESSAGE_MAP()
};


#endif // FILTERCONTROL_H
