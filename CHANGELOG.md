### Source Level Editor (SLE) Changelog

## 8.12.2025 - 2.108 update

### New:
- Added option to Freeze/Freeze Unselected/Unfreeze All. "Frozen" objects cannot be selected in the viewports - this can be desired when working in densely cluttered areas. The objects can still be selected via Entity Report if necessary.
- Autosaves work on a separate thread, practically unnoticeable (no hitching/waiting for saving to be over); this is ported from Hammer-2013.
- Maps can be exported (visible objects only) as a .map file under any config, via Tools->Export submenu; the format is Valve 220.
- Selected entities can be exported to a Commentary .txt file, working as a replacement for in-game Commentary Editor. Done via Tools->Export submenu. 
- The level_editor.fgd includes commented-out definition for point_commentary_node. Uncommenting it will make it appear in the editor; this should only be done if the entity isn't already defined in the active mod or game's own FGD, otherwise it will be superceded by this one.
- (script) and (scriptlist) FGD data types for VScript are now recognised; (script) supports browsing for single files, (scriptlist) at the moment only works as a plain string for manual typing into it - browsing for multiple files will be added in the future.

### Changes:
- Justify Top/Bottom/Left/Right/Center now rounds the texture coords to integer, as was done in Hammer 4.1
- point_commentary_node has been added to the editor .fgd, to allow placing them for exporting
- Made "Load instances" setting not saved between sessions (deemed disruptive to workflow)
- Lightmap Grid is tinted a bit differently (orange tint for scale <= 2, light blue for >= 64, green for >= 64, deep blue for >= 128)
- Displacements now show Lightmap Grid in same tint as the brushes
- Sprites and icons are now rendered normally in Flat view
- Spotlight helper is not rendered in either Flat or Lightmap Grid views
- Adjusted the way Auto-visgroups are assigned for brushes, which should speed up the process

### Fixes:
- Fixed HL:S scientist bodygroup value crashing the editor due to using -1 by default
- Fixed first-time setup dialogue linking to old Valve support website
- Removed the black bar above the volume slider in the Sound and Scene browser
- Fixed the Logical View not using its own context menu
- Fixed .map files not loading 
- Fixed saving as RMF/MAP files selection being unreliable
- Fixed TF2 Map Format not being properly accounted for in the code in several cases
- Fixed Displacement Subdivision using incorrect Subdiv function in one part of the code (mistakenly forgotten SLE A/B testing, not a previous bug fix)
- Fixed users being able to add an Auto visgroup manually by calling a new visgroup that (i. e. "Auto")