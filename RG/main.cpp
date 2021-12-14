#include <windows.h>
#include <fmod.h>
#include <gdiplus.h>
#include "Resource.h"
#pragma comment(lib, "fmodex_vc.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "msimg32.lib")
#define DEFAULTSTRPOS	83
#define SYNC			5 // SYNC <= 2 [불안정], SYNC >= 4 [안정] SYNC == 5 [권장]

using namespace			Gdiplus;

int				g_paintpoint[6];
int				g_hit;
int				g_game_status;
int				NowTime = 0;
enum SOUNDKIND			{ SD_1 = 0 };
enum EFFSOUNDKIND		{ EFFSD_1 = 0 };
RECT				Init = { 0, 560, DEFAULTSTRPOS * 6, 590 };
HWND				hWndMain;
HINSTANCE			g_hInst;
ULONG_PTR			g_gpToken;
GdiplusStartupInput		g_gpsi;
FMOD_SYSTEM			*g_psystem;
FMOD_SOUND			*g_psound[1];
FMOD_CHANNEL			*g_pChannel[1];
FMOD_SYSTEM			*effg_psystem;
FMOD_SOUND			*effg_psound[1];
FMOD_CHANNEL			*effg_pchannel[1];

void MyLine(HDC hdc, int x1, int y1, int x2, int y2);
void MyTextOut(HDC hdc, int x, int y, LPCTSTR str);
void CALLBACK DelPaintPoint(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);
void CALLBACK UpdateTime(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);
DWORD WINAPI DropNote(LPVOID rail);
void drawByDBuffering(HWND hWnd, HDC hdc, int x, int y);
void drawByDC(HWND hWnd, HDC hMemDC, int x, int y);
void DrawGame(HWND hWnd, HDC hdc);
void Menu(HDC hdc);
void Verify(int RainNum);
void soundsetup();
void effsoundsetup();
void playsound(SOUNDKIND esound);
void effsoundoff();
void effplaysound(EFFSOUNDKIND esound);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LPCTSTR	IpszClass = TEXT("「리듬게임」MP3 2021");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPSTR IpszCmdParam, int nCmdShow)
{
	HWND		hWnd;
	MSG		Message;
	WNDCLASS	WndClass;

	g_hInst = hInstance;
	
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON));
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = IpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = (CS_HREDRAW | CS_VREDRAW);
	RegisterClass(&WndClass);

	hWnd = CreateWindow(IpszClass, IpszClass, (WS_CAPTION | WS_SYSMENU), 
		CW_USEDEFAULT, CW_USEDEFAULT, 500, 650, NULL, (HMENU)NULL, g_hInst, NULL);
	ShowWindow(hWnd, nCmdShow);

	if(GdiplusStartup(&g_gpToken, &g_gpsi, NULL) != Ok) return 0;
	while(GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	
	GdiplusShutdown(g_gpToken);
	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage,
		WPARAM wParam, LPARAM IParam)
{
	HDC		hdc;
	PAINTSTRUCT	ps;
	static HBRUSH	hGray, hBlack;
	static HBRUSH	NowBrush;

	FMOD_System_Update(g_psystem);
	for(int i = 0; i < 6; i++) g_paintpoint[i] = 1;
	
	switch(iMessage)
	{
		case WM_CREATE:
			int x, y, width, height;
			RECT rtDesk, rtWindow;

			GetWindowRect(GetDesktopWindow(), &rtDesk);
			GetWindowRect(hWnd, &rtWindow);
 
			width = rtWindow.right - rtWindow.left;
			height = rtWindow.bottom - rtWindow.top;
 
			x = (rtDesk.right - width) / 2;
			y = (rtDesk.bottom - height) / 2;
 
			MoveWindow(hWnd, x, y, width, height, TRUE);

			hGray = CreateSolidBrush(RGB(211, 211, 211));
			hBlack = CreateSolidBrush(RGB(0, 0, 0));

			SetClassLong(hWnd, GCL_HBRBACKGROUND, (LONG)hGray);
			InvalidateRect(hWnd, NULL, TRUE);

			soundsetup();
			effsoundsetup();

			hWndMain = hWnd;
			return 0;

		case WM_KEYDOWN:
			if(!g_game_status)
			{
				g_game_status++;
				SetClassLong(hWnd, GCL_HBRBACKGROUND, (LONG)hBlack);
				playsound(SD_1);
				InvalidateRect(hWnd, NULL, TRUE);
				SetTimer(hWnd, 11, 700, UpdateTime); /* 호출 주기가 짧을수록 세밀한 조작 가능하나 불안정.
									uElapse == 750 [권장] */
				break;
			}

			switch(wParam)
			{
				case 0x35:
					effplaysound(EFFSD_1);
					g_paintpoint[0] = 5;
					Verify(5);
					InvalidateRect(hWnd, &Init, FALSE);
					SetTimer(hWnd, 5, 1000, DelPaintPoint);
					break;

				case 0x36:
					effplaysound(EFFSD_1);
					g_paintpoint[1] = 6;
					Verify(6);
					InvalidateRect(hWnd, &Init, FALSE);
					SetTimer(hWnd, 6, 1000, DelPaintPoint);
					break;

				case 0x37:
					effplaysound(EFFSD_1);
					g_paintpoint[2] = 7;
					Verify(7);
					InvalidateRect(hWnd, &Init, FALSE);
					SetTimer(hWnd, 7, 1000, DelPaintPoint);
					break;

				case 0x38:
					effplaysound(EFFSD_1);
					g_paintpoint[3] = 8;
					Verify(8);
					InvalidateRect(hWnd, &Init, FALSE);
					SetTimer(hWnd, 8, 1000, DelPaintPoint);
					break;

				case 0x39:
					effplaysound(EFFSD_1);
					g_paintpoint[4] = 9;
					Verify(9);
					InvalidateRect(hWnd, &Init, FALSE);
					SetTimer(hWnd, 9, 1000, DelPaintPoint);
					break;

				case 0x30:
					effplaysound(EFFSD_1);
					g_paintpoint[5] = 0;
					Verify(10);
					InvalidateRect(hWnd, &Init, FALSE);
					SetTimer(hWnd, 10, 1000, DelPaintPoint);
					break;
			}

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			DrawGame(hWnd, hdc);
			EndPaint(hWnd, &ps);
			return 0;

		case WM_DESTROY:
			FMOD_Sound_Release(g_psound[0]);
			FMOD_Sound_Release(effg_psound[0]);

			FMOD_System_Close(g_psystem);
			FMOD_System_Release(g_psystem);

			FMOD_System_Close(effg_psystem);
			FMOD_System_Release(effg_psystem);

			DeleteObject(hGray);
			DeleteObject(hBlack);
			PostQuitMessage(0);

			return 0;
	}
	return (DefWindowProc(hWnd, iMessage, wParam, IParam));
}

