#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <iostream>
using namespace std;

int main()
{
    //定义socket套接字
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);

    //定义
    struct sockaddr_in servaddr;

    memset(&servaddr, 0, sizeof(servaddr));
    //TCP/IP协议族
    servaddr.sin_family = AF_INET;
    //服务器端口
    servaddr.sin_port = htons(8023);  
    //服务器ip
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");  

    //连接服务器，成功返回0，错误返回-1
    if (connect(sock_cli, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }
    cout << "连接服务器成功！\n";

    //定义接收和发送缓冲区
    char sendbuf[100];
    char recvbuf[100];
    while (true)
    {
        memset(sendbuf, 0, sizeof(sendbuf));
        cin >> sendbuf;
        send(sock_cli, sendbuf, strlen(sendbuf), 0); //发送
        if (strcmp(sendbuf, "exit") == 0)
            break;
    }
    close(sock_cli);
    return 0;
}