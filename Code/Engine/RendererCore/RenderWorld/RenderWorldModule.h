#pragma once

#include <Core/World/WorldModule.h>

struct ezPerInstanceData;

class EZ_RENDERERCORE_DLL ezRenderWorldModule : public ezWorldModule
{
  EZ_DECLARE_WORLD_MODULE();
  EZ_ADD_DYNAMIC_REFLECTION(ezRenderWorldModule, ezWorldModule);

public:
  ezRenderWorldModule(ezWorld* pWorld);
  virtual ~ezRenderWorldModule();

  ezArrayPtr<ezPerInstanceData> EnsureInstanceDataIsAllocatedAndMapped(const ezComponent* pOwnerComponent, ezGALDynamicBufferHandle& out_hBuffer, ezUInt32& inout_uiInstanceDataOffset, ezUInt32 uiCount = 1) const;
  void DeallocateInstanceData(const ezComponent* pOwnerComponent, ezUInt32& inout_uiInstanceDataOffset) const;

  static void FillPerInstanceData(ezPerInstanceData& out_perInstanceData, const ezGameObject* pObject, ezUInt32 uiUniqueID = 0, const ezColor& color = ezColor::White, const ezVec4& vCustomData = ezVec4(0, 1, 0, 1));

private:
  void OnExtractionEvent(const ezRenderWorldExtractionEvent& e);

  mutable ezMutex m_ExtractionMutex;

  ezGALDynamicBufferHandle m_hInstanceDataBuffer[2]; // one for static objects, one for dynamic objects

  struct ExtractionData
  {
    ezGALDynamicBuffer* m_pInstanceDataBuffer[2] = {};
  };

  ExtractionData m_ExtractionData;
};

#include <RendererCore/RenderWorld/Implementation/RenderWorldModule_inl.h>
