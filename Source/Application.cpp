#include "PCH.h"
#include "Application.h"
#include "Resource.h"

const WCHAR* gTitle = L"Cooking Counter";
const WCHAR* gWindowClass = L"CounterWindowClass";

static ATOM RegisterWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = Application::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = gWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALLICON));
	return RegisterClassExW(&wcex);
}

BOOL Application::Init(HINSTANCE hInstance)
{
	hInst = hInstance;
	RegisterWindowClass(hInstance);

	const DWORD style = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;  // WS_POPUP
	const DWORD exstyle = WS_EX_NOREDIRECTIONBITMAP;

	RECT winRect = { 0, 0, m_WindowWidth, m_WindowHeight };
	AdjustWindowRectEx(&winRect, style, false, exstyle);

	hWindow = CreateWindowExW(exstyle, gWindowClass, gTitle, style, CW_USEDEFAULT, 0,
		winRect.right - winRect.left, winRect.bottom - winRect.top, nullptr, nullptr, hInst, this);

	if (!hWindow)
		return FALSE;

	//Sound Init
	m_SoundManager.Init(1); // channels
	deathsound.Load(SOUND_COUNTER);

	// Graphics Resource Init
	m_Renderer.Init(hWindow);
	ID2D1Factory2* factory = m_Renderer.GetFactory();
	ID2D1DeviceContext* dc = m_Renderer.GetDeviceContext();
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), BlackBrush.ReleaseAndGetAddressOf()));
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.1f, 0.1f, 0.1f, 1.0f), RedBrush.ReleaseAndGetAddressOf()));

	image = CreateTombstoneImage(factory, dc, 140.0f);
	image = LoadBitmapImage(dc, BITMAP_FOOD);

	// Text Init
	HR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(DWriteFactory.ReleaseAndGetAddressOf())));
	HR(DWriteFactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 36.0f, L"en-us", TextFormat.ReleaseAndGetAddressOf()));
	TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

	Loadfile();

	ShowWindow(hWindow, SW_SHOW);
	UpdateWindow(hWindow);

	return TRUE;
}


int Application::Run()
{
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

void Application::Update()
{
	// Skip Drawing if Window is Minimized
	if (!IsIconic(hWindow))
	{
		ID2D1DeviceContext* dc = m_Renderer.GetDeviceContext();
		dc->BeginDraw();

		dc->Clear();

		D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Translation(m_WindowWidth / 2.0f, m_WindowWidth / 2.0f);

		D2D1::Matrix3x2F bitmapcenter = D2D1::Matrix3x2F::Translation(-image.m_Size.width / 2.0f, -image.m_Size.height / 2.0f);
		D2D1_RECT_F rect = D2D1::RectF(0.0f, 0.0f, image.m_Size.width, image.m_Size.height);
		dc->SetTransform(bitmapcenter * transform);
		dc->DrawBitmap(image.m_Bitmap.Get(), rect);

		D2D1_RECT_F textrect = { 0, image.m_Size.width * 0.34f, image.m_Size.width, image.m_Size.width };

		M_DeathCount;
		WCHAR buffer[5];
		swprintf_s(buffer, 5, L"%i", M_DeathCount);
		dc->DrawTextW(buffer, lstrlenW(buffer), TextFormat.Get(), textrect, RedBrush.Get());

		HR(dc->EndDraw());
		HR(m_Renderer.GetSwapChain()->Present(1, 0));
	}
}

void Application::Increment()
{
	M_DeathCount++;
	if (M_DeathCount > 9999)
		M_DeathCount = 9999;

	m_SoundManager.Play(deathsound, 1.0f, 1.0f);
	RetrieveDigits();
	Update();
}

void Application::Decrement()
{
	M_DeathCount--;
	if (M_DeathCount < 0)
		M_DeathCount = 0;
	RetrieveDigits();
	Update();
}

void Application::KeyPressed(int key)
{

	if (key >= '0' && key <= '9')
	{
		Insert(key - 48);
	}
	else if (key == 8) // 8 BackSpace
	{
		BackSpace();
	}
	GetValueFromDigitArray();
	Update();

}

void Application::RetrieveDigits()
{
	m_Editfield[0] = (M_DeathCount / 1000) % 10;
	m_Editfield[1] = (M_DeathCount / 100) % 10;
	m_Editfield[2] = (M_DeathCount / 10) % 10;
	m_Editfield[3] = (M_DeathCount) % 10;

}

void Application::GetValueFromDigitArray()
{
	M_DeathCount = 0;
	M_DeathCount += m_Editfield[0] * 1000;
	M_DeathCount += m_Editfield[1] * 100;
	M_DeathCount += m_Editfield[2] * 10;
	M_DeathCount += m_Editfield[3];
}

void Application::SaveFile()
{
	std::ofstream file;
	file.open("Counter.txt");
	if (!file.is_open())
		return;
	file << M_DeathCount;
	file.close();
}

void Application::Loadfile()
{
	std::ifstream file;
	file.open("Counter.txt");
	if (!file.is_open())
		return;
	file >> M_DeathCount;
	file.close();
	if (M_DeathCount > 9999)
		M_DeathCount = 9999;
	if (M_DeathCount < 0)
		M_DeathCount = 0;
	RetrieveDigits();
}

LRESULT CALLBACK Application::InternalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int MouseDelay = 0;
	switch (message)
	{
	case WM_PAINT:
	{
		Update();
		PAINTSTRUCT ps;
		BeginPaint(hWindow, &ps);
		EndPaint(hWindow, &ps);
	}
	break;
	case WM_LBUTTONDOWN:
	{
		Increment();
	}
	break;
	case WM_RBUTTONDOWN:
	{
		Decrement();
	}
	break;
	case WM_KEYDOWN:
	{
		if (wParam == VK_DELETE)
		{
			Clear();
			GetValueFromDigitArray();
			Update();
		}
	}
	break;
	case WM_CHAR:
	{
		KeyPressed((int)wParam);
	}
	break;
	case WM_DESTROY:
		SaveFile();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static Application* app = nullptr;
	if (app)
		return app->InternalWndProc(hWnd, message, wParam, lParam);
	else
	{
		if (message == WM_CREATE)
		{
			app = reinterpret_cast<Application*>(((CREATESTRUCTW*)lParam)->lpCreateParams);
			return 0;
		}
		else
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
}
