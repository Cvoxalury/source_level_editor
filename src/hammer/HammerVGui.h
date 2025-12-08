//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements all the functions exported by the GameUI dll
//
// $NoKeywords: $
//=============================================================================//

#ifdef COMPILER_MSVC
#pragma once
#endif

#include "vgui/vgui.h"
#include "tier3/tier3.h"

namespace vgui
{
	class Panel;
}

class IMatSystemSurface;
class CVGuiWnd;

extern IMatSystemSurface *g_pMatSystemSurface;

class CHammerVGui
{
public:
	CHammerVGui(void);
	~CHammerVGui(void);

	bool Init( HWND hWindow );
	void Simulate();
	void Shutdown();
	bool HasFocus( CVGuiWnd *pWnd );
	void SetFocus( CVGuiWnd *pWnd );
	bool IsInitialized() { return m_hMainWindow != NULL; };
#ifdef SLE 
	vgui::HScheme GetHammerScheme() { return m_hHammerScheme; } //// SLE NEW: scheme is loaded and then requensted by particle browser (and possibly other things in the future)
#endif
	
protected:
	HWND			m_hMainWindow;
	CVGuiWnd		*m_pActiveWindow;	// the VGUI window that has the focus
#ifdef SLE 
	vgui::HScheme	m_hHammerScheme; //// SLE NEW: scheme is loaded and then requensted by particle browser (and possibly other things in the future)
#endif
};

CHammerVGui *HammerVGui();