//// SLE NEW: Ported from Hammer-2013.

#include "stdafx.h"
#include "KeyBinds.h"
#include "resource.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "fmtstr.h"
#include "MapView3D.h"
#include "hammer.h"

#define KEYBINDS_CONFIG_FILE "level_editor\\cfg\\level_editor_keybinds.cfg"
#define KEYBINDS_CONFIG_DEFAULT_FILE "level_editor\\cfg\\level_editor_keybinds_default.cfg"

static KeyBinds s_KeyBinds;
KeyBinds *g_pKeyBinds = &s_KeyBinds;

struct KeyNames_t
{
	WORD		code;
	char const	*string;
	char const	*displaystring;
};

static KeyNames_t g_KeyNames[] =
{
	{ '0', "0", "0" },
	{ '1', "1", "1" },
	{ '2', "2", "2" },
	{ '3', "3", "3" },
	{ '4', "4", "4" },
	{ '5', "5", "5" },
	{ '6', "6", "6" },
	{ '7', "7", "7" },
	{ '8', "8", "8" },
	{ '9', "9", "9" },
	{ 'A', "A", "A" },
	{ 'B', "B", "B" },
	{ 'C', "C", "C" },
	{ 'D', "D", "D" },
	{ 'E', "E", "E" },
	{ 'F', "F", "F" },
	{ 'G', "G", "G" },
	{ 'H', "H", "H" },
	{ 'I', "I", "I" },
	{ 'J', "J", "J" },
	{ 'K', "K", "K" },
	{ 'L', "L", "L" },
	{ 'M', "M", "M" },
	{ 'N', "N", "N" },
	{ 'O', "O", "O" },
	{ 'P', "P", "P" },
	{ 'Q', "Q", "Q" },
	{ 'R', "R", "R" },
	{ 'S', "S", "S" },
	{ 'T', "T", "T" },
	{ 'U', "U", "U" },
	{ 'V', "V", "V" },
	{ 'W', "W", "W" },
	{ 'X', "X", "X" },
	{ 'Y', "Y", "Y" },
	{ 'Z', "Z", "Z" },
	{ VK_NUMPAD0, "VK_NUMPAD0", "Key Pad 0" },
	{ VK_NUMPAD1, "VK_NUMPAD1", "Key Pad 1" },
	{ VK_NUMPAD2, "VK_NUMPAD2", "Key Pad 2" },
	{ VK_NUMPAD3, "VK_NUMPAD3", "Key Pad 3" },
	{ VK_NUMPAD4, "VK_NUMPAD4", "Key Pad 4" },
	{ VK_NUMPAD5, "VK_NUMPAD5", "Key Pad 5" },
	{ VK_NUMPAD6, "VK_NUMPAD6", "Key Pad 6" },
	{ VK_NUMPAD7, "VK_NUMPAD7", "Key Pad 7" },
	{ VK_NUMPAD8, "VK_NUMPAD8", "Key Pad 8" },
	{ VK_NUMPAD9, "VK_NUMPAD9", "Key Pad 9" },
	{ VK_DIVIDE, "VK_DIVIDE", "Key Pad /" },
	{ VK_MULTIPLY, "VK_MULTIPLY", "Key Pad *" },
	{ VK_OEM_MINUS, "VK_OEM_MINUS", "Key Pad -" },
	{ VK_OEM_PLUS, "VK_OEM_PLUS", "Key Pad +" },
	{ VK_RETURN, "VK_RETURN", "Key Pad Enter" },
	{ VK_DECIMAL, "VK_DECIMAL", "Key Pad ." },
	{ VK_OEM_6, "VK_OEM_6", "[" },
	{ VK_OEM_4, "VK_OEM_4", "]" },
	{ VK_OEM_1, "VK_OEM_1", ";" },
	{ VK_OEM_7, "VK_OEM_7", "'" },
	{ VK_OEM_3, "VK_OEM_3", "`" },
	{ VK_OEM_COMMA, "VK_OEM_COMMA", "," },
	{ VK_DECIMAL, "VK_DECIMAL", "." },
	{ VK_OEM_2, "VK_OEM_2", "/" },
	{ VK_OEM_5, "VK_OEM_5", "\\" },
	{ VK_OEM_MINUS, "VK_OEM_MINUS", "-" },
	{ VK_OEM_PLUS, "VK_OEM_PLUS", "=" },
	{ VK_RETURN, "VK_RETURN", "Enter" },
	{ VK_SPACE, "VK_SPACE", "Space" },
	{ VK_BACK, "VK_BACK", "Backspace" },
	{ VK_TAB, "VK_TAB", "Tab" },
	{ VK_CAPITAL, "VK_CAPITAL", "Caps Lock" },
	{ VK_NUMLOCK, "VK_NUMLOCK", "Num Lock" },
	{ VK_ESCAPE, "VK_ESCAPE", "Escape" },
	{ VK_SCROLL, "VK_SCROLL", "Scroll Lock" },
	{ VK_INSERT, "VK_INSERT", "Ins" },
	{ VK_DELETE, "VK_DELETE", "Del" },
	{ VK_HOME, "VK_HOME", "Home" },
	{ VK_END, "VK_END", "End" },
	{ VK_PRIOR, "VK_PRIOR", "PgUp" },
	{ VK_NEXT, "VK_NEXT", "PgDn" },
	{ VK_PAUSE, "VK_PAUSE", "Break" },
	{ VK_LSHIFT, "VK_LSHIFT", "Shift" },
	{ VK_RSHIFT, "VK_RSHIFT", "Shift" },
	{ VK_MENU, "VK_MENU", "Alt" },
	{ VK_LCONTROL, "VK_LCONTROL", "Ctrl" },
	{ VK_RCONTROL, "VK_RCONTROL", "Ctrl" },
	{ VK_LWIN, "VK_LWIN", "Windows" },
	{ VK_RWIN, "VK_RWIN", "Windows" },
	{ VK_APPS, "VK_APPS", "App" },
	{ VK_UP, "VK_UP", "Up" },
	{ VK_LEFT, "VK_LEFT", "Left" },
	{ VK_DOWN, "VK_DOWN", "Down" },
	{ VK_RIGHT, "VK_RIGHT", "Right" },
	{ VK_F1, "VK_F1", "F1" },
	{ VK_F2, "VK_F2", "F2" },
	{ VK_F3, "VK_F3", "F3" },
	{ VK_F4, "VK_F4", "F4" },
	{ VK_F5, "VK_F5", "F5" },
	{ VK_F6, "VK_F6", "F6" },
	{ VK_F7, "VK_F7", "F7" },
	{ VK_F8, "VK_F8", "F8" },
	{ VK_F9, "VK_F9", "F9" },
	{ VK_F10, "VK_F10", "F10" },
	{ VK_F11, "VK_F11", "F11" },
	{ VK_F12, "VK_F12", "F12" }
};

