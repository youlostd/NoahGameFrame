#ifndef METIN2_CLIENT_ETERLIB_GRPBASE_HPP
#define METIN2_CLIENT_ETERLIB_GRPBASE_HPP

#pragma once
#include <bx/bx.h>
#include <bx/macros.h>
#include "GrpDetector.h"
#include "Ray.h"

class MatrixStack;

void setGraphicsDebuggerPresent(bool _present);
	bool isGraphicsDebuggerPresent();

using EngineHandleNum = int;
using VertexFormatHandle = EngineHandleNum;
using ShaderHandle = EngineHandleNum;

constexpr EngineHandleNum NullEngineHandle = 0;

static const uint16_t kInvalidHandle = UINT16_MAX;

#define ENGINE_HANDLE(_name)                                                           \
	struct _name { uint16_t idx; };                                                  \
	inline bool isValid(_name _handle) { return kInvalidHandle != _handle.idx; }

#define ENGINE_INVALID_HANDLE { kInvalidHandle }

ENGINE_HANDLE(VertexBufferHandle)
ENGINE_HANDLE(IndexBufferHandle)

#define DX_RELEASE(_ptr, _expected) \
			for(;;) { \
				if (NULL != (_ptr) ) \
				{ \
					ULONG count = (_ptr)->Release(); \
					if (_expected != count) { \
					    OutputDebugString(fmt::format("{}:{} {} RefCount is {} (expected {}).", (const char*)__FILE__ ,(int)__LINE__, fmt::ptr((void*)_ptr), (int)count, (int)_expected).c_str()); \
                    } \
                _ptr = NULL; \
				} \
			break; } \


class CGraphicTexture;

typedef uint16_t TIndex;

typedef struct SFace
{
    TIndex indices[3];
} TFace;

typedef Vector3 TPosition;
typedef Vector4 TPosition2D;

typedef Vector3 TNormal;

typedef Vector2 TTextureCoordinate;

typedef uint32_t TDiffuse;
typedef uint32_t TAmbient;
typedef uint32_t TSpecular;

typedef union UDepth
{
    float f;
    long l;
    uint32_t dw;
} TDepth;

typedef struct SVertex
{
    float x, y, z;
    uint32_t color;
    float u, v;
} TVertex;

struct STVertex
{
    float x, y, z, rhw;
};

struct SPVertex
{
    float x, y, z;
};

typedef struct SPDVertex
{
    float x, y, z;
    uint32_t color;
} TPDVertex;

struct SPDTVertexRaw
{
    float px, py, pz;
    uint32_t diffuse;
    float u, v;
};

struct SPDTVertex2DRaw
{
    float px, py, pz, pw;
    uint32_t diffuse;
    float u, v;
};

typedef struct SPTVertex
{
    static const uint32_t kFVF = D3DFVF_XYZ | D3DFVF_TEX1;

    TPosition position;
    TTextureCoordinate texCoord;
} TPTVertex;

typedef struct SPTVertexRaw
{
    float px, py, pz;
    float u, v;
} TPTVertexRaw;

typedef struct SPDTVertex
{
    static const uint32_t kFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

    TPosition position;
    TDiffuse diffuse;
    TTextureCoordinate texCoord;
} TPDTVertex;

typedef struct SPDTVertex2D
{
    static const uint32_t kFVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

    TPosition2D position;
    TDiffuse diffuse;
    TTextureCoordinate texCoord;
} TPDTVertex2D;

typedef struct SPNTVertex
{
    TPosition position;
    TNormal normal;
    TTextureCoordinate texCoord;
} TPNTVertex;

typedef struct SPNT2Vertex
{
    TPosition position;
    TNormal normal;
    TTextureCoordinate texCoord;
    TTextureCoordinate texCoord2;
} TPNT2Vertex;

typedef struct SPDT2Vertex
{
    TPosition position;
    uint32_t diffuse;
    TTextureCoordinate texCoord;
    TTextureCoordinate texCoord2;
} TPDT2Vertex;

typedef struct SNameInfo
{
    uint32_t name;
    TDepth depth;
} TNameInfo;

typedef struct SBoundBox
{
    float sx, sy, sz;
    float ex, ey, ez;
    int meshIndex;
    int boneIndex;
} TBoundBox;

const uint16_t c_FillRectIndices[6] = {0, 2, 1, 2, 3, 1};

static const std::size_t kSmallPdtVertexBufferSize = 16;
static const std::size_t kLargePdtVertexBufferSize = 768;
static const std::size_t kSmallPtVertexBufferSize = 16;
static const std::size_t kLargePtVertexBufferSize = 200;

void PixelPositionToVector3(const Vector3 &c_rkPPosSrc, Vector3 *pv3Dst);
void Vector3ToPixelPosition(const Vector3 &c_rv3Src, Vector3 *pv3Dst);

class CGraphicBase
{
public:
    static uint32_t GetAvailableTextureMemory();
    static const Matrix &GetViewMatrix();
    static const Matrix &GetIdentityMatrix();

    enum
    {
        DEFAULT_IB_LINE,
        DEFAULT_IB_LINE_TRI,
        DEFAULT_IB_LINE_RECT,
        DEFAULT_IB_LINE_CUBE,
        DEFAULT_IB_FILL_TRI,
        DEFAULT_IB_FILL_RECT,
        DEFAULT_IB_FILL_CUBE,
        DEFAULT_IB_FONT,
        DEFAULT_IB_NUM,
    };

    CGraphicBase();
    virtual ~CGraphicBase();

