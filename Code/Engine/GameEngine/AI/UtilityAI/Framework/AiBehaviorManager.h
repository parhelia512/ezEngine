#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Types/UniquePtr.h>
#include <GameEngine/AI/UtilityAI/Framework/AiBehavior.h>

class ezGameObject;
class ezAiPerceptionManager;

class EZ_GAMEENGINE_DLL ezAiBehaviorManager
{
  EZ_DISALLOW_COPY_AND_ASSIGN(ezAiBehaviorManager);

public:
  ezAiBehaviorManager();
  ~ezAiBehaviorManager();

  void DetermineAvailableBehaviors(ezTime currentTime);

  void AddBehavior(ezUniquePtr<ezAiBehavior>&& pBehavior);

  void FlagNeededPerceptions(ezAiPerceptionManager& ref_PerceptionManager);

  ezAiBehaviorScore SelectBehavior(ezGameObject& owner, const ezAiPerceptionManager& perceptionManager);

  void SetActiveBehavior(ezGameObject& owner, ezAiBehavior* pBehavior, const ezAiPerception* pPerception, ezAiActionQueue& inout_ActionQueue);

private:
  struct BehaviorInfo
  {
    // bool m_bActive = true;
    ezTime m_CooldownUntil;
    ezUInt32 m_uiNeededInUpdate = 0;

    ezUniquePtr<ezAiBehavior> m_pBehavior;
  };

  ezAiBehavior* m_pActiveBehavior = nullptr;
  ezHybridArray<BehaviorInfo, 12> m_Behaviors;

  ezUInt32 m_uiUpdateCount = 1;
  ezTime m_CurrentTime;
};
