#pragma once
#include "Utilities.h"
#include "Material.h"
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

	bool operator==(const RenderPassDesc& other) const noexcept
	{
		return memcmp(this, &other, sizeof(RenderPassDesc));
	}
};

struct RenderPassDescHasher
{
	size_t operator()(const RenderPassDesc& k) const noexcept
	{
		return std::hash<uint8_t>()(k.msaaCount) ^
			std::hash<uint8_t>()(k.renderpassPlace) ^
			std::hash<uint8_t>()(k.framebufferTarget) ^
			std::hash<uint8_t>()(k.colorAttachmentCount) ^
			std::hash<uint8_t>()(k.colorFormat) ^
			std::hash<uint8_t>()(k.colorLoadOp) ^
			std::hash<uint8_t>()(k.colorStoreOp) ^
			std::hash<uint8_t>()(k.depthFormat) ^
			std::hash<uint8_t>()(k.depthLoadOp) ^
			std::hash<uint8_t>()(k.depthStoreOp);
	}
};

class RenderPass
{
public:
	void CreateRenderPass(const RenderPassDesc& desc);

	VkRenderPass GetVkRenderPass() const { return m_RenderPass; }

	void Destroy();

	// -- commands --
	void DrawQuad(Material material);
	
private:

	VkRenderPass m_RenderPass;
	RenderPassDesc m_Desc; // if this is used as a key, then maybe we dont need to store it here
#ifdef _DEBUG
	std::string m_Name;
#endif

	// temporary, just to get something working
	Material mat;

};

enum RenderCommand
{
	kRenCommandDrawQuad
};