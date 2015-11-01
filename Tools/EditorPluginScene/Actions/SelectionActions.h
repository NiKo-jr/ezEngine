#pragma once

#include <EditorPluginScene/Plugin.h>
#include <GuiFoundation/Basics.h>
#include <GuiFoundation/Action/BaseActions.h>
#include <EditorPluginScene/Scene/SceneDocument.h>

///
class EZ_EDITORPLUGINSCENE_DLL ezSelectionActions
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapActions(const char* szMapping, const char* szPath);
  static void MapPrefabActions(const char* szMapping, const char* szPath, float fPriority);
  static void MapContextMenuActions(const char* szMapping, const char* szPath);

  static ezActionDescriptorHandle s_hSelectionCategory;
  static ezActionDescriptorHandle s_hShowInScenegraph;
  static ezActionDescriptorHandle s_hFocusOnSelection;
  static ezActionDescriptorHandle s_hFocusOnSelectionAllViews;
  static ezActionDescriptorHandle s_hGroupSelectedItems;
  static ezActionDescriptorHandle s_hHideSelectedObjects;
  static ezActionDescriptorHandle s_hHideUnselectedObjects;
  static ezActionDescriptorHandle s_hShowHiddenObjects;
  static ezActionDescriptorHandle s_hPrefabMenu;
  static ezActionDescriptorHandle s_hCreatePrefab;
  static ezActionDescriptorHandle s_hRevertPrefab;
  static ezActionDescriptorHandle s_hOpenPrefabDocument;

};

///
class EZ_EDITORPLUGINSCENE_DLL ezSelectionAction : public ezButtonAction
{
  EZ_ADD_DYNAMIC_REFLECTION(ezSelectionAction);

public:

  enum class ActionType
  {
    ShowInScenegraph,
    FocusOnSelection,
    FocusOnSelectionAllViews,
    GroupSelectedItems,
    HideSelectedObjects,
    HideUnselectedObjects,
    ShowHiddenObjects,
    CreatePrefab,
    RevertPrefab,
    OpenPrefabDocument,
  };

  ezSelectionAction(const ezActionContext& context, const char* szName, ActionType type);
  ~ezSelectionAction();

  virtual void Execute(const ezVariant& value) override;

  void OpenPrefabDocument();

  void CreatePrefab();

private:
  void SelectionEventHandler(const ezSelectionManager::Event& e);

  void UpdateEnableState();

  ezSceneDocument* m_pSceneDocument;
  ActionType m_Type;
};




