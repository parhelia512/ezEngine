#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Configuration/Startup.h>
#include <RendererCore/Material/MaterialManager.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererFoundation/CommandEncoder/CommandEncoder.h>
#include <RendererFoundation/Device/Device.h>

EZ_IMPLEMENT_SINGLETON(ezMaterialManager);

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, MaterialManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ResourceManager"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    EZ_DEFAULT_NEW(ezMaterialManager);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    ezMaterialManager* pDummy = ezMaterialManager::GetSingleton();
    EZ_DEFAULT_DELETE(pDummy);
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

const ezMaterialManager::MaterialData& ezMaterialManager::GetMaterialData(const ezMaterialResource* pMaterial) const
{
  auto it = m_Materials.Find(pMaterial);
  EZ_ASSERT_DEV(it.IsValid(), "Loaded materials must always have a valid entry in m_Materials");
  return it.Value();
}

ezMaterialManager::ezMaterialManager()
  : m_SingletonRegistrar(this)
{
  ezRenderWorld::GetExtractionEvent().AddEventHandler(ezMakeDelegate(&ezMaterialManager::OnExtractionEvent, this));
  ezRenderWorld::GetRenderEvent().AddEventHandler(ezMakeDelegate(&ezMaterialManager::OnRenderEvent, this));
}

ezMaterialManager::~ezMaterialManager()
{
  ezRenderWorld::GetExtractionEvent().RemoveEventHandler(ezMakeDelegate(&ezMaterialManager::OnExtractionEvent, this));
  ezRenderWorld::GetRenderEvent().RemoveEventHandler(ezMakeDelegate(&ezMaterialManager::OnRenderEvent, this));
}

void ezMaterialManager::MaterialUpdated(ezMaterialResource* pMaterial)
{
  EZ_LOCK(m_MaterialShaderMutex);

  const ezShaderResourceHandle hOldShader = pMaterial->m_hShader;
  const ezMaterialResource::ezMaterialId oldId = pMaterial->m_MaterialId;
  const bool bShaderChanged = pMaterial->m_mDesc.m_hShader != hOldShader;

  if (!hOldShader.IsValid())
  {
    // Newly created material
    pMaterial->m_hShader = pMaterial->m_mDesc.m_hShader;
    MaterialShaderConstants& msc = GetShaderConstants(pMaterial->m_hShader);
    pMaterial->m_MaterialId = msc.AddMaterial(pMaterial->GetResourceHandle());
  }
  else if (bShaderChanged)
  {
    // Material reloaded and changed shader
    {
      // Delete old registration
      EZ_LOCK(m_ExtractionMutex);
      m_RemovedMaterials.PushBack({pMaterial, hOldShader, oldId});
    }
    // Create new material registration
    pMaterial->m_hShader = pMaterial->m_mDesc.m_hShader;
    MaterialShaderConstants& msc = GetShaderConstants(pMaterial->m_hShader);
    pMaterial->m_MaterialId = msc.AddMaterial(pMaterial->GetResourceHandle());
  }
  else
  {
    // Material reloaded with same shader. Nothing to do except marking material dirty.
  }
}

void ezMaterialManager::MaterialRemoved(ezMaterialResource* pMaterial)
{
  if (pMaterial->m_MaterialId.IsInvalidated())
    return;

  EZ_LOCK(m_ExtractionMutex);
  m_RemovedMaterials.PushBack({pMaterial, pMaterial->m_hShader, pMaterial->m_MaterialId});
}

void ezMaterialManager::MaterialModified(ezMaterialResourceHandle hMaterial)
{
  EZ_LOCK(m_ExtractionMutex);
  m_ChangedMaterials.Insert(hMaterial);
}

