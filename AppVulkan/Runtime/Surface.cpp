#include "Surface.h"

Surface::Surface()
	: m_Image(), m_SurfaceDesc(), m_FrameLastUsed(0ull)
{
}

Surface::Surface(Image img, SurfaceDesc desc, unsigned long long frameLastUsed)
	: m_Image(img), m_SurfaceDesc(desc), m_FrameLastUsed(frameLastUsed)
{
}

Image Surface::GetImage() const
{
	return m_Image;
}

const SurfaceDesc& Surface::GetDesc() const
{
	return m_SurfaceDesc;
}

void Surface::UpdateLastUsed(unsigned long long currFrame)
{
	m_FrameLastUsed = currFrame;
}
