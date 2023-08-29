#include "PCH.h"
#include "Image.h"
#include "Renderer.h"
#include "Math.h"
#include "ResourceLoader.h"

static std::vector<BYTE> FlipImage32bit(std::vector<BYTE>& Image, int pitch, int height)
{
	std::vector<BYTE> FlippedPixels;
	FlippedPixels.resize(Image.size());
	for (UINT64 row = 0; row < height; row++)
	{
		BYTE* SrcRow = Image.data() + (row * pitch);
		BYTE* DestRow = FlippedPixels.data() + (((height - 1LL) - row) * pitch);
		memcpy(DestRow, SrcRow, pitch);
	}
	return FlippedPixels;
}

inline UINT Align(UINT uLocation, UINT uAlign)
{
	return ((uLocation + (uAlign - 1)) & ~(uAlign - 1));
}

static std::vector<BYTE> FileLoader(_In_ int resource, _Out_ UINT& width, _Out_ UINT& height, _Out_ UINT& pitch)
{
	std::vector<BYTE> Pixels8Bit;
	struct BMPHeader
	{
		char bm[2];
		unsigned int bmpfilesize;
		short appspecific1;
		short appspecific2;
		unsigned int pixeldataoffset;
	} bmpHeader = {};
	struct DIBHeader
	{
		unsigned int dibSize;
		unsigned int width;
		unsigned int height;
		short colorplane;
		short bitsperpixel;
		unsigned int pixelcompression;
		unsigned int rawPixelDataSize;
	} dibHeader = {};

	ResourceLoader resLoader(resource, L"BITMAPDATA");
	resLoader.Read(bmpHeader.bm, 2);
	if (!(bmpHeader.bm[0] == 'B' && bmpHeader.bm[1] == 'M'))
		throw std::exception("Bad Bitmap");
	resLoader.Read(&bmpHeader.bmpfilesize, sizeof(bmpHeader.bmpfilesize));
	resLoader.Read(&bmpHeader.appspecific1, sizeof(bmpHeader.appspecific1));
	resLoader.Read(&bmpHeader.appspecific2, sizeof(bmpHeader.appspecific2));
	resLoader.Read(&bmpHeader.pixeldataoffset, sizeof(bmpHeader.pixeldataoffset));
	resLoader.Read(&dibHeader.dibSize, sizeof(dibHeader.dibSize));
	resLoader.Read(&dibHeader.width, sizeof(dibHeader.width));
	resLoader.Read(&dibHeader.height, sizeof(dibHeader.height));
	resLoader.Read(&dibHeader.colorplane, sizeof(dibHeader.colorplane));
	resLoader.Read(&dibHeader.bitsperpixel, sizeof(dibHeader.bitsperpixel));
	resLoader.Read(&dibHeader.pixelcompression, sizeof(dibHeader.pixelcompression));
	resLoader.Read(&dibHeader.rawPixelDataSize, sizeof(dibHeader.rawPixelDataSize));

	width = dibHeader.width;
	height = dibHeader.height;
	pitch = Align(dibHeader.width * 4, 16);

	const size_t buffersize = dibHeader.rawPixelDataSize;
	Pixels8Bit.resize(buffersize);
	resLoader.Seek(bmpHeader.pixeldataoffset);
	resLoader.Read(Pixels8Bit.data(), buffersize);

	return Pixels8Bit;
}

Image LoadBitmapImage(ID2D1DeviceContext* dc, int resource)
{
	Image image = {};

	UINT PixelWidth;
	UINT PixelHeight;
	UINT Pitch;
	std::vector<BYTE> Pixels32Bit = FileLoader(resource, PixelWidth, PixelHeight, Pitch);
	std::vector<BYTE> FlippedPixels32Bit = FlipImage32bit(Pixels32Bit, Pitch, PixelHeight);
	std::vector<UINT> PreMultiplied(FlippedPixels32Bit.size() / 4);
	memcpy(PreMultiplied.data(), FlippedPixels32Bit.data(), FlippedPixels32Bit.size());

	// modify image bytes so the pixels are premultiplied
	for (UINT j = 0; j < PixelHeight; ++j)
	{
		for (UINT i = 0; i < PixelWidth; ++i)
		{
			int index = (PixelHeight - j - 1) * PixelWidth + i;

			DWORD d = PreMultiplied[index];

			BYTE a = d >> 24;
			BYTE pmR = static_cast<BYTE>(((d & 0x00FF0000) >> 16) * a / 255);
			BYTE pmG = static_cast<BYTE>(((d & 0x0000FF00) >> 8) * a / 255);
			BYTE pmB = static_cast<BYTE>(((d & 0x000000FF)) * a / 255);
			d = pmB | (pmG << 8) | (pmR << 16) | (a << 24);

			PreMultiplied.data()[index] = d;
		}
	}


	D2D1_SIZE_U bitmapsize = {};
	bitmapsize.width = PixelWidth;
	bitmapsize.height = PixelHeight;
	image.m_Size = { (float)PixelWidth, (float)PixelHeight };

	// m_Pivot is the Center of the bitmap for positioning and rotation
	image.m_Pivot = { image.m_Size.width / 2.0f, image.m_Size.height / 2.0f };

	// Resource Bitmap
	D2D1_BITMAP_PROPERTIES bitmapprops = {};
	bitmapprops.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bitmapprops.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	bitmapprops.dpiX = 96.0f;
	bitmapprops.dpiY = 96.0f;

	HR(dc->CreateBitmap(bitmapsize, PreMultiplied.data(), Pitch, bitmapprops, image.m_Bitmap.ReleaseAndGetAddressOf()));

	return image;
}

