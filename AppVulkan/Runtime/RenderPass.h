#pragma once
#include "Utilities.h"
#include <string>

enum RenderPassPlace : uint8_t
{
	kRenderPassPlace_Opaques,
	kRenderPassPlace_AfterOpaques
};

enum FramebufferTarget : uint8_t
{
	kTargetSwapchain,
	kTargetCustom
};

enum AttachmentLoadOp : uint8_t
{
	kLoadOpLoad,
	kLoadOpClear,
	kLoadOpDontCare
};

enum AttachmentStoreOp : uint8_t
{
	kStoreOpStore,
	kStoreOpDontCare
};

struct RenderPassDesc
{
	uint8_t msaaCount;
	uint8_t renderpassPlace;
	uint8_t framebufferTarget;
	uint8_t colorAttachmentCount;
	uint8_t colorFormat;
	uint8_t colorLoadOp;
	uint8_t colorStoreOp;
	uint8_t depthFormat;
	uint8_t depthLoadOp;
	uint8_t depthStoreOp;
};

class RenderPass
{
public:
	void CreateRenderPass(const RenderPassDesc& desc);

	VkRenderPass GetVkRenderPass() const { return m_RenderPass; }
	
private:

	VkRenderPass m_RenderPass;
#ifdef _DEBUG
	std::string m_Name;
	RenderPassDesc m_Desc;
#endif

};

