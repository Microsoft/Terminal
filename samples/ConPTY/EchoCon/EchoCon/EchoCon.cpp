
// EchoCon.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <conio.h>

// Initializes the specified startup info struct with the required properties and
// updates its thread attribute list with the specified ConPTY handle
HRESULT InitializeStartupInfoAttachedToConPTY(STARTUPINFOEX* pStartupInfo, HPCON hPC)
{
    HRESULT hr = E_UNEXPECTED;
    SIZE_T size;
    
    pStartupInfo->StartupInfo.cb = sizeof(STARTUPINFOEX);

    // Allocate a thread attribute list of the correct size
    InitializeProcThreadAttributeList(NULL, 1, 0, &size);
    pStartupInfo->lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)malloc((size_t)&size);

    // Set startup info's attribute list & initialize it
    bool fSuccess = InitializeProcThreadAttributeList(
        pStartupInfo->lpAttributeList, 1, 0, &size);

    if (fSuccess)
    {
        // Set thread attribute list's Pseudo Console to the specified ConPTY
        fSuccess = UpdateProcThreadAttribute(
            pStartupInfo->lpAttributeList,
            0,
            PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
            hPC,
            sizeof(HPCON),
            NULL,
            NULL);
        return fSuccess ? S_OK : HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
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

int main()
{
    HANDLE hStdIn;
    HANDLE hStdOut;
    HANDLE hConPTYIn;
    HANDLE hConPTYOut;

    HPCON hPC;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD size = GetConsoleSize(hConsole);

    // Create the pipes to which the ConPTY will connect
    CreatePipe(&hConPTYIn, &hStdOut, NULL, 0);
    CreatePipe(&hStdIn, &hConPTYOut, NULL, 0);

    // Create ConPTY
    CreatePseudoConsole(
        size,
        hConPTYIn,
        hConPTYOut,
        0,
        &hPC);

    // Initialize the necessary startup info struct
    STARTUPINFOEX startupInfo{};
    if (S_OK == InitializeStartupInfoAttachedToConPTY(&startupInfo, hPC))
    {

        // Launch ping to echo some text back
        wchar_t szCommand[] = L"ping 8.8.8.8";

        wprintf(L"Executing Command: `'%s`", szCommand);

        PROCESS_INFORMATION piClient;
        BOOL fSuccess = CreateProcessW(
            nullptr,
            szCommand,
            nullptr,
            nullptr,
            TRUE,
            EXTENDED_STARTUPINFO_PRESENT,
            nullptr,
            nullptr,
            &startupInfo.StartupInfo,
            &piClient);

        //  Wait until the user hits a hey before quitting
        wprintf(L"\r\n ---> Press a key to quit <--- \r\n");
        do
        {
            Sleep(500);
        } while (!_kbhit());
        wprintf(L"\r\n ---> Terminating <--- \r\n");

        // Cleanup.
        ClosePseudoConsole(hPC);

        CloseHandle(hConPTYIn);
        CloseHandle(hStdOut);
        CloseHandle(hStdIn);
        CloseHandle(hConPTYOut);

        DeleteProcThreadAttributeList(startupInfo.lpAttributeList);
    }

    return S_OK;
}

