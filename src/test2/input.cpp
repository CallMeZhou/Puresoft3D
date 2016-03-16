#include "input.h"
#include <windef.h>
#include <map>

using namespace std;

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC			((USHORT) 0x01)
#endif

#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE			((USHORT) 0x02)
#endif

#ifndef HID_USAGE_GENERIC_KEYBOARD
#define HID_USAGE_GENERIC_KEYBOARD		((USHORT) 0x06)
#endif

typedef map<HWND, Input*> MapHwndToInputInstance;
static MapHwndToInputInstance g_HWndMap;

class MapLock
{
	CRITICAL_SECTION m_lock;
public:
	MapLock(void)
	{InitializeCriticalSection(&m_lock);}
	~MapLock(void)
	{DeleteCriticalSection(&m_lock);}
	void lock(void)
	{EnterCriticalSection(&m_lock);}
	void unlock(void)
	{LeaveCriticalSection(&m_lock);}
} g_mapLock;

void Input::startup(HWND hwnd)
{
	memset(m_keyStates,  0, MAXKEY * sizeof(bool));
	memset(m_holdStates, 0, MAXKEY * sizeof(bool));

	m_sensitivity = 0.07f;
	RECT client;
	GetClientRect(hwnd, &client);
	setAbsPosBoundary(float(client.right - client.left + 1), float(client.bottom - client.top + 1));
	memset(m_relativePosition, 0, sizeof(m_relativePosition));
	memset(m_absolutePosition, 0, sizeof(m_absolutePosition));

	m_kb.usUsagePage = HID_USAGE_PAGE_GENERIC;
	m_kb.usUsage = HID_USAGE_GENERIC_KEYBOARD;
	m_kb.dwFlags = RIDEV_INPUTSINK;
	m_kb.hwndTarget = hwnd;
	RegisterRawInputDevices(&m_kb, 1, sizeof(m_kb));

	m_mouse.usUsagePage = HID_USAGE_PAGE_GENERIC;
	m_mouse.usUsage = HID_USAGE_GENERIC_MOUSE;
	m_mouse.dwFlags = RIDEV_INPUTSINK;
	m_mouse.hwndTarget = hwnd;
	RegisterRawInputDevices(&m_mouse, 1, sizeof(m_mouse));

	g_mapLock.lock();
	g_HWndMap[m_wnd = hwnd] = this;
	g_mapLock.unlock();
	m_previous = (WNDPROC)SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)WndProc);
}

void Input::shutdown(void)
{
	SetWindowLongPtr(m_wnd, GWL_WNDPROC, (LONG_PTR)m_previous);
}

void Input::frameUpdate(void)
{
	memcpy(m_holdStates, m_keyStates, sizeof(m_holdStates));
	memset(m_relativePosition, 0, sizeof(m_relativePosition));
}

void Input::handleRawInputMsg(HRAWINPUT raw)
{
	UINT dwSize;
	GetRawInputData(raw, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	m_rawBuffer.resize(dwSize);
	GetRawInputData(raw, RID_INPUT, &m_rawBuffer[0], &dwSize, sizeof(RAWINPUTHEADER));

	RAWINPUT* ri = (RAWINPUT*)&m_rawBuffer[0];

	if(ri->header.dwType == RIM_TYPEKEYBOARD)
	{
		if(ri->data.keyboard.VKey < MAXKEY)
		{
			m_keyStates[ri->data.keyboard.VKey] = !(ri->data.keyboard.Flags & RI_KEY_BREAK);
		}
	}
	else if(ri->header.dwType == RIM_TYPEMOUSE)
	{
		m_relativePosition[0] +=((float)ri->data.mouse.lLastX ) * m_sensitivity;
		m_relativePosition[1] +=((float)ri->data.mouse.lLastY ) * m_sensitivity;

		m_absolutePosition[0] += (float)ri->data.mouse.lLastX;
		m_absolutePosition[1] += (float)ri->data.mouse.lLastY;
		m_absolutePosition[0] = min(max(m_absolutePosition[0], 0), m_absolutePositionBounds[0]);
		m_absolutePosition[1] = min(max(m_absolutePosition[1], 0), m_absolutePositionBounds[1]);
	}
}

bool Input::keyDown(unsigned int key) const
{
	return m_keyStates[key];
}

bool Input::keyHeld(unsigned int key) const
{
	return (keyDown(key) && m_holdStates[key]);
}

bool Input::keyTriggered(unsigned int key) const
{
	return (keyDown(key) && !keyHeld(key));
}

void Input::setAbsPos(float x, float y)
{
	m_absolutePosition[0] = x;
	m_absolutePosition[1] = y;
}

void Input::setAbsPosBoundary(float x, float y)
{
	m_absolutePositionBounds[0] = x;
	m_absolutePositionBounds[1] = y;
}

void Input::getAbsPos(float* x, float* y) const
{
	*x = m_absolutePosition[0];
	*y = m_absolutePosition[1];
}

void Input::getRelPos(float* x, float* y) const
{
	*x = m_relativePosition[0];
	*y = m_relativePosition[1];
}

LRESULT CALLBACK Input::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	g_mapLock.lock();
	Input* pThis = g_HWndMap[hwnd];
	g_mapLock.unlock();

	if(WM_INPUT == msg)
	{
		pThis->handleRawInputMsg((HRAWINPUT)lparam);
		return 0;
	}

	return CallWindowProc(pThis->m_previous, hwnd, msg, wparam, lparam);
}