void MyLine(HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
}

void MyTextOut(HDC hdc, int x, int y, LPCTSTR str) { TextOut(hdc, x, y, str, lstrlen(str)); }

void CALLBACK DelPaintPoint(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	for(int i = 0; i < 5; i++) g_paintpoint[i] = 1;
	KillTimer(hWnd, 1);
	InvalidateRect(hWnd, &Init, TRUE); 
}

void CALLBACK UpdateTime(HWND hWnd, UINT iMessage, UINT idEvent, DWORD dwTime)
{
	TCHAR	str[128];
	DWORD	ThreadID;
	DWORD	VALUE5 = 5;
	DWORD	VALUE6 = 6;
	DWORD	VALUE7 = 7;
	DWORD	VALUE8 = 8;
	DWORD	VALUE9 = 9;
	DWORD	VALUE0 = 0;

	NowTime += 1;
	switch(NowTime)
	{
		case 4:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE5, 0, &ThreadID));
			break;

		case 9:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE8, 0, &ThreadID));
			break;

		case 14:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE0, 0, &ThreadID));
			break;

		case 19:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE7, 0, &ThreadID));
			break;

		case 24:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE6, 0, &ThreadID));
			break;

		case 29:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE8, 0, &ThreadID));
			break;

		case 32:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE8, 0, &ThreadID));
			break;

		case 33:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE8, 0, &ThreadID));
			break;

		case 38:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE6, 0, &ThreadID));
			break;

		case 39:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE5, 0, &ThreadID));
			break;

		case 40:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE0, 0, &ThreadID));
			break;

		case 41:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE9, 0, &ThreadID));
			break;

		case 44:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE6, 0, &ThreadID));
			break;

		case 46:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE9, 0, &ThreadID));
			break;

		case 48:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE8, 0, &ThreadID));
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE7, 0, &ThreadID));
			break;

		case 52:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE6, 0, &ThreadID));
			break;

		case 55:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE9, 0, &ThreadID));
			break;

		case 60:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE7, 0, &ThreadID));
			break;

		case 62:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE9, 0, &ThreadID));
			break;

		case 67:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE6, 0, &ThreadID));
			break;

		case 72:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE5, 0, &ThreadID));
			break;

		case 77:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE6, 0, &ThreadID));
			break;

		case 78:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE0, 0, &ThreadID));
			break;

		case 79:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE9, 0, &ThreadID));
			break;

		case 82:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE0, 0, &ThreadID));
			break;

		case 84:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE9, 0, &ThreadID));
			break;

		case 86:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE8, 0, &ThreadID));
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE7, 0, &ThreadID));
			break;

		case 90:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE6, 0, &ThreadID));
			break;

		case 95:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE5, 0, &ThreadID));
			break;

		case 100:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE7, 0, &ThreadID));
			break;

		case 105:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE5, 0, &ThreadID));
			break;

		case 110:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE7, 0, &ThreadID));
			break;

		case 115:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE8, 0, &ThreadID));
			break;

		case 120:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE0, 0, &ThreadID));
			break;

		case 125:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE8, 0, &ThreadID));
			break;

		case 130:
			CloseHandle(CreateThread(NULL, 0, DropNote, &VALUE0, 0, &ThreadID));
			break;

		case 132:
			wsprintf(str, TEXT("GREAT: %d\r\nMISS: %d"), g_hit, 38 - g_hit);
			if(MessageBox(hWnd, str, TEXT("스코어"), MB_RETRYCANCEL) == IDRETRY)
			{
				g_game_status = 1; // TODO: 초기화 후 재시작
				NowTime = 0;
			}
			else SendMessage(hWnd, WM_CLOSE, 0, 0);
	}
}

