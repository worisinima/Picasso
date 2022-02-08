#pragma once

#include "Texture2D.h"

class RenderTarget : public Texture2D
{
public:

	RenderTarget() = default;
	RenderTarget(const DXGI_FORMAT& format, const DirectX::FMathLib::Color& inClearColor);
	virtual ~RenderTarget();

	virtual HRESULT Init(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		UINT width,
		UINT height
		);

	virtual HRESULT CreateSRV(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		ID3D12DescriptorHeap* srvHeap,
		const int& srvHeapIndex
	);

	virtual HRESULT CreateRTV(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		ID3D12DescriptorHeap* rtvHeap,
		const int& rtvHeapIndex
	);

	virtual HRESULT CreateDSV(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		ID3D12DescriptorHeap* dsvHeap,
		const int& dsvHeapIndex
	){return S_FALSE;}

	const float* GetClearData()
	{
		DirectX::FMathLib::Color& col = GetClearColor();
		clearColorData[0] = col.R();
		clearColorData[1] = col.G();
		clearColorData[2] = col.B();
		clearColorData[3] = col.A();

		return clearColorData;
	}

	DirectX::FMathLib::Color GetClearColor(){return mClearColor;}
	//The handle has been offseted when created it
	CD3DX12_CPU_DESCRIPTOR_HANDLE& GetRTVDescriptorHandle() { return mRTVDescriptorHandle; }
	CD3DX12_CPU_DESCRIPTOR_HANDLE& GetDSVDescriptorHandle() { return mDSVDescriptorHandle; }

	//��ʼ��Ⱦ�������target��Ϊ��ȾĿ��
	virtual void BeginRender(ID3D12GraphicsCommandList* commandList);
	//������Ⱦ�������targetת��ΪShaderResource
	virtual void EndRender(ID3D12GraphicsCommandList* commandList);

protected:
	DirectX::FMathLib::Color mClearColor;
	float clearColorData[4];

	CD3DX12_CPU_DESCRIPTOR_HANDLE mRTVDescriptorHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mDSVDescriptorHandle;
};


class BackBufferRenderTarget : public RenderTarget
{
public:
	
	BackBufferRenderTarget() = default;
	BackBufferRenderTarget(const DXGI_FORMAT & format, const DirectX::FMathLib::Color & inClearColor, Microsoft::WRL::ComPtr<ID3D12Resource>& buffer);
	virtual ~BackBufferRenderTarget();

	virtual HRESULT Init(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		UINT width,
		UINT height
	) override;

	//��ʼ��Ⱦ�������target��Ϊ��ȾĿ��
	virtual void BeginRender(ID3D12GraphicsCommandList* commandList) override;
	//������Ⱦ�������targetת��ΪShaderResource
	virtual void EndRender(ID3D12GraphicsCommandList* commandList) override;

private:

	//ʹ��SwapChin��ID3D12Resource��ʼ��һ��Render Target�������������������SwapChin��Resource
	//���ᱻ��ȥ��ʼ��Texture�����Resoue
	Microsoft::WRL::ComPtr<ID3D12Resource> mBackBuffer;

};

class DepthMapRenderTarget : public RenderTarget
{
public:

	DepthMapRenderTarget();
	virtual ~DepthMapRenderTarget();

	virtual HRESULT Init(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		UINT width,
		UINT height
	) override;

	virtual HRESULT CreateSRV(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		ID3D12DescriptorHeap* srvHeap,
		const int& srvHeapIndex
	) override;

	virtual HRESULT CreateDSV(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		ID3D12DescriptorHeap* dsvHeap,
		const int& dsvHeapIndex
	) override;

	//��ʼ��Ⱦ�������target��Ϊ��ȾĿ��
	virtual void BeginRender(ID3D12GraphicsCommandList* commandList) override;
	//������Ⱦ�������targetת��ΪShaderResource
	virtual void EndRender(ID3D12GraphicsCommandList* commandList) override;

};

