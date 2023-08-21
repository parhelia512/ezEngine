#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/UniquePtr.h>
#include <GameEngine/AI/UtilityAI/Framework/AiSensor.h>

class EZ_GAMEENGINE_DLL ezAiSensorManager
{
  EZ_DISALLOW_COPY_AND_ASSIGN(ezAiSensorManager);

public:
  ezAiSensorManager();
  ~ezAiSensorManager();

  void AddSensor(ezStringView sName, ezUniquePtr<ezAiSensor>&& pSensor);

  void FlagAsNeeded(ezStringView sName);

  void UpdateNeededSensors(ezGameObject* pOwner);

  const ezAiSensor* GetSensor(ezStringView sName) const;

private:
  struct SensorInfo
  {
    ezString m_sName;
    ezUniquePtr<ezAiSensor> m_pSensor;
    bool m_bActive = true;
    ezUInt32 m_uiNeededInUpdate = 0;
  };

  ezUInt32 m_uiUpdateCount = 1;
  ezHybridArray<SensorInfo, 2> m_Sensors;
};