    void SetSimpleCamera(float x, float y, float z, float pitch, float roll);
    void SetEyeCamera(float xEye, float yEye, float zEye, float xCenter, float yCenter, float zCenter, float xUp,
                      float yUp, float zUp);
    void SetAroundCamera(float distance, float pitch, float roll, float lookAtZ = 0.0f);
    void SetPositionCamera(float fx, float fy, float fz, float fDistance, float fPitch, float fRotation);
    void MoveCamera(float fdeltax, float fdeltay, float fdeltaz);

    void GetTargetPosition(float *px, float *py, float *pz);
    void GetCameraPosition(float *px, float *py, float *pz);
    void SetOrtho2D(float hres, float vres, float zres);
    void SetOrtho3D(float hres, float vres, float zmin, float zmax);
    void SetPerspective(float fov, float aspect, float nearz, float farz);
    float GetFOV();
    float GetAspect();
    float GetNear();
    float GetFar();

    void GetClipPlane(float *fNearY, float *fFarY)
    {
        *fNearY = ms_fNearY;
        *fFarY = ms_fFarY;
    }

    ////////////////////////////////////////////////////////////////////////
    void PushMatrix();

    void MultMatrix(const Matrix &pMat);
    void MultMatrixLocal(const Matrix &pMat);

    void Translate(float x, float y, float z);
    void Rotate(float degree, float x, float y, float z);
    void RotateLocal(float degree, float x, float y, float z);
    void RotateYawPitchRollLocal(float fYaw, float fPitch, float fRoll);
    void Scale(float x, float y, float z);
    void PopMatrix();
    void LoadMatrix(Matrix &c_rSrcMatrix);

    // Special Routine
    void GetSphereMatrix(Matrix *pMatrix, float fValue = 0.1f);

    ////////////////////////////////////////////////////////////////////////
    void InitScreenEffect();
    void SetScreenEffectWaving(float fDuringTime, int iPower);
    void SetScreenEffectFlashing(float fDuringTime, const DirectX::SimpleMath::Color &c_rColor);

    ////////////////////////////////////////////////////////////////////////
    uint32_t GetColor(float r, float g, float b, float a = 1.0f);

    uint32_t GetFaceCount();
    void ResetFaceCount();
    HRESULT GetLastResult();

    void UpdateProjMatrix();
    void UpdateViewMatrix();

    void SetViewport(uint32_t dwX, uint32_t dwY, uint32_t dwWidth, uint32_t dwHeight, float fMinZ, float fMaxZ);
    static void GetBackBufferSize(UINT *puWidth, UINT *puHeight);
    static bool IsTLVertexClipping();
    static bool IsFastTNL();

    static VertexBufferHandle GetSmallPdtVertexBuffer()
    {
        return m_smallPdtVertexBuffer;
    }

    static VertexBufferHandle GetLargePdtVertexBuffer()
    {
        return m_largePdtVertexBuffer;
    }


protected:
    void UpdatePipeLineMatrix();

    static Matrix ms_matIdentity;

    static Matrix ms_matView;
    static Matrix ms_matProj;
    static Matrix ms_matInverseView;
    static Matrix ms_matInverseViewYAxis;

    static Matrix ms_matWorld;
    static Matrix ms_matWorldView;

    // 각종 D3DX Mesh 들 (컬루젼 데이터 등을 표시활 때 쓴다)


    static HRESULT ms_hLastResult;

    static int ms_iWidth;
    static int ms_iHeight;

    static UINT ms_iD3DAdapterInfo;
    static UINT ms_iD3DDevInfo;
    static UINT ms_iD3DModeInfo;
    static D3D_CDisplayModeAutoDetector ms_kD3DDetector;

    static HWND ms_hWnd;
    static HDC ms_hDC;
    static LPDIRECT3D9 ms_lpd3d;
    static LPDIRECT3DDEVICE9 ms_lpd3dDevice;
    static MatrixStack *ms_lpd3dMatStack;
    static D3DVIEWPORT9 ms_Viewport;
    static bool m_deviceNeedsReset;

    static uint32_t ms_faceCount;
    static D3DCAPS9 ms_d3dCaps;
    static D3DPRESENT_PARAMETERS ms_d3dPresentParameter;

    static uint32_t ms_dwD3DBehavior;

    static Matrix ms_matScreen0;
    static Matrix ms_matScreen1;
    static Matrix ms_matScreen2;

    static Vector3 ms_vtPickRayOrig;
    static Vector3 ms_vtPickRayDir;

    static float ms_fFieldOfView;
    static float ms_fAspect;
    static float ms_fNearY;
    static float ms_fFarY;

    // Screen Effect - Waving, Flashing and so on..
    static uint64_t ms_dwWavingEndTime;
    static int ms_iWavingPower;
    static uint64_t ms_dwFlashingEndTime;
    static DirectX::SimpleMath::Color ms_FlashingColor;

    // Terrain picking용 Ray... CCamera 이용하는 버전.. 기존의 Ray와 통합 필요...
    static CRay ms_Ray;

    //
    static bool ms_bSupportDXT;

    static IndexBufferHandle ms_alpd3dDefIB[DEFAULT_IB_NUM];

    static VertexBufferHandle m_smallPdtVertexBuffer;
    static VertexBufferHandle m_largePdtVertexBuffer;

    	static const int		m_VBSizeLimit = 2048;

	static int						m_NonusedVBDataOffset;		//D3DLOCK_NOOVERWRITE 사용시에 gpu가 사용하고 있지 않는 버퍼의 메모리영역 시작주소
	static int						m_CurrentUsingVBOffset;	//현재 락을 걸어서 버텍스데이타를 카피한 주소. 렌더링시에 사용됨
	static int						m_CurrentVertexWriteCount;
};

#endif
