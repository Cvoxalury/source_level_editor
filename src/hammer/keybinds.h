#ifndef KEYBINDS_H
#define KEYBINDS_H
#pragma once

#include "Keyboard.h"
#include "UtlStringMap.h"

class KeyValues;
class CUtlSymbolTable;

struct KeyBind
{
	char bindName[128];
	KeyMap_t keymap;
};

#define BEGIN_KEYMAP(mapping) \
    CUtlStringMap<KeyMap_t> mappings; \
    if (g_pKeyBinds->LoadKeybinds(mapping, mappings))

#define _ADD_KEY(string, enumVal) \
    if (mappings.Defined(string)) \
    { \
        KeyMap_t map = mappings[string]; \
        m_Keyboard.AddKeyMap(map.uChar, map.uModifierKeys, enumVal); \
    }

#define ADD_KEY(enumVal) _ADD_KEY(#enumVal, enumVal)

class KeyBinds
{

public:
	KeyBinds();
	~KeyBinds();

	bool Init();
	void Shutdown();

	bool LoadKeybinds(const char *pMapping, CUtlStringMap<KeyMap_t> &map);

	bool GetAccelTableFor(const char *pMapping, HACCEL &out);

private:

	KeyValues *m_pKvKeybinds;

};

extern KeyBinds *g_pKeyBinds;
#endif // KEYBINDS_H