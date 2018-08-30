// EchoCon.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <conio.h>
#include <process.h>    /* _beginthread, _endthread */
#include <wchar.h>

void __cdecl PipeListener(LPVOID);
HRESULT InitializeStartupInfoAttachedToConPTY(STARTUPINFOEX*, HPCON);
COORD GetConsoleSize(HANDLE);

int main()
{
    HRESULT hr = S_OK;
    HANDLE hPipeIn = 0;
    HANDLE hPipeOut = 0;
    HANDLE hPipePTYIn = 0;
    HANDLE hPipePTYOut = 0;
    HPCON hPC = 0;

    PROCESS_INFORMATION piClient{};
    STARTUPINFOEX startupInfo{};

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode = 0;

    GetConsoleMode(hConsole, &consoleMode);
    hr = SetConsoleMode(hConsole, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)
        ? S_OK
        : GetLastError();

    if (hr == S_OK)
    {
        COORD consoleSize = GetConsoleSize(hConsole);

        // Create the pipes to which the ConPTY will connect
        if (CreatePipe(&hPipePTYIn, &hPipeOut, NULL, 0) &&
            CreatePipe(&hPipeIn, &hPipePTYOut, NULL, 0))
        {
            // Create ConPTY
            hr = CreatePseudoConsole(consoleSize, hPipePTYIn, hPipePTYOut, 0, &hPC);

            // Initialize the necessary startup info struct        
            if (S_OK == InitializeStartupInfoAttachedToConPTY(&startupInfo, hPC))
            {
                // Create & start thread to listen to the incoming pipe
                // NB: Using CRT-safe _beginthread() rather than CreateThread()
                DWORD pipeListenerThreadId = 0;
                HANDLE hPipeListenerThread = (HANDLE)_beginthread(
                    PipeListener,           // Thread function
                    0,                      // Default stack
                    hPipeIn);               // Thread arg

                // Launch ping to echo some text back via the pipe
                TCHAR szCommand[] = L"ping 8.8.8.8";
                _tprintf_s(L"\033[32;1m Executing Command: `%s` \033[0m\n", szCommand);
                hr = CreateProcess(
                    NULL,                           // No module name - use Command Line
                    szCommand,                      // Command Line
                    NULL,                           // Process handle not inheritable
                    NULL,                           // Thread handle not inheritable
                    TRUE,                           // Inherit handles
                    EXTENDED_STARTUPINFO_PRESENT,   // Creation flags
                    NULL,                           // Use parent's environment block
                    NULL,                           // Use parent's starting directory 
                    &startupInfo.StartupInfo,       // Pointer to STARTUPINFO
                    &piClient)                      // Pointer to PROCESS_INFORMATION
                    ? S_OK
                    : GetLastError();

                //  Wait until ping completes
                WaitForSingleObject(piClient.hProcess, INFINITE);

                // --- CLOSEDOWN ---

                // Close ConPTY - this will terminate client process if running
                ClosePseudoConsole(hPC);

                // Now safe to clean-up client process info process & thread
                CloseHandle(piClient.hThread);
                CloseHandle(piClient.hProcess);

                // Cleanup attribute list
                if (startupInfo.lpAttributeList)
                {
                    DeleteProcThreadAttributeList(startupInfo.lpAttributeList);
                    free(startupInfo.lpAttributeList);
                }
            }
            else
            {
                _tprintf_s(L"Error: Failed to initialize StartupInfo attached to ConPTY [0x%x]", GetLastError());
            }
        }
        else
        {
            _tprintf_s(L"Error: Failed to create pipes");
        }

        // Clean-up the pipes
        if (hPipeIn) CloseHandle(hPipeIn);
        if (hPipeOut) CloseHandle(hPipeOut);
        if (hPipePTYIn) CloseHandle(hPipePTYIn);
        if (hPipePTYOut) CloseHandle(hPipePTYOut);

        // Restore console to its original mode.
        hr = SetConsoleMode(hConsole, consoleMode)
            ? S_OK
            : GetLastError();
    }

    return hr == S_OK;
}

void __cdecl PipeListener(LPVOID pipe)
{
    HANDLE hPipe = (HANDLE)(HANDLE*)pipe;

    const DWORD BUFF_SIZE = 512;
    char* pszBuffer = (char*)malloc(BUFF_SIZE);
    if (pszBuffer != NULL)
    {
        memset(pszBuffer, 0, BUFF_SIZE);

        SHORT keyState = 0;
        DWORD dwBytesRead = -1;
        BOOL fResult = TRUE;
        while (keyState == 0
            && fResult
            && dwBytesRead > 0)
        {
            fResult = ReadFile(hPipe, pszBuffer, BUFF_SIZE, &dwBytesRead, NULL);
            printf(pszBuffer);
            _flushall();

            // Check if the user has hit ESC to exit:
            keyState = GetAsyncKeyState(VK_ESCAPE);
        }

        free(pszBuffer);

        // Automatically cleans-up
        _endthread();
    }
}

// Initializes the specified startup info struct with the required properties and
// updates its thread attribute list with the specified ConPTY handle
HRESULT InitializeStartupInfoAttachedToConPTY(STARTUPINFOEX* pStartupInfo, HPCON hPC)
{
    HRESULT hr = E_UNEXPECTED;

    if (pStartupInfo)
    {
        SIZE_T size = 0;

        pStartupInfo->StartupInfo.cb = sizeof(STARTUPINFOEX);

        // Get the size of the thread attribute list.
        InitializeProcThreadAttributeList(NULL, 1, 0, &size);

        // Allocate a thread attribute list of the correct size
        pStartupInfo->lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)malloc((size_t)size);

        // Initialize thread attribute list
        if (pStartupInfo->lpAttributeList &&
            InitializeProcThreadAttributeList(pStartupInfo->lpAttributeList, 1, 0, &size))
        {
            // Set thread attribute list's Pseudo Console to the specified ConPTY
            hr = UpdateProcThreadAttribute(
                pStartupInfo->lpAttributeList,
                0,
                PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                hPC,
                sizeof(HPCON),
                NULL,
                NULL)
                ? S_OK
                : HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return hr;
}

COORD GetConsoleSize(HANDLE hStdOut)
{
    COORD size = { 0, 0 };

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
    {
        size.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        size.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }

    return size;
}

