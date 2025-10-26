#include "StdAfx.h"
#include "GrpBase.h"

#include "../EterBase/StepTimer.h"
#include "Camera.h"
#include "GrpDevice.h"


#include "StateManager.h"

#include "../EterBase/Utils.h"
#include "../EterBase/Timer.h"
#include "MatrixStack.h"


const size_t kNaturalAlignment = 8;

	class AllocatorStub : public bx::AllocatorI
	{
	public:
		AllocatorStub()
		{
		}

		virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line) override
		{
			if (0 == _size)
			{
				if (NULL != _ptr)
				{
					if (kNaturalAlignment >= _align)
					{

						::free(_ptr);
					}
					else
					{
						bx::alignedFree(this, _ptr, _align, _file, _line);
					}
				}

				return NULL;
			}
			else if (NULL == _ptr)
			{
				if (kNaturalAlignment >= _align)
				{

					return ::malloc(_size);
				}

				return bx::alignedAlloc(this, _size, _align, _file, _line);
			}

			if (kNaturalAlignment >= _align)
			{
				return ::realloc(_ptr, _size);
			}

			return bx::alignedRealloc(this, _ptr, _size, _align, _file, _line);
		}

		void checkLeaks();

	protected:
	};

bx::AllocatorI* g_allocator = NULL;
static AllocatorStub* s_allocatorStub = NULL;


void PixelPositionToVector3(const Vector3 &c_rkPPosSrc, Vector3 *pv3Dst)
{
    pv3Dst->x = +c_rkPPosSrc.x;
    pv3Dst->y = -c_rkPPosSrc.y;
    pv3Dst->z = +c_rkPPosSrc.z;
}

void Vector3ToPixelPosition(const Vector3 &c_rv3Src, Vector3 *pv3Dst)
{
    pv3Dst->x = +c_rv3Src.x;
    pv3Dst->y = -c_rv3Src.y;
    pv3Dst->z = +c_rv3Src.z;
}

UINT CGraphicBase::ms_iD3DAdapterInfo = 0;
UINT CGraphicBase::ms_iD3DDevInfo = 0;
UINT CGraphicBase::ms_iD3DModeInfo = 0;
D3D_CDisplayModeAutoDetector CGraphicBase::ms_kD3DDetector;

HWND CGraphicBase::ms_hWnd;
HDC CGraphicBase::ms_hDC;

LPDIRECT3D9 CGraphicBase::ms_lpd3d = NULL;
LPDIRECT3DDEVICE9 CGraphicBase::ms_lpd3dDevice = NULL;
bool CGraphicBase::m_deviceNeedsReset = false;
MatrixStack *CGraphicBase::ms_lpd3dMatStack = NULL;
D3DPRESENT_PARAMETERS CGraphicBase::ms_d3dPresentParameter;
D3DVIEWPORT9 CGraphicBase::ms_Viewport;

HRESULT CGraphicBase::ms_hLastResult = NULL;

int CGraphicBase::ms_iWidth;
int CGraphicBase::ms_iHeight;

uint32_t CGraphicBase::ms_faceCount = 0;

D3DCAPS9 CGraphicBase::ms_d3dCaps;

uint32_t CGraphicBase::ms_dwD3DBehavior = 0;

Matrix CGraphicBase::ms_matIdentity;

Matrix CGraphicBase::ms_matView;
Matrix CGraphicBase::ms_matProj;
Matrix CGraphicBase::ms_matInverseView;
Matrix CGraphicBase::ms_matInverseViewYAxis;

Matrix CGraphicBase::ms_matWorld;
Matrix CGraphicBase::ms_matWorldView;

Matrix CGraphicBase::ms_matScreen0;
Matrix CGraphicBase::ms_matScreen1;
Matrix CGraphicBase::ms_matScreen2;

Vector3 CGraphicBase::ms_vtPickRayOrig;
Vector3 CGraphicBase::ms_vtPickRayDir;

float CGraphicBase::ms_fFieldOfView;
float CGraphicBase::ms_fNearY;
float CGraphicBase::ms_fFarY;
float CGraphicBase::ms_fAspect;

uint64_t CGraphicBase::ms_dwWavingEndTime;
int CGraphicBase::ms_iWavingPower;
uint64_t CGraphicBase::ms_dwFlashingEndTime;
DirectX::SimpleMath::Color CGraphicBase::ms_FlashingColor;

