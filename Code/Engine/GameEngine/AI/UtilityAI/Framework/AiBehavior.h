#pragma once

#include <GameEngine/GameEngineDLL.h>

class ezAiBehavior;
class ezAiPerceptionManager;
class ezAiPerception;
class ezGameObject;
class ezAiActionQueue;

struct ezAiBehaviorScore
{
  ezAiBehavior* m_pBehavior = nullptr;
  const ezAiPerception* m_pPerception = nullptr;
  float m_fScore = 0.0f;
};

class EZ_GAMEENGINE_DLL ezAiBehavior
{
public:
  ezAiBehavior() = default;
  virtual ~ezAiBehavior() = default;

  virtual void FlagNeededPerceptions(ezAiPerceptionManager& ref_PerceptionManager) = 0;

  virtual ezAiBehaviorScore DetermineBehaviorScore(ezGameObject& owner, const ezAiPerceptionManager& perceptionManager) = 0;

  virtual void ActivateBehavior(ezGameObject& owner, const ezAiPerception* pPerception, ezAiActionQueue& inout_ActionQueue) = 0;
  virtual void DeactivateBehavior(ezGameObject& owner, ezAiActionQueue& inout_ActionQueue) {}

  virtual ezTime GetCooldownDuration() { return ezTime::MakeZero(); }
};
