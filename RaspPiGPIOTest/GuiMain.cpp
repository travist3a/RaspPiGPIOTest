#include <windows.h>
#include <string>
#include "SSHGPIO.h"

#define IDC_GPIO_EDIT   101
#define IDC_TOGGLE_BTN  102
#define IDC_STATUS_TXT  103

static HINSTANCE g_hInst = nullptr;
static bool      g_stateHigh = false;      // tracks last state we sent

// ─────────────────────────────────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    switch (m)
    {
    case WM_CREATE:
        gpio_open_session();
        CreateWindowExA(0, "STATIC", "GPIO #:",
            WS_CHILD | WS_VISIBLE,
            10, 15, 60, 20, h, nullptr, g_hInst, nullptr);

        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "2",
            WS_CHILD | WS_VISIBLE | ES_NUMBER,
            75, 12, 50, 24, h, (HMENU)IDC_GPIO_EDIT, g_hInst, nullptr);

        CreateWindowExA(0, "BUTTON", "Toggle",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            140, 10, 80, 28, h, (HMENU)IDC_TOGGLE_BTN, g_hInst, nullptr);

        CreateWindowExA(0, "STATIC", "State: LOW",
            WS_CHILD | WS_VISIBLE,
            10, 50, 210, 20, h, (HMENU)IDC_STATUS_TXT, g_hInst, nullptr);
        break;

    case WM_COMMAND:
        if (LOWORD(w) == IDC_TOGGLE_BTN && HIWORD(w) == BN_CLICKED)
        {
            char buf[8] = {};
            GetWindowTextA(GetDlgItem(h, IDC_GPIO_EDIT), buf, sizeof buf);
            int gpio = std::atoi(buf);
            if (gpio < 0 || gpio > 27) {      // Pi 4 has GPIOs 0‑27
                MessageBoxA(h, "Enter a valid BCM GPIO number (0‑27)",
                    "Invalid Input", MB_ICONWARNING);
                break;
            }
            bool ok = gpio_set_remote(gpio, !g_stateHigh);
            if (!ok) {
                MessageBoxA(h, "Remote command failed",
                    "Error", MB_ICONERROR);
            }
            else {
                g_stateHigh = !g_stateHigh;
                SetWindowTextA(GetDlgItem(h, IDC_STATUS_TXT),
                    g_stateHigh ? "State: HIGH" : "State: LOW");
            }
        }
        break;

    case WM_DESTROY:
        gpio_close_session();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(h, m, w, l);
    }
    return 0;
}
// ─────────────────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    g_hInst = hInst;

    WNDCLASSEXA wc{ sizeof wc };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "PiGPIOToggler";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassExA(&wc);

    HWND wnd = CreateWindowExA(0, wc.lpszClassName, "Raspberry Pi GPIO Toggler",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 250, 120,
        nullptr, nullptr, hInst, nullptr);

    ShowWindow(wnd, nCmdShow);
    UpdateWindow(wnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
