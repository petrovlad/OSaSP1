#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <math.h>

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#define RECT_WIDTH 100
#define RECT_HEIGHT 50
#define ELLIPSE_WIDTH 150
#define ELLIPSE_HEIGHT 50
#define PI 3.14159265
#define DEFAULT_SPEED 5.0
#define IMAGE_NAME L"Horse.png"

VOID OnPaint(HWND hWnd);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

enum class Shape {
    RECTANGLE, ELLIPSE, PICTURE
};

int x{}, y{}, w{}, h{};
double speed{};
double direction{};
BOOL isMoving = false;
Shape shape = Shape::RECTANGLE;

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR szCmdLine, int nCmdShow) {
    MSG msg{};                             
    HWND hWnd{};                         
    WNDCLASSEX wc{ sizeof(WNDCLASSEX) }; 
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;

    // Initialize GDI+
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = L"MyAppClass";
    wc.lpszMenuName = nullptr;
    wc.style = CS_VREDRAW | CS_HREDRAW; //!!!

    if (!RegisterClassEx(&wc))
        return EXIT_FAILURE;

    if ((hWnd = CreateWindow(wc.lpszClassName, L"AAaaAa",
        WS_OVERLAPPEDWINDOW, 0, 0, 600, 600, nullptr, nullptr, 
        wc.hInstance, nullptr)) == INVALID_HANDLE_VALUE)
        return EXIT_FAILURE;

    ShowWindow(hWnd, nCmdShow); 
    UpdateWindow(hWnd);         


    while (GetMessage(&msg, nullptr, 0, 0)) { 
        TranslateMessage(&msg);
        DispatchMessage(&msg); 
    }
    GdiplusShutdown(gdiplusToken);
    return (msg.wParam); 
}

VOID OnPaint(HWND hWnd) {
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;

    hdc = BeginPaint(hWnd, &ps);
    switch (shape) {

    case Shape::RECTANGLE:
        w = RECT_WIDTH;
        h = RECT_HEIGHT;
        rect = { x, y, x + w, y + h };
        FillRect(hdc, &rect, (HBRUSH)(COLOR_BACKGROUND));
        break;

    case Shape::ELLIPSE:
        w = ELLIPSE_WIDTH;
        h = ELLIPSE_HEIGHT;
        Ellipse(hdc, x, y, x + w, y + h);
        break;

    case Shape::PICTURE:

        Graphics graphics(hWnd);
        Image image(IMAGE_NAME);
        w = image.GetWidth();
        h = image.GetHeight();
        rect = { x, y, w, h };
        Rect destRect(x, y, w, h);
        graphics.DrawImage(&image, destRect);

        EndPaint(hWnd, &ps);
        break;
    }

    EndPaint(hWnd, &ps);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int wndWidth, wndHeight;
    RECT clientRect;

    GetClientRect(hWnd, &clientRect);
    switch (uMsg) {

    case WM_DESTROY:
        PostQuitMessage(EXIT_SUCCESS);//!!!
        break;

    case WM_CHAR:
        switch (wParam) {
        case 0x6d: // key M
            switch (shape) {

            case Shape::RECTANGLE:
                shape = Shape::ELLIPSE;
                break;

            case Shape::ELLIPSE:
                shape = Shape::PICTURE;
                break;

            case Shape::PICTURE:
                shape = Shape::RECTANGLE;
                break;
            }
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case 0x0D: // key Enter
            isMoving = !isMoving;
            if (isMoving) {
                if (SetTimer(hWnd, NULL, 10, (TIMERPROC)NULL) == 0) {
                    return EXIT_FAILURE;
                }
                speed = DEFAULT_SPEED;
            }
            else {
                if (KillTimer(hWnd, NULL) == 0) {
                    return EXIT_FAILURE;
                }
            }
            direction = (double)(rand() % 360);
            break;
        }
        break;

    case WM_LBUTTONDOWN:
        x = LOWORD(lParam);
        y = HIWORD(lParam);

        if (isMoving) {
            isMoving = false;
            speed = 0.0;
        }

        if (x + w >= clientRect.right)
            x = clientRect.right - w;
        if (y + h >= clientRect.bottom)
            y = clientRect.bottom - h;

        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_MOUSEWHEEL:
        int fwKeys, delta;
        fwKeys = GET_KEYSTATE_WPARAM(wParam);
        delta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (fwKeys && MK_SHIFT) {
            // horizontal
            x += delta / WHEEL_DELTA;
        }
        else {
            // vertical
            y += delta / WHEEL_DELTA;
        }
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_TIMER:
        x += speed * cos((direction - 90) * PI / 180);
        y += speed * sin((direction - 90) * PI / 180);

        if ((y + h >= clientRect.bottom) || (y <= clientRect.top)) {
            direction = (int)(180.0 - direction) % 360;
            //speed *= 0.95;
        }

        if ((x <= clientRect.left) || (x + w >= clientRect.right)) {
            direction = (int)(360.0 - direction) % 360;
            //speed *= 0.95;
        }

        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_PAINT:
        OnPaint(hWnd);
        break;

    default:
        return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }
    return 0;
};