DWORD WINAPI DropNote(LPVOID rail)
{
	HDC hdc;

	hdc = GetDC(hWndMain);

	switch(*(DWORD *)rail)
	{
		case 5:
			for(int i = 20; i < 580;)
			{
				drawByDBuffering(hWndMain, hdc, -4, i);
				i += SYNC;
			}
			break;

		case 6:
			for(int i = 20; i < 580;)
			{
				drawByDBuffering(hWndMain, hdc, 79, i);
				i += SYNC;
			}
			break;

		case 7:
			for(int i = 20; i < 580;)
			{
				drawByDBuffering(hWndMain, hdc, 162, i);
				i += SYNC;
			}
			break;

		case 8:
			for(int i = 20; i < 580;)
			{
				drawByDBuffering(hWndMain, hdc, 245, i);
				i += SYNC;
			}
			break;

		case 9:
			for(int i = 20; i < 580;)
			{
				drawByDBuffering(hWndMain, hdc, 328, i);
				i += SYNC;
			}
			break;

		case 0:
			for(int i = 20; i < 580;)
			{
				drawByDBuffering(hWndMain, hdc, 410, i);
				i += SYNC;
			}
	}
	InvalidateRect(hWndMain, NULL, TRUE);
	ReleaseDC(hWndMain, hdc);
	return 0;
}

void drawByDBuffering(HWND hWnd, HDC hdc,int x, int y)
{
	HDC hMemDC;
	RECT windowRect;
	HBITMAP bitmap;

	GetClientRect(hWnd, &windowRect);

	hMemDC = CreateCompatibleDC(hdc);
	bitmap = CreateCompatibleBitmap(hdc, windowRect.right, windowRect.bottom);

	SelectObject(hMemDC, bitmap);

	drawByDC(hWnd, hMemDC, x, y);

	TransparentBlt(hdc, 0, 0, windowRect.right, windowRect.bottom,
		hMemDC, 0, 0, windowRect.right, windowRect.bottom, RGB(255, 255, 255));
	DeleteObject(bitmap);
	DeleteDC(hMemDC);
}

