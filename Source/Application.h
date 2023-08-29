#include "PCH.h"
#include "Renderer.h"
#include "Sound.h"
#include "Image.h"

class Application
{
public:
	BOOL Init(HINSTANCE hInstance);
	int Run();
	void Update();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	void Increment();
	void Decrement();
	void KeyPressed(int key);
	void RetrieveDigits();
	void GetValueFromDigitArray();
	void SaveFile();
	void Loadfile();
	LRESULT CALLBACK InternalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HINSTANCE hInst = nullptr;
	HWND hWindow = nullptr;
	int m_WindowWidth = 300;
	int m_WindowHeight = 300;
	Renderer m_Renderer;

	ComPtr<IDWriteFactory> DWriteFactory;
	ComPtr<IDWriteTextFormat> TextFormat;
	SoundManager m_SoundManager;
	WavFile deathsound;
	ComPtr<ID2D1SolidColorBrush> BlackBrush;
	ComPtr<ID2D1SolidColorBrush> RedBrush;
	Image image;
	int M_DeathCount = 0;
	int m_Editfield[4] = {};
	inline BOOL Insert(int value)
	{
		if (!m_Editfield[0])
		{
			for (int i = 1; i < 4; i++)
			{
				int destIndex = i - 1;
				m_Editfield[destIndex] = m_Editfield[i];
			}
			m_Editfield[3] = value;
			return TRUE;
		}
		return FALSE;
	}
	inline BOOL BackSpace()
	{
		BOOL result = FALSE;
		for (int i = 0; i < 4; i++)
		{
			if (m_Editfield[i])
				result = TRUE;
		}
		for (int i = 2; i >= 0; i--)
		{
			int destIndex = i + 1;
			m_Editfield[destIndex] = m_Editfield[i];
		}
		m_Editfield[0] = 0;
		return result;
	}
	inline void Clear()
	{
		for (int i = 0; i < 4; i++)
		{
			m_Editfield[i] = 0;
		}
	}
};