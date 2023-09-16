#include <GameEngine/GameEnginePCH.h>

#include <Core/World/GameObject.h>
#include <Foundation/Profiling/Profiling.h>
#include <GameEngine/AI/UtilityAI/Framework/AiActionQueue.h>
#include <RendererCore/Debug/DebugRenderer.h>

ezAiActionQueue::ezAiActionQueue() = default;

ezAiActionQueue::~ezAiActionQueue()
{
  InterruptAndClear();
}

bool ezAiActionQueue::IsEmpty() const
{
  return m_Queue.IsEmpty();
}

void ezAiActionQueue::InterruptAndClear()
{
  for (auto pCmd : m_Queue)
  {
    pCmd->Destroy();
  }
}

void ezAiActionQueue::CancelCurrentActions(ezGameObject* pOwner)
{
  for (ezUInt32 i = 0; i < m_Queue.GetCount(); ++i)
  {
    m_Queue[i]->Cancel(pOwner);
  }
}

void ezAiActionQueue::QueueAction(ezAiAction* pAction)
{
  m_Queue.PushBack(pAction);
}

void ezAiActionQueue::Execute(ezGameObject* pOwner, ezTime tDiff, ezLogInterface* pLog)
{
  EZ_LOG_BLOCK("AiActionExecution");
  EZ_PROFILE_SCOPE("AiActionExecution");

  while (!m_Queue.IsEmpty())
  {
    ezAiAction* pCmd = m_Queue[0];

    const ezAiActionResult res = pCmd->Execute(pOwner, tDiff, pLog);
    if (res == ezAiActionResult::Succeded)
      return;

    if (res == ezAiActionResult::Failed)
    {
      ezStringBuilder str;
      pCmd->GetDebugDesc(str);
      ezLog::Error("AI cmd failed: {}", str);

      CancelCurrentActions(pOwner);
    }

    pCmd->Destroy();
    m_Queue.RemoveAtAndCopy(0);
  }
}

void ezAiActionQueue::PrintDebugInfo(ezGameObject* pOwner)
{
  ezStringBuilder str;

  if (m_Queue.IsEmpty())
  {
    str = "<AI action queue empty>";
    ezDebugRenderer::DrawInfoText(pOwner->GetWorld(), ezDebugTextPlacement::BottomRight, "AI", str, ezColor::Orange);
  }
  else
  {
    m_Queue[0]->GetDebugDesc(str);
    ezDebugRenderer::DrawInfoText(pOwner->GetWorld(), ezDebugTextPlacement::BottomRight, "AI", str);
  }
}
