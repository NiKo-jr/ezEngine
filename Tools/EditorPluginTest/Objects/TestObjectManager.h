#pragma once

#include <ToolsFoundation/Object/DocumentObjectBase.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Reflection/ReflectedTypeDirectAccessor.h>

class ezDocumentBase;

class ezTestObjectManager : public ezDocumentObjectManagerBase
{
public:
  ezTestObjectManager();
  virtual void GetCreateableTypes(ezHybridArray<ezRTTI*, 32>& Types) const override;

private:

  virtual ezDocumentObjectBase* InternalCreateObject(const ezRTTI* pRtti) override;
  virtual void InternalDestroyObject(ezDocumentObjectBase* pObject) override;
  virtual bool InternalCanAdd(const ezRTTI* pRtti, const ezDocumentObjectBase* pParent) const override;
  virtual bool InternalCanRemove(const ezDocumentObjectBase* pObject) const override;
  virtual bool InternalCanMove(const ezDocumentObjectBase* pObject, const ezDocumentObjectBase* pNewParent, ezInt32 iChildIndex) const override;

};


