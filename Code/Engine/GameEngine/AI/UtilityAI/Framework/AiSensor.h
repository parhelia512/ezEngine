#pragma once

#include <GameEngine/GameEngineDLL.h>

class ezGameObject;

class EZ_GAMEENGINE_DLL ezAiSensor
{
public:
  ezAiSensor() = default;
  virtual ~ezAiSensor() = default;

  virtual void UpdateSensor(ezGameObject* pOwner) = 0;
};
