
#include "RenderTargetPool.h"
#include "RenderTarget.h"
#include "../Common/d3dUtil.h"

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;
using namespace FMathLib;
using namespace DirectX::PackedVector;

#ifdef ReleaseRenderTarget
#undef ReleaseRenderTarget
#endif
#define ReleaseRenderTarget(targetToBeRelease) \
if (targetToBeRelease != nullptr)\
{\
	delete(targetToBeRelease);\
	targetToBeRelease = nullptr;\
}

RenderTargetPool::~RenderTargetPool()
{
	for (auto t : mRenderTargetPool)
	{
		ReleaseRenderTarget(t)
	}
	for (auto t : mDepthRenderTargetPool)
	{
		ReleaseRenderTarget(t)
	}
}

void RenderTargetPool::CreateRtvAndDsvDescriptorHeaps(ID3D12Device* md3dDevice, ID3D12GraphicsCommandList* cmdList)
{
	//存放Render TargetView
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = GetRenderTargetPoolSize();
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	//存放渲染目标的ShaderResourceView
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = GetRenderTargetPoolSize();
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mSrvHeap)));
	
	//存放深度的DepthShaderView
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = GetDepthRenderTargetPoolSize();
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}

int RenderTargetPool::GetRenderTarget_mDsvHeap_HeapIndex(RenderTarget* rt)
{
	if (rt)
	{
		if (dynamic_cast<DepthMapRenderTarget*>(rt))
		{
			int index = 0;
			for (auto t : mDepthRenderTargetPool)
			{
				if (t == rt)
				{
					return index;
				}
				index++;
			}
		}
	}
	return -1;
}
int RenderTargetPool::GetRenderTarget_mRtvHeapAndmSrvHeap_HeapIndex(RenderTarget* rt)
{
	if (rt)
	{
		if (dynamic_cast<BackBufferRenderTarget*>(rt))
		{
			int index = 0;
			for (auto t : mBackBufferRenderTargetPool)
			{
				if (t == rt)
				{
					return index;
				}
				index++;
			}
		}
		else
		{
			//这里写死是2，因为mBackBufferRenderTarget是两个
			int index = 2;
			for (auto t : mRenderTargetPool)
			{
				if (t == rt)
				{
					return index;
				}
				index++;
			}
		}
	}
	return -1;
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetPool::GetRenderTarget_DsV_D3D12_CPU_DESCRIPTOR_HANDLE(ID3D12Device* md3dDevice, RenderTarget* destRT)
{
	if (destRT == NULL)
	{
		ThrowIfFailed(S_FALSE);
		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}

	if (dynamic_cast<DepthMapRenderTarget*>(destRT))
	{
		UINT cbvDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		int index = 0;
		for (auto t : mDepthRenderTargetPool)
		{
			if (t == destRT)
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE mDsvDescriptorCPUHandle(mDsvHeap->GetCPUDescriptorHandleForHeapStart());
				mDsvDescriptorCPUHandle.Offset(index, cbvDsvDescriptorSize);

				return mDsvDescriptorCPUHandle;
			}
			index++;
		}
	}

	return D3D12_CPU_DESCRIPTOR_HANDLE();
}