#include "stdafx.h"
#include "resource.h"

HWND hWnd;
HINSTANCE hInst;

#include "view.h"
#include "imm.h"
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "version.lib")

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   hWnd = CreateWindowExW(WS_EX_ACCEPTFILES, MNP_APPNAME, L"Untitled - MyNotePad", WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd) return FALSE;

   HDC hdc = GetDC(hWnd);
   for (auto& l : article)
	   repaintLine(hdc, l);
   ReleaseDC(hWnd, hdc);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   return TRUE;
}

std::wstring GetFileVersion(HMODULE hModule)
{
	TCHAR filename[MAX_PATH];

	if (GetModuleFileName(hModule, filename, MAX_PATH) == 0)
		return L"";

	DWORD dwHandle;
	int size = GetFileVersionInfoSize(filename, &dwHandle);
	if (size <= 0)
		return L"";

	LPBYTE buffer = new BYTE[size];
	if (GetFileVersionInfo(filename, dwHandle, size, buffer) == 0)
	{
		delete buffer;
		return L"";
	}

	VS_FIXEDFILEINFO *pVi;
	if (VerQueryValue(buffer, L"\\", (LPVOID *)&pVi, (PUINT)&size))
	{
		std::wstringstream result;
		result << HIWORD(pVi->dwFileVersionMS) << L'.'
			<< LOWORD(pVi->dwFileVersionMS) << L'.'
			<< HIWORD(pVi->dwFileVersionLS) << L'.'
			<< LOWORD(pVi->dwFileVersionLS);

		delete buffer;
		return result.str();
	}
	delete buffer;
	return L"";
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowText(GetDlgItem(hDlg, IDC_VERSION),
			(std::wstring(MNP_APPNAME) + L" v" + GetFileVersion(hInst)).c_str()
		);
		
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			switch (wmId)
			{
			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_WORDWRAP:
				OnMenuWordWrap();
				break;
			case IDM_NEW:
				OnMenuNew();
				break;
			case IDM_OPEN:
				OnMenuOpen();
				break;
			case IDM_SAVE:
				OnMenuSave();
				break;
			case IDM_SAVEAS:
				OnMenuSaveAs();
				break;
			case IDM_UNDO:
				OnMenuUndo();
				break;
			case IDM_REDO:
				OnMenuRedo();
				break;
			case IDM_CUT:
				OnMenuCut();
				break;
			case IDM_COPY:
				OnMenuCopy();
				break;
			case IDM_PASTE:
				OnMenuPaste();
				break;
			case IDM_DEL:
				OnKeyDown(VK_DELETE);
				break;
			case IDM_ALL:
				OnMenuAll();
				break;
			case IDM_EXPORT:
				OnMenuExport();
				break;
			case IDM_COPYHTML:
				OnMenuCopyHtml();
				break;
			case IDM_EXIT:
				if (sureToQuit())
					DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
        break;
	case WM_IME_STARTCOMPOSITION:
		{
			HIMC hImc = ImmGetContext(hWnd);
			COMPOSITIONFORM Composition;
			Composition.dwStyle = CFS_POINT;
			Composition.ptCurrentPos.x = caret_x + MNP_PADDING_CLIENT;	// IME follow
			Composition.ptCurrentPos.y = caret_y + MNP_FONTSIZE / 4;	// set IME position
			ImmSetCompositionWindow(hImc, &Composition);
			ImmReleaseContext(hWnd, hImc);
		}
		break;
	case WM_KEYDOWN:
		OnKeyDown(wParam);
		break;
	case WM_CHAR:
		OnChar(wParam);
		break;
	case WM_KEYUP:
		OnKeyUp(wParam);
		break;
	case WM_SIZE:
		OnSize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_ERASEBKGND:
		return TRUE;
	case WM_LBUTTONDOWN:
		OnLButtonDown(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONUP:
		OnLButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_RBUTTONDOWN:
		OnRButtonDown(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEHWHEEL:
		{
		if (word_wrap)
			break;

			RECT rc;
			GetClientRect(hWnd, &rc);
			OnMouseHWheel(
				GET_WHEEL_DELTA_WPARAM(wParam),
				(int)LOWORD(lParam) - rc.left,
				(int)HIWORD(lParam) - rc.top
			);
		}
		break;
	case WM_MOUSEWHEEL:
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			OnMouseWheel(
				GET_WHEEL_DELTA_WPARAM(wParam),
				(int)LOWORD(lParam) - rc.left,
				(int)HIWORD(lParam) - rc.top
			);
		}
		break;
	case WM_DROPFILES:
		{
			TCHAR file[MAX_PATH];
			DragQueryFile((HDROP)wParam, 0, file, MAX_PATH);
			loadFile(file);
			DragFinish((HDROP)wParam);
		}
		break;
    case WM_PAINT:
		{
			PAINTSTRUCT ps;
			OnPaint(BeginPaint(hWnd, &ps));
			EndPaint(hWnd, &ps);
		}
        break;
	case WM_CLOSE:
		if (sureToQuit())
			return DefWindowProc(hWnd, message, wParam, lParam);
		else
			return 0;
    case WM_DESTROY:
		PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYNOTEPAD));
	wcex.hCursor = LoadCursor(nullptr, IDC_IBEAM);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MYNOTEPAD);
	wcex.lpszClassName = MNP_APPNAME;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MyRegisterClass(hInstance);
	
	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	if(*lpCmdLine != '\0')
		loadFile(lpCmdLine);		// load the file specified in cmdLine

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

