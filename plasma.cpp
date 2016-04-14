#include <windows.h>

LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = wndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "Plasma";

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "RegisterClass failed!", NULL, NULL);
        return 1;
    }

    int windowWidth = 800;
    int windowHeight = 600;

    RECT clientRect = { 0, 0, windowWidth, windowHeight };
    DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    AdjustWindowRect(&clientRect, windowStyle, NULL);
    HWND hWnd = CreateWindowEx(NULL, wc.lpszClassName, "Plasma", windowStyle,
                               300, 0,
                               clientRect.right - clientRect.left,
                               clientRect.bottom - clientRect.top,
                               NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        MessageBox(NULL, "CreateWindowEx failed!", NULL, NULL);
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    bool running = true;

    while (running) {
        MSG msg;
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            switch (msg.message) {
            case WM_QUIT:
                running = false;
                break;
            case WM_KEYDOWN:
            case WM_KEYUP:
                switch (msg.wParam) {
                case VK_ESCAPE:
                    running = false;
                    break;
                }
                break;
            default:
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                break;
            }
        }
    }

    return 0;
}
