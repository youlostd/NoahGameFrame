// Camera.h: interface for the CCamera class.
//
//////////////////////////////////////////////////////////////////////

#ifndef METIN2_CLIENT_ETERLIB_CAMERA_HPP
#define METIN2_CLIENT_ETERLIB_CAMERA_HPP

#pragma once

#include <map>

#include <base/Singleton.hpp>
#include "Ray.h"

const float CAMERA_TARGET_STANDARD = 100.0f;
const float CAMERA_TARGET_FACE = 150.0f;

typedef enum _eCameraState_
{
    CAMERA_STATE_NORMAL,
    CAMERA_STATE_CANTGODOWN,
    CAMERA_STATE_CANTGORIGHT,
    CAMERA_STATE_CANTGOLEFT,
    CAMERA_STATE_SCREEN_BY_BUILDING,
    CAMERA_STATE_SCREEN_BY_BUILDING_AND_TOOCLOSE,
} eCameraState;

class CCamera
{
public:
    CCamera();
    virtual ~CCamera();

    static void SetCameraMaxDistance(float fMax);

    void Lock();
    void Unlock();
    bool IsLock();

    void Wheel(int nWheelLen);
    bool Drag(int nMouseX, int nMouseY, LPPOINT lpReturnPoint);

    bool EndDrag();
    void BeginDrag(int nMouseX, int nMouseY);

    bool IsDraging();

    void SetResistance(float fResistance);

private:
    const CCamera &operator =(const CCamera &); // 지원하지 않음
    CCamera(const CCamera &);                   //지원하지 않음

    // Camera Update
    eCameraState m_eCameraState;
    eCameraState m_eCameraStatePrev;
    float m_fPitchBackup;
    float m_fRollBackup;
    float m_fDistanceBackup;
    float m_fTargetZBackUp;

    Vector3 m_v3EyeBackup;

    uint32_t m_ulNumScreenBuilding;

    // 	protected:

    bool m_isLock;

    // Attributes for view matrix
    Vector3 m_v3Eye;
    Vector3 m_v3Target;
    Vector3 m_v3Up;

    // m_v3View = m_v3Target - m_v3Eye
    Vector3 m_v3View;
    // m_v3Cross = Cross(m_v3Up, m_v3View)
    Vector3 m_v3Cross;

    //ViewMatrixes
    Matrix m_matView;
    Matrix m_matInverseView;
    Matrix m_matBillboard; // Special matrix for billboarding effects

    //추가분
    float m_fPitch;
    float m_fRoll;
    float m_fDistance;

    // 카메라 AI를 위한 Ray 들

    // 카메라를 둘러싼 Ray
    CRay m_kCameraBottomToTerrainRay;
    CRay m_kCameraFrontToTerrainRay;
    CRay m_kCameraBackToTerrainRay;
    CRay m_kCameraLeftToTerrainRay;
    CRay m_kCameraRightToTerrainRay;

    CRay m_kTargetToCameraBottomRay;

    CRay m_ViewRay;
    CRay m_kLeftObjectCollisionRay;
    CRay m_kTopObjectCollisionRay;
    CRay m_kRightObjectCollisionRay;
    CRay m_kBottomObjectCollisionRay;

    float m_fTerrainCollisionRadius;
    float m_fObjectCollisionRadius;

    // 	protected:
    float m_fTarget_;

    float m_fEyeGroundHeightRatio;
    float m_fTargetHeightLimitRatio;
    float m_fPitchSum;
    float m_fRollSum;

    long m_lMousePosX;
    long m_lMousePosY;

    bool m_bDrag;

    //	protected:
    // 물리
    Vector3 m_v3AngularAcceleration;
    Vector3 m_v3AngularVelocity;

    float m_fResistance;
public:
    //////////////////////////////////////////////////////////////////////////
    // 물리
    //////////////////////////////////////////////////////////////////////////
    void SetAngularAcceleration(Vector3 v3AngularAcceleration)
    {
        m_v3AngularAcceleration = v3AngularAcceleration;
    }

    //////////////////////////////////////////////////////////////////////////
    // AI
    //////////////////////////////////////////////////////////////////////////
    void SetTerrainCollisionRadius(float fTerrainCollisionRadius)
    {
        m_fTerrainCollisionRadius = fTerrainCollisionRadius;
    }

    void SetObjectCollisionRadius(float fObjectCollisionRadius)
    {
        m_fObjectCollisionRadius = fObjectCollisionRadius;
    }

    CRay &GetViewRay()
    {
        return m_ViewRay;
    }

    CRay &GetLeftObjectCollisionRay()
    {
        return m_kLeftObjectCollisionRay;
    }

    CRay &GetRightObjectCollisionRay()
    {
        return m_kRightObjectCollisionRay;
    }

    CRay &GetTopObjectCollisionRay()
    {
        return m_kTopObjectCollisionRay;
    }

    CRay &GetBottomObjectCollisionRay()
    {
        return m_kBottomObjectCollisionRay;
    }

    //////////////////////////////////////////////////////////////////////////
    // Update
    //////////////////////////////////////////////////////////////////////////
    void Update();

    eCameraState GetCameraState()
    {
        return m_eCameraState;
    }

    void SetCameraState(eCameraState eNewCameraState);
    void IncreaseNumSrcreenBuilding();
    void ResetNumScreenBuilding();

    uint32_t &GetNumScreenBuilding()
    {
        return m_ulNumScreenBuilding;
    }