void ezMaterialManager::OnExtractionEvent(const ezRenderWorldExtractionEvent& e)
{
  // ezUInt32 uiDataIndex = ezRenderWorld::GetDataIndexForExtraction();
  if (e.m_Type != ezRenderWorldExtractionEvent::Type::BeginExtraction)
    return;

  EZ_ASSERT_DEBUG(m_pPendingChanges == nullptr, "OnRenderEvent should have been called to consume pending changes");

  EZ_LOCK(m_ExtractionMutex);
  m_pPendingChanges = EZ_NEW(ezFrameAllocator::GetCurrentAllocator(), PendingChanges);
  m_pPendingChanges->m_ChangedMaterials.SetCount(m_ChangedMaterials.GetCount());
  m_pPendingChanges->m_RemovedMaterials = m_RemovedMaterials;

  ezUInt32 uiCurrentIndex = 0;
  for (const ezMaterialResourceHandle& hMaterial : m_ChangedMaterials)
  {
    ExtractedMaterial& extractedMaterial = m_pPendingChanges->m_ChangedMaterials[uiCurrentIndex];
    ezResourceLock<ezMaterialResource> pMaterial(hMaterial, ezResourceAcquireMode::BlockTillLoaded);
    if (pMaterial->m_DirtyFlags.IsSet(ezMaterialResource::DirtyFlags::FlattenHierarchy))
    {
      pMaterial->FlattenHierarchy();
    }

    extractedMaterial.m_hMaterial = hMaterial;
    extractedMaterial.m_hShader = pMaterial->m_mDesc.m_hShader;
    extractedMaterial.m_MaterialId = pMaterial->m_MaterialId;
    extractedMaterial.m_DirtyFlags = pMaterial->m_DirtyFlags;
    for (ezMaterialResource::DirtyFlags::Enum flag : pMaterial->m_DirtyFlags)
    {
      switch (flag)
      {
        case ezMaterialResource::DirtyFlags::Parameter:
          extractedMaterial.m_Parameters = pMaterial->m_mDesc.m_Parameters;
          break;
        case ezMaterialResource::DirtyFlags::Texture2D:
          extractedMaterial.m_Texture2DBindings = pMaterial->m_mDesc.m_Texture2DBindings;
          break;
        case ezMaterialResource::DirtyFlags::TextureCube:
          extractedMaterial.m_TextureCubeBindings = pMaterial->m_mDesc.m_TextureCubeBindings;
          break;
        case ezMaterialResource::DirtyFlags::PermutationVar:
          extractedMaterial.m_PermutationVars = pMaterial->m_mDesc.m_PermutationVars;
          break;
        default:
          break;
      }
    }
    pMaterial->m_DirtyFlags.Clear();
    uiCurrentIndex++;
  }

  m_ChangedMaterials.Clear();
  m_RemovedMaterials.Clear();
}

void ezMaterialManager::OnRenderEvent(const ezRenderWorldRenderEvent& e)
{
  // ezUInt32 uiDataIndex = ezRenderWorld::GetDataIndexForRendering();
  if (e.m_Type != ezRenderWorldRenderEvent::Type::BeginRender)
    return;

  if (m_pPendingChanges == nullptr)
    return;

  EZ_LOCK(m_MaterialShaderMutex);
  // Execute deletions first
  for (const MaterialRegistration& materialDeletion : m_pPendingChanges->m_RemovedMaterials)
  {
    m_Materials.Remove(materialDeletion.m_pMaterial);

    auto itShader = m_MaterialShaders.Find(materialDeletion.m_hShader);
    EZ_ASSERT_DEBUG(itShader.IsValid(), "Material shader must exist if dirty");
    MaterialShaderConstants& ms = *itShader.Value();
    ms.RemoveMaterial(materialDeletion.m_MaterialId);
  }

  // Execute updates and additions
  for (const auto& extractedMaterial : m_pPendingChanges->m_ChangedMaterials)
  {
    ezResourceLock<ezMaterialResource> pMaterial(extractedMaterial.m_hMaterial, ezResourceAcquireMode::PointerOnly);
    auto it = m_Materials.FindOrAdd(pMaterial.GetPointer());
    MaterialData& md = it.Value();
    for (ezMaterialResource::DirtyFlags::Enum flag : extractedMaterial.m_DirtyFlags)
    {
      switch (flag)
      {
        case ezMaterialResource::DirtyFlags::Parameter:
          md.m_Parameters = extractedMaterial.m_Parameters;
          break;
        case ezMaterialResource::DirtyFlags::Texture2D:
          md.m_Texture2DBindings = extractedMaterial.m_Texture2DBindings;
          break;
        case ezMaterialResource::DirtyFlags::TextureCube:
          md.m_TextureCubeBindings = extractedMaterial.m_TextureCubeBindings;
          break;
        case ezMaterialResource::DirtyFlags::PermutationVar:
          md.m_PermutationVars = extractedMaterial.m_PermutationVars;
          break;
        case ezMaterialResource::DirtyFlags::ShaderAndId:
          md.m_MaterialId = extractedMaterial.m_MaterialId;
          md.m_hShader = extractedMaterial.m_hShader;
          break;
        default:
          break;
      }
    }

    ezMaterialResource::ezMaterialId mid = md.m_MaterialId;
    auto it2 = m_MaterialShaders.Find(md.m_hShader);
    EZ_ASSERT_DEBUG(it.IsValid(), "Material shader must exist if dirty");
    MaterialShaderConstants& ms = *it2.Value();
    ms.MarkDirty(mid);
  }

  // Update constant buffers or delete if empty.
  for (auto it = m_MaterialShaders.GetIterator(); it.IsValid();)
  {
    if (it.Value()->IsEmpty())
    {
      it = m_MaterialShaders.Remove(it);
    }
    else
    {
      it.Value()->UpdateConstantBuffers();
      ++it;
    }
  }
  m_pPendingChanges.Clear();
}

