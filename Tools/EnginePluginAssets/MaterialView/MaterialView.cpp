#include <PCH.h>
#include <EnginePluginAssets/MaterialView/MaterialView.h>
#include <RendererFoundation/Device/SwapChain.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <RendererCore/RenderLoop/RenderLoop.h>
#include <RendererCore/Pipeline/Extractor.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/Passes/SelectionHighlightPass.h>
#include <RendererCore/Pipeline/Passes/SimpleRenderPass.h>
#include <RendererCore/Pipeline/Passes/TargetPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererFoundation/Resources/RenderTargetSetup.h>
#include <GameFoundation/GameApplication/GameApplication.h>
#include <EditorFramework/EngineProcess/EngineProcessDocumentContext.h>
#include <EditorFramework/EngineProcess/EngineProcessMessages.h>
#include <EditorFramework/Gizmos/GizmoRenderer.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <Foundation/Utilities/GraphicsUtils.h>
#include <Core/World/GameObject.h>
#include <Core/World/Component.h>
#include <MaterialContext/MaterialContext.h>
#include <RendererCore/Pipeline/Implementation/RenderPipelineResourceLoader.h>
#include <RendererCore/Meshes/MeshRenderer.h>
#include <RendererCore/Lights/LightGatheringRenderer.h>

ezMaterialViewContext::ezMaterialViewContext(ezMaterialContext* pMaterialContext) : ezEngineProcessViewContext(pMaterialContext)
{
  m_pMaterialContext = pMaterialContext;
  m_pView = nullptr;
}

ezMaterialViewContext::~ezMaterialViewContext()
{
  ezRenderLoop::DeleteView(m_pView);

  if (GetEditorWindow().m_hWnd != 0)
  {
    static_cast<ezGameApplication*>(ezApplication::GetApplicationInstance())->RemoveWindow(&GetEditorWindow());
  }
}

ezView* ezMaterialViewContext::CreateView()
{
  ezView* pView = ezRenderLoop::CreateView("Material Editor - View");

  pView->SetRenderPipelineResource(CreateDefaultRenderPipeline());

  ezEngineProcessDocumentContext* pDocumentContext = GetDocumentContext();
  pView->SetWorld(pDocumentContext->GetWorld());
  pView->SetLogicCamera(&m_Camera);
  return pView;
}

bool ezMaterialViewContext::IsDefaultRenderPipeline(ezRenderPipelineResourceHandle hPipeline)
{
  return hPipeline == ezGameState::GetMainRenderPipeline();
}

ezRenderPipelineResourceHandle ezMaterialViewContext::CreateDefaultRenderPipeline()
{
  return ezGameState::GetMainRenderPipeline();
}