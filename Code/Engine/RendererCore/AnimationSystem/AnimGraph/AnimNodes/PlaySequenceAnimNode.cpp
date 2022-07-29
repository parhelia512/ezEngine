#include <RendererCore/RendererCorePCH.h>

#include <Core/World/GameObject.h>
#include <Core/World/World.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes/PlaySequenceAnimNode.h>
#include <RendererCore/AnimationSystem/AnimPoseGenerator.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(ezAnimSectionMode, 1)
  EZ_ENUM_CONSTANTS(ezAnimSectionMode::Once_SingleClip, ezAnimSectionMode::Once_AllClips, ezAnimSectionMode::Loop_SingleClip, ezAnimSectionMode::Loop_AllClips)
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezPlaySequenceAnimNode, 1, ezRTTIDefaultAllocator<ezPlaySequenceAnimNode>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Common", m_State),

    EZ_ACCESSOR_PROPERTY("StartClip", GetStartClip, SetStartClip)->AddAttributes(new ezAssetBrowserAttribute("Animation Clip")),
    EZ_ARRAY_ACCESSOR_PROPERTY("MiddleClips", MiddleClips_GetCount, MiddleClips_GetValue, MiddleClips_SetValue, MiddleClips_Insert, MiddleClips_Remove)->AddAttributes(new ezAssetBrowserAttribute("Animation Clip")),
    EZ_ENUM_MEMBER_PROPERTY("MiddleClipsMode", ezAnimSectionMode, m_MiddleClipsMode),
    EZ_ACCESSOR_PROPERTY("EndClip", GetEndClip, SetEndClip)->AddAttributes(new ezAssetBrowserAttribute("Animation Clip")),

    EZ_MEMBER_PROPERTY("Active", m_ActivePin)->AddAttributes(new ezHiddenAttribute()),
    EZ_MEMBER_PROPERTY("Weights", m_WeightsPin)->AddAttributes(new ezHiddenAttribute()),
    EZ_MEMBER_PROPERTY("Speed", m_SpeedPin)->AddAttributes(new ezHiddenAttribute()),
    EZ_MEMBER_PROPERTY("MiddleClipIndex", m_ClipIndexPin)->AddAttributes(new ezHiddenAttribute()),

    EZ_MEMBER_PROPERTY("LocalPose", m_LocalPosePin)->AddAttributes(new ezHiddenAttribute()),
    EZ_MEMBER_PROPERTY("OnNextClip", m_OnNextClipPin)->AddAttributes(new ezHiddenAttribute()),
    EZ_MEMBER_PROPERTY("PlayingClipIndex", m_PlayingClipIndexPin)->AddAttributes(new ezHiddenAttribute()),
    EZ_MEMBER_PROPERTY("OnFadeOut", m_OnFadeOutPin)->AddAttributes(new ezHiddenAttribute()),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Animation Sampling"),
    new ezColorAttribute(ezColor::RoyalBlue),
    new ezTitleAttribute("Sequence: '{StartClip}' '{MiddleClips[0]}' '{EndClip}'"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezResult ezPlaySequenceAnimNode::SerializeNode(ezStreamWriter& stream) const
{
  stream.WriteVersion(2);

  EZ_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  EZ_SUCCEED_OR_RETURN(m_State.Serialize(stream));
  stream << m_hStartClip;
  EZ_SUCCEED_OR_RETURN(stream.WriteArray(m_hMiddleClips));
  stream << m_hEndClip;

  EZ_SUCCEED_OR_RETURN(m_ActivePin.Serialize(stream));
  EZ_SUCCEED_OR_RETURN(m_SpeedPin.Serialize(stream));
  EZ_SUCCEED_OR_RETURN(m_ClipIndexPin.Serialize(stream));
  EZ_SUCCEED_OR_RETURN(m_WeightsPin.Serialize(stream));
  EZ_SUCCEED_OR_RETURN(m_LocalPosePin.Serialize(stream));
  EZ_SUCCEED_OR_RETURN(m_OnNextClipPin.Serialize(stream));
  EZ_SUCCEED_OR_RETURN(m_PlayingClipIndexPin.Serialize(stream));
  EZ_SUCCEED_OR_RETURN(m_OnFadeOutPin.Serialize(stream));

  stream << m_MiddleClipsMode;

  return EZ_SUCCESS;
}

ezResult ezPlaySequenceAnimNode::DeserializeNode(ezStreamReader& stream)
{
  const ezTypeVersion version = stream.ReadVersion(2);

  EZ_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  EZ_SUCCEED_OR_RETURN(m_State.Deserialize(stream));
  stream >> m_hStartClip;
  EZ_SUCCEED_OR_RETURN(stream.ReadArray(m_hMiddleClips));
  stream >> m_hEndClip;

  EZ_SUCCEED_OR_RETURN(m_ActivePin.Deserialize(stream));
  EZ_SUCCEED_OR_RETURN(m_SpeedPin.Deserialize(stream));
  EZ_SUCCEED_OR_RETURN(m_ClipIndexPin.Deserialize(stream));
  EZ_SUCCEED_OR_RETURN(m_WeightsPin.Deserialize(stream));
  EZ_SUCCEED_OR_RETURN(m_LocalPosePin.Deserialize(stream));
  EZ_SUCCEED_OR_RETURN(m_OnNextClipPin.Deserialize(stream));
  EZ_SUCCEED_OR_RETURN(m_PlayingClipIndexPin.Deserialize(stream));
  EZ_SUCCEED_OR_RETURN(m_OnFadeOutPin.Deserialize(stream));

  // make sure there are no invalid clips in the middle clip array
  for (ezUInt32 i = m_hMiddleClips.GetCount(); i > 0; i--)
  {
    if (!m_hMiddleClips[i - 1].IsValid())
    {
      m_hMiddleClips.RemoveAtAndSwap(i - 1);
    }
  }

  if (version >= 2)
  {
    stream >> m_MiddleClipsMode;
  }

  return EZ_SUCCESS;
}

void ezPlaySequenceAnimNode::SetStartClip(const char* szFile)
{
  ezAnimationClipResourceHandle hResource;

  if (!ezStringUtils::IsNullOrEmpty(szFile))
  {
    hResource = ezResourceManager::LoadResource<ezAnimationClipResource>(szFile);
  }

  m_hStartClip = hResource;
}

const char* ezPlaySequenceAnimNode::GetStartClip() const
{
  if (!m_hStartClip.IsValid())
    return "";

  return m_hStartClip.GetResourceID();
}

ezUInt32 ezPlaySequenceAnimNode::MiddleClips_GetCount() const
{
  return m_hMiddleClips.GetCount();
}

const char* ezPlaySequenceAnimNode::MiddleClips_GetValue(ezUInt32 uiIndex) const
{
  const auto& hMat = m_hMiddleClips[uiIndex];

  if (!hMat.IsValid())
    return "";

  return hMat.GetResourceID();
}

void ezPlaySequenceAnimNode::MiddleClips_SetValue(ezUInt32 uiIndex, const char* value)
{
  if (ezStringUtils::IsNullOrEmpty(value))
    m_hMiddleClips[uiIndex] = ezAnimationClipResourceHandle();
  else
    m_hMiddleClips[uiIndex] = ezResourceManager::LoadResource<ezAnimationClipResource>(value);
}

void ezPlaySequenceAnimNode::MiddleClips_Insert(ezUInt32 uiIndex, const char* value)
{
  ezAnimationClipResourceHandle hMat;

  if (!ezStringUtils::IsNullOrEmpty(value))
    hMat = ezResourceManager::LoadResource<ezAnimationClipResource>(value);

  m_hMiddleClips.Insert(hMat, uiIndex);
}

void ezPlaySequenceAnimNode::MiddleClips_Remove(ezUInt32 uiIndex)
{
  m_hMiddleClips.RemoveAtAndCopy(uiIndex);
}

void ezPlaySequenceAnimNode::SetEndClip(const char* szFile)
{
  ezAnimationClipResourceHandle hResource;

  if (!ezStringUtils::IsNullOrEmpty(szFile))
  {
    hResource = ezResourceManager::LoadResource<ezAnimationClipResource>(szFile);
  }

  m_hEndClip = hResource;
}

const char* ezPlaySequenceAnimNode::GetEndClip() const
{
  if (!m_hEndClip.IsValid())
    return "";

  return m_hEndClip.GetResourceID();
}

void ezPlaySequenceAnimNode::Step(ezAnimGraph& graph, ezTime tDiff, const ezSkeletonResource* pSkeleton, ezGameObject* pTarget)
{
  const bool bActive = m_ActivePin.IsTriggered(graph);

  if (!m_ActivePin.IsConnected() || !m_LocalPosePin.IsConnected() || m_State.WillStateBeOff(bActive) || !m_hStartClip.IsValid() || !m_hEndClip.IsValid())
  {
    m_uiClipToPlay = 0xFF;
    m_uiNextClipToPlay = 0xFF;
    return;
  }

  if (m_Phase == Phase::Off)
  {
    m_Phase = Phase::Start;

    // if we just start playback and middle mode is set to play all clips, always start with clip 0
    // we don't let the user decide anything here
    if (m_MiddleClipsMode == ezAnimSectionMode::Loop_AllClips || m_MiddleClipsMode == ezAnimSectionMode::Once_AllClips)
    {
      m_uiNextClipToPlay = 0;
    }
  }

  if (m_uiNextClipToPlay == 0xFF)
  {
    // if the next clip is supposed to be random, check whether there is user input to specify the clip index
    m_uiNextClipToPlay = static_cast<ezUInt8>(m_ClipIndexPin.GetNumber(graph, 0xFF));
  }

  if (m_uiNextClipToPlay == 0xFF)
  {
    if (!m_hMiddleClips.IsEmpty())
    {
      // if the next clip to play is still set to random, draw a random number
      m_uiNextClipToPlay = static_cast<ezUInt8>(pTarget->GetWorld()->GetRandomNumberGenerator().UIntInRange(m_hMiddleClips.GetCount()));
    }
    else
    {
      m_uiNextClipToPlay = 0;
    }
  }
  else if (m_uiNextClipToPlay >= m_hMiddleClips.GetCount())
  {
    // if the next clip to play is just outside the valid range, then we have reached the end of the playback
    if (m_MiddleClipsMode == ezAnimSectionMode::Loop_AllClips)
    {
      m_uiNextClipToPlay = 0;
    }
  }

  const bool bWasLooped = m_State.m_bLoop;
  EZ_SCOPE_EXIT(m_State.m_bLoop = bWasLooped);

  float fPrevPlaybackPos = m_State.GetNormalizedPlaybackPosition();

  m_State.m_fPlaybackSpeedFactor = static_cast<float>(m_SpeedPin.GetNumber(graph, 1.0f));
  m_State.m_DurationOfQueued.SetZero();
  m_State.m_bTriggerActive = m_State.m_bImmediateFadeOut ? bActive : true;

  ezAnimationClipResourceHandle hCurrentClip;
  ezAnimPoseGeneratorCommandID inputCmd = 0xFFFFFFFF;

  const bool bNextMiddleClipValid = m_uiNextClipToPlay < m_hMiddleClips.GetCount();

  if (m_Phase == Phase::Start)
  {
    // the middle clip can be skipped entirely
    ezAnimationClipResourceHandle hMiddleClip = bNextMiddleClipValid ? m_hMiddleClips[m_uiNextClipToPlay] : m_hEndClip;

    ezResourceLock<ezAnimationClipResource> pClipStart(m_hStartClip, ezResourceAcquireMode::BlockTillLoaded);

    m_State.m_bLoop = true;
    m_State.m_Duration = pClipStart->GetDescriptor().GetDuration();

    if (m_State.GetCurrentState() == ezAnimState::State::Running)
    {
      ezResourceLock<ezAnimationClipResource> pClipMiddle(hMiddleClip, ezResourceAcquireMode::BlockTillLoaded);

      m_State.m_DurationOfQueued = pClipMiddle->GetDescriptor().GetDuration();
    }

    m_State.UpdateState(tDiff);

    if (m_State.HasTransitioned())
    {
      // guarantee that all animation events from the just finished first clip get evaluated and sent
      {
        auto& cmdE = graph.GetPoseGenerator().AllocCommandSampleEventTrack();
        cmdE.m_hAnimationClip = m_hStartClip;
        cmdE.m_EventSampling = ezAnimPoseEventTrackSampleMode::OnlyBetween;
        cmdE.m_fPreviousNormalizedSamplePos = fPrevPlaybackPos;
        cmdE.m_fNormalizedSamplePos = 1.1f;
        inputCmd = cmdE.GetCommandID();
      }

      m_Phase = bNextMiddleClipValid ? Phase::Middle : Phase::End;
      hCurrentClip = hMiddleClip;
      fPrevPlaybackPos = 0.0f;

      m_uiClipToPlay = m_uiNextClipToPlay;

      if (m_MiddleClipsMode == ezAnimSectionMode::Once_SingleClip)
      {
        m_uiNextClipToPlay = m_hMiddleClips.GetCount(); // one past the end -> finish
      }
      else if (m_MiddleClipsMode == ezAnimSectionMode::Once_AllClips || m_MiddleClipsMode == ezAnimSectionMode::Loop_AllClips)
      {
        EZ_ASSERT_DEBUG(m_uiNextClipToPlay == 0, "");
        m_uiClipToPlay = 0;
        m_uiNextClipToPlay = 1;
      }
      else
      {
        m_uiNextClipToPlay = 0xFF;
      }

      m_OnNextClipPin.SetTriggered(graph, true);
    }
    else
    {
      hCurrentClip = m_hStartClip;
    }
  }
  else if (m_Phase == Phase::Middle)
  {
    ezAnimationClipResourceHandle hMiddleClip1 = m_hMiddleClips[m_uiClipToPlay];
    ezAnimationClipResourceHandle hMiddleClip2 = (bNextMiddleClipValid && bActive) ? m_hMiddleClips[m_uiNextClipToPlay] : m_hEndClip;

    ezResourceLock<ezAnimationClipResource> pClipMiddle1(hMiddleClip1, ezResourceAcquireMode::BlockTillLoaded);
    ezResourceLock<ezAnimationClipResource> pClipMiddle2(hMiddleClip2, ezResourceAcquireMode::BlockTillLoaded);

    m_State.m_bLoop = true;
    m_State.m_Duration = pClipMiddle1->GetDescriptor().GetDuration();
    m_State.m_DurationOfQueued = pClipMiddle2->GetDescriptor().GetDuration();

    m_State.UpdateState(tDiff);

    hCurrentClip = hMiddleClip1;

    if (m_State.HasTransitioned())
    {
      if (!bActive)
      {
        m_Phase = Phase::End;
      }
      else
      {
        m_Phase = bNextMiddleClipValid ? Phase::Middle : Phase::End;
      }

      // guarantee that all animation events from the just finished first clip get evaluated and sent
      {
        auto& cmdE = graph.GetPoseGenerator().AllocCommandSampleEventTrack();
        cmdE.m_hAnimationClip = m_hMiddleClips[m_uiClipToPlay];
        cmdE.m_EventSampling = ezAnimPoseEventTrackSampleMode::OnlyBetween;
        cmdE.m_fPreviousNormalizedSamplePos = fPrevPlaybackPos;
        cmdE.m_fNormalizedSamplePos = 1.1f;
        inputCmd = cmdE.GetCommandID();
      }

      fPrevPlaybackPos = 0.0f;
      hCurrentClip = hMiddleClip2;

      m_uiClipToPlay = m_uiNextClipToPlay;

      if (m_MiddleClipsMode == ezAnimSectionMode::Once_AllClips || m_MiddleClipsMode == ezAnimSectionMode::Loop_AllClips)
      {
        ++m_uiNextClipToPlay;
      }
      else
      {
        m_uiNextClipToPlay = 0xFF;
      }

      m_OnNextClipPin.SetTriggered(graph, true);
    }
  }
  else if (m_Phase == Phase::End)
  {
    hCurrentClip = m_hEndClip;

    ezResourceLock<ezAnimationClipResource> pClipEnd(hCurrentClip, ezResourceAcquireMode::BlockTillLoaded);

    m_State.m_bTriggerActive = bActive;
    m_State.m_bLoop = false;
    m_State.m_Duration = pClipEnd->GetDescriptor().GetDuration();

    m_State.UpdateState(tDiff);

    if (m_State.GetCurrentState() == ezAnimState::State::StartedRampDown)
    {
      m_OnFadeOutPin.SetTriggered(graph, true);
    }
    else if (m_State.GetNormalizedPlaybackPosition() >= 1.0f && bWasLooped && bActive)
    {
      m_Phase = Phase::Off;
      m_uiClipToPlay = 0xFF;
      m_uiNextClipToPlay = 0xFF;
      m_State.Reset();
      return; // TODO: this will produce one frame where no animation is applied in between loops
    }
  }

  if (m_State.GetWeight() <= 0.0f || !hCurrentClip.IsValid())
  {
    m_Phase = Phase::Off;
    m_uiClipToPlay = 0xFF;
    m_uiNextClipToPlay = 0xFF;
    m_State.Reset();
    return;
  }

  ezAnimGraphPinDataLocalTransforms* pOutputTransform = graph.AddPinDataLocalTransforms();

  void* pThis = this;
  auto& cmd = graph.GetPoseGenerator().AllocCommandSampleTrack(ezHashingUtils::xxHash32(&pThis, sizeof(pThis)));
  cmd.m_hAnimationClip = hCurrentClip;
  cmd.m_fNormalizedSamplePos = m_State.GetNormalizedPlaybackPosition();
  cmd.m_fPreviousNormalizedSamplePos = fPrevPlaybackPos;
  cmd.m_EventSampling = ezAnimPoseEventTrackSampleMode::OnlyBetween; // if there is a loop or transition, we handle that manually

  if (inputCmd != 0xFFFFFFFF)
  {
    cmd.m_Inputs.PushBack(inputCmd);
  }

  switch (m_Phase)
  {
    case ezPlaySequenceAnimNode::Phase::Start:
      m_PlayingClipIndexPin.SetNumber(graph, -1);
      break;
    case ezPlaySequenceAnimNode::Phase::Middle:
      m_PlayingClipIndexPin.SetNumber(graph, m_uiClipToPlay);
      break;
    case ezPlaySequenceAnimNode::Phase::End:
      m_PlayingClipIndexPin.SetNumber(graph, -2);
      break;

      EZ_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  {
    ezAnimGraphPinDataLocalTransforms* pLocalTransforms = graph.AddPinDataLocalTransforms();

    pLocalTransforms->m_fOverallWeight = m_State.GetWeight();
    pLocalTransforms->m_pWeights = m_WeightsPin.GetWeights(graph);

    if (m_State.m_bApplyRootMotion)
    {
      pLocalTransforms->m_bUseRootMotion = true;

      ezResourceLock<ezAnimationClipResource> pClip(hCurrentClip, ezResourceAcquireMode::BlockTillLoaded);

      pLocalTransforms->m_vRootMotion = pClip->GetDescriptor().m_vConstantRootMotion * tDiff.AsFloatInSeconds() * m_State.m_fPlaybackSpeed;
    }

    pLocalTransforms->m_CommandID = cmd.GetCommandID();

    m_LocalPosePin.SetPose(graph, pLocalTransforms);
  }
}