ezMaterialManager::MaterialShaderConstants& ezMaterialManager::GetShaderConstants(ezShaderResourceHandle hShader)
{
  auto it = m_MaterialShaders.FindOrAdd(hShader);
  if (it.Value() == nullptr)
  {
    it.Value() = EZ_DEFAULT_NEW(MaterialShaderConstants, hShader, this);
  }
  return *it.Value();
}

/////////////////////////////////////////////////
// MaterialShaderConstants
ezMaterialManager::MaterialShaderConstants::MaterialShaderConstants(ezShaderResourceHandle hShader, ezMaterialManager* pParent)
  : m_hShader(hShader)
  , m_pParent(pParent)
{
  ezResourceLock<ezShaderResource> pShader(m_hShader, ezResourceAcquireMode::PointerOnly);
  pShader->m_ResourceEvents.AddEventHandler(ezMakeDelegate(&ezMaterialManager::MaterialShaderConstants::OnResourceEvent, this), m_ShaderResourceEventSubscriber);
  OnShaderChanged(pShader.GetPointerNonConst());
}

ezMaterialManager::MaterialShaderConstants::~MaterialShaderConstants()
{
  DestroyConstantBuffers();
}

ezMaterialResource::ezMaterialId ezMaterialManager::MaterialShaderConstants::AddMaterial(ezMaterialResourceHandle hMaterial)
{
  return m_Materials.Insert(hMaterial);
}

void ezMaterialManager::MaterialShaderConstants::RemoveMaterial(ezMaterialResource::ezMaterialId id)
{
  m_Materials.Remove(id);
  // We ignore the state of m_ConstantBuffers as the next added material will reuse it.
}

void ezMaterialManager::MaterialShaderConstants::MarkDirty(ezMaterialResource::ezMaterialId id)
{
  m_DirtyMaterials.Insert(id);
}

void ezMaterialManager::MaterialShaderConstants::UpdateConstantBuffers()
{
  bool bLayoutChanged = false;

  if (m_bShaderDirty)
  {
    // Rebuild mapping
    m_bShaderDirty = false;
    ezResourceLock<ezShaderResource> pShader(m_hShader, ezResourceAcquireMode::BlockTillLoaded);
    ezSharedPtr<ezShaderConstantBufferLayout> pNewLayout = pShader->GetConstantBufferLayout();
    bLayoutChanged = m_pLayout == nullptr || *m_pLayout != *pNewLayout;
    if (bLayoutChanged)
    {
      m_pLayout = pNewLayout;
      m_ParameterNameToLayoutIndex.Clear();

      // Build map
      for (int i = 0; i < m_pLayout->m_Constants.GetCount(); ++i)
      {
        m_ParameterNameToLayoutIndex.Insert(m_pLayout->m_Constants[i].m_sName, i);
      }
      DestroyConstantBuffers();
    }
  }

  {
    // Resize data array
    m_ConstantBufferData.SetCount(m_pLayout->m_uiTotalSize * m_Materials.GetCapacity());
    m_ConstantBuffers.SetCount(m_Materials.GetCapacity());
  }

  if (!bLayoutChanged && m_DirtyMaterials.IsEmpty())
    return;

  ezGALCommandEncoder* pEncoder = ezGALDevice::GetDefaultDevice()->BeginCommands("UpdateMaterials");
  if (bLayoutChanged)
  {
    m_DirtyMaterials.Clear();
    for (auto it = m_Materials.GetIterator(); it.IsValid(); ++it)
    {
      ezMaterialResource::ezMaterialId id = it.Id();
      ezMaterialResourceHandle hMaterial = it.Value();
      UpdateMaterial(id, hMaterial, pEncoder);
    }
  }
  else
  {
    for (ezMaterialResource::ezMaterialId id : m_DirtyMaterials)
    {
      ezMaterialResourceHandle hMaterial;
      if (m_Materials.TryGetValue(id, hMaterial))
      {
        UpdateMaterial(id, hMaterial, pEncoder);
      }
    }
    m_DirtyMaterials.Clear();
  }
  ezGALDevice::GetDefaultDevice()->EndCommands(pEncoder);
}

