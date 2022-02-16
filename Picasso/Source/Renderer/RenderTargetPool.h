#pragma once

#include "RenderTarget.h"

class RenderTarget;
class BackBufferRenderTarget;

struct TextureInfo
{
	TextureInfo(){}
	TextureInfo(const UINT& w, const UINT& h):width(w), height(h){}
	UINT width = 0;
	UINT height = 0;
};

class RenderTargetPool
{
public:
	
	~RenderTargetPool();

	/*
	* RenderTarget��Rigister����
	*/
	void RigisterBackBufferToRenderTargetPool(
		RenderTarget*& TargetToRigister, 
		const DXGI_FORMAT& format,
		const DirectX::FMathLib::Color& inClearColor,
		Microsoft::WRL::ComPtr<ID3D12Resource>& buffer
	)
	{
		TargetToRigister = new BackBufferRenderTarget(format, inClearColor, buffer);
		mBackBufferRenderTargetPool.push_back(TargetToRigister);
	}

	template<class T>
	void RigisterToRenderTargetPool(RenderTarget*& TargetToRigister, UINT width, UINT height, const DXGI_FORMAT& format, const DirectX::FMathLib::Color& inClearColor)
	{
		TargetToRigister = new T(format, inClearColor);
		mRenderTargetPool.push_back(TargetToRigister);
		RenderTargetPoolTextureInfo.push_back(TextureInfo(width, height));
	}
	
	/*
	* DepthBuffer��Rigister
	*/
	template<class T>
	void RigisterDepthTargetToRenderTargetPool(RenderTarget*& TargetToRigister, UINT width, UINT height)
	{
		TargetToRigister = new T();
		mDepthRenderTargetPool.push_back(TargetToRigister);
		DepthRenderTargetPoolTextureInfo.push_back(TextureInfo(width, height));
	}

	UINT GetRenderTargetPoolSize() { return mRenderTargetPool.size() + mBackBufferRenderTargetPool.size(); }
	UINT GetDepthRenderTargetPoolSize() { return mDepthRenderTargetPool.size(); }
	
	void CreateRtvAndDsvDescriptorHeaps(ID3D12Device* md3dDevice, ID3D12GraphicsCommandList* cmdList);
	//void InitTextureAndCreatePoolRtvAbdSrvAndDsv(ID3D12Device* md3dDevice, ID3D12GraphicsCommandList* cmdList);

	//Rtv��Srv������һ�µĶ���RenderTarget��˵�Ļ�����������һ������д��
	int GetRenderTarget_mRtvHeapAndmSrvHeap_HeapIndex(RenderTarget* rt);
	int GetRenderTarget_mDsvHeap_HeapIndex(RenderTarget* rt);

	D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTarget_DsV_D3D12_CPU_DESCRIPTOR_HANDLE(ID3D12Device* md3dDevice, RenderTarget* destRT);

	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView(ID3D12Device* md3dDevice) const
	{

		int mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		int mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		int mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		//BackBufferһ����mRtvHeap�����Ž�ȥ�ģ���Ȼ��δ�����Ǵ��
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), mCurrBackBuffer, mRtvDescriptorSize);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const
	{
		//DepthStencilһ����mDsvHeap�����Ž�ȥ�ģ���Ȼ��δ�����Ǵ��
		return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	int mCurrBackBuffer = 0;
	std::vector<RenderTarget*> mBackBufferRenderTargetPool;
	std::vector<RenderTarget*> mRenderTargetPool;
	std::vector<RenderTarget*> mDepthRenderTargetPool;
	std::vector<TextureInfo> RenderTargetPoolTextureInfo;
	std::vector<TextureInfo> DepthRenderTargetPoolTextureInfo;
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvHeap;
};

