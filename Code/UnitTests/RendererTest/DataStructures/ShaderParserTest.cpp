#include <RendererTest/TestClass/SimpleRendererTest.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <RendererCore/Shader/ShaderPermutationResource.h>
#include <RendererCore/ShaderCompiler/ShaderManager.h>
#include <RendererCore/ShaderCompiler/ShaderParser.h>

void CompareLayouts(const ezShaderConstantBufferLayout& layoutA, const ezShaderConstantBufferLayout& layoutB)
{
  if (EZ_TEST_INT(layoutA.m_Constants.GetCount(), layoutB.m_Constants.GetCount()))
  {
    for (ezUInt32 i = 0; i < layoutA.m_Constants.GetCount(); ++i)
    {
      const auto& constantA = layoutA.m_Constants[i];
      const auto& constantB = layoutB.m_Constants[i];

      EZ_TEST_STRING(constantA.m_sName.GetData(), constantB.m_sName.GetData());
      if (constantA.m_Type == ezShaderConstant::Type::Bool || constantB.m_Type == ezShaderConstant::Type::Bool)
      {
        EZ_TEST_BOOL(constantA.m_Type == ezShaderConstant::Type::Bool || constantA.m_Type == ezShaderConstant::Type::Int1);
        EZ_TEST_BOOL(constantB.m_Type == ezShaderConstant::Type::Bool || constantB.m_Type == ezShaderConstant::Type::Int1);
      }
      else
      {
        EZ_TEST_INT(constantA.m_Type.GetValue(), constantB.m_Type.GetValue());
      }
      EZ_TEST_INT(constantA.m_uiArrayElements, constantB.m_uiArrayElements);
      EZ_TEST_INT(constantA.m_uiOffset, constantB.m_uiOffset);
    }
  }
  EZ_TEST_INT(layoutA.m_uiTotalSize, layoutB.m_uiTotalSize);
}


EZ_CREATE_SIMPLE_RENDERER_TEST(DataStructures, ShaderParser)
{
  EZ_TEST_BLOCK(ezTestBlock::Enabled, "First Test")
  {
    ezStringView shaderSection =
      "#include <Shaders/Common/ConstantBufferMacros.h>\n"
      "CONSTANT_BUFFER(ezMaterialConstants, 1)\n"
      "{\n"
      "  COLOR4F(BaseColor);\n"
      "  COLOR4F(EmissiveColor);\n"
      "  FLOAT1(MetallicValue);\n"
      "  FLOAT1(ReflectanceValue);\n"
      "  FLOAT1(RoughnessValue);\n"
      "  FLOAT1(MaskThreshold);\n"
      "  BOOL1(UseBaseTexture);\n"
      "  BOOL1(UseNormalTexture);\n"
      "  BOOL1(UseRoughnessTexture);\n"
      "  BOOL1(UseMetallicTexture);\n"
      "  BOOL1(UseEmissiveTexture);\n"
      "  BOOL1(UseOcclusionTexture);\n"
      "  BOOL1(UseOrmTexture);\n"
      "};\n";

    // Read template
    ezStringBuilder sEzFileContent;
    {
      ezFileReader fileEz;
      if (!EZ_TEST_RESULT(fileEz.Open("RendererTest/Shaders/ShaderParserTestTemplate.ezShader")))
      {
        return;
      }
      sEzFileContent.ReadAll(fileEz);
    }

    // Insert shaderSection
    sEzFileContent.ReplaceFirst("{{SHADER_SECTION}}", shaderSection);
    sEzFileContent.ReplaceFirst("{{SHADER_USAGE}}", "BaseColor.r");

    // Write temp shader
    ezStringBuilder sTempFile = ":imgout/Temp.ezShader";
    {
      ezFileWriter TempFile;
      EZ_TEST_BOOL(TempFile.Open(sTempFile) == EZ_SUCCESS);
      TempFile.WriteBytes(sEzFileContent.GetData(), sEzFileContent.GetElementCount()).IgnoreResult();
      TempFile.Close();
    }

    // Load / compile shader permutation
    auto m_hUVColorShader = ezResourceManager::LoadResource<ezShaderResource>("Temp.ezShader");
    ezResourceLock<ezShaderResource> pShaderResource(m_hUVColorShader, ezResourceAcquireMode::BlockTillLoaded);

    ezHashTable<ezHashedString, ezHashedString> m_PermutationVariables;
    ezShaderPermutationResourceHandle m_hActiveShaderPermutation = ezShaderManager::PreloadSinglePermutation(m_hUVColorShader, m_PermutationVariables, false);

    if (!EZ_TEST_BOOL(m_hActiveShaderPermutation.IsValid()))
      return;

    ezShaderPermutationResource* pShaderPermutation = ezResourceManager::BeginAcquireResource(m_hActiveShaderPermutation, ezResourceAcquireMode::BlockTillLoaded);
    const ezGALShader* pGalShader = ezGALDevice::GetDefaultDevice()->GetShader(pShaderPermutation->GetGALShader());

    ezTempHashedString sConstantBufferName("ezMaterialConstants");
    const ezShaderResourceBinding* pBinding = pGalShader->GetShaderResourceBinding(sConstantBufferName);

    // Compared parsed vs compiled layout
    CompareLayouts(*pShaderResource->GetConstantBufferLayout(), *pBinding->m_pLayout);
    ezResourceManager::EndAcquireResource(pShaderPermutation);
  }
}
