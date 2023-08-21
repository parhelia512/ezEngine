#include <GameEngine/GameEnginePCH.h>

#include <GameEngine/AI/UtilityAI/Framework/AiBehaviorManager.h>

ezAiBehaviorManager::ezAiBehaviorManager() = default;
ezAiBehaviorManager::~ezAiBehaviorManager() = default;

void ezAiBehaviorManager::DetermineAvailableBehaviors(ezTime currentTime)
{
  m_CurrentTime = currentTime;
  ++m_uiUpdateCount;

  for (auto& info : m_Behaviors)
  {
    if (info.m_CooldownUntil > currentTime)
      continue;

    info.m_uiNeededInUpdate = m_uiUpdateCount;
  }
}

void ezAiBehaviorManager::AddBehavior(ezUniquePtr<ezAiBehavior>&& pBehavior)
{
  auto& info = m_Behaviors.ExpandAndGetRef();
  info.m_pBehavior = std::move(pBehavior);
}

void ezAiBehaviorManager::FlagNeededPerceptions(ezAiPerceptionManager& ref_PerceptionManager)
{
  for (auto& info : m_Behaviors)
  {
    if (info.m_uiNeededInUpdate == m_uiUpdateCount)
    {
      info.m_pBehavior->FlagNeededPerceptions(ref_PerceptionManager);
    }
  }
}

ezAiBehaviorScore ezAiBehaviorManager::SelectBehavior(ezGameObject& owner, const ezAiPerceptionManager& perceptionManager)
{
  ezAiBehaviorScore res;

  for (auto& info : m_Behaviors)
  {
    if (info.m_uiNeededInUpdate == m_uiUpdateCount)
    {
      const ezAiBehaviorScore scored = info.m_pBehavior->DetermineBehaviorScore(owner, perceptionManager);

      if (scored.m_fScore > res.m_fScore)
      {
        res = scored;
      }
    }
  }

  return res;
}

void ezAiBehaviorManager::SetActiveBehavior(ezGameObject& owner, ezAiBehavior* pBehavior, const ezAiPerception* pPerception, ezAiActionQueue& inout_ActionQueue)
{
  if (m_pActiveBehavior)
  {
    m_pActiveBehavior->DeactivateBehavior(owner, inout_ActionQueue);
  }

  m_pActiveBehavior = pBehavior;

  if (m_pActiveBehavior)
  {
    m_pActiveBehavior->ActivateBehavior(owner, pPerception, inout_ActionQueue);

    for (auto& info : m_Behaviors)
    {
      if (info.m_pBehavior == m_pActiveBehavior)
      {
        info.m_CooldownUntil = m_CurrentTime + m_pActiveBehavior->GetCooldownDuration();
      }
    }
  }
}
