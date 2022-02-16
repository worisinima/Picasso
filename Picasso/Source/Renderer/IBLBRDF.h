
#pragma once

#include "ScreenPass/ScreenPass.h"

class RenderTarget;
class BackBufferRenderTarget;

using namespace DirectX;
using namespace FMathLib;

class IBLBRDF : ScreenPass
{
public:
	
	IBLBRDF();
	IBLBRDF(const string& ShaderFilePath, const Vector2& WindowCenter, const Vector2& WindowScale);

	virtual bool Init(
		ID3D12GraphicsCommandList* mCommandList,
		ID3D12Device* md3dDevice,
		DXGI_FORMAT mBackBufferFormat) override;

	virtual void Draw(
		ID3D12GraphicsCommandList* mCommandList,
		ID3D12CommandAllocator* mDirectCmdListAlloc,
		class RenderTarget* destRT
	) override;

protected:

	virtual void BuildPSO(ID3D12GraphicsCommandList* mCommandList, ID3D12Device* md3dDevice, DXGI_FORMAT& mBackBufferFormat) override;
	virtual void BuildRootSignature(ID3D12GraphicsCommandList* mCommandList, ID3D12Device* md3dDevice) override;
};
