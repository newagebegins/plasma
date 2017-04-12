#include <windows.h>
#include <math.h>
#include <assert.h>

typedef int Color;

struct BackBuffer {
    BITMAPINFO info;
    Color *memory;
    int width, height;
};

BackBuffer makeBackBuffer(int width, int height) {
    BackBuffer bb = {};

    bb.width = width;
    bb.height = height;

    bb.info.bmiHeader.biSize = sizeof(bb.info.bmiHeader);
    bb.info.bmiHeader.biWidth = width;
    bb.info.bmiHeader.biHeight = -height;
    bb.info.bmiHeader.biPlanes = 1;
    bb.info.bmiHeader.biBitCount = 32;
    bb.info.bmiHeader.biCompression = BI_RGB;

    bb.memory = (Color*)malloc(width * height * sizeof(Color));

    return bb;
}

void setPixel(BackBuffer *bb, int x, int y, Color c) {
    bb->memory[y*bb->width + x] = c;
}

Color makeColor(float r, float g, float b) {
    assert(r >= 0.0f && r <= 1.0f);
    assert(g >= 0.0f && g <= 1.0f);
    assert(b >= 0.0f && b <= 1.0f);

    int ai = 255;
    int ri = (int)(r*255.0f);
    int gi = (int)(g*255.0f);
    int bi = (int)(b*255.0f);
    Color c = (ai << 24) | (ri << 16) | (gi << 8) | bi;
    return c;
}

Color makeColor(float c) {
    return makeColor(c, c, c);
}

Color makeColorHSV(float h, float s, float v) {
    assert(h >= 0.0f && h < 360.0f);
    assert(s >= 0.0f && s <= 1.0f);
    assert(v >= 0.0f && v <= 1.0f);

    float c = v * s;
    float hPrime = h / 60.0f;
    float hPrimeMod = fmodf(hPrime, 2.0f);
    float x = c * (1.0f - fabsf(hPrimeMod - 1.0f));

    float r1, g1, b1;

    if      (hPrime < 1) r1 = c, g1 = x, b1 = 0;
    else if (hPrime < 2) r1 = x, g1 = c, b1 = 0;
    else if (hPrime < 3) r1 = 0, g1 = c, b1 = x;
    else if (hPrime < 4) r1 = 0, g1 = x, b1 = c;
    else if (hPrime < 5) r1 = x, g1 = 0, b1 = c;
    else                 r1 = c, g1 = 0, b1 = x;

    float m = v - c;
    float r = r1 + m, g = g1 + m, b = b1 + m;

    return makeColor(r, g, b);
}

struct Palette {
    int size;
    Color *colors;
};

Palette makePalette() {
    Palette p = {};
    p.size = 360;
    p.colors = (Color*)malloc(p.size*sizeof(Color));
    for (int h = 0; h < p.size; ++h) {
        p.colors[h] = makeColorHSV((float)h, 1.0f, 1.0f);
    }
    return p;
}

void drawPalette(BackBuffer *bb, Palette *p, int x, int y, int height) {
    for (int c = 0; c < p->size; ++c) {
        for (int dy = 0; dy < height; ++dy) {
            setPixel(bb, x + c, y + dy, p->colors[c]);
        }
    }
}

float* makePlasma(int width, int height) {
    float *plasma = (float*)malloc(width*height*sizeof(float));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float val =
                (
                    (0.5f + 0.5f * sinf(x / 20.0f)) +
                    (0.5f + 0.5f * sinf(y / 16.0f)) +
                    (0.5f + 0.5f * sinf((x + y) / 30.0f)) +
                    (0.5f + 0.5f * sinf(sqrtf(float(x*x + y*y)) / 18.0f))
                ) / 4.0f;
            plasma[y*width + x] = val;
        }
    }
    return plasma;
}

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

    int windowWidth = 400;
    int windowHeight = 400;

    RECT clientRect = { 0, 0, windowWidth, windowHeight };
    DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    AdjustWindowRect(&clientRect, windowStyle, NULL);
    HWND hWnd = CreateWindowEx(NULL, wc.lpszClassName, "Plasma", windowStyle,
                               300, 100,
                               clientRect.right - clientRect.left,
                               clientRect.bottom - clientRect.top,
                               NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        MessageBox(NULL, "CreateWindowEx failed!", NULL, NULL);
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    LARGE_INTEGER perfCounterFrequency = { 0 };
    QueryPerformanceFrequency(&perfCounterFrequency);
    LARGE_INTEGER perfCounter = { 0 };
    QueryPerformanceCounter(&perfCounter);
    LARGE_INTEGER prevPerfCounter = { 0 };

    float dt = 0.0f;
    float targetFps = 60.0f;
    float targetDt = 1.0f / targetFps;
    float passedTime = 0;

    bool running = true;

    BackBuffer bb = makeBackBuffer(windowWidth, windowHeight);
    Palette palette = makePalette();
    float *plasma = makePlasma(windowWidth, windowHeight);
    
    while (running) {
        prevPerfCounter = perfCounter;
        QueryPerformanceCounter(&perfCounter);
        dt = (float)(perfCounter.QuadPart - prevPerfCounter.QuadPart) / (float)perfCounterFrequency.QuadPart;

        if (dt > targetDt) {
            dt = targetDt;
        }

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

        passedTime += dt;

        int paletteShift = int(passedTime * 200.0f);
        for (int y = 0; y < bb.height; ++y) {
            for (int x = 0; x < bb.width; ++x) {
                float kHelper = plasma[y*bb.width + x]*256 + paletteShift;
                int k = (int)kHelper;
                Color c = palette.colors[k % palette.size];
                setPixel(&bb, x, y, c);
            }
        }

        HDC hDC = GetDC(hWnd);
        StretchDIBits(hDC, 0, 0, bb.width, bb.height, 0, 0, bb.width, bb.height,
                      bb.memory, &bb.info, DIB_RGB_COLORS, SRCCOPY);
        ReleaseDC(hWnd, hDC);
    }

    return 0;
}
