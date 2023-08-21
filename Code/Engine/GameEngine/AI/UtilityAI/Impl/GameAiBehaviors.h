#pragma once

#include <GameEngine/AI/UtilityAI/Framework/AiBehavior.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiBehaviorGoToPOI : public ezAiBehavior
{
public:
  ezAiBehaviorGoToPOI();
  ~ezAiBehaviorGoToPOI();

  virtual void FlagNeededPerceptions(ezAiPerceptionManager& ref_PerceptionManager) override;

  virtual ezAiBehaviorScore DetermineBehaviorScore(ezGameObject& owner, const ezAiPerceptionManager& perceptionManager) override;
  virtual void ActivateBehavior(ezGameObject& owner, const ezAiPerception* pPerception, ezAiActionQueue& inout_ActionQueue) override;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiBehaviorWander : public ezAiBehavior
{
public:
  ezAiBehaviorWander();
  ~ezAiBehaviorWander();

  virtual void FlagNeededPerceptions(ezAiPerceptionManager& ref_PerceptionManager) override;

  virtual ezAiBehaviorScore DetermineBehaviorScore(ezGameObject& owner, const ezAiPerceptionManager& perceptionManager) override;
  virtual void ActivateBehavior(ezGameObject& owner, const ezAiPerception* pPerception, ezAiActionQueue& inout_ActionQueue) override;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiBehaviorShoot : public ezAiBehavior
{
public:
  ezAiBehaviorShoot();
  ~ezAiBehaviorShoot();

  virtual void FlagNeededPerceptions(ezAiPerceptionManager& ref_PerceptionManager) override;

  virtual ezAiBehaviorScore DetermineBehaviorScore(ezGameObject& owner, const ezAiPerceptionManager& perceptionManager) override;
  virtual void ActivateBehavior(ezGameObject& owner, const ezAiPerception* pPerception, ezAiActionQueue& inout_ActionQueue) override;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiBehaviorQuip : public ezAiBehavior
{
public:
  ezAiBehaviorQuip();
  ~ezAiBehaviorQuip();

  virtual void FlagNeededPerceptions(ezAiPerceptionManager& ref_PerceptionManager) override;

  virtual ezAiBehaviorScore DetermineBehaviorScore(ezGameObject& owner, const ezAiPerceptionManager& perceptionManager) override;
  virtual void ActivateBehavior(ezGameObject& owner, const ezAiPerception* pPerception, ezAiActionQueue& inout_ActionQueue) override;

  virtual ezTime GetCooldownDuration() override;
};
