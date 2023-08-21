#include <GameEngine/GameEnginePCH.h>

#include "../SensorComponent.h"
#include <Core/Interfaces/PhysicsWorldModule.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <GameEngine/AI/AiComponent.h>
#include <GameEngine/AI/UtilityAI/Impl/GameAiBehaviors.h>
#include <GameEngine/AI/UtilityAI/Impl/GameAiPerceptionGenerators.h>
#include <GameEngine/AI/UtilityAI/Impl/GameAiSensors.h>
#include <RendererCore/Debug/DebugRenderer.h>

// clang-format off
//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_COMPONENT_TYPE(ezAiComponent, 1, ezComponentMode::Dynamic)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("DebugInfo", m_bDebugInfo),
  }
  EZ_END_PROPERTIES;

  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("AI/Components"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_ABSTRACT_COMPONENT_TYPE
// clang-format on

ezAiComponent::ezAiComponent() = default;
ezAiComponent::~ezAiComponent() = default;

void ezAiComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  auto& s = inout_stream.GetStream();

  s << m_bDebugInfo;
}

void ezAiComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const ezUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());
  auto& s = inout_stream.GetStream();

  s >> m_bDebugInfo;
}

void ezAiComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  m_SensorManager.AddSensor("Sensor_See", EZ_DEFAULT_NEW(ezAiSensorSpatial, ezTempHashedString("Sensor_POI")));
  m_PerceptionManager.AddGenerator(EZ_DEFAULT_NEW(ezAiPerceptionGenPOI));
  m_PerceptionManager.AddGenerator(EZ_DEFAULT_NEW(ezAiPerceptionGenWander));
  m_BehaviorManager.AddBehavior(EZ_DEFAULT_NEW(ezAiBehaviorGoToPOI));
  m_BehaviorManager.AddBehavior(EZ_DEFAULT_NEW(ezAiBehaviorWander));
  m_BehaviorManager.AddBehavior(EZ_DEFAULT_NEW(ezAiBehaviorShoot));
  m_BehaviorManager.AddBehavior(EZ_DEFAULT_NEW(ezAiBehaviorQuip));
}

void ezAiComponent::OnDeactivated()
{
  m_ActionQueue.InterruptAndClear();

  SUPER::OnDeactivated();
}

void ezAiComponent::Update()
{
  if (m_ActionQueue.IsEmpty())
  {
    m_fCurrentBehaviorScore = 0.0f;
  }

  if (GetWorld()->GetClock().GetAccumulatedTime() > m_LastAiUpdate + ezTime::Seconds(0.5))
  {
    m_LastAiUpdate = GetWorld()->GetClock().GetAccumulatedTime();

    m_BehaviorManager.DetermineAvailableBehaviors(m_LastAiUpdate);

    m_BehaviorManager.FlagNeededPerceptions(m_PerceptionManager);

    m_PerceptionManager.FlagNeededSensors(m_SensorManager);

    m_SensorManager.UpdateNeededSensors(GetOwner());

    m_PerceptionManager.UpdatePerceptions(GetOwner(), m_SensorManager);

    const ezAiBehaviorScore score = m_BehaviorManager.SelectBehavior(*GetOwner(), m_PerceptionManager);

    if (score.m_fScore > m_fCurrentBehaviorScore)
    {
      m_BehaviorManager.SetActiveBehavior(*GetOwner(), score.m_pBehavior, score.m_pPerception, m_ActionQueue);

      m_fCurrentBehaviorScore = score.m_fScore;
    }
  }

  m_ActionQueue.Execute(GetOwner(), GetWorld()->GetClock().GetTimeDiff(), ezLog::GetThreadLocalLogSystem());

  if (m_bDebugInfo)
  {
    m_ActionQueue.PrintDebugInfo(GetOwner());
  }
}
