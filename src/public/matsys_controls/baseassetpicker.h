//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#ifndef BASEASSETPICKER_H
#define BASEASSETPICKER_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/Frame.h"
#include "tier1/utlstring.h"
#include "tier1/utllinkedlist.h"
#include "filesystem.h"
#include "assetpickerdefs.h" //// SLE NEW - backports for matsys_controls


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CAssetTreeView;
namespace vgui
{
	class Panel;
}
FORWARD_DECLARE_HANDLE( AssetList_t );

typedef unsigned short DirHandle_t;

struct CachedAssetInfo_t //// SLE NEW - backports for matsys_controls
{
	CUtlString m_AssetName;
	int m_nModIndex;
	int m_nTimesUsed;
};

struct CacheModInfo_t
{
	CUtlString m_ModName;
	CUtlString m_Path;
};

//-----------------------------------------------------------------------------
// Purpose: Base class for choosing raw assets
//-----------------------------------------------------------------------------
class CBaseAssetPicker : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CBaseAssetPicker, vgui::EditablePanel );

public:
	CBaseAssetPicker( vgui::Panel *pParent, const char *pAssetType, 
		const char *pExt, const char *pSubDir, const char *pTextType, const char *pSearchPath = "GAME" /*SLE NEW - backports for matsys_controls*/);
	~CBaseAssetPicker();

	// overridden frame functions
	virtual void OnTick();
	virtual bool HasUserConfigSettings();
	virtual void ApplyUserConfigSettings( KeyValues *pUserConfig );
	virtual void GetUserConfigSettings( KeyValues *pUserConfig );
	virtual void OnCommand( const char *pCommand );

	// Purpose: 
	virtual void OnKeyCodePressed( vgui::KeyCode code );

	// Returns the selected asset name
	int GetSelectedAssetCount();
	const char *GetSelectedAsset( int nAssetIndex = -1 );
	int GetSelectedAssetIndex(int nSelectionIndex); //// SLE NEW - backports for matsys_controls

	// Is multiselect enabled?
	bool IsMultiselectEnabled() const;
	void SetAllowMultiselect(bool bAllowMultiselect); //// SLE NEW - backports for matsys_controls

	// Sets the initial selected asset
	void SetSelection(const char *pAssetName, bool bInitialSelection = false); //// SLE NEW - backports for matsys_controls
	void SetInitialSelection( const char *pAssetName );

	void SetUsedAssetList(CUtlVector<AssetUsageInfo_t> &usedAssets); //// SLE NEW - backports for matsys_controls

	// Set/get the filter
	void SetFilter( const char *pFilter );
	const char *GetFilter();

	// Purpose: refreshes the file tree
	void RefreshFileTree();

	virtual void Activate();
	void CloseModal(); //// SLE NEW - backports for matsys_controls

	virtual void CustomizeSelectionMessage(KeyValues *pKeyValues) {}

	//// SLE NEW - backports for matsys_controls (oof, all of these)
	// asset cache interface
	virtual int GetAssetCount();
	virtual const char *GetAssetName(int nAssetIndex);
	virtual const CachedAssetInfo_t& GetCachedAsset(int nAssetIndex);
	virtual int GetCachedAssetCount();
	virtual bool IncrementalCacheAssets(float flTimeAllowed); // return true if finished
	virtual bool BeginCacheAssets(bool bForceRecache); // return true if finished
	virtual CUtlString GetSelectedAssetFullPath(int nIndex);

	int ModCount() const;
	const CacheModInfo_t& ModInfo(int nIndex) const;
	////

protected:
	// Creates standard controls. Allows the derived class to
	// add these controls to various splitter windows
	void CreateStandardControls( vgui::Panel *pParent, bool bAllowMultiselect = false );
	void AutoLayoutStandardControls(); //// SLE NEW - backports for matsys_controls

	// Allows the picker to browse multiple asset types
	void AddExtension( const char *pExtension );

	// Derived classes have this called when the previewed asset changes
	virtual void OnSelectedAssetPicked( const char *pAssetName ) {}

	// Derived classes have this called when the next selected asset is selected by default
	virtual void OnNextSelectionIsDefault() {}

	// Derived classes have this called when the filtered list changes
	virtual void OnAssetListChanged() {} //// SLE NEW - backports for matsys_controls

	// Request focus of the filter box
	void RequestFilterFocus();

	// Rescan assets
	void RescanAssets();
	bool DoIncrementalCache(); //// SLE NEW - backports for matsys_controls

	// Is a particular asset visible?
	bool IsAssetVisible(int nAssetIndex); //// SLE NEW - backports for matsys_controls

	const char	*GetModPath( int nModIndex );

	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", kv );
	MESSAGE_FUNC_PARAMS( OnItemSelected, "ItemSelected", kv );
	MESSAGE_FUNC_PARAMS( OnItemDeselected, "ItemDeselected", kv); //// SLE NEW - backports for matsys_controls
	MESSAGE_FUNC_PARAMS( OnCheckButtonChecked, "CheckButtonChecked", kv );
	MESSAGE_FUNC( OnFileSelected, "TreeViewItemSelected" );
	
