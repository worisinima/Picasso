#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "Common/d3dUtil.h"
#include "Common/GameTimer.h"

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_dx12.h"
#include "ImGUI/imgui_impl_win32.h"
#include <dxgi1_4.h>
#include <tchar.h>

#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Common/GeometryGenerator.h"
#include "Common/Camera.h"
#include "FrameResource.h"
#include "FileHelper.h"
#include "Core/RenderCore.h"
#include "Texture2D.h"

#include "SkinMesh/SkeletonMesh.h"
#include "Light.h"

#include <cstdlib>

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

class OneFramePass;
class DebugArrowPass;
class ShadowMapPass;
class IBLBRDF;

class RenderTarget;
class RenderTargetPool;

class StaticMeshComponent;
class SkeletonMeshComponent;

class BaseRenderer
{
public:
	
	BaseRenderer(const int& clientWidth, const int& clientHeight);
	BaseRenderer(const BaseRenderer& rhs) = delete;
	BaseRenderer& operator=(const BaseRenderer& rhs) = delete;
	~BaseRenderer();
	
	virtual bool InitRenderer(class D3DApp* app);
	virtual void Draw(const GameTimer& gt){}
	virtual void Update(const GameTimer& gt){}
	virtual void CreateCommandObjects();
	virtual void CreateSwapChain();
	virtual void OnResize(){}

	// Derived class should set these in derived constructor to customize starting values.
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	UINT64 mCurrentFence = 0;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	static const int SwapChainBufferCount = 2;
	static const int mDepthStencilBufferCount = 1;
	int mCurrBackBuffer = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSRVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mMiGUISrvHeap;
	virtual void CreateRtvAndDsvDescriptorHeaps();

	std::unique_ptr<RenderTargetPool> mRenderTargetPool;
	RenderTarget* mBackBufferRendertarget[SwapChainBufferCount];

	HWND	mhMainWnd;	// main window handle
	bool	m4xMsaaState = false;	// 4X MSAA enabled
	UINT	m4xMsaaQuality = 0;		// quality level of 4X MSAA

	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;

	float AspectRatio() const;
	void SetRendererWindowSize(const int& width, const int& height) { mClientWidth = width; mClientHeight = height; }
	void FlushCommandQueue();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	ID3D12Resource* CurrentBackBuffer()const;

	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

protected:
	int mClientWidth;
	int mClientHeight;
};

class Renderer : public BaseRenderer
{
public:

	Renderer(const int& clientWidth, const int& clientHeight);
	Renderer(const Renderer& rhs) = delete;
	Renderer& operator=(const Renderer& rhs) = delete;
	~Renderer();

	virtual bool InitRenderer(class D3DApp* app) override;
	virtual void Draw(const GameTimer& gt) override;
	virtual void Update(const GameTimer& gt) override;
	virtual void OnResize() override;
	
	//RenderPass
	void UpdateCamera();
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	void LoadTextures();
	void BuildShadersAndInputLayout();
	void BuildOBJGeometry(std::string MeshPath, std::string MeshName, std::string DrawArgsName);
	void BuildFrameResources();
	void BuildRenderItems();

	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems, D3D12_GPU_VIRTUAL_ADDRESS& passCBAddress);

	OneFramePass* oneFramePass;
	OneFramePass* mDebugPass;
	DebugArrowPass* mdebugArrowPass;
	IBLBRDF* mIBLBRDFPass;
	std::unique_ptr<ShadowMapPass> mShadowMapPass;

	std::vector<MaterialResource> mMaterials;
	MaterialResource* mDebugPassMaterial;
	MaterialResource* mDebugIBLPassMaterial;
	MaterialResource* mArrowPassMaterial;
	
	void BuildLightData();
	Light* mSimpleLight = nullptr;

	RenderTarget* mShadowMap;
	RenderTarget* mHDRRendertarget;
	RenderTarget* mDebugArrowRendertarget;
	RenderTarget* mIBLBRDFTarget;
	//we just draw once IBLBRDF
	bool bDrawIBLToggle = true;
	
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	UINT mCbvSrvDescriptorSize = 0;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTexture2Ds;
	std::unordered_map<std::string, std::unique_ptr<StaticMeshComponent>> mStaticMeshes;
	std::unique_ptr<SkeletonMeshComponent> mSkeletonMesh;
	std::unique_ptr<SkeletonMesh> mTestFBXSkinMesh;;

	//std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	unique_ptr<DefaultVS>StandardVertexShader;
	unique_ptr<DefaultPS>StandardPixleShader;
	unique_ptr<SkinVS>SkinVertexShader;
	unique_ptr<SkinPS>SkinPixleShader;

	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	//std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mSkinnedInputLayout;

	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	std::vector<RenderItem*> mRitemLayer[(int)EPassType::Num];
	PassConstants mMainPassCB;

	UINT mPassCbvOffset = 0;

	bool mIsWireframe = false;

	float mTheta = 1.5f * XM_PI;
	float mPhi = 0.2f * XM_PI;
	float mRadius = 15.0f;
	Camera mCamera;
};
