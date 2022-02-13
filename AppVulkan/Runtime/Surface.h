#pragma once
#include "Image.h"

struct SurfaceDesc
{
	uint32_t width;
	uint32_t height;
	VkFormat format;
	uint32_t msaaCount;
	uint32_t finalLyout;

	inline bool operator==(const SurfaceDesc& other) const noexcept
	{
		return memcmp(this, &other, sizeof(SurfaceDesc)) == 0;
	}

	inline bool operator!=(const SurfaceDesc& other) const noexcept
	{
		return memcmp(this, &other, sizeof(SurfaceDesc)) != 0;
	}
};

class Surface
{
public:
	Surface();
	Surface(Image img, SurfaceDesc desc, unsigned long long frameLastUsed);

	Image GetImage() const;
	const SurfaceDesc& GetDesc() const;
	void UpdateLastUsed(unsigned long long currFrame);

private:
	Image m_Image;
	SurfaceDesc m_SurfaceDesc;
	unsigned long long m_FrameLastUsed;
};