// Terrain picking용 Ray... CCamera 이용하는 버전.. 기존의 Ray와 통합 필요...
CRay CGraphicBase::ms_Ray;
bool CGraphicBase::ms_bSupportDXT = true;
// 2004.11.18.myevan.DynamicVertexBuffer로 교체
/*
std::vector<TIndex>		CGraphicBase::ms_lineIdxVector;
std::vector<TIndex>		CGraphicBase::ms_lineTriIdxVector;
std::vector<TIndex>		CGraphicBase::ms_lineRectIdxVector;
std::vector<TIndex>		CGraphicBase::ms_lineCubeIdxVector;

std::vector<TIndex>		CGraphicBase::ms_fillTriIdxVector;
std::vector<TIndex>		CGraphicBase::ms_fillRectIdxVector;
std::vector<TIndex>		CGraphicBase::ms_fillCubeIdxVector;
*/



VertexBufferHandle CGraphicBase::m_smallPdtVertexBuffer;
VertexBufferHandle CGraphicBase::m_largePdtVertexBuffer;

IndexBufferHandle CGraphicBase::ms_alpd3dDefIB[DEFAULT_IB_NUM];

bool CGraphicBase::IsFastTNL()
{
    if (ms_dwD3DBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING ||
        ms_dwD3DBehavior & D3DCREATE_MIXED_VERTEXPROCESSING)
    {
        if (ms_d3dCaps.VertexShaderVersion > D3DVS_VERSION(1, 0))
            return true;
    }
    return false;
}

bool CGraphicBase::IsTLVertexClipping()
{
    if (ms_d3dCaps.PrimitiveMiscCaps & D3DPMISCCAPS_CLIPTLVERTS)
        return true;

    return false;
}

void CGraphicBase::GetBackBufferSize(UINT *puWidth, UINT *puHeight)
{
    *puWidth = ms_d3dPresentParameter.BackBufferWidth;
    *puHeight = ms_d3dPresentParameter.BackBufferHeight;
}

uint32_t CGraphicBase::GetAvailableTextureMemory()
{
    assert(ms_lpd3dDevice!=NULL && "CGraphicBase::GetAvailableTextureMemory - D3DDevice is EMPTY");

    static uint32_t s_dwNextUpdateTime = 0;
    static uint32_t s_dwTexMemSize = 0; //ms_lpd3dDevice->GetAvailableTextureMem();

    uint32_t dwCurTime = ELTimer_GetMSec();
    if (s_dwNextUpdateTime < dwCurTime)
    {
        s_dwNextUpdateTime = dwCurTime + 5000;
        s_dwTexMemSize = ms_lpd3dDevice->GetAvailableTextureMem();
    }

    return s_dwTexMemSize;
}

const Matrix &CGraphicBase::GetViewMatrix()
{
    return ms_matView;
}

const Matrix &CGraphicBase::GetIdentityMatrix()
{
    return ms_matIdentity;
}

void CGraphicBase::SetEyeCamera(float xEye, float yEye, float zEye,
                                float xCenter, float yCenter, float zCenter,
                                float xUp, float yUp, float zUp)
{
    Vector3 vectorEye(xEye, yEye, zEye);
    Vector3 vectorCenter(xCenter, yCenter, zCenter);
    Vector3 vectorUp(xUp, yUp, zUp);

    //	CCameraManager::Instance().SetCurrentCamera(CCameraManager::DEFAULT_PERSPECTIVE_CAMERA);
    CCameraManager::Instance().GetCurrentCamera()->SetViewParams(vectorEye, vectorCenter, vectorUp);
    UpdateViewMatrix();
}

void CGraphicBase::SetSimpleCamera(float x, float y, float z, float pitch, float roll)
{
    CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
    Vector3 vectorEye(x, y, z);

    pCamera->SetViewParams(Vector3(0.0f, y, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));
    pCamera->RotateEyeAroundTarget(pitch, roll);
    pCamera->Move(vectorEye);

    UpdateViewMatrix();

    // This is levites's virtual(?) code which you should not trust.
    ms_lpd3dDevice->GetTransform(D3DTS_WORLD, (D3DMATRIX*)&ms_matWorld);
    ms_matWorldView = ms_matWorld * ms_matView;
}

void CGraphicBase::SetAroundCamera(float distance, float pitch, float roll, float lookAtZ)
{
    CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
    pCamera->SetViewParams(Vector3(0.0f, -distance, 0.0f), Vector3(0.0f, 0.0f, 0.0f),
                           Vector3(0.0f, 0.0f, 1.0f));
    pCamera->RotateEyeAroundTarget(pitch, roll);
    Vector3 v3Target = pCamera->GetTarget();
    v3Target.z = lookAtZ;
    pCamera->SetTarget(v3Target);
    // 	pCamera->Move(v3Target);

    UpdateViewMatrix();

    // This is levites's virtual(?) code which you should not trust.
    ms_lpd3dDevice->GetTransform(D3DTS_WORLD, (D3DMATRIX*)&ms_matWorld);
    ms_matWorldView = ms_matWorld * ms_matView;
}

