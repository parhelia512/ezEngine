#pragma once

#include <GameEngine/AI/UtilityAI/Framework/AiAction.h>

class EZ_GAMEENGINE_DLL ezAiActionWait : public ezAiAction
{
  EZ_DECLARE_AICMD(ezAiActionWait);

public:
  ezAiActionWait();
  ~ezAiActionWait();

  virtual void Reset() override;
  virtual void GetDebugDesc(ezStringBuilder& inout_sText) override;
  virtual ezAiActionResult Execute(ezGameObject* pOwner, ezTime tDiff, ezLogInterface* pLog) override;
  virtual void Cancel(ezGameObject* pOwner) override;

  ezTime m_Duration;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiActionLerpRotation : public ezAiAction
{
  EZ_DECLARE_AICMD(ezAiActionLerpRotation);

public:
  ezAiActionLerpRotation();
  ~ezAiActionLerpRotation();

  virtual void Reset() override;
  virtual void GetDebugDesc(ezStringBuilder& inout_sText) override;
  virtual ezAiActionResult Execute(ezGameObject* pOwner, ezTime tDiff, ezLogInterface* pLog) override;
  virtual void Cancel(ezGameObject* pOwner) override;

  ezVec3 m_vTurnAxis = ezVec3::MakeAxisZ();
  ezAngle m_TurnAngle;
  ezAngle m_TurnAnglesPerSec;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiActionLerpPosition : public ezAiAction
{
  EZ_DECLARE_AICMD(ezAiActionLerpPosition);

public:
  ezAiActionLerpPosition();
  ~ezAiActionLerpPosition();

  virtual void Reset() override;
  virtual void GetDebugDesc(ezStringBuilder& inout_sText) override;
  virtual ezAiActionResult Execute(ezGameObject* pOwner, ezTime tDiff, ezLogInterface* pLog) override;
  virtual void Cancel(ezGameObject* pOwner) override;

  float m_fSpeed = 0.0f;
  ezVec3 m_vLocalSpaceSlide = ezVec3::MakeZero();
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiActionLerpRotationTowards : public ezAiAction
{
  EZ_DECLARE_AICMD(ezAiActionLerpRotationTowards);

public:
  ezAiActionLerpRotationTowards();
  ~ezAiActionLerpRotationTowards();

  virtual void Reset() override;
  virtual void GetDebugDesc(ezStringBuilder& inout_sText) override;
  virtual ezAiActionResult Execute(ezGameObject* pOwner, ezTime tDiff, ezLogInterface* pLog) override;
  virtual void Cancel(ezGameObject* pOwner) override;

  ezVec3 m_vTargetPosition = ezVec3::MakeZero();
  ezGameObjectHandle m_hTargetObject;
  ezAngle m_TargetReachedAngle = ezAngle::MakeFromDegree(5);
  ezAngle m_TurnAnglesPerSec;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// class EZ_GAMEENGINE_DLL ezAiActionFollowPath : public ezAiAction
//{
//   EZ_DECLARE_AICMD(ezAiActionFollowPath);
//
// public:
//   ezAiActionFollowPath();
//   ~ezAiActionFollowPath();
//
//   virtual void Reset() override;
//   virtual void GetDebugDesc(ezStringBuilder& inout_sText) override;
//   virtual ezAiActionResult Execute(ezGameObject* pOwner, ezTime tDiff, ezLogInterface* pLog) override;
//   virtual void Cancel(ezGameObject* pOwner) override;
//
//   ezGameObjectHandle m_hPath;
//   float m_fSpeed = 0.0f;
// };

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiActionBlackboardSetEntry : public ezAiAction
{
  EZ_DECLARE_AICMD(ezAiActionBlackboardSetEntry);

public:
  ezAiActionBlackboardSetEntry();
  ~ezAiActionBlackboardSetEntry();

  virtual void Reset() override;
  virtual void GetDebugDesc(ezStringBuilder& inout_sText) override;
  virtual ezAiActionResult Execute(ezGameObject* pOwner, ezTime tDiff, ezLogInterface* pLog) override;
  virtual void Cancel(ezGameObject* pOwner) override;

  bool m_bNoCancel = false;
  ezTempHashedString m_sEntryName;
  ezVariant m_Value;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiActionBlackboardWait : public ezAiAction
{
  EZ_DECLARE_AICMD(ezAiActionBlackboardWait);

public:
  ezAiActionBlackboardWait();
  ~ezAiActionBlackboardWait();

  virtual void Reset() override;
  virtual void GetDebugDesc(ezStringBuilder& inout_sText) override;
  virtual ezAiActionResult Execute(ezGameObject* pOwner, ezTime tDiff, ezLogInterface* pLog) override;
  virtual void Cancel(ezGameObject* pOwner) override;

  ezTempHashedString m_sEntryName;
  ezVariant m_Value;
  bool m_bEquals = true;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiActionBlackboardSetAndWait : public ezAiAction
{
  EZ_DECLARE_AICMD(ezAiActionBlackboardSetAndWait);

public:
  ezAiActionBlackboardSetAndWait();
  ~ezAiActionBlackboardSetAndWait();

  virtual void Reset() override;
  virtual void GetDebugDesc(ezStringBuilder& inout_sText) override;
  virtual ezAiActionResult Execute(ezGameObject* pOwner, ezTime tDiff, ezLogInterface* pLog) override;
  virtual void Cancel(ezGameObject* pOwner) override;

  ezTempHashedString m_sEntryName;
  ezVariant m_SetValue;
  ezVariant m_WaitValue;
  bool m_bEqualsWaitValue = true;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiActionCCMoveTo : public ezAiAction
{
  EZ_DECLARE_AICMD(ezAiActionCCMoveTo);

public:
  ezAiActionCCMoveTo();
  ~ezAiActionCCMoveTo();

  virtual void Reset() override;
  virtual void GetDebugDesc(ezStringBuilder& inout_sText) override;
  virtual ezAiActionResult Execute(ezGameObject* pOwner, ezTime tDiff, ezLogInterface* pLog) override;
  virtual void Cancel(ezGameObject* pOwner) override;

  ezVec3 m_vTargetPosition = ezVec3::MakeZero();
  ezGameObjectHandle m_hTargetObject;
  float m_fSpeed = 0.0f;
  float m_fReachedDistSQR = 1.0f;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiActionSpawn : public ezAiAction
{
  EZ_DECLARE_AICMD(ezAiActionSpawn);

public:
  ezAiActionSpawn();
  ~ezAiActionSpawn();

  virtual void Reset() override;
  virtual void GetDebugDesc(ezStringBuilder& inout_sText) override;
  virtual ezAiActionResult Execute(ezGameObject* pOwner, ezTime tDiff, ezLogInterface* pLog) override;
  virtual void Cancel(ezGameObject* pOwner) override;

  ezTempHashedString m_sChildObjectName;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EZ_GAMEENGINE_DLL ezAiActionQuip : public ezAiAction
{
  EZ_DECLARE_AICMD(ezAiActionQuip);

public:
  ezAiActionQuip();
  ~ezAiActionQuip();

  virtual void Reset() override;
  virtual void GetDebugDesc(ezStringBuilder& inout_sText) override;
  virtual ezAiActionResult Execute(ezGameObject* pOwner, ezTime tDiff, ezLogInterface* pLog) override;
  virtual void Cancel(ezGameObject* pOwner) override;

  ezString m_sLogMsg;
};
