#pragma once

#include <Core/World/Declarations.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Strings/HashedString.h>
#include <GameEngine/AI/UtilityAI/Framework/AiSensor.h>

class EZ_GAMEENGINE_DLL ezAiSensorSpatial : public ezAiSensor
{
public:
  ezAiSensorSpatial(ezTempHashedString sObjectName);
  ~ezAiSensorSpatial();

  virtual void UpdateSensor(ezGameObject* pOwner) override;
  void RetrieveSensations(ezGameObject* pOwner, ezDynamicArray<ezGameObjectHandle>& out_Sensations) const;

  ezTempHashedString m_sObjectName;

private:
  ezGameObjectHandle m_hSensorObject;
};
