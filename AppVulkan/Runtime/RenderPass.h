#pragma once
#include "Utilities.h"
#include "Material.h"
#include <string>
#include "Surface.h"

enum RenderPassPlace : uint8_t
{
	kRenderPassPlace_ColorCode,
	kRenderPassPlace_Opaques,
	kRenderPassPlace_Outline,
	kRenderPassPlace_PresentBlit,
	KRenderPassPlace_RenderpassCount
};

enum FramebufferTarget : uint8_t
{
	kTargetSwapchain,
	kTargetSceneView
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
	//uint8_t framebufferTarget;
	uint8_t colorAttachmentCount;
	uint8_t colorFormat;
	uint8_t colorLoadOp;
	uint8_t colorStoreOp;
	uint8_t depthFormat;
	uint8_t depthLoadOp;
	uint8_t depthStoreOp;

	bool operator==(const RenderPassDesc& other) const noexcept
	{
		return memcmp(this, &other, sizeof(RenderPassDesc));
	}
};

class RenderPass
{
public:

	RenderPass();
	void CreateRenderPass(const RenderPassDesc& desc, std::vector<std::pair<uint32_t, SurfaceDesc>>& surfaceDescriptions);

	VkRenderPass GetVkRenderPass() const { return m_RenderPass; }
	RenderPassDesc GetRenderPassDesc() const { return m_Desc; }
	const std::vector<std::pair<uint32_t, SurfaceDesc>>& GetSurfaceDescriptions() const { return m_SurfaceDescriptions; }

	void UpdateRenderPassViewport(uint32_t width, uint32_t height);

	void Destroy();
	
private:

	VkRenderPass m_RenderPass;
	std::vector<std::pair<uint32_t, SurfaceDesc>> m_SurfaceDescriptions;
	RenderPassDesc m_Desc;
#ifdef _DEBUG
	std::string m_Name;
#endif
};

enum RenderCommand
{
	kRenCommandDrawQuad
};