WORD GetKeyForStr(const char *pStr)
{
	for (KeyNames_t t : g_KeyNames)
	{
		if (!Q_strcmp(pStr, t.displaystring) || !Q_strcmp(pStr, t.string))
			return t.code;
	}

	return 0;
}

const char *GetStrForKey(WORD key)
{
	for (KeyNames_t t : g_KeyNames)
	{
		if (t.code == key)
			return t.string;
	}

	return nullptr;
}

typedef struct
{
	const char *pKeyName;
	int iCommand;
} CommandNames_t;

#define COM(x) {#x, x}

static CommandNames_t s_CommandNames[] =
{
	// IDR_MAINFRAME
	COM(ID_CONTEXT_HELP),
	COM(ID_EDIT_CUT),
	COM(ID_EDIT_PASTE),
	COM(ID_EDIT_UNDO),
	COM(ID_FILE_NEW),
	COM(ID_FILE_OPEN),
	COM(ID_FILE_CLOSE),
//	COM(ID_FILE_PRINT),
	COM(ID_FILE_SAVE),
	COM(ID_HELP_FINDER),
	COM(ID_NEXT_PANE),
	COM(ID_PREV_PANE),
	COM(ID_VIEW_2DXY),
	COM(ID_VIEW_2DXZ),
	COM(ID_VIEW_2DYZ),
	COM(ID_SWITCH_3DVIEW_TO_WIREFRAME),
	COM(ID_SWITCH_3DVIEW_TO_FLATPOLYS),
	COM(ID_SWITCH_3DVIEW_TO_TEXTURED),
	COM(ID_SWITCH_3DVIEW_TO_SHADED),
	COM(ID_SWITCH_3DVIEW_TO_LIGHTMAPS),
	COM(ID_VIEW_3DTEXTURED),
	COM(ID_VIEW_3DTEXTURED_SHADED),

	// IDR_MAPDOC 
	COM(ID_CREATEOBJECT),
	COM(ID_EDIT_APPLYTEXTURE),
	COM(ID_EDIT_CLEARSELECTION),
	COM(ID_EDIT_COPYWC),
	COM(ID_EDIT_CUTWC),
	COM(ID_EDIT_FINDENTITIES),
	COM(ID_EDIT_PASTESPECIAL),
	COM(ID_EDIT_PASTEWC),
	COM(ID_EDIT_PROPERTIES),
	COM(ID_EDIT_REDO),
	COM(ID_EDIT_REPLACE),
	COM(ID_EDIT_SELECTALL),
	COM(ID_EDIT_TOENTITY),
	COM(ID_EDIT_TOWORLD),
	COM(ID_FILE_RUNMAP),
	COM(ID_FLIP_HORIZONTAL),
	COM(ID_FLIP_VERTICAL),
	COM(ID_GOTO_BRUSH),
	COM(ID_HELP_TOPICS),
	COM(ID_INSERTPREFAB_ORIGINAL),
	COM(ID_MAP_CHECK),
	COM(ID_MAP_GRIDHIGHER),
	COM(ID_MAP_GRIDLOWER),
	COM(ID_MAP_SNAPTOGRID),
	COM(ID_MODE_APPLICATOR),
	COM(ID_SELECTION_ALIGNBOTTOM),
	COM(ID_SELECTION_ALIGNLEFT),
	COM(ID_SELECTION_ALIGNRIGHT),
	COM(ID_SELECTION_ALIGNTOP),
	COM(ID_TEST),
	COM(ID_TOGGLE_GROUPIGNORE),
	COM(ID_TOOLS_APPLYDECALS),
	COM(ID_TOOLS_BLOCK),
	COM(ID_TOOLS_CAMERA),
	COM(ID_TOOLS_CLIPPER),
	COM(ID_TOOLS_CREATEPREFAB),
//	COM(ID_TOOLS_DISPLACE),
	COM(ID_TOOLS_ENTITY),
	COM(ID_TOOLS_GROUP),
	COM(ID_TOOLS_HIDE_ENTITY_NAMES),
	COM(ID_TOOLS_MAGNIFY),
	COM(ID_TOOLS_MORPH),
	COM(ID_TOOLS_OVERLAY),
//	COM(ID_TOOLS_PATH),
	COM(ID_TOOLS_POINTER),
	COM(ID_VIEW_QUICKHIDE),
	COM(ID_VIEW_QUICKHIDEUNSELECTED),
	COM(ID_VIEW_QUICKUNHIDE),
	COM(ID_TOOLS_SNAP_SELECTED_TO_GRID_INDIVIDUALLY),
	COM(ID_TOOLS_SNAPSELECTEDTOGRID),
	COM(ID_TOOLS_SOUND_BROWSER),
	COM(ID_TOOLS_SPLITFACE),
	COM(ID_TOOLS_SUBTRACTSELECTION),
	COM(ID_TOOLS_TOGGLETEXLOCK),
	COM(ID_TOOLS_TRANSFORM),
	COM(ID_TOOLS_UNGROUP),
//	COM(ID_VIEW3D_BRIGHTER),
//	COM(ID_VIEW3D_DARKER),
	COM(ID_VIEW_AUTOSIZE4),
	COM(ID_VIEW_CENTER3DVIEWSONSELECTION),
	COM(ID_VIEW_CENTERONSELECTION),
	COM(ID_VIEW_GRID),
	COM(ID_VIEW_MAXIMIZERESTOREACTIVEVIEW),
	COM(ID_VIEW_SHOWMODELSIN2D),
	COM(ID_VIEW_TEXTUREBROWSER),
	COM(ID_VSCALE_TOGGLE),
	//// NEW SLE COMMANDS
	COM(ID_SEND_SELECTION_TO_SELECTED_VISGROUP),
	COM(ID_FLIP_BACKFRONT),
	COM(ID_SHOW_NODRAW_BRUSHES),
	COM(ID_MAP_ENTITYREPORT),
	COM(ID_MAP_LOADPOINTFILE),
	COM(ID_SHOW_SOLID_EDGES_NOZ),
	COM(ID_MAP_ROTATIONPIVOT),
	COM(ID_FILE_SAVE_AS),
	COM(ID_TOOLS_SPRINKLE),
	COM(ID_SHOW_TOOLBRUSH_FACES),
	COM(ID_EDIT_PASTE_ALIGN),
	COM(ID_CYCLE_GROUPOBJECTSSOLIDS),
	COM(ID_SHOW_TOOLSSKY_FACES),
	COM(ID_SHOW_EDITOR_OBJECTS),
	COM(ID_VIEW_LOD_LOWER),
	COM(ID_VIEW_LOD_HIGHER),
	COM(ID_VIEW_HOME),
	COM(ID_VIEW_PLAYERSTART),
	COM(ID_VIEW_SKYCAMERA),
	COM(ID_VIEW_MAXIMISERESTORE3DVIEW),
	COM(ID_COLLISION_WIREFRAME)
};

