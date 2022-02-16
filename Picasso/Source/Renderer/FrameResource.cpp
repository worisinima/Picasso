#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT skinnedObjectCount)
{
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

	if (passCount != 0)
	{
		PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
	}
	if (objectCount != 0)
	{
		ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
	}
	if (skinnedObjectCount != 0)
	{
		SkinnedCB = std::make_unique<UploadBuffer<SkinnedConstants>>(device, skinnedObjectCount, true);
	}
}

FrameResource::~FrameResource()
{

}