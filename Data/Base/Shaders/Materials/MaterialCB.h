#pragma once

#include <Shaders/Common/ObjectConstants.h>

// This define is set by the shader compiler if a shader contains a [MATERIALCONSTANTS]
#if defined(HAS_MATERIAL_CONSTANTS)
// This struct is defined by the shader compiler based on the [MATERIALCONSTANTS] section in the shader.
//struct EZ_SHADER_STRUCT ezMaterialConstants
//{
// ...
//};
StructuredBuffer<ezMaterialConstants> materialData;
#define GetMaterialData(x) materialData[0].x
#endif


