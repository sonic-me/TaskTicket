// Task Ticket - Ultra-lean and Ultra-simple Win32 desktop application for daily routines.
// Copyright (C) 2026 Marc-Daniel DALEBA
// License: GPLv3
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>

#include <timeapi.h>
#include <shellapi.h>

#include <iostream>
#include <string>
#include <functional>

struct App {
	HWND hwnd = nullptr;
	HDC hdc = nullptr;
	int width = 0, height = 0;
	bool running = false;
	bool inputs[256] = { false };
	bool mouse_down = false;
	bool drag_locked = false;
	std::function<void(std::string)> onDrag;

	int InitApplication(HINSTANCE hInst, std::wstring Title, int Width, int Height);
	bool IsRunning();
	void Close();
	void HandleMessages(MSG msg);
};
