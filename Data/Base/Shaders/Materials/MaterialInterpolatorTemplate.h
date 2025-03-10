

struct STAGE_TEMPLATE
{
  float4 Position : SV_Position;

#if defined(USE_WORLDPOS)
  float3 WorldPosition : WORLDPOS;
#endif

#if defined(USE_NORMAL)
  float3 Normal : NORMAL;
#endif

#if defined(USE_TANGENT)
  float3 Tangent : TANGENT;
  float3 BiTangent : BITANGENT;
#endif

#if defined(USE_TEXCOORD0)
  float2 TexCoord0 : TEXCOORD0;

#  if defined(USE_TEXCOORD1)
  float2 TexCoord1 : TEXCOORD1;
#  endif
#endif

#if defined(USE_COLOR0)
  float4 Color0 : COLOR0;

#  if defined(USE_COLOR1)
  float4 Color1 : COLOR1;
#  endif
#endif

#if defined(CUSTOM_INTERPOLATOR)
  CUSTOM_INTERPOLATOR
#endif

#if defined(USE_DEBUG_INTERPOLATOR)
  float4 DebugInterpolator : DEBUG_INTERPOLATOR;
#endif

  uint nointerpolation InstanceDataOffset : INSTANCE_DATA_OFFSET;
#if defined(USE_CUSTOM_INSTANCE_DATA)
  uint nointerpolation CustomInstanceDataOffset : CUSTOM_INSTANCE_DATA_OFFSET;
#endif

  uint nointerpolation MaterialDataOffset : MATERIAL_DATA_OFFSET;

#if defined(RENDER_TARGET_ARRAY_INDEX)
  uint RenderTargetArrayIndex : SV_RenderTargetArrayIndex;
#endif

#if defined(PIXEL_SHADER) && defined(TWO_SIDED)
#  if TWO_SIDED == TRUE
#    if defined(VULKAN) || defined(WGSL)
  // uint type is not supported by DXC/SPIR-V for SV_IsFrontFace
  bool FrontFace : SV_IsFrontFace;
#    else
  uint FrontFace : SV_IsFrontFace;
#    endif
#  endif
#endif
};
