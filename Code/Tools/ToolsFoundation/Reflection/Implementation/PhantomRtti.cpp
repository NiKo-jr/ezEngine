#include <ToolsFoundation/PCH.h>
#include <ToolsFoundation/Reflection/PhantomRtti.h>
#include <ToolsFoundation/Reflection/PhantomProperty.h>

ezPhantomRTTI::ezPhantomRTTI(const char* szName, const ezRTTI* pParentType, ezUInt32 uiTypeSize, ezUInt32 uiTypeVersion, ezUInt32 uiVariantType, ezBitflags<ezTypeFlags> flags, const char* szPluginName)
  : ezRTTI(nullptr, pParentType, uiTypeSize, uiTypeVersion, uiVariantType, flags | ezTypeFlags::Phantom, nullptr, ezArrayPtr<ezAbstractProperty*>(), ezArrayPtr<ezAbstractMessageHandler*>() )
{
  m_sTypeNameStorage = szName;
  m_sPluginNameStorage = szPluginName;

  m_szTypeName = m_sTypeNameStorage.GetData();
  m_szPluginName = m_sPluginNameStorage.GetData();
}

ezPhantomRTTI::~ezPhantomRTTI()
{
  for (auto pProp : m_PropertiesStorage)
  {
    EZ_DEFAULT_DELETE(pProp);
  }
}

void ezPhantomRTTI::SetProperties(const ezDynamicArray<ezReflectedPropertyDescriptor>& properties)
{
  m_PropertiesStorage.Clear();

  const ezUInt32 iCount = properties.GetCount();
  m_PropertiesStorage.Reserve(iCount);

  for (ezUInt32 i = 0; i < iCount; i++)
  {
    switch (properties[i].m_Category)
    {
    case ezPropertyCategory::Array:
      {
        m_PropertiesStorage.PushBack(EZ_DEFAULT_NEW(ezPhantomArrayProperty, &properties[i]));
      }
      break;
    case ezPropertyCategory::Constant:
      {
        m_PropertiesStorage.PushBack(EZ_DEFAULT_NEW(ezPhantomConstantProperty, &properties[i]));
      }
      break;
    case ezPropertyCategory::Function:
      {
        m_PropertiesStorage.PushBack(EZ_DEFAULT_NEW(ezPhantomFunctionProperty, &properties[i]));
      }
      break;
    case ezPropertyCategory::Map:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        //m_PropertiesStorage.PushBack(EZ_DEFAULT_NEW(ezPhantomMapProperty, &properties[i]));
      }
      break;
    case ezPropertyCategory::Member:
      {
        m_PropertiesStorage.PushBack(EZ_DEFAULT_NEW(ezPhantomMemberProperty, &properties[i]));
      }
      break;
    case ezPropertyCategory::Set:
      {
        m_PropertiesStorage.PushBack(EZ_DEFAULT_NEW(ezPhantomSetProperty, &properties[i]));
      }
      break;
    }

  }

  m_Properties = m_PropertiesStorage;
}

void ezPhantomRTTI::UpdateType(const ezReflectedTypeDescriptor& desc)
{
  ezRTTI::UpdateType(ezRTTI::FindTypeByName(desc.m_sParentTypeName), desc.m_uiTypeSize, desc.m_uiTypeVersion, ezVariantType::Invalid, desc.m_Flags);

  m_sPluginNameStorage = desc.m_sPluginName;
  m_szPluginName = m_sPluginNameStorage.GetData();

  SetProperties(desc.m_Properties);
}

bool ezPhantomRTTI::IsEqualToDescriptor(const ezReflectedTypeDescriptor& desc)
{
  if ((desc.m_Flags.GetValue() & ~ezTypeFlags::Phantom) != (GetTypeFlags().GetValue() & ~ezTypeFlags::Phantom))
    return false;

  if (desc.m_sParentTypeName.IsEmpty() && GetParentType() != nullptr)
      return false;

  if (desc.m_sParentTypeName != GetParentType()->GetTypeName())
    return false;

  if (desc.m_sPluginName != GetPluginName())
    return false;

  if (desc.m_sTypeName != GetTypeName())
    return false;

  if (desc.m_Properties.GetCount() != GetProperties().GetCount())
    return false;

  for (ezUInt32 i = 0; i < GetProperties().GetCount(); i++)
  {
    if (desc.m_Properties[i].m_Category != GetProperties()[i]->GetCategory())
      return false;

    if (desc.m_Properties[i].m_sName != GetProperties()[i]->GetPropertyName())
      return false;

    if ((desc.m_Properties[i].m_Flags.GetValue() & ~ezPropertyFlags::Phantom) != (GetProperties()[i]->GetFlags().GetValue() & ~ezPropertyFlags::Phantom))
      return false;

    switch (desc.m_Properties[i].m_Category)
    {
    case ezPropertyCategory::Array:
      {
        auto pProp = (ezPhantomArrayProperty*)GetProperties()[i];

        if (pProp->GetElementType() != ezRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
    case ezPropertyCategory::Constant:
      {
        auto pProp = (ezPhantomConstantProperty*)GetProperties()[i];

        if (pProp->GetPropertyType() != ezRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;

        if (pProp->GetConstant() != desc.m_Properties[i].m_ConstantValue)
          return false;
      }
      break;
    case ezPropertyCategory::Function:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
      }
      break;
    case ezPropertyCategory::Map:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
      }
      break;
    case ezPropertyCategory::Member:
      {
        auto pProp = (ezPhantomMemberProperty*)GetProperties()[i];

        if (pProp->GetPropertyType() != ezRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
    case ezPropertyCategory::Set:
      {
        auto pProp = (ezPhantomSetProperty*)GetProperties()[i];

        if (pProp->GetElementType() != ezRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
    }

  }
  return true;
}
