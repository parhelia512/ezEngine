#pragma once

#include <Foundation/Math/Vec3.h>
#include <GameEngine/AI/UtilityAI/Framework/AiPerception.h>

class EZ_GAMEENGINE_DLL ezAiPerceptionPOI : public ezAiPerception
{
public:
  ezAiPerceptionPOI() = default;
  ~ezAiPerceptionPOI() = default;

  ezVec3 m_vGlobalPosition = ezVec3::MakeZero();
};


class EZ_GAMEENGINE_DLL ezAiPerceptionWander : public ezAiPerception
{
public:
  ezAiPerceptionWander() = default;
  ~ezAiPerceptionWander() = default;

  ezVec3 m_vGlobalPosition = ezVec3::MakeZero();
};
