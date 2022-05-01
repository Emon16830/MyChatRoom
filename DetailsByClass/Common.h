#ifndef COMMON_H
#define COMMON_H
using namespace std;

#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//默认服务器端的IP地址
#define SERVER_IP "127.0.0.1"

//服务器端口号
#define SERVER_PORT 8888

//int epoll_create(int size)中的size
//为epoll支持的最大句柄数
#define EPOLL_SIZE 5000

//缓冲区的大小65535
#define BUF_SIZE 0xFFFF

//新用户登陆后的欢迎信息
#define SERVER_WELCOME "Welcome to join the chatroom,Your ID is: Client #%d"

//其他用户收到的前缀
#define SERVER_MESSAGE "ClientID %d say >> %s"

//退出系统
#define EXIT "EXIT"

#define CAUTION "There is only one int the chatroom!"

//注册新的 fd 到 epollfd 中
//参数enable_et表示是否启用ET模式，如果为 True 则启用，否则使用LT模式
static void addfd(int epollfd, int fd, bool enable_et) {
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	if (enable_et) {
		ev.events = EPOLLIN | EPOLLET;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	//设置 socket 为 nonblocking 模式
	// 执行完就转向下一条指令，不管函数有没有返回
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
	cout << "fd added to epoll" << endl;
}
#endif // !COMMON_H
