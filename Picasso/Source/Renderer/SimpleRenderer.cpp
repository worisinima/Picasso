
#include "Renderer.h"
#include <WindowsX.h>
#include "../D3DApp.h"
#include <WindowsX.h>
#include "StaticMesh.h"
#include "../SkinMesh/SkeletonMesh.h"

#include "OnFramePass.h"
#include "DebugArrowPass.h"
#include "ShadowMapPass.h"
#include "IBLBRDF.h"

#include "RenderTarget.h"
#include "RenderTargetPool.h"
#include "CubeTexture.h"

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;
using namespace FMathLib;
using namespace DirectX::PackedVector;

SimpleRenderer::SimpleRenderer(const int& clientWidth, const int& clientHeight)
	: BaseRenderer(clientWidth, clientHeight)
{

	oneFramePass = new OneFramePass();
	mDebugPass = new OneFramePass();
	mIBLBRDFPass = new IBLBRDF("Shaders\\IBLImportantSample.hlsl", Vector2(0, 0), Vector2(1, 1));
	TestScreenPass = new ScreenPass();
	mShadowMapPass = std::make_unique<ShadowMapPass>();

}

SimpleRenderer::~SimpleRenderer()
{
	if (md3dDevice != nullptr)
	{
		FlushCommandQueue();
	}

	ReleaseRenderPass(oneFramePass)
	ReleaseRenderPass(mDebugPass)
	ReleaseRenderPass(mIBLBRDFPass)
}

bool SimpleRenderer::InitRenderer(class D3DApp* app)
{
	if (BaseRenderer::InitRenderer(app) == false)
	{
		return false;
	}

	//one HDR render target, one is ArrowDebug, one is IBLBRDF target
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
		mRenderTargetPool->RigisterBackBufferToRenderTargetPool(mBackBufferRendertarget[i], mBackBufferFormat, Color(1, 1, 1, 1), mSwapChainBuffer[i]);
	}
	mRenderTargetPool->RigisterDepthTargetToRenderTargetPool<DepthMapRenderTarget>(mDepthStencilRendertarget, mClientWidth, mClientHeight);
	mRenderTargetPool->RigisterToRenderTargetPool<RenderTarget>(mHDRRendertarget, mClientWidth, mClientHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, Color(0.8, 0.8, 1, 1));
	mRenderTargetPool->RigisterToRenderTargetPool<RenderTarget>(mDebugArrowRendertarget, 512, 512, mBackBufferFormat, Color(0.8, 0.8, 1, 0));
	mRenderTargetPool->RigisterToRenderTargetPool<RenderTarget>(mIBLBRDFTarget, 1024, 1024, DXGI_FORMAT_R32G32B32A32_FLOAT, Color(0, 0, 0, 1));
	mRenderTargetPool->RigisterToRenderTargetPool<RenderTarget>(mTestScreenPassRenderTarget, 1024, 1024, DXGI_FORMAT_R32G32B32A32_FLOAT, Color(0, 0, 0, 1));
	mRenderTargetPool->RigisterDepthTargetToRenderTargetPool<DepthMapRenderTarget>(mShadowMap, 4096, 4096);

	mRenderTargetPool->CreateRtvAndDsvDescriptorHeaps(md3dDevice.Get(), mCommandList.Get());

	//Shadow map
	mShadowMap->Init(md3dDevice.Get(), mCommandList.Get(), 4096, 4096);
	int mShadowMapDsvindex = mRenderTargetPool->GetRenderTarget_mDsvHeap_HeapIndex(mShadowMap);
	mShadowMap->CreateDSV(md3dDevice.Get(), mCommandList.Get(), mRenderTargetPool->mDsvHeap.Get(), mShadowMapDsvindex);

	mIBLBRDFTarget->Init(md3dDevice.Get(), mCommandList.Get(), 1024, 1024);
	int mShadowMapSrvRtvIndex = mRenderTargetPool->GetRenderTarget_mRtvHeapAndmSrvHeap_HeapIndex(mIBLBRDFTarget);
	mIBLBRDFTarget->CreateSRV(md3dDevice.Get(), mCommandList.Get(), mRenderTargetPool->mSrvHeap.Get(), mShadowMapSrvRtvIndex);
	mIBLBRDFTarget->CreateRTV(md3dDevice.Get(), mCommandList.Get(), mRenderTargetPool->mRtvHeap.Get(), mShadowMapSrvRtvIndex);

	mTestScreenPassRenderTarget->Init(md3dDevice.Get(), mCommandList.Get(), 1024, 1024);
	int mTestScreenPassRenderTargetSrvRtvIndex = mRenderTargetPool->GetRenderTarget_mRtvHeapAndmSrvHeap_HeapIndex(mTestScreenPassRenderTarget);
	mTestScreenPassRenderTarget->CreateSRV(md3dDevice.Get(), mCommandList.Get(), mRenderTargetPool->mSrvHeap.Get(), mTestScreenPassRenderTargetSrvRtvIndex);
	mTestScreenPassRenderTarget->CreateRTV(md3dDevice.Get(), mCommandList.Get(), mRenderTargetPool->mRtvHeap.Get(), mTestScreenPassRenderTargetSrvRtvIndex);

	//存放UI的ShaderResourceView
	//Create SrvHeap for MIGUI
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = SwapChainBufferCount;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mMiGUISrvHeap.GetAddressOf())));

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	mCamera.LookAt(DirectX::XMVectorSet(-8.0f, 8.0f, 8.0f, 1), DirectX::XMVectorSet(0, 0, 0, 1), XMVectorSet(0, 0, 1, 0));

	BuildFrameResources();

	oneFramePass->InitOneFramePass(mCommandList.Get(), md3dDevice.Get(), mBackBufferFormat);
	mDebugPass->InitOneFramePass(mCommandList.Get(), md3dDevice.Get(), mBackBufferFormat);
	mIBLBRDFPass->Init(mCommandList.Get(), md3dDevice.Get(), mIBLBRDFTarget->GetFormat());
	mShadowMapPass->InitShadowMapPass(md3dDevice, mCommandList, mShadowMap, 3);

	TestScreenPass->Init(mCommandList.Get(), md3dDevice.Get(), mTestScreenPassRenderTarget->GetFormat());

	mDebugPassMaterial = new MaterialResource();
	mDebugPassMaterial->InitMaterial(md3dDevice.Get(), mCommandList.Get(), { mShadowMap });

	mDebugIBLPassMaterial = new MaterialResource();
	mDebugIBLPassMaterial->InitMaterial(md3dDevice.Get(), mCommandList.Get(), { mIBLBRDFTarget });

	mTestScrrenPassMaterial = new MaterialResource();
	mTestScrrenPassMaterial->InitMaterial(md3dDevice.Get(), mCommandList.Get(), { mTestScreenPassRenderTarget });

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	// Wait until initialization is complete.
	FlushCommandQueue();

	OnResize();

	return true;
}

