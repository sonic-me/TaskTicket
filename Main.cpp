/*

	TaskTicket

	A lean and focus todo app to be focused
	on what you are supposed to do daily.

	AUTHOR Marc-Daniel DALEBA
	DATE 2026-05-28
	LICENSE GPLv3

	DESC
		Ultra-lean and Ultra-simple Win32 desktop
		application for daily routines.

	NOTES
		- For this app, I wanted to create a Linux/Apple
		terminal look. I used Gruvbox for the colors
		(https://terminalcolors.com/)

*/
#include <string>
#include <chrono>

#include "App.h"
#include "Font.h"
#include "Tasks.h"
#include "Button.h"

std::wstring GetLocalTime();
void Update();
void Render();

void UpdateWindowSize();
void closeButtonCall();
void miniButtonCall();
void lockButtonCall();
void appOnDrag(std::string file);

constexpr int pad_top = 10;
constexpr int pad_left = 20;
constexpr int pad_bottom = 10;
constexpr int pad_right = 20;
constexpr int task_gap = 10;
App app;
HBRUSH bgBrush;
Font fontText;
Tasks tasks;
Button closeButton;
Button miniButton;
Button lockButton;

//int main()
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpCmd, int nShow)
{
	app.InitApplication(GetModuleHandle(NULL), L"TaskTicket", 480, 640);
	app.onDrag = appOnDrag;
	bgBrush = CreateSolidBrush(RGB(0x28, 0x28, 0x28));
	fontText.LoadFont(app.hdc, L"Cascadia Mono", 5, 12.0f);

	closeButton.Create(&app, app.width - 45 - 20 + 10, 15 + 10, 10, { 239, 68, 68 });
	miniButton.Create(&app, app.width - 15 - 20 + 10, 15 + 10, 10, { 234, 179, 8 });
	lockButton.Create(&app, app.width - 75 - 20 + 10, 15 + 10, 10, { 34, 197, 94 });

	closeButton.callback = closeButtonCall;
	miniButton.callback = miniButtonCall;
	lockButton.callback = lockButtonCall;

	UpdateWindowSize();

	while (app.IsRunning()) {
		Update();
		Render();
		Sleep(100); // 60FPS
	}

	DeleteBrush(bgBrush);
	app.Close();
	return 0;
}

static int kstate_up = 0;
static int kstate_down = 0;