void CGraphicBase::SetPositionCamera(float fx, float fy, float fz, float distance, float pitch, float roll)
{
    // I wanna downward this code to the game control level. - [levites]
    if (ms_dwWavingEndTime > DX::StepTimer::instance().GetTotalMillieSeconds())
    {
        if (ms_iWavingPower > 0)
        {
            fx += float(rand() % ms_iWavingPower) / 10.0f;
            fy += float(rand() % ms_iWavingPower) / 10.0f;
            fz += float(rand() % ms_iWavingPower) / 10.0f;
        }
    }

    CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
    if (!pCamera)
        return;

    pCamera->SetViewParams(Vector3(0.0f, -distance, 0.0f), Vector3(0.0f, 0.0f, 0.0f),
                           Vector3(0.0f, 0.0f, 1.0f));
    pitch = fMIN(80.0f, fMAX(-80.0f, pitch));
    //	Tracef("SetPosition Camera : %f, %f\n", pitch, roll);
    pCamera->RotateEyeAroundTarget(pitch, roll);
    pCamera->Move(Vector3(fx, fy, fz));

    UpdateViewMatrix();

    // This is levites's virtual(?) code which you should not trust.
    STATEMANAGER.GetTransform(D3DTS_WORLD, &ms_matWorld);
    ms_matWorldView = ms_matWorld * ms_matView;
}

void CGraphicBase::SetOrtho2D(float hres, float vres, float zres)
{
    //CCameraManager::Instance().SetCurrentCamera(CCameraManager::DEFAULT_ORTHO_CAMERA);
    ms_matProj = Matrix::CreateOrthographicOffCenter(0, hres, vres, 0, 0, zres);
    //UpdatePipeLineMatrix();
    UpdateProjMatrix();
}

void CGraphicBase::SetOrtho3D(float hres, float vres, float zmin, float zmax)
{
    //CCameraManager::Instance().SetCurrentCamera(CCameraManager::DEFAULT_PERSPECTIVE_CAMERA);
    ms_matProj = Matrix::CreateOrthographic(hres, vres, zmin, zmax);
    //UpdatePipeLineMatrix();
    UpdateProjMatrix();
}

void CGraphicBase::SetPerspective(float fov, float aspect, float nearz, float farz)
{
    ms_fFieldOfView = fov;

    //if (ms_d3dPresentParameter.BackBufferWidth>0 && ms_d3dPresentParameter.BackBufferHeight>0)
    //	ms_fAspect = float(ms_d3dPresentParameter.BackBufferWidth)/float(ms_d3dPresentParameter.BackBufferHeight);
    //else
    ms_fAspect = aspect;

    ms_fNearY = nearz;
    ms_fFarY = farz;

    //CCameraManager::Instance().SetCurrentCamera(CCameraManager::DEFAULT_PERSPECTIVE_CAMERA);
    ms_matProj = Matrix::CreatePerspectiveFieldOfView(DirectX::XMConvertToRadians(fov), ms_fAspect, nearz, farz);
    //UpdatePipeLineMatrix();
    UpdateProjMatrix();


}

void CGraphicBase::UpdateProjMatrix()
{
    STATEMANAGER.SetTransform(D3DTS_PROJECTION, &ms_matProj);
}

void CGraphicBase::UpdateViewMatrix()
{
    CCamera *pkCamera = CCameraManager::Instance().GetCurrentCamera();
    if (!pkCamera)
        return;

    ms_matView = pkCamera->GetViewMatrix();
    STATEMANAGER.SetTransform(D3DTS_VIEW, &ms_matView);

    ms_matInverseView = ms_matView.Invert();
    ms_matInverseViewYAxis._11 = ms_matInverseView._11;
    ms_matInverseViewYAxis._12 = ms_matInverseView._12;
    ms_matInverseViewYAxis._21 = ms_matInverseView._21;
    ms_matInverseViewYAxis._22 = ms_matInverseView._22;
}

void CGraphicBase::UpdatePipeLineMatrix()
{
    UpdateProjMatrix();
    UpdateViewMatrix();


}

void CGraphicBase::SetViewport(uint32_t dwX, uint32_t dwY, uint32_t dwWidth, uint32_t dwHeight, float fMinZ,
                               float fMaxZ)
{
    ms_Viewport.X = dwX;
    ms_Viewport.Y = dwY;
    ms_Viewport.Width = dwWidth;
    ms_Viewport.Height = dwHeight;
    ms_Viewport.MinZ = fMinZ;
    ms_Viewport.MaxZ = fMaxZ;
}

void CGraphicBase::GetTargetPosition(float *px, float *py, float *pz)
{
    *px = CCameraManager::Instance().GetCurrentCamera()->GetTarget().x;
    *py = CCameraManager::Instance().GetCurrentCamera()->GetTarget().y;
    *pz = CCameraManager::Instance().GetCurrentCamera()->GetTarget().z;
}