int GetIDForCommandStr(const char *pName)
{
	for (CommandNames_t t : s_CommandNames)
	{
		if (!Q_strcmp(pName, t.pKeyName))
			return t.iCommand;
	}

	return 0;
}

const char *GetStrForCommandID(int ID)
{
	for (CommandNames_t t : s_CommandNames)
	{
		if (t.iCommand == ID)
			return t.pKeyName;
	}

	return nullptr;
}

KeyBinds::KeyBinds() : m_pKvKeybinds(nullptr)
{
}

KeyBinds::~KeyBinds()
{
	if (m_pKvKeybinds)
	{
		m_pKvKeybinds->deleteThis();
		m_pKvKeybinds = nullptr;
	}
}

bool KeyBinds::Init()
{
	m_pKvKeybinds = new KeyValues("KeyBindings");
	char szRootDir[MAX_PATH];
	APP()->GetDirectory(DIR_PROGRAM, szRootDir);
	if (!m_pKvKeybinds->LoadFromFile(g_pFullFileSystem, KEYBINDS_CONFIG_FILE, "EXECUTABLE_PATH"))
	{
		if (!m_pKvKeybinds->LoadFromFile(g_pFullFileSystem, KEYBINDS_CONFIG_DEFAULT_FILE, "EXECUTABLE_PATH"))
		{
			// If we weren't even able to load the default, we're screwed
			Error("Could not load keybinds from %s%s", szRootDir, KEYBINDS_CONFIG_DEFAULT_FILE);
			m_pKvKeybinds->deleteThis();
			return false;
		}
		else
		{
			// Save them out for the first time
			Shutdown();
		}
	}

	return true;
}

