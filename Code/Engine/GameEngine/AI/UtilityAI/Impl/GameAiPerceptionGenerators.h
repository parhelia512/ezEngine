#pragma once

#include <GameEngine/AI/UtilityAI/Framework/AiPerceptionGenerator.h>
#include <GameEngine/AI/UtilityAI/Impl/GameAiPerceptions.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiPerceptionGenPOI : public ezAiPerceptionGenerator
{
public:
  ezAiPerceptionGenPOI();
  ~ezAiPerceptionGenPOI();

  virtual ezStringView GetPerceptionType() override { return "ezAiPerceptionPOI"_ezsv; }
  virtual void UpdatePerceptions(ezGameObject* pOwner, const ezAiSensorManager& ref_SensorManager) override;
  virtual bool HasPerceptions() const override;
  virtual void GetPerceptions(ezDynamicArray<const ezAiPerception*>& out_Perceptions) const override;
  virtual void FlagNeededSensors(ezAiSensorManager& ref_SensorManager) override;

private:
  ezDynamicArray<ezAiPerceptionPOI> m_Perceptions;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiPerceptionGenWander : public ezAiPerceptionGenerator
{
public:
  ezAiPerceptionGenWander();
  ~ezAiPerceptionGenWander();

  virtual ezStringView GetPerceptionType() override { return "ezAiPerceptionWander"_ezsv; }
  virtual void UpdatePerceptions(ezGameObject* pOwner, const ezAiSensorManager& ref_SensorManager) override;
  virtual bool HasPerceptions() const override;
  virtual void GetPerceptions(ezDynamicArray<const ezAiPerception*>& out_Perceptions) const override;
  virtual void FlagNeededSensors(ezAiSensorManager& ref_SensorManager) override;

private:
  ezDynamicArray<ezAiPerceptionWander> m_Perceptions;
};
