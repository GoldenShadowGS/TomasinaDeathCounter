#include "PCH.h"
#include "Shape.h"
#include "Renderer.h"

void Shape::Init(ID2D1Factory2* pD2DFactory, ID2D1DeviceContext* dc, float size)
{
	m_Size = { size, size };
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
		HR(pD2DFactory->CreatePathGeometry(BaseGeometry.ReleaseAndGetAddressOf()));
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
		HR(pD2DFactory->CreatePathGeometry(TombGeometry.ReleaseAndGetAddressOf()));
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
		HR(pD2DFactory->CreatePathGeometry(Line1Geometry.ReleaseAndGetAddressOf()));
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
		HR(pD2DFactory->CreatePathGeometry(Line2Geometry.ReleaseAndGetAddressOf()));
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

	HR(dc->CreateCompatibleRenderTarget(m_Size, &BitmapRenderTarget));
	BitmapRenderTarget->BeginDraw();

	D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Translation(m_Size.width / 2.0f, m_Size.height / 2.0f);

	BitmapRenderTarget->SetTransform(transform);
	BitmapRenderTarget->FillGeometry(BaseGeometry.Get(), DarkGrayBrush.Get());
	BitmapRenderTarget->FillGeometry(TombGeometry.Get(), LighterGrayBrush.Get());
	BitmapRenderTarget->DrawGeometry(BaseGeometry.Get(), BlackBrush.Get());
	BitmapRenderTarget->DrawGeometry(TombGeometry.Get(), BlackBrush.Get());
	BitmapRenderTarget->DrawGeometry(Line1Geometry.Get(), BlackBrush.Get(), size * 0.03f);
	BitmapRenderTarget->DrawGeometry(Line2Geometry.Get(), BlackBrush.Get(), size * 0.03f);

	HR(BitmapRenderTarget->EndDraw());
	HR(BitmapRenderTarget->GetBitmap(Bitmap.ReleaseAndGetAddressOf()));
}

void Shape::Draw(ID2D1DeviceContext* dc, const D2D1::Matrix3x2F& transform)
{
	D2D1::Matrix3x2F bitmapcenter = D2D1::Matrix3x2F::Translation(-m_Size.width / 2.0f, -m_Size.height / 2.0f);
	D2D1_RECT_F rect = D2D1::RectF(0.0f, 0.0f, m_Size.width, m_Size.height);
	dc->SetTransform(bitmapcenter * transform);
	dc->DrawBitmap(Bitmap.Get(), rect);
}