void SimpleRenderer::OnResize()
{
	assert(md3dDevice);
	assert(mSwapChain);
	assert(mDirectCmdListAlloc);

	// Flush before changing any resources.
	FlushCommandQueue();

	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < SwapChainBufferCount; ++i)
	{
		mSwapChainBuffer[i].Reset();
		delete(mBackBufferRendertarget[i]);
		mBackBufferRendertarget[i] = nullptr;
	}

	// Resize the swap chain.
	ThrowIfFailed(mSwapChain->ResizeBuffers(
		SwapChainBufferCount,
		mClientWidth, mClientHeight,
		mBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	mCurrBackBuffer = 0;

	//Swap chin 里面会自己创建Resource
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
		mBackBufferRendertarget[i] = new BackBufferRenderTarget(mBackBufferFormat, Color(1, 1, 1, 1), mSwapChainBuffer[i]);
		ThrowIfFailed(mBackBufferRendertarget[i]->Init(
			md3dDevice.Get(),
			mCommandList.Get(),
			0,
			0
		));
		mBackBufferRendertarget[i]->CreateSRV(md3dDevice.Get(), mCommandList.Get(), mRenderTargetPool->mSrvHeap.Get(), i);
		mBackBufferRendertarget[i]->CreateRTV(md3dDevice.Get(), mCommandList.Get(), mRenderTargetPool->mRtvHeap.Get(), i);
	}

	//Create HDR back buffer
	mHDRRendertarget->Init(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
	int mHDRRendertargetindex = mRenderTargetPool->GetRenderTarget_mRtvHeapAndmSrvHeap_HeapIndex(mHDRRendertarget);
	mHDRRendertarget->CreateSRV(md3dDevice.Get(), mCommandList.Get(), mRenderTargetPool->mSrvHeap.Get(), mHDRRendertargetindex);
	mHDRRendertarget->CreateRTV(md3dDevice.Get(), mCommandList.Get(), mRenderTargetPool->mRtvHeap.Get(), mHDRRendertargetindex);

	{
		if (mDebugArrowRendertarget == nullptr)
			mDebugArrowRendertarget = new RenderTarget(mBackBufferFormat, Color(0.8, 0.8, 1, 1));

		ThrowIfFailed(mDebugArrowRendertarget->Init(
			md3dDevice.Get(),
			mCommandList.Get(),
			512,
			512
		));
		int mDebugArrowRendertargetindex = mRenderTargetPool->GetRenderTarget_mRtvHeapAndmSrvHeap_HeapIndex(mDebugArrowRendertarget);
		mDebugArrowRendertarget->CreateSRV(md3dDevice.Get(), mCommandList.Get(), mRenderTargetPool->mSrvHeap.Get(), mDebugArrowRendertargetindex);
		mDebugArrowRendertarget->CreateRTV(md3dDevice.Get(), mCommandList.Get(), mRenderTargetPool->mRtvHeap.Get(), mDebugArrowRendertargetindex);
		if (mArrowPassMaterial == nullptr)
			mArrowPassMaterial = new MaterialResource();

		PipelineInfoForMaterialBuild Info = { mHDRRendertarget->GetFormat(), mDepthStencilFormat, m4xMsaaState ? 4 : 1 , m4xMsaaState ? (m4xMsaaQuality - 1) : 0 };
		mArrowPassMaterial->InitMaterial(md3dDevice.Get(), mCommandList.Get(), { mDebugArrowRendertarget });
	}

	//mDepthStencilRendertarget = new DepthMapRenderTarget();
	mDepthStencilRendertarget->Init(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
	mDepthStencilRendertarget->CreateDSV(md3dDevice.Get(), mCommandList.Get(), mRenderTargetPool->mDsvHeap.Get(), 0);//这里把depth view 放到了mDsvHeap的第0号位置

	// Execute the resize commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mClientWidth);
	mScreenViewport.Height = static_cast<float>(mClientHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, mClientWidth, mClientHeight };

	mCamera.SetLens(0.4f * MathHelper::Pi, AspectRatio(), 0.01f, 1000.0f);

}

void SimpleRenderer::Draw(const GameTimer & gt)
{
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), nullptr));

	ID3D12DescriptorHeap* descriptorHeaps[] = { mRenderTargetPool->mSrvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	RenderTarget* CurrentBackTarget = mBackBufferRendertarget[mCurrBackBuffer];

	//ToneMapping
	ID3D12DescriptorHeap* descriptorHeaps2[] = { mRenderTargetPool->mSrvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps2), descriptorHeaps2);
	oneFramePass->Draw(
		mCommandList.Get(),
		mDirectCmdListAlloc.Get(),
		mScreenViewport,
		mScissorRect,
		mHDRRendertarget,
		CurrentBackTarget
	);
	
	//Just draw once
	if (bDrawIBLToggle == true)
	{
		mIBLBRDFPass->Draw(mCommandList.Get(), mDirectCmdListAlloc.Get(), mIBLBRDFTarget);
		TestScreenPass->Draw(mCommandList.Get(), mDirectCmdListAlloc.Get(), mTestScreenPassRenderTarget);
		bDrawIBLToggle = false;
	}

	mDebugPass->SetWindowScaleAndCenter(Vector2(0.25, 0.25f), Vector2(10, 10));
	mDebugPass->DrawMaterialToRendertarget(
		mCommandList.Get(),
		mDirectCmdListAlloc.Get(),
		CurrentBackTarget,
		mDebugIBLPassMaterial,
		mIBLBRDFTarget->GetWidth(),
		mIBLBRDFTarget->GetHeight()
	);

	mDebugPass->SetWindowScaleAndCenter(Vector2(0.25f, 0.25f), Vector2(300, 10));
	mDebugPass->DrawMaterialToRendertarget(
		mCommandList.Get(),
		mDirectCmdListAlloc.Get(),
		CurrentBackTarget,
		mTestScrrenPassMaterial,
		1024,
		1024
	);
	
	//Begin Render UI
	//mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
	CurrentBackTarget->BeginRender(mCommandList.Get());
	mCommandList->OMSetRenderTargets(1, &CurrentBackTarget->GetRTVDescriptorHandle(), true, &DepthStencilView());
	ID3D12DescriptorHeap* ImGUIdescriptorHeaps[] = { mMiGUISrvHeap.Get() };
	mCommandList->SetDescriptorHeaps(1, ImGUIdescriptorHeaps);
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mCommandList.Get());
	CurrentBackTarget->EndRender(mCommandList.Get());
	//End Render UI

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	mCurrFrameResource->Fence = ++mCurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void SimpleRenderer::UpdateCamera()
{
	mCamera.UpdateViewMatrix();
	mShadowMapPass->mOrthCamera.UpdateViewMatrix();
}

void SimpleRenderer::Update(const GameTimer & gt)
{
	// Cycle through the circular frame resource array.
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void SimpleRenderer::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>
		(
			md3dDevice.Get(),
			0,
			0,
			0
		));
	}
}