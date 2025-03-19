#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererCore/RenderWorld/RenderWorldModule.h>
#include <RendererFoundation/Resources/DynamicBuffer.h>

// clang-format off
EZ_IMPLEMENT_WORLD_MODULE(ezRenderWorldModule);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRenderWorldModule, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezRenderWorldModule::ezRenderWorldModule(ezWorld* pWorld)
  : ezWorldModule(pWorld)
{
  ezRenderWorld::GetExtractionEvent().AddEventHandler(ezMakeDelegate(&ezRenderWorldModule::OnExtractionEvent, this));

  ezGALBufferCreationDescription desc;
  desc.m_uiStructSize = sizeof(ezPerInstanceData);
  desc.m_uiTotalSize = 1024 * desc.m_uiStructSize; // TODO: make initial size configurable
  desc.m_BufferFlags = ezGALBufferUsageFlags::StructuredBuffer | ezGALBufferUsageFlags::ShaderResource;
  desc.m_ResourceAccess.m_bImmutable = false;

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  m_hInstanceDataBuffer[0] = pDevice->CreateDynamicBuffer(desc, "Static Instance Data");
  m_hInstanceDataBuffer[1] = pDevice->CreateDynamicBuffer(desc, "Dynamic Instance Data");
}

ezRenderWorldModule::~ezRenderWorldModule()
{
  ezRenderWorld::GetExtractionEvent().RemoveEventHandler(ezMakeDelegate(&ezRenderWorldModule::OnExtractionEvent, this));
}

ezArrayPtr<ezPerInstanceData> ezRenderWorldModule::EnsureInstanceDataIsAllocatedAndMapped(const ezComponent* pOwnerComponent, ezGALDynamicBufferHandle& out_hBuffer, ezUInt32& inout_uiInstanceDataOffset, ezUInt32 uiCount /*= 1*/) const
{
  EZ_LOCK(m_ExtractionMutex);

  ezUInt32 uiIsDynamic = pOwnerComponent->GetOwner()->IsDynamic() ? 1 : 0;
  out_hBuffer = m_hInstanceDataBuffer[uiIsDynamic];

  auto pInstanceDataBuffer = m_ExtractionData.m_pInstanceDataBuffer[uiIsDynamic];
  if (pInstanceDataBuffer == nullptr)
  {
    pInstanceDataBuffer = ezGALDevice::GetDefaultDevice()->GetDynamicBuffer(out_hBuffer);
  }

  if (inout_uiInstanceDataOffset == ezInvalidIndex)
  {
    inout_uiInstanceDataOffset = pInstanceDataBuffer->Allocate(pOwnerComponent->GetHandle(), uiCount);
  }

  return pInstanceDataBuffer->MapForWriting<ezPerInstanceData>(inout_uiInstanceDataOffset);
}

void ezRenderWorldModule::DeallocateInstanceData(const ezComponent* pOwnerComponent, ezUInt32& inout_uiInstanceDataOffset) const
{
  EZ_LOCK(m_ExtractionMutex);

  EZ_ASSERT_NOT_IMPLEMENTED;

  /*if (uiInstanceDataOffset != ezInvalidIndex)
  {
    auto pInstanceDataBuffer = m_ExtractionData.m_pInstanceDataBuffer;
    if (pInstanceDataBuffer == nullptr)
    {
      pInstanceDataBuffer = ezGALDevice::GetDefaultDevice()->GetDynamicBuffer(m_hInstanceDataBuffer);
    }

    pInstanceDataBuffer->Deallocate(uiInstanceDataOffset);
    uiInstanceDataOffset = ezInvalidIndex;
  }*/
}

void ezRenderWorldModule::OnExtractionEvent(const ezRenderWorldExtractionEvent& e)
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  if (e.m_Type == ezRenderWorldExtractionEvent::Type::BeginExtraction)
  {
    m_ExtractionData.m_pInstanceDataBuffer[0] = pDevice->GetDynamicBuffer(m_hInstanceDataBuffer[0]);
    m_ExtractionData.m_pInstanceDataBuffer[1] = pDevice->GetDynamicBuffer(m_hInstanceDataBuffer[1]);
  }
  else if (e.m_Type == ezRenderWorldExtractionEvent::Type::EndExtraction)
  {
    m_ExtractionData.m_pInstanceDataBuffer[0]->UploadChangesForNextFrame();
    m_ExtractionData.m_pInstanceDataBuffer[1]->UploadChangesForNextFrame();

    m_ExtractionData = {};
  }
}
