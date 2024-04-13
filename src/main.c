#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define IDT_TIMER1 0
#define IDC_STATIC_TEXT 1
#define IDC_STATIC_ICON 2
#define IDC_OK_BUTTON 3

int dx = 5, dy = 5;
bool funMode = false;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  static HBRUSH hBrush = NULL;

  switch (msg)
  {
  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;
  case WM_DESTROY:
  {
    if (hBrush != NULL)
    {
      DeleteObject(hBrush);
    }
    PostQuitMessage(0);
  }
  break;
  case WM_CREATE:
  {
    // Create the static text control
    CreateWindowEx(0, "STATIC", "Task failed successfully.", WS_CHILD | WS_VISIBLE | SS_CENTER, 30, 40, 300, 30, hwnd, (HMENU)IDC_STATIC_TEXT, NULL, NULL);

    // Create the static icon control
    HICON hInfoIcon = LoadIcon(NULL, IDI_INFORMATION);
    CreateWindowEx(0, "STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_ICON, 40, 30, 30, 30, hwnd, (HMENU)IDC_STATIC_ICON, NULL, NULL);
    SendMessage(GetDlgItem(hwnd, IDC_STATIC_ICON), STM_SETICON, (WPARAM)hInfoIcon, 0);

    // Create the OK button
    CreateWindowEx(0, "BUTTON", "OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 100, 95, 100, 30, hwnd, (HMENU)IDC_OK_BUTTON, NULL, NULL);
  }
  break;
  case WM_COMMAND:
  {
    if (LOWORD(wParam) == IDC_OK_BUTTON)
    {
      DestroyWindow(hwnd);
    }
  }
  break;
  case WM_CTLCOLORSTATIC:
  {
    if (funMode && hBrush != NULL)
    {
      HDC hdcStatic = (HDC)wParam;
      SetBkMode(hdcStatic, TRANSPARENT);
      return (INT_PTR)hBrush;
    }
  }
  break;
  case WM_TIMER:
  {
    RECT rect;
    GetWindowRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {sizeof(mi)};
    GetMonitorInfo(hmon, &mi);

    RECT desktopRect = mi.rcWork;

    if (rect.left + dx < desktopRect.left || rect.right + dx > desktopRect.right)
    {
      dx = -dx;
      if (funMode)
      {
        if (hBrush != NULL)
        {
          DeleteObject(hBrush);
        }
        hBrush = CreateSolidBrush(RGB(rand() % 256, rand() % 256, rand() % 256));
        SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG)hBrush);
        InvalidateRect(hwnd, NULL, TRUE);
      }
    }
    if (rect.top + dy < desktopRect.top || rect.bottom + dy > desktopRect.bottom)
    {
      dy = -dy;
      if (funMode)
      {
        if (hBrush != NULL)
        {
          DeleteObject(hBrush);
        }
        hBrush = CreateSolidBrush(RGB(rand() % 256, rand() % 256, rand() % 256));
        SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG)hBrush);
        InvalidateRect(hwnd, NULL, TRUE);
      }
    }

    SetWindowPos(hwnd, HWND_TOP, rect.left + dx, rect.top + dy, width, height, SWP_SHOWWINDOW);
  }
  break;
  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

DWORD WINAPI MoveWindowThread(LPVOID lpParam)
{
  HWND hwnd = (HWND)lpParam;

  while (1)
  {
    PostMessage(hwnd, WM_TIMER, 0, 0);
    Sleep(10);
  }

  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  const char CLASS_NAME[] = "FunnyErrorWindow";

  int speed = 5;

  // Parse command line arguments
  int argc;
  LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (argv)
  {
    for (int i = 0; i < argc; i++)
    {
      if (wcscmp(argv[i], L"--speed") == 0 && i + 1 < argc)
      {
        speed = _wtoi(argv[i + 1]);
      }
      else if (wcscmp(argv[i], L"--fun") == 0)
      {
        funMode = true;
      }
    }
    LocalFree(argv);
  }

  WNDCLASS wc = {};

  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  wc.hIcon = LoadIcon(NULL, IDI_ERROR);

  RegisterClass(&wc);

  srand(time(NULL));
  RECT workArea;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

  int screenWidth = workArea.right - workArea.left - 300;
  int screenHeight = workArea.bottom - workArea.top - 175;

  int startX = workArea.left + rand() % screenWidth;
  int startY = workArea.top + rand() % screenHeight;

  dx = (rand() % 2 == 0) ? speed : -speed;
  dy = (rand() % 2 == 0) ? speed : -speed;

  HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Error", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, startX, startY, 300, 175, NULL, NULL, hInstance, NULL);

  if (hwnd == NULL)
  {
    return 0;
  }

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  CreateThread(NULL, 0, MoveWindowThread, hwnd, 0, NULL);

  MSG msg = {};
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}