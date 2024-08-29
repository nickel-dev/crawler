#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <gl/GL.h>
#include "third_party/ogl_api_defs.h"
#include "platform.h"
#include "crawler.c"

global b8 running;
global f64 gameDt;
global InputProfile input;
global InputProfile oldInput;
global WINDOWPLACEMENT windowPosition = { sizeof(windowPosition) };
global i32 windowSize[2] = { 1600, 900 };
global u64 mouseX, mouseY;
global b8 mouseDown;
global b8 fullscreen = true;
global i32 mouseWheel;

//~ Tools
internal void Win32Assert(b32 condition, char* message)
{
	if (!condition)
	{
		MessageBoxA(NULL, message, "Platform Error", MB_OK | MB_ICONERROR);
		*(volatile int*) 0 = 0; // Debug break
	}
}

internal void ToggleFullscreen(HWND Window)
{
	DWORD Style = GetWindowLong(Window, GWL_STYLE);
	if (Style & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
		if (GetWindowPlacement(Window, &windowPosition) &&
			GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
		{
			SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(Window, HWND_TOP,
						 MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
						 MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
						 MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(Window, &windowPosition);
		SetWindowPos(Window, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

double Win32GetCurrentTime() {
    LARGE_INTEGER frequency;
    LARGE_INTEGER currentTimeValue;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&currentTimeValue);
    return (double)currentTimeValue.QuadPart / frequency.QuadPart;
}

global f64 lastTime = 0.0;
internal void CalculateFPSAndDeltaTime() {
	if (!lastTime)
		lastTime = Win32GetCurrentTime();
	f64 currentTime = Win32GetCurrentTime();
	
	gameDt = (f64)(currentTime - lastTime);
	lastTime = currentTime;
}

//~ Win32
internal LRESULT CALLBACK WindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	u32 vkCode = (u32)wParam;
	
	switch (message)
	{
		case WM_QUIT:
		case WM_DESTROY:
		case WM_CLOSE:
		running = false;
		PostQuitMessage(0);
		break;
		
		case WM_SIZE:
		windowSize[0] = LOWORD(lParam);
		windowSize[1] = HIWORD(lParam);
		glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		break;
		
		case WM_SYSKEYUP:
		case WM_KEYUP:
		switch (vkCode)
		{
			default:
			input.keys[vkCode] = INPUT_STATE_UP;
			break;
		}
		break;
		
		case WM_MOUSEMOVE:
		mouseX = LOWORD(lParam); 
		mouseY = HIWORD(lParam);
		break;
		
		case WM_LBUTTONDOWN:
		mouseDown = true;
		break;
		
		case WM_MOUSEWHEEL:
		mouseWheel = GET_WHEEL_DELTA_WPARAM(wParam);
		break;
		
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		switch (vkCode)
		{
			case VK_F11:
			ToggleFullscreen(window);
			break;
			
			case VK_ESCAPE:
			running = false;
			break;
			
			default:
			input.keys[vkCode] = INPUT_STATE_DOWN;
			break;
		}
		break;
		
		default:
		result = DefWindowProc(window, message, wParam, lParam);
		break;
	}
	
	return result;
}


i32 CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, PSTR CommandLine, i32 showCode)
{
	WNDCLASS windowClass = { 0 };
	windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowCallback;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);
	windowClass.lpszClassName = "Win32WindowClass";
	
	Win32Assert(RegisterClassA(&windowClass), "Failed to register window class.");
	
	HWND window = CreateWindowExA(0, windowClass.lpszClassName, "Crawler", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
								  0, 0, windowSize[0], windowSize[1], NULL, NULL, instance, NULL);
	
	Win32Assert(window != NULL, "Failed to create Win32 window.");
	HDC deviceContext = GetDC(window);
	
	// Window initialization
	{
		// Pixel format for OpenGL context
		PIXELFORMATDESCRIPTOR pixelFormatDesc = { 0 };
		pixelFormatDesc.nSize      = sizeof(pixelFormatDesc);
		pixelFormatDesc.nVersion   = 1;
		pixelFormatDesc.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
		pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
		pixelFormatDesc.cColorBits = 32;
		
		i32 pixelFormat = ChoosePixelFormat(deviceContext, &pixelFormatDesc);
		SetPixelFormat(deviceContext, pixelFormat, &pixelFormatDesc);
		DescribePixelFormat(deviceContext, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDesc);
		
		// Create and laod OpenGL
		HGLRC openglContext = wglCreateContext(deviceContext);
		wglMakeCurrent(deviceContext, openglContext);
		
		LoadOpenGLFunctions();
		
		// Finalize window creation
		ReleaseDC(window, deviceContext);
		ShowWindow(window, showCode);
		
		// Attach console to process
		if (!AttachConsole(ATTACH_PARENT_PROCESS))
		{
			if (GetLastError() != ERROR_ACCESS_DENIED)
				AttachConsole(GetCurrentProcessId());
		}
		AttachConsole(GetCurrentProcessId());
	}
	
	// NOTE(nickel): Window main loop
	if (fullscreen)
		ToggleFullscreen(window);
	OnStart();
	fflush(stdout);
	
	running = true;
	MSG msg;
	
	while (running)
	{
		// NOTE(nickel): stdout isn't flushed automatically
		fflush(stdout);
		
		CalculateFPSAndDeltaTime();
		
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		if (msg.message == WM_QUIT)
			running = false;
		
		OnUpdate(input, oldInput, gameDt, windowSize[0], windowSize[1], Win32GetCurrentTime(), mouseX - windowSize[0] / 2, mouseY - windowSize[1] / 2, mouseDown, mouseWheel);
		mouseWheel = 0;
		
		for (int i = 0; i < 256; ++i)
			oldInput.keys[i] = input.keys[i];
		
		if (mouseDown)
			mouseDown = false;
		
		glFinish();
		UpdateWindow(window);
		SwapBuffers(deviceContext);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	
	return (i32)msg.wParam;
}