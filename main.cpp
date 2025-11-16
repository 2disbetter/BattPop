// BattPop - main.cpp

#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <cstdio>
#include "resource.h"

#define HOTKEY_ID 1
#define TRAY_ICON_ID 1
#define WM_TRAYICON (WM_USER + 1)

NOTIFYICONDATA nid = {};
HWND hWnd;

bool AddTrayIcon(HINSTANCE hInstance) {
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = TRAY_ICON_ID;
    nid.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON | NIF_INFO;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON));  // Use custom icon
    lstrcpy(nid.szTip, TEXT("Battery Status Checker"));
    return Shell_NotifyIcon(NIM_ADD, &nid);
}

void ShowBatteryInfo() {
    SYSTEM_POWER_STATUS sps;
    if (!GetSystemPowerStatus(&sps)) {
        return;
    }

    TCHAR buf[256];
    const TCHAR* status = (sps.ACLineStatus == 1) ? TEXT("Charging") : TEXT("Discharging");
    _stprintf(buf, TEXT("Battery: %d%% - %s"), sps.BatteryLifePercent, status);

    lstrcpy(nid.szInfo, buf);
    lstrcpy(nid.szInfoTitle, TEXT("Battery Status"));
    nid.uTimeout = 5000;  // 5 seconds
    nid.dwInfoFlags = NIIF_INFO;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_HOTKEY:
            if (wParam == HOTKEY_ID) {
                ShowBatteryInfo();
            }
            return 0;
        case WM_TRAYICON:
            if (wParam == TRAY_ICON_ID && lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                HMENU hMenu = CreatePopupMenu();
                if (hMenu) {
                    AppendMenu(hMenu, MF_STRING, IDM_EXIT, TEXT("Exit"));
                    SetForegroundWindow(hwnd);  // for menu dismisal
                    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
                    DestroyMenu(hMenu);
                }
            }
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDM_EXIT) {
                DestroyWindow(hwnd);
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("BatteryHotkeyClass");
    if (!RegisterClass(&wc)) {
        return 1;
    }

    // Create invisible window
    hWnd = CreateWindow(TEXT("BatteryHotkeyClass"), TEXT("Battery Checker"), 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (!hWnd) {
        return 1;
    }

    // Register global hotkey (Ctrl + Alt + B)
    if (!RegisterHotKey(hWnd, HOTKEY_ID, MOD_CONTROL | MOD_ALT, 'B')) {
        MessageBox(NULL, TEXT("Failed to register hotkey!"), TEXT("Error"), MB_OK | MB_ICONERROR);
        return 1;
    }

    // Add tray icon
    if (!AddTrayIcon(hInstance)) {
        MessageBox(NULL, TEXT("Failed to add tray icon!"), TEXT("Error"), MB_OK | MB_ICONERROR);
        return 1;
    }

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    UnregisterHotKey(hWnd, HOTKEY_ID);
    Shell_NotifyIcon(NIM_DELETE, &nid);

    return (int)msg.wParam;
}