Image CreateTombstoneImage(ID2D1Factory2* factory, ID2D1DeviceContext* dc, float size)
{
	Image image = {};
	image.m_Size = { size, size };
	image.m_Pivot = { size / 2.0f, size / 2.0f };

	// Create Shape Geometry 

	const float bottom = size * 0.48f;
	const float upperedge = size * 0.33f;
	const float topbegin = size * 0.15f;
	const float wideedge = size * 0.45f;
	const float thinedge = size * 0.35f;
	const float linewidth = size * 0.01f;
	const float linewidth2 = size * 0.08f;
	const float line1Height = size * 0.22f;
	const float line2Height = size * 0.15f;
	ComPtr<ID2D1PathGeometry> BaseGeometry;
	{
		ComPtr<ID2D1GeometrySink> Sink;
		HR(factory->CreatePathGeometry(BaseGeometry.ReleaseAndGetAddressOf()));
		HR(BaseGeometry->Open(Sink.ReleaseAndGetAddressOf()));
		Sink->SetFillMode(D2D1_FILL_MODE_WINDING);
		D2D1_POINT_2F p1 = { -wideedge, upperedge };
		D2D1_POINT_2F p2 = { wideedge, upperedge };
		D2D1_POINT_2F p3 = { wideedge, bottom };
		D2D1_POINT_2F p4 = { -wideedge, bottom };
		Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
		Sink->AddLine(p2);
		Sink->AddLine(p3);
		Sink->AddLine(p4);
		Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		HR(Sink->Close());
	}
	ComPtr<ID2D1PathGeometry> TombGeometry;
	{
		ComPtr<ID2D1GeometrySink> Sink;
		HR(factory->CreatePathGeometry(TombGeometry.ReleaseAndGetAddressOf()));
		HR(TombGeometry->Open(Sink.ReleaseAndGetAddressOf()));
		Sink->SetFillMode(D2D1_FILL_MODE_WINDING);
		D2D1_POINT_2F p1 = { -thinedge, -topbegin };
		D2D1_POINT_2F p2 = { thinedge, -topbegin };
		D2D1_POINT_2F p3 = { thinedge, upperedge };
		D2D1_POINT_2F p4 = { -thinedge, upperedge };
		Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
		Sink->AddArc(D2D1::ArcSegment(p2, D2D1::SizeF(thinedge, thinedge), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_LARGE));
		Sink->AddLine(p3);
		Sink->AddLine(p4);
		Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		HR(Sink->Close());
	}
	ComPtr<ID2D1PathGeometry> Line1Geometry;
	{
		ComPtr<ID2D1GeometrySink> Sink;
		HR(factory->CreatePathGeometry(Line1Geometry.ReleaseAndGetAddressOf()));
		HR(Line1Geometry->Open(Sink.ReleaseAndGetAddressOf()));
		Sink->SetFillMode(D2D1_FILL_MODE_WINDING);
		D2D1_POINT_2F p1 = { -linewidth, -line1Height };
		D2D1_POINT_2F p2 = { linewidth, -line1Height };

		Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
		Sink->AddLine(p2);
		Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		HR(Sink->Close());
	}
	ComPtr<ID2D1PathGeometry> Line2Geometry;
	{
		ComPtr<ID2D1GeometrySink> Sink;
		HR(factory->CreatePathGeometry(Line2Geometry.ReleaseAndGetAddressOf()));
		HR(Line2Geometry->Open(Sink.ReleaseAndGetAddressOf()));
		Sink->SetFillMode(D2D1_FILL_MODE_WINDING);
		D2D1_POINT_2F p1 = { -linewidth2, -line2Height };
		D2D1_POINT_2F p2 = { linewidth2, -line2Height };

		Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
		Sink->AddLine(p2);
		Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		HR(Sink->Close());
	}

	ComPtr<ID2D1SolidColorBrush> BlackBrush;
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), BlackBrush.ReleaseAndGetAddressOf()));
	ComPtr<ID2D1SolidColorBrush> DarkGrayBrush;
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.2f, 0.2f, 0.23f, 1.0f), DarkGrayBrush.ReleaseAndGetAddressOf()));
	ComPtr<ID2D1SolidColorBrush> LighterGrayBrush;
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.25f, 0.25f, 0.28f, 1.0f), LighterGrayBrush.ReleaseAndGetAddressOf()));

	ComPtr<ID2D1BitmapRenderTarget> BitmapRenderTarget;

	HR(dc->CreateCompatibleRenderTarget(image.m_Size, &BitmapRenderTarget));
	BitmapRenderTarget->BeginDraw();

	D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Translation(image.m_Size.width / 2.0f, image.m_Size.height / 2.0f);

	BitmapRenderTarget->SetTransform(transform);
	BitmapRenderTarget->FillGeometry(BaseGeometry.Get(), DarkGrayBrush.Get());
	BitmapRenderTarget->FillGeometry(TombGeometry.Get(), LighterGrayBrush.Get());
	BitmapRenderTarget->DrawGeometry(BaseGeometry.Get(), BlackBrush.Get());
	BitmapRenderTarget->DrawGeometry(TombGeometry.Get(), BlackBrush.Get());
	BitmapRenderTarget->DrawGeometry(Line1Geometry.Get(), BlackBrush.Get(), size * 0.03f);
	BitmapRenderTarget->DrawGeometry(Line2Geometry.Get(), BlackBrush.Get(), size * 0.03f);

	HR(BitmapRenderTarget->EndDraw());
	HR(BitmapRenderTarget->GetBitmap(image.m_Bitmap.ReleaseAndGetAddressOf()));

	return image;
}