void CGraphicBase::GetCameraPosition(float *px, float *py, float *pz)
{
    *px = CCameraManager::Instance().GetCurrentCamera()->GetEye().x;
    *py = CCameraManager::Instance().GetCurrentCamera()->GetEye().y;
    *pz = CCameraManager::Instance().GetCurrentCamera()->GetEye().z;
}


void CGraphicBase::GetSphereMatrix(Matrix *pMatrix, float fValue)
{
    *pMatrix = Matrix::Identity;
    pMatrix->_11 = fValue * ms_matWorldView._11;
    pMatrix->_21 = fValue * ms_matWorldView._21;
    pMatrix->_31 = fValue * ms_matWorldView._31;
    pMatrix->_41 = fValue;
    pMatrix->_12 = -fValue * ms_matWorldView._12;
    pMatrix->_22 = -fValue * ms_matWorldView._22;
    pMatrix->_32 = -fValue * ms_matWorldView._32;
    pMatrix->_42 = -fValue;
}

float CGraphicBase::GetFOV()
{
    return ms_fFieldOfView;
}

float CGraphicBase::GetAspect()
{
    return ms_fAspect;
}

float CGraphicBase::GetNear()
{
    return ms_fNearY;
}

float CGraphicBase::GetFar()
{
    return ms_fFarY;
}

void CGraphicBase::PushMatrix()
{
    ms_lpd3dMatStack->Push();
}

void CGraphicBase::Scale(float x, float y, float z)
{
    ms_lpd3dMatStack->Scale(x, y, z);
}

void CGraphicBase::Rotate(float degree, float x, float y, float z)
{
    Vector3 vec(x, y, z);
    ms_lpd3dMatStack->RotateAxis(vec, XMConvertToRadians(degree));
}

void CGraphicBase::RotateLocal(float degree, float x, float y, float z)
{
    Vector3 vec(x, y, z);
    ms_lpd3dMatStack->RotateAxisLocal(vec, XMConvertToRadians(degree));
}

void CGraphicBase::MultMatrix(const Matrix &pMat)
{
    ms_lpd3dMatStack->MultMatrix(pMat);
}

void CGraphicBase::MultMatrixLocal(const Matrix &pMat)
{
    ms_lpd3dMatStack->MultMatrixLocal(pMat);
}

void CGraphicBase::RotateYawPitchRollLocal(float fYaw, float fPitch, float fRoll)
{
    ms_lpd3dMatStack->RotateYawPitchRollLocal(XMConvertToRadians(fYaw), XMConvertToRadians(fPitch), XMConvertToRadians(fRoll));
}

void CGraphicBase::Translate(float x, float y, float z)
{
    ms_lpd3dMatStack->Translate(x, y, z);
}

void CGraphicBase::LoadMatrix(Matrix &c_rSrcMatrix)
{
    ms_lpd3dMatStack->LoadMatrix(c_rSrcMatrix);
}

void CGraphicBase::PopMatrix()
{
    ms_lpd3dMatStack->Pop();
}

uint32_t CGraphicBase::GetColor(float r, float g, float b, float a)
{
    uint8_t argb[4] =
    {
        (uint8_t)(255.0f * b),
        (uint8_t)(255.0f * g),
        (uint8_t)(255.0f * r),
        (uint8_t)(255.0f * a)
    };

    return *((uint32_t *)argb);
}

void CGraphicBase::InitScreenEffect()
{
    ms_dwWavingEndTime = 0;
    ms_dwFlashingEndTime = 0;
    ms_iWavingPower = 0;
    ms_FlashingColor = DirectX::SimpleMath::Color(0.0f, 0.0f, 0.0f, 0.0f);
}

void CGraphicBase::SetScreenEffectWaving(float fDuringTime, int iPower)
{
    ms_dwWavingEndTime = DX::StepTimer::instance().GetTotalMillieSeconds() + long(fDuringTime * 1000.0f);
    ms_iWavingPower = iPower;
}

void CGraphicBase::SetScreenEffectFlashing(float fDuringTime, const DirectX::SimpleMath::Color &c_rColor)
{
    ms_dwFlashingEndTime = DX::StepTimer::instance().GetTotalMillieSeconds() + long(fDuringTime * 1000.0f);
    ms_FlashingColor = c_rColor;
}

uint32_t CGraphicBase::GetFaceCount()
{
    return ms_faceCount;
}

void CGraphicBase::ResetFaceCount()
{
    ms_faceCount = 0;
}

HRESULT CGraphicBase::GetLastResult()
{
    return ms_hLastResult;
}

CGraphicBase::CGraphicBase()
{
    bx::DefaultAllocator allocator;
    g_allocator =
	    s_allocatorStub = BX_NEW(&allocator, AllocatorStub);
}

CGraphicBase::~CGraphicBase()
{
}
