#pragma once
#include "PCH.h"

class Shape
{
public:
	void Init(ID2D1Factory2* pD2DFactory, ID2D1DeviceContext* dc, float size);
	void Draw(ID2D1DeviceContext* dc, const D2D1::Matrix3x2F& transform);
	float GetSize() { return m_Size.width; }
private:
	ComPtr<ID2D1Bitmap> Bitmap;
	D2D1_SIZE_F m_Size = {};
};