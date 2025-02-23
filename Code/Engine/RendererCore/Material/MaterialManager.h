#pragma once

#include <RendererCore/RenderWorld/RenderWorld.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Memory/FrameAllocator.h>
#include <RendererCore/Material/MaterialResource.h>

class ezGALCommandEncoder;

class EZ_RENDERERCORE_DLL ezMaterialManager
{
  EZ_DECLARE_SINGLETON(ezMaterialManager);

public:
  struct MaterialData
  {
    // m_hShader + m_PermutationVars + RenderContext::m_PermutationVariables = ezShaderPermutationResourceHandle
    ezShaderResourceHandle m_hShader;
    ezMaterialResource::ezMaterialId m_MaterialId;

    ezDynamicArray<ezPermutationVar> m_PermutationVars;

    // Constant buffer + textures = descriptor set
    ezGALBufferHandle m_ConstantBuffer;

    // bindless: textures inlined as indices into constant buffer which turns into structured buffer
    ezDynamicArray<ezMaterialResourceDescriptor::Parameter> m_Parameters; // Builds constant buffer
    ezDynamicArray<ezMaterialResourceDescriptor::Texture2DBinding> m_Texture2DBindings;
    ezDynamicArray<ezMaterialResourceDescriptor::TextureCubeBinding> m_TextureCubeBindings;
  };

public:
  const MaterialData& GetMaterialData(const ezMaterialResource* pMaterial) const;

private:
  EZ_MAKE_SUBSYSTEM_STARTUP_FRIEND(RendererCore, MaterialManager);
  friend class ezMemoryUtils;
  friend class ezMaterialResource;

  class MaterialShaderConstants
  {
  public:
    MaterialShaderConstants(ezShaderResourceHandle hShader, ezMaterialManager* pParent);
    ~MaterialShaderConstants();

    ezMaterialResource::ezMaterialId AddMaterial(ezMaterialResourceHandle hMaterial);
    void RemoveMaterial(ezMaterialResource::ezMaterialId id);
    bool IsEmpty() const;

    void MarkDirty(ezMaterialResource::ezMaterialId id);
    void UpdateConstantBuffers();
    void DestroyConstantBuffers();

  private:
    void OnResourceEvent(const ezResourceEvent& e);
    void OnShaderChanged(ezShaderResource* pShader);
    void UpdateMaterial(ezMaterialResource::ezMaterialId id, ezMaterialResourceHandle hMaterial, ezGALCommandEncoder* pEncoder);

  private:
    // Material data
    ezIdTable<ezMaterialResource::ezMaterialId, ezMaterialResourceHandle> m_Materials;
    ezDynamicArray<ezUInt8> m_ConstantBufferData;
    // Bindless: structured buffer for all materials of this shader
    // OldSchool: Constant buffer for each material
    ezDynamicArray<ezGALBufferHandle> m_ConstantBuffers;

    // Shader data
    bool m_bShaderDirty = true;
    ezSet<ezMaterialResource::ezMaterialId> m_DirtyMaterials;
    ezSharedPtr<ezShaderConstantBufferLayout> m_pLayout;
    ezHashTable<ezHashedString, ezUInt32> m_ParameterNameToLayoutIndex;

    // Static data
    const ezShaderResourceHandle m_hShader;
    ezMaterialManager* m_pParent = nullptr;
    ezEvent<const ezResourceEvent&, ezMutex>::Unsubscriber m_ShaderResourceEventSubscriber;
  };


  struct ExtractedMaterial
  {
    ExtractedMaterial();

    ezMaterialResourceHandle m_hMaterial;
    ezShaderResourceHandle m_hShader;
    ezMaterialResource::ezMaterialId m_MaterialId;
    ezBitflags<ezMaterialResource::DirtyFlags> m_DirtyFlags;
    ezDynamicArray<ezMaterialResourceDescriptor::Parameter> m_Parameters;
    ezDynamicArray<ezMaterialResourceDescriptor::Texture2DBinding> m_Texture2DBindings;
    ezDynamicArray<ezMaterialResourceDescriptor::TextureCubeBinding> m_TextureCubeBindings;
    ezDynamicArray<ezPermutationVar> m_PermutationVars;
  };

  struct MaterialRegistration
  {
    ezMaterialResource* m_pMaterial = nullptr;
    ezShaderResourceHandle m_hShader;
    ezMaterialResource::ezMaterialId m_MaterialId;
  };

  struct PendingChanges
  {
    PendingChanges();

    ezDynamicArray<MaterialRegistration> m_RemovedMaterials;
    ezDynamicArray<ExtractedMaterial> m_ChangedMaterials;
  };

private:
  ezMaterialManager();
  ~ezMaterialManager();

  void MaterialUpdated(ezMaterialResource* pMaterial);
  void MaterialRemoved(ezMaterialResource* pMaterial);
  void MaterialModified(ezMaterialResourceHandle hMaterial);

  void OnExtractionEvent(const ezRenderWorldExtractionEvent& e);
  void OnRenderEvent(const ezRenderWorldRenderEvent& e);

  MaterialShaderConstants& GetShaderConstants(ezShaderResourceHandle hShader);

private:
  // Extract these materials during extraction phase.
  ezMutex m_ExtractionMutex;
  ezDynamicArray<MaterialRegistration> m_RemovedMaterials;
  ezHashSet<ezMaterialResourceHandle> m_ChangedMaterials;

  // Extraction result created by frame allocator
  ezUniquePtr<PendingChanges> m_pPendingChanges;
  
  // Used during material creation, deletion and updates.
  ezMutex m_MaterialShaderMutex;
  ezMap<ezShaderResourceHandle, ezUniquePtr<MaterialShaderConstants>> m_MaterialShaders;

  // Used by render thread only. No need for locks.
  ezMap<const ezMaterialResource*, MaterialData> m_Materials;
};