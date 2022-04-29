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
    //定义socket 套接字
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //定义sockaddr_in
    struct sockaddr_in server_sockaddr;
    //TCP/IP协议
    server_sockaddr.sin_family = AF_INET;
    //端口号
    server_sockaddr.sin_port = htons(8023);
    //本机ip 127.0.0.1
    server_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //bind 给套接字绑定地址
    if (bind(server_sockfd, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) == -1)
    {
        perror("bind");//输出错误原因，可以try catch
        exit(1);//结束程序
    }

    //listen，将创建好的socket套接字设置为监听状态
    if (listen(server_sockfd, 20) == -1)
    {
        perror("listen");//输出错误原因
        exit(1);//结束程序
    }

    //客户端套接字
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

    //accept 从已经完成连接的队列中取出连接
    int conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
    if (conn < 0)
    {
        perror("connect");//输出错误原因
        exit(1);//结束程序
    }
    cout << "客户端成功连接\n";

    //接收缓冲区
    char buffer[1000];

    //接收数据
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int len = recv(conn, buffer, sizeof(buffer), 0);
        //当客户端发送exit或者异常结束时，退出
        if (strcmp(buffer, "exit") == 0 || len <= 0)
            break;
        cout << "收到客户端信息：" << buffer << endl;
    }
    //撤销套接字
    close(conn);
    
    close(server_sockfd);
    return 0;
}