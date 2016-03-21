#pragma once
#include <windows.h>
#include <vector>

const size_t MAXKEY = VK_OEM_CLEAR + 1;

class Input
{
public:
	void startup(HWND hwnd);
	void shutdown(void);

	void handleRawInputMsg(HRAWINPUT raw);
	void frameUpdate(void);

	bool keyDown(unsigned int key) const;
	bool keyHeld(unsigned int key) const;
	bool keyTriggered(unsigned int key) const;

	void setAbsPos(float x, float y);
	void setAbsPosBoundary(float x, float y);
	void getAbsPos(float* x, float* y) const;
	void getRelPos(float* x, float* y) const;

private:
	std::vector<unsigned char> m_rawBuffer;

	RAWINPUTDEVICE m_kb;

	bool m_keyStates[MAXKEY];
	bool m_holdStates[MAXKEY];

	RAWINPUTDEVICE m_mouse;

	bool  m_lastXYGot;
	float m_lastXY[2];
	float m_sensitivity;
	float m_absolutePosition[2];
	float m_absolutePositionBounds[2];
	float m_relativePosition[2];

	HWND m_wnd;
	WNDPROC m_previous;
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
};