void drawByDC(HWND hWnd, HDC hMemDC, int x, int y)
{
	static Image* image = Image::FromFile(L"note.png");
	::Graphics g(hMemDC);

	switch (x)
	{
		case -4:
			g.DrawImage(image, -4, y, 90, 40);
			DrawGame(hWndMain, hMemDC);
			break;

		case 79:
			g.DrawImage(image, 79, y, 90, 40);
			DrawGame(hWndMain, hMemDC);
			break;

		case 162:
			g.DrawImage(image, 162, y, 90, 40);
			DrawGame(hWndMain, hMemDC);
			break;

		case 245:
			g.DrawImage(image, 245, y, 90, 40);
			DrawGame(hWndMain, hMemDC);
			break;

		case 328:
			g.DrawImage(image, 328, y, 90, 40);
			DrawGame(hWndMain, hMemDC);
			break;

		case 410:
			g.DrawImage(image, 410, y, 90, 40);
			DrawGame(hWndMain, hMemDC);
	}
}

void DrawGame(HWND hWnd, HDC hdc)
{
	HPEN	hMyPen, hOldPen;
	LOGPEN	logpen;
	POINT	pt;

	if(!g_game_status) Menu(hdc);
	else
	{
		logpen.lopnStyle = PS_SOLID;
		pt.x = 2;
		logpen.lopnWidth = pt;
		logpen.lopnColor = RGB(255, 225, 225);
		hMyPen = CreatePenIndirect(&logpen);
		hOldPen = (HPEN) SelectObject(hdc, hMyPen);

		MyLine(hdc, 0, 570, 500, 570);

		pt.x = 1;
		logpen.lopnWidth = pt;

		logpen.lopnColor = RGB(232, 236, 229);
		hMyPen = CreatePenIndirect(&logpen);
		hOldPen = (HPEN) SelectObject(hdc, hMyPen);

		for(int i = 1; i <= 5; i++) MyLine(hdc, DEFAULTSTRPOS * i, 0, DEFAULTSTRPOS * i, 650);

		pt.x = 2;
		logpen.lopnWidth = pt;

		logpen.lopnColor = RGB(255, 0, 0);
		hMyPen = CreatePenIndirect(&logpen);
		hOldPen = (HPEN) SelectObject(hdc, hMyPen);

		for(int i = 0; i < 6; i++)
		{
			switch(g_paintpoint[i])
			{
				case 5:
					MyLine(hdc, 0, 570, DEFAULTSTRPOS, 570);
					break;

				case 6:
					MyLine(hdc, DEFAULTSTRPOS, 570, DEFAULTSTRPOS * 2, 570);
					break;

				case 7:
					MyLine(hdc, DEFAULTSTRPOS * 2, 570, DEFAULTSTRPOS * 3, 570);
					break;

				case 8:
					MyLine(hdc, DEFAULTSTRPOS * 3, 570, DEFAULTSTRPOS * 4, 570);
					break;

				case 9:
					MyLine(hdc, DEFAULTSTRPOS * 4, 570, DEFAULTSTRPOS * 5, 570);
					break;

				case 0:
					MyLine(hdc, DEFAULTSTRPOS * 5, 570, DEFAULTSTRPOS * 6, 570);
			}
		}
		SelectObject(hdc, hOldPen);
		DeleteObject(hMyPen);
	}
}

