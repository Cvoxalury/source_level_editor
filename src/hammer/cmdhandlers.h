//====== Copyright c 1996-2007, Valve Corporation, All rights reserved. =======//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef CMDHANDLERS_H
#define CMDHANDLERS_H
#ifdef _WIN32
#pragma once
#endif

class IToolHandlerInfo
{
public:
	virtual BOOL UpdateCmdUI( CCmdUI *pCmdUI ) = 0;
	virtual BOOL Execute( UINT uMsg ) = 0;
};

IToolHandlerInfo * _ToolToHandlerInfo(UINT uMsg);

#endif // #ifndef CMDHANDLERS_H