void Update()
{
	closeButton.Update();
	miniButton.Update();
	lockButton.Update();

	size_t count = tasks.mTasks.size();
	if (app.inputs[VK_UP]) { tasks.selected -= 1; }
	if (app.inputs[VK_DOWN]) { tasks.selected += 1; }
	if (tasks.selected < 0) tasks.selected = 0;
	if (tasks.selected >= count) tasks.selected = count - 1;
	
	if (app.inputs[VK_RETURN]) {
		tasks.ToggleTaskState(tasks.selected);
	}
}
void DrawButton(HDC hdc, Button& button)
{
	COLORREF c = RGB(button.color.r, button.color.g, button.color.b);
	HBRUSH hbr = CreateSolidBrush(c);
	HBRUSH oldhbr = SelectBrush(hdc, hbr);
	Ellipse(hdc,
		button.x - button.radius,
		button.y - button.radius,
		button.x + button.radius,
		button.y + button.radius);
	SelectBrush(hdc, oldhbr);
	DeleteObject(hbr);
}
void Draw(HDC hdc)
{
	Color textcolor;
	Color bgcolor;
	HFONT oldFont = SelectFont(hdc, fontText.hfont);
	SetTextColor(hdc, RGB(0xEB, 0xDB, 0xB2));
	SetBkColor(hdc, RGB(0x28, 0x28, 0x28));
	std::string titlebar = "Task Ticket";
	if (!tasks.filename.empty()) {
		bool ok = false;
		int pos = tasks.filename.find_last_of('\\');
		if (pos != std::string::npos) {
			titlebar += " - " + tasks.filename.substr(pos+1);
			ok = true;
		}
		pos = tasks.filename.find_last_of('/');
		if (pos != std::string::npos && !ok) {
			titlebar += " - " + tasks.filename.substr(pos+1);
			ok = true;
		}
		if (!ok) {
			titlebar += " - " + tasks.filename;
		}
	}
	TextOutA(hdc, pad_left, pad_top, titlebar.c_str(), titlebar.size());
	int line = 0;
	for (const auto& t : tasks.mTasks) {
		std::string str = t.name;
		// check color
		if (t.check) {
			textcolor = { 0x7D, 0x72, 0x65 };
			str = "[x] " + str;
		} else {
			textcolor = { 0xEB, 0xDB, 0xB2 };
			str = "[ ] " + str;
		}
		// selected
		if (line == tasks.selected) {
			textcolor = { 0x28, 0x28, 0x28 };
			bgcolor = { 0xEB, 0xDB, 0xB2 };
		} else {
			bgcolor = { 0x28, 0x28, 0x28 };
		}
		// draw
		SetTextColor(hdc, RGB(textcolor.r, textcolor.g, textcolor.b));
		SetBkColor(hdc, RGB(bgcolor.r, bgcolor.g, bgcolor.b));
		TextOutA(hdc, pad_left, pad_top + 30 + task_gap + 27 * line, str.c_str(), str.size());
		line += 1;
	}

	// time
	{
		int x = app.width - 95 - pad_right;
		int y = pad_top + 30 + task_gap + 27 * line;
		SetTextColor(hdc, RGB(0xEB, 0xDB, 0xB2));
		SetBkColor(hdc, RGB(0x28, 0x28, 0x28));
		std::wstring time = GetLocalTime();
		TextOut(hdc, x, y, time.c_str(), time.size());
		SelectFont(hdc, oldFont);
	}

	HPEN hPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
	HPEN oldPen = SelectPen(hdc, hPen);
	DrawButton(hdc, lockButton);
	DrawButton(hdc, closeButton);
	DrawButton(hdc, miniButton);
	SelectPen(hdc, oldPen);
	DeleteObject(hPen);
}
void Render()
{
	RECT rc;
	GetClientRect(app.hwnd, &rc);
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	HDC memDC = CreateCompatibleDC(app.hdc);
	HBITMAP backbuffer = CreateCompatibleBitmap(app.hdc, width, height);
	HBITMAP oldBitmap = SelectBitmap(memDC, backbuffer);

	FillRect(memDC, &rc, bgBrush);
	Draw(memDC);
	BitBlt(app.hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

	SelectBitmap(memDC, oldBitmap);
	DeleteObject(backbuffer);
	DeleteDC(memDC);
}

std::wstring GetLocalTime()
{
	auto now = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
	std::tm localTime;
	localtime_s(&localTime, &currentTime);
	std::wstringstream wss;
	wss << std::put_time(&localTime, L"%H:%M:%S");
	return wss.str();
}

void UpdateWindowSize()
{
	int height = pad_top + 30 + task_gap + 27 * tasks.mTasks.size() + pad_bottom + 5;  // task size
	height += 27; // clock
	RECT rc = {
		0, 0, app.width,
		height
	};
	AdjustWindowRect(&rc, WS_POPUPWINDOW, FALSE);
	SetWindowPos(
		app.hwnd,
		HWND_TOPMOST,
		0,
		0,
		rc.right - rc.left,
		rc.bottom - rc.top,
		SWP_NOMOVE
	);
}

void appOnDrag(std::string file)
{
	tasks.LoadTaskFile(file);
	UpdateWindowSize();
}

void lockButtonCall()
{
    app.drag_locked = !app.drag_locked;
}

void closeButtonCall()
{
	app.running = false;
}
void miniButtonCall()
{
	ShowWindow(app.hwnd, SW_MINIMIZE);
}
