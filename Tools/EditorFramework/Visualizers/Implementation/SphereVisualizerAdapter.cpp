#include <PCH.h>
#include <EditorFramework/Visualizers/SphereVisualizerAdapter.h>
#include <EditorFramework/Gizmos/GizmoHandle.h>
#include <EditorFramework/Assets/AssetDocument.h>

ezSphereVisualizerAdapter::ezSphereVisualizerAdapter()
{
}

ezSphereVisualizerAdapter::~ezSphereVisualizerAdapter()
{
}

void ezSphereVisualizerAdapter::Finalize()
{
  auto* pDoc = m_pObject->GetDocumentObjectManager()->GetDocument();
  const ezAssetDocument* pAssetDocument = ezDynamicCast<const ezAssetDocument*>(pDoc);
  EZ_ASSERT_DEV(pAssetDocument != nullptr, "Visualizers are only supported in ezAssetDocument.");

  const ezSphereVisualizerAttribute* pAttr = static_cast<const ezSphereVisualizerAttribute*>(m_pVisualizerAttr);

  m_Gizmo.Configure(nullptr, ezEngineGizmoHandleType::Sphere, pAttr->m_Color, false, false, true);

  m_Gizmo.SetOwner(pAssetDocument);
  m_Gizmo.SetVisible(m_bVisualizerIsVisible);
}

void ezSphereVisualizerAdapter::Update()
{
  m_Gizmo.SetVisible(m_bVisualizerIsVisible);

  const ezSphereVisualizerAttribute* pAttr = static_cast<const ezSphereVisualizerAttribute*>(m_pVisualizerAttr);

  m_Scale.SetIdentity();

  if (!pAttr->GetRadiusProperty().IsEmpty())
  {
    ezVariant value = m_pObject->GetTypeAccessor().GetValue(ezPropertyPath(pAttr->GetRadiusProperty()));

    EZ_ASSERT_DEBUG(value.IsValid() && value.CanConvertTo<float>(), "Invalid property bound to ezSphereVisualizerAttribute 'radius'");
    m_Scale.SetScalingMatrix(ezVec3(value.ConvertTo<float>()));
  }

  if (!pAttr->GetColorProperty().IsEmpty())
  {
    ezVariant value = m_pObject->GetTypeAccessor().GetValue(ezPropertyPath(pAttr->GetColorProperty()));

    EZ_ASSERT_DEBUG(value.IsValid() && value.CanConvertTo<ezColor>(), "Invalid property bound to ezSphereVisualizerAttribute 'color'");
    m_Gizmo.SetColor(value.ConvertTo<ezColor>());
  }
}

void ezSphereVisualizerAdapter::UpdateGizmoTransform()
{
  m_Gizmo.SetTransformation(GetObjectTransform().GetAsMat4() * m_Scale);
}