void Menu(HDC hdc)
{
	HPEN			hMyPen, hOldPen;
	LOGPEN			logpen;
	HFONT			hFont, OldFont;
	POINT			pt;

	hFont = CreateFont(30, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 
			VARIABLE_PITCH | FF_ROMAN, TEXT("나눔바른펜 Bold"));
	OldFont = (HFONT)SelectObject(hdc, hFont);

	logpen.lopnStyle = PS_SOLID;
	pt.x = 2;
	logpen.lopnWidth = pt;

	logpen.lopnColor = RGB(179, 197, 207);
	hMyPen = CreatePenIndirect(&logpen);
	hOldPen = (HPEN) SelectObject(hdc, hMyPen);
	MyLine(hdc, 0, 570, 500, 570);

	pt.x = 1;
	logpen.lopnWidth = pt;

	logpen.lopnColor = RGB(232, 236, 229);
	hMyPen = CreatePenIndirect(&logpen);
	hOldPen = (HPEN) SelectObject(hdc, hMyPen);
	for(int i = 0; i <= 5; i++) MyLine(hdc, DEFAULTSTRPOS * i, 0, DEFAULTSTRPOS * i, 650);
				
	SelectObject(hdc, hOldPen);
	DeleteObject(hMyPen);

	SetBkMode(hdc, TRANSPARENT);
	MyTextOut(hdc, 140, 582, TEXT("아무 키나 눌러 시작합니다."));

	hFont = CreateFont(20, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 
		VARIABLE_PITCH | FF_ROMAN, TEXT("나눔바른펜 Bold"));
	OldFont = (HFONT)SelectObject(hdc, hFont);

	MyTextOut(hdc, 24, 545, TEXT("NUM 5"));
	MyTextOut(hdc, 107, 545, TEXT("NUM 6"));
	MyTextOut(hdc, 191, 545, TEXT("NUM 7"));
	MyTextOut(hdc, 270, 545, TEXT("NUM 8"));
	MyTextOut(hdc, 355, 545, TEXT("NUM 9"));
	MyTextOut(hdc, 439, 545, TEXT("NUM 0"));

	SelectObject(hdc, OldFont);
	DeleteObject(hFont);
}

void Verify(int RainNum)
{
	switch(RainNum)
	{
		case 5:
			switch(NowTime)
			{
				case 4:
				case 39:
				case 72:
				case 95:
				case 105:
					g_hit++;
			}
		case 6:
			switch(NowTime)
			{
				case 24:
				case 38:
				case 44:
				case 52:
				case 67:
				case 77:
				case 90:
					g_hit++;
			}
		case 7:
			switch(NowTime)
			{
				case 19:
				case 48:
				case 60:
				case 86:
				case 100:
				case 110:
					g_hit++;
			}
		case 8:
			switch(NowTime)
			{
				case 9:
				case 29:
				case 32:
				case 33:
				case 48:
				case 86:
				case 115:
				case 125:
					g_hit++;
			}
		case 9:
			switch(NowTime)
			{
				case 41:
				case 46:
				case 55:
				case 62:
				case 79:
				case 84:
					g_hit++;
			}
		case 0:
			switch (NowTime)
			{
				case 14:
				case 40:
				case 78:
				case 82:
				case 120:
				case 130:
					g_hit++;
			}
	}
}

void soundsetup()
{
	FMOD_System_Create(&g_psystem);
	FMOD_System_Init(g_psystem, 1, FMOD_INIT_NORMAL, NULL);
	
	FMOD_System_CreateStream(g_psystem, "BGM.mp3", FMOD_DEFAULT, 0, &g_psound[0]);
}

void playsound(SOUNDKIND esound)
{
	FMOD_System_PlaySound(g_psystem, FMOD_CHANNEL_FREE, g_psound[esound], 0, &g_pChannel[esound]);
	FMOD_System_Update(g_psystem);
}

void effsoundsetup()
{
	FMOD_System_Create(&effg_psystem);
	FMOD_System_Init(effg_psystem, 1, FMOD_INIT_NORMAL, NULL);

	FMOD_System_CreateSound(effg_psystem, "SE.mp3", FMOD_DEFAULT, 0, &effg_psound[0]);
}

void effsoundoff()
{
	FMOD_Channel_Stop(effg_pchannel[0]);
}

void effplaysound(EFFSOUNDKIND esound)
{
	FMOD_System_PlaySound(effg_psystem, FMOD_CHANNEL_FREE, effg_psound[0], 0, &effg_pchannel[0]);
	FMOD_System_Update(g_psystem);
}