void KeyBinds::Shutdown()
{
	// Write out to file if we're valid
	if (m_pKvKeybinds)
	{
		char szRootDir[MAX_PATH];
		APP()->GetDirectory(DIR_PROGRAM, szRootDir);
		m_pKvKeybinds->SaveToFile(g_pFullFileSystem, KEYBINDS_CONFIG_FILE, szRootDir);
	}
}

bool KeyBinds::LoadKeybinds(const char* pMapping, CUtlStringMap<KeyMap_t> &map)
{
	if (pMapping)
	{
		KeyValues *pKvKeybinds = m_pKvKeybinds->FindKey(pMapping);
		if (pKvKeybinds)
		{
			bool bMappedAKey = false; // Did we at least map a key?
			FOR_EACH_TRUE_SUBKEY(pKvKeybinds, pKvKeyMap)
			{
				// Each key map has a command as its name, and the key, and modifiers
				KeyMap_t mapping;

				const char *pKey = pKvKeyMap->GetString("key", nullptr);

				// If there's no/empty key, keep moving forward
				if (!pKey || !pKey[0])
					continue;

				bool bVirt = pKvKeyMap->GetBool("virtkey");

				unsigned potentialKey = bVirt ? GetKeyForStr(pKey) : pKey[0];

				if (potentialKey)
				{
					// If it doesn't have any modifiers it'll still work
					KeyValues *pKvModifiers = pKvKeyMap->FindKey("modifiers");
					if (pKvModifiers)
					{
						mapping.uModifierKeys = (pKvModifiers->GetBool("shift") * KEY_MOD_SHIFT) |
							(pKvModifiers->GetBool("ctrl") * KEY_MOD_CONTROL) |
							(pKvModifiers->GetBool("alt") * KEY_MOD_ALT);
					}
					else
						mapping.uModifierKeys = 0;

					// We're writing virtual here for the accel tables
					mapping.uModifierKeys |= bVirt * KEY_MOD_VIRT;

					mapping.uChar = potentialKey;
					mapping.uLogicalKey = 0; // This gets defined in the method that calls this

					map[pKvKeyMap->GetName()] = mapping;
					bMappedAKey = true;
				}
			}

			return bMappedAKey;
		}
	}

	return false;
}

