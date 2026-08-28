// Task Ticket - Ultra-lean and Ultra-simple Win32 desktop application for daily routines.
// Copyright (C) 2026 Marc-Daniel DALEBA
// License: GPLv3
#include "App.h"

#pragma comment(lib, "winmm.lib")

LRESULT CALLBACK WindowProc(HWND h, UINT m, WPARAM w, LPARAM l);

int App::InitApplication(HINSTANCE hInst, std::wstring Title, int Width, int Height)
{
	timeBeginPeriod(1);
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	WNDCLASS wc = {};
	wc.style = CS_OWNDC;
	wc.hInstance = hInst;
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = Title.c_str();
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) {
		return -1;
	}
	width = Width;
	height = Height;
	RECT rc = { 0,0, width, height };
	AdjustWindowRect(&rc, WS_POPUPWINDOW, FALSE);
	hwnd = CreateWindowEx(
		0,
		Title.c_str(),
		Title.c_str(),
		WS_POPUPWINDOW,
		10,
		10,
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL,
		NULL,
		hInst,
		NULL
	);
	if (!hwnd)
		return -1;
	DragAcceptFiles(hwnd, TRUE);
	ShowWindow(hwnd, SW_NORMAL);
	hdc = GetDC(hwnd);
	running = true;
}
void App::HandleMessages(MSG msg)
{
	switch (msg.message) {
	case WM_QUIT:
		running = false;
		break;
	case WM_KEYDOWN: {
		bool isRepeat = (msg.lParam & (1 << 30)) != 0;
		if (!isRepeat) {
			inputs[msg.wParam] = true;
		}
		break;
	}
	case WM_LBUTTONDOWN:
		mouse_down = true;
		break;
	case WM_DROPFILES: {
		HDROP hdrop = (HDROP)msg.wParam;
		UINT fileCount = DragQueryFileW(hdrop, 0xFFFFFFFF, NULL, 0);
		if (fileCount > 0) {
			char filepath[MAX_PATH];
			DragQueryFileA(hdrop, 0, filepath, MAX_PATH);
			std::string droppedPath(filepath);
			if (onDrag)
				onDrag(droppedPath);
		}
		DragFinish(hdrop);
		break;
	}
	default:
		break;
	}
}
bool App::IsRunning()
{
	mouse_down = false;
	memset(inputs, '\0', sizeof(inputs));
	MSG msg = {};
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		HandleMessages(msg);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return running;
}
void App::Close()
{
	ReleaseDC(hwnd, hdc);
	DestroyWindow(hwnd);
	timeEndPeriod(1);
}

LRESULT CALLBACK WindowProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
	switch (m) {
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_NCHITTEST: {
		const int top_height = 40;
		RECT rect;
		POINT pt;

		GetClientRect(h, &rect);
		pt.x = GET_X_LPARAM(l);
		pt.y = GET_Y_LPARAM(l);
		ScreenToClient(h, &pt);
		if (!app.drag_locked && pt.y < top_height && pt.x < rect.right - (75 + 20 + 10))
		{
    		return HTCAPTION;
		}
		return HTCLIENT;
	}
	default:
		return DefWindowProc(h, m, w, l);
	}
	return 0;
}
