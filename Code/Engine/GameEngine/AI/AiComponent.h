#pragma once

#include <Core/World/World.h>

#include <GameEngine/AI/UtilityAI/Framework/AiActionQueue.h>
#include <GameEngine/AI/UtilityAI/Framework/AiBehaviorManager.h>
#include <GameEngine/AI/UtilityAI/Framework/AiPerceptionManager.h>
#include <GameEngine/AI/UtilityAI/Framework/AiSensorManager.h>

using ezAiComponentManager = ezComponentManagerSimple<class ezAiComponent, ezComponentUpdateType::WhenSimulating, ezBlockStorageType::FreeList>;

class EZ_GAMEENGINE_DLL ezAiComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezAiComponent, ezComponent, ezAiComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

protected:
  virtual void OnSimulationStarted() override;
  virtual void OnDeactivated() override;

  //////////////////////////////////////////////////////////////////////////
  // ezAiComponent

  void Update();

public:
  ezAiComponent();
  ~ezAiComponent();

  bool m_bDebugInfo = false;

protected:
  ezAiActionQueue m_ActionQueue;
  ezTime m_LastAiUpdate;

  ezAiSensorManager m_SensorManager;
  ezAiPerceptionManager m_PerceptionManager;
  ezAiBehaviorManager m_BehaviorManager;
  float m_fCurrentBehaviorScore = 0.0f;
};