bool KeyBinds::GetAccelTableFor(const char *pMapping, HACCEL &out)
{
	if (pMapping)
	{
		KeyValues *pKvKeybinds = m_pKvKeybinds->FindKey(pMapping);
		if (pKvKeybinds)
		{
			CUtlVector<ACCEL> accelerators;
			FOR_EACH_TRUE_SUBKEY(pKvKeybinds, pKvKeybind)
			{
				KeyValues *pKvModifiers = pKvKeybind->FindKey("modifiers");
				if (pKvModifiers)
				{
					const char *pKey = pKvKeybind->GetString("key", nullptr);
					if (pKey && pKey[0])
					{
						bool bVirt = pKvKeybind->GetBool("virtkey");

						WORD potentialKey = bVirt ? GetKeyForStr(pKey) : (WORD)pKey[0];

						WORD potentialID = GetIDForCommandStr(pKvKeybind->GetName());

						if (potentialKey && potentialID)
						{
							ACCEL acc;
							acc.fVirt = FNOINVERT | // NOINVERT needed so nothing causes the menu to show up
								(bVirt * FVIRTKEY) |
								(pKvModifiers->GetBool("shift") * FSHIFT) |
								(pKvModifiers->GetBool("ctrl") * FCONTROL) |
								(pKvModifiers->GetBool("alt") * FALT);
							acc.cmd = potentialID;
							acc.key = potentialKey;
							accelerators.AddToTail(acc);
						}
					}
				}
			}

			if (!accelerators.IsEmpty())
			{
				out = CreateAcceleratorTable(accelerators.Base(), accelerators.Count());
				return true;
			}
		}
	}

	AssertMsg(0, "Failed to get accel table for %s", pMapping);

	return false;
}