protected:
	struct AssetInfo_t
	{
		int m_nAssetIndex;
		int m_nItemId;
	};

	void BuildAssetNameList();
	void RefreshAssetList( );
	int GetSelectedAssetModIndex( );

	// Is a particular asset visible?
//	bool IsAssetVisible( int nAssetIndex );

	// Recursively add all files matching the wildcard under this directory
	void AddAssetToList( int nAssetIndex );

	// Update column headers
	void UpdateAssetColumnHeader( );

	vgui::Splitter *m_pAssetSplitter;
	CAssetTreeView* m_pFileTree;
	vgui::CheckButton* m_pSubDirCheck;
	vgui::TextEntry *m_pFilter;
	vgui::ListPanel *m_pAssetBrowser;
	vgui::TextEntry *m_pFullPath;
	vgui::ComboBox *m_pModSelector;
	vgui::Button *m_pRescanButton;

	vgui::Button *m_pFindAssetButton; //// SLE NEW - backports for matsys_controls
	KeyValues *m_pInsertHelper;
	vgui::CheckButton *m_pOnlyUsedCheck;

	AssetList_t m_hAssetList;
	CUtlString m_FolderFilter;
	CUtlString m_Filter;
	CUtlString m_SelectedAsset;
	CUtlVector< AssetInfo_t > m_AssetList;
	const char *m_pAssetType;
	const char *m_pAssetTextType;
	const char *m_pAssetExt;
	const char *m_pAssetSubDir;
	const char *m_pAssetSearchPath; //// SLE NEW - backports for matsys_controls
	CUtlVector< const char * > m_ExtraAssetExt;

	bool m_bBuiltAssetList : 1;
	bool m_bFirstAssetScan : 1;
	bool m_bFinishedAssetListScan : 1;
	bool m_bSubDirCheck : 1;
	bool m_bOnlyUsedAssetsCheck : 1; //// SLE NEW - backports for matsys_controls

	int m_nCurrentModFilter;
	int m_nMatchingAssets;
	CUtlVector<AssetUsageInfo_t> m_usedAssets; //// SLE NEW - backports for matsys_controls

	friend class CBaseAssetPickerFrame;
};


//-----------------------------------------------------------------------------
// Purpose: Modal dialog for asset picker
//-----------------------------------------------------------------------------
class CBaseAssetPickerFrame : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CBaseAssetPickerFrame, vgui::Frame );

public:
	CBaseAssetPickerFrame( vgui::Panel *pParent );
	~CBaseAssetPickerFrame();

	// Inherited from Frame
	virtual void OnCommand( const char *pCommand );

	// Purpose: Activate the dialog
	// The message "AssetSelected" will be sent if an asset is picked
	// Pass in optional keyvalues to add to the message
	void DoModal( KeyValues *pContextKeyValues = NULL );

	// Sets the initial selected asset
	void SetInitialSelection( const char *pAssetName );

	// Set/get the filter
	void SetFilter( const char *pFilter );
	const char *GetFilter();

	void SetAllowMultiselect(bool bAllowMultiselect); //// SLE NEW - backports for matsys_controls

protected:
	// Allows the derived class to create the picker
	void SetAssetPicker( CBaseAssetPicker* pPicker ); 
	CBaseAssetPicker* GetAssetPicker() { return m_pPicker; }

	// Posts a message (passing the key values)
	void PostMessageAndClose( KeyValues *pKeyValues );
	virtual void CloseModal(); //// SLE NEW - backports for matsys_controls

private:
	void CleanUpMessage();

	CBaseAssetPicker *m_pPicker;
	vgui::Button *m_pOpenButton;
	vgui::Button *m_pCancelButton;
	KeyValues *m_pContextKeyValues;
};


#endif // BASEASSETPICKER_H
