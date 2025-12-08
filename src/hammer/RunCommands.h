//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef _RUNCOMMANDS_H
#define _RUNCOMMANDS_H

#include <afxtempl.h>
//#define SLE_NEW_SEPARATE_COMPILER_WINDOW
//
// RunCommands functions
//

enum
{
	CCChangeDir = 0x100,
	CCCopyFile,
	CCDelFile,
	CCRenameFile
};

// command:
typedef struct
{
	BOOL bEnable;				// Run this command?

	int iSpecialCmd;			// Nonzero if special command exists
	char szRun[MAX_PATH];
	char szParms[MAX_PATH];
	BOOL bLongFilenames;		// Obsolete, but kept here for file backwards compatibility
	BOOL bEnsureCheck;
	char szEnsureFn[MAX_PATH];
	BOOL bUseProcessWnd;
	BOOL bNoWait;

#ifdef SLE_NEW_SEPARATE_COMPILER_WINDOW
	void Save(KeyValues *pKv) const;
	void Load(KeyValues *pKv);
#endif
} CCOMMAND, *PCCOMMAND;

// list of commands:
typedef CArray<CCOMMAND, CCOMMAND&> CCommandArray;

// run a list of commands:
bool RunCommands(CCommandArray& Commands, LPCTSTR pszDocName);
void FixGameVars(char *pszSrc, char *pszDst, BOOL bUseQuotes = TRUE);
bool IsRunningCommands();

#endif // _RUNCOMMANDS_H