    const float &GetPitchBackUp()
    {
        return m_fPitchBackup;
    }

    const float &GetRollBackUp()
    {
        return m_fRollBackup;
    }

    const float &GetDistanceBackUp()
    {
        return m_fDistanceBackup;
    }

    //////////////////////////////////////////////////////////////////////////
    // properties
    //////////////////////////////////////////////////////////////////////////

    const Vector3 &GetEye() const
    {
        return m_v3Eye;
    }

    const Vector3 &GetTarget() const
    {
        return m_v3Target;
    }

    const Vector3 &GetUp() const
    {
        return m_v3Up;
    }

    const Vector3 &GetView() const
    {
        return m_v3View;
    }

    const Vector3 &GetCross() const
    {
        return m_v3Cross;
    }

    const Matrix &GetViewMatrix() const
    {
        return m_matView;
    }

    const Matrix &GetInverseViewMatrix() const
    {
        return m_matInverseView;
    }

    const Matrix &GetBillboardMatrix() const
    {
        return m_matBillboard;
    }

    void SetViewParams(const Vector3 &v3Eye, const Vector3 &v3Target, const Vector3 &v3Up);

    void SetEye(const Vector3 &v3Eye);
    void SetTarget(const Vector3 &v3Target);
    void SetUp(const Vector3 &v3Up);

    float GetPitch() const
    {
        return m_fPitch;
    }

    float GetRoll() const
    {
        return m_fRoll;
    }

    float GetDistance() const
    {
        return m_fDistance;
    }

    void Pitch(const float fPitchDelta); //돌아가는 각도를 넣는다.
    void Roll(const float fRollDelta);
    void SetDistance(const float fdistance);

    //////////////////////////////////////////////////////////////////////////
    // camera movement
    //////////////////////////////////////////////////////////////////////////

    // 말그대로 이동... 카메라 위치와 타겟 위치가 모두 달라진다.
    void Move(const Vector3 &v3Displacement);
    // 줌.. 카메라 위치만 이동.. 타겟 위치는 고정...
    void Zoom(float fRatio);

    // 뷰 방향으로 이동.. 타겟위치가 달라지므로 줌과는 다르다...
    void MoveAlongView(float fDistance);
    // 카메라 옆 방향으로 이동..
    void MoveAlongCross(float fDistance);
    // 카메라 업벡터 방향으로 이동...
    void MoveAlongUp(float fDistance);

    // 카메라 옆 방향으로 이동... MoveAlongCross과 동일..
    void MoveLateral(float fDistance);
    // 뷰 방향의 Z 성분을 무시한 XY평면 방향으로 이동..
    void MoveFront(float fDistance);
    // Z방향(연직 방향)으로 이동...
    void MoveVertical(float fDistance);

    //	//카메라 위치는 고정시키고 머리만 든다. 타겟이 달라지겠죠?
    //	//회전각을 라디안이 아닌 "도(Degree)"로 넣는다.
    //	void RotateUpper(float fDegree);

    // 타겟 중심으로 돈다. Eterlib의 SetAroundCamera의 기능과 유사...
    // fPitchDegree는 수평(0도)로부터 아랫쪽으로 꺽어지는 각도...
    // fRollDegree는 타겟 중심으로 시계방향으로 도는 각도...
    void RotateEyeAroundTarget(float fPitchDegree, float fRollDegree);

    // 도는 중심점을 따로 지정 그 점을 중심으로 돈다. 타겟 점도 달라지겠죠?
    void RotateEyeAroundPoint(const Vector3 &v3Point, float fPitchDegree, float fRollDegree);

protected:
    void SetViewMatrix();
    void CalculateRoll();

public:
    float GetTargetHeight();
    void SetTargetHeight(float fTarget);

    bool isTerrainCollisionEnable()
    {
        return m_bProcessTerrainCollision;
    }

    void SetTerrainCollision(bool bEnable)
    {
        m_bProcessTerrainCollision = bEnable;
    }

private:
    void ProcessTerrainCollision();
    void ProcessBuildingCollision();

private:
    bool m_bProcessTerrainCollision;

    static float CAMERA_MIN_DISTANCE;
    static float CAMERA_MAX_DISTANCE;
};

typedef std::map<size_t, std::unique_ptr<CCamera>> TCameraMap;

class CCameraManager : public CSingleton<CCameraManager>
{
public:
    enum ECameraNum
    {
        NO_CURRENT_CAMERA,
        DEFAULT_PERSPECTIVE_CAMERA,
        DEFAULT_ORTHO_CAMERA,
        ILLUSTRATION_CAMERA,
        SHOPDECO_CAMERA,
        DEFAULT_WATER_REFLECTION_CAM,
        CAMERA_MAX
    };

    CCameraManager();
    virtual ~CCameraManager();

    bool AddCamera(uint32_t ucCameraNum);
    bool RemoveCamera(uint32_t ucCameraNum);

    CCamera *GetCurrentCamera();
    void SetCurrentCamera(uint32_t ucCameraNum);
    void ResetToPreviousCamera();

    bool isCurrentCamera(uint32_t ucCameraNum);

    uint32_t GetCurrentCameraNum();

    bool isTerrainCollisionEnable();
    void SetTerrainCollision(bool bEnable);

private:
    TCameraMap m_CameraMap;
    CCamera *m_pCurrentCamera;
    CCamera *m_pPreviousCamera;
};

#endif
