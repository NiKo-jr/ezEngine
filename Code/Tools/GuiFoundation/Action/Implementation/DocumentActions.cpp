#include <GuiFoundation/PCH.h>
#include <GuiFoundation/Action/DocumentActions.h>
#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/DocumentWindow/DocumentWindow.moc.h>
#include <ToolsFoundation/Project/ToolsProject.h>
#include <Foundation/IO/OSFile.h>
#include <QProcess>
#include <QDir>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezDocumentAction, ezButtonAction, 0, ezRTTINoAllocator);
EZ_END_DYNAMIC_REFLECTED_TYPE();

////////////////////////////////////////////////////////////////////////
// ezDocumentActions
////////////////////////////////////////////////////////////////////////

ezActionDescriptorHandle ezDocumentActions::s_hSave;
ezActionDescriptorHandle ezDocumentActions::s_hSaveAs;
ezActionDescriptorHandle ezDocumentActions::s_hSaveAll;
ezActionDescriptorHandle ezDocumentActions::s_hClose;
ezActionDescriptorHandle ezDocumentActions::s_hOpenContainingFolder;

void ezDocumentActions::RegisterActions()
{
  s_hSave = EZ_REGISTER_ACTION_1("Save", "Save", ezActionScope::Document, "Document", ezDocumentAction, ezDocumentAction::ButtonType::Save);
  s_hSaveAll = EZ_REGISTER_ACTION_1("Save All", "Save All", ezActionScope::Document, "Document", ezDocumentAction, ezDocumentAction::ButtonType::SaveAll);
  s_hSaveAs = EZ_REGISTER_ACTION_1("Save As", "Save As...", ezActionScope::Document, "Document", ezDocumentAction, ezDocumentAction::ButtonType::SaveAs);
  s_hClose = EZ_REGISTER_ACTION_1("Close", "Close", ezActionScope::Document, "Document", ezDocumentAction, ezDocumentAction::ButtonType::Close);
  s_hOpenContainingFolder = EZ_REGISTER_ACTION_1("Open Containing Folder", "Open Containing Folder", ezActionScope::Document, "Document", ezDocumentAction, ezDocumentAction::ButtonType::OpenContainingFolder);
}

void ezDocumentActions::UnregisterActions()
{
  ezActionManager::UnregisterAction(s_hSave);
  ezActionManager::UnregisterAction(s_hSaveAs);
  ezActionManager::UnregisterAction(s_hSaveAll);
  ezActionManager::UnregisterAction(s_hClose);
  ezActionManager::UnregisterAction(s_hOpenContainingFolder);
}

void ezDocumentActions::MapActions(const char* szMapping, const char* szPath)
{
  ezActionMap* pMap = ezActionMapManager::GetActionMap(szMapping);
  EZ_ASSERT_DEV(pMap != nullptr, "The given mapping ('%s') does not exist, mapping the documents actions failed!", szMapping);

  pMap->MapAction(s_hSave, szPath, 1.0f);
  //pMap->MapAction(s_hSaveAs, szPath, 2.0f);
  pMap->MapAction(s_hSaveAll, szPath, 3.0f);
  pMap->MapAction(s_hClose, szPath, 4.0f);
  pMap->MapAction(s_hOpenContainingFolder, szPath, 5.0f);
}

////////////////////////////////////////////////////////////////////////
// ezDocumentAction
////////////////////////////////////////////////////////////////////////

ezDocumentAction::ezDocumentAction(const ezActionContext& context, const char* szName, ButtonType button)
  : ezButtonAction(context, szName, false)
{
  m_ButtonType = button;

  if (context.m_pDocument == nullptr)
  {
    if (button == ButtonType::Save || button == ButtonType::SaveAs)
    {
      // for actions that require a document, hide them
      SetVisible(false);
    }
  }
  else
  {
    m_Context.m_pDocument->m_EventsOne.AddEventHandler(ezMakeDelegate(&ezDocumentAction::DocumentEventHandler, this));

    if (m_ButtonType == ButtonType::Save)
    {
      SetVisible(!m_Context.m_pDocument->IsReadOnly());
      SetEnabled(m_Context.m_pDocument->IsModified());
    }
  }
}

ezDocumentAction::~ezDocumentAction()
{
  if (m_Context.m_pDocument)
  {
    m_Context.m_pDocument->m_EventsOne.RemoveEventHandler(ezMakeDelegate(&ezDocumentAction::DocumentEventHandler, this));
  }
}

void ezDocumentAction::DocumentEventHandler(const ezDocumentBase::Event& e)
{
  switch (e.m_Type)
  {
  case ezDocumentBase::Event::Type::DocumentSaved:
  case ezDocumentBase::Event::Type::ModifiedChanged:
    {
      if (m_ButtonType == ButtonType::Save)
      {
        SetEnabled(m_Context.m_pDocument->IsModified());
      }
    }
    break;
  }
}

void ezDocumentAction::Execute(const ezVariant& value)
{
  switch (m_ButtonType)
  {
  case ezDocumentAction::ButtonType::Save:
    {
      ezDocumentWindow* pWnd = ezDocumentWindow::FindWindowByDocument(m_Context.m_pDocument);
      pWnd->SaveDocument();
    }
    break;

  case ezDocumentAction::ButtonType::SaveAs:
    /// \todo Save as
    break;

  case ezDocumentAction::ButtonType::SaveAll:
    {
      for (auto pMan : ezDocumentManagerBase::GetAllDocumentManagers())
      {
        for (auto pDoc : pMan->ezDocumentManagerBase::GetAllDocuments())
        {
          ezDocumentWindow* pWnd = ezDocumentWindow::FindWindowByDocument(pDoc);
          
          if (pWnd->SaveDocument().m_Result.Failed())
            return;
        }
      }
    }
    break;

  case ezDocumentAction::ButtonType::Close:
    {
      ezDocumentWindow* pWnd = ezDocumentWindow::FindWindowByDocument(m_Context.m_pDocument);

      if (!pWnd->CanCloseWindow())
        return;

      pWnd->CloseDocumentWindow();
    }
    break;

  case ezDocumentAction::ButtonType::OpenContainingFolder:
    {
      ezString sPath;

      if (!m_Context.m_pDocument)
      {
        if (ezToolsProject::IsProjectOpen())
          sPath = ezToolsProject::GetInstance()->GetProjectPath();
        else
          sPath = ezOSFile::GetApplicationDirectory();
      }
      else
        sPath = m_Context.m_pDocument->GetDocumentPath();

      QStringList args;
      args << "/select," << QDir::toNativeSeparators(sPath.GetData());
      QProcess::startDetached("explorer", args);

    }
    break;
  }
}