void ezMaterialManager::MaterialShaderConstants::DestroyConstantBuffers()
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  for (ezUInt32 i = 0; i < m_ConstantBuffers.GetCount(); ++i)
  {
    ezGALBufferHandle& hBuffer = m_ConstantBuffers[i];
    if (!hBuffer.IsInvalidated())
    {
      pDevice->DestroyBuffer(m_ConstantBuffers[i]);
      hBuffer.Invalidate();
    }
  }
}

void ezMaterialManager::MaterialShaderConstants::OnResourceEvent(const ezResourceEvent& e)
{
  if (e.m_Type == ezResourceEvent::Type::ResourceContentUpdated)
  {
    OnShaderChanged(static_cast<ezShaderResource*>(e.m_pResource));
  }
}

void ezMaterialManager::MaterialShaderConstants::OnShaderChanged(ezShaderResource* pShader)
{
  ezSharedPtr<ezShaderConstantBufferLayout> pNewLayout = pShader->GetConstantBufferLayout();
  const bool bChanged = m_pLayout == nullptr || *m_pLayout != *pNewLayout;
  if (bChanged)
  {
    m_bShaderDirty = true;
  }
}

void ezMaterialManager::MaterialShaderConstants::UpdateMaterial(ezMaterialResource::ezMaterialId id, ezMaterialResourceHandle hMaterial, ezGALCommandEncoder* pEncoder)
{
  ezResourceLock<ezMaterialResource> pMaterial(hMaterial, ezResourceAcquireMode::PointerOnly);
  auto itMaterial = m_pParent->m_Materials.Find(pMaterial.GetPointer());
  if (!itMaterial.IsValid() || itMaterial.Value().m_hShader.IsValid())
  {
    // As materials can be added at any time, we might have entries that were registered between extraction and rendering and will this be first extracted in the next frame. Ignore these.
    return;
  }

  MaterialData& md = itMaterial.Value();
  ezArrayPtr<ezUInt8> data = m_ConstantBufferData.GetArrayPtr().GetSubArray(id.m_InstanceIndex * m_pLayout->m_uiTotalSize, m_pLayout->m_uiTotalSize);
  for (const auto& param : md.m_Parameters)
  {
    auto it = m_ParameterNameToLayoutIndex.Find(param.m_Name);
    if (it.IsValid())
    {
      const ezShaderConstant& constant = m_pLayout->m_Constants[it.Value()];
      if (constant.m_uiOffset + ezShaderConstant::s_TypeSize[constant.m_Type.GetValue()] <= data.GetCount())
      {
        ezUInt8* pDest = &data[constant.m_uiOffset];
        constant.CopyDataFormVariant(pDest, &param.m_Value);
      }
    }
  }
  // Create and update constant buffer
  if (m_ConstantBuffers[id.m_InstanceIndex].IsInvalidated())
  {
    ezGALBufferCreationDescription desc;
    desc.m_uiStructSize = 0;
    desc.m_uiTotalSize = m_pLayout->m_uiTotalSize;
    desc.m_BufferFlags = ezGALBufferUsageFlags::ConstantBuffer;
    desc.m_ResourceAccess.m_bImmutable = false;
    m_ConstantBuffers[id.m_InstanceIndex] = ezGALDevice::GetDefaultDevice()->CreateBuffer(desc);
  }
  // We lazily create m_ConstantBuffers so we need to always update the MaterialData as this could be a constant buffer from a previous material that resided in this slot.
  md.m_ConstantBuffer = m_ConstantBuffers[id.m_InstanceIndex];

  pEncoder->UpdateBuffer(md.m_ConstantBuffer, 0, data, ezGALUpdateMode::AheadOfTime);
}
bool ezMaterialManager::MaterialShaderConstants::IsEmpty() const
{
  return m_Materials.IsEmpty();
}

ezMaterialManager::ExtractedMaterial::ExtractedMaterial()
  : m_Parameters(ezFrameAllocator::GetCurrentAllocator())
  , m_Texture2DBindings(ezFrameAllocator::GetCurrentAllocator())
  , m_TextureCubeBindings(ezFrameAllocator::GetCurrentAllocator())
  , m_PermutationVars(ezFrameAllocator::GetCurrentAllocator())
{
}

ezMaterialManager::PendingChanges::PendingChanges()
  : m_RemovedMaterials(ezFrameAllocator::GetCurrentAllocator())
  , m_ChangedMaterials(ezFrameAllocator::GetCurrentAllocator())
{
}
