
#pragma once

#include "../../Core/Core.h"
#include "../../Common/d3dUtil.h"
#include "../../SkinMesh/SkinnedData.h"
#include "../../Common/UploadBuffer.h"
#include "../../Core/Shader.h"

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;
using namespace FMathLib;
using namespace DirectX::PackedVector;

extern const int gNumFrameResources;

class Texture2D;
class RenderTarget;
class BackBufferRenderTarget;

#ifdef ReleaseRenderPass
#undef ReleaseRenderPass
#endif
#define ReleaseRenderPass(RenderPass) \
if (RenderPass != nullptr)\
{\
	delete(RenderPass);\
	RenderPass = nullptr;\
}

struct ScreenVertex
{
	XMFLOAT4 Position;
	XMFLOAT4 Coord;
};

//默认是把一个shader画到一个RT上
class ScreenPass
{
public:

	ScreenPass();
	ScreenPass(const string& ShaderFilePath, const Vector2& WindowCenter, const Vector2& WindowScale);

	virtual bool Init(
		ID3D12GraphicsCommandList* mCommandList,
		ID3D12Device* md3dDevice,
		DXGI_FORMAT mBackBufferFormat);

	virtual void Draw(
		ID3D12GraphicsCommandList* mCommandList,
		ID3D12CommandAllocator* mDirectCmdListAlloc,
		class RenderTarget* destRT
	);

	inline void SetWindowScaleAndCenter(const Vector2& inScale, const Vector2& inCenter)
	{
		mWindowScale = inScale;
		mWindowCenter = inCenter;
	}

protected:

	virtual void BuildScreenGeomertry(ID3D12GraphicsCommandList* mCommandList, ID3D12Device* md3dDevice);
	virtual void BuildInputLayout();
	virtual void BuildPSO(ID3D12GraphicsCommandList* mCommandList, ID3D12Device* md3dDevice, DXGI_FORMAT& mBackBufferFormat);
	virtual void BuildRootSignature(ID3D12GraphicsCommandList* mCommandList, ID3D12Device* md3dDevice);

	//Default buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
	//Upload buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vbv;
	D3D12_INDEX_BUFFER_VIEW ibv;

	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	ComPtr<ID3D12PipelineState> mPSO = nullptr;

	ComPtr<ID3D12RootSignature> mRootSignatureA = nullptr;
	ComPtr<ID3D12RootSignature> mRootSignatureB = nullptr;

	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

	Vector2 mWindowScale;
	Vector2 mWindowCenter;

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
};
