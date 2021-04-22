#pragma once

#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>

typedef struct _IPC_PIPE
{
    HANDLE hParentWrite;
    HANDLE hChildRead;
    HANDLE hParentRead;
    HANDLE hChildWrite;
} IPC_PIPE, *PIPC_PIPE;

int ConnectShell(char *path, SOCKET s, HANDLE hEvent);
HANDLE ProcessStart(char *path, IPC_PIPE *IpcPipe);